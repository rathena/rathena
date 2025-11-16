#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

class HostRegistry;
class RedisClient;

enum class SessionState {
    Created,
    Active,
    Ended,
    Cleanup
};

struct SessionInfo {
    std::string sessionId;
    std::string hostId;
    std::string zoneId;
    SessionState state;
    // Additional session metadata as needed
};

class SessionManager {
public:
    SessionManager(std::shared_ptr<HostRegistry> hostRegistry, std::shared_ptr<RedisClient> redis);
    std::string createSession(const std::string& hostId, const std::string& zoneId);
    bool activateSession(const std::string& sessionId);
    bool endSession(const std::string& sessionId);
    bool cleanupSession(const std::string& sessionId);
    std::optional<SessionInfo> getSession(const std::string& sessionId);
    std::vector<SessionInfo> getSessionsByHost(const std::string& hostId);
    std::vector<SessionInfo> getSessionsByZone(const std::string& zoneId);

private:
    std::shared_ptr<HostRegistry> hostRegistry_;
    std::shared_ptr<RedisClient> redis_;
    std::unordered_map<std::string, SessionInfo> sessions_;
    std::mutex mutex_;
    void loadFromRedis();
    void persistToRedis(const SessionInfo& session);
};