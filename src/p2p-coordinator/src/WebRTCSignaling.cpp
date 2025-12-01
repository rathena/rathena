#include "WebRTCSignaling.h"
#include "RedisClient.h"
#include "SessionManager.h"
#include "AIServiceClient.h"
#include "Logger.h"
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

namespace {
    std::string signalToJson(const SignalMessage& msg) {
        json j;
        j["sessionId"] = msg.sessionId;
        j["type"] = msg.type;
        j["payload"] = msg.payload;
        return j.dump();
    }

    SignalMessage signalFromJson(const std::string& data) {
        auto j = json::parse(data);
        SignalMessage msg;
        msg.sessionId = j.value("sessionId", "");
        msg.type = j.value("type", "");
        msg.payload = j.value("payload", "");
        return msg;
    }
}

WebRTCSignaling::WebRTCSignaling(std::shared_ptr<RedisClient> redis,
                                 std::shared_ptr<SessionManager> sessionManager,
                                 std::shared_ptr<AIServiceClient> aiServiceClient)
    : redis_(std::move(redis)), sessionManager_(std::move(sessionManager)), aiServiceClient_(std::move(aiServiceClient)) {}

void WebRTCSignaling::run() {
    running_ = true;
    Logger::info("WebRTC signaling thread started.");
    processWebSocket();
}

void WebRTCSignaling::stop() {
    running_ = false;
    Logger::info("WebRTC signaling stopping...");
}

bool WebRTCSignaling::handleSignal(const SignalMessage& msg) {
    try {
        persistSignal(msg);
        Logger::info("Handled WebRTC signal for session: " + msg.sessionId + " type: " + msg.type);
        return true;
    } catch (const std::exception& ex) {
        Logger::error("Failed to handle WebRTC signal: " + std::string(ex.what()));
        return false;
    }
}

std::vector<SignalMessage> WebRTCSignaling::getPendingSignals(const std::string& sessionId) {
    std::vector<SignalMessage> signals;
    try {
        auto dataOpt = redis_->hget("signals", sessionId);
        if (dataOpt) {
            auto j = json::parse(*dataOpt);
            if (j.is_array()) {
                for (const auto& item : j) {
                    signals.push_back(signalFromJson(item.dump()));
                }
            }
        }
    } catch (const std::exception& ex) {
        Logger::error("Failed to get pending signals: " + std::string(ex.what()));
    }
    return signals;
}

#include "JwtAuth.h"
#include <memory>

void WebRTCSignaling::processWebSocket() {
    // Production: integrate with actual WebSocket server library (e.g., uWebSockets)
    // Accept connections, require JWT token, validate before accepting messages

    std::string jwt_secret = "REPLACE_WITH_SECURE_SECRET";
    std::unique_ptr<JwtAuth> jwtAuth = std::make_unique<JwtAuth>(jwt_secret);

    // Pseudocode for WebSocket server integration:
    // websocket_server.on_connect = [&](WebSocketConnection& conn) {
    //     std::string token = conn.get_query_param("token");
    //     std::string error;
    //     if (!jwtAuth->ValidateToken(token, error)) {
    //         Logger::warn("WebSocket authentication failed: " + error);
    //         conn.close();
    //         return;
    //     }
    //     Logger::info("WebSocket client authenticated.");
    //     conn.on_message = [&](const std::string& msg) {
    //         auto signal = signalFromJson(msg);
    //         handleSignal(signal);
    //     };
    // };

    // Placeholder event loop
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    Logger::info("WebRTC signaling thread stopped.");
}

void WebRTCSignaling::persistSignal(const SignalMessage& msg) {
    try {
        // Store signals as a JSON array per sessionId
        auto dataOpt = redis_->hget("signals", msg.sessionId);
        json arr = json::array();
        if (dataOpt) {
            arr = json::parse(*dataOpt);
        }
        arr.push_back(json::parse(signalToJson(msg)));
        redis_->hset("signals", msg.sessionId, arr.dump());
    } catch (const std::exception& ex) {
        Logger::error("Failed to persist signal: " + std::string(ex.what()));
        throw;
    }
}

void WebRTCSignaling::loadSignalsFromRedis() {
    // Optionally preload signals if needed
    // Not required for stateless API
}