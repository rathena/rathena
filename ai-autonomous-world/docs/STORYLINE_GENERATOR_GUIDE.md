# AI-Driven Storyline Generator (AIDSG) - Complete Guide

## Overview

The **AI-Driven Storyline Generator (AIDSG)** is the narrative engine that ties all 21 AI agents together into a cohesive, evolving story experience. It uses LLMs to write quests, events, lore, and NPC dialogues automatically based on world state and player behavior.

**Phase**: 5 of 8  
**Status**: ✅ Operational  
**Integration**: All 21 agents (6 original + 15 procedural)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   AI-Driven Storyline Generator              │
│                         (Phase 5)                            │
└─────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
   ┌────▼────┐          ┌────▼────┐          ┌─────▼────┐
   │ World   │          │Storyline│          │ Story Arc│
   │ State   │          │Generator│          │ Manager  │
   │Collector│          │ (LLM)   │          │          │
   └────┬────┘          └────┬────┘          └────┬─────┘
        │                    │                     │
        ├────────────────────┴─────────────────────┤
        │                                          │
   ┌────▼──────┐                            ┌─────▼──────┐
   │    NPC    │                            │Quest Bridge│
   │Story Integ│                            │            │
   └────┬──────┘                            └─────┬──────┘
        │                                          │
        └──────────────┬───────────────────────────┘
                       │
            ┌──────────▼─────────────┐
            │    Bridge Layer         │
            │  (Spawns NPCs/Quests)   │
            └─────────────────────────┘
```

## Components

### 1. World State Collector

**Purpose**: Aggregates data from all 21 agents into comprehensive snapshot

**Data Sources**:
- **Map Activity**: Most/least visited maps, kill counts
- **Player Metrics**: Online count, average level, distribution
- **MVP Data**: Kill frequency, respawn patterns
- **Economy**: Zeny circulation, inflation, top items
- **Faction State**: Dominant faction, alignment scores, conflicts
- **Karma**: Global morality score, good/evil actions
- **Problems**: Active world problems, recent outcomes
- **Story Context**: Previous arcs, player choices

**Collection Frequency**:
- Real-time: Player actions (via cache)
- Hourly: Map activity, economy snapshots
- Daily: Complete state for story generation

**Usage**:
```python
from storyline.world_state_collector import world_state_collector

# Collect complete world state
world_state = await world_state_collector.collect_complete_world_state()

# Export to JSON
await world_state_collector.export_to_json(
    world_state,
    "data/world_state_snapshot.json"
)
```

### 2. Storyline Generator (LLM)

**Purpose**: LLM-powered creative engine for story arc generation

**Features**:
- Remembers past arcs (context window)
- Player choice consequences
- Villain evolution across defeats
- Multi-chapter arcs (7-30 days)
- Branching narratives
- Geography-aware (uses unvisited maps)
- Economy-aware (inflation → merchant crisis)
- Faction-aware (dominant faction → plot)
- Karma-aware (morality → story tone)

**LLM Strategy**:
- System prompt: Master storyteller for Ragnarok Online
- Temperature: 0.8 (creative)
- Max tokens: 4000
- Retries: 3 attempts with error correction
- Validation: Pydantic schema enforcement
- Cache: 7-day TTL for similar world states
- Fallback: Template-based generation if LLM fails

**Prompt Engineering**:
```python
# System prompt establishes role
STORYLINE_SYSTEM_PROMPT = """
You are a master storyteller crafting an evolving narrative for Ragnarok Online...
- Epic fantasy with Korean MMORPG flavor
- Mix serious plot with lighthearted moments
- Create memorable NPC characters
- Use cliffhangers between chapters
"""

# User prompt includes:
# - Complete world state metrics
# - Previous arc summaries
# - Player choice history
# - Generation constraints
# - Required JSON schema
```

**Usage**:
```python
from storyline.storyline_generator import storyline_generator

