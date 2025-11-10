"""
Universal Consciousness System
Extends agent-based decision making to ALL entities with deep reasoning
Implements past-present-future reasoning chains
"""

from datetime import datetime, timedelta
from typing import Dict, List, Any, Optional, Tuple
from pydantic import BaseModel, Field
from loguru import logger

from ai_service.config import settings
from agents.base_agent import AgentContext, AgentDecision
from database import get_dragonfly_client, get_postgres_pool


class ReasoningChain(BaseModel):
    """Represents a complete reasoning chain for a decision"""
    decision_id: str
    entity_id: str
    entity_type: str  # npc, world, economy, faction, shop
    reasoning_depth: str  # shallow, medium, deep
    
    # Past Context
    past_decisions: List[Dict[str, Any]] = Field(default_factory=list)
    past_outcomes: List[Dict[str, Any]] = Field(default_factory=list)
    historical_patterns: List[str] = Field(default_factory=list)
    lessons_learned: List[str] = Field(default_factory=list)
    
    # Present Context
    current_state: Dict[str, Any] = Field(default_factory=dict)
    current_goals: List[str] = Field(default_factory=list)
    current_constraints: List[str] = Field(default_factory=list)
    nearby_entities: List[Dict[str, Any]] = Field(default_factory=list)
    world_state: Dict[str, Any] = Field(default_factory=dict)
    
    # Future Planning
    future_steps: List[Dict[str, Any]] = Field(default_factory=list)
    predicted_outcomes: List[Dict[str, Any]] = Field(default_factory=list)
    alternative_plans: List[Dict[str, Any]] = Field(default_factory=list)
    risk_assessment: Dict[str, Any] = Field(default_factory=dict)
    
    # Reasoning Process
    reasoning_steps: List[str] = Field(default_factory=list)
    decision_rationale: str = ""
    confidence_score: float = Field(default=0.5, ge=0.0, le=1.0)
    
    # Metadata
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    llm_calls_used: int = 0
    processing_time_ms: float = 0.0


