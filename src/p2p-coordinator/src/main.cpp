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

        // Load configuration from environment or config file
        int rest_port = std::getenv("P2P_REST_PORT") ? std::stoi(std::getenv("P2P_REST_PORT")) : 8080;
        int signaling_port = std::getenv("P2P_SIGNALING_PORT") ? std::stoi(std::getenv("P2P_SIGNALING_PORT")) : 9000;
        std::string redis_host = std::getenv("P2P_REDIS_HOST") ? std::getenv("P2P_REDIS_HOST") : "127.0.0.1";
        int redis_port = std::getenv("P2P_REDIS_PORT") ? std::stoi(std::getenv("P2P_REDIS_PORT")) : 6379;

        auto redisClient = std::make_shared<RedisClient>();
        redisClient->connect(redis_host, redis_port);

        auto hostRegistry = std::make_shared<HostRegistry>(redisClient);
        auto sessionManager = std::make_shared<SessionManager>(hostRegistry, redisClient);
        auto zoneManager = std::make_shared<ZoneManager>(sessionManager, redisClient);
        auto aiServiceClient = std::make_shared<AIServiceClient>();
        auto signaling = std::make_shared<WebRTCSignaling>(redisClient, sessionManager, aiServiceClient);
        auto restApi = std::make_shared<RestApiServer>(hostRegistry, sessionManager, zoneManager, aiServiceClient);
        auto healthMonitor = std::make_shared<HealthMonitor>(hostRegistry, sessionManager, zoneManager);

        std::atomic<bool> running{true};

        auto shutdown = [&]() {
            running = false;
            Logger::info("Initiating shutdown sequence...");
            // restApi->stop(); // Not implemented
            // signaling->stop(); // Not implemented
            healthMonitor->stop();
        };

        std::thread signalingThread([&]() {
            try { signaling->run(); }
            catch (const std::exception& ex) {
                Logger::error(std::string("WebRTCSignaling thread error: ") + ex.what());
                shutdown();
            }
        });
        std::thread restApiThread([&]() {
            try { restApi->run(); }
            catch (const std::exception& ex) {
                Logger::error(std::string("RestApiServer thread error: ") + ex.what());
                shutdown();
            }
        });
        std::thread healthThread([&]() {
            try { healthMonitor->run(); }
            catch (const std::exception& ex) {
                Logger::error(std::string("HealthMonitor thread error: ") + ex.what());
                shutdown();
            }
        });

        Logger::info("p2p-coordinator service started.");

        // Wait for shutdown signal (could be improved with signal handlers)
        signalingThread.join();
        restApiThread.join();
        healthThread.join();

        Logger::info("p2p-coordinator service stopped cleanly.");
    } catch (const std::exception& ex) {
        Logger::error(std::string("Fatal error: ") + ex.what());
        return 1;
    }
    return 0;
}