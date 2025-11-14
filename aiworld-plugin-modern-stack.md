# AI-Autonomous-World rAthena Plugin: Modern Cross-Platform Technology Stack & Documentation Update

## 1. Technology Stack Selection

### Core Language & Standards
- **C++20** (or latest supported by rAthena): Modern language features, improved concurrency, and cross-platform support.

### IPC & Networking
- **gRPC (C++):** High-performance, cross-platform, language-agnostic RPC framework with built-in protocol buffers, async support, and first-class Windows/Unix compatibility.
- **Protocol Buffers:** Efficient, versioned, and extensible serialization for all IPC messages.

### Build System
- **CMake (>=3.20):** Industry-standard, cross-platform build system with robust support for Windows (MSVC, MinGW, WSL) and Unix (GCC, Clang).
- **Conan:** Modern C++ package manager for dependency management (gRPC, protobuf, testing frameworks).

### Logging & Monitoring
- **spdlog:** Fast, header-only, cross-platform logging library.
- **Prometheus-cpp (optional):** Metrics and monitoring integration for production deployments.

### Testing
- **Catch2** or **GoogleTest:** Modern, cross-platform C++ unit/integration testing frameworks.

### Continuous Integration
- **GitHub Actions** or **GitLab CI:** Automated builds and tests for Windows and Unix targets.

---

## 2. Architecture & Integration Updates

### High-Level Architecture
- AIWorld server and plugin communicate via gRPC over Unix domain sockets (Linux/macOS) or Named Pipes (Windows).
- Protocol Buffers define all entity, mission, and event messages.
- Async, event-driven design for all IPC and script callbacks.
- Modular AI core with hot-pluggable modules.

### Integration Points
- `src/aiworld/` for all plugin code.
- CMake-based build integration with rAthena.
- Conanfile for dependency management.
- gRPC/protobuf code generation as part of the build.

---

## 3. Product Requirements & Implementation Plan (Key Updates)

- All code, build scripts, and dependencies must be fully cross-platform (Windows, Linux, macOS).
- CMake and Conan are required for all builds; no platform-specific hacks.
- gRPC and protobuf are used for all IPC, replacing any legacy HTTP or custom socket code.
- All logging uses spdlog, with log level configurable at runtime.
- All tests must pass on both Windows and Unix CI runners.
- Documentation must include platform-specific build, deployment, and troubleshooting instructions.

---

## 4. Build & Deployment Instructions

### Prerequisites
- CMake >= 3.20
- Conan >= 2.0
- C++20 compiler (GCC 11+, Clang 12+, MSVC 2019+)
- Python 3.x (for protobuf/gRPC codegen)
- Git

### Build Steps (Windows & Unix)
```sh
# Clone the repo and enter the directory
git clone <your-repo-url>
cd rathena-AI-world

# Install dependencies via Conan
conan install . --output-folder=build --build=missing

# Configure and build with CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target aiworld_plugin

# (Optional) Run tests
cmake --build build --target test
```

### Deployment
- Place the built plugin in the rAthena `plugins/` directory.
- Start the AIWorld server as a separate process (see architecture doc).
- Ensure both map-server and AIWorld server are running and can communicate via gRPC.
- Use provided script commands in rAthena scripts for AI/mission integration.

---

## 5. Maintenance & Extensibility

- All dependencies are managed via Conan for reproducible builds.
- gRPC/protobuf schemas are versioned and extensible.
- Plugin and AI server are modular, supporting future AI modules and features.
- Logging and monitoring are production-grade and cross-platform.

---

## 6. Documentation & Support

- All architecture, integration, requirements, and implementation docs are updated to reflect this stack.
- Platform-specific troubleshooting and FAQ sections are included.
- CI/CD pipelines ensure ongoing cross-platform compatibility.

---

## 7. References

- [gRPC C++ Documentation](https://grpc.io/docs/languages/cpp/)
- [Protocol Buffers](https://developers.google.com/protocol-buffers)
- [CMake](https://cmake.org/)
- [Conan](https://conan.io/)
- [spdlog](https://github.com/gabime/spdlog)
- [Catch2](https://github.com/catchorg/Catch2)
- [GoogleTest](https://github.com/google/googletest)
