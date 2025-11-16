# Quick Start Guide - AI-rAthena Integration

## ✅ Current Status: ALL SYSTEMS OPERATIONAL

All services are **already running** and fully functional!

### Service Status Check
```bash
# Check AI Service
curl http://localhost:8000/health
# Expected: {"status":"healthy","service":"ai-service","version":"1.0.0"}

# Check rAthena servers
ps aux | grep -E "(map-server|char-server|login-server)" | grep -v grep
# Expected: 3 processes running

# Check databases
psql -U ai_world_user -d ai_world_memory -c "\dt"  # PostgreSQL (18 tables)
redis-cli -p 6379 ping  # DragonflyDB (PONG)
```

---

## Prerequisites Check

✅ **Already Installed and Running:**
- PostgreSQL 17 (active)
- DragonflyDB 1.12.1 (running on port 6379)
- rAthena built successfully (all 4 servers)
- LLM provider credentials configured in `.env` (Azure OpenAI, OpenAI, Anthropic, Google, or DeepSeek)

---

## 3-Step Quick Start

### Step 1: Initialize Database (One-Time Setup)

```bash
cd /ai-mmorpg-world/rathena-AI-world

# Run the automated setup script
./setup-database.sh

# OR manually create database:
sudo -u postgres psql << 'EOF'
CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';
CREATE DATABASE ai_world_memory OWNER ai_world_user;
GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;
\q
EOF

# Initialize schema
PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory \
  -f ai-autonomous-world/database/init.sql
```

**Verify database:**
```bash
PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory -c "\dt"
```

You should see 7 tables: npc_memories, npc_relationships, npc_personalities, factions, faction_reputations, generated_quests, ai_performance_metrics

---

### Step 2: Start AI Service

```bash
cd /ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service

# Install dependencies (first time only)
pip install -r requirements.txt

# Start the AI service
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

**Expected output:**
```
INFO:     Started server process
INFO:     Waiting for application startup.
INFO:     Application startup complete.
INFO:     Uvicorn running on http://0.0.0.0:8000
```

**Test the service:**
```bash
# In a new terminal
curl http://localhost:8000/health
# Should return: {"status":"healthy","timestamp":"..."}
```

---

### Step 3: Start rAthena Servers

```bash
cd /ai-mmorpg-world/rathena-AI-world

# Option A: Use the quick start script (recommended for testing)
./start-services.sh

# Option B: Start manually
./login-server &
./char-server &
./map-server &
```

**Verify servers are running:**
```bash
ps aux | grep -E "(login|char|map)-server" | grep -v grep
```

---

## Testing the Integration

### 🎯 Comprehensive E2E Test Suite (Recommended)
```bash
cd /ai-mmorpg-world/rathena-AI-world
python3 tests/comprehensive_e2e_test.py
```

**Expected Output:**
```
================================================================================
AI-rAthena Integration - Comprehensive E2E Test Suite
================================================================================

[PASS] AI Service Health: Service is healthy
[PASS] NPC Registration: Agent ID: agent_e2e_test_npc_001_xxxxx
[PASS] NPC Dialogue Event: Event ID: event_xxxxx
[PASS] NPC State Retrieval: State retrieved for e2e_test_npc_001
[PASS] Database Connectivity: Both databases connected

Total: 5 | Passed: 5 | Failed: 0 | Success Rate: 100.0%
================================================================================
```

### Unit Tests
```bash
cd ai-autonomous-world/ai-service
python3 -m pytest tests/test_integration.py -v
# Expected: 16 passed, 348 warnings in ~5s
```

---

## Manual API Testing

### 1. Health Check
```bash
curl http://localhost:8000/health
```

### 2. Register an NPC
```bash
curl -X POST http://localhost:8000/ai/npc/register \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_merchant_001",
    "name": "Merchant Bob",
    "npc_class": "merchant",
    "level": 50,
    "position": {"map": "prontera", "x": 150, "y": 150},
    "personality": {
      "openness": 0.7,
      "conscientiousness": 0.8,
      "extraversion": 0.6,
      "agreeableness": 0.9,
      "neuroticism": 0.3,
      "moral_alignment": "good",
      "quirks": ["friendly", "talkative"]
    },
    "faction_id": "merchants_guild",
    "initial_goals": ["sell items", "help players"]
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "agent_id": "agent_test_merchant_001_xxxxx",
  "npc_id": "test_merchant_001",
  "message": "NPC Merchant Bob registered successfully",
  "timestamp": "2025-11-08T..."
}
```

### 3. Send Dialogue Event
```bash
curl -X POST http://localhost:8000/ai/npc/event \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_merchant_001",
    "event_type": "social",
    "event_data": {
      "player_name": "TestPlayer",
      "player_message": "Hello! What do you sell?",
      "interaction_type": "dialogue"
    },
    "context": {
      "time_of_day": "afternoon",
      "weather": "sunny"
    }
  }'
