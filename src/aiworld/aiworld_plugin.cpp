/**
 * aiworld_plugin.cpp
 * rAthena AIWorld Plugin/Module (ZeroMQ IPC)
 * Implements plugin lifecycle, ZeroMQ IPC client, and integration with rAthena event loop and script engine.
 */

#include "aiworld_plugin.hpp"
#include "aiworld_utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

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
    if (!ipc_client->connect()) {
        log_error("AIWorldPlugin: Failed to connect to AIWorld server via ZeroMQ.");
        return false;
    }
    ipc_client->start_receive_thread();
    is_initialized = true;
    log_info("AIWorldPlugin: Initialized and connected to AIWorld server.");
    return true;
}

void AIWorldPlugin::shutdown() {
    if (!is_initialized) return;
    log_info("AIWorldPlugin: Shutting down...");
    ipc_client->stop_receive_thread();
    is_initialized = false;
    log_info("AIWorldPlugin: Shutdown complete.");
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