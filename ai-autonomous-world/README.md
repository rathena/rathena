# AI Autonomous World System for rAthena

This directory contains the complete AI-driven autonomous world system for the rAthena MMORPG emulator. The system transforms Ragnarok Online into a living, breathing world with AI-driven NPCs and adaptive systems.

## 📁 Directory Structure

```
ai-autonomous-world/
├── ai-service/              # AI Service Layer (Python/FastAPI)
│   ├── agents/              # AI agent implementations
│   │   ├── base_agent.py   # Base agent class
│   │   ├── dialogue_agent.py  # Dialogue generation
│   │   ├── decision_agent.py  # Decision making
│   │   ├── memory_agent.py    # Memory management
│   │   ├── world_agent.py     # World state analysis
│   │   ├── quest_agent.py     # Quest generation
│   │   ├── economy_agent.py   # Economy simulation
│   │   ├── orchestrator.py    # Agent coordination (CrewAI)
│   │   ├── npc/            # NPC agent modules (empty - future)
│   │   ├── world/          # World system agents (empty - future)
│   │   └── meta/           # Meta coordination agents (empty - future)
│   ├── memory/              # Memory management (empty - using DragonflyDB fallback)
│   ├── llm/                 # LLM provider abstraction
│   │   ├── providers/      # Provider implementations
│   │   │   ├── openai_provider.py
│   │   │   ├── azure_openai_provider.py
│   │   │   ├── anthropic_provider.py
│   │   │   └── google_provider.py
│   │   ├── base.py         # Base provider interface
│   │   ├── factory.py      # Provider factory
│   │   └── gpu_wrapper.py  # GPU acceleration wrapper
│   ├── bridge/              # Bridge layer client (empty - future)
│   ├── config/              # Service configuration (empty)
│   ├── models/              # Data models
│   │   ├── npc.py          # NPC models
│   │   ├── player.py       # Player models
│   │   ├── world.py        # World state models
│   │   ├── quest.py        # Quest models
│   │   ├── economy.py      # Economy models
│   │   └── faction.py      # Faction models
│   ├── routers/             # FastAPI routers
│   │   ├── npc.py          # NPC endpoints
│   │   ├── player.py       # Player interaction endpoints
│   │   ├── world.py        # World state endpoints
│   │   ├── quest.py        # Quest endpoints
│   │   └── chat_command.py # Chat command interface
│   ├── utils/               # Utility functions
│   │   ├── gpu_manager.py  # GPU management
│   │   ├── movement_utils.py  # NPC movement
│   │   ├── pathfinding.py  # Pathfinding algorithms
│   │   ├── advanced_movement.py  # Advanced movement features
│   │   ├── gpu_pathfinding.py  # GPU-accelerated pathfinding
│   │   ├── gpu_vector_search.py  # GPU vector search
│   │   ├── map_data.py     # Map data handling
│   │   └── bridge_commands.py  # Bridge command utilities
│   ├── tests/               # Unit and integration tests
│   ├── logs/                # Application logs
│   ├── main.py              # FastAPI application entry point
│   ├── config.py            # Configuration management
│   ├── database.py          # DragonflyDB/Redis client
│   ├── requirements.txt     # Full Python dependencies
│   ├── requirements-minimal.txt  # Minimal dependencies
│   ├── requirements-cloud.txt    # Cloud deployment dependencies
│   ├── requirements-gpu.txt      # GPU acceleration dependencies
│   └── requirements-test.txt     # Testing dependencies
├── ai_service/              # Duplicate directory (symlink or copy)
├── config/                  # Global configuration files
│   └── ai-service-config.example.yaml
├── docs/                    # Complete documentation
│   ├── ARCHITECTURE.md      # Technical architecture
│   ├── WORLD_CONCEPT_DESIGN.md  # World design and AI systems
│   ├── EXECUTIVE_SUMMARY.md # Executive overview
│   ├── QUICK_START.md       # Quick start guide
│   ├── CONFIGURATION.md     # Configuration guide
│   ├── FREE_FORM_TEXT_INPUT.md  # Free-form text input guide
│   ├── GPU_ACCELERATION.md  # GPU acceleration guide
│   ├── GPU_INSTALLATION.md  # GPU installation guide
│   ├── INDEX.md             # Documentation index
│   └── README.md            # Documentation overview
├── tests/                   # Integration tests
├── venv/                    # Python virtual environment
└── README.md                # This file
```

