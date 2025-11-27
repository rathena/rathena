# AI-Driven Storyline Generator (AIDSG)

**Phase 5**: Dynamic Narrative Engine  
**Status**: ✅ Operational  
**Version**: 1.0

## Quick Start

### 1. Apply Database Migration
```bash
psql -U ai_world_user -d ai_world_memory -f migrations/016_storyline_system.sql
```

### 2. Configure Environment
```bash
# Add to .env
STORYLINE_ENABLED=true
STORYLINE_LLM_PROVIDER=azure_openai
STORYLINE_LLM_MODEL=gpt-4-turbo
STORYLINE_MONTHLY_BUDGET=500
```

### 3. Start Service
```bash
# Storyline scheduler auto-starts with main service
python main.py
```

### 4. Generate First Story Arc
```bash
# Via API
curl -X POST http://localhost:8000/api/v1/storyline/generate

# Via Python
from tasks.storyline_scheduler import trigger_story_generation_now
await trigger_story_generation_now()
```

## Components Overview

| Component | Purpose | Files |
|-----------|---------|-------|
| **World State Collector** | Aggregates data from 21 agents | `storyline/world_state_collector.py` |
| **Storyline Generator** | LLM-powered narrative engine | `storyline/storyline_generator.py` |
| **Story Arc Manager** | Arc lifecycle management | `storyline/story_arc_manager.py` |
| **NPC Story Integration** | Story NPC creation | `storyline/npc_story_integration.py` |
| **Story-Quest Bridge** | Quest translation | `storyline/story_quest_bridge.py` |
| **Prompts** | LLM prompt templates | `storyline/prompts.py` |
| **Models** | Pydantic validation | `models/storyline.py` |
| **API Router** | FastAPI endpoints | `routers/storyline.py` |
| **Scheduler** | Background jobs | `tasks/storyline_scheduler.py` |

## File Structure

```
ai-service/
├── storyline/
│   ├── __init__.py
│   ├── world_state_collector.py      # Data aggregation
│   ├── storyline_generator.py        # LLM generation
│   ├── story_arc_manager.py          # Arc management
│   ├── npc_story_integration.py      # NPC creation
│   ├── story_quest_bridge.py         # Quest conversion
│   ├── prompts.py                    # LLM prompts
│   └── examples/
│       └── sample_story_arc.json     # Example output
├── models/
│   └── storyline.py                  # Pydantic models
├── routers/
│   └── storyline.py                  # API endpoints
├── tasks/
│   └── storyline_scheduler.py        # Background jobs
├── migrations/
│   └── 016_storyline_system.sql      # Database schema
└── tests/
    └── storyline/
        ├── test_world_state_collector.py
        ├── test_storyline_generator.py
        └── test_story_arc_manager.py
```

## API Endpoints

### Story Arc Management
```bash
# Generate new arc
POST /api/v1/storyline/generate
Body: {"force_theme": "mystery"}

# Get current arc
GET /api/v1/storyline/current

# Get arc details
GET /api/v1/storyline/arc/{arc_id}

# Advance chapter
POST /api/v1/storyline/arc/{arc_id}/advance
Body: {"outcome": "success", "completion_rate": 0.85}
```

### Player Interaction
```bash
# Record player choice
POST /api/v1/storyline/arc/{arc_id}/player-choice
Body: {
  "player_id": 123,
  "choice_type": "dialogue",
  "choice_data": {"selected": "help_alliance"},
  "impact_score": 75
}

# Get player quests
GET /api/v1/storyline/player/{player_id}/quests?arc_id=1
```

### Metrics & History
```bash
# Get arc participants
GET /api/v1/storyline/arc/{arc_id}/participants?limit=100

# Get heroes
GET /api/v1/storyline/arc/{arc_id}/heroes

# Get arc history
GET /api/v1/storyline/history?limit=10

# Get world state
GET /api/v1/storyline/world-state?force_fresh=false

# Get dashboard
GET /api/v1/storyline/dashboard

# Get metrics
GET /api/v1/storyline/metrics
```

## Background Scheduler

### Automatic Jobs

| Job | Schedule | Description |
|-----|----------|-------------|
| World State Collection | Hourly | Collects data from all agents |
| Arc Generation | Sun 00:00 UTC | Generates new story arc weekly |
| Chapter Advancement | Daily 23:00 UTC | Checks if chapter should advance |
| Hero Selection | Sun 20:00 UTC | Selects Hero of the Week |
| Arc Expiration | Daily 00:00 UTC | Auto-completes expired arcs |

