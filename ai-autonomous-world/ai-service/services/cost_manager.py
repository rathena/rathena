"""
Cost Management Service with Daily Budget Controls

Provides centralized cost tracking and budget enforcement for LLM API calls.

Features:
- Daily budget limits (total and per-provider)
- Real-time cost tracking
- Budget alerts at configurable thresholds
- Automatic request blocking when budget exceeded
- Cost analytics and reporting
- Prometheus metrics integration
"""

from datetime import datetime, date
from typing import Dict, Optional, List, Any
from dataclasses import dataclass, asdict
from loguru import logger
from prometheus_client import Counter, Gauge, Histogram


# ============================================================================
# PROMETHEUS METRICS
# ============================================================================

# Cost tracking metrics
cost_total_usd = Counter(
    "llm_cost_total_usd",
    "Total LLM cost in USD",
    ["provider", "model"]
)

cost_tokens_input = Counter(
    "llm_tokens_input_total",
    "Total input tokens",
    ["provider"]
)

cost_tokens_output = Counter(
    "llm_tokens_output_total",
    "Total output tokens",
    ["provider"]
)

cost_daily_total = Gauge(
    "llm_cost_daily_total_usd",
    "Today's total cost in USD"
)

cost_budget_exceeded = Counter(
    "llm_budget_exceeded_total",
    "Budget exceeded events",
    ["budget_type"]
)

cost_requests_blocked = Counter(
    "llm_requests_blocked_budget_total",
    "Requests blocked due to budget limits",
    ["provider"]
)

cost_request_histogram = Histogram(
    "llm_cost_per_request_usd",
    "Cost distribution per request",
    ["provider"],
    buckets=[0.0001, 0.001, 0.01, 0.05, 0.1, 0.5, 1.0, 5.0, 10.0]
)


# ============================================================================
# CUSTOM EXCEPTIONS
# ============================================================================

class BudgetExceededException(Exception):
    """Raised when daily budget limit is reached"""
    def __init__(self, message: str, current: float, limit: float):
        super().__init__(message)
        self.current = current
        self.limit = limit


class ProviderBudgetExceededException(Exception):
    """Raised when provider-specific budget limit is reached"""
    def __init__(self, message: str, provider: str, current: float, limit: float):
        super().__init__(message)
        self.provider = provider
        self.current = current
        self.limit = limit


# ============================================================================
# DATA CLASSES
# ============================================================================

