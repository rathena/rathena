"""
Test coverage for remaining uncovered code paths.

This module tests:
- handlers/base_handler.py (legacy code)
- handlers/ai_async.py (async request handling)
- handlers/health_check.py (readiness/liveness)
- database.py (compat methods)
"""

import json
import asyncio
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch, PropertyMock
import pytest

# =============================================================================
# Tests for handlers/base_handler.py (Legacy Handler)
# =============================================================================

class TestHandlerRequest:
    """Tests for HandlerRequest dataclass."""
    
    def test_from_db_record_basic(self):
        """Test creating HandlerRequest from basic db record."""
        from handlers.base_handler import HandlerRequest
        
        record = {
            "id": 1,
            "request_type": "dialogue",
            "endpoint": "/api/dialogue",
            "request_data": {"npc_name": "Guard", "map_name": "prontera"},
            "created_at": "2024-01-01 00:00:00"
        }
        
        req = HandlerRequest.from_db_record(record)
        assert req.request_id == 1
        assert req.request_type == "dialogue"
        assert req.endpoint == "/api/dialogue"
        assert req.npc_name == "Guard"
        assert req.map_name == "prontera"
    
    def test_from_db_record_json_string(self):
        """Test creating HandlerRequest when request_data is JSON string."""
        from handlers.base_handler import HandlerRequest
        
        record = {
            "id": 2,
            "request_type": "decision",
            "endpoint": "/api/decision",
            "request_data": '{"npc_name": "Merchant", "api_key": "test123"}',
            "created_at": None
        }
        
        req = HandlerRequest.from_db_record(record)
        assert req.request_id == 2
        assert req.npc_name == "Merchant"
        assert req.api_key == "test123"
        assert req.created_at is None
    
    def test_from_db_record_invalid_json(self):
        """Test creating HandlerRequest with invalid JSON string."""
        from handlers.base_handler import HandlerRequest
        
        record = {
            "id": 3,
            "request_type": "test",
            "endpoint": "/test",
            "request_data": "not valid json {",
            "created_at": datetime.now()
        }
        
        req = HandlerRequest.from_db_record(record)
        assert req.request_id == 3
        assert req.request_data == {"raw": "not valid json {"}
    
    def test_from_db_record_missing_fields(self):
        """Test creating HandlerRequest with missing fields."""
        from handlers.base_handler import HandlerRequest
        
        record = {}
        req = HandlerRequest.from_db_record(record)
        assert req.request_id == 0
        assert req.request_type == ""
        assert req.endpoint == ""
        assert req.npc_name is None


class TestHandlerResponse:
    """Tests for HandlerResponse dataclass."""
    
    def test_to_json_success(self):
        """Test converting success response to JSON."""
        from handlers.base_handler import HandlerResponse
        
        resp = HandlerResponse(
            success=True,
            data={"message": "Hello"},
            error=None,
            status_code=200
        )
        
        json_str = resp.to_json()
        parsed = json.loads(json_str)
        assert parsed["success"] is True
        assert parsed["data"]["message"] == "Hello"
        assert parsed["error"] is None
    
    def test_to_json_error(self):
        """Test converting error response to JSON."""
        from handlers.base_handler import HandlerResponse
        
        resp = HandlerResponse(
            success=False,
            data={},
            error="Something went wrong",
            status_code=500
        )
        
        json_str = resp.to_json()
        parsed = json.loads(json_str)
        assert parsed["success"] is False
        assert parsed["error"] == "Something went wrong"


