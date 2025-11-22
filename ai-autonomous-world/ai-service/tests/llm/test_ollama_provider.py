"""
Test suite for Ollama Provider.

Tests cover:
- Local provider initialization (no API key)
- HTTP-based API communication
- Message formatting for Ollama
- Chat and generate API endpoints
- Fallback between endpoints
- Zero-cost calculation
- Error handling
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch, Mock
import pytest
import aiohttp

from llm.base_provider import (
    LLMProviderError,
    ModelNotFoundError,
    ProviderType,
    TimeoutError as LLMTimeoutError,
)
from llm.providers.ollama_provider import OllamaProvider


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def ollama_provider():
    """Create Ollama provider instance."""
    return OllamaProvider(
        model="llama2",
        base_url="http://localhost:11434",
        timeout=30.0,
        mock_mode=False
    )


@pytest.fixture
def mock_ollama_chat_response():
    """Create mock Ollama chat API response."""
    return {
        "message": {
            "role": "assistant",
            "content": "Test response from Llama2"
        },
        "model": "llama2",
        "created_at": "2024-01-01T00:00:00Z",
        "done": True,
        "total_duration": 1000000000,
        "load_duration": 100000000,
        "prompt_eval_count": 10,
        "eval_count": 15
    }


@pytest.fixture
def mock_ollama_generate_response():
    """Create mock Ollama generate API response."""
    return {
        "response": "Test response from Llama2",
        "model": "llama2",
        "done": True
    }


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
def test_ollama_provider_initialization():
    """Test Ollama provider initializes correctly."""
    provider = OllamaProvider(
        model="mistral",
        base_url="http://localhost:11434",
        timeout=120.0
    )
    
    assert provider.provider_type == ProviderType.OLLAMA
    assert provider.model == "mistral"
    assert provider.api_key is None  # No API key needed
    assert provider.base_url == "http://localhost:11434"
    assert provider.timeout == 120.0


@pytest.mark.unit
def test_ollama_provider_default_model():
    """Test default model selection."""
    provider = OllamaProvider()
    
    assert provider.model == "llama2"


@pytest.mark.unit
def test_ollama_provider_base_url_stripping():
    """Test that base URL trailing slash is stripped."""
    provider = OllamaProvider(
        base_url="http://localhost:11434/"
    )
    
    assert provider.base_url == "http://localhost:11434"


# ============================================================================
# MESSAGE FORMATTING TESTS
# ============================================================================

@pytest.mark.unit
def test_format_messages_single_user():
    """Test message formatting for single user message."""
    provider = OllamaProvider()
    
    messages = [{"role": "user", "content": "Hello"}]
    formatted = provider._format_messages_for_ollama(messages)
    
    assert formatted == "Hello"


@pytest.mark.unit
def test_format_messages_conversation():
    """Test message formatting for multi-turn conversation."""
    provider = OllamaProvider()
    
    messages = [
        {"role": "system", "content": "You are helpful"},
        {"role": "user", "content": "Hello"},
        {"role": "assistant", "content": "Hi!"},
        {"role": "user", "content": "How are you?"}
    ]
    
    formatted = provider._format_messages_for_ollama(messages)
    
    assert "System: You are helpful" in formatted
    assert "User: Hello" in formatted
    assert "Assistant: Hi!" in formatted
    assert formatted.endswith("Assistant:")


# ============================================================================
# GENERATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_success_chat_api(ollama_provider, mock_ollama_chat_response):
    """Test successful generation using chat API."""
    async def mock_post(*args, **kwargs):
        mock_response = AsyncMock()
        mock_response.status = 200
        mock_response.json = AsyncMock(return_value=mock_ollama_chat_response)
        return mock_response
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post
        
        response = await ollama_provider.generate(
            prompt="Test prompt",
            temperature=0.7,
            max_tokens=100
        )
    
    assert response.text == "Test response from Llama2"
    assert response.tokens_used > 0
    assert response.model == "llama2"
    assert response.provider == "ollama"
    assert response.cost_usd == 0.0  # Local execution is free


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_fallback_to_generate_api(ollama_provider, mock_ollama_generate_response):
    """Test fallback to generate API when chat API returns 404."""
    call_count = [0]
    
    async def mock_post(url, *args, **kwargs):
        mock_response = AsyncMock()
        call_count[0] += 1
        
        if "/api/chat" in url and call_count[0] == 1:
            # First call to chat API returns 404
            mock_response.status = 404
            mock_response.text = AsyncMock(return_value="Not found")
        else:
            # Fallback to generate API succeeds
            mock_response.status = 200
            mock_response.json = AsyncMock(return_value=mock_ollama_generate_response)
        
        return mock_response
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post
        
        response = await ollama_provider.generate("Test")
    
    assert response.text == "Test response from Llama2"
    assert call_count[0] == 2  # Both endpoints tried


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_connection_error(ollama_provider):
    """Test handling of connection errors."""
    async def mock_post_error(*args, **kwargs):
        raise aiohttp.ClientError("Connection refused")
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post_error
        
        with pytest.raises(LLMProviderError) as exc_info:
            await ollama_provider.generate("Test")
        
        assert "Failed to connect" in str(exc_info.value)
        assert "Is Ollama running?" in str(exc_info.value)


@pytest.mark.unit
@pytest.mark.asyncio
async def test_timeout_error(ollama_provider):
    """Test handling of timeout errors."""
    async def mock_post_timeout(*args, **kwargs):
        raise aiohttp.ClientTimeout()
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post_timeout
        
        with pytest.raises(LLMTimeoutError):
            await ollama_provider.generate("Test")


@pytest.mark.unit
@pytest.mark.asyncio
async def test_model_not_found_generate_api(ollama_provider):
    """Test handling of model not found in generate API."""
    async def mock_post(*args, **kwargs):
        mock_response = AsyncMock()
        if "/api/chat" in args[0]:
            # Chat API not available
            mock_response.status = 404
            mock_response.text = AsyncMock(return_value="Not found")
        else:
            # Generate API also returns 404 (model not found)
            mock_response.status = 404
            mock_response.text = AsyncMock(return_value="Model not found")
        return mock_response
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post
        
        with pytest.raises(ModelNotFoundError) as exc_info:
            await ollama_provider.generate("Test")
        
        assert "llama2" in str(exc_info.value)
        assert "ollama pull" in str(exc_info.value)


@pytest.mark.unit
@pytest.mark.asyncio
async def test_api_error_response(ollama_provider):
    """Test handling of API error responses."""
    async def mock_post(*args, **kwargs):
        mock_response = AsyncMock()
        mock_response.status = 500
        mock_response.text = AsyncMock(return_value="Internal server error")
        return mock_response
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post
        
        with pytest.raises(LLMProviderError) as exc_info:
            await ollama_provider.generate("Test")
        
        assert "500" in str(exc_info.value)


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost_always_zero():
    """Test that Ollama cost is always zero (local execution)."""
    provider = OllamaProvider()
    
    # Any token count should be zero cost
    assert provider._calculate_cost(1000) == 0.0
    assert provider._calculate_cost(10000) == 0.0
    assert provider._calculate_cost(100000) == 0.0


# ============================================================================
# METRICS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_metrics_tracking(ollama_provider, mock_ollama_chat_response):
    """Test that metrics are tracked correctly."""
    async def mock_post(*args, **kwargs):
        mock_response = AsyncMock()
        mock_response.status = 200
        mock_response.json = AsyncMock(return_value=mock_ollama_chat_response)
        return mock_response
    
    with patch('aiohttp.ClientSession') as mock_session:
        mock_session.return_value.__aenter__.return_value.post = mock_post
        
        await ollama_provider.generate("Test")
    
    metrics = ollama_provider.get_metrics()
    
    assert metrics["total_requests"] == 1
    assert metrics["successful_requests"] == 1
    assert metrics["total_cost_usd"] == 0.0  # Always free


# ============================================================================
# MOCK MODE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_mock_mode():
    """Test provider in mock mode."""
    provider = OllamaProvider(
        model="llama2",
        mock_mode=True
    )
    
    response = await provider.generate("Test")
    
    assert "Mock response" in response.text
    assert response.tokens_used > 0
    assert response.metadata["mock"] is True
    assert response.cost_usd == 0.0


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("model", [
    "llama2",
    "mistral",
    "codellama",
    "phi",
])
def test_different_models(model):
    """Test initialization with different models."""
    provider = OllamaProvider(model=model)
    assert provider.model == model