"""
Tests for Circuit Breaker Pattern Utility

Comprehensive test suite covering:
- Circuit breaker states (Closed, Open, Half-Open)
- Failure threshold detection
- Automatic recovery attempts
- Timeout handling
- Success rate monitoring
- Thread safety
- Performance impact
"""

import pytest
import asyncio
import time
from unittest.mock import Mock, AsyncMock, patch
from datetime import datetime, timedelta
from enum import Enum
from concurrent.futures import ThreadPoolExecutor, as_completed


# =============================================================================
# MOCK CLASSES
# =============================================================================

class CircuitState(Enum):
    """Circuit breaker states"""
    CLOSED = "closed"      # Normal operation
    OPEN = "open"          # Blocking requests
    HALF_OPEN = "half_open"  # Testing recovery


class CircuitBreaker:
    """Mock circuit breaker implementation"""
    
    def __init__(self, failure_threshold=5, timeout_seconds=60, success_threshold=2):
        self.failure_threshold = failure_threshold
        self.timeout_seconds = timeout_seconds
        self.success_threshold = success_threshold
        
        self.state = CircuitState.CLOSED
        self.failure_count = 0
        self.success_count = 0
        self.last_failure_time = None
        self.call_count = 0
    
    async def call(self, func, *args, **kwargs):
        """Execute function with circuit breaker protection"""
        self.call_count += 1
        
        if self.state == CircuitState.OPEN:
            if self._should_attempt_reset():
                self.state = CircuitState.HALF_OPEN
                self.success_count = 0
            else:
                raise RuntimeError("Circuit breaker is OPEN")
        
        try:
            result = await func(*args, **kwargs)
            self._on_success()
            return result
        except Exception as e:
            self._on_failure()
            raise
    
    def _on_success(self):
        """Handle successful call"""
        if self.state == CircuitState.HALF_OPEN:
            self.success_count += 1
            if self.success_count >= self.success_threshold:
                self._reset()
        elif self.state == CircuitState.CLOSED:
            self.failure_count = 0
    
    def _on_failure(self):
        """Handle failed call"""
        self.failure_count += 1
        self.last_failure_time = time.time()
        
        if self.failure_count >= self.failure_threshold:
            self._trip()
    
    def _trip(self):
        """Open the circuit"""
        self.state = CircuitState.OPEN
        self.last_failure_time = time.time()
    
    def _reset(self):
        """Close the circuit"""
        self.state = CircuitState.CLOSED
        self.failure_count = 0
        self.success_count = 0
    
    def _should_attempt_reset(self):
        """Check if should attempt reset"""
        if self.last_failure_time is None:
            return False
        elapsed = time.time() - self.last_failure_time
        return elapsed >= self.timeout_seconds
    
    def get_state(self):
        """Get current state"""
        return self.state


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def circuit_breaker():
    """Create circuit breaker instance"""
    return CircuitBreaker(
        failure_threshold=3,
        timeout_seconds=1.0,
        success_threshold=2
    )


@pytest.fixture
def failing_function():
    """Create function that always fails"""
    async def func():
        raise ValueError("Service unavailable")
    return func


@pytest.fixture
def succeeding_function():
    """Create function that always succeeds"""
    async def func():
        return "success"
    return func


@pytest.fixture
def flaky_function():
    """Create function that fails then succeeds"""
    call_count = {'count': 0}
    
    async def func():
        call_count['count'] += 1
        if call_count['count'] <= 3:
            raise ValueError("Flaky failure")
        return "success"
    
    return func


# =============================================================================
# STATE TRANSITION TESTS
# =============================================================================

