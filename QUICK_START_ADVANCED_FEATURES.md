# Quick Start Guide - Advanced Autonomous Features

## 🚀 5-Minute Setup

### 1. Run Database Migration

```bash
cd rathena-AI-world
psql -U postgres -d ai_world -f database/migrations/004_advanced_autonomous_features.sql
```

### 2. Update Environment Variables

Add to your `.env` file:

```env
# Enable all advanced features
NPC_TO_NPC_INTERACTIONS_ENABLED=true
INSTANT_RESPONSE_ENABLED=true
UNIVERSAL_CONSCIOUSNESS_ENABLED=true
LLM_OPTIMIZATION_MODE=balanced

# NPC Interactions
NPC_INTERACTION_RANGE=5
NPC_PROXIMITY_CHECK_INTERVAL=30

# Reasoning
REASONING_DEPTH=deep
FUTURE_PLANNING_ENABLED=true

# Optimization
DECISION_CACHE_ENABLED=true
DECISION_BATCH_ENABLED=true
```

### 3. Restart AI Service

```bash
cd ai-autonomous-world/ai-service
python main.py
```

### 4. Run Tests

```bash
cd rathena-AI-world
python test_advanced_autonomous_features.py
```

---

## 📖 Common Use Cases

### Use Case 1: Enable NPC Conversations

```python
from ai_service.tasks.npc_relationships import npc_relationship_manager

# NPCs will automatically detect each other and interact
# based on proximity and relationship
await npc_relationship_manager.initialize()
await npc_relationship_manager.start_background_tasks()
```

### Use Case 2: Handle Combat Events Instantly

```python
from ai_service.tasks.instant_response import instant_response_manager, EventPriority

# Combat events get instant priority automatically
await instant_response_manager.handle_event(
    event_type="combat",
    event_data={"npc_id": "NPC_001", "enemy_id": "MONSTER_001"},
    handler=combat_handler
)
```

### Use Case 3: Make Deep Reasoning Decisions

```python
from ai_service.agents.universal_consciousness import universal_consciousness

# Build deep reasoning chain
chain = await universal_consciousness.build_reasoning_chain(
    entity_id="NPC_001",
    entity_type="npc",
    decision_context=context,
    depth="deep"  # Analyzes past, present, and future
)
```

### Use Case 4: Optimize LLM Costs

```python
from ai_service.agents.decision_optimizer import decision_optimizer

# Automatically uses cheapest tier possible
decision, tier, cached = await decision_optimizer.optimize_decision(
    context=context,
    agent=agent,
    rule_based_handler=my_rules
)

# Check statistics
stats = decision_optimizer.get_statistics()
print(f"LLM call reduction: {stats['llm_call_reduction']:.1f}%")
```

---

## ⚙️ Configuration Presets

### Preset 1: Maximum Cost Savings (Aggressive)

```env
LLM_OPTIMIZATION_MODE=aggressive
REASONING_DEPTH=shallow
DECISION_CACHE_ENABLED=true
DECISION_BATCH_ENABLED=true
DECISION_BATCH_SIZE=10
FUTURE_PLANNING_ENABLED=false
```

**Expected**: 80-90% LLM cost reduction, faster responses, simpler decisions

### Preset 2: Balanced (Default)

```env
LLM_OPTIMIZATION_MODE=balanced
REASONING_DEPTH=medium
DECISION_CACHE_ENABLED=true
DECISION_BATCH_ENABLED=true
DECISION_BATCH_SIZE=5
FUTURE_PLANNING_ENABLED=true
FUTURE_PLANNING_STEPS=2
```

**Expected**: 60-75% LLM cost reduction, good quality decisions

### Preset 3: Maximum Quality

```env
LLM_OPTIMIZATION_MODE=quality
REASONING_DEPTH=deep
DECISION_CACHE_ENABLED=false
DECISION_BATCH_ENABLED=false
FUTURE_PLANNING_ENABLED=true
FUTURE_PLANNING_STEPS=5
REASONING_CHAIN_LOGGING_ENABLED=true
```

**Expected**: 30-50% LLM cost reduction, highest quality decisions

---

## 🔍 Monitoring

### Check System Status

