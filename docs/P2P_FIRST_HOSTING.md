# P2P-First Map Hosting System for rAthena

This document provides a comprehensive overview of the P2P-first map hosting system implemented in rAthena. The system prioritizes player-hosted maps for non-critical areas while maintaining server-side validation and seamless failover mechanisms.

## System Overview

The P2P-first hosting system dynamically assigns eligible players with high-performance machines to host non-critical maps, reducing server load and improving player experience through lower latency. The system includes:

1. **Dynamic Host Selection**: Automatically selects the best player host based on performance metrics
2. **Real-time Gaming Optimization**: Prioritizes hosts with low latency, minimal jitter, and high packet processing rates
3. **Seamless Migration**: Instantly reassigns maps if a host disconnects with zero downtime
4. **VPS Fallback**: Maintains VPS-hosted instances as backup for all maps
5. **Server-side Validation**: Prevents cheating through comprehensive security checks

## Key Components

### P2PHost Class

Represents a player machine capable of hosting maps:

- Tracks performance metrics (CPU, RAM, network speed, latency, jitter)
- Manages assigned maps
- Handles security validation
- Monitors real-time gaming metrics

### P2PHostManager Class

Manages the entire P2P hosting ecosystem:

- Registers and unregisters hosts
- Selects optimal hosts for maps based on performance scoring
- Handles map migration between hosts
- Performs regular health checks
- Maintains persistent host data between server restarts

### P2PMapConfig Structure

Configurable settings for the P2P hosting system:

- Host eligibility requirements
- Map eligibility settings
- Real-time gaming optimization parameters
- Failover settings
- Task distribution priorities

## Host Selection Algorithm

The system uses a sophisticated scoring algorithm to select the best host for each map:

```cpp
double P2PHostManager::calculateHostScore(const std::shared_ptr<P2PHost>& host) const {
    // Get the host stats and configuration
    const auto& stats = host->getStats();
    const auto& p2p_config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Calculate base scores for each metric
    double cpu_score = (stats.cpu_ghz * stats.cpu_cores) / config.min_cpu_ghz;
    double ram_score = static_cast<double>(stats.free_ram_mb) / config.min_ram_mb;
    double network_score = static_cast<double>(stats.network_speed_mbps) / config.min_network_mbps;
    
    // Latency is critical for real-time gaming experience
    double latency_score = 0.0;
    if (config.max_latency_ms > 0) {
        if (p2p_config.realtime_gaming.prioritize_realtime) {
            // Exponential decay: score = e^(-latency/max_latency)
            latency_score = std::exp(-static_cast<double>(stats.latency_ms) / config.max_latency_ms);
        } else {
            // Linear scaling for non-realtime prioritized mode
            latency_score = config.max_latency_ms > 0 ? 
                static_cast<double>(config.max_latency_ms) / (stats.latency_ms + 1) : 1.0;
        }
    }
    
    // Stability is also critical for real-time gaming
    double stability_score = config.max_disconnects_per_hour > 0 ? 
        1.0 - (static_cast<double>(stats.disconnects_per_hour) / config.max_disconnects_per_hour) : 1.0;
    
    // Use weights from configuration if real-time gaming is prioritized
    double score;
    if (p2p_config.realtime_gaming.prioritize_realtime) {
        score = (cpu_score * p2p_config.realtime_gaming.cpu_weight) +
                (ram_score * p2p_config.realtime_gaming.ram_weight) +
                (network_score * p2p_config.realtime_gaming.network_speed_weight) +
                (latency_score * p2p_config.realtime_gaming.latency_weight) +
                (stability_score * p2p_config.realtime_gaming.stability_weight);
    } else {
        // Default weights if real-time gaming is not prioritized
        score = (cpu_score * 0.2) + (ram_score * 0.2) + (network_score * 0.2) + 
                (latency_score * 0.3) + (stability_score * 0.1);
    }
    
    return score;
}
```

## Real-time Gaming Experience Optimization

The system includes specialized optimizations for real-time gaming:

1. **Jitter Monitoring**: Tracks network stability through jitter measurements
2. **Packet Rate Analysis**: Ensures hosts can process sufficient packets per second
3. **Frame Time Tracking**: Monitors rendering performance for smooth gameplay
4. **Weighted Scoring**: Configurable weights for different performance metrics
5. **Exponential Latency Penalty**: Heavily penalizes high-latency hosts using exponential decay

## Host Migration Process

When a host disconnects or becomes unstable:

