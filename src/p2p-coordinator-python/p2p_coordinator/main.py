"""P2P Coordinator — FastAPI server for WARP-p2p-client DLL integration.

Provides:
- JWT authentication (/api/v1/auth/token, /api/v1/auth/refresh)
- WebSocket signaling (/api/v1/signaling/ws) for SDP/ICE exchange
- Session & zone management
- Health monitoring
- Integration with rathena-AI-world ML inference service
"""

import os
import json
import uuid
import time
import asyncio
import logging
from datetime import datetime, timedelta, timezone
from typing import Optional

import jwt as pyjwt
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from pydantic import BaseModel

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

JWT_SECRET = os.environ.get("P2P_JWT_SECRET", "change-me-in-production-use-a-real-secret")
JWT_ALGORITHM = "HS256"
JWT_EXPIRY_HOURS = 24

COORDINATOR_HOST = os.environ.get("P2P_COORDINATOR_HOST", "0.0.0.0")
COORDINATOR_PORT = int(os.environ.get("P2P_COORDINATOR_PORT", "8001"))

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
# In-memory stores
# ---------------------------------------------------------------------------

_peers: dict[str, dict] = {}           # peer_id -> {token, session_id, zone, connected_at}
_sessions: dict[str, dict] = {}        # session_id -> {host_id, zone_id, peers[], created_at}
_zones: dict[str, dict] = {}           # zone_id -> {session_ids[], max_peers}
_ws_connections: dict[str, WebSocket] = {}  # peer_id -> WebSocket (active signaling connections)
_start_time = time.time()

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
    _peers[peer_id] = {
        "token": token,
        "session_id": None,
        "zone": None,
        "connected_at": datetime.now(timezone.utc).isoformat(),
    }
    logger.info("Peer authenticated: %s", peer_id)
    return AuthResponse(token=token, expires_in=JWT_EXPIRY_HOURS * 3600)


