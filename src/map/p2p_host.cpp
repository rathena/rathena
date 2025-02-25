#include "p2p_host.hpp"
#include <fstream>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "pc.hpp"
#include "map.hpp"

namespace rathena {

P2PHostManager* P2PHostManager::instance = nullptr;
const char* P2PHostManager::PERSISTENCE_FILE = "db/p2p_hosts.yml";

// P2PHost implementation
P2PHost::P2PHost(uint32 account_id, const P2PHostStats& stats)
    : account_id(account_id)
    , stats(stats)
    , status(Status::STANDBY)
    , last_update(std::chrono::system_clock::now()) {
    // Initialize security validation
    security.spawn_rate_valid = true;
    security.drop_rate_valid = true;
    security.monster_stats_valid = true;
    security.player_movement_valid = true;
    security.skill_usage_valid = true;
    security.validation_timestamp = (uint32)time(nullptr);
}

P2PHost::~P2PHost() {
    // Ensure clean migration of any assigned maps before destruction
    for (const auto& map_name : assigned_maps) {
        P2PHostManager::getInstance().migrateMap(map_name, account_id, 0);
    }
}

bool P2PHost::isEligible(const P2PHostConfig& config) const {
    return stats.cpu_ghz >= config.min_cpu_ghz &&
           stats.cpu_cores >= config.min_cpu_cores &&
           stats.free_ram_mb >= config.min_ram_mb &&
           stats.network_speed_mbps >= config.min_network_mbps &&
           stats.latency_ms <= config.max_latency_ms &&
           stats.disconnects_per_hour <= config.max_disconnects_per_hour &&
           isSecurityValid();
}

bool P2PHost::validateHosting(ValidationType type) {
    switch(type) {
        case ValidationType::SPAWN_RATE:
            security.spawn_rate_valid = validateSpawnRates();
            break;
        case ValidationType::DROP_RATE:
            security.drop_rate_valid = validateDropRates();
            break;
        case ValidationType::MONSTER_STATS:
            security.monster_stats_valid = validateMonsterStats();
            break;
        case ValidationType::PLAYER_MOVEMENT:
            security.player_movement_valid = validatePlayerMovement();
            break;
        case ValidationType::SKILL_USAGE:
            security.skill_usage_valid = validateSkillUsage();
            break;
    }
    security.validation_timestamp = (uint32)time(nullptr);
    return isSecurityValid();
}

bool P2PHost::isSecurityValid() const {
    return security.spawn_rate_valid &&
           security.drop_rate_valid &&
           security.monster_stats_valid &&
           security.player_movement_valid &&
           security.skill_usage_valid;
}

// Add these validation methods in the implementation
bool P2PHost::validateSpawnRates() {
    // Compare spawn rates with expected values
    return true;  // Placeholder
}

bool P2PHost::validateDropRates() {
    // Verify item drop rates match server settings
    return true;  // Placeholder
}

bool P2PHost::validateMonsterStats() {
    // Check if monster HP, stats match database
    return true;  // Placeholder
}

bool P2PHost::validatePlayerMovement() {
    // Verify player speed and position updates
    return true;  // Placeholder
}

bool P2PHost::validateSkillUsage() {
    // Check skill cooldowns and effects
    return true;  // Placeholder
}

bool P2PHost::updateStats(const P2PHostStats& new_stats) {
    stats = new_stats;
    last_update = std::chrono::system_clock::now();
    return true;
}

bool P2PHost::assignMap(const std::string& map_name) {
    if (std::find(assigned_maps.begin(), assigned_maps.end(), map_name) != assigned_maps.end()) {
        return false;
    }
    assigned_maps.push_back(map_name);
    status = Status::ACTIVE;
    return true;
}

bool P2PHost::unassignMap(const std::string& map_name) {
    auto it = std::find(assigned_maps.begin(), assigned_maps.end(), map_name);
    if (it == assigned_maps.end()) {
        return false;
    }
    assigned_maps.erase(it);
    if (assigned_maps.empty()) {
        status = Status::STANDBY;
    }
    return true;
}

// P2PHostManager implementation
P2PHostManager::P2PHostManager() : vps_connected(true) {
    loadConfig();
}

P2PHostManager::~P2PHostManager() {
    savePersistentData();
}

P2PHostManager& P2PHostManager::getInstance() {
    if (!instance) {
        instance = new P2PHostManager();
    }
    return *instance;
}

bool P2PHostManager::init() {
    loadPersistentData();
    add_timer_func_list(health_check_timer, "p2p_host_health_check");
    add_timer_interval(gettick() + 1000, health_check_timer, 0, 0, 15000); // Check every 15 seconds
    return true;
}

bool P2PHostManager::updateRegionalLatencies(uint32 account_id, const std::vector<RegionalLatency>& latencies) {
    auto host = getHost(account_id);
    if (!host) return false;

    host->stats.regional_latencies = latencies;
    return true;
}

std::vector<RegionalLatency> P2PHostManager::getRegionalLatencies(uint32 account_id) const {
    auto host = getHost(account_id);
    if (!host) return std::vector<RegionalLatency>();

    return host->stats.regional_latencies;
}

void P2PHostManager::performSecurityValidation() {
    for (auto& pair : hosts) {
        auto host = pair.second;
        host->validateHosting(P2PHost::ValidationType::SPAWN_RATE);
        host->validateHosting(P2PHost::ValidationType::DROP_RATE);
        host->validateHosting(P2PHost::ValidationType::MONSTER_STATS);
        host->validateHosting(P2PHost::ValidationType::PLAYER_MOVEMENT);
        host->validateHosting(P2PHost::ValidationType::SKILL_USAGE);
    }
}

void P2PHostManager::doHealthCheck() {
    auto now = std::chrono::system_clock::now();
    std::vector<uint32> hosts_to_remove;
    
    performSecurityValidation();
    
    for (auto& pair : hosts) {
        auto host = pair.second;
        auto& stats = host->stats;
        
        // Check if host is still eligible
        if (!host->isEligible(config)) {
            stats.consecutive_failed_checks++;
        } else {
            stats.consecutive_failed_checks = 0;
        }
        
        // If too many failed checks, mark for removal
        if (stats.consecutive_failed_checks >= 3) {
            hosts_to_remove.push_back(pair.first);
        }
    }
    
    // Remove unhealthy hosts
    for (auto account_id : hosts_to_remove) {
        unregisterHost(account_id);
    }
    
    // Redistribute load if needed
    redistributeLoad();
}

void P2PHostManager::loadPersistentData() {
    try {
        YAML::Node data = YAML::LoadFile(PERSISTENCE_FILE);
        
        for (const auto& host_node : data["hosts"]) {
            uint32 account_id = host_node["account_id"].as<uint32>();
            P2PHostStats stats;
            stats.cpu_ghz = host_node["cpu_ghz"].as<double>();
            stats.cpu_cores = host_node["cpu_cores"].as<uint16>();
            stats.free_ram_mb = host_node["free_ram_mb"].as<uint64>();
            stats.network_speed_mbps = host_node["network_speed_mbps"].as<uint32>();
            stats.latency_ms = host_node["latency_ms"].as<uint16>();
            stats.disconnects_per_hour = host_node["disconnects_per_hour"].as<uint32>();
            stats.vps_connection_active = host_node["vps_connection_active"].as<bool>();
            
            // Load regional latencies
            if (host_node["regional_latencies"]) {
                for (const auto& latency_node : host_node["regional_latencies"]) {
                    RegionalLatency latency;
                    latency.region = latency_node["region"].as<std::string>();
                    latency.latency_ms = latency_node["latency_ms"].as<uint16>();
                    latency.relay_address = latency_node["relay_address"].as<std::string>();
                    stats.regional_latencies.push_back(latency);
                }
            }
            
            registerHost(account_id, stats);
            
            // Restore assigned maps
            auto host = hosts[account_id];
            for (const auto& map_name : host_node["assigned_maps"]) {
                host->assignMap(map_name.as<std::string>());
            }
        }
        
        ShowInfo("Loaded P2P host data from %s\n", PERSISTENCE_FILE);
    } catch (const std::exception& e) {
        ShowWarning("Failed to load P2P host data: %s\n", e.what());
    }
}

void P2PHostManager::savePersistentData() {
    try {
        YAML::Node data;
        YAML::Node hosts_node = data["hosts"];
        
        for (const auto& pair : hosts) {
            auto host = pair.second;
            YAML::Node host_node;
            host_node["account_id"] = host->getAccountId();
            
            const auto& stats = host->getStats();
            host_node["cpu_ghz"] = stats.cpu_ghz;
            host_node["cpu_cores"] = stats.cpu_cores;
            host_node["free_ram_mb"] = stats.free_ram_mb;
            host_node["network_speed_mbps"] = stats.network_speed_mbps;
            host_node["latency_ms"] = stats.latency_ms;
            host_node["disconnects_per_hour"] = stats.disconnects_per_hour;
            host_node["vps_connection_active"] = stats.vps_connection_active;
            
            // Save regional latencies
            YAML::Node latencies_node;
            for (const auto& latency : stats.regional_latencies) {
                YAML::Node latency_node;
                latency_node["region"] = latency.region;
                latency_node["latency_ms"] = latency.latency_ms;
                latency_node["relay_address"] = latency.relay_address;
                latencies_node.push_back(latency_node);
            }
            host_node["regional_latencies"] = latencies_node;
            
            YAML::Node maps_node;
            for (const auto& map_name : host->getAssignedMaps()) {
                maps_node.push_back(map_name);
            }
            host_node["assigned_maps"] = maps_node;
            
            hosts_node.push_back(host_node);
        }
        
        std::ofstream fout(PERSISTENCE_FILE);
        fout << data;
        ShowInfo("Saved P2P host data to %s\n", PERSISTENCE_FILE);
    } catch (const std::exception& e) {
        ShowError("Failed to save P2P host data: %s\n", e.what());
    }
}

bool P2PHostManager::loadConfig() {
    // TODO: Implement configuration loading from p2p_map_config.conf
    return true; // Placeholder
}

void P2PHostManager::redistributeLoad() {
    // TODO: Implement load redistribution logic
    // This would be called periodically to balance load across P2P hosts
}

void P2PHostManager::updateHostMetrics() {
    // TODO: Implement metric collection and updates
    // This would be called periodically to update host performance metrics
}

} // namespace rathena