# Quick Start Guide

This guide will help you get the AI-driven autonomous world system up and running quickly for development and testing.

## Prerequisites

- Linux/macOS (Windows with WSL2)
- Docker and Docker Compose installed
- Python 3.11 or higher
- Git
- At least 8GB RAM available
- Azure OpenAI account (or Ollama for local testing)

## Step 1: Navigate to AI System Directory

```bash
# Navigate to the AI autonomous world system
cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world

# Verify directory structure
ls -la
# You should see: ai-service/, config/, docs/, venv/, README.md
```

## Step 2: Install Python Dependencies

```bash
# Activate the virtual environment
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
# For minimal installation (basic functionality):
pip install -r ai-service/requirements-minimal.txt

# For full installation (all features):
pip install -r ai-service/requirements.txt

# Note: Full installation requires ~5GB disk space
# Use minimal installation if disk space is limited
```

## Step 3: Set Up DragonflyDB

```bash
# Start DragonflyDB using Docker
docker run -d \
  --name dragonfly \
  -p 6379:6379 \
  -v dragonfly-data:/data \
  docker.dragonflydb.io/dragonflydb/dragonfly

# Verify it's running
docker ps | grep dragonfly

# Test connection
redis-cli ping
# Should return: PONG
```

## Step 4: Configure Environment

Create `.env` file in the ai-service directory:

```bash
# Navigate to ai-service directory
cd ai-service

# Copy the example environment file
cp .env.example .env

# Edit the file with your actual values
nano .env  # or use your preferred editor
```

Example minimal configuration for Azure OpenAI:

```bash
# ai-service/.env

# Service Configuration
SERVICE_NAME=ai-service
SERVICE_HOST=0.0.0.0
SERVICE_PORT=8000
ENVIRONMENT=development
DEBUG=true

# DragonflyDB/Redis Configuration
REDIS_HOST=127.0.0.1
REDIS_PORT=6379
REDIS_DB=0

# LLM Provider Configuration
DEFAULT_LLM_PROVIDER=azure_openai

# Azure OpenAI Configuration
AZURE_OPENAI_API_KEY=your-api-key-here
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com
AZURE_OPENAI_DEPLOYMENT=gpt-4
AZURE_OPENAI_API_VERSION=2024-02-15-preview

# Logging
LOG_LEVEL=INFO
LOG_FILE=logs/ai-service.log
```

Alternative configuration for OpenAI:

```bash
DEFAULT_LLM_PROVIDER=openai
OPENAI_API_KEY=your-api-key-here
OPENAI_MODEL=gpt-4
```

## Step 5: Verify AI Service Structure

The AI service is already implemented with the following structure:

```bash
ai-service/
├── main.py              # FastAPI application entry point (✅ implemented)
├── config.py            # Configuration management (✅ implemented)
├── database.py          # DragonflyDB client (✅ implemented)
├── agents/              # AI agents (✅ implemented)
│   ├── base_agent.py
│   ├── dialogue_agent.py
│   ├── decision_agent.py
│   ├── memory_agent.py
│   ├── world_agent.py
│   ├── quest_agent.py
│   ├── economy_agent.py
│   └── orchestrator.py
├── llm/                 # LLM providers (✅ implemented)
│   ├── base.py
│   ├── factory.py
│   └── providers/
│       ├── openai_provider.py
│       ├── azure_openai_provider.py
│       ├── anthropic_provider.py
│       └── google_provider.py
├── models/              # Data models (✅ implemented)
│   ├── npc.py
│   ├── player.py
│   ├── world.py
│   ├── quest.py
│   ├── economy.py
│   └── faction.py
├── routers/             # API endpoints (✅ implemented)
│   ├── npc.py
│   ├── player.py
│   ├── world.py
│   ├── quest.py
│   └── chat_command.py
└── utils/               # Utilities (✅ implemented)
    ├── gpu_manager.py
    ├── movement_utils.py
    └── pathfinding.py
```

No need to create files - they already exist!

## Step 6: Test AI Service

```bash
# Make sure you're in the ai-service directory
cd ai-service

# Activate virtual environment
source ../venv/bin/activate

# Start the AI service
python main.py
```

You should see output like:
```
2025-11-06 10:00:00 | INFO     | ai_service.config:get_settings:267 - Settings loaded: ai-service on 0.0.0.0:8000
2025-11-06 10:00:00 | INFO     | ai_service.main:lifespan:69 - Database connection established
2025-11-06 10:00:00 | INFO     | ai_service.main:lifespan:99 - LLM provider initialized: azure_openai
INFO:     Started server process [12345]
INFO:     Waiting for application startup.
INFO:     Application startup complete.
INFO:     Uvicorn running on http://0.0.0.0:8000 (Press CTRL+C to quit)
```

In another terminal, test the service:
```bash
# Test root endpoint
curl http://localhost:8000/

# Test health check
curl http://localhost:8000/health

# Test detailed health check
curl http://localhost:8000/health/detailed

# View API documentation
# Open in browser: http://localhost:8000/docs
```

Expected output from root endpoint:
```json
{
  "service": "ai-service",
  "version": "1.0.0",
  "status": "running",
  "docs": "/docs"
}
```

## Step 7: Example NPC Integration (Future)

**Note**: NPC integration with rAthena requires the Bridge Layer, which is not yet implemented.

When the Bridge Layer is complete, you will be able to create AI-enabled NPCs like this:

