# P2P-First Hosting System Documentation

## Overview
The P2P-first hosting system enhances rAthena by leveraging player-hosted nodes to reduce dependency on the main server (VPS). This system dynamically assigns eligible players as hosts or relays, ensuring optimal performance and minimal latency for a global player base.

## Key Features
- **Collaborative Resource Aggregation**: Combines resources (CPU, RAM, network speed) from active P2P hosts.
- **Automatic Host Selection**: Players connect to the lowest-latency eligible P2P host.
- **Seamless Failover**: Preloaded data ensures zero downtime during host migration.
- **Database Continuity**: P2P hosts handle database updates during VPS downtime, with synchronization upon recovery.
- **Regional Relays**: Players are routed through the best regional relay or proxy based on location.
- **Simultaneous P2P Nodes**: Multiple P2P nodes can host the same map collaboratively.
- **Players as Relays**: Eligible players are automatically assigned as relays to enhance network performance.

## Configuration
The system is configured via `conf/p2p_map_config.conf`. Key settings include:
- `enable_host_disabling`: Enables or disables automatic host disabling.
- `latency_spike_threshold`: Percentage threshold for latency spikes.
- `speed_drop_threshold`: Percentage threshold for speed drops.
- `max_p2p_nodes`: Maximum number of simultaneous P2P nodes per map.
- Per-map settings to enable or disable P2P hosting.
- Regional relay or proxy settings for different continents.
- Critical maps that must stay VPS-hosted.

## How to Use
1. **Enable P2P Hosting**:
   - Open `conf/p2p_map_config.conf`.
   - Set `enable_p2p_maps = 1` to enable P2P hosting.
   - Configure per-map settings to enable or disable P2P hosting for specific maps.

2. **Set Host Criteria**:
   - Define minimum requirements for P2P hosts:
     - `cpu_min_ghz`: Minimum CPU speed in GHz.
     - `cpu_min_cores`: Minimum number of CPU cores.
     - `ram_min`: Minimum free RAM in GB.
     - `net_min_speed`: Minimum network speed in Mbps.
     - `net_max_latency`: Maximum allowed latency in ms.

3. **Configure Regional Relays**:
   - Add relay addresses for different regions in `conf/p2p_map_config.conf`:
     ```
     NA = na-relay.example.com
     EU = eu-relay.example.com
     ASIA = asia-relay.example.com
     ```

4. **Monitor and Manage Hosts**:
   - Use the system's automated monitoring to validate and reassign unstable hosts.
   - Check logs for real-time feedback on host status and task assignments.

## Troubleshooting
- **No Eligible Hosts**:
  - Ensure P2P hosting is enabled and host criteria are correctly configured.
  - Verify that players meet the minimum requirements for hosting.

- **High Latency**:
  - Check regional relay configurations and ensure players are routed through the best relay.

- **VPS Downtime**:
  - Ensure P2P nodes are handling database updates and synchronizing data upon VPS recovery.

## Best Practices
- Regularly update `conf/p2p_map_config.conf` to reflect changes in host criteria and regional relay configurations.
- Monitor system logs to identify and address performance issues promptly.
- Use the `max_p2p_nodes` setting to balance load across multiple P2P nodes.

## Benefits
- Reduces dependency on the VPS, ensuring uninterrupted gameplay.
- Optimizes performance for players across different continents.
- Enhances system reliability and scalability.

For further assistance, refer to the main documentation in the `/doc/` directory or contact the rAthena community.