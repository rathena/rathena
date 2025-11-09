"""
Complete integration tests for Monitoring API

This file contains comprehensive tests to achieve 100% coverage of the monitoring API.
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, create_test_zone, create_test_session


@pytest.mark.integration
@pytest.mark.asyncio
class TestMonitoringAPIComplete:
    """Complete coverage tests for Monitoring API"""

    async def test_dashboard_metrics_with_data(self, async_client: AsyncClient):
        """Test dashboard metrics with hosts, zones, and sessions"""
        # Create test data
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        zone_data = create_test_zone()
        await async_client.post("/api/v1/zones/", json=zone_data)

        session_data = create_test_session(host_id=host_data["host_id"], zone_id=zone_data["zone_id"])
        await async_client.post("/api/v1/sessions/", json=session_data)

        # Get dashboard metrics
        response = await async_client.get("/api/v1/monitoring/dashboard")
        assert response.status_code == 200

        data = response.json()
        assert "hosts" in data
        assert "zones" in data
        assert "sessions" in data
        assert "system" in data

        # Verify host metrics
        assert data["hosts"]["total"] >= 1
        assert "online" in data["hosts"]
        assert "offline" in data["hosts"]
        assert "average_quality_score" in data["hosts"]

        # Verify zone metrics
        assert data["zones"]["total"] >= 1
        assert "enabled" in data["zones"]
        assert "disabled" in data["zones"]

        # Verify session metrics
        assert data["sessions"]["total"] >= 1
        assert "active" in data["sessions"]
        assert "inactive" in data["sessions"]
        assert "total_players" in data["sessions"]
        assert "average_latency_ms" in data["sessions"]

        # Verify system metrics
        assert data["system"]["postgres"] == "healthy"
        assert data["system"]["redis"] in ["healthy", "unhealthy"]

    async def test_host_stats(self, async_client: AsyncClient):
        """Test host statistics endpoint"""
        # Create test host
        host_data = create_test_host()
        await async_client.post("/api/v1/hosts/register", json=host_data)

        response = await async_client.get("/api/v1/monitoring/hosts/stats")
        assert response.status_code == 200

        data = response.json()
        assert "total_online" in data
        assert "quality_distribution" in data
        assert "excellent" in data["quality_distribution"]
        assert "good" in data["quality_distribution"]
        assert "fair" in data["quality_distribution"]
        assert "poor" in data["quality_distribution"]

    async def test_dashboard_database_error(self, async_client: AsyncClient, monkeypatch):
        """Test dashboard with database error"""
        from sqlalchemy.ext.asyncio import AsyncSession

        async def mock_execute_error(*args, **kwargs):
            raise Exception("Database query failed")

        # Monkeypatch the execute method of AsyncSession
        monkeypatch.setattr(AsyncSession, "execute", mock_execute_error)

        response = await async_client.get("/api/v1/monitoring/dashboard")
        assert response.status_code == 200  # Returns 200 with error in response

        data = response.json()
        assert "error" in data
        assert data["hosts"]["total"] == 0

    async def test_host_stats_database_error(self, async_client: AsyncClient, monkeypatch):
        """Test host stats with database error"""
        from sqlalchemy.ext.asyncio import AsyncSession

        async def mock_execute_error(*args, **kwargs):
            raise Exception("Database query failed")

        monkeypatch.setattr(AsyncSession, "execute", mock_execute_error)

        response = await async_client.get("/api/v1/monitoring/hosts/stats")
        assert response.status_code == 200  # Returns 200 with error in response

        data = response.json()
        assert "error" in data


