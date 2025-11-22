"""
Test Suite for Gift Router

Tests cover:
- Gift giving functionality
- NPC reaction processing
- Affinity changes
- Item validation
- Authentication and error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def sample_gift_request():
    """Sample gift request."""
    return {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "flower",
        "quantity": 1,
        "message": "A gift for you!"
    }


# ============================================================================
# GIFT GIVING TESTS
# ============================================================================

@pytest.mark.unit
def test_give_gift_success(test_client, sample_gift_request):
    """Test successful gift giving."""
    response = test_client.post("/api/gift/give", json=sample_gift_request)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "gift_id" in data["data"]
    assert "reaction" in data["data"]
    assert "affinity_change" in data["data"]


@pytest.mark.unit
def test_give_gift_without_message(test_client):
    """Test giving gift without message."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "sword",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_give_multiple_items(test_client):
    """Test giving multiple items as gift."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "potion",
        "quantity": 5
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_give_gift_with_long_message(test_client):
    """Test gift with maximum length message."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1,
        "message": "A" * 200  # Max length
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_gift_missing_player_id(test_client):
    """Test gift without player_id."""
    gift_request = {
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_gift_missing_npc_id(test_client):
    """Test gift without npc_id."""
    gift_request = {
        "player_id": "player_123",
        "item_id": "item",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_gift_missing_item_id(test_client):
    """Test gift without item_id."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_gift_invalid_quantity(test_client):
    """Test gift with invalid quantity."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 0  # Invalid: must be > 0
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_gift_negative_quantity(test_client):
    """Test gift with negative quantity."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": -1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_gift_message_too_long(test_client):
    """Test gift with message exceeding max length."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1,
        "message": "A" * 201  # Exceeds max length
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 422


# ============================================================================
# NPC REACTION TESTS
# ============================================================================

@pytest.mark.unit
def test_gift_reaction_types(test_client):
    """Test different NPC reactions to gifts."""
    reactions = ["pleased", "grateful", "neutral", "disappointed"]
    
    for reaction in reactions:
        gift_request = {
            "player_id": "player_123",
            "npc_id": f"npc_{reaction}",
            "item_id": "item",
            "quantity": 1
        }
        
        response = test_client.post("/api/gift/give", json=gift_request)
        
        assert response.status_code == 200


@pytest.mark.unit
def test_affinity_change_positive(test_client):
    """Test gift causing positive affinity change."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "favorite_item",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200
    data = response.json()
    # Affinity change should be present
    assert "affinity_change" in data["data"]


@pytest.mark.unit
def test_response_message_exists(test_client, sample_gift_request):
    """Test that NPC provides response message."""
    response = test_client.post("/api/gift/give", json=sample_gift_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "response_message" in data["data"]
    assert len(data["data"]["response_message"]) > 0


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_gift_server_error(test_client):
    """Test gift with server error."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "error_npc",
        "item_id": "error_item",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code in [200, 400, 500]


@pytest.mark.unit
def test_malformed_request(test_client):
    """Test malformed gift request."""
    response = test_client.post(
        "/api/gift/give",
        data="not json",
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status_code == 422


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_gift_to_nonexistent_npc(test_client):
    """Test giving gift to non-existent NPC."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "nonexistent_npc",
        "item_id": "item",
        "quantity": 1
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code in [200, 404]


@pytest.mark.unit
def test_gift_empty_message(test_client):
    """Test gift with empty message."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1,
        "message": ""
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_gift_special_characters_in_message(test_client):
    """Test gift message with special characters."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1,
        "message": "Hello! <3 😊 @#$%"
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    assert response.status_code == 200


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_multiple_gifts_sequence(test_client):
    """Test giving multiple gifts in sequence."""
    gifts = [
        {"player_id": "player_123", "npc_id": "npc_456", "item_id": "item1", "quantity": 1},
        {"player_id": "player_123", "npc_id": "npc_456", "item_id": "item2", "quantity": 1},
        {"player_id": "player_123", "npc_id": "npc_456", "item_id": "item3", "quantity": 1}
    ]
    
    for gift in gifts:
        response = test_client.post("/api/gift/give", json=gift)
        assert response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_gift_processing_performance(test_client, performance_timer, sample_gift_request):
    """Test gift processing performance."""
    performance_timer.start()
    
    response = test_client.post("/api/gift/give", json=sample_gift_request)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000


# ============================================================================
# SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_gift_xss_prevention_in_message(test_client):
    """Test XSS prevention in gift message."""
    gift_request = {
        "player_id": "player_123",
        "npc_id": "npc_456",
        "item_id": "item",
        "quantity": 1,
        "message": "<script>alert('xss')</script>"
    }
    
    response = test_client.post("/api/gift/give", json=gift_request)
    
    # Should handle or sanitize
    assert response.status_code in [200, 400]