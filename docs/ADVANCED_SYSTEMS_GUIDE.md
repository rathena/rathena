# Advanced Systems Guide (Phase 4E)

**Last Updated**: 2025-11-26  
**Status**: ✅ COMPLETE  
**Agent Count**: 15 procedural agents (3 advanced + 12 previous)

## Overview

Phase 4E completes the full procedural content system with **3 advanced agents** that add adaptive dungeons, archaeology exploration, and multi-step event chains to Ragnarok Online.

### Advanced Agents

1. **Adaptive Dungeon Agent** - Daily procedural dungeon instances
2. **Archaeology Agent** - Long-term artifact collection system
3. **Event Chain Agent** - Multi-step branching story events

---

## 1. Adaptive Dungeon Agent

### Purpose
Generate procedurally generated dungeon instances that change daily with random layouts, monsters, and themes.

### Key Features

- **Daily Dungeon Generation** (00:15)
  - Random theme selection (7 element types)
  - 5-10 random floors
  - Theme-based monster pools
  - 1-3 random bosses
  
- **Instance Mechanics**
  - Party-based (2-12 players)
  - 60-minute time limit
  - Progressive difficulty per floor
  - Checkpoint system (every 3 floors)
  - Death penalty (retry from checkpoint)

- **Reward System**
  - Dungeon Tokens (instance currency)
  - Theme-specific drops
  - Reputation gain (Problem Solver)
  - Time bonus multipliers
  - Party size scaling

### Dungeon Themes

| Theme | Element | Monsters | Boss Pool |
|-------|---------|----------|-----------|
| Fire Day | Fire | ELDER_WILLOW, METALLER | IFRIT, KTULLANUX |
| Ice Day | Water | ICE_TITAN, FREEZER | KTULLANUX, DETALE |
| Undead Day | Undead | ZOMBIE, GHOUL, WRAITH | DARK_LORD, DRACULA |
| Demon Day | Dark | BAPHOMET_, INCUBUS | BAPHOMET, BEELZEBUB |
| Earth Day | Earth | GOLEM, STONE_SHOOTER | EDDGA, MAYA |
| Holy Day | Holy | ANGELING, ARCHANGELING | GOLDEN_BUG, OSIRIS |
| Chaotic Day | All | Random from all pools | Random bosses |

### API Endpoints

```python
# Generate daily dungeon
POST /api/v1/advanced/dungeon/generate
{
    "player_average_level": 50,
    "active_factions": ["rune_alliance"]
}

# Get current dungeon
GET /api/v1/advanced/dungeon/current

# Create party instance
POST /api/v1/advanced/dungeon/instance/create
{
    "party_id": 123,
    "participants": [101, 102, 103, 104]
}

# Complete instance
POST /api/v1/advanced/dungeon/instance/{instance_id}/complete
{
    "participants": [101, 102, 103, 104],
    "floors_cleared": 7,
    "time_taken": 2400
}

# Get leaderboard
GET /api/v1/advanced/dungeon/leaderboard?limit=10
```

### Database Schema

**adaptive_dungeons** - Daily dungeon specifications
- `dungeon_id`, `theme`, `floor_count`
- `layout_data` (JSONB) - Complete floor layouts
- `monster_pools` (JSONB) - Monster specs per floor
- `boss_pool` (JSONB) - Boss selections
- `reward_data` (JSONB) - Reward configuration
- `difficulty_rating` (1-10)

**dungeon_instances** - Active party instances
- `instance_id`, `dungeon_id`, `party_id`
- `participants` (JSONB) - Player IDs
- `current_floor`, `floors_cleared`
- `time_elapsed`, `status`

**dungeon_completions** - Completion records
- `instance_id`, `player_id`
- `floors_cleared`, `time_taken`
- `rewards_earned` (JSONB)
- `reputation_gained`

### Optimization Strategy

- **Tier 1 (60%)**: Rule-based layout and monster selection
- **Tier 2 (30%)**: Cached dungeon specs (24-hour TTL)
- **Tier 3 (8%)**: Batched floor generation
- **Tier 4 (2%)**: LLM for creative boss names (optional)

