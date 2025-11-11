"""
Integration tests for API error paths

This file contains tests specifically designed to cover all error handling
paths in the API endpoints to achieve 100% coverage.
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, create_test_zone, create_test_session


@pytest.mark.integration
@pytest.mark.asyncio
class TestZonesAPIErrorPaths:
    """Test error paths in Zones API"""

    async def test_create_zone_exception(self, async_client: AsyncClient, monkeypatch):
        """Test create zone with exception"""
        from api import zones
        
        async def mock_create_error(*args, **kwargs):
            raise Exception("Database error")
        
        monkeypatch.setattr(zones.zone_service, "create_zone", mock_create_error)
        
        zone_data = create_test_zone()
        response = await async_client.post("/api/v1/zones/", json=zone_data)
        assert response.status_code == 500

    async def test_get_zone_not_found(self, async_client: AsyncClient):
        """Test get zone that doesn't exist"""
        response = await async_client.get("/api/v1/zones/nonexistent_zone")
        assert response.status_code == 404

    async def test_enable_p2p_not_found(self, async_client: AsyncClient, monkeypatch):
        """Test enable P2P for non-existent zone"""
        from api import zones

        async def mock_enable_none(*args, **kwargs):
            return None

        monkeypatch.setattr(zones.zone_service, "enable_p2p_for_zone", mock_enable_none)

        response = await async_client.post("/api/v1/zones/nonexistent_zone/enable-p2p")
        assert response.status_code == 404

    async def test_disable_p2p_not_found(self, async_client: AsyncClient, monkeypatch):
        """Test disable P2P for non-existent zone"""
        from api import zones

        async def mock_disable_none(*args, **kwargs):
            return None

        monkeypatch.setattr(zones.zone_service, "disable_p2p_for_zone", mock_disable_none)

        response = await async_client.post("/api/v1/zones/nonexistent_zone/disable-p2p")
        assert response.status_code == 404

    async def test_update_zone_status_not_found(self, async_client: AsyncClient, monkeypatch):
        """Test update zone status for non-existent zone"""
        from api import zones

        async def mock_update_none(*args, **kwargs):
            return None

        monkeypatch.setattr(zones.zone_service, "update_zone_status", mock_update_none)

        response = await async_client.patch("/api/v1/zones/nonexistent_zone/status?new_status=enabled")
        assert response.status_code == 400  # Returns 400 when update fails


@pytest.mark.integration
@pytest.mark.asyncio
class TestHostsAPIErrorPaths:
    """Test error paths in Hosts API"""

    async def test_register_host_exception(self, async_client: AsyncClient, monkeypatch):
        """Test register host with exception"""
        from api import hosts
        
        async def mock_register_error(*args, **kwargs):
            raise Exception("Database error")
        
        monkeypatch.setattr(hosts.host_service, "register_host", mock_register_error)
        
        host_data = create_test_host()
        response = await async_client.post("/api/v1/hosts/register", json=host_data)
        assert response.status_code == 500

    async def test_get_host_not_found(self, async_client: AsyncClient):
        """Test get host that doesn't exist"""
        response = await async_client.get("/api/v1/hosts/nonexistent_host")
        assert response.status_code == 404

    async def test_update_heartbeat_not_found(self, async_client: AsyncClient, monkeypatch):
        """Test update heartbeat for non-existent host"""
        from api import hosts
        
        async def mock_heartbeat_none(*args, **kwargs):
            return None
        
        monkeypatch.setattr(hosts.host_service, "update_heartbeat", mock_heartbeat_none)
        
        response = await async_client.post(
            "/api/v1/hosts/nonexistent_host/heartbeat",
            json={
                "latency_ms": 45.5,
                "packet_loss_percent": 0.5,
                "bandwidth_usage_mbps": 100.0,
                "current_players": 5,
                "current_zones": 2
            }
        )
        assert response.status_code == 404

    async def test_unregister_host_not_found(self, async_client: AsyncClient, monkeypatch):
        """Test unregister host that doesn't exist"""
        from api import hosts
        
        async def mock_unregister_false(*args, **kwargs):
            return False
        
        monkeypatch.setattr(hosts.host_service, "unregister_host", mock_unregister_false)
        
        response = await async_client.delete("/api/v1/hosts/nonexistent_host")
        assert response.status_code == 404


@pytest.mark.integration
@pytest.mark.asyncio
class TestSessionsAPIErrorPaths:
    """Test error paths in Sessions API"""

    async def test_create_session_failed(self, async_client: AsyncClient, monkeypatch):
        """Test create session that fails"""
        from api import sessions

        async def mock_create_none(*args, **kwargs):
            return None

        monkeypatch.setattr(sessions.session_service, "create_session", mock_create_none)

        session_data = {
            "host_id": "test_host",
            "zone_id": "test_zone",
            "player_ids": ["player1"]
        }
        response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert response.status_code == 400

    async def test_get_session_not_found(self, async_client: AsyncClient):
        """Test get session that doesn't exist"""
        response = await async_client.get("/api/v1/sessions/999999")
        assert response.status_code == 404

    async def test_activate_session_failed(self, async_client: AsyncClient, monkeypatch):
        """Test activate session that fails"""
        from api import sessions

        async def mock_activate_false(*args, **kwargs):
            return False

        monkeypatch.setattr(sessions.session_service, "activate_session", mock_activate_false)

        response = await async_client.post("/api/v1/sessions/999999/activate")
        assert response.status_code == 400

    async def test_end_session_failed(self, async_client: AsyncClient, monkeypatch):
        """Test end session that fails"""
        from api import sessions

        async def mock_end_false(*args, **kwargs):
            return False

        monkeypatch.setattr(sessions.session_service, "end_session", mock_end_false)

        response = await async_client.post("/api/v1/sessions/999999/end")
        assert response.status_code == 400

    async def test_add_player_failed(self, async_client: AsyncClient, monkeypatch):
        """Test add player that fails"""
        from api import sessions

        async def mock_add_false(*args, **kwargs):
            return False

        monkeypatch.setattr(sessions.session_service, "add_player_to_session", mock_add_false)

        response = await async_client.post(
            "/api/v1/sessions/999999/players",
            json={"player_id": "player1"}
        )
        assert response.status_code == 400

    async def test_remove_player_failed(self, async_client: AsyncClient, monkeypatch):
        """Test remove player that fails"""
        from api import sessions

        async def mock_remove_false(*args, **kwargs):
            return False

        monkeypatch.setattr(sessions.session_service, "remove_player_from_session", mock_remove_false)

        response = await async_client.delete("/api/v1/sessions/999999/players/player1")
        assert response.status_code == 400

    async def test_update_metrics_failed(self, async_client: AsyncClient, monkeypatch):
        """Test update metrics that fails"""
        from api import sessions

        async def mock_update_false(*args, **kwargs):
            return False

        monkeypatch.setattr(sessions.session_service, "update_session_metrics", mock_update_false)

        response = await async_client.patch(
            "/api/v1/sessions/999999/metrics",
            json={"avg_latency_ms": 45.5}
        )
        assert response.status_code == 400

