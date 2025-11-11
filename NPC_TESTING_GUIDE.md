# NPC Testing Guide

Complete guide for testing AI-enabled NPCs in Prontera with auto-spawning and world bootstrap features.

---

## 📋 Table of Contents

1. [Quick Start](#quick-start)
2. [Testing Example NPCs](#testing-example-npcs)
3. [Auto-Spawning System](#auto-spawning-system)
4. [World Bootstrap](#world-bootstrap)
5. [Monitoring NPCs](#monitoring-npcs)
6. [Troubleshooting](#troubleshooting)

---

## 🚀 Quick Start

### Prerequisites

Ensure all services are running:
```bash
# Check service status
screen -ls

# Should show:
# - rathena-login
# - rathena-char
# - rathena-map
# - p2p-coordinator
# - ai-service
```

### Enable NPCs in Game

1. Edit `conf/import/npc_custom.conf` or `npc/scripts_custom.conf`:
```
npc: npc/custom/ai-world/prontera_ai_npcs.txt
```

2. Reload scripts in-game:
```
@reloadscript
```

---

## 🎮 Testing Example NPCs

### 8 Diverse NPCs in Prontera

All NPCs are located in Prontera with unique personalities:

| NPC | Location | Personality | Alignment |
|-----|----------|-------------|-----------|
| **Lyra the Explorer** | (155, 185) | Cheerful, adventurous | Chaotic Good |
| **Guard Thorne** | (145, 175) | Grumpy, suspicious | Lawful Neutral |
| **Scholar Elara** | (160, 190) | Wise, scholarly | Lawful Good |
| **Merchant Marcus** | (140, 185) | Cunning, ambitious | Neutral Evil |
| **Healer Mira** | (165, 180) | Shy, anxious | Neutral Good |
| **Bard Finn** | (150, 195) | Charismatic, entertaining | Chaotic Good |
| **Blacksmith Grom** | (135, 190) | Stoic, focused | Lawful Neutral |
| **Seer Zara** | (170, 175) | Mysterious, cryptic | True Neutral |

### How to Test

1. **Go to Prontera** in-game
2. **Find an NPC** using the coordinates above
3. **Click on the NPC** to interact
4. **Observe AI-generated dialogue** based on their personality

### Expected Behavior

- **Lyra**: Enthusiastic about adventures, shares stories
- **Thorne**: Suspicious, asks about your business
- **Elara**: Offers knowledge, speaks wisely
- **Marcus**: Tries to sell you things, profit-focused
- **Mira**: Nervous but caring, offers healing
- **Finn**: Entertaining, tells jokes and stories
- **Grom**: Brief responses, focused on work
- **Zara**: Cryptic predictions, mysterious

---

## 🤖 Auto-Spawning System

### Single NPC Spawn

Spawn a single NPC with procedural generation:

```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/single" \
  -H "Content-Type: application/json" \
  -d '{
    "map": "prontera",
    "npc_class": "merchant",
    "level": 30
  }'
```

**Parameters:**
- `map` (required): Map to spawn on
- `x`, `y` (optional): Coordinates (random if not specified)
- `npc_class` (optional): NPC class (random if not specified)
- `personality` (optional): Custom personality traits
- `level` (optional): NPC level (random 1-50 if not specified)
- `faction_id` (optional): Faction assignment

**Available NPC Classes:**
- `merchant`, `guard`, `scholar`, `healer`, `explorer`, `blacksmith`, `bard`, `fortune_teller`

### Bulk NPC Spawn

Spawn multiple NPCs at once:

```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "count": 20,
    "map": "prontera",
    "level_range": [10, 40]
  }'
```

**Parameters:**
- `count` (required): Number of NPCs (1-100)
- `map` (required): Map to spawn on
- `npc_classes` (optional): List of classes to use
- `faction_distribution` (optional): Faction distribution
- `level_range` (optional): Level range [min, max]

**Example with Custom Distribution:**
```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/bulk" \
  -H "Content-Type: application/json" \
  -d '{
    "count": 50,
    "map": "geffen",
    "npc_classes": ["scholar", "merchant", "guard"],
    "faction_distribution": {
      "scholars_circle": 0.6,
      "merchant_guild": 0.3,
      "kingdom": 0.1
    },
    "level_range": [20, 60]
  }'
```

---

## 🌍 World Bootstrap

Initialize the entire autonomous world with seed NPCs, factions, and economy.

### Start Bootstrap

```bash
curl -X POST "http://192.168.0.100:8000/ai/world/bootstrap/start" \
  -H "Content-Type: application/json" \
  -d '{
    "seed_npc_count": 100,
    "maps": ["prontera", "geffen", "payon", "morocc", "alberta"],
    "enable_factions": true,
    "enable_economy": true,
    "enable_relationships": true
  }'
```

**Parameters:**
- `seed_npc_count`: Number of NPCs to create (10-500, default: 100)
- `maps`: List of maps to populate
- `enable_factions`: Create initial factions (default: true)
- `enable_economy`: Initialize economy system (default: true)
- `enable_relationships`: Create NPC relationships (default: true)
- `class_distribution`: Custom class distribution (optional)
- `faction_distribution`: Custom faction distribution (optional)

### What Bootstrap Creates

1. **5 Factions:**
   - Kingdom of Rune-Midgarts (Lawful Good)
   - Merchant Guild (Neutral Good)
   - Adventurers Guild (Chaotic Good)
   - Thieves Guild (Chaotic Neutral)
   - Scholars Circle (Lawful Neutral)

2. **Economy System:**
   - Baseline item prices
   - Supply/demand tracking
   - Trade volume monitoring
   - Inflation tracking

3. **100 Seed NPCs:**
   - Distributed across all maps
   - Diverse personalities and classes
   - Faction affiliations
   - Initial goals and motivations

4. **NPC Relationships:**
   - 2-5 relationships per NPC
   - Friends, acquaintances, rivals
   - Affinity scores

### Check Bootstrap Status

```bash
curl "http://192.168.0.100:8000/ai/world/bootstrap/status"
```

---

## 📊 Monitoring NPCs

### Using the NPC Monitor Tool

The `npc_monitor.py` tool provides real-time monitoring of NPC behavior.

#### List All NPCs

```bash
cd ai-autonomous-world
source venv/bin/activate
python tools/npc_monitor.py --list-npcs
```

#### Monitor Specific NPC

```bash
# Show NPC state
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-state

# Show recent decisions
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-decisions

# Show memories
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-memories

# Show relationships
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-relationships

# Show everything
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-state --show-decisions --show-memories --show-relationships
```

#### Monitor World State

```bash
# Show all world state
python tools/npc_monitor.py --world-state all

# Show economy only
python tools/npc_monitor.py --world-state economy

# Show environment
python tools/npc_monitor.py --world-state environment

# Show politics
python tools/npc_monitor.py --world-state politics
```

### Using API Directly

#### Get NPC State
```bash
curl "http://192.168.0.100:8000/ai/npc/ai_merchant_005/state"
```

#### Get NPC Decisions
```bash
curl "http://192.168.0.100:8000/ai/npc/ai_merchant_005/decisions?limit=10"
```

#### Get World State
```bash
curl "http://192.168.0.100:8000/ai/world/state?type=economy"
```

### Monitoring Logs

#### AI Service Logs
```bash
tail -f ai-autonomous-world/logs/ai-service.log
```

#### Map Server Logs
```bash
screen -r rathena-map
# Ctrl+A then D to detach
```

---

## 🔧 Troubleshooting

### NPCs Not Responding

1. **Check AI Service:**
```bash
curl http://192.168.0.100:8000/health
```

2. **Check Logs:**
```bash
tail -f ai-autonomous-world/logs/ai-service.log
```

3. **Verify NPC Registration:**
```bash
python tools/npc_monitor.py --list-npcs
```

### Auto-Spawn Not Working

1. **Check API Endpoint:**
```bash
curl http://192.168.0.100:8000/docs
# Look for /ai/npc/spawn endpoints
```

2. **Test with Simple Spawn:**
```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/single" \
  -H "Content-Type: application/json" \
  -d '{"map": "prontera"}'
```

### Bootstrap Fails

1. **Check Database Connections:**
```bash
# DragonflyDB
nc -z localhost 6379 && echo "✓ DragonflyDB OK" || echo "✗ DragonflyDB DOWN"

# PostgreSQL
nc -z localhost 5432 && echo "✓ PostgreSQL OK" || echo "✗ PostgreSQL DOWN"
```

2. **Check Logs:**
```bash
tail -f ai-autonomous-world/logs/ai-service.log | grep -i bootstrap
```

### Monitor Tool Not Working

1. **Install Dependencies:**
```bash
cd ai-autonomous-world
source venv/bin/activate
pip install rich httpx
```

2. **Check API URL:**
```bash
python tools/npc_monitor.py --api-url http://192.168.0.100:8000 --list-npcs
```

---

## 📚 Additional Resources

- **Architecture**: `ai-autonomous-world/docs/ARCHITECTURE.md`
- **World Concept**: `ai-autonomous-world/docs/WORLD_CONCEPT_DESIGN.md`
- **API Documentation**: http://192.168.0.100:8000/docs
- **NPC Scripts**: `npc/custom/ai-world/README.md`

---

## 🎯 Next Steps

1. **Test Example NPCs** - Interact with the 8 NPCs in Prontera
2. **Spawn More NPCs** - Use auto-spawning to create diverse populations
3. **Bootstrap World** - Initialize the full autonomous world
4. **Monitor Behavior** - Use monitoring tools to observe emergent behavior
5. **Observe Relationships** - Watch NPCs develop relationships over time
6. **Track Economy** - Monitor how NPCs affect the economy

**Have fun exploring the AI autonomous world!** 🚀

