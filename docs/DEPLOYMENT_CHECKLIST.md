# Deployment Checklist

**Version**: 1.0.0  
**Last Updated**: 2025-11-26  
**Deployment Date**: ________________  
**Deployed By**: ________________

---

## Overview

This checklist ensures all critical steps are completed for a successful production deployment of the Ragnarok Online AI Autonomous World system. Check each box as tasks are completed.

**Reference Documents**:
- 📘 [Production Deployment Guide](PRODUCTION_DEPLOYMENT_GUIDE.md) - Detailed procedures
- 📗 [Operations Runbook](OPERATIONS_RUNBOOK.md) - Daily operations
- 📕 [Administrator Guide](ADMINISTRATOR_GUIDE.md) - Dashboard usage

---

## Pre-Deployment (1-2 Weeks Before)

### Documentation & Planning

- [ ] Review all documentation (Deployment Guide, Operations Runbook, Admin Guide)
- [ ] Create deployment timeline with specific dates/times
- [ ] Identify deployment team members and assign roles
- [ ] Schedule deployment window (recommended: low-traffic period)
- [ ] Notify stakeholders of deployment schedule
- [ ] Prepare communication templates (status updates, incident alerts)

### Infrastructure Preparation

- [ ] Verify server hardware meets requirements
  - [ ] CPU: 8+ cores
  - [ ] RAM: 32+ GB
  - [ ] Storage: 500+ GB SSD
  - [ ] Network: 100+ Mbps
- [ ] Install required software packages
  - [ ] PostgreSQL 17.6
  - [ ] DragonflyDB
  - [ ] Python 3.12+
  - [ ] Node.js 18+
  - [ ] Build tools (gcc, cmake, etc.)
- [ ] Configure network & firewall rules
  - [ ] Open required ports (8000, 5432, 6379, 3000, 6900, 6121, 5121)
  - [ ] Configure internal/external access rules
  - [ ] Set up reverse proxy (if applicable)
- [ ] Obtain SSL certificates
  - [ ] Dashboard HTTPS certificate
  - [ ] WebSocket WSS certificate
  - [ ] Configure auto-renewal

### Access & Credentials

- [ ] Generate secure passwords for all services
  - [ ] PostgreSQL admin password
  [ ] PostgreSQL ai_world_user password
  - [ ] DragonflyDB password (if enabled)
  - [ ] Dashboard admin credentials
- [ ] Obtain LLM API keys
  - [ ] OpenAI API key
  - [ ] Anthropic API key
  - [ ] DeepSeek API key
  - [ ] Groq API key (optional)
  - [ ] xAI API key (optional)
- [ ] Configure SSH key access for deployment team
- [ ] Set up service accounts with appropriate permissions
- [ ] Store credentials in secure vault (LastPass, 1Password, etc.)

### Testing & Validation

- [ ] Set up staging environment matching production
- [ ] Test database backup/restore procedures
- [ ] Verify all 16 migrations apply successfully in staging
- [ ] Test complete system startup/shutdown procedures
- [ ] Run smoke tests in staging environment
- [ ] Load test with expected concurrent users
- [ ] Verify rollback procedures work in staging
- [ ] Test disaster recovery procedures

### Monitoring Setup

- [ ] Install Prometheus
- [ ] Install Grafana
- [ ] Configure alert thresholds
- [ ] Set up alert notifications (email, Slack, PagerDuty)
- [ ] Create monitoring dashboards
- [ ] Test alert delivery
- [ ] Set up log aggregation (optional)

### Backup Configuration

- [ ] Configure automated daily backups
- [ ] Test backup script execution
- [ ] Verify backup retention policy (30 days)
- [ ] Set up off-site backup replication (optional)
- [ ] Document backup restoration procedure
- [ ] Test restore from backup

### Documentation

- [ ] Update all documentation with production details
- [ ] Create on-call rotation schedule
- [ ] Prepare incident response procedures
- [ ] Create emergency contact list
- [ ] Document known issues and workarounds

### Rollback Plan

- [ ] Document complete rollback procedure
- [ ] Prepare blue-green deployment setup
- [ ] Create emergency rollback script
- [ ] Test rollback procedure in staging
- [ ] Define rollback decision criteria
- [ ] Assign rollback decision authority

---

## Deployment Day (D-Day)

### Pre-Deployment (2 hours before)

- [ ] **T-120 min**: Notify all stakeholders deployment is starting
- [ ] **T-120 min**: Post maintenance window notice to players
- [ ] **T-90 min**: Take final backup of all systems
  ```bash
  /opt/scripts/backup-system.sh
  ```
- [ ] **T-90 min**: Verify backup completed successfully
  ```bash
  ls -lh /opt/backups/$(date +%Y%m%d)/
  ```
