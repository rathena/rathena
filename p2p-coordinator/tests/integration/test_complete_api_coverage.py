"""
Comprehensive integration tests to achieve 100% API coverage

This file contains tests specifically designed to cover all uncovered lines
in the API endpoints, including error handling paths.
"""

import pytest
import uuid
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, create_test_zone


@pytest.mark.integration
@pytest.mark.asyncio
class TestHostsAPICompleteCoverage:
    """Complete coverage tests for Hosts API"""

    async def test_register_host_success_all_fields(self, async_client: AsyncClient):
        """Test register host with all optional fields"""
        host_data = create_test_host()
        host_data["public_ip"] = "203.0.113.1"
        host_data["nat_type"] = "full_cone"
        
        response = await async_client.post("/api/v1/hosts/register", json=host_data)
        assert response.status_code == 201
        data = response.json()
        assert data["host_id"] == host_data["host_id"]
        assert data["public_ip"] == "203.0.113.1"

    async def test_register_host_duplicate(self, async_client: AsyncClient):
        """Test registering duplicate host"""
        host_data = create_test_host()
        
        # Register first time
        response1 = await async_client.post("/api/v1/hosts/register", json=host_data)
        assert response1.status_code == 201
        
        # Try to register again with same host_id
        response2 = await async_client.post("/api/v1/hosts/register", json=host_data)
        # Should either succeed (update) or return error
        assert response2.status_code in [200, 201, 400, 409]

    async def test_get_host_success(self, async_client: AsyncClient):
        """Test getting a host by ID"""
        # Register a host first
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        # Get the host
        response = await async_client.get(f"/api/v1/hosts/{host_data['host_id']}")
        assert response.status_code == 200
        data = response.json()
        assert data["host_id"] == host_data["host_id"]

    async def test_get_host_not_found(self, async_client: AsyncClient):
        """Test getting non-existent host"""
        response = await async_client.get("/api/v1/hosts/nonexistent_host_12345")
        assert response.status_code == 404

    async def test_list_hosts_empty(self, async_client: AsyncClient):
        """Test listing hosts when database is empty"""
        response = await async_client.get("/api/v1/hosts/")
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)

    async def test_list_hosts_with_data(self, async_client: AsyncClient):
        """Test listing hosts with data"""
        # Register multiple hosts
        for i in range(3):
            host_data = create_test_host()
            await async_client.post("/api/v1/hosts/register", json=host_data)

        response = await async_client.get("/api/v1/hosts/")
        assert response.status_code == 200
        data = response.json()
        assert len(data) >= 3

    async def test_update_heartbeat_success(self, async_client: AsyncClient):
        """Test updating host heartbeat"""
        # Register a host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        # Update heartbeat
        response = await async_client.post(
            f"/api/v1/hosts/{host_data['host_id']}/heartbeat",
            json={
                "latency_ms": 45.5,
                "packet_loss_percent": 0.5,
                "bandwidth_usage_mbps": 100.0,
                "current_players": 5,
                "current_zones": 2
            }
        )
        assert response.status_code == 200

    async def test_update_heartbeat_not_found(self, async_client: AsyncClient):
        """Test updating heartbeat for non-existent host"""
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

    async def test_unregister_host_success(self, async_client: AsyncClient):
        """Test unregistering a host"""
        # Register a host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        # Unregister
        response = await async_client.delete(f"/api/v1/hosts/{host_data['host_id']}")
        assert response.status_code == 204

    async def test_unregister_host_not_found(self, async_client: AsyncClient):
        """Test unregistering non-existent host"""
        response = await async_client.delete("/api/v1/hosts/nonexistent_host")
        assert response.status_code == 404


