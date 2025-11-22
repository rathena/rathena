"""
Test Suite for NPC Movement Router

Tests cover:
- Movement computation and pathfinding
- Boundary validation
- Obstacle avoidance
- Movement types (walk, run, wander, patrol, teleport)
- AI decision generation
- Authentication and error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def sample_movement_request():
    """Sample movement request."""
    return {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk",
        "avoid_obstacles": True
    }


@pytest.fixture
def sample_boundaries():
    """Sample boundary constraints."""
    return {
        "min_x": 0.0,
        "max_x": 500.0,
        "min_y": 0.0,
        "max_y": 500.0
    }


# ============================================================================
# MOVEMENT COMPUTATION TESTS
# ============================================================================

@pytest.mark.unit
def test_compute_movement_success(test_client, sample_movement_request):
    """Test successful movement computation."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "path" in data["data"]
    assert "distance" in data["data"]
    assert "valid" in data["data"]


@pytest.mark.unit
def test_compute_walk_movement(test_client):
    """Test walk movement type."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 50.0, "y": 50.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_compute_run_movement(test_client):
    """Test run movement type."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 200.0, "y": 200.0},
        "movement_type": "run"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_compute_teleport_movement(test_client):
    """Test teleport movement type."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 500.0, "y": 500.0},
        "movement_type": "teleport"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200
    data = response.json()
    # Teleport should have minimal duration
    assert data["data"]["estimated_duration"] < 1.0


@pytest.mark.unit
def test_compute_wander_movement(test_client):
    """Test wander movement type."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 50.0, "y": 50.0},
        "movement_type": "wander",
        "max_distance": 20.0
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


# ============================================================================
# BOUNDARY VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_with_boundaries(test_client, sample_boundaries):
    """Test movement with boundary constraints."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 100.0, "y": 100.0},
        "target_position": {"x": 200.0, "y": 200.0},
        "movement_type": "walk",
        "boundaries": sample_boundaries
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_movement_outside_boundaries(test_client, sample_boundaries):
    """Test movement to position outside boundaries."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 100.0, "y": 100.0},
        "target_position": {"x": 1000.0, "y": 1000.0},  # Outside bounds
        "movement_type": "walk",
        "boundaries": sample_boundaries
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200
    data = response.json()
    # Should include boundary warning
    assert len(data["data"]["warnings"]) > 0


@pytest.mark.unit
def test_current_position_outside_boundaries(test_client, sample_boundaries):
    """Test movement starting from position outside boundaries."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": -10.0, "y": -10.0},  # Outside
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk",
        "boundaries": sample_boundaries
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


# ============================================================================
# OBSTACLE AVOIDANCE TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_with_obstacles(test_client):
    """Test movement with obstacle avoidance."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk",
        "avoid_obstacles": True,
        "obstacle_positions": [
            {"x": 50.0, "y": 50.0},
            {"x": 75.0, "y": 75.0}
        ]
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_movement_ignore_obstacles(test_client):
    """Test movement without obstacle avoidance."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk",
        "avoid_obstacles": False
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


# ============================================================================
# PATROL ROUTE TESTS
# ============================================================================

@pytest.mark.unit
def test_set_patrol_route_success(test_client):
    """Test setting patrol route."""
    route = {
        "waypoints": [
            {"x": 0.0, "y": 0.0},
            {"x": 100.0, "y": 0.0},
            {"x": 100.0, "y": 100.0},
            {"x": 0.0, "y": 100.0}
        ],
        "loop": True,
        "wait_time": 2.0
    }
    
    response = test_client.post("/api/npc/movement/patrol?npc_id=npc_123", json=route)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_set_patrol_route_no_loop(test_client):
    """Test patrol route without looping."""
    route = {
        "waypoints": [
            {"x": 0.0, "y": 0.0},
            {"x": 100.0, "y": 100.0}
        ],
        "loop": False
    }
    
    response = test_client.post("/api/npc/movement/patrol?npc_id=npc_123", json=route)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_patrol_minimum_waypoints(test_client):
    """Test patrol with minimum waypoints."""
    route = {
        "waypoints": [
            {"x": 0.0, "y": 0.0},
            {"x": 50.0, "y": 50.0}
        ]
    }
    
    response = test_client.post("/api/npc/movement/patrol?npc_id=npc_123", json=route)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_patrol_insufficient_waypoints(test_client):
    """Test patrol with insufficient waypoints."""
    route = {
        "waypoints": [
            {"x": 0.0, "y": 0.0}
        ]  # Less than minimum of 2
    }
    
    response = test_client.post("/api/npc/movement/patrol?npc_id=npc_123", json=route)
    
    assert response.status_code == 422


# ============================================================================
# MOVEMENT DECISION TESTS
# ============================================================================

@pytest.mark.unit
def test_get_movement_decisions(test_client):
    """Test getting movement decisions for NPC."""
    response = test_client.get("/api/npc/movement/decisions/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "decisions" in data["data"]


@pytest.mark.unit
def test_decisions_include_reasoning(test_client):
    """Test that decisions include reasoning."""
    response = test_client.get("/api/npc/movement/decisions/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    decisions = data["data"]["decisions"]
    if len(decisions) > 0:
        assert "reason" in decisions[0]


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_missing_npc_id(test_client):
    """Test movement without npc_id."""
    request = {
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_movement_missing_current_position(test_client):
    """Test movement without current position."""
    request = {
        "npc_id": "npc_123",
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_movement_invalid_type(test_client):
    """Test movement with invalid type."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "invalid_type"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_non_wander_without_target(test_client):
    """Test non-wander movement without target."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    # Should fail validation
    assert response.status_code in [400, 422]


@pytest.mark.unit
def test_teleport_without_target(test_client):
    """Test teleport without target position."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "movement_type": "teleport"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    # Should fail validation
    assert response.status_code in [400, 422]


