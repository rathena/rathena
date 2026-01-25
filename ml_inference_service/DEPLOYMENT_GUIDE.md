# ML Inference Service - Deployment Guide

## Phase 12: ML Inference Service - Complete Setup Instructions

### Overview

This guide covers the complete deployment of the ML Inference Service for the Enhanced ML Monster AI System v2.0.

## Prerequisites

### System Requirements

- **OS:** Ubuntu 20.04+ or RHEL 8+
- **CPU:** 8+ cores recommended
- **RAM:** 16GB minimum, 32GB recommended
- **GPU:** NVIDIA RTX 3060 (12GB) or better (optional, can run CPU-only)
- **Storage:** 100GB+ SSD

### Software Requirements

- PostgreSQL 17.7 (with extensions: TimescaleDB, pgvector, Apache AGE)
- Redis 6.0+ or DragonFlyDB
- Python 3.11+
- CUDA 12.1+ (if using GPU)
- NVIDIA drivers 525+ (if using GPU)

## Step-by-Step Deployment

### 1. Database Setup

#### PostgreSQL Configuration

```bash
# Check if PostgreSQL is running
sudo systemctl status postgresql

# Create ML database and user
sudo -u postgres psql << EOF
CREATE DATABASE ragnarok_ml;
CREATE USER ml_user WITH ENCRYPTED PASSWORD 'your_secure_password_here';
GRANT ALL PRIVILEGES ON DATABASE ragnarok_ml TO ml_user;
\c ragnarok_ml
GRANT ALL ON SCHEMA public TO ml_user;
ALTER DATABASE ragnarok_ml OWNER TO ml_user;
EOF
```

#### Load Extensions and Schema

```bash
# Load PostgreSQL extensions
sudo -u postgres psql -d ragnarok_ml << EOF
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS vector;
-- CREATE EXTENSION IF NOT EXISTS age;  # Optional, if Apache AGE installed
EOF

# Load ML schema
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world
sudo -u postgres psql -d ragnarok_ml -f sql-files/ml_monster_ai_schema.sql
```

#### Verify Schema

```bash
sudo -u postgres psql -d ragnarok_ml -c "\dt ai_*"
# Should show: ai_requests, ai_responses, ai_request_log

sudo -u postgres psql -d ragnarok_ml -c "\dt ml_*"
# Should show: ml_experience_replay, ml_models, ml_state_cache, etc.
```

### 2. Redis Setup

```bash
# Check if Redis is running
sudo systemctl status redis

# If not installed, install Redis
sudo apt-get update
sudo apt-get install redis-server

# Configure Redis memory limit (4GB for caching)
sudo sed -i 's/# maxmemory <bytes>/maxmemory 4gb/' /etc/redis/redis.conf
sudo sed -i 's/# maxmemory-policy noeviction/maxmemory-policy allkeys-lru/' /etc/redis/redis.conf

# Restart Redis
sudo systemctl restart redis
sudo systemctl enable redis

# Test connection
redis-cli ping
# Should return: PONG
```

### 3. Environment Configuration

#### Set Environment Variables

```bash
# Create .env file
sudo tee /opt/ml_monster_ai/.env << EOF
ML_DB_PASSWORD=your_secure_password_here
CUDA_VISIBLE_DEVICES=0
EOF

# Set permissions
sudo chmod 600 /opt/ml_monster_ai/.env
sudo chown lot399:lot399 /opt/ml_monster_ai/.env
```

#### Update Configuration File

Edit `/opt/ml_monster_ai/configs/inference_config.yaml`:

```yaml
database:
  postgresql:
    password: "\${ML_DB_PASSWORD}"  # Will be substituted from .env
```

### 4. Python Dependencies

```bash
# Activate virtual environment
source /opt/ml_monster_ai/venv/bin/activate

# Install inference service dependencies
pip install -r /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_inference_service/requirements.txt

# Verify installation
pip list | grep -E "(asyncpg|redis|onnxruntime|prometheus|psutil|PyYAML)"
```

### 5. Deploy Service

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_inference_service

# Run deployment script
./deploy.sh

# Verify deployment
ls -la /opt/ml_monster_ai/inference_service/
ls -la /opt/ml_monster_ai/configs/inference_config.yaml
```

### 6. Install Systemd Service

```bash
# Copy service unit
sudo cp ml-inference.service /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload

# Enable service (start on boot)
sudo systemctl enable ml-inference

# Check service status (should be inactive/dead, not started yet)
sudo systemctl status ml-inference
```

### 7. Initial Testing

#### Test Database Connection

```bash
# Test PostgreSQL connection
/bin/bash -c "source /opt/ml_monster_ai/venv/bin/activate && cd /opt/ml_monster_ai/inference_service && python -c \"
import asyncio
import asyncpg