class TestRateLimiter:
    """Tests for RateLimiter class."""
    
    def test_init_defaults(self):
        """Test rate limiter initialization with defaults."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter()
        assert limiter.per_npc_limit == 60
        assert limiter.global_limit == 1000
        assert limiter.window_seconds == 60
    
    def test_init_custom(self):
        """Test rate limiter with custom values."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=10, global_limit=100, window_seconds=30)
        assert limiter.per_npc_limit == 10
        assert limiter.global_limit == 100
        assert limiter.window_seconds == 30
    
    def test_check_rate_limit_allowed(self):
        """Test rate limit check when allowed."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=5, global_limit=10)
        allowed, error = limiter.check_rate_limit("test_npc")
        assert allowed is True
        assert error is None
    
    def test_check_rate_limit_global_exceeded(self):
        """Test rate limit check when global limit exceeded."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=100, global_limit=3, window_seconds=60)
        
        # Record enough requests to exceed global limit
        for _ in range(3):
            limiter.record_request()
        
        allowed, error = limiter.check_rate_limit()
        assert allowed is False
        assert "Global rate limit exceeded" in error
    
    def test_check_rate_limit_npc_exceeded(self):
        """Test rate limit check when NPC limit exceeded."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=2, global_limit=100, window_seconds=60)
        
        # Record requests for specific NPC
        for _ in range(2):
            limiter.record_request("test_npc")
        
        allowed, error = limiter.check_rate_limit("test_npc")
        assert allowed is False
        assert "NPC rate limit exceeded" in error
    
    def test_record_request(self):
        """Test recording requests."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter()
        limiter.record_request("npc1")
        limiter.record_request("npc1")
        limiter.record_request("npc2")
        
        stats = limiter.get_stats()
        assert stats["global_requests"] == 3
        assert stats["npc_counts"]["npc1"] == 2
        assert stats["npc_counts"]["npc2"] == 1
    
    def test_get_stats(self):
        """Test getting rate limiter statistics."""
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=10, global_limit=50)
        limiter.record_request("guard")
        
        stats = limiter.get_stats()
        assert "global_requests" in stats
        assert "global_limit" in stats
        assert "npc_counts" in stats
        assert stats["global_limit"] == 50


