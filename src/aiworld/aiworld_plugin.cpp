/**
 * aiworld_plugin.cpp
 * rAthena AIWorld Plugin/Module (ZeroMQ IPC)
 * Implements plugin lifecycle, ZeroMQ IPC client, and integration with rAthena event loop and script engine.
 */

#include "aiworld_plugin.hpp"
#include "aiworld_utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

// rAthena script engine headers (assumed, adjust as needed)
extern "C" {
#include "script.h"
#include "map/pc.h"
#include "map/npc.h"
}

#include "aiworld_native_api.hpp"
#include "aiworld_native_commands.hpp"
using aiworld::AIWorldNativeAPI;
using aiworld::APIResult;
using aiworld::ErrorCode;

// HTTP Script Commands - External declarations
extern void aiworld_init_http_client(const std::string& base_url);
extern void aiworld_shutdown_http_client();
extern void aiworld_register_http_script_commands();

namespace aiworld {

AIWorldPlugin::AIWorldPlugin()
    : is_initialized(false)
{
    // Set the AIWorld server endpoint (could be loaded from config)
    std::string endpoint = "tcp://127.0.0.1:5555";
    ipc_client = std::make_unique<AIWorldIPCClient>(endpoint);
}

AIWorldPlugin::~AIWorldPlugin() {
    shutdown();
}

bool AIWorldPlugin::initialize() {
    if (is_initialized) return true;
    log_info("AIWorldPlugin: Initializing...");
    
    // Initialize HTTP client for AI service communication
    aiworld_init_http_client("http://127.0.0.1:8000");
    log_info("AIWorldPlugin: HTTP client initialized");
    
    // Register HTTP-based script commands (httppost, httpget, npcwalk, npcwalkid)
    aiworld_register_http_script_commands();
    log_info("AIWorldPlugin: HTTP script commands registered");
    
    // Native script commands registered via BUILDIN_DEF in script_def.inc
    log_info("AIWorldPlugin: Native script commands (ai_npc_*) registered via script_def.inc");
    
    // Initialize Native API singleton with ZeroMQ endpoint
    if (!AIWorldNativeAPI::getInstance().initialize("tcp://127.0.0.1:5555")) {
        log_warning("AIWorldPlugin: Native API initialization failed (non-fatal)");
    } else {
        log_info("AIWorldPlugin: Native API initialized successfully");
    }
    
    // Initialize ZeroMQ IPC (for backward compatibility with existing code)
    if (!ipc_client->connect()) {
        log_warning("AIWorldPlugin: ZeroMQ connection failed (non-fatal, HTTP integration active)");
        // Don't fail initialization - HTTP integration is primary now
    } else {
        ipc_client->start_receive_thread();
        log_info("AIWorldPlugin: ZeroMQ IPC connected (legacy support)");
    }
    
    is_initialized = true;
    log_info("AIWorldPlugin: Initialization complete - HTTP REST integration active");
    return true;
}

void AIWorldPlugin::shutdown() {
    if (!is_initialized) return;
    log_info("AIWorldPlugin: Shutting down...");
    
    // Shutdown HTTP client and log statistics
    aiworld_shutdown_http_client();
    log_info("AIWorldPlugin: HTTP client shutdown complete");
    
    // Shutdown ZeroMQ IPC
    ipc_client->stop_receive_thread();
    
    is_initialized = false;
    log_info("AIWorldPlugin: Shutdown complete");
}

void AIWorldPlugin::on_tick() {
    // Called every server tick; could poll for async events or process IPC messages
    // (In practice, event-driven receive thread handles most async IPC)
}

std::string AIWorldPlugin::handle_script_command(const std::string& command, const std::string& args) {
    // Parse and route plugin commands for rAthena script engine integration
    nlohmann::json payload;
    payload["command"] = command;
    payload["args"] = args;

    if (command == "aiworld_assign_mission") {
        // Parse args for mission assignment
        nlohmann::json mission_data;
        try {
            mission_data = nlohmann::json::parse(args);
        } catch (...) {
            log_error("AIWorldPlugin: Invalid JSON for mission assignment args: " + args);
            return "{\"error\": \"Invalid mission assignment args\"}";
        }
        payload["mission_data"] = mission_data;
        AIWorldMessage msg(IPCMessageType::MISSION_ASSIGNMENT, generate_correlation_id(), payload);
        if (!ipc_client->send_message(msg)) {
            log_error("AIWorldPlugin: Failed to send mission assignment to AIWorld server.");
            return "{\"error\": \"IPC send failed\"}";
        }
        // TODO: Integrate with rAthena script engine callback for mission assignment result
        return "{\"status\": \"mission assigned\"}";
    } else if (command == "aiworld_query_state") {
        // Query entity state
        AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
        if (!ipc_client->send_message(msg)) {
            log_error("AIWorldPlugin: Failed to send entity state query to AIWorld server.");
            return "{\"error\": \"IPC send failed\"}";
        }
        // TODO: Integrate with rAthena script engine callback for entity state result
        return "{\"status\": \"state query sent\"}";
    } else {
        // Default: route as AI action request
        AIWorldMessage msg(IPCMessageType::AI_ACTION_REQUEST, generate_correlation_id(), payload);
        if (!ipc_client->send_message(msg)) {
            log_error("AIWorldPlugin: Failed to send script command to AIWorld server. Command=" + command + ", Args=" + args);
            return "{\"error\": \"IPC send failed\"}";
        }
        // TODO: Integrate with rAthena script engine callback for generic AI action result
        return "{\"status\": \"command sent\"}";
    }
}

/**
 * --- World Concept Design Integration ---
 * Implementations for updating/reading NPC world model.
 * These methods use the ZeroMQ IPC to send/receive expanded consciousness, memory, goals, emotion, etc.
 */

bool AIWorldPlugin::update_npc_consciousness(const std::string& npc_id, const nlohmann::json& consciousness) {
    // Enforce EntityStateSync model for consciousness update
    EntityStateSync state;
    state.entity_id = npc_id;
    state.entity_type = "npc";
    state.consciousness = consciousness;
    nlohmann::json payload = to_json(state);
    AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
    bool result = ipc_client->send_message(msg);
    if (!result) {
        log_error("AIWorldPlugin: Failed to update consciousness for NPC " + npc_id);
    } else {
        log_info("AIWorldPlugin: Sent consciousness update for NPC " + npc_id);
    }
    return result;
}

nlohmann::json AIWorldPlugin::get_npc_consciousness(const std::string& npc_id) {
    // For now, just a stub. In production, would send a request and wait for response.
    // Could be extended to block/wait for reply from AIWorld server.
    return nlohmann::json{};
}

bool AIWorldPlugin::update_npc_memory(const std::string& npc_id, const nlohmann::json& memory) {
    // Enforce EntityStateSync model for memory update
    EntityStateSync state;
    state.entity_id = npc_id;
    state.entity_type = "npc";
    state.memory = memory;
    nlohmann::json payload = to_json(state);
    AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
    bool result = ipc_client->send_message(msg);
    if (!result) {
        log_error("AIWorldPlugin: Failed to update memory for NPC " + npc_id);
    } else {
        log_info("AIWorldPlugin: Sent memory update for NPC " + npc_id);
    }
    return result;
}

nlohmann::json AIWorldPlugin::get_npc_memory(const std::string& npc_id) {
    return nlohmann::json{};
}

bool AIWorldPlugin::update_npc_goals(const std::string& npc_id, const nlohmann::json& goals) {
    nlohmann::json payload;
    payload["entity_id"] = npc_id;
    payload["goals"] = goals;
    AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
    bool result = ipc_client->send_message(msg);
    if (!result) {
        log_error("AIWorldPlugin: Failed to update goals for NPC " + npc_id);
    } else {
        log_info("AIWorldPlugin: Sent goals update for NPC " + npc_id);
    }
    return result;
}

nlohmann::json AIWorldPlugin::get_npc_goals(const std::string& npc_id) {
    return nlohmann::json{};
}

bool AIWorldPlugin::update_npc_emotion(const std::string& npc_id, const nlohmann::json& emotion) {
    nlohmann::json payload;
    payload["entity_id"] = npc_id;
    payload["emotional_state"] = emotion;
    AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
    bool result = ipc_client->send_message(msg);
    if (!result) {
        log_error("AIWorldPlugin: Failed to update emotion for NPC " + npc_id);
    } else {
        log_info("AIWorldPlugin: Sent emotion update for NPC " + npc_id);
    }
    return result;
}

nlohmann::json AIWorldPlugin::get_npc_emotion(const std::string& npc_id) {
    return nlohmann::json{};
}

} // namespace aiworld