```c
//===== rAthena Script =======================================
//= AI-Enabled NPC Example (FUTURE IMPLEMENTATION)
//===== Description: =========================================
//= Example of an NPC that uses the AI service for autonomous behavior
//= REQUIRES: Bridge Layer implementation
//============================================================

prontera,150,150,4	script	AI Merchant Marcus	4_M_MERCHANT,{
    // This is a PLANNED implementation - not yet functional

    // When player talks to NPC
    .@player_name$ = strcharinfo(0);
    .@player_level = BaseLevel;

    // Call AI service via Bridge Layer (NOT YET IMPLEMENTED)
    // Will use HTTP/REST API to communicate with AI service

    mes "[Marcus]";
    mes "Greetings, " + .@player_name$ + "!";
    mes "I am an AI-enabled merchant.";
    mes "My behavior is driven by artificial intelligence!";
    close;

OnInit:
    // Register this NPC with the AI service via Bridge Layer
    .npc_id$ = "npc_marcus_001";
    .personality$ = "ambitious,cunning,low_empathy";
    .role$ = "merchant";

    // Start autonomous behavior timer
    initnpctimer;
    end;

OnTimer30000:
    // Every 30 seconds, make an autonomous decision
    // via AI service (NOT YET IMPLEMENTED)

    stopnpctimer;
    initnpctimer;
    end;
}
```

**Current Status**: The AI service is ready to handle NPC interactions via API, but the Bridge Layer to connect rAthena to the AI service is not yet implemented.

## Step 8: Test AI Service API

Since the Bridge Layer is not yet implemented, you can test the AI service directly via its API:

1. **Verify DragonflyDB is running**:
```bash
redis-cli ping
# Expected: PONG
```

2. **Test AI Service endpoints**:
```bash
# Health check
curl http://localhost:8000/health

# Detailed health check
curl http://localhost:8000/health/detailed

# Test NPC dialogue generation
curl -X POST http://localhost:8000/api/npc/dialogue \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "npc_marcus_001",
    "player_id": "player_001",
    "context": "Player greets the merchant"
  }'

# Test chat command (free-form text input)
curl -X POST http://localhost:8000/api/chat/command \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "player_001",
    "command": "talk to Marcus about trading"
  }'
```

3. **View API Documentation**:
   - Open browser: http://localhost:8000/docs
   - Interactive API documentation with Swagger UI

4. **Monitor logs**:
```bash
tail -f ai-service/logs/ai-service.log
```

## Step 9: Next Steps

Now that your environment is set up:

1. **Read the Architecture**: Review [ARCHITECTURE.md](./ARCHITECTURE.md) for detailed system design
2. **Read the World Concept**: Review [WORLD_CONCEPT_DESIGN.md](./WORLD_CONCEPT_DESIGN.md) for NPC behavior design
3. **Read Configuration Guide**: Review [CONFIGURATION.md](./CONFIGURATION.md) for all configuration options
4. **Explore API**: Use the interactive API docs at http://localhost:8000/docs
5. **Implement Bridge Layer**: Create the C++ extension to rAthena web server (pending)
6. **Test with Simple NPC**: Once Bridge Layer is ready, test with one AI-enabled NPC

**Current Implementation Status**:
- ✅ AI Service (FastAPI application)
- ✅ LLM Providers (OpenAI, Azure OpenAI, Anthropic, Google)
- ✅ AI Agents (Dialogue, Decision, Memory, World, Quest, Economy)
- ✅ API Endpoints (NPC, Player, World, Quest, Chat Command)
- ⏳ Bridge Layer (not yet implemented)
- ⏳ Memori SDK integration (using DragonflyDB fallback)

## Development Workflow

```bash
# Terminal 1: DragonflyDB
docker logs -f dragonfly

# Terminal 2: AI Service
cd ai-service
source ../venv/bin/activate
python main.py

# Terminal 3: Testing/Development
# Use curl or browser to test API endpoints
curl http://localhost:8000/docs

# Terminal 4: rAthena (when Bridge Layer is ready)
# cd rathena-AI-world
# ./athena-start start
# Use this for testing API calls, running scripts, etc.
```

## Troubleshooting

### DragonflyDB won't start
```bash
# Check if port 6379 is already in use
lsof -i :6379

# If Redis is running, stop it
sudo systemctl stop redis

# Restart DragonflyDB
docker restart dragonfly
```

### AI Service won't start
```bash
# Check Python version
python --version  # Should be 3.11+

# Reinstall dependencies
pip install --force-reinstall -r requirements.txt

# Check for port conflicts
lsof -i :8000
```

### Import errors
```bash
# Make sure virtual environment is activated
source venv/bin/activate

# Verify packages are installed
pip list | grep -E "(crewai|memorisdk|fastapi)"
```

## Resources

- [FastAPI Documentation](https://fastapi.tiangolo.com/)
- [CrewAI Documentation](https://docs.crewai.com/)
- [Memori SDK Documentation](https://github.com/kingjulio8238/memori)
- [DragonflyDB Documentation](https://www.dragonflydb.io/docs)
- [rAthena Documentation](https://github.com/rathena/rathena/wiki)

## Support

If you encounter issues:
1. Check the logs in `logs/ai-service.log`
2. Review the error messages carefully
3. Consult the detailed architecture documentation
4. Open an issue on GitHub with detailed error information

---

**Next**: Proceed to implementing the Bridge Layer and AI Agents as outlined in the Architecture document.
