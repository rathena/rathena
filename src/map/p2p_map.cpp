#include "p2p_map.hpp"
#include <algorithm>
#include "../common/showmsg.hpp"
#include "../common/timer.hpp"
#include "pc.hpp"
#include "battle.hpp"
#include "clif.hpp"

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
    return true;
}

void P2PMapServer::final() {
    ShowInfo("Finalizing P2P Map Server\n");
    delete instance;
    instance = nullptr;
}

bool P2PMapServer::onPlayerEnterMap(struct map_session_data* sd, const std::string& map_name) {
    if (!sd) return false;

    // Check if map can be P2P hosted
    auto& host_mgr = P2PHostManager::getInstance();
    if (!host_mgr.isMapP2PEligible(map_name)) {
        // For VPS-hosted maps, check if we can provide P2P assistance
        return processP2PAssistance(map_name);
    }

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
}

bool P2PMapServer::canPlayerHost(struct map_session_data* sd) {
    if (!sd) return false;

    P2PHostStats stats;
    if (!collectPlayerMetrics(sd, stats)) return false;

    return P2PHostManager::getInstance().getConfig().min_cpu_ghz <= stats.cpu_ghz &&
           P2PHostManager::getInstance().getConfig().min_cpu_cores <= stats.cpu_cores &&
           P2PHostManager::getInstance().getConfig().min_ram_mb <= stats.free_ram_mb &&
           P2PHostManager::getInstance().getConfig().min_network_mbps <= stats.network_speed_mbps;
}

bool P2PMapServer::registerPlayerAsHost(struct map_session_data* sd) {
    if (!sd) return false;

    P2PHostStats stats;
    if (!collectPlayerMetrics(sd, stats)) return false;

    return P2PHostManager::getInstance().registerHost(sd->status.account_id, stats);
}

void P2PMapServer::unregisterPlayerHost(struct map_session_data* sd) {
    if (!sd) return;
    P2PHostManager::getInstance().unregisterHost(sd->status.account_id);
}

bool P2PMapServer::handleVPSFallback(const std::string& map_name) {
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

    ShowInfo("Map %s falling back to VPS hosting\n", map_name.c_str());
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

    // If no backup host available, fall back to VPS
    return handleVPSFallback(map_name);
}

bool P2PMapServer::validateMapData(const std::string& map_name, uint32 host_id) {
    // In a real implementation, this would validate map state, monster positions, etc.
    // For now, we'll just return true as a placeholder
    return true;
}

} // namespace rathena