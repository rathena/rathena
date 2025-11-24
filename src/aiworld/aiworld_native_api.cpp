#include "aiworld_native_api.hpp"
#include "aiworld_ipc.hpp"
#include "aiworld_errors.hpp"
#include "aiworld_utils.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <sstream>

namespace aiworld {

// --- Impl class ---
class AIWorldNativeAPI::Impl {
public:
    Impl() : endpoint_("tcp://127.0.0.1:5555"), connected_(false), client_index_(0) {}

    bool initialize(const std::string& endpoint) {
        std::unique_lock lock(connection_mutex_);
        endpoint_ = endpoint;
        size_t pool_size = std::thread::hardware_concurrency();
        if (pool_size == 0) pool_size = 4;
        client_pool_.clear();
        for (size_t i = 0; i < pool_size; ++i) {
            auto client = std::make_unique<AIWorldIPCClient>(endpoint_);
            if (!client->connect()) {
                log_error("Failed to connect IPC client " + std::to_string(i));
                return false;
            }
            client_pool_.emplace_back(std::move(client));
        }
        connected_ = true;
        return true;
    }

    void shutdown() {
        std::unique_lock lock(connection_mutex_);
        client_pool_.clear();
        connected_ = false;
    }

    bool isConnected() const {
        std::shared_lock lock(connection_mutex_);
        return connected_;
    }

    std::string getEndpoint() const {
        std::shared_lock lock(connection_mutex_);
        return endpoint_;
    }

    size_t getPendingRequests() const {
        std::shared_lock lock(request_mutex_);
        return pending_requests_.size();
    }

