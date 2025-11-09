# P2P-DLL Integration Guide

**Date**: 2025-11-08  
**P2P-DLL Version**: 1.0.0  
**Coordinator Version**: 1.0.0  

---

## Overview

This guide provides step-by-step instructions for integrating the WARP P2P-DLL with the p2p-coordinator service.

---

## Prerequisites

### Coordinator Requirements

- Python 3.11+
- PostgreSQL 17 with pgvector, TimescaleDB, Apache AGE extensions
- DragonflyDB (Redis-compatible) or Redis 7+
- 2GB+ RAM
- Network connectivity on port 8001

### P2P-DLL Requirements

- Windows 10/11 x64
- Ragnarok Online client (Warp.exe)
- NEMO patcher
- P2P-DLL built and ready (p2p_network.dll)

---

## Step 1: Configure Coordinator Service

### 1.1 Environment Configuration

Create or update `.env` file in `rathena-AI-world/p2p-coordinator/`:

```bash
# ============================================================================
# Service Configuration
# ============================================================================
SERVICE_NAME=p2p-coordinator
SERVICE_VERSION=1.0.0
ENVIRONMENT=development
DEBUG=true
LOG_LEVEL=INFO

# Service binding
HOST=0.0.0.0
PORT=8001

# ============================================================================
# Database Configuration
# ============================================================================
# PostgreSQL (long-term storage)
DATABASE_URL=postgresql+asyncpg://ai_world_user:ai_world_pass_2025@localhost:5432/ai_world_memory
DATABASE_POOL_SIZE=10
DATABASE_MAX_OVERFLOW=5

# DragonflyDB/Redis (high-speed caching)
REDIS_URL=redis://localhost:6379/0
REDIS_MAX_CONNECTIONS=20

# ============================================================================
# Security Configuration
# ============================================================================
# CORS - MUST include port 8001 for P2P-DLL
CORS_ORIGINS=http://localhost:3000,http://localhost:8000,http://localhost:8001

# JWT Authentication (OPTIONAL - disabled by default)
JWT_VALIDATION_ENABLED=false
JWT_SECRET_KEY=your-secret-key-change-in-production
JWT_ALGORITHM=HS256
JWT_EXPIRATION_SECONDS=3600

# API Key Validation (OPTIONAL - disabled by default)
API_KEY_VALIDATION_ENABLED=false
COORDINATOR_API_KEY=your-coordinator-api-key-here
API_KEY_HEADER=X-API-Key

# ============================================================================
# P2P Features Configuration
# ============================================================================
ENABLE_P2P=true
P2P_ENABLED_ZONES=prontera,geffen,payon
P2P_FALLBACK_ENABLED=true
P2P_MAX_PLAYERS_PER_HOST=50

# Session and host timeouts
SESSION_TIMEOUT_MINUTES=30
HOST_HEARTBEAT_TIMEOUT_MINUTES=5

# ============================================================================
# WebRTC Configuration
# ============================================================================
STUN_SERVERS=stun:stun.l.google.com:19302,stun:stun1.l.google.com:19302
TURN_SERVERS=

# ============================================================================
# AI Service Integration (OPTIONAL)
# ============================================================================
AI_SERVICE_ENABLED=false
AI_SERVICE_URL=http://localhost:8000
AI_SERVICE_TIMEOUT=30

# ============================================================================
# Monitoring Configuration
# ============================================================================
PROMETHEUS_ENABLED=true
PROMETHEUS_PORT=9091
LOG_FORMAT=json
```

### 1.2 Start Coordinator Service

```bash
cd rathena-AI-world/p2p-coordinator

# Activate virtual environment
source venv/bin/activate  # Linux/Mac
# OR
venv\Scripts\activate  # Windows

# Install/update dependencies (if needed)
pip install -r requirements.txt

# Start the service
cd coordinator-service
uvicorn main:app --host 0.0.0.0 --port 8001 --reload
```

### 1.3 Verify Coordinator is Running

```bash
# Check health endpoint
curl http://localhost:8001/health

# Expected response:
# {"status":"healthy","database":"connected","redis":"connected"}

# Check root endpoint
curl http://localhost:8001/

# Expected response:
# {
#   "service": "p2p-coordinator",
#   "version": "1.0.0",
#   "status": "running",
#   "p2p_enabled": true,
#   "enabled_zones": ["prontera", "geffen", "payon"]
# }
```

---

## Step 2: Configure P2P-DLL

### 2.1 Create P2P Configuration File

Create `p2p_config.json` in the same directory as `Warp.exe`:

```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001/api/v1",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws",
    "timeout_ms": 5000,
    "api_key": ""
  },
  "webrtc": {
    "stun_servers": [
      "stun:stun.l.google.com:19302",
      "stun:stun1.l.google.com:19302"
    ],
    "turn_servers": [],
    "max_peers": 50,
    "connection_timeout_ms": 10000
  },
  "p2p": {
    "enabled": true,
    "fallback_to_server": true,
    "packet_types": {
      "movement": true,
      "chat": true,
      "emotes": true,
      "skills": false,
      "items": false
    }
  },
  "security": {
    "encryption_enabled": false,
    "algorithm": "AES-256-GCM"
  },
  "logging": {
    "level": "INFO",
    "file": "logs/p2p_dll.log",
    "max_file_size_mb": 10,
    "max_files": 5,
    "console_output": true
  }
}
```

### 2.2 Configuration Parameters Explained

