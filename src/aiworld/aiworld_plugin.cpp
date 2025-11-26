/**
 * aiworld_plugin.cpp
 * rAthena AIWorld Plugin/Module (ZeroMQ IPC)
 * Implements plugin lifecycle, ZeroMQ IPC client, and integration with rAthena event loop and script engine.
 */

#include "aiworld_plugin.hpp"
#include "aiworld_utils.hpp"
#include "aiworld_callbacks.hpp"
#include "aiworld_threadpool.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

// rAthena script engine headers
#include "../map/script.hpp"
#include "../map/pc.hpp"
#include "../map/npc.hpp"
#include "../common/showmsg.hpp"

#include "aiworld_native_api.hpp"
#include "aiworld_native_commands.hpp"
using aiworld::AIWorldNativeAPI;
using aiworld::APIResult;
using aiworld::ErrorCode;
using aiworld::CallbackRegistry;
using aiworld::ThreadPool;

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
    
    // Initialize thread pool for async operations
    ThreadPool::getInstance().initialize(4);
    log_info("AIWorldPlugin: Thread pool initialized with 4 workers");
    
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
        log_warn("AIWorldPlugin: Native API initialization failed (non-fatal)");
    } else {
        log_info("AIWorldPlugin: Native API initialized successfully");
    }
    
    // Register IPC callbacks for async messages
    CallbackRegistry& registry = CallbackRegistry::getInstance();
    
    // Mission Assignment Callback (MISSION_ASSIGNMENT = 2)
    registry.registerCallback(aiworld::IPCMessageType::MISSION_ASSIGNMENT,
        [](aiworld::IPCMessageType type, const nlohmann::json& payload) {
            try {
                std::string mission_id = payload.value("mission_id", "");
                std::string assignee_id = payload.value("assignee_id", "");
                nlohmann::json mission_data = payload.value("mission_data", nlohmann::json::object());
                
                log_info("Mission Assignment: ID=" + mission_id + ", Assignee=" + assignee_id);
                
                // Trigger NPC event: assignee_npc::OnMissionAssigned
                if (!assignee_id.empty()) {
                    std::string event_name = assignee_id + "::OnMissionAssigned";
                    npc_event_do(event_name.c_str());
                    log_info("Triggered NPC event: " + event_name);
                }
            } catch (const std::exception& e) {
                log_error("Mission Assignment Callback error: " + std::string(e.what()));
            }
        });
    
    // Entity State Sync Callback (ENTITY_STATE_SYNC = 1)
    registry.registerCallback(aiworld::IPCMessageType::ENTITY_STATE_SYNC,
        [](aiworld::IPCMessageType type, const nlohmann::json& payload) {
            try {
                std::string entity_id = payload.value("entity_id", "");
                std::string entity_type = payload.value("entity_type", "");
                
                log_info("State Query Response: Entity=" + entity_id + ", Type=" + entity_type);
                
                // Trigger NPC event: entity_id::OnStateQueryComplete
                if (!entity_id.empty()) {
                    std::string event_name = entity_id + "::OnStateQueryComplete";
                    npc_event_do(event_name.c_str());
                    log_info("Triggered NPC event: " + event_name);
                }
            } catch (const std::exception& e) {
                log_error("State Query Callback error: " + std::string(e.what()));
            }
        });
    
    // AI Action Response Callback (AI_ACTION_RESPONSE = 5)
    registry.registerCallback(aiworld::IPCMessageType::AI_ACTION_RESPONSE,
        [](aiworld::IPCMessageType type, const nlohmann::json& payload) {
            try {
                std::string entity_id = payload.value("entity_id", "");
                std::string action_type = payload.value("action_type", "");
                nlohmann::json result = payload.value("result", nlohmann::json::object());
                
                log_info("AI Action Response: Entity=" + entity_id + ", Action=" + action_type);
                
                // Trigger NPC event: entity_id::OnAIActionComplete
                if (!entity_id.empty()) {
                    std::string event_name = entity_id + "::OnAIActionComplete";
                    npc_event_do(event_name.c_str());
                    log_info("Triggered NPC event: " + event_name);
                }
            } catch (const std::exception& e) {
                log_error("AI Action Callback error: " + std::string(e.what()));
            }
        });
    
    log_info("AIWorldPlugin: IPC callbacks registered");
    
    // Initialize ZeroMQ IPC (for backward compatibility with existing code)
    if (!ipc_client->connect()) {
        log_warn("AIWorldPlugin: ZeroMQ connection failed (non-fatal, HTTP integration active)");
        // Don't fail initialization - HTTP integration is primary now
    } else {
        ipc_client->start_receive_thread();
        log_info("AIWorldPlugin: ZeroMQ IPC connected (legacy support)");
    }
    
    is_initialized = true;
    log_info("AIWorldPlugin: Initialization complete - HTTP REST + IPC callbacks active");
    return true;
}

