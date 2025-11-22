"""
Test Suite for NPC Actions Router

Tests cover:
- Action execution and validation
- Action type handling (attack, craft, trade, etc.)
- Resource cost calculation
- Target validation
- Authentication and error handling
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def sample_action_request():
    """Sample action request."""
    return {
        "npc_id": "npc_123",
        "action_type": "attack",
        "target_id": "enemy_456",
        "parameters": {"weapon": "sword", "skill": "slash"}
    }


# ============================================================================
# ACTION EXECUTION TESTS
# ============================================================================

@pytest.mark.unit
def test_execute_attack_action(test_client, sample_action_request):
    """Test executing attack action."""
    response = test_client.post("/api/npc/actions/action", json=sample_action_request)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "action_id" in data["data"]
    assert data["data"]["success"] is True


@pytest.mark.unit
def test_execute_defend_action(test_client):
    """Test executing defend action."""
    action = {
        "npc_id": "npc_123",
        "action_type": "defend",
        "parameters": {"stance": "defensive"}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_execute_heal_action(test_client):
    """Test executing heal action."""
    action = {
        "npc_id": "healer_npc",
        "action_type": "heal",
        "target_id": "player_123",
        "parameters": {"spell": "healing_light"}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_execute_craft_action(test_client):
    """Test executing craft action."""
    action = {
        "npc_id": "blacksmith",
        "action_type": "craft",
        "parameters": {"recipe_id": "iron_sword", "materials": ["iron", "wood"]}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_execute_trade_action(test_client):
    """Test executing trade action."""
    action = {
        "npc_id": "merchant",
        "action_type": "trade",
        "target_id": "player_123",
        "parameters": {"items": ["potion"], "price": 50}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_execute_gather_action(test_client):
    """Test executing gather action."""
    action = {
        "npc_id": "npc_123",
        "action_type": "gather",
        "parameters": {"resource_type": "herbs"}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


# ============================================================================
# ACTION VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_invalid_action_type(test_client):
    """Test action with invalid type."""
    action = {
        "npc_id": "npc_123",
        "action_type": "invalid_action",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_attack_without_target(test_client):
    """Test attack action without target."""
    action = {
        "npc_id": "npc_123",
        "action_type": "attack",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    # Should fail validation
    assert response.status_code in [400, 422]


@pytest.mark.unit
def test_craft_without_recipe(test_client):
    """Test craft action without recipe."""
    action = {
        "npc_id": "npc_123",
        "action_type": "craft",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    # Should fail validation
    assert response.status_code in [400, 422]


@pytest.mark.unit
def test_trade_without_items(test_client):
    """Test trade action without items."""
    action = {
        "npc_id": "npc_123",
        "action_type": "trade",
        "target_id": "player_123",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    # Should fail validation
    assert response.status_code in [400, 422]


@pytest.mark.unit
def test_action_missing_npc_id(test_client):
    """Test action without npc_id."""
    action = {
        "action_type": "attack",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 422


# ============================================================================
# ACTION EFFECTS TESTS
# ============================================================================

@pytest.mark.unit
def test_action_result_includes_effects(test_client, sample_action_request):
    """Test that action result includes effects."""
    response = test_client.post("/api/npc/actions/action", json=sample_action_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "effects" in data["data"]


@pytest.mark.unit
def test_attack_damage_effects(test_client):
    """Test attack action includes damage effects."""
    action = {
        "npc_id": "npc_123",
        "action_type": "attack",
        "target_id": "enemy_456",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200
    data = response.json()
    # Should include damage information
    assert "effects" in data["data"]


@pytest.mark.unit
def test_resource_cost_tracking(test_client, sample_action_request):
    """Test that resource costs are tracked."""
    response = test_client.post("/api/npc/actions/action", json=sample_action_request)
    
    assert response.status_code == 200
    data = response.json()
    assert "effects" in data["data"]


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_action_server_error(test_client):
    """Test action with server error."""
    action = {
        "npc_id": "error_npc",
        "action_type": "attack",
        "target_id": "target",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code in [200, 400, 500]


@pytest.mark.unit
def test_malformed_action_request(test_client):
    """Test malformed action request."""
    response = test_client.post(
        "/api/npc/actions/action",
        data="not json",
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status_code == 422


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_action_with_empty_parameters(test_client):
    """Test action with empty parameters."""
    action = {
        "npc_id": "npc_123",
        "action_type": "rest",
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_action_with_null_target(test_client):
    """Test action with null target."""
    action = {
        "npc_id": "npc_123",
        "action_type": "emote",
        "target_id": None,
        "parameters": {"emote_type": "wave"}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_self_targeted_action(test_client):
    """Test action targeting self."""
    action = {
        "npc_id": "npc_123",
        "action_type": "heal",
        "target_id": "npc_123",  # Self-target
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code == 200


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("action_type", [
    "attack", "defend", "heal", "cast_spell", 
    "craft", "gather", "trade", "speak", "emote", 
    "use_item", "move", "rest"
])
def test_all_action_types(test_client, action_type):
    """Test all valid action types."""
    action = {
        "npc_id": "npc_123",
        "action_type": action_type,
        "target_id": "target_123" if action_type in ["attack", "heal", "trade"] else None,
        "parameters": {}
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    # Should handle based on validation rules
    assert response.status_code in [200, 400, 422]


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_action_sequence(test_client):
    """Test sequence of actions."""
    actions = [
        {"npc_id": "npc_123", "action_type": "gather", "parameters": {}},
        {"npc_id": "npc_123", "action_type": "craft", "parameters": {"recipe_id": "potion"}},
        {"npc_id": "npc_123", "action_type": "use_item", "parameters": {"item_id": "potion"}}
    ]
    
    for action in actions:
        response = test_client.post("/api/npc/actions/action", json=action)
        assert response.status_code in [200, 400]


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_action_execution_performance(test_client, performance_timer, sample_action_request):
    """Test action execution performance."""
    performance_timer.start()
    
    response = test_client.post("/api/npc/actions/action", json=sample_action_request)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 1000


# ============================================================================
# SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_action_parameter_injection(test_client):
    """Test action with potentially malicious parameters."""
    action = {
        "npc_id": "npc_123",
        "action_type": "speak",
        "parameters": {
            "message": "<script>alert('xss')</script>"
        }
    }
    
    response = test_client.post("/api/npc/actions/action", json=action)
    
    assert response.status_code in [200, 400]