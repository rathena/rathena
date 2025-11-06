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
# Copy the example environment file
cp ai-service/.env.example ai-service/.env

# Edit the file with your actual values
nano ai-service/.env  # or use your preferred editor

# At minimum, configure:
# - PRIMARY_LLM_PROVIDER (azure_openai, openai, ollama, etc.)
# - LLM API keys for your chosen provider
# - DRAGONFLY_HOST and DRAGONFLY_PORT
```

Example minimal configuration for local development with Ollama:

```bash
# ai-service/.env
PRIMARY_LLM_PROVIDER=ollama
OLLAMA_BASE_URL=http://localhost:11434
OLLAMA_MODEL=llama2
DRAGONFLY_HOST=localhost
DRAGONFLY_PORT=6379
SERVICE_PORT=8000
LOG_LEVEL=DEBUG
LOG_LEVEL=INFO
LOG_FILE=logs/ai-service.log

# Performance
MAX_CONCURRENT_LLM_CALLS=10
LLM_CACHE_TTL=3600
NPC_DECISION_CACHE_TTL=300
BATCH_SIZE=10
EOF
```

## Step 5: Create Basic AI Service Structure

Create `ai-service/main.py`:

```python
"""
AI Service Main Entry Point
"""
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
from dotenv import load_dotenv
import os

# Load environment variables
load_dotenv()

# Create FastAPI app
app = FastAPI(
    title="rAthena AI Service",
    description="Autonomous NPC and World System AI Service",
    version="1.0.0"
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Configure appropriately for production
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.get("/")
async def root():
    return {
        "service": "rAthena AI Service",
        "status": "running",
        "version": "1.0.0"
    }

@app.get("/health")
async def health_check():
    return {"status": "healthy"}

# TODO: Add more endpoints for NPC interactions, world events, etc.

if __name__ == "__main__":
    host = os.getenv("AI_SERVICE_HOST", "0.0.0.0")
    port = int(os.getenv("AI_SERVICE_PORT", 8000))
    workers = int(os.getenv("AI_SERVICE_WORKERS", 4))
    
    uvicorn.run(
        "main:app",
        host=host,
        port=port,
        workers=workers,
        reload=True  # Disable in production
    )
```

## Step 6: Test AI Service

```bash
# Start the AI service
cd ai-service
python main.py

# In another terminal, test the service
curl http://localhost:8000/
curl http://localhost:8000/health
```

Expected output:
```json
{
  "service": "rAthena AI Service",
  "status": "running",
  "version": "1.0.0"
}
```

## Step 7: Create Example NPC Script

Create `rathena-AI-world/npc/custom/ai-world/ai_npc_example.txt`:

```c
//===== rAthena Script =======================================
//= AI-Enabled NPC Example
//===== Description: =========================================
//= Example of an NPC that uses the AI service for autonomous behavior
//============================================================

prontera,150,150,4	script	AI Merchant Marcus	4_M_MERCHANT,{
    // This NPC is AI-enabled and will make autonomous decisions
    
    // When player talks to NPC
    .@player_name$ = strcharinfo(0);
    .@player_level = BaseLevel;
    
    // Call AI service via HTTP (requires custom script command or bridge)
    // For now, this is a placeholder showing the concept
    
    mes "[Marcus]";
    mes "Greetings, " + .@player_name$ + "!";
    mes "I am an AI-enabled merchant.";
    mes "My behavior is driven by artificial intelligence!";
    next;
    
    mes "[Marcus]";
    mes "I remember our past interactions,";
    mes "and I make decisions based on my personality,";
    mes "goals, and the current state of the world.";
    close;

OnInit:
    // Register this NPC with the AI service
    // This would call the Bridge API to register the NPC
    .npc_id$ = "npc_marcus_001";
    .personality$ = "ambitious,cunning,low_empathy";
    .role$ = "merchant";
    
    // Set initial state
    .wealth = 10000;
    .reputation = 50;
    .moral_alignment = 0; // Neutral
    
    // Start autonomous behavior timer
    initnpctimer;
    end;

OnTimer30000:
    // Every 30 seconds, make an autonomous decision
    // This would call the AI service to get next action
    
    // Placeholder: In real implementation, this would:
    // 1. Send current state to AI service
    // 2. Receive decision from AI
    // 3. Execute the decision (move, talk, trade, etc.)
    
    stopnpctimer;
    initnpctimer;
    end;
}
```

## Step 8: Verify Setup

Run this checklist:

```bash
# 1. Check DragonflyDB
redis-cli ping
# Expected: PONG

# 2. Check AI Service
curl http://localhost:8000/health
# Expected: {"status":"healthy"}

# 3. Check Python packages
python -c "import crewai; import memorisdk; print('Packages OK')"
# Expected: Packages OK

# 4. Check rAthena (if running)
# Navigate to rAthena directory and check if servers are running
ps aux | grep -E "(map-server|char-server|login-server)"
```

## Step 9: Next Steps

Now that your environment is set up:

1. **Read the Architecture**: Review [ARCHITECTURE.md](./ARCHITECTURE.md) for detailed system design
2. **Read the World Concept**: Review [WORLD_CONCEPT_DESIGN.md](./WORLD_CONCEPT_DESIGN.md) for NPC behavior design
3. **Implement Bridge Layer**: Create the C++ extension to rAthena web server
4. **Implement AI Agents**: Create CrewAI agents for NPCs and world systems
5. **Integrate Memori SDK**: Set up memory management for NPCs
6. **Test with Simple NPC**: Start with one AI-enabled NPC and test the full flow

## Development Workflow

```bash
# Terminal 1: DragonflyDB
docker logs -f dragonfly

# Terminal 2: AI Service
cd ai-service
source ../venv/bin/activate
python main.py

# Terminal 3: rAthena (when ready)
cd rathena-AI-world
./athena-start start

# Terminal 4: Development/Testing
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