### Manual Triggers
```python
# In Python
from tasks.storyline_scheduler import (
    trigger_story_generation_now,
    trigger_chapter_advance_now,
    trigger_hero_selection_now
)

# Trigger immediately
await trigger_story_generation_now()
await trigger_chapter_advance_now()
await trigger_hero_selection_now()
```

## Data Flow

```
┌──────────────┐
│ 21 AI Agents │ ──┐
└──────────────┘   │
┌──────────────┐   │    ┌──────────────────┐
│  PostgreSQL  │ ──┼───▶│ World State      │
└──────────────┘   │    │ Collector        │
┌──────────────┐   │    └────────┬─────────┘
│ DragonflyDB  │ ──┘             │
└──────────────┘                 │
                                 │
                          ┌──────▼─────────┐
                          │  Storyline     │
                          │  Generator     │
                          │  (LLM)         │
                          └──────┬─────────┘
                                 │
                          ┌──────▼─────────┐
                          │  Story Arc     │
                          │  Manager       │
                          └───┬────┬───────┘
                              │    │
                ┌─────────────┘    └─────────────┐
                │                                 │
         ┌──────▼──────┐                  ┌──────▼──────┐
         │     NPC     │                  │    Quest    │
         │Story Integ  │                  │   Bridge    │
         └──────┬──────┘                  └──────┬──────┘
                │                                 │
                └─────────────┬───────────────────┘
                              │
                      ┌───────▼────────┐
                      │  Bridge Layer  │
                      │ (Game Server)  │
                      └────────────────┘
```

## Integration Points

### With Procedural Agents
- **Problem Agent**: Story can reference/resolve active problems
- **Faction Agent**: Story respects faction alignment
- **Karma Agent**: Story tone adapts to world morality
- **Event Chain Agent**: Story arcs ARE advanced event chains
- **All 15 Agents**: Story can trigger any agent's systems

### With Bridge Layer
- Spawn story NPCs dynamically
- Create story-specific quests
- Apply world modifiers
- Broadcast chapter announcements

### With Dialogue Agent
- Story NPCs use DialogueAgent for conversations
- Context includes arc/chapter info
- Dialogue references player participation

## Configuration Options

### Story Generation
```python
STORYLINE_ENABLED=true              # Master switch
STORYLINE_LLM_PROVIDER=azure_openai # LLM provider
STORYLINE_LLM_MODEL=gpt-4-turbo     # Model name
STORYLINE_MAX_TOKENS=4000           # Max generation tokens
STORYLINE_TEMPERATURE=0.8           # Creativity (0.0-1.0)
```

### Arc Parameters
```python
ARC_DURATION_DAYS_MIN=7     # Minimum arc duration
ARC_DURATION_DAYS_MAX=30    # Maximum arc duration
ARC_AUTO_ADVANCE=true       # Auto-advance chapters
```

### Chapter Parameters
```python
CHAPTER_DURATION_DAYS=3        # Days per chapter (0=milestone)
CHAPTER_AUTO_GENERATE=true     # Auto-generate next chapter
```

### Features
```python
HERO_RECOGNITION_ENABLED=true          # Enable heroes
HERO_SELECTION_TOP_N=3                 # Top N heroes
RECURRING_CHARACTERS_ENABLED=true      # Recurring NPCs
VILLAIN_EVOLUTION_ENABLED=true         # Villain progression
```

### Cost Control
```python
STORYLINE_MONTHLY_BUDGET=500              # USD budget
STORYLINE_FALLBACK_TO_TEMPLATES=true      # Template fallback
```

## Testing

### Run All Tests
```bash
pytest tests/storyline/ -v
```

### Test Coverage
```bash
pytest tests/storyline/ --cov=storyline --cov-report=html
```

### Individual Tests
```bash
# World state collector
pytest tests/storyline/test_world_state_collector.py -v

# Storyline generator
pytest tests/storyline/test_storyline_generator.py -v

# Story arc manager
pytest tests/storyline/test_story_arc_manager.py -v
```

## Performance Metrics

### Target Performance
- **LLM Call**: <30s for story generation
- **World State Collection**: <5s for complete state
- **NPC Spawn**: <2s per NPC
- **Chapter Advancement**: <1s for database updates
- **Validation**: <2s for schema checking

### Cost Estimates
- **Per Arc Generation**: $5-15 USD
- **Per Villain Evolution**: $0.50-1.00 USD
- **Per NPC Personality**: $0.20-0.50 USD
- **Per Hero Recognition**: $0.30-0.70 USD
- **Monthly Total**: ~$150-300 USD (within $500 budget)

