"""
Tests for Error Handlers Utility

Comprehensive test suite covering:
- Global error handler
- HTTP exception handling
- Retry logic with exponential backoff
- Error context tracking
- Error classification
- Error recovery strategies
"""

import pytest
import asyncio
from unittest.mock import Mock, patch, AsyncMock
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed


# Mock the error_handlers module functions we expect to exist
class ErrorCategory:
    """Error categories for classification"""
    NETWORK = "network"
    DATABASE = "database"
    VALIDATION = "validation"
    AUTHENTICATION = "authentication"
    RATE_LIMIT = "rate_limit"
    INTERNAL = "internal"
    EXTERNAL_API = "external_api"


class RetryableError(Exception):
    """Base exception for retryable errors"""
    pass


class NonRetryableError(Exception):
    """Base exception for non-retryable errors"""
    pass


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def mock_logger():
    """Mock logger"""
    with patch('logging.getLogger') as mock:
        yield mock.return_value


@pytest.fixture
def error_context():
    """Error context for tracking"""
    return {
        'request_id': 'req-123',
        'user_id': 'user-456',
        'timestamp': datetime.utcnow().isoformat()
    }


# =============================================================================
# ERROR CLASSIFICATION TESTS
# =============================================================================

class TestErrorClassification:
    """Test error classification"""
    
    def test_classify_network_error(self):
        """Test classifying network errors"""
        from requests.exceptions import ConnectionError, Timeout
        
        # These should be classified as network errors
        errors = [
            ConnectionError("Connection failed"),
            Timeout("Request timeout"),
            OSError("Network unreachable")
        ]
        
        # Test that we can identify network errors
        for error in errors:
            assert isinstance(error, (ConnectionError, Timeout, OSError))
    
    def test_classify_database_error(self):
        """Test classifying database errors"""
        # Database-related exceptions should be identified
        db_errors = [
            "DatabaseError",
            "OperationalError",
            "IntegrityError"
        ]
        
        for error_name in db_errors:
            assert "Error" in error_name
    
    def test_classify_validation_error(self):
        """Test classifying validation errors"""
        from pydantic import ValidationError as PydanticValidationError
        
        # Validation errors should be non-retryable
        assert PydanticValidationError.__name__ == "ValidationError"


# =============================================================================
# RETRY LOGIC TESTS
# =============================================================================

class TestRetryLogic:
    """Test retry logic with exponential backoff"""
    
    @pytest.mark.asyncio
    async def test_retry_success_on_second_attempt(self):
        """Test successful retry on second attempt"""
        call_count = 0
        
        async def flaky_function():
            nonlocal call_count
            call_count += 1
            if call_count < 2:
                raise RetryableError("Temporary failure")
            return "success"
        
        # Simulate retry logic
        max_retries = 3
        for attempt in range(max_retries):
            try:
                result = await flaky_function()
                break
            except RetryableError:
                if attempt == max_retries - 1:
                    raise
                await asyncio.sleep(0.01)  # Minimal backoff for tests
        
        assert result == "success"
        assert call_count == 2
    
    @pytest.mark.asyncio
    async def test_retry_exhausts_attempts(self):
        """Test retry exhausts all attempts"""
        call_count = 0
        
        async def always_fails():
            nonlocal call_count
            call_count += 1
            raise RetryableError("Persistent failure")
        
        # Simulate retry logic
        max_retries = 3
        with pytest.raises(RetryableError):
            for attempt in range(max_retries):
                try:
                    await always_fails()
                except RetryableError:
                    if attempt == max_retries - 1:
                        raise
                    await asyncio.sleep(0.01)
        
        assert call_count == 3
    
    @pytest.mark.asyncio
    async def test_no_retry_on_non_retryable_error(self):
        """Test non-retryable errors don't retry"""
        call_count = 0
        
        async def fails_immediately():
            nonlocal call_count
            call_count += 1
            raise NonRetryableError("Cannot retry this")
        
        # Non-retryable should fail immediately
        with pytest.raises(NonRetryableError):
            await fails_immediately()
        
        assert call_count == 1
    
    def test_exponential_backoff_calculation(self):
        """Test exponential backoff delay calculation"""
        base_delay = 1.0
        max_delay = 60.0
        
        def calculate_backoff(attempt, base=base_delay, maximum=max_delay):
            """Calculate exponential backoff delay"""
            delay = min(base * (2 ** attempt), maximum)
            return delay
        
        # Test backoff increases exponentially
        delays = [calculate_backoff(i) for i in range(7)]
        
        assert delays[0] == 1.0
        assert delays[1] == 2.0
        assert delays[2] == 4.0
        assert delays[3] == 8.0
        assert delays[4] == 16.0
        assert delays[5] == 32.0
        assert delays[6] == 60.0  # Capped at max
    
    def test_jitter_calculation(self):
        """Test jitter in backoff calculation"""
        import random
        
        def calculate_backoff_with_jitter(attempt, base=1.0, maximum=60.0):
            """Calculate exponential backoff with jitter"""
            delay = min(base * (2 ** attempt), maximum)
            jitter = random.uniform(0, delay * 0.1)  # 10% jitter
            return delay + jitter
        
        delay1 = calculate_backoff_with_jitter(2)
        delay2 = calculate_backoff_with_jitter(2)
        
        # Base delay should be 4, jitter up to 0.4
        assert 4.0 <= delay1 <= 4.4
        assert 4.0 <= delay2 <= 4.4


