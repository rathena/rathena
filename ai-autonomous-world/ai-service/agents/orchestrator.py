"""
Agent Orchestrator - Coordinates multiple agents using CrewAI
Manages agent collaboration and task delegation
"""

from datetime import datetime
from typing import Dict, Any, List, Optional
from loguru import logger

from crewai import Crew, Process
from agents.base_agent import AgentContext, AgentResponse
from agents.dialogue_agent import DialogueAgent
from agents.decision_agent import DecisionAgent
from agents.memory_agent import MemoryAgent
from agents.world_agent import WorldAgent

class AgentOrchestrator:
    """
    Orchestrates multiple AI agents to handle complex NPC interactions
    Uses CrewAI for agent coordination and task management
    """
    def __init__(
        self,
        config: Dict[str, Any],
        npc_context: Optional[Dict[str, Any]] = None
    ):
        """
        Initialize the orchestrator with configuration and optional NPC context

        Args:
            config: Configuration dictionary with agent settings
            npc_context: Optional NPC context for initial state
        """
        self.config = config
        self.npc_context = npc_context or {}

        # Initialize agents with provided LLM provider or get default
        from llm.factory import get_llm_provider
        self.llm_provider = config.get("llm_provider") or get_llm_provider()
        self.dialogue_agent = DialogueAgent("dialogue_agent", self.llm_provider, config.get("dialogue", {}))
        self.decision_agent = DecisionAgent("decision_agent", self.llm_provider, config.get("decision", {}))
        self.memory_agent = MemoryAgent("memory_agent", self.llm_provider, config.get("memory", {}))
        self.world_agent = WorldAgent("world_agent", self.llm_provider, config.get("world", {}))

        logger.info("AgentOrchestrator initialized with {} agents", 4)

    async def handle_player_interaction(
        self,
        npc_context: AgentContext,
        interaction_type: str,
        message: str = "",
    ) -> AgentResponse:
        """
        Handle player interaction by coordinating multiple agents

        Args:
            npc_context: AgentContext for the NPC
            interaction_type: Type of interaction (e.g., "dialogue", "trade")
            message: Player's input text

        Returns:
            AgentResponse containing the generated response
        """
        try:
            # Store interaction in memory using the process method
            try:
                memory_context_obj = AgentContext(
                    npc_id=npc_context.npc_id,
                    npc_name=npc_context.npc_name,
                    personality=npc_context.personality,
                    goal_state=getattr(npc_context, "goal_state", None),
                    emotion_state=getattr(npc_context, "emotion_state", None),
                    memory_state=getattr(npc_context, "memory_state", None),
                    current_state={
                        "operation": "store",
                        "memory_data": {
                            "player_input": message,
                            "timestamp": datetime.now().isoformat(),
                            "interaction_type": interaction_type,
                        }
                    },
                    world_state=npc_context.world_state,
                    recent_events=npc_context.recent_events
                )
                await self.memory_agent.process(memory_context_obj)
            except Exception as e:
                logger.warning("Failed to store interaction in memory: {}", e)

            # Create crew for dialogue generation
            crew = self.create_crew_for_task("dialogue")

            # Execute the crew to generate response
            try:
                npc_context_dict = {
                    "npc_id": getattr(npc_context, "npc_id", "unknown"),
                    "npc_name": getattr(npc_context, "npc_name", "Unknown NPC"),
                    "personality": getattr(npc_context, "personality", {}),
                    "goal_state": getattr(npc_context, "goal_state", None),
                    "emotion_state": getattr(npc_context, "emotion_state", None),
                    "memory_state": getattr(npc_context, "memory_state", None),
                    "current_state": getattr(npc_context, "current_state", {}),
                    "world_state": getattr(npc_context, "world_state", {}),
                    "recent_events": getattr(npc_context, "recent_events", [])
                }

                result = await crew.kickoff(inputs={
                    "player_input": message,
                    "npc_context": npc_context_dict,
                    "interaction_type": interaction_type,
                })

                response = AgentResponse(
                    agent_type="dialogue",
                    success=True,
                    data={
                        "text": result.get("response", "I'm not sure how to respond to that."),
                        "emotional_state": result.get("emotional_state", "neutral"),
                        "actions": result.get("actions", [])
                    },
                    confidence=0.8,
                    metadata=result.get("metadata", {})
                )

                return response

            except Exception as e:
                logger.error("Crew execution failed: {}", e)
                # Fallback to direct dialogue agent
                fallback_response = await self.dialogue_agent.process(
                    AgentContext(
                        npc_id=getattr(npc_context, "npc_id", "unknown"),
                        npc_name=getattr(npc_context, "npc_name", "Unknown NPC"),
                        personality=getattr(npc_context, "personality", {}),
                        goal_state=getattr(npc_context, "goal_state", None),
                        emotion_state=getattr(npc_context, "emotion_state", None),
                        memory_state=getattr(npc_context, "memory_state", None),
                        current_state={"player_message": message},
                        world_state=getattr(npc_context, "world_state", {}),
                        recent_events=getattr(npc_context, "recent_events", [])
                    )
                )
                return AgentResponse(
                    agent_type="dialogue",
                    success=True,
                    data={
                        "text": fallback_response.data.get("text", "I'm not sure how to respond to that."),
                        "emotional_state": fallback_response.data.get("emotional_state", "neutral"),
                        "actions": fallback_response.data.get("actions", [])
                    },
                    confidence=0.5,
                    metadata={"fallback": True}
                )

        except Exception as e:
            logger.error("Error in player interaction handling: {}", e)
            return AgentResponse(
                agent_type="dialogue",
                success=False,
                data={"text": "I'm having trouble processing that right now. Could you try again?"},
                confidence=0.0,
                reasoning=f"Error: {str(e)}"
            )

    async def handle_npc_action_decision(
        self,
        agent_context: AgentContext
    ) -> Dict[str, Any]:
        """
        Make decisions about NPC actions based on world state

        Args:
            agent_context: AgentContext for the NPC

        Returns:
            Dictionary containing decided actions and their parameters
        """
        try:
            crew = self.create_crew_for_task("decision")
            result = await crew.kickoff(inputs={
                "world_state": agent_context.world_state,
                "npc_context": {
                    "npc_id": agent_context.npc_id,
                    "npc_name": agent_context.npc_name,
                    "personality": agent_context.personality,
                    "goal_state": getattr(agent_context, "goal_state", None),
                    "emotion_state": getattr(agent_context, "emotion_state", None),
                    "memory_state": getattr(agent_context, "memory_state", None),
                    "current_state": agent_context.current_state,
                    "recent_events": agent_context.recent_events,
                }
            })
            return result.get("actions", {})
        except Exception as e:
            logger.error("Error in NPC action decision: {}", e)
            return {"move": "stay", "interact": False}

    async def process_world_event(
        self,
        event_type: str,
        event_data: Dict[str, Any],
        affected_npcs: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """
        Process world events and coordinate NPC responses

        Args:
            event_type: Type of world event
            event_data: Event-specific data
            affected_npcs: List of NPCs affected by the event

        Returns:
            Dictionary with NPC responses and world changes
        """
        try:
            crew = self.create_crew_for_task("world_event")
            result = await crew.kickoff(inputs={
                "event_type": event_type,
                "event_data": event_data,
                "affected_npcs": affected_npcs
            })
            return result
        except Exception as e:
            logger.error("Error processing world event: {}", e)
            return {"responses": {}, "world_changes": {}}

    def create_crew_for_task(self, task_type: str) -> Crew:
        """
        Create a CrewAI crew for a specific task type

        Args:
            task_type: Type of task ("dialogue", "decision", "world_event")

        Returns:
            Configured Crew instance
        """
        tasks_config = {
            "dialogue": {
                "description": "Generate appropriate dialogue response for NPC",
                "expected_output": "Natural language response with emotional context"
            },
            "decision": {
                "description": "Decide NPC actions based on world state",
                "expected_output": "Action decisions with parameters"
            },
            "world_event": {
                "description": "Process world events and coordinate NPC responses",
                "expected_output": "NPC responses and world state changes"
            }
        }

        config = tasks_config.get(task_type, tasks_config["dialogue"])

        return Crew(
            agents=[
                self.dialogue_agent.crew_agent,
                self.decision_agent.crew_agent,
                self.memory_agent.crew_agent,
                self.world_agent.crew_agent
            ],
            tasks=[{
                "description": config["description"],
                "expected_output": config["expected_output"],
                "agent": self.dialogue_agent.crew_agent
            }],
            process=Process.sequential,
            verbose=True
        )