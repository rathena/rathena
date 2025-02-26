#include "network_optimization.hpp"
#include "sync.hpp"
#include "showmsg.hpp"
#include <zlib.h>
#include <algorithm>
#include <limits>
#include <cstring>

namespace network {

// Use our custom synchronization
static Synchronized<PriorityQueue::Queue> packet_queue;
static Synchronized<std::unordered_map<uint32, ConnectionStats>> connection_stats;

// PriorityQueue implementation
void PriorityQueue::push(std::vector<uint8_t> data, uint8_t priority) {
    auto queue = packet_queue.access();
    queue->push(Packet{std::move(data), priority, std::chrono::steady_clock::now()});
}

bool PriorityQueue::pop(std::vector<uint8_t>& data) {
    return packet_queue.with_lock([&](auto& queue) {
        if (queue.empty()) {
            return false;
        }
        data = std::move(queue.top().data);
        queue.pop();
        return true;
    });
}

size_t PriorityQueue::size() const {
    return packet_queue.with_lock([](const auto& queue) {
        return queue.size();
    });
}

bool PriorityQueue::empty() const {
    return packet_queue.with_lock([](const auto& queue) {
        return queue.empty();
    });
}

// CompressionManager implementation
bool CompressionManager::shouldCompress(size_t size) const {
    return size >= MIN_COMPRESS_SIZE;
}

std::vector<uint8_t> CompressionManager::compress(const std::vector<uint8_t>& data) {
    if (data.size() < MIN_COMPRESS_SIZE) {
        return data;
    }

    uLong compressed_size = compressBound(static_cast<uLong>(data.size()));
    std::vector<uint8_t> compressed(compressed_size + sizeof(uint32_t));

    // Store original size
    uint32_t original_size = static_cast<uint32_t>(data.size());
    std::memcpy(compressed.data(), &original_size, sizeof(uint32_t));

    int result = compress2(
        compressed.data() + sizeof(uint32_t),
        &compressed_size,
        data.data(),
        static_cast<uLong>(data.size()),
        static_cast<int>(COMPRESSION_LEVEL)
    );

    if (result != Z_OK) {
        ShowError("Compression failed with error code %d\n", result);
        return data;
    }

    compressed.resize(compressed_size + sizeof(uint32_t));
    return compressed;
}

std::vector<uint8_t> CompressionManager::decompress(const std::vector<uint8_t>& data) {
    if (data.size() <= sizeof(uint32_t)) {
        return data;
    }

    uint32_t original_size = 0;
    std::memcpy(&original_size, data.data(), sizeof(uint32_t));

    std::vector<uint8_t> decompressed(original_size);
    uLong decompressed_size = static_cast<uLong>(original_size);

    int result = uncompress(
        decompressed.data(),
        &decompressed_size,
        data.data() + sizeof(uint32_t),
        static_cast<uLong>(data.size() - sizeof(uint32_t))
    );

    if (result != Z_OK) {
        ShowError("Decompression failed with error code %d\n", result);
        return data;
    }

    return decompressed;
}

// Traffic shaping implementation
TrafficShaper::TrafficShaper(const TrafficConfig& config)
    : traffic_data_(TrafficData{config, 0, std::chrono::steady_clock::now()}) {
}

bool TrafficShaper::canSendPacket(size_t size) {
    return traffic_data_.with_lock([size](auto& data) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - data.window_start
        );

        if (elapsed.count() >= static_cast<int64_t>(data.config.window_size)) {
            data.bytes_sent = 0;
            data.window_start = now;
        }

        return (data.bytes_sent + size) <= data.config.burst_size;
    });
}

void TrafficShaper::recordSentPacket(size_t size) {
    traffic_data_.with_lock([size](auto& data) {
        data.bytes_sent += size;
    });
}

uint32 TrafficShaper::getNextSendDelay() const {
    return traffic_data_.with_lock([](const auto& data) {
        if (data.bytes_sent < data.config.target_rate) {
            return 0u;
        }
        return static_cast<uint32>((data.bytes_sent * 1000) / data.config.target_rate);
    });
}

// Network optimization manager implementation
OptimizationManager& OptimizationManager::getInstance() {
    static OptimizationManager instance;
    return instance;
}

void OptimizationManager::initialize(const TrafficConfig& config) {
    config_data_.with_lock([&](OptimizationData& data) {
        if (data.initialized) return;

        data.config = config;
        data.traffic_shaper = std::make_unique<TrafficShaper>(config);
        data.compression_manager = std::make_unique<CompressionManager>();
        data.initialized = true;

        ShowStatus("Network optimization system initialized\n");
    });
}

void OptimizationManager::shutdown() {
    config_data_.with_lock([](OptimizationData& data) {
        if (!data.initialized) return;

        data.traffic_shaper.reset();
        data.compression_manager.reset();
        data.initialized = false;

        ShowStatus("Network optimization system shutdown\n");
    });
}

bool OptimizationManager::processOutgoingPacket(uint32 ip, std::vector<uint8_t>& data, uint8_t priority) {
    return config_data_.with_lock([&](const OptimizationData& data) {
        if (!data.initialized) return true;

        // Apply compression if needed
        if (data.compression_manager->shouldCompress(data.size())) {
            data = data.compression_manager->compress(data);
        }

        // Check traffic shaping
        if (!data.traffic_shaper->canSendPacket(data.size())) {
            packet_queue.with_lock([&](auto& queue) {
                queue.push(PriorityQueue::Packet{
                    std::move(data),
                    priority,
                    std::chrono::steady_clock::now()
                });
            });
            return false;
        }

        data.traffic_shaper->recordSentPacket(data.size());
        return true;
    });
}

bool OptimizationManager::processIncomingPacket(uint32 ip, std::vector<uint8_t>& data) {
    return config_data_.with_lock([&](const OptimizationData& data) {
        if (!data.initialized) return true;
        data = data.compression_manager->decompress(data);
        return true;
    });
}

void OptimizationManager::updateNetworkStats(uint32 ip, const ConnectionMonitor::Stats& stats) {
    connection_stats.with_lock([&](auto& stats_map) {
        auto& conn_stats = stats_map[ip];
        conn_stats.last_update = std::chrono::steady_clock::now();
        conn_stats.latency = stats.latency_ms;
        conn_stats.packet_loss = stats.packet_loss_percent;
        conn_stats.bandwidth = stats.bandwidth_usage;
    });
}

} // namespace network