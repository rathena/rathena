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

3. **Set up DragonflyDB:**
   ```bash
   docker run -d --name dragonfly -p 6379:6379 docker.dragonflydb.io/dragonflydb/dragonfly
   ```

4. **Configure the service:**
   Create a `.env` file in the `ai-service` directory with your settings:
   ```bash
   cd ai-service
   # Copy example and edit with your API keys
   cp .env.example .env
   nano .env  # or use your preferred editor
   ```

### Running the AI Service

```bash
cd ai-service
source ../venv/bin/activate
python main.py
```

The service will start on `http://localhost:8000` by default.

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

### ✅ Implemented
- Directory structure created
- Python virtual environment set up
- Core dependencies installed
- FastAPI application with health checks
- Configuration management (config.py + .env support)
- PostgreSQL 17 integration with Memori SDK
- DragonflyDB/Redis integration for caching
- LLM provider abstraction (Azure OpenAI, OpenAI, DeepSeek, Anthropic Claude, Google Gemini)
- AI agents (Dialogue, Decision, Memory, World, Quest, Economy)
- Agent orchestration with CrewAI
- API routers (NPC, Player, World, Quest, Chat Command)
- Data models (NPC, Player, World, Quest, Economy, Faction)
- GPU acceleration support (optional)
- Free-form text input via chat commands
- NPC movement utilities
- Pathfinding algorithms
- Comprehensive test suite

### ⏳ Planned/Not Implemented
- Bridge Layer (C++ extension to rAthena)
- Memori SDK integration (using DragonflyDB fallback)
- NPC-specific agent modules (agents/npc/ directory empty)
- World-specific agent modules (agents/world/ directory empty)
- Meta coordination agents (agents/meta/ directory empty)
- Bridge layer client (bridge/ directory empty)
- Example NPC scripts for rAthena
- Production deployment configuration

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

