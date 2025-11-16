#pragma once
#include <memory>
#include <string>
#include <vector>

class RedisClient;
class SessionManager;
class AIServiceClient;

struct SignalMessage {
    std::string sessionId;
    std::string type; // offer, answer, candidate, etc.
    std::string payload;
};

class WebRTCSignaling {
public:
    WebRTCSignaling(std::shared_ptr<RedisClient> redis,
                    std::shared_ptr<SessionManager> sessionManager,
                    std::shared_ptr<AIServiceClient> aiServiceClient);
    void run();
    bool handleSignal(const SignalMessage& msg);
    std::vector<SignalMessage> getPendingSignals(const std::string& sessionId);

private:
    std::shared_ptr<RedisClient> redis_;
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<AIServiceClient> aiServiceClient_;
    void processWebSocket();
    void persistSignal(const SignalMessage& msg);
    void loadSignalsFromRedis();
};