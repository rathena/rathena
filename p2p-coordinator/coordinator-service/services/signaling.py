"""
WebRTC Signaling Service

Handles WebRTC signaling for P2P connections including SDP offer/answer exchange
and ICE candidate exchange.

Uses Redis/DragonflyDB for persistent state storage to enable horizontal scaling
and recovery from coordinator restarts.
"""

from typing import Dict, Set, Optional
from dataclasses import dataclass
from datetime import datetime, timezone
import json
import asyncio
from loguru import logger
from redis.asyncio import Redis


@dataclass
class SignalingMessage:
    """WebRTC signaling message"""
    type: str  # offer, answer, ice-candidate, join, leave
    from_peer: str
    to_peer: Optional[str]
    data: dict
    timestamp: datetime


class SignalingService:
    """
    WebRTC Signaling Service

    Manages WebSocket connections and WebRTC signaling messages between peers.
    Uses Redis for persistent state storage.

    Redis Keys:
    - signaling:connections:{peer_id} -> connection metadata (hash)
    - signaling:session:{session_id}:peers -> set of peer IDs in session
    - signaling:peer:{peer_id}:session -> session ID for peer
    - signaling:pubsub:session:{session_id} -> pub/sub channel for session messages
    """

    def __init__(self, redis_client: Redis):
        # Active WebSocket connections (in-memory, not persisted)
        # WebSocket objects cannot be serialized to Redis
        self.connections: Dict[str, any] = {}

        # Redis client for persistent state
        self.redis = redis_client

        # Key prefixes
        self.CONNECTION_PREFIX = "signaling:connections:"
        self.SESSION_PEERS_PREFIX = "signaling:session:"
        self.PEER_SESSION_PREFIX = "signaling:peer:"
        self.PUBSUB_PREFIX = "signaling:pubsub:session:"

        logger.info("SignalingService initialized with Redis backend")
    
    async def register_peer(self, peer_id: str, websocket: any) -> None:
        """
        Register a peer's WebSocket connection

        Args:
            peer_id: Unique peer identifier
            websocket: WebSocket connection
        """
        # Store WebSocket in memory (cannot be serialized to Redis)
        self.connections[peer_id] = websocket

        # Store connection metadata in Redis
        connection_key = f"{self.CONNECTION_PREFIX}{peer_id}"
        await self.redis.hset(
            connection_key,
            mapping={
                "peer_id": peer_id,
                "connected_at": datetime.now(timezone.utc).isoformat(),
                "status": "connected"
            }
        )
        # Set expiration to auto-cleanup stale connections (1 hour)
        await self.redis.expire(connection_key, 3600)

        logger.info(f"Peer registered: {peer_id}, total connections: {len(self.connections)}")
    
    async def unregister_peer(self, peer_id: str) -> None:
        """
        Unregister a peer's WebSocket connection

        Args:
            peer_id: Unique peer identifier
        """
        # Remove from in-memory connections
        if peer_id in self.connections:
            del self.connections[peer_id]
            logger.info(f"Peer unregistered: {peer_id}, remaining connections: {len(self.connections)}")

        # Get session ID from Redis
        peer_session_key = f"{self.PEER_SESSION_PREFIX}{peer_id}:session"
        session_id = await self.redis.get(peer_session_key)

        if session_id:
            # Remove peer from session in Redis
            session_peers_key = f"{self.SESSION_PEERS_PREFIX}{session_id}:peers"
            await self.redis.srem(session_peers_key, peer_id)

            # Check if session is now empty
            peer_count = await self.redis.scard(session_peers_key)
            if peer_count == 0:
                await self.redis.delete(session_peers_key)
                logger.info(f"Session {session_id} removed (no peers left)")

            # Remove peer-to-session mapping
            await self.redis.delete(peer_session_key)

        # Remove connection metadata
        connection_key = f"{self.CONNECTION_PREFIX}{peer_id}"
        await self.redis.delete(connection_key)
    
    async def join_session(self, peer_id: str, session_id: str) -> Set[str]:
        """
        Add peer to a session

        Args:
            peer_id: Unique peer identifier
            session_id: Session identifier

        Returns:
            Set of peer IDs in the session (excluding the joining peer)
        """
        # Add peer to session in Redis
        session_peers_key = f"{self.SESSION_PEERS_PREFIX}{session_id}:peers"

        # Get existing peers before adding new one
        existing_peers_list = await self.redis.smembers(session_peers_key)
        existing_peers = set(existing_peers_list) if existing_peers_list else set()

        # Add peer to session
        await self.redis.sadd(session_peers_key, peer_id)

        # Set peer-to-session mapping
        peer_session_key = f"{self.PEER_SESSION_PREFIX}{peer_id}:session"
        await self.redis.set(peer_session_key, session_id)

        # Set expiration for session data (1 hour)
        await self.redis.expire(session_peers_key, 3600)
        await self.redis.expire(peer_session_key, 3600)

        logger.info(f"Peer {peer_id} joined session {session_id}, total peers: {len(existing_peers) + 1}")

        return existing_peers
    
    async def leave_session(self, peer_id: str) -> Optional[str]:
        """
        Remove peer from their current session

        Args:
            peer_id: Unique peer identifier

        Returns:
            Session ID that the peer left, or None
        """
        # Get session ID from Redis
        peer_session_key = f"{self.PEER_SESSION_PREFIX}{peer_id}:session"
        session_id = await self.redis.get(peer_session_key)

        if not session_id:
            return None

        # Remove peer from session
        session_peers_key = f"{self.SESSION_PEERS_PREFIX}{session_id}:peers"
        await self.redis.srem(session_peers_key, peer_id)

        # Check if session is now empty
        peer_count = await self.redis.scard(session_peers_key)
        if peer_count == 0:
            await self.redis.delete(session_peers_key)
            logger.info(f"Session {session_id} removed (no peers left)")

        # Remove peer-to-session mapping
        await self.redis.delete(peer_session_key)

        logger.info(f"Peer {peer_id} left session {session_id}")

        return session_id
    
    async def send_to_peer(self, peer_id: str, message: dict) -> bool:
        """
        Send message to a specific peer
        
        Args:
            peer_id: Target peer identifier
            message: Message to send
            
        Returns:
            True if message was sent, False if peer not connected
        """
        if peer_id not in self.connections:
            logger.warning(f"Cannot send message to {peer_id}: peer not connected")
            return False
        
        try:
            websocket = self.connections[peer_id]
            await websocket.send_json(message)
            logger.debug(f"Message sent to {peer_id}: {message.get('type')}")
            return True
        except Exception as e:
            logger.error(f"Failed to send message to {peer_id}: {e}")
            return False

    async def broadcast_to_session(self, session_id: str, message: dict, exclude_peer: Optional[str] = None) -> int:
        """
        Broadcast message to all peers in a session

        Args:
            session_id: Session identifier
            message: Message to broadcast
            exclude_peer: Peer ID to exclude from broadcast (optional)

        Returns:
            Number of peers that received the message
        """
        # Get peers from Redis
        session_peers_key = f"{self.SESSION_PEERS_PREFIX}{session_id}:peers"
        peers_list = await self.redis.smembers(session_peers_key)

        if not peers_list:
            logger.warning(f"Cannot broadcast to session {session_id}: no peers found")
            return 0

        sent_count = 0
        for peer_id in peers_list:
            if exclude_peer and peer_id == exclude_peer:
                continue

            if await self.send_to_peer(peer_id, message):
                sent_count += 1

        logger.debug(f"Broadcast to session {session_id}: {sent_count} peers received message")
        return sent_count

    async def handle_offer(self, from_peer: str, to_peer: str, sdp: dict) -> bool:
        """
        Handle WebRTC offer from one peer to another

        Args:
            from_peer: Peer sending the offer
            to_peer: Peer receiving the offer
            sdp: SDP offer data

        Returns:
            True if offer was delivered, False otherwise
        """
        message = {
            "type": "offer",
            "from": from_peer,
            "sdp": sdp,
            "timestamp": datetime.utcnow().isoformat(),
        }

        logger.info(f"Forwarding offer from {from_peer} to {to_peer}")
        return await self.send_to_peer(to_peer, message)

    async def handle_answer(self, from_peer: str, to_peer: str, sdp: dict) -> bool:
        """
        Handle WebRTC answer from one peer to another

        Args:
            from_peer: Peer sending the answer
            to_peer: Peer receiving the answer
            sdp: SDP answer data

        Returns:
            True if answer was delivered, False otherwise
        """
        message = {
            "type": "answer",
            "from": from_peer,
            "sdp": sdp,
            "timestamp": datetime.utcnow().isoformat(),
        }

        logger.info(f"Forwarding answer from {from_peer} to {to_peer}")
        return await self.send_to_peer(to_peer, message)

    async def handle_ice_candidate(self, from_peer: str, to_peer: str, candidate: dict) -> bool:
        """
        Handle ICE candidate from one peer to another

        Args:
            from_peer: Peer sending the ICE candidate
            to_peer: Peer receiving the ICE candidate
            candidate: ICE candidate data

        Returns:
            True if ICE candidate was delivered, False otherwise
        """
        message = {
            "type": "ice-candidate",
            "from": from_peer,
            "candidate": candidate,
            "timestamp": datetime.utcnow().isoformat(),
        }

        logger.debug(f"Forwarding ICE candidate from {from_peer} to {to_peer}")
        return await self.send_to_peer(to_peer, message)

    def get_session_peers(self, session_id: str) -> Set[str]:
        """
        Get all peers in a session

        Args:
            session_id: Session identifier

        Returns:
            Set of peer IDs in the session
        """
        return self.sessions.get(session_id, set()).copy()

    def get_peer_session(self, peer_id: str) -> Optional[str]:
        """
        Get the session ID for a peer

        Args:
            peer_id: Peer identifier

        Returns:
            Session ID or None if peer is not in a session
        """
        return self.peer_sessions.get(peer_id)

    def get_connection_count(self) -> int:
        """Get total number of active connections"""
        return len(self.connections)

    def get_session_count(self) -> int:
        """Get total number of active sessions"""
        return len(self.sessions)

