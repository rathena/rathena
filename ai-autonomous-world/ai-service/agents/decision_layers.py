"""
Hierarchical Decision Layers - Five-layer decision architecture
Implements latency-based decision making from reflex to strategic planning
"""

from enum import Enum
from typing import Dict, Any, Optional, List
from dataclasses import dataclass
import time
from loguru import logger
import asyncio
from utils.prometheus_metrics import record_decision_layer


class DecisionLayer(str, Enum):
    """Five-layer hierarchical decision system"""
    REFLEX = "reflex"          # <10ms - immediate reactions
    REACTIVE = "reactive"      # 50ms - pattern matching
    DELIBERATIVE = "deliberative"  # 200ms - strategic choices
    SOCIAL = "social"          # 500ms - group dynamics
    STRATEGIC = "strategic"    # 1000ms - long-term planning


@dataclass
class LayerDecision:
    """Decision from a specific layer"""
    layer: DecisionLayer
    action: str
    confidence: float
    latency_ms: float
    reasoning: Optional[str] = None
    metadata: Optional[Dict[str, Any]] = None


class HierarchicalDecisionSystem:
    """
    Five-layer hierarchical decision architecture.
    
    Processes decisions through layers from fastest to slowest,
    returning first confident decision within latency budget.
    
    Layers:
    - Reflex: <10ms - Immediate threat/opportunity responses
    - Reactive: 50ms - Pattern-based reactions
    - Deliberative: 200ms - Complex reasoning and planning
    - Social: 500ms - Multi-agent coordination
    - Strategic: 1000ms - Long-term goal planning (can use LLM)
    """
    
    def __init__(self, llm_provider: Optional[Any] = None):
        """Initialize hierarchical decision system"""
        self.max_latencies = {
            DecisionLayer.REFLEX: 0.010,      # 10ms
            DecisionLayer.REACTIVE: 0.050,    # 50ms
            DecisionLayer.DELIBERATIVE: 0.200,  # 200ms
            DecisionLayer.SOCIAL: 0.500,      # 500ms
            DecisionLayer.STRATEGIC: 1.000,   # 1000ms
        }
        self.llm_provider = llm_provider
        logger.info("Hierarchical Decision System initialized")
    
    async def make_decision(
        self,
        npc_state: Dict[str, Any],
        context: Dict[str, Any],
        max_latency: Optional[float] = None
    ) -> LayerDecision:
        """
        Make hierarchical decision through layers.
        
        Tries layers in order until confident decision or latency budget exhausted.
        
        Args:
            npc_state: Current NPC state dictionary
            context: World and situation context
            max_latency: Maximum time budget in seconds (None = use all layers)
            
        Returns:
            LayerDecision with action and metadata
        """
        start_time = time.time()
        max_latency = max_latency or self.max_latencies[DecisionLayer.STRATEGIC]
        
        # Try each layer in order
        for layer in DecisionLayer:
            layer_start = time.time()
            layer_budget = self.max_latencies[layer]
            
            # Skip if already exceeded total budget
            elapsed = time.time() - start_time
            if elapsed >= max_latency:
                logger.warning(f"Exceeded total latency budget: {elapsed:.3f}s > {max_latency:.3f}s")
                break
            
            # Try this layer
            try:
                decision = await self._process_layer(
                    layer, npc_state, context, layer_budget
                )
                
                if decision and decision.confidence >= 0.7:  # Confident decision
                    logger.debug(
                        f"Layer {layer.value} returned confident decision "
                        f"({decision.confidence:.2f}) in {decision.latency_ms:.2f}ms"
                    )
                    
                    # Record metrics for selected layer
                    record_decision_layer(
                        layer=layer.value,
                        outcome="selected",
                        latency=decision.latency_ms / 1000.0,  # Convert ms to seconds
                        confidence=decision.confidence
                    )
                    
                    return decision
                else:
                    # Record metrics for skipped layer
                    if decision:
                        record_decision_layer(
                            layer=layer.value,
                            outcome="skipped",
                            latency=decision.latency_ms / 1000.0,
                            confidence=decision.confidence
                        )
            except Exception as e:
                logger.error(f"Layer {layer.value} failed: {e}")
                # Record error metric
                record_decision_layer(
                    layer=layer.value,
                    outcome="timeout",
                    latency=layer_budget
                )
                continue
        
        # Fallback: return best-effort decision from deliberative layer
        logger.info("No confident decision, using deliberative fallback")
        fallback_decision = await self._process_layer(
            DecisionLayer.DELIBERATIVE,
            npc_state,
            context,
            0.200
        )
        
        # Record fallback metric
        if fallback_decision:
            record_decision_layer(
                layer=DecisionLayer.DELIBERATIVE.value,
                outcome="fallback",
                latency=fallback_decision.latency_ms / 1000.0,
                confidence=fallback_decision.confidence,
                fallback_from="all_layers"
            )
        
        return fallback_decision
    
    async def _process_layer(
        self,
        layer: DecisionLayer,
        npc_state: Dict[str, Any],
        context: Dict[str, Any],
        max_latency: float
    ) -> Optional[LayerDecision]:
        """Process decision at specific layer"""
        start = time.time()
        
        try:
            if layer == DecisionLayer.REFLEX:
                decision = self._reflex_decision(npc_state, context)
            elif layer == DecisionLayer.REACTIVE:
                decision = self._reactive_decision(npc_state, context)
            elif layer == DecisionLayer.DELIBERATIVE:
                decision = await self._deliberative_decision(npc_state, context)
            elif layer == DecisionLayer.SOCIAL:
                decision = await self._social_decision(npc_state, context)
            elif layer == DecisionLayer.STRATEGIC:
                decision = await self._strategic_decision(npc_state, context)
            else:
                return None
            
            latency = (time.time() - start) * 1000  # Convert to ms
            
            if latency > max_latency * 1000:
                logger.warning(
                    f"{layer.value} exceeded latency: {latency:.2f}ms > {max_latency*1000}ms"
                )
            
            if decision:
                decision.latency_ms = latency
            
            return decision
            
        except Exception as e:
            logger.error(f"Error in layer {layer.value}: {e}")
            return None
    
    def _reflex_decision(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> Optional[LayerDecision]:
        """
        Ultra-fast reflex responses (<10ms)
        
        Handles immediate threats and critical situations
        """
        # Check HP percentage for flee reflex
        hp_percentage = context.get("hp_percentage", 100)
        if hp_percentage < 20:
            return LayerDecision(
                layer=DecisionLayer.REFLEX,
                action="flee",
                confidence=1.0,
                latency_ms=0.0,
                reasoning="Critical HP - immediate flee reflex",
                metadata={"hp_percentage": hp_percentage}
            )
        
        # Check for immediate threats
        immediate_threat = context.get("immediate_threat")
        if immediate_threat:
            return LayerDecision(
                layer=DecisionLayer.REFLEX,
                action="defensive_action",
                confidence=0.95,
                latency_ms=0.0,
                reasoning="Immediate threat detected",
                metadata={"threat": immediate_threat}
            )
        
        # No reflex-level decision needed
        return LayerDecision(
            layer=DecisionLayer.REFLEX,
            action="none",
            confidence=0.3,
            latency_ms=0.0,
            reasoning="No reflex-level action needed"
        )
    
    def _reactive_decision(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> Optional[LayerDecision]:
        """
        Pattern-based reactions (50ms)
        
        Simple pattern matching and heuristics
        """
        # Pattern: Low resources
        hunger = npc_state.get("hunger", 0)
        if hunger > 80:
            return LayerDecision(
                layer=DecisionLayer.REACTIVE,
                action="seek_food",
                confidence=0.85,
                latency_ms=0.0,
                reasoning="High hunger - seek food pattern",
                metadata={"hunger": hunger}
            )
        
        # Pattern: Social interaction opportunity
        nearby_npcs = context.get("nearby_npcs", [])
        if len(nearby_npcs) > 0 and npc_state.get("extraversion", 0.5) > 0.7:
            return LayerDecision(
                layer=DecisionLayer.REACTIVE,
                action="social_interaction",
                confidence=0.75,
                latency_ms=0.0,
                reasoning="Social NPC with nearby companions",
                metadata={"nearby_count": len(nearby_npcs)}
            )
        
        # Pattern: Return to home position
        distance_from_home = context.get("distance_from_home", 0)
        if distance_from_home > 50:
            return LayerDecision(
                layer=DecisionLayer.REACTIVE,
                action="return_home",
                confidence=0.70,
                latency_ms=0.0,
                reasoning="Far from home - return pattern",
                metadata={"distance": distance_from_home}
            )
        
        # No strong reactive pattern
        return LayerDecision(
            layer=DecisionLayer.REACTIVE,
            action="none",
            confidence=0.4,
            latency_ms=0.0,
            reasoning="No clear reactive pattern"
        )
    
    async def _deliberative_decision(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> Optional[LayerDecision]:
        """
        Complex reasoning (200ms)
        
        Can use lightweight ML or multi-factor evaluation
        """
        # Evaluate multiple factors
        scores = {}
        
        # Factor: Goal alignment
        current_goal = npc_state.get("current_goal")
        if current_goal:
            goal_actions = self._get_goal_aligned_actions(current_goal)
            for action in goal_actions:
                scores[action] = scores.get(action, 0) + 0.3
        
        # Factor: Personality fit
        personality = npc_state.get("personality", {})
        if personality.get("curiosity", 0.5) > 0.7:
            scores["exploration"] = scores.get("exploration", 0) + 0.2
        if personality.get("conscientiousness", 0.5) > 0.7:
            scores["organize_inventory"] = scores.get("organize_inventory", 0) + 0.2
        
        # Factor: Environmental opportunities
        time_of_day = context.get("time_of_day", "day")
        if time_of_day == "night" and personality.get("neuroticism", 0.5) > 0.6:
            scores["seek_shelter"] = scores.get("seek_shelter", 0) + 0.25
        
        # Select best action
        if scores:
            best_action = max(scores.items(), key=lambda x: x[1])
            return LayerDecision(
                layer=DecisionLayer.DELIBERATIVE,
                action=best_action[0],
                confidence=min(best_action[1], 0.9),
                latency_ms=0.0,
                reasoning=f"Multi-factor evaluation selected {best_action[0]}",
                metadata={"all_scores": scores}
            )
        
        # Default deliberative action
        return LayerDecision(
            layer=DecisionLayer.DELIBERATIVE,
            action="idle",
            confidence=0.5,
            latency_ms=0.0,
            reasoning="No strong deliberative preference"
        )
    
    async def _social_decision(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> Optional[LayerDecision]:
        """
        Multi-agent coordination (500ms)
        
        Can coordinate with other NPCs
        """
        # Check for group activities
        nearby_npcs = context.get("nearby_npcs", [])
        
        if len(nearby_npcs) >= 2:
            # Potential group activity
            npc_class = npc_state.get("npc_class", "generic")
            
            if npc_class == "guard":
                return LayerDecision(
                    layer=DecisionLayer.SOCIAL,
                    action="coordinate_patrol",
                    confidence=0.80,
                    latency_ms=0.0,
                    reasoning="Multiple guards - coordinate patrol",
                    metadata={"group_size": len(nearby_npcs)}
                )
            elif npc_class == "merchant":
                return LayerDecision(
                    layer=DecisionLayer.SOCIAL,
                    action="discuss_prices",
                    confidence=0.75,
                    latency_ms=0.0,
                    reasoning="Multiple merchants - discuss market",
                    metadata={"group_size": len(nearby_npcs)}
                )
        
        # Check for social relationships
        relationships = npc_state.get("relationships", {})
        if relationships:
            # Seek friend if lonely
            if npc_state.get("social_need", 0) > 70:
                return LayerDecision(
                    layer=DecisionLayer.SOCIAL,
                    action="seek_companion",
                    confidence=0.70,
                    latency_ms=0.0,
                    reasoning="High social need - seek friends"
                )
        
        return LayerDecision(
            layer=DecisionLayer.SOCIAL,
            action="none",
            confidence=0.4,
            latency_ms=0.0,
            reasoning="No social coordination needed"
        )
    
    async def _strategic_decision(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> Optional[LayerDecision]:
        """
        Long-term planning (1000ms)
        
        Can use LLM for complex reasoning
        """
        # Check if LLM is available for strategic planning
        if self.llm_provider:
            try:
                # Use LLM for strategic decision
                prompt = self._build_strategic_prompt(npc_state, context)
                response = await self.llm_provider.generate(
                    prompt=prompt,
                    system_prompt="You are a strategic planner for NPCs. Provide one strategic action.",
                    temperature=0.7
                )
                
                action = self._parse_strategic_response(response.content)
                return LayerDecision(
                    layer=DecisionLayer.STRATEGIC,
                    action=action,
                    confidence=0.85,
                    latency_ms=0.0,
                    reasoning="LLM-based strategic planning",
                    metadata={"llm_used": True}
                )
            except Exception as e:
                logger.warning(f"LLM strategic planning failed: {e}")
        
        # Fallback: Rule-based strategic planning
        long_term_goals = npc_state.get("long_term_goals", [])
        if long_term_goals:
            # Pick first achievable goal
            goal = long_term_goals[0]
            action = self._goal_to_action(goal)
            return LayerDecision(
                layer=DecisionLayer.STRATEGIC,
                action=action,
                confidence=0.70,
                latency_ms=0.0,
                reasoning=f"Working towards goal: {goal}",
                metadata={"goal": goal}
            )
        
        # Ultimate fallback
        return LayerDecision(
            layer=DecisionLayer.STRATEGIC,
            action="idle",
            confidence=0.5,
            latency_ms=0.0,
            reasoning="No strategic plan available"
        )
    
    def _get_goal_aligned_actions(self, goal: str) -> List[str]:
        """Map goals to aligned actions"""
        goal_action_map = {
            "gather_resources": ["exploration", "harvest"],
            "increase_wealth": ["trade", "check_prices"],
            "protect_area": ["patrol", "watch"],
            "socialize": ["seek_companion", "social_interaction"],
            "improve_skills": ["practice", "training"]
        }
        return goal_action_map.get(goal, ["idle"])
    
    def _build_strategic_prompt(
        self, npc_state: Dict[str, Any], context: Dict[str, Any]
    ) -> str:
        """Build prompt for LLM strategic planning"""
        return f"""
NPC State: {npc_state.get('npc_class', 'generic')} at {context.get('location', 'unknown')}
Goals: {', '.join(npc_state.get('long_term_goals', []))}
Personality: {npc_state.get('personality', {})}

What strategic action should this NPC take to achieve their long-term goals?
Respond with one action: exploration, training, networking, resource_gathering, or planning.
"""
    
    def _parse_strategic_response(self, response: str) -> str:
        """Parse LLM response to extract action"""
        response_lower = response.lower()
        actions = ["exploration", "training", "networking", "resource_gathering", "planning"]
        for action in actions:
            if action in response_lower:
                return action
        return "idle"
    
    def _goal_to_action(self, goal: str) -> str:
        """Convert goal to action"""
        goal_lower = goal.lower()
        if "wealth" in goal_lower or "money" in goal_lower:
            return "trade"
        elif "protect" in goal_lower or "guard" in goal_lower:
            return "patrol"
        elif "social" in goal_lower or "friend" in goal_lower:
            return "seek_companion"
        elif "explore" in goal_lower or "discover" in goal_lower:
            return "exploration"
        else:
            return "idle"