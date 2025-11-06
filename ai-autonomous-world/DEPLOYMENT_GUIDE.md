# AI Autonomous World - Deployment Guide

Complete guide for deploying the AI Autonomous World System to production.

---

## 📋 Prerequisites

### System Requirements
- **OS**: Linux (Ubuntu 20.04+ recommended)
- **RAM**: 8GB minimum, 16GB recommended
- **Disk**: 10GB free space
- **CPU**: 4 cores minimum

### Software Requirements
- **Python**: 3.12+
- **DragonflyDB**: Latest version (or Redis 7.0+)
- **rAthena**: Latest version with web server enabled
- **Docker** (optional): 20.10+ with docker-compose

### API Keys Required
- OpenAI API key (for GPT-4)
- Anthropic API key (for Claude-3-Sonnet)
- Google API key (for Gemini-Pro)
- Memori SDK API key (optional, for enhanced memory)

---

## 🚀 Deployment Options

### Option 1: Docker Deployment (Recommended)

**Advantages**: Isolated environment, easy scaling, consistent deployment

#### Step 1: Prepare Environment

```bash
cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world

# Create .env file
cp ai-service/.env.example ai-service/.env

# Edit .env with your API keys
nano ai-service/.env
```

**Required .env variables**:
```env
# LLM Provider API Keys
OPENAI_API_KEY=sk-...
ANTHROPIC_API_KEY=sk-ant-...
GOOGLE_API_KEY=...

# Memori SDK (optional)
MEMORI_API_KEY=...

# Database
REDIS_HOST=dragonfly
REDIS_PORT=6379

# Service Configuration
ENVIRONMENT=production
LOG_LEVEL=INFO
AI_SERVICE_PORT=8000

# Default LLM Provider
DEFAULT_LLM_PROVIDER=openai
DEFAULT_MODEL=gpt-4
```

#### Step 2: Build and Start Services

```bash
# Build and start all services
docker-compose up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f ai-service
docker-compose logs -f dragonfly
```

#### Step 3: Verify Deployment

```bash
# Check AI Service health
curl http://localhost:8000/health

# Check DragonflyDB
redis-cli ping

# View API documentation
open http://localhost:8000/docs
```

---

### Option 2: Manual Deployment

**Advantages**: More control, easier debugging

#### Step 1: Install DragonflyDB

```bash
# Download and install DragonflyDB
curl -fsSL https://www.dragonflydb.io/install.sh | bash

# Start DragonflyDB
dragonfly --port 6379 --maxmemory 2gb --daemonize yes

# Verify
redis-cli ping
```

#### Step 2: Set Up Python Environment

```bash
cd /home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world

# Activate virtual environment
source venv/bin/activate

# Install dependencies (if not already installed)
pip install -r ai-service/requirements-cloud.txt

# Create .env file
cp ai-service/.env.example ai-service/.env
nano ai-service/.env
```

#### Step 3: Configure AI Service

```bash
# Copy config template
cp ai-service/config.yaml.example ai-service/config.yaml

# Edit configuration
nano ai-service/config.yaml
```

**Key configuration options**:
```yaml
service:
  host: "0.0.0.0"
  port: 8000
  environment: "production"
  log_level: "INFO"

database:
  host: "localhost"
  port: 6379
  db: 0
  max_connections: 50

llm:
  default_provider: "openai"
  default_model: "gpt-4"
  temperature: 0.7
  max_tokens: 2000
  timeout: 30

agents:
  enable_memory: true
  enable_crewai: true
  max_concurrent_tasks: 10
```

#### Step 4: Start AI Service

```bash
cd ai-service

# Start with uvicorn (development)
uvicorn main:app --host 0.0.0.0 --port 8000 --reload

# Or start with production settings
python main.py
```

#### Step 5: Set Up as System Service (Optional)

Create systemd service file:

```bash
sudo nano /etc/systemd/system/ai-world-service.service
```

```ini
[Unit]
Description=AI Autonomous World Service
After=network.target dragonfly.service

[Service]
Type=simple
User=lot399
WorkingDirectory=/home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service
Environment="PATH=/home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/venv/bin"
ExecStart=/home/lot399/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/venv/bin/python main.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable ai-world-service
sudo systemctl start ai-world-service
sudo systemctl status ai-world-service
```

---

## 🔧 rAthena Integration

### Step 1: Compile Bridge Layer

```bash
cd /home/lot399/ai-mmorpg-world/rathena-AI-world

# Configure with web server enabled
./configure --enable-webserver

# Compile
make clean
make server

# Verify compilation
ls -lh map-server
```

