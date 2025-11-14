# AI-Autonomous-World rAthena Plugin High-Level Architecture

## Overview

This document describes the high-level architecture for the native C++ AI-Autonomous-World plugin/module for rAthena, to be implemented under `src/aiworld/`. The architecture is designed for high performance, deep integration, and extensibility, enabling advanced AI-driven world features, mission/entity management, and real-time event handling.

---

## Core Components

### 1. AIWorld Server (Native C++ Daemon)
- Runs as a dedicated process alongside map, char, account, and web servers.
- Handles all AI logic, mission orchestration, and entity management.
- Modular: supports plug-in AI modules (e.g., mission engine, dialogue, pathfinding).

### 2. IPC Layer
- High-performance inter-process communication (IPC) using ZeroMQ.
- Message-based protocol for:
  - Entity state sync (players, NPCs, world)
  - Mission/quest assignment and tracking
  - Real-time event notifications (combat, movement, dialogue)
  - AI action requests and responses
- Non-blocking, event-driven, with robust error handling and timeouts.

### 3. rAthena Plugin/Module (`src/aiworld/`)
- C++ plugin loaded by rAthena map-server (and optionally char-server).
- Registers new script commands and event hooks for AI/mission integration.
- Manages IPC client connection to AIWorld server.
- Handles all serialization/deserialization of entity and event data.
- Provides callback/event API for scripts (see implementation plan).

### 4. Script Engine Integration
- New script commands: `aiworld_assign_mission`, `aiworld_query_state`, etc.
- Async callback/event mechanism for mission assignment, completion, and feedback.
- Full access to player, NPC, and world state from scripts.

### 5. Logging & Monitoring
- Structured, configurable logging for all requests, responses, and errors.
- Audit trail for mission assignments, completions, and AI actions.
- Optional integration with external monitoring/alerting systems.

---

## Data Flow Diagram

```mermaid
graph TD
    Player/NPC -- Script Command --> rAthena Map Server
    rAthena Map Server -- IPC Message --> AIWorld Plugin/Module
    AIWorld Plugin/Module -- IPC Message --> AIWorld Server
    AIWorld Server -- AI Decision/Event --> AIWorld Plugin/Module
    AIWorld Plugin/Module -- Callback/Event --> rAthena Script Engine
    rAthena Script Engine -- State Update --> Player/NPC
```

---

## Error Handling & Thread Safety

- All IPC operations are non-blocking and have configurable timeouts.
- Errors are logged and surfaced to scripts via callback arguments.
- Thread safety is ensured via mutexes/locks in shared data structures and event queues.
- Plugin/module is safe for multi-threaded and high-concurrency deployments.

---

## Extensibility & Modularity

- AIWorld server supports hot-pluggable AI modules (e.g., new mission types, dialogue engines).
- IPC protocol is versioned and extensible for future features.
- Plugin/module can be extended with new script commands and event hooks as needed.

---

## Integration Points

- `src/aiworld/` for plugin/module code.
- rAthena script engine for new commands and event hooks.
- AIWorld server for all AI/mission logic and orchestration.
- Logging and monitoring systems for observability.
