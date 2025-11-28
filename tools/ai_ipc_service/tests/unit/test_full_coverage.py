"""
Full Coverage Tests for AI IPC Service

These tests target all remaining uncovered lines to achieve 100% code coverage.
"""

import asyncio
import json
import logging
import pytest
import signal
import sys
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, Mock, patch, PropertyMock

# Import modules to test
from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
from database import Database
from processor import RequestProcessor, ProcessingStats, ProcessorError
from handlers.base import BaseHandler, HandlerError, ValidationError, TimeoutError, RateLimiter
from handlers.http_proxy import HttpProxyHandler, HttpGetHandler, HttpPostHandler
from handlers.ai_decision import AIDecisionHandler
from handlers.ai_dialogue import AIDialogueHandler


# =============================================================================
# Fixtures
# =============================================================================

@pytest.fixture
def mock_config():
    """Create a properly configured mock config object."""
    config = MagicMock(spec=Config)
    
    # Database config
    config.database = MagicMock(spec=DatabaseConfig)
    config.database.host = "localhost"
    config.database.port = 3306
    config.database.user = "test"
    config.database.password = "test"
    config.database.database = "test"
    config.database.pool_size = 5
    
    # Polling config  
    config.polling = MagicMock(spec=PollingConfig)
    config.polling.interval_ms = 100
    config.polling.batch_size = 50
    config.polling.worker_count = 4
    config.polling.max_retries = 3
    config.polling.retry_delay_ms = 1000
    
    # AI service config
    config.ai_service = MagicMock(spec=AIServiceConfig)
    config.ai_service.base_url = "http://localhost:8000"
    config.ai_service.timeout_seconds = 10
    config.ai_service.dialogue_url = "http://localhost:8000/dialogue"
    config.ai_service.decision_url = "http://localhost:8000/decision"
    config.ai_service.emotion_url = "http://localhost:8000/emotion"
    
    # Logging config
    config.logging = MagicMock(spec=LoggingConfig)
    config.logging.level = "INFO"
    config.logging.format = "json"
    
    # Security config
    config.security = MagicMock(spec=SecurityConfig)
    config.security.auth_enabled = False
    config.security.auth_method = "none"
    config.security.rate_limit_enabled = False
    config.security.api_key = ""
    
    # Service metadata
    config.service_name = "test_service"
    config.version = "1.0.0"
    
    return config


@pytest.fixture
def mock_database():
    """Create a mock database with proper async methods."""
    db = MagicMock(spec=Database)
    db.connect = AsyncMock()
    db.disconnect = AsyncMock()
    db.health_check = AsyncMock(return_value={"status": "healthy"})
    db.fetch_and_mark_processing = AsyncMock(return_value=[])
    db.mark_failed = AsyncMock()
    db.save_response = AsyncMock()
    db.cleanup_expired = AsyncMock(return_value=0)
    db.cleanup_stuck_processing = AsyncMock(return_value=0)
    return db


class AsyncContextManagerMock:
    """Async context manager mock helper."""
    def __init__(self, return_value):
        self.return_value = return_value
        
    async def __aenter__(self):
        return self.return_value
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        pass


# =============================================================================
# Test ProcessingStats Edge Cases (processor.py lines 633, 643, 675)
# =============================================================================

class TestProcessingStatsEdgeCases:
    """Test edge cases in ProcessingStats computed properties."""
    
    def test_requests_per_second_zero_uptime(self):
        """Test requests_per_second when uptime is 0."""
        stats = ProcessingStats()
        # Force zero uptime by setting start_time to now
        stats.start_time = datetime.utcnow()
        # With zero or near-zero uptime, should handle gracefully
        result = stats.requests_per_second
        assert result >= 0  # Should not raise
    
    def test_success_rate_zero_processed(self):
        """Test success_rate when no requests processed."""
        stats = ProcessingStats()
        assert stats.success_rate == 0.0
        assert stats.total_processed == 0
    
    def test_success_rate_with_failures(self):
        """Test success_rate calculation."""
        stats = ProcessingStats()
        stats.requests_processed = 8
        stats.requests_failed = 2
        assert stats.success_rate == 0.8
    
    def test_average_processing_time_empty(self):
        """Test average processing time with no data."""
        stats = ProcessingStats()
        assert stats.average_processing_time_ms == 0.0
    
    def test_average_processing_time_with_data(self):
        """Test average processing time calculation."""
        stats = ProcessingStats()
        stats.record_success(100.0)
        stats.record_success(200.0)
        stats.record_failure(150.0)
        assert stats.average_processing_time_ms == 150.0
    
    def test_record_success_without_time(self):
        """Test record_success without processing time."""
        stats = ProcessingStats()
        stats.record_success()  # No processing_time_ms
        assert stats.requests_processed == 1
        assert len(stats._processing_times) == 0
    
    def test_record_failure_without_time(self):
        """Test record_failure without processing time."""
        stats = ProcessingStats()
        stats.record_failure()  # No processing_time_ms
        assert stats.requests_failed == 1
        assert len(stats._processing_times) == 0
    
    def test_stats_aliases(self):
        """Test test compatibility aliases."""
        stats = ProcessingStats()
        stats.successful = 5
        stats.failed = 2
        assert stats.successful == 5
        assert stats.failed == 2
        assert stats.total_failed == 2
        assert stats.requests_processed == 5
        assert stats.requests_failed == 2
    
    def test_to_dict_contains_all_keys(self):
        """Test to_dict returns all expected keys."""
        stats = ProcessingStats()
        stats.record_success(100.0)
        result = stats.to_dict()
        
        expected_keys = [
            "requests_processed", "requests_failed", "requests_expired",
            "batches_processed", "uptime_seconds", "requests_per_second",
            "start_time", "total_processed", "total_failed",
            "average_processing_time_ms", "success_rate",
            "successful", "failed", "total_processing_time_ms"
        ]
        for key in expected_keys:
            assert key in result, f"Missing key: {key}"
    
    def test_reset(self):
        """Test reset clears all stats."""
        stats = ProcessingStats()
        stats.record_success(100.0)
        stats.record_failure(50.0)
        stats.requests_expired = 5
        stats.batches_processed = 10
        
        stats.reset()
        
        assert stats.requests_processed == 0
        assert stats.requests_failed == 0
        assert stats.requests_expired == 0
        assert stats.batches_processed == 0
        assert len(stats._processing_times) == 0