class TestBaseHandler:
    """Tests for BaseHandler abstract class."""
    
    def test_init_with_config(self):
        """Test handler initialization with config."""
        from handlers.base_handler import BaseHandler
        
        # Create concrete subclass for testing
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {"status": "ok"}
        
        mock_config = MagicMock()
        mock_config.security = MagicMock()
        mock_config.security.rate_limit_per_npc = 30
        mock_config.security.rate_limit_global = 500
        
        # Reset singleton
        BaseHandler._rate_limiter = None
        
        handler = TestHandler(mock_config)
        assert handler.config == mock_config
        assert BaseHandler._rate_limiter is not None
    
    def test_init_without_config(self):
        """Test handler initialization without config."""
        from handlers.base_handler import BaseHandler
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        # Reset singleton
        BaseHandler._rate_limiter = None
        
        handler = TestHandler(None)
        assert handler.config is None
        assert BaseHandler._rate_limiter is not None
    
    @pytest.mark.asyncio
    async def test_process_success(self):
        """Test successful request processing."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={"result": "done"})
        
        # Reset and disable security
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = None
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        response = await handler.process(request)
        assert response.success is True
    
    @pytest.mark.asyncio
    async def test_process_auth_failure(self):
        """Test request processing with authentication failure."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        # Setup security config that requires auth
        mock_security = MagicMock()
        mock_security.auth_enabled = True
        mock_security.auth_method = "api_key"
        mock_security.is_npc_blocked = MagicMock(return_value=False)
        mock_security.is_request_type_allowed = MagicMock(return_value=True)
        mock_security.validate_api_key = MagicMock(return_value=False)
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={},
            api_key=None
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 401
        assert "Invalid or missing API key" in response.error
    
    @pytest.mark.asyncio
    async def test_process_npc_blocked(self):
        """Test request processing with blocked NPC."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = True
        mock_security.auth_method = "api_key"
        mock_security.is_npc_blocked = MagicMock(return_value=True)
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={},
            npc_name="blocked_npc"
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 401
        assert "blocked" in response.error.lower()
    
    @pytest.mark.asyncio
    async def test_process_request_type_not_allowed(self):
        """Test request processing with disallowed request type."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = True
        mock_security.auth_method = "api_key"
        mock_security.is_npc_blocked = MagicMock(return_value=False)
        mock_security.is_request_type_allowed = MagicMock(return_value=False)
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="forbidden_type",
            endpoint="/test",
            request_data={}
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 401
        assert "not allowed" in response.error.lower()
    
    @pytest.mark.asyncio
    async def test_process_rate_limit_exceeded(self):
        """Test request processing with rate limit exceeded."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse, RateLimiter
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = False
        mock_security.rate_limit_enabled = True
        
        # Create a rate limiter that's already at limit
        limiter = RateLimiter(per_npc_limit=1, global_limit=1, window_seconds=60)
        limiter.record_request("test_npc")
        
        BaseHandler._rate_limiter = limiter
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={},
            npc_name="test_npc"
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 429
        assert "rate limit" in response.error.lower()
    
    @pytest.mark.asyncio
    async def test_process_validation_failure(self):
        """Test request processing with validation failure."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            def validate(self, request):
                return False, "Custom validation error"
            
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = False
        mock_security.rate_limit_enabled = False
        mock_security.validate_requests = False
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 400
        assert "Custom validation error" in response.error
    
    @pytest.mark.asyncio
    async def test_process_request_too_large(self):
        """Test request processing with request data too large."""
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = False
        mock_security.rate_limit_enabled = False
        mock_security.validate_requests = True
        mock_security.max_request_size = 10  # Very small limit
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={"large_field": "x" * 100}  # Exceeds limit
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 400
        assert "too large" in response.error.lower()
    
    @pytest.mark.asyncio
    async def test_process_handler_exception(self):
        """Test request processing when handler raises exception."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                raise ValueError("Handler error")
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = None
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        response = await handler.process(request)
        assert response.success is False
        assert response.status_code == 500
        assert "Internal error" in response.error
    
    def test_set_security_config(self):
        """Test setting security configuration."""
        from handlers.base_handler import BaseHandler
        
        mock_security = MagicMock()
        BaseHandler.set_security_config(mock_security)
        assert BaseHandler._security_config == mock_security
    
    def test_authenticate_no_security(self):
        """Test authentication with no security config."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        BaseHandler._security_config = None
        handler = TestHandler(None)
        
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        error = handler._authenticate(request)
        assert error is None  # No error = allowed
    
    def test_authenticate_auth_disabled(self):
        """Test authentication when auth is disabled."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_security = MagicMock()
        mock_security.auth_enabled = False
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        error = handler._authenticate(request)
        assert error is None
    
    def test_authenticate_auth_method_none(self):
        """Test authentication with auth_method=none."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_security = MagicMock()
        mock_security.auth_enabled = True
        mock_security.auth_method = "none"
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        error = handler._authenticate(request)
        assert error is None
    
    def test_check_rate_limit_disabled(self):
        """Test rate limit check when disabled."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_security = MagicMock()
        mock_security.rate_limit_enabled = False
        BaseHandler._security_config = mock_security
        BaseHandler._rate_limiter = None
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        allowed, error = handler._check_rate_limit(request)
        assert allowed is True
        assert error is None
    
    def test_check_rate_limit_no_limiter(self):
        """Test rate limit check when no limiter configured."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_security = MagicMock()
        mock_security.rate_limit_enabled = True
        BaseHandler._security_config = mock_security
        BaseHandler._rate_limiter = None
        
        # Need to avoid _init_rate_limiter creating one
        handler = TestHandler.__new__(TestHandler)
        handler.config = None
        BaseHandler._rate_limiter = None
        
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        allowed, error = handler._check_rate_limit(request)
        assert allowed is True
    
    def test_validate_default(self):
        """Test default validation (always passes)."""
        from handlers.base_handler import BaseHandler, HandlerRequest
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        valid, error = handler.validate(request)
        assert valid is True
        assert error is None


# =============================================================================
# Tests for handlers/ai_async.py
# =============================================================================