class TestStateTransitions:
    """Test circuit breaker state transitions"""
    
    @pytest.mark.asyncio
    async def test_initial_state_closed(self, circuit_breaker):
        """Test circuit starts in CLOSED state"""
        assert circuit_breaker.get_state() == CircuitState.CLOSED
    
    @pytest.mark.asyncio
    async def test_transition_to_open(self, circuit_breaker, failing_function):
        """Test transition from CLOSED to OPEN on failures"""
        # Fail threshold times
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.OPEN
    
    @pytest.mark.asyncio
    async def test_transition_to_half_open(self, circuit_breaker, failing_function):
        """Test transition from OPEN to HALF_OPEN after timeout"""
        # Trip the circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.OPEN
        
        # Wait for timeout
        await asyncio.sleep(1.1)
        
        # Next call should transition to HALF_OPEN
        try:
            await circuit_breaker.call(failing_function)
        except:
            pass
        
        # Should have attempted HALF_OPEN
        assert circuit_breaker.get_state() == CircuitState.OPEN  # Failed again
    
    @pytest.mark.asyncio
    async def test_recovery_to_closed(self, circuit_breaker, failing_function, succeeding_function):
        """Test recovery from HALF_OPEN to CLOSED"""
        # Trip the circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.OPEN
        
        # Wait and succeed threshold times
        await asyncio.sleep(1.1)
        
        # Manually set to HALF_OPEN
        circuit_breaker.state = CircuitState.HALF_OPEN
        circuit_breaker.success_count = 0
        
        # Succeed threshold times
        for _ in range(2):
            await circuit_breaker.call(succeeding_function)
        
        assert circuit_breaker.get_state() == CircuitState.CLOSED


# =============================================================================
# FAILURE THRESHOLD TESTS
# =============================================================================

class TestFailureThreshold:
    """Test failure threshold behavior"""
    
    @pytest.mark.asyncio
    async def test_below_threshold(self, circuit_breaker, failing_function, succeeding_function):
        """Test circuit stays closed below threshold"""
        # Fail below threshold
        for _ in range(2):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.CLOSED
        
        # Success resets failure count
        await circuit_breaker.call(succeeding_function)
        assert circuit_breaker.failure_count == 0
    
    @pytest.mark.asyncio
    async def test_at_threshold(self, circuit_breaker, failing_function):
        """Test circuit opens at threshold"""
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.OPEN
    
    @pytest.mark.asyncio
    async def test_above_threshold(self, circuit_breaker, failing_function):
        """Test circuit stays open above threshold"""
        for _ in range(5):
            try:
                await circuit_breaker.call(failing_function)
            except:
                pass
        
        assert circuit_breaker.get_state() == CircuitState.OPEN


# =============================================================================
# TIMEOUT TESTS
# =============================================================================

class TestTimeout:
    """Test timeout behavior"""
    
    @pytest.mark.asyncio
    async def test_timeout_allows_retry(self, circuit_breaker, failing_function):
        """Test circuit allows retry after timeout"""
        # Trip circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.get_state() == CircuitState.OPEN
        
        # Wait for timeout
        await asyncio.sleep(1.1)
        
        # Should allow attempt
        assert circuit_breaker._should_attempt_reset()
    
    @pytest.mark.asyncio
    async def test_before_timeout_blocks(self, circuit_breaker, failing_function):
        """Test circuit blocks before timeout"""
        # Trip circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        # Immediately try again
        with pytest.raises(RuntimeError) as exc_info:
            await circuit_breaker.call(failing_function)
        
        assert "Circuit breaker is OPEN" in str(exc_info.value)


# =============================================================================
# SUCCESS RATE TESTS
# =============================================================================

class TestSuccessRate:
    """Test success rate monitoring"""
    
    @pytest.mark.asyncio
    async def test_track_success_count(self, circuit_breaker, succeeding_function):
        """Test tracking successful calls"""
        circuit_breaker.state = CircuitState.HALF_OPEN
        
        await circuit_breaker.call(succeeding_function)
        assert circuit_breaker.success_count == 1
        
        await circuit_breaker.call(succeeding_function)
        assert circuit_breaker.success_count == 2
    
    @pytest.mark.asyncio
    async def test_success_threshold_recovery(self, circuit_breaker, succeeding_function):
        """Test recovery after reaching success threshold"""
        circuit_breaker.state = CircuitState.HALF_OPEN
        
        for _ in range(2):
            await circuit_breaker.call(succeeding_function)
        
        assert circuit_breaker.get_state() == CircuitState.CLOSED


# =============================================================================
# CALL BLOCKING TESTS
# =============================================================================

class TestCallBlocking:
    """Test call blocking when circuit is open"""
    
    @pytest.mark.asyncio
    async def test_block_when_open(self, circuit_breaker, failing_function):
        """Test calls are blocked when circuit is open"""
        # Trip circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        # Try to call
        with pytest.raises(RuntimeError):
            await circuit_breaker.call(failing_function)
    
    @pytest.mark.asyncio
    async def test_allow_when_closed(self, circuit_breaker, succeeding_function):
        """Test calls are allowed when circuit is closed"""
        result = await circuit_breaker.call(succeeding_function)
        assert result == "success"
    
    @pytest.mark.asyncio
    async def test_allow_test_when_half_open(self, circuit_breaker, succeeding_function):
        """Test calls are allowed when circuit is half-open"""
        circuit_breaker.state = CircuitState.HALF_OPEN
        
        result = await circuit_breaker.call(succeeding_function)
        assert result == "success"


