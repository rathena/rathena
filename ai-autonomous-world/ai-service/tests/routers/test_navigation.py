"""
Test Suite for Navigation Router

Tests cover:
- Pathfinding calculation
- Map information retrieval
- Obstacle avoidance
- Path validation
- Authentication and error handling
"""

import pytest
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def sample_pathfind_request():
    """Sample pathfinding request."""
    return {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map",
        "avoid_obstacles": True,
        "max_distance": None
    }


# ============================================================================
# PATHFINDING TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_path_success(test_client, sample_pathfind_request):
    """Test successful pathfinding."""
    response = test_client.post("/api/navigation/pathfind", json=sample_pathfind_request)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "path" in data["data"]
    assert "total_distance" in data["data"]
    assert "is_valid" in data["data"]


@pytest.mark.unit
def test_path_same_start_end(test_client):
    """Test pathfinding with same start and end."""
    request = {
        "start": {"x": 50.0, "y": 50.0},
        "end": {"x": 50.0, "y": 50.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_path_with_max_distance(test_client):
    """Test pathfinding with maximum distance constraint."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 1000.0, "y": 1000.0},
        "map_name": "test_map",
        "max_distance": 500.0
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_path_without_obstacle_avoidance(test_client):
    """Test pathfinding without obstacle avoidance."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map",
        "avoid_obstacles": False
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_pathfind_missing_start(test_client):
    """Test pathfinding without start position."""
    request = {
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_pathfind_missing_end(test_client):
    """Test pathfinding without end position."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_pathfind_missing_map_name(test_client):
    """Test pathfinding without map name."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 100.0, "y": 100.0}
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_pathfind_invalid_position(test_client):
    """Test pathfinding with invalid position coordinates."""
    request = {
        "start": {"x": "invalid", "y": 0.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 422


# ============================================================================
# MAP INFO TESTS
# ============================================================================

@pytest.mark.unit
def test_get_map_info_success(test_client):
    """Test successful map info retrieval."""
    response = test_client.get("/api/navigation/map?map_name=test_map")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "map_name" in data["data"]
    assert "width" in data["data"]
    assert "height" in data["data"]


@pytest.mark.unit
def test_get_map_info_missing_name(test_client):
    """Test map info without map name."""
    response = test_client.get("/api/navigation/map")
    
    assert response.status_code == 422


@pytest.mark.unit
def test_map_info_includes_poi(test_client):
    """Test that map info includes points of interest."""
    response = test_client.get("/api/navigation/map?map_name=test_map")
    
    assert response.status_code == 200
    data = response.json()
    assert "points_of_interest" in data["data"]


@pytest.mark.unit
def test_map_info_nonexistent(test_client):
    """Test getting info for non-existent map."""
    response = test_client.get("/api/navigation/map?map_name=nonexistent")
    
    assert response.status_code in [200, 404]


# ============================================================================
# PATH VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_path_includes_waypoints(test_client, sample_pathfind_request):
    """Test that path includes waypoints."""
    response = test_client.post("/api/navigation/pathfind", json=sample_pathfind_request)
    
    assert response.status_code == 200
    data = response.json()
    assert len(data["data"]["path"]) > 0


@pytest.mark.unit
def test_path_estimated_duration(test_client, sample_pathfind_request):
    """Test that path includes duration estimate."""
    response = test_client.post("/api/navigation/pathfind", json=sample_pathfind_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "estimated_duration" in data["data"]
    assert data["data"]["estimated_duration"] >= 0


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_pathfind_server_error(test_client):
    """Test pathfinding with server error."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "error_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code in [200, 500]


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_path_negative_coordinates(test_client):
    """Test pathfinding with negative coordinates."""
    request = {
        "start": {"x": -50.0, "y": -50.0},
        "end": {"x": 50.0, "y": 50.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_path_very_long_distance(test_client):
    """Test pathfinding over very long distance."""
    request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 10000.0, "y": 10000.0},
        "map_name": "large_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_path_zero_distance(test_client):
    """Test pathfinding with zero distance."""
    request = {
        "start": {"x": 100.0, "y": 100.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map"
    }
    
    response = test_client.post("/api/navigation/pathfind", json=request)
    
    assert response.status_code == 200


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_map_info_to_pathfind(test_client):
    """Test flow from map info to pathfinding."""
    # Get map info
    map_response = test_client.get("/api/navigation/map?map_name=test_map")
    assert map_response.status_code == 200
    
    # Use map for pathfinding
    path_request = {
        "start": {"x": 0.0, "y": 0.0},
        "end": {"x": 100.0, "y": 100.0},
        "map_name": "test_map"
    }
    
    path_response = test_client.post("/api/navigation/pathfind", json=path_request)
    assert path_response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_pathfind_performance(test_client, performance_timer, sample_pathfind_request):
    """Test pathfinding performance."""
    performance_timer.start()
    
    response = test_client.post("/api/navigation/pathfind", json=sample_pathfind_request)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000


@pytest.mark.performance
def test_map_info_performance(test_client, performance_timer):
    """Test map info retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/navigation/map?map_name=test_map")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500