"""
Tests for LLM Fallback Chain Implementation

Tests automatic provider fallback, recovery detection, and failure tracking.
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch
from llm.fallback_provider import FallbackLLMProvider
from llm.base import LLMResponse


@pytest.fixture
def mock_providers():
    """Create mock LLM providers for testing"""
    providers = []
    for i in range(3):
        provider = Mock()
        provider.generate = AsyncMock()
        provider.generate_chat = AsyncMock()
        provider.generate_structured = AsyncMock()
        providers.append(provider)
    return providers


@pytest.fixture
def provider_names():
    """Provider names for testing"""
    return ["provider_1", "provider_2", "provider_3"]


@pytest.fixture
def fallback_provider(mock_providers, provider_names):
    """Create FallbackLLMProvider instance for testing"""
    return FallbackLLMProvider(
        providers=mock_providers,
        provider_names=provider_names,
        max_failures=3,
        recovery_check_rate=0.0  # Disable recovery checks for predictable tests
    )


@pytest.mark.asyncio
async def test_successful_primary_provider(fallback_provider, mock_providers):
    """Test successful generation with primary provider"""
    # Setup
    expected_response = LLMResponse(
        content="Test response",
        model="test-model",
        provider="provider_1"
    )
    mock_providers[0].generate.return_value = expected_response
    
    # Execute
    result = await fallback_provider.generate(prompt="test prompt")
    
    # Verify
    assert result.content == "Test response"
    assert result.provider == "provider_1"
    mock_providers[0].generate.assert_called_once()
    mock_providers[1].generate.assert_not_called()
    mock_providers[2].generate.assert_not_called()


@pytest.mark.asyncio
async def test_fallback_on_primary_failure(fallback_provider, mock_providers):
    """Test automatic fallback when primary provider fails"""
    # Setup - primary fails, secondary succeeds
    mock_providers[0].generate.side_effect = Exception("Primary failed")
    expected_response = LLMResponse(
        content="Fallback response",
        model="test-model",
        provider="provider_2"
    )
    mock_providers[1].generate.return_value = expected_response
    
    # Execute
    result = await fallback_provider.generate(prompt="test prompt")
    
    # Verify
    assert result.content == "Fallback response"
    assert result.provider == "provider_2"
    mock_providers[0].generate.assert_called_once()
    mock_providers[1].generate.assert_called_once()
    mock_providers[2].generate.assert_not_called()
    
    # Verify current provider switched
    assert fallback_provider.get_current_provider_name() == "provider_2"


@pytest.mark.asyncio
async def test_multiple_provider_failures(fallback_provider, mock_providers):
    """Test fallback through multiple provider failures"""
    # Setup - first two fail, third succeeds
    mock_providers[0].generate.side_effect = Exception("Provider 1 failed")
    mock_providers[1].generate.side_effect = Exception("Provider 2 failed")
    expected_response = LLMResponse(
        content="Third provider response",
        model="test-model",
        provider="provider_3"
    )
    mock_providers[2].generate.return_value = expected_response
    
    # Execute
    result = await fallback_provider.generate(prompt="test prompt")
    
    # Verify
    assert result.content == "Third provider response"
    assert result.provider == "provider_3"
    assert fallback_provider.get_current_provider_name() == "provider_3"
    
    # Verify all providers were tried
    mock_providers[0].generate.assert_called_once()
    mock_providers[1].generate.assert_called_once()
    mock_providers[2].generate.assert_called_once()


@pytest.mark.asyncio
async def test_all_providers_fail(fallback_provider, mock_providers):
    """Test error when all providers in chain fail"""
    # Setup - all providers fail
    for provider in mock_providers:
        provider.generate.side_effect = Exception("Provider failed")
    
    # Execute & Verify
    with pytest.raises(RuntimeError, match="All .* providers in fallback chain failed"):
        await fallback_provider.generate(prompt="test prompt")
    
    # Verify all providers were tried
    for provider in mock_providers:
        provider.generate.assert_called_once()


@pytest.mark.asyncio
async def test_failure_threshold_switching(fallback_provider, mock_providers):
    """Test permanent switch after max failures threshold"""
    # Setup - primary fails 3 times (max_failures=3)
    mock_providers[0].generate.side_effect = Exception("Primary failed")
    expected_response = LLMResponse(
        content="Fallback response",
        model="test-model",
        provider="provider_2"
    )
    mock_providers[1].generate.return_value = expected_response
    
    # Execute - 3 failures
    for _ in range(3):
        await fallback_provider.generate(prompt="test prompt")
    
    # Verify switched to provider_2 permanently
    assert fallback_provider.current_index == 1
    assert fallback_provider.get_current_provider_name() == "provider_2"
    assert fallback_provider.failure_counts[0] >= 3


@pytest.mark.asyncio
async def test_failure_count_reset_on_success(fallback_provider, mock_providers):
    """Test failure count resets on successful generation"""
    # Setup - fail once then succeed
    expected_response = LLMResponse(
        content="Test response",
        model="test-model",
        provider="provider_1"
    )
    
    # First call fails
    mock_providers[0].generate.side_effect = [
        Exception("Temporary failure"),
        expected_response
    ]
    
    # Execute first call (fails, tries provider_2)
    mock_providers[1].generate.return_value = expected_response
    await fallback_provider.generate(prompt="test prompt 1")
    
    # Verify failure count increased
    assert fallback_provider.failure_counts[0] == 1
    
    # Execute second call (succeeds with provider_1)
    await fallback_provider.generate(prompt="test prompt 2")
    
    # Verify failure count reset
    assert fallback_provider.failure_counts[0] == 0


@pytest.mark.asyncio
async def test_generate_chat_fallback(fallback_provider, mock_providers):
    """Test fallback works for generate_chat method"""
    # Setup
    mock_providers[0].generate_chat.side_effect = Exception("Primary failed")
    expected_response = LLMResponse(
        content="Chat fallback response",
        model="test-model",
        provider="provider_2"
    )
    mock_providers[1].generate_chat.return_value = expected_response
    
    messages = [{"role": "user", "content": "test"}]
    
    # Execute
    result = await fallback_provider.generate_chat(messages=messages)
    
    # Verify
    assert result.content == "Chat fallback response"
    assert fallback_provider.get_current_provider_name() == "provider_2"


@pytest.mark.asyncio
async def test_generate_structured_fallback(fallback_provider, mock_providers):
    """Test fallback works for generate_structured method"""
    # Setup
    mock_providers[0].generate_structured.side_effect = Exception("Primary failed")
    expected_result = {"key": "value"}
    mock_providers[1].generate_structured.return_value = expected_result
    
    schema = {"type": "object"}
    
    # Execute
    result = await fallback_provider.generate_structured(
        prompt="test prompt",
        schema=schema
    )
    
    # Verify
    assert result == expected_result
    assert fallback_provider.get_current_provider_name() == "provider_2"


@pytest.mark.asyncio
async def test_primary_recovery_check(mock_providers, provider_names):
    """Test primary provider recovery detection"""
    # Create fallback with recovery checks enabled
    fallback = FallbackLLMProvider(
        providers=mock_providers,
        provider_names=provider_names,
        max_failures=3,
        recovery_check_rate=1.0  # Always check for predictable test
    )
    
    # Setup - primary fails initially, secondary succeeds
    mock_providers[0].generate.side_effect = [
        Exception("Primary failed"),
        LLMResponse(content="Recovery test", model="test", provider="provider_1"),
        LLMResponse(content="Recovered", model="test", provider="provider_1")
    ]
    mock_providers[1].generate.return_value = LLMResponse(
        content="Secondary response",
        model="test",
        provider="provider_2"
    )
    
    # First call fails primary, uses secondary
    await fallback.generate(prompt="test 1")
    assert fallback.current_index == 1  # Switched to provider_2
    
    # Second call checks recovery and switches back to primary
    result = await fallback.generate(prompt="test 2")
    
    # Verify recovered to primary
    assert result.content == "Recovered"
    assert fallback.current_index == 0
    assert fallback.get_current_provider_name() == "provider_1"


def test_get_fallback_stats(fallback_provider):
    """Test fallback statistics retrieval"""
    # Execute
    stats = fallback_provider.get_fallback_stats()
    
    # Verify
    assert stats["current_provider"] == "provider_1"
    assert stats["current_index"] == 0
    assert stats["provider_chain"] == ["provider_1", "provider_2", "provider_3"]
    assert stats["total_providers"] == 3
    assert stats["max_failures_threshold"] == 3
    assert stats["recovery_check_rate"] == 0.0
    assert "failure_counts" in stats


def test_invalid_initialization():
    """Test error handling for invalid initialization"""
    # Empty providers list
    with pytest.raises(ValueError, match="At least one provider required"):
        FallbackLLMProvider(providers=[], provider_names=[])
    
    # Mismatched lengths
    with pytest.raises(ValueError, match="same length"):
        FallbackLLMProvider(
            providers=[Mock()],
            provider_names=["p1", "p2"]
        )


@pytest.mark.asyncio
async def test_concurrent_fallback_calls(fallback_provider, mock_providers):
    """Test fallback provider handles concurrent calls correctly"""
    import asyncio
    
    # Setup
    expected_response = LLMResponse(
        content="Concurrent response",
        model="test-model",
        provider="provider_1"
    )
    mock_providers[0].generate.return_value = expected_response
    
    # Execute concurrent calls
    tasks = [
        fallback_provider.generate(prompt=f"test {i}")
        for i in range(10)
    ]
    results = await asyncio.gather(*tasks)
    
    # Verify all succeeded
    assert len(results) == 10
    for result in results:
        assert result.content == "Concurrent response"
    
    # Verify provider was called for each request
    assert mock_providers[0].generate.call_count == 10


@pytest.mark.asyncio
async def test_fallback_with_custom_parameters(fallback_provider, mock_providers):
    """Test fallback preserves custom generation parameters"""
    # Setup
    expected_response = LLMResponse(
        content="Custom params response",
        model="test-model",
        provider="provider_1"
    )
    mock_providers[0].generate.return_value = expected_response
    
    # Execute with custom parameters
    result = await fallback_provider.generate(
        prompt="test prompt",
        system_prompt="custom system",
        temperature=0.5,
        max_tokens=1000
    )
    
    # Verify parameters were passed through
    call_kwargs = mock_providers[0].generate.call_args.kwargs
    assert call_kwargs["prompt"] == "test prompt"
    assert call_kwargs["system_prompt"] == "custom system"
    assert call_kwargs["temperature"] == 0.5
    assert call_kwargs["max_tokens"] == 1000