#### Coordinator Settings

| Parameter | Description | Default | Required |
|-----------|-------------|---------|----------|
| `rest_api_url` | Coordinator REST API base URL | - | ✅ YES |
| `websocket_url` | Coordinator WebSocket URL | - | ✅ YES |
| `timeout_ms` | HTTP request timeout | 5000 | ❌ NO |
| `api_key` | API key for authentication | "" | ❌ NO |

**Important**: 
- Use `http://` and `ws://` for local development
- Use `https://` and `wss://` for production
- Port must match coordinator's PORT setting (default: 8001)
- Path must include `/api/v1/` prefix

#### WebRTC Settings

| Parameter | Description | Default | Required |
|-----------|-------------|---------|----------|
| `stun_servers` | List of STUN servers | Google STUN | ✅ YES |
| `turn_servers` | List of TURN servers | [] | ❌ NO |
| `max_peers` | Maximum peer connections | 50 | ❌ NO |
| `connection_timeout_ms` | Connection timeout | 10000 | ❌ NO |

#### P2P Settings

| Parameter | Description | Default | Required |
|-----------|-------------|---------|----------|
| `enabled` | Enable P2P networking | true | ✅ YES |
| `fallback_to_server` | Fallback to server if P2P fails | true | ✅ YES |
| `packet_types` | Which packet types to send via P2P | See config | ❌ NO |

---

## Step 3: Integration Testing

### 3.1 Test Authentication (Optional)

If JWT authentication is enabled:

```bash
# Request JWT token
curl -X POST http://localhost:8001/api/v1/auth/token \
  -H "Content-Type: application/json" \
  -d '{
    "player_id": "test_player_123",
    "user_id": "test_user_456"
  }'

# Expected response:
# {
#   "access_token": "eyJ0eXAiOiJKV1QiLCJhbGc...",
#   "token_type": "Bearer",
#   "expires_in": 3600
# }
```

### 3.2 Test Host Registration

```bash
# Register as P2P host
curl -X POST http://localhost:8001/api/v1/hosts/register \
  -H "Content-Type: application/json" \
  -d '{
    "host_id": "test_host_001",
    "player_id": "test_player_123",
    "player_name": "TestPlayer",
    "ip_address": "192.168.1.100",
    "port": 7777,
    "cpu_cores": 8,
    "cpu_frequency_mhz": 3600,
    "memory_mb": 16384,
    "network_speed_mbps": 1000
  }'
```

### 3.3 Test WebSocket Signaling

Use a WebSocket client to test signaling:

```javascript
const ws = new WebSocket('ws://localhost:8001/api/v1/signaling/ws?peer_id=test_peer_001');

ws.onopen = () => {
  console.log('Connected to signaling server');
  
  // Join a session
  ws.send(JSON.stringify({
    type: 'join',
    session_id: 'test_session_001'
  }));
};

ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  console.log('Received:', message);
};
```

---

## Step 4: Production Deployment

### 4.1 Production Configuration Changes

```bash
# Update .env for production
ENVIRONMENT=production
DEBUG=false
LOG_LEVEL=WARNING

# Enable authentication
JWT_VALIDATION_ENABLED=true
JWT_SECRET_KEY=<strong-random-secret-key>

API_KEY_VALIDATION_ENABLED=true
COORDINATOR_API_KEY=<strong-random-api-key>

# Use HTTPS/WSS
CORS_ORIGINS=https://yourdomain.com

# Production database
DATABASE_URL=postgresql+asyncpg://prod_user:strong_password@db.internal:5432/p2p_prod
REDIS_URL=redis://cache.internal:6379/0
```

### 4.2 Update P2P-DLL Configuration

```json
{
  "coordinator": {
    "rest_api_url": "https://coordinator.yourdomain.com/api/v1",
    "websocket_url": "wss://coordinator.yourdomain.com/api/v1/signaling/ws",
    "api_key": "<your-api-key>"
  }
}
```

---

## Troubleshooting

### Common Issues

#### 1. Connection Refused

**Symptom**: P2P-DLL cannot connect to coordinator

**Solutions**:
- Verify coordinator is running: `curl http://localhost:8001/health`
- Check firewall allows port 8001
- Verify `rest_api_url` in P2P config matches coordinator URL
- Check coordinator logs for errors

#### 2. CORS Errors

**Symptom**: Browser console shows CORS errors

**Solutions**:
- Verify `CORS_ORIGINS` includes the correct port
- Restart coordinator after changing CORS settings
- Check browser developer tools for exact error

#### 3. WebSocket Connection Fails

**Symptom**: WebSocket connection immediately closes

**Solutions**:
- Verify WebSocket URL uses `ws://` (not `http://`)
- Check `peer_id` query parameter is provided
- Review coordinator logs for WebSocket errors
- Test with WebSocket client tool first

#### 4. Authentication Failures

**Symptom**: 401 Unauthorized errors

**Solutions**:
- Verify `JWT_VALIDATION_ENABLED` matches your setup
- Check JWT token is not expired
- Verify `JWT_SECRET_KEY` matches between token generation and validation
- Try disabling authentication for testing

---

## Next Steps

1. ✅ Coordinator configured and running
2. ✅ P2P-DLL configured
3. ⏭️ Run integration tests (see INTEGRATION_TEST_PLAN.md)
4. ⏭️ Monitor performance and logs
5. ⏭️ Scale to production

---

**Guide Version**: 1.0  
**Last Updated**: 2025-11-08

