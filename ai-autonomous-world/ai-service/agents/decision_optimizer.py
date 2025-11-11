"""
Decision Optimizer - Advanced LLM Usage Optimization
Implements 4-tier decision system with caching, batching, and complexity analysis
"""

import asyncio
import hashlib
import json
from datetime import datetime, timedelta
from typing import Dict, List, Any, Optional, Tuple
from pydantic import BaseModel, Field
from loguru import logger

from ai_service.config import settings
from database import get_dragonfly_client, get_postgres_pool
from agents.base_agent import AgentContext, AgentDecision


class DecisionComplexity(BaseModel):
    """Analysis of decision complexity"""
    complexity_score: float = Field(ge=0.0, le=1.0)
    tier: int = Field(ge=1, le=4)
    factors: Dict[str, float] = Field(default_factory=dict)
    reasoning: str = ""


class CachedDecision(BaseModel):
    """Cached decision with similarity metadata"""
    decision_id: str
    context_hash: str
    decision_data: Dict[str, Any]
    similarity_score: float = 0.0
    cache_timestamp: datetime
    use_count: int = 0


class DecisionBatch(BaseModel):
    """Batch of similar decisions"""
    batch_id: str
    decisions: List[Dict[str, Any]] = Field(default_factory=list)
    created_at: datetime = Field(default_factory=datetime.utcnow)
    timeout_at: datetime
    max_size: int = 5


