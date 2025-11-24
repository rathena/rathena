"""
Base LLM Provider Abstract Class

Provides common interface and functionality for all LLM providers including:
- Retry logic with exponential backoff
- Rate limit handling
- Token counting and cost tracking
- Metrics integration
- Error handling
"""

import asyncio
import logging
import time
from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Any, Dict, List, Optional
from enum import Enum

logger = logging.getLogger(__name__)

# Import circuit breaker
try:
    from utils.circuit_breaker import get_circuit_breaker, CircuitBreakerError
except ImportError:
    # Fallback if import fails
    get_circuit_breaker = None
    CircuitBreakerError = Exception


class ProviderType(Enum):
    """LLM provider types"""
    AZURE_OPENAI = "azure_openai"
    OPENAI = "openai"
    ANTHROPIC = "anthropic"
    DEEPSEEK = "deepseek"
    OLLAMA = "ollama"


# Custom Exceptions
class LLMProviderError(Exception):
    """Base exception for LLM provider errors"""
    pass


class RateLimitError(LLMProviderError):
    """Rate limit exceeded"""
    def __init__(self, message: str, retry_after: Optional[float] = None):
        super().__init__(message)
        self.retry_after = retry_after


class AuthenticationError(LLMProviderError):
    """Invalid API key or authentication failure"""
    pass


class ModelNotFoundError(LLMProviderError):
    """Requested model doesn't exist"""
    pass


class TimeoutError(LLMProviderError):
    """Request timeout"""
    pass


class ContentFilterError(LLMProviderError):
    """Content filtered by provider"""
    pass


@dataclass
class LLMResponse:
    """Standardized LLM response format"""
    text: str
    tokens_used: int
    cost_usd: float
    model: str
    provider: str
    latency_ms: float
    metadata: Dict[str, Any]
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            "text": self.text,
            "tokens_used": self.tokens_used,
            "cost_usd": self.cost_usd,
            "model": self.model,
            "provider": self.provider,
            "latency_ms": self.latency_ms,
            "metadata": self.metadata,
        }


