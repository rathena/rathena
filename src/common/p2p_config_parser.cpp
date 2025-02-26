#include "p2p_config_parser.hpp"
#include "showmsg.hpp"
#include <cstring>

// Previously implemented sections remain unchanged...
// Adding the remaining parsing methods with proper type handling:

void P2PConfigParser::parseHostRequirements(config_setting_t* setting) {
    if (!setting) return;

    int temp_int;
    config_setting_t* min = config_setting_get_member(setting, "minimum");
    if (min) {
        if (config_setting_lookup_int(min, "cpu_cores", &temp_int))
            host_requirements_.minimum.cpu_cores = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(min, "ram_gb", &temp_int))
            host_requirements_.minimum.ram_gb = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(min, "bandwidth_mbps", &temp_int))
            host_requirements_.minimum.bandwidth_mbps = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(min, "storage_gb", &temp_int))
            host_requirements_.minimum.storage_gb = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(min, "uptime_percent", &temp_int))
            host_requirements_.minimum.uptime_percent = static_cast<uint32>(temp_int);
    }

    config_setting_t* limits = config_setting_get_member(setting, "limits");
    if (limits) {
        if (config_setting_lookup_int(limits, "max_maps", &temp_int))
            host_requirements_.limits.max_maps = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(limits, "max_players", &temp_int))
            host_requirements_.limits.max_players = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(limits, "max_instances", &temp_int))
            host_requirements_.limits.max_instances = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(limits, "bandwidth_limit", &temp_int))
            host_requirements_.limits.bandwidth_limit = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(limits, "storage_limit", &temp_int))
            host_requirements_.limits.storage_limit = static_cast<uint32>(temp_int);
    }
}

void P2PConfigParser::parseOptimization(config_setting_t* setting) {
    if (!setting) return;

    int temp_bool, temp_int;
    config_setting_t* comp = config_setting_get_member(setting, "compression");
    if (comp) {
        if (config_setting_lookup_bool(comp, "enabled", &temp_bool))
            optimization_.compression.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_int(comp, "threshold", &temp_int))
            optimization_.compression.threshold = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(comp, "level", &temp_int))
            optimization_.compression.level = static_cast<uint8>(temp_int);
    }

    config_setting_t* priority = config_setting_get_member(setting, "packet_priority");
    if (priority) {
        if (config_setting_lookup_bool(priority, "enabled", &temp_bool))
            optimization_.packet_priority.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_int(priority, "levels", &temp_int))
            optimization_.packet_priority.levels = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(priority, "queue_size", &temp_int))
            optimization_.packet_priority.queue_size = static_cast<uint32>(temp_int);
    }

    config_setting_t* pool = config_setting_get_member(setting, "connection_pool");
    if (pool) {
        if (config_setting_lookup_bool(pool, "enabled", &temp_bool))
            optimization_.connection_pool.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_int(pool, "size", &temp_int))
            optimization_.connection_pool.size = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(pool, "timeout", &temp_int))
            optimization_.connection_pool.timeout = static_cast<uint32>(temp_int);
    }
}

void P2PConfigParser::parseThresholds(config_setting_t* setting) {
    if (!setting) return;

    int temp_int;
    config_setting_t* critical = config_setting_get_member(setting, "critical");
    if (critical) {
        if (config_setting_lookup_int(critical, "cpu_usage", &temp_int))
            thresholds_.critical.cpu_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(critical, "memory_usage", &temp_int))
            thresholds_.critical.memory_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(critical, "network_usage", &temp_int))
            thresholds_.critical.network_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(critical, "packet_loss", &temp_int))
            thresholds_.critical.packet_loss = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(critical, "latency", &temp_int))
            thresholds_.critical.latency = static_cast<uint32>(temp_int);
    }

    config_setting_t* warning = config_setting_get_member(setting, "warning");
    if (warning) {
        if (config_setting_lookup_int(warning, "cpu_usage", &temp_int))
            thresholds_.warning.cpu_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(warning, "memory_usage", &temp_int))
            thresholds_.warning.memory_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(warning, "network_usage", &temp_int))
            thresholds_.warning.network_usage = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(warning, "packet_loss", &temp_int))
            thresholds_.warning.packet_loss = static_cast<uint8>(temp_int);
            
        if (config_setting_lookup_int(warning, "latency", &temp_int))
            thresholds_.warning.latency = static_cast<uint32>(temp_int);
    }
}

