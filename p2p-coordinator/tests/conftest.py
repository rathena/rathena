"""
Pytest configuration and shared fixtures for P2P Coordinator tests.
"""

import asyncio
import os
import sys
from typing import AsyncGenerator, Generator
from unittest.mock import AsyncMock, MagicMock

import pytest
from fastapi.testclient import TestClient
from httpx import AsyncClient, ASGITransport
from sqlalchemy.ext.asyncio import AsyncSession, create_async_engine, async_sessionmaker
from redis.asyncio import Redis

# Add coordinator-service to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "coordinator-service"))

from main import app
from config import settings
from database import db_manager


# ============================================================================
# Test Configuration
# ============================================================================

@pytest.fixture(scope="session")
def event_loop() -> Generator:
    """Create an instance of the default event loop for the test session."""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()


@pytest.fixture(scope="session")
def test_settings():
    """Override settings for testing."""
    # Use test database
    os.environ["DATABASE_URL"] = "postgresql+asyncpg://test_user:test_pass@localhost:5432/test_p2p_coordinator"
    os.environ["REDIS_URL"] = "redis://localhost:6379/15"  # Use DB 15 for tests
    os.environ["ENVIRONMENT"] = "testing"
    os.environ["DEBUG"] = "true"
    os.environ["JWT_VALIDATION_ENABLED"] = "false"
    os.environ["API_KEY_VALIDATION_ENABLED"] = "false"
    
    # Reload settings
    from config import Settings
    return Settings()


# ============================================================================
# Database Fixtures
# ============================================================================

@pytest.fixture(scope="function")
async def db_session(test_settings) -> AsyncGenerator[AsyncSession, None]:
    """Create a test database session with transaction rollback."""
    # Create test engine
    engine = create_async_engine(
        test_settings.database.postgres_url,
        echo=False,
        pool_pre_ping=True,
    )

    # Create connection and begin transaction
    async with engine.begin() as connection:
        # Create session bound to the transaction
        async_session_factory = async_sessionmaker(
            bind=connection,
            class_=AsyncSession,
            expire_on_commit=False,
        )

        async with async_session_factory() as session:
            # Begin nested transaction
            await session.begin_nested()

            yield session

            # Rollback everything
            await session.rollback()

    # Cleanup
    await engine.dispose()


@pytest.fixture(scope="function")
async def mock_db_manager():
    """Mock database manager for tests that don't need real DB."""
    mock_manager = MagicMock()
    mock_manager.get_session = AsyncMock()
    mock_manager.init_db = AsyncMock()
    mock_manager.close = AsyncMock()
    return mock_manager


# ============================================================================
# Redis Fixtures
# ============================================================================

@pytest.fixture(scope="function")
async def redis_client(test_settings) -> AsyncGenerator[Redis, None]:
    """Create a test Redis client."""
    client = Redis.from_url(
        test_settings.redis.url,
        encoding="utf-8",
        decode_responses=True,
    )
    
    yield client
    
    # Cleanup - flush test database
    await client.flushdb()
    await client.aclose()


@pytest.fixture(scope="function")
async def mock_redis_client():
    """Mock Redis client for tests that don't need real Redis."""
    mock_client = AsyncMock()
    mock_client.get = AsyncMock(return_value=None)
    mock_client.set = AsyncMock(return_value=True)
    mock_client.delete = AsyncMock(return_value=1)
    mock_client.exists = AsyncMock(return_value=0)
    mock_client.hget = AsyncMock(return_value=None)
    mock_client.hset = AsyncMock(return_value=1)
    mock_client.hgetall = AsyncMock(return_value={})
    mock_client.hdel = AsyncMock(return_value=1)
    mock_client.sadd = AsyncMock(return_value=1)
    mock_client.smembers = AsyncMock(return_value=set())
    mock_client.srem = AsyncMock(return_value=1)
    return mock_client


# ============================================================================
# FastAPI Test Client Fixtures
# ============================================================================

@pytest.fixture(scope="function")
def test_client() -> Generator[TestClient, None, None]:
    """Create a synchronous test client for FastAPI."""
    with TestClient(app) as client:
        yield client


@pytest.fixture(scope="function")
async def async_client(test_settings, request) -> AsyncGenerator[AsyncClient, None]:
    """Create an async test client for FastAPI with database initialization."""
    # Check if this is an e2e test (which uses test_client/websocket_client)
    # E2E tests have the @pytest.mark.e2e marker
    is_e2e_test = request.node.get_closest_marker("e2e") is not None

    # Only initialize database if not an e2e test
    # E2E tests use test_client which runs the app's lifespan (which initializes DB)
    if not is_e2e_test:
        await db_manager.initialize()

    try:
        async with AsyncClient(
            transport=ASGITransport(app=app),
            base_url="http://test",
        ) as client:
            yield client
    finally:
        # Only close database if we initialized it
        if not is_e2e_test:
            await db_manager.close()


# ============================================================================
# WebSocket Test Fixtures
# ============================================================================

@pytest.fixture(scope="function")
def websocket_client(test_client):
    """Create a WebSocket test client."""
    return test_client


# ============================================================================
# Mock Service Fixtures
# ============================================================================

@pytest.fixture(scope="function")
def mock_host_registry():
    """Mock HostRegistryService."""
    from services.host_registry import HostRegistryService
    mock_service = AsyncMock(spec=HostRegistryService)
    return mock_service


@pytest.fixture(scope="function")
def mock_zone_manager():
    """Mock ZoneManagerService."""
    from services.zone_manager import ZoneManagerService
    mock_service = AsyncMock(spec=ZoneManagerService)
    return mock_service


@pytest.fixture(scope="function")
def mock_session_manager():
    """Mock SessionManagerService."""
    from services.session_manager import SessionManagerService
    mock_service = AsyncMock(spec=SessionManagerService)
    return mock_service


@pytest.fixture(scope="function")
def mock_signaling_service():
    """Mock SignalingService."""
    from services.signaling import SignalingService
    mock_service = AsyncMock(spec=SignalingService)
    return mock_service


# ============================================================================
# Test Data Fixtures
# ============================================================================

@pytest.fixture
def sample_host_data():
    """Sample host registration data."""
    return {
        "host_id": "test_host_001",
        "player_id": "test_player_123",
        "player_name": "TestPlayer",
        "ip_address": "192.168.1.100",
        "port": 7777,
        "cpu_cores": 8,
        "cpu_frequency_mhz": 3600,
        "memory_mb": 16384,
        "network_speed_mbps": 1000,
    }


@pytest.fixture
def sample_zone_data():
    """Sample zone data."""
    return {
        "zone_id": "prontera",
        "zone_name": "Prontera",
        "max_players": 100,
        "p2p_enabled": True,
    }


@pytest.fixture
def sample_session_data():
    """Sample session data."""
    return {
        "session_id": "test_session_001",
        "host_id": "test_host_001",
        "zone_id": "prontera",
        "max_players": 50,
    }


@pytest.fixture
def sample_jwt_token():
    """Sample JWT token for testing."""
    import jwt
    from datetime import datetime, timedelta

    payload = {
        "sub": "test_player_123",
        "player_id": "test_player_123",
        "exp": datetime.utcnow() + timedelta(hours=1),
    }

    token = jwt.encode(
        payload,
        settings.security.jwt_secret_key,
        algorithm=settings.security.jwt_algorithm,
    )

    return token


@pytest.fixture
def sample_auth_request():
    """Sample authentication request data."""
    return {
        "player_id": "test_player_123",
        "user_id": "test_user_456",
        "api_key": settings.security.coordinator_api_key,
    }

