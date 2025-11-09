# P2P Coordinator Deployment Guide

**Version**: 2.0.0
**Last Updated**: 2025-11-09
**Status**: ✅ Production-Ready - All 26 Security & Functionality Fixes Complete

---

## 🎉 What's New in Version 2.0.0

**All 26 critical fixes have been completed and are production-ready:**

✅ **New Features**:
- **Redis State Management**: Signaling state moved from in-memory to Redis for persistence and horizontal scaling
- **Rate Limiting**: Token bucket algorithm with Redis backend (API: 100/60s, WebSocket: 1000/60s, Auth: 10/60s)
- **Session Health Monitoring**: Automatic monitoring and cleanup of inactive sessions every 30 seconds
- **NPC State Broadcasting**: Fetches NPC state from AI service every 5 seconds and broadcasts to active sessions
- **Prometheus Metrics**: Full metrics endpoint with text exposition format (hosts, zones, sessions, players, latency, uptime)
- **Refresh Token Endpoint**: JWT refresh token flow with 7-day expiration
- **Custom Exception Handling**: Proper error handling with specific exception classes
- **Database Indexes**: Composite indexes for common query patterns (zone_status, host_status, etc.)
- **Security Enforcement**: Production startup validation - refuses to start with weak secrets (<32 characters)

✅ **Security Improvements**:
- Minimum 32-character requirement for `JWT_SECRET_KEY` and `COORDINATOR_API_KEY`
- Fixed deprecated `datetime.utcnow()` → `datetime.now(timezone.utc)`
- Proper SSL certificate verification enforcement
- Environment-based secret management (no hardcoded secrets)

---

## ⚠️ Important: P2P is Completely Optional

**The P2P system is entirely optional and can be disabled at any time:**
- Set `P2P_ENABLED=false` in `.env` to disable P2P globally
- The system automatically falls back to traditional server routing when P2P is unavailable
- Players experience no difference in gameplay when P2P is disabled
- All AI NPCs, authentication, and game logic remain centralized regardless of P2P status

