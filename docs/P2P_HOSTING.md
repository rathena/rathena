# P2P Hosting and Monitoring System

This document describes the P2P hosting system implementation for rAthena, including the network monitoring, security features, and integration with FluxCP.

## Architecture Overview

The system consists of three main components:

1. **rAthena Server Components**
   - Network Monitor (`network_monitor.hpp/cpp`)
   - Network Security (`network_security.hpp/cpp`)
   - Network Optimization (`network_optimization.hpp/cpp`)
   - Thread and Synchronization utilities (`thread_guard.hpp`, `sync.hpp`)

2. **FluxCP Control Panel**
   - P2P Hosting Module
   - Network Analytics Interface
   - Security Monitoring Dashboard
   - Host Management Tools

3. **Database Schema**
   - Performance Metrics Tables
   - Security Events Tables
   - System Monitoring Tables
   - Host Management Tables

## Features

### Network Monitoring
- Real-time packet analysis
- Latency tracking
- Bandwidth usage monitoring
- Connection quality metrics
- Performance analytics

### Security Features
- DDoS protection
- Packet validation
- Rate limiting
- IP blacklisting
- Security event logging
- Threat detection and response

### Host Management
- Resource monitoring
- Connection pooling
- Load balancing
- Quality-of-service controls
- Host status tracking

### Analytics and Reporting
- Performance metrics
- Security incident reports
- Resource utilization stats
- Network health monitoring
- Host performance tracking

## Configuration

### rAthena Server
```ini
# conf/network_security.conf
network.monitor.enabled: true
network.monitor.update_interval: 60
network.monitor.cleanup_interval: 300

security.ddos_protection.enabled: true
security.rate_limit.packets: 1000
security.rate_limit.connections: 100
security.rate_limit.bandwidth: 1048576

optimization.compression.enabled: true
optimization.compression.threshold: 512
```

### FluxCP Module
```php
# config/p2p_hosting.php
return array(
    'Enabled' => true,
    'RequiredLevel' => 'trusted',
    'SecurityLevel' => 'high',
    'MonitoringInterval' => 60,
    'MaxHosts' => 100
);
```

## Database Tables

### Performance Monitoring
```sql
p2p_performance
p2p_system_metrics
p2p_connections
```

### Security
```sql
p2p_security_events
p2p_security_alerts
p2p_blacklist
```

### Host Management
```sql
p2p_hosts
p2p_host_metrics
p2p_host_status
```

## Integration

### With rAthena Core
The system integrates with rAthena's networking layer to monitor and optimize network traffic. It uses the following hooks:

- `packet_pre_receive`: Packet validation and DDoS protection
- `packet_post_receive`: Performance metrics collection
- `packet_pre_send`: Traffic shaping and optimization
- `packet_post_send`: Bandwidth monitoring

### With FluxCP
The FluxCP module provides a web interface for:

- Viewing network statistics
- Managing host permissions
- Monitoring security events
- Analyzing performance metrics
- Configuring system settings

## Testing

### Unit Tests
```bash
# Run network monitoring tests
./test_network_monitor

# Run security tests
./test_network_security

# Run integration tests
./test_p2p_hosting
```

### Test Database
A test database schema is provided for development and testing purposes:
```sql
setup_test_db.sql.in
```

## Development

### Adding New Features
1. Implement feature in rAthena core
2. Add corresponding FluxCP interface
3. Update database schema if needed
4. Add unit tests
5. Update documentation

### Code Style
- Follow rAthena coding standards
- Use C++17 features where appropriate
- Maintain thread safety
- Document public interfaces
- Include unit tests

## Troubleshooting

### Common Issues
1. **High Latency**
   - Check network congestion
   - Verify rate limits
   - Monitor host resources

2. **Security Alerts**
   - Review security logs
   - Check blacklist status
   - Verify host integrity

3. **Performance Issues**
   - Monitor system resources
   - Check connection quality
   - Verify optimization settings

### Debug Tools
- Network packet analyzer
- Security event viewer
- Performance profiler
- Resource monitor

## Support

For issues and feature requests:
- GitHub Issues
- rAthena Forums
- Discord Community

## License

This implementation is part of rAthena and is licensed under the GNU General Public License v3.0.