# =============================================================================
# Test RequestProcessor (processor.py)
# =============================================================================

class TestRequestProcessorInit:
    """Test RequestProcessor initialization."""
    
    def test_processor_init_with_config(self, mock_config, mock_database):
        """Test processor initializes with config."""
        processor = RequestProcessor(mock_database, mock_config)
        assert processor.db == mock_database
        assert processor.config == mock_config
        assert processor._running == False
        assert isinstance(processor.stats, ProcessingStats)
    
    def test_processor_init_without_config(self, mock_database):
        """Test processor init without config."""
        processor = RequestProcessor(mock_database, None)
        assert processor.config is None
        assert len(processor._handlers) == 0  # No handlers initialized
    
    def test_processor_init_with_security_config(self, mock_config, mock_database):
        """Test processor init with security config parameter."""
        security_config = MagicMock()
        processor = RequestProcessor(
            mock_database, 
            mock_config, 
            security_config=security_config
        )
        assert processor._security_config == security_config


class TestRequestProcessorStartStop:
    """Test processor start/stop methods."""
    
    def test_start(self, mock_config, mock_database):
        """Test processor start."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        assert processor._running == True
        assert processor._semaphore is not None
        assert processor.is_running == True
    
    def test_stop(self, mock_config, mock_database):
        """Test processor stop."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        processor.stop()
        assert processor._running == False
        assert processor.is_running == False
    
    def test_get_stats(self, mock_config, mock_database):
        """Test get_stats returns dict."""
        processor = RequestProcessor(mock_database, mock_config)
        stats = processor.get_stats()
        assert isinstance(stats, dict)
        assert "requests_processed" in stats


class TestRequestProcessorGetHandler:
    """Test processor get_handler method."""
    
    def test_get_handler_known_type(self, mock_config, mock_database):
        """Test getting handler for known type."""
        processor = RequestProcessor(mock_database, mock_config)
        handler = processor.get_handler("ai_dialogue")
        assert handler is not None
    
    def test_get_handler_unknown_type(self, mock_config, mock_database):
        """Test getting handler for unknown type."""
        processor = RequestProcessor(mock_database, mock_config)
        handler = processor.get_handler("unknown_type_xyz")
        assert handler is None


class TestRequestProcessorProcessBatch:
    """Test process_batch method."""
    
    @pytest.mark.asyncio
    async def test_process_batch_empty(self, mock_config, mock_database):
        """Test process_batch returns 0 when no requests."""
        mock_database.fetch_and_mark_processing = AsyncMock(return_value=[])
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        result = await processor.process_batch()
        assert result == 0
    
    @pytest.mark.asyncio
    async def test_process_batch_with_requests(self, mock_config, mock_database):
        """Test process_batch processes requests."""
        requests = [
            {"id": 1, "request_type": "health_check", "endpoint": "/health"}
        ]
        mock_database.fetch_and_mark_processing = AsyncMock(return_value=requests)
        mock_database.save_response = AsyncMock()
        
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        # Mock the handler
        with patch.object(processor, 'get_handler') as mock_get_handler:
            mock_handler = AsyncMock()
            mock_handler.handle = AsyncMock(return_value={"status": "ok"})
            mock_get_handler.return_value = mock_handler
            
            result = await processor.process_batch()
            assert result == 1
            assert processor.stats.batches_processed >= 1
    
    @pytest.mark.asyncio
    async def test_process_batch_database_error(self, mock_config, mock_database):
        """Test process_batch handles database error."""
        from database import DatabaseError
        mock_database.fetch_and_mark_processing = AsyncMock(
            side_effect=DatabaseError("DB error")
        )
        
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        with pytest.raises(ProcessorError):
            await processor.process_batch()


