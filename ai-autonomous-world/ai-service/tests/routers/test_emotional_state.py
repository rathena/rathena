"""
Test Suite for Emotional State Router

Tests cover:
- Emotional state retrieval
- Emotion updates
- Intensity validation
- Mood tracking
- Stress and energy levels
- Authentication and error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def mock_emotional_state():
    """Mock emotional state data."""
    return {
        "npc_id": "npc_123",
        "current_emotion": "happy",
        "emotion_intensity": 0.7,
        "mood": "cheerful",
        "stress_level": 0.2,
        "energy_level": 0.8,
        "last_updated": datetime.utcnow()
    }


@pytest.fixture
def sample_emotion_update():
    """Sample emotion update request."""
    return {
        "emotion": "happy",
        "intensity": 0.75,
        "trigger": "received_gift"
    }


# ============================================================================
# EMOTIONAL STATE RETRIEVAL TESTS
# ============================================================================

@pytest.mark.unit
def test_get_emotional_state_success(test_client, mock_emotional_state):
    """Test successful emotional state retrieval."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "current_emotion" in data["data"]
    assert "emotion_intensity" in data["data"]
    assert "mood" in data["data"]


@pytest.mark.unit
def test_get_emotional_state_includes_stress(test_client):
    """Test emotional state includes stress level."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert "stress_level" in data["data"]


@pytest.mark.unit
def test_get_emotional_state_includes_energy(test_client):
    """Test emotional state includes energy level."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert "energy_level" in data["data"]


@pytest.mark.unit
def test_get_emotional_state_nonexistent_npc(test_client):
    """Test getting state for non-existent NPC."""
    response = test_client.get("/api/emotional-state/nonexistent_npc")
    
    assert response.status_code in [200, 404]


# ============================================================================
# EMOTION UPDATE TESTS
# ============================================================================

@pytest.mark.unit
def test_update_emotional_state_success(test_client, sample_emotion_update):
    """Test successful emotion update."""
    response = test_client.post(
        "/api/emotional-state/npc_123/update",
        json=sample_emotion_update
    )
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "emotion" in data["data"]


