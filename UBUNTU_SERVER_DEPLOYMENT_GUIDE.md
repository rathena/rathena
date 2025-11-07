# Ubuntu Server 24.04 - rAthena AI World Backend Deployment Guide

**Version**: 1.0  
**Date**: 2025-11-07  
**Target**: Ubuntu Server 24.04 LTS  
**Difficulty**: Beginner-Friendly

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites & System Requirements](#prerequisites--system-requirements)
3. [Step 1: System Preparation](#step-1-system-preparation)
4. [Step 2: PostgreSQL 17 Installation](#step-2-postgresql-17-installation)
5. [Step 3: DragonflyDB Installation](#step-3-dragonflydb-installation)
6. [Step 4: rAthena Core Server Setup](#step-4-rathena-core-server-setup)
7. [Step 5: AI Autonomous World Setup](#step-5-ai-autonomous-world-setup)
8. [Step 6: P2P Coordinator Setup](#step-6-p2p-coordinator-setup)
9. [Step 7: Starting All Services](#step-7-starting-all-services)
10. [Step 8: Verification & Testing](#step-8-verification--testing)
11. [Troubleshooting](#troubleshooting)
12. [Maintenance & Monitoring](#maintenance--monitoring)

---

## Overview

This guide will walk you through deploying the complete rAthena AI World system on Ubuntu Server 24.04. The system consists of:

- **rAthena Core Server** (C++) - MMORPG game server
- **AI Autonomous World Service** (Python/FastAPI) - AI-driven NPCs and world systems
- **P2P Coordinator Service** (Python/FastAPI) - Peer-to-peer session management
- **PostgreSQL 17** - Long-term memory storage with pgvector, TimescaleDB, Apache AGE
- **DragonflyDB** - High-speed Redis-compatible cache

**Estimated Time**: 2-3 hours  
**Skill Level**: Basic Linux command-line knowledge required

---

## Prerequisites & System Requirements

### Hardware Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CPU | 4 cores @ 2.5 GHz | 8+ cores @ 3.0+ GHz |
| RAM | 8 GB | 16+ GB |
| Storage | 50 GB SSD | 100+ GB NVMe SSD |
| Network | 100 Mbps | 1 Gbps |

### Software Requirements

- **OS**: Ubuntu Server 24.04 LTS (fresh installation recommended)
- **User**: Non-root user with sudo privileges
- **Internet**: Active internet connection for package downloads
- **Ports**: Ensure the following ports are available:
  - `6900` - rAthena login server
  - `6121` - rAthena char server
  - `5121` - rAthena map server
  - `8000` - AI service
  - `8001` - P2P coordinator
  - `5432` - PostgreSQL
  - `6379` - DragonflyDB

### Required Accounts

- **LLM Provider API Key** (at least one):
  - Azure OpenAI Foundry (recommended)
  - OpenAI
  - DeepSeek (cost-effective)
  - Anthropic Claude
  - Google Gemini

---

## Step 1: System Preparation

### 1.1 Update System

```bash
# Update package lists and upgrade existing packages
sudo apt update && sudo apt upgrade -y

# Install essential build tools
sudo apt install -y build-essential git curl wget vim nano \
    software-properties-common apt-transport-https ca-certificates \
    gnupg lsb-release
```

### 1.2 Create Working Directory

```bash
# Create directory for the project
mkdir -p ~/ai-mmorpg-world
cd ~/ai-mmorpg-world

# Clone the repository
git clone https://github.com/iskandarsulaili/ai-mmorpg-world.git
cd ai-mmorpg-world
```

> **Note**: Replace `iskandarsulaili` with your actual GitHub username or use the correct repository URL.

### 1.3 Set Up Firewall (Optional but Recommended)

```bash
# Install UFW if not already installed
sudo apt install -y ufw

# Allow SSH (IMPORTANT: Do this first!)
sudo ufw allow 22/tcp

# Allow rAthena ports
sudo ufw allow 6900/tcp  # Login server
sudo ufw allow 6121/tcp  # Char server
sudo ufw allow 5121/tcp  # Map server

# Allow AI services
sudo ufw allow 8000/tcp  # AI service
sudo ufw allow 8001/tcp  # P2P coordinator

# Enable firewall
sudo ufw enable
sudo ufw status
```

> ⚠️ **Warning**: Make sure to allow SSH (port 22) before enabling the firewall, or you may lock yourself out!

---

## Step 2: PostgreSQL 17 Installation

PostgreSQL 17 is required for long-term memory storage with advanced extensions.

### 2.1 Add PostgreSQL APT Repository

```bash
# Add PostgreSQL GPG key
curl -fsSL https://www.postgresql.org/media/keys/ACCC4CF8.asc | \
    sudo gpg --dearmor -o /usr/share/keyrings/postgresql-keyring.gpg

# Add PostgreSQL repository
echo "deb [signed-by=/usr/share/keyrings/postgresql-keyring.gpg] \
    https://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" | \
    sudo tee /etc/apt/sources.list.d/pgdg.list

# Update package lists
sudo apt update
```

### 2.2 Install PostgreSQL 17

```bash
# Install PostgreSQL 17 and contrib packages
sudo apt install -y postgresql-17 postgresql-contrib-17 postgresql-server-dev-17

# Verify installation
psql --version
# Expected output: psql (PostgreSQL) 17.x
```

### 2.3 Install PostgreSQL Extensions

#### Install pgvector (Vector Similarity Search)

```bash
# Clone pgvector repository
cd /tmp
git clone --branch v0.7.0 https://github.com/pgvector/pgvector.git
cd pgvector

# Build and install
make
sudo make install

# Verify installation
sudo -u postgres psql -c "CREATE EXTENSION IF NOT EXISTS vector;"
```

#### Install TimescaleDB (Time-Series Data)

```bash
# Add TimescaleDB repository
sudo sh -c "echo 'deb [signed-by=/usr/share/keyrings/timescale-keyring.gpg] \
    https://packagecloud.io/timescale/timescaledb/ubuntu/ $(lsb_release -c -s) main' \
    > /etc/apt/sources.list.d/timescaledb.list"

# Add GPG key
wget --quiet -O - https://packagecloud.io/timescale/timescaledb/gpgkey | \
    sudo gpg --dearmor -o /usr/share/keyrings/timescale-keyring.gpg

# Update and install
sudo apt update
sudo apt install -y timescaledb-2-postgresql-17

# Configure TimescaleDB
sudo timescaledb-tune --quiet --yes

# Restart PostgreSQL
sudo systemctl restart postgresql
```

#### Install Apache AGE (Graph Database)

```bash
# Install dependencies
sudo apt install -y postgresql-server-dev-17 libreadline-dev zlib1g-dev flex bison

# Clone Apache AGE repository
cd /tmp
git clone --branch PG17 https://github.com/apache/age.git
cd age

# Build and install
make
sudo make install

# Verify installation
sudo -u postgres psql -c "CREATE EXTENSION IF NOT EXISTS age;"
```

### 2.4 Create Database and User

```bash
# Switch to postgres user
sudo -u postgres psql

# In PostgreSQL prompt, run:
CREATE DATABASE ai_world_memory;
CREATE USER ai_world_user WITH ENCRYPTED PASSWORD 'your_secure_password_here';
GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;

# Connect to the database
\c ai_world_memory

# Enable extensions
CREATE EXTENSION IF NOT EXISTS vector;
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS age;

# Grant schema permissions
GRANT ALL ON SCHEMA public TO ai_world_user;

# Exit PostgreSQL
\q
```

> 🔒 **Security**: Replace `your_secure_password_here` with a strong password. Save this password securely!

### 2.5 Verify PostgreSQL Installation

```bash
# Test connection
psql -h localhost -U ai_world_user -d ai_world_memory -c "SELECT version();"

# Check extensions
psql -h localhost -U ai_world_user -d ai_world_memory -c "\dx"
```

**Expected output**: You should see `vector`, `timescaledb`, and `age` extensions listed.

---

## Step 3: DragonflyDB Installation

DragonflyDB is a high-performance Redis-compatible in-memory database.

### 3.1 Install DragonflyDB

```bash
# Download and run the official installation script
curl -fsSL https://www.dragonflydb.io/install.sh | bash

# Verify installation
dragonfly --version
```

### 3.2 Create Systemd Service

```bash
# Create systemd service file
sudo tee /etc/systemd/system/dragonfly.service > /dev/null <<EOF
[Unit]
Description=DragonflyDB
After=network.target

[Service]
Type=simple
User=dragonfly
Group=dragonfly
ExecStart=/usr/local/bin/dragonfly --logtostderr
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

# Create dragonfly user
sudo useradd -r -s /bin/false dragonfly

# Create data directory
sudo mkdir -p /var/lib/dragonfly
sudo chown dragonfly:dragonfly /var/lib/dragonfly

# Reload systemd and start service
sudo systemctl daemon-reload
sudo systemctl enable dragonfly
sudo systemctl start dragonfly
```

### 3.3 Verify DragonflyDB Installation

```bash
# Check service status
sudo systemctl status dragonfly

# Test connection using redis-cli
redis-cli ping
# Expected output: PONG
```

> **Note**: If `redis-cli` is not installed, install it with: `sudo apt install -y redis-tools`

---

## Step 4: rAthena Core Server Setup

### 4.1 Install rAthena Dependencies

```bash
# Install required packages
sudo apt install -y \
    gcc g++ make cmake \
    libmysqlclient-dev \
    libpcre3-dev \
    zlib1g-dev \
    libssl-dev

# Install MariaDB (required by rAthena)
sudo apt install -y mariadb-server mariadb-client

# Secure MariaDB installation
sudo mysql_secure_installation
```

> **MariaDB Setup**: Follow the prompts to set a root password and secure your installation.

### 4.2 Create rAthena Database

```bash
# Login to MariaDB
sudo mysql -u root -p

# In MariaDB prompt, run:
CREATE DATABASE ragnarok;
CREATE USER 'ragnarok'@'localhost' IDENTIFIED BY 'your_ragnarok_password';
GRANT ALL PRIVILEGES ON ragnarok.* TO 'ragnarok'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### 4.3 Import rAthena SQL Files

```bash
# Navigate to rAthena directory
cd ~/ai-mmorpg-world/rathena-AI-world

# Import main SQL files
mysql -u ragnarok -p ragnarok < sql-files/main.sql
mysql -u ragnarok -p ragnarok < sql-files/logs.sql
mysql -u ragnarok -p ragnarok < sql-files/item_db.sql
mysql -u ragnarok -p ragnarok < sql-files/mob_db.sql

# Import additional SQL files as needed
```

### 4.4 Configure rAthena

```bash
# Copy configuration templates
cd ~/ai-mmorpg-world/rathena-AI-world/conf/import-tmpl
cp char_conf.txt ../import/
cp map_conf.txt ../import/
cp login_conf.txt ../import/
cp inter_conf.txt ../import/

# Edit inter_conf.txt with database credentials
nano ../import/inter_conf.txt
```

**Edit the following lines in `inter_conf.txt`**:

```ini
sql.db_username: ragnarok
sql.db_password: your_ragnarok_password
sql.db_database: ragnarok
```

### 4.5 Build rAthena

```bash
# Navigate to rAthena directory
cd ~/ai-mmorpg-world/rathena-AI-world

# Configure build
./configure

# Compile (this may take 10-20 minutes)
make server

# Verify build
ls -lh *-server*
# You should see: login-server, char-server, map-server, web-server
```

> ⏱️ **Build Time**: The compilation process may take 10-20 minutes depending on your system.

---

## Step 5: AI Autonomous World Setup

### 5.1 Install Python and Dependencies

```bash
# Install Python 3.11+
sudo apt install -y python3.11 python3.11-venv python3-pip python3-dev

# Verify Python version
python3.11 --version
# Expected: Python 3.11.x or higher
```

### 5.2 Run Automated Installation Script

The AI autonomous world includes an automated installation script that handles everything.

```bash
# Navigate to AI autonomous world directory
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world

# Run the installation script
./install.sh
```

**The script will:**
1. ✅ Verify PostgreSQL 17 is installed
2. ✅ Verify DragonflyDB is running
3. ✅ Create Python virtual environment
4. ✅ Install all Python dependencies
5. ✅ Create `.env` configuration file
6. ✅ Verify all installations

> **Interactive Prompts**: The script will ask for your PostgreSQL password. Use the password you set in Step 2.4.

### 5.3 Configure LLM API Keys

After installation, you need to configure at least one LLM provider API key.

```bash
# Edit the .env file
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
nano .env
```

**Add your API key(s)**:

```bash
# Azure OpenAI (Recommended)
AZURE_OPENAI_API_KEY=your_azure_api_key_here
AZURE_OPENAI_ENDPOINT=https://your-resource.openai.azure.com/
AZURE_OPENAI_DEPLOYMENT=gpt-4

# OR OpenAI
OPENAI_API_KEY=your_openai_api_key_here

# OR DeepSeek (Cost-effective)
DEEPSEEK_API_KEY=your_deepseek_api_key_here

# OR Anthropic Claude
ANTHROPIC_API_KEY=your_anthropic_api_key_here

# OR Google Gemini
GOOGLE_API_KEY=your_google_api_key_here
```

> 🔑 **API Keys**: You need at least ONE LLM provider configured. Azure OpenAI Foundry is recommended for production.

### 5.4 Verify AI Service Installation

```bash
# Activate virtual environment
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world
source venv/bin/activate

# Test Python imports
python3 -c "import fastapi, redis, sqlalchemy, psycopg2; print('✓ All dependencies OK')"

# Check configuration
cd ai-service
python3 -c "from config import settings; print('✓ Configuration loaded')"
```

---

## Step 6: P2P Coordinator Setup

### 6.1 Install P2P Coordinator Dependencies

```bash
# Navigate to P2P coordinator directory
cd ~/ai-mmorpg-world/rathena-AI-world/p2p-coordinator

# Create virtual environment
python3.11 -m venv venv

# Activate virtual environment
source venv/bin/activate

# Upgrade pip
pip install --upgrade pip setuptools wheel

# Install dependencies
pip install -r requirements.txt
```

### 6.2 Configure P2P Coordinator

```bash
# Copy environment template
cp .env.example .env

# Edit configuration
nano .env
```

**Edit the following settings**:

```bash
# Server Configuration
HOST=0.0.0.0
PORT=8001
WORKERS=4

# Database Configuration
DRAGONFLY_HOST=localhost
DRAGONFLY_PORT=6379
POSTGRES_HOST=localhost
POSTGRES_PORT=5432
POSTGRES_DB=p2p_coordinator
POSTGRES_USER=p2p_user
POSTGRES_PASSWORD=your_p2p_password

# AI Service Integration
AI_SERVICE_URL=http://localhost:8000
AI_SERVICE_ENABLED=true

# Security
SECRET_KEY=generate_a_secure_random_key_here
CORS_ORIGINS=http://localhost:3000,http://localhost:8000

# Logging
LOG_LEVEL=INFO
```

> 🔐 **Secret Key**: Generate a secure random key using: `openssl rand -hex 32`

### 6.3 Create P2P Coordinator Database

```bash
# Login to PostgreSQL
sudo -u postgres psql

# Create database and user
CREATE DATABASE p2p_coordinator;
CREATE USER p2p_user WITH ENCRYPTED PASSWORD 'your_p2p_password';
GRANT ALL PRIVILEGES ON DATABASE p2p_coordinator TO p2p_user;
\q
```

### 6.4 Initialize P2P Coordinator Database

```bash
# Activate virtual environment
cd ~/ai-mmorpg-world/rathena-AI-world/p2p-coordinator
source venv/bin/activate

# Run database migrations (if available)
cd coordinator-service
python3 -c "from database import init_db; init_db()"
```

---

## Step 7: Starting All Services

### 7.1 Start Services in Order

**Terminal 1: Start rAthena Servers**

```bash
cd ~/ai-mmorpg-world/rathena-AI-world

# Start login server
./login-server &

# Start char server
./char-server &

# Start map server
./map-server &

# Check if servers are running
ps aux | grep server
```

**Terminal 2: Start AI Service**

```bash
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world
source venv/bin/activate
cd ai-service

# Start AI service
python3 main.py
```

**Terminal 3: Start P2P Coordinator**

```bash
cd ~/ai-mmorpg-world/rathena-AI-world/p2p-coordinator
source venv/bin/activate
cd coordinator-service

# Start P2P coordinator
python3 main.py
```

### 7.2 Using Screen or Tmux (Recommended)

For production, use `screen` or `tmux` to manage multiple services:

```bash
# Install screen
sudo apt install -y screen

# Create screen sessions
screen -S rathena
# In screen: start rAthena servers, then Ctrl+A, D to detach

screen -S ai-service
# In screen: start AI service, then Ctrl+A, D to detach

screen -S p2p-coordinator
# In screen: start P2P coordinator, then Ctrl+A, D to detach

# List screen sessions
screen -ls

# Reattach to a session
screen -r rathena
```

---

## Step 8: Verification & Testing

### 8.1 Verify PostgreSQL

```bash
# Check PostgreSQL service
sudo systemctl status postgresql

# Test connection
psql -h localhost -U ai_world_user -d ai_world_memory -c "SELECT 1;"

# Check extensions
psql -h localhost -U ai_world_user -d ai_world_memory -c "\dx"
```

**Expected**: PostgreSQL should be active, connection successful, extensions listed.

### 8.2 Verify DragonflyDB

```bash
# Check service status
sudo systemctl status dragonfly

# Test connection
redis-cli ping
# Expected: PONG

# Check info
redis-cli info server
```

### 8.3 Verify rAthena Servers

```bash
# Check if servers are running
ps aux | grep -E "(login|char|map)-server"

# Check server logs
tail -f ~/ai-mmorpg-world/rathena-AI-world/log/login-server.log
tail -f ~/ai-mmorpg-world/rathena-AI-world/log/char-server.log
tail -f ~/ai-mmorpg-world/rathena-AI-world/log/map-server.log
```

**Expected**: All three servers should be running without errors.

### 8.4 Verify AI Service

```bash
# Check health endpoint
curl http://localhost:8000/health

# Expected output:
# {"status":"healthy","timestamp":"..."}

# Check API documentation
curl http://localhost:8000/docs
# Or open in browser: http://your-server-ip:8000/docs
```

### 8.5 Verify P2P Coordinator

```bash
# Check health endpoint
curl http://localhost:8001/health

# Expected output:
# {"status":"healthy","database":"connected","redis":"connected"}

# Check dashboard
curl http://localhost:8001/api/monitoring/dashboard | jq .
```

### 8.6 End-to-End Test

```bash
# Test AI service NPC endpoint
curl -X POST http://localhost:8000/api/npc/dialogue \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_npc_001",
    "player_message": "Hello!",
    "context": {}
  }'

# Test P2P coordinator host registration
curl -X POST http://localhost:8001/api/hosts/ \
  -H "Content-Type: application/json" \
  -d '{
    "host_id": "test-host-001",
    "player_id": "test-player-001",
    "player_name": "TestPlayer",
    "cpu_cores": 8,
    "memory_mb": 16384,
    "network_speed_mbps": 1000
  }'
```

---

## Troubleshooting

### PostgreSQL Issues

**Problem**: Cannot connect to PostgreSQL

```bash
# Check if PostgreSQL is running
sudo systemctl status postgresql

# Check PostgreSQL logs
sudo tail -f /var/log/postgresql/postgresql-17-main.log

# Restart PostgreSQL
sudo systemctl restart postgresql
```

**Problem**: Extension installation failed

```bash
# Verify PostgreSQL development packages
sudo apt install -y postgresql-server-dev-17

# Rebuild extension
cd /tmp/pgvector
make clean
make
sudo make install
```

### DragonflyDB Issues

**Problem**: DragonflyDB not starting

```bash
# Check logs
sudo journalctl -u dragonfly -f

# Try running in foreground for debugging
dragonfly --logtostderr

# Check if port 6379 is already in use
sudo netstat -tulpn | grep 6379
```

### rAthena Server Issues

**Problem**: Server crashes on startup

```bash
# Check configuration files
cd ~/ai-mmorpg-world/rathena-AI-world/conf/import
cat inter_conf.txt

# Verify database connection
mysql -u ragnarok -p ragnarok -e "SHOW TABLES;"

# Check server logs for errors
tail -100 log/map-server.log
```

### AI Service Issues

**Problem**: AI service fails to start

```bash
# Check Python dependencies
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world
source venv/bin/activate
pip list

# Verify .env file
cd ai-service
cat .env | grep -v "PASSWORD\|KEY"

# Test imports
python3 -c "import fastapi, redis, sqlalchemy"
```

**Problem**: LLM API errors

```bash
# Verify API key is set
cd ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
grep "API_KEY" .env

# Test API connection
python3 -c "
from config import settings
print(f'Provider: {settings.LLM_DEFAULT_PROVIDER}')
print(f'API Key set: {bool(settings.OPENAI_API_KEY)}')
"
```

### P2P Coordinator Issues

**Problem**: P2P coordinator cannot connect to database

```bash
# Verify PostgreSQL database exists
sudo -u postgres psql -l | grep p2p_coordinator

# Test connection
psql -h localhost -U p2p_user -d p2p_coordinator -c "SELECT 1;"

# Check .env configuration
cd ~/ai-mmorpg-world/rathena-AI-world/p2p-coordinator
grep "POSTGRES" .env
```

### Port Conflicts

**Problem**: Port already in use

```bash
# Check which process is using a port
sudo netstat -tulpn | grep :8000
sudo netstat -tulpn | grep :8001

# Kill process if needed
sudo kill -9 <PID>
```

### Memory Issues

**Problem**: Out of memory errors

```bash
# Check memory usage
free -h

# Check swap
swapon --show

# Add swap if needed (4GB example)
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
```

---

## Maintenance & Monitoring

### Daily Checks

```bash
# Check all services status
sudo systemctl status postgresql dragonfly
ps aux | grep -E "(login|char|map)-server"
curl http://localhost:8000/health
curl http://localhost:8001/health
```

### Log Monitoring

```bash
# rAthena logs
tail -f ~/ai-mmorpg-world/rathena-AI-world/log/*.log

# AI service logs
tail -f ~/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/logs/*.log

# P2P coordinator logs
tail -f ~/ai-mmorpg-world/rathena-AI-world/p2p-coordinator/coordinator-service/logs/*.log

# System logs
sudo journalctl -f
```

### Database Backups

```bash
# Backup PostgreSQL databases
pg_dump -h localhost -U ai_world_user ai_world_memory > backup_ai_$(date +%Y%m%d).sql
pg_dump -h localhost -U p2p_user p2p_coordinator > backup_p2p_$(date +%Y%m%d).sql

# Backup MariaDB (rAthena)
mysqldump -u ragnarok -p ragnarok > backup_rathena_$(date +%Y%m%d).sql
```

### Automated Startup (Systemd Services)

Create systemd service files for automatic startup on boot.

**AI Service** (`/etc/systemd/system/ai-service.service`):

```ini
[Unit]
Description=AI Autonomous World Service
After=network.target postgresql.service dragonfly.service

[Service]
Type=simple
User=your_username
WorkingDirectory=/home/your_username/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
Environment="PATH=/home/your_username/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/venv/bin"
ExecStart=/home/your_username/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/venv/bin/python3 main.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**P2P Coordinator** (`/etc/systemd/system/p2p-coordinator.service`):

```ini
[Unit]
Description=P2P Coordinator Service
After=network.target postgresql.service dragonfly.service

[Service]
Type=simple
User=your_username
WorkingDirectory=/home/your_username/ai-mmorpg-world/rathena-AI-world/p2p-coordinator/coordinator-service
Environment="PATH=/home/your_username/ai-mmorpg-world/rathena-AI-world/p2p-coordinator/venv/bin"
ExecStart=/home/your_username/ai-mmorpg-world/rathena-AI-world/p2p-coordinator/venv/bin/python3 main.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Enable and start services**:

```bash
# Reload systemd
sudo systemctl daemon-reload

# Enable services
sudo systemctl enable ai-service
sudo systemctl enable p2p-coordinator

# Start services
sudo systemctl start ai-service
sudo systemctl start p2p-coordinator

# Check status
sudo systemctl status ai-service
sudo systemctl status p2p-coordinator
```

---

## Next Steps

After successful deployment:

1. **Configure Game Client**: Set up the WARP P2P client on Windows (see Windows guide)
2. **Create NPCs**: Add AI-enabled NPCs to your game world
3. **Test P2P Features**: Enable P2P hosting for specific zones
4. **Monitor Performance**: Use the monitoring dashboards to track system health
5. **Scale Up**: Add more resources as your player base grows

---

## Additional Resources

- **AI Service Documentation**: `rathena-AI-world/ai-autonomous-world/docs/`
- **P2P Coordinator Documentation**: `rathena-AI-world/p2p-coordinator/docs/`
- **rAthena Wiki**: https://github.com/rathena/rathena/wiki
- **PostgreSQL Documentation**: https://www.postgresql.org/docs/17/
- **DragonflyDB Documentation**: https://www.dragonflydb.io/docs

---

## Support

For issues or questions:
- Check logs in respective service directories
- Review troubleshooting section above
- Consult documentation in `docs/` directories
- Check GitHub issues for known problems

---

**Congratulations!** 🎉 You have successfully deployed the rAthena AI World backend system on Ubuntu Server 24.04!

