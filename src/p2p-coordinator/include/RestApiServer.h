#pragma once
#include <memory>
#include <string>
#include <vector>

class HostRegistry;
class SessionManager;
class ZoneManager;
class AIServiceClient;

class RestApiServer {
public:
    RestApiServer(std::shared_ptr<HostRegistry> hostRegistry,
                  std::shared_ptr<SessionManager> sessionManager,
                  std::shared_ptr<ZoneManager> zoneManager,
                  std::shared_ptr<AIServiceClient> aiServiceClient);
    void run();

    // REST endpoint handlers (examples)
    std::string handleRegisterHost(const std::string& body);
    std::string handleUnregisterHost(const std::string& hostId);
    std::string handleCreateSession(const std::string& body);
    std::string handleEndSession(const std::string& sessionId);
    std::string handleZoneMapping(const std::string& body);
    std::string handleGetHostList();
    std::string handleGetSessionList();
    std::string handleGetZoneList();
    std::string handleNpcState(const std::string& npcId);

private:
    std::shared_ptr<HostRegistry> hostRegistry_;
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<ZoneManager> zoneManager_;
    std::shared_ptr<AIServiceClient> aiServiceClient_;
    void setupRoutes();
    void processRequests();
};