**Target**: <$100/month for dungeon generation

---

## 2. Archaeology Agent

### Purpose
Long-term exploration mechanic where players discover dig spots, collect artifacts, and unlock secret locations.

### Key Features

- **Dig Spot System**
  - 10-20 daily spawns (02:00)
  - Distribution: 70% low-traffic, 20% mid, 10% high
  - 48-hour despawn timer
  - Requires shovel item to dig

- **Artifact Types**
  - **Fossil** (Common) - Ancient creature remains
  - **Relic** (Uncommon) - Magical artifacts
  - **Tome** (Rare) - Lore fragments
  - **Treasure Map** (Rare) - Unlocks secret location
  - **Rare Material** (Mythic) - Crafting components

- **Collection System**
  - Ancient Fossil Set (10 fossils)
  - Lost Kingdom Relics (15 relics)
  - Forbidden Tomes (20 lore pages)
  - Legendary Treasures (5 rare materials)

- **Progression**
  - Archaeology levels 1-10
  - Level bonus to rarity chances
  - Collections unlock titles + stats
  - Museum display system

### Collections & Rewards

| Collection | Required | Reward |
|------------|----------|--------|
| Ancient Fossils | 10 fossils | "Paleontologist" + Max HP +200 + Fossil Crown |
| Lost Kingdom | 15 relics | "Relic Hunter" + MATK +10 + Ancient Tiara |
| Forbidden Tomes | 20 tomes | "Lorekeeper" + INT/DEX +5 + Scholar Cap |
| Legendary Treasures | 5 materials | "Treasure Master" + All Stats +3 + Crown of Ancients |

### Secret Locations

Unlocked by treasure maps (30-minute access):

- **Hidden MVP Lair** - Rare MVP boss
- **Ancient Vault** - High-tier equipment
- **Forgotten Shrine** - Stat blessings
- **Secret Garden** - Rare materials

### API Endpoints

```python
# Spawn dig spots (scheduled)
POST /api/v1/advanced/archaeology/dig-spots/spawn
{
    "map_activity": {"prontera": 100, "geffen": 50},
    "count": 15
}

# List active spots (no exact coords)
GET /api/v1/advanced/archaeology/dig-spots/active

# Dig a spot
POST /api/v1/advanced/archaeology/dig-spots/{spot_id}/dig
{
    "player_id": 101,
    "archaeology_level": 5
}

# Get player collection
GET /api/v1/advanced/archaeology/{player_id}/collection

# Get archaeology progress
GET /api/v1/advanced/archaeology/{player_id}/progress

# Unlock secret location
POST /api/v1/advanced/archaeology/{player_id}/unlock-secret
{
    "treasure_map_id": "map_12345"
}
```

### Database Schema

**dig_spots** - Spawned dig locations
- `spot_id`, `spawn_map`, `spawn_x`, `spawn_y`
- `rarity_tier` (0.0-1.0)
- `dug_by`, `artifact_found`, `status`

**player_artifacts** - Player collections
- `player_id`, `artifact_id`, `artifact_type`
- `collection_name`, `acquired_at`

**artifact_collections** - Collection definitions
- `collection_id`, `collection_name`
- `required_artifacts` (JSONB)
- `completion_reward` (JSONB)
- `completed_by_count`

**archaeology_progress** - Player progression
- `player_id`, `archaeology_level` (1-10)
- `total_digs`, `artifacts_found`
- `collections_completed`, `secret_locations_unlocked`

**secret_locations** - Unlockable locations
- `location_name`, `unlock_requirement`
- `access_map`, `access_duration_minutes`
- `rewards` (JSONB), `unlocked_by_count`

### Optimization Strategy

- **Tier 1 (70%)**: Rule-based artifact generation
- **Tier 2 (20%)**: Cached dig spot locations (48-hour TTL)
- **Tier 3 (8%)**: Batched spot spawning
- **Tier 4 (2%)**: LLM for lore fragments (optional)

**Target**: <$80/month for archaeology system

