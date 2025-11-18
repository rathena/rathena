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
    zmq_socket = ::zmq_socket(zmq_context, ZMQ_REQ);
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
    auto ts_start = std::chrono::high_resolution_clock::now();
    thread_local std::vector<uint8_t> buffer;
    thread_local std::vector<uint8_t> payload_buf;
    payload_buf.clear();
    auto ser_start = std::chrono::high_resolution_clock::now();
    msg.serialize_payload(payload_buf); // Custom binary serialization
    auto ser_end = std::chrono::high_resolution_clock::now();
    int32_t type = static_cast<int32_t>(msg.message_type);
    int32_t corr_len = msg.correlation_id.size();
    int32_t payload_size = payload_buf.size();
    size_t total_size = 12 + corr_len + payload_size;
    buffer.resize(total_size);
    std::memcpy(buffer.data(), &type, 4);
    std::memcpy(buffer.data() + 4, &corr_len, 4);
    std::memcpy(buffer.data() + 8, &payload_size, 4);
    std::memcpy(buffer.data() + 12, msg.correlation_id.data(), corr_len);
    std::memcpy(buffer.data() + 12 + corr_len, payload_buf.data(), payload_size);
    zmq_msg_t zmq_msg;
    zmq_msg_init_data(&zmq_msg, buffer.data(), buffer.size(), nullptr, nullptr); // zero-copy
    int rc = zmq_msg_send(&zmq_msg, zmq_socket, 0);
    zmq_msg_close(&zmq_msg);
    auto ts_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count();
    auto ser_duration = std::chrono::duration_cast<std::chrono::microseconds>(ser_end - ser_start).count();
    nlohmann::json perf = {
        {"event", "ipc_send"},
        {"correlation_id", msg.correlation_id},
        {"msg_type", static_cast<int>(msg.message_type)},
        {"msg_size", buffer.size()},
        {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
        {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
        {"duration_us", duration},
        {"serialization_us", ser_duration},
        {"result", rc >= 0 ? "success" : "failure"}
    };
    log_performance(perf);
    return rc >= 0;
}

bool AIWorldIPCClient::receive_message(AIWorldMessage& msg, bool blocking) {
    if (!connected) return false;
    auto ts_start = std::chrono::high_resolution_clock::now();
    zmq_msg_t zmq_msg;
    zmq_msg_init(&zmq_msg);
    int flags = blocking ? 0 : ZMQ_DONTWAIT;
    int rc = zmq_msg_recv(&zmq_msg, zmq_socket, flags);
    if (rc < 0) {
        zmq_msg_close(&zmq_msg);
        return false;
    }
    uint8_t* data = static_cast<uint8_t*>(zmq_msg_data(&zmq_msg));
    size_t size = zmq_msg_size(&zmq_msg);
    zmq_msg_close(&zmq_msg);
    if (size < 12) return false;
    int32_t type, corr_len, payload_size;
    std::memcpy(&type, data, 4);
    std::memcpy(&corr_len, data + 4, 4);
    std::memcpy(&payload_size, data + 8, 4);
    if (size < 12 + corr_len + payload_size) return false;
    msg.message_type = static_cast<IPCMessageType>(type);
    msg.correlation_id.assign(reinterpret_cast<char*>(data + 12), corr_len);
    thread_local std::vector<uint8_t> payload_buf;
    payload_buf.resize(payload_size);
    auto deser_start = std::chrono::high_resolution_clock::now();
    std::memcpy(payload_buf.data(), data + 12 + corr_len, payload_size);
    bool deser_ok = msg.deserialize_payload(payload_buf); // Custom binary deserialization
    auto deser_end = std::chrono::high_resolution_clock::now();
    auto ts_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count();
    auto deser_duration = std::chrono::duration_cast<std::chrono::microseconds>(deser_end - deser_start).count();
    nlohmann::json perf = {
        {"event", "ipc_recv"},
        {"correlation_id", msg.correlation_id},
        {"msg_type", static_cast<int>(msg.message_type)},
        {"msg_size", size},
        {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
        {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
        {"duration_us", duration},
        {"deserialization_us", deser_duration},
        {"result", deser_ok ? "success" : "failure"}
    };
    log_performance(perf);
    return deser_ok;
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