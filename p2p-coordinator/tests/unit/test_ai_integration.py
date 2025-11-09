"""
Unit tests for AI Service Integration
"""

import pytest
import httpx
from unittest.mock import AsyncMock, MagicMock, patch
from services.ai_integration import AIServiceClient


@pytest.mark.unit
class TestAIServiceClient:
    """Test AIServiceClient class"""

    @pytest.fixture
    def ai_client(self):
        """Create AIServiceClient instance"""
        return AIServiceClient()

    @pytest.mark.asyncio
    async def test_initialize(self, ai_client):
        """Test client initialization"""
        await ai_client.initialize()
        assert ai_client.client is not None
        assert isinstance(ai_client.client, httpx.AsyncClient)
        await ai_client.close()

    @pytest.mark.asyncio
    async def test_close(self, ai_client):
        """Test client close"""
        await ai_client.initialize()
        assert ai_client.client is not None
        await ai_client.close()
        # Client should be closed but still exist
        assert ai_client.client is not None

    @pytest.mark.asyncio
    async def test_close_without_initialize(self, ai_client):
        """Test close without initialization"""
        # Should not raise error
        await ai_client.close()

    @pytest.mark.asyncio
    async def test_health_check_success(self, ai_client):
        """Test successful health check"""
        # Mock the HTTP client
        mock_response = MagicMock()
        mock_response.status_code = 200
        
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(return_value=mock_response)
        
        ai_client.client = mock_client
        
        result = await ai_client.health_check()
        assert result is True
        mock_client.get.assert_called_once_with("/health")

    @pytest.mark.asyncio
    async def test_health_check_failure(self, ai_client):
        """Test failed health check"""
        # Mock the HTTP client to raise exception
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=httpx.RequestError("Connection failed", request=MagicMock()))
        
        ai_client.client = mock_client
        
        result = await ai_client.health_check()
        assert result is False

    @pytest.mark.asyncio
    async def test_health_check_auto_initialize(self, ai_client):
        """Test health check auto-initializes client"""
        with patch.object(ai_client, 'initialize', new_callable=AsyncMock) as mock_init:
            mock_response = MagicMock()
            mock_response.status_code = 200
            
            mock_client = AsyncMock()
            mock_client.get = AsyncMock(return_value=mock_response)
            
            # Simulate initialize setting the client
            async def set_client():
                ai_client.client = mock_client
            mock_init.side_effect = set_client
            
            result = await ai_client.health_check()
            assert result is True
            mock_init.assert_called_once()

    @pytest.mark.asyncio
    async def test_get_zone_npcs_success(self, ai_client):
        """Test successful get zone NPCs"""
        mock_response = MagicMock()
        mock_response.json.return_value = [
            {"npc_id": "npc1", "name": "Guard"},
            {"npc_id": "npc2", "name": "Merchant"},
        ]
        mock_response.raise_for_status = MagicMock()
        
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(return_value=mock_response)
        
        ai_client.client = mock_client
        
        npcs = await ai_client.get_zone_npcs("zone_001")
        assert len(npcs) == 2
        assert npcs[0]["npc_id"] == "npc1"
        mock_client.get.assert_called_once_with("/api/world/zones/zone_001/npcs")

    @pytest.mark.asyncio
    async def test_get_zone_npcs_http_error(self, ai_client):
        """Test get zone NPCs with HTTP error"""
        mock_response = MagicMock()
        mock_response.status_code = 404
        
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=httpx.HTTPStatusError("Not found", request=MagicMock(), response=mock_response))
        
        ai_client.client = mock_client
        
        npcs = await ai_client.get_zone_npcs("zone_999")
        assert npcs == []

    @pytest.mark.asyncio
    async def test_get_zone_npcs_exception(self, ai_client):
        """Test get zone NPCs with general exception"""
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=Exception("Network error"))
        
        ai_client.client = mock_client
        
        npcs = await ai_client.get_zone_npcs("zone_001")
        assert npcs == []

    @pytest.mark.asyncio
    async def test_get_zone_npcs_auto_initialize(self, ai_client):
        """Test get zone NPCs auto-initializes client"""
        with patch.object(ai_client, 'initialize', new_callable=AsyncMock) as mock_init:
            mock_response = MagicMock()
            mock_response.json.return_value = []
            mock_response.raise_for_status = MagicMock()
            
            mock_client = AsyncMock()
            mock_client.get = AsyncMock(return_value=mock_response)
            
            async def set_client():
                ai_client.client = mock_client
            mock_init.side_effect = set_client
            
            await ai_client.get_zone_npcs("zone_001")
            mock_init.assert_called_once()

    @pytest.mark.asyncio
    async def test_get_npc_state_success(self, ai_client):
        """Test successful get NPC state"""
        mock_response = MagicMock()
        mock_response.json.return_value = {
            "npc_id": "npc1",
            "name": "Guard",
            "position": {"x": 100, "y": 200},
        }
        mock_response.raise_for_status = MagicMock()

        mock_client = AsyncMock()
        mock_client.get = AsyncMock(return_value=mock_response)

        ai_client.client = mock_client

        state = await ai_client.get_npc_state("npc1")
        assert state is not None
        assert state["npc_id"] == "npc1"
        mock_client.get.assert_called_once_with("/api/npcs/npc1")

    @pytest.mark.asyncio
    async def test_get_npc_state_not_found(self, ai_client):
        """Test get NPC state when NPC not found"""
        mock_response = MagicMock()
        mock_response.status_code = 404

        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=httpx.HTTPStatusError("Not found", request=MagicMock(), response=mock_response))

        ai_client.client = mock_client

        state = await ai_client.get_npc_state("npc_999")
        assert state is None

    @pytest.mark.asyncio
    async def test_get_npc_state_http_error(self, ai_client):
        """Test get NPC state with HTTP error (non-404)"""
        mock_response = MagicMock()
        mock_response.status_code = 500

        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=httpx.HTTPStatusError("Server error", request=MagicMock(), response=mock_response))

        ai_client.client = mock_client

        state = await ai_client.get_npc_state("npc1")
        assert state is None

    @pytest.mark.asyncio
    async def test_get_npc_state_exception(self, ai_client):
        """Test get NPC state with general exception"""
        mock_client = AsyncMock()
        mock_client.get = AsyncMock(side_effect=Exception("Network error"))

        ai_client.client = mock_client

        state = await ai_client.get_npc_state("npc1")
        assert state is None

    @pytest.mark.asyncio
    async def test_get_npc_state_auto_initialize(self, ai_client):
        """Test get NPC state auto-initializes client"""
        with patch.object(ai_client, 'initialize', new_callable=AsyncMock) as mock_init:
            mock_response = MagicMock()
            mock_response.json.return_value = {"npc_id": "npc1"}
            mock_response.raise_for_status = MagicMock()

            mock_client = AsyncMock()
            mock_client.get = AsyncMock(return_value=mock_response)

            async def set_client():
                ai_client.client = mock_client
            mock_init.side_effect = set_client

            await ai_client.get_npc_state("npc1")
            mock_init.assert_called_once()

    @pytest.mark.asyncio
    async def test_update_npc_position_success(self, ai_client):
        """Test successful update NPC position"""
        mock_response = MagicMock()
        mock_response.raise_for_status = MagicMock()

        mock_client = AsyncMock()
        mock_client.patch = AsyncMock(return_value=mock_response)

        ai_client.client = mock_client

        result = await ai_client.update_npc_position("npc1", 150, 250, "zone_001")
        assert result is True
        mock_client.patch.assert_called_once_with(
            "/api/npcs/npc1/position",
            json={"x": 150, "y": 250, "zone_id": "zone_001"},
        )

    @pytest.mark.asyncio
    async def test_update_npc_position_failure(self, ai_client):
        """Test failed update NPC position"""
        mock_client = AsyncMock()
        mock_client.patch = AsyncMock(side_effect=Exception("Network error"))

        ai_client.client = mock_client

        result = await ai_client.update_npc_position("npc1", 150, 250, "zone_001")
        assert result is False

    @pytest.mark.asyncio
    async def test_update_npc_position_auto_initialize(self, ai_client):
        """Test update NPC position auto-initializes client"""
        with patch.object(ai_client, 'initialize', new_callable=AsyncMock) as mock_init:
            mock_response = MagicMock()
            mock_response.raise_for_status = MagicMock()

            mock_client = AsyncMock()
            mock_client.patch = AsyncMock(return_value=mock_response)

            async def set_client():
                ai_client.client = mock_client
            mock_init.side_effect = set_client

            await ai_client.update_npc_position("npc1", 150, 250, "zone_001")
            mock_init.assert_called_once()