class TestRequestProcessorProcessOne:
    """Test _process_one method."""
    
    @pytest.mark.asyncio
    async def test_process_one_no_handler(self, mock_config, mock_database):
        """Test _process_one with no handler for type."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {
            "id": 1,
            "request_type": "unknown_type_xyz",
            "endpoint": "/test"
        }
        
        await processor._process_one(request)
        mock_database.mark_failed.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_process_one_success(self, mock_config, mock_database):
        """Test _process_one successful processing."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {
            "id": 1,
            "request_type": "health_check",
            "endpoint": "/health"
        }
        
        with patch.object(processor, 'get_handler') as mock_get_handler:
            mock_handler = MagicMock()
            mock_handler.handle = AsyncMock(return_value={"status": "ok"})
            mock_get_handler.return_value = mock_handler
            
            await processor._process_one(request)
            mock_database.save_response.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_process_one_validation_error(self, mock_config, mock_database):
        """Test _process_one with ValidationError."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {
            "id": 1,
            "request_type": "ai_dialogue",
            "endpoint": "/test"
        }
        
        with patch.object(processor, 'get_handler') as mock_get_handler:
            mock_handler = MagicMock()
            # ValidationError only takes message, not status_code
            mock_handler.handle = AsyncMock(
                side_effect=ValidationError("Missing required field")
            )
            mock_get_handler.return_value = mock_handler
            
            await processor._process_one(request)
            # Should save error response, not mark failed
            mock_database.save_response.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_process_one_handler_error_retryable(self, mock_config, mock_database):
        """Test _process_one with retryable HandlerError."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {
            "id": 1,
            "request_type": "ai_dialogue",
            "endpoint": "/test",
            "retry_count": 0
        }
        
        with patch.object(processor, 'get_handler') as mock_get_handler:
            mock_handler = MagicMock()
            mock_handler.handle = AsyncMock(
                side_effect=HandlerError("Temporary failure", 500)
            )
            mock_get_handler.return_value = mock_handler
            
            await processor._process_one(request)
            # Should mark failed with retry
            mock_database.mark_failed.assert_called_once()
            args = mock_database.mark_failed.call_args
            assert args.kwargs.get('should_retry') == True
    
    @pytest.mark.asyncio
    async def test_process_one_handler_error_exhausted_retries(self, mock_config, mock_database):
        """Test _process_one with exhausted retries."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {
            "id": 1,
            "request_type": "ai_dialogue",
            "endpoint": "/test",
            "retry_count": 5  # Exceeds max_retries (3)
        }
        
        with patch.object(processor, 'get_handler') as mock_get_handler:
            mock_handler = MagicMock()
            mock_handler.handle = AsyncMock(
                side_effect=HandlerError("Permanent failure", 500)
            )
            mock_get_handler.return_value = mock_handler
            
            await processor._process_one(request)
            # Should save error response (retries exhausted)
            mock_database.save_response.assert_called_once()


class TestRequestProcessorProcessOneSafe:
    """Test _process_one_safe error handling."""
    
    @pytest.mark.asyncio
    async def test_process_one_safe_success(self, mock_config, mock_database):
        """Test _process_one_safe returns True on success."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {"id": 1, "request_type": "health_check", "endpoint": "/health"}
        
        with patch.object(processor, '_process_one', new_callable=AsyncMock) as mock_process:
            mock_process.return_value = None
            result = await processor._process_one_safe(request)
            assert result == True
    
    @pytest.mark.asyncio
    async def test_process_one_safe_exception(self, mock_config, mock_database):
        """Test _process_one_safe returns False on exception."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        request = {"id": 1, "request_type": "test", "endpoint": "/test"}
        
        with patch.object(processor, '_process_one', new_callable=AsyncMock) as mock_process:
            mock_process.side_effect = Exception("Unexpected error")
            result = await processor._process_one_safe(request)
            assert result == False
            mock_database.mark_failed.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_process_one_safe_db_error_on_mark_failed(self, mock_config, mock_database):
        """Test _process_one_safe handles db error when marking failed."""
        processor = RequestProcessor(mock_database, mock_config)
        processor.start()
        
        mock_database.mark_failed = AsyncMock(side_effect=Exception("DB error"))
        
        request = {"id": 1, "request_type": "test", "endpoint": "/test"}
        
        with patch.object(processor, '_process_one', new_callable=AsyncMock) as mock_process:
            mock_process.side_effect = Exception("Processing error")
            result = await processor._process_one_safe(request)
            assert result == False  # Should still return False


class TestRequestProcessorCleanup:
    """Test cleanup methods."""
    
    @pytest.mark.asyncio
    async def test_cleanup_expired(self, mock_config, mock_database):
        """Test cleanup_expired method."""
        mock_database.cleanup_expired = AsyncMock(return_value=5)
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.cleanup_expired()
        assert result == 5
        assert processor.stats.requests_expired == 5
    
    @pytest.mark.asyncio
    async def test_cleanup_expired_error(self, mock_config, mock_database):
        """Test cleanup_expired handles database error."""
        from database import DatabaseError
        mock_database.cleanup_expired = AsyncMock(side_effect=DatabaseError("DB error"))
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.cleanup_expired()
        assert result == 0
    
    @pytest.mark.asyncio
    async def test_cleanup_stuck(self, mock_config, mock_database):
        """Test cleanup_stuck method."""
        mock_database.cleanup_stuck_processing = AsyncMock(return_value=3)
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.cleanup_stuck(stuck_threshold_seconds=300)
        assert result == 3
    
    @pytest.mark.asyncio
    async def test_cleanup_stuck_error(self, mock_config, mock_database):
        """Test cleanup_stuck handles database error."""
        from database import DatabaseError
        mock_database.cleanup_stuck_processing = AsyncMock(side_effect=DatabaseError("DB error"))
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.cleanup_stuck()
        assert result == 0


class TestRequestProcessorProcess:
    """Test the process method (test compatibility)."""
    
    @pytest.mark.asyncio
    async def test_process_missing_request_type(self, mock_config, mock_database):
        """Test process with missing request_type."""
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.process({})
        assert result["status_code"] == 400
        assert "request_type" in result["error"]
    
    @pytest.mark.asyncio
    async def test_process_invalid_json_payload(self, mock_config, mock_database):
        """Test process with invalid JSON payload."""
        processor = RequestProcessor(mock_database, mock_config)
        
        result = await processor.process({
            "request_type": "test",
            "payload": "invalid json {"
        })
        assert result["status_code"] == 400
        assert "Invalid JSON" in result["error"]
    
    @pytest.mark.asyncio
    async def test_process_no_handler(self, mock_config, mock_database):
        """Test process with no handler for type."""
        processor = RequestProcessor(mock_database, mock_config)
        # Patch _get_handler to return None
        processor._get_handler = MagicMock(return_value=None)
        
        result = await processor.process({"request_type": "unknown"})
        assert result["status_code"] == 404
    
    @pytest.mark.asyncio
    async def test_process_success(self, mock_config, mock_database):
        """Test process successful execution."""
        processor = RequestProcessor(mock_database, mock_config)
        
        mock_handler = MagicMock()
        mock_handler.handle = AsyncMock(return_value={"status": "ok", "data": "test"})
        processor._get_handler = MagicMock(return_value=mock_handler)
        
        result = await processor.process({"request_type": "test"})
        assert result["status_code"] == 200
    
    @pytest.mark.asyncio
    async def test_process_handler_exception(self, mock_config, mock_database):
        """Test process with handler exception."""
        processor = RequestProcessor(mock_database, mock_config)
        
        mock_handler = MagicMock()
        mock_handler.handle = AsyncMock(side_effect=Exception("Handler error"))
        processor._get_handler = MagicMock(return_value=mock_handler)
        
        result = await processor.process({"request_type": "test"})
        assert result["status_code"] == 500
        assert "Handler error" in result["error"]


class TestRequestProcessorProcessBatchList:
    """Test process_batch with list of requests."""
    
    @pytest.mark.asyncio
    async def test_process_batch_with_list(self, mock_config, mock_database):
        """Test process_batch with explicit list of requests."""
        processor = RequestProcessor(mock_database, mock_config)
        
        mock_handler = MagicMock()
        mock_handler.handle = AsyncMock(return_value={"status": "ok"})
        processor._get_handler = MagicMock(return_value=mock_handler)
        
        requests = [
            {"request_type": "test1"},
            {"request_type": "test2"}
        ]
        
        results = await processor.process_batch(requests)
        assert len(results) == 2


# =============================================================================
# Test RateLimiter (handlers/base.py)
# =============================================================================

class TestRateLimiter:
    """Test RateLimiter class."""
    
    def test_is_allowed_under_limit(self):
        """Test is_allowed returns True under limit."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        for _ in range(5):
            assert limiter.is_allowed("test_key") == True
    
    def test_is_allowed_at_limit(self):
        """Test is_allowed returns False at limit."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        for _ in range(5):
            limiter.is_allowed("test_key")
        assert limiter.is_allowed("test_key") == False
    
    def test_get_remaining(self):
        """Test get_remaining returns correct count."""
        limiter = RateLimiter(max_requests=10, window_seconds=60)
        assert limiter.get_remaining("new_key") == 10
        limiter.is_allowed("new_key")
        assert limiter.get_remaining("new_key") == 9
    
    def test_reset_specific_key(self):
        """Test reset with specific key."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        limiter.is_allowed("key1")
        limiter.is_allowed("key2")
        limiter.reset("key1")
        assert limiter.get_remaining("key1") == 5
        assert limiter.get_remaining("key2") == 4
    
    def test_reset_all(self):
        """Test reset all keys."""
        limiter = RateLimiter(max_requests=5, window_seconds=60)
        limiter.is_allowed("key1")
        limiter.is_allowed("key2")
        limiter.reset()
        assert limiter.get_remaining("key1") == 5
        assert limiter.get_remaining("key2") == 5