## Monitoring

### Key Logs
```bash
# Story generation
tail -f logs/ai-service.log | grep "Generating story arc"

# Chapter advancement
tail -f logs/ai-service.log | grep "Advancing arc"

# NPC spawning
tail -f logs/ai-service.log | grep "Story NPC"
```

### Health Checks
```bash
# System status
curl http://localhost:8000/api/v1/storyline/metrics

# Current arc
curl http://localhost:8000/api/v1/storyline/current

# Dashboard
curl http://localhost:8000/api/v1/storyline/dashboard
```

## Troubleshooting

### Issue: Story generation fails
**Symptoms**: No arc created after POST /generate  
**Solutions**:
1. Check LLM provider API keys
2. Verify budget not exceeded
3. Enable template fallback
4. Check logs for validation errors

### Issue: NPCs don't appear in-game
**Symptoms**: NPCs created in database but not visible  
**Solutions**:
1. Verify Bridge Layer running
2. Check spawn queue: `GET bridge:spawn_queue:npc:*`
3. Confirm rAthena server connection
4. Check map names valid

### Issue: Chapters don't advance
**Symptoms**: Stuck on same chapter  
**Solutions**:
1. Check `ARC_AUTO_ADVANCE=true`
2. Verify chapter duration settings
3. Check player completion rates
4. Manually advance via API

### Issue: High LLM costs
**Symptoms**: Budget alerts triggered  
**Solutions**:
1. Enable caching (already enabled)
2. Increase cache TTL
3. Use template fallback
4. Reduce generation frequency

## Migration from Phase 4

### Existing Systems Affected
- ✅ No breaking changes to existing agents
- ✅ New tables added (5 tables)
- ✅ New API endpoints (/api/v1/storyline/*)
- ✅ New background jobs (scheduler)

### Backward Compatibility
- All existing agent APIs unchanged
- Story system is additive only
- Can be disabled via `STORYLINE_ENABLED=false`

## Next Steps (Phase 6)

After storyline system is operational:
1. **Dashboard UI**: Web interface for story management
2. **Admin Tools**: Story arc editor and tester
3. **Analytics**: Player engagement metrics
4. **Visualization**: Story arc timeline viewer

## Support & Resources

- **Complete Guide**: [`docs/STORYLINE_GENERATOR_GUIDE.md`](../docs/STORYLINE_GENERATOR_GUIDE.md)
- **API Reference**: OpenAPI docs at `/docs`
- **Sample Output**: [`storyline/examples/sample_story_arc.json`](storyline/examples/sample_story_arc.json)
- **Tests**: `tests/storyline/`

## Architecture Decisions

### Why LLM for Story Generation?
- **Creativity**: Human-like narrative construction
- **Adaptability**: Responds to dynamic world state
- **Continuity**: References previous arcs naturally
- **Cost-Effective**: $5-15 per arc vs manual writing

### Why Template Fallback?
- **Reliability**: System works even if LLM fails
- **Budget Control**: Prevents cost overruns
- **Consistency**: Templates ensure minimum quality
- **Fallback Chain**: LLM → Error Correction → Template

### Why 7-Day Cache?
- **Cost Reduction**: Reuse similar world states
- **Performance**: Instant response for cached states
- **Freshness**: Weekly rotation ensures variety
- **Balance**: Efficiency vs novelty

### Why Quest ID Range 8000-8999?
- **Separation**: Story quests distinct from procedural
- **Capacity**: 1000 IDs supports many arcs
- **Collision Avoidance**: Dedicated range prevents conflicts
- **Management**: Easy to identify story quests

## Success Criteria Met

✅ **Functional**:
- [x] Collects world state hourly
- [x] Generates story arcs on schedule
- [x] Chapters advance based on outcomes
- [x] NPCs spawn with contextual dialogue
- [x] Player choices influence narrative
- [x] Arcs remember past events

✅ **Performance**:
- [x] LLM call <30s for arc generation
- [x] World state collection <5s
- [x] NPC spawn <2s
- [x] Chapter advancement <1s

✅ **Quality**:
- [x] 100% type coverage (Pydantic)
- [x] LLM output validation
- [x] Schema enforcement
- [x] Error handling for LLM failures
- [x] Fallback to templates

✅ **Cost**:
- [x] LLM costs <$500/month
- [x] Total system <$1,300/month (21 agents + storyline)

---

**Last Updated**: 2025-11-26  
**Maintained By**: AI Engineering Team