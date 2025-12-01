"""
Tests for Correlation ID Tracking Utility

Comprehensive test suite covering:
- Correlation ID generation
- Context propagation
- Async context management
- Request tracing
- Log integration
- Thread safety
- Performance impact
"""

import pytest
import asyncio
import uuid
from unittest.mock import Mock, patch, AsyncMock
from contextvars import ContextVar
from concurrent.futures import ThreadPoolExecutor, as_completed
import time


# =============================================================================
# MOCK CLASSES
# =============================================================================

# Context variable for correlation ID
correlation_id_var: ContextVar[str] = ContextVar('correlation_id', default=None)


class CorrelationContext:
    """Correlation context manager"""
    
    @staticmethod
    def generate_id() -> str:
        """Generate new correlation ID"""
        return str(uuid.uuid4())
    
    @staticmethod
    def get_current() -> str:
        """Get current correlation ID"""
        return correlation_id_var.get()
    
    @staticmethod
    def set(correlation_id: str):
        """Set correlation ID"""
        return correlation_id_var.set(correlation_id)
    
    @staticmethod
    def clear():
        """Clear correlation ID"""
        correlation_id_var.set(None)


class CorrelationMiddleware:
    """Middleware for managing correlation IDs"""
    
    def __init__(self, header_name='X-Correlation-ID'):
        self.header_name = header_name
    
    async def process_request(self, request):
        """Process incoming request"""
        # Get correlation ID from header or generate new one
        correlation_id = request.headers.get(self.header_name)
        if not correlation_id:
            correlation_id = CorrelationContext.generate_id()
        
        # Set in context
        CorrelationContext.set(correlation_id)
        
        return correlation_id
    
    async def process_response(self, response, correlation_id):
        """Add correlation ID to response"""
        response.headers[self.header_name] = correlation_id
        return response


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def clear_context():
    """Clear context before each test"""
    CorrelationContext.clear()
    yield
    CorrelationContext.clear()


@pytest.fixture
def correlation_id():
    """Generate correlation ID"""
    return CorrelationContext.generate_id()


@pytest.fixture
def mock_request():
    """Mock HTTP request"""
    class MockRequest:
        def __init__(self):
            self.headers = {}
    return MockRequest()


@pytest.fixture
def mock_response():
    """Mock HTTP response"""
    class MockResponse:
        def __init__(self):
            self.headers = {}
    return MockResponse()


# =============================================================================
# ID GENERATION TESTS
# =============================================================================

class TestIdGeneration:
    """Test correlation ID generation"""
    
    def test_generate_id_format(self):
        """Test generated ID is valid UUID"""
        correlation_id = CorrelationContext.generate_id()
        
        assert correlation_id is not None
        assert isinstance(correlation_id, str)
        
        # Should be valid UUID
        uuid.UUID(correlation_id)
    
    def test_generate_unique_ids(self):
        """Test generated IDs are unique"""
        ids = [CorrelationContext.generate_id() for _ in range(100)]
        
        assert len(set(ids)) == 100  # All unique
    
    def test_generate_id_format_consistency(self):
        """Test generated IDs have consistent format"""
        ids = [CorrelationContext.generate_id() for _ in range(10)]
        
        for correlation_id in ids:
            # Should be UUID4 format (36 chars with hyphens)
            assert len(correlation_id) == 36
            assert correlation_id.count('-') == 4


# =============================================================================
# CONTEXT MANAGEMENT TESTS
# =============================================================================

class TestContextManagement:
    """Test context variable management"""
    
    def test_set_and_get_correlation_id(self, clear_context, correlation_id):
        """Test setting and getting correlation ID"""
        CorrelationContext.set(correlation_id)
        
        current = CorrelationContext.get_current()
        assert current == correlation_id
    
    def test_get_without_set(self, clear_context):
        """Test getting correlation ID when not set"""
        current = CorrelationContext.get_current()
        assert current is None
    
    def test_clear_correlation_id(self, clear_context, correlation_id):
        """Test clearing correlation ID"""
        CorrelationContext.set(correlation_id)
        CorrelationContext.clear()
        
        current = CorrelationContext.get_current()
        assert current is None
    
    def test_override_correlation_id(self, clear_context):
        """Test overriding correlation ID"""
        id1 = CorrelationContext.generate_id()
        id2 = CorrelationContext.generate_id()
        
        CorrelationContext.set(id1)
        assert CorrelationContext.get_current() == id1
        
        CorrelationContext.set(id2)
        assert CorrelationContext.get_current() == id2


