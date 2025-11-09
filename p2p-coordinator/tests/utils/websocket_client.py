"""
WebSocket test client utilities.
"""

import json
from typing import Dict, Any, Optional, List
from contextlib import asynccontextmanager
import websockets
from loguru import logger


class WebSocketTestClient:
    """
    WebSocket test client for testing signaling functionality.
    """
    
    def __init__(self, base_url: str = "ws://localhost:8001"):
        """
        Initialize WebSocket test client.
        
        Args:
            base_url: Base WebSocket URL
        """
        self.base_url = base_url
        self.connection = None
        self.received_messages: List[Dict[str, Any]] = []
    
    @asynccontextmanager
    async def connect(
        self,
        peer_id: str,
        session_id: Optional[str] = None,
        token: Optional[str] = None,
    ):
        """
        Connect to WebSocket endpoint.
        
        Args:
            peer_id: Peer ID for connection
            session_id: Optional session ID
            token: Optional JWT token
        
        Yields:
            WebSocket connection
        """
        # Build URL with query parameters
        url = f"{self.base_url}/api/v1/signaling/ws?peer_id={peer_id}"
        if session_id:
            url += f"&session_id={session_id}"
        if token:
            url += f"&token={token}"
        
        logger.info(f"Connecting to WebSocket: {url}")
        
        try:
            async with websockets.connect(url) as websocket:
                self.connection = websocket
                yield websocket
        finally:
            self.connection = None
    
    async def send_message(self, message: Dict[str, Any]) -> None:
        """
        Send a message through WebSocket.
        
        Args:
            message: Message dictionary to send
        """
        if not self.connection:
            raise RuntimeError("WebSocket not connected")
        
        message_json = json.dumps(message)
        logger.debug(f"Sending message: {message_json}")
        await self.connection.send(message_json)
    
    async def receive_message(self, timeout: float = 5.0) -> Dict[str, Any]:
        """
        Receive a message from WebSocket.
        
        Args:
            timeout: Timeout in seconds
        
        Returns:
            Received message as dictionary
        """
        if not self.connection:
            raise RuntimeError("WebSocket not connected")
        
        try:
            message_str = await asyncio.wait_for(
                self.connection.recv(),
                timeout=timeout
            )
            message = json.loads(message_str)
            logger.debug(f"Received message: {message}")
            self.received_messages.append(message)
            return message
        except asyncio.TimeoutError:
            logger.warning(f"Timeout waiting for message after {timeout}s")
            raise
    
    async def send_join(
        self,
        session_id: str,
        peer_id: Optional[str] = None,
    ) -> None:
        """
        Send a join message.
        
        Args:
            session_id: Session ID to join
            peer_id: Optional peer ID (uses connection peer_id if not provided)
        """
        message = {
            "type": "join",
            "session_id": session_id,
        }
        if peer_id:
            message["peer_id"] = peer_id
        
        await self.send_message(message)
    
    async def send_leave(self, peer_id: Optional[str] = None) -> None:
        """
        Send a leave message.
        
        Args:
            peer_id: Optional peer ID (uses connection peer_id if not provided)
        """
        message = {"type": "leave"}
        if peer_id:
            message["peer_id"] = peer_id
        
        await self.send_message(message)
    
    async def send_offer(
        self,
        target_peer_id: str,
        sdp: str,
        peer_id: Optional[str] = None,
    ) -> None:
        """
        Send a WebRTC offer.
        
        Args:
            target_peer_id: Target peer ID
            sdp: SDP offer string
            peer_id: Optional peer ID (uses connection peer_id if not provided)
        """
        message = {
            "type": "offer",
            "target_peer_id": target_peer_id,
            "sdp": sdp,
        }
        if peer_id:
            message["peer_id"] = peer_id
        
        await self.send_message(message)
    
    async def send_answer(
        self,
        target_peer_id: str,
        sdp: str,
        peer_id: Optional[str] = None,
    ) -> None:
        """
        Send a WebRTC answer.
        
        Args:
            target_peer_id: Target peer ID
            sdp: SDP answer string
            peer_id: Optional peer ID (uses connection peer_id if not provided)
        """
        message = {
            "type": "answer",
            "target_peer_id": target_peer_id,
            "sdp": sdp,
        }
        if peer_id:
            message["peer_id"] = peer_id
        
        await self.send_message(message)
    
    async def send_ice_candidate(
        self,
        target_peer_id: str,
        candidate: str,
        peer_id: Optional[str] = None,
    ) -> None:
        """
        Send an ICE candidate.
        
        Args:
            target_peer_id: Target peer ID
            candidate: ICE candidate string
            peer_id: Optional peer ID (uses connection peer_id if not provided)
        """
        message = {
            "type": "ice-candidate",
            "target_peer_id": target_peer_id,
            "candidate": candidate,
        }
        if peer_id:
            message["peer_id"] = peer_id
        
        await self.send_message(message)
    
    def clear_received_messages(self) -> None:
        """Clear the list of received messages."""
        self.received_messages.clear()
    
    def get_received_messages(self) -> List[Dict[str, Any]]:
        """Get all received messages."""
        return self.received_messages.copy()


# Add missing import
import asyncio

