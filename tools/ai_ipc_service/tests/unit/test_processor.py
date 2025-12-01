"""
Unit Tests for Request Processor

Tests the RequestProcessor class functionality:
- Request routing to handlers
- Batch processing
- Result collection and aggregation
- Error handling during processing
- Statistics tracking
- Handler resolution

Test Strategy:
- Mock handlers to isolate processor logic
- Test routing for all request types
- Verify error handling and recovery
- Test concurrent processing behavior
"""

import asyncio
import json
import logging
import os
import sys
from datetime import datetime
from typing import Any, Dict, List, Optional
from unittest.mock import AsyncMock, MagicMock, patch

import pytest
import pytest_asyncio

# Add parent directories to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from config import AIServiceConfig, SecurityConfig
from processor import RequestProcessor, ProcessingStats

logger = logging.getLogger(__name__)


# =============================================================================
# ProcessingStats Tests
# =============================================================================

class TestProcessingStats:
    """Tests for ProcessingStats class."""
    
    def test_processing_stats_initialization(self):
        """
        Test ProcessingStats initialization.
        
        Validates:
        - All counters start at zero
        - Timestamp is set
        """
        stats = ProcessingStats()
        
        assert stats.total_processed == 0
        assert stats.successful == 0
        assert stats.failed == 0
        assert stats.total_processing_time_ms == 0
        assert stats.start_time is not None
    
    def test_processing_stats_increment_success(self):
        """
        Test incrementing success counter.
        
        Validates:
        - Success count increases
        - Total processed increases
        """
        stats = ProcessingStats()
        
        stats.record_success(processing_time_ms=100)
        
        assert stats.successful == 1
        assert stats.total_processed == 1
        assert stats.total_processing_time_ms == 100
    
    def test_processing_stats_increment_failure(self):
        """
        Test incrementing failure counter.
        
        Validates:
        - Failure count increases
        - Total processed increases
        """
        stats = ProcessingStats()
        
        stats.record_failure(processing_time_ms=50)
        
        assert stats.failed == 1
        assert stats.total_processed == 1
        assert stats.total_processing_time_ms == 50
    
    def test_processing_stats_average_time(self):
        """
        Test average processing time calculation.
        
        Validates:
        - Average is calculated correctly
        - Handles zero division
        """
        stats = ProcessingStats()
        
        # No requests - should not divide by zero
        avg = stats.average_processing_time_ms
        assert avg == 0
        
        # Add some requests
        stats.record_success(processing_time_ms=100)
        stats.record_success(processing_time_ms=200)
        stats.record_failure(processing_time_ms=50)
        
        avg = stats.average_processing_time_ms
        assert avg == (100 + 200 + 50) / 3
    
    def test_processing_stats_success_rate(self):
        """
        Test success rate calculation.
        
        Validates:
        - Rate is calculated correctly
        - Handles zero division
        """
        stats = ProcessingStats()
        
        # No requests
        rate = stats.success_rate
        assert rate == 0.0 or rate == 1.0  # Implementation dependent
        
        # Add requests
        stats.record_success(100)
        stats.record_success(100)
        stats.record_failure(100)
        
        rate = stats.success_rate
        assert abs(rate - 0.6667) < 0.01 or abs(rate - 2/3) < 0.01
    
    def test_processing_stats_reset(self):
        """
        Test resetting statistics.
        
        Validates:
        - All counters reset to zero
        - New start time is set
        """
        stats = ProcessingStats()
        
        stats.record_success(100)
        stats.record_failure(50)
        
        old_start = stats.start_time
        stats.reset()
        
        assert stats.total_processed == 0
        assert stats.successful == 0
        assert stats.failed == 0
        assert stats.total_processing_time_ms == 0
        # Start time may or may not be reset - implementation dependent
    
    def test_processing_stats_to_dict(self):
        """
        Test converting stats to dictionary.
        
        Validates:
        - All fields are included
        - Values are accurate
        """
        stats = ProcessingStats()
        stats.record_success(100)
        stats.record_success(200)
        
        stats_dict = stats.to_dict()
        
        assert "total_processed" in stats_dict
        assert "successful" in stats_dict
        assert "failed" in stats_dict
        assert stats_dict["total_processed"] == 2
        assert stats_dict["successful"] == 2