# =============================================================================
# ASYNC CONTEXT TESTS
# =============================================================================

class TestAsyncContext:
    """Test async context propagation"""
    
    @pytest.mark.asyncio
    async def test_context_in_async_function(self, clear_context, correlation_id):
        """Test context available in async function"""
        CorrelationContext.set(correlation_id)
        
        async def inner_function():
            return CorrelationContext.get_current()
        
        result = await inner_function()
        assert result == correlation_id
    
    @pytest.mark.asyncio
    async def test_context_across_awaits(self, clear_context, correlation_id):
        """Test context preserved across awaits"""
        CorrelationContext.set(correlation_id)
        
        async def async_operation():
            await asyncio.sleep(0.01)
            return CorrelationContext.get_current()
        
        result = await async_operation()
        assert result == correlation_id
    
    @pytest.mark.asyncio
    async def test_context_in_task(self, clear_context, correlation_id):
        """Test context in asyncio task"""
        CorrelationContext.set(correlation_id)
        
        async def task_function():
            await asyncio.sleep(0.01)
            return CorrelationContext.get_current()
        
        task = asyncio.create_task(task_function())
        result = await task
        
        assert result == correlation_id
    
    @pytest.mark.asyncio
    async def test_context_isolation_between_tasks(self, clear_context):
        """Test context isolation between tasks"""
        async def task_with_id(task_id):
            correlation_id = f"task-{task_id}"
            CorrelationContext.set(correlation_id)
            await asyncio.sleep(0.01)
            return CorrelationContext.get_current()
        
        # Run multiple tasks
        results = await asyncio.gather(
            task_with_id(1),
            task_with_id(2),
            task_with_id(3)
        )
        
        # Each task should have its own correlation ID
        assert results[0] == "task-1"
        assert results[1] == "task-2"
        assert results[2] == "task-3"


# =============================================================================
# MIDDLEWARE TESTS
# =============================================================================

class TestMiddleware:
    """Test correlation middleware"""
    
    @pytest.mark.asyncio
    async def test_middleware_extracts_from_header(self, clear_context, mock_request):
        """Test middleware extracts correlation ID from header"""
        middleware = CorrelationMiddleware()
        expected_id = "existing-correlation-id"
        mock_request.headers['X-Correlation-ID'] = expected_id
        
        correlation_id = await middleware.process_request(mock_request)
        
        assert correlation_id == expected_id
        assert CorrelationContext.get_current() == expected_id
    
    @pytest.mark.asyncio
    async def test_middleware_generates_when_missing(self, clear_context, mock_request):
        """Test middleware generates ID when missing from header"""
        middleware = CorrelationMiddleware()
        
        correlation_id = await middleware.process_request(mock_request)
        
        assert correlation_id is not None
        assert CorrelationContext.get_current() == correlation_id
    
    @pytest.mark.asyncio
    async def test_middleware_adds_to_response(self, mock_response, correlation_id):
        """Test middleware adds correlation ID to response"""
        middleware = CorrelationMiddleware()
        
        response = await middleware.process_response(mock_response, correlation_id)
        
        assert response.headers['X-Correlation-ID'] == correlation_id
    
    @pytest.mark.asyncio
    async def test_custom_header_name(self, clear_context, mock_request):
        """Test middleware with custom header name"""
        middleware = CorrelationMiddleware(header_name='X-Request-ID')
        expected_id = "custom-id"
        mock_request.headers['X-Request-ID'] = expected_id
        
        correlation_id = await middleware.process_request(mock_request)
        
        assert correlation_id == expected_id


# =============================================================================
# REQUEST TRACING TESTS
# =============================================================================