    // --- Core API Methods ---
    APIResult registerNPC(const std::string& npc_id, const nlohmann::json& npc_data, int timeout_ms) {
        auto ts_start = std::chrono::steady_clock::now();
        nlohmann::json payload = npc_data;
        payload["npc_id"] = npc_id;
        std::string corr_id = generate_correlation_id();
        size_t input_size = payload.dump().size();
        APIResult result = sendRequest(IPCMessageType::MISSION_ASSIGNMENT, payload, corr_id, timeout_ms);
        auto ts_end = std::chrono::steady_clock::now();
        nlohmann::json perf = {
            {"function", "registerNPC"},
            {"correlation_id", corr_id},
            {"input_size", input_size},
            {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
            {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
            {"duration_us", std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count()},
            {"result", result.success ? "success" : "failure"},
            {"error_code", result.error_code},
            {"error_message", result.error_message}
        };
        log_performance(perf);
        return result;
    }

    APIResult processNPCEvent(const std::string& event_type, const nlohmann::json& event_data,
                              const std::string& source_id, const std::string& target_id, int timeout_ms) {
        auto ts_start = std::chrono::steady_clock::now();
        nlohmann::json payload;
        payload["event_type"] = event_type;
        payload["event_data"] = event_data;
        payload["source_id"] = source_id;
        payload["target_id"] = target_id;
        std::string corr_id = generate_correlation_id();
        size_t input_size = payload.dump().size();
        APIResult result = sendRequest(IPCMessageType::EVENT_NOTIFICATION, payload, corr_id, timeout_ms);
        auto ts_end = std::chrono::steady_clock::now();
        nlohmann::json perf = {
            {"function", "processNPCEvent"},
            {"correlation_id", corr_id},
            {"input_size", input_size},
            {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
            {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
            {"duration_us", std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count()},
            {"result", result.success ? "success" : "failure"},
            {"error_code", result.error_code},
            {"error_message", result.error_message}
        };
        log_performance(perf);
        return result;
    }

    APIResult handleNPCInteraction(const std::string& npc_id, const std::string& interaction_type,
                                   const nlohmann::json& interaction_data, const std::string& initiator_id, int timeout_ms) {
        auto ts_start = std::chrono::steady_clock::now();
        nlohmann::json payload;
        payload["npc_id"] = npc_id;
        payload["interaction_type"] = interaction_type;
        payload["interaction_data"] = interaction_data;
        payload["initiator_id"] = initiator_id;
        std::string corr_id = generate_correlation_id();
        size_t input_size = payload.dump().size();
        APIResult result = sendRequest(IPCMessageType::AIACTION, payload, corr_id, timeout_ms);
        auto ts_end = std::chrono::steady_clock::now();
        nlohmann::json perf = {
            {"function", "handleNPCInteraction"},
            {"correlation_id", corr_id},
            {"input_size", input_size},
            {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
            {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
            {"duration_us", std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count()},
            {"result", result.success ? "success" : "failure"},
            {"error_code", result.error_code},
            {"error_message", result.error_message}
        };
        log_performance(perf);
        return result;
    }

    APIResult getNPCState(const std::string& npc_id, const std::vector<std::string>& fields, int timeout_ms) {
        auto ts_start = std::chrono::steady_clock::now();
        nlohmann::json payload;
        payload["npc_id"] = npc_id;
        if (!fields.empty()) payload["fields"] = fields;
        std::string corr_id = generate_correlation_id();
        size_t input_size = payload.dump().size();
        APIResult result = sendRequest(IPCMessageType::ENTITY_STATE_SYNC, payload, corr_id, timeout_ms);
        auto ts_end = std::chrono::steady_clock::now();
        nlohmann::json perf = {
            {"function", "getNPCState"},
            {"correlation_id", corr_id},
            {"input_size", input_size},
            {"ts_start", std::chrono::duration_cast<std::chrono::microseconds>(ts_start.time_since_epoch()).count()},
            {"ts_end", std::chrono::duration_cast<std::chrono::microseconds>(ts_end.time_since_epoch()).count()},
            {"duration_us", std::chrono::duration_cast<std::chrono::microseconds>(ts_end - ts_start).count()},
            {"result", result.success ? "success" : "failure"},
            {"error_code", result.error_code},
            {"error_message", result.error_message}
        };
        log_performance(perf);
        return result;
    }

private:
    std::string endpoint_;
    bool connected_;
    mutable std::shared_mutex connection_mutex_;
    mutable std::shared_mutex request_mutex_;
    std::vector<std::unique_ptr<AIWorldIPCClient>> client_pool_;
    std::atomic<size_t> client_index_;
    std::unordered_map<std::string, std::promise<APIResult>> pending_requests_;

    AIWorldIPCClient* acquireClient() {
        std::shared_lock lock(connection_mutex_);
        if (client_pool_.empty()) return nullptr;
        size_t idx = client_index_.fetch_add(1) % client_pool_.size();
        return client_pool_[idx].get();
    }

    APIResult sendRequest(IPCMessageType type, const nlohmann::json& payload,
                          const std::string& corr_id, int timeout_ms) {
        AIWorldIPCClient* client = acquireClient();
        if (!client || !client->is_connected()) {
            return APIResult(false, static_cast<int>(ErrorCode::CONNECTION_FAILED),
                             errorCodeToString(ErrorCode::CONNECTION_FAILED));
        }
        AIWorldMessage msg(type, corr_id, payload);
        if (!client->send_message(msg)) {
            return APIResult(false, static_cast<int>(ErrorCode::INTERNAL_ERROR),
                             "Failed to send IPC message");
        }
        auto start = std::chrono::steady_clock::now();
        AIWorldMessage resp_msg;
        while (true) {
            if (client->receive_message(resp_msg, true)) {
                // Logging with correlation ID
                log_info("[NativeAPI] IPC response (" + resp_msg.correlation_id + "): " + resp_msg.payload.dump());
                bool success = resp_msg.payload.value("status", "") == "success" || resp_msg.payload.value("status", "") == "ok";
                int error_code = resp_msg.payload.value("error_code", 0);
                std::string error_message = resp_msg.payload.value("error_message", "");
                nlohmann::json data = resp_msg.payload.value("data", nlohmann::json::object());
                if (!success) {
                    if (error_code == 0) error_code = static_cast<int>(ErrorCode::INTERNAL_ERROR);
                    if (error_message.empty()) error_message = "Unknown error";
                }
                return APIResult(success, error_code, error_message, data);
            }
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeout_ms) {
                log_error("[NativeAPI] IPC response timeout (" + corr_id + ")");
                return APIResult(false, static_cast<int>(ErrorCode::TIMEOUT),
                                 errorCodeToString(ErrorCode::TIMEOUT));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

// --- Singleton Implementation ---
AIWorldNativeAPI& AIWorldNativeAPI::getInstance() {
    static AIWorldNativeAPI instance;
    return instance;
}

AIWorldNativeAPI::AIWorldNativeAPI() : pimpl(std::make_unique<Impl>()) {}
AIWorldNativeAPI::~AIWorldNativeAPI() = default;

bool AIWorldNativeAPI::initialize(const std::string& zmq_endpoint) {
    return pimpl->initialize(zmq_endpoint);
}
void AIWorldNativeAPI::shutdown() {
    pimpl->shutdown();
}
APIResult AIWorldNativeAPI::registerNPC(const std::string& npc_id, const nlohmann::json& npc_data, int timeout_ms) {
    return pimpl->registerNPC(npc_id, npc_data, timeout_ms);
}
APIResult AIWorldNativeAPI::processNPCEvent(const std::string& event_type, const nlohmann::json& event_data,
                                            const std::string& source_id, const std::string& target_id, int timeout_ms) {
    return pimpl->processNPCEvent(event_type, event_data, source_id, target_id, timeout_ms);
}
APIResult AIWorldNativeAPI::handleNPCInteraction(const std::string& npc_id, const std::string& interaction_type,
                                                 const nlohmann::json& interaction_data, const std::string& initiator_id, int timeout_ms) {
    return pimpl->handleNPCInteraction(npc_id, interaction_type, interaction_data, initiator_id, timeout_ms);
}
APIResult AIWorldNativeAPI::getNPCState(const std::string& npc_id, const std::vector<std::string>& fields, int timeout_ms) {
    return pimpl->getNPCState(npc_id, fields, timeout_ms);
}
bool AIWorldNativeAPI::isConnected() const {
    return pimpl->isConnected();
}
std::string AIWorldNativeAPI::getEndpoint() const {
    return pimpl->getEndpoint();
}
size_t AIWorldNativeAPI::getPendingRequests() const {
    return pimpl->getPendingRequests();
}

} // namespace aiworld