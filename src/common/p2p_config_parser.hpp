#ifndef P2P_CONFIG_PARSER_HPP
#define P2P_CONFIG_PARSER_HPP

#include "cbasetypes.hpp"
#include "conf.hpp"
#include <string>
#include <memory>

class P2PConfigParser {
public:
    struct NetworkMonitorConfig {
        bool enabled;
        uint32 update_interval;
        uint32 cleanup_interval;
        struct {
            uint32 store_duration;
            uint32 alert_timeout;
            uint32 max_samples;
        } metrics;
    };

    struct SecurityConfig {
        struct {
            bool enabled;
            uint32 packet_rate;
            uint32 connection_rate;
            uint32 bandwidth_limit;
            uint32 burst_threshold;
            uint32 blacklist_duration;
        } ddos_protection;

        struct {
            bool enabled;
            uint32 max_size;
            bool encryption;
            bool checksum;
        } packet_validation;

        struct {
            bool enabled;
            uint32 scan_interval;
            uint32 alert_threshold;
            bool auto_blacklist;
        } threat_detection;
    };

    struct HostRequirements {
        struct {
            uint32 cpu_cores;
            uint32 ram_gb;
            uint32 bandwidth_mbps;
            uint32 storage_gb;
            uint32 uptime_percent;
        } minimum;

        struct {
            uint32 max_maps;
            uint32 max_players;
            uint32 max_instances;
            uint32 bandwidth_limit;
            uint32 storage_limit;
        } limits;
    };

    struct OptimizationConfig {
        struct {
            bool enabled;
            uint32 threshold;
            uint8 level;
        } compression;

        struct {
            bool enabled;
            uint8 levels;
            uint32 queue_size;
        } packet_priority;

        struct {
            bool enabled;
            uint32 size;
            uint32 timeout;
        } connection_pool;
    };

    struct ThresholdConfig {
        struct {
            uint8 cpu_usage;
            uint8 memory_usage;
            uint8 network_usage;
            uint8 packet_loss;
            uint32 latency;
        } critical;

        struct {
            uint8 cpu_usage;
            uint8 memory_usage;
            uint8 network_usage;
            uint8 packet_loss;
            uint32 latency;
        } warning;
    };

    struct DatabaseConfig {
        struct {
            uint32 performance_data;
            uint32 security_events;
            uint32 metrics;
            uint32 alerts;
        } cleanup;

        struct {
            bool enabled;
            uint32 interval;
            uint32 keep_count;
        } backup;
    };

    struct LoggingConfig {
        bool security_events;
        bool performance_metrics;
        bool network_stats;
        bool host_status;
        bool debug_info;
    };

    struct NotificationConfig {
        struct {
            bool enabled;
            std::string server;
            uint16 port;
            std::string username;
            std::string password;
            std::string from;
            std::string to;
        } email;

        struct {
            bool enabled;
            std::string webhook_url;
            std::string username;
            std::string avatar_url;
        } discord;
    };

    static P2PConfigParser& getInstance();

    bool load(const char* filename);
    void reload();

    const NetworkMonitorConfig& getNetworkMonitorConfig() const;
    const SecurityConfig& getSecurityConfig() const;
    const HostRequirements& getHostRequirements() const;
    const OptimizationConfig& getOptimizationConfig() const;
    const ThresholdConfig& getThresholdConfig() const;
    const DatabaseConfig& getDatabaseConfig() const;
    const LoggingConfig& getLoggingConfig() const;
    const NotificationConfig& getNotificationConfig() const;

private:
    P2PConfigParser() = default;
    ~P2PConfigParser() = default;
    P2PConfigParser(const P2PConfigParser&) = delete;
    P2PConfigParser& operator=(const P2PConfigParser&) = delete;

    void parseNetworkMonitor(config_setting_t* setting);
    void parseSecurity(config_setting_t* setting);
    void parseHostRequirements(config_setting_t* setting);
    void parseOptimization(config_setting_t* setting);
    void parseThresholds(config_setting_t* setting);
    void parseDatabase(config_setting_t* setting);
    void parseLogging(config_setting_t* setting);
    void parseNotifications(config_setting_t* setting);

    NetworkMonitorConfig network_monitor_;
    SecurityConfig security_;
    HostRequirements host_requirements_;
    OptimizationConfig optimization_;
    ThresholdConfig thresholds_;
    DatabaseConfig database_;
    LoggingConfig logging_;
    NotificationConfig notifications_;

    std::unique_ptr<config_t> config_;
    bool initialized_ = false;
};

#endif // P2P_CONFIG_PARSER_HPP