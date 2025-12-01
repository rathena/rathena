"""
Pytest Configuration and Shared Fixtures

This module provides shared fixtures for all test modules including:
- Database connection management
- Test data factories
- Mock AI backend responses
- Cleanup utilities
- Configuration fixtures

Fixtures are organized by scope:
- session: Created once for entire test session
- function: Created fresh for each test function
- module: Created once per test module
"""

import asyncio
import json
import logging
import os
import sys
import uuid
from contextlib import asynccontextmanager
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import Any, AsyncGenerator, Generator, Optional
from unittest.mock import AsyncMock, MagicMock, patch

import aiomysql
import pytest
import pytest_asyncio
from aioresponses import aioresponses
from faker import Faker

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from config import (
    AIServiceConfig,
    Config,
    DatabaseConfig,
    LoggingConfig,
    PollingConfig,
    SecurityConfig,
)
from database import Database

# Initialize faker for test data generation
fake = Faker()

# Configure logging for tests
logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)8s] %(name)s: %(message)s",
)
logger = logging.getLogger(__name__)


# =============================================================================
# Test Configuration
# =============================================================================

@dataclass
class TestConfig:
    """Test environment configuration."""
    
    db_host: str = "localhost"
    db_port: int = 3306
    db_user: str = "ragnarok"
    db_password: str = "ragnarok"
    db_name: str = "ragnarok"
    test_table_prefix: str = "test_ai_"
    ai_backend_url: str = "http://localhost:8080"
    cleanup_on_teardown: bool = True


# =============================================================================
# Session-Scoped Fixtures
# =============================================================================

@pytest.fixture(scope="session")
def event_loop_policy():
    """Return the event loop policy for the session."""
    return asyncio.DefaultEventLoopPolicy()


@pytest.fixture(scope="session")
def test_config() -> TestConfig:
    """
    Provide test configuration from environment or defaults.
    
    Returns:
        TestConfig: Configuration for test environment
    """
    return TestConfig(
        db_host=os.getenv("TEST_DB_HOST", "localhost"),
        db_port=int(os.getenv("TEST_DB_PORT", "3306")),
        db_user=os.getenv("TEST_DB_USER", "ragnarok"),
        db_password=os.getenv("TEST_DB_PASSWORD", "ragnarok"),
        db_name=os.getenv("TEST_DB_NAME", "ragnarok"),
        ai_backend_url=os.getenv("TEST_AI_BACKEND_URL", "http://localhost:8080"),
    )


@pytest.fixture(scope="session")
def database_config(test_config: TestConfig) -> DatabaseConfig:
    """
    Create DatabaseConfig from test configuration.
    
    Args:
        test_config: Test environment configuration
        
    Returns:
        DatabaseConfig: Database configuration for tests
    """
    return DatabaseConfig(
        host=test_config.db_host,
        port=test_config.db_port,
        user=test_config.db_user,
        password=test_config.db_password,
        database=test_config.db_name,
        pool_size=5,
        pool_recycle=300,
        connect_timeout=10,
        read_timeout=30,
        write_timeout=30,
    )


@pytest.fixture(scope="session")
def polling_config() -> PollingConfig:
    """
    Create PollingConfig for tests with faster intervals.
    
    Returns:
        PollingConfig: Polling configuration for tests
    """
    return PollingConfig(
        interval_ms=100,  # Fast polling for tests
        batch_size=10,
        worker_count=2,
        max_retries=3,
        retry_delay_ms=50,
    )


@pytest.fixture(scope="session")
def security_config() -> SecurityConfig:
    """
    Create SecurityConfig for tests.
    
    Returns:
        SecurityConfig: Security configuration for tests
    """
    return SecurityConfig(
        auth_enabled=False,  # Disable auth for tests
        auth_method="none",
        api_key="test-api-key-12345",
        rate_limit_enabled=True,
        rate_limit_per_npc=100,
        rate_limit_global=1000,
        validate_requests=True,
        max_request_size=65535,
        allowed_request_types=[],
        blocked_npcs=[],
    )


@pytest.fixture(scope="session")
def ai_service_config(test_config: TestConfig) -> AIServiceConfig:
    """
    Create AIServiceConfig for tests.
    
    Args:
        test_config: Test environment configuration
        
    Returns:
        AIServiceConfig: AI service configuration for tests
    """
    return AIServiceConfig(
        base_url=test_config.ai_backend_url,
        timeout_seconds=10,
        max_retries=2,
        retry_backoff_factor=0.1,
    )