# =============================================================================
# Test BaseHandler Methods (handlers/base.py)
# =============================================================================

class TestBaseHandlerMethods:
    """Test BaseHandler helper methods."""
    
    def test_get_request_data_empty(self, mock_config):
        """Test get_request_data with empty data."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        result = handler.get_request_data({"request_data": ""})
        assert result == {}
    
    def test_get_request_data_invalid_json(self, mock_config):
        """Test get_request_data with invalid JSON."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        result = handler.get_request_data({"request_data": "invalid{json"})
        assert result == {}
    
    def test_get_request_data_non_dict(self, mock_config):
        """Test get_request_data with non-dict JSON."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        result = handler.get_request_data({"request_data": "[1, 2, 3]"})
        assert result == {"data": [1, 2, 3]}
    
    def test_create_success_response_with_data(self, mock_config):
        """Test create_success_response with data."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        result = handler.create_success_response(data={"key": "value"}, extra_field="extra")
        assert result["status"] == "ok"
        assert result["data"] == {"key": "value"}
        assert result["extra_field"] == "extra"
        assert "timestamp" in result
    
    def test_create_error_response(self, mock_config):
        """Test create_error_response."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        result = handler.create_error_response("Test error", code=400, detail="details")
        assert result["status"] == "error"
        assert result["error"] == "Test error"
        assert result["code"] == 400
        assert result["detail"] == "details"
    
    def test_validate_required_fields_missing(self, mock_config):
        """Test validate_required_fields raises ValidationError."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        with pytest.raises(ValidationError) as exc_info:
            handler.validate_required_fields({"a": 1}, ["a", "b", "c"])
        assert "b" in str(exc_info.value)
        assert "c" in str(exc_info.value)
    
    def test_validate_required_fields_none_value(self, mock_config):
        """Test validate_required_fields with None value."""
        class TestHandler(BaseHandler):
            async def handle(self, request):
                return {}
        
        handler = TestHandler(mock_config)
        with pytest.raises(ValidationError):
            handler.validate_required_fields({"a": None}, ["a"])


# =============================================================================
# Test Handler Exceptions (handlers/base.py lines 40-51)
# =============================================================================

class TestHandlerExceptions:
    """Test handler exception classes."""
    
    def test_handler_error_default_status(self):
        """Test HandlerError default status code."""
        err = HandlerError("Test error")
        assert err.message == "Test error"
        assert err.status_code == 500
    
    def test_handler_error_custom_status(self):
        """Test HandlerError with custom status code."""
        err = HandlerError("Test error", status_code=503)
        assert err.status_code == 503
    
    def test_validation_error_status(self):
        """Test ValidationError has 400 status."""
        err = ValidationError("Invalid input")
        assert err.message == "Invalid input"
        assert err.status_code == 400
    
    def test_timeout_error_status(self):
        """Test TimeoutError has 504 status."""
        err = TimeoutError("Request timed out")
        assert err.message == "Request timed out"
        assert err.status_code == 504


# =============================================================================
# Test HttpProxyHandler (handlers/http_proxy.py)
# =============================================================================

class TestHttpProxyHandler:
    """Test HttpProxyHandler class."""
    
    @pytest.mark.asyncio
    async def test_handle_missing_endpoint(self, mock_config):
        """Test handle raises ValidationError for missing endpoint."""
        handler = HttpProxyHandler(mock_config)
        
        with pytest.raises(ValidationError) as exc_info:
            await handler.handle({"request_type": "http_get", "endpoint": ""})
        assert "Endpoint is required" in str(exc_info.value)
    
    def test_get_method_get(self, mock_config):
        """Test _get_method returns GET for get types."""
        handler = HttpProxyHandler(mock_config)
        assert handler._get_method("http_get") == "GET"
        assert handler._get_method("httpget") == "GET"
        assert handler._get_method("get") == "GET"
    
    def test_get_method_post(self, mock_config):
        """Test _get_method returns POST for post types."""
        handler = HttpProxyHandler(mock_config)
        assert handler._get_method("http_post") == "POST"
        assert handler._get_method("httppost") == "POST"
        assert handler._get_method("post") == "POST"
    
    def test_construct_url_full_url(self, mock_config):
        """Test _construct_url with full URL."""
        handler = HttpProxyHandler(mock_config)
        url = handler._construct_url("https://example.com/api")
        assert url == "https://example.com/api"
    
    def test_construct_url_relative_path(self, mock_config):
        """Test _construct_url with relative path."""
        handler = HttpProxyHandler(mock_config)
        url = handler._construct_url("/api/v1/test")
        assert url == "http://localhost:8000/api/v1/test"
    
    def test_construct_url_path_without_slash(self, mock_config):
        """Test _construct_url adds leading slash."""
        handler = HttpProxyHandler(mock_config)
        url = handler._construct_url("api/test")
        assert url == "http://localhost:8000/api/test"
    
    @pytest.mark.asyncio
    async def test_close_session(self, mock_config):
        """Test close method closes session when session exists."""
        handler = HttpProxyHandler(mock_config)
        # Create mock session and save reference (handler.close() sets _session to None)
        mock_session = MagicMock()
        mock_session.closed = False
        mock_session.close = AsyncMock()
        handler._session = mock_session
        
        await handler.close()
        # Assert on our saved reference since handler._session may be None now
        mock_session.close.assert_called_once()


class TestHttpGetPostHandlers:
    """Test HttpGetHandler and HttpPostHandler."""
    
    @pytest.mark.asyncio
    async def test_http_get_handler_forces_get(self, mock_config):
        """Test HttpGetHandler forces GET method."""
        handler = HttpGetHandler(mock_config)
        
        with patch.object(HttpProxyHandler, 'handle', new_callable=AsyncMock) as mock_handle:
            mock_handle.return_value = {"status": "ok"}
            
            request = {"request_type": "http_post", "endpoint": "/test"}
            await handler.handle(request)
            
            # Verify request_type was changed to http_get
            called_request = mock_handle.call_args[0][0]
            assert called_request["request_type"] == "http_get"
    
    @pytest.mark.asyncio
    async def test_http_post_handler_forces_post(self, mock_config):
        """Test HttpPostHandler forces POST method."""
        handler = HttpPostHandler(mock_config)
        
        with patch.object(HttpProxyHandler, 'handle', new_callable=AsyncMock) as mock_handle:
            mock_handle.return_value = {"status": "ok"}
            
            request = {"request_type": "http_get", "endpoint": "/test"}
            await handler.handle(request)
            
            # Verify request_type was changed to http_post
            called_request = mock_handle.call_args[0][0]
            assert called_request["request_type"] == "http_post"


# =============================================================================
# Test AIIPCService (main.py)
# =============================================================================

class TestAIIPCServiceStart:
    """Test AIIPCService start method."""
    
    @pytest.mark.asyncio
    async def test_start_db_connection_error(self):
        """Test start handles database connection error."""
        from main import AIIPCService
        from database import ConnectionError as DBConnectionError
        
        service = AIIPCService()
        
        with patch('main.Config.load') as mock_load, \
             patch('main.configure_logging'), \
             patch('main.Database') as mock_db_class:
            
            mock_config = MagicMock()
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.validate.return_value = []
            mock_config.to_dict.return_value = {}
            mock_load.return_value = mock_config
            
            mock_db = MagicMock()
            mock_db.connect = AsyncMock(side_effect=DBConnectionError("Connection failed"))
            mock_db_class.return_value = mock_db
            
            with pytest.raises(DBConnectionError):
                await service.start()


class TestAIIPCServiceRun:
    """Test AIIPCService run method."""
    
    @pytest.mark.asyncio
    async def test_run_not_started(self):
        """Test run raises error if not started."""
        from main import AIIPCService
        
        service = AIIPCService()
        
        with pytest.raises(RuntimeError):
            await service.run()
    
    @pytest.mark.asyncio
    async def test_run_with_cleanup(self):
        """Test run executes cleanup tasks."""
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service.config = MagicMock()
        service.config.polling = MagicMock()
        service.config.polling.interval_ms = 100
        
        service.processor = MagicMock()
        service.processor.process_batch = AsyncMock(return_value=0)
        service.processor.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0
        })
        service.processor.cleanup_expired = AsyncMock(return_value=0)
        service.processor.cleanup_stuck = AsyncMock(return_value=0)
        
        # Stop after first iteration
        async def stop_after_one():
            await asyncio.sleep(0.05)
            service._running = False
            service._shutdown_event.set()
        
        asyncio.create_task(stop_after_one())
        await service.run()
        
        # Verify process_batch was called
        service.processor.process_batch.assert_called()


