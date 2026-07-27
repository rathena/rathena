#include "RestApiServer.h"
#include "HostRegistry.h"
#include "SessionManager.h"
#include "ZoneManager.h"
#include "AIServiceClient.h"
#include "Logger.h"
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <httplib.h>

using json = nlohmann::json;

RestApiServer::RestApiServer(std::shared_ptr<HostRegistry> hostRegistry,
                             std::shared_ptr<SessionManager> sessionManager,
                             std::shared_ptr<ZoneManager> zoneManager,
                             std::shared_ptr<AIServiceClient> aiServiceClient)
    : hostRegistry_(std::move(hostRegistry)),
      sessionManager_(std::move(sessionManager)),
      zoneManager_(std::move(zoneManager)),
      aiServiceClient_(std::move(aiServiceClient)),
      running_(false) {}

void RestApiServer::run() {
    running_ = true;
    Logger::info("REST API server thread started.");
    
    httplib::Server svr;
    
    // Setup routes
    svr.Post("/api/hosts/register", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content(handleRegisterHost(req.body), "application/json");
    });
    
    svr.Post("/api/hosts/unregister", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string hostId = body.value("hostId", "");
        res.set_content(handleUnregisterHost(hostId), "application/json");
    });
    
    svr.Post("/api/sessions/create", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content(handleCreateSession(req.body), "application/json");
    });
    
    svr.Post("/api/sessions/end", [this](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string sessionId = body.value("sessionId", "");
        res.set_content(handleEndSession(sessionId), "application/json");
    });
    
    svr.Post("/api/zones/mapping", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content(handleZoneMapping(req.body), "application/json");
    });
    
    svr.Get("/api/hosts", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(handleGetHostList(), "application/json");
    });
    
    svr.Get("/api/sessions", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(handleGetSessionList(), "application/json");
    });
    
    svr.Get("/api/zones", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(handleGetZoneList(), "application/json");
    });
    
    svr.Get("/api/npc/(.+)", [this](const httplib::Request& req, httplib::Response& res) {
        std::string npcId = req.matches[1];
        res.set_content(handleNpcState(npcId), "application/json");
    });
    
    // Health check endpoint
    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        json j;
        j["status"] = "ok";
        res.set_content(j.dump(), "application/json");
    });
    
    // Start listening (non-blocking)
    svr.listen("0.0.0.0", 8080);
    
    Logger::info("REST API server thread stopped.");
}

void RestApiServer::stop() {
    running_ = false;
    Logger::info("REST API server stopping...");
}

void RestApiServer::setupRoutes() {
    // Routes are set up in run() using httplib
}

void RestApiServer::processRequests() {
    // Processing is handled by httplib's listen() in run()
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::string RestApiServer::handleRegisterHost(const std::string& body) {
    try {
        auto j = json::parse(body);
        HostInfo host;
        host.id = j.value("id", "");
        host.address = j.value("address", "");
        host.port = j.value("port", 0);
        host.healthy = true;
        host.qualityScore = j.value("qualityScore", 1.0);
        
        if (host.id.empty()) {
            json err;
            err["error"] = "Host ID is required";
            return err.dump();
        }
        
        hostRegistry_->registerHost(host);
        json resp;
        resp["status"] = "ok";
        resp["hostId"] = host.id;
        return resp.dump();
    } catch (const std::exception& ex) {
        json err;
        err["error"] = std::string("Failed to register host: ") + ex.what();
        return err.dump();
    }
}

std::string RestApiServer::handleUnregisterHost(const std::string& hostId) {
    if (hostId.empty()) {
        json err;
        err["error"] = "Host ID is required";
        return err.dump();
    }
    
    hostRegistry_->unregisterHost(hostId);
    json resp;
    resp["status"] = "ok";
    return resp.dump();
}

std::string RestApiServer::handleCreateSession(const std::string& body) {
    try {
        auto j = json::parse(body);
        std::string hostId = j.value("hostId", "");
        std::string zoneId = j.value("zoneId", "");
        
        if (hostId.empty()) {
            json err;
            err["error"] = "Host ID is required";
            return err.dump();
        }
        
        std::string sessionId = sessionManager_->createSession(hostId, zoneId);
        json resp;
        resp["status"] = "ok";
        resp["sessionId"] = sessionId;
        return resp.dump();
    } catch (const std::exception& ex) {
        json err;
        err["error"] = std::string("Failed to create session: ") + ex.what();
        return err.dump();
    }
}

std::string RestApiServer::handleEndSession(const std::string& sessionId) {
    if (sessionId.empty()) {
        json err;
        err["error"] = "Session ID is required";
        return err.dump();
    }
    
    sessionManager_->endSession(sessionId);
    json resp;
    resp["status"] = "ok";
    return resp.dump();
}

std::string RestApiServer::handleZoneMapping(const std::string& body) {
    try {
        auto j = json::parse(body);
        std::string sessionId = j.value("sessionId", "");
        std::string zoneId = j.value("zoneId", "");
        std::string action = j.value("action", "map");
        
        if (sessionId.empty() || zoneId.empty()) {
            json err;
            err["error"] = "sessionId and zoneId are required";
            return err.dump();
        }
        
        bool success = false;
        if (action == "map") {
            success = zoneManager_->mapSessionToZone(sessionId, zoneId);
        } else if (action == "unmap") {
            success = zoneManager_->unmapSessionFromZone(sessionId, zoneId);
        }
        
        json resp;
        resp["status"] = success ? "ok" : "error";
        return resp.dump();
    } catch (const std::exception& ex) {
        json err;
        err["error"] = std::string("Failed to process zone mapping: ") + ex.what();
        return err.dump();
    }
}

std::string RestApiServer::handleGetHostList() {
    auto hosts = hostRegistry_->getAllHosts();
    json j = json::array();
    for (const auto& host : hosts) {
        json h;
        h["id"] = host.id;
        h["address"] = host.address;
        h["port"] = host.port;
        h["healthy"] = host.healthy;
        h["qualityScore"] = host.qualityScore;
        j.push_back(h);
    }
    return j.dump();
}

std::string RestApiServer::handleGetSessionList() {
    // Return all sessions from all hosts
    json j = json::array();
    auto hosts = hostRegistry_->getAllHosts();
    for (const auto& host : hosts) {
        auto sessions = sessionManager_->getSessionsByHost(host.id);
        for (const auto& session : sessions) {
            json s;
            s["sessionId"] = session.sessionId;
            s["hostId"] = session.hostId;
            s["zoneId"] = session.zoneId;
            s["state"] = static_cast<int>(session.state);
            j.push_back(s);
        }
    }
    return j.dump();
}

std::string RestApiServer::handleGetZoneList() {
    auto zones = zoneManager_->getAllZones();
    json j = json::array();
    for (const auto& zone : zones) {
        json z;
        z["zoneId"] = zone.zoneId;
        z["sessionIds"] = zone.sessionIds;
        j.push_back(z);
    }
    return j.dump();
}

std::string RestApiServer::handleNpcState(const std::string& npcId) {
    auto state = aiServiceClient_->getNpcState(npcId);
    if (state) {
        return *state;
    }
    json err;
    err["error"] = "NPC state not found";
    err["npcId"] = npcId;
    return err.dump();
}