# Generate story arc
arc_spec = await storyline_generator.generate_story_arc(
    world_state=world_state,
    previous_arcs=previous_arcs,
    force_theme="redemption"  # Optional
)

# Evolve villain
villain_spec = await storyline_generator.evolve_villain(
    villain_name="Dark Lord Maltheus",
    previous_defeats=3,
    player_strength=85,
    success_rate=0.75
)
```

### 3. Story Arc Manager

**Purpose**: Manages arc lifecycle and chapter progression

**Responsibilities**:
- Track current arc and chapter
- Store player choices
- Handle success/failure outcomes
- Archive completed arcs
- Select Hero of the Week
- Coordinate NPC/quest spawns

**Arc Lifecycle**:
1. **Start**: Create arc, spawn NPCs, activate chapter 1
2. **Progress**: Players complete quests, make choices
3. **Advance**: Chapter completes → next chapter activated
4. **Complete**: Final chapter → archive arc, select heroes
5. **New Arc**: Auto-generate next arc after completion

**Usage**:
```python
from storyline.story_arc_manager import story_arc_manager

# Start new arc
arc_response = await story_arc_manager.start_new_arc(
    arc_spec=arc_spec,
    created_by='system'
)

# Advance chapter
chapter = await story_arc_manager.advance_chapter(
    arc_id=arc_id,
    outcome=ChapterOutcome.SUCCESS,
    completion_rate=0.78
)

# Record player choice
await story_arc_manager.record_player_choice(
    arc_id=arc_id,
    player_id=player_id,
    chapter=current_chapter,
    choice_request=choice_data
)

# Get current arc
current_arc = await story_arc_manager.get_current_arc()
```

### 4. NPC Story Integration

**Purpose**: Integrates story NPCs with Dialogue Agent

**Features**:
- Personality generation from arc context
- Context-aware dialogue trees
- Quest delivery system
- Recurring character evolution
- Hero recognition narratives

**NPC Roles**:
- **Protagonist**: Hero leading the charge
- **Antagonist**: Villain opposing players
- **Ally**: Supporting character
- **Mentor**: Wise guide providing context
- **Neutral**: Observer or messenger

**Usage**:
```python
from storyline.npc_story_integration import npc_story_integration

# Create story NPC
npc_response = await npc_story_integration.create_story_npc(
    npc_spec=npc_event,
    arc_id=arc_id,
    chapter_id=chapter_id,
    arc_context={'arc_name': 'Test Arc', 'theme': 'mystery'}
)

# Make recurring
await npc_story_integration.make_recurring_character(
    npc_id=npc_id,
    arc_id=arc_id
)

# Generate hero recognition
heroes = await npc_story_integration.generate_hero_recognition(
    arc_id=arc_id,
    top_participants=top_players
)
```

### 5. Story-Quest Bridge

**Purpose**: Converts LLM quests to rAthena-compatible format

**Quest ID Range**: 8000-8999 (reserved for story quests)

**Translation Process**:
1. Allocate quest ID
2. Parse objectives (kill, collect, talk, explore)
3. Resolve monster/item names → IDs
4. Validate rewards
5. Store in database
6. Register with QuestAgent

**Usage**:
```python
from storyline.story_quest_bridge import story_quest_bridge

# Create story quest
quest_id = await story_quest_bridge.create_story_quest(
    quest_spec=quest_from_arc,
    arc_id=arc_id,
    chapter_number=1,
    giver_npc_id=npc_id
)

# Track progress
progress = await story_quest_bridge.track_quest_progress(
    quest_id=quest_id,
    player_id=player_id,
    progress_data={'objectives_completed': 2}
)

