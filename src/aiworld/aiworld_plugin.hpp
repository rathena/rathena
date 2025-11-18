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
    // Now supports world concept design: can handle commands for personality, memory, goals, etc.
    std::string handle_script_command(const std::string& command, const std::string& args);

    // IPC client for communication with AIWorld server
    std::unique_ptr<AIWorldIPCClient> ipc_client;

        // Plugin state
        bool is_initialized;
    
        // --- World Concept Design Integration ---
        // Expose methods for updating/reading NPC world model
        // (To be implemented in .cpp)
        bool update_npc_consciousness(const std::string& npc_id, const nlohmann::json& consciousness);
        nlohmann::json get_npc_consciousness(const std::string& npc_id);
        bool update_npc_memory(const std::string& npc_id, const nlohmann::json& memory);
        nlohmann::json get_npc_memory(const std::string& npc_id);
        bool update_npc_goals(const std::string& npc_id, const nlohmann::json& goals);
        nlohmann::json get_npc_goals(const std::string& npc_id);
        bool update_npc_emotion(const std::string& npc_id, const nlohmann::json& emotion);
        nlohmann::json get_npc_emotion(const std::string& npc_id);
        // etc. for all world concept fields
};

} // namespace aiworld