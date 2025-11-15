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
    log_info("Initializing AIWorldPlugin...");
    if (!ipc_client->connect()) {
        log_error("Failed to connect to AIWorld server via ZeroMQ.");
        return false;
    }
    ipc_client->start_receive_thread();
    is_initialized = true;
    log_info("AIWorldPlugin initialized and connected to AIWorld server.");
    return true;
}

void AIWorldPlugin::shutdown() {
    if (!is_initialized) return;
    log_info("Shutting down AIWorldPlugin...");
    ipc_client->stop_receive_thread();
    is_initialized = false;
}

void AIWorldPlugin::on_tick() {
    // Called every server tick; could poll for async events or process IPC messages
    // (In practice, event-driven receive thread handles most async IPC)
}

std::string AIWorldPlugin::handle_script_command(const std::string& command, const std::string& args) {
    // Example: handle aiworld_assign_mission, aiworld_query_state, etc.
    nlohmann::json payload;
    payload["command"] = command;
    payload["args"] = args;
    // AIWorldMessage now expects IPCMessageType (enum class) as first argument
    // No cast needed, constructor signature matches
    if (!ipc_client->send_message(msg)) {
        log_error("Failed to send script command to AIWorld server.");
        return "{\"error\": \"IPC send failed\"}";
    }

    // For synchronous commands, block and wait for response (not implemented here)
    // For async, return immediately
    return "{\"status\": \"command sent\"}";
}

} // namespace aiworld