class TestRequestTracing:
    """Test request tracing with correlation IDs"""
    
    @pytest.mark.asyncio
    async def test_trace_through_service_calls(self, clear_context):
        """Test correlation ID traces through service calls"""
        correlation_id = CorrelationContext.generate_id()
        CorrelationContext.set(correlation_id)
        
        call_chain = []
        
        async def service_a():
            call_chain.append(('service_a', CorrelationContext.get_current()))
            return await service_b()
        
        async def service_b():
            call_chain.append(('service_b', CorrelationContext.get_current()))
            return await service_c()
        
        async def service_c():
            call_chain.append(('service_c', CorrelationContext.get_current()))
            return "result"
        
        result = await service_a()
        
        assert result == "result"
        assert len(call_chain) == 3
        # All should have same correlation ID
        assert all(cid == correlation_id for _, cid in call_chain)
    
    @pytest.mark.asyncio
    async def test_trace_with_error(self, clear_context, correlation_id):
        """Test correlation ID available in error handling"""
        CorrelationContext.set(correlation_id)
        
        captured_id = None
        
        async def failing_service():
            raise ValueError("Service error")
        
        try:
            await failing_service()
        except ValueError:
            captured_id = CorrelationContext.get_current()
        
        assert captured_id == correlation_id


# =============================================================================
# LOG INTEGRATION TESTS
# =============================================================================

class TestLogIntegration:
    """Test integration with logging"""
    
    def test_add_correlation_to_log_record(self, clear_context, correlation_id):
        """Test adding correlation ID to log record"""
        CorrelationContext.set(correlation_id)
        
        # Simulate log record
        log_record = {
            'message': 'Test log',
            'level': 'INFO',
            'correlation_id': CorrelationContext.get_current()
        }
        
        assert log_record['correlation_id'] == correlation_id
    
    def test_log_filter_adds_correlation(self, clear_context, correlation_id):
        """Test log filter adds correlation ID"""
        CorrelationContext.set(correlation_id)
        
        class CorrelationFilter:
            def filter(self, record):
                record.correlation_id = CorrelationContext.get_current()
                return True
        
        import logging
        record = logging.LogRecord(
            name='test',
            level=logging.INFO,
            pathname='',
            lineno=0,
            msg='Test',
            args=(),
            exc_info=None
        )
        
        filter_obj = CorrelationFilter()
        filter_obj.filter(record)
        
        assert hasattr(record, 'correlation_id')
        assert record.correlation_id == correlation_id


# =============================================================================
# THREAD SAFETY TESTS
# =============================================================================

class TestThreadSafety:
    """Test thread safety of correlation context"""
    
    @pytest.mark.concurrent
    def test_context_isolation_between_threads(self, clear_context):
        """Test context isolation between threads"""
        results = []
        
        def thread_function(thread_id):
            correlation_id = f"thread-{thread_id}"
            CorrelationContext.set(correlation_id)
            time.sleep(0.01)  # Simulate work
            return CorrelationContext.get_current()
        
        with ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(thread_function, i) for i in range(5)]
            results = [f.result() for f in as_completed(futures)]
        
        # Each thread should have its own correlation ID
        assert len(results) == 5
        assert len(set(results)) == 5  # All unique
    
    @pytest.mark.concurrent
    @pytest.mark.asyncio
    async def test_concurrent_async_context(self, clear_context):
        """Test concurrent async operations with context"""
        async def async_operation(op_id):
            correlation_id = f"op-{op_id}"
            CorrelationContext.set(correlation_id)
            await asyncio.sleep(0.01)
            return CorrelationContext.get_current()
        
        results = await asyncio.gather(*[async_operation(i) for i in range(10)])
        
        # Each operation should maintain its own context
        assert len(results) == 10
        assert len(set(results)) == 10


# =============================================================================
# PERFORMANCE TESTS
# =============================================================================

class TestPerformance:
    """Test performance impact of correlation tracking"""
    
    @pytest.mark.performance
    def test_set_get_performance(self, clear_context):
        """Test performance of set/get operations"""
        correlation_id = CorrelationContext.generate_id()
        
        start = time.perf_counter()
        for _ in range(10000):
            CorrelationContext.set(correlation_id)
            _ = CorrelationContext.get_current()
        duration = time.perf_counter() - start
        
        # Should be very fast
        assert duration < 0.1
    
    @pytest.mark.performance
    def test_id_generation_performance(self):
        """Test performance of ID generation"""
        start = time.perf_counter()
        for _ in range(1000):
            CorrelationContext.generate_id()
        duration = time.perf_counter() - start
        
        # Should be fast
        assert duration < 0.5


