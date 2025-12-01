#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <mutex>
#include <vector>

class RedisClient;

struct HostInfo {
    std::string id;
    std::string address;
    int port;
    double qualityScore;
    bool healthy;
    // Additional metadata as needed
};

class HostRegistry {
public:
    explicit HostRegistry(std::shared_ptr<RedisClient> redis);
    bool registerHost(const HostInfo& host);
    bool unregisterHost(const std::string& hostId);
    bool updateQualityScore(const std::string& hostId, double score);
    std::vector<HostInfo> getAllHosts();
    std::optional<HostInfo> getHost(const std::string& hostId);
    void markHostHealthy(const std::string& hostId, bool healthy);

private:
    std::shared_ptr<RedisClient> redis_;
    std::unordered_map<std::string, HostInfo> hosts_;
    std::mutex mutex_;
    void loadFromRedis();
    void persistToRedis(const HostInfo& host);
};