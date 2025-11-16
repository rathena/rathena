#pragma once
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

class HostRegistry;
class SessionManager;
class ZoneManager;

class HealthMonitor {
public:
    HealthMonitor(std::shared_ptr<HostRegistry> hostRegistry,
                  std::shared_ptr<SessionManager> sessionManager,
                  std::shared_ptr<ZoneManager> zoneManager);
    void run();
    void stop();

private:
    std::shared_ptr<HostRegistry> hostRegistry_;
    std::shared_ptr<SessionManager> sessionManager_;
    std::shared_ptr<ZoneManager> zoneManager_;
    std::atomic<bool> running_;
    void monitorLoop();
    void checkHosts();
    void checkSessions();
    void checkZones();
    void cleanup();
};