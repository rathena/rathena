#ifndef P2P_HOST_HPP
#define P2P_HOST_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <set>
#include "../common/timer.hpp"
#include "../common/cbasetypes.hpp"
#include "../common/db.hpp"
#include "../common/showmsg.hpp"
#include "map.hpp"

namespace rathena {

struct RegionalLatency {
    std::string region;
    uint16 latency_ms;
    std::string relay_address;
};

struct P2PHostStats {
    double cpu_ghz;
    uint16 cpu_cores;
    uint64 free_ram_mb;
    uint32 network_speed_mbps;
    uint16 latency_ms;
    uint32 disconnects_per_hour;
    uint16 jitter_ms;                // Network jitter (variation in latency)
    uint32 packet_rate;              // Packets processed per second
    std::chrono::system_clock::time_point last_health_check;
    uint16 consecutive_failed_checks;
    std::vector<RegionalLatency> regional_latencies;
    uint16 frame_time_ms;            // Average frame processing time
    bool vps_connection_active;
};

struct P2PHostConfig {
    double min_cpu_ghz;
    uint16 min_cpu_cores;
    uint64 min_ram_mb;
    uint32 min_network_mbps;
    uint16 max_latency_ms;
    uint16 max_disconnects_per_hour;
};

struct SecurityValidation {
    bool spawn_rate_valid;
    bool drop_rate_valid;
    bool monster_stats_valid;
    bool player_movement_valid;
    bool skill_usage_valid;
    uint32 validation_timestamp;
};

class P2PHostManager;

class P2PHost {
    friend class P2PHostManager;
public:
    enum class Status {
        ACTIVE,
        STANDBY,
        MIGRATING,
        DISCONNECTED
    };

    enum class ValidationType {
        SPAWN_RATE,
        DROP_RATE,
        MONSTER_STATS,
        PLAYER_MOVEMENT,
        SKILL_USAGE
    };

    P2PHost(uint32 account_id, const P2PHostStats& stats);
    ~P2PHost();

    bool isEligible(const P2PHostConfig& config) const;
    bool updateStats(const P2PHostStats& new_stats);
    bool assignMap(const std::string& map_name);
    bool unassignMap(const std::string& map_name);
    Status getStatus() const { return status; }
    uint32 getAccountId() const { return account_id; }
    const P2PHostStats& getStats() const { return stats; }
    const std::vector<std::string>& getAssignedMaps() const { return assigned_maps; }
    
    // Security validation methods
    bool validateHosting(ValidationType type);
    bool isSecurityValid() const;
    const SecurityValidation& getSecurityStatus() const { return security; }

protected:
    // Protected validation methods
    bool validateSpawnRates();
    bool validateDropRates();
    bool validateMonsterStats();
    bool validatePlayerMovement();
    bool validateSkillUsage();

private:
    uint32 account_id;
    P2PHostStats stats;
    Status status;
    std::vector<std::string> assigned_maps;
    std::chrono::system_clock::time_point last_update;
    SecurityValidation security;
};

class P2PHostManager {
public:
    static P2PHostManager& getInstance();

    bool init();
    void final();
    
    bool registerHost(uint32 account_id, const P2PHostStats& stats);
    bool unregisterHost(uint32 account_id);
    
    std::shared_ptr<P2PHost> getBestHostForMap(const std::string& map_name);
    std::vector<std::shared_ptr<P2PHost>> getBackupHostsForMap(const std::string& map_name, size_t count);
    
    bool migrateMap(const std::string& map_name, uint32 from_host, uint32 to_host);
    bool isMapP2PEligible(const std::string& map_name) const;
    
    void doHealthCheck();
    void loadPersistentData();
    void savePersistentData();

    const P2PHostConfig& getConfig() const { return config; }
    void setConfig(const P2PHostConfig& new_config) { config = new_config; }

    // Get a specific host
    std::shared_ptr<P2PHost> getHost(uint32 account_id) const {
        auto it = hosts.find(account_id);
        return (it != hosts.end()) ? it->second : nullptr;
    }

    // Get all hosts
    const std::map<uint32, std::shared_ptr<P2PHost>>& getHosts() const { return hosts; }

    // VPS connection status
    bool isVPSConnected() const { return vps_connected; }
    void setVPSConnected(bool connected) { vps_connected = connected; }

    // Regional latency methods
    bool updateRegionalLatencies(uint32 account_id, const std::vector<RegionalLatency>& latencies);
    std::vector<RegionalLatency> getRegionalLatencies(uint32 account_id) const;

    // Calculate host score based on performance metrics
    double calculateHostScore(const std::shared_ptr<P2PHost>& host) const;

    static int health_check_timer(int32 tid, t_tick tick, int32 id, intptr_t data);

private:
    P2PHostManager();
    ~P2PHostManager();

    bool loadConfig();
    void redistributeLoad();
    void updateHostMetrics();
    void performSecurityValidation();
    
    std::map<uint32, std::shared_ptr<P2PHost>> hosts;
    std::map<std::string, std::vector<uint32>> map_hosts;
    std::set<std::string> p2p_eligible_maps;
    P2PHostConfig config;
    bool vps_connected;
    
    static P2PHostManager* instance;
    static const char* PERSISTENCE_FILE;
};

} // namespace rathena

#endif // P2P_HOST_HPP