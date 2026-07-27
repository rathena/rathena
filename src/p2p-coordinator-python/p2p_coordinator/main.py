"""P2P Coordinator — FastAPI server for WARP-p2p-client DLL integration.

Backed by DragonflyDB (Redis-compatible) for persistent state.
Provides:
- JWT authentication (/api/v1/auth/token, /api/v1/auth/refresh)
- WebSocket signaling (/api/v1/signaling/ws) for SDP/ICE exchange
- Session & zone management (DragonflyDB persisted)
- Stale peer cleanup (heartbeat TTL)
- Health monitoring
"""

import os
import json
import uuid
import time
import asyncio
import logging
import secrets
from datetime import datetime, timedelta, timezone
from typing import Optional

import jwt as pyjwt
import uvicorn
import redis.asyncio as aioredis
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from pydantic import BaseModel

# ---------------------------------------------------------------------------
# Configuration — all from env, zero hardcoded paths
# ---------------------------------------------------------------------------

JWT_SECRET = os.environ.get("P2P_JWT_SECRET", "")
if not JWT_SECRET:
    JWT_SECRET = secrets.token_hex(32)
    logging.warning("P2P_JWT_SECRET not set — generated ephemeral secret. "
                    "Set it in env for persistent tokens across restarts.")

JWT_ALGORITHM = "HS256"
JWT_EXPIRY_HOURS = 24
PEER_TTL_SECONDS = 90  # peers expire if no heartbeat for 90s

COORDINATOR_HOST = os.environ.get("P2P_COORDINATOR_HOST", "0.0.0.0")
COORDINATOR_PORT = int(os.environ.get("P2P_COORDINATOR_PORT", "8001"))
REDIS_URL = os.environ.get("P2P_REDIS_URL", "redis://localhost:6379/0")
LOG_LEVEL = os.environ.get("P2P_LOG_LEVEL", "INFO")

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------

logging.basicConfig(
    level=getattr(logging, LOG_LEVEL.upper(), logging.INFO),
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)
logger = logging.getLogger("p2p_coordinator")

# ---------------------------------------------------------------------------
# FastAPI app
# ---------------------------------------------------------------------------

