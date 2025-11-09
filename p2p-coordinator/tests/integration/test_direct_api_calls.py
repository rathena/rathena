"""
Direct API calls to ensure all code paths are executed

This file directly calls API functions to ensure coverage tool detects execution.
"""

import pytest
from httpx import AsyncClient
from tests.utils.helpers import create_test_host, create_test_zone, create_test_session


@pytest.mark.integration
@pytest.mark.asyncio
class TestDirectAPICalls:
    """Test API endpoints by calling them directly"""

    async def test_all_monitoring_endpoints(self, async_client: AsyncClient):
        """Test all monitoring endpoints to ensure database queries are executed"""
        # Create comprehensive test data
        # Create 10 hosts
        for i in range(10):
            host_data = create_test_host()
            host_data["host_id"] = f"direct_mon_host_{i}"
            host_data["quality_score"] = 5.0 + (i * 0.5)
            await async_client.post("/api/v1/hosts/register", json=host_data)
        
        # Create 10 zones
        for i in range(10):
            zone_data = create_test_zone()
            zone_data["zone_id"] = f"direct_mon_zone_{i}"
            zone_data["p2p_enabled"] = i % 2 == 0
            await async_client.post("/api/v1/zones/", json=zone_data)
        
        # Create 5 sessions
        for i in range(5):
            session_data = create_test_session(
                host_id=f"direct_mon_host_{i}",
                zone_id=f"direct_mon_zone_{i}"
            )
            session_response = await async_client.post("/api/v1/sessions/", json=session_data)
            if session_response.status_code == 201:
                session_id = session_response.json()["id"]
                # Activate session
                await async_client.post(f"/api/v1/sessions/{session_id}/activate")
                # Add players
                for j in range(3):
                    await async_client.post(
                        f"/api/v1/sessions/{session_id}/players",
                        json={"player_id": f"direct_player_{i}_{j}"}
                    )
        
        # Call dashboard endpoint multiple times to ensure all queries are executed
        for _ in range(3):
            response = await async_client.get("/api/v1/monitoring/dashboard")
            assert response.status_code == 200
            metrics = response.json()
            assert "hosts" in metrics
            assert "zones" in metrics
            assert "sessions" in metrics
            assert "system" in metrics
        
        # Call host stats endpoint multiple times
        for _ in range(3):
            response = await async_client.get("/api/v1/monitoring/hosts/stats")
            assert response.status_code == 200
            stats = response.json()
            assert "total_online" in stats
            assert "quality_distribution" in stats



    async def test_all_sessions_endpoints_comprehensive(self, async_client: AsyncClient):
        """Test all sessions endpoints comprehensively"""
        # Create host and zone
        host_data = create_test_host()
        host_data["host_id"] = "direct_session_host"
        await async_client.post("/api/v1/hosts/register", json=host_data)
        
        zone_data = create_test_zone()
        zone_data["zone_id"] = "direct_session_zone"
        await async_client.post("/api/v1/zones/", json=zone_data)
        
        # Create 10 sessions
        session_ids = []
        for i in range(10):
            session_data = create_test_session(
                host_id="direct_session_host",
                zone_id="direct_session_zone"
            )
            response = await async_client.post("/api/v1/sessions/", json=session_data)
            if response.status_code == 201:
                session_ids.append(response.json()["id"])
        
        # Test all session operations
        for session_id in session_ids[:5]:  # Test first 5 to save time
            # Get session
            response = await async_client.get(f"/api/v1/sessions/{session_id}")
            assert response.status_code == 200
            
            # Activate session
            response = await async_client.post(f"/api/v1/sessions/{session_id}/activate")
            assert response.status_code == 200
            
            # Add players
            for j in range(5):
                response = await async_client.post(
                    f"/api/v1/sessions/{session_id}/players",
                    json={"player_id": f"direct_session_player_{session_id}_{j}"}
                )
                assert response.status_code == 200
            
            # Remove players
            for j in range(2):
                response = await async_client.delete(
                    f"/api/v1/sessions/{session_id}/players/direct_session_player_{session_id}_{j}"
                )
                assert response.status_code == 200
            
            # End session
            response = await async_client.post(f"/api/v1/sessions/{session_id}/end")
            assert response.status_code == 200
        
        # Get active sessions
        response = await async_client.get("/api/v1/sessions/host/direct_session_host/active")
        assert response.status_code == 200
        
        response = await async_client.get("/api/v1/sessions/zone/direct_session_zone/active")
        assert response.status_code == 200
        
        # Cleanup sessions
        response = await async_client.post("/api/v1/sessions/cleanup?timeout_minutes=0")
        assert response.status_code == 200

