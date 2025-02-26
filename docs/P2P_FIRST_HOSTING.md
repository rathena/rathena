# P2P-First Hosting System for rAthena

This document describes the implementation of a P2P-first hosting system for rAthena map servers, where players with high-performance machines dynamically host maps as the first priority.

## Overview

The P2P-first hosting system allows eligible players to host maps instead of relying solely on VPS servers. This approach offers several advantages:

1. **Reduced Server Load**: By distributing map hosting across player machines, the central server experiences reduced load.
2. **Lower Latency**: Players connect to the lowest-latency host available, improving gameplay experience.
3. **Scalability**: The system scales automatically with the number of players.
4. **Fault Tolerance**: If a P2P host disconnects, the system seamlessly migrates to another host or falls back to VPS.

## Architecture

The system consists of the following components:

1. **P2PMapServer**: Manages map hosting and player connections.
2. **P2PHostManager**: Manages P2P hosts, their eligibility, and assignments.
3. **P2PHost**: Represents a player's machine that can host maps.
4. **P2PMapConfig**: Configuration for the P2P hosting system.

## P2P-First Approach

When a player enters a map, the system follows this decision flow:

1. Check if the map is eligible for P2P hosting.
2. If eligible and P2P hosting is enabled, find the best P2P host for the map.
3. If a suitable P2P host is found, connect the player to that host.
4. If no suitable P2P host is found, fall back to VPS hosting.

## Host Selection

The system selects the best host for a map based on the following criteria:

1. **Performance Metrics**: CPU speed, core count, available RAM, network speed.
2. **Latency**: Lower latency hosts are preferred.
3. **Stability**: Hosts with fewer disconnects are preferred.
4. **Security**: Hosts must pass security validation checks.

A composite score is calculated for each eligible host, and the host with the highest score is selected.

## Failover Handling

If a P2P host disconnects or becomes unavailable, the system automatically migrates the map to another host:

1. The system maintains a list of backup hosts for each map.
2. When the primary host disconnects, the system selects the best backup host.
3. If no backup host is available, the system falls back to VPS hosting.
4. The migration process is seamless, with no player disconnection.

## Security Measures

To prevent cheating or manipulation by P2P hosts, the system implements several security measures:

1. **Server-Side Validation**: Critical game data (EXP, loot, quest progress) remains server-verified.
2. **Regular Checks**: The system performs regular validation checks on P2P hosts.
3. **Blacklisting**: Hosts that fail validation checks are blacklisted from future hosting.

## Configuration

The system is highly configurable through the `p2p_map_config.conf` file, which allows administrators to adjust:

1. **Eligibility Criteria**: Minimum requirements for P2P hosts.
2. **Map Eligibility**: Which maps can be P2P-hosted.
3. **Failover Settings**: How backup hosts are selected and when to fall back to VPS.
4. **Monitoring Settings**: How often to check host health and collect metrics.

## P2P-Eligible Maps

By default, the following types of maps are eligible for P2P hosting:

1. Field Maps with low player density (e.g., moc_fild01, pay_fild03, gef_fild07)
2. Non-MVP Dungeon Floors (e.g., pay_dun01, gef_dun02, moc_pryd01)
3. Low-Level Training Maps (e.g., iz_dun01, morocc_fild12)
4. Personal Instance Maps (e.g., custom_housing, player_base)
5. Guild Hall Rooms (e.g., guild_room)
6. Quest-Only Maps (e.g., job_quest_room, test_room)

Critical maps like main cities and important dungeons remain VPS-hosted by default.

## Implementation Details

### P2P Host Registration

When a player enters a map, the system checks if they meet the minimum requirements to be a P2P host. If they do, they are registered as a potential host and may be assigned to host maps.

### Host Metrics Collection

The system collects performance metrics from player clients, including:

1. CPU speed and core count
2. Available RAM
3. Network speed and latency
4. Stability (disconnects per hour)

These metrics are used to calculate a host score for each player.

### Map Assignment

Maps are assigned to the best available host based on the host score. If no suitable host is found, the map is hosted on the VPS.

### Host Migration

If a host disconnects, the system automatically migrates the map to another host. The migration process is designed to be seamless, with no player disconnection.

### VPS Fallback

The system always maintains a VPS fallback option for each map. If no suitable P2P host is available, or if all P2P hosts fail, the map is hosted on the VPS.

## Future Enhancements

1. **Regional Hosting**: Implement region-based host selection to further reduce latency.
2. **Dynamic Task Distribution**: Distribute specific tasks (NPC processing, monster AI, etc.) across multiple P2P nodes.
3. **Advanced Security Measures**: Implement more sophisticated anti-cheat measures for P2P hosts.
4. **Performance Optimization**: Optimize the host selection algorithm for better performance.