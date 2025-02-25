#include "p2p_commands.hpp"
#include "../common/showmsg.hpp"
#include "../common/nullpo.hpp"
#include "../common/strlib.hpp"
#include "pc.hpp"
#include "clif.hpp"
#include "packets.hpp"
#include "atcommand.hpp"
#include "p2p_host.hpp"
#include "p2p_map.hpp"

using namespace rathena;

// Helper function to format byte sizes
static std::string formatBytes(uint64 bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double size = bytes;
    
    while (size > 1024 && i < 3) {
        size /= 1024;
        i++;
    }
    
    char buf[32];
    safesnprintf(buf, sizeof(buf), "%.2f %s", size, units[i]);
    return std::string(buf);
}

/*==========================================
 * @p2pstatus - Show P2P system status
 *------------------------------------------*/
int atcommand_p2pstatus(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    nullpo_retr(-1, sd);
    
    auto& host_mgr = P2PHostManager::getInstance();
    clif_displaymessage(fd, "==== P2P System Status ====");
    
    char msg[CHAT_SIZE_MAX];
    // Show VPS connection status
    safesnprintf(msg, sizeof(msg), "VPS Connection: %s", 
        host_mgr.isVPSConnected() ? "^00FF00Online^000000" : "^FF0000Offline^000000");
    clif_displaymessage(fd, msg);
    
    // Show active hosts
    auto host = host_mgr.getHost(sd->status.account_id);
    if (host) {
        const auto& stats = host->getStats();
        clif_displaymessage(fd, "Your Host Stats:");
        
        safesnprintf(msg, sizeof(msg), "CPU: %.1f GHz (%d cores)", stats.cpu_ghz, stats.cpu_cores);
        clif_displaymessage(fd, msg);
        
        safesnprintf(msg, sizeof(msg), "RAM: %s free", 
            formatBytes(stats.free_ram_mb * 1024 * 1024).c_str());
        clif_displaymessage(fd, msg);
        
        safesnprintf(msg, sizeof(msg), "Network: %d Mbps (Latency: %d ms)", 
            stats.network_speed_mbps, stats.latency_ms);
        clif_displaymessage(fd, msg);
        
        if (!host->getAssignedMaps().empty()) {
            clif_displaymessage(fd, "Hosting Maps:");
            for (const auto& map : host->getAssignedMaps()) {
                safesnprintf(msg, sizeof(msg), "- %s", map.c_str());
                clif_displaymessage(fd, msg);
            }
        }
    }
    return 0;
}

/*==========================================
 * @p2plist - List all active P2P hosts
 *------------------------------------------*/
int atcommand_p2plist(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    nullpo_retr(-1, sd);
    
    auto& host_mgr = P2PHostManager::getInstance();
    clif_displaymessage(fd, "==== Active P2P Hosts ====");
    
    int host_count = 0;
    char msg[CHAT_SIZE_MAX];

    for (const auto& pair : host_mgr.getHosts()) {
        auto host = pair.second;
        if (host->getStatus() == P2PHost::Status::ACTIVE) {
            const auto& stats = host->getStats();
            safesnprintf(msg, sizeof(msg), "Host #%d (Account ID: %d)", 
                ++host_count, host->getAccountId());
            clif_displaymessage(fd, msg);
            
            safesnprintf(msg, sizeof(msg), "CPU: %.1f GHz (%d cores), RAM: %s free", 
                stats.cpu_ghz, stats.cpu_cores, 
                formatBytes(stats.free_ram_mb * 1024 * 1024).c_str());
            clif_displaymessage(fd, msg);
            
            if (!stats.regional_latencies.empty()) {
                clif_displaymessage(fd, "Regional Latencies:");
                for (const auto& latency : stats.regional_latencies) {
                    safesnprintf(msg, sizeof(msg), "- %s: %d ms", 
                        latency.region.c_str(), latency.latency_ms);
                    clif_displaymessage(fd, msg);
                }
            }
            
            if (!host->getAssignedMaps().empty()) {
                clif_displaymessage(fd, "Hosting Maps:");
                for (const auto& map : host->getAssignedMaps()) {
                    safesnprintf(msg, sizeof(msg), "- %s", map.c_str());
                    clif_displaymessage(fd, msg);
                }
            }
            clif_displaymessage(fd, "---------------");
        }
    }
    
    safesnprintf(msg, sizeof(msg), "Total Active Hosts: %d", host_count);
    clif_displaymessage(fd, msg);
    
    return 0;
}

/*==========================================
 * @p2pvalidate - Run security validation
 *------------------------------------------*/
