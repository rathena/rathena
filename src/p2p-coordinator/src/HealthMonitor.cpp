#include "HealthMonitor.h"
#include "HostRegistry.h"
#include "SessionManager.h"
#include "ZoneManager.h"
#include "Logger.h"
#include <thread>
#include <chrono>
#include <vector>

HealthMonitor::HealthMonitor(std::shared_ptr<HostRegistry> hostRegistry,
                             std::shared_ptr<SessionManager> sessionManager,
                             std::shared_ptr<ZoneManager> zoneManager)
    : hostRegistry_(std::move(hostRegistry)),
      sessionManager_(std::move(sessionManager)),
      zoneManager_(std::move(zoneManager)),
      running_(true) {}

void HealthMonitor::run() {
    running_ = true;
    Logger::info("Health monitor thread started.");
    monitorLoop();
}

void HealthMonitor::stop() {
    running_ = false;
    Logger::info("Health monitor stopping...");
}

void HealthMonitor::monitorLoop() {
    while (running_) {
        checkHosts();
        checkSessions();
        checkZones();
        cleanup();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    Logger::info("Health monitor thread stopped.");
}

void HealthMonitor::checkHosts() {
    auto hosts = hostRegistry_->getAllHosts();
    for (const auto& host : hosts) {
        // In production, this would ping the host to verify it's alive
        // For now, we just log the check
        Logger::debug("HealthMonitor: Checking host " + host.id);
    }
}

void HealthMonitor::checkSessions() {
    auto hosts = hostRegistry_->getAllHosts();
    for (const auto& host : hosts) {
        auto sessions = sessionManager_->getSessionsByHost(host.id);
        for (const auto& session : sessions) {
            // Check if session is still valid
            Logger::debug("HealthMonitor: Checking session " + session.sessionId);
        }
    }
}

void HealthMonitor::checkZones() {
    auto zones = zoneManager_->getAllZones();
    for (const auto& zone : zones) {
        // Check if zone is still active
        Logger::debug("HealthMonitor: Checking zone " + zone.zoneId);
    }
}

void HealthMonitor::cleanup() {
    // Cleanup expired/invalid hosts/sessions/zones
    auto hosts = hostRegistry_->getAllHosts();
    for (const auto& host : hosts) {
        if (!host.healthy) {
            // Mark unhealthy hosts for cleanup
            Logger::warn("HealthMonitor: Host " + host.id + " is unhealthy, initiating cleanup");
            auto sessions = sessionManager_->getSessionsByHost(host.id);
            for (const auto& session : sessions) {
                sessionManager_->endSession(session.sessionId);
            }
            hostRegistry_->unregisterHost(host.id);
        }
    }
    Logger::debug("HealthMonitor: Cleanup complete");
}