class TestAIAsyncRequestHandler:
    """Tests for AIAsyncRequestHandler."""
    
    @pytest.fixture
    def mock_config(self):
        """Create mock config for handler."""
        config = MagicMock()
        config.service_name = "test_service"
        return config
    
    @pytest.mark.asyncio
    async def test_handle_basic_request(self, mock_config):
        """Test handling a basic async request."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(mock_config)
        request = {
            "command": "ai_request_async",
            "data": {
                "request_type": "dialogue",
                "npc_id": "guard_001",
                "data": {"message": "Hello"},
            }
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert "async_request_id" in response
        assert response["state"] == "pending"
    
    @pytest.mark.asyncio
    async def test_handle_custom_request_type(self, mock_config):
        """Test handling custom request type."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(mock_config)
        request = {
            "data": {
                "request_type": "custom",
                "npc_id": "merchant",
                "data": {"custom_field": "value"},
                "timeout_seconds": 60,
            }
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert response["state"] == "pending"
    
    @pytest.mark.asyncio
    async def test_handle_with_callback_event(self, mock_config):
        """Test handling request with callback event."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        import json
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "request_type": "decision",
                "npc_id": "boss",
                "data": {},
                "callback_event": "OnAIComplete",
            })
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        async_id = response["async_request_id"]
        assert _async_requests[async_id]["callback_event"] == "OnAIComplete"
    
    @pytest.mark.asyncio
    async def test_validate_invalid_request_type(self, mock_config):
        """Test validation with invalid request type."""
        from handlers.ai_async import AIAsyncRequestHandler
        from handlers.base import ValidationError
        import json
        
        handler = AIAsyncRequestHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "request_type": "invalid_type",
                "npc_id": "test",
            })
        }
        
        with pytest.raises(ValidationError) as exc_info:
            await handler.handle(request)
        
        assert "invalid_type" in str(exc_info.value).lower()
    
    @pytest.mark.asyncio
    async def test_timeout_capped_to_max(self, mock_config):
        """Test timeout is capped to maximum."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(mock_config)
        request = {
            "data": {
                "request_type": "dialogue",
                "npc_id": "test",
                "data": {},
                "timeout_seconds": 9999,  # Exceeds MAX_TIMEOUT_SECONDS
            }
        }
        
        response = await handler.handle(request)
        
        async_id = response["async_request_id"]
        assert _async_requests[async_id]["timeout_seconds"] <= 300


class TestAIAsyncProcessing:
    """Tests for async request processing."""
    
    @pytest.mark.asyncio
    async def test_process_async_request_not_found(self):
        """Test processing when request not found."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(MagicMock())
        
        # Process non-existent request
        await handler._process_async_request("non-existent-id")
        # Should log error but not raise


class TestAICheckResultHandler:
    """Tests for AICheckResultHandler."""
    
    @pytest.fixture
    def mock_config(self):
        """Create mock config."""
        config = MagicMock()
        config.service_name = "test"
        return config
    
    @pytest.mark.asyncio
    async def test_check_result_not_found(self, mock_config):
        """Test checking result for non-existent request."""
        from handlers.ai_async import AICheckResultHandler, _async_requests
        import json
        
        _async_requests.clear()
        
        handler = AICheckResultHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "async_request_id": "non-existent-uuid"
            })
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "error"
        assert "not found" in response["error"].lower()
    
    @pytest.mark.asyncio
    async def test_check_result_pending(self, mock_config):
        """Test checking result for pending request."""
        from handlers.ai_async import AICheckResultHandler, _async_requests
        from datetime import datetime
        import json
        
        _async_requests.clear()
        
        # Add a pending request
        test_id = "test-pending-id"
        _async_requests[test_id] = {
            "id": test_id,
            "request_type": "dialogue",
            "npc_id": "guard",
            "state": "pending",
            "created_at": datetime.utcnow().isoformat(),
        }
        
        handler = AICheckResultHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "async_request_id": test_id
            })
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert response["state"] == "pending"
    
    @pytest.mark.asyncio
    async def test_check_result_completed(self, mock_config):
        """Test checking result for completed request."""
        from handlers.ai_async import AICheckResultHandler, _async_requests, _async_results
        from datetime import datetime
        import json
        
        _async_requests.clear()
        _async_results.clear()
        
        # Add a completed request
        test_id = "test-completed-id"
        _async_requests[test_id] = {
            "id": test_id,
            "request_type": "dialogue",
            "npc_id": "guard",
            "state": "completed",
            "created_at": datetime.utcnow().isoformat(),
        }
        _async_results[test_id] = {
            "status": "completed",
            "result": {"dialogue": "Hello!"},
            "completed_at": datetime.utcnow().isoformat(),
        }
        
        handler = AICheckResultHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "async_request_id": test_id
            })
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert response["state"] == "completed"
        assert response["result"]["dialogue"] == "Hello!"
    
    @pytest.mark.asyncio
    async def test_check_result_failed(self, mock_config):
        """Test checking result for failed request."""
        from handlers.ai_async import AICheckResultHandler, _async_requests, _async_results
        from datetime import datetime
        import json
        
        _async_requests.clear()
        _async_results.clear()
        
        # Add a failed request
        test_id = "test-failed-id"
        _async_requests[test_id] = {
            "id": test_id,
            "request_type": "dialogue",
            "npc_id": "guard",
            "state": "failed",
            "created_at": datetime.utcnow().isoformat(),
        }
        _async_results[test_id] = {
            "status": "failed",
            "error": "AI service unavailable",
            "failed_at": datetime.utcnow().isoformat(),
        }
        
        handler = AICheckResultHandler(mock_config)
        request = {
            "request_data": json.dumps({
                "async_request_id": test_id
            })
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert response["state"] == "failed"
        assert "AI service unavailable" in response["error"]
    
    @pytest.mark.asyncio
    async def test_check_result_missing_id(self, mock_config):
        """Test checking result without async_request_id."""
        from handlers.ai_async import AICheckResultHandler
        from handlers.base import ValidationError
        
        handler = AICheckResultHandler(mock_config)
        request = {
            "data": {}
        }
        
        with pytest.raises(ValidationError) as exc_info:
            await handler.handle(request)
        
        assert "async_request_id" in str(exc_info.value).lower()


class TestAsyncCallbackHandler:
    """Tests for async callback handling."""
    
    @pytest.mark.asyncio
    async def test_handle_callback_completed(self):
        """Test handling completed callback."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests, _async_results
        from datetime import datetime
        
        _async_requests.clear()
        _async_results.clear()
        
        # Setup a request waiting for callback
        test_job_id = "job-123"
        test_request_id = "req-456"
        _async_requests[test_request_id] = {
            "id": test_request_id,
            "job_id": test_job_id,
            "state": "processing",
            "request_type": "dialogue",
            "created_at": datetime.utcnow().isoformat(),
        }
        
        handler = AIAsyncRequestHandler(MagicMock())
        callback_data = {
            "job_id": test_job_id,
            "status": "completed",
            "result": {"response": "Hello!"}
        }
        
        response = await handler.handle_callback(1, callback_data)
        
        assert response["status"] == "ok"
        assert _async_requests[test_request_id]["state"] == "completed"
        assert _async_results[test_request_id]["status"] == "completed"
    
    @pytest.mark.asyncio
    async def test_handle_callback_failed(self):
        """Test handling failed callback."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests, _async_results
        from datetime import datetime
        
        _async_requests.clear()
        _async_results.clear()
        
        # Setup a request waiting for callback
        test_job_id = "job-789"
        test_request_id = "req-012"
        _async_requests[test_request_id] = {
            "id": test_request_id,
            "job_id": test_job_id,
            "state": "processing",
            "request_type": "dialogue",
            "created_at": datetime.utcnow().isoformat(),
        }
        
        handler = AIAsyncRequestHandler(MagicMock())
        callback_data = {
            "job_id": test_job_id,
            "status": "failed",
            "error": "Timeout occurred"
        }
        
        response = await handler.handle_callback(1, callback_data)
        
        assert response["status"] == "ok"
        assert _async_requests[test_request_id]["state"] == "failed"
        assert _async_results[test_request_id]["status"] == "failed"
    
    @pytest.mark.asyncio
    async def test_handle_callback_unknown_job(self):
        """Test handling callback for unknown job."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        
        _async_requests.clear()
        
        handler = AIAsyncRequestHandler(MagicMock())
        callback_data = {
            "job_id": "unknown-job",
            "status": "completed",
            "result": {}
        }
        
        response = await handler.handle_callback(1, callback_data)
        
        # Should still return ok but not update anything
        assert response["status"] == "ok"


class TestAsyncUtilityFunctions:
    """Tests for async utility functions."""
    
    def test_cleanup_expired_requests(self):
        """Test cleaning up expired async requests."""
        from handlers.ai_async import cleanup_expired_requests, _async_requests, _async_results
        from datetime import datetime, timedelta
        
        _async_requests.clear()
        _async_results.clear()
        
        # Add an old request
        old_time = (datetime.utcnow() - timedelta(hours=2)).isoformat()
        _async_requests["old-req"] = {
            "id": "old-req",
            "created_at": old_time,
            "state": "completed"
        }
        _async_results["old-req"] = {"status": "completed"}
        
        # Add a recent request
        recent_time = datetime.utcnow().isoformat()
        _async_requests["new-req"] = {
            "id": "new-req",
            "created_at": recent_time,
            "state": "pending"
        }
        
        cleaned = cleanup_expired_requests(max_age_seconds=3600)  # 1 hour
        
        assert cleaned == 1
        assert "old-req" not in _async_requests
        assert "old-req" not in _async_results
        assert "new-req" in _async_requests
    
    def test_get_pending_request_count(self):
        """Test counting pending requests."""
        from handlers.ai_async import get_pending_request_count, _async_requests
        from datetime import datetime
        
        _async_requests.clear()
        
        _async_requests["req1"] = {"state": "pending", "created_at": datetime.utcnow().isoformat()}
        _async_requests["req2"] = {"state": "pending", "created_at": datetime.utcnow().isoformat()}
        _async_requests["req3"] = {"state": "processing", "created_at": datetime.utcnow().isoformat()}
        _async_requests["req4"] = {"state": "completed", "created_at": datetime.utcnow().isoformat()}
        
        count = get_pending_request_count()
        assert count == 2
    
    def test_get_processing_request_count(self):
        """Test counting processing requests."""
        from handlers.ai_async import get_processing_request_count, _async_requests
        from datetime import datetime
        
        _async_requests.clear()
        
        _async_requests["req1"] = {"state": "pending", "created_at": datetime.utcnow().isoformat()}
        _async_requests["req2"] = {"state": "processing", "created_at": datetime.utcnow().isoformat()}
        _async_requests["req3"] = {"state": "processing", "created_at": datetime.utcnow().isoformat()}
        
        count = get_processing_request_count()
        assert count == 2


# =============================================================================
# Tests for handlers/health_check.py
# =============================================================================

class TestHealthCheckHandler:
    """Tests for HealthCheckHandler."""
    
    @pytest.fixture
    def mock_config(self):
        """Create mock config with all required attributes."""
        config = MagicMock()
        config.service_name = "ai_ipc_service"
        config.version = "1.0.0"
        config.ai_service = MagicMock()
        config.ai_service.base_url = "http://localhost:8000"
        return config
    
    @pytest.mark.asyncio
    async def test_basic_health_check(self, mock_config):
        """Test basic health check response."""
        from handlers.health_check import HealthCheckHandler
        
        handler = HealthCheckHandler(mock_config)
        request = {
            "endpoint": "/health"
        }
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert response["service"] == "ai_ipc_service"
        assert response["version"] == "1.0.0"
        assert "timestamp" in response
    
    @pytest.mark.asyncio
    async def test_detailed_health_check(self, mock_config):
        """Test detailed health check with components."""
        from handlers.health_check import HealthCheckHandler
        
        handler = HealthCheckHandler(mock_config)
        request = {
            "endpoint": "/health/detailed"
        }
        
        with patch("aiohttp.ClientSession") as mock_session:
            # Mock successful AI service check
            mock_response = AsyncMock()
            mock_response.status = 200
            mock_context = AsyncMock()
            mock_context.__aenter__.return_value = mock_response
            mock_session.return_value.__aenter__.return_value.get.return_value = mock_context
            
            response = await handler.handle(request)
        
        assert response["status"] == "ok"
        assert "components" in response
        assert "system" in response
    
    @pytest.mark.asyncio
    async def test_health_check_with_uptime(self, mock_config):
        """Test health check includes uptime."""
        from handlers.health_check import HealthCheckHandler
        
        # Reset start time
        HealthCheckHandler._start_time = None
        
        handler = HealthCheckHandler(mock_config)
        request = {"endpoint": "/health"}
        
        response = await handler.handle(request)
        
        assert "uptime_seconds" in response
        assert response["uptime_seconds"] >= 0


class TestHealthCheckAIServiceCheck:
    """Tests for AI service health check."""
    
    @pytest.fixture
    def mock_config(self):
        """Create mock config."""
        config = MagicMock()
        config.service_name = "test"
        config.version = "1.0"
        config.ai_service = MagicMock()
        config.ai_service.base_url = "http://ai-service:8000"
        return config
    
    @pytest.mark.asyncio
    async def test_ai_service_healthy(self, mock_config):
        """Test AI service check when healthy."""
        from handlers.health_check import HealthCheckHandler
        
        handler = HealthCheckHandler(mock_config)
        
        with patch("handlers.health_check.aiohttp.ClientSession") as MockSession:
            # Create proper async context manager chain
            mock_response = MagicMock()
            mock_response.status = 200
            
            mock_get_context = AsyncMock()
            mock_get_context.__aenter__.return_value = mock_response
            
            mock_session = MagicMock()
            mock_session.get = MagicMock(return_value=mock_get_context)
            
            mock_session_context = AsyncMock()
            mock_session_context.__aenter__.return_value = mock_session
            
            MockSession.return_value = mock_session_context
            
            result = await handler._check_ai_service()
        
        assert result["status"] == "healthy"
        assert result["url"] == "http://ai-service:8000"
    
    @pytest.mark.asyncio
    async def test_ai_service_degraded(self, mock_config):
        """Test AI service check when degraded (non-200 response)."""
        from handlers.health_check import HealthCheckHandler
        
        handler = HealthCheckHandler(mock_config)
        
        with patch("handlers.health_check.aiohttp.ClientSession") as MockSession:
            mock_response = MagicMock()
            mock_response.status = 503
            
            mock_get_context = AsyncMock()
            mock_get_context.__aenter__.return_value = mock_response
            
            mock_session = MagicMock()
            mock_session.get = MagicMock(return_value=mock_get_context)
            
            mock_session_context = AsyncMock()
            mock_session_context.__aenter__.return_value = mock_session
            
            MockSession.return_value = mock_session_context
            
            result = await handler._check_ai_service()
        
        assert result["status"] == "degraded"
        assert result["http_status"] == 503
    
    @pytest.mark.asyncio
    async def test_ai_service_timeout(self, mock_config):
        """Test AI service check when timeout occurs."""
        from handlers.health_check import HealthCheckHandler
        import aiohttp
        
        handler = HealthCheckHandler(mock_config)
        
        with patch("handlers.health_check.aiohttp.ClientSession") as MockSession:
            mock_session = MagicMock()
            mock_session.get = MagicMock(side_effect=asyncio.TimeoutError())
            
            mock_session_context = AsyncMock()
            mock_session_context.__aenter__.return_value = mock_session
            
            MockSession.return_value = mock_session_context
            
            result = await handler._check_ai_service()
        
        assert result["status"] == "timeout"
        assert "timeout" in result["error"].lower()
    
    @pytest.mark.asyncio
    async def test_ai_service_connection_error(self, mock_config):
        """Test AI service check when connection fails."""
        from handlers.health_check import HealthCheckHandler
        import aiohttp
        
        handler = HealthCheckHandler(mock_config)
        
        with patch("handlers.health_check.aiohttp.ClientSession") as MockSession:
            mock_session = MagicMock()
            mock_session.get = MagicMock(side_effect=aiohttp.ClientError("Connection refused"))
            
            mock_session_context = AsyncMock()
            mock_session_context.__aenter__.return_value = mock_session
            
            MockSession.return_value = mock_session_context
            
            result = await handler._check_ai_service()
        
        assert result["status"] == "unhealthy"
        assert "error" in result
    
    @pytest.mark.asyncio
    async def test_ai_service_unexpected_error(self, mock_config):
        """Test AI service check with unexpected error."""
        from handlers.health_check import HealthCheckHandler
        
        handler = HealthCheckHandler(mock_config)
        
        with patch("handlers.health_check.aiohttp.ClientSession") as MockSession:
            mock_session = MagicMock()
            mock_session.get = MagicMock(side_effect=RuntimeError("Unexpected"))
            
            mock_session_context = AsyncMock()
            mock_session_context.__aenter__.return_value = mock_session
            
            MockSession.return_value = mock_session_context
            
            result = await handler._check_ai_service()
        
        assert result["status"] == "error"


class TestSystemInfo:
    """Tests for system info retrieval."""
    
    def test_get_system_info(self):
        """Test getting system information."""
        from handlers.health_check import HealthCheckHandler
        
        mock_config = MagicMock()
        mock_config.service_name = "test"
        mock_config.version = "1.0"
        
        handler = HealthCheckHandler(mock_config)
        info = handler._get_system_info()
        
        assert "python_version" in info
        assert "platform" in info
        assert "processor" in info


class TestReadinessHandler:
    """Tests for ReadinessHandler."""
    
    @pytest.mark.asyncio
    async def test_readiness_check(self):
        """Test readiness probe returns ready."""
        from handlers.health_check import ReadinessHandler
        
        mock_config = MagicMock()
        mock_config.service_name = "test"
        mock_config.version = "1.0"
        
        handler = ReadinessHandler(mock_config)
        response = await handler.handle({})
        
        assert response["ready"] is True
        assert "timestamp" in response


class TestLivenessHandler:
    """Tests for LivenessHandler."""
    
    @pytest.mark.asyncio
    async def test_liveness_check(self):
        """Test liveness probe returns alive."""
        from handlers.health_check import LivenessHandler
        
        mock_config = MagicMock()
        mock_config.service_name = "test"
        mock_config.version = "1.0"
        
        handler = LivenessHandler(mock_config)
        response = await handler.handle({})
        
        assert response["alive"] is True
        assert "timestamp" in response


# =============================================================================
# Tests for database.py compatibility methods
# =============================================================================

class TestDatabaseCompatMethods:
    """Tests for database compatibility methods (mocked)."""
    
    @pytest.fixture
    def mock_db_config(self):
        """Create mock database config."""
        config = MagicMock()
        config.host = "localhost"
        config.port = 3306
        config.user = "test"
        config.password = "test"
        config.database = "test_db"
        config.charset = "utf8mb4"
        config.pool_size = 5
        config.pool_recycle = 3600
        config.connect_timeout = 10
        return config
    
    def test_database_init(self, mock_db_config):
        """Test Database initialization."""
        from database import Database
        
        db = Database(mock_db_config)
        assert db.config == mock_db_config
        assert db._pool is None
        assert db._connected is False
    
    def test_is_connected_property(self, mock_db_config):
        """Test is_connected property."""
        from database import Database
        
        db = Database(mock_db_config)
        assert db.is_connected is False
        
        db._connected = True
        assert db.is_connected is False  # Still false because pool is None
        
        db._pool = MagicMock()
        assert db.is_connected is True
    
    @pytest.mark.asyncio
    async def test_disconnect_when_not_connected(self, mock_db_config):
        """Test disconnect when not connected."""
        from database import Database
        
        db = Database(mock_db_config)
        await db.disconnect()  # Should not raise
        assert db._pool is None
    
    @pytest.mark.asyncio
    async def test_ensure_connected_already_connected(self, mock_db_config):
        """Test _ensure_connected when already connected."""
        from database import Database
        
        db = Database(mock_db_config)
        db._connected = True
        db._pool = MagicMock()
        
        await db._ensure_connected()  # Should return immediately


class TestDatabaseManagerAlias:
    """Test DatabaseManager alias."""
    
    def test_database_manager_alias(self):
        """Test that DatabaseManager is an alias for Database."""
        from database import Database, DatabaseManager
        
        assert DatabaseManager is Database