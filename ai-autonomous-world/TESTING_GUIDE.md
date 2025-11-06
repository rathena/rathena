# Testing Guide - AI Autonomous World System

Complete guide for testing the AI Autonomous World System integration.

## Prerequisites

### Required Services
1. **DragonflyDB** - Port 6379
2. **AI Service** - Port 8000
3. **rAthena Web Server** (Bridge Layer) - Port 8888

### Required Configuration
1. LLM Provider API key (OpenAI, Anthropic, or Google)
2. Environment variables or `.env` file configured
3. Python virtual environment activated

## Step 1: Start DragonflyDB

### Install DragonflyDB
```bash
# Ubuntu/Debian
curl -fsSL https://packages.dragonflydb.io/dragonfly.gpg | sudo gpg --dearmor -o /usr/share/keyrings/dragonfly.gpg
echo "deb [signed-by=/usr/share/keyrings/dragonfly.gpg] https://packages.dragonflydb.io/apt stable main" | sudo tee /etc/apt/sources.list.d/dragonfly.list
sudo apt update
sudo apt install dragonfly

# Or use Docker
docker run -d --name dragonfly -p 6379:6379 docker.dragonflydb.io/dragonflydb/dragonfly
```

### Start DragonflyDB
```bash
# Direct
dragonfly --port 6379 --logtostderr

# Or Docker
docker start dragonfly
```

### Verify Connection
```bash
redis-cli -p 6379 ping
# Expected: PONG
```

## Step 2: Configure AI Service

### Create Configuration File
```bash
cd ai-autonomous-world/ai-service
cp .env.example .env
```

### Edit `.env` File
```bash
# Minimum required configuration
DEFAULT_LLM_PROVIDER=openai
OPENAI_API_KEY=your-actual-api-key-here
REDIS_HOST=127.0.0.1
REDIS_PORT=6379
```

### Verify Configuration
```bash
source ../venv/bin/activate
python -c "from config import settings; print(f'Provider: {settings.default_llm_provider}')"
```

## Step 3: Start AI Service

### Activate Virtual Environment
```bash
cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world
source venv/bin/activate
```

### Start Service
```bash
cd ai-service
python main.py
```

### Expected Output
```
2024-XX-XX XX:XX:XX | INFO     | Logging configured
2024-XX-XX XX:XX:XX | INFO     | Starting AI Autonomous World Service
2024-XX-XX XX:XX:XX | INFO     | Environment: development
2024-XX-XX XX:XX:XX | INFO     | Connecting to Redis at 127.0.0.1:6379
2024-XX-XX XX:XX:XX | INFO     | Successfully connected to Redis/DragonflyDB
2024-XX-XX XX:XX:XX | INFO     | LLM provider initialized: openai
2024-XX-XX XX:XX:XX | INFO     | AI Service startup complete
INFO:     Uvicorn running on http://0.0.0.0:8000
```

### Verify AI Service
```bash
# In another terminal
curl http://localhost:8000/health
```

Expected response:
```json
{
  "status": "healthy",
  "service": "AI Autonomous World Service",
  "version": "1.0.0",
  "environment": "development",
  "database": "connected"
}
```

## Step 4: Test AI Service Endpoints

### Test NPC Registration
```bash
curl -X POST http://localhost:8000/ai/npc/register \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_npc_001",
    "name": "Test Merchant",
    "npc_class": "merchant",
    "level": 50,
    "position": {"map": "prontera", "x": 150, "y": 180},
    "personality": {
      "openness": 0.7,
      "conscientiousness": 0.8,
      "extraversion": 0.6,
      "agreeableness": 0.9,
      "neuroticism": 0.3,
      "moral_alignment": "lawful_good"
    }
  }'
```

Expected response:
```json
{
  "status": "success",
  "agent_id": "agent_test_npc_001_XXXXXXXX",
  "npc_id": "test_npc_001",
  "message": "NPC Test Merchant registered successfully"
}
```

### Test Player Interaction
```bash
curl -X POST http://localhost:8000/ai/player/interaction \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_npc_001",
    "player_id": "player_001",
    "interaction_type": "talk",
    "message": "Hello!",
    "timestamp": "2024-01-01T12:00:00",
    "context": {
      "player_name": "TestPlayer",
      "player_class": "Swordsman",
      "player_level": 50,
      "location": {"map": "prontera", "x": 150, "y": 180},
      "time_of_day": "day",
      "weather": "clear"
    }
  }'
```

