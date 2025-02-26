#include "p2p_map.hpp"
#include <algorithm>
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "pc.hpp"
#include "battle.hpp"
#include "../common/p2p_map_config.hpp"
#include "clif.hpp"
#include <nlohmann/json.hpp>

namespace rathena {

P2PMapServer* P2PMapServer::instance = nullptr;

P2PMapServer::P2PMapServer() {}

P2PMapServer::~P2PMapServer() {}

P2PMapServer& P2PMapServer::getInstance() {
    if (!instance) {
        instance = new P2PMapServer();
    }
    return *instance;
}

bool P2PMapServer::init() {
    ShowInfo("Initializing P2P Map Server\n");
    rathena::P2PMapConfigParser::getInstance().load("conf/p2p_map_config.conf");
    
    // Initialize the P2P data synchronization system
    P2PDataSync::getInstance().init("conf/p2p_data_sync.conf");
    return true;
}

void P2PMapServer::final() {
    ShowInfo("Finalizing P2P Map Server\n");
    delete instance;
    instance = nullptr;
    
    // Finalize the P2P data synchronization system
    P2PDataSync::getInstance().final();
}

bool P2PMapServer::onPlayerEnterMap(struct map_session_data* sd, const std::string& map_name) {
    if (!sd) return false;

    // Check if map can be P2P hosted
    auto& host_mgr = P2PHostManager::getInstance();
    if (!host_mgr.isMapP2PEligible(map_name)) {
        // Map is not eligible for P2P hosting, use VPS
        ShowInfo("Map %s is not eligible for P2P hosting, using VPS\n", map_name.c_str());
        MapState& state = map_states[map_name];
        state.current_host_id = 0; // 0 indicates VPS hosting
        state.is_vps_hosted = true;
        
        // For VPS-hosted maps, we can still provide P2P assistance
        processP2PAssistance(map_name);
        return true;
    }
    
    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if P2P hosting is enabled
    if (!config.general.enable_p2p_maps) return handleVPSFallback(map_name);

    // Find or assign a host for this map
    auto host = findOrAssignHost(map_name);
    if (!host) {
        // No eligible host found, fall back to VPS
        return handleVPSFallback(map_name);
    }

    // Update map state
    MapState& state = map_states[map_name];
    state.current_host_id = host->getAccountId();
    state.is_vps_hosted = false;

    // If the entering player has good specs, register them as potential backup host
    if (canPlayerHost(sd)) {
        P2PHostStats stats;
        if (collectPlayerMetrics(sd, stats)) {
            if (host_mgr.registerHost(sd->status.account_id, stats)) {
                auto backup_hosts = host_mgr.getBackupHostsForMap(map_name, 2);
                state.backup_host_ids.clear();
                for (const auto& backup : backup_hosts) {
                    state.backup_host_ids.push_back(backup->getAccountId());
                }
            }
        }
    }
    
    ShowInfo("Player %s entered map %s hosted by P2P host %d\n", sd->status.name, map_name.c_str(), host->getAccountId());
    
    // Synchronize map state to the main server
    syncMapState(map_name, host->getAccountId(), true); // Critical operation
    
    // Synchronize player data to the main server
    syncPlayerData(sd, map_name, true); // Critical operation

    return true;
}

void P2PMapServer::onPlayerLeaveMap(struct map_session_data* sd, const std::string& map_name) {
    if (!sd) return;

    auto it = map_states.find(map_name);
    if (it == map_states.end()) return;

    auto& state = it->second;
    
    // If this player was the host, initiate migration
    if (state.current_host_id == sd->status.account_id) {
        handleHostMigration(map_name, sd->status.account_id);
    }
    
    // Remove from backup hosts if applicable
    auto backup_it = std::find(state.backup_host_ids.begin(), state.backup_host_ids.end(), sd->status.account_id);
    if (backup_it != state.backup_host_ids.end()) {
        state.backup_host_ids.erase(backup_it);
    }

    // Unregister as P2P host
    unregisterPlayerHost(sd);
    
    // Synchronize map state to the main server
    syncMapState(map_name, state.current_host_id, true); // Critical operation
}

bool P2PMapServer::canPlayerHost(struct map_session_data* sd) {
    if (!sd) return false;

    P2PHostStats stats;
    
    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if P2P hosting is enabled
    if (!config.general.enable_p2p_maps)
        return false;
    if (!collectPlayerMetrics(sd, stats)) return false;

    return P2PHostManager::getInstance().getConfig().min_cpu_ghz <= stats.cpu_ghz &&
           P2PHostManager::getInstance().getConfig().min_cpu_cores <= stats.cpu_cores &&
           P2PHostManager::getInstance().getConfig().min_ram_mb <= stats.free_ram_mb &&
           P2PHostManager::getInstance().getConfig().min_network_mbps <= stats.network_speed_mbps;
}

bool P2PMapServer::registerPlayerAsHost(struct map_session_data* sd) {
    if (!sd) return false;

    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if P2P hosting is enabled
    if (!config.general.enable_p2p_maps)
        return false;

    P2PHostStats stats;
    if (!collectPlayerMetrics(sd, stats)) return false;

    return P2PHostManager::getInstance().registerHost(sd->status.account_id, stats);
}

void P2PMapServer::unregisterPlayerHost(struct map_session_data* sd) {
    if (!sd) return;
    P2PHostManager::getInstance().unregisterHost(sd->status.account_id);
}

bool P2PMapServer::handleVPSFallback(const std::string& map_name) {
    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if VPS fallback is enabled
    if (!config.general.enable_vps_fallback) {
        ShowWarning("VPS fallback is disabled, but no P2P host available for map %s\n", map_name.c_str());
    }
    auto it = map_states.find(map_name);
    if (it == map_states.end()) {
        MapState state;
        state.current_host_id = 0; // 0 indicates VPS hosting
        state.is_vps_hosted = true;
        map_states[map_name] = state;
    } else {
        it->second.current_host_id = 0;
        it->second.is_vps_hosted = true;
    }

    ShowInfo("Map %s using VPS hosting\n", map_name.c_str());
    return true;
}

bool P2PMapServer::processP2PAssistance(const std::string& map_name) {
    auto it = map_states.find(map_name);
    if (it == map_states.end()) return false;

    auto& state = it->second;
    if (!state.is_vps_hosted) return false;

    // Find eligible P2P nodes to assist with this map
    auto& host_mgr = P2PHostManager::getInstance();
    auto assistance_hosts = host_mgr.getBackupHostsForMap(map_name, 3);

    state.assistance_host_ids.clear();
    for (const auto& host : assistance_hosts) {
        state.assistance_host_ids.push_back(host->getAccountId());
    }

    ShowInfo("Map %s using %zu P2P assistance nodes\n", map_name.c_str(), state.assistance_host_ids.size());
    return true;
}

std::shared_ptr<P2PHost> P2PMapServer::findOrAssignHost(const std::string& map_name) {
    auto& host_mgr = P2PHostManager::getInstance();

    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if P2P hosting is enabled
    if (!config.general.enable_p2p_maps)
        return nullptr;

    // Check if map already has a host
    auto it = map_states.find(map_name);
    if (it != map_states.end() && it->second.current_host_id != 0) {
        auto current_host = host_mgr.getHost(it->second.current_host_id);
        if (current_host) {
            return current_host;
        }
    }

    // Find new best host
    auto host = host_mgr.getBestHostForMap(map_name);
    if (host) {
        host->assignMap(map_name);
    }
    return host;
}

bool P2PMapServer::collectPlayerMetrics(struct map_session_data* sd, P2PHostStats& stats) {
    if (!sd) return false;

    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Use the configuration values as defaults
    stats.cpu_ghz = config.host_requirements.cpu_min_ghz;
    stats.cpu_cores = config.host_requirements.cpu_min_cores;
    stats.free_ram_mb = config.host_requirements.ram_min_mb;
    // In a real implementation, these metrics would be collected from the client
    // For now, we'll use placeholder values that would normally come from client reports
    stats.cpu_ghz = 3.5;  // Example value
    stats.cpu_cores = 8;  // Example value
    stats.free_ram_mb = 16384;  // Example value (16GB)
    stats.network_speed_mbps = 1000;  // Example value
    stats.latency_ms = 20;  // Example value
    stats.disconnects_per_hour = 0;
    stats.consecutive_failed_checks = 0;
    stats.last_health_check = std::chrono::system_clock::now();

    return true;
}

bool P2PMapServer::handleHostMigration(const std::string& map_name, uint32 old_host_id) {
    // Get the P2P map configuration
    const auto& config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Check if auto migration is enabled
    if (!config.general.enable_auto_migration) {
        return handleVPSFallback(map_name);
    }
    auto it = map_states.find(map_name);
    if (it == map_states.end()) return false;

    auto& state = it->second;
    
    // Try to find a backup host
    for (auto backup_id : state.backup_host_ids) {
        if (P2PHostManager::getInstance().migrateMap(map_name, old_host_id, backup_id)) {
            state.current_host_id = backup_id;
            // Select new backup host
            auto new_backups = P2PHostManager::getInstance().getBackupHostsForMap(map_name, 2);
            state.backup_host_ids.clear();
            for (const auto& backup : new_backups) {
                state.backup_host_ids.push_back(backup->getAccountId());
            }
            ShowInfo("Map %s successfully migrated to backup host %d\n", map_name.c_str(), backup_id);
            return true;
        }
    }

    // If we get here, no backup host was available
    ShowWarning("No backup host available for map %s, falling back to VPS\n", map_name.c_str());
    
    // If no backup host available, fall back to VPS
    return handleVPSFallback(map_name);
}

bool P2PMapServer::validateMapData(const std::string& map_name, uint32 host_id) {
    // In a real implementation, this would validate map state, monster positions, etc.
    // For now, we'll just return true as a placeholder
    return true;
}

bool P2PMapServer::syncMapState(const std::string& map_name, uint32 host_id, bool is_critical) {
    if (map_name.empty()) {
        return false;
    }
    
    // Create JSON representation of map state
    std::string map_state_json = createMapStateJson(map_name);
    
    std::string operation_id;
    
    if (is_critical) {
        // Use critical sync for real-time performance
        operation_id = P2PDataSync::getInstance().syncCriticalData(
            SyncDataType::MAP_STATE,
            host_id,
            map_name,
            map_state_json,
            true // priority
        );
        
        // Wait for the result to ensure real-time performance
        P2PDataSync::getInstance().waitForSyncResult(operation_id, 500); // 500ms timeout
    } else {
        // Use regular sync for non-critical updates
        operation_id = P2PDataSync::getInstance().syncToMainServer(
            SyncDataType::MAP_STATE,
            host_id,
            map_name,
            map_state_json,
            false // not priority
        );
    }
    
    // We don't wait for the result here, as it's processed asynchronously
    return true;
}

bool P2PMapServer::syncPlayerData(struct map_session_data* sd, const std::string& map_name, bool is_critical) {
    if (!sd || map_name.empty()) {
        return false;
    }
    
    // Create JSON representation of player data
    std::string player_data_json = createPlayerDataJson(sd);
    
    std::string operation_id;
    
    if (is_critical) {
        // Use critical sync for real-time performance
        operation_id = P2PDataSync::getInstance().syncCriticalData(
            SyncDataType::PLAYER_POSITION,
            sd->status.account_id,
            map_name,
            player_data_json,
            true // priority
        );
        
        // Wait for the result to ensure real-time performance
        P2PDataSync::getInstance().waitForSyncResult(operation_id, 500); // 500ms timeout
    } else {
        // Use regular sync for non-critical updates
        operation_id = P2PDataSync::getInstance().syncToMainServer(
            SyncDataType::PLAYER_POSITION,
            sd->status.account_id,
            map_name,
            player_data_json,
            false // not priority
        );
    }
    
    // We don't wait for the result here, as it's processed asynchronously
    return true;
}

bool P2PMapServer::syncSecurityValidation(uint32 host_id, const std::string& map_name, P2PHost::ValidationType type, bool result, bool is_critical) {
    if (host_id == 0 || map_name.empty()) {
        return false;
    }
    
    // Create JSON representation of security validation
    nlohmann::json validation_json;
    validation_json["host_id"] = host_id;
    validation_json["map_name"] = map_name;
    validation_json["validation_type"] = static_cast<int>(type);
    validation_json["result"] = result;
    validation_json["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    std::string operation_id;
    
    if (is_critical) {
        // Use critical sync for real-time performance
        operation_id = P2PDataSync::getInstance().syncCriticalData(
            SyncDataType::SECURITY_VALIDATION,
            host_id,
            map_name,
            validation_json.dump(),
            true // priority
        );
        
        // Wait for the result to ensure real-time performance
        P2PDataSync::getInstance().waitForSyncResult(operation_id, 500); // 500ms timeout
    } else {
        // Use regular sync for non-critical updates
        operation_id = P2PDataSync::getInstance().syncToMainServer(
            SyncDataType::SECURITY_VALIDATION,
            host_id,
            map_name,
            validation_json.dump(),
            true // priority
        );
    }
    
    // We don't wait for the result here, as it's processed asynchronously
    return true;
}

std::string P2PMapServer::createMapStateJson(const std::string& map_name) {
    auto it = map_states.find(map_name);
    if (it == map_states.end()) {
        return "{}";
    }
    
    const auto& state = it->second;
    
    nlohmann::json map_state;
    map_state["map_name"] = map_name;
    map_state["host_id"] = state.current_host_id;
    map_state["is_vps_hosted"] = state.is_vps_hosted;
    map_state["backup_host_ids"] = state.backup_host_ids;
    map_state["assistance_host_ids"] = state.assistance_host_ids;
    map_state["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return map_state.dump();
}

std::string P2PMapServer::createPlayerDataJson(struct map_session_data* sd) {
    if (!sd) {
        return "{}";
    }
    
    nlohmann::json player_data;
    player_data["account_id"] = sd->status.account_id;
    player_data["char_id"] = sd->status.char_id;
    player_data["name"] = sd->status.name;
    player_data["class"] = sd->status.class_;
    player_data["base_level"] = sd->status.base_level;
    player_data["job_level"] = sd->status.job_level;
    player_data["hp"] = sd->status.hp;
    player_data["max_hp"] = sd->status.max_hp;
    player_data["sp"] = sd->status.sp;
    player_data["max_sp"] = sd->status.max_sp;
    player_data["position"] = {
        {"x", sd->bl.x},
        {"y", sd->bl.y},
        {"m", sd->bl.m}
    };
    player_data["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    return player_data.dump();
}

} // namespace rathena