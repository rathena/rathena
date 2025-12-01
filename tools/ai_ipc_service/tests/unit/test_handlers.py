"""
Unit Tests for Request Handlers

Tests all handler classes including:
- HttpProxyHandler: HTTP GET/POST forwarding
- HealthCheckHandler: Service health checks
- AIDialogueHandler: NPC dialogue generation
- AIDecisionHandler: NPC action decisions
- AIEmotionHandler: NPC emotion queries
- AIAsyncRequestHandler: Async operation handlers
- BaseHandler: Base handler functionality
- RateLimiter: Rate limiting functionality

Test Strategy:
- Mock external HTTP calls using aioresponses
- Test validation, error handling, and success cases
- Verify request/response transformations
"""

import asyncio
import json
import logging
import os
import sys
import time
from datetime import datetime, timedelta
from typing import Any
from unittest.mock import AsyncMock, MagicMock, patch

import aiohttp
import pytest
import pytest_asyncio
from aioresponses import aioresponses

# Add parent directories to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from config import Config, AIServiceConfig, SecurityConfig, DatabaseConfig, PollingConfig, LoggingConfig
from handlers import HANDLER_REGISTRY, get_handler
from handlers.base import BaseHandler, RateLimiter, HandlerError, ValidationError, TimeoutError
from handlers.http_proxy import HttpProxyHandler, HttpGetHandler, HttpPostHandler
from handlers.health_check import HealthCheckHandler
from handlers.ai_dialogue import AIDialogueHandler
from handlers.ai_decision import AIDecisionHandler
from handlers.ai_emotion import AIEmotionHandler
from handlers.ai_async import AIAsyncRequestHandler, AICheckResultHandler

logger = logging.getLogger(__name__)


# =============================================================================
# Test Fixtures
# =============================================================================

@pytest.fixture
def mock_config() -> Config:
    """Create a mock Config object for testing."""
    return Config(
        database=DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        ),
        ai_service=AIServiceConfig(
            base_url="http://localhost:8000",
            timeout_seconds=10,
            max_retries=2,
        ),
        security=SecurityConfig(
            api_key="test-api-key-12345",
            blocked_npcs=[],
            rate_limit_per_npc=60,
            rate_limit_global=1000,
            rate_limit_window_seconds=60,
        ),
        polling=PollingConfig(
            interval_ms=100,
            batch_size=10,
            worker_count=2,
        ),
        logging=LoggingConfig(
            level="DEBUG",
            format="json",
        ),
    )


@pytest.fixture
def mock_request() -> dict[str, Any]:
    """Create a mock request dictionary."""
    return {
        "id": 1,
        "request_type": "test",
        "endpoint": "/api/test",
        "request_data": json.dumps({"key": "value"}),
        "status": "pending",
        "priority": 5,
        "correlation_id": "test-correlation-123",
        "source_npc": "Test NPC",
        "source_map": "prontera",
        "created_at": datetime.utcnow(),
        "expires_at": None,
        "retry_count": 0,
    }


# =============================================================================
# RateLimiter Tests
# =============================================================================

