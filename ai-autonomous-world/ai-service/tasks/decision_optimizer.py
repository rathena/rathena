"""
Decision Optimizer

Learns from past decisions to improve future decision-making.
Analyzes decision patterns and caches successful strategies.

Features:
- Pattern identification in successful decisions
- Decision caching for common scenarios
- Success rate tracking
- Adaptive weighting
- PostgreSQL integration for decision history
"""

import asyncio
import logging
from typing import Dict, List, Optional, Any, Tuple
from datetime import datetime, timedelta
from collections import defaultdict

logger = logging.getLogger(__name__)


class DecisionPattern:
    """Represents a learned decision pattern."""
    
    def __init__(
        self,
        context: Dict[str, Any],
        decision: str,
        success_rate: float,
        sample_size: int
    ):
        """
        Initialize decision pattern.
        
        Args:
            context: Context that led to decision
            decision: Decision that was made
            success_rate: Success rate (0.0-1.0)
            sample_size: Number of samples
        """
        self.context = context
        self.decision = decision
        self.success_rate = success_rate
        self.sample_size = sample_size
        self.last_updated = datetime.utcnow()


class DecisionOptimizer:
    """
    Analyzes past decisions to optimize future ones.
    
    Identifies successful decision patterns and caches them
    for quick lookup in similar situations.
    """
    
    def __init__(
        self,
        db,
        postgres,
        interval_seconds: int = 1800,
        min_samples: int = 5,
        success_threshold: float = 0.7
    ):
        """
        Initialize decision optimizer.
        
        Args:
            db: DragonflyDB connection
            postgres: PostgreSQL connection
            interval_seconds: Analysis interval (default: 1800s = 30min)
            min_samples: Minimum samples for pattern
            success_threshold: Minimum success rate for caching
        """
        self.db = db
        self.postgres = postgres
        self.interval = interval_seconds
        self.min_samples = min_samples
        self.success_threshold = success_threshold
        self.running = False
        self.task: Optional[asyncio.Task] = None
        
        # Pattern cache
        self.patterns: Dict[str, DecisionPattern] = {}
        
        # Metrics
        self.metrics = {
            'decisions_analyzed': 0,
            'patterns_identified': 0,
            'patterns_cached': 0,
            'cache_hits': 0,
            'cache_misses': 0
        }
    
    async def start(self):
        """Start periodic decision analysis."""
        if self.running:
            logger.warning("Decision optimizer already running")
            return
        
        self.running = True
        self.task = asyncio.create_task(self._run_loop())
        logger.info(
            f"Decision optimizer started (interval: {self.interval}s)"
        )
    
    async def stop(self):
        """Stop decision analysis."""
        self.running = False
        
        if self.task:
            self.task.cancel()
            try:
                await self.task
            except asyncio.CancelledError:
                pass
        
        logger.info("Decision optimizer stopped")
    
    async def _run_loop(self):
        """Main analysis loop."""
        while self.running:
            try:
                await self._analyze_decisions()
                await self._cache_patterns()
                await asyncio.sleep(self.interval)
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Decision analysis error: {e}")
                await asyncio.sleep(60)
    
    async def _analyze_decisions(self):
        """Analyze recent decisions to find patterns."""
        try:
            # Fetch recent decisions from PostgreSQL
            decisions = await self._fetch_recent_decisions()
            
            logger.debug(f"Analyzing {len(decisions)} decisions")
            
            # Group by context similarity
            grouped = self._group_similar_contexts(decisions)
            
            # Analyze each group
            for context_key, group in grouped.items():
                pattern = await self._analyze_group(context_key, group)
                if pattern:
                    self.patterns[context_key] = pattern
                    self.metrics['patterns_identified'] += 1
            
            self.metrics['decisions_analyzed'] += len(decisions)
            
            logger.info(
                f"Analyzed {len(decisions)} decisions, "
                f"identified {len(self.patterns)} patterns"
            )
            
        except Exception as e:
            logger.error(f"Failed to analyze decisions: {e}")
            raise
    
    async def _fetch_recent_decisions(
        self,
        hours: int = 24
    ) -> List[Dict[str, Any]]:
        """
        Fetch recent decisions from database.
        
        Args:
            hours: Hours to look back
            
        Returns:
            List of decision records
        """
        # TODO: Implement actual PostgreSQL query
        # Mock implementation
        cutoff = datetime.utcnow() - timedelta(hours=hours)
        
        return [
            {
                'entity_id': 'npc-1',
                'context': {'mood': 'friendly', 'location': 'tavern'},
                'decision': 'offer_quest',
                'outcome': 'success',
                'timestamp': datetime.utcnow() - timedelta(hours=2)
            },
            {
                'entity_id': 'npc-1',
                'context': {'mood': 'friendly', 'location': 'tavern'},
                'decision': 'offer_quest',
                'outcome': 'success',
                'timestamp': datetime.utcnow() - timedelta(hours=5)
            },
            {
                'entity_id': 'npc-2',
                'context': {'mood': 'busy', 'location': 'market'},
                'decision': 'decline_interaction',
                'outcome': 'success',
                'timestamp': datetime.utcnow() - timedelta(hours=1)
            }
        ]
    
    def _group_similar_contexts(
        self,
        decisions: List[Dict[str, Any]]
    ) -> Dict[str, List[Dict[str, Any]]]:
        """
        Group decisions by context similarity.
        
        Args:
            decisions: List of decisions
            
        Returns:
            Dict mapping context key to decisions
        """
        grouped = defaultdict(list)
        
        for decision in decisions:
            context_key = self._make_context_key(decision['context'])
            grouped[context_key].append(decision)
        
        return dict(grouped)
    
    def _make_context_key(self, context: Dict[str, Any]) -> str:
        """
        Create context key for grouping.
        
        Args:
            context: Context dict
            
        Returns:
            Context key string
        """
        # Simple key based on major context factors
        mood = context.get('mood', 'neutral')
        location = context.get('location', 'unknown')
        return f"{mood}:{location}"
    
    async def _analyze_group(
        self,
        context_key: str,
        decisions: List[Dict[str, Any]]
    ) -> Optional[DecisionPattern]:
        """
        Analyze a group of similar decisions.
        
        Args:
            context_key: Context key
            decisions: List of decisions with similar context
            
        Returns:
            DecisionPattern or None
        """
        if len(decisions) < self.min_samples:
            return None
        
        # Count decision outcomes
        decision_stats = defaultdict(lambda: {'success': 0, 'total': 0})
        
        for dec in decisions:
            decision_type = dec['decision']
            decision_stats[decision_type]['total'] += 1
            if dec['outcome'] == 'success':
                decision_stats[decision_type]['success'] += 1
        
        # Find best decision
        best_decision = None
        best_rate = 0.0
        best_samples = 0
        
        for decision, stats in decision_stats.items():
            rate = stats['success'] / stats['total']
            if rate > best_rate and stats['total'] >= self.min_samples:
                best_decision = decision
                best_rate = rate
                best_samples = stats['total']
        
        # Create pattern if success rate meets threshold
        if best_decision and best_rate >= self.success_threshold:
            context = decisions[0]['context']
            return DecisionPattern(
                context=context,
                decision=best_decision,
                success_rate=best_rate,
                sample_size=best_samples
            )
        
        return None
    
    async def _cache_patterns(self):
        """Cache successful patterns in DragonflyDB."""
        cached_count = 0
        
        for context_key, pattern in self.patterns.items():
            try:
                cache_data = {
                    'decision': pattern.decision,
                    'success_rate': pattern.success_rate,
                    'sample_size': pattern.sample_size,
                    'last_updated': pattern.last_updated.isoformat()
                }
                
                # TODO: Store in DragonflyDB
                logger.debug(f"Cached pattern: {context_key} -> {cache_data}")
                cached_count += 1
                
            except Exception as e:
                logger.error(f"Failed to cache pattern {context_key}: {e}")
        
        self.metrics['patterns_cached'] += cached_count
        
        if cached_count > 0:
            logger.info(f"Cached {cached_count} decision patterns")
    
    async def get_recommended_decision(
        self,
        context: Dict[str, Any]
    ) -> Optional[Tuple[str, float]]:
        """
        Get recommended decision for given context.
        
        Args:
            context: Current context
            
        Returns:
            Tuple of (decision, confidence) or None
        """
        context_key = self._make_context_key(context)
        
        # Check cache
        if context_key in self.patterns:
            pattern = self.patterns[context_key]
            self.metrics['cache_hits'] += 1
            return (pattern.decision, pattern.success_rate)
        
        # Check DragonflyDB
        try:
            cached = await self._get_from_cache(context_key)
            if cached:
                self.metrics['cache_hits'] += 1
                return (cached['decision'], cached['success_rate'])
        except Exception as e:
            logger.error(f"Cache lookup error: {e}")
        
        self.metrics['cache_misses'] += 1
        return None
    
    async def _get_from_cache(
        self,
        context_key: str
    ) -> Optional[Dict[str, Any]]:
        """
        Get pattern from DragonflyDB cache.
        
        Args:
            context_key: Context key
            
        Returns:
            Cached pattern or None
        """
        # TODO: Implement actual DragonflyDB query
        return None
    
    def get_pattern_stats(self) -> List[Dict[str, Any]]:
        """
        Get statistics for all patterns.
        
        Returns:
            List of pattern statistics
        """
        stats = []
        
        for context_key, pattern in self.patterns.items():
            stats.append({
                'context_key': context_key,
                'decision': pattern.decision,
                'success_rate': pattern.success_rate,
                'sample_size': pattern.sample_size,
                'last_updated': pattern.last_updated.isoformat()
            })
        
        # Sort by success rate
        stats.sort(key=lambda x: x['success_rate'], reverse=True)
        
        return stats
    
    def get_metrics(self) -> Dict[str, Any]:
        """
        Get optimizer metrics.
        
        Returns:
            Dict with metrics
        """
        metrics = self.metrics.copy()
        
        # Calculate hit rate
        total_lookups = metrics['cache_hits'] + metrics['cache_misses']
        if total_lookups > 0:
            metrics['cache_hit_rate'] = metrics['cache_hits'] / total_lookups
        else:
            metrics['cache_hit_rate'] = 0.0
        
        metrics['pattern_count'] = len(self.patterns)
        
        return metrics
    
    def reset_metrics(self):
        """Reset all metrics to zero."""
        self.metrics = {
            'decisions_analyzed': 0,
            'patterns_identified': 0,
            'patterns_cached': 0,
            'cache_hits': 0,
            'cache_misses': 0
        }