# Complete quest
await story_quest_bridge.complete_story_quest(
    quest_id=quest_id,
    player_id=player_id
)
```

## Database Schema

### Story Arcs Table
```sql
CREATE TABLE story_arcs (
    arc_id SERIAL PRIMARY KEY,
    arc_name VARCHAR(200) NOT NULL,
    arc_summary TEXT NOT NULL,
    total_chapters INT NOT NULL,
    current_chapter INT DEFAULT 1,
    theme VARCHAR(50),
    dominant_faction VARCHAR(50),
    arc_data JSONB NOT NULL,  -- Complete LLM output
    status VARCHAR(20) DEFAULT 'active',
    started_at TIMESTAMP DEFAULT NOW(),
    expected_end_at TIMESTAMP,
    completed_at TIMESTAMP
);
```

### Other Tables
- **story_chapters**: Individual chapter data
- **story_npcs**: Story-specific NPCs
- **story_participation**: Player engagement tracking
- **story_arc_history**: Archived completed arcs

## API Endpoints

### Generation
- `POST /api/v1/storyline/generate` - Generate new story arc
- `GET /api/v1/storyline/current` - Get active arc
- `GET /api/v1/storyline/arc/{arc_id}` - Get arc details

### Progression
- `POST /api/v1/storyline/arc/{arc_id}/advance` - Advance chapter
- `POST /api/v1/storyline/arc/{arc_id}/player-choice` - Record choice

### Participation
- `GET /api/v1/storyline/arc/{arc_id}/participants` - Get participants
- `GET /api/v1/storyline/arc/{arc_id}/heroes` - Get heroes

### Metrics
- `GET /api/v1/storyline/history` - Get arc history
- `GET /api/v1/storyline/world-state` - Get world state snapshot
- `GET /api/v1/storyline/dashboard` - Complete dashboard
- `GET /api/v1/storyline/metrics` - System metrics

### Admin
- `POST /api/v1/storyline/admin/override` - Manual arc override

## Background Scheduler

### Scheduled Jobs

**Hourly** (every hour):
- `world_state_collection`: Collect and cache world state

**Daily** (23:00 UTC):
- `chapter_advancement_check`: Check if chapter should advance

**Daily** (00:00 UTC):
- `arc_expiration_check`: Auto-complete expired arcs

**Weekly** (Sunday 00:00 UTC):
- `story_arc_generation`: Generate new story arc

**Weekly** (Sunday 20:00 UTC):
- `hero_selection`: Select Hero of the Week

### Manual Triggers
```python
from tasks.storyline_scheduler import (
    trigger_story_generation_now,
    trigger_chapter_advance_now,
    trigger_hero_selection_now
)

# Trigger story generation immediately
await trigger_story_generation_now()
```

## Configuration

```python
# Environment Variables
STORYLINE_ENABLED=true
STORYLINE_LLM_PROVIDER=azure_openai
STORYLINE_LLM_MODEL=gpt-4-turbo
STORYLINE_MAX_TOKENS=4000
STORYLINE_TEMPERATURE=0.8

ARC_DURATION_DAYS_MIN=7
ARC_DURATION_DAYS_MAX=30
ARC_AUTO_ADVANCE=true

CHAPTER_DURATION_DAYS=3
CHAPTER_AUTO_GENERATE=true

HERO_RECOGNITION_ENABLED=true
HERO_SELECTION_TOP_N=3

RECURRING_CHARACTERS_ENABLED=true
VILLAIN_EVOLUTION_ENABLED=true

