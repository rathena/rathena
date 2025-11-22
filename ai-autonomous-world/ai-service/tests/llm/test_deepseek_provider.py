"""
Test suite for DeepSeek Provider.

Tests cover:
- Provider initialization
- OpenAI-compatible API integration
- Cost-effective pricing model
- Error handling
- Token counting and cost calculation
- Custom base URL configuration
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
import pytest
import openai

from llm.base_provider import (
    AuthenticationError,
    ContentFilterError,
    ModelNotFoundError,
    ProviderType,
    RateLimitError,
    TimeoutError as LLMTimeoutError,
)
from llm.providers.deepseek_provider import DeepSeekProvider


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def deepseek_provider():
    """Create DeepSeek provider instance."""
    return DeepSeekProvider(
        api_key="test_api_key",
        model="deepseek-chat",
        timeout=30.0,
        mock_mode=False
    )


@pytest.fixture
def mock_deepseek_response():
    """Create mock DeepSeek API response."""
    response = MagicMock()
    response.choices = [MagicMock()]
    response.choices[0].message.content = "Test response from DeepSeek"
    response.choices[0].finish_reason = "stop"
    response.usage = MagicMock()
    response.usage.prompt_tokens = 10
    response.usage.completion_tokens = 15
    response.model = "deepseek-chat"
    return response


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_deepseek_provider_initialization():
    """Test DeepSeek provider initializes correctly."""
    provider = DeepSeekProvider(
        api_key="test_key",
        model="deepseek-coder",
        timeout=60.0
    )
    
    assert provider.provider_type == ProviderType.DEEPSEEK
    assert provider.model == "deepseek-coder"
    assert provider.api_key == "test_key"
    assert provider.base_url == "https://api.deepseek.com/v1"
    assert provider.timeout == 60.0


@pytest.mark.unit
def test_deepseek_provider_custom_base_url():
    """Test custom base URL configuration."""
    custom_url = "https://custom.deepseek.com"
    provider = DeepSeekProvider(
        api_key="test_key",
        model="deepseek-chat",
        base_url=custom_url
    )
    
    assert provider.base_url == custom_url


@pytest.mark.unit
def test_deepseek_provider_pricing():
    """Test that DeepSeek pricing is cost-effective."""
    provider = DeepSeekProvider(api_key="key", model="deepseek-chat")
    
    # DeepSeek should be significantly cheaper than OpenAI
    assert provider.COST_PER_1K_INPUT_TOKENS == 0.00014
    assert provider.COST_PER_1K_OUTPUT_TOKENS == 0.00028
    
    # Both models should have same pricing
    coder = DeepSeekProvider(api_key="key", model="deepseek-coder")
    assert coder.COST_PER_1K_INPUT_TOKENS == provider.COST_PER_1K_INPUT_TOKENS


# ============================================================================
# GENERATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success(deepseek_provider, mock_deepseek_response):
    """Test successful generation."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_deepseek_response)
    ):
        response = await deepseek_provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from DeepSeek"
    assert response.tokens_used == 25  # 10 + 15
    assert response.model == "deepseek-chat"
    assert response.provider == "deepseek"
    assert response.cost_usd >= 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_success(deepseek_provider, mock_deepseek_response):
    """Test successful chat completion."""
    messages = [
        {"role": "user", "content": "Hello"},
        {"role": "assistant", "content": "Hi!"},
        {"role": "user", "content": "How are you?"}
    ]
    
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_deepseek_response)
    ):
        response = await deepseek_provider.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from DeepSeek"
    assert response.tokens_used > 0


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_authentication_error(deepseek_provider):
    """Test handling of authentication errors."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.AuthenticationError(
            message="Invalid API key",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(AuthenticationError):
            await deepseek_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_rate_limit_error(deepseek_provider):
    """Test handling of rate limit errors."""
    error = openai.RateLimitError(
        message="Rate limit exceeded",
        response=MagicMock(),
        body=None
    )
    error.retry_after = 3.0
    
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=error)
    ):
        with pytest.raises(RateLimitError) as exc_info:
            await deepseek_provider.generate("Test")
        
        assert exc_info.value.retry_after == 3.0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_model_not_found_error(deepseek_provider):
    """Test handling of model not found errors."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.NotFoundError(
            message="Model not found",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ModelNotFoundError):
            await deepseek_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_timeout_error(deepseek_provider):
    """Test handling of timeout errors."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.APITimeoutError(request=MagicMock()))
    ):
        with pytest.raises(LLMTimeoutError):
            await deepseek_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_content_filter_error(deepseek_provider):
    """Test handling of content filter errors."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.BadRequestError(
            message="content violation",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ContentFilterError):
            await deepseek_provider.generate("Test")


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost(deepseek_provider):
    """Test cost calculation for DeepSeek."""
    cost = deepseek_provider._calculate_cost(1000)
    
    # Should use averaged pricing
    expected = (1000 / 1000) * (
        (deepseek_provider.COST_PER_1K_INPUT_TOKENS +
         deepseek_provider.COST_PER_1K_OUTPUT_TOKENS) / 2
    )
    assert cost == expected
    
    # DeepSeek should be very cheap
    assert cost < 0.001  # Less than $0.001 for 1000 tokens


@pytest.mark.unit
def test_deepseek_cost_effectiveness():
    """Test that DeepSeek is cost-effective compared to other providers."""
    deepseek = DeepSeekProvider(api_key="key", model="deepseek-chat")
    
    # Calculate cost for 10k tokens
    cost_10k = deepseek._calculate_cost(10000)
    
    # Should be less than $0.01 for 10k tokens
    assert cost_10k < 0.01


# ============================================================================
# METRICS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_tracking(deepseek_provider, mock_deepseek_response):
    """Test that metrics are tracked correctly."""
    with patch.object(
        deepseek_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_deepseek_response)
    ):
        await deepseek_provider.generate("Test")
    
    metrics = deepseek_provider.get_metrics()
    
    assert metrics["total_requests"] == 1
    assert metrics["successful_requests"] == 1
    assert metrics["total_tokens"] > 0


# ============================================================================
# MOCK MODE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_mock_mode():
    """Test provider in mock mode."""
    provider = DeepSeekProvider(
        api_key="test_key",
        model="deepseek-chat",
        mock_mode=True
    )
    
    response = await provider.generate("Test")
    
    assert "Mock response" in response.text
    assert response.tokens_used > 0
    assert response.metadata["mock"] is True