---

## 3. Event Chain Agent

### Purpose
Create multi-step dynamic story events with branching outcomes based on player success/failure.

### Key Features

- **Chain Structure**
  - 3-7 steps per chain
  - Each step unlocks next
  - Branching based on outcomes
  - Persistent story across days/weeks
  - Maximum 3 concurrent chains

- **Chain Types**
  - **Investigation** - Multi-step mystery (4-6 steps)
  - **Invasion** - Progressive monster attack (3-5 steps)
  - **Rescue** - Time-limited save quest (3-5 steps)
  - **Treasure Hunt** - Multi-stage discovery (4-7 steps)
  - **Faction War** - Escalating conflict (5-7 steps)

- **Branching Mechanics**
  - **Success** → Standard progression, 1.5x rewards, positive world impact
  - **Failure** → Alternate path, 0.5x rewards, negative world impact
  - **Partial** → Modified next step, 1.0x rewards, neutral impact

- **World Impact**
  - Faction alignment shifts
  - Problem resolution/creation
  - NPC spawns (friendly/hostile)
  - Map transformations
  - Reputation bonuses

### Step Types

| Step Type | Description | Success Condition |
|-----------|-------------|-------------------|
| npc_dialogue | Talk to NPC | Talk to NPC |
| item_collection | Gather items | Collect required items |
| location_discovery | Find location | Reach marked location |
| boss_battle | Defeat boss | Defeat boss |
| wave_defense | Survive waves | Survive all waves |
| combat_rescue | Rescue NPCs | Rescue all captives |
| puzzle_solve | Solve puzzle | Complete puzzle |

### Example Chain: Investigation

```
Step 1: NPC Dialogue → Strange Traveler gives hint
Step 2: Location Discovery → Find corrupted map area
Step 3: Boss Battle → Defeat corruption source
Step 4: Item Collection → Gather evidence
Step 5: Resolution → Report findings (outcome affects world)
```

### API Endpoints

```python
# Start event chain
POST /api/v1/advanced/chains/start
{
    "chain_type": "investigation",
    "trigger_condition": {"player_action": "discovery"}
}

# Get active chains
GET /api/v1/advanced/chains/active

# Get chain details
GET /api/v1/advanced/chains/{chain_id}

# Advance chain
POST /api/v1/advanced/chains/{chain_id}/advance
{
    "outcome": "success"
}

# Get current step
GET /api/v1/advanced/chains/{chain_id}/current-step

# Record participation
POST /api/v1/advanced/chains/{chain_id}/participate
{
    "player_id": 101,
    "contribution": 100
}

# Get chain history
GET /api/v1/advanced/chains/history?limit=20
```

### Database Schema

**event_chains** - Chain definitions
- `chain_id`, `chain_type`, `chain_name`
- `total_steps`, `current_step`
- `chain_data` (JSONB) - Full spec
- `trigger_condition` (JSONB)
- `status`, `expires_at`

**chain_steps** - Individual steps
- `step_id`, `chain_id`, `step_number`
- `step_type`, `objective`, `dialogue`
- `success_condition` (JSONB)
- `npc_data`, `spawn_data` (JSONB)
- `completed`, `outcome`

**chain_participation** - Player tracking
- `chain_id`, `player_id`
- `steps_completed`, `contribution_score`
- `rewards_earned` (JSONB)

**chain_outcomes** - Final results
- `chain_id`, `final_outcome`
- `world_changes` (JSONB)
- `participant_count`, `success_rate`

### Optimization Strategy

- **Tier 1 (60%)**: Template-based chains
- **Tier 2 (25%)**: Cached chain specs (7-day TTL)
- **Tier 3 (10%)**: Batched step generation
- **Tier 4 (5%)**: LLM creative narratives (optional)

**Target**: <$70/month for event chains

---

## Background Tasks Schedule

All tasks use APScheduler with cron and interval triggers:

