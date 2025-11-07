# P2P Coordinator Service

**Version**: 1.0.0
**Status**: Production Ready
**Last Updated**: 2025-11-07

---

## Overview

The P2P Coordinator is a server-side service that manages peer-to-peer connections for a hybrid MMORPG architecture. It coordinates host registration, zone management, WebRTC signaling, and session lifecycle management for the rAthena AI autonomous world system.

### Key Features

✅ **Host Registry**: Manage P2P hosts with quality scoring
✅ **Zone Management**: Configure P2P-enabled zones with requirements
✅ **WebRTC Signaling**: Facilitate peer-to-peer connections
✅ **Session Management**: Track active P2P sessions and player connections
✅ **AI Integration**: Sync with AI autonomous world service
✅ **Background Tasks**: Automated NPC broadcasting, health monitoring, and cleanup
✅ **Monitoring Dashboard**: Real-time metrics and health checks
✅ **Production Ready**: No mocks, verbose logging, comprehensive error handling

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    P2P Coordinator Service                   │
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   FastAPI    │  │  Background  │  │  Monitoring  │     │
│  │   REST API   │  │    Tasks     │  │  Dashboard   │     │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │
│         │                  │                  │              │
│  ┌──────┴──────────────────┴──────────────────┴───────┐    │
│  │              Service Layer                          │    │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐           │    │
│  │  │   Host   │ │   Zone   │ │ Session  │           │    │
│  │  │ Registry │ │ Manager  │ │ Manager  │           │    │
│  │  └──────────┘ └──────────┘ └──────────┘           │    │
│  └─────────────────────────────────────────────────────┘    │
│         │                                                    │
│  ┌──────┴──────────────────────────────────────────────┐   │
│  │              Data Layer                              │   │
│  │  ┌──────────┐           ┌──────────┐               │   │
│  │  │PostgreSQL│           │DragonflyDB│              │   │
│  │  │  (Main)  │           │  (Redis)  │              │   │
│  │  └──────────┘           └──────────┘               │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## Quick Start

### Prerequisites

- Python 3.11+
- PostgreSQL 17+
- DragonflyDB (Redis 8.2.3 compatible) or Redis 7+

### Installation

```bash
# Navigate to p2p-coordinator directory
cd <workspace>/rathena-AI-world/p2p-coordinator

# Create virtual environment
python3.11 -m venv venv
source venv/bin/activate

# Install dependencies
pip install --upgrade pip setuptools wheel
pip install -r requirements.txt

# Configure environment
cp .env.example .env
nano .env  # Edit configuration

# Run the service
cd coordinator-service
python main.py
```

The service will start on `http://localhost:8001`.

### Verify Installation

```bash
# Health check
curl http://localhost:8001/health

# Dashboard metrics
curl http://localhost:8001/api/monitoring/dashboard
```

---

## Documentation

- **[Architecture](docs/ARCHITECTURE.md)**: System architecture and component design
- **[API Documentation](docs/API.md)**: Complete API reference
- **[Deployment Guide](docs/DEPLOYMENT.md)**: Production deployment instructions
- **[Configuration Guide](docs/CONFIGURATION.md)**: Configuration options and examples

---

## API Endpoints

### Core Services

- **Host Management**: `/api/hosts/`
  - Register, update, and manage P2P hosts
  - Quality scoring and health monitoring

- **Zone Management**: `/api/zones/`
  - Configure P2P-enabled zones
  - Set requirements and priorities

- **Session Management**: `/api/sessions/`
  - Create and manage P2P sessions
  - Track players and metrics

- **WebRTC Signaling**: `/api/signaling/`
  - Offer/answer exchange
  - ICE candidate management

- **Monitoring**: `/api/monitoring/`
  - Dashboard metrics
  - Host statistics
  - System health

### Example Requests

**Register a Host**:
```bash
curl -X POST http://localhost:8001/api/hosts/ \
  -H "Content-Type: application/json" \
  -d '{
    "host_id": "host-12345",
    "player_id": "player-67890",
    "player_name": "PlayerOne",
    "cpu_cores": 8,
    "memory_mb": 16384,
    "network_speed_mbps": 1000
  }'
```

**Create a Session**:
```bash
curl -X POST http://localhost:8001/api/sessions/ \
  -H "Content-Type: application/json" \
  -d '{
    "host_id": "host-12345",
    "zone_id": "prontera",
    "max_players": 50
  }'
```

**Get Dashboard Metrics**:
```bash
curl http://localhost:8001/api/monitoring/dashboard
```

---

## Technology Stack

- **FastAPI 0.115.5**: Modern async web framework
- **SQLAlchemy 2.0.36**: Async ORM for PostgreSQL
- **PostgreSQL 17**: Primary database
- **DragonflyDB**: Redis-compatible in-memory database
- **Pydantic 2.10.3**: Data validation and settings
- **Uvicorn 0.32.1**: ASGI server
- **aiortc 1.10.0**: WebRTC implementation
- **httpx**: Async HTTP client
- **Loguru 0.7.3**: Advanced logging

---

## Project Structure