```

**Expected Response:**
```json
{
  "status": "queued",
  "event_id": "event_xxxxx",
  "npc_id": "test_merchant_001",
  "message": "Event queued for processing",
  "estimated_processing_time": 5.0
}
```

### 4. Get NPC State
```bash
curl http://localhost:8000/ai/npc/test_merchant_001/state
```

**Expected Response:**
```json
{
  "npc_id": "test_merchant_001",
  "state": {
    "npc_id": "test_merchant_001",
    "name": "Merchant Bob",
    "class": "merchant",
    "level": "50",
    "map": "prontera",
    "x": "150",
    "y": "150",
    "agent_id": "agent_test_merchant_001_xxxxx",
    "status": "active"
  },
  "source": "cache",
  "timestamp": "2025-11-08T..."
}
```

---

## Testing NPC Scripts with HTTP Commands

Create a test NPC script in `npc/custom/test_ai_npc.txt`:

```c
prontera,150,150,4	script	AI Test NPC	4_M_ALCHE_A,{
	mes "[AI Test NPC]";
	mes "Hello! I'm powered by AI.";
	mes "Ask me anything!";
	next;
	
	// Prepare for AI dialogue
	ai_chat_start("test_ai_npc");
	input .@player_input$;
	
	// Build JSON request
	.@json$ = "{\"player_id\":\"" + getcharid(0) + "\",\"player_name\":\"" + strcharinfo(0) + "\",\"message\":\"" + .@player_input$ + "\"}";
	
	// Make HTTP POST request to AI service
	.@response$ = httppost("http://localhost:8000/ai/npc/test_ai_npc/interact", .@json$);
	
	// Parse and display response
	mes "[AI Test NPC]";
	mes .@response$;
	close;
}
```

**Reload scripts:**
```bash
# In rAthena console
@reloadscript
```

---

---

## API Documentation

Interactive API documentation:
- **Swagger UI:** http://localhost:8000/docs
- **ReDoc:** http://localhost:8000/redoc

---

## Service Management

### View Logs
```bash
# AI Service logs
tail -f ai-autonomous-world/ai-service/logs/ai-service.log

# rAthena map server logs
tail -f log/map-server.log
tail -f log/char-server.log
tail -f log/login-server.log
```

### Restart AI Service (if needed)
```bash
pkill -f "uvicorn main:app"
cd ai-autonomous-world/ai-service
uvicorn main:app --host 0.0.0.0 --port 8000 &
```

### Restart rAthena Servers (if needed)
```bash
./stop-services.sh
./start-services.sh
```

### Database Queries
```bash
# Check NPC memories
PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory \
  -c "SELECT * FROM npc_memories LIMIT 5;"

# Check performance metrics
PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory \
  -c "SELECT * FROM ai_performance_metrics ORDER BY timestamp DESC LIMIT 10;"

# Monitor database connections
psql -U ai_world_user -d ai_world_memory -c "SELECT count(*) FROM pg_stat_activity;"

# Check DragonflyDB stats
redis-cli -p 6379 INFO stats
```

---

## Troubleshooting

### Issue: AI Service won't start
**Solution:** Check if port 8000 is already in use
```bash
lsof -i :8000
# Kill the process if needed
```

### Issue: Database connection failed
**Solution:** Verify PostgreSQL is running and credentials are correct
```bash
systemctl status postgresql
PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory -c "SELECT 1;"
```

### Issue: DragonflyDB connection failed
**Solution:** Verify DragonflyDB is running
```bash
redis-cli ping
# Should return: PONG (DragonflyDB is Redis-compatible)
```

### Issue: HTTP script commands not working
**Solution:** Verify map-server was rebuilt with custom code
```bash
./map-server --version
# Check if binary is recent (Nov 8 13:33)
ls -lh map-server
```

---

## Performance Tuning

Edit `ai-autonomous-world/ai-service/.env`:

```bash
# Increase worker threads for higher load
MAX_WORKERS=8

# Adjust LLM timeout
LLM_TIMEOUT=60

# Enable caching
LLM_CACHE_ENABLED=true
LLM_CACHE_TTL=3600
```

---

## Advanced Features

### Enable Advanced Autonomous Features

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

### Run Advanced Features Tests

```bash
cd rathena-AI-world
python test_advanced_autonomous_features.py
```

For more details, see `docs/ADVANCED_AUTONOMOUS_FEATURES.md`

---

## Next Steps

1. ✅ Complete database setup
2. ✅ Start AI service
3. ✅ Start rAthena servers
4. ✅ Run E2E tests
5. ✅ Test NPC scripts
6. ✅ Monitor performance
7. 🎯 Deploy to production

---

## Additional Documentation

📄 **Verification & Status:**
- `FINAL_VERIFICATION_REPORT.md` - Complete test results and verification

🚀 **Deployment:**
- `UBUNTU_SERVER_DEPLOYMENT_GUIDE.md` - Comprehensive production deployment guide

📚 **Advanced Features:**
- `docs/ADVANCED_AUTONOMOUS_FEATURES.md` - Advanced autonomous features documentation
- `ai-autonomous-world/README.md` - AI autonomous world system overview
- `src/p2p-coordinator/README.md` - P2P coordinator service documentation

🔧 **Technical Details:**
- `ai-autonomous-world/docs/ARCHITECTURE.md` - System architecture
- `ai-autonomous-world/docs/CONFIGURATION.md` - Configuration guide
- `src/p2p-coordinator/README.md` - P2P coordinator API documentation

