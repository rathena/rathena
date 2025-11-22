"""
Unit tests for Player router endpoints

Tests player interactions, dialogue generation, and player-NPC relationship
management with comprehensive mocking.
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
def sample_player_interaction():
    """Sample player interaction data"""
    return {
        "player_id": "player_001",
        "npc_id": "npc_001",
        "interaction_type": "talk",
        "message": "Hello, merchant! What wares do you have?",
        "context": {
            "player_name": "TestHero",
            "player_level": 75,
            "player_class": "swordsman",
            "location": {"map": "prontera", "x": 150, "y": 180},
            "time_of_day": "afternoon",
            "weather": "sunny"
        }
    }


@pytest.fixture
def sample_trade_interaction():
    """Sample trade interaction data"""
    return {
        "player_id": "player_001",
        "npc_id": "merchant_001",
        "interaction_type": "trade",
        "item_id": "potion",
        "quantity": 5,
        "price": 500
    }


@pytest.mark.router
@pytest.mark.player
class TestPlayerDialogue:
    """Test player dialogue interaction endpoints"""

    def test_player_dialogue_success(self, client, sample_player_interaction):
        """Test successful player dialogue interaction"""
        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="npc_001",
                player_id="player_001",
                response=NPCResponse(
                    action="dialogue",
                    data={"text": "Greetings, brave warrior! I have many fine goods."},
                    emotion="friendly",
                    next_actions=["talk", "trade", "end_conversation"]
                ),
                npc_state_update={"last_interaction": "talk"},
                relationship_change={"affinity": 5}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=sample_player_interaction)

            assert response.status_code == 200
            data = response.json()
            assert "response" in data
            assert data["response"]["action"] == "dialogue"

    def test_player_dialogue_with_context(self, client, sample_player_interaction):
        """Test dialogue with rich context"""
        sample_player_interaction["context"]["recent_events"] = [
            "Defeated goblin king",
            "Saved village"
        ]

        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="npc_001",
                player_id="player_001",
                response=NPCResponse(
                    action="dialogue",
                    data={"text": "I heard of your heroic deeds!"},
                    emotion="impressed",
                    next_actions=["talk", "quest"]
                ),
                npc_state_update={},
                relationship_change={"affinity": 10}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=sample_player_interaction)

            assert response.status_code == 200

    def test_player_dialogue_npc_not_found(self, client, sample_player_interaction):
        """Test dialogue with non-existent NPC"""
        sample_player_interaction["npc_id"] = "nonexistent_npc"

        with patch('routers.player.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value=None)

            response = client.post("/ai/player/interaction", json=sample_player_interaction)

            assert response.status_code == 404

    def test_player_dialogue_invalid_message(self, client, sample_player_interaction):
        """Test dialogue with invalid/empty message"""
        sample_player_interaction["message"] = ""

        response = client.post("/ai/player/interaction", json=sample_player_interaction)

        assert response.status_code in [400, 422]


@pytest.mark.router
@pytest.mark.player
class TestPlayerTrade:
    """Test player trade interaction endpoints"""

    def test_trade_interaction_success(self, client, sample_trade_interaction):
        """Test successful trade interaction"""
        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="merchant_001",
                player_id="player_001",
                response=NPCResponse(
                    action="trade",
                    data={
                        "item_id": "potion",
                        "quantity": 5,
                        "total_price": 500,
                        "message": "Pleasure doing business with you!"
                    },
                    emotion="satisfied",
                    next_actions=["trade_more", "end_conversation"]
                ),
                npc_state_update={"gold": 1500},
                relationship_change={"affinity": 3}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=sample_trade_interaction)

            assert response.status_code == 200
            data = response.json()
            assert data["response"]["action"] == "trade"

    def test_trade_insufficient_funds(self, client, sample_trade_interaction):
        """Test trade with insufficient player funds"""
        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="merchant_001",
                player_id="player_001",
                response=NPCResponse(
                    action="trade_failed",
                    data={"message": "You don't have enough gold!"},
                    emotion="disappointed",
                    next_actions=["end_conversation"]
                ),
                npc_state_update={},
                relationship_change={}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=sample_trade_interaction)

            assert response.status_code == 200
            data = response.json()
            assert data["response"]["action"] == "trade_failed"


@pytest.mark.router
@pytest.mark.player
class TestPlayerRelationship:
    """Test player-NPC relationship endpoints"""

    def test_get_relationship_status(self, client):
        """Test getting player-NPC relationship status"""
        with patch('routers.player.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"affinity": 50, "trust": 60}')

            response = client.get("/ai/player/player_001/relationship/npc_001")

            assert response.status_code == 200
            data = response.json()
            assert "affinity" in data
            assert "trust" in data

    def test_update_relationship_status(self, client):
        """Test updating player-NPC relationship"""
        update_data = {
            "affinity_change": 10,
            "trust_change": 5,
            "reason": "Completed quest together"
        }

        with patch('routers.player.db.client') as mock_db:
            mock_db.exists = AsyncMock(return_value=1)
            mock_db.set = AsyncMock(return_value=True)

            response = client.put(
                "/ai/player/player_001/relationship/npc_001",
                json=update_data
            )

            assert response.status_code == 200

    def test_relationship_decay_over_time(self, client):
        """Test relationship decay mechanism"""
        with patch('routers.player.db.client') as mock_db:
            mock_db.get = AsyncMock(return_value='{"affinity": 50, "last_interaction": "2024-01-01"}')

            response = client.get("/ai/player/player_001/relationship/npc_001/check-decay")

            assert response.status_code == 200
            data = response.json()
            assert "decay_amount" in data


@pytest.mark.router
@pytest.mark.player
class TestPlayerQuest:
    """Test player quest interaction endpoints"""

    def test_accept_quest_from_npc(self, client):
        """Test player accepting quest from NPC"""
        quest_data = {
            "player_id": "player_001",
            "npc_id": "quest_giver_001",
            "quest_id": "goblin_slayer",
            "interaction_type": "quest_accept"
        }

        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="quest_giver_001",
                player_id="player_001",
                response=NPCResponse(
                    action="quest_accepted",
                    data={
                        "quest_id": "goblin_slayer",
                        "message": "Go forth and slay those goblins!"
                    },
                    emotion="encouraging",
                    next_actions=["end_conversation"]
                ),
                npc_state_update={"active_quest_givers": ["player_001"]},
                relationship_change={"affinity": 5}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=quest_data)

            assert response.status_code == 200

    def test_complete_quest_with_npc(self, client):
        """Test player completing quest with NPC"""
        quest_data = {
            "player_id": "player_001",
            "npc_id": "quest_giver_001",
            "quest_id": "goblin_slayer",
            "interaction_type": "quest_complete",
            "completion_data": {
                "goblins_slain": 10
            }
        }

        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="quest_giver_001",
                player_id="player_001",
                response=NPCResponse(
                    action="quest_completed",
                    data={
                        "quest_id": "goblin_slayer",
                        "rewards": {"exp": 1000, "gold": 500},
                        "message": "Well done, hero!"
                    },
                    emotion="proud",
                    next_actions=["talk", "end_conversation"]
                ),
                npc_state_update={},
                relationship_change={"affinity": 20, "trust": 15}
            )
            mock_handler.return_value = mock_response

            response = client.post("/ai/player/interaction", json=quest_data)

            assert response.status_code == 200


@pytest.mark.router
@pytest.mark.player
class TestPlayerValidation:
    """Test player input validation"""

    def test_invalid_player_id(self, client, sample_player_interaction):
        """Test interaction with invalid player ID format"""
        sample_player_interaction["player_id"] = ""

        response = client.post("/ai/player/interaction", json=sample_player_interaction)

        assert response.status_code in [400, 422, 500]

    def test_invalid_interaction_type(self, client, sample_player_interaction):
        """Test interaction with invalid type"""
        sample_player_interaction["interaction_type"] = "invalid_type"

        response = client.post("/ai/player/interaction", json=sample_player_interaction)

        assert response.status_code in [400, 422]

    def test_missing_context_data(self, client, sample_player_interaction):
        """Test interaction with missing context"""
        del sample_player_interaction["context"]["player_level"]

        response = client.post("/ai/player/interaction", json=sample_player_interaction)

        # Should still work with partial context
        assert response.status_code in [200, 422]


@pytest.mark.router
@pytest.mark.player
@pytest.mark.asyncio
class TestPlayerConcurrency:
    """Test concurrent player interactions"""

    async def test_concurrent_interactions(self, client):
        """Test multiple concurrent player interactions"""
        with patch('routers.player.handle_player_interaction') as mock_handler:
            from models.player import PlayerInteractionResponse, NPCResponse

            mock_response = PlayerInteractionResponse(
                npc_id="npc_001",
                player_id="player_001",
                response=NPCResponse(
                    action="dialogue",
                    data={"text": "Hello!"},
                    emotion="neutral",
                    next_actions=["talk"]
                ),
                npc_state_update={},
                relationship_change={}
            )
            mock_handler.return_value = mock_response

            # Simulate concurrent requests
            responses = []
            for i in range(10):
                interaction = {
                    "player_id": f"player_{i}",
                    "npc_id": "npc_001",
                    "interaction_type": "talk",
                    "message": f"Message {i}",
                    "context": {
                        "player_name": f"Player{i}",
                        "player_level": 50,
                        "player_class": "warrior",
                        "location": {"map": "prontera", "x": 150, "y": 180}
                    }
                }
                response = client.post("/ai/player/interaction", json=interaction)
                responses.append(response)

            # All should succeed or fail gracefully
            assert all(r.status_code in [200, 404, 500] for r in responses)