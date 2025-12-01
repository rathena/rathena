# Environmental Agents Guide

**Phase 4C: Environmental System Agents**  
**Version:** 1.0.0  
**Date:** 2025-11-26

---

## Overview

Environmental Agents add dynamic map conditions, weather effects, and procedural treasure hunts to Ragnarok Online. These 3 agents create environmental variety and exploration incentives, coordinating with Phase 4A (Procedural) and Phase 4B (Progression) agents.

### Agents Implemented

1. **Map Hazard Agent** - Dynamic map modifiers (Blood Moon, Magic Storm, etc.)
2. **Weather/Time Agent** - Weather patterns and time-of-day effects
3. **Treasure Agent** - Procedural treasure spawns with card fragment system

---

## Architecture

### System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                 Environmental System (Phase 4C)              │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Map Hazard   │  │ Weather/Time │  │   Treasure   │      │
│  │    Agent     │  │    Agent     │  │    Agent     │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
│         │                  │                  │               │
│         └─────────┬────────┴────────┬─────────┘              │
│                   │                  │                        │
│              ┌────▼────┐        ┌───▼────┐                  │
│              │PostgreSQL│        │Dragonfly│                 │
│              │   DB    │        │  Cache  │                 │
│              └─────────┘        └────────┘                  │
└─────────────────────────────────────────────────────────────┘
                            │
                     ┌──────▼───────┐
                     │  Bridge Layer │
                     │ (Future: C++) │
                     └──────────────┘
```

### Data Flow

```
Daily Cycle:
00:00 → Problem Agent generates world problem
00:30 → Map Hazard Agent applies 3-5 hazards (coordinates with problems)
01:00 → Treasure Agent spawns 5-15 treasures (avoids hazard maps)
Every 3h → Weather Agent updates weather patterns
Every 10m → Time effects update (dawn/day/dusk/night)
Hourly → Cleanup expired hazards and treasures
```

---

## Agent 1: Map Hazard Agent

### Purpose

Applies temporary conditions to 3-5 maps daily that modify gameplay mechanics and create strategic variety.

### Hazard Types (10)

| Hazard | Type | Effects | Duration |
|--------|------|---------|----------|
| Blood Moon | Negative | Undead +40% ATK, +30% HP | 24h |
| Magic Storm | Mixed | Cast time -30%, Flee -15% | 24h |
| Thick Fog | Mixed | Hit -15%, Ranged damage +15% | 24h |
| Fairy Blessing | Positive | Drop rate +20%, LUK +10% | 24h |
| Mana Drain | Negative | SP regen 0, skill cost -50% | 24h |
| Scorching Heat | Mixed | Fire +30%, Water -20%, SP -20% | 24h |
| Frozen Wasteland | Mixed | Ice +30%, Fire -20%, ASPD -10% | 24h |
| Toxic Miasma | Negative | HP drain 10/sec, poison +40% | 24h |
| Holy Ground | Positive | Undead damage +50%, HP regen +20% | 24h |
| Chaotic Void | Negative | Random stat variance ±20% | 24h |

### Selection Strategy

```python
Hazard Selection:
1. Identify low-traffic maps (bottom 30%)
2. Match hazard to map theme:
   - Undead maps → Blood Moon, Holy Ground
   - Desert maps → Scorching Heat
   - Ice maps → Frozen Wasteland
   - Forest maps → Fairy Blessing
3. If Problem Agent active in map → choose complementary hazard
4. Balance: 40% positive, 60% negative hazards
5. Ensure geographic distribution (min 3 maps apart)
```

### API Endpoints

```python
# Generate daily hazards
POST /api/v1/environmental/hazards/generate
Request: {
    "map_activity": {"prt_fild01": 5, "moc_fild01": 3},
    "active_problems": [{"problem_id": 1, "map_name": "prt_fild01"}],
    "count": 4
}

# List active hazards
GET /api/v1/environmental/hazards/active

# Get map-specific hazards
GET /api/v1/environmental/hazards/{map_name}