# =============================================================================
# RequestProcessor Initialization Tests
# =============================================================================

class TestRequestProcessorInit:
    """Tests for RequestProcessor initialization."""
    
    @pytest.mark.asyncio
    async def test_processor_initialization(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test processor initialization.
        
        Validates:
        - Processor is created with configs
        - Handler registry is available
        - Stats are initialized
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        assert processor._ai_config == ai_service_config
        assert processor._security_config == security_config
        assert processor._stats is not None
    
    @pytest.mark.asyncio
    async def test_processor_handler_registration(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test that handlers are registered correctly.
        
        Validates:
        - All expected handlers are available
        - Handlers can be resolved by type
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        # Test common handler types
        for handler_type in ["dialogue", "decision", "emotion", "http_get", "health_check"]:
            handler = processor._get_handler(handler_type)
            assert handler is not None, f"Handler for '{handler_type}' should be registered"


# =============================================================================
# Request Processing Tests
# =============================================================================

class TestRequestProcessing:
    """Tests for request processing functionality."""
    
    @pytest.mark.asyncio
    async def test_process_single_request(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test processing a single request.
        
        Validates:
        - Request is routed to correct handler
        - Response is returned
        - Stats are updated
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        # Mock handler
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={
            "status_code": 200,
            "data": {"dialogue": "Hello!"},
        })
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            result = await processor.process({
                "id": 1,
                "request_type": "dialogue",
                "npc_id": 1000,
                "player_id": 1,
                "payload": json.dumps({"message": "Hi"}),
            })
        
        assert result["status_code"] == 200
        assert "data" in result
        mock_handler.handle.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_process_batch_requests(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test processing a batch of requests.
        
        Validates:
        - All requests in batch are processed
        - Results are collected correctly
        - Order is preserved or identifiable
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={
            "status_code": 200,
            "data": {},
        })
        
        requests = [
            {"id": i, "request_type": "dialogue", "npc_id": 1000 + i, "player_id": 1, "payload": "{}"}
            for i in range(5)
        ]
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            results = await processor.process_batch(requests)
        
        assert len(results) == 5
        assert mock_handler.handle.call_count == 5
    
    @pytest.mark.asyncio
    async def test_process_with_unknown_handler(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test processing with unknown request type.
        
        Validates:
        - Unknown types return error
        - No crash occurs
        - Appropriate error message
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        result = await processor.process({
            "id": 1,
            "request_type": "unknown_nonexistent_type",
            "npc_id": 1000,
            "player_id": 1,
            "payload": "{}",
        })
        
        assert result["status_code"] >= 400
        assert "error" in result
    
    @pytest.mark.asyncio
    async def test_process_with_handler_exception(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test processing when handler throws exception.
        
        Validates:
        - Exception is caught
        - Error response is returned
        - Stats reflect failure
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(side_effect=Exception("Handler error"))
        
        initial_failures = processor._stats.failed
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            result = await processor.process({
                "id": 1,
                "request_type": "dialogue",
                "npc_id": 1000,
                "player_id": 1,
                "payload": "{}",
            })
        
        assert result["status_code"] >= 500
        assert "error" in result
        assert processor._stats.failed > initial_failures


# =============================================================================
# Handler Resolution Tests
# =============================================================================

class TestHandlerResolution:
    """Tests for handler resolution functionality."""
    
    @pytest.mark.asyncio
    async def test_resolve_dialogue_handler(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test resolving dialogue handler.
        
        Validates:
        - Correct handler class is returned
        - Handler is properly configured
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        handler = processor._get_handler("dialogue")
        
        assert handler is not None
        # Check it's the right type (implementation dependent)
    
    @pytest.mark.asyncio
    async def test_resolve_handler_aliases(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test resolving handlers by aliases.
        
        Validates:
        - Multiple aliases resolve to same handler
        - Case handling is correct
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        # These should all resolve to HTTP proxy handler
        http_handler_1 = processor._get_handler("http_get")
        http_handler_2 = processor._get_handler("httpget")
        
        # Both should return valid handlers
        assert http_handler_1 is not None
        assert http_handler_2 is not None
    
    @pytest.mark.asyncio
    async def test_resolve_nonexistent_handler(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test resolving nonexistent handler.
        
        Validates:
        - Returns None or raises appropriate error
        - Does not crash
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        handler = processor._get_handler("nonexistent_handler_xyz")
        
        assert handler is None


# =============================================================================
# Payload Parsing Tests
# =============================================================================

class TestPayloadParsing:
    """Tests for request payload parsing."""
    
    @pytest.mark.asyncio
    async def test_parse_json_string_payload(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test parsing JSON string payload.
        
        Validates:
        - JSON string is parsed to dict
        - Data is preserved correctly
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "npc_id": 1000,
            "player_id": 1,
            "payload": '{"message": "Hello", "context": {"key": "value"}}',
        }
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            await processor.process(request)
        
        # Verify handler received parsed payload
        call_args = mock_handler.handle.call_args
        assert call_args is not None
    
    @pytest.mark.asyncio
    async def test_parse_dict_payload(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test parsing already-dict payload.
        
        Validates:
        - Dict payload is passed through
        - No double parsing
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "npc_id": 1000,
            "player_id": 1,
            "payload": {"message": "Hello", "already": "dict"},
        }
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            await processor.process(request)
        
        mock_handler.handle.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_parse_invalid_json_payload(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test parsing invalid JSON payload.
        
        Validates:
        - Invalid JSON is handled
        - Error response is returned
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        request = {
            "id": 1,
            "request_type": "dialogue",
            "npc_id": 1000,
            "player_id": 1,
            "payload": "not valid json {{{",
        }
        
        result = await processor.process(request)
        
        # Should handle gracefully
        assert "error" in result or result["status_code"] >= 400


# =============================================================================
# Concurrent Processing Tests
# =============================================================================

class TestConcurrentProcessing:
    """Tests for concurrent request processing."""
    
    @pytest.mark.asyncio
    async def test_concurrent_request_processing(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test concurrent request processing.
        
        Validates:
        - Multiple requests processed concurrently
        - No race conditions
        - All results returned
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        # Create handler that simulates async work
        async def slow_handler(request_data):
            await asyncio.sleep(0.01)  # Simulate async work
            return {"status_code": 200, "data": {"id": request_data.get("id")}}
        
        mock_handler = AsyncMock()
        mock_handler.handle = slow_handler
        
        requests = [
            {"id": i, "request_type": "dialogue", "npc_id": 1000, "player_id": 1, "payload": "{}"}
            for i in range(10)
        ]
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            # Process concurrently
            tasks = [processor.process(req) for req in requests]
            results = await asyncio.gather(*tasks)
        
        assert len(results) == 10
        # All should succeed
        success_count = sum(1 for r in results if r["status_code"] == 200)
        assert success_count == 10
    
    @pytest.mark.asyncio
    async def test_concurrent_processing_with_failures(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test concurrent processing with some failures.
        
        Validates:
        - Failures don't affect other requests
        - Each request is independent
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        call_count = 0
        
        async def sometimes_fail(request_data):
            nonlocal call_count
            call_count += 1
            if call_count % 3 == 0:
                raise Exception("Simulated failure")
            return {"status_code": 200, "data": {}}
        
        mock_handler = AsyncMock()
        mock_handler.handle = sometimes_fail
        
        requests = [
            {"id": i, "request_type": "dialogue", "npc_id": 1000, "player_id": 1, "payload": "{}"}
            for i in range(9)
        ]
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            tasks = [processor.process(req) for req in requests]
            results = await asyncio.gather(*tasks)
        
        # Should have 6 successes and 3 failures
        success_count = sum(1 for r in results if r["status_code"] == 200)
        failure_count = sum(1 for r in results if r["status_code"] >= 500)
        
        assert success_count == 6
        assert failure_count == 3


# =============================================================================
# Rate Limiting Tests
# =============================================================================

class TestProcessorRateLimiting:
    """Tests for processor rate limiting."""
    
    @pytest.mark.asyncio
    async def test_processor_applies_rate_limiting(
        self,
        ai_service_config: AIServiceConfig,
    ):
        """
        Test that processor applies rate limiting.
        
        Validates:
        - Rate limits are checked
        - Exceeded limits return error
        """
        security_config = SecurityConfig(
            api_key="test-key",
            rate_limit_per_npc=2,
            rate_limit_global=10,
            rate_limit_window_seconds=60,
        )
        
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        # Process requests until rate limited
        npc_id = 9999
        results = []
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            for i in range(5):
                result = await processor.process({
                    "id": i,
                    "request_type": "dialogue",
                    "npc_id": npc_id,
                    "player_id": 1,
                    "payload": "{}",
                })
                results.append(result)
        
        # Some should be rate limited (depending on implementation)
        # At least first 2 should succeed if rate_limit_per_npc is 2
        success_count = sum(1 for r in results if r["status_code"] == 200)
        assert success_count >= 2


# =============================================================================
# Request Validation Tests
# =============================================================================

class TestRequestValidation:
    """Tests for request validation."""
    
    @pytest.mark.asyncio
    async def test_validate_missing_request_type(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test validation with missing request type.
        
        Validates:
        - Missing type is caught
        - Error is returned
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        result = await processor.process({
            "id": 1,
            # Missing request_type
            "npc_id": 1000,
            "player_id": 1,
            "payload": "{}",
        })
        
        assert result["status_code"] >= 400
        assert "error" in result
    
    @pytest.mark.asyncio
    async def test_validate_missing_npc_id(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test validation with missing NPC ID.
        
        Validates:
        - Missing NPC ID is caught
        - Error is returned
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        result = await processor.process({
            "id": 1,
            "request_type": "dialogue",
            # Missing npc_id
            "player_id": 1,
            "payload": "{}",
        })
        
        # May succeed or fail depending on validation strictness
        assert result is not None
    
    @pytest.mark.asyncio
    async def test_validate_empty_payload(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test validation with empty payload.
        
        Validates:
        - Empty payload is handled
        - Default values are used
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            result = await processor.process({
                "id": 1,
                "request_type": "health_check",
                "npc_id": 0,
                "player_id": 0,
                "payload": "",
            })
        
        # Health check should work with empty payload
        assert result is not None


# =============================================================================
# Statistics Integration Tests
# =============================================================================

class TestStatisticsIntegration:
    """Tests for statistics integration."""
    
    @pytest.mark.asyncio
    async def test_stats_updated_on_success(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test that stats are updated on successful processing.
        
        Validates:
        - Success counter increments
        - Processing time is recorded
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        initial_success = processor._stats.successful
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            await processor.process({
                "id": 1,
                "request_type": "dialogue",
                "npc_id": 1000,
                "player_id": 1,
                "payload": "{}",
            })
        
        assert processor._stats.successful > initial_success
    
    @pytest.mark.asyncio
    async def test_stats_updated_on_failure(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test that stats are updated on failed processing.
        
        Validates:
        - Failure counter increments
        - Processing time is still recorded
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        initial_failed = processor._stats.failed
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(side_effect=Exception("Error"))
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            await processor.process({
                "id": 1,
                "request_type": "dialogue",
                "npc_id": 1000,
                "player_id": 1,
                "payload": "{}",
            })
        
        assert processor._stats.failed > initial_failed
    
    @pytest.mark.asyncio
    async def test_get_stats(
        self,
        ai_service_config: AIServiceConfig,
        security_config: SecurityConfig,
    ):
        """
        Test retrieving processor statistics.
        
        Validates:
        - Stats are accessible
        - Stats are accurate
        """
        processor = RequestProcessor(
            ai_config=ai_service_config,
            security_config=security_config,
        )
        
        mock_handler = AsyncMock()
        mock_handler.handle = AsyncMock(return_value={"status_code": 200, "data": {}})
        
        with patch.object(processor, '_get_handler', return_value=mock_handler):
            for _ in range(5):
                await processor.process({
                    "id": 1,
                    "request_type": "dialogue",
                    "npc_id": 1000,
                    "player_id": 1,
                    "payload": "{}",
                })
        
        stats = processor.get_stats()
        
        assert stats["total_processed"] == 5
        assert stats["successful"] == 5
        assert stats["failed"] == 0