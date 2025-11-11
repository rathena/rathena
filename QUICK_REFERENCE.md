# Quick Reference - AI NPC System

## 🎮 In-Game Testing

### Enable NPCs
```
# Add to conf/import/npc_custom.conf
npc: npc/custom/ai-world/prontera_ai_npcs.txt

# Reload in-game
@reloadscript
```

### Find NPCs in Prontera
- Lyra (Explorer) - (155, 185)
- Thorne (Guard) - (145, 175)
- Elara (Scholar) - (160, 190)
- Marcus (Merchant) - (140, 185)
- Mira (Healer) - (165, 180)
- Finn (Bard) - (150, 195)
- Grom (Blacksmith) - (135, 190)
- Zara (Seer) - (170, 175)

---

## 🤖 Auto-Spawn NPCs

### Spawn Single NPC
```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/single" \
  -H "Content-Type: application/json" \
  -d '{"map": "prontera", "npc_class": "merchant"}'
```

### Spawn 20 NPCs
```bash
curl -X POST "http://192.168.0.100:8000/ai/npc/spawn/bulk" \
  -H "Content-Type: application/json" \
  -d '{"count": 20, "map": "geffen"}'
```

---

## 🌍 Bootstrap World

### Initialize 100 Seed NPCs
```bash
curl -X POST "http://192.168.0.100:8000/ai/world/bootstrap/start" \
  -H "Content-Type: application/json" \
  -d '{
    "seed_npc_count": 100,
    "maps": ["prontera", "geffen", "payon"],
    "enable_factions": true,
    "enable_economy": true,
    "enable_relationships": true
  }'
```

---

## 📊 Monitor NPCs

### Setup
```bash
cd ai-autonomous-world
source venv/bin/activate
```

### List All NPCs
```bash
python tools/npc_monitor.py --list-npcs
```

### Monitor Specific NPC
```bash
# State
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-state

# Decisions
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-decisions

# Memories
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-memories

# Relationships
python tools/npc_monitor.py --npc-id ai_merchant_005 --show-relationships

# Everything
python tools/npc_monitor.py --npc-id ai_merchant_005 \
  --show-state --show-decisions --show-memories --show-relationships
```

### Monitor World
```bash
# Economy
python tools/npc_monitor.py --world-state economy

# All
python tools/npc_monitor.py --world-state all
```

---

## 🔍 Check Services

```bash
# Service status
screen -ls

# AI Service health
curl http://192.168.0.100:8000/health

# DragonflyDB
nc -z localhost 6379 && echo "✓ OK" || echo "✗ DOWN"

# PostgreSQL
nc -z localhost 5432 && echo "✓ OK" || echo "✗ DOWN"
```

---

## 📖 API Endpoints

- **Spawn Single**: `POST /ai/npc/spawn/single`
- **Spawn Bulk**: `POST /ai/npc/spawn/bulk`
- **Bootstrap**: `POST /ai/world/bootstrap/start`
- **List NPCs**: `GET /ai/npc/list`
- **NPC State**: `GET /ai/npc/{npc_id}/state`
- **NPC Decisions**: `GET /ai/npc/{npc_id}/decisions`
- **World State**: `GET /ai/world/state`
- **API Docs**: http://192.168.0.100:8000/docs

---

## 📚 Full Documentation

- **Testing Guide**: `NPC_TESTING_GUIDE.md`
- **Features Summary**: `NPC_FEATURES_SUMMARY.md`
- **Architecture**: `ai-autonomous-world/docs/ARCHITECTURE.md`
- **World Concept**: `ai-autonomous-world/docs/WORLD_CONCEPT_DESIGN.md`

