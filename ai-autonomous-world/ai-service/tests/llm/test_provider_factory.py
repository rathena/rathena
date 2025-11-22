"""
Test suite for LLM Provider Factory.

Tests cover:
- Factory initialization and configuration
- Fallback chain logic
- Circuit breaker pattern
- Provider health tracking
- Auto-failover between providers
- Provider priority and selection
"""

import asyncio
import time
from unittest.mock import AsyncMock, MagicMock, patch
import pytest

from llm.base_provider import (
    LLMProviderError,
    LLMResponse,
    ProviderType,
    RateLimitError,
)
from llm.factory import (
    LLMProviderFactory,
    CircuitBreakerState,
    ProviderHealth
)


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def factory_config():
    """Create factory configuration."""
    return {
        "OPENAI_API_KEY": "test_openai_key",
        "ANTHROPIC_API_KEY": "test_anthropic_key",
        "DEEPSEEK_API_KEY": "test_deepseek_key",
    }


@pytest.fixture
def mock_llm_response():
    """Create mock LLM response."""
    return LLMResponse(
        text="Test response",
        tokens_used=25,
        cost_usd=0.001,
        model="test-model",
        provider="test",
        latency_ms=100.0,
        metadata={}
    )


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_factory_initialization():
    """Test factory initializes with available providers."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test_key',
        'ANTHROPIC_API_KEY': 'test_key'
    }):
        factory = LLMProviderFactory()
        
        available = factory.get_available_providers()
        
        # Should have at least Ollama (always available)
        assert len(available) > 0
        assert ProviderType.OLLAMA in available


@pytest.mark.unit
def test_factory_initialization_with_config(factory_config):
    """Test factory initialization with config dict."""
    with patch('llm.factory.OpenAIProvider'), \
         patch('llm.factory.AnthropicProvider'), \
         patch('llm.factory.DeepSeekProvider'), \
         patch('llm.factory.OllamaProvider'):
        
        factory = LLMProviderFactory(config=factory_config)
        
        # Should attempt to initialize providers based on config
        assert factory.config == factory_config


@pytest.mark.unit
def test_factory_fallback_chain_order():
    """Test that fallback chain is in correct priority order."""
    factory = LLMProviderFactory()
    
    expected_order = [
        ProviderType.AZURE_OPENAI,
        ProviderType.OPENAI,
        ProviderType.ANTHROPIC,
        ProviderType.DEEPSEEK,
        ProviderType.OLLAMA,
    ]
    
    assert factory._fallback_chain == expected_order


# ============================================================================
# PROVIDER SELECTION TESTS
# ============================================================================

@pytest.mark.unit
def test_get_provider_priority_default():
    """Test default provider priority."""
    factory = LLMProviderFactory()
    
    priority = factory._get_provider_priority()
    
    # Should return fallback chain in order
    assert priority == factory._fallback_chain


@pytest.mark.unit
def test_get_provider_priority_with_preferred():
    """Test provider priority with preferred provider."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}):
        factory = LLMProviderFactory()
        
        # Prefer Anthropic
        priority = factory._get_provider_priority(ProviderType.ANTHROPIC)
        
        # Anthropic should be first
        assert priority[0] == ProviderType.ANTHROPIC


