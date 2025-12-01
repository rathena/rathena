"""
Unit tests for NPC router endpoints

Tests NPC registration, updates, actions, and state management with
mocked dependencies and comprehensive error handling.
"""

import pytest
from fastapi.testclient import TestClient
from unittest.mock import AsyncMock, patch, MagicMock
from datetime import datetime, UTC

from main import app


@pytest.fixture
def client():
    """FastAPI test client"""
    return TestClient(app)


@pytest.fixture
def sample_npc_registration():
    """Sample NPC registration data"""
    return {
        "npc_id": "test_merchant_001",
        "name": "Wealthy Merchant",
        "level": 65,
        "npc_class": "merchant",
        "position": {"map": "prontera", "x": 150, "y": 180},
        "personality": {
            "traits": ["friendly", "greedy", "talkative"],
            "background": {
                "summary": "A wealthy merchant from the capital",
                "history": ["Started as a street vendor", "Built trading empire"]
            },
            "goals": ["Maximize profit", "Expand trade routes"],
            "speech_style": "Cheerful and persuasive"
        }
    }


@pytest.fixture
def sample_npc_update():
    """Sample NPC state update"""
    return {
        "position": {"map": "prontera", "x": 155, "y": 185},
        "mood": "happy",
        "inventory_count": 50
    }


@pytest.mark.router
@pytest.mark.npc
class TestNPCRegistration:
    """Test NPC registration endpoints"""

    def test_register_npc_success(self, client, sample_npc_registration):
        """Test successful NPC registration"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.set = AsyncMock(return_value=True)
            mock_db.hset = AsyncMock(return_value=1)

            response = client.post("/ai/npc/register", json=sample_npc_registration)

            assert response.status_code == 200
            data = response.json()
            assert data["npc_id"] == "test_merchant_001"
            assert data["name"] == "Wealthy Merchant"
            assert "registered_at" in data

    def test_register_npc_duplicate(self, client, sample_npc_registration):
        """Test duplicate NPC registration rejection"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)

            response = client.post("/ai/npc/register", json=sample_npc_registration)

            assert response.status_code == 409  # Conflict

    def test_register_npc_invalid_level(self, client, sample_npc_registration):
        """Test NPC registration with invalid level"""
        sample_npc_registration["level"] = 1000  # Exceeds max_npc_level

        response = client.post("/ai/npc/register", json=sample_npc_registration)

        assert response.status_code == 422  # Validation error

    def test_register_npc_invalid_position(self, client, sample_npc_registration):
        """Test NPC registration with invalid position"""
        sample_npc_registration["position"]["x"] = -10  # Negative coordinate

        response = client.post("/ai/npc/register", json=sample_npc_registration)

        assert response.status_code in [422, 500]  # Validation or custom error

    def test_register_npc_missing_required_fields(self, client):
        """Test NPC registration with missing required fields"""
        invalid_data = {
            "npc_id": "test_npc",
            "name": "Test"
            # Missing level, npc_class, position
        }

        response = client.post("/ai/npc/register", json=invalid_data)

        assert response.status_code == 422


