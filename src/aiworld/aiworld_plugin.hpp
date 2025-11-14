#pragma once
// aiworld_plugin.hpp
// rAthena AIWorld Plugin/Module (ZeroMQ IPC)
// Defines plugin interface and lifecycle for rAthena integration

#include <string>
#include <memory>
#include "aiworld_ipc.hpp"
#include "aiworld_messages.hpp"

namespace aiworld {

class AIWorldPlugin {
public:
    AIWorldPlugin();
    ~AIWorldPlugin();

    // Initialize plugin (called on map-server/char-server startup)
    bool initialize();

    // Shutdown plugin (called on server shutdown)
    void shutdown();

    // Main event loop integration (called every tick)
    void on_tick();

    // Handle incoming script command from rAthena script engine
    std::string handle_script_command(const std::string& command, const std::string& args);

    // IPC client for communication with AIWorld server
    std::unique_ptr<AIWorldIPCClient> ipc_client;

    // Plugin state
    bool is_initialized;
};

} // namespace aiworld