@pytest.mark.integration
@pytest.mark.asyncio
class TestSessionsAPICompleteCoverage:
    """Complete coverage tests for Sessions API"""

    async def test_create_session_success(self, async_client: AsyncClient):
        """Test creating a session"""
        # Register host and zone first
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)
        
        # Create session
        session_data = {
            "host_id": host_data["host_id"],
            "zone_id": zone_data["zone_id"],
            "player_ids": ["player1", "player2"]
        }
        response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert response.status_code in [200, 201]

    async def test_create_session_invalid_host(self, async_client: AsyncClient):
        """Test creating session with invalid host"""
        session_data = {
            "host_id": "nonexistent_host",
            "zone_id": "some_zone",
            "player_ids": ["player1"]
        }
        response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert response.status_code in [400, 404]

    async def test_get_session_not_found(self, async_client: AsyncClient):
        """Test getting non-existent session"""
        response = await async_client.get("/api/v1/sessions/999999")
        assert response.status_code == 404


@pytest.mark.integration
@pytest.mark.asyncio
class TestZonesAPICompleteCoverage:
    """Complete coverage tests for Zones API"""

    async def test_create_zone_success(self, async_client: AsyncClient):
        """Test creating a zone"""
        zone_data = create_test_zone()
        response = await async_client.post("/api/v1/zones/", json=zone_data)
        assert response.status_code in [200, 201]

    async def test_create_zone_duplicate(self, async_client: AsyncClient):
        """Test creating duplicate zone"""
        zone_data = create_test_zone()

        # Create first time
        response1 = await async_client.post("/api/v1/zones/", json=zone_data)
        assert response1.status_code in [200, 201]

        # Try to create again
        response2 = await async_client.post("/api/v1/zones/", json=zone_data)
        assert response2.status_code in [200, 201, 400, 409]

    async def test_get_zone_success(self, async_client: AsyncClient):
        """Test getting a zone"""
        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        response = await async_client.get(f"/api/v1/zones/{zone_data['zone_id']}")
        assert response.status_code == 200

    async def test_get_zone_not_found(self, async_client: AsyncClient):
        """Test getting non-existent zone"""
        response = await async_client.get("/api/v1/zones/nonexistent_zone")
        assert response.status_code == 404

    async def test_list_zones(self, async_client: AsyncClient):
        """Test listing zones"""
        response = await async_client.get("/api/v1/zones/")
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)

    async def test_enable_p2p_success(self, async_client: AsyncClient):
        """Test enabling P2P for a zone"""
        zone_data = create_test_zone()
        zone_data["p2p_enabled"] = False
        await async_client.post("/api/v1/zones/", json=zone_data)

        response = await async_client.post(f"/api/v1/zones/{zone_data['zone_id']}/enable-p2p")
        assert response.status_code == 200

    async def test_enable_p2p_not_found(self, async_client: AsyncClient):
        """Test enabling P2P for non-existent zone"""
        response = await async_client.post("/api/v1/zones/nonexistent_zone/enable-p2p")
        assert response.status_code == 404

    async def test_disable_p2p_success(self, async_client: AsyncClient):
        """Test disabling P2P for a zone"""
        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        response = await async_client.post(f"/api/v1/zones/{zone_data['zone_id']}/disable-p2p")
        assert response.status_code == 200

    async def test_disable_p2p_not_found(self, async_client: AsyncClient):
        """Test disabling P2P for non-existent zone"""
        response = await async_client.post("/api/v1/zones/nonexistent_zone/disable-p2p")
        assert response.status_code == 404


@pytest.mark.integration
@pytest.mark.asyncio
class TestMonitoringAPICompleteCoverage:
    """Complete coverage tests for Monitoring API"""

    async def test_dashboard_with_all_data(self, async_client: AsyncClient):
        """Test dashboard with hosts, zones, and sessions"""
        # Create host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        # Create zone
        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        # Get dashboard
        response = await async_client.get("/api/v1/monitoring/dashboard")
        assert response.status_code == 200
        data = response.json()
        assert "hosts" in data
        assert "zones" in data
        assert "sessions" in data
        assert "system" in data




