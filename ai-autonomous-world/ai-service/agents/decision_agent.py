"""
Decision Agent - Makes decisions for NPC actions and behaviors
Determines what NPCs should do based on context, personality, and goals
"""

from typing import Dict, Any, List, Optional
from loguru import logger
import json

from crewai import Agent
from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from config import settings

from agents.decision_optimizer import DecisionOptimizer
from agents.moral_alignment import MoralAlignment
from agents.decision_layers import HierarchicalDecisionSystem, DecisionLayer
from agents.utility_system import UtilityBasedPlanner

class DecisionAgent(BaseAIAgent):
    """
    Specialized agent for NPC decision-making (Hybrid ML/LLM, CPU/GPU, runtime selection)
    
    Responsibilities:
    - Decide NPC actions based on current state
    - Prioritize goals and objectives
    - Consider personality in decision-making
    - Evaluate risks and rewards
    - Plan short-term actions
    - Integrate goal, emotion, and memory state for full consciousness model
    - Use DecisionOptimizer for ML-based decision (CPU/GPU), fallback to LLM if needed
    - Use HierarchicalDecisionSystem for latency-based decision layers
    - Use UtilityBasedPlanner for weighted factor evaluation
    """
    def __init__(self, agent_id: str, llm_provider: Any, config: Dict[str, Any]):
        """Initialize Decision Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="decision",
            llm_provider=llm_provider,
            config=config
        )
        self.ml_mode = config.get("ml_mode", "auto")  # "auto", "cpu", "gpu", "sklearn", "xgboost", "torch", "tf"
        self.optimizer = DecisionOptimizer(mode=self.ml_mode, fallback=None)
        self.moral_alignment = MoralAlignment()
        
        # NEW: Initialize Hierarchical Decision System
        self.hierarchical_system = HierarchicalDecisionSystem(llm_provider=self.llm_provider)
        
        # NEW: Initialize Utility-Based Planner with default weights
        self.utility_planner = UtilityBasedPlanner()
        
        logger.info(
            f"Decision Agent {agent_id} initialized "
            f"(ML mode: {self.ml_mode}, Hierarchical: ✓, Utility: ✓)"
        )

    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for decision-making"""
        from crewai import LLM
        import os
        try:
            from config import settings
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version
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

    async def _process(self, context: AgentContext) -> AgentResponse:
        """
        Make decision for NPC action using hierarchical layers and utility-based planning.
        
        Internal implementation called by base class process() method.

        Args:
            context: AgentContext with NPC state and world information

        Returns:
            AgentResponse with decided action
        """
        try:
            logger.info(f"Decision Agent processing for NPC: {context.npc_id}")

            # Get available actions
            available_actions = self._get_available_actions(context)
            action_list = [a["type"] for a in available_actions]

            # Build decision context (now includes goal, emotion, memory)
            decision_context = self._build_decision_context(context)

            # NEW: Assess urgency for latency budget
            urgency = self._assess_urgency(context)
            latency_budget = self._get_latency_budget(urgency)
            
            # NEW: Prepare state for hierarchical system
            npc_state = {
                "npc_id": context.npc_id,
                "npc_class": context.current_state.get("npc_class", "generic"),
                "personality": {
                    "openness": context.personality.openness,
                    "conscientiousness": context.personality.conscientiousness,
                    "extraversion": context.personality.extraversion,
                    "agreeableness": context.personality.agreeableness,
                    "neuroticism": context.personality.neuroticism,
                    "curiosity": getattr(context.personality, "curiosity", 0.5)
                },
                "hunger": context.current_state.get("hunger", 0),
                "social_need": context.current_state.get("social_need", 0),
                "current_goal": getattr(context.goal_state, "current_goal", None) if hasattr(context, "goal_state") else None,
                "long_term_goals": getattr(context.goal_state, "long_term_goals", []) if hasattr(context, "goal_state") else [],
                "relationships": context.current_state.get("relationships", {}),
                "visited_locations": context.current_state.get("visited_locations", [])
            }
            
            hierarchical_context = {
                "hp_percentage": context.current_state.get("hp_percentage", 100),
                "immediate_threat": context.current_state.get("immediate_threat"),
                "nearby_npcs": context.current_state.get("nearby_npcs", []),
                "distance_from_home": context.current_state.get("distance_from_home", 0),
                "time_of_day": context.current_state.get("time_of_day", "day"),
                "location": context.current_state.get("location", {})
            }

            # NEW: Try hierarchical decision system first
            hierarchical_decision = None
            try:
                layer_decision = await self.hierarchical_system.make_decision(
                    npc_state=npc_state,
                    context=hierarchical_context,
                    max_latency=latency_budget
                )
                
                if layer_decision and layer_decision.action in action_list:
                    hierarchical_decision = {
                        "action_type": layer_decision.action,
                        "reasoning": f"[{layer_decision.layer.value}] {layer_decision.reasoning}",
                        "priority": int(layer_decision.confidence * 10),
                        "confidence": layer_decision.confidence,
                        "layer": layer_decision.layer.value,
                        "latency_ms": layer_decision.latency_ms
                    }
                    logger.info(
                        f"Hierarchical decision for {context.npc_id}: "
                        f"{layer_decision.action} (layer: {layer_decision.layer.value}, "
                        f"confidence: {layer_decision.confidence:.2f})"
                    )
            except Exception as e:
                logger.warning(f"Hierarchical decision failed: {e}")

            # NEW: If hierarchical decision uncertain, use utility-based planner
            if hierarchical_decision and hierarchical_decision["confidence"] >= 0.7:
                decision = hierarchical_decision
            else:
                logger.info("Using utility-based planner for decision refinement")
                try:
                    # Adjust utility weights based on personality
                    self.utility_planner.adjust_weights_by_personality(npc_state["personality"])
                    
                    # Select best action using utility system
                    best_action, utility_score = self.utility_planner.select_best_action(
                        possible_actions=action_list,
                        npc_state=npc_state,
                        context=hierarchical_context
                    )
                    
                    decision = {
                        "action_type": best_action,
                        "reasoning": f"Utility-based selection (score: {utility_score:.3f})",
                        "priority": int(utility_score * 10),
                        "confidence": utility_score,
                        "method": "utility_planner"
                    }
                    logger.info(
                        f"Utility decision for {context.npc_id}: "
                        f"{best_action} (utility: {utility_score:.3f})"
                    )
                except Exception as e:
                    logger.warning(f"Utility planner failed: {e}")
                    # Final fallback to LLM
                    decision = await self._make_decision(
                        npc_name=context.npc_name,
                        personality=self._build_personality_prompt(context.personality),
                        goal_state=self._build_goal_prompt(getattr(context, "goal_state", None)),
                        emotion_state=self._build_emotion_prompt(getattr(context, "emotion_state", None)),
                        memory_state=self._build_memory_prompt(getattr(context, "memory_state", None)),
                        available_actions=available_actions,
                        decision_context=decision_context,
                        recent_events=context.recent_events
                    )
                    
                    if not isinstance(decision, dict):
                        logger.warning(f"Decision is not a dict (type: {type(decision)}), using fallback")
                        decision = {
                            "action_type": "idle",
                            "reasoning": "Fallback due to invalid decision format",
                            "priority": 1,
                            "confidence": 0.5
                        }

            # --- Moral Alignment System: update from action ---
            self.moral_alignment.update_from_action({"type": decision.get("action_type", "idle")})

            action_data = self._parse_decision(decision, available_actions, context)
            logger.info(f"Final decision for {context.npc_id}: {action_data.get('action_type')}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=action_data,
                confidence=decision.get("confidence", 0.80),
                reasoning=decision.get("reasoning", "Decision based on hierarchical/utility systems"),
                metadata={
                    "available_actions_count": len(available_actions),
                    "events_considered": len(context.recent_events),
                    "decision_method": decision.get("method", decision.get("layer", "unknown")),
                    "urgency": urgency,
                    "latency_budget_ms": latency_budget * 1000 if latency_budget else None,
                    "alignment": self.moral_alignment.to_dict()
                }
            )

        except Exception as e:
            logger.error(f"Decision-making failed for {context.npc_id}: {e}")
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
        from utils.movement_utils import can_npc_move, get_movement_capabilities

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

    import numpy as np
    def _extract_features(self, context: AgentContext, available_actions: List[Dict[str, Any]]) -> Optional[np.ndarray]:
        """
        Extracts features from context for ML model input.
        Returns: np.ndarray of shape (1, input_dim) or None if not enough data.
        """
        try:
            # Example: flatten personality, goal, emotion, and some state into a feature vector
            features = []
            p = context.personality
            features.extend([
                getattr(p, "openness", 0.5),
                getattr(p, "conscientiousness", 0.5),
                getattr(p, "extraversion", 0.5),
                getattr(p, "agreeableness", 0.5),
                getattr(p, "neuroticism", 0.5)
            ])
            g = getattr(context, "goal_state", None)
            if g:
                features.extend([
                    getattr(g, "survival", 1.0),
                    getattr(g, "security", 1.0),
                    getattr(g, "social", 1.0),
                    getattr(g, "esteem", 1.0),
                    getattr(g, "self_actualization", 1.0)
                ])
            else:
                features.extend([1.0, 1.0, 1.0, 1.0, 1.0])
            # Pad/truncate to input_dim
            features = features[:self.optimizer.input_dim] + [0.0] * (self.optimizer.input_dim - len(features))
            return np.array([features])
        except Exception as e:
            logger.warning(f"Failed to extract ML features: {e}")
            return None

    async def _make_decision(
        self,
        npc_name: str,
        personality: str,
        goal_state: str,
        emotion_state: str,
        memory_state: str,
        available_actions: List[Dict[str, Any]],
        decision_context: str,
        recent_events: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """Make decision using LLM (now with full consciousness context)"""
        actions_list = "\n".join([
            f"- {action['type']}: {action['description']}"
            for action in available_actions
        ])
        events_summary = "No recent events."
        if recent_events:
            events_summary = "\n".join([
                f"- {event.get('event_type', 'unknown')}: {event.get('summary', 'no details')}"
                for event in recent_events[-5:]
            ])
        system_message = f"""You are making decisions for {npc_name}, an NPC in a medieval fantasy MMORPG.
        
{personality}
{goal_state}
{emotion_state}
{memory_state}

Context: {decision_context}

Recent events:
{events_summary}

Available actions:
{actions_list}

Choose the most appropriate action based on the NPC's personality, goals, emotion, memory, and current context.
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
        import random
        from utils.movement_utils import get_movement_capabilities
        action_data_defaults = {
            "idle": {"duration": 5},
            "emote": {"emote_type": "wave"},
            "advertise": {"message": "Come see my wares!"},
            "watch": {"direction": "forward", "duration": 10}
        }
        movement_actions = ["wander", "patrol", "exploration", "return_home", "goal_directed", "social", "retreat"]
        if action_type in movement_actions and context:
            return self._generate_movement_action_data(action_type, context)
        return action_data_defaults.get(action_type, {})

    def _generate_movement_action_data(self, movement_type: str, context: AgentContext) -> Dict[str, Any]:
        import random
        from utils.movement_utils import get_movement_capabilities, is_position_within_boundary, calculate_distance
        from models.npc import NPCPosition
        movement_caps = get_movement_capabilities(context.current_state)
        current_location = context.current_state.get("location", {})
        current_map = current_location.get("map", "prontera")
        current_x = int(current_location.get("x", 150))
        current_y = int(current_location.get("y", 150))
        if movement_type == "wander":
            max_dist = min(movement_caps.max_wander_distance, 10)
            if movement_caps.movement_mode == "radius_restricted" and movement_caps.spawn_point:
                spawn_pos = {
                    "map": movement_caps.spawn_point.map,
                    "x": movement_caps.spawn_point.x,
                    "y": movement_caps.spawn_point.y
                }
                current_pos = {"map": current_map, "x": current_x, "y": current_y}
                current_distance = calculate_distance(spawn_pos, current_pos)
                remaining_radius = movement_caps.movement_radius - current_distance
                max_dist = min(max_dist, max(1, int(remaining_radius)))
            for attempt in range(10):
                offset_x = random.randint(-max_dist, max_dist)
                offset_y = random.randint(-max_dist, max_dist)
                target_position = {
                    "map": current_map,
                    "x": current_x + offset_x,
                    "y": current_y + offset_y
                }
                if is_position_within_boundary(target_position, movement_caps, context.npc_id):
                    return {
                        "movement_type": "wander",
                        "target_position": target_position,
                        "movement_reason": "Random wandering behavior"
                    }
            logger.warning(f"NPC {context.npc_id}: Could not find valid wander position within boundaries")
            return {"movement_type": "idle", "duration": 5}
        elif movement_type == "patrol":
            if movement_caps.patrol_route:
                return {
                    "movement_type": "patrol",
                    "patrol_route": [pos.dict() for pos in movement_caps.patrol_route],
                    "movement_reason": "Following patrol route"
                }
            else:
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
            if movement_caps.home_position:
                return {
                    "movement_type": "return_home",
                    "target_position": movement_caps.home_position.dict(),
                    "movement_reason": "Returning to home position"
                }
            else:
                return {"movement_type": "idle", "duration": 5}
        elif movement_type == "exploration":
            explore_dist = min(movement_caps.max_wander_distance, 15)
            if movement_caps.movement_mode == "radius_restricted" and movement_caps.spawn_point:
                spawn_pos = {
                    "map": movement_caps.spawn_point.map,
                    "x": movement_caps.spawn_point.x,
                    "y": movement_caps.spawn_point.y
                }
                current_pos = {"map": current_map, "x": current_x, "y": current_y}
                current_distance = calculate_distance(spawn_pos, current_pos)
                remaining_radius = movement_caps.movement_radius - current_distance
                explore_dist = min(explore_dist, max(1, int(remaining_radius)))
            for attempt in range(10):
                offset_x = random.randint(-explore_dist, explore_dist)
                offset_y = random.randint(-explore_dist, explore_dist)
                target_position = {
                    "map": current_map,
                    "x": current_x + offset_x,
                    "y": current_y + offset_y
                }
                if is_position_within_boundary(target_position, movement_caps, context.npc_id):
                    return {
                        "movement_type": "exploration",
                        "target_position": target_position,
                        "movement_reason": "Exploring nearby area"
                    }
            logger.warning(f"NPC {context.npc_id}: Could not find valid exploration position within boundaries")
            return {"movement_type": "idle", "duration": 5}
        return {"movement_type": movement_type, "duration": 5}
    
    def _assess_urgency(self, context: AgentContext) -> str:
        """
        Assess situation urgency
        
        Returns:
            "critical", "high", "normal", or "low"
        """
        # Check for critical situations
        hp = context.current_state.get("hp", 100)
        if hp < 20:
            return "critical"
        
        # Check for immediate threats
        if context.current_state.get("immediate_threat"):
            return "high"
        
        # Check primary emotion
        if hasattr(context, "emotion_state") and context.emotion_state:
            if context.emotion_state.primary_emotion == "fear":
                return "high"
            elif context.emotion_state.primary_emotion in ["anger", "surprise"]:
                return "normal"
        
        # Default normal
        return "normal"
    
    def _get_latency_budget(self, urgency: str) -> Optional[float]:
        """
        Get time budget based on urgency
        
        Returns:
            Max latency in seconds or None for full budget
        """
        budgets = {
            "critical": 0.010,  # 10ms - reflex only
            "high": 0.050,      # 50ms - up to reactive
            "normal": 0.200,    # 200ms - up to deliberative
            "low": None         # Full budget - all layers
        }
        return budgets.get(urgency, 0.200)