"""
Integration tests for Hosts API endpoints
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, generate_host_id


@pytest.mark.integration
@pytest.mark.asyncio
class TestHostsAPI:
    """Test suite for Hosts API endpoints"""
    
    async def test_register_host_success(self, async_client: AsyncClient):
        """Test successful host registration"""
        # Arrange
        host_data = create_test_host()

        # Act
        response = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )

        # Assert
        assert response.status_code == 201  # Created
        data = response.json()
        assert data["host_id"] == host_data["host_id"]
        assert data["player_id"] == host_data["player_id"]
        assert data["status"] == "online"
    
    async def test_register_host_missing_required_fields(self, async_client: AsyncClient):
        """Test host registration fails with missing required fields"""
        # Arrange
        incomplete_data = {
            "host_id": generate_host_id(),
            "player_id": "test_player",
            # Missing other required fields
        }
        
        # Act
        response = await async_client.post(
            "/api/v1/hosts/register",
            json=incomplete_data,
        )
        
        # Assert
        assert response.status_code == 422  # Validation error
    
    async def test_update_heartbeat_success(self, async_client: AsyncClient):
        """Test successful heartbeat update"""
        # Arrange - First register a host
        host_data = create_test_host()
        register_response = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )
        assert register_response.status_code == 201  # Created

        # Act - Update heartbeat
        heartbeat_data = {
            "latency_ms": 50.0,
            "packet_loss_percent": 0.1,
            "bandwidth_usage_mbps": 10.0,
            "current_players": 5,
            "current_zones": 2,
        }
        response = await async_client.post(
            f"/api/v1/hosts/{host_data['host_id']}/heartbeat",
            json=heartbeat_data,
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert data["host_id"] == host_data["host_id"]
        assert data["latency_ms"] == heartbeat_data["latency_ms"]
        assert data["current_players"] == heartbeat_data["current_players"]
    
    async def test_update_heartbeat_nonexistent_host(self, async_client: AsyncClient):
        """Test heartbeat update fails for non-existent host"""
        # Arrange
        nonexistent_host_id = "nonexistent_host_12345"

        # Act
        response = await async_client.post(
            f"/api/v1/hosts/{nonexistent_host_id}/heartbeat",
            json={
                "latency_ms": 50.0,
                "packet_loss_percent": 0.1,
                "bandwidth_usage_mbps": 10.0,
                "current_players": 0,
                "current_zones": 0,
            },
        )

        # Assert
        assert response.status_code == 404
    
    async def test_get_host_success(self, async_client: AsyncClient):
        """Test getting a host by ID"""
        # Arrange - Register a host first
        host_data = create_test_host()
        register_response = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )
        assert register_response.status_code == 201  # Created

        # Act
        response = await async_client.get(
            f"/api/v1/hosts/{host_data['host_id']}",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert data["host_id"] == host_data["host_id"]
        assert data["player_id"] == host_data["player_id"]
    
    async def test_get_host_not_found(self, async_client: AsyncClient):
        """Test getting non-existent host returns 404"""
        # Arrange
        nonexistent_host_id = "nonexistent_host_12345"
        
        # Act
        response = await async_client.get(
            f"/api/v1/hosts/{nonexistent_host_id}",
        )
        
        # Assert
        assert response.status_code == 404
    
    async def test_get_all_hosts(self, async_client: AsyncClient):
        """Test getting all hosts"""
        # Arrange - Register multiple hosts
        host1 = create_test_host()
        host2 = create_test_host()
        
        await async_client.post("/api/v1/hosts/register", json=host1)
        await async_client.post("/api/v1/hosts/register", json=host2)
        
        # Act
        response = await async_client.get("/api/v1/hosts/")
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)
        assert len(data) >= 2
    
    async def test_get_available_hosts(self, async_client: AsyncClient):
        """Test getting available hosts"""
        # Arrange - Register a host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        # Act
        response = await async_client.get("/api/v1/hosts/available/list")
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)
        # Should contain at least the host we just registered
        host_ids = [h["host_id"] for h in data]
        assert host_data["host_id"] in host_ids
    
    async def test_get_best_host(self, async_client: AsyncClient):
        """Test getting the best available host"""
        # Arrange - Register a host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        # Act
        response = await async_client.get("/api/v1/hosts/available/best")
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "host_id" in data
        assert "quality_score" in data
    
    async def test_delete_host_success(self, async_client: AsyncClient):
        """Test successful host deletion"""
        # Arrange - Register a host first
        host_data = create_test_host()
        register_response = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )
        assert register_response.status_code == 201  # Created

        # Act
        response = await async_client.delete(
            f"/api/v1/hosts/{host_data['host_id']}",
        )

        # Assert
        assert response.status_code == 204  # No Content

        # Verify host is deleted
        get_response = await async_client.get(
            f"/api/v1/hosts/{host_data['host_id']}",
        )
        assert get_response.status_code == 404
    
    async def test_delete_nonexistent_host(self, async_client: AsyncClient):
        """Test deleting non-existent host returns 404"""
        # Arrange
        nonexistent_host_id = "nonexistent_host_12345"
        
        # Act
        response = await async_client.delete(
            f"/api/v1/hosts/{nonexistent_host_id}",
        )
        
        # Assert
        assert response.status_code == 404
    
    async def test_register_same_host_twice_updates(self, async_client: AsyncClient):
        """Test that registering the same host twice updates it"""
        # Arrange
        host_data = create_test_host()

        # Act - Register twice
        response1 = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )

        # Modify some data
        host_data["player_name"] = "UpdatedPlayerName"
        response2 = await async_client.post(
            "/api/v1/hosts/register",
            json=host_data,
        )

        # Assert
        assert response1.status_code == 201  # Created
        assert response2.status_code == 201  # Created (upsert behavior)
        assert response2.json()["player_name"] == "UpdatedPlayerName"