### Daily Tasks (Cron)
- **00:00** - Problem Agent (daily problems)
- **00:15** - Adaptive Dungeon (daily dungeon) ⭐ NEW
- **00:30** - Map Hazard (daily hazards)
- **01:00** - Treasure Agent (daily treasures)
- **02:00** - Archaeology Agent (dig spots) ⭐ NEW
- **03:00** - Cleanup (old data)
- **04:00** - Social Agent (guild tasks)
- **06:00** - Dynamic NPC (daily spawns)
- **07:00** - Social Agent (social events)
- **23:59** - Dynamic NPC (despawn)

### Interval Tasks
- **Every 60s** - World Event (monitoring)
- **Every 5m** - Faction (alignment update)
- **Every 15m** - Boss Agent, Karma (updates)
- **Every 30m** - Event Chain (advancement check) ⭐ NEW
- **Every 1h** - Reputation, Hazard/Treasure cleanup
- **Every 2h** - Event Chain (evaluation) ⭐ NEW
- **Every 3h** - Weather (updates)
- **Every 6h** - Economy (analysis)
- **Every 10m** - Weather (time effects)

**Total Background Jobs**: ~20 scheduled tasks

---

## Integration Points

### Adaptive Dungeon ↔ Other Agents

- **Faction Agent**: Theme influenced by dominant faction
- **Dynamic Boss Agent**: Boss pool selection
- **Reputation Agent**: Completion rewards
- **Problem Agent**: Can spawn as problem solution

### Archaeology ↔ Other Agents

- **Treasure Agent**: Coordinate spawning (avoid overlap)
- **Reputation Agent**: Completion reputation
- **Map Hazard Agent**: Dig spots avoid hazardous maps
- **Event Chain Agent**: Maps can trigger chains

### Event Chain ↔ ALL Agents

- **Problem Agent**: Chains can resolve problems
- **Faction Agent**: Outcomes shift faction alignment
- **NPC Agent**: Chains spawn dynamic NPCs
- **Boss Agent**: Chains can trigger boss fights
- **Karma Agent**: Outcomes affect world karma
- **Weather Agent**: Chains can trigger weather
- **Social Agent**: Chains create community events

---

## Configuration

Add to [`config.py`](../ai-autonomous-world/ai-service/config.py):

```python
# Adaptive Dungeon
ADAPTIVE_DUNGEON_ENABLED: bool = True
DUNGEON_FLOOR_COUNT_MIN: int = 5
DUNGEON_FLOOR_COUNT_MAX: int = 10
DUNGEON_TIME_LIMIT_MINUTES: int = 60
DUNGEON_PARTY_SIZE_MIN: int = 2
DUNGEON_PARTY_SIZE_MAX: int = 12

# Archaeology
ARCHAEOLOGY_ENABLED: bool = True
DAILY_DIG_SPOT_COUNT: int = 15  # 10-20
DIG_SPOT_DESPAWN_HOURS: int = 48
ARCHAEOLOGY_MAX_LEVEL: int = 10
LORE_FRAGMENT_SET_SIZE: int = 20

# Event Chains
EVENT_CHAIN_ENABLED: bool = True
MAX_CONCURRENT_CHAINS: int = 3
CHAIN_STEP_TIMEOUT_HOURS: int = 24
CHAIN_LLM_NARRATIVE: bool = False  # Use LLM for creative chains
CHAIN_MIN_STEPS: int = 3
CHAIN_MAX_STEPS: int = 7
```

---

## Cost Analysis

### Phase 4E Target (3 Agents)

| Agent | LLM Usage | Est. Cost/Month |
|-------|-----------|-----------------|
| Adaptive Dungeon | 2% (boss names only) | $80 |
| Archaeology | 2% (lore fragments) | $70 |
| Event Chain | 5% (creative narratives) | $100 |
| **TOTAL** | **~3%** | **$250** |

### Full System Cost (All 15 Procedural Agents)

| Phase | Agents | Est. Cost/Month |
|-------|--------|-----------------|
| 4A: Core Procedural | 3 | $200 |
| 4B: Progression | 3 | $150 |
| 4C: Environmental | 3 | $100 |
| 4D: Economy/Social | 3 | $100 |
| 4E: Advanced | 3 | $250 |
| **TOTAL** | **15** | **$800** |

