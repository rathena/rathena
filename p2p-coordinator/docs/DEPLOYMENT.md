# P2P Coordinator Deployment Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-07

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
- **Python**: 3.11 or higher
- **PostgreSQL**: 17 or higher
- **Redis**: DragonflyDB (Redis 8.2.3 compatible) or Redis 7+
- **Memory**: Minimum 2GB RAM
- **CPU**: Minimum 2 cores
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
SERVICE_VERSION=1.0.0
ENVIRONMENT=development
DEBUG=true
LOG_LEVEL=DEBUG

# Server Configuration
HOST=0.0.0.0
PORT=8001

# Database Configuration
DATABASE_URL=postgresql+asyncpg://ai_world_user:ai_world_pass_2025@localhost:5432/ai_world_memory
DATABASE_POOL_SIZE=20
DATABASE_MAX_OVERFLOW=10

# Redis Configuration
REDIS_URL=redis://localhost:6379/0
REDIS_MAX_CONNECTIONS=50

# Security Configuration
SECRET_KEY=your-secret-key-here-change-in-production
CORS_ORIGINS=*

# AI Service Integration
AI_SERVICE_ENABLED=true
AI_SERVICE_URL=http://localhost:8000

# P2P Configuration
P2P_ENABLED=true
SESSION_TIMEOUT_MINUTES=30
HOST_HEARTBEAT_TIMEOUT_MINUTES=5
```

**Important**: Change `SECRET_KEY` in production!

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
```

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