int atcommand_p2pvalidate(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    nullpo_retr(-1, sd);
    
    if (!pc_has_permission(sd, PC_PERM_RECEIVE_REQUESTS))
        return -1;
    
    auto& host_mgr = P2PHostManager::getInstance();
    clif_displaymessage(fd, "==== Security Validation ====");
    
    char msg[CHAT_SIZE_MAX];

    for (const auto& pair : host_mgr.getHosts()) {
        auto host = pair.second;
        if (host->getStatus() == P2PHost::Status::ACTIVE) {
            safesnprintf(msg, sizeof(msg), "Validating Host %d...", host->getAccountId());
            clif_displaymessage(fd, msg);
            
            host->validateHosting(P2PHost::ValidationType::SPAWN_RATE);
            host->validateHosting(P2PHost::ValidationType::DROP_RATE);
            host->validateHosting(P2PHost::ValidationType::MONSTER_STATS);
            host->validateHosting(P2PHost::ValidationType::PLAYER_MOVEMENT);
            host->validateHosting(P2PHost::ValidationType::SKILL_USAGE);
            
            const auto& security = host->getSecurityStatus();
            clif_displaymessage(fd, "Results:");
            
            safesnprintf(msg, sizeof(msg), "- Spawn Rates: %s", 
                security.spawn_rate_valid ? "^00FF00Valid^000000" : "^FF0000Invalid^000000");
            clif_displaymessage(fd, msg);
            
            safesnprintf(msg, sizeof(msg), "- Drop Rates: %s", 
                security.drop_rate_valid ? "^00FF00Valid^000000" : "^FF0000Invalid^000000");
            clif_displaymessage(fd, msg);
            
            safesnprintf(msg, sizeof(msg), "- Monster Stats: %s", 
                security.monster_stats_valid ? "^00FF00Valid^000000" : "^FF0000Invalid^000000");
            clif_displaymessage(fd, msg);
            
            safesnprintf(msg, sizeof(msg), "- Player Movement: %s", 
                security.player_movement_valid ? "^00FF00Valid^000000" : "^FF0000Invalid^000000");
            clif_displaymessage(fd, msg);
            
            safesnprintf(msg, sizeof(msg), "- Skill Usage: %s", 
                security.skill_usage_valid ? "^00FF00Valid^000000" : "^FF0000Invalid^000000");
            clif_displaymessage(fd, msg);
            
            clif_displaymessage(fd, "---------------");
        }
    }
    return 0;
}

/*==========================================
 * @p2pmetrics - Show performance metrics
 *------------------------------------------*/
int atcommand_p2pmetrics(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
    nullpo_retr(-1, sd);
    
    if (!pc_has_permission(sd, PC_PERM_RECEIVE_REQUESTS))
        return -1;
    
    auto& host_mgr = P2PHostManager::getInstance();
    auto& map_server = P2PMapServer::getInstance();
    
    clif_displaymessage(fd, "==== P2P Performance Metrics ====");
    
    int total_hosts = 0;
    int active_hosts = 0;
    int total_p2p_maps = 0;
    char msg[CHAT_SIZE_MAX];
    
    for (const auto& pair : host_mgr.getHosts()) {
        total_hosts++;
        if (pair.second->getStatus() == P2PHost::Status::ACTIVE) {
            active_hosts++;
            total_p2p_maps += pair.second->getAssignedMaps().size();
        }
    }
    
    safesnprintf(msg, sizeof(msg), "Total Hosts: %d (Active: %d)", total_hosts, active_hosts);
    clif_displaymessage(fd, msg);
    
    safesnprintf(msg, sizeof(msg), "P2P-Hosted Maps: %d", total_p2p_maps);
    clif_displaymessage(fd, msg);
    
    clif_displaymessage(fd, "\nLoad Distribution:");
    for (const auto& pair : host_mgr.getHosts()) {
        auto host = pair.second;
        if (host->getStatus() == P2PHost::Status::ACTIVE) {
            const auto& stats = host->getStats();
            safesnprintf(msg, sizeof(msg), "Host %d - CPU: %.1f%%, RAM: %s used, Maps: %zu", 
                host->getAccountId(),
                (stats.cpu_ghz * stats.cpu_cores) / 
                (host_mgr.getConfig().min_cpu_ghz * host_mgr.getConfig().min_cpu_cores) * 100.0,
                formatBytes(stats.free_ram_mb * 1024 * 1024).c_str(),
                host->getAssignedMaps().size());
            clif_displaymessage(fd, msg);
        }
    }
    return 0;
}

// Initialize P2P commands
void p2p_commands_init() {
    add_atcommand("p2pstatus", atcommand_p2pstatus);
    add_atcommand("p2plist", atcommand_p2plist);
    add_atcommand("p2pvalidate", atcommand_p2pvalidate);
    add_atcommand("p2pmetrics", atcommand_p2pmetrics);
}