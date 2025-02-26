#include "p2p_map_config.hpp"
#include "showmsg.hpp"
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace rathena {

P2PMapConfigParser* P2PMapConfigParser::instance = nullptr;

P2PMapConfigParser::P2PMapConfigParser() {
    // Initialize with default values
    config.general.enable_p2p_maps = true;
    config.general.prefer_low_latency = true;
    config.general.enable_auto_migration = true;
    config.general.enable_vps_fallback = true;
    config.general.data_check_interval = 5;
    config.general.enable_p2p_security_checks = true;
    config.general.persistent_hosts = true;

    config.host_requirements.cpu_min_ghz = 3.0;
    config.host_requirements.cpu_min_cores = 4;
    config.host_requirements.ram_min_mb = 8192;
    config.host_requirements.net_min_speed = 100;
    config.host_requirements.net_max_latency = 30;
    config.host_requirements.max_disconnects_per_hour = 1;

    // Initialize real-time gaming experience settings with default values
    config.realtime_gaming.prioritize_realtime = true;
    config.realtime_gaming.max_jitter_ms = 15;
    config.realtime_gaming.min_packet_rate = 100;
    config.realtime_gaming.realtime_sync_interval_ms = 50;
    config.realtime_gaming.max_frame_time_ms = 16;
    config.realtime_gaming.latency_weight = 0.35f;
    config.realtime_gaming.network_speed_weight = 0.25f;
    config.realtime_gaming.cpu_weight = 0.15f;
    config.realtime_gaming.ram_weight = 0.15f;
    config.realtime_gaming.stability_weight = 0.10f;

    config.task_distribution.max_tasks_per_node = 3;
    config.task_distribution.reassignment_interval = 30;

    config.failover.backup_hosts_count = 2;
    config.failover.migration_timeout = 1000;
    config.failover.sync_interval = 100;
    config.failover.always_keep_vps_ready = true;

    config.monitoring.health_check_interval = 15;
    config.monitoring.metrics_interval = 60;
    config.monitoring.detailed_logging = true;
    config.monitoring.alert_on_failure = true;
}

P2PMapConfigParser& P2PMapConfigParser::getInstance() {
    if (!instance) {
        instance = new P2PMapConfigParser();
    }
    return *instance;
}

bool P2PMapConfigParser::load(const char* filename) {
    try {
        last_filename = filename;
        YAML::Node yaml = YAML::LoadFile(filename);

        // Parse general settings
        if (yaml["general"]) {
            auto general = yaml["general"];
            config.general.enable_p2p_maps = general["enable_p2p_maps"].as<int>(1) != 0;
            config.general.prefer_low_latency = general["prefer_low_latency"].as<int>(1) != 0;
            config.general.enable_auto_migration = general["enable_auto_migration"].as<int>(1) != 0;
            config.general.enable_vps_fallback = general["enable_vps_fallback"].as<int>(1) != 0;
            config.general.data_check_interval = general["data_check_interval"].as<int>(5);
            config.general.enable_p2p_security_checks = general["enable_p2p_security_checks"].as<int>(1) != 0;
            config.general.persistent_hosts = general["persistent_hosts"].as<int>(1) != 0;
        }

        // Parse host requirements
        if (yaml["host_requirements"]) {
            auto requirements = yaml["host_requirements"];
            config.host_requirements.cpu_min_ghz = requirements["cpu_min_ghz"].as<double>(3.0);
            config.host_requirements.cpu_min_cores = requirements["cpu_min_cores"].as<uint16>(4);
            config.host_requirements.ram_min_mb = requirements["ram_min_mb"].as<uint64>(8192);
            config.host_requirements.net_min_speed = requirements["net_min_speed"].as<uint32>(100);
            config.host_requirements.net_max_latency = requirements["net_max_latency"].as<uint16>(30);
            config.host_requirements.max_disconnects_per_hour = requirements["max_disconnects_per_hour"].as<uint16>(1);
        }

        // Parse real-time gaming experience settings
        if (yaml["realtime_gaming"]) {
            auto realtime = yaml["realtime_gaming"];
            config.realtime_gaming.prioritize_realtime = realtime["prioritize_realtime"].as<int>(1) != 0;
            config.realtime_gaming.max_jitter_ms = realtime["max_jitter_ms"].as<uint16>(15);
            config.realtime_gaming.min_packet_rate = realtime["min_packet_rate"].as<uint32>(100);
            config.realtime_gaming.realtime_sync_interval_ms = realtime["realtime_sync_interval_ms"].as<uint16>(50);
            config.realtime_gaming.max_frame_time_ms = realtime["max_frame_time_ms"].as<uint16>(16);
            
            config.realtime_gaming.latency_weight = realtime["latency_weight"].as<float>(0.35f);
            config.realtime_gaming.network_speed_weight = realtime["network_speed_weight"].as<float>(0.25f);
            config.realtime_gaming.cpu_weight = realtime["cpu_weight"].as<float>(0.15f);
            config.realtime_gaming.ram_weight = realtime["ram_weight"].as<float>(0.15f);
            config.realtime_gaming.stability_weight = realtime["stability_weight"].as<float>(0.10f);
        }

        // Parse P2P-eligible maps
        if (yaml["p2p_eligible_maps"]) {
            auto maps = yaml["p2p_eligible_maps"];
            for (const auto& map_pair : maps) {
                std::string map_name = map_pair.first.as<std::string>();
                bool eligible = map_pair.second.as<int>(0) != 0;
                config.p2p_eligible_maps[map_name] = eligible;
            }
        }

        // Parse task distribution
        if (yaml["task_distribution"]) {
            auto tasks = yaml["task_distribution"];
            config.task_distribution.max_tasks_per_node = tasks["max_tasks_per_node"].as<uint16>(3);
            config.task_distribution.reassignment_interval = tasks["reassignment_interval"].as<uint16>(30);

            if (tasks["priorities"]) {
                auto priorities = tasks["priorities"];
                for (const auto& priority_pair : priorities) {
                    std::string task_name = priority_pair.first.as<std::string>();
                    uint8 priority = priority_pair.second.as<uint8>(5);
                    config.task_distribution.task_priorities[task_name] = priority;
                }
            }
        }

        // Parse failover settings
        if (yaml["failover"]) {
            auto failover = yaml["failover"];
            config.failover.backup_hosts_count = failover["backup_hosts_count"].as<uint8>(2);
            config.failover.migration_timeout = failover["migration_timeout"].as<uint16>(1000);
            config.failover.sync_interval = failover["sync_interval"].as<uint16>(100);
            config.failover.always_keep_vps_ready = failover["always_keep_vps_ready"].as<int>(1) != 0;
        }

        // Parse monitoring settings
        if (yaml["monitoring"]) {
            auto monitoring = yaml["monitoring"];
            config.monitoring.health_check_interval = monitoring["health_check_interval"].as<uint16>(15);
            config.monitoring.metrics_interval = monitoring["metrics_interval"].as<uint16>(60);
            config.monitoring.detailed_logging = monitoring["detailed_logging"].as<int>(1) != 0;
            config.monitoring.alert_on_failure = monitoring["alert_on_failure"].as<int>(1) != 0;
        }

        ShowInfo("Loaded P2P map configuration from %s\n", filename);
        return true;
    } catch (const std::exception& e) {
        ShowError("Failed to load P2P map configuration: %s\n", e.what());
        return false;
    }
}

void P2PMapConfigParser::reload() {
    if (!last_filename.empty()) {
        load(last_filename.c_str());
    }
}

const P2PMapConfig& P2PMapConfigParser::getConfig() const {
    return config;
}

} // namespace rathena