class UniversalConsciousnessEngine:
    """
    Universal consciousness engine for all entities
    Provides deep reasoning capabilities with past-present-future analysis
    """
    
    def __init__(self):
        self.dragonfly = get_dragonfly_client()
        self.postgres_pool = None
        logger.info("Universal Consciousness Engine initialized")
    
    async def initialize(self):
        """Initialize the consciousness engine"""
        self.postgres_pool = await get_postgres_pool()
        logger.info("✓ Universal Consciousness Engine ready")
    
    async def build_reasoning_chain(
        self,
        entity_id: str,
        entity_type: str,
        decision_context: Dict[str, Any],
        depth: Optional[str] = None
    ) -> ReasoningChain:
        """
        Build a complete reasoning chain for an entity's decision
        
        Args:
            entity_id: ID of the entity making the decision
            entity_type: Type of entity (npc, world, economy, faction, shop)
            decision_context: Current decision context
            depth: Reasoning depth override (shallow/medium/deep)
        
        Returns:
            Complete reasoning chain with past, present, and future analysis
        """
        start_time = datetime.utcnow()
        depth = depth or settings.reasoning_depth
        
        logger.debug(f"Building {depth} reasoning chain for {entity_type} {entity_id}")
        
        chain = ReasoningChain(
            decision_id=decision_context.get("decision_id", "unknown"),
            entity_id=entity_id,
            entity_type=entity_type,
            reasoning_depth=depth
        )
        
        # Build past context (all depths)
        if depth in ["medium", "deep"]:
            chain.past_decisions = await self._get_past_decisions(
                entity_id, entity_type, limit=settings.agent_context_window_size
            )
            chain.past_outcomes = await self._get_past_outcomes(
                entity_id, entity_type, days=settings.reasoning_history_days
            )
            chain.historical_patterns = await self._identify_patterns(
                chain.past_decisions, chain.past_outcomes
            )
            chain.lessons_learned = await self._extract_lessons(
                chain.past_outcomes
            )
        
        # Build present context (all depths)
        chain.current_state = decision_context.get("current_state", {})
        chain.current_goals = decision_context.get("goals", [])
        chain.current_constraints = decision_context.get("constraints", [])
        chain.nearby_entities = await self._get_nearby_entities(
            entity_id, entity_type, decision_context
        )
        chain.world_state = await self._get_world_state()
        
        # Build future planning (deep only)
        if depth == "deep" and settings.future_planning_enabled:
            chain.future_steps = await self._plan_future_steps(
                entity_id, entity_type, chain, decision_context
            )
            chain.predicted_outcomes = await self._predict_outcomes(
                chain.future_steps, chain
            )
            chain.alternative_plans = await self._generate_alternatives(
                chain.future_steps, chain
            )
            chain.risk_assessment = await self._assess_risks(
                chain.future_steps, chain
            )
        
        # Calculate processing time
        processing_time = (datetime.utcnow() - start_time).total_seconds() * 1000
        chain.processing_time_ms = processing_time
        
        logger.debug(f"Reasoning chain built in {processing_time:.1f}ms "
                    f"(depth: {depth}, past decisions: {len(chain.past_decisions)}, "
                    f"future steps: {len(chain.future_steps)})")
        
        return chain
    
    async def reason_and_decide(
        self,
        entity_id: str,
        entity_type: str,
        decision_context: Dict[str, Any],
        agent: Any  # The specific agent (DecisionAgent, EconomyAgent, etc.)
    ) -> Tuple[AgentDecision, ReasoningChain]:
        """
        Perform deep reasoning and make a decision
        
        Returns:
            Tuple of (decision, reasoning_chain)
        """
        # Build reasoning chain
        chain = await self.build_reasoning_chain(
            entity_id, entity_type, decision_context
        )
        
        # Enhance agent context with reasoning chain
        enhanced_context = self._enhance_context_with_reasoning(
            decision_context, chain
        )
        
        # Make decision using agent
        decision = await agent.process(enhanced_context)
        
        # Store reasoning chain if logging enabled
        if settings.reasoning_chain_logging_enabled:
            await self._store_reasoning_chain(chain, decision)

        return decision, chain

    # Helper methods for building reasoning chains

    async def _get_past_decisions(
        self,
        entity_id: str,
        entity_type: str,
        limit: int = 10
    ) -> List[Dict[str, Any]]:
        """Get past decisions for this entity"""
        if not self.postgres_pool:
            return []

        try:
            async with self.postgres_pool.acquire() as conn:
                rows = await conn.fetch(
                    """
                    SELECT decision_id, decision_type, decision_data, outcome, timestamp
                    FROM entity_decisions
                    WHERE entity_id = $1 AND entity_type = $2
                    ORDER BY timestamp DESC
                    LIMIT $3
                    """,
                    entity_id, entity_type, limit
                )
                return [dict(row) for row in rows]
        except Exception as e:
            logger.error(f"Error getting past decisions: {e}")
            return []

    async def _get_past_outcomes(
        self,
        entity_id: str,
        entity_type: str,
        days: int = 7
    ) -> List[Dict[str, Any]]:
        """Get past decision outcomes for this entity"""
        if not self.postgres_pool:
            return []

        try:
            cutoff_date = datetime.utcnow() - timedelta(days=days)
            async with self.postgres_pool.acquire() as conn:
                rows = await conn.fetch(
                    """
                    SELECT decision_id, outcome_type, outcome_data, success, timestamp
                    FROM decision_outcomes
                    WHERE entity_id = $1 AND entity_type = $2 AND timestamp > $3
                    ORDER BY timestamp DESC
                    """,
                    entity_id, entity_type, cutoff_date
                )
                return [dict(row) for row in rows]
        except Exception as e:
            logger.error(f"Error getting past outcomes: {e}")
            return []

    async def _identify_patterns(
        self,
        past_decisions: List[Dict[str, Any]],
        past_outcomes: List[Dict[str, Any]]
    ) -> List[str]:
        """Identify patterns in past decisions and outcomes"""
        patterns = []

        if not past_decisions or not past_outcomes:
            return patterns

        # Pattern 1: Successful decision types
        successful_types = {}
        for outcome in past_outcomes:
            if outcome.get("success"):
                decision_type = outcome.get("outcome_data", {}).get("decision_type")
                if decision_type:
                    successful_types[decision_type] = successful_types.get(decision_type, 0) + 1

        if successful_types:
            most_successful = max(successful_types.items(), key=lambda x: x[1])
            patterns.append(f"Most successful decision type: {most_successful[0]} ({most_successful[1]} successes)")

        # Pattern 2: Time-based patterns
        time_patterns = {}
        for decision in past_decisions:
            hour = decision.get("timestamp", datetime.utcnow()).hour
            time_patterns[hour] = time_patterns.get(hour, 0) + 1

        if time_patterns:
            most_active_hour = max(time_patterns.items(), key=lambda x: x[1])
            patterns.append(f"Most active hour: {most_active_hour[0]}:00 ({most_active_hour[1]} decisions)")

        return patterns

    async def _extract_lessons(
        self,
        past_outcomes: List[Dict[str, Any]]
    ) -> List[str]:
        """Extract lessons learned from past outcomes"""
        lessons = []

        if not past_outcomes:
            return lessons

        # Lesson 1: Success rate
        total = len(past_outcomes)
        successful = sum(1 for o in past_outcomes if o.get("success"))
        success_rate = (successful / total) * 100 if total > 0 else 0
        lessons.append(f"Overall success rate: {success_rate:.1f}% ({successful}/{total})")

        # Lesson 2: Failed decision types to avoid
        failed_types = {}
        for outcome in past_outcomes:
            if not outcome.get("success"):
                decision_type = outcome.get("outcome_data", {}).get("decision_type")
                if decision_type:
                    failed_types[decision_type] = failed_types.get(decision_type, 0) + 1

        if failed_types:
            most_failed = max(failed_types.items(), key=lambda x: x[1])
            lessons.append(f"Avoid decision type: {most_failed[0]} ({most_failed[1]} failures)")

        return lessons

    async def _get_nearby_entities(
        self,
        entity_id: str,
        entity_type: str,
        context: Dict[str, Any]
    ) -> List[Dict[str, Any]]:
        """Get nearby entities for context"""
        # This would query the spatial index for nearby NPCs, players, etc.
        # For now, return from context if available
        return context.get("nearby_entities", [])

    async def _get_world_state(self) -> Dict[str, Any]:
        """Get current world state"""
        import json
        world_data = await self.dragonfly.get("world:state")
        if world_data:
            return json.loads(world_data)
        return {
            "time": "day",
            "weather": "clear",
            "player_count": 0,
            "active_events": []
        }

    async def _plan_future_steps(
        self,
        entity_id: str,
        entity_type: str,
        chain: ReasoningChain,
        context: Dict[str, Any]
    ) -> List[Dict[str, Any]]:
        """Plan future steps based on current state and goals"""
        future_steps = []

        if not settings.future_planning_enabled:
            return future_steps

        # Generate future steps based on goals
        for i, goal in enumerate(chain.current_goals[:settings.future_planning_steps]):
            step = {
                "step_number": i + 1,
                "goal": goal,
                "action": f"work_towards_{goal}",
                "estimated_duration": 300,  # 5 minutes
                "prerequisites": [],
                "expected_outcome": f"progress_on_{goal}"
            }
            future_steps.append(step)

        return future_steps

    async def _predict_outcomes(
        self,
        future_steps: List[Dict[str, Any]],
        chain: ReasoningChain
    ) -> List[Dict[str, Any]]:
        """Predict outcomes of future steps"""
        predictions = []

        for step in future_steps:
            # Use historical patterns to predict outcome
            success_probability = 0.7  # Default

            # Adjust based on past success rate
            if chain.lessons_learned:
                for lesson in chain.lessons_learned:
                    if "success rate" in lesson.lower():
                        # Extract success rate from lesson
                        import re
                        match = re.search(r'(\d+\.?\d*)%', lesson)
                        if match:
                            success_probability = float(match.group(1)) / 100.0

            prediction = {
                "step_number": step["step_number"],
                "predicted_success": success_probability > 0.5,
                "success_probability": success_probability,
                "predicted_duration": step["estimated_duration"],
                "potential_issues": []
            }
            predictions.append(prediction)

        return predictions

    async def _generate_alternatives(
        self,
        future_steps: List[Dict[str, Any]],
        chain: ReasoningChain
    ) -> List[Dict[str, Any]]:
        """Generate alternative plans"""
        alternatives = []

        # Alternative 1: Reverse order of steps
        if len(future_steps) > 1:
            alternatives.append({
                "plan_id": "reverse_order",
                "description": "Execute steps in reverse order",
                "steps": list(reversed(future_steps)),
                "estimated_success": 0.6
            })

        # Alternative 2: Focus on highest priority goal only
        if future_steps:
            alternatives.append({
                "plan_id": "focus_single_goal",
                "description": "Focus on single highest priority goal",
                "steps": [future_steps[0]],
                "estimated_success": 0.8
            })

        return alternatives

    async def _assess_risks(
        self,
        future_steps: List[Dict[str, Any]],
        chain: ReasoningChain
    ) -> Dict[str, Any]:
        """Assess risks of the planned steps"""
        risks = {
            "overall_risk_level": "low",
            "risk_factors": [],
            "mitigation_strategies": []
        }

        # Risk factor 1: Too many steps
        if len(future_steps) > 5:
            risks["risk_factors"].append("Too many planned steps may lead to failure")
            risks["mitigation_strategies"].append("Break down into smaller batches")
            risks["overall_risk_level"] = "medium"

        # Risk factor 2: Low historical success rate
        if chain.lessons_learned:
            for lesson in chain.lessons_learned:
                if "success rate" in lesson.lower():
                    import re
                    match = re.search(r'(\d+\.?\d*)%', lesson)
                    if match and float(match.group(1)) < 50:
                        risks["risk_factors"].append("Low historical success rate")
                        risks["mitigation_strategies"].append("Review past failures before proceeding")
                        risks["overall_risk_level"] = "high"

        return risks

    def _enhance_context_with_reasoning(
        self,
        context: Dict[str, Any],
        chain: ReasoningChain
    ) -> Dict[str, Any]:
        """Enhance agent context with reasoning chain data"""
        enhanced = context.copy()

        enhanced["reasoning_chain"] = {
            "depth": chain.reasoning_depth,
            "past_patterns": chain.historical_patterns,
            "lessons_learned": chain.lessons_learned,
            "future_plan": chain.future_steps,
            "predicted_outcomes": chain.predicted_outcomes,
            "risk_assessment": chain.risk_assessment
        }

        return enhanced

    async def _store_reasoning_chain(
        self,
        chain: ReasoningChain,
        decision: AgentDecision
    ):
        """Store reasoning chain for future analysis"""
        if not self.postgres_pool:
            return

        try:
            import json
            async with self.postgres_pool.acquire() as conn:
                await conn.execute(
                    """
                    INSERT INTO reasoning_chains (
                        decision_id, entity_id, entity_type, reasoning_depth,
                        past_context, present_context, future_context,
                        reasoning_steps, decision_rationale, confidence_score,
                        llm_calls_used, processing_time_ms, timestamp
                    ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
                    """,
                    chain.decision_id,
                    chain.entity_id,
                    chain.entity_type,
                    chain.reasoning_depth,
                    json.dumps({
                        "past_decisions": chain.past_decisions,
                        "past_outcomes": chain.past_outcomes,
                        "historical_patterns": chain.historical_patterns,
                        "lessons_learned": chain.lessons_learned
                    }),
                    json.dumps({
                        "current_state": chain.current_state,
                        "current_goals": chain.current_goals,
                        "current_constraints": chain.current_constraints,
                        "nearby_entities": chain.nearby_entities,
                        "world_state": chain.world_state
                    }),
                    json.dumps({
                        "future_steps": chain.future_steps,
                        "predicted_outcomes": chain.predicted_outcomes,
                        "alternative_plans": chain.alternative_plans,
                        "risk_assessment": chain.risk_assessment
                    }),
                    chain.reasoning_steps,
                    chain.decision_rationale,
                    chain.confidence_score,
                    chain.llm_calls_used,
                    chain.processing_time_ms,
                    chain.timestamp
                )
                logger.debug(f"Stored reasoning chain for {chain.entity_type} {chain.entity_id}")
        except Exception as e:
            logger.error(f"Error storing reasoning chain: {e}")


# Global instance
universal_consciousness = UniversalConsciousnessEngine()