@app.post("/api/v1/auth/refresh", response_model=RefreshResponse)
async def auth_refresh(payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    token = _create_token(peer_id)
    if peer_id in _peers:
        _peers[peer_id]["token"] = token
    logger.info("Token refreshed for peer: %s", peer_id)
    return RefreshResponse(token=token)


@app.get("/api/v1/health", response_model=HealthResponse)
async def health():
    return HealthResponse(
        status="ok",
        peers=len(_peers),
        sessions=len(_sessions),
        zones=len(_zones),
        uptime_seconds=time.time() - _start_time,
    )


@app.get("/api/v1/peers")
async def list_peers(_=Depends(_get_current_peer)):
    return {
        pid: {
            "session_id": info.get("session_id"),
            "zone": info.get("zone"),
            "connected_at": info.get("connected_at"),
        }
        for pid, info in _peers.items()
    }


@app.post("/api/v1/sessions/create")
async def create_session(zone_id: str, payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    session_id = str(uuid.uuid4())
    _sessions[session_id] = {
        "host_id": peer_id,
        "zone_id": zone_id,
        "peers": [peer_id],
        "created_at": datetime.now(timezone.utc).isoformat(),
    }
    if peer_id in _peers:
        _peers[peer_id]["session_id"] = session_id
        _peers[peer_id]["zone"] = zone_id
    if zone_id not in _zones:
        _zones[zone_id] = {"session_ids": [], "max_peers": 50}
    _zones[zone_id]["session_ids"].append(session_id)
    logger.info("Session %s created by %s in zone %s", session_id, peer_id, zone_id)
    return {"session_id": session_id, "status": "ok"}


@app.post("/api/v1/sessions/join")
async def join_session(session_id: str, payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    if session_id not in _sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    session = _sessions[session_id]
    if peer_id not in session["peers"]:
        session["peers"].append(peer_id)
    if peer_id in _peers:
        _peers[peer_id]["session_id"] = session_id
        _peers[peer_id]["zone"] = session["zone_id"]
    logger.info("Peer %s joined session %s", peer_id, session_id)
    return {"session_id": session_id, "peers": session["peers"], "status": "ok"}


@app.post("/api/v1/sessions/leave")
async def leave_session(payload: dict = Depends(_get_current_peer)):
    peer_id = payload["peer_id"]
    if peer_id in _peers:
        sid = _peers[peer_id].get("session_id")
        if sid and sid in _sessions:
            _sessions[sid]["peers"] = [p for p in _sessions[sid]["peers"] if p != peer_id]
        _peers[peer_id]["session_id"] = None
    logger.info("Peer %s left session", peer_id)
    return {"status": "ok"}


@app.get("/api/v1/sessions")
async def list_sessions(_=Depends(_get_current_peer)):
    return {
        sid: {
            "host_id": s["host_id"],
            "zone_id": s["zone_id"],
            "peer_count": len(s["peers"]),
            "peers": s["peers"],
            "created_at": s["created_at"],
        }
        for sid, s in _sessions.items()
    }


@app.get("/api/v1/zones")
async def list_zones(_=Depends(_get_current_peer)):
    return {
        zid: {
            "session_count": len(z["session_ids"]),
            "max_peers": z["max_peers"],
        }
        for zid, z in _zones.items()
    }

# ---------------------------------------------------------------------------
# WebSocket signaling — direct peer-to-peer relay
# ---------------------------------------------------------------------------

async def _send_to_peer(target_peer_id: str, message: dict) -> bool:
    """Send a JSON message to a peer's active WebSocket connection."""
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
    """WebSocket signaling for SDP/ICE exchange between peers.

    Protocol:
      1. Client sends:  {"type":"auth","token":"<jwt>"}
         Server sends:  {"type":"auth_ok","peer_id":"<id>"}
      2. Client sends:  {"type":"join","session_id":"<id>"}
         Server sends:  {"type":"joined","session_id":"<id>"}
      3. Client sends:  {"type":"offer","peer_id":"<target>","sdp":"..."}
         Server relays: {"type":"offer","from":"<sender>","sdp":"..."}
      4. Client sends:  {"type":"answer","peer_id":"<target>","sdp":"..."}
         Server relays: {"type":"answer","from":"<sender>","sdp":"..."}
      5. Client sends:  {"type":"ice_candidate","peer_id":"<target>","candidate":"..."}
         Server relays: {"type":"ice_candidate","from":"<sender>","candidate":"..."}
      6. Client sends:  {"type":"leave"}
    """
    await websocket.accept()
    peer_id: str = ""  # Will be set after auth

    try:
        # --- Step 1: Auth ---
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
        logger.info("WebSocket connected: peer=%s", peer_id)
        await websocket.send_json({"type": "auth_ok", "peer_id": peer_id})

        # --- Step 2+: Message loop ---
        while True:
            raw = await websocket.receive_text()
            msg = json.loads(raw)
            msg_type = msg.get("type", "")

            if msg_type == "join":
                session_id = msg.get("session_id", "")
                if session_id not in _sessions:
                    await websocket.send_json({"type": "error", "message": "Session not found"})
                    continue
                if peer_id in _peers:
                    _peers[peer_id]["session_id"] = session_id
                logger.info("Peer %s joined signaling room %s", peer_id, session_id)
                await websocket.send_json({"type": "joined", "session_id": session_id})

                # Notify other peers in the session
                session = _sessions.get(session_id, {})
                for other_pid in session.get("peers", []):
                    if other_pid != peer_id:
                        await _send_to_peer(other_pid, {
                            "type": "peer_joined",
                            "peer_id": peer_id,
                            "session_id": session_id,
                        })

            elif msg_type == "leave":
                # Notify other peers
                if peer_id:
                    for pid, ws in list(_ws_connections.items()):
                        if pid != peer_id:
                            await _send_to_peer(pid, {
                                "type": "peer_left",
                                "peer_id": peer_id,
                            })
                break

            elif msg_type in ("offer", "answer", "ice_candidate"):
                target_peer = msg.get("peer_id", "")
                if not target_peer:
                    await websocket.send_json({"type": "error", "message": "Missing peer_id"})
                    continue

                envelope = {
                    "type": msg_type,
                    "from": peer_id,
                }
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
            if peer_id in _peers:
                _peers[peer_id]["session_id"] = None
            # Notify other peers
            for pid in list(_ws_connections.keys()):
                if pid != peer_id:
                    await _send_to_peer(pid, {
                        "type": "peer_left",
                        "peer_id": peer_id,
                    })

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main():
    logger.info("Starting P2P Coordinator on %s:%s", COORDINATOR_HOST, COORDINATOR_PORT)
    uvicorn.run(
        "p2p_coordinator.main:app",
        host=COORDINATOR_HOST,
        port=COORDINATOR_PORT,
        log_level=LOG_LEVEL.lower(),
        reload=False,
    )

if __name__ == "__main__":
    main()
