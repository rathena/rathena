#include "ZoneManager.h"
#include "SessionManager.h"
#include "RedisClient.h"
#include "Logger.h"
#include <stdexcept>
#include <algorithm>

ZoneManager::ZoneManager(std::shared_ptr<SessionManager> sessionManager, std::shared_ptr<RedisClient> redis)
    : sessionManager_(std::move(sessionManager)), redis_(std::move(redis)) {
    loadFromRedis();
}

bool ZoneManager::addZone(const ZoneInfo& zone) {
    std::lock_guard<std::mutex> lock(mutex_);
    zones_[zone.zoneId] = zone;
    persistToRedis(zone);
    Logger::info("Added zone: " + zone.zoneId);
    return true;
}

bool ZoneManager::removeZone(const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = zones_.find(zoneId);
    if (it != zones_.end()) {
        zones_.erase(it);
        redis_->hdel("zones", zoneId);
        Logger::info("Removed zone: " + zoneId);
        return true;
    }
    return false;
}

std::optional<ZoneInfo> ZoneManager::getZone(const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = zones_.find(zoneId);
    if (it != zones_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<ZoneInfo> ZoneManager::getAllZones() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ZoneInfo> result;
    for (const auto& [id, zone] : zones_) {
        result.push_back(zone);
    }
    return result;
}

bool ZoneManager::mapSessionToZone(const std::string& sessionId, const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = zones_.find(zoneId);
    if (it != zones_.end()) {
        it->second.sessionIds.push_back(sessionId);
        persistToRedis(it->second);
        Logger::info("Mapped session " + sessionId + " to zone " + zoneId);
        return true;
    }
    return false;
}

bool ZoneManager::unmapSessionFromZone(const std::string& sessionId, const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = zones_.find(zoneId);
    if (it != zones_.end()) {
        auto& sessions = it->second.sessionIds;
        sessions.erase(std::remove(sessions.begin(), sessions.end(), sessionId), sessions.end());
        persistToRedis(it->second);
        Logger::info("Unmapped session " + sessionId + " from zone " + zoneId);
        return true;
    }
    return false;
}

void ZoneManager::loadFromRedis() {
    std::lock_guard<std::mutex> lock(mutex_);
    zones_.clear();
    auto zoneIds = redis_->hkeys("zones");
    for (const auto& id : zoneIds) {
        auto data = redis_->hget("zones", id);
        if (data) {
            // Deserialize data to ZoneInfo (assume JSON, to be implemented)
            // ZoneInfo zone = ZoneInfo::fromJson(*data);
            // zones_[id] = zone;
        }
    }
}

void ZoneManager::persistToRedis(const ZoneInfo& zone) {
    // Serialize ZoneInfo to JSON (to be implemented)
    // std::string data = zone.toJson();
    // redis_->hset("zones", zone.zoneId, data);
}