class TestRateLimiter:
    """Tests for RateLimiter class."""
    
    def test_rate_limiter_initialization(self):
        """Test RateLimiter initialization."""
        limiter = RateLimiter(max_requests=10, window_seconds=60)
        
        assert limiter.max_requests == 10
        assert limiter.window_seconds == 60
    
    def test_rate_limiter_allows_under_limit(self):
        """Test that requests under limit are allowed."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        
        for i in range(5):
            assert limiter.is_allowed("test_key") is True
    
    def test_rate_limiter_blocks_over_limit(self):
        """Test that requests over limit are blocked."""
        limiter = RateLimiter(max_requests=3, window_seconds=60)
        
        for _ in range(3):
            limiter.is_allowed("test_key")
        
        assert limiter.is_allowed("test_key") is False
    
    def test_rate_limiter_separate_keys(self):
        """Test that different keys have separate limits."""
        limiter = RateLimiter(max_requests=2, window_seconds=60)
        
        # Key 1 can make 2 requests
        assert limiter.is_allowed("key1") is True
        assert limiter.is_allowed("key1") is True
        assert limiter.is_allowed("key1") is False
        
        # Key 2 still has its own limit
        assert limiter.is_allowed("key2") is True
        assert limiter.is_allowed("key2") is True
        assert limiter.is_allowed("key2") is False
    
    def test_rate_limiter_window_expiry(self):
        """Test that old requests expire from window."""
        limiter = RateLimiter(max_requests=2, window_seconds=1)
        
        limiter.is_allowed("test_key")
        limiter.is_allowed("test_key")
        assert limiter.is_allowed("test_key") is False
        
        # Wait for window to expire
        time.sleep(1.1)
        
        # Should be allowed again
        assert limiter.is_allowed("test_key") is True
    
    def test_rate_limiter_get_remaining(self):
        """Test get_remaining returns correct count."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        
        assert limiter.get_remaining("test_key") == 5
        
        limiter.is_allowed("test_key")
        assert limiter.get_remaining("test_key") == 4
        
        limiter.is_allowed("test_key")
        limiter.is_allowed("test_key")
        assert limiter.get_remaining("test_key") == 2
    
    def test_rate_limiter_reset_key(self):
        """Test resetting a specific key."""
        limiter = RateLimiter(max_requests=2, window_seconds=60)
        
        limiter.is_allowed("key1")
        limiter.is_allowed("key1")
        limiter.is_allowed("key2")
        
        # Both keys have requests
        assert limiter.get_remaining("key1") == 0
        assert limiter.get_remaining("key2") == 1
        
        # Reset key1 only
        limiter.reset("key1")
        
        assert limiter.get_remaining("key1") == 2
        assert limiter.get_remaining("key2") == 1
    
    def test_rate_limiter_reset_all(self):
        """Test resetting all keys."""
        limiter = RateLimiter(max_requests=2, window_seconds=60)
        
        limiter.is_allowed("key1")
        limiter.is_allowed("key2")
        
        # Reset all
        limiter.reset()
        
        assert limiter.get_remaining("key1") == 2
        assert limiter.get_remaining("key2") == 2


# =============================================================================
# BaseHandler Helper Method Tests (via concrete implementation)
# =============================================================================