Expected response (with LLM):
```json
{
  "npc_id": "test_npc_001",
  "player_id": "player_001",
  "response": {
    "action": "dialogue",
    "data": {
      "text": "Greetings, brave Swordsman! Welcome to my shop...",
      "speaker": "Test Merchant"
    },
    "emotion": "friendly",
    "next_actions": ["talk", "trade", "quest", "end_conversation"]
  },
  "npc_state_update": {...},
  "relationship_change": {"player_001": 1}
}
```

### Test World State
```bash
# Update world state
curl -X POST http://localhost:8000/ai/world/state \
  -H "Content-Type: application/json" \
  -d '{
    "state_type": "economy",
    "state_data": {
      "item_prices": {"potion": 500, "sword": 10000},
      "trade_volume": 1000000.0,
      "inflation_rate": 0.02
    },
    "timestamp": "2024-01-01T12:00:00",
    "source": "test"
  }'

# Query world state
curl "http://localhost:8000/ai/world/state?state_type=economy"
```

## Step 5: Run Integration Tests

### Install Test Dependencies
```bash
source venv/bin/activate
pip install pytest pytest-asyncio
```

### Run Tests
```bash
cd ai-autonomous-world
pytest tests/test_integration.py -v -s
```

Expected output:
```
tests/test_integration.py::TestAIServiceHealth::test_ai_service_health PASSED
tests/test_integration.py::TestAIServiceHealth::test_ai_service_root PASSED
tests/test_integration.py::TestNPCEndpoints::test_npc_registration PASSED
tests/test_integration.py::TestNPCEndpoints::test_npc_event PASSED
tests/test_integration.py::TestNPCEndpoints::test_npc_action PASSED
tests/test_integration.py::TestWorldEndpoints::test_world_state_update PASSED
tests/test_integration.py::TestWorldEndpoints::test_world_state_query PASSED
tests/test_integration.py::TestPlayerEndpoints::test_player_interaction PASSED

======================== 8 passed in X.XXs ========================
```

## Step 6: Test Bridge Layer (Future)

### Compile rAthena with Bridge Layer
```bash
cd /home/lot399/ai-mmorpg-world/rathena-AI-world
mkdir build && cd build
cmake ..
make
```

### Start rAthena Servers
```bash
./login-server
./char-server
./map-server
```

### Test Bridge Layer Endpoints
```bash
# Test via Bridge Layer (proxies to AI Service)
curl -X POST http://localhost:8888/ai/npc/register \
  -H "Content-Type: application/json" \
  -d '{...}'
```

## Troubleshooting

### AI Service Won't Start

**Error**: `Failed to connect to Redis`
```bash
# Check if DragonflyDB is running
redis-cli -p 6379 ping

# Check port availability
netstat -tuln | grep 6379
```

**Error**: `LLM provider initialization failed`
```bash
# Verify API key is set
echo $OPENAI_API_KEY

# Check .env file
cat ai-service/.env | grep OPENAI_API_KEY
```

### Tests Failing

**Error**: `Connection refused`
```bash
# Verify services are running
curl http://localhost:8000/health
curl http://localhost:6379  # Should fail (Redis protocol)
redis-cli -p 6379 ping      # Should return PONG
```

**Error**: `LLM generation failed`
- Check API key validity
- Verify internet connection
- Check LLM provider status
- Review logs: `ai-service/logs/ai-service.log`

### Database Issues

**Check Redis/DragonflyDB**:
```bash
redis-cli -p 6379
> KEYS *
> HGETALL npc:test_npc_001
> QUIT
```

**Clear Database**:
```bash
redis-cli -p 6379 FLUSHDB
```

## Next Steps

1. ✅ AI Service running and tested
2. ✅ Integration tests passing
3. ⏳ Compile Bridge Layer with rAthena
4. ⏳ Test end-to-end flow (rAthena → Bridge → AI Service)
5. ⏳ Load example NPC script
6. ⏳ Test in-game interaction

## Monitoring

### View Logs
```bash
# AI Service logs
tail -f ai-service/logs/ai-service.log

# DragonflyDB logs
docker logs -f dragonfly  # If using Docker
```

### Check Database State
```bash
redis-cli -p 6379
> DBSIZE
> KEYS npc:*
> KEYS world:*
> ZRANGE events:queue 0 -1
```

### Monitor Performance
```bash
# AI Service metrics
curl http://localhost:8000/health

# Redis/DragonflyDB info
redis-cli -p 6379 INFO
```

