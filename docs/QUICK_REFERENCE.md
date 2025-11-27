# Quick Reference Card

**Version**: 1.0.0 | **Last Updated**: 2025-11-26

---

## Service Endpoints

| Service | URL | Port | Protocol |
|---------|-----|------|----------|
| **AI Service** | http://192.168.0.100:8000 | 8000 | HTTP |
| **API Docs** | http://192.168.0.100:8000/docs | 8000 | HTTP |
| **PostgreSQL** | 192.168.0.100:5432 | 5432 | TCP |
| **DragonflyDB** | 192.168.0.100:6379 | 6379 | TCP |
| **Dashboard** | http://localhost:3000 | 3000 | HTTP |
| **Prometheus** | http://192.168.0.100:8000/metrics | 8000 | HTTP |

---

## Common Commands

### Service Management

```bash
# Start all services
sudo systemctl start postgresql dragonfly ai-service
pm2 start ai-dashboard
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start start

# Stop all services
sudo systemctl stop ai-service
pm2 stop ai-dashboard
./athena-start stop

# Restart all services
sudo systemctl restart postgresql dragonfly ai-service
pm2 restart ai-dashboard
./athena-start restart

# Check status
sudo systemctl status postgresql dragonfly ai-service
pm2 status
ps aux | grep -E "(login-server|char-server|map-server)"
```

### Health Checks

```bash
# AI Service health
curl http://192.168.0.100:8000/api/v1/health

# Detailed health
curl http://192.168.0.100:8000/api/v1/health/detailed | jq

# Agent status (all 21 agents)
curl http://192.168.0.100:8000/api/v1/world/agents/status | jq '.agents[] | {name, status}'

# Database connection
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"

# Cache health
redis-cli -h 192.168.0.100 PING

# Dashboard health
curl http://localhost:3000/api/health
```

### Log Viewing

```bash
# AI Service logs (live)
sudo journalctl -u ai-service -f

# Last 100 lines
sudo journalctl -u ai-service -n 100

# Since 1 hour ago
sudo journalctl -u ai-service --since "1 hour ago"

# Errors only
sudo journalctl -u ai-service | grep -i "error\|exception\|critical"

# rAthena logs
tail -f /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/map-msg_log.log

# Dashboard logs
pm2 logs ai-dashboard --lines 50

# PostgreSQL logs
sudo tail -f /var/log/postgresql/postgresql-17-main.log
```

### Database Operations

```bash
# Connect to database
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory

# Backup database
pg_dump -h 192.168.0.100 -U ai_world_user ai_world_memory | \
  gzip > backup_$(date +%Y%m%d_%H%M%S).sql.gz

# Restore database
gunzip -c backup.sql.gz | psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory

# Check connection count
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory \
  -c "SELECT count(*) FROM pg_stat_activity;"

# Vacuum database
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "VACUUM ANALYZE;"
```

### Configuration Reload

```bash
# Hot reload AI service config
curl -X POST http://192.168.0.100:8000/api/v1/admin/reload

# Restart service (full reload)
sudo systemctl restart ai-service

# Reload rAthena config
./athena-start reload
```

---

## Agent Schedule (Daily Tasks)

| Time | Agent | Task |
|------|-------|------|
| **00:00** | Problem | Daily world problem |
| **00:15** | AdaptiveDungeon | Daily dungeon generation |
| **00:30** | MapHazard | Daily hazards |
| **01:00** | Treasure | Daily treasures |
| **02:00** | Archaeology | Dig spots spawn |
| **03:00** | System | Cleanup old data, Karma decay |
| **04:00** | Social | Guild tasks |
| **06:00** | DynamicNPC | Daily NPC spawns |
| **07:00** | Social | Social events |
| **23:59** | DynamicNPC | Despawn daily NPCs |

**Interval Tasks**:
- Every 5m: Faction alignment update
- Every 15m: Boss agent, Karma updates
- Every 30m: Event chain advancement check
- Every 1h: Reputation updates, cleanup
- Every 2h: Event chain evaluation
- Every 3h: Weather updates
- Every 6h: Economy analysis

