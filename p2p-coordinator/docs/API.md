# P2P Coordinator API Documentation

**Version**: 1.0.0  
**Base URL**: `http://localhost:8001`  
**Last Updated**: 2025-11-07

---

## Table of Contents

1. [Overview](#overview)
2. [Authentication](#authentication)
3. [Host Management](#host-management)
4. [Zone Management](#zone-management)
5. [Session Management](#session-management)
6. [WebRTC Signaling](#webrtc-signaling)
7. [Monitoring](#monitoring)
8. [Error Handling](#error-handling)

---

## Overview

The P2P Coordinator API provides RESTful endpoints for managing P2P hosts, zones, sessions, and WebRTC signaling.

### Base URL
```
http://localhost:8001
```

### Content Type
All requests and responses use `application/json`.

---

## Authentication

**Status**: Not yet implemented  
**Planned**: JWT-based authentication

---

## Host Management

### Register Host

**Endpoint**: `POST /api/hosts/`

**Description**: Register a new P2P host

**Request Body**:
```json
{
  "host_id": "host-12345",
  "player_id": "player-67890",
  "player_name": "PlayerOne",
  "ip_address": "192.168.1.100",
  "port": 8080,
  "public_ip": "203.0.113.1",
  "cpu_cores": 8,
  "cpu_frequency_mhz": 3600,
  "memory_mb": 16384,
  "network_speed_mbps": 1000,
  "max_players": 50,
  "max_zones": 5,
  "ice_servers": [
    {
      "urls": "stun:stun.l.google.com:19302"
    }
  ]
}
```

**Response**: `201 Created`
```json
{
  "id": 1,
  "host_id": "host-12345",
  "player_id": "player-67890",
  "status": "ONLINE",
  "quality_score": 100.0,
  "created_at": "2025-11-07T10:00:00Z"
}
```

### Get Host

**Endpoint**: `GET /api/hosts/{host_id}`

**Response**: `200 OK`
```json
{
  "id": 1,
  "host_id": "host-12345",
  "player_id": "player-67890",
  "player_name": "PlayerOne",
  "status": "ONLINE",
  "quality_score": 100.0,
  "current_players": 0,
  "current_zones": 0,
  "latency_ms": 0.0,
  "packet_loss_percent": 0.0
}
```

### List Hosts

**Endpoint**: `GET /api/hosts/`

**Query Parameters**:
- `status` (optional): Filter by status (`ONLINE`, `OFFLINE`, `BUSY`, `MAINTENANCE`)
- `min_quality_score` (optional): Minimum quality score (0-100)

**Response**: `200 OK`
```json
[
  {
    "id": 1,
    "host_id": "host-12345",
    "status": "ONLINE",
    "quality_score": 100.0
  }
]
```

### Update Host Heartbeat

**Endpoint**: `POST /api/hosts/{host_id}/heartbeat`

**Request Body**:
```json
{
  "latency_ms": 25.5,
  "packet_loss_percent": 0.1,
  "bandwidth_usage_mbps": 50.0,
  "current_players": 10
}
```

**Response**: `200 OK`
```json
{
  "message": "Heartbeat updated",
  "quality_score": 98.5
}
```

### Unregister Host

**Endpoint**: `DELETE /api/hosts/{host_id}`

**Response**: `200 OK`
```json
{
  "message": "Host unregistered successfully"
}
```

---

## Zone Management

### Create Zone

**Endpoint**: `POST /api/zones/`

**Request Body**:
```json
{
  "zone_id": "prontera",
  "zone_name": "Prontera",
  "zone_display_name": "Prontera City",
  "map_name": "prontera",
  "max_players": 100,
  "recommended_players": 50,
  "p2p_enabled": true,
  "p2p_priority": 8,
  "min_host_quality_score": 80.0,
  "min_bandwidth_mbps": 100,
  "max_latency_ms": 50
}
```

**Response**: `201 Created`
```json
{
  "id": 1,
  "zone_id": "prontera",
  "zone_name": "Prontera",
  "status": "ENABLED",
  "p2p_enabled": true,
  "created_at": "2025-11-07T10:00:00Z"
}
```

### Get Zone

**Endpoint**: `GET /api/zones/{zone_id}`

**Response**: `200 OK`
```json
{
  "id": 1,
  "zone_id": "prontera",
  "zone_name": "Prontera",
  "status": "ENABLED",
  "p2p_enabled": true,
  "active_sessions": 5,
  "total_players_served": 250
}
```

### List Zones

**Endpoint**: `GET /api/zones/`

**Query Parameters**:
- `p2p_enabled` (optional): Filter by P2P enabled status
- `status` (optional): Filter by status (`ENABLED`, `DISABLED`, `MAINTENANCE`)

**Response**: `200 OK`
```json
[
  {
    "id": 1,
    "zone_id": "prontera",
    "zone_name": "Prontera",
    "status": "ENABLED",
    "p2p_enabled": true
  }
]
```

### Update Zone

**Endpoint**: `PATCH /api/zones/{zone_id}`

**Request Body**:
```json
{
  "p2p_enabled": false,
  "status": "MAINTENANCE"
}
```

**Response**: `200 OK`
```json
{
  "id": 1,
  "zone_id": "prontera",
  "status": "MAINTENANCE",
  "p2p_enabled": false
}
```

---

## Session Management

### Create Session

**Endpoint**: `POST /api/sessions/`

**Request Body**:
```json
{
  "host_id": "host-12345",
  "zone_id": "prontera",
  "max_players": 50
}
```

**Response**: `201 Created`
```json
{
  "id": 1,
  "session_id": "session-abc123",
  "host_id": 1,
  "zone_id": 1,
  "status": "PENDING",
  "max_players": 50,
  "current_players": 0,
  "created_at": "2025-11-07T10:00:00Z"
}
```

### Get Session

**Endpoint**: `GET /api/sessions/{session_id}`

**Response**: `200 OK`
```json
{
  "id": 1,
  "session_id": "session-abc123",
  "host_id": 1,
  "zone_id": 1,
  "status": "ACTIVE",
  "current_players": 10,
  "connected_players": ["player-1", "player-2"],
  "average_latency_ms": 45.5
}
```

### Activate Session

**Endpoint**: `POST /api/sessions/{session_id}/activate`

**Response**: `200 OK`
```json
{
  "message": "Session activated successfully"
}
```

### Update Session Metrics

**Endpoint**: `PATCH /api/sessions/{session_id}/metrics`

**Request Body**:
```json
{
  "average_latency_ms": 45.5,
  "average_packet_loss_percent": 0.2,
  "bandwidth_usage_mbps": 75.0,
  "quality_score": 95.0
}
```

**Response**: `200 OK`
```json
{
  "message": "Session metrics updated successfully"
}
```

### Add Player to Session

**Endpoint**: `POST /api/sessions/{session_id}/players`

**Request Body**:
```json
{
  "player_id": "player-12345"
}
```

**Response**: `200 OK`
```json
{
  "message": "Player player-12345 added to session 1"
}
```

### Remove Player from Session

**Endpoint**: `DELETE /api/sessions/{session_id}/players/{player_id}`

**Response**: `200 OK`
```json
{
  "message": "Player player-12345 removed from session 1"
}
```

---

## WebRTC Signaling

### Create Offer

**Endpoint**: `POST /api/signaling/offer`

**Request Body**:
```json
{
  "session_id": "session-abc123",
  "peer_id": "peer-12345",
  "sdp": "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\n..."
}
```

**Response**: `200 OK`
```json
{
  "message": "Offer created successfully"
}
```

### Get Offer

**Endpoint**: `GET /api/signaling/offer/{session_id}`

**Response**: `200 OK`
```json
{
  "peer_id": "peer-12345",
  "sdp": "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\n..."
}
```

### Create Answer

**Endpoint**: `POST /api/signaling/answer`

**Request Body**:
```json
{
  "session_id": "session-abc123",
  "peer_id": "peer-67890",
  "sdp": "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\n..."
}
```

**Response**: `200 OK`
```json
{
  "message": "Answer created successfully"
}
```

### Get Answer

**Endpoint**: `GET /api/signaling/answer/{session_id}`

**Response**: `200 OK`
```json
{
  "peer_id": "peer-67890",
  "sdp": "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\n..."
}
```

### Add ICE Candidate

**Endpoint**: `POST /api/signaling/ice-candidate`

**Request Body**:
```json
{
  "session_id": "session-abc123",
  "peer_id": "peer-12345",
  "candidate": "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host"
}
```

**Response**: `200 OK`
```json
{
  "message": "ICE candidate added successfully"
}
```

---

## Monitoring

### Dashboard Metrics

**Endpoint**: `GET /api/monitoring/dashboard`

**Response**: `200 OK`
```json
{
  "hosts": {
    "total": 10,
    "online": 8,
    "offline": 2,
    "average_quality_score": 95.5
  },
  "zones": {
    "total": 5,
    "enabled": 4,
    "disabled": 1
  },
  "sessions": {
    "total": 20,
    "active": 15,
    "inactive": 5,
    "total_players": 150,
    "average_latency_ms": 45.5
  },
  "system": {
    "postgres": "healthy",
    "redis": "healthy"
  }
}
```

### Host Statistics

**Endpoint**: `GET /api/monitoring/hosts/stats`

**Response**: `200 OK`
```json
{
  "total_online": 8,
  "quality_distribution": {
    "excellent": 5,
    "good": 2,
    "fair": 1,
    "poor": 0
  }
}
```

---

## Error Handling

### Error Response Format

```json
{
  "detail": "Error message description"
}
```

### HTTP Status Codes

- `200 OK`: Successful request
- `201 Created`: Resource created successfully
- `400 Bad Request`: Invalid request parameters
- `404 Not Found`: Resource not found
- `500 Internal Server Error`: Server error

### Common Errors

**Host Not Found**:
```json
{
  "detail": "Host not found: host-12345"
}
```

**Zone Not Found**:
```json
{
  "detail": "Zone not found: prontera"
}
```

**Session Full**:
```json
{
  "detail": "Failed to add player player-12345 to session 1"
}
```

---

## Rate Limiting

**Status**: Not yet implemented  
**Planned**: 100 requests per minute per IP

---

## Versioning

**Current Version**: 1.0.0  
**API Versioning**: Not yet implemented  
**Planned**: `/api/v1/` prefix for versioned endpoints

