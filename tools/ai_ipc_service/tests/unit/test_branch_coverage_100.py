"""
Tests to achieve 100% branch coverage.

This module specifically targets the remaining partial branches identified
by the coverage report to reach 100% coverage.
"""

import asyncio
import json
import time
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch, PropertyMock
import pytest


# =============================================================================
# handlers/ai_async.py - branches 292->300, 364->368
# =============================================================================

class TestAIAsyncBranchCoverage:
    """Cover missing branches in ai_async.py."""
    
    @pytest.mark.asyncio
    async def test_handle_callback_with_unknown_status(self):
        """Test handle_callback when status is neither 'completed' nor 'failed'.
        
        Covers branch 292->300: status not in (completed, failed)
        """
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests, _async_results
        
        _async_requests.clear()
        _async_results.clear()
        
        # Setup a request
        test_job_id = "job-unknown-status"
        test_request_id = "req-unknown-status"
        _async_requests[test_request_id] = {
            "id": test_request_id,
            "job_id": test_job_id,
            "state": "processing",
            "request_type": "dialogue",
            "created_at": datetime.utcnow().isoformat(),
        }
        
        handler = AIAsyncRequestHandler(MagicMock())
        
        # Call with an unknown status (not completed, not failed)
        callback_data = {
            "job_id": test_job_id,
            "status": "pending",  # Neither completed nor failed
            "result": {}
        }
        
        response = await handler.handle_callback(1, callback_data)
        
        # Should still return ok but not update results
        assert response["status"] == "ok"
        assert response["callback_status"] == "pending"
        # The request should NOT be updated to completed or failed
        assert _async_requests[test_request_id]["state"] == "processing"
        assert test_request_id not in _async_results
    
    @pytest.mark.asyncio
    async def test_check_result_with_unknown_result_status(self):
        """Test check_result when result exists but has unexpected status.
        
        Covers branch 364->368: result_info["status"] not in (completed, failed)
        """
        from handlers.ai_async import AICheckResultHandler, _async_requests, _async_results
        
        _async_requests.clear()
        _async_results.clear()
        
        # Add a request with result that has an unusual status
        test_id = "test-unusual-status"
        _async_requests[test_id] = {
            "id": test_id,
            "request_type": "dialogue",
            "npc_id": "guard",
            "state": "processing",
            "created_at": datetime.utcnow().isoformat(),
        }
        # Result with status that's neither "completed" nor "failed"
        _async_results[test_id] = {
            "status": "unknown",  # Not completed or failed
            "data": {"partial": True},
        }
        
        handler = AICheckResultHandler(MagicMock())
        request = {
            "request_data": json.dumps({
                "async_request_id": test_id
            })
        }
        
        response = await handler.handle(request)
        
        # Should return OK status (outer) but no result/error fields added
        assert response["status"] == "ok"
        assert response["state"] == "processing"
        # Neither "result" nor "error" should be added for unknown status
        assert "completed_at" not in response
        assert "failed_at" not in response


# =============================================================================
# handlers/ai_decision.py - branch 260->264
# =============================================================================

class TestAIDecisionBranchCoverage:
    """Cover missing branches in ai_decision.py."""
    
    def test_analyze_context_high_threat_no_flee_no_alert(self):
        """Test high threat when neither alert nor flee are available.
        
        Covers branch 260->264: threat > 0.7 but flee NOT in available_actions
        """
        from handlers.ai_decision import AIDecisionHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.base_url = "http://test"
        mock_config.ai_service.timeout_seconds = 30
        
        handler = AIDecisionHandler(mock_config)
        
        # High threat but neither alert nor flee available
        context = {"threat_level": 0.9}
        available_actions = ["patrol", "rest", "idle"]  # No alert, no flee
        
        action, confidence, reasoning = handler._analyze_context(context, available_actions)
        
        # Should fall through to next condition (nearby_players, etc.)
        # With empty context for other conditions, should default to weighted random
        assert action in available_actions
        assert confidence >= 0.0