@pytest.fixture(scope="session")
def logging_config() -> LoggingConfig:
    """
    Create LoggingConfig for tests.
    
    Returns:
        LoggingConfig: Logging configuration for tests
    """
    return LoggingConfig(
        level="DEBUG",
        format="text",  # Use text format for tests
        include_timestamp=True,
        include_request_id=True,
    )


# =============================================================================
# Function-Scoped Database Fixtures
# =============================================================================

@pytest_asyncio.fixture
async def db_pool(database_config: DatabaseConfig) -> AsyncGenerator[aiomysql.Pool, None]:
    """
    Create a database connection pool for tests.
    
    Args:
        database_config: Database configuration
        
    Yields:
        aiomysql.Pool: Database connection pool
    """
    pool = await aiomysql.create_pool(
        host=database_config.host,
        port=database_config.port,
        user=database_config.user,
        password=database_config.password,
        db=database_config.database,
        minsize=1,
        maxsize=5,
        autocommit=True,
        connect_timeout=database_config.connect_timeout,
    )
    
    try:
        yield pool
    finally:
        pool.close()
        await pool.wait_closed()


@pytest_asyncio.fixture
async def db_connection(db_pool: aiomysql.Pool) -> AsyncGenerator[aiomysql.Connection, None]:
    """
    Get a single database connection from the pool.
    
    Args:
        db_pool: Database connection pool
        
    Yields:
        aiomysql.Connection: Database connection
    """
    async with db_pool.acquire() as conn:
        yield conn


@pytest_asyncio.fixture
async def db_manager(database_config: DatabaseConfig) -> AsyncGenerator[Database, None]:
    """
    Create and initialize a Database for tests.
    
    Args:
        database_config: Database configuration
        
    Yields:
        Database: Initialized database instance
    """
    db = Database(database_config)
    await db.connect()
    
    try:
        yield db
    finally:
        await db.disconnect()


@pytest_asyncio.fixture
async def test_database(db_connection: aiomysql.Connection) -> AsyncGenerator[None, None]:
    """
    Set up and tear down test database tables.
    
    Ensures test tables exist and cleans up test data after each test.
    
    Args:
        db_connection: Database connection
        
    Yields:
        None
    """
    async with db_connection.cursor() as cursor:
        # Ensure tables exist (they should already from the main schema)
        await cursor.execute("SELECT 1 FROM ai_requests LIMIT 1")
        await cursor.execute("SELECT 1 FROM ai_responses LIMIT 1")
        await cursor.execute("SELECT 1 FROM ai_request_log LIMIT 1")
    
    yield
    
    # Cleanup test data after each test
    async with db_connection.cursor() as cursor:
        # Delete test data created during tests (identified by test- prefix in request_type)
        await cursor.execute(
            "DELETE FROM ai_responses WHERE request_id IN "
            "(SELECT id FROM ai_requests WHERE request_type LIKE 'test_%')"
        )
        await cursor.execute(
            "DELETE FROM ai_request_log WHERE original_request_id IN "
            "(SELECT id FROM ai_requests WHERE request_type LIKE 'test_%')"
        )
        await cursor.execute("DELETE FROM ai_requests WHERE request_type LIKE 'test_%'")
        await db_connection.commit()


# =============================================================================
# Request Data Factories
# =============================================================================

@dataclass
class SampleRequest:
    """Factory for generating sample AI requests."""
    
    id: Optional[int] = None
    request_type: str = "test_dialogue"
    npc_id: int = field(default_factory=lambda: fake.random_int(min=1000, max=9999))
    player_id: int = field(default_factory=lambda: fake.random_int(min=1, max=999999))
    payload: dict = field(default_factory=dict)
    priority: int = 5
    status: str = "pending"
    created_at: Optional[datetime] = None
    expires_at: Optional[datetime] = None
    
    def __post_init__(self):
        if not self.payload:
            self.payload = {
                "message": fake.sentence(),
                "context": {
                    "location": fake.city(),
                    "time_of_day": fake.random_element(["morning", "afternoon", "evening", "night"]),
                },
            }
        if self.created_at is None:
            self.created_at = datetime.utcnow()
        if self.expires_at is None:
            self.expires_at = datetime.utcnow() + timedelta(minutes=5)
    
    def to_dict(self) -> dict:
        """Convert to dictionary for database insertion."""
        return {
            "request_type": self.request_type,
            "npc_id": self.npc_id,
            "player_id": self.player_id,
            "payload": json.dumps(self.payload),
            "priority": self.priority,
            "status": self.status,
            "created_at": self.created_at,
            "expires_at": self.expires_at,
        }


