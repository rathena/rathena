"""
End-to-End Integration Tests for C++ ↔ Python HTTP Communication
Tests the complete bidirectional flow: NPC Script → C++ Plugin → Python Service → Response
"""

import pytest
import httpx
import json
from typing import Dict, Any
import asyncio
from datetime import datetime


# Test configuration
BASE_URL = "http://127.0.0.1:8000"
TIMEOUT = 5.0  # Longer than C++ client's 3s timeout for testing


class TestHealthCheck:
    """Test health check endpoint"""
    
    @pytest.mark.asyncio
    async def test_health_endpoint(self):
        """Verify service health endpoint is accessible"""
        async with httpx.AsyncClient() as client:
            response = await client.get(f"{BASE_URL}/health", timeout=TIMEOUT)
            
            assert response.status_code == 200
            data = response.json()
            assert data["status"] == "healthy"
            assert "service" in data
            assert "version" in data
    
    @pytest.mark.asyncio
    async def test_detailed_health_endpoint(self):
        """Verify detailed health check"""
        async with httpx.AsyncClient() as client:
            response = await client.get(f"{BASE_URL}/health/detailed", timeout=TIMEOUT)
            
            assert response.status_code == 200
            data = response.json()
            assert "status" in data
            assert "database" in data


class TestNPCRegistration:
    """Test NPC registration flow (Point 1: NPC scripts → C++ → Python)"""
    
    @pytest.mark.asyncio
    async def test_npc_register_success(self):
        """Test successful NPC registration"""
        async with httpx.AsyncClient() as client:
            npc_data = {
                "npc_id": "TEST_NPC_001",
                "name": "Test Guard",
                "personality": {"trait": "brave", "mood": "neutral"},
                "initial_goals": ["patrol", "protect"]
            }
            
            response = await client.post(
                f"{BASE_URL}/api/npc/register",
                json=npc_data,
                timeout=TIMEOUT
            )
            
            assert response.status_code in [200, 201]
            data = response.json()
            assert "npc_id" in data or "status" in data
    
    @pytest.mark.asyncio
    async def test_npc_register_invalid_data(self):
        """Test NPC registration with invalid data"""
        async with httpx.AsyncClient() as client:
            invalid_data = {
                "npc_id": "",  # Invalid: empty ID
                "name": "Test"
            }
            
            response = await client.post(
                f"{BASE_URL}/api/npc/register",
                json=invalid_data,
                timeout=TIMEOUT
            )
            
            # Should return error (400 or 422 validation error)
            assert response.status_code in [400, 422]


class TestNPCInteraction:
    """Test player-NPC interaction flow (Point 2: C++ → Python AI processing)"""
    
    @pytest.mark.asyncio
    async def test_npc_interact_chat(self):
        """Test NPC interaction with player message"""
        async with httpx.AsyncClient() as client:
            interaction_data = {
                "npc_id": "TEST_NPC_002",
                "player_id": "PLAYER_001",
                "interaction_type": "chat",
                "message": "Hello, how are you?"
            }
            
            response = await client.post(
                f"{BASE_URL}/api/npc/TEST_NPC_002/interact",
                json=interaction_data,
                timeout=TIMEOUT
            )
            
            # Should process interaction successfully
            assert response.status_code == 200
            data = response.json()
            
            # Response should contain AI-generated reply
            assert "response" in data or "message" in data or "dialogue" in data
    
    @pytest.mark.asyncio
    async def test_npc_get_state(self):
        """Test retrieving NPC state (Point 3: Python → C++ response)"""
        async with httpx.AsyncClient() as client:
            response = await client.get(
                f"{BASE_URL}/api/npc/TEST_NPC_002/state",
                timeout=TIMEOUT
            )
            
            assert response.status_code == 200
            data = response.json()
            assert "npc_id" in data or "state" in data