- [ ] **T-60 min**: Verify deployment team available on Slack/call
- [ ] **T-60 min**: Review deployment checklist with team
- [ ] **T-30 min**: Announce 30-minute warning to players

### Service Shutdown (T-0)

- [ ] **T-0**: Stop accepting new player connections
- [ ] **T-0**: Wait for active sessions to complete (5-10 min)
- [ ] **T-10**: Stop dashboard service
  ```bash
  pm2 stop ai-dashboard
  ```
- [ ] **T-10**: Stop rAthena servers
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world
  ./athena-start stop
  ```
- [ ] **T-15**: Stop AI service
  ```bash
  sudo systemctl stop ai-service
  ```
- [ ] **T-15**: Verify all services stopped
  ```bash
  ps aux | grep -E "(uvicorn|athena|node)"
  ```

### Database Migration

- [ ] **T-20**: Backup database (pre-migration)
  ```bash
  pg_dump -h 192.168.0.100 -U ai_world_user ai_world_memory > \
    /opt/backups/$(date +%Y%m%d)/pre_migration.sql
  ```
- [ ] **T-25**: Apply migrations 001-016 sequentially
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/migrations
  for i in {001..016}; do
    psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -f ${i}_*.sql
  done
  ```
- [ ] **T-30**: Verify migrations completed
  ```bash
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory \
    -c "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='public';"
  ```
- [ ] **T-30**: Run verification queries
  ```bash
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory \
    -c "SELECT 'world_problems', COUNT(*) FROM world_problems;"
  ```

### Code Deployment

- [ ] **T-35**: Deploy AI service code
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
  git pull origin main
  source venv/bin/activate
  pip install -r requirements.txt
  ```
- [ ] **T-40**: Compile Bridge Layer
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world
  make clean && make server -j8
  ```