# Record player encounter
POST /api/v1/environmental/hazards/{hazard_id}/encounter
Request: {
    "player_id": 123,
    "time_in_hazard": 300,
    "items_dropped": 5,
    "deaths": 0
}

# Cleanup expired (admin)
DELETE /api/v1/environmental/hazards/cleanup
```

### Database Schema

```sql
-- Active and historical hazards
CREATE TABLE map_hazards (
    hazard_id SERIAL PRIMARY KEY,
    map_name VARCHAR(50) NOT NULL,
    hazard_type VARCHAR(50) NOT NULL,
    hazard_name VARCHAR(100) NOT NULL,
    effect_data JSONB NOT NULL,
    status VARCHAR(20) DEFAULT 'active',
    applied_at TIMESTAMP DEFAULT NOW(),
    expires_at TIMESTAMP NOT NULL
);

-- Player encounters with hazards
CREATE TABLE hazard_encounters (
    id SERIAL PRIMARY KEY,
    hazard_id INT REFERENCES map_hazards(hazard_id),
    player_id INT NOT NULL,
    time_in_hazard INT DEFAULT 0,
    items_dropped INT DEFAULT 0,
    deaths INT DEFAULT 0,
    timestamp TIMESTAMP DEFAULT NOW()
);
```

### Usage Example

```python
from agents.environmental.map_hazard_agent import map_hazard_agent

# Generate daily hazards
map_activity = {"prt_fild01": 10, "pay_fild01": 5, "moc_fild01": 2}
active_problems = [{"problem_id": 1, "map_name": "prt_fild01", "problem_type": "monster_surge"}]

response = await map_hazard_agent.generate_daily_hazards(
    map_activity=map_activity,
    active_problems=active_problems,
    count=4
)

# Response
{
    "hazards_generated": 4,
    "hazards": [
        {
            "hazard_id": 1,
            "map_name": "moc_fild01",
            "hazard_type": "scorching_heat",
            "hazard_name": "Burning Sands",
            "description": "Intense heat amplifies fire while draining stamina",
            "effects": {
                "monster_modifiers": {"element_resist": {"fire": 0.3}},
                "player_modifiers": {"element_damage": {"fire": 1.3}, "sp_regen_mult": 0.8}
            }
        }
    ]
}
```

---

## Agent 2: Weather/Time Agent

### Purpose

Manages dynamic weather and time-of-day effects that modify combat stats and create immersion.

### Weather Types (7)

| Weather | Element Effects | Stat Effects | Visual |
|---------|----------------|--------------|--------|
| Clear | None | Normal | Bright sky |
| Rain | Water +20%, Fire -20% | Flee -5%, Hit -5% | Rain, puddles |
| Storm | Wind +30%, Water +20% | Flee -10%, ASPD -5% | Lightning, thunder |
| Snow | Ice +30%, Fire -30% | Move speed -10%, ASPD -8% | Snow, frost |
| Heat Wave | Fire +30%, Water -20% | SP regen -20%, HP regen -10% | Heat waves |
| Sandstorm | Earth +25%, Wind +15% | Hit -10%, Flee -15% | Sand, dust |
| Aurora | Holy +20%, Shadow +20% | MATK +15%, Magic dmg +15% | Colorful lights |

### Time-of-Day Effects (5)

| Period | Hours | Effects | Description |
|--------|-------|---------|-------------|
| Dawn | 05-07 | EXP +10%, HP regen +10% | Fresh start bonus |
| Day | 07-18 | Normal | Standard gameplay |
| Dusk | 18-20 | Rare drops +15% | Twilight discovery |
| Night | 20-05 | Ghost spawns, Thief drops +20% | Supernatural |
| Full Moon | Random | EXP +15%, all effects amplified | Ancient terrors |

### Weather Transitions

```python
Weather Patterns (Markov Chain):
Clear → Clear (60%), Rain (20%), Heat Wave (10%), Other (10%)
Rain → Rain (50%), Storm (30%), Clear (15%), Snow (5%)
Storm → Storm (40%), Rain (40%), Clear (20%)
Snow → Snow (60%), Clear (25%), Storm (10%), Aurora (5%)

