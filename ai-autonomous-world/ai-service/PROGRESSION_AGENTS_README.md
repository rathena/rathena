# Progression Agents - Quick Start

Phase 4B implementation of Faction, Reputation, and Dynamic Boss systems.

## Quick Setup

### 1. Apply Database Migration

```bash
cd rathena-AI-world/ai-autonomous-world/ai-service

# Apply progression tables
psql -U ai_world_user -d ai_world_memory -f migrations/012_progression_agents.sql
```

### 2. Configure Environment

Add to `.env`:

```bash
# Faction System
FACTION_AGENT_ENABLED=true
FACTION_ALIGNMENT_UPDATE_INTERVAL=300
FACTION_DECAY_RATE=0.01
FACTION_SHIFT_THRESHOLD=1000

# Reputation System
REPUTATION_AGENT_ENABLED=true
REPUTATION_DAILY_CAP_PER_TYPE=500

# Dynamic Boss System
DYNAMIC_BOSS_ENABLED=true
BOSS_SPAWN_CHECK_INTERVAL=900
BOSS_ANTI_FARM_THRESHOLD=100
BOSS_RANDOM_SPAWN_CHANCE=0.05
```

### 3. Start Service

```bash
# Start AI service (includes progression agents)
python main.py
```

### 4. Verify Agents Running

```bash
# Check status
curl http://localhost:8000/api/v1/progression/status

# Check faction alignment
curl http://localhost:8000/api/v1/progression/factions/alignment

# Check active bosses
curl http://localhost:8000/api/v1/progression/boss/active
```

## Key Features

### Faction System
- **3 factions:** Rune Alliance, Shadow Cult, Nature Spirits
- **World alignment:** Player actions shift global faction dominance
- **5 reputation tiers** per faction with unique rewards

### Reputation System
- **5 reputation sources:** World Guardian, Explorer, Problem Solver, Event Participant, Faction Loyalist
- **6 influence tiers:** Newcomer → Known → Respected → Renowned → Legendary → Mythic
- **Progressive benefits:** Shops, titles, headgears, stat bonuses, services

### Dynamic Boss System
- **5 boss types:** Anti-Farm, Treasure Guard, Problem Escalation, Faction Champion, Random Encounter
- **Adaptive scaling:** Boss difficulty adjusts to party level and wipe count
- **Rich rewards:** Refine materials, rare costumes, special enchantments

## API Endpoints

**Base URL:** `http://localhost:8000/api/v1/progression`

### Faction Endpoints
- `GET /factions/alignment` - Get world faction state
- `POST /factions/update` - Update faction alignment
- `GET /factions/{player_id}/reputation` - Get player's faction rep
- `POST /factions/{player_id}/action` - Record faction action
- `GET /factions/{player_id}/rewards` - Get available rewards

### Reputation Endpoints
- `GET /reputation/{player_id}` - Get total influence
- `POST /reputation/{player_id}/gain` - Record reputation gain
- `GET /reputation/{player_id}/benefits` - Get unlocked benefits
- `GET /reputation/leaderboard` - Top players by influence

### Boss Endpoints
- `POST /boss/evaluate-spawn` - Check spawn conditions
- `POST /boss/spawn` - Spawn dynamic boss
- `GET /boss/active` - List active bosses
- `POST /boss/{boss_id}/encounter` - Record player participation
- `POST /boss/{boss_id}/defeat` - Process boss defeat
- `GET /boss/history` - Boss spawn history

## Background Tasks

**Auto-scheduled tasks:**

1. **Faction Alignment Update** (every 5 minutes)
   - Processes recent player actions
   - Applies decay
   - Triggers faction effects

2. **Boss Spawn Evaluation** (every 15 minutes)
   - Checks anti-farm conditions
   - Evaluates treasure guards
   - Random encounter roll

3. **Reputation Tier Check** (every 1 hour)
   - Validates tier consistency
   - Auto-corrects mismatches

## Integration with Phase 4A

Progression agents integrate with existing procedural agents:

- **Problem Agent** → Reputation gain (World Guardian)
- **Dynamic NPC Agent** → Reputation gain (Explorer)
- **World Event Agent** → Reputation gain (Event Participant)
- **Faction Agent** ⟷ **All agents** (influences decisions)
- **Boss Agent** ⟷ **Problem Agent** (escalation)

## Testing

```bash
# Run all progression tests
pytest tests/agents/progression/ -v

# Run integration tests
pytest tests/test_progression_integration.py -v

# Check coverage (target: >80%)
pytest tests/agents/progression/ --cov=agents/progression --cov-report=term
```

## Performance

**Optimization targets:**
- API response time: <200ms (p95)
- Database queries: <10ms (p95)
- LLM call reduction: 60-90%
- Cache hit rate: >70%

**Cost targets:**
- Monthly LLM costs: <$400
- Database storage: <500MB/month

## Documentation

- **Complete Guide:** [`docs/PROGRESSION_AGENTS_GUIDE.md`](../docs/PROGRESSION_AGENTS_GUIDE.md)
- **Architecture:** [`docs/PROCEDURAL_CONTENT_ARCHITECTURE.md`](../docs/PROCEDURAL_CONTENT_ARCHITECTURE.md)
- **Phase 4A:** [`PROCEDURAL_AGENTS_README.md`](PROCEDURAL_AGENTS_README.md)

## Troubleshooting

**Common issues:**

1. **"Boss not spawning"**
   - Check `BOSS_SPAWN_CHECK_INTERVAL` not too long
   - Verify monster kill thresholds met
   - Review spawn condition logs

2. **"Reputation not increasing"**
   - Check daily cap not reached
   - Verify reputation type is valid
   - Ensure player_influence record exists

3. **"Faction alignment stuck"**
   - Check decay rate configuration
   - Verify actions being recorded
   - Clear cache: `faction:alignments:current`

## Support

For issues or questions:
- Check logs: `logs/ai-service.log`
- Review metrics: `http://localhost:8000/metrics`
- Run diagnostics: `GET /api/v1/progression/status`