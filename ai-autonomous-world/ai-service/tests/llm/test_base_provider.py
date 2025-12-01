"""
Tests for BaseLLMProvider

Tests:
- Provider initialization
- Request retry logic
- Rate limiting
- Circuit breaker
- Cost calculation
- Metrics tracking
- Mock mode
- Error handling
"""

import asyncio
import pytest
from unittest.mock import AsyncMock, MagicMock, patch
from typing import Dict, Any, List

from llm.base_provider import (
    BaseLLMProvider,
    LLMResponse,
    LLMProviderError,
    RateLimitError,
    AuthenticationError,
    ModelNotFoundError,
    TimeoutError,
    ContentFilterError,
    ProviderType,
)


class ConcreteTestProvider(BaseLLMProvider):
    """Concrete implementation for testing"""
    
    def __init__(self, *args, should_fail=False, fail_count=0, **kwargs):
        super().__init__(ProviderType.OPENAI, "test-model", *args, **kwargs)
        self.should_fail = should_fail
        self.fail_count = fail_count
        self.call_count = 0
        self.COST_PER_1K_INPUT_TOKENS = 0.01
        self.COST_PER_1K_OUTPUT_TOKENS = 0.03
    
    async def _make_request(
        self,
        messages: List[Dict[str, str]],
        temperature: float,
        max_tokens: int,
        **kwargs
    ) -> Dict[str, Any]:
        """Test implementation"""
        self.call_count += 1
        
        if self.should_fail:
            if self.call_count <= self.fail_count:
                raise RateLimitError("Rate limit exceeded", retry_after=0.1)
        
        await asyncio.sleep(0.01)  # Simulate API call
        
        return {
            "text": "Test response",
            "tokens_used": 100,
            "metadata": {"test": True}
        }