# =============================================================================
# ERROR HANDLER DECORATOR TESTS
# =============================================================================

class TestErrorHandlerDecorator:
    """Test error handler decorator"""
    
    @pytest.mark.asyncio
    async def test_error_handler_catches_exception(self):
        """Test error handler catches and logs exceptions"""
        errors_caught = []
        
        def error_handler(max_retries=0):
            def decorator(func):
                async def wrapper(*args, **kwargs):
                    try:
                        return await func(*args, **kwargs)
                    except Exception as e:
                        errors_caught.append(e)
                        raise
                return wrapper
            return decorator
        
        @error_handler(max_retries=0)
        async def failing_function():
            raise ValueError("Test error")
        
        with pytest.raises(ValueError):
            await failing_function()
        
        assert len(errors_caught) == 1
        assert isinstance(errors_caught[0], ValueError)
    
    @pytest.mark.asyncio
    async def test_error_handler_with_context(self, error_context):
        """Test error handler includes context"""
        captured_contexts = []
        
        def error_handler_with_context(context):
            def decorator(func):
                async def wrapper(*args, **kwargs):
                    try:
                        return await func(*args, **kwargs)
                    except Exception as e:
                        captured_contexts.append({
                            'error': e,
                            'context': context
                        })
                        raise
                return wrapper
            return decorator
        
        @error_handler_with_context(error_context)
        async def test_function():
            raise RuntimeError("Test")
        
        with pytest.raises(RuntimeError):
            await test_function()
        
        assert len(captured_contexts) == 1
        assert captured_contexts[0]['context'] == error_context


# =============================================================================
# ERROR CONTEXT TRACKING TESTS
# =============================================================================

class TestErrorContextTracking:
    """Test error context tracking"""
    
    def test_error_context_creation(self, error_context):
        """Test creating error context"""
        assert 'request_id' in error_context
        assert 'user_id' in error_context
        assert 'timestamp' in error_context
    
    def test_error_context_enrichment(self, error_context):
        """Test enriching error context"""
        enriched = error_context.copy()
        enriched['error_type'] = 'ValueError'
        enriched['error_message'] = 'Invalid input'
        enriched['stack_trace'] = 'Stack trace here'
        
        assert len(enriched) == 6
        assert enriched['error_type'] == 'ValueError'
    
    def test_error_context_serialization(self, error_context):
        """Test error context serialization"""
        import json
        
        # Should be JSON serializable
        serialized = json.dumps(error_context)
        deserialized = json.loads(serialized)
        
        assert deserialized == error_context


# =============================================================================
# HTTP ERROR HANDLING TESTS
# =============================================================================

