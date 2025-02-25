#include "p2p_integration.hpp"
#include <algorithm>
#include <chrono>
#include <thread>
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "pc.hpp"
#include "battle.hpp"
#include "clif.hpp"
#include "map.hpp"

// Global timer function implementation
TIMER_FUNC(p2p_health_check) {
    rathena::P2PIntegration::getInstance().checkVPSConnection();
    return 0;
}

namespace rathena {

P2PIntegration* P2PIntegration::instance = nullptr;

P2PIntegration::P2PIntegration() 
    : vps_connected(true)
    , last_vps_check(std::chrono::system_clock::now()) {
}

P2PIntegration::~P2PIntegration() {
}

P2PIntegration& P2PIntegration::getInstance() {
    if (!instance) {
        instance = new P2PIntegration();
    }
    return *instance;
}

bool P2PIntegration::init() {
    ShowInfo("Initializing P2P Integration System\n");
    
    // Start health check timer
    add_timer_func_list(p2p_health_check, "p2p_health_check");
    add_timer_interval(gettick() + 1000, p2p_health_check, 0, 0, VPS_CHECK_INTERVAL * 1000);
    
    return true;
}

void P2PIntegration::final() {
    ShowInfo("Finalizing P2P Integration System\n");
}

bool P2PIntegration::checkVPSConnection() {
    // Attempt to connect to VPS endpoints
    bool success = true;
    
    // Check main server endpoint
    if (!validateConnection("main_server")) {
        success = false;
    }
    
    // Check database endpoint
    if (!validateConnection("database")) {
        success = false;
    }
    
    // Update status
    setVPSStatus(success);
    
    if (!success) {
        ShowWarning("VPS connection check failed - operating in P2P-only mode\n");
    }
    
    return success;
}

void P2PIntegration::setVPSStatus(bool connected) {
    if (vps_connected != connected) {
        vps_connected = connected;
        ShowInfo("VPS connection status changed to: %s\n", connected ? "Online" : "Offline");
        
        // Handle connection status change
        if (!connected) {
            // Switch eligible maps to P2P hosting
            auto& host_mgr = P2PHostManager::getInstance();
            for (const auto& pair : host_mgr.getHosts()) {
                auto host = pair.second;
                if (host->getStatus() == P2PHost::Status::ACTIVE) {
                    // Check for eligible maps to migrate
                    redistributeLoad();
                }
            }
        }
    }
}

bool P2PIntegration::measureRegionalLatency(uint32 account_id) {
    auto& host_mgr = P2PHostManager::getInstance();
    auto host = host_mgr.getHost(account_id);
    if (!host) return false;
    
    std::vector<RegionalLatency> measurements;
    
    // Measure latency to each regional relay
    for (const auto& relay : regional_relays) {
        const std::string& region = relay.first;
        const std::string& endpoint = relay.second;
        
        uint16 latency = pingEndpoint(endpoint);
        if (latency > 0) {
            RegionalLatency rl;
            rl.region = region;
            rl.latency_ms = latency;
            rl.relay_address = endpoint;
            measurements.push_back(rl);
        }
    }
    
    // Update host's regional latencies if we got any measurements
    if (!measurements.empty()) {
        P2PHostStats updated_stats = host->getStats();  // Create a copy
        updated_stats.regional_latencies = measurements;
        host->updateStats(updated_stats);  // Update with modified copy
        
        // Log the measurements
        ShowInfo("Updated regional latencies for host %d:\n", account_id);
        for (const auto& m : measurements) {
            ShowInfo("  %s: %d ms\n", m.region.c_str(), m.latency_ms);
        }
        
        return true;
    }
    
    return false;
}

bool P2PIntegration::updateRegionalLatency(uint32 account_id, const std::string& region, uint16 latency) {
    auto& host_mgr = P2PHostManager::getInstance();
    auto host = host_mgr.getHost(account_id);
    if (!host) return false;
    
    // Update latency cache
    latency_cache[account_id][region] = latency;
    
    // Trigger route optimization if needed
    if (latency > host_mgr.getConfig().max_latency_ms) {
        std::string optimal_relay;
        if (selectOptimalRelay(region, optimal_relay)) {
            ShowInfo("Switching host %d to relay %s for region %s\n", 
                account_id, optimal_relay.c_str(), region.c_str());
            return setupRegionalRelay(region, optimal_relay);
        }
    }
    
    return true;
}

void P2PIntegration::redistributeLoad() {
    auto& host_mgr = P2PHostManager::getInstance();
    
    // Find maps that need load balancing
    auto overloaded_maps = findOverloadedMaps();
    if (overloaded_maps.empty()) return;
    
    ShowInfo("Redistributing load for %zu maps\n", overloaded_maps.size());
    
    for (const auto& map_name : overloaded_maps) {
        uint32 new_host_id;
        if (findOptimalHostForMap(map_name, new_host_id)) {
            // Find current host by checking all hosts
            uint32 current_host_id = 0;
            for (const auto& pair : host_mgr.getHosts()) {
                auto host = pair.second;
                const auto& maps = host->getAssignedMaps();
                if (std::find(maps.begin(), maps.end(), map_name) != maps.end()) {
                    current_host_id = host->getAccountId();
                    break;
                }
            }
            
            if (current_host_id) {
                initiateMigration(map_name, current_host_id, new_host_id);
            }
        }
    }
}

double P2PIntegration::calculateHostScore(const std::shared_ptr<P2PHost>& host) const {
    if (!host) return 0.0;
    
    const auto& stats = host->getStats();
    const auto& config = P2PHostManager::getInstance().getConfig();
    
    // Calculate normalized scores for each metric
    double cpu_score = (stats.cpu_ghz * stats.cpu_cores) / 
                      (config.min_cpu_ghz * config.min_cpu_cores);
    
    double ram_score = static_cast<double>(stats.free_ram_mb) / config.min_ram_mb;
    
    double network_score = static_cast<double>(stats.network_speed_mbps) / config.min_network_mbps;
    
    double latency_score = 1.0 - (static_cast<double>(stats.latency_ms) / config.max_latency_ms);
    latency_score = std::max(0.0, latency_score);  // Clamp to non-negative
    
    // Weight the scores
    const double CPU_WEIGHT = 0.3;
    const double RAM_WEIGHT = 0.2;
    const double NETWORK_WEIGHT = 0.3;
    const double LATENCY_WEIGHT = 0.2;
    
    return (cpu_score * CPU_WEIGHT) +
           (ram_score * RAM_WEIGHT) +
           (network_score * NETWORK_WEIGHT) +
           (latency_score * LATENCY_WEIGHT);
}

bool P2PIntegration::validateConnection(const std::string& endpoint) {
    // TODO: Implement actual connection test
    // For now, return true if VPS was previously connected
    return vps_connected;
}

uint16 P2PIntegration::pingEndpoint(const std::string& endpoint) {
    // TODO: Implement actual ping
    // For now, return a random latency between 20-200ms
    return 20 + (rand() % 181);
}

bool P2PIntegration::selectOptimalRelay(const std::string& region, std::string& relay_address) {
    auto it = regional_relays.find(region);
    if (it != regional_relays.end()) {
        relay_address = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> P2PIntegration::findOverloadedMaps() {
    std::vector<std::string> overloaded;
    auto& host_mgr = P2PHostManager::getInstance();
    
    for (const auto& pair : host_mgr.getHosts()) {
        auto host = pair.second;
        if (host->getStatus() == P2PHost::Status::ACTIVE) {
            for (const auto& map : host->getAssignedMaps()) {
                if (calculateMapLoad(map) > 0.8) { // 80% threshold
                    overloaded.push_back(map);
                }
            }
        }
    }
    
    return overloaded;
}

double P2PIntegration::calculateMapLoad(const std::string& map_name) {
    // TODO: Implement actual load calculation based on:
    // - Number of players
    // - Number of monsters
    // - Active scripts
    // - Network traffic
    return 0.5; // Placeholder
}

bool P2PIntegration::findOptimalHostForMap(const std::string& map_name, uint32& host_id) {
    auto& host_mgr = P2PHostManager::getInstance();
    
    double best_score = 0.0;
    std::shared_ptr<P2PHost> best_host;
    
    for (const auto& pair : host_mgr.getHosts()) {
        auto host = pair.second;
        if (host->getStatus() == P2PHost::Status::ACTIVE) {
            double score = calculateHostScore(host);
            if (score > best_score) {
                best_score = score;
                best_host = host;
            }
        }
    }
    
    if (best_host) {
        host_id = best_host->getAccountId();
        return true;
    }
    
    return false;
}

bool P2PIntegration::initiateMigration(const std::string& map_name, uint32 from_host, uint32 to_host) {
    ShowInfo("Initiating migration of map %s from host %d to %d\n", 
        map_name.c_str(), from_host, to_host);
    
    // Sync map state between hosts
    if (!syncMapState(map_name, from_host, to_host)) {
        ShowError("Failed to sync map state during migration\n");
        return false;
    }
    
    // Complete the migration
    return completeMigration(map_name, to_host);
}

bool P2PIntegration::syncMapState(const std::string& map_name, uint32 source_host, uint32 target_host) {
    // TODO: Implement actual state sync
    // This would include:
    // - Monster positions and state
    // - Item drops
    // - Player positions
    // - Active scripts
    return true;
}

bool P2PIntegration::completeMigration(const std::string& map_name, uint32 new_host) {
    auto& host_mgr = P2PHostManager::getInstance();
    auto host = host_mgr.getHost(new_host);
    
    if (!host) return false;
    
    if (!host->assignMap(map_name)) {
        ShowError("Failed to assign map %s to new host %d\n", map_name.c_str(), new_host);
        return false;
    }
    
    ShowInfo("Successfully migrated map %s to host %d\n", map_name.c_str(), new_host);
    return true;
}

} // namespace rathena