app = FastAPI(title="P2P Coordinator", version="1.0.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)
security = HTTPBearer(auto_error=False)

# ---------------------------------------------------------------------------
# DragonflyDB / Redis client
# ---------------------------------------------------------------------------

_redis: Optional[aioredis.Redis] = None
_ws_connections: dict[str, WebSocket] = {}  # peer_id -> WebSocket (ephemeral, not persisted)
_start_time = time.time()

# Redis key prefixes
_K_PEER = "p2p:peer:"         # hash: {token, session_id, zone, connected_at}
_K_SESSION = "p2p:session:"   # hash: {host_id, zone_id, peers_json, created_at}
_K_ZONE = "p2p:zone:"         # hash: {session_ids_json, max_peers}
_K_PEER_SET = "p2p:peers"     # set of all peer_ids
_K_SESSION_SET = "p2p:sessions"  # set of all session_ids
_K_ZONE_SET = "p2p:zones"     # set of all zone_ids


async def _get_redis() -> aioredis.Redis:
    global _redis
    if _redis is None:
        _redis = aioredis.from_url(REDIS_URL, decode_responses=True)
        await _redis.ping()
        logger.info("Connected to DragonflyDB at %s", REDIS_URL)
    return _redis


@app.on_event("startup")
async def startup():
    await _get_redis()
    # Start stale peer cleanup background task
    asyncio.create_task(_stale_peer_cleanup_loop())


@app.on_event("shutdown")
async def shutdown():
    global _redis
    if _redis:
        await _redis.aclose()
        _redis = None


# ---------------------------------------------------------------------------
# Stale peer cleanup — runs every 30s
# ---------------------------------------------------------------------------

async def _stale_peer_cleanup_loop():
    """Remove peers whose TTL has expired (crashed without sending 'leave')."""
    while True:
        try:
            await asyncio.sleep(30)
            r = await _get_redis()
            now = time.time()
            peer_ids = await r.smembers(_K_PEER_SET)
            for pid in peer_ids:
                ttl = await r.ttl(f"{_K_PEER}{pid}")
                if ttl == -2:  # key doesn't exist (expired)
                    await r.srem(_K_PEER_SET, pid)
                    logger.info("Cleaned up expired peer: %s", pid)
        except Exception as e:
            logger.warning("Stale peer cleanup error: %s", e)

# ---------------------------------------------------------------------------
# Models
# ---------------------------------------------------------------------------

class AuthRequest(BaseModel):
    peer_id: str
    client_version: str = "1.0.0"
    timestamp: Optional[int] = None

class AuthResponse(BaseModel):
    token: str
    expires_in: int

class RefreshResponse(BaseModel):
    token: str

class HealthResponse(BaseModel):
    status: str
    peers: int
    sessions: int
    zones: int
    uptime_seconds: float

# ---------------------------------------------------------------------------
# JWT helpers
# ---------------------------------------------------------------------------

def _create_token(peer_id: str) -> str:
    now = datetime.now(timezone.utc)
    payload = {
        "peer_id": peer_id,
        "iat": now,
        "exp": now + timedelta(hours=JWT_EXPIRY_HOURS),
    }
    return pyjwt.encode(payload, JWT_SECRET, algorithm=JWT_ALGORITHM)


def _verify_token(token: str) -> Optional[dict]:
    try:
        return pyjwt.decode(token, JWT_SECRET, algorithms=[JWT_ALGORITHM])
    except pyjwt.ExpiredSignatureError:
        return None
    except pyjwt.InvalidTokenError:
        return None


async def _get_current_peer(
    credentials: Optional[HTTPAuthorizationCredentials] = Depends(security),
) -> dict:
    if credentials is None:
        raise HTTPException(status_code=401, detail="Missing authorization header")
    payload = _verify_token(credentials.credentials)
    if payload is None:
        raise HTTPException(status_code=401, detail="Invalid or expired token")
    return payload

# ---------------------------------------------------------------------------
# REST endpoints
# ---------------------------------------------------------------------------

@app.post("/api/v1/auth/token", response_model=AuthResponse)
async def auth_token(req: AuthRequest):
    peer_id = req.peer_id.strip()
    if not peer_id:
        raise HTTPException(status_code=400, detail="peer_id is required")

    token = _create_token(peer_id)
    r = await _get_redis()
    key = f"{_K_PEER}{peer_id}"
    await r.hset(key, mapping={
        "token": token,
        "session_id": "",
        "zone": "",
        "connected_at": datetime.now(timezone.utc).isoformat(),
    })
    await r.expire(key, PEER_TTL_SECONDS)
    await r.sadd(_K_PEER_SET, peer_id)
    logger.info("Peer authenticated: %s", peer_id)
    return AuthResponse(token=token, expires_in=JWT_EXPIRY_HOURS * 3600)


@app.post("/api/v1/auth/refresh", response_model=RefreshResponse)
async def auth_refresh(payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    token = _create_token(peer_id)
    r = await _get_redis()
    key = f"{_K_PEER}{peer_id}"
    await r.hset(key, "token", token)
    await r.expire(key, PEER_TTL_SECONDS)
    logger.info("Token refreshed for peer: %s", peer_id)
    return RefreshResponse(token=token)


@app.get("/api/v1/health", response_model=HealthResponse)
async def health():
    r = await _get_redis()
    peers = await r.scard(_K_PEER_SET)
    sessions = await r.scard(_K_SESSION_SET)
    zones = await r.scard(_K_ZONE_SET)
    return HealthResponse(
        status="ok",
        peers=peers,
        sessions=sessions,
        zones=zones,
        uptime_seconds=time.time() - _start_time,
    )


@app.get("/api/v1/peers")
async def list_peers(_=Depends(_get_current_peer)):
    r = await _get_redis()
    peer_ids = await r.smembers(_K_PEER_SET)
    result = {}
    for pid in peer_ids:
        data = await r.hgetall(f"{_K_PEER}{pid}")
        if data:
            result[pid] = {
                "session_id": data.get("session_id", ""),
                "zone": data.get("zone", ""),
                "connected_at": data.get("connected_at", ""),
            }
    return result


@app.post("/api/v1/sessions/create")
async def create_session(zone_id: str, payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    session_id = str(uuid.uuid4())
    r = await _get_redis()

    # Store session
    skey = f"{_K_SESSION}{session_id}"
    await r.hset(skey, mapping={
        "host_id": peer_id,
        "zone_id": zone_id,
        "peers": json.dumps([peer_id]),
        "created_at": datetime.now(timezone.utc).isoformat(),
    })
    await r.sadd(_K_SESSION_SET, session_id)

    # Update peer
    pkey = f"{_K_PEER}{peer_id}"
    await r.hset(pkey, "session_id", session_id)
    await r.hset(pkey, "zone", zone_id)
    await r.expire(pkey, PEER_TTL_SECONDS)

    # Update zone
    zkey = f"{_K_ZONE}{zone_id}"
    exists = await r.exists(zkey)
    if not exists:
        await r.hset(zkey, mapping={
            "session_ids": json.dumps([session_id]),
            "max_peers": "50",
        })
        await r.sadd(_K_ZONE_SET, zone_id)
    else:
        raw = await r.hget(zkey, "session_ids")
        ids = json.loads(raw) if raw else []
        ids.append(session_id)
        await r.hset(zkey, "session_ids", json.dumps(ids))

    logger.info("Session %s created by %s in zone %s", session_id, peer_id, zone_id)
    return {"session_id": session_id, "status": "ok"}


@app.post("/api/v1/sessions/join")
async def join_session(session_id: str, payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    r = await _get_redis()
    skey = f"{_K_SESSION}{session_id}"

    if not await r.exists(skey):
        raise HTTPException(status_code=404, detail="Session not found")

    raw_peers = await r.hget(skey, "peers")
    peers = json.loads(raw_peers) if raw_peers else []
    if peer_id not in peers:
        peers.append(peer_id)
        await r.hset(skey, "peers", json.dumps(peers))

    zone_id = await r.hget(skey, "zone_id") or ""
    pkey = f"{_K_PEER}{peer_id}"
    await r.hset(pkey, "session_id", session_id)
    await r.hset(pkey, "zone", zone_id)
    await r.expire(pkey, PEER_TTL_SECONDS)

    logger.info("Peer %s joined session %s", peer_id, session_id)
    return {"session_id": session_id, "peers": peers, "status": "ok"}


@app.post("/api/v1/sessions/leave")
async def leave_session(payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    r = await _get_redis()
    pkey = f"{_K_PEER}{peer_id}"

    sid = await r.hget(pkey, "session_id")
    if sid:
        skey = f"{_K_SESSION}{sid}"
        raw_peers = await r.hget(skey, "peers")
        peers = json.loads(raw_peers) if raw_peers else []
        peers = [p for p in peers if p != peer_id]
        await r.hset(skey, "peers", json.dumps(peers))

    await r.hset(pkey, "session_id", "")
    await r.expire(pkey, PEER_TTL_SECONDS)
    logger.info("Peer %s left session", peer_id)
    return {"status": "ok"}


@app.get("/api/v1/sessions")
async def list_sessions(_=Depends(_get_current_peer)):
    r = await _get_redis()
    session_ids = await r.smembers(_K_SESSION_SET)
    result = {}
    for sid in session_ids:
        data = await r.hgetall(f"{_K_SESSION}{sid}")
        if data:
            peers = json.loads(data.get("peers", "[]"))
            result[sid] = {
                "host_id": data.get("host_id", ""),
                "zone_id": data.get("zone_id", ""),
                "peer_count": len(peers),
                "peers": peers,
                "created_at": data.get("created_at", ""),
            }
    return result


@app.get("/api/v1/zones")
async def list_zones(_=Depends(_get_current_peer)):
    r = await _get_redis()
    zone_ids = await r.smembers(_K_ZONE_SET)
    result = {}
    for zid in zone_ids:
        data = await r.hgetall(f"{_K_ZONE}{zid}")
        if data:
            session_ids = json.loads(data.get("session_ids", "[]"))
            result[zid] = {
                "session_count": len(session_ids),
                "max_peers": int(data.get("max_peers", 50)),
            }
    return result

# ---------------------------------------------------------------------------
# WebSocket signaling — direct peer-to-peer relay
# ---------------------------------------------------------------------------

async def _send_to_peer(target_peer_id: str, message: dict) -> bool:
    ws = _ws_connections.get(target_peer_id)
    if ws is None:
        return False
    try:
        await ws.send_json(message)
        return True
    except Exception:
        _ws_connections.pop(target_peer_id, None)
        return False


@app.websocket("/api/v1/signaling/ws")
async def signaling_ws(websocket: WebSocket):
    await websocket.accept()
    peer_id: str = ""

    try:
        # --- Auth ---
        raw = await websocket.receive_text()
        msg = json.loads(raw)
        if msg.get("type") != "auth":
            await websocket.send_json({"type": "error", "message": "First message must be auth"})
            await websocket.close()
            return

        token = msg.get("token", "")
        payload = _verify_token(token)
        if payload is None:
            await websocket.send_json({"type": "error", "message": "Invalid or expired token"})
            await websocket.close()
            return

        peer_id = str(payload["peer_id"])
        _ws_connections[peer_id] = websocket
        # Refresh TTL on DragonflyDB
        r = await _get_redis()
        await r.expire(f"{_K_PEER}{peer_id}", PEER_TTL_SECONDS)
        logger.info("WebSocket connected: peer=%s", peer_id)
        await websocket.send_json({"type": "auth_ok", "peer_id": peer_id})

        # --- Message loop ---
        while True:
            raw = await websocket.receive_text()
            msg = json.loads(raw)
            msg_type = msg.get("type", "")

            if msg_type == "join":
                session_id = msg.get("session_id", "")
                skey = f"{_K_SESSION}{session_id}"
                if not await r.exists(skey):
                    await websocket.send_json({"type": "error", "message": "Session not found"})
                    continue
                await r.hset(f"{_K_PEER}{peer_id}", "session_id", session_id)
                await r.expire(f"{_K_PEER}{peer_id}", PEER_TTL_SECONDS)
                logger.info("Peer %s joined signaling room %s", peer_id, session_id)
                await websocket.send_json({"type": "joined", "session_id": session_id})

                # Notify other peers in the session
                raw_peers = await r.hget(skey, "peers")
                peers = json.loads(raw_peers) if raw_peers else []
                for other_pid in peers:
                    if other_pid != peer_id:
                        await _send_to_peer(other_pid, {
                            "type": "peer_joined",
                            "peer_id": peer_id,
                            "session_id": session_id,
                        })

            elif msg_type == "leave":
                for pid in list(_ws_connections.keys()):
                    if pid != peer_id:
                        await _send_to_peer(pid, {"type": "peer_left", "peer_id": peer_id})
                break

            elif msg_type in ("offer", "answer", "ice_candidate"):
                target_peer = msg.get("peer_id", "")
                if not target_peer:
                    await websocket.send_json({"type": "error", "message": "Missing peer_id"})
                    continue

                envelope = {"type": msg_type, "from": peer_id}
                if "sdp" in msg:
                    envelope["sdp"] = msg["sdp"]
                if "candidate" in msg:
                    envelope["candidate"] = msg["candidate"]

                relayed = await _send_to_peer(target_peer, envelope)
                if relayed:
                    logger.debug("Relayed %s from %s to %s", msg_type, peer_id, target_peer)
                else:
                    logger.warning("Failed to relay %s to %s (not connected)", msg_type, target_peer)
                    await websocket.send_json({
                        "type": "error",
                        "message": f"Peer {target_peer} is not connected",
                    })
            else:
                await websocket.send_json({"type": "error", "message": f"Unknown type: {msg_type}"})

    except WebSocketDisconnect:
        logger.info("WebSocket disconnected: peer=%s", peer_id)
    except Exception as e:
        logger.error("WebSocket error for peer %s: %s", peer_id, e)
    finally:
        if peer_id:
            _ws_connections.pop(peer_id, None)
            for pid in list(_ws_connections.keys()):
                if pid != peer_id:
                    await _send_to_peer(pid, {"type": "peer_left", "peer_id": peer_id})

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    logger.info("Starting P2P Coordinator on %s:%s", COORDINATOR_HOST, COORDINATOR_PORT)
    logger.info("Using DragonflyDB at %s", REDIS_URL)
    uvicorn.run(
        "p2p_coordinator.main:app",
        host=COORDINATOR_HOST,
        port=COORDINATOR_PORT,
        log_level=LOG_LEVEL.lower(),
        reload=False,
    )

if __name__ == "__main__":
    main()
