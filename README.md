<img src="doc/logo.png" align="right" height="90" />

# rAthena AI World 🤖✨

**The World's First AI-Driven Autonomous MMORPG Server**

[![Production Ready](https://img.shields.io/badge/status-production%20ready-brightgreen.svg)](ai-autonomous-world/FINAL_SUMMARY.md)
[![AI Agents](https://img.shields.io/badge/AI%20agents-6%20specialized-blue.svg)](ai-autonomous-world/PHASE_2_COMPLETE.md)
[![LLM Providers](https://img.shields.io/badge/LLM%20providers-3%20(OpenAI%2C%20Anthropic%2C%20Google)-orange.svg)](ai-autonomous-world/ai-service/llm/)
![GitHub](https://img.shields.io/github/license/rathena/rathena.svg)

---

## 🌟 What Makes This Special?

**rAthena AI World** is an enhanced fork of rAthena that brings NPCs to life with cutting-edge AI technology. Imagine a Ragnarok Online world where:

- 🧠 **NPCs have personalities** - Each NPC exhibits unique behavior based on the Big Five personality model and moral alignment
- 💬 **Conversations feel real** - AI-generated dialogue that adapts to player interactions and remembers past conversations
- 🎯 **Quests are dynamic** - AI creates unique, contextual quests tailored to your character and the world state
- 💰 **Economy is alive** - Supply and demand drive realistic market fluctuations with emergent economic events
- 🏰 **Factions evolve** - Complex reputation systems and dynamic faction relationships that respond to player actions
- 🧩 **World feels autonomous** - NPCs make decisions, react to events, and create emergent storylines

### 🚀 Powered by Multi-Agent AI

This isn't just scripted behavior - it's a **production-grade AI system** with:

- **6 Specialized AI Agents** working in concert via CrewAI orchestration
- **Long-term Memory** using Memori SDK + DragonflyDB for persistent NPC memories
- **3 LLM Providers** (OpenAI GPT-4, Anthropic Claude-3, Google Gemini) for diverse AI capabilities
- **~10,000 lines** of production-ready Python and C++ code
- **Zero mocks** - all real, working implementations

---

## 🎮 AI Autonomous World Features

### 🤖 Multi-Agent AI System
- **Dialogue Agent**: Generates personality-driven conversations with emotional context
- **Decision Agent**: Makes intelligent NPC action decisions based on personality and world state
- **Memory Agent**: Stores and retrieves long-term memories, tracks relationships (-100 to +100)
- **World Agent**: Analyzes world state and generates dynamic events
- **Quest Agent**: Creates unique quests using AI (8 quest types, 6 difficulty levels)
- **Economy Agent**: Simulates realistic market dynamics with supply/demand mechanics

### 🎭 Personality-Driven NPCs
- **Big Five Personality Model**: Openness, Conscientiousness, Extraversion, Agreeableness, Neuroticism
- **9 Moral Alignments**: From lawful good to chaotic evil
- **Consistent Behavior**: NPCs act according to their personality traits across all interactions
- **Emotional Responses**: NPCs express emotions that match their personality and situation

### 🎯 Dynamic Quest Generation
- **8 Quest Types**: Fetch, Kill, Escort, Delivery, Explore, Dialogue, Craft, Investigate
- **AI-Generated Content**: Unique quest narratives created by LLMs
- **Contextual Quests**: Based on NPC personality, player level, and world events
- **6 Difficulty Levels**: From trivial to epic challenges

### 💰 Economic Simulation
- **Supply & Demand**: Realistic price fluctuations based on market forces
- **Market Trends**: Rising, falling, stable, and volatile market conditions
- **Economic Events**: Shortages, surpluses, crises, booms, and more
- **Trade Recommendations**: AI-driven market analysis for players

### 🏰 Faction System
- **7 Faction Types**: Kingdom, Guild, Merchant, Religious, Military, Criminal, Neutral
- **8 Reputation Tiers**: From Hated to Exalted
- **Dynamic Relationships**: Factions interact, ally, and conflict based on world events
- **Reputation Impact**: Your standing affects NPC behavior and quest availability

### 🧠 Long-Term Memory
- **Persistent Memories**: NPCs remember player interactions across sessions
- **Relationship Tracking**: Every interaction affects how NPCs perceive you
- **Context-Aware**: Past conversations influence future dialogue
- **Memori SDK Integration**: Advanced memory management with DragonflyDB fallback

---

## 🚀 Quick Start - AI System

### Prerequisites (AI System)
- **Python 3.12+**
- **DragonflyDB** (Redis-compatible) or Redis 7.0+
- **LLM API Keys**: OpenAI, Anthropic, or Google Gemini
- **8GB RAM** minimum (16GB recommended)

### Quick Deploy with Docker

```bash
cd ai-autonomous-world

# Configure environment
cp ai-service/.env.example ai-service/.env
# Edit .env with your API keys

# Start all services
docker-compose up -d

# Verify deployment
curl http://localhost:8000/health
```

### Manual Setup

```bash
cd ai-autonomous-world

# Activate virtual environment
source venv/bin/activate

# Start DragonflyDB
dragonfly --port 6379 &

# Start AI Service
cd ai-service
python main.py
```

### 📚 Complete Documentation

- **[FINAL_SUMMARY.md](ai-autonomous-world/FINAL_SUMMARY.md)** - Complete AI system overview
- **[DEPLOYMENT_GUIDE.md](ai-autonomous-world/DEPLOYMENT_GUIDE.md)** - Detailed deployment instructions
- **[ARCHITECTURE.md](ai-autonomous-world/docs/ARCHITECTURE.md)** - System architecture details
- **[PHASE_2_COMPLETE.md](ai-autonomous-world/PHASE_2_COMPLETE.md)** - Agent system documentation

### 🎯 API Documentation

Once running, visit `http://localhost:8000/docs` for interactive API documentation.

---

## 📊 Technical Highlights

- **Production-Grade**: No mocks, comprehensive error handling, verbose logging
- **Cloud-Optimized**: 3.5GB footprint (no local LLM models)
- **Scalable**: Horizontal scaling ready with Docker
- **Async/Await**: Non-blocking operations throughout
- **Type-Safe**: Pydantic models for all data structures
- **Well-Tested**: Integration and unit tests included

---

## 🏗️ Architecture

```
rAthena Game Server (C++)
         ↓
Bridge Layer (C++ HTTP Controller)
         ↓
AI Service Layer (Python/FastAPI)
    ├── Agent Orchestrator (CrewAI)
    │   ├── Dialogue Agent
    │   ├── Decision Agent
    │   ├── Memory Agent
    │   ├── World Agent
    │   ├── Quest Agent
    │   └── Economy Agent
    ├── API Routers
    └── LLM Providers
         ↓
State Management (DragonflyDB/Redis)
         ↓
LLM Provider Layer (OpenAI/Anthropic/Google)
```

---

## 🎓 About rAthena (Base Project)

> rAthena is a collaborative software development project revolving around the creation of a robust massively multiplayer online role playing game (MMORPG) server package. Written in C++, the program is very versatile and provides NPCs, warps and modifications. The project is jointly managed by a group of volunteers located around the world as well as a tremendous community providing QA and support. rAthena is a continuation of the eAthena project.

**rAthena AI World** builds upon this solid foundation by adding a complete AI-driven autonomous world system that makes NPCs feel truly alive.

### rAthena Resources

[Forum](https://rathena.org/board)|[Discord](https://rathena.org/discord)|[Wiki](https://github.com/rathena/rathena/wiki)|[FluxCP](https://github.com/rathena/FluxCP)|[Crowdfunding](https://rathena.org/board/crowdfunding/)|[Fork and Pull Request Q&A](https://rathena.org/board/topic/86913-pull-request-qa/)
--------|--------|--------|--------|--------|--------

---

### Table of Contents
1. [AI System Prerequisites](#1-ai-system-prerequisites)
2. [rAthena Prerequisites](#2-rathena-prerequisites)
3. [Installation](#3-installation)
4. [Troubleshooting](#4-troubleshooting)
5. [More Documentation](#5-more-documentation)
6. [How to Contribute](#6-how-to-contribute)
7. [License](#7-license)

---

## 1. AI System Prerequisites

The AI Autonomous World System requires additional components beyond the base rAthena server.

### AI System Hardware
Hardware Type | Minimum | Recommended
------|------|------
CPU | 2 Cores | 4 Cores
RAM | 8 GB | 16 GB
Disk Space | 5 GB | 10 GB

### AI System Software
Application | Version | Purpose
------|------|------
Python | 3.12+ | AI Service runtime
DragonflyDB | Latest | State management (Redis-compatible)
Docker | 20.10+ | Optional - for containerized deployment

### Required API Keys
You need at least one LLM provider API key:
- **OpenAI** (GPT-4) - Recommended for best results
- **Anthropic** (Claude-3-Sonnet) - Alternative/backup
- **Google** (Gemini-Pro) - Alternative/backup

Optional:
- **Memori SDK** API key - For enhanced memory management

### Python Dependencies
All Python dependencies are managed via `requirements-cloud.txt` in the `ai-autonomous-world/ai-service/` directory. The system is cloud-optimized (3.5GB) and does not require local LLM models.

---

## 2. rAthena Prerequisites
Before installing the base rAthena server, you need certain tools and applications which
differ between the varying operating systems available.

### rAthena Hardware
Hardware Type | Minimum | Recommended
------|------|------
CPU | 1 Core | 2 Cores
RAM | 1 GB | 2 GB
Disk Space | 300 MB | 500 MB

### Operating System & Preferred Compiler
Operating System | Compiler
------|------
Linux  | [gcc-6 or newer](https://www.gnu.org/software/gcc/gcc-6/) / [Make](https://www.gnu.org/software/make/)
Windows | [MS Visual Studio 2017 or newer](https://www.visualstudio.com/downloads/)

### Required Applications
Application | Name
------|------
Database | [MySQL 5 or newer](https://www.mysql.com/downloads/) / [MariaDB 5 or newer](https://downloads.mariadb.org/)
Git | [Windows](https://gitforwindows.org/) / [Linux](https://git-scm.com/download/linux)

### Optional Applications
Application | Name
------|------
Database | [MySQL Workbench 5 or newer](http://www.mysql.com/downloads/workbench/)

---

## 3. Installation

### AI System Installation

**Complete AI System Setup Guide**: See [ai-autonomous-world/DEPLOYMENT_GUIDE.md](ai-autonomous-world/DEPLOYMENT_GUIDE.md)

**Quick Docker Setup**:
```bash
cd ai-autonomous-world
cp ai-service/.env.example ai-service/.env
# Edit .env with your LLM API keys
docker-compose up -d
```

**Quick Manual Setup**:
```bash
cd ai-autonomous-world
source venv/bin/activate
dragonfly --port 6379 &
cd ai-service
python main.py
```

### rAthena Base Server Installation
  * [Windows](https://github.com/rathena/rathena/wiki/Install-on-Windows)
  * [CentOS](https://github.com/rathena/rathena/wiki/Install-on-Centos)
  * [Debian](https://github.com/rathena/rathena/wiki/Install-on-Debian)
  * [FreeBSD](https://github.com/rathena/rathena/wiki/Install-on-FreeBSD)

**Note**: The AI Bridge Layer is integrated into the rAthena web server. Compile with `--enable-webserver` flag.

---

## 4. Troubleshooting

### AI System Troubleshooting

**AI Service won't start**:
```bash
# Check logs
docker-compose logs ai-service
# Or for manual setup
tail -f ai-service/logs/ai-service.log
```

**DragonflyDB connection issues**:
```bash
# Test connection
redis-cli ping
# Check if running
ps aux | grep dragonfly
```

**Bridge Layer compilation errors**:
```bash
# Ensure web server is enabled
./configure --enable-webserver
make clean && make server
```

See [ai-autonomous-world/DEPLOYMENT_GUIDE.md](ai-autonomous-world/DEPLOYMENT_GUIDE.md) for complete troubleshooting guide.

### rAthena Troubleshooting

If you're having problems with starting your rAthena server, the first thing you should
do is check what's happening on your consoles. More often than not, all support issues
can be solved simply by looking at the error messages given. Check out the [wiki](https://github.com/rathena/rathena/wiki)
or [forums](https://rathena.org/forum) if you need more support on troubleshooting.

---

## 5. More Documentation

### AI System Documentation

- **[FINAL_SUMMARY.md](ai-autonomous-world/FINAL_SUMMARY.md)** - Complete project overview and statistics
- **[DEPLOYMENT_GUIDE.md](ai-autonomous-world/DEPLOYMENT_GUIDE.md)** - Comprehensive deployment instructions
- **[ARCHITECTURE.md](ai-autonomous-world/docs/ARCHITECTURE.md)** - System architecture and design
- **[PHASE_2_COMPLETE.md](ai-autonomous-world/PHASE_2_COMPLETE.md)** - Agent system details (~1,800 lines)
- **[TESTING_GUIDE.md](ai-autonomous-world/TESTING_GUIDE.md)** - Testing procedures
- **API Documentation**: `http://localhost:8000/docs` (when AI Service is running)

### rAthena Documentation
rAthena has a large collection of help files and sample NPC scripts located in the `/doc/`
directory. These include detailed explanations of NPC script commands, atcommands (@),
group permissions, item bonuses, and packet structures, among many other topics. We
recommend that all users take the time to look over this directory before asking for
assistance elsewhere.

---

## 6. How to Contribute

### Contributing to AI System
The AI Autonomous World System is a complete, production-ready implementation. If you'd like to:
- Add new AI agents
- Improve existing agent behavior
- Add new LLM providers
- Enhance the economic simulation
- Extend the faction system

Please review the architecture documentation first, then submit pull requests with comprehensive tests.

### Contributing to rAthena Base
Details on how to contribute to the base rAthena project can be found in [CONTRIBUTING.md](https://github.com/rathena/rathena/blob/master/.github/CONTRIBUTING.md)!

---

## 7. License

### AI Autonomous World System
The AI enhancement system (located in `ai-autonomous-world/`) is licensed under [GNU General Public License v3.0](LICENSE).

### rAthena Base
Copyright (c) rAthena Development Team - Licensed under [GNU General Public License v3.0](https://github.com/rathena/rathena/blob/master/LICENSE)

---

## 🌟 Credits

### AI Autonomous World System
- **Architecture & Implementation**: Complete multi-agent AI system with CrewAI orchestration
- **Technologies**: Python 3.12, FastAPI, CrewAI, Memori SDK, DragonflyDB, OpenAI, Anthropic, Google Gemini
- **Code**: ~10,000 lines of production-grade Python and C++
- **Status**: Production-ready, fully tested, deployment-ready

### rAthena Base Project
- **Original Project**: [rAthena](https://github.com/rathena/rathena)
- **Community**: rAthena Development Team and contributors worldwide
- **Foundation**: Continuation of the eAthena project

---

## 🚀 Get Started Now!

1. **Clone this repository**
2. **Set up the AI system**: Follow [ai-autonomous-world/DEPLOYMENT_GUIDE.md](ai-autonomous-world/DEPLOYMENT_GUIDE.md)
3. **Install rAthena base**: Follow standard rAthena installation guides above
4. **Configure your LLM API keys** in `ai-autonomous-world/ai-service/.env`
5. **Launch and experience** NPCs that feel truly alive!

**Questions?** Check the comprehensive documentation in `ai-autonomous-world/` or the rAthena [wiki](https://github.com/rathena/rathena/wiki).

---

**Welcome to the future of MMORPGs - where AI brings the world to life! 🎮✨**
