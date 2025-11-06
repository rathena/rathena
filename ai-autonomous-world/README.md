# AI Autonomous World System for rAthena

This directory contains the complete AI-driven autonomous world system for the rAthena MMORPG emulator. The system transforms Ragnarok Online into a living, breathing world with AI-driven NPCs and adaptive systems.

## 📁 Directory Structure

```
ai-autonomous-world/
├── ai-service/              # AI Service Layer (Python/FastAPI)
│   ├── agents/              # CrewAI agent implementations
│   │   ├── npc/            # NPC agent modules
│   │   ├── world/          # World system agents
│   │   └── meta/           # Meta coordination agents
│   ├── memory/              # Memory management (Memori SDK)
│   ├── llm/                 # LLM provider abstraction
│   │   ├── providers/      # Provider implementations
│   │   └── factory/        # Provider factory
│   ├── bridge/              # Bridge layer client
│   ├── config/              # Service configuration
│   ├── models/              # Data models
│   ├── utils/               # Utility functions
│   ├── tests/               # Unit and integration tests
│   ├── logs/                # Application logs
│   ├── main.py              # FastAPI application entry point
│   ├── requirements.txt     # Full Python dependencies
│   └── requirements-minimal.txt  # Minimal dependencies (for limited disk space)
├── config/                  # Global configuration files
│   └── ai-service-config.example.yaml
├── docs/                    # Complete documentation
│   ├── ARCHITECTURE.md      # Technical architecture
│   ├── WORLD_CONCEPT_DESIGN.md  # World design and AI systems
│   ├── EXECUTIVE_SUMMARY.md # Executive overview
│   ├── QUICK_START.md       # Quick start guide
│   ├── INDEX.md             # Documentation index
│   └── README.md            # Documentation overview
├── venv/                    # Python virtual environment
└── README.md                # This file
```

## 🚀 Quick Start

### Prerequisites

- Python 3.11 or higher
- DragonflyDB (Redis-compatible in-memory database)
- rAthena server (located in parent directory)
- LLM API access (Azure OpenAI, OpenAI, Ollama, etc.)

### Installation

1. **Activate the virtual environment:**
   ```bash
   cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world
   source venv/bin/activate
   ```

2. **Install dependencies:**
   
   For minimal installation (basic functionality):
   ```bash
   pip install -r ai-service/requirements-minimal.txt
   ```
   
   For full installation (all features including CrewAI, Memori SDK, all LLM providers):
   ```bash
   pip install -r ai-service/requirements.txt
   ```
   
   **Note:** Full installation requires ~5GB of disk space. Use minimal installation if disk space is limited.

3. **Configure the service:**
   ```bash
   cp config/ai-service-config.example.yaml config/ai-service-config.yaml
   # Edit config/ai-service-config.yaml with your settings
   ```

4. **Set up environment variables:**
   ```bash
   cp ai-service/.env.example ai-service/.env
   # Edit ai-service/.env with your API keys
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
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - Complete technical architecture (709 lines)
- **[docs/WORLD_CONCEPT_DESIGN.md](docs/WORLD_CONCEPT_DESIGN.md)** - AI systems and world design
- **[docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md)** - Executive overview and roadmap
- **[docs/QUICK_START.md](docs/QUICK_START.md)** - Detailed setup guide
- **[docs/INDEX.md](docs/INDEX.md)** - Complete documentation index

## 🏗️ Architecture Overview

The system uses a 5-layer architecture:

1. **rAthena Game Server** - Core MMORPG server (C++)
2. **Bridge Layer** - REST API extension to rAthena web server
3. **AI Service Layer** - FastAPI application with CrewAI agents
4. **State Management** - DragonflyDB for shared state
5. **LLM Provider Layer** - Abstraction for multiple LLM providers

## 🔧 Integration with rAthena

The AI system is completely isolated from the rAthena core codebase:

- **Bridge Layer**: Extends the existing rAthena web server (`../src/web/web.cpp`)
- **NPC Scripts**: Custom AI-enabled NPCs in `../npc/custom/ai-world/`
- **No Core Modifications**: The system works as an extension, not a modification

## 🧪 Testing

Run tests with:
```bash
cd ai-service
source ../venv/bin/activate
pytest tests/
```

## 📊 Current Status

- ✅ Directory structure created
- ✅ Python virtual environment set up
- ✅ Core dependencies installed (minimal)
- ⏳ Bridge Layer implementation (pending)
- ⏳ AI Service skeleton (pending)
- ⏳ Example NPC integration (pending)

## 🔗 Related Directories

- **rAthena Source**: `../src/` - Core rAthena C++ source code
- **rAthena NPCs**: `../npc/` - NPC scripts (AI NPCs will go in `../npc/custom/ai-world/`)
- **rAthena Web Server**: `../src/web/web.cpp` - Where Bridge Layer endpoints will be added

## 📝 Notes

- The system is designed to be non-invasive and can be disabled without affecting rAthena
- All AI-related files are contained within this directory
- The virtual environment is self-contained and portable
- Configuration files use relative paths where possible

## 🆘 Support

For issues, questions, or contributions, refer to the documentation in the `docs/` directory.

