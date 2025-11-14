# rAthena AIWorld Plugin/Server (ZeroMQ IPC)

## Overview

This directory contains the native C++ AIWorld plugin/module and standalone server for rAthena, providing high-performance, in-process and out-of-process AI orchestration, mission/entity management, and real-time event handling. All inter-process communication (IPC) is handled exclusively via ZeroMQ.

- **Plugin/Module:** `aiworld_plugin` (shared library, loaded by map-server/char-server)
- **Standalone Server:** `aiworld_server` (independent process, like char/login/map servers)
- **IPC:** ZeroMQ (REQ/REP, PUB/SUB, or ROUTER/DEALER patterns as needed)
- **Message Format:** JSON (using nlohmann/json)
- **Location:** `src/aiworld/`

## Architecture

- The AIWorld plugin is loaded by rAthena map-server (and optionally char-server) and integrates with the event loop and script engine.
- The plugin communicates with the AIWorld server via ZeroMQ for all AI, mission, and event operations.
- The AIWorld server runs as a separate process, handling all AI logic, mission orchestration, and entity management.
- All message formats are defined in [`aiworld_messages.hpp`](aiworld_messages.hpp:1) and use JSON for extensibility.

## Key Files

- [`CMakeLists.txt`](CMakeLists.txt:1): Build configuration for plugin and server.
- [`aiworld_plugin.hpp`](aiworld_plugin.hpp:1): Plugin interface and lifecycle.
- [`aiworld_plugin.cpp`](aiworld_plugin.cpp:1): Plugin implementation.
- [`aiworld_ipc.hpp`](aiworld_ipc.hpp:1): ZeroMQ IPC client/server interface.
- [`aiworld_ipc.cpp`](aiworld_ipc.cpp:1): IPC implementation.
- [`aiworld_messages.hpp`](aiworld_messages.hpp:1): Message format definitions.
- [`aiworld_utils.hpp`](aiworld_utils.hpp:1), [`aiworld_utils.cpp`](aiworld_utils.cpp:1): Utility functions.
- [`aiworld_server.cpp`](aiworld_server.cpp:1): Standalone AIWorld server process.

## Usage

- **Build:** Use CMake to build both the plugin and the standalone server.
- **Run:** Start `aiworld_server` as a separate process (like char, login, map servers). The plugin will connect to it via ZeroMQ.
- **Integration:** The plugin exposes script commands (e.g., `aiworld_assign_mission`, `aiworld_query_state`) and handles all AI/mission/event logic via IPC.

## Extensibility

- All message formats are JSON and versioned for future expansion.
- The server and plugin are modular and can be extended with new AI modules, mission types, and event handlers.
- All logging is verbose and can be redirected to rAthena's logging system.

## Requirements

- C++20 or later
- ZeroMQ (libzmq, czmq)
- nlohmann/json

## Status

- [x] Architecture and IPC scaffolding complete
- [ ] Core AI logic, mission engine, and event router (to be implemented)
- [ ] Full integration with rAthena script engine and event loop
- [ ] 100% test coverage and production-grade error handling
