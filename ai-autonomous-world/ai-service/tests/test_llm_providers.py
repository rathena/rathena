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
        # Clear cache before test
        LLMProviderFactory.clear_cache()

        config = {
            "api_key": "test-key",
            "model": "gpt-4",
            "temperature": 0.7
        }

        with patch('llm.providers.openai_provider.AsyncOpenAI') as mock_openai_client:
            mock_client = MagicMock()
            mock_openai_client.return_value = mock_client

            provider = LLMProviderFactory.get_or_create_provider("openai", config)

            assert provider is not None
            assert provider.__class__.__name__ == "OpenAIProvider"

    def test_factory_get_provider_azure(self, mock_settings):
        """Test factory creates Azure OpenAI provider"""
        # Clear cache before test
        LLMProviderFactory.clear_cache()

        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }

        with patch('llm.providers.azure_openai_provider.AsyncAzureOpenAI') as mock_azure_client:
            mock_client = MagicMock()
            mock_azure_client.return_value = mock_client

            provider = LLMProviderFactory.get_or_create_provider("azure_openai", config)

            assert provider is not None
            assert provider.__class__.__name__ == "AzureOpenAIProvider"

    def test_factory_api_key_validation(self):
        """Test factory validates API key in provider initialization"""
        # Clear cache before test
        LLMProviderFactory.clear_cache()

        config = {
            "api_key": "",  # Empty API key
            "model": "gpt-4"
        }

        # The factory doesn't validate, but the provider should
        with pytest.raises(Exception):  # Provider will raise an error
            LLMProviderFactory.get_or_create_provider("openai", config)

    def test_factory_caching(self, mock_settings):
        """Test factory caches provider instances"""
        # Clear cache before test
        LLMProviderFactory.clear_cache()

        config = {
            "api_key": "test-key",
            "model": "gpt-4"
        }

        with patch('llm.providers.openai_provider.AsyncOpenAI') as mock_openai_client:
            mock_client = MagicMock()
            mock_openai_client.return_value = mock_client

            # First call creates instance
            provider1 = LLMProviderFactory.get_or_create_provider("openai", config)

            # Second call returns cached instance
            provider2 = LLMProviderFactory.get_or_create_provider("openai", config)

            assert provider1 is provider2
            # OpenAI client should only be created once
            assert mock_openai_client.call_count == 1

    def test_factory_unsupported_provider(self):
        """Test factory raises error for unsupported provider"""
        with pytest.raises(ValueError, match="Unknown provider"):
            LLMProviderFactory.get_or_create_provider("invalid_provider", {})


