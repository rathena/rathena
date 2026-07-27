#include "HostRegistry.h"
#include "RedisClient.h"
#include "Logger.h"
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

HostRegistry::HostRegistry(std::shared_ptr<RedisClient> redis)
    : redis_(std::move(redis)) {
    loadFromRedis();
}

bool HostRegistry::registerHost(const HostInfo& host) {
    std::lock_guard<std::mutex> lock(mutex_);
    hosts_[host.id] = host;
    persistToRedis(host);
    Logger::info("Registered host: " + host.id);
    return true;
}

bool HostRegistry::unregisterHost(const std::string& hostId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hosts_.find(hostId);
    if (it != hosts_.end()) {
        hosts_.erase(it);
        redis_->hdel("hosts", hostId);
        Logger::info("Unregistered host: " + hostId);
        return true;
    }
    return false;
}

bool HostRegistry::updateQualityScore(const std::string& hostId, double score) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hosts_.find(hostId);
    if (it != hosts_.end()) {
        it->second.qualityScore = score;
        persistToRedis(it->second);
        Logger::info("Updated quality score for host: " + hostId);
        return true;
    }
    return false;
}

std::vector<HostInfo> HostRegistry::getAllHosts() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<HostInfo> result;
    for (const auto& [id, host] : hosts_) {
        result.push_back(host);
    }
    return result;
}

std::optional<HostInfo> HostRegistry::getHost(const std::string& hostId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hosts_.find(hostId);
    if (it != hosts_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void HostRegistry::markHostHealthy(const std::string& hostId, bool healthy) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = hosts_.find(hostId);
    if (it != hosts_.end()) {
        it->second.healthy = healthy;
        persistToRedis(it->second);
        Logger::info("Host " + hostId + " health set to " + (healthy ? "true" : "false"));
    }
}

void HostRegistry::loadFromRedis() {
    std::lock_guard<std::mutex> lock(mutex_);
    hosts_.clear();
    auto hostIds = redis_->hkeys("hosts");
    for (const auto& id : hostIds) {
        auto data = redis_->hget("hosts", id);
        if (data) {
            try {
                auto j = json::parse(*data);
                HostInfo host;
                host.id = j.value("id", "");
                host.address = j.value("address", "");
                host.port = j.value("port", 0);
                host.healthy = j.value("healthy", true);
                host.qualityScore = j.value("qualityScore", 1.0);
                hosts_[id] = host;
            } catch (const std::exception& ex) {
                Logger::error("Failed to deserialize host " + id + ": " + ex.what());
            }
        }
    }
}

void HostRegistry::persistToRedis(const HostInfo& host) {
    try {
        json j;
        j["id"] = host.id;
        j["address"] = host.address;
        j["port"] = host.port;
        j["healthy"] = host.healthy;
        j["qualityScore"] = host.qualityScore;
        redis_->hset("hosts", host.id, j.dump());
    } catch (const std::exception& ex) {
        Logger::error("Failed to persist host to Redis: " + std::string(ex.what()));
    }
}
