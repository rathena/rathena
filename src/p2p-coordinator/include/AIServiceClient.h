#pragma once
#include <string>
#include <optional>
#include <mutex>

class AIServiceClient {
public:
    AIServiceClient();
    std::optional<std::string> getNpcState(const std::string& npcId);
    bool updateNpcState(const std::string& npcId, const std::string& state);

private:
    std::mutex mutex_;
    // Connection details, e.g., HTTP client, endpoint, etc.
    std::string endpoint_;
    void connect();
};