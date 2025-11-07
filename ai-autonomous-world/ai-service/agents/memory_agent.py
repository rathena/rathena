"""
Memory Agent - Manages long-term memory using Memori SDK
Handles storage, retrieval, and contextualization of NPC memories
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from loguru import logger

from crewai import Agent
try:
    from ai_service.agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse

try:
    # Memori is used in _store_with_memori and _retrieve_with_memori methods
    from memori import Memori  # noqa: F401
    MEMORI_AVAILABLE = True
except ImportError:
    logger.warning("Memori SDK not available, memory features will be limited")
    MEMORI_AVAILABLE = False


class MemoryAgent(BaseAIAgent):
    """
    Specialized agent for NPC memory management
    
    Responsibilities:
    - Store significant interactions and events
    - Retrieve relevant memories for context
    - Manage relationship tracking
    - Consolidate and summarize memories
    - Forget less important memories over time
    """
    
    def __init__(
        self,
        agent_id: str,
        llm_provider: Any,
        config: Dict[str, Any],
        memori_client: Optional[Any] = None
    ):
        """Initialize Memory Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="memory",
            llm_provider=llm_provider,
            config=config
        )
        
        self.memori_client = memori_client
        self.use_memori = MEMORI_AVAILABLE and memori_client is not None
        
        if self.use_memori:
            logger.info(f"Memory Agent {agent_id} initialized with Memori SDK")
        else:
            logger.info(f"Memory Agent {agent_id} initialized with basic memory (Memori SDK not available)")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for memory management"""
        return Agent(
            role="NPC Memory Curator",
            goal="Manage NPC memories to create consistent, evolving character experiences",
            backstory="""You are an expert in memory and cognition. You understand what experiences 
            are significant and worth remembering, how memories fade over time, and how past experiences 
            shape current behavior. You excel at helping NPCs maintain consistent personalities while 
            allowing them to grow and change based on their experiences.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False
        )
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Process memory operations (store or retrieve)
        
        Args:
            context: AgentContext with operation type and data
            
        Returns:
            AgentResponse with memory results
        """
        try:
            operation = context.current_state.get("operation", "retrieve")
            
            if operation == "store":
                result = await self._store_memory(context)
            elif operation == "retrieve":
                result = await self._retrieve_memories(context)
            elif operation == "update_relationship":
                result = await self._update_relationship(context)
            else:
                raise ValueError(f"Unknown memory operation: {operation}")
            
            logger.info(f"Memory operation '{operation}' completed for {context.npc_id}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result,
                confidence=0.90,
                reasoning=f"Memory {operation} operation completed successfully"
            )
            
        except Exception as e:
            logger.error(f"Memory operation failed for {context.npc_id}: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error during memory operation: {e}"
            )
    
    async def _store_memory(self, context: AgentContext) -> Dict[str, Any]:
        """Store a new memory"""
        memory_data = context.current_state.get("memory_data", {})
        
        memory_entry = {
            "npc_id": context.npc_id,
            "timestamp": datetime.utcnow().isoformat(),
            "type": memory_data.get("type", "interaction"),
            "content": memory_data.get("content", ""),
            "participants": memory_data.get("participants", []),
            "location": memory_data.get("location", {}),
            "importance": memory_data.get("importance", 5),  # 1-10 scale
            "emotional_valence": memory_data.get("emotional_valence", 0)  # -1 to 1
        }
        
        if self.use_memori:
            # Store using Memori SDK
            memory_id = await self._store_with_memori(memory_entry)
            logger.info(f"Memory stored with Memori SDK: {memory_id}")
        else:
            # Store in DragonflyDB as fallback
            memory_id = await self._store_in_db(memory_entry)
            logger.info(f"Memory stored in database: {memory_id}")
        
        return {
            "memory_id": memory_id,
            "stored": True,
            "timestamp": memory_entry["timestamp"]
        }
    
    async def _retrieve_memories(self, context: AgentContext) -> Dict[str, Any]:
        """Retrieve relevant memories"""
        query = context.current_state.get("query", "")
        player_id = context.current_state.get("player_id")
        limit = context.current_state.get("limit", 5)
        
        if self.use_memori:
            memories = await self._retrieve_with_memori(
                npc_id=context.npc_id,
                query=query,
                player_id=player_id,
                limit=limit
            )
        else:
            memories = await self._retrieve_from_db(
                npc_id=context.npc_id,
                player_id=player_id,
                limit=limit
            )
        
        # Summarize memories for context
        summary = await self._summarize_memories(memories)
        
        return {
            "memories": memories,
            "summary": summary,
            "count": len(memories)
        }

    async def _update_relationship(self, context: AgentContext) -> Dict[str, Any]:
        """Update relationship with a player"""
        player_id = context.current_state.get("player_id")
        change = context.current_state.get("relationship_change", 0)

        if not player_id:
            raise ValueError("player_id required for relationship update")

        # Get current relationship level from database
        from database import db

        relationship_key = f"relationship:{context.npc_id}:{player_id}"
        current_level = await db.redis.get(relationship_key)
        current_level = int(current_level) if current_level else 0

        # Update relationship
        new_level = max(-100, min(100, current_level + change))
        await db.redis.set(relationship_key, new_level)

        logger.info(f"Relationship updated: {context.npc_id} -> {player_id}: {current_level} -> {new_level}")

        return {
            "player_id": player_id,
            "previous_level": current_level,
            "new_level": new_level,
            "change": change
        }

    async def _store_with_memori(self, memory_entry: Dict[str, Any]) -> str:
        """Store memory using Memori SDK"""
        logger.info(f"Storing memory with Memori SDK for NPC {memory_entry['npc_id']}")

        try:
            # Generate memory ID
            memory_id = f"mem_{memory_entry['npc_id']}_{int(datetime.utcnow().timestamp())}"

            # Prepare memory content
            content = memory_entry.get('content', '')

            # Prepare metadata for Memori SDK
            metadata = {
                'memory_id': memory_id,
                'npc_id': memory_entry.get('npc_id'),
                'player_id': memory_entry.get('player_id'),
                'memory_type': memory_entry.get('memory_type', 'interaction'),
                'importance': memory_entry.get('importance', 0.5),
                'emotional_valence': memory_entry.get('emotional_valence', 0.0),
                'timestamp': memory_entry.get('timestamp', datetime.utcnow().isoformat()),
                'location': memory_entry.get('location'),
                'tags': memory_entry.get('tags', [])
            }

            # Store using Memori SDK
            # The add() method returns a memory_id
            stored_id = self.memori_client.add(
                text=content,
                metadata=metadata
            )

            logger.info(f"Memory stored successfully with Memori SDK: {stored_id}")
            return stored_id

        except Exception as e:
            logger.error(f"Failed to store memory with Memori SDK: {e}")
            # Fallback to database storage
            logger.info("Falling back to database storage")
            return await self._store_in_db(memory_entry)

    async def _store_in_db(self, memory_entry: Dict[str, Any]) -> str:
        """Store memory in DragonflyDB as fallback"""
        from database import db
        import json

        memory_id = f"mem_{memory_entry['npc_id']}_{int(datetime.utcnow().timestamp())}"
        memory_key = f"memory:{memory_entry['npc_id']}:{memory_id}"

        await db.redis.set(memory_key, json.dumps(memory_entry))

        # Add to sorted set for time-based retrieval
        memories_set = f"memories:{memory_entry['npc_id']}"
        timestamp = datetime.utcnow().timestamp()
        await db.redis.zadd(memories_set, {memory_id: timestamp})

        return memory_id

    async def _retrieve_with_memori(
        self,
        npc_id: str,
        query: str,
        player_id: Optional[str],
        limit: int
    ) -> List[Dict[str, Any]]:
        """Retrieve memories using Memori SDK"""
        logger.info(f"Retrieving memories with Memori SDK for NPC {npc_id}, query: {query}")

        try:
            # Build search query that includes NPC context
            search_query = f"NPC {npc_id}: {query}"
            if player_id:
                search_query += f" player {player_id}"

            # Search using Memori SDK
            # The search() method returns a list of memory dictionaries
            raw_memories = self.memori_client.search(
                query=search_query,
                limit=limit
            )

            # Transform Memori SDK results to our expected format
            memories = []
            for mem in raw_memories:
                # Extract metadata from Memori SDK result
                metadata = mem.get('metadata', {})

                # Filter by NPC ID if metadata contains it
                if metadata.get('npc_id') == npc_id:
                    # Filter by player ID if specified
                    if player_id is None or metadata.get('player_id') == player_id:
                        memory_entry = {
                            'memory_id': metadata.get('memory_id', mem.get('memory_id')),
                            'npc_id': metadata.get('npc_id'),
                            'player_id': metadata.get('player_id'),
                            'content': mem.get('searchable_content', mem.get('summary', '')),
                            'memory_type': metadata.get('memory_type', 'interaction'),
                            'importance': metadata.get('importance', 0.5),
                            'emotional_valence': metadata.get('emotional_valence', 0.0),
                            'timestamp': metadata.get('timestamp', mem.get('created_at')),
                            'location': metadata.get('location'),
                            'tags': metadata.get('tags', [])
                        }
                        memories.append(memory_entry)

            logger.info(f"Retrieved {len(memories)} memories from Memori SDK")
            return memories

        except Exception as e:
            logger.error(f"Failed to retrieve memories with Memori SDK: {e}")
            # Fallback to database
            logger.info("Falling back to database retrieval")
            return await self._retrieve_from_db(npc_id, player_id, limit)

    async def _retrieve_from_db(
        self,
        npc_id: str,
        player_id: Optional[str],
        limit: int
    ) -> List[Dict[str, Any]]:
        """Retrieve memories from DragonflyDB"""
        from database import db
        import json

        memories_set = f"memories:{npc_id}"

        # Get most recent memory IDs
        memory_ids = await db.redis.zrevrange(memories_set, 0, limit - 1)

        memories = []
        for memory_id in memory_ids:
            memory_key = f"memory:{npc_id}:{memory_id}"
            memory_data = await db.redis.get(memory_key)

            if memory_data:
                memory = json.loads(memory_data)

                # Filter by player_id if specified
                if player_id is None or player_id in memory.get("participants", []):
                    memories.append(memory)

        return memories[:limit]

    async def _summarize_memories(self, memories: List[Dict[str, Any]]) -> str:
        """Summarize memories for context"""
        if not memories:
            return "No relevant memories found."

        # Build summary
        summary_parts = []

        for memory in memories[:3]:  # Summarize top 3 memories
            content = memory.get("content", "")
            timestamp = memory.get("timestamp", "")
            memory_type = memory.get("type", "interaction")

            summary_parts.append(f"{memory_type}: {content} (at {timestamp})")

        return "; ".join(summary_parts)