Synergies:
- Rain + Night → Storm chance increased
- Clear + Full Moon → Aurora chance increased
- Heat Wave + Day → Effects amplified 1.5x
- Snow + Night → Blizzard conditions
```

### API Endpoints

```python
# Get current weather
GET /api/v1/environmental/weather/current

# Update weather
POST /api/v1/environmental/weather/update
Request: {"force_weather": "storm"}  # Optional, for events

# Get forecast
GET /api/v1/environmental/weather/forecast?hours=24

# Get time effects
GET /api/v1/environmental/time/current
```

### Usage Example

```python
from agents.environmental.weather_time_agent import weather_time_agent, WeatherType

# Natural weather update
response = await weather_time_agent.update_weather()

# Forced weather (for events)
response = await weather_time_agent.update_weather(force_weather=WeatherType.STORM)

# Get combined effects
effects = await weather_time_agent.apply_weather_time_effects()
# Returns: {"weather": {...}, "time": {...}, "synergies": [...]}
```

---

## Agent 3: Treasure Agent

### Purpose

Spawns procedural treasure hunts in low-traffic maps to encourage exploration.

### Treasure Rarity Distribution

| Rarity | Base Rate | Very Low Traffic | High Traffic | Rewards |
|--------|-----------|------------------|--------------|---------|
| Bronze | 60% | 0% | 60% | 3-5 common items, 1-5k zeny |
| Silver | 30% | 40% | 30% | 2-3 rare items, 10-25k zeny, 1 fragment |
| Gold | 9% | 40% | 9.5% | 1-2 very rare, dye, 50-100k zeny, 5 fragments |
| Mythic | 1% | 20% | 0.5% | Unique item, 200-500k zeny, 15+ fragments |

**Incentive**: Forgotten maps have 40x higher mythic chance!

### Card Fragment System

- **Fragments**: Dropped from Silver+ treasures
- **Exchange Rate**: 100 fragments = 1 random card
- **Accumulation**: Silver (1), Gold (5), Mythic (15+)
- **Estimated Time**: ~50 treasures for 1 card (balanced exploration)

### Treasure Types (3)

1. **Chest** (60%) - Standard treasure chest with visual effect
2. **Dig Spot** (30%) - Hidden ground location, no visual until close
3. **Hidden NPC** (10%) - Rare NPC that gives treasure upon interaction

### Spawn Strategy

```python
Map Traffic Distribution:
- Very Low (0-10%): 40% of treasures, high mythic chance
- Low (10-30%): 30% of treasures, medium-high rarity
- Medium (30-70%): 20% of treasures, balanced rarity
- High (70-100%): 10% of treasures, mostly bronze

Spawn Rules:
- Minimum 20 cells apart
- Avoid NPC spawn conflicts
- Prefer corners, dead ends, hidden areas
- Random despawn: 2-12 hours
```

### API Endpoints

```python
# Spawn daily treasures
POST /api/v1/environmental/treasure/spawn
Request: {
    "map_activity": {"prt_fild01": 10, "pay_fild01": 2},
    "count": 10
}

# List active treasures
GET /api/v1/environmental/treasure/active?include_hints=false

# Discover treasure
POST /api/v1/environmental/treasure/{treasure_id}/discover?player_id=123

# Get player fragments
GET /api/v1/environmental/treasure/{player_id}/fragments

# Claim card from fragments
POST /api/v1/environmental/treasure/{player_id}/claim-card
```

### Usage Example

```python
from agents.environmental.treasure_agent import treasure_agent

# Spawn treasures
map_activity = {"prt_fild01": 15, "pay_fild01": 3, "moc_fild01": 1}
response = await treasure_agent.spawn_daily_treasures(
    map_activity=map_activity,
    count=10
)