# =============================================================================
# handlers/ai_emotion.py - branches 361->360, 363->360
# =============================================================================

class TestAIEmotionBranchCoverage:
    """Cover missing branches in ai_emotion.py."""
    
    def test_apply_personality_modifier_unknown_trait(self):
        """Test personality modifier with unknown trait.
        
        Covers branch 361->360: trait NOT in trait_modifiers
        """
        from handlers.ai_emotion import AIEmotionHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.base_url = "http://test"
        mock_config.ai_service.timeout_seconds = 30
        
        handler = AIEmotionHandler(mock_config)
        
        # Use personality traits that don't exist in trait_modifiers
        result = handler._apply_personality_modifier(
            base_intensity=0.5,
            emotion="happy",
            personality_traits=["unknown_trait", "nonexistent_trait"],
            relationship_level=50
        )
        
        # Should return modified intensity without errors
        assert 0.0 <= result <= 1.0
    
    def test_apply_personality_modifier_trait_without_emotion(self):
        """Test personality modifier when trait exists but emotion not in its modifiers.
        
        Covers branch 363->360: trait in trait_modifiers but emotion NOT in modifiers[trait]
        """
        from handlers.ai_emotion import AIEmotionHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.base_url = "http://test"
        mock_config.ai_service.timeout_seconds = 30
        
        handler = AIEmotionHandler(mock_config)
        
        # "friendly" trait exists but doesn't have modifier for "surprised"
        # friendly: {"happy": 0.1, "angry": -0.1, "fearful": -0.05}
        result = handler._apply_personality_modifier(
            base_intensity=0.5,
            emotion="surprised",  # Not in friendly's modifiers
            personality_traits=["friendly"],
            relationship_level=50
        )
        
        # Should return base intensity (no modification for this emotion)
        assert 0.0 <= result <= 1.0


# =============================================================================
# handlers/base.py - branches 211->214, 380->exit
# =============================================================================

class TestBaseBranchCoverage:
    """Cover missing branches in base.py."""
    
    def test_create_success_response_with_none_data(self):
        """Test create_success_response when data is None.
        
        Covers branch 211->214: data IS None
        """
        from handlers.base import BaseHandler
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_config = MagicMock()
        handler = TestHandler(mock_config)
        
        # Call with data=None (default)
        response = handler.create_success_response()
        
        assert response["status"] == "ok"
        assert "data" not in response  # Should NOT have data key
        assert "timestamp" in response
    
    def test_create_success_response_with_data(self):
        """Test create_success_response when data is provided."""
        from handlers.base import BaseHandler
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_config = MagicMock()
        handler = TestHandler(mock_config)
        
        # Call with data provided
        response = handler.create_success_response(data={"key": "value"})
        
        assert response["status"] == "ok"
        assert response["data"] == {"key": "value"}
    
    def test_rate_limiter_reset_nonexistent_key(self):
        """Test RateLimiter.reset with key that doesn't exist.
        
        Covers branch 380->exit: key provided but not in _request_counts
        """
        from handlers.base import RateLimiter
        
        limiter = RateLimiter()
        
        # Add some data for a different key
        limiter._request_counts["existing_key"] = [datetime.utcnow()]
        
        # Reset a key that doesn't exist - should not raise
        limiter.reset("nonexistent_key")
        
        # The existing key should still be there
        assert "existing_key" in limiter._request_counts
    
    def test_rate_limiter_reset_all_keys(self):
        """Test RateLimiter.reset without key (resets all)."""
        from handlers.base import RateLimiter
        
        limiter = RateLimiter()
        limiter._request_counts["key1"] = [datetime.utcnow()]
        limiter._request_counts["key2"] = [datetime.utcnow()]
        
        # Reset all
        limiter.reset(None)
        
        assert len(limiter._request_counts) == 0


