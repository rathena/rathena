"""
Agent Orchestrator - Coordinates multiple agents using CrewAI
Manages agent collaboration and task delegation
"""

from typing import Dict, Any, List, Optional
from loguru import logger

from crewai import Crew, Process
try:
    from ai_service.agents.base_agent import AgentContext, AgentResponse
    from ai_service.agents.dialogue_agent import DialogueAgent
    from ai_service.agents.decision_agent import DecisionAgent
    from ai_service.agents.memory_agent import MemoryAgent
    from ai_service.agents.world_agent import WorldAgent
except ModuleNotFoundError:
    from agents.base_agent import AgentContext, AgentResponse
    from agents.dialogue_agent import DialogueAgent
    from agents.decision_agent import DecisionAgent
    from agents.memory_agent import MemoryAgent
    from agents.world_agent import WorldAgent


class AgentOrchestrator:
    """
    Orchestrates multiple AI agents to handle complex NPC behaviors
    
    Uses CrewAI to coordinate:
    - Dialogue Agent: Generates NPC dialogue
    - Decision Agent: Makes action decisions
    - Memory Agent: Manages long-term memory
    - World Agent: Analyzes world state
    """
    
    def __init__(
        self,
        llm_provider: Any,
        config: Dict[str, Any],
        memori_client: Optional[Any] = None
    ):
        """
        Initialize Agent Orchestrator
        
        Args:
            llm_provider: LLM provider instance
            config: Configuration dictionary
            memori_client: Optional Memori SDK client
        """
        self.llm_provider = llm_provider
        self.config = config
        self.memori_client = memori_client
        
        # Initialize agents
        self.dialogue_agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=llm_provider,
            config=config.get("dialogue_agent", {})
        )
        
        self.decision_agent = DecisionAgent(
            agent_id="decision_001",
            llm_provider=llm_provider,
            config=config.get("decision_agent", {})
        )
        
        self.memory_agent = MemoryAgent(
            agent_id="memory_001",
            llm_provider=llm_provider,
            config=config.get("memory_agent", {}),
            memori_client=memori_client
        )
        
        self.world_agent = WorldAgent(
            agent_id="world_001",
            llm_provider=llm_provider,
            config=config.get("world_agent", {})
        )
        
        logger.info("Agent Orchestrator initialized with 4 specialized agents")
    
    async def handle_player_interaction(
        self,
        npc_context: AgentContext,
        player_message: str,
        interaction_type: str = "talk"
    ) -> Dict[str, Any]:
        """
        Handle player-NPC interaction using multiple agents
        
        Args:
            npc_context: Context for the NPC
            player_message: Message from player
            interaction_type: Type of interaction
            
        Returns:
            Complete interaction response
        """
        logger.info(f"Orchestrating player interaction for NPC: {npc_context.npc_id}")
        
        try:
            # Step 1: Retrieve relevant memories
            memory_context = npc_context.current_state.copy()
            memory_context["operation"] = "retrieve"
            memory_context["player_id"] = npc_context.current_state.get("player_id")
            memory_context["query"] = player_message

            try:
                memory_response = await self.memory_agent.process(
                    AgentContext(
                        npc_id=npc_context.npc_id,
                        npc_name=npc_context.npc_name,
                        personality=npc_context.personality,
                        current_state=memory_context,
                        world_state=npc_context.world_state,
                        recent_events=npc_context.recent_events
                    )
                )
            except Exception as e:
                logger.error(f"Memory retrieval failed for NPC {npc_context.npc_id}: {e}")
                # Create empty memory response to continue
                memory_response = AgentResponse(
                    success=False,
                    data={},
                    metadata={"error": str(e)}
                )
            
            # Step 2: Generate dialogue with memory context
            dialogue_context = npc_context.current_state.copy()
            dialogue_context["player_message"] = player_message
            dialogue_context["interaction_type"] = interaction_type

            if memory_response.success:
                npc_context.memory_context = memory_response.data

            try:
                dialogue_response = await self.dialogue_agent.process(npc_context)
            except Exception as e:
                logger.error(f"Dialogue generation failed for NPC {npc_context.npc_id}: {e}")
                return AgentResponse(
                    success=False,
                    data={"error": "Failed to generate dialogue"},
                    metadata={"error_details": str(e)}
                )

            # Step 3: Store this interaction as a memory
            if dialogue_response.success:
                store_context = {
                    "operation": "store",
                    "memory_data": {
                        "type": "interaction",
                        "content": f"Player said: '{player_message}'. I responded: '{dialogue_response.data.get('text', '')}'",
                        "participants": [npc_context.current_state.get("player_id", "unknown")],
                        "location": npc_context.current_state.get("location", {}),
                        "importance": 5,
                        "emotional_valence": 0.5 if dialogue_response.data.get("emotion") == "friendly" else 0.0
                    }
                }

                try:
                    await self.memory_agent.process(
                        AgentContext(
                            npc_id=npc_context.npc_id,
                            npc_name=npc_context.npc_name,
                            personality=npc_context.personality,
                            current_state=store_context,
                            world_state=npc_context.world_state,
                            recent_events=npc_context.recent_events
                        )
                    )
                except Exception as e:
                    logger.warning(f"Failed to store interaction memory for NPC {npc_context.npc_id}: {e}")
            
            # Step 4: Update relationship
            relationship_change = 1 if dialogue_response.success else 0
            relationship_context = {
                "operation": "update_relationship",
                "player_id": npc_context.current_state.get("player_id"),
                "relationship_change": relationship_change
            }
            
            relationship_response = await self.memory_agent.process(
                AgentContext(
                    npc_id=npc_context.npc_id,
                    npc_name=npc_context.npc_name,
                    personality=npc_context.personality,
                    current_state=relationship_context,
                    world_state=npc_context.world_state,
                    recent_events=npc_context.recent_events
                )
            )
            
            return {
                "dialogue": dialogue_response.data if dialogue_response.success else {"text": "..."},
                "relationship_change": relationship_response.data if relationship_response.success else {},
                "memory_stored": True
            }
            
        except Exception as e:
            logger.error(f"Orchestration failed for player interaction: {e}")
            return {
                "dialogue": {"text": "I'm not sure what to say...", "emotion": "confused"},
                "relationship_change": {},
                "memory_stored": False,
                "error": str(e)
            }

    async def handle_npc_action_decision(
        self,
        npc_context: AgentContext
    ) -> Dict[str, Any]:
        """
        Determine next action for NPC using Decision and World agents

        Args:
            npc_context: Context for the NPC

        Returns:
            Action decision
        """
        logger.info(f"Orchestrating action decision for NPC: {npc_context.npc_id}")

        try:
            # Step 1: Analyze world state impact
            world_context = npc_context.current_state.copy()
            world_context["operation"] = "impact"
            world_context["npc_class"] = npc_context.current_state.get("npc_class", "generic")

            world_response = await self.world_agent.process(
                AgentContext(
                    npc_id=npc_context.npc_id,
                    npc_name=npc_context.npc_name,
                    personality=npc_context.personality,
                    current_state=world_context,
                    world_state=npc_context.world_state,
                    recent_events=npc_context.recent_events
                )
            )

            # Step 2: Make decision considering world impact
            if world_response.success and world_response.data.get("affected"):
                # Add world impact to context
                npc_context.current_state["world_impact"] = world_response.data
                npc_context.current_state["mood_modifier"] = world_response.data.get("mood_modifier", 0.0)

            decision_response = await self.decision_agent.process(npc_context)

            return {
                "action": decision_response.data if decision_response.success else {"action_type": "idle"},
                "world_impact": world_response.data if world_response.success else {},
                "confidence": decision_response.confidence
            }

        except Exception as e:
            logger.error(f"Orchestration failed for action decision: {e}")
            return {
                "action": {"action_type": "idle", "action_data": {}, "priority": 1},
                "world_impact": {},
                "confidence": 0.5,
                "error": str(e)
            }

    async def process_world_event(
        self,
        event_data: Dict[str, Any],
        affected_npcs: List[str]
    ) -> Dict[str, Any]:
        """
        Process a world event and determine NPC reactions

        Args:
            event_data: Event information
            affected_npcs: List of NPC IDs affected by event

        Returns:
            Event processing results
        """
        logger.info(f"Processing world event: {event_data.get('type', 'unknown')}")

        try:
            # Analyze event using World Agent
            world_context = {
                "operation": "analyze",
                "event": event_data
            }

            # Create minimal context for world analysis
            analysis_context = AgentContext(
                npc_id="world_system",
                npc_name="World System",
                personality=None,
                current_state=world_context,
                world_state=event_data.get("world_state", {}),
                recent_events=[event_data]
            )

            world_response = await self.world_agent.process(analysis_context)

            # Store event in memory for affected NPCs
            npc_reactions = []
            for npc_id in affected_npcs:
                memory_context = {
                    "operation": "store",
                    "memory_data": {
                        "type": "world_event",
                        "content": event_data.get("description", "A world event occurred"),
                        "participants": [],
                        "location": {},
                        "importance": event_data.get("severity", 5),
                        "emotional_valence": -0.5 if event_data.get("severity", 5) > 7 else 0.0
                    }
                }

                # Store memory for this NPC
                npc_memory_context = AgentContext(
                    npc_id=npc_id,
                    npc_name=f"NPC_{npc_id}",
                    personality=None,
                    current_state=memory_context,
                    world_state={},
                    recent_events=[]
                )

                memory_response = await self.memory_agent.process(npc_memory_context)
                npc_reactions.append({
                    "npc_id": npc_id,
                    "memory_stored": memory_response.success
                })

            return {
                "event_analyzed": world_response.success,
                "analysis": world_response.data if world_response.success else {},
                "npc_reactions": npc_reactions,
                "affected_count": len(affected_npcs)
            }

        except Exception as e:
            logger.error(f"World event processing failed: {e}")
            return {
                "event_analyzed": False,
                "analysis": {},
                "npc_reactions": [],
                "affected_count": 0,
                "error": str(e)
            }

    def create_crew_for_task(self, task_type: str) -> Crew:
        """
        Create a CrewAI Crew for specific task type

        Args:
            task_type: Type of task (dialogue, decision, complex)

        Returns:
            Configured Crew instance
        """
        if task_type == "dialogue":
            agents = [self.dialogue_agent.crew_agent, self.memory_agent.crew_agent]
        elif task_type == "decision":
            agents = [self.decision_agent.crew_agent, self.world_agent.crew_agent]
        elif task_type == "complex":
            agents = [
                self.dialogue_agent.crew_agent,
                self.decision_agent.crew_agent,
                self.memory_agent.crew_agent,
                self.world_agent.crew_agent
            ]
        else:
            agents = [self.dialogue_agent.crew_agent]

        crew = Crew(
            agents=agents,
            process=Process.sequential,
            verbose=self.config.get("verbose", False)
        )

        logger.info(f"Created crew for task type: {task_type} with {len(agents)} agents")

        return crew