# Response
{
    "treasures_spawned": 10,
    "distribution": {
        "very_low_traffic": 4,  # moc_fild01 gets 4 treasures
        "low_traffic": 3,
        "medium_traffic": 2,
        "high_traffic": 1
    },
    "rarity_breakdown": {
        "bronze": 6,
        "silver": 3,
        "gold": 1,
        "mythic": 0
    }
}

# Player discovers treasure
response = await treasure_agent.discover_treasure(
    treasure_id=1,
    player_id=123
)

# Response
{
    "treasure_id": 1,
    "rarity": "gold",
    "rewards": {
        "items": [{"name": "Enriched Elunium", "quantity": 2}],
        "zeny": 75000,
        "reputation_reward": 50,
        "card_fragments": 5
    }
}
```

---

## 4-Tier LLM Optimization

All environmental agents use 4-tier optimization for **60-90% LLM cost reduction**.

### Optimization Breakdown

```
Tier 1: Rule-Based (40% coverage, 0 LLM calls)
├─ Hazard type selection (theme matching)
├─ Weather transitions (Markov chain)
├─ Treasure rarity calculation (traffic-based)
└─ Time-of-day effects (hour-based)

Tier 2: Cached Decisions (30% coverage, 0 LLM calls)
├─ Hazard effects (24h TTL)
├─ Weather effects (24h TTL)
├─ Treasure templates (permanent)
└─ Full moon determination (24h TTL)

Tier 3: Batched Generation (20% coverage, reduced calls)
├─ Multiple hazard generation
├─ Multiple treasure spawns
└─ Batch weather forecasts

Tier 4: Full LLM (10% coverage, 1 call each)
├─ Creative hazard descriptions (optional)
├─ Unique treasure names (optional)
└─ Weather narrative text (optional)
```

**Result**: ~70% reduction = **$300/month → $90/month** for 3 agents

---

## Configuration

### Environment Variables

```bash
# Map Hazard System
MAP_HAZARD_ENABLED=true
DAILY_HAZARD_COUNT=4  # 3-5 recommended
HAZARD_DURATION_HOURS=24
HAZARD_MIN_MAP_DISTANCE=3

# Weather/Time System
WEATHER_TIME_ENABLED=true
WEATHER_UPDATE_INTERVAL_HOURS=3  # 2-6 recommended
WEATHER_CONTINUITY_CHANCE=0.6  # 60% stay same
FULL_MOON_RANDOM_CHANCE=0.1  # 10% per night
TIME_EFFECTS_ENABLED=true

# Treasure System
TREASURE_AGENT_ENABLED=true
DAILY_TREASURE_COUNT=10  # 5-15 recommended
TREASURE_MIN_SPAWN_DISTANCE=20  # cells
TREASURE_DESPAWN_HOURS_MIN=2
TREASURE_DESPAWN_HOURS_MAX=12
TREASURE_HINT_ENABLED=true
CARD_FRAGMENT_EXCHANGE_RATE=100
```

### Scheduled Tasks

```python
# APScheduler jobs
Job: daily_hazards
├─ Trigger: Cron 00:30 daily
├─ Action: Generate 3-5 hazards
└─ Duration: ~2-5 seconds

Job: hazard_cleanup
├─ Trigger: Interval 1 hour
├─ Action: Remove expired hazards
└─ Duration: ~0.5 seconds

Job: weather_update
├─ Trigger: Interval 3 hours
├─ Action: Update weather pattern
└─ Duration: ~1 second

Job: time_effects_update
├─ Trigger: Interval 10 minutes
├─ Action: Update time-of-day
└─ Duration: ~0.1 seconds

Job: daily_treasures
├─ Trigger: Cron 01:00 daily
├─ Action: Spawn 5-15 treasures
└─ Duration: ~3-8 seconds

Job: treasure_cleanup
├─ Trigger: Interval 1 hour
├─ Action: Despawn expired treasures
└─ Duration: ~0.5 seconds
```

---

## Integration with Other Agents

### Phase 4A Integration (Procedural)

```python
# Map Hazard ↔ Problem Agent
When Problem Agent spawns "Monster Surge" in map X:
→ Map Hazard Agent adds complementary hazard (Blood Moon, Chaotic Void)
→ Creates synergy: more monsters + stronger monsters

