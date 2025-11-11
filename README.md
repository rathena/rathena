# rAthena AI World - AI-Driven MMORPG Server with Autonomous NPCs

[![Production Ready](https://img.shields.io/badge/status-production%20ready-brightgreen.svg)](ai-autonomous-world/README.md)
[![AI Agents](https://img.shields.io/badge/AI%20agents-6%20specialized-blue.svg)](ai-autonomous-world/README.md#-current-status)
[![LLM Providers](https://img.shields.io/badge/LLM%20providers-5%20(Azure%2C%20OpenAI%2C%20Anthropic%2C%20Google%2C%20DeepSeek)-orange.svg)](ai-autonomous-world/ai-service/llm/)
![GitHub](https://img.shields.io/github/license/rathena/rathena.svg)

---

## Motivation

This project was born from a frustration with arbitrary moderation practices in the rAthena community. After being banned from the rAthena Discord server without solid justification—simply because a staff member decided they didn't like me—I realized the importance of creating an independent, community-driven alternative.

<div align="center">
  <img src="doc/rathena staff power abuse.png" width="700" alt="Evidence of rAthena staff power abuse - arbitrary ban without justification" />
  <br>
  <em><strong>Evidence of arbitrary moderation:</strong> Banned from rAthena Discord without solid justification</em>
</div>

<br>

**This fork represents:**
- **Freedom from arbitrary authority**: No single person should have unchecked power to silence contributors
- **Technical innovation**: Pushing rAthena beyond its traditional boundaries with cutting-edge AI integration
- **Community empowerment**: Building a space where ideas and code matter more than personal preferences
- **Open collaboration**: Welcoming all developers regardless of past conflicts with other communities

If you've ever felt silenced, dismissed, or unfairly treated by those in positions of power, this project is for you. Let's build something better together.

---

## Overview

rAthena AI World is an enhanced fork of rAthena MMORPG server that integrates multi-agent AI systems for autonomous NPC behavior, dynamic quest generation, and emergent gameplay. The system implements personality-driven NPCs using the Big Five personality model, AI-generated dialogue with long-term memory, and real-time economic simulation.

### Core Features

- **Personality-Driven NPCs**: Each NPC exhibits unique behavior based on the Big Five personality model (Openness, Conscientiousness, Extraversion, Agreeableness, Neuroticism) and nine moral alignments
- **AI-Generated Dialogue**: Adaptive conversations using LLM providers with persistent memory across sessions
- **Dynamic Quest System**: AI-generated quests with eight quest types and six difficulty levels, contextually tailored to player state and world events
- **Economic Simulation**: Supply and demand mechanics with realistic market fluctuations and emergent economic events
- **Faction System**: Dynamic reputation systems with seven faction types and eight reputation tiers
- **Autonomous World State**: NPCs make independent decisions and react to world events, creating emergent storylines

## ⚠️ EXPERIMENTAL FEATURES DISCLAIMER

**This project contains experimental AI features that are actively under development.**

While the core rAthena server and AI autonomous world system are production-ready, the following newly implemented features are **experimental** and should be considered **beta quality**:

- **NPC Social Intelligence & Information Sharing System** (NEW)
- **Configurable NPC Movement Boundaries** (NEW)

**What to expect during testing:**
- 🐛 **Bugs and edge cases** - These features have passed initial testing but may exhibit unexpected behavior in production scenarios
- 🔧 **Ongoing refinement** - Implementation details may change based on testing feedback and performance analysis
- 📊 **Performance variations** - System behavior may vary under different load conditions
- 🔄 **Breaking changes possible** - Configuration formats and APIs may evolve as we refine the implementation

**We encourage testing and feedback!** Please report any issues, unexpected behavior, or suggestions via [GitHub Issues](https://github.com/iskandarsulaili/rathena-AI-world/issues).

**For production deployments:** Consider thoroughly testing these features in a development environment before enabling them on live servers.

---

### Technical Architecture

The system consists of approximately 10,000 lines of production-grade Python and C++ code implementing:

- **6 Specialized AI Agents**: Dialogue, Decision, Memory, World, Quest, and Economy agents orchestrated via CrewAI framework
- **Long-term Memory Management**: Memori SDK integration with DragonflyDB fallback for persistent NPC memories and relationship tracking
- **Multi-Provider LLM Support**: OpenAI GPT-4, Anthropic Claude-3, and Google Gemini integration
- **Production-Grade Implementation**: Comprehensive error handling, verbose logging, async/await operations, and type-safe Pydantic models

---

## 📚 Quick Start & Deployment Guides

### For Server Administrators

**[Ubuntu Server 24.04 Deployment Guide](UBUNTU_SERVER_DEPLOYMENT_GUIDE.md)** - Complete step-by-step guide for deploying the entire rAthena AI World backend system on Ubuntu Server 24.04, including:
- PostgreSQL 17 with pgvector, TimescaleDB, and Apache AGE
- DragonflyDB installation and configuration
- rAthena core server setup
- AI autonomous world service deployment
- P2P coordinator service setup
- Verification, troubleshooting, and maintenance

**[Server Management Guide](SERVER_MANAGEMENT.md)** - Quick reference for managing your deployed server:
- Start/Stop/Restart commands for all services
- Status checking and monitoring
- Log viewing and troubleshooting
- Common scenarios and one-command solutions

### For Players/Clients

**[Windows WARP P2P Client Setup Guide](../WARP-p2p-client/WINDOWS_CLIENT_SETUP_GUIDE.md)** - Beginner-friendly guide for setting up the WARP P2P client on Windows to connect to the rAthena AI World server with P2P hosting capabilities.

---

## P2P Coordinator Service

**Location**: `p2p-coordinator/`
**Version**: 2.0.0
**Status**: ✅ Production-Ready (All 26 Security & Functionality Fixes Complete)

The P2P Coordinator Service is a FastAPI-based WebSocket signaling server that manages P2P session discovery, host selection, and WebRTC signaling for the WARP P2P Client.

### 🎉 New Features in Version 2.0.0

- **Redis State Management**: Persistent signaling state for horizontal scaling
- **Rate Limiting**: Token bucket algorithm with Redis backend (API: 100/60s, WebSocket: 1000/60s, Auth: 10/60s)
- **Session Health Monitoring**: Automatic monitoring and cleanup of inactive sessions every 30 seconds
- **NPC State Broadcasting**: Fetches NPC state from AI service every 5 seconds and broadcasts to active sessions
- **Prometheus Metrics**: Full metrics endpoint with text exposition format
- **Refresh Token Endpoint**: JWT refresh token flow with 7-day expiration
- **Custom Exception Handling**: Proper error handling with specific exception classes
- **Database Indexes**: Composite indexes for common query patterns
- **Security Enforcement**: Production startup validation - refuses to start with weak secrets (<32 characters)

### Key Features

- ✅ WebSocket signaling for WebRTC offer/answer/ICE candidate exchange
- ✅ Session discovery and joining with automatic host selection
- ✅ JWT authentication with refresh token support
- ✅ Rate limiting to protect against abuse
- ✅ Session health monitoring with auto-cleanup
- ✅ NPC state broadcasting to all active sessions
- ✅ Prometheus metrics for monitoring
- ✅ Redis state management for horizontal scaling
- ✅ PostgreSQL 17 with composite indexes
- ✅ DragonflyDB (Redis-compatible) for caching

### API Endpoints

- `POST /api/v1/auth/login` - Authenticate and get JWT tokens
- `POST /api/v1/auth/refresh` - Refresh JWT access token
- `GET /api/v1/hosts` - List available hosts
- `POST /api/v1/hosts` - Register a new host
- `GET /api/v1/zones` - List zones
- `GET /api/v1/sessions` - List active sessions
- `POST /api/v1/sessions` - Create a new session
- `WS /api/v1/signaling/ws` - WebSocket signaling endpoint
- `GET /api/v1/monitoring/metrics` - Prometheus metrics

### Documentation

- [P2P Coordinator Deployment Guide](p2p-coordinator/docs/DEPLOYMENT.md) - Complete deployment guide with all new features
- [API Documentation](p2p-coordinator/docs/API.md) - REST API reference
- [Architecture Documentation](p2p-coordinator/docs/ARCHITECTURE.md) - System architecture
- [Configuration Guide](p2p-coordinator/docs/CONFIGURATION.md) - Configuration reference

---

## AI Autonomous World System Components

### Multi-Agent AI System
- **Dialogue Agent**: Generates personality-driven conversations with emotional context based on NPC traits
- **Decision Agent**: Processes NPC action decisions using personality parameters and current world state
- **Memory Agent**: Manages long-term memory storage and retrieval, tracks relationship values (-100 to +100 scale)
- **World Agent**: Analyzes world state data and generates dynamic events
- **Quest Agent**: Creates procedural quests using LLM providers (8 quest types, 6 difficulty levels)
- **Economy Agent**: Simulates market dynamics with supply/demand mechanics

### NPC Personality System
- **Big Five Personality Model**: Implements Openness, Conscientiousness, Extraversion, Agreeableness, and Neuroticism traits
- **Moral Alignment System**: Nine alignment types from lawful good to chaotic evil
- **Behavioral Consistency**: NPCs maintain consistent behavior patterns across all interactions based on personality configuration
- **Emotional Response System**: Emotion generation matched to personality traits and situational context

### Dynamic Quest Generation
- **Quest Types**: Fetch, Kill, Escort, Delivery, Explore, Dialogue, Craft, Investigate
- **LLM-Generated Content**: Unique quest narratives generated by configured LLM provider
- **Contextual Generation**: Quest parameters based on NPC personality, player level, and current world events
- **Difficulty Scaling**: Six difficulty levels from trivial to epic

### Economic Simulation Engine
- **Supply and Demand Mechanics**: Price fluctuations calculated from market forces
- **Market Trend Analysis**: Rising, falling, stable, and volatile market condition states
- **Economic Event System**: Shortage, surplus, crisis, boom, and other economic events
- **Trade Analysis**: AI-driven market recommendations for player trading decisions

### Faction Reputation System
- **Faction Types**: Kingdom, Guild, Merchant, Religious, Military, Criminal, Neutral (7 types)
- **Reputation Tiers**: Hated, Hostile, Unfriendly, Neutral, Friendly, Honored, Revered, Exalted (8 tiers)
- **Dynamic Faction Relations**: Faction interactions, alliances, and conflicts based on world events
- **Reputation Effects**: Player standing modifies NPC behavior and quest availability

### Long-Term Memory Management
- **Persistent Memory Storage**: NPC memories retained across server sessions
- **Relationship Tracking System**: Interaction history affects NPC perception and behavior
- **Context-Aware Dialogue**: Historical conversation data influences future dialogue generation
- **Memori SDK Integration**: Advanced memory management with DragonflyDB fallback storage

### 🆕 NPC Social Intelligence & Information Sharing System

**Status**: ⚠️ Experimental (Beta Quality)

NPCs now intelligently decide what information to share with players based on trust, relationship level, and personality traits, creating more realistic and dynamic social interactions.

#### Information Sensitivity Levels

NPCs categorize information into four sensitivity levels, each requiring different relationship thresholds:

- **PUBLIC** (Threshold: 0) - General information available to anyone
- **PRIVATE** (Threshold: 5) - Personal information shared with acquaintances
- **SECRET** (Threshold: 8) - Sensitive information shared only with trusted friends
- **CONFIDENTIAL** (Threshold: 10) - Highly sensitive information shared only with closest allies

#### Personality-Based Sharing Modifiers

NPC personality traits dynamically adjust information sharing thresholds:

- **High Agreeableness** (>0.7): -1 threshold modifier (shares more easily, friendly and open)
- **Low Agreeableness** (<0.3): +1 threshold modifier (shares less easily, guarded and suspicious)
- **High Neuroticism** (>0.7): +1 threshold modifier (more cautious, anxious about sharing)
- **Low Neuroticism** (<0.3): -1 threshold modifier (less cautious, confident in sharing)
- **High Openness** (>0.7): -1 threshold modifier (more willing to share, curious and expressive)

**Example**: An NPC with high agreeableness (0.8), low neuroticism (0.2), and high openness (0.95) would have a -3 total adjustment, making them very open to sharing information even with players they've just met.

#### Relationship-Based Information Filtering

- NPCs evaluate player relationship level before sharing information
- Information is filtered in real-time during dialogue generation
- Players must build trust over time to access more sensitive information
- Different NPCs with different personalities share information at different rates

#### Information Sharing History

- All information sharing events are stored in OpenMemory SDK
- Tracks what specific information has been shared with each player
- Prevents repetitive information sharing
- Enables NPCs to reference past shared information in future conversations

#### Example Behavior

**Lyra the Explorer** (Friendly, Open Personality):
- Agreeableness: 0.80, Neuroticism: 0.20, Openness: 0.95
- Threshold Adjustment: -3 (very open to sharing)
- At relationship level 0: Shares PUBLIC information
- At relationship level 6: Shares PUBLIC, PRIVATE, and most SECRET information
- Response style: Warm, engaging, hints at deeper secrets

**Guard Thorne** (Cautious, Guarded Personality):
- Agreeableness: 0.25, Neuroticism: 0.85, Openness: 0.20
- Threshold Adjustment: +2 (very restrictive)
- At relationship level 0: Shares nothing (even PUBLIC requires relationship 2)
- At relationship level 1: Still shares nothing
- Response style: Professional, guarded, emphasizes discretion

### 🆕 Configurable NPC Movement Boundaries

**Status**: ⚠️ Experimental (Beta Quality)

NPCs can now be configured with different movement restrictions, allowing for more realistic and controlled autonomous behavior.

#### Movement Modes

- **Global** - NPCs can move freely across all maps without restrictions
- **Map-Restricted** - NPCs stay within their current map, cannot cross map boundaries
- **Radius-Restricted** - NPCs stay within a defined tile radius from their spawn point
- **Disabled** - NPCs remain stationary at their spawn location

#### Per-NPC Configuration

Each NPC can be individually configured with:
- **Movement Mode**: One of the four modes above
- **Movement Radius**: Tile radius for radius-restricted mode (0-100 tiles)
- **Spawn Point**: Reference point for radius calculations (map, x, y coordinates)

#### Boundary Validation

- All movement decisions are validated before execution
- NPCs attempting to move outside boundaries are blocked
- Detailed logging of boundary violations for debugging
- Graceful fallback to idle behavior when no valid movement position exists

#### DecisionAgent Integration

- Movement decisions respect configured boundaries
- Wander and exploration behaviors stay within allowed areas
- Automatic radius adjustment based on current distance from spawn
- Maximum 10 attempts to find valid position before falling back to idle

#### Example Configurations

**Guard Thorne** (Patrol Guard):
- Movement Mode: `radius_restricted`
- Movement Radius: `7 tiles`
- Spawn Point: `prontera (145, 175)`
- Behavior: Patrols a small area around the guard post

**Lyra the Explorer** (Stationary NPC):
- Movement Mode: `disabled`
- Movement Radius: `0 tiles`
- Spawn Point: `prontera (155, 185)`
- Behavior: Remains at fixed location for player interactions

#### Technical Implementation

- Boundary validation occurs at two levels: decision generation and movement execution
- Euclidean distance calculation for radius checks
- Infinite distance for cross-map movement in restricted modes
- Integration with existing NPCMovementManager and event-driven movement system

---

## 🔗 Related Projects

### WARP P2P Client

The **[WARP-p2p-client](https://github.com/iskandarsulaili/WARP-p2p-client)** is the C++ WebRTC client implementation that connects to the rathena-AI-world P2P coordinator service. It enables hybrid P2P architecture where players can host game zones while maintaining centralized AI NPCs and authentication.

**Version**: 2.0.0 - ✅ Production-Ready (All 26 Security & Functionality Fixes Complete)

### ⚠️ Important: P2P is Completely Optional

**The P2P system is entirely optional and can be disabled at any time:**
- When P2P is disabled or unavailable, the system **automatically falls back** to traditional server routing
- Players experience **no difference in gameplay** when P2P is disabled
- The fallback is **transparent** - no manual intervention required
- You can enable/disable P2P per zone or globally via configuration

**P2P provides benefits when enabled:**
- Reduced server load in high-traffic zones
- Lower latency for player-to-player interactions
- Distributed bandwidth usage

**But the game works perfectly without it** - P2P is a performance enhancement, not a requirement.

**Key Features**:
- ✅ WebRTC-based P2P connections for zone hosting
- ✅ Automatic host discovery and selection
- ✅ Secure communication with SSL certificate verification
- ✅ **Automatic graceful fallback** to main server when P2P unavailable
- ✅ Performance monitoring and host validation
- ✅ **NEW**: Packet hooking for transparent P2P routing
- ✅ **NEW**: JWT token refresh with 7-day expiration
- ✅ **NEW**: Rate limiting (token bucket algorithm)
- ✅ **NEW**: Session health monitoring with auto-cleanup
- ✅ **NEW**: NPC state broadcasting
- ✅ **NEW**: Prometheus metrics for monitoring

**Integration**: The WARP client connects to the P2P coordinator service (`rathena-AI-world/p2p-coordinator`) via WebSocket signaling at `/api/v1/signaling/ws`. See [P2P_INTEGRATION_ANALYSIS.md](../P2P_INTEGRATION_ANALYSIS.md) for detailed integration requirements.

**Architecture**: Hybrid P2P model where:
- **Centralized**: AI NPCs, authentication, anti-cheat, critical game logic (always active)
- **P2P**: Zone-based player interactions, reducing server load and latency (optional, with automatic fallback)

**Documentation**:
- [WARP P2P Client README](../WARP-p2p-client/README.md) - Overview and features
- [P2P DLL Deployment Guide](../WARP-p2p-client/P2P-DLL/DEPLOYMENT_GUIDE.md) - Complete deployment guide with all 26 fixes
- [P2P Coordinator Deployment Guide](p2p-coordinator/docs/DEPLOYMENT.md) - Coordinator service deployment

---

## 💬 Community & Support

### rAthena AI World Community

We maintain an independent, welcoming community free from arbitrary moderation:

- **GitHub Issues**: [Report bugs and request features](https://github.com/iskandarsulaili/rathena-AI-world/issues)
- **GitHub Discussions**: [Ask questions and share ideas](https://github.com/iskandarsulaili/rathena-AI-world/discussions)
- **Pull Requests**: Contributions are always welcome! See [How to Contribute](#6-how-to-contribute)

### Getting Help

- **AI System Issues**: Check [ai-autonomous-world/INSTALL.md](ai-autonomous-world/INSTALL.md) and [Troubleshooting](#4-troubleshooting)
- **Documentation**: Browse our comprehensive [documentation](#-documentation)
- **Quick Questions**: Use GitHub Discussions for community support

### Why We're Independent

This project was created to provide a space free from arbitrary authority and unfair moderation practices. We believe in:
- **Open collaboration** without fear of unjust bans
- **Merit-based contributions** where code and ideas matter most
- **Transparent governance** with clear, fair community guidelines
- **Inclusive environment** welcoming all developers regardless of past conflicts

Everyone is welcome here, especially those who have felt silenced or dismissed elsewhere.

---

## 📚 Documentation

### Core Documentation
- **[Architecture Overview](ai-autonomous-world/docs/ARCHITECTURE.md)** - System architecture and component interactions
- **[Configuration Guide](ai-autonomous-world/docs/CONFIGURATION.md)** - Complete configuration reference
- **[Free-Form Text Input](ai-autonomous-world/docs/FREE_FORM_TEXT_INPUT.md)** - Natural language player-NPC interactions
- **[GPU Acceleration](ai-autonomous-world/docs/GPU_ACCELERATION.md)** - GPU acceleration setup and optimization

### Feature Documentation
- **[NPC Movement System](ai-autonomous-world/README.md#-current-status)** - Autonomous NPC movement with pathfinding
- **[Memory System](ai-autonomous-world/README.md#-current-status)** - Hyper-personalized per-player memory with PostgreSQL and DragonflyDB
- **[LLM Providers](ai-autonomous-world/ai-service/llm/)** - Multi-provider LLM support (Azure OpenAI, OpenAI, Anthropic, Google, DeepSeek)
- **[GPU Acceleration](ai-autonomous-world/docs/GPU_ACCELERATION.md)** - 10-100x faster inference with GPU support

### Implementation Guides
- **[Installation Guide](ai-autonomous-world/INSTALL.md)** - Complete installation instructions
- **[Quick Start](ai-autonomous-world/README.md#-quick-start)** - Get started quickly
- **[Architecture Overview](ai-autonomous-world/docs/ARCHITECTURE.md)** - System architecture and design

### Quick Reference
- **[API Endpoints](ai-autonomous-world/docs/FREE_FORM_TEXT_INPUT.md#api-documentation)** - REST API reference
- **[Troubleshooting](ai-autonomous-world/docs/FREE_FORM_TEXT_INPUT.md#troubleshooting)** - Common issues and solutions
- **[NPC Scripts](npc/custom/ai-world/README.md)** - rAthena NPC script examples

---

## Quick Start Guide - AI System

### System Requirements

#### Minimum Requirements
- Python 3.12 or higher
- DragonflyDB (Redis-compatible) or Redis 7.0+
- LLM API Keys: At least one of OpenAI, Anthropic, or Google Gemini
- 8GB RAM minimum (16GB recommended for production)

#### Optional: GPU Acceleration
- **NVIDIA GPU**: GTX 1060 (6GB VRAM) or higher, CUDA 11.8+
- **Apple Silicon**: M1/M2/M3 with 16GB+ unified memory
- **AMD GPU**: RX 6000/7000 series with ROCm 5.4+
- **Performance**: 10-100x faster LLM inference, 5-20x faster vector search

### Manual Installation

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

### Documentation Resources

- [README.md](ai-autonomous-world/README.md) - Complete AI system overview
- [INSTALL.md](ai-autonomous-world/INSTALL.md) - Detailed installation instructions
- [ARCHITECTURE.md](ai-autonomous-world/docs/ARCHITECTURE.md) - System architecture details
- [Documentation Index](ai-autonomous-world/docs/INDEX.md) - Complete documentation index

### API Documentation

Interactive API documentation available at `http://localhost:8000/docs` when AI Service is running.

---

## Technical Specifications

- **Production Implementation**: Comprehensive error handling and verbose logging throughout codebase
- **Cloud-Optimized Deployment**: 3.5GB memory footprint without local LLM models
- **Horizontal Scaling**: Native deployment supports horizontal scaling
- **Asynchronous Operations**: Non-blocking async/await pattern implementation
- **Type Safety**: Pydantic models for all data structures and API contracts
- **Test Coverage**: Integration and unit tests included

---

## System Architecture

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

## About rAthena Base Project

rAthena is a collaborative software development project for creating a massively multiplayer online role playing game (MMORPG) server package. Written in C++, the program provides NPCs, warps, and modifications. The project is managed by volunteers worldwide with community QA and support. rAthena is a continuation of the eAthena project.

rAthena AI World extends the base rAthena server with an AI-driven autonomous world system for enhanced NPC behavior and emergent gameplay.

> **Note**: We maintain independence from the original rAthena community channels. For support with the AI World system, please use GitHub Issues or Discussions on this repository.

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

### Hardware Requirements
Hardware Type | Minimum | Recommended
------|------|------
CPU | 2 Cores | 4 Cores
RAM | 8 GB | 16 GB
Disk Space | 5 GB | 10 GB

### Software Requirements
Application | Version | Purpose
------|------|------
Python | 3.12+ | AI Service runtime
DragonflyDB | Latest | State management (Redis-compatible)

### LLM Provider API Keys
At least one LLM provider API key is required:
- OpenAI (GPT-4)
- Anthropic (Claude-3-Sonnet)
- Google (Gemini-Pro)

Optional:
- Memori SDK API key for enhanced memory management

### Python Dependencies
Python dependencies are managed via `requirements-cloud.txt` in the `ai-autonomous-world/ai-service/` directory. The system is cloud-optimized (3.5GB footprint) and does not require local LLM models.

---

## 2. rAthena Base Server Prerequisites
The base rAthena server requires specific tools and applications depending on the operating system.

### Hardware Requirements
Hardware Type | Minimum | Recommended
------|------|------
CPU | 1 Core | 2 Cores
RAM | 1 GB | 2 GB
Disk Space | 300 MB | 500 MB

### Operating System and Compiler
Operating System | Compiler
------|------
Linux  | [gcc-6 or newer](https://www.gnu.org/software/gcc/gcc-6/) / [Make](https://www.gnu.org/software/make/)
Windows | [MS Visual Studio 2017 or newer](https://www.visualstudio.com/downloads/)

### Required Software
Application | Name
------|------
Database | [MySQL 5 or newer](https://www.mysql.com/downloads/) / [MariaDB 5 or newer](https://downloads.mariadb.org/)
Git | [Windows](https://gitforwindows.org/) / [Linux](https://git-scm.com/download/linux)

### Optional Software
Application | Name
------|------
Database | [MySQL Workbench 5 or newer](http://www.mysql.com/downloads/workbench/)

---

## 3. Installation

### AI System Installation

Complete setup guide: [ai-autonomous-world/INSTALL.md](ai-autonomous-world/INSTALL.md)

Installation:
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

Note: The AI Bridge Layer is integrated into the rAthena web server. Compile with `--enable-webserver` flag.

---

## 4. Troubleshooting

### AI System Troubleshooting

AI Service startup issues:
```bash
# Check logs
tail -f ai-service/logs/ai-service.log
```

DragonflyDB connection issues:
```bash
# Test connection
redis-cli ping
# Check if running
ps aux | grep dragonfly
```

Bridge Layer compilation errors:
```bash
# Ensure web server is enabled
./configure --enable-webserver
make clean && make server
```

Complete troubleshooting guide: [ai-autonomous-world/INSTALL.md](ai-autonomous-world/INSTALL.md)

### rAthena Base Server Troubleshooting

For rAthena server startup issues, check console output for error messages. Most support issues can be resolved by examining error messages. Additional support available at the [wiki](https://github.com/rathena/rathena/wiki) or [forums](https://rathena.org/forum).

---

## 5. More Documentation

### AI System Documentation

- [README.md](ai-autonomous-world/README.md) - Complete project overview
- [INSTALL.md](ai-autonomous-world/INSTALL.md) - Comprehensive installation instructions
- [ARCHITECTURE.md](ai-autonomous-world/docs/ARCHITECTURE.md) - System architecture and design
- [Documentation Index](ai-autonomous-world/docs/INDEX.md) - Complete documentation index
- [Configuration Guide](ai-autonomous-world/docs/CONFIGURATION.md) - Configuration options
- API Documentation: `http://localhost:8000/docs` (when AI Service is running)

### rAthena Base Server Documentation
The `/doc/` directory contains help files and sample NPC scripts with detailed explanations of NPC script commands, atcommands (@), group permissions, item bonuses, and packet structures.

---

## 6. How to Contribute

### Contributing to AI System
Contributions to the AI Autonomous World System:
- New AI agents
- Agent behavior improvements
- Additional LLM providers
- Economic simulation enhancements
- Faction system extensions

Review the architecture documentation before submitting pull requests. Include comprehensive tests with all contributions.

### Contributing to rAthena Base
Contribution guidelines for the base rAthena project: [CONTRIBUTING.md](https://github.com/rathena/rathena/blob/master/.github/CONTRIBUTING.md)

---

## 7. License

### AI Autonomous World System
The AI enhancement system (located in `ai-autonomous-world/`) is licensed under [GNU General Public License v3.0](LICENSE).

### rAthena Base
Copyright (c) rAthena Development Team - Licensed under [GNU General Public License v3.0](https://github.com/rathena/rathena/blob/master/LICENSE)

---

## Credits

### AI Autonomous World System
- Architecture & Implementation: Multi-agent AI system with CrewAI orchestration
- Technologies: Python 3.12, FastAPI, CrewAI, Memori SDK, DragonflyDB, OpenAI, Anthropic, Google Gemini
- Codebase: Approximately 10,000 lines of production-grade Python and C++
- Status: Production-ready with comprehensive testing

### rAthena Base Project
- Original Project: [rAthena](https://github.com/rathena/rathena)
- Community: rAthena Development Team and contributors worldwide
- Foundation: Continuation of the eAthena project

---

## Getting Started

### Quick Start (10 Minutes)
See [QUICK_START.md](QUICK_START.md) for a rapid deployment guide.

### Production Deployment
See [UBUNTU_SERVER_DEPLOYMENT_GUIDE.md](UBUNTU_SERVER_DEPLOYMENT_GUIDE.md) for comprehensive production deployment instructions.

### Documentation

📚 **Core Documentation:**
- [QUICK_START.md](QUICK_START.md) - Quick start guide with service status checks and testing
- [UBUNTU_SERVER_DEPLOYMENT_GUIDE.md](UBUNTU_SERVER_DEPLOYMENT_GUIDE.md) - Complete production deployment guide
- [FINAL_VERIFICATION_REPORT.md](FINAL_VERIFICATION_REPORT.md) - Verification, completion status, and test results
- [SERVER_MANAGEMENT.md](SERVER_MANAGEMENT.md) - Server management and operations guide

📦 **Component Documentation:**
- [ai-autonomous-world/README.md](ai-autonomous-world/README.md) - AI autonomous world system overview
- [ai-autonomous-world/INSTALL.md](ai-autonomous-world/INSTALL.md) - AI system installation guide
- [p2p-coordinator/README.md](p2p-coordinator/README.md) - P2P coordinator service documentation

🔧 **Advanced Features:**
- [docs/ADVANCED_AUTONOMOUS_FEATURES.md](docs/ADVANCED_AUTONOMOUS_FEATURES.md) - Advanced autonomous features guide
- [ai-autonomous-world/docs/ARCHITECTURE.md](ai-autonomous-world/docs/ARCHITECTURE.md) - System architecture details
- [ai-autonomous-world/docs/CONFIGURATION.md](ai-autonomous-world/docs/CONFIGURATION.md) - Configuration reference
- [p2p-coordinator/docs/API.md](p2p-coordinator/docs/API.md) - P2P coordinator API documentation
