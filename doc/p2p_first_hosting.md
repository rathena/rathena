# Quick Start Guide: P2P Map Hosting

This guide helps you get started with hosting maps in rAthena's P2P-first system.

## Requirements

### Basic Hosting (Level 1)
- CPU: 3.0 GHz or better
- Cores: 4 or more
- RAM: 8GB minimum
- Network: 100 Mbps minimum
- Latency: Under 100ms to server

### Enhanced Hosting (Level 2)
- CPU: 3.5 GHz or better
- Cores: 6 or more
- RAM: 16GB minimum
- Network: 200 Mbps minimum
- Latency: Under 50ms to server

### Premium Hosting (Level 3)
- CPU: 4.0 GHz or better
- Cores: 8 or more
- RAM: 32GB minimum
- Network: 1000 Mbps minimum
- Latency: Under 20ms to server

## Getting Started

1. **Check Eligibility**
   ```
   @p2pstatus
   ```
   This command shows if your system meets hosting requirements.

2. **Register as Host**
   ```
   @p2phost register
   ```
   The system will verify your system specs and network connection.

3. **View Available Maps**
   ```
   @p2plist maps
   ```
   Shows maps eligible for P2P hosting at your level.

4. **Start Hosting**
   ```
   @p2phost start <map_name>
   ```
   Begin hosting a specific map.

## Monitoring Your Hosting

### Performance Metrics
```
@p2pmetrics
```
Shows:
- CPU usage
- Memory usage
- Network stats
- Player count
- Active NPCs

### Security Status
```
@p2pvalidate
```
Verifies:
- Spawn rates
- Drop rates
- Monster stats
- Movement validation
- Skill usage

## Best Practices

### Resource Management
1. Don't run intensive programs while hosting
2. Ensure stable internet connection
3. Monitor system temperatures
4. Keep background processes minimal

### Network Optimization
1. Use wired connection when possible
2. Configure QoS settings for game traffic
3. Monitor bandwidth usage
4. Choose regional relay servers

### Performance Tips
1. Close unnecessary applications
2. Update system drivers
3. Monitor system resources
4. Keep OS and game updated

## Common Issues

### High Latency
1. Check internet connection
2. Use network diagnostics
3. Try different relay servers
4. Monitor network traffic

### Resource Issues
1. Check system resources
2. Close background applications
3. Monitor temperature
4. Check for system updates

### Connection Drops
1. Verify network stability
2. Check firewall settings
3. Update network drivers
4. Monitor connection logs

## Security Guidelines

### DO:
- Keep system updated
- Monitor performance metrics
- Report suspicious activity
- Follow server rules
- Back up host settings

### DON'T:
- Use game modifications
- Run unauthorized scripts
- Share hosting credentials
- Bypass security checks
- Host restricted maps

## Support

### Community Help
- Discord: #p2p-hosting
- Forums: P2P Hosting Section
- In-game: @p2phelp

### Technical Support
- Report issues: @p2preport
- Contact GMs: @p2padmin
- Emergency: @p2pemergency

## Next Steps

1. Read the full [Admin Guide](p2p_admin_guide.md)
2. Join P2P hosting community
3. Learn advanced features
4. Help other hosts

## Quick Reference

### Essential Commands
```
@p2pstatus          - Check system status
@p2phost register   - Register as host
@p2phost start      - Start hosting
@p2phost stop       - Stop hosting
@p2pmetrics         - View performance
@p2pvalidate        - Security check
@p2phelp           - Get help
```

### Status Codes
```
READY     - Ready to host
ACTIVE    - Currently hosting
STANDBY   - Available backup
WARNING   - Performance issues
ERROR     - Security problem
BLOCKED   - Hosting disabled
```

### Performance Indicators
```
Green   - Excellent
Yellow  - Warning
Red     - Critical
Blue    - Initializing
Gray    - Inactive
```

Remember: P2P hosting is a community service. Follow guidelines and maintain good performance to help create a better gaming experience for everyone.