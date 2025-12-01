"""
Test Suite for Faction Router

Tests cover:
- Faction information retrieval
- Player reputation management
- Faction relationships
- Authentication and validation
- Error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def mock_faction_data():
    """Mock faction data."""
    return {
        "faction_id": "royal_guard",
        "name": "Royal Guard",
        "faction_type": "military",
        "reputation_tier": "neutral",
        "player_reputation": 0.0,
        "is_allied": False,
        "members_count": 150
    }


@pytest.fixture
def sample_reputation_update():
    """Sample reputation update."""
    return {
        "player_id": "player_123",
        "change": 10.0,
        "reason": "completed_quest"
    }


# ============================================================================
# FACTION INFO TESTS
# ============================================================================

@pytest.mark.unit
def test_get_faction_success(test_client, mock_faction_data):
    """Test successful faction retrieval."""
    response = test_client.get("/api/faction/royal_guard")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "faction_id" in data["data"]
    assert "name" in data["data"]


@pytest.mark.unit
def test_get_faction_with_auth(authenticated_client):
    """Test faction retrieval with authentication."""
    response = authenticated_client.get("/api/faction/test_faction")
    
    assert response.status_code == 200


@pytest.mark.unit
def test_get_nonexistent_faction(test_client):
    """Test getting non-existent faction."""
    response = test_client.get("/api/faction/nonexistent_faction")
    
    # Should still return 200 with mock data or 404
    assert response.status_code in [200, 404]


# ============================================================================
# REPUTATION TESTS
# ============================================================================

@pytest.mark.unit
def test_get_reputation_success(test_client):
    """Test successful reputation retrieval."""
    response = test_client.get(
        "/api/faction/royal_guard/reputation?player_id=player_123"
    )
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "reputation" in data["data"]
    assert "tier" in data["data"]


@pytest.mark.unit
def test_get_reputation_missing_player_id(test_client):
    """Test reputation without player_id."""
    response = test_client.get("/api/faction/royal_guard/reputation")
    
    assert response.status_code == 422  # Missing required param


@pytest.mark.unit
def test_update_reputation_positive(test_client, sample_reputation_update):
    """Test updating reputation with positive change."""
    response = test_client.post(
        "/api/faction/royal_guard/reputation",
        json=sample_reputation_update
    )
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "new_reputation" in data["data"]


@pytest.mark.unit
def test_update_reputation_negative(test_client):
    """Test updating reputation with negative change."""
    update = {
        "player_id": "player_123",
        "change": -15.0,
        "reason": "hostile_action"
    }
    
    response = test_client.post("/api/faction/royal_guard/reputation", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_reputation_missing_reason(test_client):
    """Test reputation update without reason."""
    update = {
        "player_id": "player_123",
        "change": 5.0
    }
    
    response = test_client.post("/api/faction/test_faction/reputation", json=update)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_reputation_tier_changes(test_client):
    """Test reputation tier transitions."""
    large_update = {
        "player_id": "player_123",
        "change": 50.0,
        "reason": "major_contribution"
    }
    
    response = test_client.post("/api/faction/royal_guard/reputation", json=large_update)
    
    assert response.status_code == 200
    data = response.json()
    # Should indicate tier change
    assert "tier_changed" in data["data"]


# ============================================================================
# FACTION RELATIONSHIPS TESTS
# ============================================================================

@pytest.mark.unit
def test_get_all_relationships(test_client):
    """Test getting all faction relationships."""
    response = test_client.get("/api/faction/relationships")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "relationships" in data["data"]


@pytest.mark.unit
def test_get_specific_faction_relationships(test_client):
    """Test getting relationships for specific faction."""
    response = test_client.get("/api/faction/relationships?faction_id=royal_guard")
    
    assert response.status_code == 200
    data = response.json()
    assert "faction_id" in data["data"]


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_reputation_update_validation(test_client):
    """Test reputation update field validation."""
    invalid_update = {
        "player_id": "",  # Empty player ID
        "change": 10.0,
        "reason": "test"
    }
    
    response = test_client.post("/api/faction/test/reputation", json=invalid_update)
    
    assert response.status_code in [400, 422]


@pytest.mark.unit
def test_faction_id_validation(test_client):
    """Test faction ID format validation."""
    response = test_client.get("/api/faction/")
    
    assert response.status_code == 404  # Invalid path


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_faction_server_error(test_client):
    """Test faction endpoint with server error."""
    response = test_client.get("/api/faction/error_trigger_faction")
    
    # Should handle gracefully
    assert response.status_code in [200, 500]


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_faction_reputation_flow(test_client):
    """Test complete faction reputation flow."""
    faction_id = "royal_guard"
    player_id = "player_123"
    
    # Get initial reputation
    get_response = test_client.get(
        f"/api/faction/{faction_id}/reputation?player_id={player_id}"
    )
    assert get_response.status_code == 200
    
    # Update reputation
    update = {
        "player_id": player_id,
        "change": 25.0,
        "reason": "quest_completion"
    }
    
    update_response = test_client.post(
        f"/api/faction/{faction_id}/reputation",
        json=update
    )
    assert update_response.status_code == 200


@pytest.mark.integration
def test_faction_info_to_relationships(test_client):
    """Test flow from faction info to relationships."""
    # Get faction info
    faction_response = test_client.get("/api/faction/royal_guard")
    assert faction_response.status_code == 200
    
    # Get relationships
    rel_response = test_client.get("/api/faction/relationships?faction_id=royal_guard")
    assert rel_response.status_code == 200


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_reputation_zero_change(test_client):
    """Test reputation update with zero change."""
    update = {
        "player_id": "player_123",
        "change": 0.0,
        "reason": "neutral_action"
    }
    
    response = test_client.post("/api/faction/test/reputation", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_reputation_very_large_change(test_client):
    """Test reputation with very large change."""
    update = {
        "player_id": "player_123",
        "change": 1000.0,
        "reason": "legendary_deed"
    }
    
    response = test_client.post("/api/faction/test/reputation", json=update)
    
    assert response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_faction_retrieval_performance(test_client, performance_timer):
    """Test faction retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/faction/royal_guard")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


@pytest.mark.performance
def test_reputation_update_performance(test_client, performance_timer):
    """Test reputation update performance."""
    update = {
        "player_id": "player_123",
        "change": 10.0,
        "reason": "test"
    }
    
    performance_timer.start()
    
    response = test_client.post("/api/faction/test/reputation", json=update)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000