class TestBaseHandlerHelpers:
    """Tests for BaseHandler helper methods via HealthCheckHandler."""
    
    def test_get_request_data_valid_json(self, mock_config: Config):
        """Test parsing valid JSON request data."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"request_data": '{"key": "value", "num": 123}'}
        data = handler.get_request_data(request)
        
        assert data == {"key": "value", "num": 123}
    
    def test_get_request_data_empty(self, mock_config: Config):
        """Test parsing empty request data."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"request_data": ""}
        data = handler.get_request_data(request)
        
        assert data == {}
    
    def test_get_request_data_invalid_json(self, mock_config: Config):
        """Test parsing invalid JSON returns empty dict."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"request_data": "not valid json{{{"}
        data = handler.get_request_data(request)
        
        assert data == {}
    
    def test_get_request_data_missing_field(self, mock_config: Config):
        """Test missing request_data field."""
        handler = HealthCheckHandler(mock_config)
        
        request = {}
        data = handler.get_request_data(request)
        
        assert data == {}
    
    def test_get_endpoint(self, mock_config: Config):
        """Test endpoint extraction."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"endpoint": "/api/test"}
        endpoint = handler.get_endpoint(request)
        
        assert endpoint == "/api/test"
    
    def test_get_endpoint_missing(self, mock_config: Config):
        """Test missing endpoint returns empty string."""
        handler = HealthCheckHandler(mock_config)
        
        request = {}
        endpoint = handler.get_endpoint(request)
        
        assert endpoint == ""
    
    def test_get_request_id(self, mock_config: Config):
        """Test request ID extraction."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"id": 12345}
        request_id = handler.get_request_id(request)
        
        assert request_id == 12345
    
    def test_get_source_info(self, mock_config: Config):
        """Test source info extraction."""
        handler = HealthCheckHandler(mock_config)
        
        request = {
            "source_npc": "Guard",
            "source_map": "prontera",
            "correlation_id": "abc123",
        }
        source_info = handler.get_source_info(request)
        
        assert source_info["source_npc"] == "Guard"
        assert source_info["source_map"] == "prontera"
        assert source_info["correlation_id"] == "abc123"
    
    def test_create_success_response(self, mock_config: Config):
        """Test success response creation."""
        handler = HealthCheckHandler(mock_config)
        
        response = handler.create_success_response(data={"result": "ok"}, extra_field="value")
        
        assert response["status"] == "ok"
        assert response["data"] == {"result": "ok"}
        assert response["extra_field"] == "value"
        assert "timestamp" in response
    
    def test_create_error_response(self, mock_config: Config):
        """Test error response creation."""
        handler = HealthCheckHandler(mock_config)
        
        response = handler.create_error_response(error="Something failed", code=500)
        
        assert response["status"] == "error"
        assert response["error"] == "Something failed"
        assert response["code"] == 500
        assert "timestamp" in response
    
    def test_validate_required_fields_success(self, mock_config: Config):
        """Test validation passes with all required fields."""
        handler = HealthCheckHandler(mock_config)
        
        data = {"field1": "value1", "field2": "value2"}
        # Should not raise
        handler.validate_required_fields(data, ["field1", "field2"])
    
    def test_validate_required_fields_missing(self, mock_config: Config):
        """Test validation raises on missing fields."""
        handler = HealthCheckHandler(mock_config)
        
        data = {"field1": "value1"}
        
        with pytest.raises(ValidationError) as exc_info:
            handler.validate_required_fields(data, ["field1", "field2"])
        
        assert "field2" in str(exc_info.value)


# =============================================================================
# HttpProxyHandler Tests
# =============================================================================

class TestHttpProxyHandler:
    """Tests for HTTP proxy handler."""
    
    @pytest.mark.asyncio
    async def test_http_proxy_get_success(self, mock_config: Config):
        """Test successful HTTP GET proxy request."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",
            "endpoint": "/api/data",
            "request_data": "{}",
        }
        
        expected_response = {"data": "test_value", "status": "ok"}
        
        with aioresponses() as mocked:
            mocked.get(
                "http://localhost:8000/api/data",
                payload=expected_response,
                status=200,
                content_type="application/json",
            )
            
            result = await handler.handle(request)
            
            assert result == expected_response
    
    @pytest.mark.asyncio
    async def test_http_proxy_post_success(self, mock_config: Config):
        """Test successful HTTP POST proxy request."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_post",
            "endpoint": "/api/submit",
            "request_data": json.dumps({"body": {"name": "test", "value": 123}}),
        }
        
        expected_response = {"id": 1, "created": True}
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/submit",
                payload=expected_response,
                status=200,
                content_type="application/json",
            )
            
            result = await handler.handle(request)
            
            assert result == expected_response
    
    @pytest.mark.asyncio
    async def test_http_proxy_timeout(self, mock_config: Config):
        """Test HTTP proxy timeout handling."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",
            "endpoint": "/api/slow",
            "request_data": "{}",
        }
        
        with aioresponses() as mocked:
            mocked.get(
                "http://localhost:8000/api/slow",
                exception=asyncio.TimeoutError(),
            )
            
            with pytest.raises(TimeoutError):
                await handler.handle(request)
    
    @pytest.mark.asyncio
    async def test_http_proxy_connection_error(self, mock_config: Config):
        """Test HTTP proxy connection error handling."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",
            "endpoint": "/api/test",
            "request_data": "{}",
        }
        
        with aioresponses() as mocked:
            mocked.get(
                "http://localhost:8000/api/test",
                exception=aiohttp.ClientError("Connection failed"),
            )
            
            with pytest.raises(HandlerError):
                await handler.handle(request)
    
    @pytest.mark.asyncio
    async def test_http_proxy_missing_endpoint(self, mock_config: Config):
        """Test HTTP proxy with missing endpoint."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",
            "endpoint": "",
            "request_data": "{}",
        }
        
        with pytest.raises(ValidationError):
            await handler.handle(request)
    
    @pytest.mark.asyncio
    async def test_http_proxy_full_url_passthrough(self, mock_config: Config):
        """Test HTTP proxy with full URL in endpoint."""
        handler = HttpProxyHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",
            "endpoint": "http://other-server.com/api/data",
            "request_data": "{}",
        }
        
        expected_response = {"source": "other-server"}
        
        with aioresponses() as mocked:
            mocked.get(
                "http://other-server.com/api/data",
                payload=expected_response,
                status=200,
                content_type="application/json",
            )
            
            result = await handler.handle(request)
            
            assert result == expected_response
    
    @pytest.mark.asyncio
    async def test_http_get_handler_forces_get(self, mock_config: Config):
        """Test HttpGetHandler always uses GET method."""
        handler = HttpGetHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_post",  # Even if post is specified
            "endpoint": "/api/data",
            "request_data": "{}",
        }
        
        expected_response = {"method": "GET"}
        
        with aioresponses() as mocked:
            mocked.get(
                "http://localhost:8000/api/data",
                payload=expected_response,
                status=200,
                content_type="application/json",
            )
            
            result = await handler.handle(request)
            
            assert result == expected_response
    
    @pytest.mark.asyncio
    async def test_http_post_handler_forces_post(self, mock_config: Config):
        """Test HttpPostHandler always uses POST method."""
        handler = HttpPostHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "http_get",  # Even if get is specified
            "endpoint": "/api/data",
            "request_data": "{}",
        }
        
        expected_response = {"method": "POST"}
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/data",
                payload=expected_response,
                status=200,
                content_type="application/json",
            )
            
            result = await handler.handle(request)
            
            assert result == expected_response


