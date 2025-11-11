"""
Unit tests for SignalingService

Tests the SignalingService for WebRTC signaling operations.
"""

import pytest
from unittest.mock import AsyncMock, MagicMock

from services.signaling import SignalingService


@pytest.mark.unit
@pytest.mark.asyncio
class TestSignalingService:
    """Test suite for SignalingService"""
    
    @pytest.fixture
    def service(self):
        """Create SignalingService instance"""
        return SignalingService()
    
    @pytest.fixture
    def mock_websocket(self):
        """Create mock WebSocket"""
        ws = AsyncMock()
        ws.send_json = AsyncMock()
        return ws
    
    async def test_register_peer(self, service, mock_websocket):
        """Test registering a peer"""
        # Act
        await service.register_peer("peer_001", mock_websocket)
        
        # Assert
        assert "peer_001" in service.connections
        assert service.connections["peer_001"] == mock_websocket
        assert service.get_connection_count() == 1
    
    async def test_unregister_peer(self, service, mock_websocket):
        """Test unregistering a peer"""
        # Arrange
        await service.register_peer("peer_002", mock_websocket)
        
        # Act
        await service.unregister_peer("peer_002")
        
        # Assert
        assert "peer_002" not in service.connections
        assert service.get_connection_count() == 0
    
    async def test_join_session(self, service, mock_websocket):
        """Test peer joining a session"""
        # Arrange
        await service.register_peer("peer_003", mock_websocket)
        
        # Act
        existing_peers = await service.join_session("peer_003", "session_001")
        
        # Assert
        assert len(existing_peers) == 0  # First peer in session
        assert "session_001" in service.sessions
        assert "peer_003" in service.sessions["session_001"]
        assert service.get_peer_session("peer_003") == "session_001"
        assert service.get_session_count() == 1
    
    async def test_join_session_with_existing_peers(self, service, mock_websocket):
        """Test peer joining a session that already has peers"""
        # Arrange
        ws1 = AsyncMock()
        ws2 = AsyncMock()
        await service.register_peer("peer_004", ws1)
        await service.register_peer("peer_005", ws2)
        await service.join_session("peer_004", "session_002")
        
        # Act
        existing_peers = await service.join_session("peer_005", "session_002")
        
        # Assert
        assert len(existing_peers) == 1
        assert "peer_004" in existing_peers
        assert len(service.sessions["session_002"]) == 2
    
    async def test_leave_session(self, service, mock_websocket):
        """Test peer leaving a session"""
        # Arrange
        await service.register_peer("peer_006", mock_websocket)
        await service.join_session("peer_006", "session_003")
        
        # Act
        left_session_id = await service.leave_session("peer_006")
        
        # Assert
        assert left_session_id == "session_003"
        assert "peer_006" not in service.peer_sessions
        assert "session_003" not in service.sessions  # Session removed when empty
    
    async def test_leave_session_not_in_session(self, service):
        """Test leaving session when not in one"""
        # Act
        result = await service.leave_session("peer_nonexistent")
        
        # Assert
        assert result is None
    
    async def test_send_to_peer(self, service, mock_websocket):
        """Test sending message to a peer"""
        # Arrange
        await service.register_peer("peer_007", mock_websocket)
        message = {"type": "test", "data": "hello"}
        
        # Act
        result = await service.send_to_peer("peer_007", message)
        
        # Assert
        assert result is True
        mock_websocket.send_json.assert_called_once_with(message)
    
    async def test_send_to_peer_not_connected(self, service):
        """Test sending message to non-connected peer"""
        # Act
        result = await service.send_to_peer("peer_nonexistent", {"type": "test"})
        
        # Assert
        assert result is False
    
    async def test_broadcast_to_session(self, service):
        """Test broadcasting message to session"""
        # Arrange
        ws1 = AsyncMock()
        ws2 = AsyncMock()
        ws3 = AsyncMock()
        await service.register_peer("peer_008", ws1)
        await service.register_peer("peer_009", ws2)
        await service.register_peer("peer_010", ws3)
        await service.join_session("peer_008", "session_004")
        await service.join_session("peer_009", "session_004")
        await service.join_session("peer_010", "session_004")
        
        message = {"type": "broadcast", "data": "hello all"}
        
        # Act
        count = await service.broadcast_to_session("session_004", message)
        
        # Assert
        assert count == 3
        ws1.send_json.assert_called_once_with(message)
        ws2.send_json.assert_called_once_with(message)
        ws3.send_json.assert_called_once_with(message)
    
    async def test_broadcast_to_session_exclude_peer(self, service):
        """Test broadcasting with excluded peer"""
        # Arrange
        ws1 = AsyncMock()
        ws2 = AsyncMock()
        await service.register_peer("peer_011", ws1)
        await service.register_peer("peer_012", ws2)
        await service.join_session("peer_011", "session_005")
        await service.join_session("peer_012", "session_005")
        
        message = {"type": "broadcast", "data": "hello"}
        
        # Act
        count = await service.broadcast_to_session("session_005", message, exclude_peer="peer_011")
        
        # Assert
        assert count == 1
        ws1.send_json.assert_not_called()
        ws2.send_json.assert_called_once_with(message)
    
    async def test_handle_offer(self, service):
        """Test handling WebRTC offer"""
        # Arrange
        ws_from = AsyncMock()
        ws_to = AsyncMock()
        await service.register_peer("peer_013", ws_from)
        await service.register_peer("peer_014", ws_to)
        
        sdp = {"type": "offer", "sdp": "v=0..."}
        
        # Act
        result = await service.handle_offer("peer_013", "peer_014", sdp)
        
        # Assert
        assert result is True
        ws_to.send_json.assert_called_once()
        call_args = ws_to.send_json.call_args[0][0]
        assert call_args["type"] == "offer"
        assert call_args["from"] == "peer_013"
        assert call_args["sdp"] == sdp
    
    async def test_handle_answer(self, service):
        """Test handling WebRTC answer"""
        # Arrange
        ws_from = AsyncMock()
        ws_to = AsyncMock()
        await service.register_peer("peer_015", ws_from)
        await service.register_peer("peer_016", ws_to)
        
        sdp = {"type": "answer", "sdp": "v=0..."}
        
        # Act
        result = await service.handle_answer("peer_015", "peer_016", sdp)
        
        # Assert
        assert result is True
        ws_to.send_json.assert_called_once()
        call_args = ws_to.send_json.call_args[0][0]
        assert call_args["type"] == "answer"
        assert call_args["from"] == "peer_015"
        assert call_args["sdp"] == sdp
    
    async def test_handle_ice_candidate(self, service):
        """Test handling ICE candidate"""
        # Arrange
        ws_from = AsyncMock()
        ws_to = AsyncMock()
        await service.register_peer("peer_017", ws_from)
        await service.register_peer("peer_018", ws_to)
        
        candidate = {"candidate": "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host"}
        
        # Act
        result = await service.handle_ice_candidate("peer_017", "peer_018", candidate)
        
        # Assert
        assert result is True
        ws_to.send_json.assert_called_once()
        call_args = ws_to.send_json.call_args[0][0]
        assert call_args["type"] == "ice-candidate"
        assert call_args["from"] == "peer_017"
        assert call_args["candidate"] == candidate
    
    async def test_get_session_peers(self, service, mock_websocket):
        """Test getting peers in a session"""
        # Arrange
        await service.register_peer("peer_019", mock_websocket)
        await service.register_peer("peer_020", mock_websocket)
        await service.join_session("peer_019", "session_006")
        await service.join_session("peer_020", "session_006")
        
        # Act
        peers = service.get_session_peers("session_006")
        
        # Assert
        assert len(peers) == 2
        assert "peer_019" in peers
        assert "peer_020" in peers
    
    async def test_get_session_peers_nonexistent(self, service):
        """Test getting peers from non-existent session"""
        # Act
        peers = service.get_session_peers("nonexistent_session")
        
        # Assert
        assert len(peers) == 0
    
    async def test_get_peer_session(self, service, mock_websocket):
        """Test getting session for a peer"""
        # Arrange
        await service.register_peer("peer_021", mock_websocket)
        await service.join_session("peer_021", "session_007")
        
        # Act
        session_id = service.get_peer_session("peer_021")
        
        # Assert
        assert session_id == "session_007"
    
    async def test_get_peer_session_not_in_session(self, service):
        """Test getting session for peer not in session"""
        # Act
        session_id = service.get_peer_session("peer_nonexistent")

        # Assert
        assert session_id is None

    async def test_unregister_peer_removes_from_session(self, service, mock_websocket):
        """Test unregistering peer removes it from session and cleans up empty session"""
        # Arrange - Register peer and join session
        await service.register_peer("peer_cleanup", mock_websocket)
        await service.join_session("peer_cleanup", "session_cleanup")

        # Verify peer is in session
        assert "session_cleanup" in service.sessions
        assert "peer_cleanup" in service.sessions["session_cleanup"]

        # Act - Unregister peer
        await service.unregister_peer("peer_cleanup")

        # Assert - Session should be cleaned up (no peers left)
        assert "peer_cleanup" not in service.connections
        assert "peer_cleanup" not in service.peer_sessions
        assert "session_cleanup" not in service.sessions  # Session removed when empty

    async def test_send_to_peer_error_handling(self, service):
        """Test send_to_peer handles errors gracefully"""
        # Arrange - Create a websocket that raises an error
        bad_websocket = AsyncMock()
        bad_websocket.send_json = AsyncMock(side_effect=Exception("Connection error"))

        await service.register_peer("peer_error", bad_websocket)

        # Act - Try to send message
        result = await service.send_to_peer("peer_error", {"type": "test", "data": "test"})

        # Assert - Should return False on error
        assert result is False

    async def test_unregister_peer_with_multiple_peers_in_session(self, service, mock_websocket):
        """Test unregistering one peer doesn't remove session if other peers remain"""
        # Arrange - Register multiple peers in same session
        mock_websocket_2 = AsyncMock()
        mock_websocket_2.send_json = AsyncMock()

        await service.register_peer("peer_multi_1", mock_websocket)
        await service.register_peer("peer_multi_2", mock_websocket_2)
        await service.join_session("peer_multi_1", "session_multi")
        await service.join_session("peer_multi_2", "session_multi")

        # Act - Unregister one peer
        await service.unregister_peer("peer_multi_1")

        # Assert - Session should still exist with remaining peer
        assert "session_multi" in service.sessions
        assert "peer_multi_2" in service.sessions["session_multi"]
        assert "peer_multi_1" not in service.sessions["session_multi"]

    async def test_broadcast_to_nonexistent_session(self, service):
        """Test broadcasting to non-existent session returns 0"""
        # Act - Try to broadcast to non-existent session
        result = await service.broadcast_to_session(
            "nonexistent_session",
            {"type": "test", "data": "test"}
        )

        # Assert - Should return 0 (no peers received message)
        assert result == 0
