#pragma once
#include <string>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <future>
#include <unordered_map>
#include <atomic>
#include <nlohmann/json.hpp>
#include "aiworld_messages.hpp"
#include "aiworld_types.hpp"
#include "aiworld_errors.hpp"
#include "aiworld_ipc.hpp"

namespace aiworld {

struct APIResult {
    bool success;
    int error_code;
    std::string error_message;
    nlohmann::json data;
    std::string correlation_id;

    APIResult() : success(false), error_code(0) {}
    APIResult(bool s, int ec, const std::string& em, const nlohmann::json& d = {})
        : success(s), error_code(ec), error_message(em), data(d) {}
};

class AIWorldNativeAPI {
public:
    static AIWorldNativeAPI& getInstance();

    bool initialize(const std::string& zmq_endpoint = "tcp://127.0.0.1:5555");
    void shutdown();

    APIResult registerNPC(const std::string& npc_id,
                         const nlohmann::json& npc_data,
                         int timeout_ms = 3000);

    APIResult processNPCEvent(const std::string& event_type,
                             const nlohmann::json& event_data,
                             const std::string& source_id = "",
                             const std::string& target_id = "",
                             int timeout_ms = 3000);

    APIResult handleNPCInteraction(const std::string& npc_id,
                                  const std::string& interaction_type,
                                  const nlohmann::json& interaction_data,
                                  const std::string& initiator_id = "",
                                  int timeout_ms = 3000);

    APIResult getNPCState(const std::string& npc_id,
                         const std::vector<std::string>& fields = {},
                         int timeout_ms = 3000);

    bool isConnected() const;
    std::string getEndpoint() const;
    size_t getPendingRequests() const;

private:
    AIWorldNativeAPI();
    ~AIWorldNativeAPI();

    class Impl;
    std::unique_ptr<Impl> pimpl;
};

} // namespace aiworld