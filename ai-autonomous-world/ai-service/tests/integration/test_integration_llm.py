"""
LLM Provider Integration Tests

Tests LLM provider switching, fallback chains, and error handling:
- Provider switching (OpenAI → Anthropic → Google)
- Automatic fallback on provider failure
- Circuit breaker patterns
- Response validation
- Cost tracking across providers
- Concurrent request handling
- Rate limit handling
"""

import asyncio
import pytest
from datetime import datetime
from typing import Dict, List, Any
from unittest.mock import AsyncMock, MagicMock, patch
import time

from llm.factory import LLMProviderFactory
from llm.base_provider import (
    BaseLLMProvider,
    LLMResponse,
    LLMProviderError,
    RateLimitError,
    AuthenticationError,
    TimeoutError,
    ProviderType
)


@pytest.fixture
def mock_openai_provider():
    """Mock OpenAI provider"""
    provider = MagicMock(spec=BaseLLMProvider)
    provider.provider_type = ProviderType.OPENAI
    provider.model = "gpt-4"
    
    async def mock_chat(*args, **kwargs):
        return LLMResponse(
            text="Response from OpenAI GPT-4",
            tokens_used=50,
            cost_usd=0.0025,
            model="gpt-4",
            provider="openai",
            latency_ms=150.0,
            metadata={"finish_reason": "stop"}
        )
    
    provider.chat = AsyncMock(side_effect=mock_chat)
    provider.generate = AsyncMock(side_effect=mock_chat)
    return provider


@pytest.fixture
def mock_anthropic_provider():
    """Mock Anthropic provider"""
    provider = MagicMock(spec=BaseLLMProvider)
    provider.provider_type = ProviderType.ANTHROPIC
    provider.model = "claude-3-sonnet"
    
    async def mock_chat(*args, **kwargs):
        return LLMResponse(
            text="Response from Anthropic Claude",
            tokens_used=45,
            cost_usd=0.0020,
            model="claude-3-sonnet",
            provider="anthropic",
            latency_ms=180.0,
            metadata={"stop_reason": "end_turn"}
        )
    
    provider.chat = AsyncMock(side_effect=mock_chat)
    provider.generate = AsyncMock(side_effect=mock_chat)
    return provider


@pytest.fixture
def mock_deepseek_provider():
    """Mock DeepSeek provider"""
    provider = MagicMock(spec=BaseLLMProvider)
    provider.provider_type = ProviderType.DEEPSEEK
    provider.model = "deepseek-chat"
    
    async def mock_chat(*args, **kwargs):
        return LLMResponse(
            text="Response from DeepSeek",
            tokens_used=40,
            cost_usd=0.0005,
            model="deepseek-chat",
            provider="deepseek",
            latency_ms=200.0,
            metadata={}
        )
    
    provider.chat = AsyncMock(side_effect=mock_chat)
    provider.generate = AsyncMock(side_effect=mock_chat)
    return provider


