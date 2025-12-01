# Economy & Social Agents - Quick Setup Guide

Phase 4D implementation adds 3 agents for economic balance, world morality, and social engagement.

## Quick Start

### 1. Apply Database Migration

```bash
cd rathena-AI-world/ai-autonomous-world/ai-service
psql -U ai_world_user -d ai_world_memory -f migrations/014_economy_social_agents.sql
```

### 2. Configure Environment

Add to `.env`:

```bash
# Merchant Economy
MERCHANT_ECONOMY_ENABLED=true
ECONOMY_ANALYSIS_INTERVAL_HOURS=6
PRICE_ADJUSTMENT_MAX_PERCENT=0.5

# Karma System
KARMA_AGENT_ENABLED=true
KARMA_UPDATE_INTERVAL=900
PROTECTED_MONSTERS=PORING,LUNATIC,DROPS

# Social Interaction
SOCIAL_AGENT_ENABLED=true
DAILY_SOCIAL_EVENT_COUNT=5
GUILD_TASKS_PER_DAY=3
```

### 3. Register Router

Add to [`main.py`](main.py):

```python
from routers.economy_social import router as economy_social_router

app.include_router(economy_social_router)
```

### 4. Start Service

```bash
python main.py
```

The scheduler will automatically:
- Analyze economy every 6 hours
- Update karma every 15 minutes
- Spawn social events daily at 07:00
- Generate guild tasks daily at 04:00

## Agent Overview

### 1. Merchant Economy Agent 💰
**Purpose**: Dynamic pricing and inflation control

**What it does:**
- Monitors zeny circulation and item supply
- Adjusts NPC shop prices (+/- 50%)
- Creates zeny sink events when inflation detected
- Coordinates with other agents for drop rates

**Key Methods:**
```python
# Analyze economy
indicator = await merchant_economy_agent.analyze_economic_health(economy_data)

# Adjust prices
response = await merchant_economy_agent.adjust_merchant_prices(indicator)

# Create zeny sink
event = await merchant_economy_agent.create_zeny_sink_event(severity=0.8)
```

### 2. Karma Agent ⚖️
**Purpose**: Track world morality and apply atmospheric effects

**What it does:**
- Tracks good/evil player actions
- Maintains global karma score (-10000 to +10000)
- Applies world effects based on alignment
- Manages daily karma decay

**Key Methods:**
```python
# Update karma
response = await karma_agent.update_global_karma(player_actions)

# Get current state
state = await karma_agent.get_global_karma_state()

# Apply effects
effects = await karma_agent.apply_karma_effects(alignment)
```

**Karma Effects:**
- VERY_GOOD: Extended daylight, +20% healing, angel NPCs
- VERY_EVIL: Permanent night, demon spawns, chaos

### 3. Social Interaction Agent 🎉
**Purpose**: Create social content for community bonding

**What it does:**
- Spawns daily social events (picnics, dances, photo spots)
- Generates guild tasks
- Tracks social health metrics
- Distributes participation rewards

**Key Methods:**
```python
# Spawn events
events = await social_interaction_agent.spawn_daily_social_events(
    player_distribution, count=5
)

# Generate guild tasks
tasks = await social_interaction_agent.generate_guild_tasks(
    guild_count=10, average_guild_size=12
)

# Handle participation
response = await social_interaction_agent.handle_social_participation(
    event_id, participants, event_type
)
```

## API Endpoints

### Economy
```
POST /api/v1/economy_social/economy/analyze
POST /api/v1/economy_social/economy/adjust-prices
POST /api/v1/economy_social/economy/zeny-sink
GET  /api/v1/economy_social/economy/snapshot
GET  /api/v1/economy_social/economy/history
```

### Karma
```
GET  /api/v1/economy_social/karma/global
POST /api/v1/economy_social/karma/update
GET  /api/v1/economy_social/karma/{player_id}
GET  /api/v1/economy_social/karma/effects
GET  /api/v1/economy_social/karma/history
```

### Social
```
POST /api/v1/economy_social/social/events/spawn
GET  /api/v1/economy_social/social/events/active
POST /api/v1/economy_social/social/events/{event_id}/participate
POST /api/v1/economy_social/social/guild-tasks/generate
GET  /api/v1/economy_social/social/guild-tasks/{guild_id}
POST /api/v1/economy_social/social/guild-tasks/{task_id}/progress
POST /api/v1/economy_social/social/metrics/analyze
```

### Status
```
GET  /api/v1/economy_social/status
GET  /api/v1/economy_social/metrics
```

## Testing

```bash
# Run tests
pytest tests/agents/economy_social/ -v

# Expected: 30+ tests pass, >80% coverage
```

## Monitoring

### Check Agent Status
```bash
curl http://localhost:8000/api/v1/economy_social/status
```

### View Metrics
```bash
curl http://localhost:8000/api/v1/economy_social/metrics
```

### Check Scheduled Tasks
```python
from tasks.procedural_scheduler import get_scheduler_status

status = get_scheduler_status()
print(status)
```

## Integration Points

### With Existing Agents

**Merchant Economy ↔ Problem/Event/Treasure:**
- Recommends drop rate changes
- Coordinates on item supply

**Karma ↔ Faction:**
- Karma affects faction alignment
- Evil karma = Shadow Cult boost

**Social ↔ Reputation:**
- Event participation awards reputation
- "Event Participant" reputation type

## Performance

### Cost Estimates (100 players)
- Merchant Economy: ~$150-200/month
- Karma Agent: ~$100-150/month  
- Social Interaction: ~$150-200/month
- **Total**: ~$400-550/month

### LLM Optimization
- **Tier 1**: 50-70% (rule-based)
- **Tier 2**: 20-30% (cached)
- **Tier 3**: 8-15% (batched)
- **Tier 4**: 2-5% (full LLM)

**Result**: 60-85% reduction in LLM calls

## Troubleshooting

### Database Issues
```bash
# Check tables exist
psql -U ai_world_user -d ai_world_memory \
  -c "\dt *economy*; \dt *karma*; \dt *social*;"

# Verify global karma initialized
psql -U ai_world_user -d ai_world_memory \
  -c "SELECT * FROM global_karma;"
```

### Import Errors
```bash
# Verify agents importable
python -c "from agents.economy_social import merchant_economy_agent, karma_agent, social_interaction_agent; print('OK')"
```

### Scheduler Not Running
```bash
# Check logs
tail -f logs/ai-service.log | grep -i "economy\|karma\|social"
```

## Documentation

- **Complete Guide**: [`docs/ECONOMY_SOCIAL_AGENTS_GUIDE.md`](../docs/ECONOMY_SOCIAL_AGENTS_GUIDE.md)
- **API Reference**: http://localhost:8000/docs
- **Example Integration**: [`npc/custom/ai-world/examples/economy_social_handler.txt`](../../../npc/custom/ai-world/examples/economy_social_handler.txt)

---

**Phase 4D Complete** - Economy & Social agents fully operational!

For detailed documentation, see [`ECONOMY_SOCIAL_AGENTS_GUIDE.md`](../docs/ECONOMY_SOCIAL_AGENTS_GUIDE.md)