@pytest.mark.unit
def test_get_specific_provider():
    """Test getting specific provider instance."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_provider, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        factory = LLMProviderFactory()
        
        provider = factory.get_provider(ProviderType.OPENAI)
        
        assert provider is not None


# ============================================================================
# GENERATE WITH FALLBACK TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success_first_provider(mock_llm_response):
    """Test successful generation on first provider."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # Mock OpenAI provider
        mock_instance = MagicMock()
        mock_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_openai.return_value = mock_instance
        
        factory = LLMProviderFactory()
        
        response = await factory.generate("Test prompt")
        
        assert response.text == "Test response"
        assert mock_instance.chat.called


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_fallback_on_failure(mock_llm_response):
    """Test fallback to next provider on failure."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test',
        'ANTHROPIC_API_KEY': 'test'
    }), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.anthropic_provider.AnthropicProvider') as mock_anthropic, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # OpenAI fails
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(side_effect=Exception("OpenAI error"))
        mock_openai.return_value = openai_instance
        
        # Anthropic succeeds
        anthropic_instance = MagicMock()
        anthropic_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_anthropic.return_value = anthropic_instance
        
        factory = LLMProviderFactory()
        
        response = await factory.generate("Test prompt")
        
        # Should succeed with Anthropic
        assert response.text == "Test response"
        assert anthropic_instance.chat.called


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_all_providers_fail():
    """Test error when all providers fail."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.ollama_provider.OllamaProvider') as mock_ollama:
        
        # All providers fail
        for mock_provider in [mock_openai, mock_ollama]:
            instance = MagicMock()
            instance.chat = AsyncMock(side_effect=Exception("Provider error"))
            mock_provider.return_value = instance
        
        factory = LLMProviderFactory()
        
        with pytest.raises(LLMProviderError) as exc_info:
            await factory.generate("Test prompt")
        
        assert "All LLM providers failed" in str(exc_info.value)


# ============================================================================
# CIRCUIT BREAKER TESTS
# ============================================================================

@pytest.mark.unit
def test_circuit_breaker_opens_on_failures():
    """Test circuit breaker opens after threshold failures."""
    factory = LLMProviderFactory()
    provider_type = ProviderType.OPENAI
    
    # Initialize circuit breaker
    factory._init_health_tracking(provider_type)
    
    # Record multiple failures
    for _ in range(CircuitBreakerState.FAILURE_THRESHOLD):
        factory._record_failure(provider_type)
    
    # Circuit should be open
    assert factory._is_circuit_open(provider_type) is True


@pytest.mark.unit
def test_circuit_breaker_resets_after_cooldown():
    """Test circuit breaker resets after cooldown period."""
    factory = LLMProviderFactory()
    provider_type = ProviderType.OPENAI
    
    factory._init_health_tracking(provider_type)
    
    # Open circuit
    for _ in range(CircuitBreakerState.FAILURE_THRESHOLD):
        factory._record_failure(provider_type)
    
    # Simulate cooldown period passing
    breaker = factory._circuit_breakers[provider_type]
    breaker.last_failure_time = time.time() - (CircuitBreakerState.COOLDOWN_PERIOD + 1)
    
    # Circuit should reset
    assert factory._is_circuit_open(provider_type) is False
    assert breaker.failures == 0


@pytest.mark.unit
def test_circuit_breaker_resets_on_success():
    """Test circuit breaker resets on successful request."""
    factory = LLMProviderFactory()
    provider_type = ProviderType.OPENAI
    
    factory._init_health_tracking(provider_type)
    
    # Record some failures
    for _ in range(3):
        factory._record_failure(provider_type)
    
    # Record success
    factory._record_success(provider_type, 100.0)
    
    breaker = factory._circuit_breakers[provider_type]
    assert breaker.failures == 0
    assert breaker.is_open is False


