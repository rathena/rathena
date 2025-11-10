"""
Memory Agent - Manages long-term memory using OpenMemory SDK
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


class MemoryAgent(BaseAIAgent):
    """
    Specialized agent for NPC memory management using OpenMemory SDK

    Responsibilities:
    - Store significant interactions and events in PostgreSQL
    - Retrieve relevant memories for context using vector similarity
    - Manage relationship tracking
    - Consolidate and summarize memories
    - Support multi-sector memory (episodic, semantic, procedural, emotional, reflective)
    """

    def __init__(
        self,
        agent_id: str,
        llm_provider: Any,
        config: Dict[str, Any],
        memori_client: Any = None
    ):
        """
        Initialize Memory Agent

        Args:
            agent_id: Unique identifier for this agent
            llm_provider: LLM provider instance
            config: Agent configuration
            memori_client: Deprecated parameter (kept for backward compatibility)

        Note:
            Memory storage now uses OpenMemory SDK (accessed globally via get_openmemory_manager())
        """
        super().__init__(
            agent_id=agent_id,
            agent_type="memory",
            llm_provider=llm_provider,
            config=config
        )

        # OpenMemory SDK is accessed globally via get_openmemory_manager()
        # The memori_client parameter is deprecated but kept for backward compatibility
        logger.info(f"Memory Agent {agent_id} initialized with OpenMemory SDK")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for memory management"""
        # Import CrewAI's LLM class
        from crewai import LLM
        import os

        # Create CrewAI-compatible LLM using litellm format for Azure OpenAI
        try:
            from config import settings

            # Set Azure OpenAI environment variables for litellm
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version

            # Use litellm format: azure/<deployment_name>
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.7,
                max_tokens=2000
            )
            logger.info(f"Created Azure OpenAI LLM with deployment: {settings.azure_openai_deployment}")
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            raise

        return Agent(
            role="NPC Memory Curator",
            goal="Manage NPC memories to create consistent, evolving character experiences",
            backstory="""You are an expert in memory and cognition. You understand what experiences
            are significant and worth remembering, how memories fade over time, and how past experiences
            shape current behavior. You excel at helping NPCs maintain consistent personalities while
            allowing them to grow and change based on their experiences.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
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
        
        # Store using OpenMemory SDK
        memory_id = await self._store_with_openmemory(memory_entry)
        logger.info(f"Memory stored with OpenMemory SDK: {memory_id}")
        
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
        
        # Retrieve using OpenMemory SDK
        memories = await self._retrieve_with_openmemory(
            npc_id=context.npc_id,
            query=query,
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

    async def _store_with_openmemory(self, memory_entry: Dict[str, Any]) -> str:
        """Store memory using OpenMemory SDK"""
        logger.info(f"Storing memory with OpenMemory SDK for NPC {memory_entry['npc_id']}")

        try:
            # Get OpenMemory manager
            from ai_service.memory.openmemory_manager import get_openmemory_manager
            om_manager = get_openmemory_manager()

            if not om_manager or not om_manager.is_available():
                logger.warning("OpenMemory SDK not available, skipping memory storage")
                return "memory_skipped"

            # Prepare memory content
            content = memory_entry.get('content', '')

            # Prepare metadata for OpenMemory SDK
            metadata = {
                'npc_id': memory_entry.get('npc_id'),
                'player_id': memory_entry.get('player_id'),
                'memory_type': memory_entry.get('memory_type', 'interaction'),
                'importance': memory_entry.get('importance', 0.5),
                'emotional_valence': memory_entry.get('emotional_valence', 0.0),
                'timestamp': memory_entry.get('timestamp', datetime.utcnow().isoformat()),
                'location': memory_entry.get('location'),
            }

            # Prepare tags
            tags = memory_entry.get('tags', [])

            # Determine sector based on memory type
            memory_type = memory_entry.get('memory_type', 'interaction')
            if memory_type == 'interaction':
                metadata['sector'] = 'episodic'  # Event memories
            elif memory_type == 'fact':
                metadata['sector'] = 'semantic'  # Facts & preferences
            elif memory_type == 'habit':
                metadata['sector'] = 'procedural'  # Habits, triggers
            elif memory_type == 'emotion':
                metadata['sector'] = 'emotional'  # Sentiment states
            else:
                metadata['sector'] = 'reflective'  # Meta memory & logs

            # Get salience from importance
            salience = memory_entry.get('importance', 0.5)

            # Get user_id for isolation
            user_id = f"player_{memory_entry.get('player_id', 'unknown')}"

            # Store using OpenMemory SDK
            client = om_manager.get_client()
            result = client.add(
                content=content,
                tags=tags,
                metadata=metadata,
                salience=salience,
                user_id=user_id
            )

            memory_id = result.get('id', 'unknown')
            logger.info(f"Memory stored successfully with OpenMemory SDK: {memory_id}")
            return memory_id

        except Exception as e:
            logger.error(f"Failed to store memory with OpenMemory SDK: {e}", exc_info=True)
            logger.warning("Continuing without memory storage")
            return "memory_error"

    async def _retrieve_with_openmemory(
        self,
        npc_id: str,
        query: str,
        player_id: Optional[str],
        limit: int
    ) -> List[Dict[str, Any]]:
        """Retrieve memories using OpenMemory SDK"""
        logger.info(f"Retrieving memories with OpenMemory SDK for NPC {npc_id}, query: {query}")

        try:
            # Get OpenMemory manager
            from ai_service.memory.openmemory_manager import get_openmemory_manager
            om_manager = get_openmemory_manager()

            if not om_manager or not om_manager.is_available():
                logger.warning("OpenMemory SDK not available, returning empty memories")
                return []

            # Build search query that includes NPC context
            search_query = f"NPC {npc_id}: {query}"
            if player_id:
                search_query += f" player {player_id}"

            # Get user_id for filtering
            user_id = f"player_{player_id}" if player_id else None

            # Query using OpenMemory SDK
            client = om_manager.get_client()

            # Build filters
            filters = {}
            if user_id:
                filters['user_id'] = user_id

            result = client.query(
                query=search_query,
                k=limit,
                filters=filters if filters else None
            )

            # Transform OpenMemory SDK results to our expected format
            memories = []
            raw_memories = result.get('memories', [])

            for mem in raw_memories:
                # Extract metadata from OpenMemory SDK result
                metadata = mem.get('metadata', {})

                # Filter by NPC ID if metadata contains it
                if metadata.get('npc_id') == npc_id:
                    # Filter by player ID if specified
                    if player_id is None or metadata.get('player_id') == player_id:
                        memory_entry = {
                            'memory_id': mem.get('id', 'unknown'),
                            'npc_id': metadata.get('npc_id'),
                            'player_id': metadata.get('player_id'),
                            'content': mem.get('content', ''),
                            'memory_type': metadata.get('memory_type', 'interaction'),
                            'importance': metadata.get('importance', 0.5),
                            'emotional_valence': metadata.get('emotional_valence', 0.0),
                            'timestamp': metadata.get('timestamp', mem.get('created_at')),
                            'location': metadata.get('location'),
                            'tags': mem.get('tags', []),
                            'score': mem.get('score', 0.0)  # Similarity score
                        }
                        memories.append(memory_entry)

            logger.info(f"Retrieved {len(memories)} memories from OpenMemory SDK")
            return memories

        except Exception as e:
            logger.error(f"Failed to retrieve memories with OpenMemory SDK: {e}", exc_info=True)
            logger.warning("Returning empty memories due to error")
            return []

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

