"""
Integration tests for Sessions API endpoints
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_session, create_test_host, generate_player_id


@pytest.mark.integration
@pytest.mark.asyncio
class TestSessionsAPI:
    """Test suite for Sessions API endpoints"""
    
    async def test_create_session_success(self, async_client: AsyncClient):
        """Test successful session creation"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Register a host and zone first
        host_data = create_test_host()
        host_response = await async_client.post("/api/v1/hosts/register", json=host_data)
        assert host_response.status_code == 201

        zone_data = create_test_zone()
        zone_response = await async_client.post("/api/v1/zones/", json=zone_data)
        assert zone_response.status_code in [200, 201]

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])

        # Act
        response = await async_client.post(
            "/api/v1/sessions/",
            json=session_data,
        )

        # Assert
        assert response.status_code in [200, 201]  # Created
        data = response.json()
        assert "id" in data or "session_id" in data
    
    async def test_create_session_missing_fields(self, async_client: AsyncClient):
        """Test session creation fails with missing required fields"""
        # Arrange
        incomplete_data = {
            "session_id": "test_session",
            # Missing other required fields
        }
        
        # Act
        response = await async_client.post(
            "/api/v1/sessions/",
            json=incomplete_data,
        )
        
        # Assert
        assert response.status_code == 422  # Validation error
    
    async def test_get_session_success(self, async_client: AsyncClient):
        """Test getting a session by ID"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create a session first
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert create_response.status_code in [200, 201]  # Created
        session_id = create_response.json().get("id") or create_response.json().get("session_id")

        # Act
        response = await async_client.get(
            f"/api/v1/sessions/{session_id}",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert data.get("id") == session_id or data.get("session_id") == session_id
    
    async def test_get_session_not_found(self, async_client: AsyncClient):
        """Test getting non-existent session returns 404"""
        # Arrange
        nonexistent_session_id = 999999  # Use integer ID

        # Act
        response = await async_client.get(
            f"/api/v1/sessions/{nonexistent_session_id}",
        )

        # Assert
        assert response.status_code == 404
    
    async def test_activate_session_success(self, async_client: AsyncClient):
        """Test activating a session"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create a session first
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        assert create_response.status_code in [200, 201]  # Created
        session_id = create_response.json().get("id") or create_response.json().get("session_id")

        # Act
        response = await async_client.post(
            f"/api/v1/sessions/{session_id}/activate",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "message" in data or "status" in data
    
    async def test_end_session_success(self, async_client: AsyncClient):
        """Test ending a session"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create and activate a session
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        session_id = create_response.json().get("id") or create_response.json().get("session_id")
        await async_client.post(f"/api/v1/sessions/{session_id}/activate")

        # Act
        response = await async_client.post(
            f"/api/v1/sessions/{session_id}/end",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "message" in data or "status" in data
    
    async def test_add_player_to_session(self, async_client: AsyncClient):
        """Test adding a player to a session"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create and activate a session
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        session_id = create_response.json().get("id") or create_response.json().get("session_id")
        await async_client.post(f"/api/v1/sessions/{session_id}/activate")

        player_id = generate_player_id()

        # Act
        response = await async_client.post(
            f"/api/v1/sessions/{session_id}/players",
            json={"player_id": player_id},
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "message" in data or "player_id" in data
    
    async def test_remove_player_from_session(self, async_client: AsyncClient):
        """Test removing a player from a session"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create session and add player
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        session_id = create_response.json().get("id") or create_response.json().get("session_id")
        await async_client.post(f"/api/v1/sessions/{session_id}/activate")

        player_id = generate_player_id()
        await async_client.post(
            f"/api/v1/sessions/{session_id}/players",
            json={"player_id": player_id},
        )

        # Act
        response = await async_client.delete(
            f"/api/v1/sessions/{session_id}/players/{player_id}",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "message" in data or "player_id" in data
    
    async def test_update_session_metrics(self, async_client: AsyncClient):
        """Test updating session metrics"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create and activate a session
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        session_id = create_response.json().get("id") or create_response.json().get("session_id")
        await async_client.post(f"/api/v1/sessions/{session_id}/activate")

        metrics_data = {
            "avg_latency_ms": 45.5,
            "packet_loss_rate": 0.01,
        }

        # Act
        response = await async_client.patch(
            f"/api/v1/sessions/{session_id}/metrics",
            json=metrics_data,
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "message" in data or "status" in data

    async def test_get_active_sessions_by_host(self, async_client: AsyncClient):
        """Test getting active sessions for a host"""
        from tests.utils.helpers import create_test_zone

        # Arrange - Create and activate a session
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        create_response = await async_client.post("/api/v1/sessions/", json=session_data)
        session_id = create_response.json().get("id") or create_response.json().get("session_id")
        await async_client.post(f"/api/v1/sessions/{session_id}/activate")

        # Act
        response = await async_client.get(
            f"/api/v1/sessions/host/{host_data['host_id']}/active",
        )

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)