class BaseLLMProvider(ABC):
    """
    Abstract base class for LLM providers.
    
    All providers must implement:
    - generate(): Single prompt completion
    - chat(): Multi-turn conversation
    - _make_request(): Provider-specific API call
    """
    
    # Retry configuration
    MAX_RETRIES = 3
    INITIAL_BACKOFF = 1.0
    MAX_BACKOFF = 60.0
    BACKOFF_MULTIPLIER = 2.0
    
    # Rate limit configuration
    RATE_LIMIT_WINDOW = 60.0  # seconds
    
    # Token pricing (override in subclasses)
    COST_PER_1K_INPUT_TOKENS = 0.0
    COST_PER_1K_OUTPUT_TOKENS = 0.0
    
    def __init__(
        self,
        provider_type: ProviderType,
        model: str,
        api_key: Optional[str] = None,
        timeout: float = 10.0,  # Reduced from 60s to 10s
        mock_mode: bool = False,
        debug: bool = False,
    ):
        """
        Initialize base provider.
        
        Args:
            provider_type: Type of provider
            model: Model identifier
            api_key: API key for authentication
            timeout: Request timeout in seconds (default: 10s)
            mock_mode: Enable mock mode for testing
            debug: Enable debug logging
        """
        self.provider_type = provider_type
        self.model = model
        self.api_key = api_key
        self.timeout = timeout
        self.mock_mode = mock_mode
        self.debug = debug
        
        # Circuit breaker for this provider
        self.circuit_breaker = get_circuit_breaker(provider_type.value) if get_circuit_breaker else None
        
        # Metrics
        self._total_requests = 0
        self._successful_requests = 0
        self._failed_requests = 0
        self._total_tokens = 0
        self._total_cost = 0.0
        self._total_latency = 0.0
        
        # Rate limiting
        self._request_times: List[float] = []
        
        if self.debug:
            logger.setLevel(logging.DEBUG)
    
    @abstractmethod
    async def _make_request(
        self,
        messages: List[Dict[str, str]],
        temperature: float,
        max_tokens: int,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Make provider-specific API request.
        
        Args:
            messages: List of message dicts with 'role' and 'content'
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            **kwargs: Provider-specific parameters
            
        Returns:
            Raw API response
        """
        pass
    
    async def generate(
        self,
        prompt: str,
        temperature: float = 0.7,
        max_tokens: int = 500,
        **kwargs
    ) -> LLMResponse:
        """
        Generate completion for a single prompt.
        
        Args:
            prompt: Input prompt text
            temperature: Sampling temperature (0.0-1.0)
            max_tokens: Maximum tokens to generate
            **kwargs: Provider-specific parameters
            
        Returns:
            LLMResponse with generated text and metadata
        """
        messages = [{"role": "user", "content": prompt}]
        return await self.chat(messages, temperature, max_tokens, **kwargs)
    
    async def chat(
        self,
        messages: List[Dict[str, str]],
        temperature: float = 0.7,
        max_tokens: int = 500,
        **kwargs
    ) -> LLMResponse:
        """
        Generate completion for multi-turn conversation.
        
        Args:
            messages: List of message dicts with 'role' and 'content'
            temperature: Sampling temperature (0.0-1.0)
            max_tokens: Maximum tokens to generate
            **kwargs: Provider-specific parameters
            
        Returns:
            LLMResponse with generated text and metadata
        """
        start_time = time.time()
        self._total_requests += 1
        
        try:
            # Mock mode for testing
            if self.mock_mode:
                return await self._mock_response(messages)
            
            # Rate limit check
            await self._check_rate_limit()
            
            # Retry with exponential backoff
            response = await self._retry_with_backoff(
                messages, temperature, max_tokens, **kwargs
            )
            
            # Calculate metrics
            latency_ms = (time.time() - start_time) * 1000
            tokens_used = response.get("tokens_used", 0)
            cost = self._calculate_cost(tokens_used)
            
            # Update metrics
            self._successful_requests += 1
            self._total_tokens += tokens_used
            self._total_cost += cost
            self._total_latency += latency_ms
            
            # Log request
            if self.debug:
                logger.debug(
                    f"Request successful: {self.provider_type.value} "
                    f"model={self.model} tokens={tokens_used} "
                    f"latency={latency_ms:.2f}ms cost=${cost:.6f}"
                )
            
            return LLMResponse(
                text=response["text"],
                tokens_used=tokens_used,
                cost_usd=cost,
                model=self.model,
                provider=self.provider_type.value,
                latency_ms=latency_ms,
                metadata=response.get("metadata", {}),
            )
            
        except Exception as e:
            self._failed_requests += 1
            logger.error(
                f"Request failed: {self.provider_type.value} "
                f"model={self.model} error={str(e)}"
            )
            raise
    
    async def _retry_with_backoff(
        self,
        messages: List[Dict[str, str]],
        temperature: float,
        max_tokens: int,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Retry request with exponential backoff and circuit breaker protection.
        
        Args:
            messages: Message list
            temperature: Sampling temperature
            max_tokens: Maximum tokens
            **kwargs: Additional parameters
            
        Returns:
            API response
        """
        backoff = self.INITIAL_BACKOFF
        last_exception = None
        
        # Wrap in circuit breaker if available
        async def make_request_with_timeout():
            try:
                # Apply timeout to prevent hanging
                return await asyncio.wait_for(
                    self._make_request(messages, temperature, max_tokens, **kwargs),
                    timeout=self.timeout
                )
            except asyncio.TimeoutError:
                raise TimeoutError(f"LLM request timed out after {self.timeout}s")
        
        for attempt in range(self.MAX_RETRIES):
            try:
                # Use circuit breaker if available
                if self.circuit_breaker:
                    return await self.circuit_breaker.call(make_request_with_timeout)
                else:
                    return await make_request_with_timeout()
                    
            except CircuitBreakerError as e:
                # Circuit is open - fail fast
                logger.error(f"Circuit breaker open for {self.provider_type.value}: {e}")
                raise
                
            except RateLimitError as e:
                last_exception = e
                # Use provider's suggested retry time if available
                wait_time = e.retry_after or backoff
                logger.warning(
                    f"Rate limit hit on attempt {attempt + 1}, "
                    f"retrying in {wait_time:.2f}s"
                )
                await asyncio.sleep(wait_time)
                backoff = min(backoff * self.BACKOFF_MULTIPLIER, self.MAX_BACKOFF)
                
            except (TimeoutError, ConnectionError) as e:
                last_exception = e
                if attempt < self.MAX_RETRIES - 1:
                    logger.warning(
                        f"Transient error on attempt {attempt + 1}, "
                        f"retrying in {backoff:.2f}s: {str(e)}"
                    )
                    await asyncio.sleep(backoff)
                    backoff = min(backoff * self.BACKOFF_MULTIPLIER, self.MAX_BACKOFF)
                    
            except (AuthenticationError, ModelNotFoundError, ContentFilterError):
                # Don't retry these errors
                raise
        
        # All retries exhausted
        if last_exception:
            raise last_exception
        raise LLMProviderError("Max retries exceeded")
    
    async def _check_rate_limit(self):
        """Check and enforce rate limiting"""
        current_time = time.time()
        
        # Remove old requests outside the window
        self._request_times = [
            t for t in self._request_times
            if current_time - t < self.RATE_LIMIT_WINDOW
        ]
        
        # Record this request
        self._request_times.append(current_time)
    
    def _calculate_cost(self, tokens_used: int) -> float:
        """
        Calculate cost based on token usage.
        
        Args:
            tokens_used: Total tokens used
            
        Returns:
            Cost in USD
        """
        # Simple calculation - can be overridden for more complex pricing
        return (tokens_used / 1000) * self.COST_PER_1K_INPUT_TOKENS
    
    async def _mock_response(self, messages: List[Dict[str, str]]) -> LLMResponse:
        """Generate mock response for testing"""
        await asyncio.sleep(0.1)  # Simulate latency
        return LLMResponse(
            text=f"Mock response from {self.provider_type.value}",
            tokens_used=10,
            cost_usd=0.0001,
            model=self.model,
            provider=self.provider_type.value,
            latency_ms=100.0,
            metadata={"mock": True},
        )
    
    def get_metrics(self) -> Dict[str, Any]:
        """
        Get provider metrics.
        
        Returns:
            Dictionary with metrics
        """
        return {
            "provider": self.provider_type.value,
            "model": self.model,
            "total_requests": self._total_requests,
            "successful_requests": self._successful_requests,
            "failed_requests": self._failed_requests,
            "success_rate": (
                self._successful_requests / self._total_requests
                if self._total_requests > 0 else 0
            ),
            "total_tokens": self._total_tokens,
            "total_cost_usd": self._total_cost,
            "average_latency_ms": (
                self._total_latency / self._successful_requests
                if self._successful_requests > 0 else 0
            ),
        }
    
    def reset_metrics(self):
        """Reset all metrics"""
        self._total_requests = 0
        self._successful_requests = 0
        self._failed_requests = 0
        self._total_tokens = 0
        self._total_cost = 0.0
        self._total_latency = 0.0