**✅ Under $1,000/month budget** (20% safety margin)

---

## Performance Benchmarks

### Target Metrics

- **API Response Time**: <250ms (p95, complex queries allowed)
- **Database Queries**: <15ms (p95, complex JOINs)
- **LLM Call Reduction**: 60-90% through 4-tier optimization
- **Background Tasks**: Non-blocking, complete within interval

### Actual Performance (Expected)

- **Dungeon Generation**: ~200ms (rule-based)
- **Artifact Generation**: ~50ms (rule-based)
- **Chain Start**: ~150ms (template-based)
- **Chain Advance**: ~100ms (template + DB)

---

## Testing

### Unit Tests (35+ tests)

- [`test_adaptive_dungeon_agent.py`](../ai-autonomous-world/ai-service/tests/agents/advanced/test_adaptive_dungeon_agent.py) - 15 tests
- [`test_archaeology_agent.py`](../ai-autonomous-world/ai-service/tests/agents/advanced/test_archaeology_agent.py) - 12 tests
- [`test_event_chain_agent.py`](../ai-autonomous-world/ai-service/tests/agents/advanced/test_event_chain_agent.py) - 11 tests
- [`test_advanced_integration.py`](../ai-autonomous-world/ai-service/tests/agents/advanced/test_advanced_integration.py) - 10+ tests

### Run Tests

```bash
# All advanced systems tests
pytest tests/agents/advanced/ -v

# Specific agent
pytest tests/agents/advanced/test_adaptive_dungeon_agent.py -v

# Integration tests
pytest tests/agents/advanced/test_advanced_integration.py -v

# With coverage
pytest tests/agents/advanced/ --cov=agents/advanced --cov-report=html
```

**Target Coverage**: >80%

---

## Deployment

### Database Migration

Run migration to create 12 new tables:

```bash
# Apply migration
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory \
  -f ai-service/migrations/015_advanced_systems.sql
```

### Start Services

The scheduler automatically starts advanced tasks if enabled in config:

```bash
# Start AI service (includes scheduler)
cd ai-service
python main.py
```

### Verify Deployment

```bash
# Check scheduler status
curl http://192.168.0.100:8000/api/v1/world/scheduler/status

# Generate test dungeon
curl -X POST http://192.168.0.100:8000/api/v1/advanced/dungeon/generate \
  -H "Content-Type: application/json" \
  -d '{"player_average_level": 50, "active_factions": []}'

# Check active chains
curl http://192.168.0.100:8000/api/v1/advanced/chains/active
```

---

## Known Limitations

1. **LLM Narrative (Optional)**: Chain dialogue uses templates by default. Set `CHAIN_LLM_NARRATIVE=true` for creative narratives (+cost).

2. **Dungeon Instances**: Currently tracked in database only. Full integration with rAthena instance system requires C++ bridge extension.

3. **Secret Locations**: Access tracking is basic. Production would need client-side support for custom maps.

4. **Dig Spot Coordinates**: Simplified random spawning. Production should validate against actual map walkable cells.

5. **Chain Advancement**: Currently requires manual API calls. Could be automated with game event hooks.

---

## Next Steps (Phase 5)

With all 15 procedural agents complete, the next phase focuses on:

1. **AI Storyline Generator** - LLM-driven persistent narratives
2. **Dynamic Quest System** - Procedural quest generation
3. **Player Behavior Analysis** - ML-based player modeling
4. **Advanced AI NPCs** - Multi-agent NPC systems
5. **World Evolution Engine** - Long-term world state changes

---

## Support

**Documentation**:
- Main Architecture: [`PROCEDURAL_CONTENT_ARCHITECTURE.md`](PROCEDURAL_CONTENT_ARCHITECTURE.md)
- Agent README: [`ADVANCED_SYSTEMS_README.md`](../ai-autonomous-world/ai-service/ADVANCED_SYSTEMS_README.md)

**Contact**: AI Service Development Team