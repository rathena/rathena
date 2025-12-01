"""
Test suite for OpenAI Provider.

Tests cover:
- Provider initialization with different configurations
- API request handling and response parsing
- Error handling (auth, rate limits, timeouts, model not found)
- Token counting and cost calculation
- Metrics tracking
- Mock mode for testing
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
import pytest
import openai

from llm.base_provider import (
    AuthenticationError,
    ContentFilterError,
    LLMProviderError,
    ModelNotFoundError,
    ProviderType,
    RateLimitError,
    TimeoutError as LLMTimeoutError,
)
from llm.providers.openai_provider import OpenAIProvider


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def openai_provider():
    """Create OpenAI provider instance."""
    return OpenAIProvider(
        api_key="test_api_key",
        model="gpt-4-turbo-preview",
        timeout=30.0,
        mock_mode=False,
        debug=False
    )


@pytest.fixture
def mock_openai_response():
    """Create mock OpenAI API response."""
    response = MagicMock()
    response.choices = [MagicMock()]
    response.choices[0].message.content = "Test response from GPT-4"
    response.choices[0].finish_reason = "stop"
    response.usage = MagicMock()
    response.usage.prompt_tokens = 10
    response.usage.completion_tokens = 15
    response.model = "gpt-4-turbo-preview"
    response.system_fingerprint = "fp_test"
    return response


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_openai_provider_initialization():
    """Test OpenAI provider initializes correctly."""
    provider = OpenAIProvider(
        api_key="test_key",
        model="gpt-3.5-turbo",
        organization="test_org",
        timeout=60.0
    )
    
    assert provider.provider_type == ProviderType.OPENAI
    assert provider.model == "gpt-3.5-turbo"
    assert provider.api_key == "test_key"
    assert provider.organization == "test_org"
    assert provider.timeout == 60.0


@pytest.mark.unit
def test_openai_provider_model_pricing():
    """Test that model-specific pricing is set correctly."""
    # GPT-4
    gpt4 = OpenAIProvider(api_key="key", model="gpt-4")
    assert gpt4.COST_PER_1K_INPUT_TOKENS == 0.03
    assert gpt4.COST_PER_1K_OUTPUT_TOKENS == 0.06
    
    # GPT-3.5-turbo
    gpt35 = OpenAIProvider(api_key="key", model="gpt-3.5-turbo")
    assert gpt35.COST_PER_1K_INPUT_TOKENS == 0.0005
    assert gpt35.COST_PER_1K_OUTPUT_TOKENS == 0.0015


# ============================================================================
# GENERATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success(openai_provider, mock_openai_response):
    """Test successful generation."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_openai_response)
    ):
        response = await openai_provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from GPT-4"
    assert response.tokens_used == 25  # 10 + 15
    assert response.model == "gpt-4-turbo-preview"
    assert response.provider == "openai"
    assert response.cost_usd > 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_success(openai_provider, mock_openai_response):
    """Test successful chat completion."""
    messages = [
        {"role": "system", "content": "You are helpful"},
        {"role": "user", "content": "Hello"}
    ]
    
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_openai_response)
    ):
        response = await openai_provider.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from GPT-4"
    assert response.tokens_used > 0
    assert response.metadata["finish_reason"] == "stop"


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_authentication_error(openai_provider):
    """Test handling of authentication errors."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.AuthenticationError(
            message="Invalid API key",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(AuthenticationError):
            await openai_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_rate_limit_error(openai_provider):
    """Test handling of rate limit errors."""
    error = openai.RateLimitError(
        message="Rate limit exceeded",
        response=MagicMock(),
        body=None
    )
    error.retry_after = 5.0
    
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=error)
    ):
        with pytest.raises(RateLimitError) as exc_info:
            await openai_provider.generate("Test")
        
        assert exc_info.value.retry_after == 5.0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_model_not_found_error(openai_provider):
    """Test handling of model not found errors."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.NotFoundError(
            message="Model not found",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ModelNotFoundError):
            await openai_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_timeout_error(openai_provider):
    """Test handling of timeout errors."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.APITimeoutError(request=MagicMock()))
    ):
        with pytest.raises(LLMTimeoutError):
            await openai_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_content_filter_error(openai_provider):
    """Test handling of content filter errors."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.BadRequestError(
            message="content_policy violation",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ContentFilterError):
            await openai_provider.generate("Test")


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost(openai_provider):
    """Test cost calculation."""
    cost = openai_provider._calculate_cost(1000)
    
    # Should use averaged pricing
    expected = (1000 / 1000) * (
        (openai_provider.COST_PER_1K_INPUT_TOKENS +
         openai_provider.COST_PER_1K_OUTPUT_TOKENS) / 2
    )
    assert cost == expected


# ============================================================================
# METRICS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_tracking(openai_provider, mock_openai_response):
    """Test that metrics are tracked correctly."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_openai_response)
    ):
        await openai_provider.generate("Test")
    
    metrics = openai_provider.get_metrics()
    
    assert metrics["total_requests"] == 1
    assert metrics["successful_requests"] == 1
    assert metrics["total_tokens"] > 0
    assert metrics["total_cost_usd"] > 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_track_failures(openai_provider):
    """Test that failed requests are tracked."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=Exception("Test error"))
    ):
        with pytest.raises(Exception):
            await openai_provider.generate("Test")
    
    metrics = openai_provider.get_metrics()
    
    assert metrics["total_requests"] == 1
    assert metrics["failed_requests"] == 1
    assert metrics["successful_requests"] == 0


# ============================================================================
# MOCK MODE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_mock_mode():
    """Test provider in mock mode."""
    provider = OpenAIProvider(
        api_key="test_key",
        model="gpt-4",
        mock_mode=True
    )
    
    response = await provider.generate("Test")
    
    assert "Mock response" in response.text
    assert response.tokens_used > 0
    assert response.metadata["mock"] is True


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("temperature,max_tokens", [
    (0.0, 50),
    (0.5, 100),
    (0.9, 500),
    (1.0, 1000),
])
@pytest.mark.asyncio
async def test_different_parameters(openai_provider, mock_openai_response, temperature, max_tokens):
    """Test generation with different parameters."""
    with patch.object(
        openai_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_openai_response)
    ) as mock_create:
        await openai_provider.generate(
            prompt="Test",
            temperature=temperature,
            max_tokens=max_tokens
        )
        
        # Verify parameters were passed correctly
        call_kwargs = mock_create.call_args[1]
        assert call_kwargs["temperature"] == temperature
        assert call_kwargs["max_tokens"] == max_tokens