class TestNPCMovement:
    """Test AI-driven NPC movement decisions"""
    
    @pytest.mark.asyncio
    async def test_movement_decision_approved(self):
        """Test movement request that should be approved"""
        async with httpx.AsyncClient() as client:
            movement_data = {
                "npc_id": "Guard#001",
                "action": "move",
                "target_x": 100,
                "target_y": 150,
                "current_x": 95,
                "current_y": 145
            }
            
            response = await client.post(
                f"{BASE_URL}/api/npc/movement",
                json=movement_data,
                timeout=TIMEOUT
            )
            
            assert response.status_code == 200
            data = response.json()
            
            # Verify response structure
            assert "approved" in data
            assert "target_x" in data
            assert "target_y" in data
            assert "reasoning" in data
            assert data["target_x"] == 100
            assert data["target_y"] == 150
    
    @pytest.mark.asyncio
    async def test_movement_decision_invalid_coordinates(self):
        """Test movement request with invalid coordinates"""
        async with httpx.AsyncClient() as client:
            movement_data = {
                "npc_id": "Guard#002",
                "action": "move",
                "target_x": 9999,  # Exceeds map boundaries
                "target_y": 9999
            }
            
            response = await client.post(
                f"{BASE_URL}/api/npc/movement",
                json=movement_data,
                timeout=TIMEOUT
            )
            
            assert response.status_code == 200
            data = response.json()
            
            # Movement should be denied
            assert data["approved"] == False
            assert "reasoning" in data
            assert "boundaries" in data["reasoning"].lower()
    
    @pytest.mark.asyncio
    async def test_movement_status_query(self):
        """Test querying NPC movement status"""
        async with httpx.AsyncClient() as client:
            response = await client.get(
                f"{BASE_URL}/api/npc/movement/status/Guard#001",
                timeout=TIMEOUT
            )
            
            assert response.status_code == 200
            data = response.json()
            assert "npc_id" in data
            assert "status" in data


class TestErrorHandling:
    """Test error handling and resilience"""
    
    @pytest.mark.asyncio
    async def test_malformed_json(self):
        """Test handling of malformed JSON"""
        async with httpx.AsyncClient() as client:
            # Send invalid JSON
            response = await client.post(
                f"{BASE_URL}/api/npc/movement",
                content="{ invalid json }",
                headers={"Content-Type": "application/json"},
                timeout=TIMEOUT
            )
            
            # Should return 422 validation error
            assert response.status_code == 422
    
    @pytest.mark.asyncio
    async def test_not_found_endpoint(self):
        """Test 404 handling for non-existent endpoints"""
        async with httpx.AsyncClient() as client:
            response = await client.get(
                f"{BASE_URL}/api/nonexistent/endpoint",
                timeout=TIMEOUT
            )
            
            assert response.status_code == 404
    
    @pytest.mark.asyncio
    async def test_timeout_handling(self):
        """Test client timeout behavior"""
        async with httpx.AsyncClient() as client:
            try:
                # Use very short timeout to force timeout
                response = await client.get(
                    f"{BASE_URL}/health",
                    timeout=0.001  # 1ms - should timeout
                )
                # If it doesn't timeout, that's also acceptable
                assert response.status_code == 200
            except httpx.TimeoutException:
                # Expected timeout
                pass


class TestConcurrency:
    """Test concurrent request handling"""
    
    @pytest.mark.asyncio
    async def test_concurrent_health_checks(self):
        """Test service under concurrent load"""
        async with httpx.AsyncClient() as client:
            # Create 50 concurrent requests
            tasks = [
                client.get(f"{BASE_URL}/health", timeout=TIMEOUT)
                for _ in range(50)
            ]
            
            responses = await asyncio.gather(*tasks, return_exceptions=True)
            
            # Count successful responses
            successful = sum(
                1 for r in responses 
                if not isinstance(r, Exception) and r.status_code == 200
            )
            
            # At least 90% should succeed
            assert successful >= 45
    
    @pytest.mark.asyncio
    async def test_concurrent_movement_decisions(self):
        """Test concurrent NPC movement decisions"""
        async with httpx.AsyncClient() as client:
            # Create 20 concurrent movement requests
            tasks = []
            for i in range(20):
                movement_data = {
                    "npc_id": f"Guard#{i:03d}",
                    "action": "move",
                    "target_x": 100 + i,
                    "target_y": 150 + i
                }
                tasks.append(
                    client.post(
                        f"{BASE_URL}/api/npc/movement",
                        json=movement_data,
                        timeout=TIMEOUT
                    )
                )
            
            responses = await asyncio.gather(*tasks, return_exceptions=True)
            
            # Count successful responses
            successful = sum(
                1 for r in responses 
                if not isinstance(r, Exception) and r.status_code == 200
            )
            
            # At least 95% should succeed
            assert successful >= 19


