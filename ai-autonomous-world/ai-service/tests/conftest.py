"""
Pytest configuration and fixtures for all tests
"""

import pytest
import asyncio
from typing import AsyncGenerator, Generator
from unittest.mock import Mock, AsyncMock, MagicMock
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


@pytest.fixture(scope="session")
def event_loop():
    """Create an instance of the default event loop for the test session."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()


@pytest.fixture
def mock_settings():
    """Mock settings for testing"""
    from config import Settings
    settings = Settings(
        service_name="test-service",
        service_host="127.0.0.1",
        service_port=8001,
        environment="test",
        debug=True,
        redis_host="127.0.0.1",
        redis_port=6379,
        redis_db=1,  # Use different DB for tests
        default_llm_provider="openai",
        openai_api_key="test-key-123",
        azure_openai_api_key="test-azure-key",
        azure_openai_endpoint="https://test.openai.azure.com/",
        cors_origins=["http://localhost:8001"],
        max_movement_distance=50,
        dialogue_temperature=0.8,
        llm_timeout=30.0,
        llm_max_retries=3,
    )
    return settings


@pytest.fixture
async def mock_database():
    """Mock database for testing"""
    db = AsyncMock()
    db.client = AsyncMock()
    db.client.get = AsyncMock(return_value=None)
    db.client.set = AsyncMock(return_value=True)
    db.client.setex = AsyncMock(return_value=True)
    db.client.delete = AsyncMock(return_value=1)
    db.client.exists = AsyncMock(return_value=0)
    db.client.hget = AsyncMock(return_value=None)
    db.client.hset = AsyncMock(return_value=1)
    db.client.hgetall = AsyncMock(return_value={})
    db.client.sadd = AsyncMock(return_value=1)
    db.client.smembers = AsyncMock(return_value=set())
    db.client.srem = AsyncMock(return_value=1)
    return db


@pytest.fixture
def mock_llm_provider():
    """Mock LLM provider for testing"""
    provider = AsyncMock()
    provider.generate = AsyncMock(return_value=Mock(
        text="Test response",
        usage={"prompt_tokens": 10, "completion_tokens": 20, "total_tokens": 30},
        model="gpt-4"
    ))
    provider.generate_chat = AsyncMock(return_value=Mock(
        text="Test chat response",
        usage={"prompt_tokens": 15, "completion_tokens": 25, "total_tokens": 40},
        model="gpt-4"
    ))
    return provider


@pytest.fixture
def sample_npc_data():
    """Sample NPC data for testing"""
    return {
        "npc_id": "test_npc_001",
        "name": "Test Merchant",
        "level": 50,
        "npc_class": "merchant",
        "position": {"map": "prontera", "x": 150, "y": 180},
        "personality": {
            "traits": ["friendly", "helpful", "greedy"],
            "background": "A merchant who loves gold",
            "goals": ["Make profit", "Help adventurers"],
            "speech_style": "Cheerful and enthusiastic"
        }
    }


@pytest.fixture
def sample_player_data():
    """Sample player data for testing"""
    return {
        "player_id": "player_001",
        "player_name": "TestHero",
        "player_level": 75,
        "player_class": "swordsman",
        "position": {"map": "prontera", "x": 151, "y": 181}
    }


@pytest.fixture
def sample_interaction_context():
    """Sample interaction context for testing"""
    return {
        "interaction_type": "talk",
        "location": {"map": "prontera", "x": 150, "y": 180},
        "time_of_day": "afternoon",
        "weather": "sunny",
        "nearby_npcs": [],
        "nearby_players": 5
    }


@pytest.fixture
async def mock_agent_context(sample_npc_data, sample_player_data):
    """Mock agent context for testing"""
    from agents.base_agent import AgentContext
    from datetime import datetime
    
    return AgentContext(
        npc_id=sample_npc_data["npc_id"],
        npc_name=sample_npc_data["name"],
        personality=sample_npc_data["personality"],
        current_state={"health": 100, "mood": "happy"},
        world_state={"economy": "stable", "politics": "peaceful"},
        recent_events=["Player greeted NPC", "NPC sold item"],
        timestamp=datetime.utcnow()
    )


@pytest.fixture
def mock_fastapi_app():
    """Mock FastAPI app for testing"""
    from fastapi import FastAPI
    from fastapi.testclient import TestClient
    
    app = FastAPI()
    
    @app.get("/health")
    async def health():
        return {"status": "healthy"}
    
    return TestClient(app)