# ============================================================================
# PATH VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_path_includes_waypoints(test_client, sample_movement_request):
    """Test that path includes waypoints."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    assert len(data["data"]["path"]) > 0


@pytest.mark.unit
def test_path_estimated_duration(test_client, sample_movement_request):
    """Test path includes duration estimate."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "estimated_duration" in data["data"]


@pytest.mark.unit
def test_path_distance_calculation(test_client, sample_movement_request):
    """Test path distance calculation."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    assert data["data"]["distance"] > 0


# ============================================================================
# MOVEMENT TYPE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("movement_type", [
    "walk", "run", "teleport", "wander", "patrol", "chase", "flee"
])
def test_all_movement_types(test_client, movement_type):
    """Test all valid movement types."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "movement_type": movement_type
    }
    
    # Add target for non-wander types
    if movement_type != "wander":
        request["target_position"] = {"x": 100.0, "y": 100.0}
    else:
        request["max_distance"] = 20.0
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


# ============================================================================
# AI DECISION TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_includes_decisions(test_client, sample_movement_request):
    """Test that movement response includes AI decisions."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "decisions" in data["data"]


@pytest.mark.unit
def test_decisions_have_priority(test_client, sample_movement_request):
    """Test that decisions include priority."""
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    assert response.status_code == 200
    data = response.json()
    decisions = data["data"]["decisions"]
    if len(decisions) > 0:
        assert "priority" in decisions[0]


# ============================================================================
# OBSTACLE HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_blocked_path_handling(test_client):
    """Test handling of blocked path."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 10.0, "y": 10.0},
        "movement_type": "walk",
        "avoid_obstacles": True,
        "obstacle_positions": [{"x": 5.0, "y": 5.0}]  # Blocking obstacle
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200
    data = response.json()
    # Should include warning about obstacle
    assert len(data["data"]["warnings"]) > 0


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_server_error(test_client):
    """Test movement with server error."""
    request = {
        "npc_id": "error_npc",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code in [200, 400, 500]


@pytest.mark.unit
def test_malformed_movement_request(test_client):
    """Test malformed movement request."""
    response = test_client.post(
        "/api/npc/movement/movement",
        data="not json",
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status_code == 422


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_movement_negative_coordinates(test_client):
    """Test movement with negative coordinates."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": -50.0, "y": -50.0},
        "target_position": {"x": 50.0, "y": 50.0},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_movement_very_small_distance(test_client):
    """Test movement over very small distance."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 0.1, "y": 0.1},
        "movement_type": "walk"
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_wander_with_max_distance(test_client):
    """Test wander with maximum distance constraint."""
    request = {
        "npc_id": "npc_123",
        "current_position": {"x": 50.0, "y": 50.0},
        "movement_type": "wander",
        "max_distance": 10.0
    }
    
    response = test_client.post("/api/npc/movement/movement", json=request)
    
    assert response.status_code == 200


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_patrol_to_decisions_flow(test_client):
    """Test flow from setting patrol to checking decisions."""
    # Set patrol route
    route = {
        "waypoints": [
            {"x": 0.0, "y": 0.0},
            {"x": 100.0, "y": 0.0},
            {"x": 100.0, "y": 100.0}
        ],
        "loop": True
    }
    
    patrol_response = test_client.post(
        "/api/npc/movement/patrol?npc_id=npc_123",
        json=route
    )
    assert patrol_response.status_code == 200
    
    # Check decisions
    decisions_response = test_client.get("/api/npc/movement/decisions/npc_123")
    assert decisions_response.status_code == 200


@pytest.mark.integration
def test_movement_computation_flow(test_client):
    """Test complete movement computation flow."""
    # Compute movement
    movement_request = {
        "npc_id": "npc_123",
        "current_position": {"x": 0.0, "y": 0.0},
        "target_position": {"x": 100.0, "y": 100.0},
        "movement_type": "walk"
    }
    
    movement_response = test_client.post(
        "/api/npc/movement/movement",
        json=movement_request
    )
    assert movement_response.status_code == 200
    
    # Get decisions
    decisions_response = test_client.get("/api/npc/movement/decisions/npc_123")
    assert decisions_response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_movement_computation_performance(test_client, performance_timer, sample_movement_request):
    """Test movement computation performance."""
    performance_timer.start()
    
    response = test_client.post("/api/npc/movement/movement", json=sample_movement_request)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000


@pytest.mark.performance
def test_patrol_setup_performance(test_client, performance_timer):
    """Test patrol route setup performance."""
    route = {
        "waypoints": [{"x": float(i*10), "y": float(i*10)} for i in range(10)],
        "loop": True
    }
    
    performance_timer.start()
    
    response = test_client.post("/api/npc/movement/patrol?npc_id=npc_123", json=route)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500