1. The system detects the issue through regular health checks
2. A new host is immediately selected from backup candidates
3. Map state is synchronized to the new host
4. Players are seamlessly redirected to the new host
5. If no suitable P2P hosts are available, the map falls back to VPS hosting

## Security Validation

To prevent cheating, the system implements comprehensive security checks:

1. **Spawn Rate Validation**: Ensures monster spawn rates match server settings
2. **Drop Rate Validation**: Verifies item drop rates are not manipulated
3. **Monster Stats Validation**: Checks monster HP and stats against database
4. **Player Movement Validation**: Prevents speed hacking and position manipulation
5. **Skill Usage Validation**: Monitors skill cooldowns and effects

## Configuration

The system is highly configurable through `p2p_map_config.conf`:

```
//================= P2P Map Hosting Configuration ===================
// Configuration file for rAthena P2P-first map hosting system

//----------------------- General Settings -----------------------
general: {
    // Enable P2P hosting for non-critical maps (1 = Yes, 0 = No)
    enable_p2p_maps: 1
    
    // Always prioritize lowest-latency host (1 = Yes, 0 = No)
    prefer_low_latency: 1
    
    // Enable automatic host migration if current host disconnects (1 = Yes, 0 = No)
    enable_auto_migration: 1
    
    // Enable VPS backup for non-critical maps (1 = Yes, 0 = No)
    enable_vps_fallback: 1
    
    // P2P task validation interval (seconds)
    data_check_interval: 5
    
    // Enable security verification for P2P hosts (1 = Yes, 0 = No)
    enable_p2p_security_checks: 1
    
    // Persistence between server restarts (1 = Yes, 0 = No)
    persistent_hosts: 1
}

//----------------------- Host Requirements -----------------------
host_requirements: {
    // Minimum CPU requirement (GHz)
    cpu_min_ghz: 3.0
    
    // Minimum core count
    cpu_min_cores: 4
    
    // Minimum free RAM (MB)
    ram_min_mb: 8192
    
    // Minimum network speed (Mbps)
    net_min_speed: 100
    
    // Maximum allowed latency (ms)
    net_max_latency: 30
    
    // Maximum allowed session instability (disconnects per hour)
    max_disconnects_per_hour: 1
}

//------------------ Real-time Gaming Experience ------------------
realtime_gaming: {
    // Prioritize real-time gaming experience (1 = Yes, 0 = No)
    prioritize_realtime: 1
    
    // Maximum allowed jitter (ms) for real-time gaming
    max_jitter_ms: 15
    
    // Minimum packet processing rate (packets per second)
    min_packet_rate: 100
    
    // Real-time data synchronization interval (milliseconds)
    realtime_sync_interval_ms: 50
    
    // Maximum allowed frame time (ms) for smooth gameplay
    max_frame_time_ms: 16
    
    // Latency weight in host scoring algorithm (higher = more important)
    latency_weight: 0.35
    
    // Network speed weight in host scoring algorithm
    network_speed_weight: 0.25
    
    // CPU performance weight in host scoring algorithm
    cpu_weight: 0.15
    
    // RAM availability weight in host scoring algorithm
    ram_weight: 0.15
    
    // Stability weight in host scoring algorithm
    stability_weight: 0.10
}

//----------------------- P2P-Eligible Maps -----------------------
// Maps that can be hosted by P2P players
// Format: "map_name": 1 (eligible) or 0 (not eligible)
p2p_eligible_maps: {
    // Field Maps (low player density)
    "moc_fild01": 1
    "pay_fild03": 1
    "gef_fild07": 1
    
    // Dungeon Floors (Non-MVP Levels)
    "pay_dun01": 1
    "gef_dun02": 1
    "moc_pryd01": 1
    
    // Low-Level Training Maps
    "iz_dun01": 1
    "morocc_fild12": 1
    
    // Personal Instance Maps
    "custom_housing": 1
    "player_base": 1
    
    // Guild Hall Rooms
    "guild_room": 1
    
    // Quest-Only Maps
    "job_quest_room": 1
    "test_room": 1
    
    // Critical maps that should always be VPS-hosted
    "prontera": 0
    "geffen": 0
    "payon": 0
    // ... other critical maps ...
}
```

## P2P Host Persistence

The system maintains persistent host data between server restarts using YAML storage:

