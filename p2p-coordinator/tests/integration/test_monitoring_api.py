"""
Integration tests for Monitoring API endpoints
"""

import pytest
from httpx import AsyncClient


@pytest.mark.integration
@pytest.mark.asyncio
class TestMonitoringAPI:
    """Test suite for Monitoring API endpoints"""
    
    @pytest.mark.skip(reason="Health endpoint not implemented yet")
    async def test_health_check(self, async_client: AsyncClient):
        """Test health check endpoint"""
        # Act
        response = await async_client.get("/api/v1/monitoring/health")

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "status" in data
        assert data["status"] in ["healthy", "degraded", "unhealthy"]
    
    async def test_dashboard_metrics(self, async_client: AsyncClient):
        """Test dashboard metrics endpoint"""
        # Act
        response = await async_client.get("/api/v1/monitoring/dashboard")

        # Assert
        assert response.status_code == 200
        data = response.json()
        # Check nested structure
        assert "hosts" in data
        assert "sessions" in data
        assert "zones" in data
        assert "system" in data
        assert isinstance(data["hosts"]["total"], int)
        assert isinstance(data["sessions"]["active"], int)
        assert isinstance(data["sessions"]["total_players"], int)
    
    async def test_signaling_stats(self, async_client: AsyncClient):
        """Test signaling statistics endpoint"""
        # Act
        response = await async_client.get("/api/v1/signaling/stats")

        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "active_connections" in data
        assert "active_sessions" in data  # Changed from total_sessions
        assert isinstance(data["active_connections"], int)
        assert isinstance(data["active_sessions"], int)
    
    async def test_root_endpoint(self, async_client: AsyncClient):
        """Test root endpoint returns service information"""
        # Act
        response = await async_client.get("/")
        
        # Assert
        assert response.status_code == 200
        data = response.json()
        assert "service" in data
        assert "version" in data
        assert "status" in data
        assert data["service"] == "p2p-coordinator"

