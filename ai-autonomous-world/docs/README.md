# AI-Driven Autonomous World System for rAthena

## Overview

This project transforms the rAthena MMORPG emulator into a **living, breathing world** with AI-driven NPCs and adaptive systems that simulate real-world complexity and emergent behavior. Unlike traditional MMORPGs with scripted NPCs and predetermined events, this system creates a dynamic world where:

- **NPCs have individual consciousness** with unique personalities, memories, goals, and emotions
- **The world evolves autonomously** through economic, political, environmental, and social systems
- **Emergent behavior** creates unpredictable but coherent narratives
- **Player actions have lasting impact** on a world that remembers and adapts

## Key Features

### 🧠 Autonomous NPC Consciousness
- Individual personalities based on psychological models (Big Five + custom traits)
- Dynamic moral alignment that evolves based on experiences
- Episodic, semantic, and procedural memory systems
- Goal-oriented behavior with hierarchy of needs
- Emotional responses and mood management
- Relationship formation and social dynamics

### 🌍 Adaptive World Systems
- **Dynamic Economy**: Supply/demand, price fluctuations, trade networks, economic cycles
- **Political Systems**: Factions, alliances, conflicts, territory control, leadership changes
- **Environmental Systems**: Weather, seasons, natural disasters, resource management
- **Quest Generation**: Dynamic quests based on actual world state and NPC needs
- **Cultural Evolution**: Beliefs, norms, art, and social movements that change over time

### 🔧 Technical Excellence
- **Non-invasive Architecture**: Minimal modifications to rAthena core
- **Multi-LLM Support**: Azure OpenAI (default), OpenAI, DeepSeek, Anthropic Claude, Google Gemini
- **Scalable Design**: Support for hundreds to thousands of autonomous NPCs
- **High Performance**: Caching, batching, and optimization strategies
- **Production Ready**: Native deployment with PostgreSQL 17 and DragonflyDB

## Architecture

### System Layers

1. **rAthena Game Server** - Existing MMORPG server with custom NPC scripts
2. **Bridge Layer** - REST API extension connecting rAthena to AI services
3. **AI Service Layer** - Python-based autonomous agent orchestration (CrewAI + Memori SDK)
4. **State Management** - DragonflyDB for shared state and caching
5. **LLM Providers** - Abstracted interface to multiple AI providers

### High-Level Flow

```
Player → rAthena → Bridge API → AI Service (CrewAI + Memori) → LLM Provider
                        ↓                    ↓
                  DragonflyDB ←──────────────┘
```

For detailed architecture, see [ARCHITECTURE.md](./ARCHITECTURE.md)

## World Concept

### NPC Consciousness Example

**Marcus the Merchant**
- **Personality**: Ambitious (high), Cunning (high), Empathy (low)
- **Current Goal**: Become wealthy and influential
- **Recent Memory**: "Player helped me when I was robbed - I owe them"
- **Emotional State**: Anxious (worried about competition)
- **Moral Alignment**: Neutral → Selfish (evolving)

**Autonomous Behavior:**
1. Notices wheat shortage due to drought
2. Decides to hoard wheat (driven by ambition and low empathy)
3. Sells at inflated prices (profit-seeking)
4. Other merchants form alliance against him (social consequence)
5. Reputation drops, customers avoid him (economic consequence)
6. Learns lesson, adjusts behavior (character development)

### Emergent World Events

**Example: The Goblin Peace Treaty**
- Drought affects both humans and goblins
- Increased goblin raids due to food scarcity
- Empathetic NPC proposes peace talks (individual initiative)
- Player helps facilitate negotiations (player agency)
- Treaty succeeds despite opposition from both sides
- Cultural fusion emerges over time (long-term evolution)

**No scripting required** - all emerged from NPC decisions and world systems!

For detailed world concept, see [WORLD_CONCEPT_DESIGN.md](./WORLD_CONCEPT_DESIGN.md)

## Technology Stack

### Core Technologies
- **rAthena**: C++ MMORPG server (minimal modifications planned)
- **Bridge Layer**: C++ REST API extension (planned, not yet implemented)
- **AI Service**: Python 3.11+ with FastAPI (✅ implemented)
- **Agent Framework**: CrewAI (multi-agent orchestration) (✅ implemented)
- **Memory SDK**: Memori SDK with PostgreSQL backend (✅ implemented)
- **State Management**: DragonflyDB (Redis-compatible, high-performance) (✅ implemented)
- **LLM Providers**: Azure OpenAI (default), OpenAI, DeepSeek, Anthropic Claude, Google Gemini (✅ implemented)

### Infrastructure
- **Deployment**: Native installation (PostgreSQL 17, DragonflyDB)
- **Orchestration**: Kubernetes (production, ⏳ planned)
- **Monitoring**: Prometheus + Grafana (⏳ planned)
- **Logging**: ELK Stack (⏳ planned)
- **CI/CD**: GitHub Actions (⏳ planned)

## Getting Started

### Prerequisites
- rAthena server (located at `/home/lot399/ai-mmorpg-world/rathena-AI-world/`)
- Python 3.11+
- PostgreSQL 17 (native installation)
- DragonflyDB (native installation)
- Azure OpenAI account (or other LLM provider)

### Installation

1. **Install PostgreSQL 17**
```bash
# Ubuntu/Debian
sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh -y
sudo apt install -y postgresql-17 postgresql-contrib-17

# Create database and user
sudo -u postgres psql -c "CREATE DATABASE ai_world_memory;"
sudo -u postgres psql -c "CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;"
```

