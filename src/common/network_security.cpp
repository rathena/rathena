#include "network_security.hpp"
#include "showmsg.hpp"
#include "timer.hpp"

// Default configuration values
static const NetworkSecurity::DDoSConfig DEFAULT_DDOS_CONFIG = {
    .packet_rate_limit = 1000,      // 1000 packets per second
    .connection_rate_limit = 100,    // 100 new connections per second
    .bandwidth_limit = 1048576,      // 1 MB per second
    .burst_threshold = 2000,         // Allow burst up to 2000 packets
    .blacklist_duration = 3600       // 1 hour blacklist duration
};

static const NetworkSecurity::NetworkConfig DEFAULT_NETWORK_CONFIG = {
    .compression_enabled = true,
    .compression_threshold = 512,     // Compress packets larger than 512 bytes
    .priority_levels = 4,            // 4 priority levels
    .connection_pool_size = 1000,    // Pool of 1000 connections
    .max_packet_size = 65536         // 64 KB max packet size
};

void NetworkSecurity::init(const DDoSConfig& ddos_cfg, const NetworkConfig& net_cfg) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    ddos_config = ddos_cfg;
    network_config = net_cfg;

    // Initialize connection pool
    for (uint32 i = 0; i < network_config.connection_pool_size; ++i) {
        connection_pool.push(i);
    }

    ShowStatus("Network security initialized with packet rate limit: %u/s\n", ddos_config.packet_rate_limit);
}

void NetworkSecurity::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    connections.clear();
    blacklist.clear();
    
    // Clear connection pool
    std::queue<uint32> empty;
    std::swap(connection_pool, empty);
}

bool NetworkSecurity::checkPacket(uint32 ip, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check blacklist
    if (isBlacklisted(ip)) {
        updateStats(ip, size, true);
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto& conn = connections[ip];

    // Reset counters if more than 1 second has passed
    if (std::chrono::duration_cast<std::chrono::seconds>(now - conn.last_packet).count() >= 1) {
        conn.packet_count = 0;
        conn.byte_count = 0;
    }

    // Update counters
    conn.packet_count++;
    conn.byte_count += size;
    conn.last_packet = now;

    // Check rate limits
    if (!conn.is_whitelisted) {
        if (conn.packet_count > ddos_config.packet_rate_limit ||
            conn.byte_count > ddos_config.bandwidth_limit) {
            blacklistIP(ip, ddos_config.blacklist_duration);
            updateStats(ip, size, true);
            return false;
        }
    }

    updateStats(ip, size, false);
    return true;
}

bool NetworkSecurity::checkConnection(uint32 ip) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (isBlacklisted(ip)) {
        return false;
    }

    auto& conn = connections[ip];
    conn.connection_count++;

    if (!conn.is_whitelisted && conn.connection_count > ddos_config.connection_rate_limit) {
        blacklistIP(ip, ddos_config.blacklist_duration);
        return false;
    }

    return true;
}

void NetworkSecurity::whitelistIP(uint32 ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    connections[ip].is_whitelisted = true;
    blacklist.erase(ip);
}

void NetworkSecurity::blacklistIP(uint32 ip, uint32 duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    blacklist[ip] = std::chrono::steady_clock::now() + std::chrono::seconds(duration);
    ShowWarning("IP %u.%u.%u.%u has been blacklisted for %u seconds\n",
        (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF, duration);
}

bool NetworkSecurity::shouldCompress(size_t size) const {
    return network_config.compression_enabled && size >= network_config.compression_threshold;
}

uint8 NetworkSecurity::getPacketPriority(uint16 packet_id) const {
    // Priority assignment based on packet type
    if (packet_id < 0x0100) return static_cast<uint8>(PacketPriority::CRITICAL);
    if (packet_id < 0x0300) return static_cast<uint8>(PacketPriority::HIGH);
    if (packet_id < 0x0900) return static_cast<uint8>(PacketPriority::NORMAL);
    return static_cast<uint8>(PacketPriority::LOW);
}

void NetworkSecurity::optimizeBandwidth() {
    std::lock_guard<std::mutex> lock(mutex_);
    cleanupExpiredEntries();
}

void NetworkSecurity::updateLatencyStats(uint32 ip, uint32 latency) {
    std::lock_guard<std::mutex> lock(mutex_);
    stats_.total_latency += latency;
    stats_.latency_samples++;
}

NetworkSecurity::NetworkStats NetworkSecurity::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {
        .total_packets = stats_.packets_processed,
        .blocked_packets = stats_.packets_blocked,
        .compressed_packets = stats_.packets_compressed,
        .active_connections = stats_.current_connections,
        .peak_connections = stats_.peak_connections,
        .average_latency = stats_.latency_samples > 0 ? 
            stats_.total_latency / stats_.latency_samples : 0,
        .bandwidth_usage = stats_.current_bandwidth
    };
}

bool NetworkSecurity::isBlacklisted(uint32 ip) const {
    auto it = blacklist.find(ip);
    if (it == blacklist.end()) return false;
    return std::chrono::steady_clock::now() < it->second;
}

void NetworkSecurity::updateStats(uint32 ip, size_t size, bool blocked) {
    stats_.packets_processed++;
    if (blocked) stats_.packets_blocked++;
    stats_.current_bandwidth += size;
}

void NetworkSecurity::cleanupExpiredEntries() {
    auto now = std::chrono::steady_clock::now();
    
    // Cleanup blacklist
    for (auto it = blacklist.begin(); it != blacklist.end();) {
        if (now >= it->second) {
            it = blacklist.erase(it);
        } else {
            ++it;
        }
    }

    // Cleanup old connections
    for (auto it = connections.begin(); it != connections.end();) {
        if (std::chrono::duration_cast<std::chrono::hours>(now - it->second.last_packet).count() >= 24) {
            it = connections.erase(it);
        } else {
            ++it;
        }
    }
}