// --------------------
// Script Command Bindings for rAthena
// --------------------

/**
 * @brief Registers a new NPC with the AI system.
 * Script usage: ai_npc_register(npc_name$, personality_json$, initial_goals_json$)
 * @param npc_name$ (string) - Unique NPC identifier
 * @param personality_json$ (string) - JSON string for personality traits
 * @param initial_goals_json$ (string) - JSON string for initial goals
 * @return int (0=success, <0=error)
 */
BUILDIN(ai_npc_register) {
    if (script_isstring(st, 2) == 0 || script_isstring(st, 3) == 0 || script_isstring(st, 4) == 0) {
        script_pushint(st, -1);
        ShowError("ai_npc_register: All parameters must be strings (npc_name, personality_json, initial_goals_json)\n");
        return SCRIPT_CMD_SUCCESS;
    }
    const char* npc_name = script_getstr(st, 2);
    const char* personality_json = script_getstr(st, 3);
    const char* initial_goals_json = script_getstr(st, 4);

    try {
        nlohmann::json npc_data;
        npc_data["personality"] = nlohmann::json::parse(personality_json);
        npc_data["initial_goals"] = nlohmann::json::parse(initial_goals_json);

        APIResult result = AIWorldNativeAPI::getInstance().registerNPC(npc_name, npc_data);
        if (!result.success) {
            script_pushint(st, -result.error_code);
            ShowError("ai_npc_register: %s\n", result.error_message.c_str());
        } else {
            script_pushint(st, 0);
        }
    } catch (const nlohmann::json::exception& e) {
        script_pushint(st, -102); // INVALID_JSON
        ShowError("ai_npc_register: JSON parse error: %s\n", e.what());
    } catch (const std::exception& e) {
        script_pushint(st, -500);
        ShowError("ai_npc_register: Exception: %s\n", e.what());
    }
    return SCRIPT_CMD_SUCCESS;
}

