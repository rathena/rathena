/**
 * aiworld_http_bridge.cpp
 * HTTP-to-ZeroMQ bridge for rAthena AIWorld
 * Exposes legacy HTTP endpoints, translates requests to ZeroMQ, and returns responses.
 * Ensures 100% backward compatibility for NPC scripts.
 */

#include "aiworld_ipc.hpp"
#include "aiworld_utils.hpp"
#include "aiworld_messages.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <unordered_map>

using namespace aiworld;

namespace {
    // Configurable parameters
    constexpr int REQUEST_TIMEOUT_MS = 3000;
    constexpr int MAX_RETRIES = 2;
    constexpr int HTTP_PORT = 8000;

    // Logging utility
    void log_http(const std::string& msg) {
        log_info("[HTTP-BRIDGE] " + msg);
    }

    // Thread-safe ZeroMQ client pool
    class IPCClientPool {
    public:
        IPCClientPool(const std::string& endpoint, size_t pool_size = 4)
            : endpoint_(endpoint), stop_flag_(false) {
            for (size_t i = 0; i < pool_size; ++i) {
                clients_.emplace_back(std::make_unique<AIWorldIPCClient>(endpoint));
                clients_.back()->connect();
            }
        }
        ~IPCClientPool() {
            stop_flag_ = true;
        }
        AIWorldIPCClient* acquire() {
            std::unique_lock<std::mutex> lock(mutex_);
            // Simple round-robin
            idx_ = (idx_ + 1) % clients_.size();
            return clients_[idx_].get();
        }
    private:
        std::string endpoint_;
        std::vector<std::unique_ptr<AIWorldIPCClient>> clients_;
        std::mutex mutex_;
        size_t idx_ = 0;
        std::atomic<bool> stop_flag_;
    };

    // HTTP-to-ZeroMQ bridge handler
    class HTTPBridgeServer {
    public:
        HTTPBridgeServer(const std::string& zmq_endpoint, int http_port)
            : ipc_pool_(zmq_endpoint, std::thread::hardware_concurrency()), http_port_(http_port) {}

        void start() {
            httplib::Server svr;

            // Health check
            svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
                nlohmann::json j = {
                    {"status", "healthy"},
                    {"service", "aiworld-http-bridge"},
                    {"version", "1.0.0"}
                };
                res.set_content(j.dump(), "application/json");
            });

            // NPC register
            svr.Post(R"(/ai/npc/register)", [this](const httplib::Request& req, httplib::Response& res) {
                handle_ipc_request("NPC_REGISTER", req, res);
            });

            // NPC event
            svr.Post(R"(/ai/npc/event)", [this](const httplib::Request& req, httplib::Response& res) {
                handle_ipc_request("NPC_EVENT", req, res);
            });

            // NPC interact
            svr.Post(R"(/ai/npc/.*?/interact)", [this](const httplib::Request& req, httplib::Response& res) {
                handle_ipc_request("NPC_INTERACT", req, res);
            });

            // NPC state query
            svr.Get(R"(/ai/npc/.*?/state)", [this](const httplib::Request& req, httplib::Response& res) {
                handle_ipc_request("NPC_STATE", req, res);
            });

            // Fallback for unknown endpoints
            svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
                nlohmann::json j = {
                    {"error", "Endpoint not found"},
                    {"path", req.path}
                };
                res.status = 404;
                res.set_content(j.dump(), "application/json");
            });

            log_http("Starting HTTP bridge on port " + std::to_string(http_port_));
            svr.listen("0.0.0.0", http_port_);
        }

    private:
        IPCClientPool ipc_pool_;
        int http_port_;

        void handle_ipc_request(const std::string& api_type, const httplib::Request& req, httplib::Response& res) {
            log_http("Received HTTP request: " + api_type + " " + req.path);

            nlohmann::json req_json;
            try {
                if (req.body.empty()) {
                    req_json = nlohmann::json::object();
                } else {
                    req_json = nlohmann::json::parse(req.body);
                }
            } catch (const std::exception& e) {
                log_http("JSON parse error: " + std::string(e.what()));
                res.status = 400;
                res.set_content(R"({"error":"Invalid JSON"})", "application/json");
                return;
            }

            // Map API type to IPC message type
            IPCMessageType msg_type = IPCMessageType::EVENT_NOTIFICATION;
            if (api_type == "NPC_REGISTER") msg_type = IPCMessageType::MISSION_ASSIGNMENT;
            else if (api_type == "NPC_EVENT") msg_type = IPCMessageType::EVENT_NOTIFICATION;
            else if (api_type == "NPC_INTERACT") msg_type = IPCMessageType::AIACTION;
            else if (api_type == "NPC_STATE") msg_type = IPCMessageType::ENTITY_STATE_SYNC;

            AIWorldMessage ipc_msg;
            ipc_msg.message_type = msg_type;
            ipc_msg.correlation_id = generate_correlation_id();
            ipc_msg.payload = req_json;

            int attempt = 0;
            bool success = false;
            AIWorldIPCClient* client = nullptr;
            AIWorldMessage resp_msg;
            std::string error_msg;

            while (attempt <= MAX_RETRIES && !success) {
                client = ipc_pool_.acquire();
                if (!client || !client->is_connected()) {
                    error_msg = "IPC client unavailable";
                    break;
                }
                if (!client->send_message(ipc_msg)) {
                    error_msg = "Failed to send IPC message";
                    ++attempt;
                    continue;
                }
                auto start = std::chrono::steady_clock::now();
                while (true) {
                    if (client->receive_message(resp_msg, true)) {
                        success = true;
                        break;
                    }
                    auto now = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > REQUEST_TIMEOUT_MS) {
                        error_msg = "IPC response timeout";
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                ++attempt;
            }

            if (!success) {
                log_http("IPC error: " + error_msg);
                res.status = 504;
                nlohmann::json j = {
                    {"error", error_msg},
                    {"correlation_id", ipc_msg.correlation_id}
                };
                res.set_content(j.dump(), "application/json");
                return;
            }

            // Success: return response
            log_http("IPC response: " + resp_msg.payload.dump());
            res.status = 200;
            nlohmann::json j = {
                {"status", "ok"},
                {"correlation_id", resp_msg.correlation_id},
                {"payload", resp_msg.payload}
            };
            res.set_content(j.dump(), "application/json");
        }
    };
}

int main(int argc, char* argv[]) {
    std::string zmq_endpoint = "tcp://localhost:5555";
    int http_port = HTTP_PORT;
    if (argc > 1) zmq_endpoint = argv[1];
    if (argc > 2) http_port = std::stoi(argv[2]);
    HTTPBridgeServer bridge(zmq_endpoint, http_port);
    bridge.start();
    return 0;
}