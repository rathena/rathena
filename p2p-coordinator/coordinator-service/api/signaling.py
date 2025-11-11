"""
WebRTC Signaling API Endpoints

WebSocket endpoints for WebRTC signaling.
"""

from typing import Optional
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, Query, Depends
from loguru import logger
import json
import jwt

from services.signaling import SignalingService
from config import settings
from database import db_manager


router = APIRouter(prefix="/api/v1/signaling", tags=["signaling"])

# Global signaling service instance (will be initialized on startup)
signaling_service: Optional[SignalingService] = None


async def initialize_signaling_service():
    """Initialize signaling service with Redis client"""
    global signaling_service
    redis = await db_manager.get_redis()
    signaling_service = SignalingService(redis)
    logger.info("Signaling service initialized with Redis backend")


def validate_jwt_token(token: Optional[str]) -> Optional[dict]:
    """
    Validate JWT token (optional validation)

    Args:
        token: JWT token to validate

    Returns:
        Decoded token payload if valid, None if invalid or validation disabled
    """
    # If JWT validation is disabled, skip validation
    if not settings.security.jwt_validation_enabled:
        logger.debug("JWT validation is disabled, skipping token validation")
        return None

    # If no token provided, return None (graceful degradation)
    if not token:
        logger.debug("No JWT token provided")
        return None

    try:
        # Decode and validate token
        payload = jwt.decode(
            token,
            settings.security.jwt_secret_key,
            algorithms=[settings.security.jwt_algorithm]
        )
        logger.debug(f"JWT token validated for player_id: {payload.get('player_id')}")
        return payload
    except jwt.ExpiredSignatureError:
        logger.warning("JWT token has expired")
        return None
    except jwt.InvalidTokenError as e:
        logger.warning(f"Invalid JWT token: {e}")
        return None


