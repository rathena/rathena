#ifndef NETWORK_MONITOR_HPP
#define NETWORK_MONITOR_HPP

#include "cbasetypes.hpp"
#include "timer.hpp"
#include "db.hpp"
#include "sql.hpp"
#include "thread_guard.hpp"
#include "sync.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <atomic>

namespace rathena {

class NetworkMonitor {
public:
    struct NetworkStats {
        std::atomic<uint32> packets_processed{0};
        std::atomic<uint32> bytes_transferred{0};
        std::atomic<double> packet_loss_rate{0.0};
        std::atomic<uint32> active_connections{0};
        std::atomic<double> average_latency{0.0};
        std::atomic<uint32> bandwidth_usage{0};
        std::atomic<uint32> security_events{0};
    };

    struct HostMetrics {
        uint32 account_id;
        std::string ip_address;
        std::atomic<uint32> connected_players{0};
        std::atomic<double> cpu_usage{0.0};
        std::atomic<double> memory_usage{0.0};
        std::atomic<double> network_usage{0.0};
        std::chrono::system_clock::time_point last_update;
        std::atomic<bool> is_active{false};
    };

    static NetworkMonitor& getInstance();

    // Thread-safe initialization and shutdown
    bool initialize(const char* db_hostname, const char* db_username, 
                   const char* db_password, const char* db_database, uint16 db_port);
    void shutdown();

    // Network monitoring (thread-safe)
    void recordPacket(uint32 account_id, size_t size, bool is_incoming);
    void recordLatency(uint32 account_id, double latency_ms);
    void recordPacketLoss(uint32 account_id);
    void updateHostMetrics(uint32 account_id, const HostMetrics& metrics);
    void recordSecurityEvent(const std::string& event_type, uint32 account_id, 
                           const std::string& details);

    // Stats retrieval (thread-safe)
    NetworkStats getNetworkStats(uint32 account_id) const;
    HostMetrics getHostMetrics(uint32 account_id) const;
    bool isHostActive(uint32 account_id) const;

    // System monitoring (thread-safe)
    void updateSystemMetrics();
    void checkThresholds();
    void generateAlerts();

    // Timer callbacks
    static int cleanupOldData(int tid, t_tick tick, int id, intptr_t data);
    static int checkHostStatus(int tid, t_tick tick, int id, intptr_t data);

    class Error : public std::runtime_error {
    public:
        explicit Error(const std::string& msg) : std::runtime_error(msg) {}
    };

private:
    NetworkMonitor();
    ~NetworkMonitor();
    NetworkMonitor(const NetworkMonitor&) = delete;
    NetworkMonitor& operator=(const NetworkMonitor&) = delete;

    struct ConnectionStats {
        std::atomic<uint32> packets_sent{0};
        std::atomic<uint32> packets_received{0};
        std::atomic<uint32> bytes_sent{0};
        std::atomic<uint32> bytes_received{0};
        std::atomic<uint32> packets_lost{0};
        std::atomic<double> total_latency{0.0};
        std::atomic<uint32> latency_samples{0};
        std::chrono::system_clock::time_point last_update;
    };

    // Thread-safe containers and synchronization
    mutable rw_spinlock connection_lock_;
    mutable rw_spinlock host_lock_;
    spinlock sql_lock_;
    std::atomic<bool> initialized_{false};

    // Thread pool for async operations
    thread_pool worker_pool_;

    // Data storage
    std::unordered_map<uint32, ConnectionStats> connection_stats;
    std::unordered_map<uint32, HostMetrics> host_metrics;
    sync_queue<std::string> alert_queue_;
    Sql* sql_handle;

    // Helper methods (thread-safe)
    void updateDatabase();
    bool exceedsThreshold(const std::string& metric_type, double value) const;
    void createAlert(const std::string& alert_type, const std::string& message, 
                    const std::string& severity);

    // SQL utilities
    std::string escapeSQLString(const std::string& str);
    bool executeQuery(const std::string& query, const std::vector<std::string>& params = {});
    void handleSQLError(const std::string& context);

    // Resource cleanup
    void cleanupExpiredData();
    void checkActiveHosts();
};

// Global access function
inline NetworkMonitor& network_monitor() {
    return NetworkMonitor::getInstance();
}

} // namespace rathena

#endif // NETWORK_MONITOR_HPP