class TestAIIPCServiceStop:
    """Test AIIPCService stop method."""
    
    @pytest.mark.asyncio
    async def test_stop_no_pending_tasks(self):
        """Test stop with no pending tasks."""
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service.processor = MagicMock()
        service.processor.stop = MagicMock()
        service.processor.get_stats = MagicMock(return_value={
            "requests_processed": 10,
            "requests_failed": 2,
            "requests_expired": 0,
            "batches_processed": 5,
            "uptime_seconds": 100,
            "requests_per_second": 0.1
        })
        service.db = MagicMock()
        service.db.disconnect = AsyncMock()
        
        await service.stop()
        
        assert service._running == False
        service.processor.stop.assert_called_once()
        service.db.disconnect.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_stop_with_pending_tasks(self):
        """Test stop waits for pending tasks."""
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service.processor = MagicMock()
        service.processor.stop = MagicMock()
        service.processor.get_stats = MagicMock(return_value={
            "requests_processed": 10,
            "requests_failed": 2,
            "requests_expired": 0,
            "batches_processed": 5,
            "uptime_seconds": 100,
            "requests_per_second": 0.1
        })
        service.db = MagicMock()
        service.db.disconnect = AsyncMock()
        
        # Create a pending task
        async def slow_task():
            await asyncio.sleep(0.1)
            return "done"
        
        task = asyncio.create_task(slow_task())
        service._pending_tasks.add(task)
        
        await service.stop(timeout_seconds=5.0)
        
        assert service._running == False
    
    @pytest.mark.asyncio
    async def test_stop_with_task_timeout(self):
        """Test stop cancels tasks that don't complete in time."""
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service.processor = MagicMock()
        service.processor.stop = MagicMock()
        service.processor.get_stats = MagicMock(return_value={
            "requests_processed": 10,
            "requests_failed": 2,
            "requests_expired": 0,
            "batches_processed": 5,
            "uptime_seconds": 100,
            "requests_per_second": 0.1
        })
        service.db = MagicMock()
        service.db.disconnect = AsyncMock()
        
        # Create a slow task that won't complete in time
        async def very_slow_task():
            await asyncio.sleep(100)  # Very long
            return "done"
        
        task = asyncio.create_task(very_slow_task())
        service._pending_tasks.add(task)
        
        await service.stop(timeout_seconds=0.1)  # Short timeout
        
        assert service._running == False
    
    def test_request_shutdown(self):
        """Test request_shutdown sets event."""
        from main import AIIPCService
        
        service = AIIPCService()
        service.request_shutdown()
        assert service._shutdown_event.is_set()


