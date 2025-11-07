"""
Integration tests for AI Autonomous World System
Tests Bridge Layer ↔ AI Service communication
"""

import pytest
import httpx
import asyncio
from datetime import datetime


# Test configuration
BRIDGE_LAYER_URL = "http://localhost:8888"
AI_SERVICE_URL = "http://localhost:8000"


class TestAIServiceHealth:
    """Test AI Service health and availability"""
    
    @pytest.mark.asyncio
    async def test_ai_service_health(self):
        """Test AI Service health endpoint"""
        async with httpx.AsyncClient() as client:
            response = await client.get(f"{AI_SERVICE_URL}/health")
            assert response.status_code == 200
            data = response.json()
            assert data["status"] in ["healthy", "degraded"]
            assert "database" in data
    
    @pytest.mark.asyncio
    async def test_ai_service_root(self):
        """Test AI Service root endpoint"""
        async with httpx.AsyncClient() as client:
            response = await client.get(f"{AI_SERVICE_URL}/")
            assert response.status_code == 200
            data = response.json()
            assert data["status"] == "running"


class TestNPCEndpoints:
    """Test NPC-related endpoints"""
    
    @pytest.mark.asyncio
    async def test_npc_registration(self):
        """Test NPC registration via AI Service"""
        npc_data = {
            "npc_id": "test_npc_001",
            "name": "Test Merchant",
            "npc_class": "merchant",
            "level": 50,
            "position": {
                "map": "prontera",
                "x": 150,
                "y": 180
            },
            "personality": {
                "openness": 0.7,
                "conscientiousness": 0.8,
                "extraversion": 0.6,
                "agreeableness": 0.9,
                "neuroticism": 0.3,
                "moral_alignment": "lawful_good"
            }
        }
        
        async with httpx.AsyncClient() as client:
            response = await client.post(
                f"{AI_SERVICE_URL}/ai/npc/register",
                json=npc_data
            )
            assert response.status_code == 200
            data = response.json()
            assert data["status"] == "success"
            assert "agent_id" in data
            assert data["npc_id"] == "test_npc_001"
    
    @pytest.mark.asyncio
    async def test_npc_event(self):
        """Test sending NPC event"""
        event_data = {
            "npc_id": "test_npc_001",
            "event_type": "social",
            "event_data": {
                "action": "greeting",
                "target": "player_001"
            },
            "timestamp": datetime.utcnow().isoformat(),
            "context": {}
        }
        
        async with httpx.AsyncClient() as client:
            response = await client.post(
                f"{AI_SERVICE_URL}/ai/npc/event",
                json=event_data
            )
            assert response.status_code == 200
            data = response.json()
            assert data["status"] == "queued"
            assert "event_id" in data
    
    @pytest.mark.asyncio
    async def test_npc_action(self):
        """Test getting NPC action"""
        async with httpx.AsyncClient() as client:
            response = await client.get(
                f"{AI_SERVICE_URL}/ai/npc/test_npc_001/action"
            )
            assert response.status_code == 200
            data = response.json()
            assert "action" in data
            assert data["npc_id"] == "test_npc_001"


class TestWorldEndpoints:
    """Test world state endpoints"""
    
    @pytest.mark.asyncio
    async def test_world_state_update(self):
        """Test updating world state"""
        state_data = {
            "state_type": "economy",
            "state_data": {
                "item_prices": {"potion": 500, "sword": 10000},
                "trade_volume": 1000000.0,
                "inflation_rate": 0.02
            },
            "timestamp": datetime.utcnow().isoformat(),
            "source": "test"
        }
        
        async with httpx.AsyncClient() as client:
            response = await client.post(
                f"{AI_SERVICE_URL}/ai/world/state",
                json=state_data
            )
            assert response.status_code == 200
            data = response.json()
            assert data["status"] == "success"
            assert data["state_type"] == "economy"
    
    @pytest.mark.asyncio
    async def test_world_state_query(self):
        """Test querying world state"""
        async with httpx.AsyncClient() as client:
            response = await client.get(
                f"{AI_SERVICE_URL}/ai/world/state?state_type=economy"
            )
            assert response.status_code == 200
            data = response.json()
            assert data["state_type"] == "economy"


class TestPlayerEndpoints:
    """Test player interaction endpoints"""
    
    @pytest.mark.asyncio
    async def test_player_interaction(self):
        """Test player-NPC interaction"""
        interaction_data = {
            "npc_id": "test_npc_001",
            "player_id": "player_001",
            "interaction_type": "talk",
            "message": "Hello!",
            "timestamp": datetime.utcnow().isoformat(),
            "context": {
                "player_name": "TestPlayer",
                "player_class": "Swordsman",
                "player_level": 50,
                "location": {"map": "prontera", "x": 150, "y": 180},
                "time_of_day": "day",
                "weather": "clear"
            }
        }
        
        async with httpx.AsyncClient(timeout=30.0) as client:
            response = await client.post(
                f"{AI_SERVICE_URL}/ai/player/interaction",
                json=interaction_data
            )
            assert response.status_code == 200
            data = response.json()
            assert "response" in data
            assert data["npc_id"] == "test_npc_001"
            assert data["player_id"] == "player_001"


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])

