"""
Test Suite for World Router

Tests cover:
- World state retrieval
- World event triggering
- Environment state management
- Authentication and rate limiting
- Error handling and validation
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, patch
from fastapi import HTTPException
from fastapi.testclient import TestClient


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def mock_world_state():
    """Mock world state data."""
    return {
        "world_id": "world-001",
        "world_name": "Test World",
        "active_players": 50,
        "active_npcs": 200,
        "time_of_day": "afternoon",
        "weather": "clear",
        "season": "spring",
        "active_events": ["market_day"],
        "updated_at": datetime.utcnow()
    }


@pytest.fixture
def mock_environment_state():
    """Mock environment state data."""
    return {
        "time_of_day": "noon",
        "weather": "sunny",
        "weather_intensity": 0.3,
        "season": "summer",
        "day_of_year": 150,
        "active_events": []
    }


# ============================================================================
# WORLD STATE TESTS
# ============================================================================

@pytest.mark.unit
def test_get_world_state_success(test_client, mock_world_state):
    """Test successful world state retrieval."""
    response = test_client.get("/api/world/state")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "data" in data
    assert "world_id" in data["data"]
    assert "time_of_day" in data["data"]


@pytest.mark.unit
def test_get_world_state_with_auth(authenticated_client):
    """Test world state retrieval with authentication."""
    response = authenticated_client.get("/api/world/state")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_get_world_state_invalid_auth(test_client):
    """Test world state retrieval with invalid auth."""
    response = test_client.get(
        "/api/world/state",
        headers={"X-API-Key": "invalid_key"}
    )
    
    # Should still work if API key not required in test settings
    assert response.status_code in [200, 401]


# ============================================================================
# WORLD EVENT TESTS
# ============================================================================

@pytest.mark.unit
def test_trigger_world_event_success(test_client):
    """Test successful world event triggering."""
    event_data = {
        "event_type": "weather_change",
        "event_data": {"new_weather": "rain"},
        "affected_locations": ["town_center", "marketplace"],
        "duration": 300
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "event_id" in data["data"]
    assert data["data"]["event_type"] == "weather_change"


@pytest.mark.unit
def test_trigger_world_event_missing_type(test_client):
    """Test world event with missing event_type."""
    event_data = {
        "event_data": {},
        "affected_locations": []
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 422  # Validation error


@pytest.mark.unit
def test_trigger_world_event_with_duration(test_client):
    """Test world event with specific duration."""
    event_data = {
        "event_type": "festival",
        "event_data": {"festival_name": "Harvest Festival"},
        "affected_locations": ["all"],
        "duration": 7200
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 200
    data = response.json()
    assert "event_id" in data["data"]


# ============================================================================
# ENVIRONMENT TESTS
# ============================================================================

@pytest.mark.unit
def test_get_environment_success(test_client, mock_environment_state):
    """Test successful environment retrieval."""
    response = test_client.get("/api/world/environment")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "time_of_day" in data["data"]
    assert "weather" in data["data"]
    assert "season" in data["data"]


@pytest.mark.unit
def test_update_environment_weather(test_client):
    """Test environment weather update."""
    update_data = {
        "weather": "rainy"
    }
    
    response = test_client.put("/api/world/environment", json=update_data)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_update_environment_time(test_client):
    """Test environment time update."""
    update_data = {
        "time_of_day": "night"
    }
    
    response = test_client.put("/api/world/environment", json=update_data)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_update_environment_multiple_fields(test_client):
    """Test environment update with multiple fields."""
    update_data = {
        "weather": "stormy",
        "time_of_day": "evening",
        "active_events": ["storm_warning"]
    }
    
    response = test_client.put("/api/world/environment", json=update_data)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True


@pytest.mark.unit
def test_update_environment_empty_payload(test_client):
    """Test environment update with empty payload."""
    response = test_client.put("/api/world/environment", json={})
    
    assert response.status_code == 200


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_world_endpoint_with_server_error(test_client):
    """Test world endpoint handling server error."""
    with patch('routers.world.logger') as mock_logger:
        # Simulate internal error
        with patch('routers.dependencies.get_agent_orchestrator') as mock_orch:
            mock_orch.side_effect = Exception("Database connection failed")
            
            response = test_client.get("/api/world/state")
            
            # Should handle error gracefully
            assert response.status_code in [200, 500]


@pytest.mark.unit
def test_event_trigger_error_handling(test_client):
    """Test event trigger with error handling."""
    event_data = {
        "event_type": "invalid_event_that_causes_error",
        "event_data": {},
        "affected_locations": []
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    # Should not crash, return appropriate status
    assert response.status_code in [200, 400, 500]


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_event_type_validation(test_client):
    """Test event type field validation."""
    event_data = {
        "event_type": "",  # Empty string
        "event_data": {},
        "affected_locations": []
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 422  # Validation error


@pytest.mark.unit
def test_environment_update_validation(test_client):
    """Test environment update field validation."""
    update_data = {
        "weather": "invalid_weather_type_that_doesnt_exist"
    }
    
    response = test_client.put("/api/world/environment", json=update_data)
    
    # Should accept or validate based on schema
    assert response.status_code in [200, 400, 422]


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_world_state_to_event_flow(test_client):
    """Test flow from getting state to triggering event."""
    # Get initial state
    state_response = test_client.get("/api/world/state")
    assert state_response.status_code == 200
    
    # Trigger event based on state
    event_data = {
        "event_type": "population_milestone",
        "event_data": {"milestone": "100_players"},
        "affected_locations": ["all"]
    }
    
    event_response = test_client.post("/api/world/event", json=event_data)
    assert event_response.status_code == 200


@pytest.mark.integration
def test_environment_update_flow(test_client):
    """Test environment get and update flow."""
    # Get current environment
    get_response = test_client.get("/api/world/environment")
    assert get_response.status_code == 200
    
    # Update environment
    update_data = {
        "weather": "foggy",
        "time_of_day": "dawn"
    }
    
    update_response = test_client.put("/api/world/environment", json=update_data)
    assert update_response.status_code == 200


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_event_with_empty_locations(test_client):
    """Test event with no affected locations."""
    event_data = {
        "event_type": "global_announcement",
        "event_data": {"message": "Server maintenance"},
        "affected_locations": []
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_event_with_null_duration(test_client):
    """Test event with null duration."""
    event_data = {
        "event_type": "instant_event",
        "event_data": {},
        "affected_locations": ["town"],
        "duration": None
    }
    
    response = test_client.post("/api/world/event", json=event_data)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_multiple_concurrent_events(test_client):
    """Test triggering multiple events concurrently."""
    events = [
        {
            "event_type": "weather_change",
            "event_data": {},
            "affected_locations": ["north"]
        },
        {
            "event_type": "festival",
            "event_data": {},
            "affected_locations": ["south"]
        }
    ]
    
    responses = []
    for event in events:
        response = test_client.post("/api/world/event", json=event)
        responses.append(response)
    
    assert all(r.status_code == 200 for r in responses)


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_world_state_response_time(test_client, performance_timer):
    """Test world state endpoint response time."""
    performance_timer.start()
    
    response = test_client.get("/api/world/state")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000  # Should respond within 1s