@pytest.mark.unit
class TestBaseLLMProviderInitialization:
    """Test provider initialization"""
    
    def test_basic_initialization(self):
        """Test basic provider initialization"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            timeout=30.0,
        )
        
        assert provider.provider_type == ProviderType.OPENAI
        assert provider.model == "test-model"
        assert provider.api_key == "test_key"
        assert provider.timeout == 30.0
        assert not provider.mock_mode
    
    def test_mock_mode_initialization(self):
        """Test provider with mock mode enabled"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            mock_mode=True,
        )
        
        assert provider.mock_mode is True
    
    def test_debug_mode(self):
        """Test provider with debug mode"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            debug=True,
        )
        
        assert provider.debug is True


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseLLMProviderGeneration:
    """Test text generation"""
    
    async def test_successful_generation(self):
        """Test successful text generation"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        response = await provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100,
        )
        
        assert isinstance(response, LLMResponse)
        assert response.text == "Test response"
        assert response.tokens_used == 100
        assert response.model == "test-model"
        assert response.provider == "openai"
        assert response.latency_ms > 0
        assert response.cost_usd > 0
    
    async def test_chat_generation(self):
        """Test chat completion"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        messages = [
            {"role": "user", "content": "Hello"},
            {"role": "assistant", "content": "Hi there!"},
            {"role": "user", "content": "How are you?"},
        ]
        
        response = await provider.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=100,
        )
        
        assert isinstance(response, LLMResponse)
        assert response.text == "Test response"
        assert provider.call_count == 1
    
    async def test_mock_mode_response(self):
        """Test mock mode returns mock response"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            mock_mode=True,
        )
        
        response = await provider.generate("Test prompt")
        
        assert "Mock response" in response.text
        assert response.metadata.get("mock") is True


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseLLMProviderRetry:
    """Test retry logic"""
    
    async def test_retry_on_rate_limit(self):
        """Test automatic retry on rate limit"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            should_fail=True,
            fail_count=2,  # Fail first 2 attempts
        )
        
        response = await provider.generate("Test prompt")
        
        # Should succeed on 3rd attempt
        assert provider.call_count == 3
        assert response.text == "Test response"
    
    async def test_retry_exhaustion(self):
        """Test all retries exhausted"""
        provider = ConcreteTestProvider(
            api_key="test_key",
            should_fail=True,
            fail_count=10,  # Fail all attempts
        )
        
        with pytest.raises(RateLimitError):
            await provider.generate("Test prompt")
        
        # Should try MAX_RETRIES times
        assert provider.call_count == provider.MAX_RETRIES
    
    async def test_no_retry_on_auth_error(self):
        """Test no retry on authentication error"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        async def failing_request(*args, **kwargs):
            raise AuthenticationError("Invalid API key")
        
        provider._make_request = failing_request
        
        with pytest.raises(AuthenticationError):
            await provider.generate("Test prompt")


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseLLMProviderMetrics:
    """Test metrics tracking"""
    
    async def test_metrics_initialization(self):
        """Test metrics are initialized to zero"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        metrics = provider.get_metrics()
        
        assert metrics["total_requests"] == 0
        assert metrics["successful_requests"] == 0
        assert metrics["failed_requests"] == 0
        assert metrics["total_tokens"] == 0
        assert metrics["total_cost_usd"] == 0.0
    
    async def test_metrics_on_success(self):
        """Test metrics updated on successful request"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        await provider.generate("Test prompt")
        
        metrics = provider.get_metrics()
        assert metrics["total_requests"] == 1
        assert metrics["successful_requests"] == 1
        assert metrics["failed_requests"] == 0
        assert metrics["total_tokens"] == 100
        assert metrics["total_cost_usd"] > 0
    
    async def test_metrics_on_failure(self):
        """Test metrics updated on failed request"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        async def failing_request(*args, **kwargs):
            raise ValueError("Test error")
        
        provider._make_request = failing_request
        
        with pytest.raises(ValueError):
            await provider.generate("Test prompt")
        
        metrics = provider.get_metrics()
        assert metrics["total_requests"] == 1
        assert metrics["successful_requests"] == 0
        assert metrics["failed_requests"] == 1
    
    async def test_reset_metrics(self):
        """Test metrics can be reset"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        await provider.generate("Test prompt")
        provider.reset_metrics()
        
        metrics = provider.get_metrics()
        assert metrics["total_requests"] == 0
        assert metrics["successful_requests"] == 0


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseLLMProviderRateLimit:
    """Test rate limiting"""
    
    async def test_rate_limit_tracking(self):
        """Test rate limit tracking"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        # Make several requests
        for _ in range(5):
            await provider.generate("Test prompt")
        
        # Check request times are tracked
        assert len(provider._request_times) == 5
    
    async def test_rate_limit_window(self):
        """Test rate limit window cleanup"""
        provider = ConcreteTestProvider(api_key="test_key")
        provider.RATE_LIMIT_WINDOW = 0.1  # 100ms window
        
        # Make request
        await provider.generate("Test prompt")
        
        # Wait for window to pass
        await asyncio.sleep(0.2)
        
        # Make another request
        await provider.generate("Test prompt")
        
        # Old request should be removed
        assert len(provider._request_times) == 1


@pytest.mark.unit
def test_cost_calculation():
    """Test cost calculation"""
    provider = ConcreteTestProvider(api_key="test_key")
    
    # 1000 tokens at $0.01 per 1K = $0.01
    cost = provider._calculate_cost(1000)
    assert cost == 0.01
    
    # 500 tokens at $0.01 per 1K = $0.005
    cost = provider._calculate_cost(500)
    assert cost == 0.005


@pytest.mark.unit
def test_llm_response_to_dict():
    """Test LLMResponse to_dict method"""
    response = LLMResponse(
        text="Test response",
        tokens_used=100,
        cost_usd=0.001,
        model="test-model",
        provider="test-provider",
        latency_ms=150.0,
        metadata={"key": "value"},
    )
    
    response_dict = response.to_dict()
    
    assert response_dict["text"] == "Test response"
    assert response_dict["tokens_used"] == 100
    assert response_dict["cost_usd"] == 0.001
    assert response_dict["model"] == "test-model"
    assert response_dict["provider"] == "test-provider"
    assert response_dict["latency_ms"] == 150.0
    assert response_dict["metadata"] == {"key": "value"}


@pytest.mark.unit
def test_provider_type_enum():
    """Test ProviderType enum values"""
    assert ProviderType.AZURE_OPENAI.value == "azure_openai"
    assert ProviderType.OPENAI.value == "openai"
    assert ProviderType.ANTHROPIC.value == "anthropic"
    assert ProviderType.DEEPSEEK.value == "deepseek"
    assert ProviderType.OLLAMA.value == "ollama"


@pytest.mark.unit
def test_custom_exceptions():
    """Test custom exception types"""
    # RateLimitError with retry_after
    error = RateLimitError("Rate limit", retry_after=30.0)
    assert error.retry_after == 30.0
    assert "Rate limit" in str(error)
    
    # Other errors
    assert isinstance(AuthenticationError("test"), LLMProviderError)
    assert isinstance(ModelNotFoundError("test"), LLMProviderError)
    assert isinstance(TimeoutError("test"), LLMProviderError)
    assert isinstance(ContentFilterError("test"), LLMProviderError)


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseLLMProviderEdgeCases:
    """Test edge cases and error conditions"""
    
    async def test_empty_prompt(self):
        """Test generation with empty prompt"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        response = await provider.generate("")
        
        assert isinstance(response, LLMResponse)
    
    async def test_very_long_prompt(self):
        """Test generation with very long prompt"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        long_prompt = "test " * 10000
        response = await provider.generate(long_prompt)
        
        assert isinstance(response, LLMResponse)
    
    async def test_zero_max_tokens(self):
        """Test generation with zero max_tokens"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        response = await provider.generate("Test", max_tokens=0)
        
        assert isinstance(response, LLMResponse)
    
    async def test_invalid_temperature(self):
        """Test generation with invalid temperature"""
        provider = ConcreteTestProvider(api_key="test_key")
        
        # Should still work (provider may clamp values)
        response = await provider.generate("Test", temperature=2.0)
        assert isinstance(response, LLMResponse)