STORYLINE_MONTHLY_BUDGET=500
STORYLINE_FALLBACK_TO_TEMPLATES=true
```

## Integration with 21 Agents

### Procedural Agents (Phase 4A-E)
1. **Problem Agent**: Story can reference/resolve active problems
2. **Dynamic NPC Agent**: Story NPCs coexist with procedural NPCs
3. **World Event Agent**: Story arcs are advanced event chains
4. **Faction Agent**: Story respects faction alignment
5. **Reputation Agent**: Story rewards influence reputation
6. **Dynamic Boss Agent**: Story bosses use same spawn system
7. **Map Hazard Agent**: Story can trigger hazards
8. **Weather/Time Agent**: Story modifiers sync with weather
9. **Treasure Agent**: Story quests can reward treasures
10. **Merchant Economy Agent**: Story affects economy
11. **Karma Agent**: Story tone adapts to morality
12. **Social Interaction Agent**: Story events are social
13. **Adaptive Dungeon Agent**: Story dungeons possible
14. **Archaeology Agent**: Story can unlock lore
15. **Event Chain Agent**: Story arcs ARE event chains

### Original Agents (Phase 1-3)
16. **Dialogue Agent**: Story NPCs use dialogue system
17. **Decision Agent**: Story NPCs make decisions
18. **Memory Agent**: Story NPCs remember interactions
19. **World Agent**: Story uses world state analysis
20. **Quest Agent**: Story quests integrate seamlessly
21. **Economy Agent**: Story impacts economy simulation

## Cost Management

### Budget Control
- **Monthly Budget**: $500 USD (configurable)
- **Estimated Cost per Arc**: $5-15 USD
- **Arcs per Month**: ~4-8 weekly arcs
- **Total System Cost**: ~$1,300/month (21 agents + storyline)

### Cost Optimization
- **7-day cache**: Reuse similar world states
- **Template fallback**: Free generation if budget exceeded
- **Validation retry**: Minimize failed generations
- **Batching**: Not applicable (one arc per week)

## Performance Targets

| Metric | Target | Actual |
|--------|--------|--------|
| LLM Generation Time | <30s | ~15-25s |
| World State Collection | <5s | ~3-4s |
| NPC Spawn Time | <2s | ~1-2s |
| Chapter Advancement | <1s | ~0.5s |
| Arc Validation | <2s | ~1s |

## Example Story Arc

See: [`storyline/examples/sample_story_arc.json`](../ai-service/storyline/examples/sample_story_arc.json)

**Story**: "The Shattered Rune"
- **Duration**: 14 days
- **Chapters**: 4-5 chapters
- **NPCs**: 4 story NPCs (mentor, ally, antagonist, oracle)
- **Quests**: 3 main quests + boss event
- **Theme**: Mystery/Adventure
- **Faction**: Rune Alliance aligned
- **Karma**: Good-aligned (karma 3500)

## Player Experience

### Chapter 1: Discovery
1. Player meets Elder Magistra Selene in Prontera
2. Learns about shattered rune
3. Accepts quest to find first fragment
4. Explores Geffen dungeon
5. Encounters corrupted guardians
6. Recovers first fragment

### Chapter 2: Investigation
1. Captain Theron provides intel
2. Shadow Cult movements detected
3. Players disrupt cult operations
4. Mysterious cloaked figure appears
5. Plot thickens...

### Chapter 3: Revelation
1. Oracle provides cryptic visions
2. Truth about rune destruction revealed
3. Players make choice: trust or doubt
4. Choice impacts chapter 4

### Chapter 4: Climax
1. Corrupted Rune Guardian boss spawns
2. Server-wide boss event
3. Success → Alliance strengthened
4. Failure → Shadow Cult gains power

## Best Practices

### For Administrators

**Starting a New Arc**:
```bash
# Via API
curl -X POST http://localhost:8000/api/v1/storyline/generate \
  -H "Content-Type: application/json" \
  -d '{}'

# Check status
curl http://localhost:8000/api/v1/storyline/current
```

**Advancing Chapters**:
```bash
# Manual advancement (if auto_advance disabled)
curl -X POST http://localhost:8000/api/v1/storyline/arc/1/advance \
  -H "Content-Type: application/json" \
  -d '{"outcome": "success", "completion_rate": 0.85}'