void AIWorldPlugin::shutdown() {
    if (!is_initialized) return;
    log_info("AIWorldPlugin: Shutting down...");
    
    // Shutdown thread pool
    ThreadPool::getInstance().shutdown();
    log_info("AIWorldPlugin: Thread pool shutdown complete");
    
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
        // Callback will be triggered by IPC receive thread
        return "{\"status\": \"mission assigned\"}";
    } else if (command == "aiworld_query_state") {
        // Query entity state
        AIWorldMessage msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), payload);
        if (!ipc_client->send_message(msg)) {
            log_error("AIWorldPlugin: Failed to send entity state query to AIWorld server.");
            return "{\"error\": \"IPC send failed\"}";
        }
        // Callback will be triggered by IPC receive thread
        return "{\"status\": \"state query sent\"}";
    } else {
        // Default: route as AI action request
        AIWorldMessage msg(IPCMessageType::AI_ACTION_REQUEST, generate_correlation_id(), payload);
        if (!ipc_client->send_message(msg)) {
            log_error("AIWorldPlugin: Failed to send script command to AIWorld server. Command=" + command + ", Args=" + args);
            return "{\"error\": \"IPC send failed\"}";
        }
        // Callback will be triggered by IPC receive thread
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
    state.personality = consciousness; // Store consciousness in personality field
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
    if (!is_initialized || !ipc_client || !ipc_client->is_connected()) {
        log_error("AIWorldPlugin::get_npc_consciousness: Plugin not initialized or IPC not connected");
        return nlohmann::json{{"error", "Plugin not initialized"}};
    }
    
    try {
        // Build query message
        nlohmann::json query_payload;
        query_payload["entity_id"] = npc_id;
        query_payload["query_type"] = "get_consciousness";
        
        AIWorldMessage query_msg(IPCMessageType::ENTITY_STATE_SYNC, generate_correlation_id(), query_payload);
        
        // Send query
        if (!ipc_client->send_message(query_msg)) {
            log_error("AIWorldPlugin::get_npc_consciousness: Failed to send IPC query for NPC " + npc_id);
            return nlohmann::json{{"error", "IPC send failed"}};
        }
        
        // Wait for response with 5s timeout
        AIWorldMessage response_msg;
        auto start_time = std::chrono::steady_clock::now();
        bool received = false;
        
        while (!received && std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
            if (ipc_client->receive_message(response_msg, false)) {
                if (response_msg.correlation_id == query_msg.correlation_id) {
                    received = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        if (!received) {
            log_warn("AIWorldPlugin::get_npc_consciousness: Timeout waiting for response for NPC " + npc_id);
            return nlohmann::json{{"error", "Timeout"}};
        }
        
        // Parse consciousness data from response
        if (response_msg.payload.contains("personality")) {
            nlohmann::json consciousness = {
                {"personality", response_msg.payload["personality"]},
                {"goals", response_msg.payload.value("goals", nlohmann::json::array())},
                {"emotions", response_msg.payload.value("emotional_state", nlohmann::json::object())}
            };
            return consciousness;
        }
        
        return response_msg.payload;
    } catch (const std::exception& e) {
        log_error("AIWorldPlugin::get_npc_consciousness: Exception: " + std::string(e.what()));
        return nlohmann::json{{"error", e.what()}};
    }
}

bool AIWorldPlugin::update_npc_memory(const std::string& npc_id, const nlohmann::json& memory) {
    // Enforce EntityStateSync model for memory update
    EntityStateSync state;
    state.entity_id = npc_id;
    state.entity_type = "npc";
    state.episodic_memory = memory; // Use episodic_memory field
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
int32 buildin_ai_npc_register(struct script_state* st) {
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3) || !script_hasdata(st, 4)) {
        script_pushint(st, -1);
        ShowError("ai_npc_register: Missing required parameters\n");
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
int32 buildin_ai_npc_event(struct script_state* st) {
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3) || !script_hasdata(st, 4)) {
        script_pushint(st, -1);
        ShowError("ai_npc_event: Missing required parameters\n");
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
int32 buildin_ai_npc_interact(struct script_state* st) {
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3) || !script_hasdata(st, 4) || !script_hasdata(st, 5)) {
        script_pushint(st, -1);
        ShowError("ai_npc_interact: Missing required parameters\n");
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
int32 buildin_ai_npc_get_state(struct script_state* st) {
    if (!script_hasdata(st, 2)) {
        script_pushstrcopy(st, "{\"error\":\"Missing npc_id parameter\"}");
        ShowError("ai_npc_get_state: Missing npc_id parameter\n");
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
// Script commands registered via BUILDIN_DEF in script_def.inc
// No manual registration needed
