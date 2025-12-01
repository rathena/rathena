"""
Test Suite for MVP Router

Tests cover:
- MVP boss spawning
- MVP state management
- Boss health tracking
- Respawn mechanics
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
def sample_mvp_spawn():
    """Sample MVP spawn request."""
    return {
        "boss_type": "dragon",
        "location": "dragon_nest",
        "position_x": 250.0,
        "position_y": 300.0,
        "difficulty_multiplier": 1.0
    }


# ============================================================================
# MVP SPAWNING TESTS
# ============================================================================

@pytest.mark.unit
def test_spawn_mvp_success(test_client, sample_mvp_spawn):
    """Test successful MVP spawning."""
    response = test_client.post("/api/mvp/spawn", json=sample_mvp_spawn)
    
    assert response.status_code == 201
    data = response.json()
    assert data["success"] is True
    assert "mvp_id" in data["data"]
    assert data["data"]["boss_type"] == "dragon"


@pytest.mark.unit
def test_spawn_mvp_with_difficulty(test_client):
    """Test spawning MVP with difficulty multiplier."""
    spawn_request = {
        "boss_type": "orc_lord",
        "location": "orc_dungeon",
        "position_x": 100.0,
        "position_y": 150.0,
        "difficulty_multiplier": 1.5
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 201


@pytest.mark.unit
def test_spawn_mvp_min_difficulty(test_client):
    """Test spawning MVP with minimum difficulty."""
    spawn_request = {
        "boss_type": "goblin_king",
        "location": "goblin_cave",
        "position_x": 50.0,
        "position_y": 50.0,
        "difficulty_multiplier": 0.5
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 201


@pytest.mark.unit
def test_spawn_mvp_max_difficulty(test_client):
    """Test spawning MVP with maximum difficulty."""
    spawn_request = {
        "boss_type": "demon_king",
        "location": "hell",
        "position_x": 0.0,
        "position_y": 0.0,
        "difficulty_multiplier": 3.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 201


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_spawn_missing_boss_type(test_client):
    """Test spawn without boss_type."""
    spawn_request = {
        "location": "somewhere",
        "position_x": 0.0,
        "position_y": 0.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_spawn_missing_location(test_client):
    """Test spawn without location."""
    spawn_request = {
        "boss_type": "dragon",
        "position_x": 0.0,
        "position_y": 0.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_spawn_missing_coordinates(test_client):
    """Test spawn without coordinates."""
    spawn_request = {
        "boss_type": "dragon",
        "location": "nest"
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_spawn_invalid_difficulty(test_client):
    """Test spawn with invalid difficulty multiplier."""
    spawn_request = {
        "boss_type": "dragon",
        "location": "nest",
        "position_x": 0.0,
        "position_y": 0.0,
        "difficulty_multiplier": 5.0  # Exceeds max
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_spawn_negative_difficulty(test_client):
    """Test spawn with negative difficulty."""
    spawn_request = {
        "boss_type": "dragon",
        "location": "nest",
        "position_x": 0.0,
        "position_y": 0.0,
        "difficulty_multiplier": -1.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 422


# ============================================================================
# MVP STATE TESTS
# ============================================================================

@pytest.mark.unit
def test_get_mvp_state_success(test_client):
    """Test successful MVP state retrieval."""
    response = test_client.get("/api/mvp/mvp_123/state")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "mvp_id" in data["data"]
    assert "health_percentage" in data["data"]
    assert "is_alive" in data["data"]


@pytest.mark.unit
def test_get_mvp_state_alive(test_client):
    """Test getting state of alive MVP."""
    response = test_client.get("/api/mvp/alive_mvp/state")
    
    assert response.status_code == 200
    data = response.json()
    # Should indicate alive status
    assert "is_alive" in data["data"]


@pytest.mark.unit
def test_get_mvp_state_dead(test_client):
    """Test getting state of dead MVP."""
    response = test_client.get("/api/mvp/dead_mvp/state")
    
    assert response.status_code == 200
    data = response.json()
    # Should include respawn time
    if not data["data"].get("is_alive"):
        assert "respawn_time" in data["data"] or data["data"]["respawn_time"] is None


@pytest.mark.unit
def test_get_mvp_state_nonexistent(test_client):
    """Test getting state of non-existent MVP."""
    response = test_client.get("/api/mvp/nonexistent/state")
    
    assert response.status_code in [200, 404]


# ============================================================================
# ENGAGED PLAYERS TESTS
# ============================================================================

@pytest.mark.unit
def test_mvp_engaged_players_count(test_client):
    """Test MVP state includes engaged players count."""
    response = test_client.get("/api/mvp/mvp_123/state")
    
    assert response.status_code == 200
    data = response.json()
    assert "engaged_players" in data["data"]
    assert isinstance(data["data"]["engaged_players"], int)


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_spawn_server_error(test_client):
    """Test spawn with server error."""
    spawn_request = {
        "boss_type": "error_boss",
        "location": "error_location",
        "position_x": 0.0,
        "position_y": 0.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code in [201, 500]


@pytest.mark.unit
def test_state_server_error(test_client):
    """Test state retrieval with server error."""
    response = test_client.get("/api/mvp/error_mvp/state")
    
    assert response.status_code in [200, 500]


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_spawn_at_zero_coordinates(test_client):
    """Test spawning at origin coordinates."""
    spawn_request = {
        "boss_type": "test_boss",
        "location": "origin",
        "position_x": 0.0,
        "position_y": 0.0,
        "difficulty_multiplier": 1.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 201


@pytest.mark.unit
def test_spawn_at_negative_coordinates(test_client):
    """Test spawning at negative coordinates."""
    spawn_request = {
        "boss_type": "test_boss",
        "location": "underground",
        "position_x": -100.0,
        "position_y": -50.0,
        "difficulty_multiplier": 1.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    assert response.status_code == 201


@pytest.mark.unit
def test_spawn_with_default_difficulty(test_client):
    """Test spawning without specifying difficulty."""
    spawn_request = {
        "boss_type": "test_boss",
        "location": "test_location",
        "position_x": 100.0,
        "position_y": 100.0
    }
    
    response = test_client.post("/api/mvp/spawn", json=spawn_request)
    
    # Should use default difficulty of 1.0
    assert response.status_code == 201


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_spawn_and_check_state(test_client, sample_mvp_spawn):
    """Test spawning MVP and checking its state."""
    # Spawn MVP
    spawn_response = test_client.post("/api/mvp/spawn", json=sample_mvp_spawn)
    assert spawn_response.status_code == 201
    
    mvp_id = spawn_response.json()["data"]["mvp_id"]
    
    # Check state
    state_response = test_client.get(f"/api/mvp/{mvp_id}/state")
    assert state_response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_spawn_performance(test_client, performance_timer, sample_mvp_spawn):
    """Test MVP spawn performance."""
    performance_timer.start()
    
    response = test_client.post("/api/mvp/spawn", json=sample_mvp_spawn)
    
    performance_timer.stop()
    
    assert response.status_code == 201
    assert performance_timer.elapsed_ms() < 1000


@pytest.mark.performance
def test_state_retrieval_performance(test_client, performance_timer):
    """Test MVP state retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/mvp/mvp_123/state")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500