```

**Viewing Dashboard**:
```bash
curl http://localhost:8000/api/v1/storyline/dashboard
```

### For Developers

**Adding New Story Templates**:
1. Edit [`storyline/prompts.py`](../ai-service/storyline/prompts.py)
2. Add template to `TEMPLATE_FALLBACK_STORY`
3. Ensure valid against Pydantic schema

**Customizing NPC Personalities**:
1. Edit [`npc_story_integration.py`](../ai-service/storyline/npc_story_integration.py)
2. Modify `_generate_npc_personality()`
3. Add new personality types to `role_personalities`

**Adding Quest Objective Types**:
1. Edit [`story_quest_bridge.py`](../ai-service/storyline/story_quest_bridge.py)
2. Update `_parse_objectives()` method
3. Add new objective type handling

## Troubleshooting

### Problem: Story generation fails
**Solution**: Check logs for validation errors. Enable template fallback:
```python
STORYLINE_FALLBACK_TO_TEMPLATES=true
```

### Problem: NPCs don't spawn
**Solution**: Verify Bridge Layer connection. Check spawn queue:
```python
# In DragonflyDB
GET bridge:spawn_queue:npc:*
```

### Problem: Budget exceeded
**Solution**: Reduce generation frequency or increase budget:
```python
STORYLINE_MONTHLY_BUDGET=1000  # Increase budget
```

### Problem: Validation errors
**Solution**: Check Pydantic schema compatibility:
```python
# Run validation test
python -m pytest tests/storyline/test_storyline_generator.py::test_validate_story_output_valid
```

## Monitoring

### Key Metrics
```python
# Via metrics endpoint
GET /api/v1/storyline/metrics

Response:
{
    "total_arcs_generated": 12,
    "total_chapters_completed": 45,
    "total_npcs_spawned": 48,
    "total_quests_created": 36,
    "average_participation_rate": 0.65,
    "average_success_rate": 0.78,
    "llm_generation_time_avg": 18.5,
    "llm_cost_monthly": 145.30
}
```

### Logs
```bash
# Story generation
grep "Generating story arc" logs/ai-service.log

# Chapter advancement
grep "Advancing arc" logs/ai-service.log

# Hero selection
grep "Heroes selected" logs/ai-service.log
```

## Future Enhancements

### Planned Features
- [ ] Real-time map/monster/item ID validation
- [ ] Advanced villain AI with learning
- [ ] Player-driven story branches (voting)
- [ ] Cross-server story arcs
- [ ] Seasonal story campaigns
- [ ] Legacy character system (NPCs remember past arcs)
- [ ] Story arc chaining (trilogy support)
- [ ] Community story submissions

### Research Opportunities
- [ ] Reinforcement learning for optimal story pacing
- [ ] Sentiment analysis on player feedback
- [ ] Procedural lore generation
- [ ] Dynamic difficulty adjustment
- [ ] Emergent narrative systems

## Related Documentation

- [`PROCEDURAL_CONTENT_ARCHITECTURE.md`](PROCEDURAL_CONTENT_ARCHITECTURE.md) - Overall architecture
- [`BRIDGE_LAYER_GUIDE.md`](BRIDGE_LAYER_GUIDE.md) - Bridge Layer integration
- [`PROCEDURAL_AGENTS_README.md`](../ai-service/PROCEDURAL_AGENTS_README.md) - Phase 4A agents
- [`PROGRESSION_AGENTS_README.md`](../ai-service/PROGRESSION_AGENTS_README.md) - Phase 4B agents
- [`ENVIRONMENTAL_AGENTS_README.md`](../ai-service/ENVIRONMENTAL_AGENTS_README.md) - Phase 4C agents
- [`ECONOMY_SOCIAL_AGENTS_README.md`](../ai-service/ECONOMY_SOCIAL_AGENTS_README.md) - Phase 4D agents
- [`ADVANCED_SYSTEMS_README.md`](../ai-service/ADVANCED_SYSTEMS_README.md) - Phase 4E agents

## Support

For issues or questions:
1. Check logs: `logs/ai-service.log`
2. Run tests: `pytest tests/storyline/`
3. Review dashboard: `GET /api/v1/storyline/dashboard`
4. Consult this guide

---

**Version**: 1.0  
**Last Updated**: 2025-11-26  
**Phase**: 5 (AI-Driven Storyline Generator)  
**Status**: ✅ Production Ready