# =============================================================================
# handlers/base_handler.py - branches 131->138, 264->268, 359->363
# =============================================================================

class TestBaseHandlerBranchCoverage:
    """Cover missing branches in base_handler.py."""
    
    def test_check_rate_limit_without_npc_name(self):
        """Test check_rate_limit when npc_name is None.
        
        Covers branch 131->138: npc_name is None/empty
        """
        from handlers.base_handler import RateLimiter
        
        limiter = RateLimiter(per_npc_limit=10, global_limit=100)
        
        # Check without npc_name - should only check global limit
        allowed, error = limiter.check_rate_limit(npc_name=None)
        
        assert allowed is True
        assert error is None
        
        # Should NOT add to any NPC tracking
        limiter.record_request(npc_name=None)
        assert len(limiter._npc_requests) == 0
    
    @pytest.mark.asyncio
    async def test_process_without_rate_limiter(self):
        """Test process when _rate_limiter is None.
        
        Covers branch 264->268: BaseHandler._rate_limiter is None
        """
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={"test": True})
        
        # Ensure no rate limiter and no security config
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = None
        
        # Create handler without going through __init__ to avoid creating limiter
        handler = TestHandler.__new__(TestHandler)
        handler.config = None
        
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={}
        )
        
        response = await handler.process(request)
        assert response.success is True
    
    @pytest.mark.asyncio
    async def test_validate_request_size_ok(self):
        """Test _validate_request when size is within limit.
        
        Covers branch 359->363: len(data_str) <= max_request_size
        """
        from handlers.base_handler import BaseHandler, HandlerRequest, HandlerResponse
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return HandlerResponse(success=True, data={})
        
        mock_security = MagicMock()
        mock_security.auth_enabled = False
        mock_security.rate_limit_enabled = False
        mock_security.validate_requests = True
        mock_security.max_request_size = 10000  # Large limit
        
        BaseHandler._rate_limiter = None
        BaseHandler._security_config = mock_security
        
        handler = TestHandler(None)
        request = HandlerRequest(
            request_id=1,
            request_type="test",
            endpoint="/test",
            request_data={"small": "data"}  # Small data
        )
        
        valid, error = handler._validate_request(request)
        assert valid is True
        assert error is None


# =============================================================================
# handlers/health_check.py - branch 90->95
# =============================================================================

class TestHealthCheckBranchCoverage:
    """Cover missing branches in health_check.py."""
    
    @pytest.mark.asyncio
    async def test_health_check_without_start_time(self):
        """Test health check when _start_time is None.
        
        Covers branch 90->95: self._start_time is None
        """
        from handlers.health_check import HealthCheckHandler
        
        mock_config = MagicMock()
        mock_config.service_name = "test"
        mock_config.version = "1.0"
        mock_config.ai_service.base_url = "http://test"
        
        # Force _start_time to None
        HealthCheckHandler._start_time = None
        
        # Create handler but force _start_time to None again after init
        handler = HealthCheckHandler(mock_config)
        HealthCheckHandler._start_time = None  # Force to None
        
        request = {"endpoint": "/health"}
        
        response = await handler.handle(request)
        
        assert response["status"] == "ok"
        # uptime_seconds should NOT be in response when _start_time is None
        assert "uptime_seconds" not in response


# =============================================================================
# handlers/http_proxy.py - branches 68->81, 70->81, 118->122, 279->exit
# =============================================================================