/**
 * @brief Processes an NPC event.
 * Script usage: ai_npc_event(npc_id$, event_type$, event_data_json$)
 * @param npc_id$ (string) - Target NPC identifier
 * @param event_type$ (string) - Event type
 * @param event_data_json$ (string) - JSON string for event data
 * @return int (0=success, <0=error)
 */
BUILDIN(ai_npc_event) {
    if (script_isstring(st, 2) == 0 || script_isstring(st, 3) == 0 || script_isstring(st, 4) == 0) {
        script_pushint(st, -1);
        ShowError("ai_npc_event: All parameters must be strings (npc_id, event_type, event_data_json)\n");
        return SCRIPT_CMD_SUCCESS;
    }
    const char* npc_id = script_getstr(st, 2);
    const char* event_type = script_getstr(st, 3);
    const char* event_data_json = script_getstr(st, 4);

    try {
        nlohmann::json event_data = nlohmann::json::parse(event_data_json);
        APIResult result = AIWorldNativeAPI::getInstance().processNPCEvent(event_type, event_data, "", npc_id);
        if (!result.success) {
            script_pushint(st, -result.error_code);
            ShowError("ai_npc_event: %s\n", result.error_message.c_str());
        } else {
            script_pushint(st, 0);
        }
    } catch (const nlohmann::json::exception& e) {
        script_pushint(st, -102);
        ShowError("ai_npc_event: JSON parse error: %s\n", e.what());
    } catch (const std::exception& e) {
        script_pushint(st, -500);
        ShowError("ai_npc_event: Exception: %s\n", e.what());
    }
    return SCRIPT_CMD_SUCCESS;
}

