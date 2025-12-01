# Administrator Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-26  
**Target Audience**: Server Administrators, Game Masters, System Operators

---

## Table of Contents

1. [Dashboard Overview](#dashboard-overview)
2. [Dashboard Modules](#dashboard-modules)
3. [Manual Interventions](#manual-interventions)
4. [Configuration Management](#configuration-management)
5. [Player Management](#player-management)
6. [Content Management](#content-management)
7. [Reporting & Analytics](#reporting--analytics)
8. [Best Practices](#best-practices)

---

## Dashboard Overview

### Accessing the Dashboard

**URL**: http://localhost:3000 or https://dashboard.yourdomain.com

**Login Credentials**: Configured during setup (see [Deployment Guide](PRODUCTION_DEPLOYMENT_GUIDE.md))

### Dashboard Layout

```
┌─────────────────────────────────────────────────────────┐
│  Header: Logo | Navigation | User Menu                  │
├─────────────────────────────────────────────────────────┤
│  Sidebar                │  Main Content Area            │
│  ┌──────────────┐       │  ┌─────────────────────────┐ │
│  │ Agent Status │       │  │                         │ │
│  │ World State  │       │  │  Selected Module        │ │
│  │ Economy      │       │  │  Content                │ │
│  │ Player Stats │       │  │                         │ │
│  │ Story Arcs   │       │  │                         │ │
│  │ Performance  │       │  │                         │ │
│  │ System Logs  │       │  │                         │ │
│  │ Admin Panel  │       │  │                         │ │
│  └──────────────┘       │  └─────────────────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Real-Time Updates

Dashboard uses WebSocket (WSS) for real-time data:
- Agent status updates every 5 seconds
- World state updates every 10 seconds
- Performance metrics every 30 seconds
- Log streaming (live)

---

## Dashboard Modules

### Module 1: Agent Status

**Purpose**: Monitor all 21 AI agents in real-time

**Features**:
- Live agent health indicators (🟢 active, 🔴 inactive, 🟡 degraded)
- Last execution time for each agent
- Success/failure counts
- Average execution time
- LLM usage per agent

**Key Metrics**:

| Agent | Expected Status | Execution Freq | Avg Time |
|-------|----------------|----------------|----------|
| **Dialogue** | Active | On-demand | <200ms |
| **Decision** | Active | On-demand | <300ms |
| **Memory** | Active | On-demand | <100ms |
| **World** | Active | Every 1h | <500ms |
| **Quest** | Active | On-demand | <400ms |
| **Economy** | Active | Every 6h | <1s |
| **Problem** | Active | Daily 00:00 | <2s |
| **DynamicNPC** | Active | Daily 06:00 | <3s |
| **WorldEvent** | Active | Every 1h | <1s |
| **Faction** | Active | Every 5m | <500ms |
| **Reputation** | Active | Every 1h | <200ms |
| **DynamicBoss** | Active | Every 15m | <800ms |
| **MapHazard** | Active | Daily 00:30 | <2s |
| **Treasure** | Active | Daily 01:00 | <2s |
| **Weather** | Active | Every 3h | <500ms |
| **Karma** | Active | Every 15m, Daily 03:00 | <300ms |
| **Archaeology** | Active | Daily 02:00 | <2s |
| **AdaptiveDungeon** | Active | Daily 00:15 | <3s |
| **EventChain** | Active | Every 30m, Every 2h | <1s |
| **Social** | Active | Daily 04:00, 07:00 | <2s |
| **EconomySocial** | Active | Every 6h | <1s |

**Actions**:
- Click agent name → View detailed logs
- Toggle agent on/off (requires admin permission)
- Force agent execution (manual trigger)
- View agent configuration

---

### Module 2: World State

**Purpose**: Monitor current world conditions and active content

**Displays**:

**World Problems** (current active):
- Problem description
- Affected maps
- Severity level
- Resolution status
- Player participation

**Dynamic NPCs** (currently spawned):
- NPC name and archetype
- Spawn location (map, coordinates)
- Behavior type
- Interaction count
- Despawn time

**World Events** (active):
- Event name and type
- Affected areas
- Start/end time
- Participation count
- Rewards available

**Map Conditions**:
- Hazardous maps (name, hazard type, danger level)
- Weather effects (map, weather type, intensity)
- Treasure locations (count per map, no exact coords)

**Heatmap Visualization**:
- Click map to view density heatmap
- Shows: Player activity, NPC density, event concentration
- Time filters: Last hour, Last day, Last week

---

### Module 3: Economy

**Purpose**: Monitor economic health and market conditions

**Economy Dashboard**:

```
┌─────────────────────────────────────────────────────┐
│  Total Zeny in Circulation: 1,234,567,890           │
│  Inflation Rate: 2.5% (⬆ 0.3%)                      │
│  Market Health Score: 0.78 (Healthy)                │
└─────────────────────────────────────────────────────┘

Recent Price Changes (Top 10):
┌─────────────┬──────────┬──────────┬─────────┐
│ Item        │ Old Price│ New Price│ Change  │
├─────────────┼──────────┼──────────┼─────────┤
│ White Potion│ 1,200z   │ 1,380z   │ +15%    │
│ Butterfly   │ 300z     │ 270z     │ -10%    │
│ ...         │          │          │         │
└─────────────┴──────────┴──────────┴─────────┘

Shop Inventory Status:
┌──────────────┬──────────┬───────────┬──────────┐
│ Shop         │ Location │ Stock %   │ Last     │
│              │          │           │ Restock  │
├──────────────┼──────────┼───────────┼──────────┤
│ Potion Shop  │ prontera │ 45%       │ 2h ago   │
│ Weapon Shop  │ geffen   │ 78%       │ 30m ago  │
│ ...          │          │           │          │
└──────────────┴──────────┴───────────┴──────────┘
```

**Actions Available**:
- Adjust inflation rate (±0.5%)
- Force shop restock
- Reset economy (emergency only)
- Export economy report (CSV)

**Warning Indicators**:
- 🔴 Inflation >8%: Manual intervention needed
- 🔴 Market health <0.5: Economy imbalanced
- ⚠️ Total circulation <500M: Low liquidity
- ⚠️ Total circulation >5B: Excessive inflation

---

### Module 4: Player Stats

**Purpose**: Monitor player engagement and progression

**Displays**:

**Top Players by Reputation**:
- Player name, character ID
- Total reputation score
- Faction alignment
- Active titles
- Recent achievements

**Player Participation Metrics**:
- World problem participants (today)
- Story arc participants (current)
- Event chain participants (active)
- Dungeon completions (today)
- Archaeology finds (today)

**Reputation Leaderboard**:
- Problem Solver rank
- Faction Champion rank
- Treasure Hunter rank
- Boss Slayer rank
- Archaeologist rank

**Actions Available**:
- View detailed player profile
- Award special titles manually
- Grant reputation bonus
- Reset player progression (with confirmation)

---

### Module 5: Story Arcs

**Purpose**: Monitor and manage ongoing narratives

**Current Story Arc Display**:

```
Arc: "The Awakening of Ancient Powers"
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Chapter 3 of 5: "The Shadow Council"
Status: Active
Duration: Day 12 of 14

Player Participation: 487 players
Top Contributors:
  1. HeroPlayer99 (1,245 points)
  2. QuestMaster (998 points)
  3. AdventureSoul (876 points)

Hero of the Week: HeroPlayer99
Benefits Active: +10% EXP, Special Title, Faction Boost

Recent Choices:
  ✓ 65% chose "Aid the Alliance" → Chapter 3A
  ✗ 35% chose "Support Neutrality" → Chapter 3B
  
Current Narrative:
"The Shadow Council has revealed their presence in Geffen. 
Players must choose whether to expose them or negotiate..."

Next Chapter: Triggers in 2 days or at 500 participants
```

**Actions Available**:
- Force chapter advancement
- Modify participation thresholds
- Award Hero of the Week manually
- Preview next chapter
- Generate emergency replacement arc

---

### Module 6: Performance Metrics

**Purpose**: Monitor system performance and resource usage

**Real-Time Metrics**:

```
┌─────────────────────────────────────────────────┐
│  System Health: ✅ All Systems Operational      │
├─────────────────────────────────────────────────┤
│  API Response Time (p95):     178ms  ✅         │
│  Database Query Time (p95):    12ms  ✅         │
│  Active Connections:           45/100 ✅         │
│  Memory Usage:                 62%    ✅         │
│  CPU Usage:                    34%    ✅         │
│  Disk Usage:                   58%    ✅         │
└─────────────────────────────────────────────────┘

LLM Usage (Today):
  Total Calls:      1,234
  Cache Hits:       789 (64%)
  Tier 1 (Rules):   30%
  Tier 2 (Cache):   40%
  Tier 3 (Batch):   20%
  Tier 4 (Full):    10%
  
  Cost Today:       $38.50
  Cost This Month:  $847.20
  Budget Used:      56%
  
Agent Performance:
  Success Rate:     97.8%
  Failed Tasks:     27
  Avg Execution:    245ms
```

**Graphs**:
- API latency (24h timeline)
- LLM cost trends (30 days)
- Agent success rates (per agent)
- Resource utilization (CPU, memory, disk)

**Actions Available**:
- Export performance report
- Set custom alert thresholds
- Download metrics CSV
- View historical data (up to 90 days)

---

### Module 7: System Logs

**Purpose**: Real-time log monitoring and search

**Features**:
- Live log streaming (WebSocket)
- Log level filtering (DEBUG, INFO, WARNING, ERROR, CRITICAL)
- Search functionality (regex supported)
- Time range selection
- Agent-specific logs
- Export logs (text file)

**Log View Example**:

```
[2025-11-26 10:15:23] INFO  [ProblemAgent] Daily problem generated: "Goblin Invasion"
[2025-11-26 10:15:24] INFO  [EventDispatcher] Dispatched 45 events in batch
[2025-11-26 10:15:28] WARN  [HTTPClient] Retry attempt 1/3 for GET /api/v1/health
[2025-11-26 10:15:30] INFO  [HTTPClient] Connection recovered
[2025-11-26 10:15:45] ERROR [DecisionAgent] LLM call failed: API timeout
```

**Filters**:
- Log Level: ALL, ERROR, WARN, INFO, DEBUG
- Agent: ALL, Dialogue, Decision, Problem, etc.
- Time Range: Last 1h, Last 24h, Last 7d, Custom
- Search: Enter regex pattern

---

### Module 8: Admin Panel

**Purpose**: System administration and manual controls

**Sections**:

#### System Controls
- **Reload Configuration**: Hot reload without restart
- **Restart Agents**: Restart specific agent or all
- **Force Content Generation**: Trigger any scheduled task manually
- **Clear Caches**: Clear decision cache, LLM cache, etc.

#### Emergency Controls
- **Emergency Shutdown**: Stop all AI agents gracefully
- **Circuit Breaker Reset**: Reset HTTP circuit breaker
- **Rollback**: Initiate system rollback (requires confirmation)

#### Maintenance Mode
- **Enable Maintenance**: Disable all agents, show maintenance message
- **Scheduled Maintenance**: Schedule future maintenance window
- **Gradual Rollout**: Enable features for % of players

#### Analytics
- **Generate Report**: Custom date range reports
- **Export Data**: Export system data (JSON, CSV)
- **Cost Analysis**: Detailed LLM cost breakdown

---

## Manual Interventions

### Force Story Arc Generation

**When to Use**: Current arc completed early, arc stuck, or emergency replacement needed

**Procedure**:

1. Navigate to **Story Arcs** module
2. Click **"Generate New Arc"** button
3. Configure parameters:
   ```
   Faction Dominance: Rune Alliance (or current dominant)
   Trigger Reason: Manual Admin Request
   Urgency: Normal / High / Emergency
   Template: Standard / Custom
   ```
4. Click **"Generate Arc"** → Confirm
5. New arc replaces current within 30 seconds
6. Notify players via broadcast

**API Alternative**:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/storyline/generate \
  -H "Content-Type: application/json" \
  -d '{
    "world_state": {
      "faction_dominance": "rune_alliance",
      "recent_events": ["goblin_invasion"]
    },
    "force_new": true,
    "admin_override": true
  }'
```

---

### Manually Trigger Events

**When to Use**: Special occasions, bug fixes, testing

**Available Triggers**:

#### Spawn World Event
```bash
curl -X POST http://192.168.0.100:8000/api/v1/world/events/spawn \
  -H "Content-Type: application/json" \
  -d '{
    "event_type": "boss_invasion",
    "target_map": "prontera",
    "duration_minutes": 30,
    "difficulty": "hard"
  }'
```

#### Force Daily Reset
```bash
# Manually trigger all daily tasks
curl -X POST http://192.168.0.100:8000/api/v1/admin/trigger/daily-reset
```

#### Spawn Dynamic NPC
```bash
curl -X POST http://192.168.0.100:8000/api/v1/world/npcs/spawn \
  -H "Content-Type: application/json" \
  -d '{
    "archetype": "wandering_merchant",
    "spawn_map": "prontera",
    "duration_hours": 24
  }'
```

---

### Adjust Economy Parameters

**When to Use**: Inflation too high/low, market imbalanced

**Via Dashboard**:
1. Navigate to **Economy** module
2. Click **"Adjust Parameters"**
3. Modify settings:
   - Inflation rate: ±0.5% adjustment
   - Price volatility: Low/Medium/High
   - Shop restock frequency
4. Click **"Apply Changes"**
5. Monitor for 24h to see effects

**Via API**:
```bash
# Adjust inflation rate
curl -X POST http://192.168.0.100:8000/api/v1/economy_social/economy/adjust \
  -H "Content-Type: application/json" \
  -d '{
    "inflation_adjustment": -0.5,
    "reason": "High inflation correction"
  }'

# Force shop restock
curl -X POST http://192.168.0.100:8000/api/v1/economy_social/shops/restock-all
```

**⚠️ Warning**: Large adjustments (>1%) can shock the economy. Make gradual changes.

---

### Override Agent Decisions

**When to Use**: Bug in agent logic, special event requirement

**Example: Override Boss Spawn**

```bash
# Cancel automatic boss spawn
curl -X DELETE http://192.168.0.100:8000/api/v1/progression/boss/{boss_id}

# Manually spawn specific boss
curl -X POST http://192.168.0.100:8000/api/v1/progression/boss/spawn \
  -H "Content-Type: application/json" \
  -d '{
    "boss_id": 1039,
    "map": "gef_fild10",
    "duration_minutes": 60,
    "override_schedule": true
  }'
```

**Example: Remove Problematic NPC**

```bash
# Force despawn NPC
curl -X DELETE http://192.168.0.100:8000/api/v1/world/npcs/{npc_id}

# Block NPC archetype temporarily
curl -X POST http://192.168.0.100:8000/api/v1/admin/blocklist/npc \
  -H "Content-Type: application/json" \
  -d '{"archetype": "buggy_merchant", "duration_hours": 24}'
```

---

### Reset Problematic State

**When to Use**: Stuck quest chain, corrupted world state

**Via Dashboard**:
1. Navigate to **Admin Panel**
2. Click **"State Management"**
3. Select reset type:
   - Reset single event chain
   - Reset daily content (problems, NPCs, events)
   - Reset faction alignments
   - Reset reputation (all or single player)
   - Full world state reset (extreme)
4. Confirm reset (requires reason)
5. Notify affected players

**Via API**:
```bash
# Reset event chain
curl -X POST http://192.168.0.100:8000/api/v1/advanced/chains/{chain_id}/reset

# Reset daily content
curl -X POST http://192.168.0.100:8000/api/v1/admin/reset/daily-content

# Reset player reputation
curl -X POST http://192.168.0.100:8000/api/v1/progression/reputation/{player_id}/reset
```

**⚠️ Critical**: Always backup database before major resets!

---

## Configuration Management

### Hot Reload Configurations

**Supported Changes** (no restart required):

- Agent enable/disable flags
- LLM optimization mode
- Reasoning depth
- Cache TTL values
- Batch sizes
- Timeouts

**Procedure**:

```bash
# 1. Edit .env file
nano /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/.env

# 2. Trigger reload
curl -X POST http://192.168.0.100:8000/api/v1/admin/reload

# 3. Verify changes applied
curl http://192.168.0.100:8000/api/v1/admin/config | jq
```

**Requires Restart**:
- Database connection strings
- Redis URL
- API host/port
- Worker count

---

### Adjust Agent Parameters

**Via Dashboard** (Admin Panel → Agent Configuration):

**Problem Agent**:
- Daily problem count (1-5)
- Problem severity distribution
- Escalation thresholds

**Dynamic NPC Agent**:
- Daily spawn count (20-50)
- Archetype distribution
- Behavior variation

**Economy Agent**:
- Update frequency (hourly/daily)
- Inflation target (2-5%)
- Price volatility (low/medium/high)

**Via API**:
```bash
curl -X PUT http://192.168.0.100:8000/api/v1/admin/agents/problem/config \
  -H "Content-Type: application/json" \
  -d '{
    "daily_problem_count": 3,
    "severity_distribution": {"low": 0.6, "medium": 0.3, "high": 0.1}
  }'
```

---

### Modify Reward Scaling

**When to Use**: Progression too fast/slow, economy imbalance

**Adjustable Rewards**:

| System | Default | Range | Impact |
|--------|---------|-------|--------|
| **Quest XP** | 1.0x | 0.5-2.0x | Player leveling speed |
| **Boss Loot** | 1.0x | 0.5-2.0x | Item inflation |
| **Reputation Gain** | 1.0x | 0.5-2.0x | Progression speed |
| **Zeny Drops** | 1.0x | 0.5-1.5x | Economy inflation |
| **Dungeon Tokens** | 1.0x | 0.8-1.5x | Dungeon participation |

**Example**:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/admin/rewards/scaling \
  -H "Content-Type: application/json" \
  -d '{
    "quest_xp_multiplier": 0.8,
    "boss_loot_multiplier": 1.2,
    "reason": "Adjust progression speed"
  }'
```

---

### Update LLM Prompts

**⚠️ Advanced**: Requires understanding of prompt engineering

**Via Dashboard** (Admin Panel → Prompt Management):
1. Select agent to modify
2. View current prompt template
3. Edit prompt (preview available)
4. Test prompt with sample data
5. Apply changes (versioned)
6. Monitor agent performance

**Prompt Version Control**:
- All prompts are versioned
- Rollback to previous version available
- A/B testing supported

---

### Change Schedule Timing

**When to Use**: Adjust for different timezones, server maintenance windows

**Configurable Schedules**:

```bash
# View current schedule
curl http://192.168.0.100:8000/api/v1/world/scheduler/status | jq

# Modify schedule (requires service restart)
# Edit config.py:
# DAILY_RESET_TIME = "00:00"  # Change to "02:00" for UTC+2
# DUNGEON_RESET_TIME = "00:15"
# etc.
```

**⚠️ Note**: Schedule changes require AI service restart

---

## Player Management

### View Player Reputation

**Via Dashboard** (Player Stats → Search Player):
1. Enter player ID or name
2. View reputation profile:
   - Total reputation: 1,234 points
   - Faction alignment: Rune Alliance (800), Neutral (434)
   - Active titles: "Problem Solver", "Boss Slayer"
   - Reputation history (last 30 days)
   - Recent activities

**Via API**:
```bash
curl http://192.168.0.100:8000/api/v1/progression/reputation/{player_id} | jq
```

---

### Award Special Titles

**Via Dashboard**:
1. Navigate to **Player Stats** → **Title Management**
2. Search for player
3. Click **"Award Title"**
4. Select title or create custom:
   - Predefined: Hero of the Week, Legendary Hunter, etc.
   - Custom: Enter title name and benefits
5. Set expiration (permanent or temporary)
6. Confirm award

**Via API**:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/progression/reputation/{player_id}/award-title \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Event Champion",
    "benefits": {
      "stat_bonus": {"str": 5, "agi": 5},
      "discount": 10
    },
    "expires_in_days": 30,
    "reason": "Won community event"
  }'
```

---

### Grant/Revoke Benefits

**Benefit Types**:
- Stat bonuses (temporary or permanent)
- XP/Drop rate bonuses
- Shop discounts
- Faction reputation boosts
- Access to special areas

**Example: Grant Event Winner Benefits**

```bash
curl -X POST http://192.168.0.100:8000/api/v1/admin/players/{player_id}/benefits/grant \
  -H "Content-Type: application/json" \
  -d '{
    "benefit_type": "xp_bonus",
    "value": 20,
    "duration_hours": 168,
    "reason": "Weekly event winner"
  }'
```

**Example: Revoke Abused Benefits**

```bash
curl -X DELETE http://192.168.0.100:8000/api/v1/admin/players/{player_id}/benefits/{benefit_id}
```

---

### Reset Player Progression

**⚠️ Use with Extreme Caution**: This removes player data

**When to Use**: 
- Player account compromised
- Progression bug needs manual fix
- Player requests reset

**Via Dashboard**:
1. Admin Panel → Player Management
2. Search player ID
3. Click **"Reset Progression"**
4. Select what to reset:
   - [ ] Reputation (all factions)
   - [ ] Titles
   - [ ] Benefits
   - [ ] Archaeology progress
   - [ ] Story arc participation
   - [ ] Event chain progress
5. Enter reason (required for audit)
6. Type player ID to confirm
7. Execute reset

**Via API**:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/admin/players/{player_id}/reset \
  -H "Content-Type: application/json" \
  -d '{
    "reset_reputation": true,
    "reset_titles": false,
    "reset_archaeology": false,
    "reason": "Player requested reset"
  }'
```

---

### Handle Player Reports

**Report Types**:
- Inappropriate NPC dialogue
- Unfair content generation
- Economy exploitation
- Bug reports

**Handling Procedure**:
1. Review report details
2. Investigate issue:
   - Check agent logs for mentioned NPC/event
   - Verify player claims
   - Review system state at time of report
3. Take action:
   - Despawn problematic NPC
   - Adjust economy if exploited
   - Fix bug and notify player
   - Escalate to development team if needed
4. Document resolution
5. Follow up with player

---

## Content Management

### Review Auto-Generated Quests

**Via Dashboard** (Content → Quest Review):

**Display**:
- Quest ID and name
- Difficulty level
- Objectives
- Rewards
- Player completion rate
- Average completion time
- Player feedback score

**Actions**:
- Approve quest (make permanent)
- Modify quest parameters
- Disable quest (if problematic)
- Delete quest

**Quality Indicators**:
- ✅ Green: >70% completion, >4.0 rating
- ⚠️ Yellow: 40-70% completion, 3.0-4.0 rating
- 🔴 Red: <40% completion, <3.0 rating

---

### Moderate Story Arcs

**Review Process**:
1. Navigate to **Story Arcs** → **Arc History**
2. Select completed arc
3. Review metrics:
   - Participation count
   - Completion rate
   - Player feedback
   - Hero selections
4. Rate arc quality (1-5 stars)
5. Flag for improvement or mark as exemplary

**Flag problematic content**:
- Inappropriate narrative
- Confusing objectives
- Unbalanced difficulty
- Technical issues

---

### Curate Recurring NPCs

**NPCs That Persist**:
- High player interaction (>100 interactions)
- Positive feedback (>4.0 rating)
- Unique/interesting archetype
- Community favorites

**Curation Process**:
1. Content → NPC Management
2. Filter by: Interaction count, Rating, Archetype
3. Select NPC to curate
4. Options:
   - **Make Permanent**: NPC no longer despawns
   - **Add to Favorites**: Higher spawn priority
   - **Customize**: Modify dialogue, behavior
   - **Create Variant**: Generate similar NPCs
5. Save changes

---

### Manage Artifact Collections

**Via Dashboard** (Content → Archaeology):

**View Collections**:
- Collection name and description
- Required artifacts (count)
- Completion reward details
- Completion count (how many players)
- Average completion time

**Modify Collections**:
```bash
curl -X PUT http://192.168.0.100:8000/api/v1/advanced/archaeology/collections/{collection_id} \
  -H "Content-Type: application/json" \
  -d '{
    "required_artifacts": 12,
    "completion_reward": {
      "title": "Master Archaeologist",
      "stats": {"int": 5, "dex": 5},
      "item_id": 12345
    }
  }'
```

---

### Configure Dungeon Themes

**Via Dashboard** (Content → Dungeons):

**Today's Dungeon**:
- Theme: Fire Day
- Floor count: 7
- Difficulty: Hard
- Estimated completion: 45 minutes
- Current runs: 23 parties
- Completion rate: 65%

**Modify Theme Rotation**:
```bash
curl -X PUT http://192.168.0.100:8000/api/v1/advanced/dungeon/themes \
  -H "Content-Type: application/json" \
  -d '{
    "rotation": ["fire", "ice", "undead", "demon", "earth", "holy", "chaos"],
    "weights": [0.15, 0.15, 0.15, 0.15, 0.15, 0.15, 0.10]
  }'
```

---

## Reporting & Analytics

### Generate Custom Reports

**Via Dashboard** (Analytics → Custom Report):

**Available Reports**:
1. **Player Engagement Report**
   - Active players per day
   - Average session duration
   - Feature usage breakdown
   - Retention metrics

2. **Content Performance Report**
   - Story arc engagement
   - Quest completion rates
   - NPC interaction counts
   - Event participation

3. **Economic Report**
   - Zeny circulation trends
   - Price volatility analysis
   - Shop performance
   - Market health history

4. **Technical Performance Report**
   - API response times
   - Agent success rates
   - Database performance
   - LLM usage patterns

5. **Cost Analysis Report**
   - LLM costs by provider
   - Cost per agent
   - Optimization effectiveness
   - Budget tracking

**Export Formats**: CSV, JSON, PDF

---

### Cost Optimization Recommendations

Dashboard automatically recommends optimizations:

```
💡 Optimization Recommendations:

1. 🟡 Cache hit rate is 45% (target: 60%)
   → Increase DECISION_CACHE_TTL to 7200
   → Estimated savings: $150/month

2. 🟡 25% of decisions are Tier 4 (full LLM)
   → Switch to LLM_OPTIMIZATION_MODE=aggressive
   → Estimated savings: $200/month

3. ✅ Deep reasoning is effective (95% success rate)
   → Keep REASONING_DEPTH=deep
   → No change recommended

4. 🟢 DeepSeek provider is cost-effective
   → Continue using DeepSeek
   → Savings vs OpenAI: $800/month
```

---

## Best Practices

### Daily Administrator Checklist

- [ ] Review agent status (morning)
- [ ] Check error logs for new issues
- [ ] Monitor LLM budget usage
- [ ] Review player feedback/reports
- [ ] Verify daily content generated correctly
- [ ] Check system resource utilization
- [ ] Review story arc progression

### Configuration Change Guidelines

1. **Always backup** before major changes
2. **Test in staging** if available
3. **Apply during low-traffic** hours
4. **Monitor for 30 minutes** after change
5. **Document all changes** in change log
6. **Prepare rollback** procedure

### Emergency Response Protocol

1. **Assess**: Determine severity (P1-P4)
2. **Notify**: Alert appropriate team members
3. **Investigate**: Check logs, metrics, status
4. **Act**: Apply fix or execute rollback
5. **Verify**: Confirm resolution
6. **Document**: Create incident report
7. **Follow-up**: Post-mortem if P1/P2

### Communication Templates

**Maintenance Announcement**:
```
🔧 Scheduled Maintenance Notice

The AI World system will undergo maintenance on [DATE] from [TIME] to [TIME].

Expected Impact:
- Server will be offline for approximately [DURATION]
- All progress will be saved before shutdown
- No data loss expected

Thank you for your patience!
```

**Incident Announcement**:
```
⚠️ Service Disruption Notice

We're currently experiencing [ISSUE DESCRIPTION].

Status: Investigating / Working on fix / Resolved
Impact: [AFFECTED FEATURES]
ETA: [ESTIMATED TIME]

Updates will be posted every 30 minutes.
```

---

## Security Best Practices

### Access Control

- Use role-based access (admin, operator, viewer)
- Rotate passwords monthly
- Use strong passwords (16+ characters)
- Enable 2FA for admin accounts
- Log all admin actions for audit

### Configuration Security

```bash
# Secure file permissions
chmod 600 .env .env.local
chmod 700 conf/

# Restrict admin panel access
# Configure IP whitelist in dashboard config
ADMIN_IP_WHITELIST=["192.168.0.0/24"]
```

### Audit Logging

All admin actions are logged:
```bash
# View admin actions
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT * FROM admin_audit_log
WHERE timestamp > NOW() - INTERVAL '7 days'
ORDER BY timestamp DESC;
EOF
```

---

## Advanced Features

### A/B Testing Content

**Enable A/B Testing**:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/admin/ab-test/create \
  -H "Content-Type: application/json" \
  -d '{
    "test_name": "Story Arc Variants",
    "variant_a": {"template": "standard"},
    "variant_b": {"template": "dramatic"},
    "traffic_split": 0.5,
    "duration_days": 7
  }'
```

### Custom Event Creation

Create fully custom events:
```bash
curl -X POST http://192.168.0.100:8000/api/v1/admin/events/custom \
  -H "Content-Type: application/json" \
  -d '{
    "event_name": "Server Anniversary",
    "event_type": "celebration",
    "duration_hours": 24,
    "rewards": {
      "xp_bonus": 50,
      "drop_rate_bonus": 20
    },
    "spawn_special_npcs": true
  }'
```

---

**Document Version**: 1.0.0  
**Last Reviewed**: 2025-11-26  
**Next Review**: 2026-02-26  
**Maintained By**: Operations Team

**Related Documentation**:
- [Production Deployment Guide](PRODUCTION_DEPLOYMENT_GUIDE.md)
- [Operations Runbook](OPERATIONS_RUNBOOK.md)
- [Player Guide](PLAYER_GUIDE.md)
- [Quick Reference](QUICK_REFERENCE.md)