async def test():
    pool = await asyncpg.create_pool(
        host='localhost',
        port=5432,
        database='ragnarok_ml',
        user='ml_user',
        password='your_password'  # Replace with actual password
    )
    async with pool.acquire() as conn:
        result = await conn.fetchval('SELECT COUNT(*) FROM ai_requests')
        print(f'✓ Database connected, ai_requests count: {result}')
    await pool.close()

asyncio.run(test())
\""
```

#### Test Redis Connection

```bash
redis-cli ping
redis-cli info memory | grep used_memory_human
```

### 8. Start Service

#### Option A: Start via Systemd (Production)

```bash
# Start service
sudo systemctl start ml-inference

# Check status
sudo systemctl status ml-inference

# View logs
sudo journalctl -u ml-inference -f
```

#### Option B: Start Manually (Development/Testing)

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_inference_service
./start_inference.sh
```

### 9. Verify Operation

#### Check Service Health

```bash
# Check systemd status
sudo systemctl status ml-inference

# Check Prometheus metrics
curl http://localhost:9090/metrics | grep ml_service_up
# Should show: ml_service_up 1.0

curl http://localhost:9090/metrics | grep ml_fallback_level
# Should show current fallback level
```

#### Check Logs

```bash
# Systemd logs
sudo journalctl -u ml-inference --since "5 minutes ago"

# Service log file
tail -f /opt/ml_monster_ai/logs/inference_service.log
```

#### Run Integration Tests

```bash
cd /opt/ml_monster_ai/inference_service
/bin/bash -c "source /opt/ml_monster_ai/venv/bin/activate && python test_inference.py"
```

## Expected Behavior

### With Models (After Phase 11 Complete)

```
✓ Service starts successfully
✓ Loads 54 ONNX models (9 types × 6 archetypes)
✓ Fallback level: 1 (GPU FP16)
✓ Processes requests at <15ms latency
✓ Cache hit rate: 70-90%
✓ Prometheus metrics exposed on :9090
```

### Without Models (Current State - Phase 11 Incomplete)

```
⚠ Service starts successfully
⚠ No models loaded (0/54)
⚠ Fallback level: 5 (Traditional AI)
⚠ All requests return action_id=255 (traditional AI fallback)
⚠ Latency: ~1ms (rule-based only)
✓ Service operational, game functions normally
```

**This is expected and by design.** The service will automatically use better models once they become available (after Phase 11 training).

## Monitoring

### Prometheus Metrics

Access metrics at: `http://localhost:9090/metrics`

**Key metrics to monitor:**

```bash
# Service health
curl -s http://localhost:9090/metrics | grep ml_service_up

# Fallback level (1=best, 5=worst)
curl -s http://localhost:9090/metrics | grep ml_fallback_level

# Inference latency
curl -s http://localhost:9090/metrics | grep ml_inference_latency

# Cache hit rate
curl -s http://localhost:9090/metrics | grep ml_cache_hit_rate

# VRAM usage (if GPU)
curl -s http://localhost:9090/metrics | grep ml_vram_usage_bytes
```

### Grafana Dashboard (Optional)

Create dashboard with panels for:
- Inference latency (p50, p95, p99)
- Cache hit rate
- Fallback level over time
- VRAM usage
- Request throughput

## Troubleshooting

### Service Won't Start

**Check dependencies:**

```bash
# PostgreSQL
sudo systemctl status postgresql
psql -U ml_user -d ragnarok_ml -c "SELECT 1"  # Should prompt for password

# Redis
sudo systemctl status redis
redis-cli ping  # Should return PONG

# Python packages
source /opt/ml_monster_ai/venv/bin/activate
python -c "import asyncpg, redis, onnxruntime; print('✓ All packages available')"
```

**Check permissions:**

```bash
ls -la /opt/ml_monster_ai/inference_service/
# Should be owned by lot399:lot399

sudo chown -R lot399:lot399 /opt/ml_monster_ai/
```

**Check logs for errors:**

```bash
sudo journalctl -u ml-inference -n 50 --no-pager
```

### Authentication Errors

If you see "password authentication failed":

```bash
# Set ML_DB_PASSWORD environment variable
echo "ML_DB_PASSWORD=your_password" | sudo tee -a /opt/ml_monster_ai/.env

# Or update systemd service to use EnvironmentFile
sudo systemctl edit ml-inference
# Add: EnvironmentFile=/opt/ml_monster_ai/.env
```

### No Models Available

This is **expected** until Phase 11 (ML Training) is complete.

**What happens:**
- Service starts successfully
- Automatically falls back to Level 5 (Traditional AI)
- All responses have `action_id=255`
- C++ uses traditional rAthena AI
- Game functions normally (zero impact)