class DecisionOptimizer:
    """
    Optimizes LLM usage through intelligent caching, batching, and complexity analysis
    
    Tier 1 (No LLM): Rule-based decisions
    Tier 2 (Cached LLM): Reuse similar past decisions
    Tier 3 (Batched LLM): Batch multiple decisions
    Tier 4 (Full LLM): Complex unique decisions
    """
    
    def __init__(self):
        self.dragonfly = get_dragonfly_client()
        self.postgres_pool = None
        
        # Active batches
        self._batches: Dict[str, DecisionBatch] = {}
        self._batch_lock = asyncio.Lock()
        
        # Statistics
        self.stats = {
            "tier1_count": 0,  # Rule-based
            "tier2_count": 0,  # Cached
            "tier3_count": 0,  # Batched
            "tier4_count": 0,  # Full LLM
            "total_llm_calls": 0,
            "total_llm_calls_saved": 0
        }
        
        logger.info(f"Decision Optimizer initialized (mode: {settings.llm_optimization_mode})")
    
    async def initialize(self):
        """Initialize the optimizer"""
        self.postgres_pool = await get_postgres_pool()
        logger.info("✓ Decision Optimizer ready")
    
    async def optimize_decision(
        self,
        context: AgentContext,
        agent: Any,
        rule_based_handler: Optional[callable] = None
    ) -> Tuple[AgentDecision, int, bool]:
        """
        Optimize a decision using the 4-tier system
        
        Args:
            context: Agent context for the decision
            agent: The agent that will make the decision
            rule_based_handler: Optional function for tier 1 rule-based decisions
        
        Returns:
            Tuple of (decision, tier, cached)
        """
        # Tier 1: Try rule-based decision
        if settings.rule_based_decisions_enabled and rule_based_handler:
            tier1_decision = await rule_based_handler(context)
            if tier1_decision:
                self.stats["tier1_count"] += 1
                self.stats["total_llm_calls_saved"] += 1
                logger.debug(f"✓ Tier 1 (Rule-based) decision: {tier1_decision.data.get('action_type', 'unknown')}")
                return tier1_decision, 1, False
        
        # Analyze decision complexity
        complexity = await self.analyze_complexity(context)
        
        # Tier 2: Try cached decision
        if settings.decision_cache_enabled and complexity.tier <= 3:
            cached_decision = await self.get_cached_decision(context)
            if cached_decision:
                self.stats["tier2_count"] += 1
                self.stats["total_llm_calls_saved"] += 1
                logger.debug(f"✓ Tier 2 (Cached) decision (similarity: {cached_decision.similarity_score:.2f})")
                
                # Convert cached decision to AgentDecision
                decision = AgentDecision(
                    success=True,
                    data=cached_decision.decision_data,
                    reasoning=f"Cached decision (similarity: {cached_decision.similarity_score:.2f})"
                )
                return decision, 2, True
        
        # Tier 3: Try batching (for low-medium complexity)
        if settings.decision_batch_enabled and complexity.tier <= 2:
            batched_decision = await self.try_batch_decision(context, agent)
            if batched_decision:
                self.stats["tier3_count"] += 1
                logger.debug(f"✓ Tier 3 (Batched) decision")
                return batched_decision, 3, False
        
        # Tier 4: Full LLM decision
        self.stats["tier4_count"] += 1
        self.stats["total_llm_calls"] += 1
        logger.debug(f"✓ Tier 4 (Full LLM) decision (complexity: {complexity.complexity_score:.2f})")
        
        decision = await agent.process(context)
        
        # Cache the decision if enabled
        if settings.decision_cache_enabled:
            await self.cache_decision(context, decision)
        
        return decision, 4, False
    
    async def analyze_complexity(self, context: AgentContext) -> DecisionComplexity:
        """
        Analyze decision complexity to determine appropriate tier
        
        Returns complexity score 0.0-1.0:
        - 0.0-0.3: Simple (Tier 1-2)
        - 0.3-0.6: Medium (Tier 2-3)
        - 0.6-1.0: Complex (Tier 4)
        """
        factors = {}
        
        # Factor 1: Number of goals (more goals = more complex)
        goals_count = len(context.data.get("goals", []))
        factors["goals_complexity"] = min(goals_count / 5.0, 1.0)
        
        # Factor 2: Number of constraints
        constraints_count = len(context.data.get("constraints", []))
        factors["constraints_complexity"] = min(constraints_count / 5.0, 1.0)
        
        # Factor 3: Number of nearby entities (more entities = more complex)
        entities_count = len(context.data.get("nearby_entities", []))
        factors["entities_complexity"] = min(entities_count / 10.0, 1.0)
        
        # Factor 4: Reasoning depth requirement
        reasoning_depth = context.data.get("reasoning_depth", settings.reasoning_depth)
        depth_scores = {"shallow": 0.2, "medium": 0.5, "deep": 0.9}
        factors["depth_complexity"] = depth_scores.get(reasoning_depth, 0.5)
        
        # Factor 5: Decision urgency (instant = simpler heuristics acceptable)
        priority = context.data.get("priority", "normal")
        priority_scores = {"instant": 0.3, "high": 0.5, "normal": 0.7, "low": 0.9}
        factors["priority_complexity"] = priority_scores.get(priority, 0.7)
        
        # Calculate weighted average
        weights = {
            "goals_complexity": 0.25,
            "constraints_complexity": 0.20,
            "entities_complexity": 0.15,
            "depth_complexity": 0.25,
            "priority_complexity": 0.15
        }
        
        complexity_score = sum(factors[k] * weights[k] for k in factors)
        
        # Determine tier based on complexity and optimization mode
        if settings.llm_optimization_mode == "aggressive":
            # More aggressive caching/batching
            if complexity_score < 0.4:
                tier = 1
            elif complexity_score < 0.7:
                tier = 2
            else:
                tier = 4
        elif settings.llm_optimization_mode == "quality":
            # Prefer full LLM for better quality
            if complexity_score < 0.2:
                tier = 1
            elif complexity_score < 0.4:
                tier = 2
            else:
                tier = 4
        else:  # balanced
            if complexity_score < 0.3:
                tier = 1
            elif complexity_score < 0.6:
                tier = 2
            else:
                tier = 4
        
        return DecisionComplexity(
            complexity_score=complexity_score,
            tier=tier,
            factors=factors,
            reasoning=f"Complexity: {complexity_score:.2f}, Tier: {tier}"
        )

    async def get_cached_decision(self, context: AgentContext) -> Optional[CachedDecision]:
        """
        Try to find a cached similar decision
        Uses context hash and embedding similarity
        """
        # Generate context hash
        context_hash = self._hash_context(context)

        # Try exact match first
        cache_key = f"decision_cache:{context_hash}"
        cached_data = await self.dragonfly.get(cache_key)

        if cached_data:
            cached = json.loads(cached_data)
            logger.debug(f"Exact cache hit for context hash {context_hash[:8]}")
            return CachedDecision(
                decision_id=cached["decision_id"],
                context_hash=context_hash,
                decision_data=cached["decision_data"],
                similarity_score=1.0,
                cache_timestamp=datetime.fromisoformat(cached["timestamp"]),
                use_count=cached.get("use_count", 0) + 1
            )

        # Try similarity search (if embeddings enabled)
        if settings.decision_embedding_similarity_threshold > 0:
            similar_decision = await self._find_similar_decision(context)
            if similar_decision:
                return similar_decision

        return None

    async def cache_decision(self, context: AgentContext, decision: AgentDecision):
        """Cache a decision for future reuse"""
        context_hash = self._hash_context(context)
        cache_key = f"decision_cache:{context_hash}"

        cache_data = {
            "decision_id": str(datetime.utcnow().timestamp()),
            "decision_data": decision.data,
            "timestamp": datetime.utcnow().isoformat(),
            "use_count": 0
        }

        # Store in cache with TTL
        await self.dragonfly.setex(
            cache_key,
            settings.decision_cache_ttl,
            json.dumps(cache_data)
        )

        logger.debug(f"Cached decision with hash {context_hash[:8]}")

    async def try_batch_decision(
        self,
        context: AgentContext,
        agent: Any
    ) -> Optional[AgentDecision]:
        """
        Try to batch this decision with similar pending decisions
        Returns None if batching not possible, decision if batched
        """
        # Generate batch key based on decision type and entity type
        batch_key = self._get_batch_key(context)

        async with self._batch_lock:
            # Get or create batch
            if batch_key not in self._batches:
                timeout_at = datetime.utcnow() + timedelta(seconds=settings.decision_batch_timeout)
                self._batches[batch_key] = DecisionBatch(
                    batch_id=batch_key,
                    timeout_at=timeout_at,
                    max_size=settings.decision_batch_size
                )

            batch = self._batches[batch_key]

            # Add to batch
            batch.decisions.append({
                "context": context,
                "timestamp": datetime.utcnow()
            })

            # Check if batch is ready
            if len(batch.decisions) >= batch.max_size or datetime.utcnow() >= batch.timeout_at:
                # Process batch
                decisions = await self._process_batch(batch, agent)

                # Remove batch
                del self._batches[batch_key]

                # Return this decision's result
                for i, item in enumerate(batch.decisions):
                    if item["context"] == context:
                        return decisions[i]

            # Batch not ready yet, return None to trigger full LLM
            return None

    async def _process_batch(
        self,
        batch: DecisionBatch,
        agent: Any
    ) -> List[AgentDecision]:
        """Process a batch of decisions together"""
        logger.info(f"Processing batch {batch.batch_id} with {len(batch.decisions)} decisions")

        # For now, process each decision individually
        # In a real implementation, this would combine contexts and make a single LLM call
        decisions = []
        for item in batch.decisions:
            decision = await agent.process(item["context"])
            decisions.append(decision)

        # Count as single LLM call (batched)
        self.stats["total_llm_calls"] += 1
        self.stats["total_llm_calls_saved"] += len(batch.decisions) - 1

        return decisions

    def _hash_context(self, context: AgentContext) -> str:
        """Generate hash of context for caching"""
        # Create a normalized representation of context
        context_repr = {
            "entity_type": context.data.get("entity_type"),
            "decision_type": context.data.get("decision_type"),
            "goals": sorted(context.data.get("goals", [])),
            "constraints": sorted(context.data.get("constraints", [])),
            "state": context.data.get("current_state", {})
        }

        # Generate hash
        context_str = json.dumps(context_repr, sort_keys=True)
        return hashlib.sha256(context_str.encode()).hexdigest()

    def _get_batch_key(self, context: AgentContext) -> str:
        """Generate batch key for grouping similar decisions"""
        entity_type = context.data.get("entity_type", "unknown")
        decision_type = context.data.get("decision_type", "unknown")
        return f"{entity_type}:{decision_type}"

    async def _find_similar_decision(self, context: AgentContext) -> Optional[CachedDecision]:
        """Find similar cached decision using embedding similarity"""
        # This would use vector embeddings to find similar decisions
        # For now, return None (will be implemented with vector search)
        return None

    def get_statistics(self) -> Dict[str, Any]:
        """Get optimization statistics"""
        total_decisions = sum([
            self.stats["tier1_count"],
            self.stats["tier2_count"],
            self.stats["tier3_count"],
            self.stats["tier4_count"]
        ])

        if total_decisions == 0:
            return self.stats

        return {
            **self.stats,
            "total_decisions": total_decisions,
            "tier1_percentage": (self.stats["tier1_count"] / total_decisions) * 100,
            "tier2_percentage": (self.stats["tier2_count"] / total_decisions) * 100,
            "tier3_percentage": (self.stats["tier3_count"] / total_decisions) * 100,
            "tier4_percentage": (self.stats["tier4_count"] / total_decisions) * 100,
            "llm_call_reduction": (self.stats["total_llm_calls_saved"] /
                                  (self.stats["total_llm_calls"] + self.stats["total_llm_calls_saved"]) * 100
                                  if (self.stats["total_llm_calls"] + self.stats["total_llm_calls_saved"]) > 0 else 0)
        }


# Global instance
decision_optimizer = DecisionOptimizer()
