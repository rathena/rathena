"""
Test suite for Anthropic (Claude) Provider.

Tests cover:
- Provider initialization
- Message format conversion (system messages)
- API request handling and response parsing
- Error handling
- Token counting and cost calculation
- Metrics tracking
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
import pytest
import anthropic

from llm.base_provider import (
    AuthenticationError,
    ContentFilterError,
    ModelNotFoundError,
    ProviderType,
    RateLimitError,
    TimeoutError as LLMTimeoutError,
)
from llm.providers.anthropic_provider import AnthropicProvider


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def anthropic_provider():
    """Create Anthropic provider instance."""
    return AnthropicProvider(
        api_key="test_api_key",
        model="claude-3-5-sonnet-20241022",
        timeout=30.0,
        mock_mode=False
    )


@pytest.fixture
def mock_anthropic_response():
    """Create mock Anthropic API response."""
    response = MagicMock()
    response.content = [MagicMock()]
    response.content[0].text = "Test response from Claude"
    response.stop_reason = "end_turn"
    response.usage = MagicMock()
    response.usage.input_tokens = 12
    response.usage.output_tokens = 18
    response.model = "claude-3-5-sonnet-20241022"
    return response


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_anthropic_provider_initialization():
    """Test Anthropic provider initializes correctly."""
    provider = AnthropicProvider(
        api_key="test_key",
        model="claude-3-opus-20240229",
        timeout=60.0
    )
    
    assert provider.provider_type == ProviderType.ANTHROPIC
    assert provider.model == "claude-3-opus-20240229"
    assert provider.api_key == "test_key"
    assert provider.timeout == 60.0


@pytest.mark.unit
def test_anthropic_provider_model_pricing():
    """Test that model-specific pricing is set correctly."""
    # Claude 3.5 Sonnet
    sonnet = AnthropicProvider(api_key="key", model="claude-3-5-sonnet-20241022")
    assert sonnet.COST_PER_1K_INPUT_TOKENS == 0.003
    assert sonnet.COST_PER_1K_OUTPUT_TOKENS == 0.015
    
    # Claude 3 Opus
    opus = AnthropicProvider(api_key="key", model="claude-3-opus-20240229")
    assert opus.COST_PER_1K_INPUT_TOKENS == 0.015
    assert opus.COST_PER_1K_OUTPUT_TOKENS == 0.075
    
    # Claude 3 Haiku
    haiku = AnthropicProvider(api_key="key", model="claude-3-haiku-20240307")
    assert haiku.COST_PER_1K_INPUT_TOKENS == 0.00025
    assert haiku.COST_PER_1K_OUTPUT_TOKENS == 0.00125


# ============================================================================
# MESSAGE CONVERSION TESTS
# ============================================================================

@pytest.mark.unit
def test_convert_messages_with_system():
    """Test message conversion with system message."""
    provider = AnthropicProvider(api_key="test", model="claude-3-5-sonnet-20241022")
    
    messages = [
        {"role": "system", "content": "You are helpful"},
        {"role": "user", "content": "Hello"},
        {"role": "assistant", "content": "Hi there"},
        {"role": "user", "content": "How are you?"}
    ]
    
    system_msg, anthropic_msgs = provider._convert_messages(messages)
    
    assert system_msg == "You are helpful"
    assert len(anthropic_msgs) == 3  # Excludes system message
    assert anthropic_msgs[0]["role"] == "user"
    assert anthropic_msgs[1]["role"] == "assistant"


@pytest.mark.unit
def test_convert_messages_no_system():
    """Test message conversion without system message."""
    provider = AnthropicProvider(api_key="test", model="claude-3-5-sonnet-20241022")
    
    messages = [
        {"role": "user", "content": "Hello"}
    ]
    
    system_msg, anthropic_msgs = provider._convert_messages(messages)
    
    assert system_msg is None
    assert len(anthropic_msgs) == 1


# ============================================================================
# GENERATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success(anthropic_provider, mock_anthropic_response):
    """Test successful generation."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(return_value=mock_anthropic_response)
    ):
        response = await anthropic_provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from Claude"
    assert response.tokens_used == 30  # 12 + 18
    assert response.model == "claude-3-5-sonnet-20241022"
    assert response.provider == "anthropic"
    assert response.cost_usd > 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_with_system_message(anthropic_provider, mock_anthropic_response):
    """Test chat with system message."""
    messages = [
        {"role": "system", "content": "You are a helpful assistant"},
        {"role": "user", "content": "Hello"}
    ]
    
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(return_value=mock_anthropic_response)
    ) as mock_create:
        response = await anthropic_provider.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=100
        )
        
        # Verify system message was passed separately
        call_kwargs = mock_create.call_args[1]
        assert "system" in call_kwargs
        assert call_kwargs["system"] == "You are a helpful assistant"
    
    assert response.text == "Test response from Claude"


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_authentication_error(anthropic_provider):
    """Test handling of authentication errors."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(side_effect=anthropic.AuthenticationError(
            message="Invalid API key",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(AuthenticationError):
            await anthropic_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_rate_limit_error(anthropic_provider):
    """Test handling of rate limit errors."""
    error = anthropic.RateLimitError(
        message="Rate limit exceeded",
        response=MagicMock(),
        body=None
    )
    error.response = MagicMock()
    error.response.headers = {"retry-after": "10"}
    
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(side_effect=error)
    ):
        with pytest.raises(RateLimitError) as exc_info:
            await anthropic_provider.generate("Test")
        
        assert exc_info.value.retry_after == 10.0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_model_not_found_error(anthropic_provider):
    """Test handling of model not found errors."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(side_effect=anthropic.NotFoundError(
            message="Model not found",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ModelNotFoundError):
            await anthropic_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_timeout_error(anthropic_provider):
    """Test handling of timeout errors."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(side_effect=anthropic.APITimeoutError(request=MagicMock()))
    ):
        with pytest.raises(LLMTimeoutError):
            await anthropic_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_content_filter_error(anthropic_provider):
    """Test handling of content filter errors."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(side_effect=anthropic.BadRequestError(
            message="content policy violation",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ContentFilterError):
            await anthropic_provider.generate("Test")


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost(anthropic_provider):
    """Test cost calculation."""
    cost = anthropic_provider._calculate_cost(1000)
    
    # Should use averaged pricing
    expected = (1000 / 1000) * (
        (anthropic_provider.COST_PER_1K_INPUT_TOKENS +
         anthropic_provider.COST_PER_1K_OUTPUT_TOKENS) / 2
    )
    assert cost == expected