## 🚀 Quick Start

### Prerequisites

- Python 3.11 or higher
- PostgreSQL 17 (for persistent memory storage)
- DragonflyDB (Redis-compatible in-memory database for caching)
- rAthena server (located in parent directory)
- LLM API access (Azure OpenAI, OpenAI, DeepSeek, Anthropic Claude, Google Gemini)

### Installation

1. **Activate the virtual environment:**
   ```bash
   cd <workspace>/rathena-AI-world/ai-autonomous-world
   source venv/bin/activate
   ```

2. **Install dependencies:**

   For minimal installation (basic functionality):
   ```bash
   pip install -r ai-service/requirements-minimal.txt
   ```

   For full installation (all features including CrewAI, all LLM providers):
   ```bash
   pip install -r ai-service/requirements.txt
   ```

   For cloud deployment:
   ```bash
   pip install -r ai-service/requirements-cloud.txt
   ```

   For GPU acceleration:
   ```bash
   pip install -r ai-service/requirements-gpu.txt
   ```

   **Note:** Full installation requires ~5GB of disk space. Use minimal installation if disk space is limited.

3. **Set up PostgreSQL 17:**
   ```bash
   # Install PostgreSQL 17 with required extensions
   # See INSTALL.md for detailed instructions

   # Create database and user
   sudo -u postgres psql -c "CREATE DATABASE ai_world_memory;"
   sudo -u postgres psql -c "CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';"
   sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;"

   # Run database migrations
   cd ai-service
   PGPASSWORD=ai_world_pass_2025 psql -h localhost -U ai_world_user -d ai_world_memory -f migrations/001_create_factions_table.sql
   ```

4. **Set up DragonflyDB:**
   ```bash
   # Install DragonflyDB natively (recommended)
   # See INSTALL.md for detailed instructions

   # Or use Docker (not recommended per project requirements)
   # docker run -d --name dragonfly -p 6379:6379 docker.dragonflydb.io/dragonflydb/dragonfly
   ```

5. **Configure the service:**
   Create a `.env` file in the `ai-service` directory with your settings:
   ```bash
   cd ai-service
   # Copy example and edit with your API keys
   cp .env.example .env
   nano .env  # or use your preferred editor

   # REQUIRED: Add at least one LLM provider API key
   # AZURE_OPENAI_API_KEY=your-key-here (recommended)
   # or OPENAI_API_KEY=your-key-here
   # or ANTHROPIC_API_KEY=your-key-here
   ```

### Running the AI Service

```bash
cd ai-service
source ../venv/bin/activate

# Start the service
python main.py

# Or use uvicorn directly
python -m uvicorn main:app --host 0.0.0.0 --port 8000
```

The service will start on `http://localhost:8000` by default.

### Verifying the Installation

Test all endpoints to ensure 100% pass rate:

```bash
# From the rathena-AI-world directory
python3 test_endpoints_simple.py

# Expected output:
# ================================================================================
# RESULTS: 10/10 tests passed (100% pass rate)
# ================================================================================
```

## 📚 Documentation

All documentation is located in the `docs/` directory:

- **[docs/README.md](docs/README.md)** - Documentation overview and project introduction
- **[docs/INDEX.md](docs/INDEX.md)** - Complete documentation index
- **[docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md)** - Executive overview and roadmap
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Complete technical architecture
- **[docs/WORLD_CONCEPT_DESIGN.md](docs/WORLD_CONCEPT_DESIGN.md)** - AI systems and world design
- **[docs/QUICK_START.md](docs/QUICK_START.md)** - Detailed setup guide
- **[docs/CONFIGURATION.md](docs/CONFIGURATION.md)** - Configuration options
- **[docs/FREE_FORM_TEXT_INPUT.md](docs/FREE_FORM_TEXT_INPUT.md)** - Free-form text input guide
- **[docs/GPU_ACCELERATION.md](docs/GPU_ACCELERATION.md)** - GPU acceleration guide
- **[docs/GPU_INSTALLATION.md](docs/GPU_INSTALLATION.md)** - GPU installation guide

## 🏗️ Architecture Overview

The system uses a 5-layer architecture:

1. **rAthena Game Server** - Core MMORPG server (C++)
2. **Bridge Layer** - REST API extension to rAthena web server
3. **AI Service Layer** - FastAPI application with CrewAI agents
4. **State Management** - DragonflyDB for shared state
5. **LLM Provider Layer** - Abstraction for multiple LLM providers