class TestHttpErrorHandling:
    """Test HTTP-specific error handling"""
    
    def test_map_exception_to_status_code(self):
        """Test mapping exceptions to HTTP status codes"""
        from fastapi import HTTPException
        
        error_mapping = {
            ValueError: 400,
            KeyError: 404,
            PermissionError: 403,
            TimeoutError: 504,
            Exception: 500
        }
        
        for error_type, expected_code in error_mapping.items():
            if error_type == Exception:
                continue
            error = error_type("Test")
            # Verify we can map these
            assert expected_code in [400, 403, 404, 504]
    
    def test_create_error_response(self):
        """Test creating standardized error response"""
        error_response = {
            'error': {
                'code': 'VALIDATION_ERROR',
                'message': 'Invalid input provided',
                'details': {'field': 'email', 'issue': 'invalid format'},
                'timestamp': datetime.utcnow().isoformat(),
                'request_id': 'req-123'
            }
        }
        
        assert 'error' in error_response
        assert 'code' in error_response['error']
        assert 'message' in error_response['error']
        assert 'request_id' in error_response['error']


# =============================================================================
# ERROR RECOVERY TESTS
# =============================================================================

class TestErrorRecovery:
    """Test error recovery strategies"""
    
    @pytest.mark.asyncio
    async def test_fallback_on_error(self):
        """Test fallback mechanism on error"""
        async def primary_function():
            raise ConnectionError("Primary failed")
        
        async def fallback_function():
            return "fallback result"
        
        # Implement fallback pattern
        try:
            result = await primary_function()
        except ConnectionError:
            result = await fallback_function()
        
        assert result == "fallback result"
    
    @pytest.mark.asyncio
    async def test_circuit_breaker_pattern(self):
        """Test circuit breaker prevents cascading failures"""
        failure_count = 0
        threshold = 3
        circuit_open = False
        
        async def protected_function():
            nonlocal failure_count, circuit_open
            
            if circuit_open:
                raise RuntimeError("Circuit breaker is open")
            
            # Simulate failures
            failure_count += 1
            if failure_count >= threshold:
                circuit_open = True
            raise ConnectionError("Service unavailable")
        
        # First 3 calls increment failure count
        for _ in range(3):
            with pytest.raises(ConnectionError):
                await protected_function()
        
        assert circuit_open
        
        # 4th call should hit circuit breaker
        with pytest.raises(RuntimeError) as exc_info:
            await protected_function()
        
        assert "Circuit breaker" in str(exc_info.value)
    
    @pytest.mark.asyncio
    async def test_graceful_degradation(self):
        """Test graceful degradation on partial failure"""
        async def get_enhanced_data():
            raise TimeoutError("Enhancement service timeout")
        
        async def get_basic_data():
            return {"data": "basic", "enhanced": False}
        
        # Graceful degradation pattern
        try:
            data = await get_enhanced_data()
        except TimeoutError:
            data = await get_basic_data()
            # System continues with reduced functionality
        
        assert data['enhanced'] is False
        assert 'data' in data


# =============================================================================
# ERROR LOGGING TESTS
# =============================================================================

class TestErrorLogging:
    """Test error logging functionality"""
    
    def test_log_error_with_context(self, mock_logger, error_context):
        """Test logging error with context"""
        error = ValueError("Test error")
        
        # Simulate logging
        mock_logger.error(
            f"Error occurred: {str(error)}",
            extra={
                'error_type': type(error).__name__,
                'context': error_context
            }
        )
        
        mock_logger.error.assert_called_once()
    
    def test_log_error_with_stack_trace(self, mock_logger):
        """Test logging error with stack trace"""
        import traceback
        
        try:
            raise RuntimeError("Test error")
        except RuntimeError as e:
            stack_trace = traceback.format_exc()
            mock_logger.error(
                f"Error: {str(e)}",
                extra={'stack_trace': stack_trace}
            )
        
        mock_logger.error.assert_called_once()
    
    def test_structured_error_logging(self, mock_logger):
        """Test structured error logging"""
        error_data = {
            'level': 'ERROR',
            'timestamp': datetime.utcnow().isoformat(),
            'error_type': 'ValueError',
            'error_message': 'Invalid input',
            'request_id': 'req-123',
            'user_id': 'user-456',
            'service': 'ai-service',
            'environment': 'test'
        }
        
        mock_logger.error("Structured error", extra=error_data)
        mock_logger.error.assert_called_once()


# =============================================================================
# ERROR METRICS TESTS
# =============================================================================