### Step 2: Configure rAthena

Edit `conf/import/web_config.conf`:

```conf
// AI Service Configuration
ai_service_url: "http://localhost:8000"
ai_service_enabled: true
ai_service_timeout: 30
```

### Step 3: Start rAthena

```bash
# Start login server
./login-server &

# Start char server
./char-server &

# Start map server (with Bridge Layer)
./map-server &
```

---

## 📊 Monitoring and Logging

### View Logs

**AI Service logs**:
```bash
# Docker
docker-compose logs -f ai-service

# Manual
tail -f ai-service/logs/ai-service.log
```

**DragonflyDB logs**:
```bash
# Docker
docker-compose logs -f dragonfly

# Manual
dragonfly-cli monitor
```

### Health Checks

```bash
# AI Service health
curl http://localhost:8000/health

# Detailed health with database status
curl http://localhost:8000/health | jq

# API documentation
curl http://localhost:8000/docs
```

### Performance Monitoring

```bash
# Check DragonflyDB stats
redis-cli info stats

# Check memory usage
redis-cli info memory

# Monitor AI Service
curl http://localhost:8000/metrics  # If metrics endpoint added
```

---

## 🧪 Testing Deployment

### Run Integration Tests

```bash
cd ai-autonomous-world

# Activate venv
source venv/bin/activate

# Run tests
pytest tests/test_integration.py -v

# Run agent tests
pytest tests/test_agents.py -v
```

### Manual API Testing

```bash
# Register test NPC
curl -X POST http://localhost:8000/ai/npc/register \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_merchant_001",
    "name": "Test Merchant",
    "npc_class": "merchant",
    "level": 50,
    "position": {"map": "prontera", "x": 150, "y": 180},
    "personality": {
      "openness": 0.7,
      "conscientiousness": 0.8,
      "extraversion": 0.6,
      "agreeableness": 0.9,
      "neuroticism": 0.3,
      "moral_alignment": "lawful_good"
    }
  }'

# Test player interaction
curl -X POST http://localhost:8000/ai/player/interaction \
  -H "Content-Type: application/json" \
  -d '{
    "npc_id": "test_merchant_001",
    "player_id": "player_001",
    "interaction_type": "talk",
    "message": "Hello! What do you sell?",
    "context": {
      "player_name": "TestPlayer",
      "player_class": "Swordsman",
      "player_level": 25
    }
  }'
```

---

## 🔒 Security Considerations

1. **API Keys**: Never commit .env files to version control
2. **Firewall**: Restrict access to ports 6379 (DragonflyDB) and 8000 (AI Service)
3. **HTTPS**: Use reverse proxy (nginx) with SSL certificates for production
4. **Rate Limiting**: Configure rate limits in FastAPI
5. **Authentication**: Add API authentication for production use

---

## 🚨 Troubleshooting

### AI Service won't start
```bash
# Check logs
docker-compose logs ai-service

# Verify .env file
cat ai-service/.env

# Check port availability
netstat -tulpn | grep 8000
```

### DragonflyDB connection issues
```bash
# Test connection
redis-cli ping

# Check if running
ps aux | grep dragonfly

# Restart
docker-compose restart dragonfly
```

### Bridge Layer compilation errors
```bash
# Check dependencies
ls -la 3rdparty/httplib

# Clean and rebuild
make clean
./configure --enable-webserver
make server
```

---

## 📈 Scaling for Production

### Horizontal Scaling

1. **Multiple AI Service instances** behind load balancer
2. **DragonflyDB clustering** for high availability
3. **Separate LLM provider pools** to avoid rate limits

### Performance Tuning

```yaml
# config.yaml optimizations
database:
  max_connections: 100  # Increase for high load
  
llm:
  timeout: 60  # Increase for complex queries
  max_concurrent_requests: 20
  
agents:
  max_concurrent_tasks: 50  # Increase for high NPC count
```

---

## ✅ Deployment Checklist

- [ ] DragonflyDB installed and running
- [ ] Python 3.12+ virtual environment created
- [ ] All dependencies installed
- [ ] .env file configured with API keys
- [ ] config.yaml customized for environment
- [ ] AI Service starts without errors
- [ ] Health check endpoint returns 200
- [ ] Integration tests pass
- [ ] rAthena Bridge Layer compiled
- [ ] rAthena servers can connect to AI Service
- [ ] Logs are being written correctly
- [ ] Monitoring set up
- [ ] Backups configured for DragonflyDB

---

**Deployment Status**: Ready for production deployment