```yaml
hosts:
  - account_id: 1000123
    cpu_ghz: 3.8
    cpu_cores: 8
    free_ram_mb: 16384
    network_speed_mbps: 500
    latency_ms: 15
    disconnects_per_hour: 0
    jitter_ms: 5
    packet_rate: 1200
    frame_time_ms: 12
    vps_connection_active: true
    regional_latencies:
      - region: "NA"
        latency_ms: 15
        relay_address: "relay-na.example.com"
      - region: "EU"
        latency_ms: 120
        relay_address: "relay-eu.example.com"
    assigned_maps:
      - "moc_fild01"
      - "pay_dun01"
```

## Implementation Details

### Host Eligibility Check

```cpp
bool P2PHost::isEligible(const P2PHostConfig& config) const {
    // Get the P2P map configuration
    const auto& p2p_config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Basic eligibility checks
    bool basic_eligible = stats.cpu_ghz >= config.min_cpu_ghz &&
           stats.cpu_cores >= config.min_cpu_cores &&
           stats.free_ram_mb >= config.min_ram_mb &&
           stats.network_speed_mbps >= config.min_network_mbps &&
           stats.latency_ms <= config.max_latency_ms &&
           stats.disconnects_per_hour <= config.max_disconnects_per_hour &&
           isSecurityValid();
    
    // If real-time gaming is prioritized, perform additional checks
    if (basic_eligible && p2p_config.realtime_gaming.prioritize_realtime) {
        // Check jitter (network stability)
        if (stats.jitter_ms > p2p_config.realtime_gaming.max_jitter_ms) {
            return false;
        }
        // Check packet processing rate
        if (stats.packet_rate < p2p_config.realtime_gaming.min_packet_rate) {
            return false;
        }
    }
    
    return basic_eligible;
}
```

### Load Redistribution

```cpp
void P2PHostManager::redistributeLoad() {
    const auto& p2p_config = rathena::P2PMapConfigParser::getInstance().getConfig();
    
    // Get all maps that are currently P2P hosted
    std::vector<std::string> hosted_maps;
    for (const auto& pair : map_hosts) {
        if (!pair.second.empty()) {
            hosted_maps.push_back(pair.first);
        }
    }
    
    // For each map, check if there's a better host available
    for (const auto& map_name : hosted_maps) {
        auto current_host_id = map_hosts[map_name].front();
        auto current_host = getHost(current_host_id);
        
        if (!current_host) continue;
        
        // Find the best host for this map
        std::shared_ptr<P2PHost> best_host = nullptr;
        double best_score = calculateHostScore(current_host);
        
        for (const auto& pair : hosts) {
            auto host = pair.second;
            double score = calculateHostScore(host);
            
            // Determine migration threshold based on real-time gaming priority
            double migration_threshold;
            if (p2p_config.realtime_gaming.prioritize_realtime) {
                // Lower threshold (10%) for real-time gaming to ensure optimal experience
                migration_threshold = 1.1;
            } else {
                // Higher threshold (20%) for non-real-time to reduce unnecessary migrations
                migration_threshold = 1.2;
            }
            
            // Migrate if the new host is better by the threshold percentage
            if (score > best_score * migration_threshold) {
                best_host = host;
                best_score = score;
            }
        }
    }
}
```

## Benefits

1. **Reduced Server Load**: Offloads non-critical maps to player machines
2. **Lower Latency**: Players connect to geographically closer hosts
3. **Improved Scalability**: System scales with player base
4. **Enhanced Reliability**: Multiple failover mechanisms prevent downtime
5. **Optimized Gaming Experience**: Real-time metrics ensure smooth gameplay

## Limitations and Considerations

1. **Security**: While the system implements comprehensive validation, P2P hosting inherently has more security risks than centralized hosting
2. **Network Complexity**: P2P connections may face NAT traversal issues in some network configurations
3. **Host Availability**: System effectiveness depends on having sufficient eligible player hosts
4. **Bandwidth Requirements**: P2P hosts need sufficient upload bandwidth to support multiple players

## Future Enhancements

1. **Regional Hosting Optimization**: Further improve host selection based on geographic regions
2. **Advanced Security Measures**: Implement additional anti-cheat mechanisms
3. **Dynamic Resource Allocation**: Adjust resource requirements based on map complexity and player count
4. **Host Incentives**: Reward system for players who host maps
5. **Predictive Migration**: Use machine learning to predict host disconnections before they occur

## Conclusion

The P2P-first map hosting system provides a robust, scalable solution for distributing map hosting in rAthena. By prioritizing player-hosted maps while maintaining strong security measures and seamless failover mechanisms, the system significantly improves player experience while reducing server load.