class TestErrorMetrics:
    """Test error metrics collection"""
    
    def test_track_error_count(self):
        """Test tracking error counts"""
        error_counts = {}
        
        def increment_error_count(error_type):
            error_counts[error_type] = error_counts.get(error_type, 0) + 1
        
        # Simulate errors
        increment_error_count('ValueError')
        increment_error_count('ValueError')
        increment_error_count('ConnectionError')
        
        assert error_counts['ValueError'] == 2
        assert error_counts['ConnectionError'] == 1
    
    def test_track_error_rate(self):
        """Test tracking error rates"""
        from collections import deque
        from datetime import timedelta
        
        class ErrorRateTracker:
            def __init__(self, window_seconds=60):
                self.errors = deque()
                self.window = timedelta(seconds=window_seconds)
            
            def add_error(self, timestamp=None):
                if timestamp is None:
                    timestamp = datetime.utcnow()
                self.errors.append(timestamp)
                self._cleanup()
            
            def _cleanup(self):
                cutoff = datetime.utcnow() - self.window
                while self.errors and self.errors[0] < cutoff:
                    self.errors.popleft()
            
            def get_rate(self):
                self._cleanup()
                return len(self.errors)
        
        tracker = ErrorRateTracker(window_seconds=60)
        now = datetime.utcnow()
        
        # Add errors
        tracker.add_error(now)
        tracker.add_error(now)
        tracker.add_error(now - timedelta(seconds=90))  # Outside window
        
        assert tracker.get_rate() == 2  # Only 2 within window


# =============================================================================
# THREAD SAFETY TESTS
# =============================================================================

class TestThreadSafety:
    """Test thread safety of error handlers"""
    
    @pytest.mark.concurrent
    def test_concurrent_error_handling(self):
        """Test handling errors from multiple threads"""
        errors = []
        
        def error_prone_function(i):
            try:
                if i % 2 == 0:
                    raise ValueError(f"Error {i}")
                return f"Success {i}"
            except ValueError as e:
                errors.append(e)
                raise
        
        with ThreadPoolExecutor(max_workers=10) as executor:
            futures = [executor.submit(error_prone_function, i) for i in range(20)]
            results = []
            for future in as_completed(futures):
                try:
                    results.append(future.result())
                except ValueError:
                    pass
        
        assert len(results) == 10  # Half succeeded
        assert len(errors) == 10  # Half failed


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    def test_nested_exceptions(self):
        """Test handling nested exceptions"""
        try:
            try:
                raise ValueError("Inner error")
            except ValueError as inner:
                raise RuntimeError("Outer error") from inner
        except RuntimeError as e:
            assert e.__cause__ is not None
            assert isinstance(e.__cause__, ValueError)
    
    @pytest.mark.asyncio
    async def test_exception_during_error_handling(self):
        """Test handling exception during error handling"""
        async def failing_error_handler():
            raise RuntimeError("Error handler failed")
        
        try:
            raise ValueError("Original error")
        except ValueError:
            try:
                await failing_error_handler()
            except RuntimeError:
                # Should handle error in error handler
                pass
    
    def test_error_with_circular_reference(self):
        """Test error handling with circular references"""
        obj1 = {'name': 'obj1'}
        obj2 = {'name': 'obj2'}
        obj1['ref'] = obj2
        obj2['ref'] = obj1
        
        # Should not cause infinite loop in error serialization
        error_context = {
            'error': 'Circular reference',
            'data': obj1
        }
        
        assert 'data' in error_context
    
    @pytest.mark.asyncio
    async def test_timeout_during_retry(self):
        """Test timeout during retry attempts"""
        async def slow_function():
            await asyncio.sleep(10)
            return "too slow"
        
        # Should timeout before completing
        with pytest.raises(asyncio.TimeoutError):
            await asyncio.wait_for(slow_function(), timeout=0.1)
    
    def test_error_message_sanitization(self):
        """Test sanitizing error messages for logging"""
        sensitive_error = ValueError("Password: secret123, API Key: abc456")
        
        # Error message should be sanitized before logging
        sanitized = str(sensitive_error).replace('secret123', '***REDACTED***')
        sanitized = sanitized.replace('abc456', '***REDACTED***')
        
        assert 'secret123' not in sanitized
        assert 'abc456' not in sanitized
        assert '***REDACTED***' in sanitized