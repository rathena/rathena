#ifndef NETWORK_SECURITY_HPP
#define NETWORK_SECURITY_HPP

#include <memory>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <string>
#include <mutex>
#include "cbasetypes.hpp"
#include "timer.hpp"
#include "db.hpp"

struct connection_data {
    std::chrono::steady_clock::time_point last_packet;
    uint32 packet_count;
    uint32 byte_count;
    uint16 connection_count;
    bool is_whitelisted;
};

class NetworkSecurity {
public:
    static NetworkSecurity& getInstance() {
        static NetworkSecurity instance;
        return instance;
    }

    // DDoS Protection Settings
    struct DDoSConfig {
        uint32 packet_rate_limit;      // Max packets per second
        uint32 connection_rate_limit;   // Max new connections per second
        uint32 bandwidth_limit;         // Max bytes per second
        uint32 burst_threshold;         // Allowed burst size
        uint32 blacklist_duration;      // Duration in seconds for blacklisting
    };

    // Network Optimization Settings
    struct NetworkConfig {
        bool compression_enabled;
        uint32 compression_threshold;   // Minimum size for compression
        uint8 priority_levels;         // Number of priority levels
        uint32 connection_pool_size;   // Size of connection pool
        uint32 max_packet_size;        // Maximum allowed packet size
    };

    void init(const DDoSConfig& ddos_cfg, const NetworkConfig& net_cfg);
    void shutdown();

    // DDoS Protection
    bool checkPacket(uint32 ip, size_t size);
    bool checkConnection(uint32 ip);
    void whitelistIP(uint32 ip);
    void blacklistIP(uint32 ip, uint32 duration);

    // Network Enhancement
    bool shouldCompress(size_t size) const;
    uint8 getPacketPriority(uint16 packet_id) const;
    void optimizeBandwidth();
    void updateLatencyStats(uint32 ip, uint32 latency);

    // Connection Management
    bool acquireConnection();
    void releaseConnection();
    void pruneConnections();

    // Monitoring
    struct NetworkStats {
        uint32 total_packets;
        uint32 blocked_packets;
        uint32 compressed_packets;
        uint32 active_connections;
        uint32 peak_connections;
        double average_latency;
        uint32 bandwidth_usage;
    };
    NetworkStats getStats() const;

private:
    NetworkSecurity() = default;
    ~NetworkSecurity() = default;
    NetworkSecurity(const NetworkSecurity&) = delete;
    NetworkSecurity& operator=(const NetworkSecurity&) = delete;

    // Configuration
    DDoSConfig ddos_config;
    NetworkConfig network_config;

    // Protection data
    std::unordered_map<uint32, connection_data> connections;
    std::unordered_map<uint32, std::chrono::steady_clock::time_point> blacklist;
    std::queue<uint32> connection_pool;

    // Statistics
    struct stats {
        uint32 packets_processed{0};
        uint32 packets_blocked{0};
        uint32 packets_compressed{0};
        uint32 current_connections{0};
        uint32 peak_connections{0};
        double total_latency{0};
        uint32 latency_samples{0};
        uint32 current_bandwidth{0};
    } stats_;

    // Thread safety
    mutable std::mutex mutex_;

    // Helper functions
    bool isBlacklisted(uint32 ip) const;
    void updateStats(uint32 ip, size_t size, bool blocked);
    void cleanupExpiredEntries();
};

// Packet Priority Definitions
enum class PacketPriority : uint8 {
    CRITICAL = 0,   // System messages, disconnects
    HIGH = 1,       // Player actions, combat
    NORMAL = 2,     // Movement, chat
    LOW = 3         // Updates, announcements
};

// Network Enhancement Functions
namespace network {
    // Compression
    std::vector<uint8> compressPacket(const std::vector<uint8>& data);
    std::vector<uint8> decompressPacket(const std::vector<uint8>& data);

    // Traffic Shaping
    void prioritizePacket(int fd, PacketPriority priority);
    void adjustBandwidth(int fd, uint32 current_usage);

    // Connection Pooling
    int getPooledConnection();
    void returnConnectionToPool(int fd);

    // Latency Management
    void trackLatency(int fd, uint32 latency);
    void optimizeRoute(int fd);
}

#endif // NETWORK_SECURITY_HPP