2. **Install DragonflyDB**
```bash
# Ubuntu/Debian
curl -fsSL https://www.dragonflydb.io/install.sh | bash
sudo systemctl start dragonfly
sudo systemctl enable dragonfly
```

3. **Install Python Dependencies**
```bash
cd ai-autonomous-world/ai-service
pip install -r requirements.txt
```

4. **Configure LLM Provider**
Create a `.env` file in the `ai-service` directory:
```bash
cd ai-service
cp .env.example .env
# Edit .env with your API keys and database settings
```

Example configuration:
```bash
# PostgreSQL (persistent memory)
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=ai_world_memory
POSTGRES_USER=ai_world_user
POSTGRES_PASSWORD=ai_world_pass_2025

# DragonflyDB (caching)
REDIS_HOST=localhost
REDIS_PORT=6379

# LLM Provider
DEFAULT_LLM_PROVIDER=azure_openai
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com
AZURE_OPENAI_API_KEY=your-key
AZURE_OPENAI_DEPLOYMENT=gpt-4
```

4. **Start AI Service**
```bash
cd ai-service
python main.py
```

The service will start on `http://localhost:8000` by default.

**Note**: Bridge Layer integration with rAthena is not yet implemented. The AI service currently runs as a standalone FastAPI application.

### Development Status

**Phase 1: Foundation** ✅ COMPLETE
- [x] Set up development environment
- [x] Implement AI Service with FastAPI
- [x] Set up DragonflyDB integration
- [ ] Implement Bridge Layer basic structure (pending)

**Phase 2: Core Systems** ✅ COMPLETE
- [x] Implement LLM Provider abstraction (OpenAI, Azure OpenAI, DeepSeek, Anthropic, Google)
- [x] Integrate CrewAI for agent orchestration
- [x] Implement basic NPC consciousness model
- [x] Implement AI agents (Dialogue, Decision, Memory, World, Quest, Economy)
- [ ] Integrate Memori SDK for memory management (using DragonflyDB fallback)

**Phase 3: World Systems** 🚧 IN PROGRESS
- [x] Implement economy models
- [x] Implement faction models
- [x] Implement quest generation system
- [ ] Implement politics/faction system (models only)
- [ ] Implement environment system (pending)

**Phase 4: Integration & Testing** 🚧 IN PROGRESS
- [x] Unit tests for core components
- [x] Integration tests
- [ ] End-to-end integration with rAthena (pending Bridge Layer)
- [ ] Performance testing and optimization
- [ ] Load testing with 100+ NPCs

**Phase 5: Deployment** ⏳ PENDING
- [ ] Production deployment setup
- [ ] Monitoring and alerting setup
- [ ] Documentation (in progress)
- [ ] Initial world bootstrap

## Performance Considerations

### Estimated Capacity
- **1000 NPCs**: 100 decisions/second
- **LLM Calls**: ~5 calls/second (with 50% cache hit + batching)
- **Latency**: ~500ms per decision (acceptable for MMORPG)
- **Infrastructure**: 66 CPU cores, 136GB RAM (recommended for 1000 NPCs)

### Optimization Strategies
- **Caching**: LLM responses, NPC decisions, world state
- **Batching**: Multiple NPC decisions in single LLM call
- **Tiering**: Cheap models for simple decisions, advanced models for complex ones
- **Event-Driven**: Decisions triggered by significant events, not just timers
- **Proximity-Based**: NPCs near players decide more frequently

## Documentation

- **[INDEX.md](./INDEX.md)** - Complete documentation index
- **[ARCHITECTURE.md](./ARCHITECTURE.md)** - Detailed technical architecture
- **[WORLD_CONCEPT_DESIGN.md](./WORLD_CONCEPT_DESIGN.md)** - World design and NPC consciousness model
- **[QUICK_START.md](./QUICK_START.md)** - Quick start guide
- **[CONFIGURATION.md](./CONFIGURATION.md)** - Configuration options
- **[FREE_FORM_TEXT_INPUT.md](./FREE_FORM_TEXT_INPUT.md)** - Free-form text input guide
- **[GPU_ACCELERATION.md](./GPU_ACCELERATION.md)** - GPU acceleration guide
- **[GPU_INSTALLATION.md](./GPU_INSTALLATION.md)** - GPU installation guide
- **API_REFERENCE.md** - API documentation (TBD)
- **DEPLOYMENT_GUIDE.md** - Production deployment guide (TBD)

## Contributing

This is a complex system that requires expertise in:
- C++ (rAthena modifications)
- Python (AI service development)
- AI/ML (LLM integration, agent design)
- Game design (world systems, NPC behavior)
- DevOps (deployment, monitoring)

Contributions are welcome! Please read the contributing guidelines (TBD).

## License

This project extends rAthena, which is licensed under GNU GPL. See [LICENSE](../rathena-AI-world/LICENSE) for details.

## Acknowledgments

- **rAthena Team** - For the excellent MMORPG emulator
- **CrewAI** - For the multi-agent orchestration framework
- **DragonflyDB** - For high-performance state management
- **OpenAI, Anthropic, Google** - For LLM APIs

## Contact

For questions, suggestions, or collaboration:
- Open an issue on GitHub
- Join our Discord (TBD)
- Email: (TBD)

---

**Status**: Core Implementation Phase (AI Service functional, Bridge Layer pending)
**Version**: 1.0
**Last Updated**: 2025-11-06