class TestHttpProxyBranchCoverage:
    """Cover missing branches in http_proxy.py."""
    
    @pytest.mark.asyncio
    async def test_get_session_when_session_closed(self):
        """Test _get_session when session exists but is closed.
        
        Covers branches 68->81, 70->81: session.closed is True
        """
        from handlers.http_proxy import HttpProxyHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.timeout_seconds = 10
        mock_config.ai_service.base_url = "http://test"
        mock_config.version = "1.0"
        
        handler = HttpProxyHandler(mock_config)
        
        # Create a mock closed session
        mock_closed_session = MagicMock()
        mock_closed_session.closed = True
        handler._session = mock_closed_session
        
        # Mock ClientSession to return a new session
        with patch('handlers.http_proxy.aiohttp.ClientSession') as MockSession:
            mock_new_session = MagicMock()
            mock_new_session.closed = False
            MockSession.return_value = mock_new_session
            
            result = await handler._get_session()
            
            # Should create new session since old one was closed
            MockSession.assert_called_once()
            assert result == mock_new_session
    
    @pytest.mark.asyncio
    async def test_handle_with_non_dict_body(self):
        """Test handle when body is not a dict.
        
        Covers branch 118->122: body is NOT isinstance dict
        """
        from handlers.http_proxy import HttpProxyHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.timeout_seconds = 10
        mock_config.ai_service.base_url = "http://test"
        mock_config.version = "1.0"
        
        handler = HttpProxyHandler(mock_config)
        
        # Request with non-dict body (just the string)
        request = {
            "id": 1,
            "request_type": "http_post",
            "endpoint": "/api/test",
            "request_data": json.dumps({
                "body": "string body",  # Not a dict
                "headers": {}
            })
        }
        
        with patch.object(handler, '_get_session') as mock_get_session:
            mock_session = MagicMock()
            mock_response = MagicMock()
            mock_response.status = 200
            mock_response.headers = {"Content-Type": "application/json"}
            mock_response.json = AsyncMock(return_value={"result": "ok"})
            mock_response.raise_for_status = MagicMock()
            mock_response.__aenter__ = AsyncMock(return_value=mock_response)
            mock_response.__aexit__ = AsyncMock(return_value=None)
            
            mock_session.request = MagicMock(return_value=mock_response)
            mock_get_session.return_value = mock_session
            
            result = await handler.handle(request)
            
            # Should handle string body without filtering
            assert "status" in result or "result" in result
    
    @pytest.mark.asyncio
    async def test_close_when_session_none(self):
        """Test close when session is None.
        
        Covers branch 279->exit: self._session is None
        """
        from handlers.http_proxy import HttpProxyHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.timeout_seconds = 10
        mock_config.ai_service.base_url = "http://test"
        mock_config.version = "1.0"
        
        handler = HttpProxyHandler(mock_config)
        handler._session = None  # Ensure None
        
        # Should not raise
        await handler.close()
        
        assert handler._session is None
    
    @pytest.mark.asyncio
    async def test_close_when_session_already_closed(self):
        """Test close when session is already closed.
        
        Covers branch 279->exit: self._session.closed is True
        """
        from handlers.http_proxy import HttpProxyHandler
        
        mock_config = MagicMock()
        mock_config.ai_service.timeout_seconds = 10
        mock_config.ai_service.base_url = "http://test"
        mock_config.version = "1.0"
        
        handler = HttpProxyHandler(mock_config)
        
        mock_session = MagicMock()
        mock_session.closed = True  # Already closed
        handler._session = mock_session
        
        # Should not raise or try to close again
        await handler.close()


# =============================================================================
# main.py - branch 221->275
# =============================================================================

class TestMainBranchCoverage:
    """Cover missing branches in main.py."""
    
    @pytest.mark.asyncio
    async def test_run_exits_via_shutdown_event(self):
        """Test run loop exits via shutdown event (not exception).
        
        Covers branch 221->275: loop exits normally via shutdown
        """
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service.config = MagicMock()
        service.config.polling.interval_ms = 10  # Very short interval
        
        mock_processor = MagicMock()
        mock_processor.process_batch = AsyncMock(return_value=0)
        mock_processor.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0
        })
        mock_processor.cleanup_expired = AsyncMock(return_value=0)
        mock_processor.cleanup_stuck = AsyncMock(return_value=0)
        service.processor = mock_processor
        
        # Set shutdown event after first iteration
        async def trigger_shutdown():
            await asyncio.sleep(0.02)
            service._shutdown_event.set()
        
        # Run both concurrently
        await asyncio.gather(
            service.run(),
            trigger_shutdown(),
        )
        
        # Should have exited cleanly via shutdown
        assert service._shutdown_event.is_set()