# ============================================================================
# METRICS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_tracking(anthropic_provider, mock_anthropic_response):
    """Test that metrics are tracked correctly."""
    with patch.object(
        anthropic_provider.client.messages,
        'create',
        new=AsyncMock(return_value=mock_anthropic_response)
    ):
        await anthropic_provider.generate("Test")
    
    metrics = anthropic_provider.get_metrics()
    
    assert metrics["total_requests"] == 1
    assert metrics["successful_requests"] == 1
    assert metrics["total_tokens"] > 0
    assert metrics["total_cost_usd"] > 0


# ============================================================================
# MOCK MODE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_mock_mode():
    """Test provider in mock mode."""
    provider = AnthropicProvider(
        api_key="test_key",
        model="claude-3-5-sonnet-20241022",
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
@pytest.mark.parametrize("model,expected_input_cost,expected_output_cost", [
    ("claude-3-5-sonnet-20241022", 0.003, 0.015),
    ("claude-3-opus-20240229", 0.015, 0.075),
    ("claude-3-haiku-20240307", 0.00025, 0.00125),
])
def test_model_pricing_parametrized(model, expected_input_cost, expected_output_cost):
    """Test pricing for different models."""
    provider = AnthropicProvider(api_key="test", model=model)
    assert provider.COST_PER_1K_INPUT_TOKENS == expected_input_cost
    assert provider.COST_PER_1K_OUTPUT_TOKENS == expected_output_cost