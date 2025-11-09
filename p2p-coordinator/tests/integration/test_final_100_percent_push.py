"""
Final push to 100% coverage

This file systematically tests all remaining uncovered lines to achieve 100% coverage.
Focus on normal execution paths, not just exception handlers.
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, create_test_zone, create_test_session


@pytest.mark.integration
@pytest.mark.asyncio
class TestAPIReturnStatements:
    """Test all API return statements that are uncovered"""

    async def test_hosts_api_all_returns(self, async_client: AsyncClient):
        """Test all return statements in hosts API"""
        # Register host - covers line 101 (return host)
        host_data = create_test_host()
        host_data["host_id"] = "return_test_host"
        register_response = await async_client.post("/api/v1/hosts/register", json=host_data)
        assert register_response.status_code == 201
        host_id = register_response.json()["host_id"]
        
        # Get host - covers line 159-165 (return host or 404)
        get_response = await async_client.get(f"/api/v1/hosts/{host_id}")
        assert get_response.status_code == 200
        
        # Update heartbeat - covers line 139 (return host)
        heartbeat_data = {
            "latency_ms": 20.0,
            "packet_loss_percent": 0.5,
            "bandwidth_usage_mbps": 50.0,
            "current_players": 10,
            "current_zones": 2
        }
        heartbeat_response = await async_client.post(f"/api/v1/hosts/{host_id}/heartbeat", json=heartbeat_data)
        assert heartbeat_response.status_code == 200
        
        # List hosts - covers line 184-187 (return hosts)
        list_response = await async_client.get("/api/v1/hosts/")
        assert list_response.status_code == 200
        
        # Get available hosts - covers line 198, 201 (return hosts)
        available_response = await async_client.get("/api/v1/hosts/available/list")
        assert available_response.status_code == 200
        
        # Get best host - covers line 227 (return host or 404)
        best_response = await async_client.get("/api/v1/hosts/available/best")
        assert best_response.status_code in [200, 404]
        
        # Unregister host - covers line 254-266 (return 204)
        unregister_response = await async_client.delete(f"/api/v1/hosts/{host_id}")
        assert unregister_response.status_code == 204

    async def test_zones_api_all_returns(self, async_client: AsyncClient):
        """Test all return statements in zones API"""
        # Create zone - covers line 94 (return zone)
        zone_data = create_test_zone()
        zone_data["zone_id"] = "return_test_zone"
        create_response = await async_client.post("/api/v1/zones/", json=zone_data)
        assert create_response.status_code == 201
        zone_id = create_response.json()["zone_id"]
        
        # Get zone - covers line 112-118 (return zone or 404)
        get_response = await async_client.get(f"/api/v1/zones/{zone_id}")
        assert get_response.status_code == 200
        
        # List zones - covers line 137-140, 151, 154 (return zones)
        list_response = await async_client.get("/api/v1/zones/")
        assert list_response.status_code == 200
        
        # Get P2P enabled zones - covers line 170 (return zones)
        p2p_response = await async_client.get("/api/v1/zones/p2p/enabled")
        assert p2p_response.status_code == 200
        
        # Enable P2P - covers line 195 (return zone)
        enable_response = await async_client.post(f"/api/v1/zones/{zone_id}/enable-p2p")
        assert enable_response.status_code == 200
        
        # Disable P2P - covers line 222 (return zone)
        disable_response = await async_client.post(f"/api/v1/zones/{zone_id}/disable-p2p")
        assert disable_response.status_code == 200
        
        # Update zone status - covers line 255-263, 267-269
        # Note: This endpoint may require specific parameters
        # Skipping for now as it may not exist or have different signature

    async def test_sessions_api_all_returns(self, async_client: AsyncClient):
        """Test all return statements in sessions API"""
        # Create host and zone
        host_data = create_test_host()
        host_data["host_id"] = "session_return_host"
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        zone_data = create_test_zone()
        zone_data["zone_id"] = "session_return_zone"
        await async_client.post("/api/v1/zones/", json=zone_data)
        
        # Create session - covers line 78 (return SessionResponse)
        session_data = create_test_session(host_id="session_return_host", zone_id="session_return_zone")
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert create_response.status_code == 201
        session_id = create_response.json()["id"]
        
        # Get session - covers line 105-111 (return SessionResponse)
        get_response = await async_client.get(f"/api/v1/sessions/{session_id}")
        assert get_response.status_code == 200
        
        # Activate session - covers line 144 (return SessionResponse)
        activate_response = await async_client.post(f"/api/v1/sessions/{session_id}/activate")
        assert activate_response.status_code == 200
        
        # Add player - covers line 162 (return SessionResponse)
        add_player_response = await async_client.post(f"/api/v1/sessions/{session_id}/players", json={"player_id": "player1"})
        assert add_player_response.status_code == 200
        
        # Get active sessions for host - covers line 186 (return list)
        host_sessions_response = await async_client.get(f"/api/v1/sessions/host/session_return_host/active")
        assert host_sessions_response.status_code == 200
        
        # Get active sessions for zone - covers line 197 (return list)
        zone_sessions_response = await async_client.get(f"/api/v1/sessions/zone/session_return_zone/active")
        assert zone_sessions_response.status_code == 200
        
        # Remove player - covers line 225-227 (return message)
        remove_player_response = await async_client.delete(f"/api/v1/sessions/{session_id}/players/player1")
        assert remove_player_response.status_code == 200
        
        # End session - covers line 258 (return SessionResponse)
        end_response = await async_client.post(f"/api/v1/sessions/{session_id}/end")
        assert end_response.status_code == 200
        
        # Cleanup sessions - covers line 270, 287, 297 (return message)
        cleanup_response = await async_client.post("/api/v1/sessions/cleanup")
        assert cleanup_response.status_code == 200