**P2P provides benefits when enabled, but the game works perfectly without it.**

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
3. [Configuration](#configuration)
4. [Database Setup](#database-setup)
5. [Running the Service](#running-the-service)
6. [Production Deployment](#production-deployment)
7. [Monitoring](#monitoring)
8. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### System Requirements

- **OS**: Linux (Ubuntu 20.04+ recommended)
- **Python**: 3.12 or higher (3.11+ supported)
- **PostgreSQL**: 17 or higher
- **Redis**: DragonflyDB (Redis 8.2.3 compatible) or Redis 7+
- **Memory**: Minimum 2GB RAM (4GB recommended for production)
- **CPU**: Minimum 2 cores (4+ recommended for production)
- **Disk**: Minimum 10GB free space

### Software Dependencies

```bash
# Update system packages
sudo apt update && sudo apt upgrade -y

# Install Python 3.11+
sudo apt install python3.11 python3.11-venv python3-pip -y

# Install PostgreSQL 17
sudo apt install postgresql-17 postgresql-contrib-17 -y

# Install DragonflyDB (or Redis)
# See: https://www.dragonflydb.io/docs/getting-started
```

---

## Installation

### 1. Clone Repository

```bash
cd <workspace>/rathena-AI-world
```

### 2. Create Virtual Environment

```bash
cd p2p-coordinator
python3.11 -m venv venv
source venv/bin/activate
```

### 3. Install Dependencies

```bash
pip install --upgrade pip setuptools wheel
pip install -r requirements.txt
```

---

## Configuration

### 1. Copy Environment Template

```bash
cp .env.example .env
```

### 2. Edit Configuration

Edit `.env` file with your settings:

```bash
# Service Configuration
SERVICE_NAME=p2p-coordinator
SERVICE_VERSION=2.0.0
ENVIRONMENT=production
DEBUG=false
LOG_LEVEL=INFO

# Server Configuration
HOST=0.0.0.0
PORT=8001

# Database Configuration
DATABASE_URL=postgresql+asyncpg://ai_world_user:ai_world_pass_2025@localhost:5432/ai_world_memory
DATABASE_POOL_SIZE=20
DATABASE_MAX_OVERFLOW=10

# Redis Configuration (NEW: Required for state management)
REDIS_URL=redis://localhost:6379/0
REDIS_MAX_CONNECTIONS=50

# Security Configuration (NEW: Minimum 32 characters required)
JWT_SECRET_KEY=your-jwt-secret-key-minimum-32-characters-required-here
COORDINATOR_API_KEY=your-coordinator-api-key-minimum-32-characters-required
CORS_ORIGINS=https://your-domain.com

# AI Service Integration
AI_SERVICE_ENABLED=true
AI_SERVICE_URL=http://localhost:8000

# P2P Configuration
P2P_ENABLED=true
SESSION_TIMEOUT_MINUTES=30
HOST_HEARTBEAT_TIMEOUT_MINUTES=5

# Rate Limiting (NEW in v2.0.0)
RATE_LIMIT_ENABLED=true
RATE_LIMIT_API_REQUESTS=100
RATE_LIMIT_API_WINDOW=60
RATE_LIMIT_WS_REQUESTS=1000
RATE_LIMIT_WS_WINDOW=60
RATE_LIMIT_AUTH_REQUESTS=10
RATE_LIMIT_AUTH_WINDOW=60

# Background Tasks (NEW in v2.0.0)
NPC_BROADCAST_INTERVAL=5
SESSION_HEALTH_CHECK_INTERVAL=30
```

### 3. Security Requirements (NEW in v2.0.0)

**⚠️ CRITICAL: The service will refuse to start in production if:**

1. `JWT_SECRET_KEY` is less than 32 characters
2. `COORDINATOR_API_KEY` is less than 32 characters
3. `ENVIRONMENT=production` without proper secrets

**Generate Strong Secrets:**

```bash
# Generate 64-character JWT secret
openssl rand -hex 32

# Generate 64-character API key
openssl rand -hex 32
```

**Example Production Configuration:**

```bash
JWT_SECRET_KEY=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7r8s9t0u1v2w3x4y5z6a7b8c9d0e1f2
COORDINATOR_API_KEY=f2e1d0c9b8a7z6y5x4w3v2u1t0s9r8q7p6o5n4m3l2k1j0i9h8g7f6e5d4c3b2a1
ENVIRONMENT=production
```

**Important Notes:**
- Never commit `.env` to version control
- Rotate secrets every 90 days
- Use different secrets for development and production
- Store production secrets in a secure vault (e.g., AWS Secrets Manager, HashiCorp Vault)

---

## Database Setup

### 1. Create Database User

```bash
sudo -u postgres psql
```

```sql
-- Create user (if not exists)
CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;

-- Exit
\q
```

### 2. Initialize Database

The database tables will be created automatically on first run using SQLAlchemy's `create_all()`.

Alternatively, you can manually create tables:

```bash
cd coordinator-service
source ../venv/bin/activate
python -c "
from database import db_manager
import asyncio

async def init_db():
    await db_manager.init_db()
    print('Database initialized successfully')

asyncio.run(init_db())
"
```

### 3. Verify Database Connection

```bash
psql -U ai_world_user -d ai_world_memory -h localhost
```

```sql
-- List tables
\dt

-- Expected tables:
-- hosts
-- zones
-- p2p_sessions

-- Verify indexes (NEW in v2.0.0)
\di

-- Expected indexes:
-- idx_zone_status
-- idx_host_status
-- idx_zone_status_capacity
-- idx_status_ended_at
```

### 4. Database Indexes (NEW in v2.0.0)

The following composite indexes are automatically created for query optimization:

```sql
-- Session queries by zone and status
CREATE INDEX idx_zone_status ON p2p_sessions(zone_id, status);

-- Host queries by status
CREATE INDEX idx_host_status ON hosts(status);

-- Session queries by zone, status, and capacity
CREATE INDEX idx_zone_status_capacity ON p2p_sessions(zone_id, status, current_players);

-- Cleanup queries for ended sessions
CREATE INDEX idx_status_ended_at ON p2p_sessions(status, ended_at);
```

These indexes significantly improve performance for:
- Session discovery queries
- Host selection algorithms
- Session health monitoring
- Cleanup operations

---

## New Features in v2.0.0

### 1. Redis State Management

**Purpose**: Persistent signaling state for horizontal scaling

**Configuration**:
```bash
REDIS_URL=redis://localhost:6379/0
REDIS_MAX_CONNECTIONS=50
```

**Key Prefixes**:
- `signaling:connections:{connection_id}` - WebSocket connection metadata
- `signaling:sessions:{session_id}` - Session participant lists
- `signaling:peer:{peer_id}` - Peer connection information

**Benefits**:
- Horizontal scaling support (multiple coordinator instances)
- State persistence across service restarts
- Automatic cleanup of stale connections

### 2. Rate Limiting

**Purpose**: Protect against abuse and DDoS attacks

**Configuration**:
```bash
RATE_LIMIT_ENABLED=true
RATE_LIMIT_API_REQUESTS=100    # Requests per window
RATE_LIMIT_API_WINDOW=60       # Window in seconds
RATE_LIMIT_WS_REQUESTS=1000
RATE_LIMIT_WS_WINDOW=60
RATE_LIMIT_AUTH_REQUESTS=10
RATE_LIMIT_AUTH_WINDOW=60
```

**Implementation**: Token bucket algorithm with Redis backend

**Endpoints Protected**:
- REST API: 100 requests per 60 seconds per IP
- WebSocket: 1000 messages per 60 seconds per connection
- Authentication: 10 attempts per 60 seconds per IP

### 3. Session Health Monitoring

**Purpose**: Automatic cleanup of inactive sessions

**Configuration**:
```bash
SESSION_HEALTH_CHECK_INTERVAL=30  # Check every 30 seconds
SESSION_TIMEOUT_MINUTES=30        # Timeout after 30 minutes
```

**Behavior**:
- Monitors all active sessions every 30 seconds
- Automatically ends sessions with no activity for 30+ minutes
- Cleans up orphaned peer connections
- Logs all cleanup operations

### 4. NPC State Broadcasting

**Purpose**: Real-time NPC state updates to all sessions

**Configuration**:
```bash
NPC_BROADCAST_INTERVAL=5  # Broadcast every 5 seconds
AI_SERVICE_URL=http://localhost:8000
```

**Behavior**:
- Fetches NPC state from AI service every 5 seconds
- Broadcasts state to all active P2P sessions
- Includes NPC positions, dialogue state, and quest status
- Automatic retry on AI service failures

### 5. Prometheus Metrics

**Purpose**: Production monitoring and observability

**Endpoint**: `GET /api/v1/monitoring/metrics`

**Metrics Exposed**:
```
# HELP p2p_hosts_total Total number of registered hosts
# TYPE p2p_hosts_total gauge
p2p_hosts_total{status="active"} 5

# HELP p2p_zones_total Total number of zones
# TYPE p2p_zones_total gauge
p2p_zones_total{p2p_enabled="true"} 7

# HELP p2p_sessions_total Total number of sessions
# TYPE p2p_sessions_total gauge
p2p_sessions_total{status="active"} 12

# HELP p2p_players_total Total number of players in P2P sessions
# TYPE p2p_players_total gauge
p2p_players_total 48

# HELP p2p_average_latency_ms Average latency in milliseconds
# TYPE p2p_average_latency_ms gauge
p2p_average_latency_ms 45.2

# HELP p2p_uptime_seconds Service uptime in seconds
# TYPE p2p_uptime_seconds counter
p2p_uptime_seconds 3600
```

**Integration**: Compatible with Prometheus, Grafana, and other monitoring tools

### 6. Refresh Token Endpoint

**Purpose**: JWT token refresh without re-authentication

**Endpoint**: `POST /api/v1/auth/refresh`

**Request**:
```json
{
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

**Response**:
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "refresh_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "bearer",
  "expires_in": 3600
}
```

**Token Lifetimes**:
- Access Token: 1 hour
- Refresh Token: 7 days

---

## Running the Service

### Development Mode

```bash
cd coordinator-service
source ../venv/bin/activate
python main.py
```

The service will start on `http://localhost:8001` with auto-reload enabled.

### Production Mode

For production, use a production ASGI server like Gunicorn with Uvicorn workers:

```bash
# Install Gunicorn
pip install gunicorn

# Run with Gunicorn
gunicorn main:app \
  --workers 4 \
  --worker-class uvicorn.workers.UvicornWorker \
  --bind 0.0.0.0:8001 \
  --access-logfile logs/access.log \
  --error-logfile logs/error.log \
  --log-level info
```

---

## Production Deployment

### 1. Systemd Service

Create `/etc/systemd/system/p2p-coordinator.service`:

```ini
[Unit]
Description=P2P Coordinator Service
After=network.target postgresql.service redis.service

[Service]
Type=notify
User=www-data
Group=www-data
WorkingDirectory=<workspace>/rathena-AI-world/p2p-coordinator/coordinator-service
Environment="PATH=<workspace>/rathena-AI-world/p2p-coordinator/venv/bin"
ExecStart=<workspace>/rathena-AI-world/p2p-coordinator/venv/bin/gunicorn main:app \
  --workers 4 \
  --worker-class uvicorn.workers.UvicornWorker \
  --bind 0.0.0.0:8001 \
  --access-logfile logs/access.log \
  --error-logfile logs/error.log \
  --log-level info
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### 2. Enable and Start Service

```bash
sudo systemctl daemon-reload
sudo systemctl enable p2p-coordinator
sudo systemctl start p2p-coordinator
sudo systemctl status p2p-coordinator
```

### 3. Nginx Reverse Proxy (Optional)

Create `/etc/nginx/sites-available/p2p-coordinator`:

```nginx
server {
    listen 80;
    server_name your-domain.com;

    location / {
        proxy_pass http://localhost:8001;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # WebSocket support
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

Enable the site:

```bash
sudo ln -s /etc/nginx/sites-available/p2p-coordinator /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl reload nginx
```

---

## Monitoring

### 1. Health Check

```bash
curl http://localhost:8001/health
```

Expected response:
```json
{
  "status": "healthy",
  "service": "p2p-coordinator",
  "version": "1.0.0",
  "database": {
    "postgres": "connected",
    "redis": "connected"
  }
}
```

### 2. Dashboard Metrics

```bash
curl http://localhost:8001/api/monitoring/dashboard
```

### 3. Logs

```bash
# Application logs
tail -f coordinator-service/logs/p2p-coordinator.log

# Systemd logs
sudo journalctl -u p2p-coordinator -f

# Nginx logs
sudo tail -f /var/log/nginx/access.log
sudo tail -f /var/log/nginx/error.log
```

---

## Troubleshooting

### Service Won't Start

**Check logs**:
```bash
sudo journalctl -u p2p-coordinator -n 50
```

**Common issues**:
- Database connection failed: Check PostgreSQL is running and credentials are correct
- Redis connection failed: Check DragonflyDB/Redis is running
- Port already in use: Check if another service is using port 8001

### Database Connection Issues

**Test connection**:
```bash
psql -U ai_world_user -d ai_world_memory -h localhost
```

**Check PostgreSQL status**:
```bash
sudo systemctl status postgresql
```

### Redis Connection Issues

**Test connection**:
```bash
redis-cli ping
```

**Check Redis status**:
```bash
sudo systemctl status redis
# or for DragonflyDB
sudo systemctl status dragonfly
```

### High Memory Usage

**Check worker count**:
- Reduce Gunicorn workers if memory is limited
- Default: 4 workers (adjust based on available RAM)

**Monitor memory**:
```bash
htop
# or
ps aux | grep gunicorn
```

### Performance Issues

**Check database connections**:
```sql
SELECT count(*) FROM pg_stat_activity WHERE datname = 'ai_world_memory';
```

**Optimize database**:
```sql
VACUUM ANALYZE;
```

**Check Redis memory**:
```bash
redis-cli info memory
```

---

## Backup and Recovery

### Database Backup

```bash
pg_dump -U ai_world_user -d ai_world_memory > backup_$(date +%Y%m%d).sql
```

### Database Restore

```bash
psql -U ai_world_user -d ai_world_memory < backup_20251107.sql
```

### Redis Backup

```bash
redis-cli SAVE
cp /var/lib/redis/dump.rdb backup_redis_$(date +%Y%m%d).rdb
```

---

## Security Checklist

- [ ] Change `SECRET_KEY` in production
- [ ] Use strong database passwords
- [ ] Configure CORS origins (don't use `*` in production)
- [ ] Enable HTTPS with SSL certificates
- [ ] Set up firewall rules
- [ ] Enable rate limiting (when implemented)
- [ ] Regular security updates
- [ ] Monitor logs for suspicious activity

---

## Scaling

### Horizontal Scaling

- Deploy multiple instances behind a load balancer
- Use shared PostgreSQL and Redis instances
- Configure session affinity if needed

### Vertical Scaling

- Increase Gunicorn workers
- Increase database connection pool size
- Increase Redis max connections

---

## Support

For issues and questions:
- Check logs first
- Review configuration
- Consult [ARCHITECTURE.md](ARCHITECTURE.md) and [API.md](API.md)
- Check GitHub issues (if applicable)

