"""
Test suite for Azure OpenAI Provider.

Tests cover:
- Provider initialization with Azure-specific configuration
- Azure endpoint and deployment handling
- API request handling
- Error handling
- Token counting and cost calculation
- Metrics tracking
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
from llm.providers.azure_openai_provider import AzureOpenAIProvider


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def azure_provider():
    """Create Azure OpenAI provider instance."""
    return AzureOpenAIProvider(
        api_key="test_api_key",
        endpoint="https://test.openai.azure.com",
        deployment="gpt-4-deployment",
        api_version="2024-02-15-preview",
        timeout=30.0,
        mock_mode=False
    )


@pytest.fixture
def mock_azure_response():
    """Create mock Azure OpenAI API response."""
    response = MagicMock()
    response.choices = [MagicMock()]
    response.choices[0].message.content = "Test response from Azure GPT-4"
    response.choices[0].finish_reason = "stop"
    response.usage = MagicMock()
    response.usage.prompt_tokens = 10
    response.usage.completion_tokens = 15
    response.model = "gpt-4"
    return response


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_azure_provider_initialization():
    """Test Azure OpenAI provider initializes correctly."""
    provider = AzureOpenAIProvider(
        api_key="test_key",
        endpoint="https://example.openai.azure.com",
        deployment="my-deployment",
        api_version="2024-02-15-preview",
        timeout=60.0
    )
    
    assert provider.provider_type == ProviderType.AZURE_OPENAI
    assert provider.model == "my-deployment"
    assert provider.api_key == "test_key"
    assert provider.endpoint == "https://example.openai.azure.com"
    assert provider.deployment == "my-deployment"
    assert provider.api_version == "2024-02-15-preview"
    assert provider.timeout == 60.0


@pytest.mark.unit
def test_azure_provider_default_api_version():
    """Test default API version."""
    provider = AzureOpenAIProvider(
        api_key="test_key",
        endpoint="https://example.openai.azure.com",
        deployment="my-deployment"
    )
    
    assert provider.api_version == "2024-02-15-preview"


# ============================================================================
# GENERATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success(azure_provider, mock_azure_response):
    """Test successful generation."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_azure_response)
    ):
        response = await azure_provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from Azure GPT-4"
    assert response.tokens_used == 25  # 10 + 15
    assert response.model == "gpt-4-deployment"
    assert response.provider == "azure_openai"
    assert response.cost_usd > 0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_chat_success(azure_provider, mock_azure_response):
    """Test successful chat completion."""
    messages = [
        {"role": "system", "content": "You are helpful"},
        {"role": "user", "content": "Hello"}
    ]
    
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_azure_response)
    ) as mock_create:
        response = await azure_provider.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=100
        )
        
        # Verify deployment was used
        call_kwargs = mock_create.call_args[1]
        assert call_kwargs["model"] == "gpt-4-deployment"
    
    assert response.text == "Test response from Azure GPT-4"
    assert response.metadata["deployment"] == "gpt-4-deployment"


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_authentication_error(azure_provider):
    """Test handling of authentication errors."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.AuthenticationError(
            message="Invalid credentials",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(AuthenticationError):
            await azure_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_rate_limit_error(azure_provider):
    """Test handling of rate limit errors."""
    error = openai.RateLimitError(
        message="Rate limit exceeded",
        response=MagicMock(),
        body=None
    )
    error.retry_after = 5.0
    
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=error)
    ):
        with pytest.raises(RateLimitError) as exc_info:
            await azure_provider.generate("Test")
        
        assert exc_info.value.retry_after == 5.0


@pytest.mark.unit
@pytest.mark.asyncio
async def test_deployment_not_found_error(azure_provider):
    """Test handling of deployment not found errors."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.NotFoundError(
            message="Deployment not found",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ModelNotFoundError) as exc_info:
            await azure_provider.generate("Test")
        
        assert "gpt-4-deployment" in str(exc_info.value)


@pytest.mark.unit
@pytest.mark.asyncio
async def test_timeout_error(azure_provider):
    """Test handling of timeout errors."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.APITimeoutError(request=MagicMock()))
    ):
        with pytest.raises(LLMTimeoutError):
            await azure_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_content_filter_error(azure_provider):
    """Test handling of Azure content filter errors."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(side_effect=openai.BadRequestError(
            message="content_filter triggered",
            response=MagicMock(),
            body=None
        ))
    ):
        with pytest.raises(ContentFilterError):
            await azure_provider.generate("Test")


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost(azure_provider):
    """Test cost calculation."""
    cost = azure_provider._calculate_cost(1000)
    
    # Should use averaged pricing
    expected = (1000 / 1000) * (
        (azure_provider.COST_PER_1K_INPUT_TOKENS +
         azure_provider.COST_PER_1K_OUTPUT_TOKENS) / 2
    )
    assert cost == expected


# ============================================================================
# METRICS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_tracking(azure_provider, mock_azure_response):
    """Test that metrics are tracked correctly."""
    with patch.object(
        azure_provider.client.chat.completions,
        'create',
        new=AsyncMock(return_value=mock_azure_response)
    ):
        await azure_provider.generate("Test")
    
    metrics = azure_provider.get_metrics()
    
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
    provider = AzureOpenAIProvider(
        api_key="test_key",
        endpoint="https://test.openai.azure.com",
        deployment="test-deployment",
        mock_mode=True
    )
    
    response = await provider.generate("Test")
    
    assert "Mock response" in response.text
    assert response.tokens_used > 0
    assert response.metadata["mock"] is True