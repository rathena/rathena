/**
 * aiworld_server.cpp
 * Standalone AIWorld server process for rAthena (ZeroMQ IPC)
 * Runs independently, like char, login, and map servers.
 * Handles all AI logic, mission orchestration, and entity management.
 */

#include "aiworld_ipc.hpp"
#include "aiworld_utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <zmq.h>

using namespace aiworld;

class AIWorldServer {
public:
    AIWorldServer(const std::string& endpoint)
        : zmq_context(nullptr), zmq_socket(nullptr), running(false), server_endpoint(endpoint) {}

    ~AIWorldServer() {
        stop();
    }

    bool start() {
        zmq_context = zmq_ctx_new();
        if (!zmq_context) {
            log_error("AIWorldServer: ZeroMQ context creation failed.");
            return false;
        }
        zmq_socket = zmq_socket(zmq_context, ZMQ_REP);
        if (!zmq_socket) {
            log_error("AIWorldServer: ZeroMQ socket creation failed.");
            return false;
        }
        if (zmq_bind(zmq_socket, server_endpoint.c_str()) != 0) {
            log_error("AIWorldServer: ZeroMQ bind failed: " + std::string(zmq_strerror(zmq_errno())));
            return false;
        }
        running = true;
        log_info("AIWorldServer started and listening on " + server_endpoint);
        server_thread = std::thread(&AIWorldServer::main_loop, this);
        return true;
    }

    void stop() {
        running = false;
        if (server_thread.joinable()) server_thread.join();
        if (zmq_socket) zmq_close(zmq_socket);
        if (zmq_context) zmq_ctx_term(zmq_context);
        log_info("AIWorldServer stopped.");
    }

private:
    void* zmq_context;
    void* zmq_socket;
    std::atomic<bool> running;
    std::thread server_thread;
    std::string server_endpoint;

    void main_loop() {
        while (running) {
            zmq_msg_t zmq_msg;
            zmq_msg_init(&zmq_msg);
            int rc = zmq_msg_recv(&zmq_msg, zmq_socket, 0);
            if (rc < 0) {
                zmq_msg_close(&zmq_msg);
                continue;
            }
            std::string msg_str(static_cast<char*>(zmq_msg_data(&zmq_msg)), zmq_msg_size(&zmq_msg));
            zmq_msg_close(&zmq_msg);

            log_info("AIWorldServer received message: " + msg_str);

            // Parse message and dispatch
            nlohmann::json response_json;
            try {
                nlohmann::json req = nlohmann::json::parse(msg_str);
                int msg_type = req.value("message_type", 0);
                std::string corr_id = req.value("correlation_id", "");
                nlohmann::json payload = req.value("payload", nlohmann::json::object());

                // Example: echo back with status
                response_json["message_type"] = msg_type;
                response_json["correlation_id"] = corr_id;
                response_json["payload"] = {
                    {"status", "ok"},
                    {"echo", payload}
                };
            } catch (const std::exception& e) {
                response_json["message_type"] = 99;
                response_json["payload"] = {{"error", e.what()}};
            }

            std::string resp_str = response_json.dump();
            zmq_msg_t resp_msg;
            zmq_msg_init_size(&resp_msg, resp_str.size());
            std::memcpy(zmq_msg_data(&resp_msg), resp_str.data(), resp_str.size());
            zmq_msg_send(&resp_msg, zmq_socket, 0);
            zmq_msg_close(&resp_msg);
        }
    }
};

int main(int argc, char* argv[]) {
    std::string endpoint = "tcp://*:5555";
    if (argc > 1) {
        endpoint = argv[1];
    }
    AIWorldServer server(endpoint);
    if (!server.start()) {
        log_error("Failed to start AIWorldServer.");
        return 1;
    }
    // Wait for server to finish (Ctrl+C to exit)
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}