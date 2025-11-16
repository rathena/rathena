#include "HostRegistry.h"
#include "SessionManager.h"
#include "ZoneManager.h"
#include "WebRTCSignaling.h"
#include "RestApiServer.h"
#include "RedisClient.h"
#include "AIServiceClient.h"
#include "HealthMonitor.h"
#include "Logger.h"
#include "ErrorHandling.h"

#include <iostream>
#include <memory>
#include <thread>

int main(int argc, char* argv[]) {
    try {
        Logger::init();

        auto redisClient = std::make_shared<RedisClient>();
        auto hostRegistry = std::make_shared<HostRegistry>(redisClient);
        auto sessionManager = std::make_shared<SessionManager>(hostRegistry, redisClient);
        auto zoneManager = std::make_shared<ZoneManager>(sessionManager, redisClient);
        auto aiServiceClient = std::make_shared<AIServiceClient>();
        auto signaling = std::make_shared<WebRTCSignaling>(redisClient, sessionManager, aiServiceClient);
        auto restApi = std::make_shared<RestApiServer>(hostRegistry, sessionManager, zoneManager, aiServiceClient);
        auto healthMonitor = std::make_shared<HealthMonitor>(hostRegistry, sessionManager, zoneManager);

        std::thread signalingThread([&]() { signaling->run(); });
        std::thread restApiThread([&]() { restApi->run(); });
        std::thread healthThread([&]() { healthMonitor->run(); });

        Logger::info("p2p-coordinator service started.");

        signalingThread.join();
        restApiThread.join();
        healthThread.join();
    } catch (const std::exception& ex) {
        Logger::error(std::string("Fatal error: ") + ex.what());
        return 1;
    }
    return 0;
}