class TestSignalHandlers:
    """Test signal handler setup."""
    
    def test_setup_signal_handlers(self):
        """Test signal handlers are setup correctly."""
        from main import AIIPCService, setup_signal_handlers
        
        service = AIIPCService()
        loop = MagicMock()
        loop.add_signal_handler = MagicMock()
        
        setup_signal_handlers(service, loop)
        
        # Should add handlers for SIGINT and SIGTERM
        assert loop.add_signal_handler.call_count == 2
    
    def test_setup_signal_handlers_windows_fallback(self):
        """Test signal handlers fallback on Windows."""
        from main import AIIPCService, setup_signal_handlers
        
        service = AIIPCService()
        loop = MagicMock()
        loop.add_signal_handler = MagicMock(side_effect=NotImplementedError)
        
        with patch('signal.signal') as mock_signal:
            setup_signal_handlers(service, loop)
            # On Windows, falls back to signal.signal
            assert mock_signal.call_count == 2


class TestMainFunction:
    """Test main function."""
    
    @pytest.mark.asyncio
    async def test_main_keyboard_interrupt(self):
        """Test main handles KeyboardInterrupt."""
        from main import main, AIIPCService
        
        with patch.object(AIIPCService, 'start', new_callable=AsyncMock) as mock_start, \
             patch.object(AIIPCService, 'run', new_callable=AsyncMock) as mock_run, \
             patch.object(AIIPCService, 'stop', new_callable=AsyncMock) as mock_stop, \
             patch('main.setup_signal_handlers'):
            
            mock_start.return_value = None
            mock_run.side_effect = KeyboardInterrupt()
            mock_stop.return_value = None
            
            exit_code = await main()
            assert exit_code == 0
            mock_stop.assert_called()
    
    @pytest.mark.asyncio
    async def test_main_connection_error(self):
        """Test main handles ConnectionError."""
        from main import main, AIIPCService
        from database import ConnectionError as DBConnectionError
        
        with patch.object(AIIPCService, 'start', new_callable=AsyncMock) as mock_start, \
             patch('main.setup_signal_handlers'):
            
            mock_start.side_effect = DBConnectionError("DB error")
            
            exit_code = await main()
            assert exit_code == 1
    
    @pytest.mark.asyncio
    async def test_main_unexpected_error(self):
        """Test main handles unexpected errors."""
        from main import main, AIIPCService
        
        with patch.object(AIIPCService, 'start', new_callable=AsyncMock) as mock_start, \
             patch.object(AIIPCService, 'stop', new_callable=AsyncMock) as mock_stop, \
             patch('main.setup_signal_handlers'):
            
            mock_start.side_effect = Exception("Unexpected error")
            mock_stop.return_value = None
            
            exit_code = await main()
            assert exit_code == 1