@pytest.fixture
def mock_ollama_provider():
    """Mock Ollama local provider"""
    provider = MagicMock(spec=BaseLLMProvider)
    provider.provider_type = ProviderType.OLLAMA
    provider.model = "llama3"
    
    async def mock_chat(*args, **kwargs):
        return LLMResponse(
            text="Response from local Ollama",
            tokens_used=35,
            cost_usd=0.0,
            model="llama3",
            provider="ollama",
            latency_ms=300.0,
            metadata={}
        )
    
    provider.chat = AsyncMock(side_effect=mock_chat)
    provider.generate = AsyncMock(side_effect=mock_chat)
    return provider


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestProviderSwitching:
    """Test switching between LLM providers"""
    
    async def test_successful_primary_provider(
        self,
        mock_openai_provider
    ):
        """Test successful request with primary provider"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        
        # Act
        response = await factory.generate(
            prompt="Test prompt",
            temperature=0.7
        )
        
        # Assert
        assert response is not None
        assert response.provider == "openai"
        assert response.text == "Response from OpenAI GPT-4"
        assert response.tokens_used == 50
        mock_openai_provider.chat.assert_called_once()
    
    async def test_fallback_to_secondary_provider(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test fallback when primary provider fails"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        
        # Make OpenAI fail
        mock_openai_provider.chat = AsyncMock(
            side_effect=Exception("OpenAI unavailable")
        )
        
        # Act
        response = await factory.chat(
            messages=[{"role": "user", "content": "Test"}],
            temperature=0.7
        )
        
        # Assert
        assert response is not None
        assert response.provider == "anthropic"
        assert response.text == "Response from Anthropic Claude"
        mock_openai_provider.chat.assert_called_once()
        mock_anthropic_provider.chat.assert_called_once()
    
    async def test_cascade_through_multiple_providers(
        self,
        mock_openai_provider,
        mock_anthropic_provider,
        mock_deepseek_provider
    ):
        """Test cascading through multiple provider failures"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider,
            ProviderType.DEEPSEEK: mock_deepseek_provider
        }
        
        # Make first two fail
        mock_openai_provider.chat = AsyncMock(side_effect=TimeoutError("Timeout"))
        mock_anthropic_provider.chat = AsyncMock(side_effect=RateLimitError("Rate limit"))
        
        # Act
        response = await factory.chat(
            messages=[{"role": "user", "content": "Test"}],
            temperature=0.7
        )
        
        # Assert
        assert response is not None
        assert response.provider == "deepseek"
        assert mock_openai_provider.chat.call_count == 1
        assert mock_anthropic_provider.chat.call_count == 1
        assert mock_deepseek_provider.chat.call_count == 1
    
    async def test_preferred_provider_priority(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test that preferred provider is tried first"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        
        # Act - Request with preferred provider
        response = await factory.chat(
            messages=[{"role": "user", "content": "Test"}],
            temperature=0.7,
            preferred_provider=ProviderType.ANTHROPIC
        )
        
        # Assert - Should use Anthropic first
        assert response.provider == "anthropic"
        mock_anthropic_provider.chat.assert_called_once()
        mock_openai_provider.chat.assert_not_called()


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestCircuitBreaker:
    """Test circuit breaker pattern for provider failures"""
    
    async def test_circuit_breaker_opens_after_failures(self):
        """Test circuit breaker opens after threshold failures"""
        # Arrange
        factory = LLMProviderFactory(config={})
        failing_provider = MagicMock(spec=BaseLLMProvider)
        failing_provider.provider_type = ProviderType.OPENAI
        failing_provider.chat = AsyncMock(side_effect=Exception("Provider error"))
        
        factory._providers = {ProviderType.OPENAI: failing_provider}
        
        # Act - Make multiple failing requests
        for _ in range(5):  # Circuit breaker threshold
            try:
                await factory.chat(
                    messages=[{"role": "user", "content": "Test"}],
                    temperature=0.7
                )
            except LLMProviderError:
                pass
        
        # Assert - Circuit should be open
        breaker = factory._circuit_breakers.get(ProviderType.OPENAI)
        assert breaker is not None
        assert breaker.is_open
        assert breaker.failures >= 5
    
    async def test_circuit_breaker_resets_after_cooldown(self):
        """Test circuit breaker resets after cooldown period"""
        # Arrange
        factory = LLMProviderFactory(config={})
        provider = MagicMock(spec=BaseLLMProvider)
        provider.provider_type = ProviderType.OPENAI
        provider.chat = AsyncMock(side_effect=Exception("Error"))
        
        factory._providers = {ProviderType.OPENAI: provider}
        
        # Open circuit
        for _ in range(5):
            try:
                await factory.chat([{"role": "user", "content": "Test"}])
            except:
                pass
        
        breaker = factory._circuit_breakers.get(ProviderType.OPENAI)
        assert breaker.is_open
        
        # Act - Simulate cooldown by manually resetting
        breaker.last_failure_time = time.time() - 31  # Past cooldown
        
        # Check if circuit can be tried again
        is_open = factory._is_circuit_open(ProviderType.OPENAI)
        
        # Assert - Circuit should be reset
        assert not is_open
        assert breaker.failures == 0
    
    async def test_successful_request_resets_circuit_breaker(self):
        """Test successful request resets circuit breaker"""
        # Arrange
        factory = LLMProviderFactory(config={})
        provider = MagicMock(spec=BaseLLMProvider)
        provider.provider_type = ProviderType.OPENAI
        
        # First fail, then succeed
        call_count = [0]
        async def mock_chat(*args, **kwargs):
            call_count[0] += 1
            if call_count[0] < 3:
                raise Exception("Temporary error")
            return LLMResponse(
                text="Success",
                tokens_used=10,
                cost_usd=0.001,
                model="gpt-4",
                provider="openai",
                latency_ms=100.0,
                metadata={}
            )
        
        provider.chat = AsyncMock(side_effect=mock_chat)
        factory._providers = {ProviderType.OPENAI: provider}
        factory._init_health_tracking(ProviderType.OPENAI)
        
        # Fail twice
        for _ in range(2):
            try:
                await factory.chat([{"role": "user", "content": "Test"}])
            except:
                pass
        
        breaker = factory._circuit_breakers.get(ProviderType.OPENAI)
        assert breaker.failures == 2
        
        # Act - Successful request
        response = await factory.chat([{"role": "user", "content": "Test"}])
        
        # Assert - Circuit breaker should be reset
        assert response.text == "Success"
        assert breaker.failures == 0
        assert not breaker.is_open


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestResponseValidation:
    """Test LLM response validation"""
    
    async def test_validate_response_structure(self, mock_openai_provider):
        """Test response has all required fields"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        
        # Act
        response = await factory.generate("Test prompt")
        
        # Assert
        assert hasattr(response, 'text')
        assert hasattr(response, 'tokens_used')
        assert hasattr(response, 'cost_usd')
        assert hasattr(response, 'model')
        assert hasattr(response, 'provider')
        assert hasattr(response, 'latency_ms')
        assert hasattr(response, 'metadata')
        
        assert isinstance(response.text, str)
        assert isinstance(response.tokens_used, int)
        assert isinstance(response.cost_usd, float)
        assert isinstance(response.latency_ms, float)
    
    async def test_validate_non_empty_response(self, mock_openai_provider):
        """Test response text is not empty"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        
        # Act
        response = await factory.generate("Test prompt")
        
        # Assert
        assert response.text is not None
        assert len(response.text) > 0
        assert response.tokens_used > 0
    
    async def test_validate_cost_calculation(self, mock_openai_provider):
        """Test cost is calculated correctly"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        
        # Act
        response = await factory.generate("Test prompt")
        
        # Assert
        assert response.cost_usd >= 0.0
        assert response.tokens_used > 0


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestErrorHandling:
    """Test error handling across providers"""
    
    async def test_all_providers_fail(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test when all providers fail"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        
        # Make all fail
        mock_openai_provider.chat = AsyncMock(side_effect=Exception("Error 1"))
        mock_anthropic_provider.chat = AsyncMock(side_effect=Exception("Error 2"))
        
        # Act & Assert
        with pytest.raises(LLMProviderError) as exc_info:
            await factory.chat([{"role": "user", "content": "Test"}])
        
        assert "All LLM providers failed" in str(exc_info.value)
    
    async def test_authentication_error_propagation(self):
        """Test authentication errors are not retried"""
        # Arrange
        factory = LLMProviderFactory(config={})
        provider = MagicMock(spec=BaseLLMProvider)
        provider.provider_type = ProviderType.OPENAI
        provider.chat = AsyncMock(side_effect=AuthenticationError("Invalid API key"))
        
        factory._providers = {ProviderType.OPENAI: provider}
        
        # Act & Assert
        with pytest.raises(AuthenticationError):
            await factory.chat([{"role": "user", "content": "Test"}])
        
        # Should only try once (no retries for auth errors)
        assert provider.chat.call_count == 1
    
    async def test_rate_limit_with_fallback(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test rate limit triggers fallback"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        
        # OpenAI hits rate limit
        mock_openai_provider.chat = AsyncMock(
            side_effect=RateLimitError("Rate limit exceeded", retry_after=1.0)
        )
        
        # Act
        response = await factory.chat([{"role": "user", "content": "Test"}])
        
        # Assert - Should fallback to Anthropic
        assert response.provider == "anthropic"
        mock_openai_provider.chat.assert_called_once()
        mock_anthropic_provider.chat.assert_called_once()


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestConcurrentRequests:
    """Test concurrent LLM requests"""
    
    async def test_concurrent_requests_same_provider(self, mock_openai_provider):
        """Test multiple concurrent requests to same provider"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        
        # Act - Make concurrent requests
        tasks = [
            factory.generate(f"Prompt {i}")
            for i in range(10)
        ]
        responses = await asyncio.gather(*tasks)
        
        # Assert
        assert len(responses) == 10
        assert all(r.provider == "openai" for r in responses)
        assert mock_openai_provider.chat.call_count == 10
    
    async def test_concurrent_requests_different_providers(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test concurrent requests across different providers"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        
        # Act - Concurrent requests with different preferred providers
        tasks = []
        for i in range(10):
            provider = ProviderType.OPENAI if i % 2 == 0 else ProviderType.ANTHROPIC
            tasks.append(
                factory.generate(
                    f"Prompt {i}",
                    preferred_provider=provider
                )
            )
        
        responses = await asyncio.gather(*tasks)
        
        # Assert
        assert len(responses) == 10
        openai_count = sum(1 for r in responses if r.provider == "openai")
        anthropic_count = sum(1 for r in responses if r.provider == "anthropic")
        
        assert openai_count == 5
        assert anthropic_count == 5


@pytest.mark.integration
@pytest.mark.llm
@pytest.mark.asyncio
class TestHealthMetrics:
    """Test provider health tracking"""
    
    async def test_health_metrics_tracking(self, mock_openai_provider):
        """Test health metrics are tracked correctly"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {ProviderType.OPENAI: mock_openai_provider}
        factory._init_health_tracking(ProviderType.OPENAI)
        
        # Act - Make successful requests
        for _ in range(5):
            await factory.generate("Test")
        
        # Get health status
        health = factory._health[ProviderType.OPENAI]
        
        # Assert
        assert health.success_count == 5
        assert health.failure_count == 0
        assert health.success_rate == 1.0
        assert health.is_healthy
    
    async def test_health_degradation_on_failures(self):
        """Test health degrades with failures"""
        # Arrange
        factory = LLMProviderFactory(config={})
        provider = MagicMock(spec=BaseLLMProvider)
        provider.provider_type = ProviderType.OPENAI
        
        call_count = [0]
        async def mixed_results(*args, **kwargs):
            call_count[0] += 1
            if call_count[0] % 3 == 0:  # Fail every 3rd request
                raise Exception("Intermittent error")
            return LLMResponse(
                text="Success",
                tokens_used=10,
                cost_usd=0.001,
                model="gpt-4",
                provider="openai",
                latency_ms=100.0,
                metadata={}
            )
        
        provider.chat = AsyncMock(side_effect=mixed_results)
        factory._providers = {ProviderType.OPENAI: provider}
        factory._init_health_tracking(ProviderType.OPENAI)
        
        # Act - Make requests with mixed results
        for _ in range(9):
            try:
                await factory.chat([{"role": "user", "content": "Test"}])
            except:
                pass
        
        # Get health
        health = factory._health[ProviderType.OPENAI]
        
        # Assert - Success rate should be ~66% (6 success, 3 failures)
        assert health.success_count == 6
        assert health.failure_count == 3
        assert 0.6 <= health.success_rate <= 0.7
    
    async def test_get_health_status_all_providers(
        self,
        mock_openai_provider,
        mock_anthropic_provider
    ):
        """Test getting health status for all providers"""
        # Arrange
        factory = LLMProviderFactory(config={})
        factory._providers = {
            ProviderType.OPENAI: mock_openai_provider,
            ProviderType.ANTHROPIC: mock_anthropic_provider
        }
        factory._init_health_tracking(ProviderType.OPENAI)
        factory._init_health_tracking(ProviderType.ANTHROPIC)
        
        # Make some requests
        await factory.generate("Test", preferred_provider=ProviderType.OPENAI)
        await factory.generate("Test", preferred_provider=ProviderType.ANTHROPIC)
        
        # Act
        status = factory.get_health_status()
        
        # Assert
        assert "openai" in status
        assert "anthropic" in status
        assert status["openai"]["available"]
        assert status["anthropic"]["available"]
        assert status["openai"]["success_count"] >= 1
        assert status["anthropic"]["success_count"] >= 1