# Weather ↔ World Event Agent
When World Event triggers "Demonic Invasion":
→ Weather Agent forces Storm weather
→ Creates dramatic atmosphere for event

# Treasure ↔ Dynamic NPC Agent
When Dynamic NPC spawns at (100, 150):
→ Treasure Agent ensures treasure spawns >20 cells away
→ Prevents spawn conflicts and clustering
```

### Phase 4B Integration (Progression)

```python
# Treasure → Reputation Agent
On treasure discovery:
→ Award Explorer reputation (+10 to +100 based on rarity)
→ Bronze: +10, Silver: +25, Gold: +50, Mythic: +100

# All Agents → Faction Agent
Environmental discoveries count toward faction alignment:
→ Rune Alliance: Treasure in sanctuaries
→ Shadow Cult: Treasures during Full Moon
→ Nature Spirits: Treasures in forests during dawn
```

---

## Performance Targets

### API Response Times

```
Endpoint Performance (p95):
- Hazard generation: <150ms
- Weather update: <100ms
- Treasure spawn: <200ms
- Treasure discovery: <50ms
- Get active hazards: <30ms (cached)
- Get current weather: <20ms (cached)
```

### Cache Strategy

```
Cache Keys:
- hazard:effects:{type} → 24h TTL
- hazard:active:{map} → TTL = hazard duration
- weather:current → 5min TTL
- weather:effects:{type} → 24h TTL
- weather:full_moon:active → 10min TTL
- treasure:active:{map} → TTL = treasure duration

Cache Hit Rates:
- Hazard effects: >90% (static data)
- Weather current: >80% (5min TTL)
- Treasure lookups: >70% (varies by discovery rate)

Total Cache Hit Rate: >75%
```

### Database Performance

```sql
-- All queries use indexes
Query Performance (p95):
- INSERT hazard/treasure: <5ms
- SELECT active hazards: <8ms (idx_map_hazards_map_status)
- SELECT current weather: <3ms (idx_weather_state_started)
- UPDATE expired: <10ms (idx_treasure_spawns_despawn)
- SELECT player fragments: <5ms (PRIMARY KEY)

Index Strategy:
- map_hazards: 3 indexes (map+status, expires, applied)
- treasure_spawns: 4 indexes (map+status, rarity, despawn, spawned)
- card_fragments: 1 index (fragment_count for leaderboards)
```

---

## Cost Analysis

### Monthly LLM Costs

```
Map Hazard Agent:
- Daily hazards: 1 generation (rule-based) = $0
- Creative descriptions (optional): 4 calls/day = $2/month
Total: ~$2/month

Weather/Time Agent:
- Weather updates: 8/day (transition-based) = $0
- Weather descriptions (optional): 8 calls/day = $4/month
Total: ~$4/month

Treasure Agent:
- Daily spawns: 10 treasures (template-based) = $0
- Creative names (optional): 10 calls/day = $5/month
Total: ~$5/month

Combined Total: ~$11/month (with optional LLM creativity)
Without optional LLM: ~$0/month (100% rule-based)

Target: <$300/month → Achieved: $11/month (96% under budget)
```

### Database Storage

```
Storage Estimates:
- map_hazards: ~1KB/hazard × 4/day × 30 days = 120KB/month
- hazard_encounters: ~200 bytes × 100/day = 6KB/month
- weather_state: ~500 bytes × 8/day × 90 days = 36KB/month
- weather_history: ~200 bytes × 8/day × 90 days = 14KB/month
- treasure_spawns: ~1KB/treasure × 10/day × 30 days = 300KB/month
- treasure_discoveries: ~500 bytes × 30/day = 15KB/month
- card_fragments: ~100 bytes × 1000 players = 100KB/month