class TestBaseLLMProvider:
    """Test Base LLM Provider"""

    def test_base_provider_abstract_methods(self):
        """Test base provider is abstract and cannot be instantiated"""
        # BaseLLMProvider is abstract and cannot be instantiated directly
        with pytest.raises(TypeError, match="Can't instantiate abstract class"):
            provider = BaseLLMProvider({"api_key": "test"})

    def test_base_provider_concrete_methods(self):
        """Test base provider concrete methods using a test implementation"""
        # Create a concrete test implementation
        class TestLLMProvider(BaseLLMProvider):
            async def generate(self, prompt, system_prompt=None, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_chat(self, messages, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test chat response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_structured(self, prompt, schema, system_prompt=None, **kwargs):
                return {"result": "test"}

        # Test with custom config
        config = {
            "model": "custom-model",
            "temperature": 0.5,
            "max_tokens": 1000
        }
        provider = TestLLMProvider(config)

        # Test getters
        assert provider.get_model_name() == "custom-model"
        assert provider.get_default_temperature() == 0.5
        assert provider.get_default_max_tokens() == 1000

    def test_base_provider_default_values(self):
        """Test base provider default values when not in config"""
        # Create a concrete test implementation
        class TestLLMProvider(BaseLLMProvider):
            async def generate(self, prompt, system_prompt=None, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_chat(self, messages, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test chat response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_structured(self, prompt, schema, system_prompt=None, **kwargs):
                return {"result": "test"}

        # Test with empty config
        provider = TestLLMProvider({})

        # Test default values
        assert provider.get_model_name() == "unknown"
        assert provider.get_default_temperature() == 0.7
        assert provider.get_default_max_tokens() == 2000

    @pytest.mark.asyncio
    async def test_base_provider_health_check_success(self):
        """Test base provider health check success"""
        # Create a concrete test implementation
        class TestLLMProvider(BaseLLMProvider):
            async def generate(self, prompt, system_prompt=None, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="OK",
                    model="test-model",
                    provider="test",
                    tokens_used=1
                )

            async def generate_chat(self, messages, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test chat response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_structured(self, prompt, schema, system_prompt=None, **kwargs):
                return {"result": "test"}

        provider = TestLLMProvider({})

        # Health check should succeed
        result = await provider.health_check()
        assert result is True

    @pytest.mark.asyncio
    async def test_base_provider_health_check_failure(self):
        """Test base provider health check failure"""
        # Create a concrete test implementation that fails
        class TestLLMProvider(BaseLLMProvider):
            async def generate(self, prompt, system_prompt=None, temperature=None, max_tokens=None, **kwargs):
                raise Exception("Connection failed")

            async def generate_chat(self, messages, temperature=None, max_tokens=None, **kwargs):
                return LLMResponse(
                    content="Test chat response",
                    model="test-model",
                    provider="test",
                    tokens_used=10
                )

            async def generate_structured(self, prompt, schema, system_prompt=None, **kwargs):
                return {"result": "test"}

        provider = TestLLMProvider({})

        # Health check should fail
        result = await provider.health_check()
        assert result is False

    @pytest.mark.asyncio
    async def test_base_provider_abstract_method_not_implemented(self):
        """Test calling abstract methods raises NotImplementedError"""
        # Create a partial implementation that doesn't override all methods
        class PartialLLMProvider(BaseLLMProvider):
            async def generate(self, prompt, system_prompt=None, temperature=None, max_tokens=None, **kwargs):
                # Call parent to trigger NotImplementedError
                return await super().generate(prompt, system_prompt, temperature, max_tokens, **kwargs)

            async def generate_chat(self, messages, temperature=None, max_tokens=None, **kwargs):
                # Call parent to trigger NotImplementedError
                return await super().generate_chat(messages, temperature, max_tokens, **kwargs)

            async def generate_structured(self, prompt, schema, system_prompt=None, **kwargs):
                # Call parent to trigger NotImplementedError
                return await super().generate_structured(prompt, schema, system_prompt, **kwargs)

        provider = PartialLLMProvider({})

        # Test that calling parent methods raises NotImplementedError
        with pytest.raises(NotImplementedError, match="must implement generate"):
            await provider.generate("test")

        with pytest.raises(NotImplementedError, match="must implement generate_chat"):
            await provider.generate_chat([{"role": "user", "content": "test"}])

        with pytest.raises(NotImplementedError, match="must implement generate_structured"):
            await provider.generate_structured("test", {})


class TestAzureOpenAIProvider:
    """Test Azure OpenAI Provider"""
    
    @pytest.mark.asyncio
    async def test_azure_provider_initialization(self):
        """Test Azure provider initializes correctly"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider

        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }

        with patch('llm.providers.azure_openai_provider.AsyncAzureOpenAI') as mock_azure_client:
            mock_client = MagicMock()
            mock_azure_client.return_value = mock_client

            provider = AzureOpenAIProvider(config)

            # Endpoint is stored without trailing slash
            assert provider.endpoint == "https://test.openai.azure.com"
            assert provider.deployment == "gpt-4"
            assert provider.api_version == "2024-02-15-preview"

    @pytest.mark.asyncio
    async def test_azure_provider_generate(self):
        """Test Azure provider generate method"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider

        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }

        with patch('llm.providers.azure_openai_provider.AsyncAzureOpenAI') as mock_azure_client:
            # Create mock client with proper async structure
            mock_client = MagicMock()
            mock_completion = MagicMock()
            mock_completion.choices = [MagicMock(message=MagicMock(content="Test response"), finish_reason="stop")]
            mock_completion.usage = MagicMock(prompt_tokens=10, completion_tokens=20, total_tokens=30)
            mock_completion.model = "gpt-4"
            mock_completion.id = "test-id-123"

            # Make the create method async
            mock_create = AsyncMock(return_value=mock_completion)
            mock_client.chat.completions.create = mock_create
            mock_azure_client.return_value = mock_client

            provider = AzureOpenAIProvider(config)
            response = await provider.generate("Test prompt")

            assert response.content == "Test response"
            assert response.tokens_used == 30
            assert response.provider == "azure_openai"
            assert response.model == "gpt-4"
    
    @pytest.mark.asyncio
    async def test_azure_provider_retry_logic(self):
        """Test Azure provider retry logic with exponential backoff"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider
        from openai import RateLimitError

        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview",
            "max_retries": 3
        }

        with patch('llm.providers.azure_openai_provider.AsyncAzureOpenAI') as mock_azure_client:
            # Create mock client with proper async structure
            mock_client = MagicMock()
            mock_completion = MagicMock()
            mock_completion.choices = [MagicMock(message=MagicMock(content="Success"), finish_reason="stop")]
            mock_completion.usage = MagicMock(prompt_tokens=10, completion_tokens=20, total_tokens=30)
            mock_completion.model = "gpt-4"
            mock_completion.id = "test-id-123"

            # Make the create method async with side effects
            mock_create = AsyncMock(side_effect=[
                RateLimitError("Rate limit", response=MagicMock(status_code=429), body={}),
                RateLimitError("Rate limit", response=MagicMock(status_code=429), body={}),
                mock_completion
            ])
            mock_client.chat.completions.create = mock_create
            mock_azure_client.return_value = mock_client

            provider = AzureOpenAIProvider(config)
            response = await provider.generate("Test prompt")

            assert response.content == "Success"
            assert response.tokens_used == 30
            assert mock_create.call_count == 3

    @pytest.mark.asyncio
    async def test_azure_provider_tools_api(self):
        """Test Azure provider structured output using tools API"""
        from llm.providers.azure_openai_provider import AzureOpenAIProvider

        config = {
            "api_key": "test-key",
            "endpoint": "https://test.openai.azure.com",
            "deployment": "gpt-4",
            "api_version": "2024-02-15-preview"
        }

        with patch('llm.providers.azure_openai_provider.AsyncAzureOpenAI') as mock_azure_client:
            # Create mock client
            mock_client = MagicMock()

            # Mock tool call response
            mock_tool_call = MagicMock()
            mock_tool_call.function.arguments = '{"result": "test"}'

            mock_message = MagicMock()
            mock_message.tool_calls = [mock_tool_call]

            mock_completion = MagicMock()
            mock_completion.choices = [MagicMock(message=mock_message)]

            # Make the create method async
            mock_create = AsyncMock(return_value=mock_completion)
            mock_client.chat.completions.create = mock_create
            mock_azure_client.return_value = mock_client

            provider = AzureOpenAIProvider(config)

            # Test structured output
            schema = {"type": "object", "properties": {"result": {"type": "string"}}}
            response = await provider.generate_structured("Test prompt", schema)

            assert response == {"result": "test"}
            assert isinstance(response, dict)

