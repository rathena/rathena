# Operations Runbook

**Version**: 1.0.0  
**Last Updated**: 2025-11-26  
**Target Audience**: Operations Team, System Administrators, On-Call Engineers

---

## Table of Contents

1. [Daily Operations](#daily-operations)
2. [Monitoring & Alerts](#monitoring--alerts)
3. [Troubleshooting Common Issues](#troubleshooting-common-issues)
4. [Maintenance Tasks](#maintenance-tasks)
5. [Scaling Procedures](#scaling-procedures)
6. [Emergency Procedures](#emergency-procedures)
7. [Runbook Quick Reference](#runbook-quick-reference)

---

## Daily Operations

### Morning Health Checks (15 minutes)

**Time**: 08:00 - 08:15 daily

#### 1. Service Status Check

```bash
# Check all services
sudo systemctl status postgresql dragonfly ai-service ai-dashboard

# Expected: All services "active (running)"
```

#### 2. Agent Status Verification

```bash
# Get agent status
curl -s http://192.168.0.100:8000/api/v1/world/agents/status | jq '.agents[] | {name, status}'

# Expected: All 21 agents with status "active"
```

**Agent List (21 total)**:
- Core: Dialogue, Decision, Memory, World, Quest, Economy (6)
- Procedural: Problem, DynamicNPC, WorldEvent, Faction, Reputation, DynamicBoss, MapHazard, Treasure, Weather, Karma, Archaeology, AdaptiveDungeon, EventChain, Social, EconomySocial (15)

#### 3. Database Health

```bash
# PostgreSQL connection count
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT count(*) as active_connections, max_connections 
FROM pg_stat_activity, (SELECT setting::int as max_connections FROM pg_settings WHERE name='max_connections') s
GROUP BY max_connections;
EOF

# DragonflyDB status
redis-cli -h 192.168.0.100 INFO stats | grep -E "total_connections_received|instantaneous_ops_per_sec"
```

**Healthy Indicators**:
- PostgreSQL: <80% max connections
- DragonflyDB: <1000 ops/sec (normal load)

#### 4. Review Error Logs

```bash
# AI Service errors (last 24 hours)
sudo journalctl -u ai-service --since "24 hours ago" | grep -i "error\|critical\|exception" | wc -l

# Bridge Layer errors
tail -100 /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/map-msg_log.log | grep -i "error\|fail"

# Dashboard errors
pm2 logs ai-dashboard --lines 50 --nostream | grep -i "error"
```

**Action Required If**:
- >50 errors in AI service logs → Investigate immediately
- Bridge Layer connection failures → Check AI service connectivity
- Dashboard crashes → Check API connectivity

#### 5. LLM Budget Check

```bash
# Get cost report
curl -s http://192.168.0.100:8000/api/v1/admin/llm-costs | jq '{
  total_cost: .total_cost_usd,
  daily_average: .daily_average_usd,
  monthly_projection: .monthly_projection_usd,
  budget_usage_percent: .budget_usage_percent
}'
```

**Budget Thresholds**:
- ✅ Green: <70% monthly budget ($1,050 / $1,500)
- ⚠️ Yellow: 70-85% ($1,050-$1,275)
- 🔴 Red: >85% (>$1,275)

**Action Required If Red**:
- Review LLM call patterns
- Switch to `LLM_OPTIMIZATION_MODE=aggressive`
- Reduce `REASONING_DEPTH` to `medium` or `shallow`

#### 6. Dashboard Access Verification

```bash
# Check dashboard
curl -s http://localhost:3000/api/health | jq

# Check WebSocket
wscat -c ws://192.168.0.100:8000/ws
# Press Ctrl+C after connection confirmed
```

#### 7. Player Feedback Monitoring

**Sources to Check**:
- In-game feedback system (if available)
- Discord/Forums player reports
- Support tickets related to AI content

**Look For**:
- Repetitive content complaints
- NPC behavior issues
- Quest/event bugs
- Economy imbalances

---

### Afternoon Performance Review (10 minutes)

**Time**: 14:00 - 14:10 daily

#### 1. Performance Metrics

```bash
# API response times
curl -s http://192.168.0.100:8000/metrics | grep "http_request_duration" | tail -5

# Database query performance
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT query, calls, mean_exec_time, max_exec_time
FROM pg_stat_statements
ORDER BY mean_exec_time DESC
LIMIT 10;
EOF
```

**Healthy Indicators**:
- API p95: <250ms
- Database queries: <15ms average

#### 2. System Resources

```bash
# CPU, Memory, Disk
ssh ai-server "top -bn1 | head -20"
ssh ai-server "free -h"
ssh ai-server "df -h | grep -E '(Filesystem|/opt|/var)'"
```

**Action Required If**:
- CPU >80% sustained → Investigate process, consider scaling
- Memory >90% → Check for memory leaks, restart services
- Disk >85% → Clean old logs, extend storage

#### 3. Content Generation Review

```bash
# Check daily content generation
curl -s http://192.168.0.100:8000/api/v1/world/problems/active | jq '.problems | length'
curl -s http://192.168.0.100:8000/api/v1/world/npcs/active | jq '.npcs | length'
curl -s http://192.168.0.100:8000/api/v1/world/events/active | jq '.events | length'
```

**Expected Ranges**:
- World problems: 1-3 active
- Dynamic NPCs: 20-50 active
- World events: 0-5 active

---

### Evening System Check (10 minutes)

**Time**: 20:00 - 20:10 daily

#### 1. Story Arc Status

```bash
# Check current story arc
curl -s http://192.168.0.100:8000/api/v1/storyline/current | jq '{
  arc_id: .arc_id,
  current_chapter: .current_chapter,
  total_chapters: .total_chapters,
  player_participation: .player_participation_count,
  hero_of_week: .hero_of_week
}'
```

#### 2. Economy Health

```bash
# Economy snapshot
curl -s http://192.168.0.100:8000/api/v1/economy_social/economy/snapshot | jq '{
  inflation_rate: .inflation_rate,
  zeny_circulation: .total_zeny_in_circulation,
  market_health: .market_health_score
}'
```

**Action Required If**:
- Inflation rate >10%: Review price adjustments
- Market health <0.6: Investigate supply/demand issues

#### 3. Backup Verification

```bash
# Check last backup
ls -lh /opt/backups/$(date +%Y%m%d)/ 2>/dev/null || echo "No backup today!"

# Verify backup size (should be >100MB)
du -sh /opt/backups/$(date +%Y%m%d)/
```

**Action Required If**:
- No backup found → Investigate backup script
- Backup <50MB → Possible incomplete backup

---

## Monitoring & Alerts

### Prometheus Metrics to Watch

#### Critical Metrics (Alert if exceeded)

```yaml
# HTTP Request Duration (p95 > 250ms)
http_request_duration_seconds{quantile="0.95"} > 0.25

# Error Rate (>5%)
rate(http_requests_total{status=~"5.."}[5m]) / rate(http_requests_total[5m]) > 0.05

# Database Connection Pool (>80%)
database_connections_active / database_connections_max > 0.8

# Memory Usage (>90%)
process_resident_memory_bytes / node_memory_MemTotal_bytes > 0.9

# Disk Usage (>85%)
node_filesystem_avail_bytes / node_filesystem_size_bytes < 0.15

# LLM API Failures (>10%)
rate(llm_api_calls_total{status="error"}[5m]) / rate(llm_api_calls_total[5m]) > 0.1
```

#### Warning Metrics (Investigate if sustained)

```yaml
# High CPU (>70% for 5 minutes)
rate(process_cpu_seconds_total[5m]) > 0.7

# Slow Queries (>50ms average)
database_query_duration_seconds{quantile="0.5"} > 0.05

# High LLM Costs (>$50/day)
llm_cost_daily_usd > 50

# Agent Failures (>5 in 1 hour)
increase(agent_execution_failures_total[1h]) > 5
```

### Alert Thresholds & Response

| Severity | Threshold | Response Time | Action |
|----------|-----------|---------------|--------|
| **P1 Critical** | Services down | <15 min | Emergency rollback |
| **P2 High** | Performance degraded >50% | <30 min | Investigate, scale if needed |
| **P3 Medium** | Error rate 5-10% | <2 hours | Review logs, fix bugs |
| **P4 Low** | Minor issues | <24 hours | Create ticket, schedule fix |

### Grafana Dashboard Usage

**Access**: http://localhost:3001

**Key Dashboards**:

1. **System Overview**
   - Service health status
   - Request rate & latency
   - Error rates
   - Resource utilization

2. **AI Service Performance**
   - Agent execution times
   - LLM API call rates
   - Decision cache hit rates
   - Optimization tier distribution

3. **Database Metrics**
   - Connection pool usage
   - Query performance
   - Transaction rates
   - Table sizes

4. **Business Metrics**
   - Player engagement
   - Content generation rates
   - Story arc progression
   - Economy health

### Log Aggregation

**Log Locations**:

```bash
# AI Service logs
sudo journalctl -u ai-service -f

# rAthena logs
tail -f /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/*.log

# Dashboard logs
pm2 logs ai-dashboard

# PostgreSQL logs
sudo tail -f /var/log/postgresql/postgresql-17-main.log

# System logs
sudo tail -f /var/log/syslog
```

**Log Analysis Commands**:

```bash
# Count errors by type (last hour)
sudo journalctl -u ai-service --since "1 hour ago" | \
  grep -oP 'ERROR:.*?:' | sort | uniq -c | sort -rn

# Find slowest API calls
sudo journalctl -u ai-service --since "1 hour ago" | \
  grep "duration_ms" | \
  awk '{print $NF}' | \
  sort -rn | head -20

# Database connection errors
sudo journalctl -u ai-service --since "1 hour ago" | \
  grep -i "database\|connection" | \
  grep -i "error\|fail"
```

---

## Troubleshooting Common Issues

### Issue 1: Agent Not Responding

**Symptoms**:
- Agent shows "inactive" in status
- Agent tasks not executing
- Related features not working

**Diagnosis**:

```bash
# Check agent status
curl -s http://192.168.0.100:8000/api/v1/world/agents/status | \
  jq '.agents[] | select(.status != "active")'

# Check scheduler
curl -s http://192.168.0.100:8000/api/v1/world/scheduler/status | jq

# Check agent-specific logs
sudo journalctl -u ai-service | grep "AGENT_NAME" | tail -50
```

**Common Causes & Solutions**:

1. **Agent disabled in config**
   ```bash
   # Check .env
   grep "AGENT_NAME_ENABLED" .env
   # If false, set to true and restart
   ```

2. **Scheduler not running**
   ```bash
   sudo systemctl restart ai-service
   ```

3. **Database connection lost**
   ```bash
   # Test database
   psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"
   # If failed, restart PostgreSQL
   sudo systemctl restart postgresql
   ```

4. **LLM API key invalid**
   ```bash
   # Test API key
   curl -H "Authorization: Bearer $OPENAI_API_KEY" \
     https://api.openai.com/v1/models
   # If 401, update API key in .env
   ```

**Resolution Steps**:
1. Identify root cause from logs
2. Apply fix (update config, restart service, fix database)
3. Verify agent status returns to "active"
4. Test agent functionality

---

### Issue 2: High API Response Times

**Symptoms**:
- Dashboard slow to load
- In-game delays
- p95 latency >500ms

**Diagnosis**:

```bash
# Check current latency
curl -s http://192.168.0.100:8000/metrics | \
  grep "http_request_duration_seconds" | \
  grep "quantile=\"0.95\""

# Identify slow endpoints
sudo journalctl -u ai-service --since "30 minutes ago" | \
  grep "duration_ms" | \
  awk '{print $(NF-1), $NF}' | \
  sort -t: -k2 -rn | head -20

# Check database query performance
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT query, calls, mean_exec_time, max_exec_time
FROM pg_stat_statements
WHERE mean_exec_time > 50
ORDER BY mean_exec_time DESC
LIMIT 10;
EOF
```

**Common Causes & Solutions**:

1. **Database connection pool exhausted**
   ```bash
   # Check pool usage
   psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c \
     "SELECT count(*) FROM pg_stat_activity;"
   
   # Increase pool size in .env
   # CONNECTION_POOL_SIZE=30
   sudo systemctl restart ai-service
   ```

2. **LLM API calls not cached**
   ```bash
   # Check cache hit rate
   curl -s http://192.168.0.100:8000/api/v1/admin/stats | \
     jq '.decision_cache_hit_rate'
   
   # If <50%, enable caching
   # DECISION_CACHE_ENABLED=true
   # DECISION_CACHE_TTL=3600
   ```

3. **Expensive database queries**
   ```bash
   # Add missing indexes
   psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
   CREATE INDEX CONCURRENTLY idx_world_problems_status 
   ON world_problems(status) WHERE status = 'active';
   EOF
   ```

4. **High concurrent load**
   ```bash
   # Scale workers
   # Edit .env: MAX_WORKERS=16
   sudo systemctl restart ai-service
   ```

**Resolution Steps**:
1. Apply appropriate optimization
2. Monitor latency for 10 minutes
3. If still high, proceed to next solution
4. Document optimization in change log

---

### Issue 3: Database Connection Errors

**Symptoms**:
- "connection refused" errors
- "too many connections" errors
- API returns 500 errors

**Diagnosis**:

```bash
# Test direct connection
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT version();"

# Check connection count
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT count(*) as current, 
       (SELECT setting::int FROM pg_settings WHERE name='max_connections') as max
FROM pg_stat_activity;
EOF

# Check for idle connections
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT state, count(*) 
FROM pg_stat_activity 
GROUP BY state;
EOF
```

**Common Causes & Solutions**:

1. **PostgreSQL not running**
   ```bash
   sudo systemctl status postgresql
   sudo systemctl start postgresql
   ```

2. **Max connections reached**
   ```bash
   # Terminate idle connections
   psql -h 192.168.0.100 -U postgres -d ai_world_memory << EOF
   SELECT pg_terminate_backend(pid)
   FROM pg_stat_activity
   WHERE state = 'idle' AND state_change < now() - interval '1 hour';
   EOF
   
   # Increase max connections
   sudo nano /etc/postgresql/17/main/postgresql.conf
   # max_connections = 200
   sudo systemctl restart postgresql
   ```

3. **Connection pool misconfigured**
   ```bash
   # Reduce pool size
   # Edit .env: CONNECTION_POOL_SIZE=10
   sudo systemctl restart ai-service
   ```

4. **Network issues**
   ```bash
   # Test network
   ping -c 5 192.168.0.100
   
   # Check firewall
   sudo ufw status | grep 5432
   ```

**Resolution Steps**:
1. Restore database connectivity
2. Clear connection pool
3. Restart AI service
4. Monitor connection count

---

### Issue 4: Bridge Layer Errors

**Symptoms**:
- `httpget()` / `httppost()` return empty strings
- In-game NPCs not responding
- Events not reaching AI service

**Diagnosis**:

```bash
# Check AI service health
curl http://192.168.0.100:8000/api/v1/health

# Check bridge logs
tail -100 /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/map-msg_log.log | \
  grep -i "ai_bridge\|http"

# Check circuit breaker state (in-game NPC command)
# ai_bridge_status()
```

**Common Causes & Solutions**:

1. **AI service offline**
   ```bash
   sudo systemctl status ai-service
   sudo systemctl start ai-service
   ```

2. **Circuit breaker open**
   ```bash
   # Wait 60 seconds for recovery
   sleep 60
   
   # Or restart to reset
   sudo systemctl restart ai-service
   ```

3. **Network connectivity**
   ```bash
   # Test from rAthena server
   curl http://192.168.0.100:8000/api/v1/health
   
   # Check firewall
   sudo ufw allow from 192.168.0.0/24 to any port 8000
   ```

4. **Configuration mismatch**
   ```bash
   # Verify AI service URL
   cat /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/conf/ai_bridge.conf | \
     grep "base_url"
   ```

**Resolution Steps**:
1. Restore AI service connectivity
2. Wait for circuit breaker recovery
3. Test bridge commands in-game
4. Monitor dead letter queue for lost events

---

### Issue 5: Dashboard Not Loading

**Symptoms**:
- Dashboard shows blank page
- Connection timeout errors
- WebSocket disconnections

**Diagnosis**:

```bash
# Check dashboard service
pm2 status ai-dashboard

# Check dashboard logs
pm2 logs ai-dashboard --lines 100

# Test API connectivity
curl http://192.168.0.100:8000/api/v1/health

# Test WebSocket
wscat -c ws://192.168.0.100:8000/ws
```

**Common Causes & Solutions**:

1. **Dashboard service crashed**
   ```bash
   pm2 restart ai-dashboard
   pm2 monit
   ```

2. **API unreachable**
   ```bash
   # Check .env.local configuration
   cat /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard/.env.local
   
   # Update if needed
   NEXT_PUBLIC_API_URL=http://192.168.0.100:8000
   
   # Rebuild and restart
   cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard
   pnpm build
   pm2 restart ai-dashboard
   ```

3. **WebSocket connection blocked**
   ```bash
   # Check CORS settings in AI service
   # Ensure dashboard domain is allowed
   
   # Test WebSocket directly
   wscat -c ws://192.168.0.100:8000/ws
   ```

4. **Build errors**
   ```bash
   cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard
   rm -rf .next
   pnpm build
   pm2 restart ai-dashboard
   ```

**Resolution Steps**:
1. Restart dashboard service
2. Verify API connectivity
3. Test WebSocket connection
4. Clear browser cache and retry

---

### Issue 6: High LLM Costs

**Symptoms**:
- Daily costs >$50
- Monthly projection >$1,500
- Budget alerts triggered

**Diagnosis**:

```bash
# Get detailed cost breakdown
curl -s http://192.168.0.100:8000/api/v1/admin/llm-costs | jq '{
  daily_cost: .daily_cost_usd,
  monthly_projection: .monthly_projection_usd,
  provider_breakdown: .provider_costs,
  agent_breakdown: .agent_costs,
  tier_distribution: .optimization_tier_distribution
}'

# Check LLM call patterns
curl -s http://192.168.0.100:8000/api/v1/admin/stats | jq '{
  total_llm_calls: .total_llm_calls,
  cache_hit_rate: .decision_cache_hit_rate,
  optimization_mode: .llm_optimization_mode
}'
```

**Common Causes & Solutions**:

1. **Too many Tier 4 (full LLM) decisions**
   ```bash
   # Switch to aggressive optimization
   # Edit .env: LLM_OPTIMIZATION_MODE=aggressive
   sudo systemctl restart ai-service
   ```

2. **Cache not enabled**
   ```bash
   # Enable decision caching
   # Edit .env:
   # DECISION_CACHE_ENABLED=true
   # DECISION_CACHE_TTL=7200
   sudo systemctl restart ai-service
   ```

3. **Deep reasoning on all decisions**
   ```bash
   # Reduce reasoning depth
   # Edit .env: REASONING_DEPTH=medium
   sudo systemctl restart ai-service
   ```

4. **Expensive LLM provider**
   ```bash
   # Switch to DeepSeek (cheaper)
   # Edit .env:
   # DEFAULT_LLM_PROVIDER=deepseek
   # DEFAULT_LLM_MODEL=deepseek-chat
   sudo systemctl restart ai-service
   ```

**Resolution Steps**:
1. Apply cost optimization settings
2. Monitor costs for 24 hours
3. Adjust further if needed
4. Document cost-saving measures

---

### Issue 7: Memory Leak

**Symptoms**:
- Increasing memory usage over time
- OOM errors
- Service crashes

**Diagnosis**:

```bash
# Monitor memory usage
watch -n 5 'free -h && ps aux | grep -E "(uvicorn|python)" | head -10'

# Check for memory leaks in AI service
curl -s http://192.168.0.100:8000/metrics | \
  grep "process_resident_memory_bytes"

# Database memory
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c \
  "SELECT pg_size_pretty(pg_database_size('ai_world_memory'));"
```

**Common Causes & Solutions**:

1. **Cache growing unbounded**
   ```bash
   # Reduce cache TTL
   # Edit .env: DECISION_CACHE_TTL=1800
   sudo systemctl restart ai-service
   ```

2. **Connection pool leaks**
   ```bash
   # Restart service to clear pool
   sudo systemctl restart ai-service
   ```

3. **Long-running queries**
   ```bash
   # Kill long queries
   psql -h 192.168.0.100 -U postgres -d ai_world_memory << EOF
   SELECT pg_terminate_backend(pid)
   FROM pg_stat_activity
   WHERE state = 'active' 
     AND state_change < now() - interval '10 minutes';
   EOF
   ```

4. **Application memory leak**
   ```bash
   # Schedule periodic restarts (cron)
   # Add to crontab: 0 4 * * * systemctl restart ai-service
   ```

**Resolution Steps**:
1. Apply memory optimization
2. Monitor for 24 hours
3. If persists, schedule periodic restarts
4. Report to development team if bug suspected

---

## Maintenance Tasks

### Weekly Tasks (Every Monday, 10:00)

#### 1. Review Agent Performance (30 min)

```bash
# Get agent execution stats
curl -s http://192.168.0.100:8000/api/v1/admin/stats | jq '{
  agent_execution_times: .agent_execution_times,
  agent_success_rates: .agent_success_rates,
  agent_failure_counts: .agent_failure_counts
}'

# Identify underperforming agents
# Success rate <90% OR avg execution time >5s
```

**Action Items**:
- Document any agent with <90% success rate
- Investigate agents with >5s execution time
- Review error logs for failed agents

#### 2. Check Error Logs (20 min)

```bash
# Analyze error patterns
sudo journalctl -u ai-service --since "7 days ago" | \
  grep -i "error\|exception" | \
  awk '{print $5}' | sort | uniq -c | sort -rn | head -20

# Common error types to look for:
# - Database connection errors
# - LLM API failures
# - Timeout errors
# - Validation errors
```

**Action Items**:
- Create tickets for recurring errors (>10 occurrences)
- Prioritize errors affecting core agents
- Document workarounds if available

#### 3. Database Maintenance (15 min)

```bash
# Vacuum and analyze
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
VACUUM ANALYZE;
EOF

# Check table sizes
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT schemaname, tablename, 
       pg_size_pretty(pg_total_relation_size(schemaname||'.'||tablename)) AS size
FROM pg_tables
WHERE schemaname = 'public'
ORDER BY pg_total_relation_size(schemaname||'.'||tablename) DESC
LIMIT 10;
EOF

# Check index usage
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT schemaname, tablename, indexname, idx_scan
FROM pg_stat_user_indexes
WHERE idx_scan = 0 AND schemaname = 'public'
ORDER BY pg_relation_size(indexrelid) DESC
LIMIT 10;
EOF
```

**Action Items**:
- Drop unused indexes (0 scans)
- Archive old data from large tables
- Consider partitioning if tables >10GB

---

### Monthly Tasks (First Monday, 14:00)

#### 1. Cost Analysis (45 min)

```bash
# Generate monthly cost report
curl -s http://192.168.0.100:8000/api/v1/admin/llm-costs?period=monthly | \
  jq > /opt/reports/llm_costs_$(date +%Y%m).json

# Analyze:
# - Total cost vs budget
# - Cost per provider
# - Cost per agent
# - Optimization tier distribution
# - Cost trends
```

**Action Items**:
- Compare to budget ($1,500/month)
- Identify cost optimization opportunities
- Forecast next month's costs
- Present findings to stakeholders

#### 2. Dependency Updates (30 min)

```bash
# Check for security updates
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
source venv/bin/activate
pip list --outdated

# Check dashboard dependencies
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard
pnpm outdated
```

**Action Items**:
- Apply security patches immediately
- Test minor version updates in staging
- Schedule major version upgrades
- Update documentation

#### 3. Backup Verification (20 min)

```bash
# Test backup restoration
mkdir -p /tmp/backup_test
pg_dump -h 192.168.0.100 -U ai_world_user ai_world_memory | \
  head -100 | grep -E "(CREATE TABLE|INSERT INTO)"

# Verify backup retention
find /opt/backups -type f -name "*.sql.gz" -mtime -30 | wc -l
# Should be ~30 (daily backups)

# Check backup sizes
du -sh /opt/backups/$(date +%Y%m%d -d "7 days ago")/
```

**Action Items**:
- Document backup verification results
- Test full restore in staging (quarterly)
- Verify off-site backup replication
- Update disaster recovery procedures

---

### Quarterly Tasks (First Monday of Quarter)

#### 1. Security Audit (2 hours)

```bash
# Review access logs
sudo journalctl --since "3 months ago" | \
  grep -i "authentication\|authorized\|sudo" | \
  grep -v "successful" | wc -l

# Check SSL certificate expiry
openssl x509 -in /etc/letsencrypt/live/yourdomain.com/cert.pem \
  -noout -dates

# Review firewall rules
sudo ufw status numbered
```

**Action Items**:
- Rotate all passwords and API keys
- Review user access permissions
- Update SSL certificates if needed
- Patch security vulnerabilities
- Update security documentation

#### 2. Performance Baseline (1 hour)

```bash
# Run performance tests
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
python tests/performance/benchmark_suite.py

# Document results:
# - API response times (p50, p95, p99)
# - Database query times
# - Agent execution times
# - LLM API latencies
# - System resource usage
```

**Action Items**:
- Compare to previous baseline
- Identify performance regressions
- Create optimization tickets
- Update performance documentation

---

### Yearly Tasks (January)

#### 1. Major Version Upgrades (1 week)

```bash
# Upgrade PostgreSQL
# Upgrade Python
# Upgrade Node.js
# Upgrade rAthena
# Upgrade dependencies
```

**Action Items**:
- Plan upgrade window (low traffic)
- Test upgrades in staging
- Create rollback plan
- Notify stakeholders
- Execute upgrades
- Monitor for issues

#### 2. Disaster Recovery Drill (4 hours)

```bash
# Simulate complete system failure
# Test restoration from backups
# Verify recovery time objectives (RTO)
# Verify recovery point objectives (RPO)
```

**Action Items**:
- Document drill results
- Update DR procedures
- Fix identified gaps
- Train team on DR procedures

---

## Scaling Procedures

### Horizontal Scaling (Add AI Service Instances)

#### Prerequisites
- Load balancer configured
- Shared storage for logs
- Database can handle additional connections

#### Steps

```bash
# 1. Provision new server
# 2. Clone repository and install dependencies
# 3. Copy configuration from primary server
scp /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/.env \
  ai-server-2:/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/

# 4. Start AI service on new server
ssh ai-server-2 "cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service && \
  systemctl start ai-service"

# 5. Add to load balancer
# (Configure your load balancer - HAProxy, Nginx, etc.)

# 6. Verify health
curl http://ai-server-2:8000/api/v1/health
```

**Cost Impact**: +$1,147/month per instance

---

### Database Read Replicas

#### Setup PostgreSQL Streaming Replication

```bash
# On primary server
psql -h 192.168.0.100 -U postgres << EOF
CREATE ROLE replicator WITH REPLICATION LOGIN ENCRYPTED PASSWORD 'replica_password';
EOF

# Configure pg_hba.conf
echo "host replication replicator 192.168.0.101/32 scram-sha-256" | \
  sudo tee -a /etc/postgresql/17/main/pg_hba.conf

# On replica server
pg_basebackup -h 192.168.0.100 -D /var/lib/postgresql/17/main -U replicator -P

# Configure AI service to use replica for reads
# Edit .env:
# DATABASE_READ_URL=postgresql://ai_world_user:password@192.168.0.101:5432/ai_world_memory
```

**Cost Impact**: Minimal (existing hardware)

---

### Load Balancer Configuration

#### HAProxy Example

```bash
# Install HAProxy
sudo apt install -y haproxy

# Configure /etc/haproxy/haproxy.cfg
cat <<EOF | sudo tee -a /etc/haproxy/haproxy.cfg
frontend ai_service
    bind *:8000
    default_backend ai_servers

backend ai_servers
    balance roundrobin
    option httpchk GET /api/v1/health
    server ai1 192.168.0.100:8000 check
    server ai2 192.168.0.101:8000 check
EOF

# Restart HAProxy
sudo systemctl restart haproxy
```

---

### CDN Setup for Dashboard

```bash
# Use CloudFlare, AWS CloudFront, or similar
# Point dashboard.yourdomain.com to CDN
# Configure origin: http://your-server:3000
# Enable caching for static assets
```

**Cost Impact**: ~$20/month (CDN fees)

---

## Emergency Procedures

### Service Outage Response

#### Step 1: Assess Impact (2 minutes)

```bash
# Check all services
sudo systemctl status postgresql dragonfly ai-service ai-dashboard

# Check system resources
uptime && free -h && df -h
```

#### Step 2: Identify Root Cause (5 minutes)

```bash
# Check recent logs
sudo journalctl -u ai-service --since "10 minutes ago" | tail -100
sudo journalctl --since "10 minutes ago" | grep -i "error\|critical"

# Check system events
dmesg | tail -50
```

#### Step 3: Execute Recovery (10 minutes)

**If AI Service Down**:
```bash
sudo systemctl restart ai-service
# Monitor startup
sudo journalctl -u ai-service -f
```

**If Database Down**:
```bash
sudo systemctl restart postgresql
# Wait for startup (30-60s)
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"
```

**If Complete Failure**:
```bash
# Execute emergency rollback
cd /opt/ai-mmorpg
./scripts/emergency-rollback.sh
```

#### Step 4: Notify Stakeholders (2 minutes)

```bash
# Send alert via configured channels
# - Slack: #incidents
# - Email: team@yourdomain.com
# - Status page: status.yourdomain.com
```

#### Step 5: Post-Mortem (After resolution)

Document:
- Timeline of events
- Root cause analysis
- Actions taken
- Preventive measures
- Lessons learned

---

### Data Corruption Recovery

#### Step 1: Stop All Services

```bash
sudo systemctl stop ai-service ai-dashboard
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world
./athena-start stop
```

#### Step 2: Assess Corruption

```bash
# Check database integrity
psql -h 192.168.0.100 -U postgres -d ai_world_memory << EOF
SELECT tablename 
FROM pg_tables 
WHERE schemaname = 'public' 
  AND tablename NOT IN (
    SELECT tablename FROM pg_stat_user_tables WHERE n_tup_ins > 0
  );
EOF
```

#### Step 3: Restore from Backup

```bash
# Drop corrupted database
dropdb -h 192.168.0.100 -U postgres ai_world_memory

# Recreate database
createdb -h 192.168.0.100 -U postgres ai_world_memory

# Restore from latest backup
gunzip -c /opt/backups/$(date +%Y%m%d -d "1 day ago")/ai_world_memory.sql.gz | \
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory
```

#### Step 4: Verify Data Integrity

```bash
# Check table counts
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT 'world_problems' as table_name, COUNT(*) FROM world_problems
UNION ALL
SELECT 'dynamic_npcs', COUNT(*) FROM dynamic_npcs
UNION ALL
SELECT 'world_events', COUNT(*) FROM world_events;
EOF
```

#### Step 5: Restart Services

```bash
sudo systemctl start ai-service
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world
./athena-start start
pm2 restart ai-dashboard
```

---

### Rollback to Previous Version

See [Production Deployment Guide - Rollback Procedures](PRODUCTION_DEPLOYMENT_GUIDE.md#rollback-procedures)

---

## Runbook Quick Reference

### Emergency Commands

```bash
# Restart all services
sudo systemctl restart postgresql dragonfly ai-service
pm2 restart ai-dashboard
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start restart

# Check all service status
sudo systemctl status postgresql dragonfly ai-service && pm2 status

# View live logs
sudo journalctl -u ai-service -f

# Emergency shutdown
sudo systemctl stop ai-service ai-dashboard
./athena-start stop

# Force kill processes
pkill -9 -f "uvicorn\|node"
```

### Health Check Commands

```bash
# AI Service
curl http://192.168.0.100:8000/api/v1/health

# Database
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"

# Cache
redis-cli -h 192.168.0.100 PING

# Dashboard
curl http://localhost:3000/api/health

# Agent Status
curl -s http://192.168.0.100:8000/api/v1/world/agents/status | jq '.agents[] | {name, status}'
```

### Log Locations

```
AI Service:    sudo journalctl -u ai-service
rAthena:       /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/*.log
Dashboard:     pm2 logs ai-dashboard
PostgreSQL:    /var/log/postgresql/postgresql-17-main.log
System:        /var/log/syslog
```

### Common Fixes

| Problem | Quick Fix |
|---------|-----------|
| Agent inactive | `sudo systemctl restart ai-service` |
| High latency | Check database pool: `CONNECTION_POOL_SIZE=30` |
| DB connection error | `sudo systemctl restart postgresql` |
| Dashboard blank | `pm2 restart ai-dashboard` |
| High LLM costs | Set `LLM_OPTIMIZATION_MODE=aggressive` |
| Memory leak | `sudo systemctl restart ai-service` |
| Circuit breaker open | Wait 60s or restart service |

---

## Incident Report Template

```markdown
# Incident Report: [Title]

**Date**: YYYY-MM-DD  
**Severity**: P1/P2/P3/P4  
**Duration**: HH:MM  
**Affected Systems**: [List]

## Summary
[Brief description of what happened]

## Timeline
- HH:MM - [Event 1]
- HH:MM - [Event 2]
- HH:MM - [Resolution]

## Root Cause
[What caused the incident]

## Impact
- Users affected: [Number/Percentage]
- Services affected: [List]
- Data loss: [Yes/No/Details]

## Resolution
[What actions were taken]

## Preventive Measures
1. [Action item 1]
2. [Action item 2]

## Lessons Learned
[What we learned from this incident]

**Reported By**: [Name]  
**Reviewed By**: [Name]
```

---

## Contacts & Escalation

### On-Call Rotation

| Week | Primary | Secondary | Backup |
|------|---------|-----------|--------|
| Week 1 | Engineer A | Engineer B | Engineer C |
| Week 2 | Engineer B | Engineer C | Engineer A |
| Week 3 | Engineer C | Engineer A | Engineer B |
| Week 4 | Engineer A | Engineer B | Engineer C |

### Contact Methods

**Slack**: #ai-world-oncall  
**PagerDuty**: https://yourorg.pagerduty.com  
**Phone**: +1-XXX-XXX-XXXX (emergency only)

### Escalation Path

1. **L1 (0-15 min)**: On-call engineer investigates
2. **L2 (15-30 min)**: Escalate to senior engineer
3. **L3 (30-60 min)**: Escalate to AI service team lead
4. **L4 (>60 min)**: Escalate to CTO, consider rollback

---

**Document Version**: 1.0.0  
**Last Reviewed**: 2025-11-26  
**Next Review**: 2026-02-26  
**Maintained By**: Operations Team