# P2P Coordinator Configuration Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-07

---

## Table of Contents

1. [Overview](#overview)
2. [Environment Variables](#environment-variables)
3. [Configuration File](#configuration-file)
4. [Service Configuration](#service-configuration)
5. [Database Configuration](#database-configuration)
6. [Redis Configuration](#redis-configuration)
7. [Security Configuration](#security-configuration)
8. [AI Integration Configuration](#ai-integration-configuration)
9. [P2P Configuration](#p2p-configuration)
10. [Logging Configuration](#logging-configuration)

---

## Overview

The P2P Coordinator uses environment variables for configuration, loaded from a `.env` file. All configuration is managed through the `config.py` module using Pydantic settings.

---

## Environment Variables

### Quick Start

1. Copy the example environment file:
```bash
cp .env.example .env
```

2. Edit `.env` with your settings:
```bash
nano .env
```

3. Restart the service to apply changes.

---

## Configuration File

### Location

```
rathena-AI-world/p2p-coordinator/.env
```

### Format

```bash
# Comments start with #
VARIABLE_NAME=value
ANOTHER_VARIABLE=another_value
```

---

## Service Configuration

### SERVICE_NAME

**Description**: Name of the service  
**Type**: String  
**Default**: `p2p-coordinator`  
**Example**: `SERVICE_NAME=p2p-coordinator`

### SERVICE_VERSION

**Description**: Version of the service  
**Type**: String  
**Default**: `1.0.0`  
**Example**: `SERVICE_VERSION=1.0.0`

### ENVIRONMENT

**Description**: Deployment environment  
**Type**: String  
**Options**: `development`, `staging`, `production`  
**Default**: `development`  
**Example**: `ENVIRONMENT=production`

### DEBUG

**Description**: Enable debug mode  
**Type**: Boolean  
**Default**: `true` (development), `false` (production)  
**Example**: `DEBUG=false`

### LOG_LEVEL

**Description**: Logging level  
**Type**: String  
**Options**: `DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`  
**Default**: `DEBUG` (development), `INFO` (production)  
**Example**: `LOG_LEVEL=INFO`

---

## Server Configuration

### HOST

**Description**: Server bind address  
**Type**: String  
**Default**: `0.0.0.0`  
**Example**: `HOST=0.0.0.0`

**Notes**:
- `0.0.0.0`: Listen on all interfaces
- `127.0.0.1`: Listen on localhost only

### PORT

**Description**: Server port  
**Type**: Integer  
**Default**: `8001`  
**Example**: `PORT=8001`

**Notes**:
- Must be available (not used by another service)
- Requires root/sudo for ports < 1024

---

## Database Configuration

### DATABASE_URL

**Description**: PostgreSQL connection URL  
**Type**: String  
**Format**: `postgresql+asyncpg://user:password@host:port/database`  
**Default**: `postgresql+asyncpg://ai_world_user:ai_world_pass_2025@localhost:5432/ai_world_memory`  
**Example**: `DATABASE_URL=postgresql+asyncpg://myuser:mypass@db.example.com:5432/mydb`

**Components**:
- `postgresql+asyncpg`: Driver (async PostgreSQL)
- `user`: Database username
- `password`: Database password
- `host`: Database host
- `port`: Database port (default: 5432)
- `database`: Database name

### DATABASE_POOL_SIZE

**Description**: Database connection pool size  
**Type**: Integer  
**Default**: `20`  
**Example**: `DATABASE_POOL_SIZE=50`

**Notes**:
- Higher values = more concurrent connections
- Adjust based on available RAM and expected load

### DATABASE_MAX_OVERFLOW

**Description**: Maximum overflow connections beyond pool size  
**Type**: Integer  
**Default**: `10`  
**Example**: `DATABASE_MAX_OVERFLOW=20`

**Notes**:
- Total max connections = pool_size + max_overflow

---

## Redis Configuration

### REDIS_URL

**Description**: Redis/DragonflyDB connection URL  
**Type**: String  
**Format**: `redis://host:port/database`  
**Default**: `redis://localhost:6379/0`  
**Example**: `REDIS_URL=redis://cache.example.com:6379/1`

**Components**:
- `redis://`: Protocol
- `host`: Redis host
- `port`: Redis port (default: 6379)
- `database`: Redis database number (0-15)

### REDIS_MAX_CONNECTIONS

**Description**: Maximum Redis connections  
**Type**: Integer  
**Default**: `50`  
**Example**: `REDIS_MAX_CONNECTIONS=100`

**Notes**:
- Adjust based on expected concurrent operations

---

## Security Configuration

### SECRET_KEY

**Description**: Secret key for signing tokens  
**Type**: String  
**Default**: `dev-secret-key-change-in-production`  
**Example**: `SECRET_KEY=your-random-secret-key-here`

**⚠️ IMPORTANT**:
- **MUST** be changed in production
- Use a strong, random string (32+ characters)
- Keep secret and secure

**Generate a secure key**:
```bash
python -c "import secrets; print(secrets.token_urlsafe(32))"
```

### CORS_ORIGINS

**Description**: Allowed CORS origins  
**Type**: String (comma-separated)  
**Default**: `*`  
**Example**: `CORS_ORIGINS=https://example.com,https://app.example.com`

**Notes**:
- `*`: Allow all origins (development only!)
- Production: Specify exact origins
- Multiple origins: Comma-separated

---

## AI Integration Configuration

### AI_SERVICE_ENABLED

**Description**: Enable AI service integration  
**Type**: Boolean  
**Default**: `true`  
**Example**: `AI_SERVICE_ENABLED=false`

**Notes**:
- Set to `false` to disable AI integration
- Useful for testing P2P coordinator independently

### AI_SERVICE_URL

**Description**: AI autonomous world service URL  
**Type**: String  
**Default**: `http://localhost:8000`  
**Example**: `AI_SERVICE_URL=http://ai-service.example.com:8000`

**Notes**:
- Must be accessible from P2P coordinator
- Include protocol (http:// or https://)

---

## P2P Configuration

### P2P_ENABLED

**Description**: Enable P2P functionality  
**Type**: Boolean  
**Default**: `true`  
**Example**: `P2P_ENABLED=false`

**Notes**:
- Set to `false` to disable P2P features
- Service will still run but P2P operations will be disabled

### SESSION_TIMEOUT_MINUTES

**Description**: Session timeout in minutes  
**Type**: Integer  
**Default**: `30`  
**Example**: `SESSION_TIMEOUT_MINUTES=60`

**Notes**:
- Sessions inactive for this duration will be cleaned up
- Affects stale session cleanup background task

### HOST_HEARTBEAT_TIMEOUT_MINUTES

**Description**: Host heartbeat timeout in minutes  
**Type**: Integer  
**Default**: `5`  
**Example**: `HOST_HEARTBEAT_TIMEOUT_MINUTES=10`

**Notes**:
- Hosts not sending heartbeat for this duration will be marked offline
- Affects host health monitoring

---

## Logging Configuration

### Log Files

**Location**: `coordinator-service/logs/`

**Files**:
- `p2p-coordinator.log`: Main application log
- `access.log`: HTTP access log (Gunicorn)
- `error.log`: Error log (Gunicorn)

### Log Rotation

**Manual rotation**:
```bash
cd coordinator-service/logs
mv p2p-coordinator.log p2p-coordinator.log.$(date +%Y%m%d)
```

**Automatic rotation** (using logrotate):

Create `/etc/logrotate.d/p2p-coordinator`:
```
<workspace>/rathena-AI-world/p2p-coordinator/coordinator-service/logs/*.log {
    daily
    rotate 7
    compress
    delaycompress
    missingok
    notifempty
    create 0644 www-data www-data
}
```

---

## Configuration Examples

### Development

```bash
SERVICE_NAME=p2p-coordinator
SERVICE_VERSION=1.0.0
ENVIRONMENT=development
DEBUG=true
LOG_LEVEL=DEBUG

HOST=0.0.0.0
PORT=8001

DATABASE_URL=postgresql+asyncpg://ai_world_user:ai_world_pass_2025@localhost:5432/ai_world_memory
DATABASE_POOL_SIZE=10
DATABASE_MAX_OVERFLOW=5

REDIS_URL=redis://localhost:6379/0
REDIS_MAX_CONNECTIONS=20

SECRET_KEY=dev-secret-key-change-in-production
CORS_ORIGINS=*

AI_SERVICE_ENABLED=true
AI_SERVICE_URL=http://localhost:8000

P2P_ENABLED=true
SESSION_TIMEOUT_MINUTES=30
HOST_HEARTBEAT_TIMEOUT_MINUTES=5
```

### Production

```bash
SERVICE_NAME=p2p-coordinator
SERVICE_VERSION=1.0.0
ENVIRONMENT=production
DEBUG=false
LOG_LEVEL=INFO

HOST=0.0.0.0
PORT=8001

DATABASE_URL=postgresql+asyncpg://prod_user:strong_password@db.internal:5432/p2p_prod
DATABASE_POOL_SIZE=50
DATABASE_MAX_OVERFLOW=20

REDIS_URL=redis://cache.internal:6379/0
REDIS_MAX_CONNECTIONS=100

SECRET_KEY=your-super-secret-production-key-here
CORS_ORIGINS=https://game.example.com,https://admin.example.com

AI_SERVICE_ENABLED=true
AI_SERVICE_URL=http://ai-service.internal:8000

P2P_ENABLED=true
SESSION_TIMEOUT_MINUTES=60
HOST_HEARTBEAT_TIMEOUT_MINUTES=10
```

---

## Validation

### Check Configuration

```bash
cd coordinator-service
source ../venv/bin/activate
python -c "from config import settings; print(settings.model_dump_json(indent=2))"
```

### Test Database Connection

```bash
python -c "
from database import db_manager
import asyncio

async def test():
    await db_manager.init_db()
    print('✅ Database connection successful')

asyncio.run(test())
"
```

### Test Redis Connection

```bash
python -c "
from database import db_manager
import asyncio

async def test():
    redis = await db_manager.get_redis()
    result = await redis.ping()
    print(f'✅ Redis connection successful: {result}')

asyncio.run(test())
"
```

---

## Troubleshooting

### Configuration Not Loading

**Check file location**:
```bash
ls -la .env
```

**Check file permissions**:
```bash
chmod 600 .env
```

### Invalid Configuration

**Check for syntax errors**:
- No spaces around `=`
- No quotes needed for values
- Comments start with `#`

**Example of common mistakes**:
```bash
# ❌ Wrong
PORT = 8001
DATABASE_URL = "postgresql://..."

# ✅ Correct
PORT=8001
DATABASE_URL=postgresql://...
```

---

## Security Best Practices

1. **Never commit `.env` to version control**
   - Add `.env` to `.gitignore`
   - Use `.env.example` as template

2. **Use strong passwords**
   - Database passwords: 16+ characters
   - Secret key: 32+ characters

3. **Restrict file permissions**
   ```bash
   chmod 600 .env
   ```

4. **Use environment-specific configurations**
   - Different `.env` for dev/staging/prod
   - Never use production credentials in development

5. **Rotate secrets regularly**
   - Change passwords periodically
   - Update secret keys on security incidents

