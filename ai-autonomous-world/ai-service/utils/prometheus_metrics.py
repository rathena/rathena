"""
Comprehensive Prometheus Metrics for AI Autonomous World
Tracks agent performance, LLM providers, decision layers, economy, and more
"""

from prometheus_client import Counter, Gauge, Histogram, Summary
from typing import Optional

# ============================================================================
# AGENT PERFORMANCE METRICS
# ============================================================================

agent_request_duration = Histogram(
    "agent_request_duration_seconds",
    "Agent request processing time",
    ["agent_type", "operation"],
    buckets=[0.001, 0.01, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0]
)

agent_requests_total = Counter(
    "agent_requests_total",
    "Total agent requests",
    ["agent_type", "operation", "status"]
)

agent_errors_total = Counter(
    "agent_errors_total",
    "Total agent errors",
    ["agent_type", "error_type"]
)

agent_confidence_score = Histogram(
    "agent_confidence_score",
    "Agent decision confidence scores",
    ["agent_type"],
    buckets=[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
)

# ============================================================================
# LLM PROVIDER METRICS (Enhanced)
# ============================================================================

llm_request_duration = Histogram(
    "llm_request_duration_seconds",
    "LLM request duration",
    ["provider", "model", "operation"],
    buckets=[0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 30.0, 60.0]
)

llm_token_usage = Counter(
    "llm_token_usage_total",
    "Total tokens used",
    ["provider", "model", "token_type"]  # token_type: input/output/total
)

llm_provider_availability = Gauge(
    "llm_provider_availability",
    "Provider availability (1=up, 0=down)",
    ["provider"]
)

llm_requests_by_provider = Counter(
    "llm_requests_by_provider_total",
    "Total requests per provider",
    ["provider", "status"]  # status: success/failure/timeout
)

llm_cost_estimate = Counter(
    "llm_cost_estimate_usd",
    "Estimated cost in USD",
    ["provider", "model"]
)

llm_response_tokens = Histogram(
    "llm_response_tokens",
    "Distribution of response token counts",
    ["provider", "model"],
    buckets=[10, 50, 100, 250, 500, 1000, 2000, 4000]
)

# ============================================================================
# DECISION LAYER METRICS
# ============================================================================

decision_layer_usage = Counter(
    "decision_layer_usage_total",
    "Decision layer usage count",
    ["layer", "outcome"]  # outcome: selected/skipped/timeout/fallback
)

decision_layer_latency = Histogram(
    "decision_layer_latency_seconds",
    "Decision layer processing time",
    ["layer"],
    buckets=[0.001, 0.01, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0]
)

decision_layer_confidence = Histogram(
    "decision_layer_confidence",
    "Decision confidence by layer",
    ["layer"],
    buckets=[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
)

decision_fallback_count = Counter(
    "decision_fallback_total",
    "Count of fallback decisions",
    ["from_layer", "to_layer"]
)

# ============================================================================
# UTILITY SYSTEM METRICS
# ============================================================================

utility_factor_values = Histogram(
    "utility_factor_values",
    "Utility factor value distribution",
    ["factor"],  # safety, hunger, social, curiosity, aggression
    buckets=[0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
)

utility_decision_score = Histogram(
    "utility_decision_score",
    "Utility-based decision scores",
    buckets=[0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
)

utility_action_selection = Counter(
    "utility_action_selection_total",
    "Actions selected by utility system",
    ["action_type"]
)

# ============================================================================
# ECONOMIC METRICS
# ============================================================================

economic_agent_actions = Counter(
    "economic_agent_actions_total",
    "Economic agent actions",
    ["agent_type", "action"]  # merchant/buy, craftsman/produce, etc.
)

production_chain_executions = Counter(
    "production_chain_executions_total",
    "Production chain execution count",
    ["chain_id", "status"]  # success/failure/partial
)

trade_route_volume = Counter(
    "trade_route_volume_total",
    "Trade volume by route",
    ["from_location", "to_location", "item"]
)

economic_price_changes = Histogram(
    "economic_price_changes",
    "Price change magnitudes",
    ["item_type"],
    buckets=[-1.0, -0.5, -0.2, -0.1, 0.0, 0.1, 0.2, 0.5, 1.0]
)

market_transaction_value = Histogram(
    "market_transaction_value",
    "Transaction values in game currency",
    buckets=[10, 100, 1000, 10000, 100000, 1000000]
)

# ============================================================================
# MEMORY & RELATIONSHIP METRICS
# ============================================================================

memory_operations = Counter(
    "memory_operations_total",
    "Memory operations",
    ["operation", "memory_type"]  # operation: store/retrieve, type: episodic/semantic/etc
)

relationship_changes = Counter(
    "relationship_changes_total",
    "Relationship value changes",
    ["change_type"]  # positive/negative/neutral
)

relationship_strength = Histogram(
    "relationship_strength",
    "Relationship strength values",
    buckets=[-1.0, -0.5, 0.0, 0.25, 0.5, 0.75, 1.0]
)

memory_retrieval_latency = Histogram(
    "memory_retrieval_latency_seconds",
    "Memory retrieval time",
    ["memory_type"],
    buckets=[0.001, 0.01, 0.05, 0.1, 0.5, 1.0]
)

# ============================================================================
# QUEST METRICS
# ============================================================================

quest_generation = Counter(
    "quest_generation_total",
    "Quest generation events",
    ["quest_type", "difficulty", "source"]  # source: llm/template/dynamic
)

quest_completion = Counter(
    "quest_completion_total",
    "Quest completion events",
    ["quest_type", "difficulty", "outcome"]  # outcome: success/failure/abandoned
)

quest_progress = Gauge(
    "quest_progress",
    "Active quest progress percentage",
    ["quest_id", "quest_type"]
)

quest_reward_value = Histogram(
    "quest_reward_value",
    "Quest reward values",
    buckets=[10, 100, 500, 1000, 5000, 10000, 50000]
)

# ============================================================================
# DATABASE METRICS (Enhanced)
# ============================================================================

database_query_duration = Histogram(
    "database_query_duration_seconds",
    "Database query execution time",
    ["operation", "table"],
    buckets=[0.001, 0.01, 0.05, 0.1, 0.5, 1.0, 5.0]
)

database_connection_pool = Gauge(
    "database_connection_pool_size",
    "Database connection pool metrics",
    ["state", "database"]  # state: total/idle/active, database: postgres/dragonfly
)

database_errors = Counter(
    "database_errors_total",
    "Database errors",
    ["operation", "error_type", "database"]
)

database_query_count = Counter(
    "database_query_count_total",
    "Total database queries",
    ["operation", "table", "database"]
)

# ============================================================================
# NPC BEHAVIOR METRICS
# ============================================================================

npc_spawn_events = Counter(
    "npc_spawn_events_total",
    "NPC spawn events",
    ["npc_type", "location"]
)

npc_death_events = Counter(
    "npc_death_events_total",
    "NPC death events",
    ["npc_type", "cause"]
)

npc_interaction_duration = Histogram(
    "npc_interaction_duration_seconds",
    "NPC interaction duration",
    ["interaction_type"],
    buckets=[1, 5, 10, 30, 60, 300, 600]
)

npc_dialogue_length = Histogram(
    "npc_dialogue_length_chars",
    "NPC dialogue response length",
    ["npc_type"],
    buckets=[10, 50, 100, 200, 500, 1000, 2000]
)

# ============================================================================
# WORLD SIMULATION METRICS
# ============================================================================

world_tick_duration = Histogram(
    "world_tick_duration_seconds",
    "World simulation tick duration",
    buckets=[0.01, 0.05, 0.1, 0.5, 1.0, 2.0, 5.0]
)

active_npcs = Gauge(
    "active_npcs",
    "Number of active NPCs",
    ["location", "type"]
)

active_players = Gauge(
    "active_players",
    "Number of active players",
    ["location"]
)

world_events = Counter(
    "world_events_total",
    "World events",
    ["event_type", "location"]
)

# ============================================================================
# COST MANAGEMENT METRICS
# ============================================================================

cost_budget_remaining = Gauge(
    "cost_budget_remaining_usd",
    "Remaining budget in USD",
    ["budget_type"]  # daily/provider/total
)

cost_budget_utilization = Gauge(
    "cost_budget_utilization_percentage",
    "Budget utilization percentage",
    ["budget_type"]
)

cost_alert_events = Counter(
    "cost_alert_events_total",
    "Cost alert events",
    ["alert_type", "provider"]  # warning/critical/exceeded
)

# ============================================================================
# FALLBACK CHAIN METRICS
# ============================================================================

fallback_chain_attempts = Counter(
    "fallback_chain_attempts_total",
    "Fallback chain attempts",
    ["initial_provider", "final_provider", "success"]
)

fallback_chain_latency = Histogram(
    "fallback_chain_latency_seconds",
    "Total fallback chain latency",
    ["provider_count"],
    buckets=[0.5, 1.0, 2.0, 5.0, 10.0, 30.0, 60.0]
)

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def record_agent_request(
    agent_type: str,
    operation: str,
    duration: float,
    status: str,
    confidence: Optional[float] = None,
    error_type: Optional[str] = None
):
    """
    Record agent request metrics
    
    Args:
        agent_type: Type of agent (dialogue, quest, decision, etc.)
        operation: Operation performed (generate, process, decide)
        duration: Request duration in seconds
        status: Request status (success/failure)
        confidence: Optional confidence score (0.0-1.0)
        error_type: Optional error type if failed
    """
    agent_request_duration.labels(
        agent_type=agent_type,
        operation=operation
    ).observe(duration)
    
    agent_requests_total.labels(
        agent_type=agent_type,
        operation=operation,
        status=status
    ).inc()
    
    if confidence is not None:
        agent_confidence_score.labels(
            agent_type=agent_type
        ).observe(confidence)
    
    if error_type:
        agent_errors_total.labels(
            agent_type=agent_type,
            error_type=error_type
        ).inc()


def record_llm_request(
    provider: str,
    model: str,
    operation: str,
    duration: float,
    status: str,
    input_tokens: Optional[int] = None,
    output_tokens: Optional[int] = None,
    cost_usd: Optional[float] = None
):
    """
    Record LLM provider request metrics
    
    Args:
        provider: Provider name (openai, anthropic, etc.)
        model: Model name
        operation: Operation type (generate, embed, etc.)
        duration: Request duration in seconds
        status: Request status (success/failure/timeout)
        input_tokens: Input token count
        output_tokens: Output token count
        cost_usd: Estimated cost in USD
    """
    llm_request_duration.labels(
        provider=provider,
        model=model,
        operation=operation
    ).observe(duration)
    
    llm_requests_by_provider.labels(
        provider=provider,
        status=status
    ).inc()
    
    if input_tokens:
        llm_token_usage.labels(
            provider=provider,
            model=model,
            token_type="input"
        ).inc(input_tokens)
    
    if output_tokens:
        llm_token_usage.labels(
            provider=provider,
            model=model,
            token_type="output"
        ).inc(output_tokens)
        
        llm_response_tokens.labels(
            provider=provider,
            model=model
        ).observe(output_tokens)
    
    if cost_usd:
        llm_cost_estimate.labels(
            provider=provider,
            model=model
        ).inc(cost_usd)
    
    # Update availability (1 if success, keep previous if failure)
    if status == "success":
        llm_provider_availability.labels(provider=provider).set(1)


def record_decision_layer(
    layer: str,
    outcome: str,
    latency: float,
    confidence: Optional[float] = None,
    fallback_from: Optional[str] = None
):
    """
    Record decision layer metrics
    
    Args:
        layer: Decision layer (reflex/reactive/deliberative/social/strategic)
        outcome: Outcome (selected/skipped/timeout/fallback)
        latency: Processing latency in seconds
        confidence: Optional confidence score
        fallback_from: If fallback, the original layer
    """
    decision_layer_usage.labels(
        layer=layer,
        outcome=outcome
    ).inc()
    
    decision_layer_latency.labels(
        layer=layer
    ).observe(latency)
    
    if confidence is not None:
        decision_layer_confidence.labels(
            layer=layer
        ).observe(confidence)
    
    if fallback_from:
        decision_fallback_count.labels(
            from_layer=fallback_from,
            to_layer=layer
        ).inc()