class TestRunEntryPoint:
    """Test run() entry point."""
    
    def test_run_with_args(self):
        """Test run parses command line arguments."""
        from main import run
        
        with patch('sys.argv', ['main.py', '-c', '/path/to/config.yaml']), \
             patch('asyncio.run') as mock_asyncio_run, \
             patch('sys.exit') as mock_exit:
            
            mock_asyncio_run.return_value = 0
            
            run()
            
            mock_asyncio_run.assert_called_once()
            mock_exit.assert_called_with(0)


# =============================================================================
# Test Config Edge Cases (config.py)
# =============================================================================

class TestConfigEdgeCases:
    """Test config edge cases."""
    
    def test_config_validate_warnings(self):
        """Test config validate returns warnings."""
        config = Config(
            database=DatabaseConfig(password=""),
            polling=PollingConfig(worker_count=10),
            ai_service=AIServiceConfig(base_url="http://localhost:8000"),
        )
        
        warnings = config.validate()
        assert len(warnings) > 0  # Should have warnings
    
    def test_database_config_invalid_pool_size(self):
        """Test DatabaseConfig with invalid pool_size."""
        with pytest.raises(ValueError):
            DatabaseConfig(pool_size=0)
    
    def test_database_config_invalid_port(self):
        """Test DatabaseConfig with invalid port."""
        with pytest.raises(ValueError):
            DatabaseConfig(port=0)
        with pytest.raises(ValueError):
            DatabaseConfig(port=70000)
    
    def test_polling_config_invalid_interval(self):
        """Test PollingConfig with invalid interval."""
        with pytest.raises(ValueError):
            PollingConfig(interval_ms=5)
    
    def test_polling_config_invalid_batch_size(self):
        """Test PollingConfig with invalid batch_size."""
        with pytest.raises(ValueError):
            PollingConfig(batch_size=0)
    
    def test_polling_config_invalid_worker_count(self):
        """Test PollingConfig with invalid worker_count."""
        with pytest.raises(ValueError):
            PollingConfig(worker_count=0)
    
    def test_ai_service_config_invalid_timeout(self):
        """Test AIServiceConfig with invalid timeout."""
        with pytest.raises(ValueError):
            AIServiceConfig(timeout_seconds=0)
    
    def test_ai_service_config_empty_url(self):
        """Test AIServiceConfig with empty URL."""
        with pytest.raises(ValueError):
            AIServiceConfig(base_url="")
    
    def test_logging_config_invalid_level(self):
        """Test LoggingConfig with invalid level."""
        with pytest.raises(ValueError):
            LoggingConfig(level="INVALID")
    
    def test_logging_config_invalid_format(self):
        """Test LoggingConfig with invalid format."""
        with pytest.raises(ValueError):
            LoggingConfig(format="invalid")
    
    def test_security_config_invalid_auth_method(self):
        """Test SecurityConfig with invalid auth method."""
        with pytest.raises(ValueError):
            SecurityConfig(auth_method="invalid")
    
    def test_security_config_validate_api_key(self):
        """Test SecurityConfig validate_api_key."""
        config = SecurityConfig(
            auth_enabled=True,
            auth_method="api_key",
            api_key="secret123"
        )
        assert config.validate_api_key("secret123") == True
        assert config.validate_api_key("wrong") == False
    
    def test_security_config_validate_api_key_disabled(self):
        """Test validate_api_key when auth disabled."""
        config = SecurityConfig(auth_enabled=False)
        assert config.validate_api_key("any") == True
    
    def test_security_config_is_npc_blocked(self):
        """Test is_npc_blocked method."""
        config = SecurityConfig(blocked_npcs=["blocked_npc"])
        assert config.is_npc_blocked("blocked_npc") == True
        assert config.is_npc_blocked("allowed_npc") == False
    
    def test_security_config_is_request_type_allowed(self):
        """Test is_request_type_allowed method."""
        config = SecurityConfig(allowed_request_types=["type1", "type2"])
        assert config.is_request_type_allowed("type1") == True
        assert config.is_request_type_allowed("type3") == False
    
    def test_security_config_is_request_type_allowed_empty(self):
        """Test is_request_type_allowed with empty list (all allowed)."""
        config = SecurityConfig(allowed_request_types=[])
        assert config.is_request_type_allowed("any_type") == True
    
    def test_config_to_dict(self):
        """Test Config to_dict masks sensitive data."""
        config = Config(
            database=DatabaseConfig(password="secret"),
            security=SecurityConfig(api_key="api_secret")
        )
        
        result = config.to_dict()
        assert result["database"]["password"] == "***"
        assert result["security"]["api_key"] == "***"
    
    def test_ai_service_url_properties(self):
        """Test AIServiceConfig URL properties."""
        config = AIServiceConfig(base_url="http://localhost:8000/")
        
        assert config.dialogue_url == "http://localhost:8000/dialogue"
        assert config.decision_url == "http://localhost:8000/decision"
        assert config.emotion_url == "http://localhost:8000/emotion"
        assert config.memory_url == "http://localhost:8000/memory"
        assert config.health_url == "http://localhost:8000/health"
        assert config.async_url == "http://localhost:8000/async"
        
        # Test aliases
        assert config.dialogue_endpoint == config.dialogue_url
        assert config.decision_endpoint == config.decision_url
        assert config.emotion_endpoint == config.emotion_url
        assert config.memory_endpoint == config.memory_url
        assert config.health_endpoint == config.health_url
        assert config.async_endpoint == config.async_url