@pytest.mark.unit
@pytest.mark.asyncio
async def test_circuit_breaker_skips_open_providers(mock_llm_response):
    """Test that open circuit breakers are skipped."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test',
        'ANTHROPIC_API_KEY': 'test'
    }), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.anthropic_provider.AnthropicProvider') as mock_anthropic, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # Setup providers
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_openai.return_value = openai_instance
        
        anthropic_instance = MagicMock()
        anthropic_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_anthropic.return_value = anthropic_instance
        
        factory = LLMProviderFactory()
        
        # Open circuit for OpenAI
        breaker = factory._circuit_breakers[ProviderType.OPENAI]
        breaker.is_open = True
        breaker.last_failure_time = time.time()
        
        response = await factory.generate("Test")
        
        # Should skip OpenAI and use Anthropic
        assert not openai_instance.chat.called
        assert anthropic_instance.chat.called


# ============================================================================
# HEALTH TRACKING TESTS
# ============================================================================

@pytest.mark.unit
def test_record_success_updates_health():
    """Test that successful requests update health metrics."""
    factory = LLMProviderFactory()
    provider_type = ProviderType.OPENAI
    
    factory._init_health_tracking(provider_type)
    
    factory._record_success(provider_type, 150.0)
    factory._record_success(provider_type, 100.0)
    
    health = factory._health[provider_type]
    assert health.success_count == 2
    assert health.average_latency > 0
    assert health.last_success_time is not None


@pytest.mark.unit
def test_record_failure_updates_health():
    """Test that failed requests update health metrics."""
    factory = LLMProviderFactory()
    provider_type = ProviderType.OPENAI
    
    factory._init_health_tracking(provider_type)
    
    factory._record_failure(provider_type)
    
    health = factory._health[provider_type]
    assert health.failure_count == 1
    assert health.last_failure_time is not None


@pytest.mark.unit
def test_health_success_rate_calculation():
    """Test health success rate calculation."""
    health = ProviderHealth()
    
    # No requests
    assert health.success_rate == 0.0
    
    # 70% success rate
    health.success_count = 7
    health.failure_count = 3
    assert health.success_rate == 0.7
    
    # 100% success
    health.success_count = 10
    health.failure_count = 0
    assert health.success_rate == 1.0


@pytest.mark.unit
def test_health_is_healthy_threshold():
    """Test health status threshold."""
    health = ProviderHealth()
    
    # Below threshold
    health.success_count = 6
    health.failure_count = 4
    assert health.is_healthy is False
    
    # At/above threshold (70%)
    health.success_count = 7
    health.failure_count = 3
    assert health.is_healthy is True


@pytest.mark.unit
def test_get_health_status():
    """Test comprehensive health status retrieval."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}), \
         patch('llm.providers.openai_provider.OpenAIProvider'), \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        factory = LLMProviderFactory()
        
        # Record some activity
        factory._record_success(ProviderType.OPENAI, 100.0)
        factory._record_failure(ProviderType.OLLAMA)
        
        status = factory.get_health_status()
        
        assert ProviderType.OPENAI.value in status
        assert status[ProviderType.OPENAI.value]["available"] is True
        assert status[ProviderType.OPENAI.value]["success_count"] == 1


# ============================================================================
# CHAT WITH FALLBACK TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_with_preferred_provider(mock_llm_response):
    """Test chat with preferred provider."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test',
        'ANTHROPIC_API_KEY': 'test'
    }), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.anthropic_provider.AnthropicProvider') as mock_anthropic, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # Setup providers
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_openai.return_value = openai_instance
        
        anthropic_instance = MagicMock()
        anthropic_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_anthropic.return_value = anthropic_instance
        
        factory = LLMProviderFactory()
        
        # Request with preferred Anthropic
        messages = [{"role": "user", "content": "Test"}]
        response = await factory.chat(
            messages,
            preferred_provider=ProviderType.ANTHROPIC
        )
        
        # Should use Anthropic first
        assert anthropic_instance.chat.called
        assert not openai_instance.chat.called


@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_fallback_chain(mock_llm_response):
    """Test complete fallback chain execution."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test',
        'ANTHROPIC_API_KEY': 'test',
        'DEEPSEEK_API_KEY': 'test'
    }), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.anthropic_provider.AnthropicProvider') as mock_anthropic, \
         patch('llm.providers.deepseek_provider.DeepSeekProvider') as mock_deepseek, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # OpenAI fails
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(side_effect=Exception("OpenAI down"))
        mock_openai.return_value = openai_instance
        
        # Anthropic fails
        anthropic_instance = MagicMock()
        anthropic_instance.chat = AsyncMock(side_effect=Exception("Anthropic down"))
        mock_anthropic.return_value = anthropic_instance
        
        # DeepSeek succeeds
        deepseek_instance = MagicMock()
        deepseek_instance.chat = AsyncMock(return_value=mock_llm_response)
        mock_deepseek.return_value = deepseek_instance
        
        factory = LLMProviderFactory()
        
        response = await factory.generate("Test")
        
        # Should eventually succeed with DeepSeek
        assert response.text == "Test response"
        assert deepseek_instance.chat.called


