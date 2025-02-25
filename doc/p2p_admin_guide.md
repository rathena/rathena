# P2P-First Map Server Administration Guide

## Overview
The P2P-first map server system allows rAthena servers to leverage player resources for hosting non-critical maps. This distributed approach improves scalability and resilience while reducing server load.

## System Architecture

### Components
1. **P2P Host Manager**
   - Manages host registration and status
   - Performs health checks and validation
   - Handles host selection and load balancing

2. **P2P Integration**
   - Manages VPS connectivity
   - Handles regional latency optimization
   - Coordinates map migrations

3. **Security System**
   - Real-time gameplay validation
   - Host behavior monitoring
   - Automatic blacklisting

## Configuration

### Map Eligibility (p2p_map_eligibility.conf)
```ini
// Map eligibility levels
// 1 = Basic (3.0GHz, 4 cores, 8GB RAM, 100Mbps)
// 2 = Enhanced (3.5GHz, 6 cores, 16GB RAM, 200Mbps)
// 3 = Premium (4.0GHz, 8 cores, 32GB RAM, 1000Mbps)

// Example configurations
prontera: 0          // Never P2P hosted
p2p_test: 1          // Basic hosting requirements
guild_castle: 2      // Enhanced requirements
custom_event: 3      // Premium requirements

// Restricted maps (never P2P hosted)
restricted_maps: {
    prontera
    alberta
    payon
}
```

### Performance Requirements (p2p_map_config.conf)
```ini
// Host Requirements
cpu_min_ghz: 3.0
cpu_min_cores: 4
ram_min_gb: 8
network_min_mbps: 100
max_latency_ms: 100

// Regional Settings
relay_servers: {
    NA: "na-relay.example.com"
    EU: "eu-relay.example.com"
    ASIA: "asia-relay.example.com"
}

// Load Balancing
max_maps_per_host: 5
load_check_interval: 60
```

## Administration Commands

### System Status
```
@p2pstatus
- Shows overall P2P system status
- Displays VPS connection state
- Lists your host stats if hosting

@p2plist
- Lists all active P2P hosts
- Shows host performance metrics
- Displays hosted maps
```

### Security Management
```
@p2pvalidate
- Runs security validation on all hosts
- Checks spawn rates, drops, movement
- Reports validation results

@p2pmetrics
- Shows detailed performance metrics
- Reports load distribution
- Displays regional latencies
```

## Monitoring

### Health Checks
The system performs regular checks on:
1. Host CPU and memory usage
2. Network latency and bandwidth
3. Game mechanic validation
4. VPS connectivity

### Logs
Monitor these log files:
```
log/p2p_hosts.log     - Host registration and status
log/p2p_security.log  - Security validation results
log/p2p_metrics.log   - Performance metrics
```

## Security

### Host Validation
Hosts are validated for:
- Spawn rates matching server settings
- Drop rates within acceptable ranges
- Monster stats matching database
- Player movement validity
- Skill usage patterns

### Blacklisting
Hosts are automatically blacklisted when:
- Failing 3 consecutive validations
- Exceeding maximum latency thresholds
- Showing suspicious behavior patterns

## Troubleshooting

### Common Issues

#### No Eligible Hosts
1. Check minimum requirements
2. Verify client metric reporting
3. Check network connectivity
4. Review host logs

#### High Latency
1. Check regional relay settings
2. Monitor network performance
3. Review host selection metrics
4. Check load balancing settings

#### Security Failures
1. Review validation logs
2. Check affected maps
3. Monitor host metrics
4. Verify game mechanics

### Recovery Procedures

#### VPS Failure
1. System automatically switches to P2P hosting
2. Monitors VPS connectivity
3. Syncs data when VPS recovers

#### Host Migration
1. Backup hosts take over immediately
2. Data is synchronized
3. New backup hosts are selected

## Best Practices

### Host Selection
1. Monitor CPU and network usage
2. Track host reliability history
3. Consider geographic distribution
4. Review security validation history

### Load Balancing
1. Configure optimal thresholds
2. Monitor resource distribution
3. Review migration patterns
4. Adjust requirements as needed

### Security
1. Regular validation checks
2. Monitor drop rates
3. Track spawn patterns
4. Verify player movements
5. Review skill usage patterns

## Performance Optimization

### Regional Configuration
1. Set up regional relays
2. Configure latency thresholds
3. Optimize routing paths
4. Monitor regional metrics

### Load Distribution
1. Balance map assignments
2. Monitor host resources
3. Track migration frequency
4. Adjust thresholds as needed

## Maintenance

### Regular Tasks
1. Review security logs daily
2. Monitor host performance metrics
3. Update relay configurations
4. Adjust minimum requirements
5. Review blacklist entries

### Backup Procedures
1. Regular state snapshots
2. Host configuration backups
3. Security log archives
4. Metrics history

## Future Considerations

### Planned Features
1. Dynamic resource allocation
2. Machine learning-based host selection
3. Automated performance optimization
4. Enhanced security validation
5. Regional load prediction

### Integration Points
1. Custom map systems
2. Event management
3. Guild features
4. Economy systems
5. PvP management