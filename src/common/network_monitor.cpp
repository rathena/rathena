#include "network_monitor.hpp"
#include "showmsg.hpp"
#include "timer.hpp"
#include "sql.hpp"
#include "malloc.hpp"
#include <sstream>
#include <memory>
#include <algorithm>
#include <cstring>

// Constants
static constexpr size_t MAX_PACKET_SIZE = 0x10000; // 64KB
static constexpr double MAX_LATENCY_MS = 10000.0;  // 10 seconds

NetworkMonitor::NetworkMonitor() : sql_handle(nullptr) {}

NetworkMonitor::~NetworkMonitor() {
    if (initialized_) {
        shutdown();
    }
}

NetworkMonitor& NetworkMonitor::getInstance() {
    static NetworkMonitor instance;
    return instance;
}

bool NetworkMonitor::initialize(const char* db_hostname, const char* db_username, 
                              const char* db_password, const char* db_database, uint16 db_port) {
    std::lock_guard<std::mutex> lock(sql_mutex_);
    
    if (initialized_) return true;

    try {
        sql_handle = Sql_Malloc();
        if (!sql_handle) {
            throw Error("Failed to allocate SQL handle");
        }

        if (SQL_ERROR == Sql_Connect(sql_handle, db_username, db_password, 
                                   db_hostname, db_port, db_database)) {
            throw Error(Sql_GetError(sql_handle));
        }

        // Prepare database for use
        executeQuery("SET NAMES 'utf8'");
        executeQuery("SET SESSION sql_mode = ''");

        // Initialize cleanup timer (every 5 minutes)
        add_timer_func_list(cleanupOldData, "NetworkMonitor::cleanupOldData");
        add_timer_interval(gettick() + 300000, cleanupOldData, 0, 0, 300000);

        // Initialize status check timer (every minute)
        add_timer_func_list(checkHostStatus, "NetworkMonitor::checkHostStatus");
        add_timer_interval(gettick() + 60000, checkHostStatus, 0, 0, 60000);

        initialized_ = true;
        ShowStatus("Network monitoring system initialized\n");
        return true;

    } catch (const Error& e) {
        ShowError("Network monitor initialization failed: %s\n", e.what());
        if (sql_handle) {
            Sql_Free(sql_handle);
            sql_handle = nullptr;
        }
        return false;
    }
}

void NetworkMonitor::updateHostMetrics(uint32 account_id, const HostMetrics& metrics) {
    if (!initialized_) return;

    try {
        std::unique_lock<std::shared_mutex> lock(host_mutex_);
        host_metrics[account_id] = metrics;

        std::string query = 
            "INSERT INTO p2p_system_metrics "
            "(account_id, metrics_data, connected_players, timestamp) "
            "VALUES (?, ?, ?, NOW()) "
            "ON DUPLICATE KEY UPDATE "
            "metrics_data = VALUES(metrics_data), "
            "connected_players = VALUES(connected_players)";

        std::stringstream ss;
        ss << "{\"cpu_usage\":" << metrics.cpu_usage
           << ",\"memory_usage\":" << metrics.memory_usage
           << ",\"network_usage\":" << metrics.network_usage
           << ",\"ip_address\":\"" << escapeSQLString(metrics.ip_address) << "\"}";

        std::vector<std::string> params = {
            std::to_string(account_id),
            ss.str(),
            std::to_string(metrics.connected_players)
        };

        if (!executeQuery(query, params)) {
            throw Error("Failed to update host metrics");
        }

        checkThresholds();

    } catch (const Error& e) {
        ShowError("Failed to update host metrics: %s\n", e.what());
    }
}

bool NetworkMonitor::isHostActive(uint32 account_id) const {
    std::shared_lock<std::shared_mutex> lock(host_mutex_);
    
    auto it = host_metrics.find(account_id);
    if (it == host_metrics.end()) return false;

    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(
        now - it->second.last_update).count();

    return it->second.is_active && elapsed < 5; // Consider inactive after 5 minutes
}

void NetworkMonitor::checkThresholds() {
    std::shared_lock<std::shared_mutex> lock(host_mutex_);
    
    for (const auto& [account_id, metrics] : host_metrics) {
        if (!isHostActive(account_id)) continue;

        try {
            // CPU usage check
            if (metrics.cpu_usage > 90.0) {
                createAlert("high_cpu", 
                          "CPU usage above 90% for host " + std::to_string(account_id), 
                          "critical");
            } else if (metrics.cpu_usage > 80.0) {
                createAlert("high_cpu", 
                          "CPU usage above 80% for host " + std::to_string(account_id), 
                          "warning");
            }

            // Memory usage check
            if (metrics.memory_usage > 90.0) {
                createAlert("high_memory", 
                          "Memory usage above 90% for host " + std::to_string(account_id), 
                          "critical");
            }

            // Network usage check
            if (metrics.network_usage > 90.0) {
                createAlert("high_network", 
                          "Network usage above 90% for host " + std::to_string(account_id), 
                          "warning");
            }

        } catch (const Error& e) {
            ShowError("Failed to check thresholds: %s\n", e.what());
        }
    }
}

int NetworkMonitor::cleanupOldData(int tid, t_tick tick, int id, intptr_t data) {
    auto& instance = NetworkMonitor::getInstance();
    
    try {
        instance.cleanupExpiredData();
    } catch (const Error& e) {
        ShowError("Failed to clean up old data: %s\n", e.what());
    }
    
    return 0;
}

int NetworkMonitor::checkHostStatus(int tid, t_tick tick, int id, intptr_t data) {
    auto& instance = NetworkMonitor::getInstance();
    
    try {
        instance.updateSystemMetrics();
        instance.checkActiveHosts();
    } catch (const Error& e) {
        ShowError("Failed to check host status: %s\n", e.what());
    }
    
    return 0;
}

// Helper methods implementation...