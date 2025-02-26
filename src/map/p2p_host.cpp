#include "p2p_host.hpp"
#include <fstream>
#include <algorithm>
#include <yaml-cpp/yaml.h>
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "../common/p2p_map_config.hpp"
#include "pc.hpp"
#include "map.hpp"
#include "p2p_map.hpp"
#include <algorithm>

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
    
    // Synchronize validation result to the main server
    P2PMapServer::getInstance().syncSecurityValidation(account_id, assigned_maps.empty() ? "" : assigned_maps[0], type, result, true); // Use critical sync for security validation
    
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

bool P2PHostManager::registerHost(uint32 account_id, const P2PHostStats& stats) {
    // Check if host already exists
    auto it = hosts.find(account_id);
    if (it != hosts.end()) {
        // Update existing host stats
        return it->second->updateStats(stats);
    }
    
    // Create new host
    auto host = std::make_shared<P2PHost>(account_id, stats);
    
    // Check if host meets minimum requirements
    if (!host->isEligible(config)) {
        ShowInfo("Host %d does not meet minimum requirements\n", account_id);
        return false;
    }
    
    // Add to hosts map
    hosts[account_id] = host;
    ShowInfo("Registered new P2P host: Account ID %d\n", account_id);
    return true;
}

bool P2PHostManager::unregisterHost(uint32 account_id) {
    auto it = hosts.find(account_id);
    if (it == hosts.end()) {
        return false;
    }
    
    auto host = it->second;
    
    // Migrate any assigned maps to other hosts or VPS
    for (const auto& map_name : host->getAssignedMaps()) {
        migrateMap(map_name, account_id, 0); // 0 indicates VPS hosting
    }
    
    // Remove from hosts map
    hosts.erase(it);
    ShowInfo("Unregistered P2P host: Account ID %d\n", account_id);
    return true;
}

bool P2PHostManager::isMapP2PEligible(const std::string& map_name) const {
    return p2p_eligible_maps.find(map_name) != p2p_eligible_maps.end();
}

std::shared_ptr<P2PHost> P2PHostManager::getBestHostForMap(const std::string& map_name) {
    if (!isMapP2PEligible(map_name)) {
        return nullptr;
    }
    
    // Check if map already has assigned hosts
    auto map_it = map_hosts.find(map_name);
    if (map_it != map_hosts.end() && !map_it->second.empty()) {
        // Return the first host in the list (should be the best one)
        uint32 host_id = map_it->second.front();
        return getHost(host_id);
    }
    
    // Find the best host based on performance metrics
    std::shared_ptr<P2PHost> best_host = nullptr;
    double best_score = 0.0;
    
    for (const auto& pair : hosts) {
        auto host = pair.second;
        
        // Skip hosts that don't meet requirements
        if (!host->isEligible(config)) {
            continue;
        }
        
        // Calculate host score based on performance metrics
        double score = calculateHostScore(host);
        
        // Update best host if this one has a better score
        if (score > best_score) {
            best_score = score;
            best_host = host;
        }
    }
    
    // If a host was found, assign the map to it
    if (best_host) {
        best_host->assignMap(map_name);
        
        // Update map_hosts
        if (map_it == map_hosts.end()) {
            map_hosts[map_name] = {best_host->getAccountId()};
        } else {
            map_it->second.insert(map_it->second.begin(), best_host->getAccountId());
        }
    }
    
    return best_host;
}