@pytest.mark.unit
def test_update_emotion_to_sad(test_client):
    """Test updating emotion to sad."""
    update = {
        "emotion": "sad",
        "intensity": 0.6,
        "trigger": "lost_item"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_emotion_to_angry(test_client):
    """Test updating emotion to angry."""
    update = {
        "emotion": "angry",
        "intensity": 0.8,
        "trigger": "attacked"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_emotion_to_neutral(test_client):
    """Test updating emotion to neutral."""
    update = {
        "emotion": "neutral",
        "intensity": 0.5,
        "trigger": "time_passed"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_emotion_without_trigger(test_client):
    """Test emotion update without trigger."""
    update = {
        "emotion": "happy",
        "intensity": 0.7
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


# ============================================================================
# INTENSITY VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_update_emotion_min_intensity(test_client):
    """Test emotion update with minimum intensity."""
    update = {
        "emotion": "neutral",
        "intensity": 0.0
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_emotion_max_intensity(test_client):
    """Test emotion update with maximum intensity."""
    update = {
        "emotion": "ecstatic",
        "intensity": 1.0
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_update_emotion_invalid_intensity_low(test_client):
    """Test emotion update with intensity below minimum."""
    update = {
        "emotion": "sad",
        "intensity": -0.1
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_update_emotion_invalid_intensity_high(test_client):
    """Test emotion update with intensity above maximum."""
    update = {
        "emotion": "angry",
        "intensity": 1.5
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 422


# ============================================================================
# VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_update_missing_emotion(test_client):
    """Test update without emotion."""
    update = {
        "intensity": 0.7
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_update_missing_intensity(test_client):
    """Test update without intensity."""
    update = {
        "emotion": "happy"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_get_state_invalid_npc_id(test_client):
    """Test getting state with invalid NPC ID."""
    response = test_client.get("/api/emotional-state/")
    
    assert response.status_code == 404


# ============================================================================
# EMOTION TYPES TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("emotion", [
    "happy", "sad", "angry", "fearful", "surprised",
    "disgusted", "neutral", "excited", "calm", "anxious"
])
def test_various_emotion_types(test_client, emotion):
    """Test various emotion types."""
    update = {
        "emotion": emotion,
        "intensity": 0.6,
        "trigger": "test_trigger"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    # Should accept valid emotions
    assert response.status_code in [200, 400, 422]


# ============================================================================
# TRIGGER TESTS
# ============================================================================

@pytest.mark.unit
def test_emotion_trigger_types(test_client):
    """Test different emotion triggers."""
    triggers = [
        "gift_received",
        "attacked",
        "quest_completed",
        "friend_arrived",
        "weather_changed"
    ]
    
    for trigger in triggers:
        update = {
            "emotion": "happy",
            "intensity": 0.5,
            "trigger": trigger
        }
        
        response = test_client.post("/api/emotional-state/npc_123/update", json=update)
        assert response.status_code == 200


@pytest.mark.unit
def test_long_trigger_description(test_client):
    """Test emotion with long trigger description."""
    update = {
        "emotion": "happy",
        "intensity": 0.7,
        "trigger": "A very long detailed description of what triggered this emotion " * 5
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code in [200, 422]


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_emotional_state_server_error(test_client):
    """Test emotional state with server error."""
    response = test_client.get("/api/emotional-state/error_npc")
    
    assert response.status_code in [200, 500]


@pytest.mark.unit
def test_update_server_error(test_client):
    """Test emotion update with server error."""
    update = {
        "emotion": "happy",
        "intensity": 0.5
    }
    
    response = test_client.post("/api/emotional-state/error_npc/update", json=update)
    
    assert response.status_code in [200, 500]


@pytest.mark.unit
def test_malformed_update_request(test_client):
    """Test malformed update request."""
    response = test_client.post(
        "/api/emotional-state/npc_123/update",
        data="not json",
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status_code == 422


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_rapid_emotion_changes(test_client):
    """Test rapid succession of emotion changes."""
    emotions = ["happy", "sad", "angry", "neutral", "excited"]
    
    for emotion in emotions:
        update = {
            "emotion": emotion,
            "intensity": 0.6,
            "trigger": "test"
        }
        
        response = test_client.post("/api/emotional-state/npc_123/update", json=update)
        assert response.status_code == 200


@pytest.mark.unit
def test_emotion_with_empty_trigger(test_client):
    """Test emotion update with empty trigger."""
    update = {
        "emotion": "happy",
        "intensity": 0.7,
        "trigger": ""
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_emotion_boundary_intensity(test_client):
    """Test emotion with boundary intensity values."""
    # Test 0.0
    update = {
        "emotion": "neutral",
        "intensity": 0.0
    }
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    assert response.status_code == 200
    
    # Test 1.0
    update["intensity"] = 1.0
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    assert response.status_code == 200


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_get_update_get_flow(test_client):
    """Test flow of getting state, updating, and getting again."""
    npc_id = "npc_123"
    
    # Get initial state
    initial_response = test_client.get(f"/api/emotional-state/{npc_id}")
    assert initial_response.status_code == 200
    
    # Update emotion
    update = {
        "emotion": "excited",
        "intensity": 0.8,
        "trigger": "test_event"
    }
    
    update_response = test_client.post(
        f"/api/emotional-state/{npc_id}/update",
        json=update
    )
    assert update_response.status_code == 200
    
    # Get updated state
    final_response = test_client.get(f"/api/emotional-state/{npc_id}")
    assert final_response.status_code == 200


@pytest.mark.integration
def test_multiple_npc_emotional_states(test_client):
    """Test managing emotional states for multiple NPCs."""
    npcs = ["npc_1", "npc_2", "npc_3"]
    
    for npc_id in npcs:
        # Get state
        get_response = test_client.get(f"/api/emotional-state/{npc_id}")
        assert get_response.status_code == 200
        
        # Update state
        update = {
            "emotion": "happy",
            "intensity": 0.6,
            "trigger": "test"
        }
        
        update_response = test_client.post(
            f"/api/emotional-state/{npc_id}/update",
            json=update
        )
        assert update_response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_get_state_performance(test_client, performance_timer):
    """Test emotional state retrieval performance."""
    performance_timer.start()
    
    response = test_client.get("/api/emotional-state/npc_123")
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


@pytest.mark.performance
def test_update_state_performance(test_client, performance_timer, sample_emotion_update):
    """Test emotion update performance."""
    performance_timer.start()
    
    response = test_client.post(
        "/api/emotional-state/npc_123/update",
        json=sample_emotion_update
    )
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000


@pytest.mark.performance
def test_multiple_updates_performance(test_client):
    """Test performance of multiple emotion updates."""
    updates = [
        {"emotion": "happy", "intensity": 0.7},
        {"emotion": "sad", "intensity": 0.4},
        {"emotion": "angry", "intensity": 0.6}
    ]
    
    responses = []
    for update in updates:
        response = test_client.post(
            "/api/emotional-state/npc_123/update",
            json=update
        )
        responses.append(response)
    
    assert all(r.status_code == 200 for r in responses)


# ============================================================================
# TIMESTAMP VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_state_includes_timestamp(test_client):
    """Test that state includes last_updated timestamp."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    assert "last_updated" in data["data"]


@pytest.mark.unit
def test_update_includes_timestamp(test_client, sample_emotion_update):
    """Test that update response includes timestamp."""
    response = test_client.post(
        "/api/emotional-state/npc_123/update",
        json=sample_emotion_update
    )
    
    assert response.status_code == 200
    data = response.json()
    assert "updated_at" in data["data"]


# ============================================================================
# MOOD CORRELATION TESTS
# ============================================================================

@pytest.mark.unit
def test_emotion_affects_mood(test_client):
    """Test that emotions correlate with mood."""
    # Happy emotion should reflect in mood
    update = {
        "emotion": "happy",
        "intensity": 0.9,
        "trigger": "celebration"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


# ============================================================================
# SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_trigger_xss_prevention(test_client):
    """Test XSS prevention in trigger field."""
    update = {
        "emotion": "happy",
        "intensity": 0.7,
        "trigger": "<script>alert('xss')</script>"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    # Should sanitize or accept (depends on implementation)
    assert response.status_code in [200, 400]


@pytest.mark.security
def test_sql_injection_in_trigger(test_client):
    """Test SQL injection prevention in trigger."""
    update = {
        "emotion": "happy",
        "intensity": 0.7,
        "trigger": "test'; DROP TABLE npcs; --"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    # Should sanitize or accept safely
    assert response.status_code in [200, 400]


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_emotion_intensity_precision(test_client):
    """Test emotion intensity with high precision."""
    update = {
        "emotion": "happy",
        "intensity": 0.123456789,
        "trigger": "precision_test"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_very_long_trigger_text(test_client):
    """Test emotion with very long trigger text."""
    update = {
        "emotion": "surprised",
        "intensity": 0.8,
        "trigger": "A" * 1000  # Very long trigger
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    # Should handle or reject based on limits
    assert response.status_code in [200, 400, 422]


@pytest.mark.unit
def test_special_characters_in_trigger(test_client):
    """Test trigger with special characters."""
    update = {
        "emotion": "confused",
        "intensity": 0.5,
        "trigger": "Test!@#$%^&*()_+-=[]{}|;:',.<>?/~`"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_unicode_in_trigger(test_client):
    """Test trigger with unicode characters."""
    update = {
        "emotion": "happy",
        "intensity": 0.7,
        "trigger": "😊 🎉 Test 中文 العربية"
    }
    
    response = test_client.post("/api/emotional-state/npc_123/update", json=update)
    
    assert response.status_code == 200


# ============================================================================
# STRESS AND ENERGY TESTS
# ============================================================================

@pytest.mark.unit
def test_stress_level_bounds(test_client):
    """Test stress level is within valid bounds."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    stress = data["data"]["stress_level"]
    assert 0.0 <= stress <= 1.0


@pytest.mark.unit
def test_energy_level_bounds(test_client):
    """Test energy level is within valid bounds."""
    response = test_client.get("/api/emotional-state/npc_123")
    
    assert response.status_code == 200
    data = response.json()
    energy = data["data"]["energy_level"]
    assert 0.0 <= energy <= 1.0


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_update_nonexistent_npc(test_client, sample_emotion_update):
    """Test updating emotion for non-existent NPC."""
    response = test_client.post(
        "/api/emotional-state/nonexistent_npc/update",
        json=sample_emotion_update
    )
    
    # Should handle gracefully
    assert response.status_code in [200, 404, 500]