@dataclass
class SampleResponse:
    """Factory for generating sample AI responses."""
    
    request_id: int = 0
    response_data: dict = field(default_factory=dict)
    status_code: int = 200
    processing_time_ms: int = field(default_factory=lambda: fake.random_int(min=50, max=500))
    created_at: Optional[datetime] = None
    
    def __post_init__(self):
        if not self.response_data:
            self.response_data = {
                "dialogue": fake.paragraph(),
                "emotion": fake.random_element(["happy", "sad", "neutral", "curious"]),
                "action": fake.random_element(["wave", "nod", "bow", None]),
            }
        if self.created_at is None:
            self.created_at = datetime.utcnow()
    
    def to_dict(self) -> dict:
        """Convert to dictionary for database insertion."""
        return {
            "request_id": self.request_id,
            "response_data": json.dumps(self.response_data),
            "status_code": self.status_code,
            "processing_time_ms": self.processing_time_ms,
            "created_at": self.created_at,
        }


@pytest.fixture
def sample_request_factory() -> type[SampleRequest]:
    """
    Provide SampleRequest factory class for generating test requests.
    
    Returns:
        type[SampleRequest]: The SampleRequest factory class
    """
    return SampleRequest


@pytest.fixture
def sample_response_factory() -> type[SampleResponse]:
    """
    Provide SampleResponse factory class for generating test responses.
    
    Returns:
        type[SampleResponse]: The SampleResponse factory class
    """
    return SampleResponse


@pytest.fixture
def sample_dialogue_request() -> SampleRequest:
    """
    Create a sample dialogue request.
    
    Returns:
        SampleRequest: A dialogue request with test data
    """
    return SampleRequest(
        request_type="test_dialogue",
        payload={
            "message": "Hello, how are you?",
            "npc_name": "Guard Captain",
            "context": {
                "location": "Prontera Castle",
                "player_level": 50,
                "quest_stage": "introduction",
            },
        },
    )


@pytest.fixture
def sample_decision_request() -> SampleRequest:
    """
    Create a sample decision request.
    
    Returns:
        SampleRequest: A decision request with test data
    """
    return SampleRequest(
        request_type="test_decision",
        payload={
            "situation": "Player approaches with weapon drawn",
            "npc_personality": "cautious",
            "available_actions": ["attack", "flee", "negotiate", "call_guards"],
            "context": {
                "npc_health": 100,
                "player_reputation": -50,
            },
        },
    )


@pytest.fixture
def sample_emotion_request() -> SampleRequest:
    """
    Create a sample emotion request.
    
    Returns:
        SampleRequest: An emotion request with test data
    """
    return SampleRequest(
        request_type="test_emotion",
        payload={
            "current_emotion": "neutral",
            "stimulus": "Player gave gift",
            "gift_value": 1000,
            "relationship_level": 50,
        },
    )


@pytest.fixture
def sample_http_request() -> SampleRequest:
    """
    Create a sample HTTP proxy request.
    
    Returns:
        SampleRequest: An HTTP proxy request with test data
    """
    return SampleRequest(
        request_type="test_http_get",
        payload={
            "url": "http://example.com/api/data",
            "method": "GET",
            "headers": {"Accept": "application/json"},
        },
    )


# =============================================================================
# Mock AI Backend Fixtures
# =============================================================================

@pytest.fixture
def mock_ai_responses() -> dict[str, dict]:
    """
    Provide mock responses for AI backend endpoints.
    
    Returns:
        dict: Mock responses keyed by endpoint type
    """
    return {
        "dialogue": {
            "dialogue": "Greetings, brave adventurer! How may I assist you today?",
            "emotion": "friendly",
            "action": "wave",
            "metadata": {
                "confidence": 0.95,
                "model": "gpt-4",
                "tokens_used": 150,
            },
        },
        "decision": {
            "action": "negotiate",
            "confidence": 0.87,
            "reasoning": "Player appears non-hostile despite drawn weapon",
            "fallback_action": "call_guards",
        },
        "emotion": {
            "new_emotion": "happy",
            "intensity": 0.8,
            "duration_seconds": 300,
            "expression": "smile",
        },
        "health": {
            "status": "healthy",
            "services": {
                "database": "ok",
                "ai_backend": "ok",
                "cache": "ok",
            },
            "uptime_seconds": 86400,
        },
        "error": {
            "error": "Internal server error",
            "code": "INTERNAL_ERROR",
            "message": "The AI backend encountered an unexpected error",
        },
    }