/**
 * @brief Handles an NPC interaction.
 * Script usage: ai_npc_interact(npc_id$, player_id$, interaction_type$, data_json$)
 * @param npc_id$ (string) - Target NPC identifier
 * @param player_id$ (string) - Player or initiator identifier
 * @param interaction_type$ (string) - Type of interaction
 * @param data_json$ (string) - JSON string for interaction data
 * @return int (0=success, <0=error)
 */
BUILDIN(ai_npc_interact) {
    if (script_isstring(st, 2) == 0 || script_isstring(st, 3) == 0 || script_isstring(st, 4) == 0 || script_isstring(st, 5) == 0) {
        script_pushint(st, -1);
        ShowError("ai_npc_interact: All parameters must be strings (npc_id, player_id, interaction_type, data_json)\n");
        return SCRIPT_CMD_SUCCESS;
    }
    const char* npc_id = script_getstr(st, 2);
    const char* player_id = script_getstr(st, 3);
    const char* interaction_type = script_getstr(st, 4);
    const char* data_json = script_getstr(st, 5);

    try {
        nlohmann::json interaction_data = nlohmann::json::parse(data_json);
        APIResult result = AIWorldNativeAPI::getInstance().handleNPCInteraction(npc_id, interaction_type, interaction_data, player_id);
        if (!result.success) {
            script_pushint(st, -result.error_code);
            ShowError("ai_npc_interact: %s\n", result.error_message.c_str());
        } else {
            script_pushint(st, 0);
        }
    } catch (const nlohmann::json::exception& e) {
        script_pushint(st, -102);
        ShowError("ai_npc_interact: JSON parse error: %s\n", e.what());
    } catch (const std::exception& e) {
        script_pushint(st, -500);
        ShowError("ai_npc_interact: Exception: %s\n", e.what());
    }
    return SCRIPT_CMD_SUCCESS;
}

/**
 * @brief Gets the state of an NPC.
 * Script usage: ai_npc_get_state(npc_id$)
 * @param npc_id$ (string) - Target NPC identifier
 * @return string (JSON) - NPC state as JSON string, or error JSON
 */
BUILDIN(ai_npc_get_state) {
    if (script_isstring(st, 2) == 0) {
        script_pushstrcopy(st, "{\"error\":\"npc_id must be a string\"}");
        ShowError("ai_npc_get_state: npc_id must be a string\n");
        return SCRIPT_CMD_SUCCESS;
    }
    const char* npc_id = script_getstr(st, 2);

    try {
        APIResult result = AIWorldNativeAPI::getInstance().getNPCState(npc_id);
        if (!result.success) {
            nlohmann::json err = {
                {"error_code", result.error_code},
                {"error_message", result.error_message}
            };
            script_pushstrcopy(st, err.dump().c_str());
            ShowError("ai_npc_get_state: %s\n", result.error_message.c_str());
        } else {
            script_pushstrcopy(st, result.data.dump().c_str());
        }
    } catch (const std::exception& e) {
        nlohmann::json err = {
            {"error_code", 500},
            {"error_message", e.what()}
        };
        script_pushstrcopy(st, err.dump().c_str());
        ShowError("ai_npc_get_state: Exception: %s\n", e.what());
    }
    return SCRIPT_CMD_SUCCESS;
}

// --------------------
// Script Command Registration
// --------------------
/**
 * @brief Registers all AIWorld script commands with the rAthena script engine.
 * Call this in plugin initialization.
 */
void aiworld_register_script_commands() {
    addScriptCommand("ai_npc_register", &buildin_ai_npc_register);
    addScriptCommand("ai_npc_event", &buildin_ai_npc_event);
    addScriptCommand("ai_npc_interact", &buildin_ai_npc_interact);
    addScriptCommand("ai_npc_get_state", &buildin_ai_npc_get_state);
}

// In plugin initialization (example, adjust as needed):
// aiworld_register_script_commands();
