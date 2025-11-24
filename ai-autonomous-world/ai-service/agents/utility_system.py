"""
Utility-Based Decision System
Implements weighted factor evaluation for action selection
"""

from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from loguru import logger


@dataclass
class UtilityFactor:
    """Single utility consideration"""
    name: str
    weight: float  # 0.0 to 1.0
    current_value: float  # 0.0 to 1.0
    
    def get_weighted_utility(self) -> float:
        """Calculate weighted utility contribution"""
        return self.weight * self.current_value


class UtilityBasedPlanner:
    """
    Utility-based decision making with weighted factors.
    
    Factors (must sum to 1.0):
    - Safety: 30% - Protection and risk avoidance
    - Hunger: 25% - Resource acquisition needs
    - Social: 20% - Interaction and relationships
    - Curiosity: 15% - Exploration and discovery
    - Aggression: 10% - Combat and dominance
    """
    
    DEFAULT_WEIGHTS = {
        "safety": 0.30,
        "hunger": 0.25,
        "social": 0.20,
        "curiosity": 0.15,
        "aggression": 0.10
    }
    
    def __init__(self, weights: Optional[Dict[str, float]] = None):
        """
        Initialize utility planner with weights
        
        Args:
            weights: Optional custom weights (must sum to 1.0)
        """
        self.weights = weights or self.DEFAULT_WEIGHTS.copy()
        
        # Validate weights sum to 1.0
        total = sum(self.weights.values())
        if abs(total - 1.0) > 0.01:
            logger.warning(f"Weights sum to {total:.3f}, normalizing to 1.0")
            # Normalize weights
            for key in self.weights:
                self.weights[key] /= total
        
        logger.info(f"UtilityBasedPlanner initialized with weights: {self.weights}")
    
    def evaluate_action(
        self,
        action: str,
        npc_state: Dict[str, Any],
        context: Dict[str, Any]
    ) -> float:
        """
        Calculate total utility for an action.
        
        Args:
            action: Action to evaluate
            npc_state: Current NPC state
            context: World context
            
        Returns:
            Utility score 0.0-1.0
        """
        factors = self._calculate_factors(action, npc_state, context)
        total_utility = sum(f.get_weighted_utility() for f in factors)
        
        logger.debug(
            f"Action '{action}' utility: {total_utility:.3f} "
            f"(safety: {factors[0].current_value:.2f}, "
            f"hunger: {factors[1].current_value:.2f}, "
            f"social: {factors[2].current_value:.2f}, "
            f"curiosity: {factors[3].current_value:.2f}, "
            f"aggression: {factors[4].current_value:.2f})"
        )
        
        return total_utility
    
    def _calculate_factors(
        self,
        action: str,
        npc_state: Dict[str, Any],
        context: Dict[str, Any]
    ) -> List[UtilityFactor]:
        """Calculate all utility factors for action"""
        return [
            self._calculate_safety(action, npc_state, context),
            self._calculate_hunger(action, npc_state, context),
            self._calculate_social(action, npc_state, context),
            self._calculate_curiosity(action, npc_state, context),
            self._calculate_aggression(action, npc_state, context)
        ]
    
    def _calculate_safety(
        self, action: str, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> UtilityFactor:
        """
        Calculate safety utility (30% weight)
        
        High value if action increases safety
        """
        safety_value = 0.5  # Default neutral
        
        # High-safety actions
        if action in ["flee", "retreat", "defensive_action"]:
            safety_value = 1.0
        elif action in ["seek_shelter", "return_home"]:
            safety_value = 0.85
        elif action == "idle":
            safety_value = 0.7
        
        # Low-safety actions
        elif action in ["attack", "challenge", "aggressive_action"]:
            safety_value = 0.2
        elif action in ["exploration", "wander"]:
            safety_value = 0.4
        
        # Context modifiers
        hp_percentage = context.get("hp_percentage", 100)
        if hp_percentage < 50:
            # Low HP makes safety more important
            if action in ["flee", "seek_shelter"]:
                safety_value = min(1.0, safety_value + 0.3)
        
        threat_level = context.get("threat_level", 0)
        if threat_level > 5:
            # High threat reduces safety of aggressive actions
            if action in ["attack", "challenge"]:
                safety_value = max(0.0, safety_value - 0.3)
        
        return UtilityFactor("safety", self.weights["safety"], safety_value)
    
    def _calculate_hunger(
        self, action: str, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> UtilityFactor:
        """
        Calculate hunger utility (25% weight)
        
        High value if action satisfies resource needs
        """
        hunger_value = 0.5  # Default neutral
        
        current_hunger = npc_state.get("hunger", 0)
        
        # Resource-gathering actions
        if action in ["seek_food", "harvest", "forage"]:
            hunger_value = 0.9
            # More valuable when hungry
            if current_hunger > 70:
                hunger_value = 1.0
        elif action in ["trade", "visit_merchant"]:
            hunger_value = 0.7
        
        # Non-productive actions (opportunity cost)
        elif action in ["idle", "emote", "social_interaction"]:
            if current_hunger > 60:
                hunger_value = 0.2  # Should be getting food
            else:
                hunger_value = 0.5
        
        # Actions that delay food
        elif action in ["exploration", "wander"]:
            if current_hunger > 50:
                hunger_value = 0.3
            else:
                hunger_value = 0.5
        
        return UtilityFactor("hunger", self.weights["hunger"], hunger_value)
    
    def _calculate_social(
        self, action: str, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> UtilityFactor:
        """
        Calculate social utility (20% weight)
        
        High value if action improves relationships
        """
        social_value = 0.5  # Default neutral
        
        extraversion = npc_state.get("personality", {}).get("extraversion", 0.5)
        social_need = npc_state.get("social_need", 0)
        nearby_npcs = context.get("nearby_npcs", [])
        
        # Social actions
        if action in ["social_interaction", "seek_companion", "networking"]:
            social_value = 0.9
            # More valuable for extraverts
            if extraversion > 0.7:
                social_value = 1.0
            # More valuable when lonely
            if social_need > 70:
                social_value = 1.0
        elif action in ["coordinate_patrol", "discuss_prices", "group_activity"]:
            social_value = 0.8
            if len(nearby_npcs) > 0:
                social_value = 0.9
        
        # Neutral social actions
        elif action in ["trade", "advertise"]:
            social_value = 0.6
        
        # Anti-social actions
        elif action in ["flee", "retreat", "isolation"]:
            if extraversion > 0.7:
                social_value = 0.2  # Bad for extraverts
            else:
                social_value = 0.6  # Fine for introverts
        
        return UtilityFactor("social", self.weights["social"], social_value)
    
    def _calculate_curiosity(
        self, action: str, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> UtilityFactor:
        """
        Calculate curiosity utility (15% weight)
        
        High value if action enables discovery
        """
        curiosity_value = 0.5  # Default neutral
        
        openness = npc_state.get("personality", {}).get("openness", 0.5)
        visited_locations = len(npc_state.get("visited_locations", []))
        
        # Exploration actions
        if action in ["exploration", "wander", "discover"]:
            curiosity_value = 0.9
            # More valuable for open personalities
            if openness > 0.7:
                curiosity_value = 1.0
            # Less valuable if explored a lot already
            if visited_locations > 20:
                curiosity_value = max(0.5, curiosity_value - 0.2)
        elif action in ["investigate", "examine", "research"]:
            curiosity_value = 0.85
        
        # Learning actions
        elif action in ["training", "practice", "study"]:
            curiosity_value = 0.7
            if openness > 0.6:
                curiosity_value = 0.8
        
        # Routine actions (low curiosity)
        elif action in ["idle", "patrol", "routine_task"]:
            if openness > 0.7:
                curiosity_value = 0.3  # Boring for curious NPCs
            else:
                curiosity_value = 0.6  # Fine for routine-oriented NPCs
        
        return UtilityFactor("curiosity", self.weights["curiosity"], curiosity_value)
    
    def _calculate_aggression(
        self, action: str, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> UtilityFactor:
        """
        Calculate aggression utility (10% weight)
        
        High value if action asserts dominance
        """
        aggression_value = 0.5  # Default neutral
        
        agreeableness = npc_state.get("personality", {}).get("agreeableness", 0.5)
        neuroticism = npc_state.get("personality", {}).get("neuroticism", 0.5)
        
        # Aggressive actions
        if action in ["attack", "challenge", "confront"]:
            aggression_value = 0.9
            # More valuable for disagreeable NPCs
            if agreeableness < 0.3:
                aggression_value = 1.0
            # Less valuable for agreeable NPCs
            if agreeableness > 0.7:
                aggression_value = 0.3
        elif action in ["guard", "patrol", "watch"]:
            aggression_value = 0.6
        
        # Assertive but not aggressive
        elif action in ["advertise", "negotiate", "compete"]:
            aggression_value = 0.5
        
        # Passive actions
        elif action in ["flee", "retreat", "yield"]:
            if agreeableness < 0.3:
                aggression_value = 0.2  # Unsatisfying for aggressive NPCs
            else:
                aggression_value = 0.7  # Fine for peaceful NPCs
        
        # High neuroticism may reduce aggressive tendency
        if neuroticism > 0.7:
            if action in ["attack", "challenge"]:
                aggression_value = max(0.3, aggression_value - 0.3)
        
        return UtilityFactor("aggression", self.weights["aggression"], aggression_value)
    
    def select_best_action(
        self,
        possible_actions: List[str],
        npc_state: Dict[str, Any],
        context: Dict[str, Any]
    ) -> Tuple[str, float]:
        """
        Select action with highest utility.
        
        Args:
            possible_actions: List of available actions
            npc_state: Current NPC state
            context: World context
            
        Returns:
            Tuple of (best_action, utility_score)
        """
        if not possible_actions:
            logger.warning("No possible actions provided")
            return ("idle", 0.5)
        
        action_utilities = [
            (action, self.evaluate_action(action, npc_state, context))
            for action in possible_actions
        ]
        
        best_action, best_utility = max(action_utilities, key=lambda x: x[1])
        
        logger.info(
            f"Selected action '{best_action}' with utility {best_utility:.3f} "
            f"from {len(possible_actions)} options"
        )
        
        return best_action, best_utility
    
    def adjust_weights_by_personality(
        self, personality: Dict[str, float]
    ) -> None:
        """
        Adjust utility weights based on personality traits
        
        Args:
            personality: Personality trait dictionary
        """
        # High neuroticism increases safety weight
        if personality.get("neuroticism", 0.5) > 0.7:
            self._reweight("safety", 0.05)
        
        # High extraversion increases social weight
        if personality.get("extraversion", 0.5) > 0.7:
            self._reweight("social", 0.05)
        
        # High openness increases curiosity weight
        if personality.get("openness", 0.5) > 0.7:
            self._reweight("curiosity", 0.05)
        
        # Low agreeableness increases aggression weight
        if personality.get("agreeableness", 0.5) < 0.3:
            self._reweight("aggression", 0.05)
        
        # Renormalize
        total = sum(self.weights.values())
        for key in self.weights:
            self.weights[key] /= total
        
        logger.debug(f"Adjusted weights: {self.weights}")
    
    def _reweight(self, factor: str, adjustment: float) -> None:
        """Adjust a single weight"""
        if factor in self.weights:
            self.weights[factor] = min(1.0, self.weights[factor] + adjustment)