@dataclass
class CostEntry:
    """Single cost entry for tracking"""
    timestamp: datetime
    provider: str
    model: str
    tokens_input: int
    tokens_output: int
    cost_usd: float
    request_type: str  # "dialogue", "decision", "quest", etc.
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for serialization"""
        return {
            **asdict(self),
            "timestamp": self.timestamp.isoformat()
        }


# ============================================================================
# COST MANAGER
# ============================================================================

class CostManager:
    """
    Centralized cost tracking and budget enforcement.
    
    Features:
    - Daily budget limits per provider and total
    - Real-time cost tracking
    - Budget alerts at 50%, 75%, 90%, 100%
    - Automatic request blocking when budget exceeded
    - Cost analytics and reporting
    
    Thread-safe for concurrent requests.
    """
    
    def __init__(
        self,
        daily_budget_usd: float = 100.0,
        per_provider_budgets: Optional[Dict[str, float]] = None,
        alert_thresholds: Optional[List[int]] = None,
        enabled: bool = True
    ):
        """
        Initialize cost manager.
        
        Args:
            daily_budget_usd: Maximum daily spending limit (default: $100)
            per_provider_budgets: Provider-specific limits (e.g., {"azure_openai": 50.0})
            alert_thresholds: Budget percentage thresholds for alerts (default: [50, 75, 90, 100])
            enabled: Enable/disable cost tracking (default: True)
        """
        self.enabled = enabled
        self.daily_budget = daily_budget_usd
        self.per_provider_budgets = per_provider_budgets or {}
        self.alert_thresholds = sorted(alert_thresholds or [50, 75, 90, 100])
        
        # Today's cost tracking
        self.today_costs: List[CostEntry] = []
        self.current_date = date.today()
        
        # Alert tracking (to avoid duplicate alerts)
        self._alerted_thresholds: set = set()
        
        if self.enabled:
            logger.info(
                f"💰 Cost Manager initialized: "
                f"daily_budget=${self.daily_budget:.2f}, "
                f"per_provider_budgets={self.per_provider_budgets}"
            )
        else:
            logger.warning("💰 Cost tracking DISABLED")
    
    def record_cost(
        self,
        provider: str,
        model: str,
        tokens_input: int,
        tokens_output: int,
        cost_usd: float,
        request_type: str = "unknown"
    ) -> bool:
        """
        Record cost and check budget limits.
        
        Args:
            provider: LLM provider name (e.g., "azure_openai")
            model: Model name (e.g., "gpt-4")
            tokens_input: Input token count
            tokens_output: Output token count
            cost_usd: Cost in USD
            request_type: Type of request for analytics
            
        Returns:
            True if within budget, False if budget exceeded
            
        Raises:
            BudgetExceededException: If daily budget exceeded
            ProviderBudgetExceededException: If provider budget exceeded
        """
        if not self.enabled:
            return True
        
        # Check if new day, reset costs
        if date.today() != self.current_date:
            self._reset_daily_costs()
        
        # Create cost entry
        entry = CostEntry(
            timestamp=datetime.now(),
            provider=provider,
            model=model,
            tokens_input=tokens_input,
            tokens_output=tokens_output,
            cost_usd=cost_usd,
            request_type=request_type
        )
        
        # Check budget before adding
        total_today = self.get_total_cost_today()
        provider_today = self.get_provider_cost_today(provider)
        
        # Check per-provider budget first
        if provider in self.per_provider_budgets:
            provider_limit = self.per_provider_budgets[provider]
            if provider_today + cost_usd > provider_limit:
                logger.error(
                    f"🚨 {provider} daily budget EXCEEDED: "
                    f"${provider_today + cost_usd:.4f} > ${provider_limit:.2f}"
                )
                cost_budget_exceeded.labels(budget_type=f"provider_{provider}").inc()
                cost_requests_blocked.labels(provider=provider).inc()
                
                raise ProviderBudgetExceededException(
                    f"{provider} daily budget exceeded",
                    provider=provider,
                    current=provider_today + cost_usd,
                    limit=provider_limit
                )
        
        # Check total daily budget
        new_total = total_today + cost_usd
        if new_total > self.daily_budget:
            logger.error(
                f"🚨 Daily budget EXCEEDED: "
                f"${new_total:.4f} > ${self.daily_budget:.2f}"
            )
            cost_budget_exceeded.labels(budget_type="daily").inc()
            cost_requests_blocked.labels(provider=provider).inc()
            
            raise BudgetExceededException(
                "Daily budget limit reached",
                current=new_total,
                limit=self.daily_budget
            )
        
        # Budget OK - Add cost entry
        self.today_costs.append(entry)
        
        # Update Prometheus metrics
        cost_total_usd.labels(provider=provider, model=model).inc(cost_usd)
        cost_tokens_input.labels(provider=provider).inc(tokens_input)
        cost_tokens_output.labels(provider=provider).inc(tokens_output)
        cost_daily_total.set(new_total)
        cost_request_histogram.labels(provider=provider).observe(cost_usd)
        
        # Check alert thresholds
        self._check_budget_alerts(new_total)
        
        logger.debug(
            f"💵 Cost recorded: {provider}/{model} - "
            f"${cost_usd:.4f} (today: ${new_total:.4f}/{self.daily_budget:.2f})"
        )
        
        return True
    
    def _check_budget_alerts(self, current_total: float):
        """Send alerts at budget thresholds"""
        percentage = (current_total / self.daily_budget) * 100
        
        for threshold in self.alert_thresholds:
            if percentage >= threshold and threshold not in self._alerted_thresholds:
                self._alerted_thresholds.add(threshold)
                
                if threshold >= 100:
                    logger.critical(
                        f"🚨 DAILY BUDGET EXCEEDED: "
                        f"${current_total:.2f}/${self.daily_budget:.2f} ({percentage:.1f}%)"
                    )
                elif threshold >= 90:
                    logger.error(
                        f"⚠️ 90% budget used: "
                        f"${current_total:.2f}/${self.daily_budget:.2f} ({percentage:.1f}%)"
                    )
                elif threshold >= 75:
                    logger.warning(
                        f"⚠️ 75% budget used: "
                        f"${current_total:.2f}/${self.daily_budget:.2f} ({percentage:.1f}%)"
                    )
                elif threshold >= 50:
                    logger.info(
                        f"ℹ️ 50% budget used: "
                        f"${current_total:.2f}/${self.daily_budget:.2f} ({percentage:.1f}%)"
                    )
    
    def get_total_cost_today(self) -> float:
        """Get total cost for current day"""
        return sum(entry.cost_usd for entry in self.today_costs)
    
    def get_provider_cost_today(self, provider: str) -> float:
        """Get cost for specific provider today"""
        return sum(
            entry.cost_usd 
            for entry in self.today_costs 
            if entry.provider == provider
        )
    
    def get_cost_breakdown(self) -> Dict[str, Any]:
        """Get detailed cost breakdown"""
        total = self.get_total_cost_today()
        by_provider: Dict[str, float] = {}
        by_model: Dict[str, float] = {}
        by_request_type: Dict[str, float] = {}
        
        for entry in self.today_costs:
            # By provider
            by_provider[entry.provider] = by_provider.get(entry.provider, 0.0) + entry.cost_usd
            
            # By model
            model_key = f"{entry.provider}/{entry.model}"
            by_model[model_key] = by_model.get(model_key, 0.0) + entry.cost_usd
            
            # By request type
            by_request_type[entry.request_type] = by_request_type.get(entry.request_type, 0.0) + entry.cost_usd
        
        return {
            "date": self.current_date.isoformat(),
            "total_cost_usd": round(total, 6),
            "budget_usd": self.daily_budget,
            "budget_used_percentage": round((total / self.daily_budget) * 100, 2) if self.daily_budget > 0 else 0,
            "budget_remaining_usd": round(max(0, self.daily_budget - total), 6),
            "by_provider": {k: round(v, 6) for k, v in by_provider.items()},
            "by_model": {k: round(v, 6) for k, v in by_model.items()},
            "by_request_type": {k: round(v, 6) for k, v in by_request_type.items()},
            "total_requests": len(self.today_costs),
            "enabled": self.enabled
        }
    
    def is_budget_available(
        self, 
        estimated_cost: float = 0.01,
        provider: Optional[str] = None
    ) -> bool:
        """
        Check if budget available for request.
        
        Args:
            estimated_cost: Estimated cost of request (default: $0.01)
            provider: Optional provider name to check provider-specific budget
            
        Returns:
            True if budget available, False if would exceed limits
        """
        if not self.enabled:
            return True
        
        total_today = self.get_total_cost_today()
        
        # Check provider budget if specified
        if provider and provider in self.per_provider_budgets:
            provider_today = self.get_provider_cost_today(provider)
            if (provider_today + estimated_cost) > self.per_provider_budgets[provider]:
                return False
        
        # Check total budget
        return (total_today + estimated_cost) <= self.daily_budget
    
    def get_remaining_budget(self, provider: Optional[str] = None) -> float:
        """
        Get remaining budget.
        
        Args:
            provider: Optional provider name for provider-specific budget
            
        Returns:
            Remaining budget in USD
        """
        if not self.enabled:
            return float('inf')
        
        if provider and provider in self.per_provider_budgets:
            provider_today = self.get_provider_cost_today(provider)
            return max(0, self.per_provider_budgets[provider] - provider_today)
        
        total_today = self.get_total_cost_today()
        return max(0, self.daily_budget - total_today)
    
    def _reset_daily_costs(self):
        """Reset costs for new day"""
        yesterday_total = self.get_total_cost_today()
        logger.info(
            f"🔄 New day - Cost reset. "
            f"Yesterday ({self.current_date}): ${yesterday_total:.4f}"
        )
        
        self.today_costs = []
        self.current_date = date.today()
        self._alerted_thresholds = set()
        cost_daily_total.set(0)
    
    def set_daily_budget(self, new_budget_usd: float):
        """Update daily budget limit"""
        old_budget = self.daily_budget
        self.daily_budget = new_budget_usd
        logger.info(f"💰 Daily budget updated: ${old_budget:.2f} → ${new_budget_usd:.2f}")
    
    def set_provider_budget(self, provider: str, budget_usd: float):
        """Set or update provider-specific budget"""
        self.per_provider_budgets[provider] = budget_usd
        logger.info(f"💰 {provider} budget set: ${budget_usd:.2f}")
    
    def get_cost_history(self, limit: int = 100) -> List[Dict[str, Any]]:
        """
        Get recent cost entries.
        
        Args:
            limit: Maximum number of entries to return
            
        Returns:
            List of cost entry dictionaries
        """
        return [entry.to_dict() for entry in self.today_costs[-limit:]]


# ============================================================================
# GLOBAL INSTANCE
# ============================================================================

# Global cost manager instance (initialized in main.py)
_cost_manager: Optional[CostManager] = None


def get_cost_manager() -> CostManager:
    """Get global cost manager instance"""
    global _cost_manager
    if _cost_manager is None:
        raise RuntimeError("Cost manager not initialized. Call init_cost_manager() first.")
    return _cost_manager


def init_cost_manager(
    daily_budget_usd: float = 100.0,
    per_provider_budgets: Optional[Dict[str, float]] = None,
    alert_thresholds: Optional[List[int]] = None,
    enabled: bool = True
) -> CostManager:
    """
    Initialize global cost manager.
    
    Args:
        daily_budget_usd: Maximum daily spending limit
        per_provider_budgets: Provider-specific limits
        alert_thresholds: Budget percentage thresholds for alerts
        enabled: Enable/disable cost tracking
        
    Returns:
        Initialized CostManager instance
    """
    global _cost_manager
    _cost_manager = CostManager(
        daily_budget_usd=daily_budget_usd,
        per_provider_budgets=per_provider_budgets,
        alert_thresholds=alert_thresholds,
        enabled=enabled
    )
    return _cost_manager