- [ ] **T-45**: Deploy dashboard
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard
  git pull origin main
  pnpm install
  pnpm build
  ```
- [ ] **T-45**: Verify all code deployed successfully

### Configuration

- [ ] **T-50**: Update `.env` with production settings
- [ ] **T-50**: Update `ai_bridge.conf` with production settings
- [ ] **T-50**: Update dashboard `.env.local`
- [ ] **T-50**: Verify all API keys configured
- [ ] **T-50**: Verify all database connection strings correct
- [ ] **T-50**: Set appropriate optimization mode (`LLM_OPTIMIZATION_MODE=balanced`)

### Service Startup

- [ ] **T-55**: Start PostgreSQL (if stopped)
  ```bash
  sudo systemctl start postgresql
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"
  ```
- [ ] **T-60**: Start DragonflyDB (if stopped)
  ```bash
  sudo systemctl start dragonfly
  redis-cli -h 192.168.0.100 PING
  ```
- [ ] **T-65**: Start AI service
  ```bash
  sudo systemctl start ai-service
  ```
- [ ] **T-65**: Monitor AI service startup logs
  ```bash
  sudo journalctl -u ai-service -f
  ```
- [ ] **T-70**: Verify AI service healthy
  ```bash
  curl http://192.168.0.100:8000/api/v1/health
  ```
- [ ] **T-75**: Start rAthena servers
  ```bash
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world
  ./athena-start start
  ```
- [ ] **T-80**: Start dashboard
  ```bash
  pm2 start ai-dashboard
  ```
- [ ] **T-80**: Verify dashboard accessible
  ```bash
  curl http://localhost:3000/api/health
  ```

### Smoke Testing

- [ ] **T-85**: Run complete smoke test suite
- [ ] **T-85**: Verify all 21 agents status "active"
  ```bash
  curl -s http://192.168.0.100:8000/api/v1/world/agents/status | \
    jq '.agents[] | select(.status != "active")'
  ```
- [ ] **T-90**: Test Bridge Layer HTTP commands
  ```bash
  curl http://192.168.0.100:8000/api/v1/health
  ```
- [ ] **T-90**: Test WebSocket connections
  ```bash
  wscat -c ws://192.168.0.100:8000/ws
  ```
- [ ] **T-90**: Verify dashboard loads all modules
- [ ] **T-95**: Test story arc generation
  ```bash
  curl -X POST http://192.168.0.100:8000/api/v1/storyline/generate
  ```
- [ ] **T-95**: Test complete player interaction flow
- [ ] **T-100**: Verify no critical errors in logs
  ```bash
  sudo journalctl -u ai-service --since "10 minutes ago" | grep -i "critical\|error"
  ```

### Monitoring Activation

- [ ] **T-105**: Enable Prometheus scraping
- [ ] **T-105**: Verify Grafana dashboards loading
- [ ] **T-105**: Enable alerting rules
- [ ] **T-105**: Test alert delivery
- [ ] **T-105**: Verify metrics being collected

### Go-Live

- [ ] **T-110**: Remove maintenance notice
- [ ] **T-110**: Enable player connections
- [ ] **T-110**: Monitor first player logins
- [ ] **T-115**: Post deployment success announcement
- [ ] **T-115**: Update status page to "operational"
- [ ] **T-120**: Document actual deployment timeline
- [ ] **T-120**: Notify stakeholders deployment complete

---

## Post-Deployment (First 24-48 Hours)

### Immediate Monitoring (First 4 Hours)

- [ ] **Hour 1**: Monitor error logs continuously
  ```bash
  sudo journalctl -u ai-service -f
  ```
- [ ] **Hour 1**: Check all agent execution
- [ ] **Hour 1**: Monitor API response times (target: <250ms p95)
- [ ] **Hour 1**: Watch database connection pool usage
- [ ] **Hour 1**: Verify LLM API calls working
- [ ] **Hour 2**: Review first hour metrics in Grafana
- [ ] **Hour 2**: Check player feedback channels
- [ ] **Hour 2**: Verify content generation (problems, NPCs, events)
- [ ] **Hour 3**: Review LLM cost tracking
- [ ] **Hour 3**: Check for any circuit breaker trips
- [ ] **Hour 4**: Verify backup script ready for tonight
- [ ] **Hour 4**: Document any issues encountered

### Day 1 Tasks

- [ ] **08:00**: Morning health check (see Operations Runbook)
  - [ ] All services running
  - [ ] All agents active
  - [ ] Database healthy
  - [ ] Error log review
  - [ ] LLM budget check
- [ ] **14:00**: Afternoon performance review
  - [ ] API response times
  - [ ] System resources
  - [ ] Content generation
- [ ] **20:00**: Evening system check
  - [ ] Story arc status
  - [ ] Economy health
  - [ ] Backup verification
- [ ] **23:59**: Review full day metrics
- [ ] **23:59**: Check LLM daily costs (target: <$40/day)
- [ ] **23:59**: Verify first backup completed
- [ ] **23:59**: Document Day 1 summary

### Day 2 Tasks

- [ ] **08:00**: Morning health check
- [ ] **08:00**: Review overnight logs for issues
- [ ] **08:00**: Verify daily content regenerated (00:00 tasks)
- [ ] **08:00**: Check player retention metrics
- [ ] **14:00**: Afternoon performance review
- [ ] **14:00**: Compare Day 2 vs Day 1 performance
- [ ] **20:00**: Evening system check
- [ ] **20:00**: Prepare 48-hour status report

### 48-Hour Status Report

- [ ] Total uptime percentage: ____%
- [ ] Number of critical incidents: ____
- [ ] Number of player-reported issues: ____
- [ ] Average API response time: ____ms
- [ ] Total LLM cost (48h): $____
- [ ] Agent execution success rate: ____%
- [ ] Player satisfaction feedback: ____
- [ ] Known issues identified: ____
- [ ] Action items for next week: ____

---

## First Week Monitoring

### Daily Tasks (Days 3-7)

- [ ] **Day 3**: Daily health checks (morning, afternoon, evening)
- [ ] **Day 3**: Performance comparison vs baseline
- [ ] **Day 3**: Review player feedback
- [ ] **Day 4**: Cost analysis (compare vs projections)
- [ ] **Day 4**: Optimization opportunities identified
- [ ] **Day 5**: Agent performance review
- [ ] **Day 5**: Database optimization (vacuum, analyze)
- [ ] **Day 6**: Security audit
- [ ] **Day 6**: Access log review
- [ ] **Day 7**: Weekly summary report
- [ ] **Day 7**: Team retrospective meeting

### Weekly Checkpoints

#### Monday (Day 7)
- [ ] Generate weekly performance report
  - [ ] Uptime: ____%
  - [ ] Avg response time: ____ms
  - [ ] Total LLM cost: $____
  - [ ] Critical incidents: ____
  - [ ] P2/P3 issues: ____
  - [ ] Player satisfaction: ____
- [ ] Review and prioritize issues
- [ ] Plan optimization tasks for Week 2
- [ ] Schedule any required maintenance windows
- [ ] Update documentation with lessons learned

#### Week 1 Success Criteria

System is stable if ALL of these are met:

- [ ] ✅ Uptime >99.5% (max 50 minutes downtime)
- [ ] ✅ All 21 agents executing successfully (>95% success rate)
- [ ] ✅ API p95 response time <250ms
- [ ] ✅ Zero critical (P1) incidents
- [ ] ✅ LLM daily costs <$45 ($40 target + 12.5% margin)
- [ ] ✅ Database connection pool <80% utilization
- [ ] ✅ No data loss or corruption
- [ ] ✅ Player feedback mostly positive (>70%)
- [ ] ✅ All backups completing successfully

---

## Emergency Rollback Checklist

**⚠️ Use only if deployment fails or critical issues arise**

### Decision Criteria

Rollback if ANY of these occur:

- [ ] Complete service outage >30 minutes
- [ ] Data corruption detected
- [ ] Critical security vulnerability
- [ ] LLM costs >$100/day
- [ ] >50% of agents failing
- [ ] Database integrity compromised
- [ ] Unable to restore from current state

### Rollback Procedure

- [ ] **Step 1**: Stop all services
  ```bash
  sudo systemctl stop ai-service
  pm2 stop ai-dashboard
  cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start stop
  ```
- [ ] **Step 2**: Restore database from pre-deployment backup
  ```bash
  psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory < \
    /opt/backups/$(date +%Y%m%d)/pre_migration.sql
  ```
- [ ] **Step 3**: Restore previous code version
  ```bash
  cd /opt/ai-mmorpg
  rm active && ln -s v0.9.0 active
  ```
- [ ] **Step 4**: Start services from previous version
  ```bash
  cd /opt/ai-mmorpg/active
  sudo systemctl start ai-service
  ./athena-start start
  pm2 start ai-dashboard
  ```
- [ ] **Step 5**: Verify rollback successful
  ```bash
  curl http://192.168.0.100:8000/api/v1/health
  ```
- [ ] **Step 6**: Notify stakeholders of rollback
- [ ] **Step 7**: Schedule post-mortem meeting
- [ ] **Step 8**: Document rollback reason and timeline
- [ ] **Step 9**: Plan fix and retry deployment

---

## Post-Deployment Review

### 1-Week Review Meeting

**Attendees**: Deployment team, operations, stakeholders

**Agenda**:
1. Deployment execution review
2. System performance metrics
3. Issues encountered and resolved
4. Player feedback summary
5. Cost analysis
6. Lessons learned
7. Continuous improvement recommendations

**Action Items**:
- [ ] Document lessons learned
- [ ] Update deployment procedures
- [ ] Create tickets for identified issues
- [ ] Schedule follow-up tasks
- [ ] Update monitoring/alerting if needed

### Documentation Updates

- [ ] Update deployment guide with lessons learned
- [ ] Add new troubleshooting entries
- [ ] Document configuration changes
- [ ] Update known issues list
- [ ] Revise timeline estimates
- [ ] Add new best practices

### Optimization Planning

- [ ] Review LLM cost optimization opportunities
- [ ] Identify performance bottlenecks
- [ ] Plan database optimizations
- [ ] Schedule any required scaling
- [ ] Prioritize technical debt items

---

## Sign-Off

### Pre-Deployment Approval

**Prepared By**: _______________________  Date: _______

**Reviewed By**: _______________________  Date: _______

**Approved By**: _______________________  Date: _______

### Deployment Execution

**Deployment Started**: _______ Date/Time: _______

**Deployment Completed**: _______ Date/Time: _______

**Total Duration**: _______ minutes

**Issues Encountered**: 

_______________________________________________

_______________________________________________

### Post-Deployment Approval

**System Stable** (48 hours): ☐ Yes ☐ No

**Verified By**: _______________________  Date: _______

**Deployment Status**: ☐ Successful ☐ Partial ☐ Rolled Back

---

## Appendix: Quick Command Reference

### Health Checks
```bash
# All services
sudo systemctl status postgresql dragonfly ai-service && pm2 status

# API health
curl http://192.168.0.100:8000/api/v1/health

# Agent status
curl http://192.168.0.100:8000/api/v1/world/agents/status | jq

# Database
psql -h 192.168.0.100 -U ai_world_user -d ai_world_memory -c "SELECT 1;"
```

### Emergency Commands
```bash
# Restart all services
sudo systemctl restart ai-service
pm2 restart ai-dashboard
cd /opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world && ./athena-start restart

# View live logs
sudo journalctl -u ai-service -f

# Check errors
sudo journalctl -u ai-service --since "1 hour ago" | grep -i "error"
```

### Monitoring
```bash
# System resources
htop
free -h
df -h

# API metrics
curl http://192.168.0.100:8000/metrics | grep "http_request_duration"

# LLM costs
curl http://192.168.0.100:8000/api/v1/admin/llm-costs | jq
```

---

**Document Version**: 1.0.0  
**Last Updated**: 2025-11-26  
**Next Review**: After first deployment  
**Maintained By**: Deployment Team