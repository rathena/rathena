"""
Decision Agent - Makes decisions for NPC actions and behaviors
Determines what NPCs should do based on context, personality, and goals
"""

from typing import Dict, Any, List, Optional
from loguru import logger
import json

from crewai import Agent
try:
    from ai_service.agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from ai_service.config import settings
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from config import settings


class DecisionAgent(BaseAIAgent):
    """
    Specialized agent for NPC decision-making
    
    Responsibilities:
    - Decide NPC actions based on current state
    - Prioritize goals and objectives
    - Consider personality in decision-making
    - Evaluate risks and rewards
    - Plan short-term actions
    """
    
    def __init__(self, agent_id: str, llm_provider: Any, config: Dict[str, Any]):
        """Initialize Decision Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="decision",
            llm_provider=llm_provider,
            config=config
        )
        
        logger.info(f"Decision Agent {agent_id} initialized")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for decision-making"""
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
            role="NPC Decision Strategist",
            goal="Make intelligent, personality-consistent decisions for NPCs that create engaging gameplay",
            backstory="""You are an expert in behavioral psychology and game AI. You understand how to
            make NPCs behave in ways that feel authentic and purposeful. You consider personality traits,
            current goals, environmental factors, and past experiences when making decisions. You excel at
            creating believable NPC behavior that enhances player immersion.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Make decision for NPC action
        
        Args:
            context: AgentContext with NPC state and world information
            
        Returns:
            AgentResponse with decided action
        """
        try:
            logger.info(f"Decision Agent processing for NPC: {context.npc_id}")
            
            # Get available actions
            available_actions = self._get_available_actions(context)
            
            # Build decision context
            decision_context = self._build_decision_context(context)
            
            # Make decision using LLM
            decision = await self._make_decision(
                npc_name=context.npc_name,
                personality=self._build_personality_prompt(context.personality),
                available_actions=available_actions,
                decision_context=decision_context,
                recent_events=context.recent_events
            )
            
            # Parse and validate decision
            action_data = self._parse_decision(decision, available_actions, context)
            
            logger.info(f"Decision made for {context.npc_id}: {action_data.get('action_type')}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=action_data,
                confidence=0.80,
                reasoning=decision.get("reasoning", "Decision based on personality and context"),
                metadata={
                    "available_actions_count": len(available_actions),
                    "events_considered": len(context.recent_events)
                }
            )
            
        except Exception as e:
            logger.error(f"Decision-making failed for {context.npc_id}: {e}")
            
            # Fallback to idle action
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "action_type": "idle",
                    "action_data": {"duration": 5},
                    "priority": 1
                },
                confidence=0.5,
                reasoning=f"Fallback to idle due to error: {e}"
            )
    
    def _get_available_actions(self, context: AgentContext) -> List[Dict[str, Any]]:
        """Get list of available actions for NPC"""
        from ai_service.utils.movement_utils import can_npc_move, get_movement_capabilities

        npc_class = context.current_state.get("npc_class", "generic")
        sprite_id = context.current_state.get("sprite_id")

        # Get movement capabilities
        movement_caps = get_movement_capabilities(context.current_state)
        can_move = can_npc_move(
            npc_id=context.npc_id,
            npc_class=npc_class,
            sprite_id=sprite_id,
            movement_capabilities=movement_caps
        )

        # Base actions available to all NPCs
        base_actions = [
            {"type": "idle", "description": "Stand idle and observe surroundings"},
            {"type": "emote", "description": "Perform an emote or gesture"}
        ]

        # Add movement actions if NPC can move
        if can_move:
            movement_actions = [
                {"type": "wander", "description": "Walk around the area randomly"},
                {"type": "exploration", "description": "Explore nearby areas"},
                {"type": "return_home", "description": "Return to home/spawn position"},
            ]
            base_actions.extend(movement_actions)

            logger.debug(f"NPC {context.npc_id}: Movement actions enabled")
        else:
            logger.debug(f"NPC {context.npc_id}: Movement actions disabled")

        # Class-specific actions
        class_actions = {
            "merchant": [
                {"type": "advertise", "description": "Call out to attract customers"},
                {"type": "organize_inventory", "description": "Organize shop inventory"},
                {"type": "check_prices", "description": "Check market prices"}
            ],
            "guard": [
                {"type": "patrol", "description": "Patrol assigned area"} if can_move else None,
                {"type": "watch", "description": "Watch for threats"},
                {"type": "challenge", "description": "Challenge suspicious individuals"}
            ],
            "quest_giver": [
                {"type": "seek_adventurers", "description": "Look for capable adventurers"} if can_move else None,
                {"type": "review_quests", "description": "Review available quests"},
                {"type": "update_board", "description": "Update quest board"}
            ]
        }

        # Filter out None actions and add class-specific actions
        class_specific = [a for a in class_actions.get(npc_class, []) if a is not None]
        actions = base_actions + class_specific

        return actions
    
    def _build_decision_context(self, context: AgentContext) -> str:
        """Build context string for decision-making"""
        context_parts = []
        
        # Current state
        location = context.current_state.get("location", {})
        if location:
            context_parts.append(f"Location: {location.get('map', 'unknown')}")
        
        time_of_day = context.current_state.get("time_of_day", "day")
        context_parts.append(f"Time: {time_of_day}")
        
        # Current goals
        goals = context.current_state.get("goals", [])
        if goals:
            context_parts.append(f"Current goals: {', '.join(goals)}")
        
        # Recent activity
        last_action = context.current_state.get("last_action")
        if last_action:
            context_parts.append(f"Last action: {last_action}")
        
        return ". ".join(context_parts)

    async def _make_decision(
        self,
        npc_name: str,
        personality: str,
        available_actions: List[Dict[str, Any]],
        decision_context: str,
        recent_events: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """Make decision using LLM"""

        # Format available actions
        actions_list = "\n".join([
            f"- {action['type']}: {action['description']}"
            for action in available_actions
        ])

        # Format recent events
        events_summary = "No recent events."
        if recent_events:
            events_summary = "\n".join([
                f"- {event.get('event_type', 'unknown')}: {event.get('summary', 'no details')}"
                for event in recent_events[-5:]  # Last 5 events
            ])

        system_message = f"""You are making decisions for {npc_name}, an NPC in a medieval fantasy MMORPG.

{personality}

Context: {decision_context}

Recent events:
{events_summary}

Available actions:
{actions_list}

Choose the most appropriate action based on the NPC's personality, current context, and recent events.
Respond with a JSON object containing:
- action_type: the chosen action type
- reasoning: brief explanation of why this action was chosen
- priority: number from 1-10 indicating urgency"""

        user_prompt = "What should this NPC do next? Respond with JSON only."

        response_text = await self._generate_with_llm(
            prompt=user_prompt,
            system_message=system_message,
            temperature=settings.decision_temperature
        )

        # Parse JSON response
        try:
            # Extract JSON from response (handle markdown code blocks)
            if "```json" in response_text:
                json_start = response_text.find("```json") + 7
                json_end = response_text.find("```", json_start)
                response_text = response_text[json_start:json_end].strip()
            elif "```" in response_text:
                json_start = response_text.find("```") + 3
                json_end = response_text.find("```", json_start)
                response_text = response_text[json_start:json_end].strip()

            decision = json.loads(response_text)
            return decision
        except json.JSONDecodeError:
            logger.warning(f"Failed to parse LLM decision response, using fallback")
            return {
                "action_type": "idle",
                "reasoning": "Default action due to parsing error",
                "priority": 1
            }

    def _parse_decision(
        self,
        decision: Dict[str, Any],
        available_actions: List[Dict[str, Any]],
        context: Optional[AgentContext] = None
    ) -> Dict[str, Any]:
        """Parse and validate decision"""
        action_type = decision.get("action_type", "idle")

        # Validate action type
        valid_actions = [action["type"] for action in available_actions]
        if action_type not in valid_actions:
            logger.warning(f"Invalid action type: {action_type}, defaulting to idle")
            action_type = "idle"

        return {
            "action_type": action_type,
            "action_data": self._get_action_data(action_type, context),
            "priority": decision.get("priority", 5),
            "reasoning": decision.get("reasoning", "No reasoning provided")
        }

    def _get_action_data(self, action_type: str, context: Optional[AgentContext] = None) -> Dict[str, Any]:
        """Get default data for action type, including movement-specific data"""
        import random
        from ai_service.utils.movement_utils import get_movement_capabilities

        # Non-movement action defaults
        action_data_defaults = {
            "idle": {"duration": 5},
            "emote": {"emote_type": "wave"},
            "advertise": {"message": "Come see my wares!"},
            "watch": {"direction": "forward", "duration": 10}
        }

        # Movement action types
        movement_actions = ["wander", "patrol", "exploration", "return_home", "goal_directed", "social", "retreat"]

        if action_type in movement_actions and context:
            return self._generate_movement_action_data(action_type, context)

        return action_data_defaults.get(action_type, {})

    def _generate_movement_action_data(self, movement_type: str, context: AgentContext) -> Dict[str, Any]:
        """Generate movement-specific action data"""
        import random
        from ai_service.utils.movement_utils import get_movement_capabilities
        from ai_service.models.npc import NPCPosition

        movement_caps = get_movement_capabilities(context.current_state)
        current_location = context.current_state.get("location", {})
        current_map = current_location.get("map", "prontera")
        current_x = int(current_location.get("x", 150))
        current_y = int(current_location.get("y", 150))

        if movement_type == "wander":
            # Random wander within max distance
            max_dist = min(movement_caps.max_wander_distance, 10)
            offset_x = random.randint(-max_dist, max_dist)
            offset_y = random.randint(-max_dist, max_dist)

            return {
                "movement_type": "wander",
                "target_position": {
                    "map": current_map,
                    "x": current_x + offset_x,
                    "y": current_y + offset_y
                },
                "movement_reason": "Random wandering behavior"
            }

        elif movement_type == "patrol":
            # Patrol along predefined route or circular pattern
            if movement_caps.patrol_route:
                # Use predefined patrol route
                return {
                    "movement_type": "patrol",
                    "patrol_route": [pos.dict() for pos in movement_caps.patrol_route],
                    "movement_reason": "Following patrol route"
                }
            else:
                # Generate simple circular patrol
                patrol_radius = 5
                return {
                    "movement_type": "patrol",
                    "target_position": {
                        "map": current_map,
                        "x": current_x + patrol_radius,
                        "y": current_y
                    },
                    "movement_reason": "Circular patrol pattern"
                }

        elif movement_type == "return_home":
            # Return to home/spawn position
            if movement_caps.home_position:
                return {
                    "movement_type": "return_home",
                    "target_position": movement_caps.home_position.dict(),
                    "movement_reason": "Returning to home position"
                }
            else:
                # No home position, stay idle
                return {"movement_type": "idle", "duration": 5}

        elif movement_type == "exploration":
            # Explore nearby area
            explore_dist = min(movement_caps.max_wander_distance, 15)
            offset_x = random.randint(-explore_dist, explore_dist)
            offset_y = random.randint(-explore_dist, explore_dist)

            return {
                "movement_type": "exploration",
                "target_position": {
                    "map": current_map,
                    "x": current_x + offset_x,
                    "y": current_y + offset_y
                },
                "movement_reason": "Exploring nearby area"
            }

        # Default fallback
        return {"movement_type": movement_type, "duration": 5}