@router.websocket("/ws")
async def websocket_endpoint(
    websocket: WebSocket,
    peer_id: str = Query(..., description="Unique peer identifier"),
    session_id: Optional[str] = Query(None, description="Session identifier to join"),
    token: Optional[str] = Query(None, description="JWT token for authentication (optional)"),
):
    """
    WebSocket endpoint for WebRTC signaling

    Query parameters:
    - peer_id: Unique identifier for this peer (required)
    - session_id: Session to join (optional, can join later via message)
    - token: JWT token for authentication (optional, only validated if JWT_VALIDATION_ENABLED=true)

    Message types:
    - join: Join a session
    - leave: Leave current session
    - offer: Send WebRTC offer to another peer
    - answer: Send WebRTC answer to another peer
    - ice-candidate: Send ICE candidate to another peer

    **NOTE**: JWT authentication is OPTIONAL. If JWT_VALIDATION_ENABLED is false (default),
    tokens are not validated and connections are accepted without authentication.
    """
    await websocket.accept()
    logger.info(f"WebSocket connection accepted for peer: {peer_id}")

    # Validate JWT token if provided and validation is enabled
    token_payload = validate_jwt_token(token)
    if settings.security.jwt_validation_enabled and token and not token_payload:
        logger.warning(f"Invalid or expired JWT token for peer: {peer_id}")
        await websocket.send_json({
            "type": "error",
            "message": "Invalid or expired authentication token"
        })
        await websocket.close(code=1008, reason="Invalid authentication token")
        return

    if token_payload:
        logger.info(f"Peer {peer_id} authenticated as player_id: {token_payload.get('player_id')}")
    
    try:
        # Register peer
        await signaling_service.register_peer(peer_id, websocket)
        
        # Auto-join session if provided
        if session_id:
            existing_peers = await signaling_service.join_session(peer_id, session_id)
            
            # Notify peer of existing peers in session
            await websocket.send_json({
                "type": "session-joined",
                "session_id": session_id,
                "peers": list(existing_peers),
            })
            
            # Notify existing peers of new peer
            await signaling_service.broadcast_to_session(
                session_id,
                {
                    "type": "peer-joined",
                    "peer_id": peer_id,
                },
                exclude_peer=peer_id,
            )
        
        # Message handling loop
        while True:
            try:
                # Receive message from peer
                data = await websocket.receive_text()
                message = json.loads(data)
                
                message_type = message.get("type")
                logger.debug(f"Received message from {peer_id}: {message_type}")
                
                # Handle different message types
                if message_type == "join":
                    # Join a session
                    target_session_id = message.get("session_id")
                    if not target_session_id:
                        await websocket.send_json({"type": "error", "message": "session_id required"})
                        continue
                    
                    existing_peers = await signaling_service.join_session(peer_id, target_session_id)
                    
                    # Notify peer of existing peers
                    await websocket.send_json({
                        "type": "session-joined",
                        "session_id": target_session_id,
                        "peers": list(existing_peers),
                    })
                    
                    # Notify existing peers
                    await signaling_service.broadcast_to_session(
                        target_session_id,
                        {
                            "type": "peer-joined",
                            "peer_id": peer_id,
                        },
                        exclude_peer=peer_id,
                    )
                
                elif message_type == "leave":
                    # Leave current session
                    left_session_id = await signaling_service.leave_session(peer_id)
                    if left_session_id:
                        # Notify remaining peers
                        await signaling_service.broadcast_to_session(
                            left_session_id,
                            {
                                "type": "peer-left",
                                "peer_id": peer_id,
                            },
                        )
                        
                        await websocket.send_json({
                            "type": "session-left",
                            "session_id": left_session_id,
                        })
                
                elif message_type == "offer":
                    # Forward WebRTC offer to target peer
                    to_peer = message.get("to")
                    sdp = message.get("sdp")
                    
                    if not to_peer or not sdp:
                        await websocket.send_json({"type": "error", "message": "to and sdp required"})
                        continue
                    
                    success = await signaling_service.handle_offer(peer_id, to_peer, sdp)
                    if not success:
                        await websocket.send_json({
                            "type": "error",
                            "message": f"Failed to deliver offer to {to_peer}",
                        })
                
                elif message_type == "answer":
                    # Forward WebRTC answer to target peer
                    to_peer = message.get("to")
                    sdp = message.get("sdp")
                    
                    if not to_peer or not sdp:
                        await websocket.send_json({"type": "error", "message": "to and sdp required"})
                        continue
                    
                    success = await signaling_service.handle_answer(peer_id, to_peer, sdp)
                    if not success:
                        await websocket.send_json({
                            "type": "error",
                            "message": f"Failed to deliver answer to {to_peer}",
                        })
                
                elif message_type == "ice-candidate":
                    # Forward ICE candidate to target peer
                    to_peer = message.get("to")
                    candidate = message.get("candidate")
                    
                    if not to_peer or not candidate:
                        await websocket.send_json({"type": "error", "message": "to and candidate required"})
                        continue
                    
                    success = await signaling_service.handle_ice_candidate(peer_id, to_peer, candidate)
                    if not success:
                        logger.warning(f"Failed to deliver ICE candidate to {to_peer}")
                
                else:
                    logger.warning(f"Unknown message type from {peer_id}: {message_type}")
                    await websocket.send_json({"type": "error", "message": f"Unknown message type: {message_type}"})
            
            except json.JSONDecodeError as e:
                logger.error(f"Invalid JSON from {peer_id}: {e}")
                await websocket.send_json({"type": "error", "message": "Invalid JSON"})
    
    except WebSocketDisconnect:
        logger.info(f"WebSocket disconnected for peer: {peer_id}")
    except Exception as e:
        logger.error(f"WebSocket error for peer {peer_id}: {e}")
    finally:
        # Cleanup
        left_session_id = await signaling_service.leave_session(peer_id)
        if left_session_id:
            # Notify remaining peers
            await signaling_service.broadcast_to_session(
                left_session_id,
                {
                    "type": "peer-left",
                    "peer_id": peer_id,
                },
            )
        
        await signaling_service.unregister_peer(peer_id)
        logger.info(f"Peer {peer_id} cleanup complete")


@router.get("/stats")
async def get_signaling_stats():
    """Get signaling service statistics"""
    return {
        "active_connections": signaling_service.get_connection_count(),
        "active_sessions": signaling_service.get_session_count(),
    }

