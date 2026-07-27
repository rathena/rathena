# rAthena AI World - Setup Guide

**Version**: 2.1.0  
**Last Updated**: 2026-07-27  
**Systems**: AI Autonomous World (Production ✅) + ML Monster AI (Infrastructure Ready ⚠️)

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start (Automated Install)](#quick-start-automated-install)
3. [Manual Installation](#manual-installation)
4. [Configuration Guide](#configuration-guide)
5. [Running the Server](#running-the-server)
6. [Testing AI Features](#testing-ai-features)
7. [Production Deployment](#production-deployment)
8. [Troubleshooting](#troubleshooting)
9. [Reference](#reference)

---

## Prerequisites

### Hardware Requirements

| Component | Minimum | Recommended | Notes |
|-----------|---------|-------------|-------|
| **CPU** | 4 cores | 8+ cores | More cores = faster AI responses |
| **RAM** | 8 GB | 16-32 GB | AI services are memory-intensive |
| **Storage** | 20 GB | 100+ GB SSD | ML training needs significant space |
| **GPU** | Optional | NVIDIA 12GB+ VRAM | Only required for ML training/inference |

### Software Requirements

**Operating System:**
- Ubuntu 24.04 LTS (tested and recommended)
- Ubuntu 22.04+, Debian 11+ (compatible)
- Windows 10/11 with WSL2 (development only)

**Core Dependencies:**
- Python 3.12+ (for AI services)
- PostgreSQL 17+ (for AI/ML data storage)
- MariaDB 10.6+ or MySQL 8.0+ (for game data)
- DragonflyDB 1.12+ or Redis 7+ (for caching)
- Node.js 20+ (for OpenMemory module)
- C++ compiler (gcc-6+ or MS Visual Studio 2017+)
- CMake 3.13+

**LLM Provider API Keys** (at least one required):
- OpenAI (GPT-4/GPT-4o)
- Anthropic (Claude 3/Claude 3.5)
- Azure OpenAI
- DeepSeek
- Ollama (local, no API key needed)

### Network Requirements

- Internet connection for package downloads and LLM API calls
- Ports: 8000 (AI service), 5432 (PostgreSQL), 3306 (MariaDB), 6379 (DragonflyDB/Redis)
- Firewall: Ensure required ports are open for your deployment

---

## Quick Start (Automated Install)

The fastest way to get started is with the automated install wizard:

```bash
# 1. Clone the repository
git clone https://github.com/iskandarsulaili/rathena-AI-world.git
cd rathena-AI-world

# 2. Run the install wizard
./install.sh
```

The wizard will:
1. Check system requirements (OS, RAM, disk, GPU)
2. Install system packages (build-essential, cmake, libraries)
3. Build the C++ server from source
4. Set up PostgreSQL database for AI services
5. Set up MariaDB/MySQL for game data
6. Install DragonflyDB/Redis for caching
7. Create Python virtual environments and install dependencies
8. Create `.env` configuration from template
9. Create systemd service files
10. Run verification tests

### Quick Install (Non-Interactive)

```bash
./install.sh --quick
```

Uses default values for all prompts. Generates random passwords automatically.

### Development Install

```bash
./install.sh --dev
```

Enables debug mode, verbose logging, and development-friendly defaults.

### Dry Run

```bash
./install.sh --dry-run
```

Shows what would be installed without making any changes.

---

## Manual Installation

If you prefer to install components manually, follow these steps.

### 1. Install System Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    git \
    cmake \
    gcc \
    g++ \
    make \
    pkg-config \
    libmysqlclient-dev \
    zlib1g-dev \
    libpcre3-dev \
    libssl-dev \
    libpq-dev \
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    curl \
    wget \
    screen \
    unzip \
    openssl \
    jq
```

### 2. Set Up PostgreSQL (AI Services)

```bash
# Install PostgreSQL
sudo apt-get install -y postgresql postgresql-contrib

# Start PostgreSQL
sudo systemctl enable postgresql
sudo systemctl start postgresql

# Create database and user
sudo -u postgres psql -c "CREATE DATABASE ai_world_memory;"
sudo -u postgres psql -c "CREATE USER ai_world_user WITH PASSWORD 'your_strong_password';"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;"
sudo -u postgres psql -d ai_world_memory -c "GRANT ALL ON SCHEMA public TO ai_world_user;"

# Install extensions (optional but recommended)
sudo apt-get install -y postgresql-17-timescaledb postgresql-17-pgvector

# Enable extensions
sudo -u postgres psql -d ai_world_memory -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;"
sudo -u postgres psql -d ai_world_memory -c "CREATE EXTENSION IF NOT EXISTS vector CASCADE;"

# Initialize schema
sudo -u postgres psql -d ai_world_memory -f ai-autonomous-world/init_postgres.sql
```

### 3. Set Up MariaDB/MySQL (Game Data)

```bash
# Install MariaDB
sudo apt-get install -y mariadb-server mariadb-client

# Start MariaDB
sudo systemctl enable mariadb
sudo systemctl start mariadb

# Create database and user
sudo mysql -u root -e "CREATE DATABASE IF NOT EXISTS ragnarok CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;"
sudo mysql -u root -e "CREATE USER IF NOT EXISTS 'ragnarok'@'localhost' IDENTIFIED BY 'your_strong_password';"
sudo mysql -u root -e "GRANT ALL PRIVILEGES ON ragnarok.* TO 'ragnarok'@'localhost';"
sudo mysql -u root -e "FLUSH PRIVILEGES;"

# Import rAthena SQL files (if they exist)
if [ -f sql-files/main.sql ]; then
    sudo mysql -u root ragnarok < sql-files/main.sql
fi
if [ -f sql-files/logs.sql ]; then
    sudo mysql -u root ragnarok < sql-files/logs.sql
fi
```

### 4. Set Up Cache Layer

**Option A: DragonflyDB (Recommended)**

```bash
curl -fsSL https://www.dragonflydb.io/install.sh | bash
sudo systemctl enable dragonfly
sudo systemctl start dragonfly
```

**Option B: Redis (Fallback)**

```bash
sudo apt-get install -y redis-server
sudo systemctl enable redis-server
sudo systemctl start redis-server
```

### 5. Build C++ Server

```bash
cd rathena-AI-world
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DINSTALL_TO_SOURCE=ON
make -j$(nproc)
cd ..
```

Verify the binaries were created:
```bash
ls -lh map-server login-server char-server
```

### 6. Set Up Python Environments

**AI Autonomous World:**

```bash
cd ai-autonomous-world
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r ai-service/requirements.txt
deactivate
cd ..
```

**ML Inference Service (optional):**

```bash
python3 -m venv venv_ml_inference
source venv_ml_inference/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r ml_inference_service/requirements.txt
deactivate
```

**ML Training (optional):**

```bash
python3 -m venv venv_ml_training
source venv_ml_training/bin/activate
pip install --upgrade pip setuptools wheel
pip install -r ml_training/requirements.txt
deactivate
```

### 7. Create Configuration

```bash
# Root configuration
cp .env.example .env
# Edit .env with your LLM API keys and database passwords
nano .env

# AI service configuration
cp ai-autonomous-world/ai-service/.env.example ai-autonomous-world/ai-service/.env
```

---

## Configuration Guide

### Environment Variables

The `.env` file controls all configuration. Below are the key sections:

#### Database Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `POSTGRES_DB` | `ai_world_memory` | PostgreSQL database for AI services |
| `POSTGRES_USER` | `ai_world_user` | PostgreSQL user |
| `POSTGRES_PASSWORD` | *(required)* | PostgreSQL password |
| `POSTGRES_HOST` | `localhost` | PostgreSQL host |
| `POSTGRES_PORT` | `5432` | PostgreSQL port |
| `DRAGONFLY_HOST` | `localhost` | DragonflyDB/Redis host |
| `DRAGONFLY_PORT` | `6379` | DragonflyDB/Redis port |
| `DRAGONFLY_PASSWORD` | *(optional)* | DragonflyDB password |

#### LLM Provider Configuration

At least one LLM provider must be configured:

**OpenAI:**
```env
OPENAI_API_KEY=sk-your-key-here
OPENAI_MODEL=gpt-4
```

**Anthropic Claude:**
```env
ANTHROPIC_API_KEY=sk-ant-your-key-here
ANTHROPIC_MODEL=claude-3-opus
```

**Azure OpenAI:**
```env
AZURE_OPENAI_API_KEY=your-azure-key
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com/
AZURE_OPENAI_DEPLOYMENT=gpt-4
```

**DeepSeek:**
```env
DEEPSEEK_API_KEY=your-deepseek-key
DEEPSEEK_MODEL=deepseek-chat
```

**Default Provider Selection:**
```env
DEFAULT_LLM_PROVIDER=openai
DEFAULT_MODEL=gpt-4
```

#### AI Service Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `AI_SERVICE_API_KEY` | *(required)* | Internal API key for service bridge |
| `API_KEY_REQUIRED` | `true` | Require API key for all requests |
| `LOG_LEVEL` | `INFO` | Logging level (DEBUG, INFO, WARN, ERROR) |
| `MAX_WORKERS` | `4` | Number of AI worker processes |
| `CACHE_TTL` | `3600` | Cache time-to-live in seconds |

#### Feature Flags

| Variable | Default | Description |
|----------|---------|-------------|
| `ENABLE_MEMORY_SYSTEM` | `true` | Enable NPC long-term memory |
| `ENABLE_PERSONALITY_SYSTEM` | `true` | Enable NPC personality system |
| `ENABLE_FACTION_SYSTEM` | `true` | Enable faction/reputation system |
| `ENABLE_ECONOMY_AGENT` | `true` | Enable economic simulation |
| `ENABLE_QUEST_GENERATION` | `true` | Enable dynamic quest generation |

### Security Configuration

**⚠️ CRITICAL: Default configuration is INSECURE**

Before production deployment:

1. **Generate strong passwords:**
   ```bash
   openssl rand -base64 32  # For database passwords
   openssl rand -base64 43  # For AI_SERVICE_API_KEY
   ```

2. **Enable API key authentication:**
   ```env
   API_KEY_REQUIRED=true
   ```

3. **Configure SSL/TLS:**
   ```env
   SSL_ENABLED=true
   SSL_CERTFILE=/path/to/cert.pem
   SSL_KEYFILE=/path/to/key.pem
   ```

4. **Set DragonflyDB password:**
   ```env
   DRAGONFLY_PASSWORD=your-strong-password
   ```

5. **Never commit `.env` files to git.**

---

## Running the Server

### Starting All Services

**Using systemd (recommended for production):**

```bash
# Enable services to start on boot
sudo systemctl enable rathena-ai-service
sudo systemctl enable rathena-map

# Start services
sudo systemctl start rathena-ai-service
sudo systemctl start rathena-map

# Check status
sudo systemctl status rathena-ai-service
sudo systemctl status rathena-map
```

**Using the start scripts:**

```bash
# Start AI service
cd ai-autonomous-world/ai-service
source ../venv/bin/activate
python main.py &

# Start game servers
cd ../..
./login-server &
./char-server &
./map-server &
```

**Using the all-in-one script:**
```bash
./start-all.sh
```

### Verifying Services Are Running

```bash
# Check AI service health
curl http://localhost:8000/health

# Check AI service API docs
# Open http://localhost:8000/docs in your browser

# Check PostgreSQL
pg_isready

# Check MariaDB
mysqladmin ping -u root

# Check cache
redis-cli ping  # Works for both DragonflyDB and Redis
```

### Stopping Services

```bash
# Using systemd
sudo systemctl stop rathena-ai-service
sudo systemctl stop rathena-map

# Using scripts
./stop-all.sh
```

### Viewing Logs

```bash
# AI service logs
sudo journalctl -u rathena-ai-service -f

# Map server logs
sudo journalctl -u rathena-map -f

# AI service application logs
tail -f ai-autonomous-world/ai-service/logs/ai-service.log

# PostgreSQL logs
sudo journalctl -u postgresql -f
```

---

## Testing AI Features

### Health Check

```bash
curl http://localhost:8000/health
```

Expected response:
```json
{
  "status": "healthy",
  "version": "2.1.0",
  "services": {
    "database": "connected",
    "cache": "connected",
    "llm": "configured"
  }
}
```

### Test NPC Dialogue

```bash
curl -X POST http://localhost:8000/api/dialogue \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_npc_01",
    "player_id": "test_player_01",
    "message": "Hello there!",
    "personality": {
      "openness": 0.7,
      "conscientiousness": 0.6,
      "extraversion": 0.8,
      "agreeableness": 0.7,
      "neuroticism": 0.3
    }
  }'
```

### Test Quest Generation

```bash
curl -X POST http://localhost:8000/api/quests/generate \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "test_player_01",
    "level": 50,
    "preferences": ["combat", "exploration"]
  }'
```

### Test Economy Simulation

```bash
curl -X POST http://localhost:8000/api/economy/simulate \
  -H "Content-Type: application/json" \
  -d '{
    "agent_type": "merchant",
    "action": "trade",
    "items": ["Potion", "Scroll"]
  }'
```

### Run Automated Tests

```bash
# AI service tests
cd ai-autonomous-world/ai-service
source ../venv/bin/activate
python -m pytest tests/ -v

# Integration tests
cd ../..
./test_integration.sh
```

### ML Inference Service (Fallback Mode)

The ML service runs in Level 5 fallback (traditional AI) until models are trained:

```bash
# Start ML inference service
cd ml_inference_service
source ../venv_ml_inference/bin/activate
python main.py &

# Check metrics
curl http://localhost:9090/metrics | grep ml_
```

---

## Production Deployment

### Deployment Checklist

- [ ] Strong passwords generated and configured
- [ ] API key authentication enabled (`API_KEY_REQUIRED=true`)
- [ ] SSL/TLS configured and enabled
- [ ] Database passwords set in `.env`
- [ ] DragonflyDB password set
- [ ] `.env` file permissions restricted (`chmod 600 .env`)
- [ ] Firewall configured (only required ports open)
- [ ] Systemd services enabled for auto-start
- [ ] Monitoring configured (Prometheus + Grafana)
- [ ] Backup strategy implemented
- [ ] Log rotation configured

### Performance Tuning

**AI Service:**
```env
MAX_WORKERS=8           # Adjust based on CPU cores
CACHE_TTL=3600          # Cache responses for 1 hour
DIALOGUE_TEMPERATURE=0.7  # Lower = more consistent responses
```

**PostgreSQL:**
```bash
# Edit /etc/postgresql/17/main/postgresql.conf
shared_buffers = '4GB'        # 25% of RAM
effective_cache_size = '12GB'  # 75% of RAM
work_mem = '256MB'
maintenance_work_mem = '1GB'
```

**DragonflyDB:**
```bash
# Edit /etc/dragonfly/dragonfly.conf
--maxmemory=8gb
--cache_mode=true
```

### Monitoring

```bash
# Prometheus metrics
curl http://localhost:8000/metrics
curl http://localhost:9090/metrics  # ML service

# Health check
curl http://localhost:8000/health

# Service status
sudo systemctl status rathena-ai-service
sudo systemctl status rathena-map
```

### Backup

```bash
# Backup PostgreSQL
pg_dump -U ai_world_user ai_world_memory > backup_ai_$(date +%Y%m%d).sql

# Backup MariaDB
mysqldump -u ragnarok -p ragnarok > backup_game_$(date +%Y%m%d).sql

# Use the backup script
./ai-autonomous-world/scripts/backup-system.sh
```

---

## Troubleshooting

### AI Service Won't Start

**Check logs:**
```bash
sudo journalctl -u rathena-ai-service -f --no-pager
tail -f ai-autonomous-world/ai-service/logs/ai-service.log
```

**Common issues:**

| Symptom | Likely Cause | Solution |
|---------|-------------|----------|
| `Connection refused` to PostgreSQL | PostgreSQL not running | `sudo systemctl start postgresql` |
| `Role does not exist` | Database user not created | Run `setup-database.sh` |
| `ModuleNotFoundError` | Python deps not installed | `pip install -r requirements.txt` |
| `API key not configured` | Missing LLM API key | Edit `.env` and add API key |
| `Address already in use` | Port conflict | Check port 8000 usage: `lsof -i :8000` |

### LLM API Issues

```bash
# Verify API keys are set
grep -E "OPENAI_API_KEY|ANTHROPIC_API_KEY|DEEPSEEK_API_KEY" .env

# Test OpenAI connection
curl -X GET https://api.openai.com/v1/models \
  -H "Authorization: Bearer $OPENAI_API_KEY"

# Test Anthropic connection
curl -X POST https://api.anthropic.com/v1/messages \
  -H "x-api-key: $ANTHROPIC_API_KEY" \
  -H "anthropic-version: 2023-06-01" \
  -H "Content-Type: application/json" \
  -d '{"model": "claude-3-opus", "max_tokens": 10, "messages": [{"role": "user", "content": "Hello"}]}'
```

### Database Connection Issues

```bash
# Test PostgreSQL
pg_isready
psql -U ai_world_user -d ai_world_memory -h localhost

# Test MariaDB
mysql -u ragnarok -p -h localhost ragnarok -e "SELECT 1;"

# Test cache
redis-cli -h localhost -p 6379 ping
```

### C++ Server Build Issues

```bash
# Clean rebuild
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DINSTALL_TO_SOURCE=ON
make -j$(nproc) 2>&1 | tail -50

# Check for missing dependencies
dpkg -l | grep -E "libmysql|zlib|pcre|ssl"
```

### ML Service Issues

```bash
# Check service status
sudo systemctl status ml-inference

# View logs
sudo journalctl -u ml-inference -f

# Verify models directory
ls -la /opt/ml_monster_ai/models/

# Note: Service runs in Level 5 fallback (traditional AI) until models are trained
# See ml_training/docs/TRAINING_GUIDE.md for training instructions
```

### Performance Issues

```bash
# Check system resources
htop
free -h
df -h
nvidia-smi  # If GPU available

# Check AI service response times
curl -w "%{time_total}s\n" -o /dev/null -s http://localhost:8000/health

# Check cache hit rate
redis-cli info stats | grep hit_rate
```

### Getting Help

- **GitHub Issues**: [Report bugs and request features](https://github.com/iskandarsulaili/rathena-AI-world/issues)
- **GitHub Discussions**: [Ask questions and share ideas](https://github.com/iskandarsulaili/rathena-AI-world/discussions)
- **Documentation**: See the [full documentation suite](../docs/README.md)

---

## Reference

### Directory Structure

```
rathena-AI-world/
├── install.sh                          # Unified install wizard
├── .env.example                        # Environment configuration template
├── .env                                # Your configuration (created by install.sh)
├── README.md                           # This file
├── docs/
│   ├── SETUP.md                        # This setup guide
│   ├── PRODUCTION_DEPLOYMENT_GUIDE.md  # Production deployment
│   ├── OPERATIONS_RUNBOOK.md           # Daily operations
│   ├── ADMINISTRATOR_GUIDE.md          # Admin dashboard
│   ├── PLAYER_GUIDE.md                 # Player documentation
│   ├── QUICK_REFERENCE.md              # Command cheat sheet
│   └── ARCHITECTURE_OVERVIEW.md        # System architecture
├── ai-autonomous-world/                # AI World System (21 agents)
│   ├── install.sh                      # AI World install script
│   ├── ai-service/                     # FastAPI service
│   ├── venv/                           # Python virtual environment
│   └── docs/                           # AI World documentation
├── ml_inference_service/               # ML Monster AI inference
├── ml_training/                        # ML model training pipeline
├── ml_scheduler/                       # Training scheduler
├── src/custom/                         # C++ AI integration
├── npc/custom/ai-world/                # AI-integrated NPC scripts
├── conf/                               # Server configuration
└── db/                                 # Game database files
```

### Quick Command Reference

| Action | Command |
|--------|---------|
| Install everything | `./install.sh` |
| Quick install | `./install.sh --quick` |
| Start AI service | `sudo systemctl start rathena-ai-service` |
| Start game server | `sudo systemctl start rathena-map` |
| Check AI health | `curl http://localhost:8000/health` |
| View AI logs | `sudo journalctl -u rathena-ai-service -f` |
| View map logs | `sudo journalctl -u rathena-map -f` |
| Test PostgreSQL | `pg_isready` |
| Test cache | `redis-cli ping` |
| Stop all | `./stop-all.sh` |
| Check status | `./status-all.sh` |

### Port Reference

| Port | Service | Purpose |
|------|---------|---------|
| 8000 | AI Service | REST API for AI features |
| 5432 | PostgreSQL | AI/ML data storage |
| 3306 | MariaDB/MySQL | Game data storage |
| 6379 | DragonflyDB/Redis | Cache layer |
| 9090 | ML Inference | Prometheus metrics |
| 9100 | Worker Pool | Prometheus metrics |
| 5121 | Login Server | rAthena login |
| 6121 | Char Server | rAthena character |
| 5121 | Map Server | rAthena map |

---

**Document Version**: 2.1.0  
**Last Updated**: 2026-07-27  
**Next Review**: 2026-10-27