Total: ~591KB/month
Target: <500MB/month → Achieved: <1MB/month (99.8% under budget)
```

---

## Deployment

### 1. Apply Database Migration

```bash
cd rathena-AI-world/ai-autonomous-world/ai-service
psql -U ai_world_user -d ai_world_memory -f migrations/013_environmental_agents.sql
```

### 2. Verify Tables Created

```bash
psql -U ai_world_user -d ai_world_memory -c "
SELECT table_name 
FROM information_schema.tables 
WHERE table_schema = 'public' 
  AND (table_name LIKE '%hazard%' 
    OR table_name LIKE '%weather%' 
    OR table_name LIKE '%treasure%')
ORDER BY table_name;
"
```

Expected output:
```
card_fragments
hazard_encounters
map_hazards
treasure_discoveries
treasure_spawns
weather_history
weather_state
(7 rows)
```

### 3. Configure Environment

```bash
# Add to .env (or use defaults)
echo "MAP_HAZARD_ENABLED=true" >> .env
echo "WEATHER_TIME_ENABLED=true" >> .env
echo "TREASURE_AGENT_ENABLED=true" >> .env
```

### 4. Restart AI Service

```bash
cd rathena-AI-world
./stop-services.sh
./start-ai-service.sh
```

### 5. Verify Deployment

```bash
# Check status
curl http://192.168.0.100:8000/api/v1/environmental/status

# Expected response
{
    "map_hazard_enabled": true,
    "weather_time_enabled": true,
    "treasure_enabled": true,
    "active_hazards": 0,
    "current_weather": "clear",
    "active_treasures": 0,
    "timestamp": "2025-11-26T00:00:00Z"
}
```

---

## Testing

### Run Unit Tests

```bash
cd ai-service

# Test individual agents
pytest tests/agents/environmental/test_map_hazard_agent.py -v
pytest tests/agents/environmental/test_weather_time_agent.py -v
pytest tests/agents/environmental/test_treasure_agent.py -v

# Test integration
pytest tests/test_environmental_integration.py -v

# Test all environmental
pytest tests/agents/environmental/ tests/test_environmental_integration.py -v --cov=agents/environmental
```

### Expected Coverage

```
Test Coverage Targets:
- Map Hazard Agent: >80%
- Weather/Time Agent: >80%
- Treasure Agent: >80%
- Integration: >70%

Total Coverage: >80%
```

---

## Monitoring

### Key Metrics

```python
# Environmental status
GET /api/v1/environmental/status

# Environmental metrics
GET /api/v1/environmental/metrics
Response: {
    "hazards_generated_today": 4,
    "weather_changes_today": 8,
    "treasures_spawned_today": 10,
    "treasures_discovered_today": 7,
    "total_fragments_awarded_today": 25
}
```

### Prometheus Metrics

```
# All environmental agent metrics exposed at /metrics
ai_agent_exec_time_seconds{agent_type="map_hazard"}
ai_agent_success_total{agent_type="map_hazard"}
ai_agent_exec_time_seconds{agent_type="weather_time"}
ai_agent_exec_time_seconds{agent_type="treasure"}
```

---

## Troubleshooting

### No Hazards Spawning

**Symptoms**: Active hazards count = 0 after 00:30  
**Causes**:
- MAP_HAZARD_ENABLED=false
- Scheduler not running
- Database permission error
- No map activity data

**Solutions**:
```bash
# Check config
grep MAP_HAZARD_ENABLED .env

# Check scheduler
curl http://localhost:8000/api/v1/procedural/status

# Check logs
tail -f logs/ai-service.log | grep -i hazard

# Manual trigger
curl -X POST http://localhost:8000/api/v1/environmental/hazards/generate \
  -H "Content-Type: application/json" \
  -d '{"map_activity": {"prt_fild01": 10}, "active_problems": [], "count": 1}'
```

### Weather Not Changing

**Symptoms**: Same weather for >6 hours  
**Causes**:
- WEATHER_TIME_ENABLED=false
- WEATHER_CONTINUITY_CHANCE=1.0 (always same)
- Scheduler interval too long

**Solutions**:
```bash
# Force weather change
curl -X POST "http://localhost:8000/api/v1/environmental/weather/update?force_weather=storm"