@pytest.fixture
def mock_ai_backend(mock_ai_responses: dict, ai_service_config: AIServiceConfig):
    """
    Create a mock AI backend using aioresponses.
    
    This fixture mocks all AI backend HTTP endpoints to return
    predictable responses without making real network calls.
    
    Args:
        mock_ai_responses: Dictionary of mock responses
        ai_service_config: AI service configuration
        
    Yields:
        aioresponses: Configured mock context
    """
    with aioresponses() as mocked:
        # Mock dialogue endpoint
        mocked.post(
            ai_service_config.dialogue_url,
            payload=mock_ai_responses["dialogue"],
            repeat=True,
        )
        
        # Mock decision endpoint
        mocked.post(
            ai_service_config.decision_url,
            payload=mock_ai_responses["decision"],
            repeat=True,
        )
        
        # Mock emotion endpoint
        mocked.post(
            ai_service_config.emotion_url,
            payload=mock_ai_responses["emotion"],
            repeat=True,
        )
        
        # Mock memory endpoint
        mocked.post(
            ai_service_config.memory_url,
            payload={"stored": True, "memory_id": str(uuid.uuid4())},
            repeat=True,
        )
        mocked.get(
            ai_service_config.memory_url,
            payload={"memories": [], "count": 0},
            repeat=True,
        )
        
        yield mocked


@pytest.fixture
def mock_ai_backend_error(ai_service_config: AIServiceConfig):
    """
    Create a mock AI backend that returns errors.
    
    Args:
        ai_service_config: AI service configuration
        
    Yields:
        aioresponses: Configured mock context with error responses
    """
    with aioresponses() as mocked:
        error_response = {
            "error": "Service temporarily unavailable",
            "code": "SERVICE_UNAVAILABLE",
        }
        
        # Mock all endpoints to return errors
        mocked.post(ai_service_config.dialogue_url, status=503, payload=error_response, repeat=True)
        mocked.post(ai_service_config.decision_url, status=503, payload=error_response, repeat=True)
        mocked.post(ai_service_config.emotion_url, status=503, payload=error_response, repeat=True)
        mocked.post(ai_service_config.memory_url, status=503, payload=error_response, repeat=True)
        
        yield mocked


@pytest.fixture
def mock_ai_backend_timeout(ai_service_config: AIServiceConfig):
    """
    Create a mock AI backend that simulates timeouts.
    
    Args:
        ai_service_config: AI service configuration
        
    Yields:
        aioresponses: Configured mock context with timeout behavior
    """
    with aioresponses() as mocked:
        # Mock all endpoints to raise timeout
        mocked.post(ai_service_config.dialogue_url, exception=asyncio.TimeoutError(), repeat=True)
        mocked.post(ai_service_config.decision_url, exception=asyncio.TimeoutError(), repeat=True)
        mocked.post(ai_service_config.emotion_url, exception=asyncio.TimeoutError(), repeat=True)
        
        yield mocked


# =============================================================================
# Cleanup Fixtures
# =============================================================================

@pytest_asyncio.fixture
async def cleanup_test_data(db_connection: aiomysql.Connection):
    """
    Fixture that ensures test data is cleaned up after each test.
    
    This fixture tracks all request IDs created during a test and
    removes them from all related tables after the test completes.
    
    Args:
        db_connection: Database connection
        
    Yields:
        list: List to track created request IDs
    """
    created_ids: list[int] = []
    
    yield created_ids
    
    if created_ids:
        async with db_connection.cursor() as cursor:
            placeholders = ",".join(["%s"] * len(created_ids))
            
            # Delete responses first (foreign key constraint)
            await cursor.execute(
                f"DELETE FROM ai_responses WHERE request_id IN ({placeholders})",
                tuple(created_ids),
            )
            
            # Delete logs (using original_request_id column)
            await cursor.execute(
                f"DELETE FROM ai_request_log WHERE original_request_id IN ({placeholders})",
                tuple(created_ids),
            )
            
            # Delete requests
            await cursor.execute(
                f"DELETE FROM ai_requests WHERE id IN ({placeholders})",
                tuple(created_ids),
            )
            
            await db_connection.commit()
            logger.debug(f"Cleaned up {len(created_ids)} test requests")


# =============================================================================
# Helper Functions
# =============================================================================