## 🔧 Integration with rAthena

The AI system is designed to be isolated from the rAthena core codebase:

- **Bridge Layer**: Planned extension to rAthena web server (not yet implemented)
- **NPC Scripts**: Custom AI-enabled NPCs will be in `../npc/custom/ai-world/` (not yet implemented)
- **No Core Modifications**: The system is designed to work as an extension, not a modification
- **Current Status**: AI service is functional as standalone FastAPI application

## 🧪 Testing

Run tests with:
```bash
cd ai-service
source ../venv/bin/activate
pytest tests/
```

## 📊 Current Status

🎉 **100% ENDPOINT PASS RATE ACHIEVED!** 🎉

### ✅ Fully Implemented and Tested
- **Core Infrastructure**
  - Directory structure created
  - Python virtual environment set up
  - Core dependencies installed (CrewAI, FastAPI, PostgreSQL, DragonflyDB)
  - Configuration management (config.py + .env + YAML support)

- **Database Layer**
  - PostgreSQL 17 integration with pgvector, TimescaleDB, Apache AGE extensions
  - DragonflyDB/Redis integration for high-speed caching
  - Database migrations system (factions, player_reputation, faction_events, faction_conflicts)
  - Dual-database architecture (PostgreSQL for persistence, DragonflyDB for caching)

- **LLM Provider System**
  - Multi-provider abstraction layer with factory pattern
  - Azure OpenAI provider (default, production-ready)
  - OpenAI provider
  - Anthropic Claude provider
  - Google Gemini provider
  - Environment variable configuration for all providers
  - Automatic fallback and error handling

- **AI Agent System**
  - DialogueAgent - NPC conversation generation
  - DecisionAgent - NPC decision making
  - MemoryAgent - Memory management (DragonflyDB fallback)
  - WorldAgent - World state analysis
  - QuestAgent - Dynamic quest generation
  - EconomyAgent - Economic simulation
  - Agent Orchestrator - CrewAI-based multi-agent coordination

- **API Endpoints (10/10 passing)**
  - ✅ Health Check (200)
  - ✅ Detailed Health (200)
  - ✅ World State (200)
  - ✅ NPC Registration (200)
  - ✅ Quest Generation (200)
  - ✅ Chat Command (200)
  - ✅ List Factions (200)
  - ✅ Create Faction (200)
  - ✅ Economy State (200)
  - ✅ Market Trends (200)

- **Data Models**
  - NPC models (NPCRegisterRequest, NPCPosition, NPCPersonality)
  - Player models (PlayerInteractionRequest, PlayerInteractionResponse)
  - World models (WorldState, WorldStateQuery)
  - Quest models (Quest, QuestGenerationRequest, QuestObjective, QuestReward)
  - Economy models (EconomicState, MarketTrend)
  - Faction models (Faction, PlayerReputation, FactionEvent)

- **Additional Features**
  - Free-form text input via chat commands
  - NPC movement utilities
  - Pathfinding algorithms
  - GPU acceleration support (optional)
  - Rate limiting middleware
  - Comprehensive logging with Loguru
  - DateTime serialization for caching
  - Redis Pub/Sub for async NPC actions

### ⏳ Planned/Not Implemented
- Bridge Layer (C++ extension to rAthena web server)
- Memori SDK integration (currently using DragonflyDB fallback)
- NPC-specific agent modules (agents/npc/ directory - future expansion)
- World-specific agent modules (agents/world/ directory - future expansion)
- Meta coordination agents (agents/meta/ directory - future expansion)
- Bridge layer client (bridge/ directory - future integration)
- Example NPC scripts for rAthena
- Production deployment configuration (Kubernetes, monitoring)
- Docker support (intentionally excluded per project requirements)

## 🔗 Related Directories

- **rAthena Source**: `../src/` - Core rAthena C++ source code
- **rAthena NPCs**: `../npc/` - NPC scripts (AI NPCs planned for `../npc/custom/ai-world/`)
- **rAthena Web Server**: `../src/web/web.cpp` - Where Bridge Layer endpoints will be added (planned)

## 📝 Notes

- The system is designed to be non-invasive and can be disabled without affecting rAthena
- All AI-related files are contained within this directory
- The virtual environment is self-contained and portable
- Configuration files use relative paths where possible

## 🆘 Support

For issues, questions, or contributions, refer to the documentation in the `docs/` directory.