# Check current weather
curl http://localhost:8000/api/v1/environmental/weather/current

# Check forecast
curl http://localhost:8000/api/v1/environmental/weather/forecast?hours=24
```

### Treasures Not Discovered

**Symptoms**: All treasures expire undiscovered  
**Causes**:
- Spawn locations not walkable
- No players in low-traffic maps
- TREASURE_HINT_ENABLED=false
- Treasures spawn too far apart

**Solutions**:
```bash
# Check active treasures (admin)
curl "http://localhost:8000/api/v1/environmental/treasure/active?include_hints=true"

# Increase spawn count
sed -i 's/DAILY_TREASURE_COUNT=.*/DAILY_TREASURE_COUNT=15/' .env

# Lower rarity (more bronze = easier to find)
# Adjust map_activity to increase high-traffic treasure spawns
```

---

## Advanced Usage

### Event-Driven Weather

```python
# World Event triggers weather
from agents.procedural.world_event_agent import world_event_agent
from agents.environmental.weather_time_agent import weather_time_agent, WeatherType

# When "Demon Invasion" event starts
event_response = await world_event_agent.trigger_event(...)

if event_response.success:
    # Force storm weather for dramatic effect
    await weather_time_agent.update_weather(force_weather=WeatherType.STORM)
```

### Hazard-Problem Synergy

```python
# Problem Agent spawns "Monster Surge"
problem_response = await problem_agent.generate_daily_problem(world_state)

problem_map = problem_response.data['map_name']

# Map Hazard Agent adds Blood Moon to same map
await map_hazard_agent.apply_hazard(
    map_name=problem_map,
    hazard_type=HazardType.BLOOD_MOON,
    duration_hours=24
)

# Result: More monsters (problem) + stronger monsters (hazard) = challenging content
```

### Treasure Discovery Rewards

```python
# When player discovers mythic treasure
response = await treasure_agent.discover_treasure(treasure_id=1, player_id=123)

if response.success:
    rewards = response.data['rewards']
    
    # Award card fragments (tracked automatically)
    fragments = rewards['card_fragments']  # 15+ for mythic
    
    # Award explorer reputation (via Reputation Agent)
    await reputation_agent.record_reputation_gain(
        player_id=123,
        reputation_type='explorer',
        amount=rewards['reputation_reward'],
        source=f"treasure_{rewards['rarity']}"
    )
    
    # Check if player can claim card
    fragment_data = await treasure_agent.get_card_fragments(123)
    if fragment_data['fragment_count'] >= 100:
        # Eligible for card claim
        await treasure_agent.claim_card_from_fragments(123)
```

---

## Future Enhancements

### Planned Features

1. **Client-Side Visual Effects**
   - Hazard particle effects (blood moon glow, fog, sparkles)
   - Weather animations (rain, snow, lightning)
   - Treasure sparkle/sound effects

2. **Advanced Weather Patterns**
   - Regional weather (different per continent)
   - Weather influenced by player actions
   - Seasonal weather cycles

3. **Treasure Mini-Games**
   - Lockpicking for chests
   - Digging mechanics for dig spots
   - Riddles from hidden NPCs

4. **Dynamic Difficulty**
   - Hazard intensity scales with map level
   - Weather severity varies
   - Treasure rarity adapts to discovery rate

5. **Analytics Dashboard**
   - Hazard effectiveness tracking
   - Weather preference analytics
   - Treasure discovery heatmaps

---

## References

- [Phase 4A: Procedural Agents](../PROCEDURAL_AGENTS_README.md)
- [Phase 4B: Progression Agents](../PROGRESSION_AGENTS_README.md)
- [Quick Setup Guide](../ai-service/ENVIRONMENTAL_AGENTS_README.md)
- [API Documentation](http://192.168.0.100:8000/docs)
- [Database Schema](../migrations/013_environmental_agents.sql)

---

**Last Updated:** 2025-11-26  
**Version:** 1.0.0  
**Status:** Production Ready ✅