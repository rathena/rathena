# P2P Coordinator Architecture

**Version**: 1.0.0  
**Last Updated**: 2025-11-07

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Component Design](#component-design)
4. [Data Flow](#data-flow)
5. [Database Schema](#database-schema)
6. [API Design](#api-design)
7. [Background Tasks](#background-tasks)
8. [Integration Points](#integration-points)

---

## Overview

The P2P Coordinator is a server-side service that manages peer-to-peer connections for a hybrid MMORPG architecture. It coordinates host registration, zone management, WebRTC signaling, and session lifecycle management.

### Key Features

- **Host Registry**: Manage P2P hosts with quality scoring
- **Zone Management**: Configure P2P-enabled zones with requirements
- **WebRTC Signaling**: Facilitate peer-to-peer connections
- **Session Management**: Track active P2P sessions and player connections
- **AI Integration**: Sync with AI autonomous world service
- **Monitoring**: Real-time metrics and health checks

---

## System Architecture

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
│  │  ┌──────────┐ ┌──────────┐                        │    │
│  │  │Signaling │ │    AI    │                        │    │
│  │  │ Service  │ │Integration│                        │    │
│  │  └──────────┘ └──────────┘                        │    │
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
         │                                    │
         │                                    │
    ┌────┴────┐                          ┌───┴────┐
    │  WARP   │                          │   AI   │
    │ P2P     │                          │ Service│
    │ Client  │                          │        │
    └─────────┘                          └────────┘
```

---

## Component Design

### 1. API Layer (`api/`)

**Purpose**: Expose REST API endpoints for client interactions

**Components**:
- `hosts.py`: Host registration and management
- `zones.py`: Zone configuration and status
- `signaling.py`: WebRTC signaling (offer/answer/ICE)
- `sessions.py`: Session lifecycle management
- `monitoring.py`: Metrics and health checks

**Technology**: FastAPI 0.115.5 with async/await

### 2. Service Layer (`services/`)

**Purpose**: Business logic and orchestration

**Components**:
- `host_registry.py`: Host quality scoring and lifecycle
- `zone_manager.py`: Zone configuration and validation
- `session_manager.py`: Session creation and player management
- `signaling.py`: WebRTC signaling coordination
- `ai_integration.py`: AI service communication
- `background_tasks.py`: Periodic maintenance tasks

**Design Patterns**:
- Service pattern for business logic
- Dependency injection via FastAPI
- Async/await for I/O operations

### 3. Data Layer (`models/`, `database.py`)

**Purpose**: Data persistence and ORM

**Components**:
- `models/host.py`: Host entity and status
- `models/zone.py`: Zone entity and configuration
- `models/session.py`: Session entity and player tracking
- `database.py`: Database connection management

**Technology**:
- SQLAlchemy 2.0.36 (async ORM)
- PostgreSQL 17 (primary database)
- DragonflyDB (Redis-compatible cache)

### 4. Background Tasks (`services/background_tasks.py`)

**Purpose**: Periodic maintenance and monitoring

**Tasks**:
- NPC state broadcasting (every 5 seconds)
- Session health monitoring (every 30 seconds)
- Stale session cleanup (every 5 minutes)

**Technology**: asyncio with concurrent task execution

---

## Data Flow

### Session Creation Flow

```
Client → POST /api/sessions/
    ↓
SessionManager.create_session()
    ↓
1. Validate host (online, capacity)
2. Validate zone (enabled, requirements)
3. Create P2PSession entity
4. Update host/zone counters
5. Return session details
    ↓
Client ← Session ID + Details
```

### WebRTC Signaling Flow

```
Peer A → POST /api/signaling/offer
    ↓
SignalingService.create_offer()
    ↓
Store offer in Redis
    ↓
Peer B → GET /api/signaling/offer/{session_id}
    ↓
Retrieve offer from Redis
    ↓
Peer B → POST /api/signaling/answer
    ↓
Store answer in Redis
    ↓
Peer A → GET /api/signaling/answer/{session_id}
    ↓
WebRTC connection established
```

---

## Database Schema

### Hosts Table

```sql
CREATE TABLE hosts (
    id SERIAL PRIMARY KEY,
    host_id VARCHAR(255) UNIQUE NOT NULL,
    player_id VARCHAR(255) NOT NULL,
    player_name VARCHAR(255),
    ip_address VARCHAR(45),
    port INTEGER,
    public_ip VARCHAR(45),
    cpu_cores INTEGER,
    cpu_frequency_mhz INTEGER,
    memory_mb INTEGER,
    network_speed_mbps INTEGER,
    latency_ms FLOAT DEFAULT 0.0,
    packet_loss_percent FLOAT DEFAULT 0.0,
    bandwidth_usage_mbps FLOAT DEFAULT 0.0,
    status hoststatus DEFAULT 'ONLINE',
    max_players INTEGER DEFAULT 50,
    current_players INTEGER DEFAULT 0,
    max_zones INTEGER DEFAULT 5,
    current_zones INTEGER DEFAULT 0,
    quality_score FLOAT DEFAULT 100.0,
    ice_servers JSON,
    extra_metadata JSON,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    last_heartbeat TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
```

### Zones Table

```sql
CREATE TABLE zones (
    id SERIAL PRIMARY KEY,
    zone_id VARCHAR(255) UNIQUE NOT NULL,
    zone_name VARCHAR(255) NOT NULL,
    zone_display_name VARCHAR(255),
    map_name VARCHAR(255),
    max_players INTEGER DEFAULT 100,
    recommended_players INTEGER DEFAULT 50,
    p2p_enabled BOOLEAN DEFAULT TRUE,
    p2p_priority INTEGER DEFAULT 5,
    fallback_enabled BOOLEAN DEFAULT TRUE,
    min_host_quality_score FLOAT DEFAULT 7.0,
    min_bandwidth_mbps INTEGER DEFAULT 100,
    max_latency_ms INTEGER DEFAULT 100,
    status zonestatus DEFAULT 'ENABLED',
    total_sessions INTEGER DEFAULT 0,
    active_sessions INTEGER DEFAULT 0,
    total_players_served INTEGER DEFAULT 0,
    description TEXT,
    extra_metadata JSON,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
```

### P2P Sessions Table

```sql
CREATE TABLE p2p_sessions (
    id SERIAL PRIMARY KEY,
    session_id VARCHAR(255) UNIQUE NOT NULL,
    host_id INTEGER REFERENCES hosts(id),
    zone_id INTEGER REFERENCES zones(id),
    max_players INTEGER,
    current_players INTEGER DEFAULT 0,
    status sessionstatus DEFAULT 'PENDING',
    average_latency_ms FLOAT DEFAULT 0.0,
    average_packet_loss_percent FLOAT DEFAULT 0.0,
    bandwidth_usage_mbps FLOAT DEFAULT 0.0,
    quality_score FLOAT DEFAULT 100.0,
    player_satisfaction_score FLOAT DEFAULT 0.0,
    connection_type VARCHAR(50),
    ice_connection_state VARCHAR(50),
    connected_players JSON,
    events JSON,
    extra_metadata JSON,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    started_at TIMESTAMP WITH TIME ZONE,
    ended_at TIMESTAMP WITH TIME ZONE
);
```

---

## API Design

See [API.md](API.md) for complete API documentation.

---

## Background Tasks

### NPC State Broadcasting
- **Frequency**: Every 5 seconds
- **Purpose**: Broadcast NPC states to P2P hosts
- **Implementation**: Placeholder for future integration

### Session Health Monitoring
- **Frequency**: Every 30 seconds
- **Purpose**: Monitor session metrics and trigger fallback if needed
- **Implementation**: Placeholder for future integration

### Stale Session Cleanup
- **Frequency**: Every 5 minutes
- **Purpose**: Clean up inactive sessions (timeout: 30 minutes)
- **Implementation**: Fully functional

---

## Integration Points

### AI Autonomous World Service
- **URL**: `http://localhost:8000` (configurable)
- **Endpoints**:
  - `GET /health` - Health check
  - `GET /api/world/zones/{zone_id}/npcs` - Get NPCs in zone
  - `GET /api/npcs/{npc_id}` - Get NPC state
  - `PATCH /api/npcs/{npc_id}/position` - Update NPC position

### WARP P2P Client
- **Protocol**: HTTP REST API
- **Endpoints**: All `/api/*` endpoints
- **Authentication**: None (to be implemented)

---

## Security Considerations

1. **Authentication**: Not yet implemented (planned for future)
2. **Rate Limiting**: Not yet implemented (planned for future)
3. **Input Validation**: Pydantic models for all API inputs
4. **CORS**: Configurable origins (default: `["*"]`)
5. **SQL Injection**: Protected by SQLAlchemy ORM

---

## Performance Considerations

1. **Database Connection Pooling**: Async SQLAlchemy engine
2. **Redis Caching**: DragonflyDB for fast key-value operations
3. **Async I/O**: All I/O operations use async/await
4. **Background Tasks**: Concurrent execution with asyncio

---

## Deployment

See [DEPLOYMENT.md](DEPLOYMENT.md) for deployment instructions.

---

## Configuration

See [CONFIGURATION.md](CONFIGURATION.md) for configuration options.

