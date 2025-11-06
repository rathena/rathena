"""
Unit tests for API routers
"""

import pytest
from fastapi.testclient import TestClient
from unittest.mock import AsyncMock, patch
from main import app


class TestHealthEndpoints:
    """Test health check endpoints"""
    
    def test_health_endpoint(self):
        """Test lightweight health endpoint"""
        client = TestClient(app)
        response = client.get("/health")
        
        assert response.status_code == 200
        data = response.json()
        assert data["status"] == "healthy"
    
    def test_detailed_health_endpoint(self):
        """Test detailed health endpoint"""
        client = TestClient(app)
        
        with patch('main.db') as mock_db:
            mock_db.client = AsyncMock()
            mock_db.client.ping = AsyncMock(return_value=True)
            
            response = client.get("/health/detailed")
            
            assert response.status_code == 200
            data = response.json()
            assert "status" in data
            assert "database" in data
            assert "llm_provider" in data


class TestNPCRouter:
    """Test NPC router endpoints"""
    
    def test_register_npc_success(self):
        """Test successful NPC registration"""
        client = TestClient(app)
        
        npc_data = {
            "npc_id": "test_npc_001",
            "name": "Test Merchant",
            "level": 50,
            "npc_class": "merchant",
            "position": {"map": "prontera", "x": 150, "y": 180},
            "personality": {
                "traits": ["friendly", "helpful"],
                "background": "A helpful merchant",
                "goals": ["Make profit"],
                "speech_style": "Cheerful"
            }
        }
        
        with patch('routers.npc.db') as mock_db:
            mock_db.set_npc_state = AsyncMock()
            
            response = client.post("/ai/npc/register", json=npc_data)
            
            assert response.status_code == 200
            data = response.json()
            assert data["npc_id"] == "test_npc_001"
    
    def test_register_npc_validation_errors(self):
        """Test NPC registration validation"""
        client = TestClient(app)
        
        # Test invalid level
        invalid_data = {
            "npc_id": "test_npc",
            "name": "Test",
            "level": 1000,  # Exceeds max_npc_level
            "npc_class": "merchant",
            "position": {"map": "prontera", "x": 150, "y": 180}
        }
        
        response = client.post("/ai/npc/register", json=invalid_data)
        assert response.status_code == 400
    
    def test_npc_action_validation(self):
        """Test NPC action validation"""
        client = TestClient(app)
        
        # Test invalid movement distance
        action_data = {
            "action_type": "move",
            "target_x": 1000,  # Too far
            "target_y": 1000
        }
        
        with patch('routers.npc.db') as mock_db:
            mock_db.get_npc_state = AsyncMock(return_value={
                "position": {"x": 100, "y": 100}
            })
            
            response = client.post("/ai/npc/test_npc/execute-action", json=action_data)
            
            # Should reject due to distance validation
            assert response.status_code in [400, 404]


class TestChatCommandRouter:
    """Test chat command router"""
    
    def test_chat_command_rate_limiting(self):
        """Test chat command rate limiting"""
        client = TestClient(app)
        
        command_data = {
            "player_id": "player_001",
            "npc_id": "npc_001",
            "message": "Hello",
            "player_name": "TestPlayer",
            "player_level": 50,
            "player_position": {"map": "prontera", "x": 150, "y": 180}
        }
        
        with patch('routers.chat_command.db') as mock_db:
            # First request - not rate limited
            mock_db.client.exists = AsyncMock(return_value=0)
            mock_db.client.setex = AsyncMock()
            
            with patch('routers.chat_command.handle_player_interaction') as mock_handler:
                mock_handler.return_value = {
                    "npc_response": "Hello, adventurer!",
                    "emotion": "happy"
                }
                
                response = client.post("/ai/chat/command", json=command_data)
                assert response.status_code == 200
            
            # Second request - rate limited
            mock_db.client.exists = AsyncMock(return_value=1)
            mock_db.client.ttl = AsyncMock(return_value=5)
            
            response = client.post("/ai/chat/command", json=command_data)
            assert response.status_code == 429  # Too Many Requests


class TestPlayerRouter:
    """Test player interaction router"""
    
    def test_player_interaction_success(self):
        """Test successful player interaction"""
        client = TestClient(app)
        
        interaction_data = {
            "player_id": "player_001",
            "player_name": "TestHero",
            "player_level": 75,
            "npc_id": "npc_001",
            "interaction_type": "talk",
            "message": "Hello, merchant!",
            "context": {
                "location": {"map": "prontera", "x": 150, "y": 180}
            }
        }
        
        with patch('routers.player.db') as mock_db:
            mock_db.get_npc_state = AsyncMock(return_value={
                "npc_id": "npc_001",
                "name": "Test Merchant",
                "personality": {}
            })
            
            with patch('routers.player.AgentOrchestrator') as mock_orchestrator:
                mock_instance = AsyncMock()
                mock_instance.process_interaction = AsyncMock(return_value={
                    "dialogue": "Greetings, brave hero!",
                    "emotion": "friendly"
                })
                mock_orchestrator.return_value = mock_instance
                
                response = client.post("/ai/player/interact", json=interaction_data)
                
                assert response.status_code == 200
                data = response.json()
                assert "npc_response" in data


class TestInputValidation:
    """Test input validation across all endpoints"""
    
    def test_npc_id_validation(self):
        """Test NPC ID validation"""
        client = TestClient(app)
        
        # Empty NPC ID
        invalid_data = {
            "npc_id": "",
            "name": "Test",
            "level": 50,
            "npc_class": "merchant",
            "position": {"map": "prontera", "x": 150, "y": 180}
        }
        
        response = client.post("/ai/npc/register", json=invalid_data)
        assert response.status_code == 422  # Validation error
    
    def test_position_validation(self):
        """Test position coordinate validation"""
        client = TestClient(app)
        
        # Negative coordinates
        invalid_data = {
            "npc_id": "test_npc",
            "name": "Test",
            "level": 50,
            "npc_class": "merchant",
            "position": {"map": "prontera", "x": -10, "y": 180}
        }
        
        response = client.post("/ai/npc/register", json=invalid_data)
        assert response.status_code == 400

