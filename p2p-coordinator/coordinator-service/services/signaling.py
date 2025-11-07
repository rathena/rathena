"""
WebRTC Signaling Service

Handles WebRTC signaling for P2P connections including SDP offer/answer exchange
and ICE candidate exchange.
"""

from typing import Dict, Set, Optional
from dataclasses import dataclass
from datetime import datetime
import json
import asyncio
from loguru import logger


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
    """
    
    def __init__(self):
        # Active WebSocket connections: {peer_id: websocket}
        self.connections: Dict[str, any] = {}
        
        # Peer sessions: {session_id: Set[peer_ids]}
        self.sessions: Dict[str, Set[str]] = {}
        
        # Peer to session mapping: {peer_id: session_id}
        self.peer_sessions: Dict[str, str] = {}
        
        logger.info("SignalingService initialized")
    
    async def register_peer(self, peer_id: str, websocket: any) -> None:
        """
        Register a peer's WebSocket connection
        
        Args:
            peer_id: Unique peer identifier
            websocket: WebSocket connection
        """
        self.connections[peer_id] = websocket
        logger.info(f"Peer registered: {peer_id}, total connections: {len(self.connections)}")
    
    async def unregister_peer(self, peer_id: str) -> None:
        """
        Unregister a peer's WebSocket connection
        
        Args:
            peer_id: Unique peer identifier
        """
        if peer_id in self.connections:
            del self.connections[peer_id]
            logger.info(f"Peer unregistered: {peer_id}, remaining connections: {len(self.connections)}")
        
        # Remove from session
        if peer_id in self.peer_sessions:
            session_id = self.peer_sessions[peer_id]
            if session_id in self.sessions:
                self.sessions[session_id].discard(peer_id)
                if not self.sessions[session_id]:
                    del self.sessions[session_id]
                    logger.info(f"Session {session_id} removed (no peers left)")
            del self.peer_sessions[peer_id]
    
    async def join_session(self, peer_id: str, session_id: str) -> Set[str]:
        """
        Add peer to a session
        
        Args:
            peer_id: Unique peer identifier
            session_id: Session identifier
            
        Returns:
            Set of peer IDs in the session (excluding the joining peer)
        """
        if session_id not in self.sessions:
            self.sessions[session_id] = set()
        
        # Get existing peers before adding new one
        existing_peers = self.sessions[session_id].copy()
        
        # Add peer to session
        self.sessions[session_id].add(peer_id)
        self.peer_sessions[peer_id] = session_id
        
        logger.info(f"Peer {peer_id} joined session {session_id}, total peers: {len(self.sessions[session_id])}")
        
        return existing_peers
    
    async def leave_session(self, peer_id: str) -> Optional[str]:
        """
        Remove peer from their current session
        
        Args:
            peer_id: Unique peer identifier
            
        Returns:
            Session ID that the peer left, or None
        """
        if peer_id not in self.peer_sessions:
            return None
        
        session_id = self.peer_sessions[peer_id]
        
        if session_id in self.sessions:
            self.sessions[session_id].discard(peer_id)
            if not self.sessions[session_id]:
                del self.sessions[session_id]
                logger.info(f"Session {session_id} removed (no peers left)")
        
        del self.peer_sessions[peer_id]
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
        if session_id not in self.sessions:
            logger.warning(f"Cannot broadcast to session {session_id}: session not found")
            return 0

        sent_count = 0
        for peer_id in self.sessions[session_id]:
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

