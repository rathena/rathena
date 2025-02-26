#ifndef NETWORK_OPTIMIZATION_HPP
#define NETWORK_OPTIMIZATION_HPP

#include "sync.hpp"
#include <cstdint>
#include <vector>
#include <queue>
#include <memory>
#include <chrono>
#include <unordered_map>
#include "cbasetypes.hpp"

namespace network {

// Traffic shaping configuration
struct TrafficConfig {
    uint32 burst_size;           // Maximum burst size in bytes
    uint32 target_rate;         // Target rate in bytes per second
    uint32 window_size;         // Time window for rate calculation in ms
    uint32 min_packet_size;     // Minimum packet size for compression
    uint32 max_packet_size;     // Maximum allowed packet size
};

// Connection monitoring stats
struct ConnectionMonitor {
    struct Stats {
        double latency_ms{0.0};
        double packet_loss_percent{0.0};
        double bandwidth_usage{0.0};
        uint32 active_connections{0};
    };
};

// Priority queue for packet management
class PriorityQueue {
public:
    struct Packet {
        std::vector<uint8_t> data;
        uint8_t priority;
        std::chrono::steady_clock::time_point timestamp;
        
        bool operator<(const Packet& other) const {
            return priority > other.priority;
        }
    };

    using Queue = std::priority_queue<Packet>;

    void push(std::vector<uint8_t> data, uint8_t priority);
    bool pop(std::vector<uint8_t>& data);
    size_t size() const;
    bool empty() const;
};

// Connection monitoring
struct ConnectionStats {
    double latency{0.0};
    double packet_loss{0.0};
    double bandwidth{0.0};
    std::chrono::steady_clock::time_point last_update;
};

// Compression management
class CompressionManager {
public:
    bool shouldCompress(size_t size) const;
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);

private:
    static const size_t MIN_COMPRESS_SIZE = 64;
    static const size_t COMPRESSION_LEVEL = 6;
};

// Traffic shaping
class TrafficShaper {
public:
    explicit TrafficShaper(const TrafficConfig& config);
    
    bool canSendPacket(size_t size);
    void recordSentPacket(size_t size);
    uint32 getNextSendDelay() const;

private:
    struct TrafficData {
        TrafficConfig config;
        uint64 bytes_sent;
        std::chrono::steady_clock::time_point window_start;
    };
    
    Synchronized<TrafficData> traffic_data_;
};

// Network optimization manager
class OptimizationManager {
public:
    static OptimizationManager& getInstance();

    void initialize(const TrafficConfig& config);
    void shutdown();

    bool processOutgoingPacket(uint32 ip, std::vector<uint8_t>& data, uint8_t priority);
    bool processIncomingPacket(uint32 ip, std::vector<uint8_t>& data);

    void updateNetworkStats(uint32 ip, const ConnectionMonitor::Stats& stats);

private:
    OptimizationManager() = default;
    ~OptimizationManager() = default;
    OptimizationManager(const OptimizationManager&) = delete;
    OptimizationManager& operator=(const OptimizationManager&) = delete;

    struct OptimizationData {
        TrafficConfig config;
        std::unique_ptr<TrafficShaper> traffic_shaper;
        std::unique_ptr<CompressionManager> compression_manager;
        bool initialized{false};
    };

    Synchronized<OptimizationData> config_data_;
};

// Reference to global instance
inline OptimizationManager& getOptimizationManager() {
    return OptimizationManager::getInstance();
}

} // namespace network

#endif // NETWORK_OPTIMIZATION_HPP