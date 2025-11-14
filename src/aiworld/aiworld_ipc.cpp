/**
 * aiworld_ipc.cpp
 * ZeroMQ IPC client implementation for rAthena AIWorld Plugin
 */

#include "aiworld_ipc.hpp"
#include "aiworld_utils.hpp"
#include <iostream>
#include <cstring>

namespace aiworld {

AIWorldIPCClient::AIWorldIPCClient(const std::string& endpoint)
    : zmq_context(nullptr), zmq_socket(nullptr), server_endpoint(endpoint), connected(false), running(false)
{}

AIWorldIPCClient::~AIWorldIPCClient() {
    stop_receive_thread();
    if (zmq_socket) zmq_close(zmq_socket);
    if (zmq_context) zmq_ctx_term(zmq_context);
}

bool AIWorldIPCClient::connect() {
    zmq_context = zmq_ctx_new();
    if (!zmq_context) {
        log_error("ZeroMQ context creation failed.");
        return false;
    }
    zmq_socket = zmq_socket(zmq_context, ZMQ_REQ);
    if (!zmq_socket) {
        log_error("ZeroMQ socket creation failed.");
        return false;
    }
    if (zmq_connect(zmq_socket, server_endpoint.c_str()) != 0) {
        log_error("ZeroMQ connect failed: " + std::string(zmq_strerror(zmq_errno())));
        return false;
    }
    connected = true;
    log_info("ZeroMQ IPC client connected to " + server_endpoint);
    return true;
}

bool AIWorldIPCClient::send_message(const AIWorldMessage& msg) {
    if (!connected) return false;
    std::string msg_str = msg.payload.dump();
    zmq_msg_t zmq_msg;
    zmq_msg_init_size(&zmq_msg, msg_str.size());
    std::memcpy(zmq_msg_data(&zmq_msg), msg_str.data(), msg_str.size());
    int rc = zmq_msg_send(&zmq_msg, zmq_socket, 0);
    zmq_msg_close(&zmq_msg);
    return rc >= 0;
}

bool AIWorldIPCClient::receive_message(AIWorldMessage& msg, bool blocking) {
    if (!connected) return false;
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);
    int flags = blocking ? 0 : ZMQ_DONTWAIT;
    int rc = zmq_msg_recv(&zmq_msg, zmq_socket, flags);
    if (rc < 0) {
        zmq_msg_close(&zmq_msg);
        return false;
    }
    std::string msg_str(static_cast<char*>(zmq_msg_data(&zmq_msg)), zmq_msg_size(&zmq_msg));
    zmq_msg_close(&zmq_msg);
    try {
        msg.payload = nlohmann::json::parse(msg_str);
        // Optionally parse message_type/correlation_id if present
        msg.message_type = msg.payload.value("message_type", 0);
        msg.correlation_id = msg.payload.value("correlation_id", "");
    } catch (...) {
        return false;
    }
    return true;
}

void AIWorldIPCClient::start_receive_thread() {
    if (running) return;
    running = true;
    receive_thread = std::thread(&AIWorldIPCClient::receive_loop, this);
}

void AIWorldIPCClient::stop_receive_thread() {
    if (!running) return;
    running = false;
    if (receive_thread.joinable()) receive_thread.join();
}

void AIWorldIPCClient::receive_loop() {
    while (running) {
        AIWorldMessage msg;
        if (receive_message(msg, false)) {
            // TODO: Dispatch event to plugin/event system
            log_info("Received async message from AIWorld server: " + msg.payload.dump());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool AIWorldIPCClient::is_connected() const {
    return connected;
}

} // namespace aiworld