**When models become available:**
- Copy `.onnx` files to `/opt/ml_monster_ai/models/{archetype}/`
- Restart service: `sudo systemctl restart ml-inference`
- Service automatically loads models
- Fallback level upgrades to Level 1 (GPU FP16)

### High Memory Usage

**VRAM:**

```bash
nvidia-smi
# Check VRAM usage

# If > 95%, service will auto-downgrade to INT8 or CPU
```

**RAM:**

```bash
free -h
# Check available RAM

# If low, reduce batch_size in config
```

## Integration with rAthena

### C++ Side Configuration

The C++ ML gateway ([`src/map/mob_ml_gateway.cpp`](../src/map/mob_ml_gateway.cpp:1)) is already configured to:

1. Write requests to `ai_requests` table
2. Poll `ai_responses` table  
3. Handle `action_id=255` as fallback to traditional AI

**No C++ changes needed** for Phase 12.

### Database Flow

```
C++ writes:
INSERT INTO ai_requests (monster_id, archetype, state_vector, ...)
↓
Python polls:
SELECT ... FROM ai_requests WHERE status='pending' FOR UPDATE SKIP LOCKED
↓
Python processes:
- Check cache
- Run inference or use fallback
↓
Python writes:
INSERT INTO ai_responses (action_id, confidence, ...)
↓
C++ reads:
SELECT action_id FROM ai_responses WHERE request_id=...
```

## Performance Tuning

### Optimal Configuration

**For 12GB GPU:**

```yaml
inference:
  batch_size: 128
  max_latency_ms: 15

hardware:
  device: "cuda:0"
  max_vram_gb: 6.0
```

**For 8GB GPU:**

```yaml
inference:
  batch_size: 64
  max_latency_ms: 20

hardware:
  device: "cuda:0"
  max_vram_gb: 4.0
```

**For CPU-only:**

```yaml
inference:
  batch_size: 32
  max_latency_ms: 60

hardware:
  device: "cpu"
  cpu_threads: 16
```

## Maintenance

### Regular Tasks

```bash
# Weekly: Check service health
sudo systemctl status ml-inference
curl http://localhost:9090/metrics | grep ml_service

# Monthly: Cleanup old logs
sudo journalctl --vacuum-time=30d

# As needed: Restart service
sudo systemctl restart ml-inference
```

### Updating Models

```bash
# After training new models (Phase 11):
cp new_model.onnx /opt/ml_monster_ai/models/aggressive/combat_dqn.onnx

# Restart service to load new models
sudo systemctl restart ml-inference

# Verify models loaded
curl http://localhost:9090/metrics | grep ml_models_loaded_total
```

## Security Considerations

### Database Password

**Never commit passwords to git.** Use environment variables:

```bash
# Set in systemd service (preferred)
sudo systemctl edit ml-inference
# Add:
[Service]
Environment="ML_DB_PASSWORD=your_password"

# Or use EnvironmentFile
EnvironmentFile=/opt/ml_monster_ai/.env
```

### Network Security

Service binds to localhost only (127.0.0.1):
- PostgreSQL: localhost:5432
- Redis: localhost:6379
- Prometheus: localhost:9090

**No external exposure** by default.

## Next Steps

After deploying ML Inference Service:

1. **Complete Phase 11:** Train and export ONNX models
2. **Deploy Models:** Copy `.onnx` files to `/opt/ml_monster_ai/models/`
3. **Restart Service:** `sudo systemctl restart ml-inference`
4. **Monitor Performance:** Check Prometheus metrics
5. **Phase 13:** Set up Grafana dashboards for monitoring

## Support

### Log Locations

- **Service logs:** `/opt/ml_monster_ai/logs/inference_service.log`
- **Systemd logs:** `sudo journalctl -u ml-inference`
- **PostgreSQL logs:** `/var/log/postgresql/postgresql-17-main.log`
- **Redis logs:** `/var/log/redis/redis-server.log`

### Common Issues

| Issue | Solution |
|-------|----------|
| "No models loaded" | Expected until Phase 11 complete, service uses fallback |
| "Password authentication failed" | Set ML_DB_PASSWORD in .env file |
| "Redis connection failed" | Check `sudo systemctl status redis` |
| "CUDA not available" | Install NVIDIA drivers, or use device: "cpu" |
| "High latency" | Check fallback level, may need to reduce batch_size |

### Health Check

```bash
# Quick health check
sudo systemctl is-active ml-inference && \
curl -s http://localhost:9090/metrics | grep ml_service_up | \
grep -q "1.0" && echo "✓ Service healthy" || echo "✗ Service unhealthy"
```

---

**Last Updated:** 2026-01-21  
**Version:** 2.0.0  
**Status:** ✅ Ready for Deployment