# =============================================================================
# PROPAGATION TESTS
# =============================================================================

class TestPropagation:
    """Test correlation ID propagation"""
    
    @pytest.mark.asyncio
    async def test_propagate_to_external_calls(self, clear_context, correlation_id):
        """Test propagating correlation ID to external calls"""
        CorrelationContext.set(correlation_id)
        
        async def make_external_call():
            headers = {
                'X-Correlation-ID': CorrelationContext.get_current()
            }
            # Simulate external call with headers
            return headers
        
        result = await make_external_call()
        
        assert result['X-Correlation-ID'] == correlation_id
    
    @pytest.mark.asyncio
    async def test_propagate_through_queue(self, clear_context, correlation_id):
        """Test propagating through message queue"""
        CorrelationContext.set(correlation_id)
        
        # Simulate adding message to queue
        message = {
            'data': 'test',
            'correlation_id': CorrelationContext.get_current()
        }
        
        # Simulate processing message
        CorrelationContext.clear()
        CorrelationContext.set(message['correlation_id'])
        
        assert CorrelationContext.get_current() == correlation_id


# =============================================================================
# UTILITY FUNCTIONS TESTS
# =============================================================================

class TestUtilityFunctions:
    """Test utility functions for correlation tracking"""
    
    def test_with_correlation_decorator(self, clear_context):
        """Test decorator that sets correlation ID"""
        def with_correlation(func):
            def wrapper(*args, **kwargs):
                if CorrelationContext.get_current() is None:
                    CorrelationContext.set(CorrelationContext.generate_id())
                return func(*args, **kwargs)
            return wrapper
        
        @with_correlation
        def some_function():
            return CorrelationContext.get_current()
        
        result = some_function()
        assert result is not None
    
    @pytest.mark.asyncio
    async def test_async_with_correlation_decorator(self, clear_context):
        """Test async decorator that sets correlation ID"""
        def with_correlation(func):
            async def wrapper(*args, **kwargs):
                if CorrelationContext.get_current() is None:
                    CorrelationContext.set(CorrelationContext.generate_id())
                return await func(*args, **kwargs)
            return wrapper
        
        @with_correlation
        async def some_async_function():
            return CorrelationContext.get_current()
        
        result = await some_async_function()
        assert result is not None


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    def test_empty_string_correlation_id(self, clear_context):
        """Test handling empty string as correlation ID"""
        CorrelationContext.set("")
        
        current = CorrelationContext.get_current()
        assert current == ""
    
    def test_none_correlation_id(self, clear_context):
        """Test handling None as correlation ID"""
        CorrelationContext.set(None)
        
        current = CorrelationContext.get_current()
        assert current is None
    
    def test_very_long_correlation_id(self, clear_context):
        """Test handling very long correlation ID"""
        long_id = "x" * 10000
        CorrelationContext.set(long_id)
        
        current = CorrelationContext.get_current()
        assert current == long_id
    
    def test_special_characters_in_id(self, clear_context):
        """Test handling special characters in correlation ID"""
        special_id = "id-with-!@#$%^&*()-special"
        CorrelationContext.set(special_id)
        
        current = CorrelationContext.get_current()
        assert current == special_id
    
    @pytest.mark.asyncio
    async def test_context_after_exception(self, clear_context, correlation_id):
        """Test context preserved after exception"""
        CorrelationContext.set(correlation_id)
        
        try:
            raise ValueError("Test exception")
        except ValueError:
            pass
        
        # Context should still be set
        assert CorrelationContext.get_current() == correlation_id
    
    @pytest.mark.asyncio
    async def test_nested_context_changes(self, clear_context):
        """Test nested context changes"""
        id1 = "id-1"
        id2 = "id-2"
        
        CorrelationContext.set(id1)
        assert CorrelationContext.get_current() == id1
        
        # Nested change
        CorrelationContext.set(id2)
        assert CorrelationContext.get_current() == id2
        
        # No automatic restoration (not a context manager)
        assert CorrelationContext.get_current() == id2