# =============================================================================
# HealthCheckHandler Tests
# =============================================================================

class TestHealthCheckHandler:
    """Tests for health check handler."""
    
    @pytest.mark.asyncio
    async def test_health_check_basic(self, mock_config: Config):
        """Test basic health check response."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"id": 1, "request_type": "health_check"}
        
        result = await handler.handle(request)
        
        assert result["status"] == "ok"
    
    @pytest.mark.asyncio
    async def test_health_check_includes_timestamp(self, mock_config: Config):
        """Test health check includes timestamp."""
        handler = HealthCheckHandler(mock_config)
        
        request = {"id": 1, "request_type": "health_check"}
        
        result = await handler.handle(request)
        
        assert "timestamp" in result


# =============================================================================
# AIDialogueHandler Tests
# =============================================================================

class TestAIDialogueHandler:
    """Tests for AI dialogue handler."""
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_valid_request(self, mock_config: Config):
        """Test successful dialogue request."""
        handler = AIDialogueHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "request_data": json.dumps({
                "npc_id": "merchant_001",
                "npc_name": "Alice",
                "player_message": "Hello!",
            }),
        }
        
        ai_response = {
            "response": "Welcome to my shop!",
            "emotion": "friendly",
        }
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/v1/npc/merchant_001/dialogue",
                payload=ai_response,
                status=200,
            )
            
            result = await handler.handle(request)
            
            assert result["status"] == "ok"
            assert "response" in result
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_fallback_on_error(self, mock_config: Config):
        """Test dialogue falls back on AI service error."""
        handler = AIDialogueHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "request_data": json.dumps({
                "npc_id": "merchant_001",
                "npc_name": "Alice the Merchant",
                "player_message": "Hello!",
            }),
        }
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/v1/npc/merchant_001/dialogue",
                status=500,
                payload={"error": "Service unavailable"},
            )
            
            result = await handler.handle(request)
            
            # Should get fallback response
            assert result["status"] == "ok"
            assert "response" in result
            assert result["metadata"]["source"] == "fallback"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_missing_npc_id_and_name(self, mock_config: Config):
        """Test dialogue request with missing NPC identification."""
        handler = AIDialogueHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "request_data": json.dumps({
                "player_message": "Hello!",
                # Missing both npc_id and npc_name
            }),
        }
        
        with pytest.raises(ValidationError):
            await handler.handle(request)
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_npc_type_detection_merchant(self, mock_config: Config):
        """Test NPC type detection for merchant."""
        handler = AIDialogueHandler(mock_config)
        
        npc_type = handler._determine_npc_type("", "Alice the Merchant")
        assert npc_type == "merchant"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_npc_type_detection_guard(self, mock_config: Config):
        """Test NPC type detection for guard."""
        handler = AIDialogueHandler(mock_config)
        
        npc_type = handler._determine_npc_type("guard_001", "City Guard")
        assert npc_type == "guard"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_npc_type_detection_default(self, mock_config: Config):
        """Test NPC type detection defaults to 'default'."""
        handler = AIDialogueHandler(mock_config)
        
        npc_type = handler._determine_npc_type("npc_001", "Bob")
        assert npc_type == "default"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_greeting(self, mock_config: Config):
        """Test emotion detection for greetings."""
        handler = AIDialogueHandler(mock_config)
        
        emotion = handler._determine_emotion("Hello there!")
        assert emotion == "friendly"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_help(self, mock_config: Config):
        """Test emotion detection for help requests."""
        handler = AIDialogueHandler(mock_config)
        
        emotion = handler._determine_emotion("I need help please")
        assert emotion == "helpful"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_no_match(self, mock_config: Config):
        """Test emotion detection returns neutral when no keywords match."""
        handler = AIDialogueHandler(mock_config)
        
        # Message with no matching keywords
        emotion = handler._determine_emotion("xyz random words abc")
        assert emotion == "neutral"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_interested(self, mock_config: Config):
        """Test emotion detection for buy/sell/trade keywords returns interested."""
        handler = AIDialogueHandler(mock_config)
        
        # Test with "buy" keyword (avoid words containing "hi" like "something")
        emotion = handler._determine_emotion("I want to buy goods")
        assert emotion == "interested"
        
        # Test with "sell" keyword
        emotion = handler._determine_emotion("Can I sell stuff?")
        assert emotion == "interested"
        
        # Test with "trade" keyword
        emotion = handler._determine_emotion("trade now")
        assert emotion == "interested"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_excited(self, mock_config: Config):
        """Test emotion detection for quest/adventure/mission keywords returns excited."""
        handler = AIDialogueHandler(mock_config)
        
        emotion = handler._determine_emotion("I'm looking for a quest")
        assert emotion == "excited"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_emotion_detection_goodbye(self, mock_config: Config):
        """Test emotion detection for goodbye keywords returns neutral."""
        handler = AIDialogueHandler(mock_config)
        
        emotion = handler._determine_emotion("Goodbye, see you later")
        assert emotion == "neutral"
    
    @pytest.mark.asyncio
    async def test_ai_dialogue_menu_options_merchant(self, mock_config: Config):
        """Test menu options for merchant NPC."""
        handler = AIDialogueHandler(mock_config)
        
        options = handler._generate_menu_options("merchant")
        
        assert "Buy Items" in options
        assert "Sell Items" in options
        assert "Leave" in options


# =============================================================================
# AIDecisionHandler Tests
# =============================================================================

class TestAIDecisionHandler:
    """Tests for AI decision handler."""
    
    @pytest.mark.asyncio
    async def test_ai_decision_valid_request(self, mock_config: Config):
        """Test successful decision request."""
        handler = AIDecisionHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "decision",
            "request_data": json.dumps({
                "npc_id": "guard_001",
                "situation": "Player approaches with weapon drawn",
                "available_actions": ["attack", "flee", "negotiate"],
            }),
        }
        
        ai_response = {
            "action": "negotiate",
            "confidence": 0.85,
            "reasoning": "Attempting peaceful resolution first",
        }
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/v1/npc/guard_001/decision",
                payload=ai_response,
                status=200,
            )
            
            result = await handler.handle(request)
            
            assert result["status"] == "ok"
            assert "action" in result


# =============================================================================
# AIEmotionHandler Tests
# =============================================================================

class TestAIEmotionHandler:
    """Tests for AI emotion handler."""
    
    @pytest.mark.asyncio
    async def test_ai_emotion_valid_request(self, mock_config: Config):
        """Test successful emotion query."""
        handler = AIEmotionHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "emotion",
            "request_data": json.dumps({
                "npc_id": "merchant_001",
                "current_emotion": "neutral",
                "stimulus": "Player gave gift",
                "stimulus_value": 1000,
            }),
        }
        
        ai_response = {
            "new_emotion": "happy",
            "intensity": 0.8,
            "duration_seconds": 300,
        }
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/v1/npc/merchant_001/emotion",
                payload=ai_response,
                status=200,
            )
            
            result = await handler.handle(request)
            
            assert result["status"] == "ok"
            assert "emotion" in result or "new_emotion" in result


# =============================================================================
# AIAsyncRequestHandler Tests
# =============================================================================

class TestAIAsyncRequestHandler:
    """Tests for async request handler."""
    
    @pytest.mark.asyncio
    async def test_ai_async_request_submission(self, mock_config: Config):
        """Test async request submission."""
        handler = AIAsyncRequestHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "ai_request_async",
            "request_data": json.dumps({
                "npc_id": "sage_001",
                "operation": "generate_story",
                "payload": {"prompt": "Tell a tale"},
            }),
        }
        
        result = await handler.handle(request)
        
        assert result["status"] == "ok"
        # Actual implementation returns async_request_id
        assert "async_request_id" in result


class TestAICheckResultHandler:
    """Tests for async result check handler."""
    
    @pytest.mark.asyncio
    async def test_ai_check_result(self, mock_config: Config):
        """Test checking async result."""
        handler = AICheckResultHandler(mock_config)
        
        request = {
            "id": 1,
            "request_type": "ai_check_result",
            "request_data": json.dumps({
                "async_request_id": "async-123",  # Actual field name
            }),
        }
        
        result = await handler.handle(request)
        
        # Should return status (pending/complete/not_found)
        assert "status" in result


# =============================================================================
# Handler Registry Tests
# =============================================================================

class TestHandlerRegistry:
    """Tests for handler registry functionality."""
    
    def test_registry_contains_health_check(self):
        """Test health check handler is registered."""
        assert "health_check" in HANDLER_REGISTRY
        assert "health" in HANDLER_REGISTRY
    
    def test_registry_contains_http_handlers(self):
        """Test HTTP proxy handlers are registered."""
        assert "http_get" in HANDLER_REGISTRY
        assert "http_post" in HANDLER_REGISTRY
        assert "httpget" in HANDLER_REGISTRY
        assert "httppost" in HANDLER_REGISTRY
    
    def test_registry_contains_ai_handlers(self):
        """Test AI handlers are registered."""
        assert "dialogue" in HANDLER_REGISTRY
        assert "ai_dialogue" in HANDLER_REGISTRY
        assert "decision" in HANDLER_REGISTRY
        assert "ai_decision" in HANDLER_REGISTRY
        assert "emotion" in HANDLER_REGISTRY
        assert "ai_emotion" in HANDLER_REGISTRY
    
    def test_registry_contains_async_handlers(self):
        """Test async handlers are registered."""
        assert "ai_request_async" in HANDLER_REGISTRY
        assert "async_request" in HANDLER_REGISTRY
        assert "ai_check_result" in HANDLER_REGISTRY
        assert "check_result" in HANDLER_REGISTRY
    
    def test_get_handler_returns_instance(self, mock_config: Config):
        """Test get_handler returns handler instance."""
        handler = get_handler("health_check", mock_config)
        
        assert handler is not None
        assert isinstance(handler, HealthCheckHandler)
    
    def test_get_handler_returns_none_for_unknown(self, mock_config: Config):
        """Test get_handler returns None for unknown type."""
        handler = get_handler("unknown_type", mock_config)
        
        assert handler is None
    
    def test_registry_handlers_are_callable(self):
        """Test all registered handlers are callable classes."""
        for handler_type, handler_class in HANDLER_REGISTRY.items():
            assert handler_class is not None
            assert callable(handler_class)
    
    def test_http_aliases_point_to_same_handler(self):
        """Test HTTP aliases point to HttpProxyHandler."""
        http_types = ["http_get", "http_post", "httpget", "httppost", "get", "post"]
        
        for handler_type in http_types:
            assert HANDLER_REGISTRY[handler_type] == HttpProxyHandler


# =============================================================================
# Handler Error Cases Tests
# =============================================================================

class TestHandlerErrors:
    """Tests for handler error scenarios."""
    
    def test_handler_error_initialization(self):
        """Test HandlerError initialization."""
        error = HandlerError("Test error", status_code=500)
        
        assert error.message == "Test error"
        assert error.status_code == 500
        assert str(error) == "Test error"
    
    def test_validation_error_initialization(self):
        """Test ValidationError initialization."""
        error = ValidationError("Invalid input")
        
        assert error.message == "Invalid input"
        assert error.status_code == 400
    
    def test_timeout_error_initialization(self):
        """Test TimeoutError initialization."""
        error = TimeoutError("Request timed out")
        
        assert error.message == "Request timed out"
        assert error.status_code == 504


# =============================================================================
# Handler Integration Tests
# =============================================================================

class TestHandlerIntegration:
    """Integration tests for handler workflows."""
    
    @pytest.mark.asyncio
    async def test_dialogue_to_decision_flow(self, mock_config: Config):
        """Test dialogue followed by decision request."""
        dialogue_handler = AIDialogueHandler(mock_config)
        decision_handler = AIDecisionHandler(mock_config)
        
        # First, NPC has dialogue
        dialogue_request = {
            "id": 1,
            "request_type": "dialogue",
            "request_data": json.dumps({
                "npc_id": "guard_001",
                "npc_name": "City Guard",
                "player_message": "I need to pass!",
            }),
        }
        
        # Then NPC decides action
        decision_request = {
            "id": 2,
            "request_type": "decision",
            "request_data": json.dumps({
                "npc_id": "guard_001",
                "situation": "Player demands passage",
                "available_actions": ["allow", "deny", "quest"],
            }),
        }
        
        with aioresponses() as mocked:
            # Mock dialogue
            mocked.post(
                "http://localhost:8000/api/v1/npc/guard_001/dialogue",
                status=500,  # Force fallback
            )
            # Mock decision
            mocked.post(
                "http://localhost:8000/api/v1/npc/guard_001/decision",
                payload={"action": "quest", "confidence": 0.9},
                status=200,
            )
            
            dialogue_result = await dialogue_handler.handle(dialogue_request)
            decision_result = await decision_handler.handle(decision_request)
            
            assert dialogue_result["status"] == "ok"
            assert decision_result["status"] == "ok"
    
    @pytest.mark.asyncio
    async def test_multiple_npcs_handled_separately(self, mock_config: Config):
        """Test that different NPCs get separate handling."""
        handler = AIDialogueHandler(mock_config)
        
        requests = [
            {
                "id": 1,
                "request_type": "dialogue",
                "request_data": json.dumps({
                    "npc_id": "merchant_001",
                    "npc_name": "Shop Keeper",
                    "player_message": "Hello!",
                }),
            },
            {
                "id": 2,
                "request_type": "dialogue",
                "request_data": json.dumps({
                    "npc_id": "guard_001",
                    "npc_name": "Guard",
                    "player_message": "Hello!",
                }),
            },
        ]
        
        with aioresponses() as mocked:
            mocked.post(
                "http://localhost:8000/api/v1/npc/merchant_001/dialogue",
                status=500,
            )
            mocked.post(
                "http://localhost:8000/api/v1/npc/guard_001/dialogue",
                status=500,
            )
            
            results = []
            for request in requests:
                result = await handler.handle(request)
                results.append(result)
            
            # Both should succeed (with fallback)
            assert all(r["status"] == "ok" for r in results)
            # Different NPC IDs
            assert results[0]["npc_id"] == "merchant_001"
            assert results[1]["npc_id"] == "guard_001"