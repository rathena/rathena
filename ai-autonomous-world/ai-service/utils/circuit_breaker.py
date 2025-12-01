"""
Circuit Breaker Pattern Implementation
Prevents cascade failures when LLM providers are slow or unavailable
"""

import time
import asyncio
from enum import Enum
from typing import Callable, Any, Optional
from loguru import logger
from prometheus_client import Counter, Gauge


class CircuitState(Enum):
    """Circuit breaker states"""
    CLOSED = "closed"  # Normal operation
    OPEN = "open"  # Failing - reject requests
    HALF_OPEN = "half_open"  # Testing if service recovered


# Prometheus metrics
circuit_breaker_state_changes = Counter(
    "circuit_breaker_state_changes_total",
    "Circuit breaker state changes",
    ["provider", "from_state", "to_state"]
)

circuit_breaker_state_gauge = Gauge(
    "circuit_breaker_state",
    "Current circuit breaker state (0=closed, 1=open, 2=half_open)",
    ["provider"]
)

circuit_breaker_failures = Counter(
    "circuit_breaker_failures_total",
    "Circuit breaker failures",
    ["provider"]
)


class CircuitBreaker:
    """
    Circuit breaker for LLM provider calls
    
    Prevents cascade failures by:
    - Opening circuit after N consecutive failures
    - Rejecting requests when circuit is open
    - Testing service recovery periodically
    """
    
    def __init__(
        self,
        name: str,
        failure_threshold: int = 5,
        recovery_timeout: float = 60.0,
        half_open_requests: int = 3
    ):
        """
        Initialize circuit breaker
        
        Args:
            name: Identifier for this circuit breaker
            failure_threshold: Number of consecutive failures before opening
            recovery_timeout: Seconds to wait before testing recovery
            half_open_requests: Number of test requests in half-open state
        """
        self.name = name
        self.failure_threshold = failure_threshold
        self.recovery_timeout = recovery_timeout
        self.half_open_requests = half_open_requests
        
        self.state = CircuitState.CLOSED
        self.failure_count = 0
        self.success_count = 0
        self.last_failure_time: Optional[float] = None
        self.open_until: Optional[float] = None
        
        logger.info(
            f"Circuit breaker '{name}' initialized: "
            f"threshold={failure_threshold}, recovery={recovery_timeout}s"
        )
    
    async def call(self, func: Callable, *args, **kwargs) -> Any:
        """
        Execute function with circuit breaker protection
        
        Args:
            func: Async function to execute
            *args: Positional arguments for func
            **kwargs: Keyword arguments for func
            
        Returns:
            Function result
            
        Raises:
            CircuitBreakerError: If circuit is open
            Original exception: If function fails
        """
        # Check circuit state
        if self.state == CircuitState.OPEN:
            if time.time() >= self.open_until:
                # Try transitioning to half-open
                self._transition_to(CircuitState.HALF_OPEN)
            else:
                # Circuit still open - reject request
                raise CircuitBreakerError(
                    f"Circuit breaker '{self.name}' is OPEN "
                    f"(opens in {self.open_until - time.time():.1f}s)"
                )
        
        # Execute function
        try:
            result = await func(*args, **kwargs)
            await self._on_success()
            return result
            
        except Exception as e:
            await self._on_failure(e)
            raise
    
    async def _on_success(self):
        """Handle successful request"""
        if self.state == CircuitState.HALF_OPEN:
            self.success_count += 1
            
            if self.success_count >= self.half_open_requests:
                # Service recovered - close circuit
                self._transition_to(CircuitState.CLOSED)
                self.failure_count = 0
                self.success_count = 0
                logger.info(f"Circuit breaker '{self.name}' recovered after testing")
        
        elif self.state == CircuitState.CLOSED:
            # Reset failure count on success
            self.failure_count = 0
    
    async def _on_failure(self, exception: Exception):
        """Handle failed request"""
        self.failure_count += 1
        self.last_failure_time = time.time()
        
        circuit_breaker_failures.labels(provider=self.name).inc()
        
        logger.warning(
            f"Circuit breaker '{self.name}' failure "
            f"({self.failure_count}/{self.failure_threshold}): {exception}"
        )
        
        if self.state == CircuitState.HALF_OPEN:
            # Failed during recovery test - reopen circuit
            self._transition_to(CircuitState.OPEN)
            self.open_until = time.time() + self.recovery_timeout
            self.success_count = 0
            
        elif self.failure_count >= self.failure_threshold:
            # Too many failures - open circuit
            self._transition_to(CircuitState.OPEN)
            self.open_until = time.time() + self.recovery_timeout
            logger.error(
                f"Circuit breaker '{self.name}' OPENED "
                f"(will retry in {self.recovery_timeout}s)"
            )
    
    def _transition_to(self, new_state: CircuitState):
        """Transition to new circuit state"""
        if self.state != new_state:
            old_state = self.state
            self.state = new_state
            
            # Update metrics
            circuit_breaker_state_changes.labels(
                provider=self.name,
                from_state=old_state.value,
                to_state=new_state.value
            ).inc()
            
            # Map states to numeric values for gauge
            state_values = {
                CircuitState.CLOSED: 0,
                CircuitState.OPEN: 1,
                CircuitState.HALF_OPEN: 2
            }
            circuit_breaker_state_gauge.labels(provider=self.name).set(
                state_values[new_state]
            )
            
            logger.info(
                f"Circuit breaker '{self.name}' transitioned: "
                f"{old_state.value} -> {new_state.value}"
            )
    
    def get_state(self) -> CircuitState:
        """Get current circuit state"""
        return self.state
    
    def reset(self):
        """Reset circuit breaker to closed state"""
        self.state = CircuitState.CLOSED
        self.failure_count = 0
        self.success_count = 0
        self.open_until = None
        logger.info(f"Circuit breaker '{self.name}' manually reset")


class CircuitBreakerError(Exception):
    """Raised when circuit breaker is open"""
    pass


# Global circuit breakers for each LLM provider
_circuit_breakers: dict[str, CircuitBreaker] = {}


def get_circuit_breaker(provider_name: str) -> CircuitBreaker:
    """
    Get or create circuit breaker for provider
    
    Args:
        provider_name: LLM provider name
        
    Returns:
        CircuitBreaker instance
    """
    if provider_name not in _circuit_breakers:
        _circuit_breakers[provider_name] = CircuitBreaker(
            name=provider_name,
            failure_threshold=5,  # Open after 5 failures
            recovery_timeout=60.0,  # Wait 60s before retry
            half_open_requests=3  # Test with 3 requests
        )
    
    return _circuit_breakers[provider_name]