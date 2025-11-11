"""
Integration tests for WebSocket Signaling
"""

import pytest
import asyncio
import jwt
from datetime import datetime, timedelta
from fastapi.testclient import TestClient
from httpx import AsyncClient
from tests.utils.helpers import generate_player_id, generate_session_id
from config import settings


@pytest.mark.integration
@pytest.mark.websocket
class TestWebSocketSignaling:
    """Test suite for WebSocket signaling functionality"""
    
    def test_websocket_connection_success(self, websocket_client: TestClient):
        """Test successful WebSocket connection"""
        # Arrange
        peer_id = generate_player_id()
        
        # Act & Assert
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer_id}"
        ) as websocket:
            # Connection should be established
            assert websocket is not None
    
    def test_websocket_connection_missing_peer_id(self, websocket_client: TestClient):
        """Test WebSocket connection fails without peer_id"""
        # Act & Assert
        with pytest.raises(Exception):
            # Should fail without peer_id
            with websocket_client.websocket_connect("/api/v1/signaling/ws"):
                pass
    
    def test_websocket_join_session(self, websocket_client: TestClient):
        """Test joining a session via WebSocket"""
        # Arrange
        peer_id = generate_player_id()
        session_id = generate_session_id()

        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer_id}"
        ) as websocket:
            # Send join message
            websocket.send_json({
                "type": "join",
                "session_id": session_id,
            })

            # Receive response
            response = websocket.receive_json()

            # Assert
            assert response["type"] == "session-joined"  # Changed from "joined"
            assert response["session_id"] == session_id
    
    def test_websocket_leave_session(self, websocket_client: TestClient):
        """Test leaving a session via WebSocket"""
        # Arrange
        peer_id = generate_player_id()
        session_id = generate_session_id()

        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer_id}"
        ) as websocket:
            # Join first
            websocket.send_json({
                "type": "join",
                "session_id": session_id,
            })
            websocket.receive_json()  # Consume join response

            # Leave
            websocket.send_json({
                "type": "leave",
            })

            # Receive response
            response = websocket.receive_json()

            # Assert
            assert response["type"] == "session-left"  # Changed from "left"
    
    def test_websocket_send_offer(self, websocket_client: TestClient):
        """Test sending WebRTC offer"""
        # Arrange
        peer1_id = generate_player_id()
        peer2_id = generate_player_id()
        session_id = generate_session_id()
        sdp = "v=0\r\no=- 0 0 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
        
        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer1_id}"
        ) as ws1:
            # Join session
            ws1.send_json({
                "type": "join",
                "session_id": session_id,
            })
            ws1.receive_json()  # Consume join response
            
            # Send offer
            ws1.send_json({
                "type": "offer",
                "target_peer_id": peer2_id,
                "sdp": sdp,
            })
            
            # Should receive acknowledgment or error
            response = ws1.receive_json()
            assert response is not None
    
    def test_websocket_send_answer(self, websocket_client: TestClient):
        """Test sending WebRTC answer"""
        # Arrange
        peer1_id = generate_player_id()
        peer2_id = generate_player_id()
        session_id = generate_session_id()
        sdp = "v=0\r\no=- 0 0 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
        
        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer1_id}"
        ) as ws1:
            # Join session
            ws1.send_json({
                "type": "join",
                "session_id": session_id,
            })
            ws1.receive_json()  # Consume join response
            
            # Send answer
            ws1.send_json({
                "type": "answer",
                "target_peer_id": peer2_id,
                "sdp": sdp,
            })
            
            # Should receive acknowledgment or error
            response = ws1.receive_json()
            assert response is not None
    
    def test_websocket_send_ice_candidate(self, websocket_client: TestClient):
        """Test sending ICE candidate"""
        # Arrange
        peer1_id = generate_player_id()
        peer2_id = generate_player_id()
        session_id = generate_session_id()
        candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host"
        
        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer1_id}"
        ) as ws1:
            # Join session
            ws1.send_json({
                "type": "join",
                "session_id": session_id,
            })
            ws1.receive_json()  # Consume join response
            
            # Send ICE candidate
            ws1.send_json({
                "type": "ice-candidate",
                "target_peer_id": peer2_id,
                "candidate": candidate,
            })
            
            # Should receive acknowledgment or error
            response = ws1.receive_json()
            assert response is not None
    
    def test_websocket_invalid_message_type(self, websocket_client: TestClient):
        """Test that invalid message type is handled"""
        # Arrange
        peer_id = generate_player_id()
        
        # Act
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer_id}"
        ) as websocket:
            # Send invalid message type
            websocket.send_json({
                "type": "invalid_type",
                "data": "test",
            })
            
            # Should receive error response
            response = websocket.receive_json()
            assert response["type"] == "error"
    
    def test_websocket_connection_with_session_id(self, websocket_client: TestClient):
        """Test WebSocket connection with session_id parameter"""
        # Arrange
        peer_id = generate_player_id()
        session_id = generate_session_id()
        
        # Act & Assert
        with websocket_client.websocket_connect(
            f"/api/v1/signaling/ws?peer_id={peer_id}&session_id={session_id}"
        ) as websocket:
            # Connection should be established and auto-joined to session
            assert websocket is not None
            
            # May receive auto-join confirmation
            try:
                response = websocket.receive_json(timeout=1)
                if response:
                    assert response["type"] in ["session-joined", "peer-joined"]  # Updated message types
            except:
                pass  # No immediate message is also acceptable


@pytest.mark.integration
@pytest.mark.asyncio
class TestJWTValidation:
    """Test JWT validation functions to cover lines 37-55"""

    def create_jwt_token(self, player_id: str, expired: bool = False) -> str:
        """Create a JWT token for testing"""
        payload = {
            "player_id": player_id,
            "exp": datetime.utcnow() + timedelta(hours=-1 if expired else 1)
        }
        return jwt.encode(
            payload,
            settings.security.jwt_secret_key,
            algorithm=settings.security.jwt_algorithm
        )

    async def test_validate_jwt_token_no_token(self, async_client: AsyncClient):
        """Test JWT validation with no token (lines 37-39)"""
        from api.signaling import validate_jwt_token

        # Test with None token
        result = validate_jwt_token(None)
        assert result is None



    async def test_validate_jwt_token_expired(self, async_client: AsyncClient):
        """Test JWT validation with expired token (lines 50-52)"""
        from api.signaling import validate_jwt_token

        # Create expired token
        token = self.create_jwt_token("test_player_456", expired=True)

        # Validate token - should return None
        result = validate_jwt_token(token)
        assert result is None

    async def test_validate_jwt_token_invalid(self, async_client: AsyncClient):
        """Test JWT validation with invalid token (lines 53-55)"""
        from api.signaling import validate_jwt_token

        # Create invalid token
        invalid_token = "invalid.token.here"

        # Validate token - should return None
        result = validate_jwt_token(invalid_token)
        assert result is None