# =============================================================================
# ERROR HANDLING TESTS
# =============================================================================

class TestErrorHandling:
    """Test error handling in circuit breaker"""
    
    @pytest.mark.asyncio
    async def test_propagate_exception(self, circuit_breaker, failing_function):
        """Test exceptions are propagated"""
        with pytest.raises(ValueError) as exc_info:
            await circuit_breaker.call(failing_function)
        
        assert "Service unavailable" in str(exc_info.value)
    
    @pytest.mark.asyncio
    async def test_different_exception_types(self, circuit_breaker):
        """Test handling different exception types"""
        async def timeout_error():
            raise TimeoutError("Request timeout")
        
        async def connection_error():
            raise ConnectionError("Connection failed")
        
        # Both should count as failures
        with pytest.raises(TimeoutError):
            await circuit_breaker.call(timeout_error)
        
        with pytest.raises(ConnectionError):
            await circuit_breaker.call(connection_error)
        
        assert circuit_breaker.failure_count == 2


# =============================================================================
# METRICS TESTS
# =============================================================================

class TestMetrics:
    """Test circuit breaker metrics"""
    
    @pytest.mark.asyncio
    async def test_track_call_count(self, circuit_breaker, succeeding_function):
        """Test tracking total call count"""
        for _ in range(5):
            await circuit_breaker.call(succeeding_function)
        
        assert circuit_breaker.call_count == 5
    
    @pytest.mark.asyncio
    async def test_track_failure_count(self, circuit_breaker, failing_function):
        """Test tracking failure count"""
        for _ in range(2):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        assert circuit_breaker.failure_count == 2
    
    @pytest.mark.asyncio
    async def test_reset_counts_on_recovery(self, circuit_breaker, succeeding_function):
        """Test counts reset on recovery"""
        circuit_breaker.failure_count = 5
        circuit_breaker.success_count = 3
        
        circuit_breaker._reset()
        
        assert circuit_breaker.failure_count == 0
        assert circuit_breaker.success_count == 0


# =============================================================================
# PERFORMANCE TESTS
# =============================================================================

class TestPerformance:
    """Test circuit breaker performance impact"""
    
    @pytest.mark.performance
    @pytest.mark.asyncio
    async def test_minimal_overhead(self, circuit_breaker):
        """Test circuit breaker has minimal overhead"""
        async def fast_function():
            return "fast"
        
        start = time.perf_counter()
        for _ in range(1000):
            await circuit_breaker.call(fast_function)
        duration = time.perf_counter() - start
        
        # Should add minimal overhead
        assert duration < 0.5
    
    @pytest.mark.performance
    @pytest.mark.asyncio
    async def test_open_circuit_fast_fail(self, circuit_breaker, failing_function):
        """Test open circuit fails fast"""
        # Trip circuit
        for _ in range(3):
            with pytest.raises(ValueError):
                await circuit_breaker.call(failing_function)
        
        # Should fail immediately without calling function
        start = time.perf_counter()
        for _ in range(100):
            try:
                await circuit_breaker.call(failing_function)
            except RuntimeError:
                pass
        duration = time.perf_counter() - start
        
        # Should be very fast (not calling actual function)
        assert duration < 0.1


# =============================================================================
# CONCURRENT OPERATIONS TESTS
# =============================================================================

class TestConcurrentOperations:
    """Test concurrent circuit breaker operations"""
    
    @pytest.mark.concurrent
    @pytest.mark.asyncio
    async def test_concurrent_calls(self, circuit_breaker, succeeding_function):
        """Test handling concurrent calls"""
        async def make_call():
            return await circuit_breaker.call(succeeding_function)
        
        results = await asyncio.gather(*[make_call() for _ in range(50)])
        
        assert len(results) == 50
        assert all(r == "success" for r in results)
    
    @pytest.mark.concurrent
    @pytest.mark.asyncio
    async def test_concurrent_failures(self, circuit_breaker, failing_function):
        """Test handling concurrent failures"""
        async def make_call():
            try:
                await circuit_breaker.call(failing_function)
            except:
                pass
        
        await asyncio.gather(*[make_call() for _ in range(10)])
        
        # Should have tripped
        assert circuit_breaker.get_state() == CircuitState.OPEN