---

## Important API Endpoints

### Core Endpoints

```bash
# Health check
GET http://192.168.0.100:8000/api/v1/health

# Agent status
GET http://192.168.0.100:8000/api/v1/world/agents/status

# Current story arc
GET http://192.168.0.100:8000/api/v1/storyline/current

# Economy snapshot
GET http://192.168.0.100:8000/api/v1/economy_social/economy/snapshot

# Active world problems
GET http://192.168.0.100:8000/api/v1/world/problems/active

# Active NPCs
GET http://192.168.0.100:8000/api/v1/world/npcs/active

# Player reputation
GET http://192.168.0.100:8000/api/v1/progression/reputation/{player_id}
```

### Admin Endpoints

```bash
# LLM costs
GET http://192.168.0.100:8000/api/v1/admin/llm-costs

# System stats
GET http://192.168.0.100:8000/api/v1/admin/stats

# Reload config
POST http://192.168.0.100:8000/api/v1/admin/reload

# Scheduler status
GET http://192.168.0.100:8000/api/v1/world/scheduler/status
```

---

## Troubleshooting Quick Fixes

| Problem | Quick Fix |
|---------|-----------|
| **Agent inactive** | `sudo systemctl restart ai-service` |
| **High API latency** | Check DB pool: Edit `.env` set `CONNECTION_POOL_SIZE=30` |
| **DB connection error** | `sudo systemctl restart postgresql` |
| **Dashboard not loading** | `pm2 restart ai-dashboard` |
| **Bridge layer timeout** | Check AI service: `curl http://192.168.0.100:8000/api/v1/health` |
| **High LLM costs** | Edit `.env` set `LLM_OPTIMIZATION_MODE=aggressive` |
| **Memory leak** | `sudo systemctl restart ai-service` |
| **Circuit breaker open** | Wait 60s or `sudo systemctl restart ai-service` |
| **WebSocket disconnect** | Check `wscat -c ws://192.168.0.100:8000/ws` |
| **rAthena crash** | `./athena-start restart && tail -f log/*.log` |

---

## File Paths

### Important Directories

```
# AI Service
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/

# rAthena
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/

# Dashboard
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard/

# Logs
/var/log/ai-service/
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/log/

# Backups
/opt/backups/

# Configuration
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/conf/
/opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/.env
```

### Configuration Files

```
# AI Service config
.env                    # Main configuration
config.py              # Python settings

# Bridge Layer config
conf/ai_bridge.conf    # C++ bridge settings

# Dashboard config
.env.local             # Next.js environment

# Database config
/etc/postgresql/17/main/postgresql.conf
/etc/postgresql/17/main/pg_hba.conf
```

---

## Performance Targets

| Metric | Target | Alert Threshold |
|--------|--------|-----------------|
| **API Response (p95)** | <250ms | >500ms |
| **Uptime** | >99.5% | <99% |
| **Agent Success Rate** | >95% | <90% |
| **DB Connections** | <80% max | >90% max |
| **LLM Daily Cost** | <$40 | >$50 |
| **LLM Monthly Cost** | <$1,200 | >$1,400 |
| **Memory Usage** | <80% | >90% |
| **Disk Usage** | <80% | >85% |
| **Error Rate** | <1% | >5% |

---

## Database Connection Strings

```bash
# PostgreSQL (production)
postgresql://ai_world_user:password@192.168.0.100:5432/ai_world_memory

# DragonflyDB (Redis-compatible)
redis://192.168.0.100:6379/0

# PostgreSQL admin
postgresql://postgres:admin_password@192.168.0.100:5432/postgres
```

---

## Emergency Procedures

### Complete System Restart

```bash
# 1. Stop everything
sudo systemctl stop ai-service
pm2 stop all
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start stop

# 2. Restart databases
sudo systemctl restart postgresql dragonfly

# 3. Start AI service
sudo systemctl start ai-service
sleep 10  # Wait for startup

# 4. Start game servers
./athena-start start

# 5. Start dashboard
pm2 restart ai-dashboard

# 6. Verify all healthy
curl http://192.168.0.100:8000/api/v1/health
```