```python
# NPC Relationships
relationship = await npc_relationship_manager.get_relationship("NPC_001", "NPC_002")
print(f"Relationship: {relationship.relationship_value:.1f}")

# Decision Optimizer Stats
stats = decision_optimizer.get_statistics()
print(f"Tier 1 (Rule-based): {stats['tier1_percentage']:.1f}%")
print(f"Tier 2 (Cached): {stats['tier2_percentage']:.1f}%")
print(f"Tier 3 (Batched): {stats['tier3_percentage']:.1f}%")
print(f"Tier 4 (Full LLM): {stats['tier4_percentage']:.1f}%")
print(f"Total LLM call reduction: {stats['llm_call_reduction']:.1f}%")
```

### Database Queries

```sql
-- Check NPC relationships
SELECT * FROM npc_relationships ORDER BY relationship_value DESC LIMIT 10;

-- Check recent interactions
SELECT * FROM npc_interactions ORDER BY timestamp DESC LIMIT 10;

-- Check reasoning chains
SELECT entity_id, reasoning_depth, confidence_score, processing_time_ms
FROM reasoning_chains ORDER BY timestamp DESC LIMIT 10;

-- Check decision cache hit rate
SELECT COUNT(*) as total, SUM(use_count) as total_uses
FROM decision_cache;
```

---

## 🐛 Troubleshooting

### Problem: NPCs not interacting

**Solution**:
```env
# Increase interaction range
NPC_INTERACTION_RANGE=10

# Decrease proximity check interval
NPC_PROXIMITY_CHECK_INTERVAL=15
```

### Problem: High LLM costs

**Solution**:
```env
# Switch to aggressive mode
LLM_OPTIMIZATION_MODE=aggressive

# Enable all optimizations
DECISION_CACHE_ENABLED=true
DECISION_BATCH_ENABLED=true
RULE_BASED_DECISIONS_ENABLED=true

# Reduce reasoning depth
REASONING_DEPTH=shallow
```

### Problem: Poor decision quality

**Solution**:
```env
# Switch to quality mode
LLM_OPTIMIZATION_MODE=quality

# Increase reasoning depth
REASONING_DEPTH=deep

# Enable future planning
FUTURE_PLANNING_ENABLED=true
FUTURE_PLANNING_STEPS=5
```

### Problem: Slow response times

**Solution**:
```env
# Increase instant response workers
INSTANT_RESPONSE_MAX_CONCURRENT=20

# Reduce batch timeout
DECISION_BATCH_TIMEOUT=1.0

# Use shallow reasoning
REASONING_DEPTH=shallow
```

---

## 📊 Performance Benchmarks

### Expected Performance (100 NPCs, Balanced Mode)

| Metric | Value |
|--------|-------|
| LLM Calls/Day | ~3,000 (70% reduction) |
| Monthly Cost | ~$15,552 (70% savings) |
| Tier 1 Decisions | ~30% |
| Tier 2 Decisions | ~40% |
| Tier 3 Decisions | ~20% |
| Tier 4 Decisions | ~10% |
| Avg Response Time | 200-500ms |
| Instant Response Time | <100ms |

---

## 🎯 Best Practices

1. **Start with Balanced Mode** - Good mix of cost and quality
2. **Monitor Statistics** - Track tier distribution and adjust
3. **Enable Caching** - Significant cost savings with minimal quality impact
4. **Use Instant Priority Wisely** - Only for truly time-sensitive events
5. **Log Reasoning Chains** - Helps debug and improve decision quality
6. **Tune Batch Size** - Larger batches = more savings, but higher latency
7. **Review Relationships** - Check NPC relationships periodically for realism

---

## 📚 Additional Resources

- **Full Documentation**: `docs/ADVANCED_AUTONOMOUS_FEATURES.md`
- **Implementation Summary**: `ADVANCED_FEATURES_IMPLEMENTATION_SUMMARY.md`
- **Test Suite**: `test_advanced_autonomous_features.py`
- **Database Schema**: `database/migrations/004_advanced_autonomous_features.sql`

---

## 🆘 Support

For issues or questions:
1. Check the troubleshooting section above
2. Review full documentation in `docs/ADVANCED_AUTONOMOUS_FEATURES.md`
3. Run test suite to verify setup: `python test_advanced_autonomous_features.py`
4. Check logs in `logs/ai-service.log`