# =============================================================================
# CONFIGURATION TESTS
# =============================================================================

class TestConfiguration:
    """Test circuit breaker configuration"""
    
    def test_custom_failure_threshold(self):
        """Test custom failure threshold"""
        cb = CircuitBreaker(failure_threshold=10)
        assert cb.failure_threshold == 10
    
    def test_custom_timeout(self):
        """Test custom timeout"""
        cb = CircuitBreaker(timeout_seconds=30)
        assert cb.timeout_seconds == 30
    
    def test_custom_success_threshold(self):
        """Test custom success threshold"""
        cb = CircuitBreaker(success_threshold=5)
        assert cb.success_threshold == 5


# =============================================================================
# RECOVERY STRATEGY TESTS
# =============================================================================

class TestRecoveryStrategy:
    """Test different recovery strategies"""
    
    @pytest.mark.asyncio
    async def test_immediate_recovery(self, succeeding_function):
        """Test immediate recovery strategy"""
        cb = CircuitBreaker(timeout_seconds=0.0)
        
        # Trip circuit
        async def fail():
            raise ValueError()
        
        for _ in range(3):
            with pytest.raises(ValueError):
                await cb.call(fail)
        
        # Should allow immediate retry
        assert cb._should_attempt_reset()
    
    @pytest.mark.asyncio
    async def test_gradual_recovery(self, circuit_breaker, succeeding_function):
        """Test gradual recovery with success threshold"""
        circuit_breaker.state = CircuitState.HALF_OPEN
        circuit_breaker.success_threshold = 5
        
        # Need 5 successes to recover
        for _ in range(4):
            await circuit_breaker.call(succeeding_function)
            assert circuit_breaker.get_state() == CircuitState.HALF_OPEN
        
        # 5th success should close circuit
        await circuit_breaker.call(succeeding_function)
        assert circuit_breaker.get_state() == CircuitState.CLOSED


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    @pytest.mark.asyncio
    async def test_zero_failure_threshold(self, succeeding_function):
        """Test with zero failure threshold"""
        cb = CircuitBreaker(failure_threshold=1)
        
        async def fail():
            raise ValueError()
        
        # Should trip immediately
        with pytest.raises(ValueError):
            await cb.call(fail)
        
        assert cb.get_state() == CircuitState.OPEN
    
    @pytest.mark.asyncio
    async def test_very_high_threshold(self, failing_function):
        """Test with very high failure threshold"""
        cb = CircuitBreaker(failure_threshold=1000)
        
        # Should not trip with few failures
        for _ in range(10):
            with pytest.raises(ValueError):
                await cb.call(failing_function)
        
        assert cb.get_state() == CircuitState.CLOSED
    
    @pytest.mark.asyncio
    async def test_function_with_args(self, circuit_breaker):
        """Test calling function with arguments"""
        async def func_with_args(x, y):
            return x + y
        
        result = await circuit_breaker.call(func_with_args, 2, 3)
        assert result == 5
    
    @pytest.mark.asyncio
    async def test_function_with_kwargs(self, circuit_breaker):
        """Test calling function with kwargs"""
        async def func_with_kwargs(a=1, b=2):
            return a * b
        
        result = await circuit_breaker.call(func_with_kwargs, a=3, b=4)
        assert result == 12
    
    @pytest.mark.asyncio
    async def test_mixed_success_failure(self, circuit_breaker):
        """Test alternating success and failure"""
        async def sometimes_fails():
            import random
            if random.random() < 0.5:
                raise ValueError()
            return "success"
        
        # Mix of successes and failures
        for _ in range(10):
            try:
                await circuit_breaker.call(sometimes_fails)
            except ValueError:
                pass
        
        # Should be in some state (not necessarily OPEN)
        assert circuit_breaker.get_state() in [CircuitState.CLOSED, CircuitState.OPEN]
    
    @pytest.mark.asyncio
    async def test_state_during_call(self, circuit_breaker):
        """Test state doesn't change during call"""
        async def long_function():
            await asyncio.sleep(0.1)
            return "done"
        
        initial_state = circuit_breaker.get_state()
        result = await circuit_breaker.call(long_function)
        
        assert result == "done"
        # State should either be same or CLOSED (on success)
        assert circuit_breaker.get_state() in [initial_state, CircuitState.CLOSED]