```
p2p-coordinator/
├── coordinator-service/
│   ├── api/                    # API endpoints
│   │   ├── hosts.py           # Host management
│   │   ├── zones.py           # Zone management
│   │   ├── sessions.py        # Session management
│   │   ├── signaling.py       # WebRTC signaling
│   │   └── monitoring.py      # Monitoring endpoints
│   ├── models/                 # Database models
│   │   ├── host.py            # Host entity
│   │   ├── zone.py            # Zone entity
│   │   └── session.py         # Session entity
│   ├── services/               # Business logic
│   │   ├── host_registry.py   # Host management service
│   │   ├── zone_manager.py    # Zone management service
│   │   ├── session_manager.py # Session management service
│   │   ├── signaling.py       # Signaling service
│   │   ├── ai_integration.py  # AI service client
│   │   └── background_tasks.py # Background tasks
│   ├── config.py               # Configuration management
│   ├── database.py             # Database connection
│   ├── main.py                 # Application entry point
│   └── logs/                   # Log files
├── docs/                       # Documentation
│   ├── ARCHITECTURE.md         # Architecture documentation
│   ├── API.md                  # API documentation
│   ├── DEPLOYMENT.md           # Deployment guide
│   ├── CONFIGURATION.md        # Configuration guide
│   └── PHASE_4_COMPLETION_REPORT.md
├── .env.example                # Environment template
├── .env                        # Environment configuration
├── requirements.txt            # Python dependencies
└── README.md                   # This file
```

---

## Background Tasks

The service runs three background tasks:

1. **NPC State Broadcasting** (every 5 seconds)
   - Broadcasts NPC states to P2P hosts
   - Ensures synchronized game state

2. **Session Health Monitoring** (every 30 seconds)
   - Monitors session metrics
   - Triggers fallback if needed

3. **Stale Session Cleanup** (every 5 minutes)
   - Cleans up inactive sessions
   - Timeout: 30 minutes (configurable)

---

## Integration

### AI Autonomous World Service

The P2P Coordinator integrates with the AI autonomous world service for NPC state synchronization.

**Configuration**:
```bash
AI_SERVICE_ENABLED=true
AI_SERVICE_URL=http://localhost:8000
```

### WARP P2P Client

The **[WARP-p2p-client](https://github.com/iskandarsulaili/WARP-p2p-client)** is a C++ WebRTC client that connects to this P2P Coordinator for session management and WebRTC signaling.

**Connection Details**:
- **WebSocket Endpoint**: `ws://localhost:8001/api/signaling/ws` (development)
- **Production Endpoint**: `wss://coordinator.yourdomain.com/api/signaling/ws`
- **Protocol**: WebSocket-based signaling with JSON messages
- **Message Types**: `join`, `leave`, `offer`, `answer`, `ice-candidate`

**Integration Requirements**:
The WARP client requires WebSocket support and specific message handling to work with this coordinator. See the [P2P Integration Analysis](../../P2P_INTEGRATION_ANALYSIS.md) for detailed compatibility assessment and required modifications.

**Configuration**:
```ini
[P2P_HOSTS]
0=ws://localhost:8001/api/signaling/ws
```

---

## Production Deployment

### Using Systemd

```bash
# Create systemd service
sudo nano /etc/systemd/system/p2p-coordinator.service

# Enable and start
sudo systemctl enable p2p-coordinator
sudo systemctl start p2p-coordinator
sudo systemctl status p2p-coordinator
```

See [DEPLOYMENT.md](docs/DEPLOYMENT.md) for complete instructions.

---

## Monitoring

### Health Check

```bash
curl http://localhost:8001/health
```

### Dashboard

```bash
curl http://localhost:8001/api/monitoring/dashboard | jq .
```

### Logs

```bash
tail -f coordinator-service/logs/p2p-coordinator.log
```

---

## Security

- **Authentication**: Not yet implemented (planned)
- **Rate Limiting**: Not yet implemented (planned)
- **CORS**: Configurable origins
- **Input Validation**: Pydantic models
- **SQL Injection**: Protected by SQLAlchemy ORM

**Production Checklist**:
- [ ] Change `SECRET_KEY` in `.env`
- [ ] Configure CORS origins (don't use `*`)
- [ ] Enable HTTPS with SSL certificates
- [ ] Set up firewall rules
- [ ] Implement authentication
- [ ] Implement rate limiting

---

## License

This project is part of the rAthena AI autonomous world system.

---

## Support

For issues and questions:
- Check [Documentation](docs/)
- Review logs: `coordinator-service/logs/p2p-coordinator.log`
- Check system health: `curl http://localhost:8001/health`

---

## Changelog

### Version 1.0.0 (2025-11-07)

✅ **Phase 1: Foundation** - Complete
- Directory structure and project setup
- FastAPI application skeleton
- Database models (Host, Zone, Session)
- Configuration management with Pydantic v2
- PostgreSQL and Redis integration
- Logging with Loguru
- Health check endpoints

✅ **Phase 2: Core Services** - Complete
- Host Registry service with quality scoring
- Zone Manager service with P2P configuration
- Session Manager service with player tracking
- API endpoints for all core services
- Host health checking and heartbeat monitoring

✅ **Phase 3: WebRTC Signaling** - Complete
- WebRTC signaling server
- ICE candidate exchange
- SDP offer/answer handling
- Redis-based signaling state storage

✅ **Phase 4: Integration** - Complete
- AI service integration client
- Background tasks (NPC broadcast, health monitoring, cleanup)
- Monitoring dashboard endpoints
- Session player management

✅ **Phase 5: Testing & Documentation** - Complete
- Architecture documentation
- API documentation
- Deployment guide
- Configuration guide
- Production-ready code (no mocks, verbose logging)