void P2PConfigParser::parseDatabase(config_setting_t* setting) {
    if (!setting) return;

    int temp_bool, temp_int;
    config_setting_t* cleanup = config_setting_get_member(setting, "cleanup");
    if (cleanup) {
        if (config_setting_lookup_int(cleanup, "performance_data", &temp_int))
            database_.cleanup.performance_data = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(cleanup, "security_events", &temp_int))
            database_.cleanup.security_events = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(cleanup, "metrics", &temp_int))
            database_.cleanup.metrics = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(cleanup, "alerts", &temp_int))
            database_.cleanup.alerts = static_cast<uint32>(temp_int);
    }

    config_setting_t* backup = config_setting_get_member(setting, "backup");
    if (backup) {
        if (config_setting_lookup_bool(backup, "enabled", &temp_bool))
            database_.backup.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_int(backup, "interval", &temp_int))
            database_.backup.interval = static_cast<uint32>(temp_int);
            
        if (config_setting_lookup_int(backup, "keep_count", &temp_int))
            database_.backup.keep_count = static_cast<uint32>(temp_int);
    }
}

void P2PConfigParser::parseLogging(config_setting_t* setting) {
    if (!setting) return;

    int temp_bool;
    if (config_setting_lookup_bool(setting, "security_events", &temp_bool))
        logging_.security_events = (temp_bool != 0);
        
    if (config_setting_lookup_bool(setting, "performance_metrics", &temp_bool))
        logging_.performance_metrics = (temp_bool != 0);
        
    if (config_setting_lookup_bool(setting, "network_stats", &temp_bool))
        logging_.network_stats = (temp_bool != 0);
        
    if (config_setting_lookup_bool(setting, "host_status", &temp_bool))
        logging_.host_status = (temp_bool != 0);
        
    if (config_setting_lookup_bool(setting, "debug_info", &temp_bool))
        logging_.debug_info = (temp_bool != 0);
}

void P2PConfigParser::parseNotifications(config_setting_t* setting) {
    if (!setting) return;

    int temp_bool, temp_int;
    const char* str_val;
    
    config_setting_t* email = config_setting_get_member(setting, "email");
    if (email) {
        if (config_setting_lookup_bool(email, "enabled", &temp_bool))
            notifications_.email.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_string(email, "server", &str_val))
            notifications_.email.server = str_val;
            
        if (config_setting_lookup_int(email, "port", &temp_int))
            notifications_.email.port = static_cast<uint16>(temp_int);
            
        if (config_setting_lookup_string(email, "username", &str_val))
            notifications_.email.username = str_val;
            
        if (config_setting_lookup_string(email, "password", &str_val))
            notifications_.email.password = str_val;
            
        if (config_setting_lookup_string(email, "from", &str_val))
            notifications_.email.from = str_val;
            
        if (config_setting_lookup_string(email, "to", &str_val))
            notifications_.email.to = str_val;
    }

    config_setting_t* discord = config_setting_get_member(setting, "discord");
    if (discord) {
        if (config_setting_lookup_bool(discord, "enabled", &temp_bool))
            notifications_.discord.enabled = (temp_bool != 0);
            
        if (config_setting_lookup_string(discord, "webhook_url", &str_val))
            notifications_.discord.webhook_url = str_val;
            
        if (config_setting_lookup_string(discord, "username", &str_val))
            notifications_.discord.username = str_val;
            
        if (config_setting_lookup_string(discord, "avatar_url", &str_val))
            notifications_.discord.avatar_url = str_val;
    }
}

// Getter implementations remain unchanged...