class TestEndToEndFlow:
    """Test complete end-to-end integration flows"""
    
    @pytest.mark.asyncio
    async def test_complete_npc_lifecycle(self):
        """
        Test complete NPC lifecycle:
        1. Register NPC
        2. Process interaction
        3. Request movement
        4. Query state
        """
        async with httpx.AsyncClient() as client:
            npc_id = f"LIFECYCLE_TEST_{datetime.now().timestamp()}"
            
            # Step 1: Register NPC
            register_data = {
                "npc_id": npc_id,
                "name": "Lifecycle Test NPC",
                "personality": {"mood": "friendly"},
                "initial_goals": ["greet_players"]
            }
            
            reg_response = await client.post(
                f"{BASE_URL}/api/npc/register",
                json=register_data,
                timeout=TIMEOUT
            )
            assert reg_response.status_code in [200, 201]
            
            # Step 2: Process interaction
            interact_data = {
                "npc_id": npc_id,
                "player_id": "TEST_PLAYER",
                "interaction_type": "chat",
                "message": "Hello!"
            }
            
            interact_response = await client.post(
                f"{BASE_URL}/api/npc/{npc_id}/interact",
                json=interact_data,
                timeout=TIMEOUT
            )
            assert interact_response.status_code == 200
            
            # Step 3: Request movement
            movement_data = {
                "npc_id": npc_id,
                "action": "move",
                "target_x": 120,
                "target_y": 180
            }
            
            movement_response = await client.post(
                f"{BASE_URL}/api/npc/movement",
                json=movement_data,
                timeout=TIMEOUT
            )
            assert movement_response.status_code == 200
            movement_result = movement_response.json()
            assert "approved" in movement_result
            
            # Step 4: Query state
            state_response = await client.get(
                f"{BASE_URL}/api/npc/{npc_id}/state",
                timeout=TIMEOUT
            )
            assert state_response.status_code == 200


class TestPerformance:
    """Performance and latency tests"""
    
    @pytest.mark.asyncio
    async def test_health_check_latency(self):
        """Verify health check responds quickly"""
        async with httpx.AsyncClient() as client:
            start = asyncio.get_event_loop().time()
            response = await client.get(f"{BASE_URL}/health", timeout=TIMEOUT)
            latency = (asyncio.get_event_loop().time() - start) * 1000  # ms
            
            assert response.status_code == 200
            assert latency < 100  # Should respond in < 100ms
    
    @pytest.mark.asyncio
    async def test_movement_decision_latency(self):
        """Verify movement decisions respond within acceptable time"""
        async with httpx.AsyncClient() as client:
            movement_data = {
                "npc_id": "PERF_TEST_NPC",
                "action": "move",
                "target_x": 100,
                "target_y": 150
            }
            
            latencies = []
            for _ in range(10):
                start = asyncio.get_event_loop().time()
                response = await client.post(
                    f"{BASE_URL}/api/npc/movement",
                    json=movement_data,
                    timeout=TIMEOUT
                )
                latency = (asyncio.get_event_loop().time() - start) * 1000
                latencies.append(latency)
                
                assert response.status_code == 200
            
            # Calculate p95 latency
            latencies.sort()
            p95_latency = latencies[int(len(latencies) * 0.95)]
            
            # p95 should be < 500ms (C++ has 3000ms timeout)
            assert p95_latency < 500, f"p95 latency {p95_latency}ms exceeds 500ms target"


# Pytest fixtures
@pytest.fixture(scope="module")
async def cleanup_test_npcs():
    """Cleanup test NPCs after tests complete"""
    yield
    # Add cleanup logic here if needed
    pass


if __name__ == "__main__":
    pytest.main([__file__, "-v", "--tb=short"])