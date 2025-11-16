# p2p-coordinator (C++)

This service is a native C++ implementation of the p2p-coordinator for the AI MMORPG world, replacing the original Python version. It is designed for high performance, reliability, and seamless integration with the P2P multi-CPU architecture.

## Features

- Host registry and management with quality scoring (Redis-backed)
- Session lifecycle management (create, activate, end, cleanup)
- Zone management and session-to-zone mapping
- WebRTC signaling via WebSocket with Redis-backed state
- REST API endpoints for all management operations
- Integration with external AI service for NPC state
- Background tasks for health monitoring and cleanup
- Comprehensive logging and error handling
- Modern C++20 codebase, modular and extensible
- Full backward compatibility with existing clients/services

## Directory Structure

- `src/` - Implementation files
- `include/` - Public headers for all modules
- `CMakeLists.txt` - Build configuration
- `README.md` - This documentation

## Build

```sh
cd rathena-AI-world/src/p2p-coordinator
mkdir build && cd build
cmake ..
make
```

## Run

```sh
./p2p_coordinator
```

## Dependencies

- C++20 compiler
- CMake >= 3.16
- Redis client library (e.g., hiredis)
- WebSocket and REST libraries (e.g., cpprestsdk, websocketpp, crow, or similar)
- JSON library (e.g., nlohmann/json)

## Configuration

Configuration is handled via environment variables or a config file (to be implemented).

## API

The service exposes both REST and WebSocket endpoints for management and signaling. See code for details.

## License

MIT (see project root)