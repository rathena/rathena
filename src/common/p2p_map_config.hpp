#ifndef P2P_MAP_CONFIG_HPP
#define P2P_MAP_CONFIG_HPP

#include "cbasetypes.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace rathena {

/**
 * Configuration structure for P2P map hosting
 */
struct P2PMapConfig {
    // General settings
    struct {
        bool enable_p2p_maps;
        bool prefer_low_latency;
        bool enable_auto_migration;
        bool enable_vps_fallback;
        uint16 data_check_interval;
        bool enable_p2p_security_checks;
        bool persistent_hosts;
    } general;

    // Host requirements
    struct {
        double cpu_min_ghz;
        uint16 cpu_min_cores;
        uint64 ram_min_mb;
        uint32 net_min_speed;
        uint16 net_max_latency;
        uint16 max_disconnects_per_hour;
    } host_requirements;

    // Map eligibility
    std::map<std::string, bool> p2p_eligible_maps;

    // Task distribution
    struct {
        std::map<std::string, uint8> task_priorities;
        uint16 max_tasks_per_node;
        uint16 reassignment_interval;
    } task_distribution;

    // Failover settings
    struct {
        uint8 backup_hosts_count;
        uint16 migration_timeout;
        uint16 sync_interval;
        bool always_keep_vps_ready;
    } failover;

    // Monitoring settings
    struct {
        uint16 health_check_interval;
        uint16 metrics_interval;
        bool detailed_logging;
        bool alert_on_failure;
    } monitoring;
};

/**
 * Parser for P2P map hosting configuration
 */
class P2PMapConfigParser {
public:
    /**
     * Get the singleton instance
     * @return Reference to the singleton instance
     */
    static P2PMapConfigParser& getInstance();

    /**
     * Load configuration from file
     * @param filename Path to the configuration file
     * @return True if loading was successful, false otherwise
     */
    bool load(const char* filename);

    /**
     * Reload configuration from the last loaded file
     */
    void reload();

    /**
     * Get the current configuration
     * @return Reference to the current configuration
     */
    const P2PMapConfig& getConfig() const;

private:
    P2PMapConfigParser();
    ~P2PMapConfigParser() = default;
    P2PMapConfigParser(const P2PMapConfigParser&) = delete;
    P2PMapConfigParser& operator=(const P2PMapConfigParser&) = delete;

    P2PMapConfig config;
    std::string last_filename;
    static P2PMapConfigParser* instance;
};

} // namespace rathena

#endif // P2P_MAP_CONFIG_HPP