async def insert_test_request(
    conn: aiomysql.Connection,
    request: SampleRequest,
) -> int:
    """
    Insert a test request into the database.
    
    Args:
        conn: Database connection
        request: Sample request to insert
        
    Returns:
        int: The ID of the inserted request
    """
    async with conn.cursor() as cursor:
        # Map request_type to endpoint
        endpoint_map = {
            'test_dialogue': '/dialogue',
            'test_decision': '/decision',
            'test_emotion': '/emotion',
            'dialogue': '/dialogue',
            'decision': '/decision',
            'emotion': '/emotion',
        }
        endpoint = endpoint_map.get(request.request_type, f'/{request.request_type}')
        
        await cursor.execute(
            """
            INSERT INTO ai_requests
            (request_type, endpoint, source_npc, request_data, priority, status, created_at, expires_at)
            VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
            """,
            (
                request.request_type,
                endpoint,
                str(request.npc_id),  # Use npc_id as source_npc
                json.dumps(request.payload),
                request.priority,
                request.status,
                request.created_at,
                request.expires_at,
            ),
        )
        await conn.commit()
        return cursor.lastrowid


async def insert_test_response(
    conn: aiomysql.Connection,
    response: SampleResponse,
) -> int:
    """
    Insert a test response into the database.
    
    Args:
        conn: Database connection
        response: Sample response to insert
        
    Returns:
        int: The ID of the inserted response
    """
    async with conn.cursor() as cursor:
        await cursor.execute(
            """
            INSERT INTO ai_responses
            (request_id, response_data, http_status, processing_time_ms, created_at)
            VALUES (%s, %s, %s, %s, %s)
            """,
            (
                response.request_id,
                json.dumps(response.response_data),
                response.status_code,  # maps to http_status column
                response.processing_time_ms,
                response.created_at,
            ),
        )
        await conn.commit()
        return cursor.lastrowid


async def get_request_by_id(
    conn: aiomysql.Connection,
    request_id: int,
) -> Optional[dict]:
    """
    Get a request from the database by ID.
    
    Args:
        conn: Database connection
        request_id: ID of the request to fetch
        
    Returns:
        Optional[dict]: Request data or None if not found
    """
    async with conn.cursor(aiomysql.DictCursor) as cursor:
        await cursor.execute(
            "SELECT * FROM ai_requests WHERE id = %s",
            (request_id,),
        )
        return await cursor.fetchone()


async def get_response_by_request_id(
    conn: aiomysql.Connection,
    request_id: int,
) -> Optional[dict]:
    """
    Get a response from the database by request ID.
    
    Args:
        conn: Database connection
        request_id: ID of the request to find response for
        
    Returns:
        Optional[dict]: Response data or None if not found
    """
    async with conn.cursor(aiomysql.DictCursor) as cursor:
        await cursor.execute(
            "SELECT * FROM ai_responses WHERE request_id = %s",
            (request_id,),
        )
        return await cursor.fetchone()


async def count_requests_by_status(
    conn: aiomysql.Connection,
    status: str,
) -> int:
    """
    Count requests with a specific status.
    
    Args:
        conn: Database connection
        status: Status to count
        
    Returns:
        int: Number of requests with the given status
    """
    async with conn.cursor() as cursor:
        await cursor.execute(
            "SELECT COUNT(*) FROM ai_requests WHERE status = %s AND request_type LIKE 'test_%'",
            (status,),
        )
        result = await cursor.fetchone()
        return result[0] if result else 0


# =============================================================================
# Pytest Hooks and Configuration
# =============================================================================

def pytest_configure(config):
    """Configure pytest with custom markers."""
    config.addinivalue_line("markers", "unit: Unit tests")
    config.addinivalue_line("markers", "integration: Integration tests")
    config.addinivalue_line("markers", "e2e: End-to-end tests")
    config.addinivalue_line("markers", "slow: Slow tests")
    config.addinivalue_line("markers", "database: Tests requiring database")


def pytest_collection_modifyitems(config, items):
    """Automatically mark tests based on their location."""
    for item in items:
        # Mark tests based on their module path
        if "/unit/" in str(item.fspath):
            item.add_marker(pytest.mark.unit)
        elif "/integration/" in str(item.fspath):
            item.add_marker(pytest.mark.integration)
            item.add_marker(pytest.mark.database)
        elif "/e2e/" in str(item.fspath):
            item.add_marker(pytest.mark.e2e)
            item.add_marker(pytest.mark.database)


@pytest.fixture(autouse=True)
def reset_faker_seed():
    """Reset faker seed before each test for reproducibility."""
    Faker.seed(12345)


@pytest.fixture
def anyio_backend():
    """Configure asyncio as the async backend."""
    return "asyncio"