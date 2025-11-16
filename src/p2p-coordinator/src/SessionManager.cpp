#include "SessionManager.h"
#include "HostRegistry.h"
#include "RedisClient.h"
#include "Logger.h"
#include <stdexcept>

SessionManager::SessionManager(std::shared_ptr<HostRegistry> hostRegistry, std::shared_ptr<RedisClient> redis)
    : hostRegistry_(std::move(hostRegistry)), redis_(std::move(redis)) {
    loadFromRedis();
}

std::string SessionManager::createSession(const std::string& hostId, const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string sessionId = "sess_" + std::to_string(sessions_.size() + 1); // Replace with UUID
    SessionInfo session{sessionId, hostId, zoneId, SessionState::Created};
    sessions_[sessionId] = session;
    persistToRedis(session);
    Logger::info("Created session: " + sessionId);
    return sessionId;
}

bool SessionManager::activateSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sessionId);
    if (it != sessions_.end()) {
        it->second.state = SessionState::Active;
        persistToRedis(it->second);
        Logger::info("Activated session: " + sessionId);
        return true;
    }
    return false;
}

bool SessionManager::endSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sessionId);
    if (it != sessions_.end()) {
        it->second.state = SessionState::Ended;
        persistToRedis(it->second);
        Logger::info("Ended session: " + sessionId);
        return true;
    }
    return false;
}

bool SessionManager::cleanupSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sessionId);
    if (it != sessions_.end()) {
        it->second.state = SessionState::Cleanup;
        persistToRedis(it->second);
        Logger::info("Cleaned up session: " + sessionId);
        return true;
    }
    return false;
}

std::optional<SessionInfo> SessionManager::getSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sessionId);
    if (it != sessions_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<SessionInfo> SessionManager::getSessionsByHost(const std::string& hostId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SessionInfo> result;
    for (const auto& [id, session] : sessions_) {
        if (session.hostId == hostId) {
            result.push_back(session);
        }
    }
    return result;
}

std::vector<SessionInfo> SessionManager::getSessionsByZone(const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SessionInfo> result;
    for (const auto& [id, session] : sessions_) {
        if (session.zoneId == zoneId) {
            result.push_back(session);
        }
    }
    return result;
}

void SessionManager::loadFromRedis() {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.clear();
    auto sessionIds = redis_->hkeys("sessions");
    for (const auto& id : sessionIds) {
        auto data = redis_->hget("sessions", id);
        if (data) {
            // Deserialize data to SessionInfo (assume JSON, to be implemented)
            // SessionInfo session = SessionInfo::fromJson(*data);
            // sessions_[id] = session;
        }
    }
}

void SessionManager::persistToRedis(const SessionInfo& session) {
    // Serialize SessionInfo to JSON (to be implemented)
    // std::string data = session.toJson();
    // redis_->hset("sessions", session.sessionId, data);
}