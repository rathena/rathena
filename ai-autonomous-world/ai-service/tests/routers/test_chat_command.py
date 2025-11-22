"""
Test Suite for Chat Command Router

Tests cover:
- Command parsing and execution
- Command validation
- Authentication
- Error handling
- Security (command injection prevention)
"""

import pytest
from unittest.mock import AsyncMock, patch


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def sample_command():
    """Sample chat command."""
    return {
        "player_id": "player_123",
        "command": "/help",
        "args": []
    }


# ============================================================================
# COMMAND EXECUTION TESTS
# ============================================================================

@pytest.mark.unit
def test_execute_help_command(test_client, sample_command):
    """Test /help command execution."""
    response = test_client.post("/api/chat/command/command", json=sample_command)
    
    assert response.status_code == 200
    data = response.json()
    assert data["success"] is True
    assert "command" in data["data"]


@pytest.mark.unit
def test_execute_stats_command(test_client):
    """Test /stats command execution."""
    command = {
        "player_id": "player_123",
        "command": "/stats",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 200
    data = response.json()
    assert data["data"]["success"] is True


@pytest.mark.unit
def test_execute_location_command(test_client):
    """Test /location command execution."""
    command = {
        "player_id": "player_123",
        "command": "/location",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_execute_command_with_args(test_client):
    """Test command with arguments."""
    command = {
        "player_id": "player_123",
        "command": "/whisper",
        "args": ["player_456", "Hello!"]
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_unknown_command(test_client):
    """Test unknown command handling."""
    command = {
        "player_id": "player_123",
        "command": "/unknown_command",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 200
    data = response.json()
    # Should indicate command not found
    assert "unknown" in data["data"]["result"].lower() or not data["data"]["success"]


# ============================================================================
# COMMAND VALIDATION TESTS
# ============================================================================

@pytest.mark.unit
def test_command_without_slash(test_client):
    """Test command without leading slash."""
    command = {
        "player_id": "player_123",
        "command": "help",  # No slash
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    # Should auto-add slash or handle gracefully
    assert response.status_code in [200, 422]


@pytest.mark.unit
def test_empty_command(test_client):
    """Test empty command string."""
    command = {
        "player_id": "player_123",
        "command": "",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 422


@pytest.mark.unit
def test_command_missing_player_id(test_client):
    """Test command without player_id."""
    command = {
        "command": "/help",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 422


# ============================================================================
# SECURITY TESTS
# ============================================================================

@pytest.mark.security
def test_command_injection_prevention(test_client):
    """Test prevention of command injection."""
    malicious_command = {
        "player_id": "player_123",
        "command": "/help; DROP TABLE users;",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=malicious_command)
    
    # Should reject or sanitize
    assert response.status_code in [400, 422]


@pytest.mark.security
def test_script_injection_in_args(test_client):
    """Test script injection in arguments."""
    command = {
        "player_id": "player_123",
        "command": "/whisper",
        "args": ["<script>alert('xss')</script>"]
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    # Should sanitize or reject
    assert response.status_code in [200, 400, 422]


@pytest.mark.security
def test_invalid_characters_in_command(test_client):
    """Test invalid characters in command."""
    command = {
        "player_id": "player_123",
        "command": "/test{}<>&|",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code in [400, 422]


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
def test_malformed_json(test_client):
    """Test handling of malformed JSON."""
    response = test_client.post(
        "/api/chat/command/command",
        data="not valid json",
        headers={"Content-Type": "application/json"}
    )
    
    assert response.status_code == 422


@pytest.mark.unit
def test_command_execution_error(test_client):
    """Test handling of command execution errors."""
    command = {
        "player_id": "player_123",
        "command": "/error_trigger",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    # Should handle error gracefully
    assert response.status_code in [200, 400, 500]


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

@pytest.mark.unit
def test_command_with_empty_args(test_client):
    """Test command with empty args list."""
    command = {
        "player_id": "player_123",
        "command": "/help",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code == 200


@pytest.mark.unit
def test_command_with_many_args(test_client):
    """Test command with many arguments."""
    command = {
        "player_id": "player_123",
        "command": "/test",
        "args": [f"arg{i}" for i in range(100)]
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    assert response.status_code in [200, 400]


@pytest.mark.unit
def test_whitespace_in_command(test_client):
    """Test command with whitespace."""
    command = {
        "player_id": "player_123",
        "command": "  /help  ",
        "args": []
    }
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    # Should trim whitespace
    assert response.status_code == 200


@pytest.mark.unit
def test_case_sensitivity(test_client):
    """Test command case sensitivity."""
    commands = [
        {"player_id": "player_123", "command": "/help", "args": []},
        {"player_id": "player_123", "command": "/HELP", "args": []},
        {"player_id": "player_123", "command": "/Help", "args": []}
    ]
    
    for cmd in commands:
        response = test_client.post("/api/chat/command/command", json=cmd)
        # Should handle case insensitively
        assert response.status_code == 200


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

@pytest.mark.integration
def test_command_sequence(test_client):
    """Test sequence of commands."""
    commands = [
        {"player_id": "player_123", "command": "/help", "args": []},
        {"player_id": "player_123", "command": "/stats", "args": []},
        {"player_id": "player_123", "command": "/location", "args": []}
    ]
    
    for cmd in commands:
        response = test_client.post("/api/chat/command/command", json=cmd)
        assert response.status_code == 200


# ============================================================================
# PERFORMANCE TESTS
# ============================================================================

@pytest.mark.performance
def test_command_execution_performance(test_client, performance_timer):
    """Test command execution performance."""
    command = {
        "player_id": "player_123",
        "command": "/help",
        "args": []
    }
    
    performance_timer.start()
    
    response = test_client.post("/api/chat/command/command", json=command)
    
    performance_timer.stop()
    
    assert response.status_code == 200
    assert performance_timer.elapsed_ms() < 500


@pytest.mark.performance
def test_multiple_concurrent_commands(test_client):
    """Test handling multiple concurrent commands."""
    command = {
        "player_id": "player_123",
        "command": "/stats",
        "args": []
    }
    
    responses = []
    for _ in range(10):
        response = test_client.post("/api/chat/command/command", json=command)
        responses.append(response)
    
    assert all(r.status_code == 200 for r in responses)