# ============================================================================
# METRICS AND HEALTH TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_updated_on_success(mock_llm_response):
    """Test that metrics are updated on successful requests."""
    with patch.dict('os.environ', {'OPENAI_API_KEY': 'test'}), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(return_value=mock_llm_response)
        openai_instance.get_metrics = MagicMock(return_value={
            "total_requests": 1,
            "successful_requests": 1,
            "total_tokens": 25
        })
        mock_openai.return_value = openai_instance
        
        factory = LLMProviderFactory()
        
        await factory.generate("Test")
        
        health = factory._health[ProviderType.OPENAI]
        assert health.success_count == 1
        assert health.failure_count == 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_updated_on_failure():
    """Test that metrics are updated on failed requests."""
    with patch.dict('os.environ', {
        'OPENAI_API_KEY': 'test',
        'ANTHROPIC_API_KEY': 'test'
    }), \
         patch('llm.providers.openai_provider.OpenAIProvider') as mock_openai, \
         patch('llm.providers.anthropic_provider.AnthropicProvider') as mock_anthropic, \
         patch('llm.providers.ollama_provider.OllamaProvider'):
        
        # OpenAI fails
        openai_instance = MagicMock()
        openai_instance.chat = AsyncMock(side_effect=Exception("Error"))
        mock_openai.return_value = openai_instance
        
        # Anthropic succeeds
        anthropic_instance = MagicMock()
        mock_response = MagicMock()
        mock_response.latency_ms = 100.0
        anthropic_instance.chat = AsyncMock(return_value=mock_response)
        mock_anthropic.return_value = anthropic_instance
        
        factory = LLMProviderFactory()
        
        await factory.chat([{"role": "user", "content": "Test"}])
        
        # OpenAI should have failure recorded
        openai_health = factory._health[ProviderType.OPENAI]
        assert openai_health.failure_count == 1


# ============================================================================
# CONFIGURATION TESTS
# ============================================================================

@pytest.mark.unit
def test_has_config_from_dict():
    """Test configuration detection from config dict."""
    config = {"TEST_KEY": "value"}
    factory = LLMProviderFactory(config=config)
    
    assert factory._has_config("TEST_KEY") is True
    assert factory._has_config("MISSING_KEY") is False


@pytest.mark.unit
def test_has_config_from_env():
    """Test configuration detection from environment."""
    with patch.dict('os.environ', {'TEST_KEY': 'value'}):
        factory = LLMProviderFactory()
        
        assert factory._has_config("TEST_KEY") is True


@pytest.mark.unit
def test_get_config_priority():
    """Test config retrieval priority (dict > env > default)."""
    with patch.dict('os.environ', {'TEST_KEY': 'env_value'}):
        config = {"TEST_KEY": "dict_value"}
        factory = LLMProviderFactory(config=config)
        
        # Should prefer dict over env
        assert factory._get_config("TEST_KEY") == "dict_value"
        
        # Should use default if not found
        assert factory._get_config("MISSING", default="default") == "default"


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("provider_type,env_var", [
    (ProviderType.OPENAI, "OPENAI_API_KEY"),
    (ProviderType.ANTHROPIC, "ANTHROPIC_API_KEY"),
    (ProviderType.DEEPSEEK, "DEEPSEEK_API_KEY"),
    (ProviderType.AZURE_OPENAI, "AZURE_OPENAI_API_KEY"),
])
def test_provider_initialization_requires_config(provider_type, env_var):
    """Test that providers require appropriate configuration."""
    # Without config, provider shouldn't be initialized
    factory = LLMProviderFactory()
    providers = factory.get_available_providers()
    
    # With Ollama always available, we check if API-based provider is missing
    if env_var != "OLLAMA":  # Ollama doesn't need API key
        assert provider_type not in providers or provider_type == ProviderType.OLLAMA