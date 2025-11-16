#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>

class SessionManager;
class RedisClient;

struct ZoneInfo {
    std::string zoneId;
    std::string name;
    std::vector<std::string> sessionIds;
    // Additional zone metadata as needed
};

class ZoneManager {
public:
    ZoneManager(std::shared_ptr<SessionManager> sessionManager, std::shared_ptr<RedisClient> redis);
    bool addZone(const ZoneInfo& zone);
    bool removeZone(const std::string& zoneId);
    std::optional<ZoneInfo> getZone(const std::string& zoneId);
    std::vector<ZoneInfo> getAllZones();
    bool mapSessionToZone(const std::string& sessionId, const std::string& zoneId);
    bool unmapSessionFromZone(const std::string& sessionId, const std::string& zoneId);

private:
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<RedisClient> redis_;
    std::unordered_map<std::string, ZoneInfo> zones_;
    std::mutex mutex_;
    void loadFromRedis();
    void persistToRedis(const ZoneInfo& zone);
};