@pytest.mark.router
@pytest.mark.npc
class TestNPCStateManagement:
    """Test NPC state management endpoints"""

    def test_get_npc_state_success(self, client):
        """Test successful NPC state retrieval"""
        with patch('routers.npc.db.client') as mock_db:
            mock_state = {
                "npc_id": "npc_001",
                "name": "Test Merchant",
                "position": {"map": "prontera", "x": 150, "y": 180},
                "mood": "neutral"
            }
            mock_db.get = AsyncMock(return_value=str(mock_state))

            response = client.get("/ai/npc/npc_001/state")

            assert response.status_code == 200
            data = response.json()
            assert data["npc_id"] == "npc_001"

    def test_get_npc_state_not_found(self, client):
        """Test NPC state retrieval for non-existent NPC"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value=None)

            response = client.get("/ai/npc/nonexistent_npc/state")

            assert response.status_code == 404

    def test_update_npc_state_success(self, client, sample_npc_update):
        """Test successful NPC state update"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)
            mock_db.set = AsyncMock(return_value=True)
            mock_db.hset = AsyncMock(return_value=1)

            response = client.put("/ai/npc/npc_001/state", json=sample_npc_update)

            assert response.status_code == 200
            data = response.json()
            assert "updated_at" in data

    def test_update_npc_state_not_found(self, client, sample_npc_update):
        """Test NPC state update for non-existent NPC"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=0)

            response = client.put("/ai/npc/nonexistent/state", json=sample_npc_update)

            assert response.status_code == 404


@pytest.mark.router
@pytest.mark.npc
class TestNPCActions:
    """Test NPC action execution endpoints"""

    def test_execute_action_move_success(self, client):
        """Test successful movement action"""
        action_data = {
            "action_type": "move",
            "target_x": 155,
            "target_y": 185
        }

        with patch('routers.npc.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"position": {"x": 150, "y": 180}}')
            mock_db.set = AsyncMock(return_value=True)

            response = client.post("/ai/npc/npc_001/execute-action", json=action_data)

            assert response.status_code == 200
            data = response.json()
            assert data["action_type"] == "move"

    def test_execute_action_invalid_distance(self, client):
        """Test action with invalid movement distance"""
        action_data = {
            "action_type": "move",
            "target_x": 1000,  # Too far
            "target_y": 1000
        }

        with patch('routers.npc.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"position": {"x": 100, "y": 100}}')

            response = client.post("/ai/npc/npc_001/execute-action", json=action_data)

            assert response.status_code in [400, 422, 404]

    def test_execute_action_unknown_type(self, client):
        """Test action with unknown action type"""
        action_data = {
            "action_type": "teleport",  # Not a valid action
            "target_x": 150,
            "target_y": 180
        }

        response = client.post("/ai/npc/npc_001/execute-action", json=action_data)

        assert response.status_code in [400, 422]


@pytest.mark.router
@pytest.mark.npc
class TestNPCDeletion:
    """Test NPC deletion endpoints"""

    def test_delete_npc_success(self, client):
        """Test successful NPC deletion"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)
            mock_db.delete = AsyncMock(return_value=1)

            response = client.delete("/ai/npc/npc_001")

            assert response.status_code == 200
            data = response.json()
            assert data["success"] is True

    def test_delete_npc_not_found(self, client):
        """Test deletion of non-existent NPC"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=0)

            response = client.delete("/ai/npc/nonexistent")

            assert response.status_code == 404


@pytest.mark.router
@pytest.mark.npc
class TestNPCBulkOperations:
    """Test NPC bulk operation endpoints"""

    def test_bulk_register_npcs(self, client, sample_npc_registration):
        """Test bulk NPC registration"""
        bulk_data = {
            "npcs": [
                sample_npc_registration,
                {**sample_npc_registration, "npc_id": "test_merchant_002", "name": "Another Merchant"}
            ]
        }

        with patch('routers.npc.db.client') as mock_db:
            mock_db.set = AsyncMock(return_value=True)
            mock_db.hset = AsyncMock(return_value=1)

            response = client.post("/ai/npc/bulk-register", json=bulk_data)

            assert response.status_code == 200
            data = response.json()
            assert "registered_count" in data

    def test_list_npcs_with_filters(self, client):
        """Test listing NPCs with filters"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.keys = AsyncMock(return_value=["npc:npc_001", "npc:npc_002"])
            mock_db.get = AsyncMock(return_value='{"npc_id": "npc_001", "npc_class": "merchant"}')

            response = client.get("/ai/npc/list?npc_class=merchant&map=prontera")

            assert response.status_code == 200
            data = response.json()
            assert isinstance(data["npcs"], list)


@pytest.mark.router
@pytest.mark.npc
@pytest.mark.asyncio
class TestNPCAsyncOperations:
    """Test NPC async operations"""

    async def test_async_state_updates(self, client):
        """Test async NPC state updates"""
        with patch('routers.npc.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)
            mock_db.set = AsyncMock(return_value=True)

            # Simulate multiple concurrent updates
            responses = []
            for i in range(5):
                update_data = {"mood": f"state_{i}"}
                response = client.put(f"/ai/npc/npc_{i}/state", json=update_data)
                responses.append(response)

            assert all(r.status_code in [200, 404] for r in responses)