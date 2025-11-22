"""
Circuit Breaker Pattern

Implements circuit breaker pattern for resilient external service calls.
Prevents cascading failures by failing fast when service is down.

States:
- CLOSED: Normal operation, requests go through
- OPEN: Service failing, requests fail immediately
- HALF_OPEN: Testing if service recovered

Features:
- Configurable failure threshold
- Automatic state transitions
- Timeout and cooldown periods
- Metrics tracking
- Thread-safe operation
"""

import asyncio
import logging
from typing import Callable, Optional, Any
from datetime import datetime, timedelta
from enum import Enum
import functools

from prometheus_client import Counter, Gauge

logger = logging.getLogger(__name__)


class CircuitState(str, Enum):
    """Circuit breaker states."""
    
    CLOSED = "closed"        # Normal operation
    OPEN = "open"            # Failing, block requests
    HALF_OPEN = "half_open"  # Testing recovery


class CircuitBreakerError(Exception):
    """Raised when circuit is open."""
    pass


class CircuitBreaker:
    """
    Circuit breaker for protecting external service calls.
    
    Usage:
        breaker = CircuitBreaker(
            failure_threshold=5,
            timeout_seconds=60
        )
        
        @breaker
        async def call_external_service():
            # Your code here
            pass
    """
    
    # Metrics (class-level, shared)
    _metrics_initialized = False
    
    def __init__(
        self,
        name: str = "default",
        failure_threshold: int = 5,
        success_threshold: int = 2,
        timeout_seconds: float = 60.0,
        half_open_max_calls: int = 1
    ):
        """
        Initialize circuit breaker.
        
        Args:
            name: Circuit breaker name for metrics
            failure_threshold: Failures before opening circuit
            success_threshold: Successes in half-open before closing
            timeout_seconds: Cooldown before trying half-open
            half_open_max_calls: Max concurrent calls in half-open
        """
        self.name = name
        self.failure_threshold = failure_threshold
        self.success_threshold = success_threshold
        self.timeout_seconds = timeout_seconds
        self.half_open_max_calls = half_open_max_calls
        
        # State
        self._state = CircuitState.CLOSED
        self._failure_count = 0
        self._success_count = 0
        self._last_failure_time: Optional[datetime] = None
        self._half_open_calls = 0
        
        # Lock for thread safety
        self._lock = asyncio.Lock()
        
        # Initialize metrics
        if not CircuitBreaker._metrics_initialized:
            self._init_metrics()
            CircuitBreaker._metrics_initialized = True
        
        logger.info(f"Circuit breaker '{name}' initialized")
    
    @classmethod
    def _init_metrics(cls):
        """Initialize Prometheus metrics."""
        cls.state_gauge = Gauge(
            'circuit_breaker_state',
            'Circuit breaker state (0=closed, 1=open, 2=half_open)',
            ['name']
        )
        
        cls.calls_total = Counter(
            'circuit_breaker_calls_total',
            'Total circuit breaker calls',
            ['name', 'state', 'result']
        )
        
        cls.state_transitions = Counter(
            'circuit_breaker_state_transitions_total',
            'Circuit breaker state transitions',
            ['name', 'from_state', 'to_state']
        )
    
    @property
    def state(self) -> CircuitState:
        """Get current state."""
        return self._state
    
    @property
    def is_closed(self) -> bool:
        """Check if circuit is closed."""
        return self._state == CircuitState.CLOSED
    
    @property
    def is_open(self) -> bool:
        """Check if circuit is open."""
        return self._state == CircuitState.OPEN
    
    @property
    def is_half_open(self) -> bool:
        """Check if circuit is half-open."""
        return self._state == CircuitState.HALF_OPEN
    
    def _transition_to(self, new_state: CircuitState) -> None:
        """
        Transition to new state.
        
        Args:
            new_state: Target state
        """
        old_state = self._state
        
        if old_state != new_state:
            logger.info(
                f"Circuit breaker '{self.name}' transitioning: "
                f"{old_state.value} -> {new_state.value}"
            )
            
            self._state = new_state
            
            # Update metrics
            if hasattr(self, 'state_transitions'):
                self.state_transitions.labels(
                    name=self.name,
                    from_state=old_state.value,
                    to_state=new_state.value
                ).inc()
            
            if hasattr(self, 'state_gauge'):
                state_value = {
                    CircuitState.CLOSED: 0,
                    CircuitState.OPEN: 1,
                    CircuitState.HALF_OPEN: 2
                }[new_state]
                self.state_gauge.labels(name=self.name).set(state_value)
            
            # Reset counters on transition
            if new_state == CircuitState.CLOSED:
                self._failure_count = 0
                self._success_count = 0
                self._half_open_calls = 0
            elif new_state == CircuitState.HALF_OPEN:
                self._success_count = 0
                self._half_open_calls = 0
    
    def _should_attempt_reset(self) -> bool:
        """Check if circuit should attempt reset to half-open."""
        if self._state != CircuitState.OPEN:
            return False
        
        if not self._last_failure_time:
            return False
        
        time_since_failure = (datetime.utcnow() - self._last_failure_time).total_seconds()
        return time_since_failure >= self.timeout_seconds
    
    async def call(self, func: Callable, *args, **kwargs) -> Any:
        """
        Execute function through circuit breaker.
        
        Args:
            func: Async function to call
            *args: Positional arguments
            **kwargs: Keyword arguments
            
        Returns:
            Function result
            
        Raises:
            CircuitBreakerError: If circuit is open
        """
        async with self._lock:
            # Check if should attempt reset
            if self._should_attempt_reset():
                self._transition_to(CircuitState.HALF_OPEN)
            
            # Block if open
            if self.is_open:
                if hasattr(self, 'calls_total'):
                    self.calls_total.labels(
                        name=self.name,
                        state='open',
                        result='blocked'
                    ).inc()
                
                raise CircuitBreakerError(
                    f"Circuit breaker '{self.name}' is open"
                )
            
            # Limit concurrent calls in half-open
            if self.is_half_open:
                if self._half_open_calls >= self.half_open_max_calls:
                    if hasattr(self, 'calls_total'):
                        self.calls_total.labels(
                            name=self.name,
                            state='half_open',
                            result='blocked'
                        ).inc()
                    
                    raise CircuitBreakerError(
                        f"Circuit breaker '{self.name}' is at half-open capacity"
                    )
                
                self._half_open_calls += 1
        
        # Execute function
        try:
            result = await func(*args, **kwargs)
            
            # Record success
            async with self._lock:
                await self._on_success()
            
            return result
            
        except Exception as e:
            # Record failure
            async with self._lock:
                await self._on_failure()
            
            raise
        
        finally:
            # Decrement half-open calls
            if self.is_half_open:
                async with self._lock:
                    self._half_open_calls -= 1
    
    async def _on_success(self) -> None:
        """Handle successful call."""
        if hasattr(self, 'calls_total'):
            self.calls_total.labels(
                name=self.name,
                state=self._state.value,
                result='success'
            ).inc()
        
        if self.is_half_open:
            self._success_count += 1
            
            if self._success_count >= self.success_threshold:
                self._transition_to(CircuitState.CLOSED)
        elif self.is_closed:
            # Reset failure count on success
            self._failure_count = 0
    
    async def _on_failure(self) -> None:
        """Handle failed call."""
        if hasattr(self, 'calls_total'):
            self.calls_total.labels(
                name=self.name,
                state=self._state.value,
                result='failure'
            ).inc()
        
        self._failure_count += 1
        self._last_failure_time = datetime.utcnow()
        
        if self.is_half_open:
            # Any failure in half-open reopens circuit
            self._transition_to(CircuitState.OPEN)
        elif self.is_closed:
            if self._failure_count >= self.failure_threshold:
                self._transition_to(CircuitState.OPEN)
    
    def __call__(self, func: Callable) -> Callable:
        """
        Decorator usage.
        
        Usage:
            @circuit_breaker
            async def my_function():
                pass
        """
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            return await self.call(func, *args, **kwargs)
        
        return wrapper
    
    async def reset(self) -> None:
        """Manually reset circuit to closed state."""
        async with self._lock:
            self._transition_to(CircuitState.CLOSED)
            self._failure_count = 0
            self._success_count = 0
            self._last_failure_time = None
        
        logger.info(f"Circuit breaker '{self.name}' manually reset")
    
    def get_stats(self) -> dict:
        """
        Get circuit breaker statistics.
        
        Returns:
            dict: Current statistics
        """
        return {
            'name': self.name,
            'state': self._state.value,
            'failure_count': self._failure_count,
            'success_count': self._success_count,
            'failure_threshold': self.failure_threshold,
            'success_threshold': self.success_threshold,
            'timeout_seconds': self.timeout_seconds,
            'last_failure_time': self._last_failure_time.isoformat() if self._last_failure_time else None
        }