### Emergency Rollback

```bash
# Execute rollback script
cd /opt/ai-mmorpg
./scripts/emergency-rollback.sh

# Or manual rollback:
# 1. Stop services
sudo systemctl stop ai-service
./athena-start stop
pm2 stop ai-dashboard

# 2. Restore database
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory < \
  /opt/backups/$(date +%Y%m%d -d "1 day ago")/ai_world_memory.sql.gz

# 3. Switch to previous version
cd /opt/ai-mmorpg
rm active && ln -s v0.9.0 active

# 4. Start services
cd active
sudo systemctl start ai-service
./athena-start start
pm2 start ai-dashboard
```

### Force Kill All Processes

```bash
# Use only as last resort
pkill -9 -f "uvicorn"
pkill -9 -f "python.*ai-service"
pkill -9 -f "login-server"
pkill -9 -f "char-server"
pkill -9 -f "map-server"
pkill -9 -f "node.*dashboard"

# Then restart normally
sudo systemctl start postgresql dragonfly ai-service
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start start
pm2 start ai-dashboard
```

---

## Monitoring Commands

### System Resources

```bash
# CPU, Memory, Processes
htop

# Memory usage
free -h

# Disk usage
df -h

# Network connections
netstat -tuln | grep -E "(8000|5432|6379|3000)"

# Top CPU processes
ps aux --sort=-%cpu | head -10

# Top memory processes
ps aux --sort=-%mem | head -10
```

### Performance Metrics

```bash
# API metrics
curl -s http://192.168.0.100:8000/metrics | grep "http_request"

# Database stats
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory << EOF
SELECT schemaname, tablename, n_tup_ins, n_tup_upd, n_tup_del
FROM pg_stat_user_tables
ORDER BY n_tup_ins DESC
LIMIT 10;
EOF

# Cache stats
redis-cli -h 192.168.0.100 INFO stats

# LLM cost tracking
curl -s http://192.168.0.100:8000/api/v1/admin/llm-costs | jq '{
  daily: .daily_cost_usd,
  monthly: .monthly_projection_usd,
  budget: .budget_usage_percent
}'
```

---

## Environment Variables (Key Settings)

```bash
# Database
DATABASE_URL=postgresql://ai_world_user:password@192.168.0.100:5432/ai_world_memory
REDIS_URL=redis://192.168.0.100:6379/0

# LLM Provider
DEFAULT_LLM_PROVIDER=deepseek  # openai, anthropic, deepseek, groq, xai
DEFAULT_LLM_MODEL=deepseek-chat

# Optimization
LLM_OPTIMIZATION_MODE=balanced  # aggressive, balanced, quality
REASONING_DEPTH=deep            # shallow, medium, deep

# Features (21 agents)
DIALOGUE_AGENT_ENABLED=true
DECISION_AGENT_ENABLED=true
MEMORY_AGENT_ENABLED=true
WORLD_AGENT_ENABLED=true
QUEST_AGENT_ENABLED=true
ECONOMY_AGENT_ENABLED=true
PROBLEM_AGENT_ENABLED=true
DYNAMIC_NPC_AGENT_ENABLED=true
WORLD_EVENT_AGENT_ENABLED=true
FACTION_AGENT_ENABLED=true
REPUTATION_AGENT_ENABLED=true
DYNAMIC_BOSS_AGENT_ENABLED=true
MAP_HAZARD_AGENT_ENABLED=true
TREASURE_AGENT_ENABLED=true
WEATHER_AGENT_ENABLED=true
KARMA_AGENT_ENABLED=true
ARCHAEOLOGY_AGENT_ENABLED=true
ADAPTIVE_DUNGEON_AGENT_ENABLED=true
EVENT_CHAIN_AGENT_ENABLED=true
SOCIAL_AGENT_ENABLED=true
ECONOMY_SOCIAL_AGENT_ENABLED=true

# Performance
MAX_WORKERS=8
CONNECTION_POOL_SIZE=20
DECISION_CACHE_ENABLED=true
DECISION_BATCH_ENABLED=true
```