# =============================================================================
# Test Config Loading (config.py lines 290-311)
# =============================================================================

class TestConfigLoading:
    """Test config loading functionality."""
    
    def test_load_from_nonexistent_file(self, tmp_path):
        """Test loading from nonexistent file uses defaults."""
        config = Config.load(tmp_path / "nonexistent.yaml")
        assert config is not None
        assert isinstance(config.database, DatabaseConfig)
    
    def test_load_from_yaml_file(self, tmp_path):
        """Test loading from valid YAML file."""
        yaml_content = """
database:
  host: testhost
  port: 3307
polling:
  interval_ms: 200
"""
        config_file = tmp_path / "test_config.yaml"
        config_file.write_text(yaml_content)
        
        config = Config.load(config_file)
        assert config.database.host == "testhost"
        assert config.database.port == 3307
        assert config.polling.interval_ms == 200
    
    def test_load_from_invalid_yaml(self, tmp_path):
        """Test loading from invalid YAML raises error."""
        config_file = tmp_path / "invalid.yaml"
        config_file.write_text("invalid: yaml: content: :")
        
        with pytest.raises(ValueError):
            Config.load(config_file)
    
    def test_load_with_env_override(self, tmp_path, monkeypatch):
        """Test environment variables override YAML values."""
        yaml_content = """
database:
  host: yamlhost
"""
        config_file = tmp_path / "config.yaml"
        config_file.write_text(yaml_content)
        
        monkeypatch.setenv("DB_HOST", "envhost")
        
        config = Config.load(config_file)
        assert config.database.host == "envhost"  # ENV overrides YAML
    
    def test_parse_bool(self):
        """Test _parse_bool method."""
        assert Config._parse_bool("true") == True
        assert Config._parse_bool("True") == True
        assert Config._parse_bool("1") == True
        assert Config._parse_bool("yes") == True
        assert Config._parse_bool("on") == True
        assert Config._parse_bool("false") == False
        assert Config._parse_bool("0") == False
        assert Config._parse_bool("no") == False


# =============================================================================
# Test Configure Logging (main.py lines 52-103)
# =============================================================================

class TestConfigureLogging:
    """Test configure_logging function."""
    
    def test_configure_logging_json(self, mock_config):
        """Test configuring JSON logging."""
        from main import configure_logging
        
        mock_config.logging.format = "json"
        mock_config.logging.level = "DEBUG"
        
        with patch('structlog.configure') as mock_structlog, \
             patch('logging.basicConfig') as mock_basic:
            configure_logging(mock_config)
            mock_structlog.assert_called_once()
            mock_basic.assert_called_once()
    
    def test_configure_logging_text(self, mock_config):
        """Test configuring text logging."""
        from main import configure_logging
        
        mock_config.logging.format = "text"
        mock_config.logging.level = "INFO"
        
        with patch('logging.basicConfig') as mock_basic:
            configure_logging(mock_config)
            mock_basic.assert_called_once()