std::vector<std::shared_ptr<P2PHost>> P2PHostManager::getBackupHostsForMap(const std::string& map_name, size_t count) {
    std::vector<std::shared_ptr<P2PHost>> backup_hosts;
    
    if (!isMapP2PEligible(map_name)) {
        return backup_hosts;
    }
    
    // Get all eligible hosts sorted by score
    std::vector<std::pair<std::shared_ptr<P2PHost>, double>> eligible_hosts;
    
    for (const auto& pair : hosts) {
        auto host = pair.second;
        
        // Skip hosts that don't meet requirements or are already assigned to this map
        if (!host->isEligible(config) || std::find(host->getAssignedMaps().begin(), host->getAssignedMaps().end(), map_name) != host->getAssignedMaps().end()) {
            continue;
        }
        
        // Calculate host score
        double score = calculateHostScore(host);
        eligible_hosts.push_back({host, score});
    }
    
    // Sort hosts by score (descending)
    std::sort(eligible_hosts.begin(), eligible_hosts.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Take the top 'count' hosts
    for (size_t i = 0; i < count && i < eligible_hosts.size(); ++i) {
        backup_hosts.push_back(eligible_hosts[i].first);
    }
    
    return backup_hosts;
}

bool P2PHostManager::migrateMap(const std::string& map_name, uint32 from_host, uint32 to_host) {
    // Check if the map is eligible for P2P hosting
    if (!isMapP2PEligible(map_name)) {
        return false;
    }
    
    // Get the source host
    auto source_host = getHost(from_host);
    if (!source_host) {
        ShowError("Failed to migrate map %s: Source host %d not found\n", map_name.c_str(), from_host);
        return false;
    }
    
    // Unassign map from source host
    if (!source_host->unassignMap(map_name)) {
        ShowError("Failed to unassign map %s from host %d\n", map_name.c_str(), from_host);
        return false;
    }
    
    // If to_host is 0, migrate to VPS
    if (to_host == 0) {
        ShowInfo("Map %s migrated from host %d to VPS\n", map_name.c_str(), from_host);
        
        // Update map_hosts
        auto it = map_hosts.find(map_name);
        if (it != map_hosts.end()) {
            auto& hosts_list = it->second;
            hosts_list.erase(std::remove(hosts_list.begin(), hosts_list.end(), from_host), hosts_list.end());
        }
        
        return true;
    }
    
    // Get the target host
    auto target_host = getHost(to_host);
    if (!target_host) {
        ShowError("Failed to migrate map %s: Target host %d not found\n", map_name.c_str(), to_host);
        return false;
    }
    
    // Assign map to target host
    if (!target_host->assignMap(map_name)) {
        ShowError("Failed to assign map %s to host %d\n", map_name.c_str(), to_host);
        return false;
    }
    
    // Update map_hosts
    auto it = map_hosts.find(map_name);
    if (it != map_hosts.end()) {
        auto& hosts_list = it->second;
        hosts_list.erase(std::remove(hosts_list.begin(), hosts_list.end(), from_host), hosts_list.end());
        hosts_list.insert(hosts_list.begin(), to_host);
    } else {
        map_hosts[map_name] = {to_host};
    }
    
    ShowInfo("Map %s successfully migrated from host %d to host %d\n", map_name.c_str(), from_host, to_host);
    return true;
}

double P2PHostManager::calculateHostScore(const std::shared_ptr<P2PHost>& host) const {
    if (!host) return 0.0;
    
    const auto& stats = host->getStats();
    
    // Calculate score based on performance metrics
    // Higher score = better host
    double cpu_score = (stats.cpu_ghz * stats.cpu_cores) / config.min_cpu_ghz;
    double ram_score = static_cast<double>(stats.free_ram_mb) / config.min_ram_mb;
    double network_score = static_cast<double>(stats.network_speed_mbps) / config.min_network_mbps;
    double latency_score = config.max_latency_ms > 0 ? static_cast<double>(config.max_latency_ms) / (stats.latency_ms + 1) : 1.0;
    double stability_score = config.max_disconnects_per_hour > 0 ? 1.0 - (static_cast<double>(stats.disconnects_per_hour) / config.max_disconnects_per_hour) : 1.0;
    
    // Combine scores with weights
    double score = (cpu_score * 0.2) + (ram_score * 0.2) + (network_score * 0.2) + (latency_score * 0.3) + (stability_score * 0.1);
    
    return score;
}

int P2PHostManager::health_check_timer(int32 tid, t_tick tick, int32 id, intptr_t data) {
    P2PHostManager::getInstance().doHealthCheck();
    return 0;
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
    auto& map_config = rathena::P2PMapConfigParser::getInstance();
    if (!map_config.load("conf/p2p_map_config.conf")) {
        ShowError("Failed to load P2P map configuration\n");
        return false;
    }
    
    const auto& cfg = map_config.getConfig();
    
    // Update host requirements
    config.min_cpu_ghz = cfg.host_requirements.cpu_min_ghz;
    config.min_cpu_cores = cfg.host_requirements.cpu_min_cores;
    config.min_ram_mb = cfg.host_requirements.ram_min_mb;
    config.min_network_mbps = cfg.host_requirements.net_min_speed;
    config.max_latency_ms = cfg.host_requirements.net_max_latency;
    config.max_disconnects_per_hour = cfg.host_requirements.max_disconnects_per_hour;
    
    // Store eligible maps for quick lookup
    p2p_eligible_maps.clear();
    for (const auto& map_pair : cfg.p2p_eligible_maps) {
        if (map_pair.second) {
            p2p_eligible_maps.insert(map_pair.first);
        }
    }
    
    ShowInfo("P2P host configuration loaded successfully\n");
    return true;
}

void P2PHostManager::redistributeLoad() {
    // Get all maps that are currently P2P hosted
    std::vector<std::string> hosted_maps;
    for (const auto& pair : map_hosts) {
        if (!pair.second.empty()) {
            hosted_maps.push_back(pair.first);
        }
    }
    
    // For each map, check if there's a better host available
    for (const auto& map_name : hosted_maps) {
        auto current_host_id = map_hosts[map_name].front();
        auto current_host = getHost(current_host_id);
        
        if (!current_host) continue;
        
        // Find the best host for this map
        std::shared_ptr<P2PHost> best_host = nullptr;
        double best_score = calculateHostScore(current_host);
        
        for (const auto& pair : hosts) {
            auto host = pair.second;
            double score = calculateHostScore(host);
            
            if (score > best_score * 1.2) { // Only migrate if new host is at least 20% better
                best_host = host;
                best_score = score;
            }
        }
    }
}

void P2PHostManager::updateHostMetrics() {
}
} // namespace rathena