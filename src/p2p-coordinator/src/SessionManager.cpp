#include "SessionManager.h"
#include "HostRegistry.h"
#include "RedisClient.h"
#include "Logger.h"
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>

using json = nlohmann::json;

namespace {
    std::string sessionStateToString(SessionState state) {
        switch (state) {
            case SessionState::Created: return "Created";
            case SessionState::Active: return "Active";
            case SessionState::Ended: return "Ended";
            case SessionState::Cleanup: return "Cleanup";
            default: return "Unknown";
        }
    }
    SessionState sessionStateFromString(const std::string& s) {
        if (s == "Created") return SessionState::Created;
        if (s == "Active") return SessionState::Active;
        if (s == "Ended") return SessionState::Ended;
        if (s == "Cleanup") return SessionState::Cleanup;
        throw std::invalid_argument("Unknown SessionState: " + s);
    }
    std::string sessionToJson(const SessionInfo& session) {
        json j;
        j["sessionId"] = session.sessionId;
        j["hostId"] = session.hostId;
        j["zoneId"] = session.zoneId;
        j["state"] = sessionStateToString(session.state);
        return j.dump();
    }
    SessionInfo sessionFromJson(const std::string& data) {
        auto j = json::parse(data);
        SessionInfo session;
        session.sessionId = j.value("sessionId", "");
        session.hostId = j.value("hostId", "");
        session.zoneId = j.value("zoneId", "");
        session.state = sessionStateFromString(j.value("state", "Created"));
        return session;
    }
    std::string generateUUID() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 32; ++i) {
            ss << dis(gen);
        }
        return ss.str();
    }
}

SessionManager::SessionManager(std::shared_ptr<HostRegistry> hostRegistry, std::shared_ptr<RedisClient> redis)
    : hostRegistry_(std::move(hostRegistry)), redis_(std::move(redis)) {
    loadFromRedis();
}

std::string SessionManager::createSession(const std::string& hostId, const std::string& zoneId) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string sessionId = generateUUID();
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
    // Try to load from Redis if not in memory
    try {
        auto dataOpt = redis_->hget("sessions", sessionId);
        if (dataOpt) {
            SessionInfo session = sessionFromJson(*dataOpt);
            sessions_[sessionId] = session;
            return session;
        }
    } catch (const std::exception& ex) {
        Logger::error("Failed to get session from Redis: " + std::string(ex.what()));
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
    try {
        auto sessionIds = redis_->hkeys("sessions");
        for (const auto& id : sessionIds) {
            auto data = redis_->hget("sessions", id);
            if (data) {
                SessionInfo session = sessionFromJson(*data);
                sessions_[id] = session;
            }
        }
    } catch (const std::exception& ex) {
        Logger::error("Failed to load sessions from Redis: " + std::string(ex.what()));
    }
}

void SessionManager::persistToRedis(const SessionInfo& session) {
    try {
        std::string data = sessionToJson(session);
        redis_->hset("sessions", session.sessionId, data);
    } catch (const std::exception& ex) {
        Logger::error("Failed to persist session to Redis: " + std::string(ex.what()));
        throw;
    }
}