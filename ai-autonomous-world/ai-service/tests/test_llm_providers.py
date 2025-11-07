"""
Unit tests for LLM providers
"""

import pytest
from unittest.mock import AsyncMock, MagicMock, patch
from llm.factory import LLMProviderFactory
from llm.base import BaseLLMProvider, LLMResponse


class TestLLMProviderFactory:
    """Test LLM Provider Factory"""
    
    def test_factory_get_provider_openai(self, mock_settings):
        """Test factory creates OpenAI provider"""
        config = {
            "api_key": "test-key",
            "model": "gpt-4",
            "temperature": 0.7
        }
        
        with patch('llm.factory.OpenAIProvider') as mock_provider:
            mock_instance = MagicMock()
            mock_provider.return_value = mock_instance
            
            provider = LLMProviderFactory.get_or_create_provider("openai", config)
            
            assert provider is not None
            mock_provider.assert_called_once_with(config)
    
    def test_factory_get_provider_azure(self, mock_settings):
        """Test factory creates Azure OpenAI provider"""
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com/",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }
        
        with patch('llm.factory.AzureOpenAIProvider') as mock_provider:
            mock_instance = MagicMock()
            mock_provider.return_value = mock_instance
            
            provider = LLMProviderFactory.get_or_create_provider("azure_openai", config)
            
            assert provider is not None
            mock_provider.assert_called_once_with(config)
    
    def test_factory_api_key_validation(self):
        """Test factory validates API key"""
        config = {
            "api_key": "",  # Empty API key
            "model": "gpt-4"
        }
        
        with pytest.raises(ValueError, match="API key.*empty"):
            LLMProviderFactory.get_or_create_provider("openai", config)
    
    def test_factory_caching(self, mock_settings):
        """Test factory caches provider instances"""
        config = {
            "api_key": "test-key",
            "model": "gpt-4"
        }
        
        with patch('llm.factory.OpenAIProvider') as mock_provider:
            mock_instance = MagicMock()
            mock_provider.return_value = mock_instance
            
            # First call creates instance
            provider1 = LLMProviderFactory.get_or_create_provider("openai", config)
            
            # Second call returns cached instance
            provider2 = LLMProviderFactory.get_or_create_provider("openai", config)
            
            assert provider1 is provider2
            assert mock_provider.call_count == 1  # Only called once
    
    def test_factory_unsupported_provider(self):
        """Test factory raises error for unsupported provider"""
        with pytest.raises(ValueError, match="Unsupported LLM provider"):
            LLMProviderFactory.get_or_create_provider("invalid_provider", {})


class TestBaseLLMProvider:
    """Test Base LLM Provider"""
    
    def test_base_provider_abstract_methods(self):
        """Test base provider has abstract methods"""
        provider = BaseLLMProvider({"api_key": "test"})
        
        with pytest.raises(NotImplementedError):
            asyncio.run(provider.generate("test prompt"))
        
        with pytest.raises(NotImplementedError):
            asyncio.run(provider.generate_chat([{"role": "user", "content": "test"}]))
        
        with pytest.raises(NotImplementedError):
            asyncio.run(provider.generate_with_tools("test", []))


class TestAzureOpenAIProvider:
    """Test Azure OpenAI Provider"""
    
    @pytest.mark.asyncio
    async def test_azure_provider_initialization(self):
        """Test Azure provider initializes correctly"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com/",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }
        
        provider = AzureOpenAIProvider(config)
        
        assert provider.endpoint == "https://test.openai.azure.com/"
        assert provider.deployment == "gpt-4"
        assert provider.api_version == "2024-02-15-preview"
    
    @pytest.mark.asyncio
    async def test_azure_provider_generate(self):
        """Test Azure provider generate method"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com/",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }
        
        provider = AzureOpenAIProvider(config)
        
        with patch.object(provider.client.chat.completions, 'create') as mock_create:
            mock_response = MagicMock()
            mock_response.choices = [MagicMock(message=MagicMock(content="Test response"))]
            mock_response.usage = MagicMock(prompt_tokens=10, completion_tokens=20, total_tokens=30)
            mock_response.model = "gpt-4"
            mock_create.return_value = mock_response
            
            response = await provider.generate("Test prompt")
            
            assert response.text == "Test response"
            assert response.usage["total_tokens"] == 30
    
    @pytest.mark.asyncio
    async def test_azure_provider_retry_logic(self):
        """Test Azure provider retry logic with exponential backoff"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        from openai import RateLimitError
        
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com/",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview",
            "max_retries": 3
        }
        
        provider = AzureOpenAIProvider(config)
        
        with patch.object(provider.client.chat.completions, 'create') as mock_create:
            # Fail twice, succeed on third attempt
            mock_response = MagicMock()
            mock_response.choices = [MagicMock(message=MagicMock(content="Success"))]
            mock_response.usage = MagicMock(prompt_tokens=10, completion_tokens=20, total_tokens=30)
            mock_response.model = "gpt-4"
            
            mock_create.side_effect = [
                RateLimitError("Rate limit", response=MagicMock(status_code=429), body={}),
                RateLimitError("Rate limit", response=MagicMock(status_code=429), body={}),
                mock_response
            ]
            
            response = await provider.generate("Test prompt")
            
            assert response.text == "Success"
            assert mock_create.call_count == 3
    
    @pytest.mark.asyncio
    async def test_azure_provider_tools_api(self):
        """Test Azure provider uses tools API (not deprecated functions)"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        
        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com/",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }
        
        provider = AzureOpenAIProvider(config)
        
        tools = [
            {
                "type": "function",
                "function": {
                    "name": "test_function",
                    "description": "Test function",
                    "parameters": {}
                }
            }
        ]
        
        with patch.object(provider.client.chat.completions, 'create') as mock_create:
            mock_response = MagicMock()
            mock_response.choices = [MagicMock(message=MagicMock(content="Test", tool_calls=None))]
            mock_response.usage = MagicMock(prompt_tokens=10, completion_tokens=20, total_tokens=30)
            mock_response.model = "gpt-4"
            mock_create.return_value = mock_response
            
            response = await provider.generate_with_tools("Test prompt", tools)
            
            # Verify tools parameter is used (not deprecated functions)
            call_kwargs = mock_create.call_args[1]
            assert "tools" in call_kwargs
            assert "functions" not in call_kwargs