# =============================================================================
# processor.py - branches 348->351, 373->376
# =============================================================================

class TestProcessorBranchCoverage:
    """Cover missing branches in processor.py."""
    
    @pytest.mark.asyncio
    async def test_cleanup_expired_returns_zero(self):
        """Test cleanup_expired when no requests are expired.
        
        Covers branch 348->351: count == 0
        """
        from processor import RequestProcessor
        
        mock_config = MagicMock()
        mock_config.polling.batch_size = 10
        mock_config.polling.max_retries = 3
        mock_config.polling.worker_count = 4
        
        mock_db = AsyncMock()
        mock_db.cleanup_expired = AsyncMock(return_value=0)  # No expired
        
        processor = RequestProcessor(mock_db, mock_config)
        processor.stats.requests_expired = 0
        
        result = await processor.cleanup_expired()
        
        assert result == 0
        # Stats should NOT be updated when count is 0
        assert processor.stats.requests_expired == 0
    
    @pytest.mark.asyncio
    async def test_cleanup_stuck_returns_zero(self):
        """Test cleanup_stuck when no requests are stuck.
        
        Covers branch 373->376: count == 0
        """
        from processor import RequestProcessor
        
        mock_config = MagicMock()
        mock_config.polling.batch_size = 10
        mock_config.polling.max_retries = 3
        mock_config.polling.worker_count = 4
        
        mock_db = AsyncMock()
        mock_db.cleanup_stuck_processing = AsyncMock(return_value=0)  # No stuck
        
        processor = RequestProcessor(mock_db, mock_config)
        processor.stats.requests_failed = 0
        
        result = await processor.cleanup_stuck(stuck_threshold_seconds=300)
        
        assert result == 0
        # Stats should NOT be updated when count is 0
        assert processor.stats.requests_failed == 0


# =============================================================================
# Additional edge case tests for full coverage
# =============================================================================

class TestAdditionalBranchCoverage:
    """Additional tests to ensure complete branch coverage."""
    
    def test_base_handler_get_request_data_non_dict_json(self):
        """Test get_request_data when JSON parses to non-dict."""
        from handlers.base import BaseHandler
        
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        mock_config = MagicMock()
        handler = TestHandler(mock_config)
        
        # JSON that parses to a list, not dict
        request = {"request_data": '["item1", "item2"]'}
        data = handler.get_request_data(request)
        
        # Should wrap in {"data": ...}
        assert data == {"data": ["item1", "item2"]}
    
    @pytest.mark.asyncio
    async def test_ai_async_route_to_handler_custom(self):
        """Test _route_to_handler for custom request type."""
        from handlers.ai_async import AIAsyncRequestHandler
        
        handler = AIAsyncRequestHandler(MagicMock())
        
        result = await handler._route_to_handler(
            request_type="custom",
            npc_id="test_npc",
            data={"key": "value"}
        )
        
        assert result["status"] == "ok"
        assert "Custom request processed" in result["message"]
    
    def test_processing_stats_average_time_no_samples(self):
        """Test average_processing_time_ms with no samples."""
        from processor import ProcessingStats
        
        stats = ProcessingStats()
        # No processing times recorded
        
        avg = stats.average_processing_time_ms
        assert avg == 0.0
    
    def test_processing_stats_success_rate_zero_total(self):
        """Test success_rate when total is zero."""
        from processor import ProcessingStats
        
        stats = ProcessingStats()
        # No requests processed at all
        
        rate = stats.success_rate
        assert rate == 0.0