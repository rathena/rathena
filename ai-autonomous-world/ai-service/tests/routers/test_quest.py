"""
Unit tests for Quest router endpoints

Tests quest creation, assignment, progress tracking, and completion
with mocked dependencies.
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
def sample_quest_data():
    """Sample quest creation data"""
    return {
        "quest_id": "goblin_slayer_quest",
        "title": "Goblin Slayer",
        "description": "Defeat 10 goblins terrorizing the village",
        "quest_giver_npc_id": "village_elder_001",
        "requirements": {
            "level": 10,
            "class": ["warrior", "knight"]
        },
        "objectives": [
            {
                "type": "kill",
                "target": "goblin",
                "count": 10
            }
        ],
        "rewards": {
            "exp": 1000,
            "gold": 500,
            "items": ["potion:5"]
        },
        "time_limit": 3600,
        "repeatable": False
    }


@pytest.fixture
def sample_quest_progress():
    """Sample quest progress data"""
    return {
        "player_id": "player_001",
        "quest_id": "goblin_slayer_quest",
        "objectives_progress": [
            {"type": "kill", "target": "goblin", "current": 7, "required": 10}
        ]
    }


@pytest.mark.router
@pytest.mark.quest
class TestQuestCreation:
    """Test quest creation endpoints"""

    def test_create_quest_success(self, client, sample_quest_data):
        """Test successful quest creation"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.set = AsyncMock(return_value=True)
            mock_db.hset = AsyncMock(return_value=1)

            response = client.post("/ai/quest/create", json=sample_quest_data)

            assert response.status_code == 200
            data = response.json()
            assert data["quest_id"] == "goblin_slayer_quest"
            assert "created_at" in data

    def test_create_quest_duplicate(self, client, sample_quest_data):
        """Test duplicate quest creation rejection"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)

            response = client.post("/ai/quest/create", json=sample_quest_data)

            assert response.status_code == 409

    def test_create_quest_invalid_requirements(self, client, sample_quest_data):
        """Test quest creation with invalid requirements"""
        sample_quest_data["requirements"]["level"] = -5

        response = client.post("/ai/quest/create", json=sample_quest_data)

        assert response.status_code == 422


@pytest.mark.router
@pytest.mark.quest
class TestQuestAssignment:
    """Test quest assignment endpoints"""

    def test_assign_quest_to_player(self, client):
        """Test assigning quest to player"""
        assignment_data = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "quest_giver_npc_id": "village_elder_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"quest_id": "goblin_slayer_quest"}')
            mock_db.set = AsyncMock(return_value=True)
            mock_db.sadd = AsyncMock(return_value=1)

            response = client.post("/ai/quest/assign", json=assignment_data)

            assert response.status_code == 200
            data = response.json()
            assert "assigned_at" in data

    def test_assign_quest_player_not_qualified(self, client):
        """Test quest assignment to unqualified player"""
        assignment_data = {
            "player_id": "player_001",
            "quest_id": "high_level_quest",
            "quest_giver_npc_id": "elder_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            # Quest requires level 50, player is level 10
            mock_db.get = AsyncMock(side_effect=[
                '{"requirements": {"level": 50}}',  # Quest data
                '{"level": 10}'  # Player data
            ])

            response = client.post("/ai/quest/assign", json=assignment_data)

            assert response.status_code == 400

    def test_assign_quest_already_active(self, client):
        """Test assigning already active quest"""
        assignment_data = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "quest_giver_npc_id": "elder_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            mock_db.sismember = AsyncMock(return_value=True)

            response = client.post("/ai/quest/assign", json=assignment_data)

            assert response.status_code == 409


@pytest.mark.router
@pytest.mark.quest
class TestQuestProgress:
    """Test quest progress tracking endpoints"""

    def test_update_quest_progress(self, client, sample_quest_progress):
        """Test updating quest progress"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"objectives": [{"count": 10}]}')
            mock_db.set = AsyncMock(return_value=True)

            response = client.put("/ai/quest/progress", json=sample_quest_progress)

            assert response.status_code == 200
            data = response.json()
            assert "progress_percentage" in data

    def test_get_quest_progress(self, client):
        """Test retrieving quest progress"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"current": 7, "required": 10}')

            response = client.get("/ai/quest/player_001/quest/goblin_slayer_quest/progress")

            assert response.status_code == 200
            data = response.json()
            assert "objectives_progress" in data

    def test_quest_progress_completion_detection(self, client):
        """Test automatic quest completion detection"""
        completion_progress = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "objectives_progress": [
                {"type": "kill", "target": "goblin", "current": 10, "required": 10}
            ]
        }

        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"objectives": [{"count": 10}]}')
            mock_db.set = AsyncMock(return_value=True)

            response = client.put("/ai/quest/progress", json=completion_progress)

            assert response.status_code == 200
            data = response.json()
            assert data.get("completed") is True


@pytest.mark.router
@pytest.mark.quest
class TestQuestCompletion:
    """Test quest completion endpoints"""

    def test_complete_quest_success(self, client):
        """Test successful quest completion"""
        completion_data = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "npc_id": "village_elder_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"rewards": {"exp": 1000, "gold": 500}}')
            mock_db.set = AsyncMock(return_value=True)
            mock_db.srem = AsyncMock(return_value=1)
            mock_db.sadd = AsyncMock(return_value=1)

            response = client.post("/ai/quest/complete", json=completion_data)

            assert response.status_code == 200
            data = response.json()
            assert "rewards" in data
            assert data["rewards"]["exp"] == 1000

    def test_complete_quest_not_finished(self, client):
        """Test completing unfinished quest"""
        completion_data = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "npc_id": "elder_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            # Quest progress shows incomplete objectives
            mock_db.get = AsyncMock(return_value='{"current": 5, "required": 10}')

            response = client.post("/ai/quest/complete", json=completion_data)

            assert response.status_code == 400

    def test_complete_quest_wrong_npc(self, client):
        """Test completing quest with wrong NPC"""
        completion_data = {
            "player_id": "player_001",
            "quest_id": "goblin_slayer_quest",
            "npc_id": "random_npc_001"
        }

        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"quest_giver_npc_id": "village_elder_001"}')

            response = client.post("/ai/quest/complete", json=completion_data)

            assert response.status_code == 400


@pytest.mark.router
@pytest.mark.quest
class TestQuestChains:
    """Test quest chain functionality"""

    def test_get_quest_chain(self, client):
        """Test retrieving quest chain"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"next_quest": "goblin_king_quest"}')

            response = client.get("/ai/quest/goblin_slayer_quest/chain")

            assert response.status_code == 200
            data = response.json()
            assert "next_quest" in data

    def test_unlock_next_quest_in_chain(self, client):
        """Test unlocking next quest after completion"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"quest_chain": ["quest1", "quest2", "quest3"]}')
            mock_db.set = AsyncMock(return_value=True)

            response = client.post("/ai/quest/player_001/unlock-next/goblin_slayer_quest")

            assert response.status_code == 200


@pytest.mark.router
@pytest.mark.quest
class TestQuestListing:
    """Test quest listing and filtering endpoints"""

    def test_list_available_quests(self, client):
        """Test listing available quests for player"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.keys = AsyncMock(return_value=["quest:quest1", "quest:quest2"])
            mock_db.get = AsyncMock(return_value='{"level": 10}')

            response = client.get("/ai/quest/available?player_id=player_001")

            assert response.status_code == 200
            data = response.json()
            assert isinstance(data["quests"], list)

    def test_list_active_quests(self, client):
        """Test listing player's active quests"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.smembers = AsyncMock(return_value={"quest1", "quest2"})
            mock_db.get = AsyncMock(return_value='{"quest_id": "quest1"}')

            response = client.get("/ai/quest/player_001/active")

            assert response.status_code == 200
            data = response.json()
            assert isinstance(data["active_quests"], list)

    def test_list_completed_quests(self, client):
        """Test listing player's completed quests"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.smembers = AsyncMock(return_value={"quest1", "quest2"})

            response = client.get("/ai/quest/player_001/completed")

            assert response.status_code == 200


