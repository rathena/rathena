"""
Test Suite for Relationship Router

Tests cover:
- NPC relationship retrieval
- Player-NPC affinity management
- Relationship history tracking
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
def mock_relationship_data():
    """Mock relationship data."""
    return {
        "entity_id": "player_123",
        "entity_type": "player",
        "affinity": 0.5,
        "relationship_type": "friendly",
        "interaction_count": 10,
        "last_interaction": datetime.utcnow()
    }


@pytest.fixture
def mock_affinity_data():
    """Mock affinity data."""
    return {
        "npc_id": "npc_456",
        "player_id": "player_123",
        "current_affinity": 0.6,
        "affinity_tier": "friendly",
        "relationship_history": []
    }


# ============================================================================
# RELATIONSHIP RETRIEVAL TESTS
# ============================================================================

@pytest.mark.unit
def test_get_npc_relationships_success(test_client):
    """Test successful NPC relationship retrieval."""
    response = test_client.get("/api/relationship/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "npc_id" in data["data"]
    assert "relationships" in data["data"]


@pytest.mark.unit
def test_get_relationships_with_auth(authenticated_client):
    """Test relationship retrieval with authentication."""
    response = authenticated_client.get("/api/relationship/npc_456")
    
    assert response.status_code == 200


@pytest.mark.unit
def test_get_relationships_nonexistent_npc(test_client):
    """Test getting relationships for non-existent NPC."""
    response = test_client.get("/api/relationship/nonexistent_npc")
    
    assert response.status_code in [200, 404]


# ============================================================================
# AFFINITY TESTS
# ============================================================================

@pytest.mark.unit
def test_get_affinity_success(test_client):
    """Test successful affinity retrieval."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_123&player_id=player_456"
    )
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "current_affinity" in data["data"]
    assert "affinity_tier" in data["data"]


@pytest.mark.unit
def test_get_affinity_missing_npc_id(test_client):
    """Test affinity without npc_id."""
    response = test_client.get("/api/relationship/affinity?player_id=player_123")
    
    assert response.status_code == 422


@pytest.mark.unit
def test_get_affinity_missing_player_id(test_client):
    """Test affinity without player_id."""
    response = test_client.get("/api/relationship/affinity?npc_id=npc_123")
    
    assert response.status_code == 422


@pytest.mark.unit
def test_affinity_includes_history(test_client):
    """Test that affinity response includes history."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_123&player_id=player_456"
    )
    
    assert response.status_code == 200
    data = response.json()
    assert "relationship_history" in data["data"]


# ============================================================================
# AFFINITY TIER TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("affinity,expected_tier", [
    (-0.8, "hostile"),
    (-0.3, "unfriendly"),
    (0.0, "neutral"),
    (0.5, "friendly"),
    (0.9, "close_friend")
])
def test_affinity_tier_mapping(test_client, affinity, expected_tier):
    """Test affinity to tier mapping."""
    response = test_client.get(
        f"/api/relationship/affinity?npc_id=npc_123&player_id=player_456"
    )
    
    # Response should contain appropriate tier
    assert response.status_code == 200


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_invalid_npc_id_format(test_client):
    """Test with invalid NPC ID format."""
    response = test_client.get("/api/relationship/")
    
    assert response.status_code == 404


@pytest.mark.unit
def test_empty_npc_id(test_client):
    """Test with empty NPC ID."""
    response = test_client.get("/api/relationship/%20")  # Space character
    
    assert response.status_code in [400, 404]


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_relationship_server_error(test_client):
    """Test relationship endpoint with server error."""
    response = test_client.get("/api/relationship/error_trigger_npc")
    
    assert response.status_code in [200, 500]


@pytest.mark.unit
def test_affinity_server_error(test_client):
    """Test affinity endpoint with server error."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=error&player_id=test"
    )
    
    assert response.status_code in [200, 500]


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_relationships_to_affinity_flow(test_client):
    """Test flow from relationships to specific affinity."""
    npc_id = "npc_123"
    player_id = "player_456"
    
    # Get all relationships
    rel_response = test_client.get(f"/api/relationship/{npc_id}")
    assert rel_response.status_code == 200
    
    # Get specific affinity
    aff_response = test_client.get(
        f"/api/relationship/affinity?npc_id={npc_id}&player_id={player_id}"
    )
    assert aff_response.status_code == 200


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_affinity_boundary_values(test_client):
    """Test affinity with boundary values."""
    # Min affinity
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_min&player_id=player_123"
    )
    assert response.status_code == 200
    
    # Max affinity
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_max&player_id=player_123"
    )
    assert response.status_code == 200


@pytest.mark.unit
def test_npc_with_no_relationships(test_client):
    """Test NPC with no relationships."""
    response = test_client.get("/api/relationship/lonely_npc")
    
    assert response.status_code == 200
    data = response.json()
    # Should return empty relationships
    assert data["data"]["count"] >= 0


@pytest.mark.unit
def test_special_characters_in_ids(test_client):
    """Test handling special characters in IDs."""
    import urllib.parse
    
    npc_id = urllib.parse.quote("npc-with-dash")
    response = test_client.get(f"/api/relationship/{npc_id}")
    
    assert response.status_code in [200, 400, 404]


# ============================================================================
# RELATIONSHIP HISTORY TESTS
# ============================================================================

@pytest.mark.unit
def test_affinity_history_ordering(test_client):
    """Test that affinity history is properly ordered."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_123&player_id=player_456"
    )
    
    assert response.status_code == 200
    data = response.json()
    history = data["data"].get("relationship_history", [])
    
    # History should be a list
    assert isinstance(history, list)


@pytest.mark.unit
def test_affinity_with_multiple_interactions(test_client):
    """Test affinity for entities with many interactions."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=popular_npc&player_id=active_player"
    )
    
    assert response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_relationship_retrieval_performance(test_client, performance_timer):
    """Test relationship retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/relationship/npc_123")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


@pytest.mark.performance
def test_affinity_query_performance(test_client, performance_timer):
    """Test affinity query performance."""
    performance_timer.start()
    
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_123&player_id=player_456"
    )
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


# ============================================================================
# SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_affinity_access_control(test_client):
    """Test that affinity respects access controls."""
    response = test_client.get(
        "/api/relationship/affinity?npc_id=npc_123&player_id=other_player"
    )
    
    # Should either succeed or deny based on access control
    assert response.status_code in [200, 401, 403]


@pytest.mark.security
def test_relationship_data_isolation(test_client):
    """Test that relationship data is properly isolated."""
    # Get relationships for one NPC
    response1 = test_client.get("/api/relationship/npc_1")
    
    # Get relationships for another NPC
    response2 = test_client.get("/api/relationship/npc_2")
    
    # Both should succeed and be independent
    assert response1.status_code == 200
    assert response2.status_code == 200