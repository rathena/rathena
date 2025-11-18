#pragma once
// aiworld_ipc.hpp
// ZeroMQ IPC client/server for rAthena AIWorld Plugin

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <zmq.h>
#include "aiworld_types.hpp"
#include "aiworld_messages.hpp"

namespace aiworld {
// IPC message types are now defined in aiworld_types.hpp

// IPC client for rAthena <-> AIWorld server communication
class AIWorldIPCClient {
public:
    AIWorldIPCClient(const std::string& endpoint);
    ~AIWorldIPCClient();

    // Connect to AIWorld server
    bool connect();

    // Send message to AIWorld server (zero-copy, binary)
    bool send_message(const AIWorldMessage& msg);

    // Receive message from AIWorld server (zero-copy, binary)
    bool receive_message(AIWorldMessage& msg, bool blocking = false);

    // Start background receive thread for async event handling
    void start_receive_thread();

    // Stop background receive thread
    void stop_receive_thread();

    // Connection state
    bool is_connected() const;

private:
    void* zmq_context;
    void* zmq_socket;
    std::string server_endpoint;
    std::atomic<bool> connected;
    std::thread receive_thread;
    std::atomic<bool> running;

    // Internal receive loop
    void receive_loop();
};

} // namespace aiworld