@pytest.mark.router
@pytest.mark.quest
class TestQuestAbandon:
    """Test quest abandonment"""

    def test_abandon_quest_success(self, client):
        """Test successfully abandoning a quest"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.sismember = AsyncMock(return_value=True)
            mock_db.srem = AsyncMock(return_value=1)
            mock_db.delete = AsyncMock(return_value=1)

            response = client.post("/ai/quest/player_001/abandon/goblin_slayer_quest")

            assert response.status_code == 200

    def test_abandon_quest_not_active(self, client):
        """Test abandoning non-active quest"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.sismember = AsyncMock(return_value=False)

            response = client.post("/ai/quest/player_001/abandon/nonexistent_quest")

            assert response.status_code == 404


@pytest.mark.router
@pytest.mark.quest
@pytest.mark.asyncio
class TestQuestConcurrency:
    """Test concurrent quest operations"""

    async def test_concurrent_progress_updates(self, client):
        """Test concurrent quest progress updates"""
        with patch('routers.quest.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"objectives": [{"count": 10}]}')
            mock_db.set = AsyncMock(return_value=True)

            # Simulate concurrent progress updates
            responses = []
            for i in range(5):
                progress_data = {
                    "player_id": f"player_{i}",
                    "quest_id": "goblin_slayer_quest",
                    "objectives_progress": [{"current": i + 1}]
                }
                response = client.put("/ai/quest/progress", json=progress_data)
                responses.append(response)

            assert all(r.status_code in [200, 404] for r in responses)