---

## Backup & Recovery

```bash
# Daily backup (automated via cron)
/opt/scripts/backup-system.sh

# Manual backup
pg_dump -h 192.168.0.100 -U ai_world_user ai_world_memory | \
  gzip > /opt/backups/manual_$(date +%Y%m%d_%H%M%S).sql.gz

# List backups
ls -lh /opt/backups/

# Restore from backup
gunzip -c /opt/backups/20251126/ai_world_memory.sql.gz | \
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory

# Test backup integrity
gunzip -c backup.sql.gz | head -100 | grep -E "(CREATE TABLE|INSERT INTO)"
```

---

## Contact Information

### Emergency Contacts

| Role | Contact | Method |
|------|---------|--------|
| **On-Call Engineer** | Primary rotation | PagerDuty, Slack |
| **AI Service Team** | ai-service@domain.com | Email, Slack |
| **Database Admin** | dba@domain.com | Email, Phone |
| **DevOps Team** | devops@domain.com | Email, Slack |

### Communication Channels

- **Slack**: #ai-world-support (general), #ai-world-oncall (urgent)
- **PagerDuty**: https://yourorg.pagerduty.com
- **Status Page**: https://status.yourdomain.com
- **Documentation**: https://docs.yourdomain.com

---

## Version Information

| Component | Version | Notes |
|-----------|---------|-------|
| **System** | v1.0.0 | Production ready |
| **PostgreSQL** | 17.6 | Main database |
| **Python** | 3.12+ | AI service |
| **Node.js** | 18+ | Dashboard |
| **DragonflyDB** | 1.14.0 | Redis-compatible cache |
| **rAthena** | 2025 | Game server |

---

## Quick Diagnostic Checklist

When something goes wrong, check in this order:

1. ☐ All services running? `sudo systemctl status postgresql dragonfly ai-service && pm2 status`
2. ☐ AI service healthy? `curl http://192.168.0.100:8000/api/v1/health`
3. ☐ All agents active? `curl http://192.168.0.100:8000/api/v1/world/agents/status | jq`
4. ☐ Database connected? `psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"`
5. ☐ Recent errors? `sudo journalctl -u ai-service --since "10 minutes ago" | grep -i error`
6. ☐ Disk space OK? `df -h | grep -E '(Filesystem|/opt|/var)'`
7. ☐ Memory OK? `free -h`
8. ☐ Network OK? `ping -c 3 192.168.0.100`

If all checks pass but issue persists → Check [Operations Runbook](OPERATIONS_RUNBOOK.md)

---

## Useful One-Liners

```bash
# Count errors in last hour
sudo journalctl -u ai-service --since "1 hour ago" | grep -c -i "error"

# Get agent with most failures
curl -s http://192.168.0.100:8000/api/v1/admin/stats | \
  jq '.agent_failure_counts | to_entries | max_by(.value)'

# Check if LLM budget OK
curl -s http://192.168.0.100:8000/api/v1/admin/llm-costs | \
  jq '.budget_usage_percent | if . > 85 then "🔴 OVER BUDGET" elif . > 70 then "⚠️ WARNING" else "✅ OK" end'

# Find slowest API endpoints
sudo journalctl -u ai-service --since "1 hour ago" | \
  grep "duration_ms" | awk '{print $(NF-1), $NF}' | sort -t: -k2 -rn | head -5

# Database connection pool usage
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c \
  "SELECT count(*)::float / (SELECT setting::int FROM pg_settings WHERE name='max_connections') * 100 AS pool_usage_percent FROM pg_stat_activity;"

# Check if any service crashed recently
sudo journalctl --since "1 hour ago" | grep -i "failed\|crash\|killed"
```

---

**Quick Reference v1.0.0** | [Full Documentation](README.md) | [Deployment Guide](PRODUCTION_DEPLOYMENT_GUIDE.md) | [Operations Runbook](OPERATIONS_RUNBOOK.md)