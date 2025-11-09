"""
LLM Provider Factory
"""

from typing import Dict, Any, Optional
from loguru import logger

from .base import BaseLLMProvider
from .providers.openai_provider import OpenAIProvider
from .providers.azure_openai_provider import AzureOpenAIProvider
from .providers.anthropic_provider import AnthropicProvider
from .providers.google_provider import GoogleProvider
from .providers.deepseek_provider import DeepSeekProvider


class LLMProviderFactory:
    """Factory for creating LLM provider instances"""

    _providers = {
        "openai": OpenAIProvider,
        "azure_openai": AzureOpenAIProvider,
        "anthropic": AnthropicProvider,
        "google": GoogleProvider,
        "gemini": GoogleProvider,  # Alias
        "deepseek": DeepSeekProvider,
    }

    # Class-level cache for provider instances
    _instances: Dict[str, BaseLLMProvider] = {}

    def __init__(self):
        """Initialize factory"""
        pass
    
    def create_provider(
        self,
        provider_name: str,
        config: Dict[str, Any]
    ) -> BaseLLMProvider:
        """
        Create LLM provider instance

        Args:
            provider_name: Name of provider (openai, anthropic, google)
            config: Provider configuration

        Returns:
            LLM provider instance

        Raises:
            ValueError: If provider name is not supported
        """
        provider_name = provider_name.lower()

        if provider_name not in self._providers:
            available = ", ".join(self._providers.keys())
            raise ValueError(f"Unknown provider: {provider_name}. Available: {available}")

        provider_class = self._providers[provider_name]

        try:
            provider = provider_class(config)
            logger.info(f"Created {provider_name} provider")
            return provider
        except Exception as e:
            logger.error(f"Failed to create {provider_name} provider: {e}")
            raise

    @classmethod
    def get_or_create_provider(
        cls,
        provider_name: str,
        config: Dict[str, Any]
    ) -> BaseLLMProvider:
        """
        Get existing provider instance or create new one

        Args:
            provider_name: Name of provider
            config: Provider configuration

        Returns:
            LLM provider instance (cached)
        """
        provider_name = provider_name.lower()

        if provider_name not in cls._instances:
            # Create instance of factory to call instance method
            factory = cls()
            cls._instances[provider_name] = factory.create_provider(provider_name, config)

        return cls._instances[provider_name]

    @classmethod
    def clear_cache(cls):
        """Clear cached provider instances"""
        cls._instances.clear()
        logger.info("Cleared LLM provider cache")


def get_llm_provider(
    provider_name: Optional[str] = None,
    config: Optional[Dict[str, Any]] = None
) -> BaseLLMProvider:
    """
    Get LLM provider instance

    Args:
        provider_name: Name of provider. If None, uses default from settings.
        config: Provider configuration. If None, uses settings.

    Returns:
        LLM provider instance
    """
    from config import settings
    
    # Use default provider if not specified
    if provider_name is None:
        provider_name = settings.default_llm_provider
    
    # Build config from settings if not provided
    if config is None:
        config = {}

        if provider_name == "openai":
            if not settings.openai_api_key:
                raise ValueError("OpenAI API key is required but not configured. Set OPENAI_API_KEY environment variable.")
            config = {
                "api_key": settings.openai_api_key,
                "model": settings.openai_model,
                "temperature": settings.openai_temperature,
                "max_tokens": settings.openai_max_tokens,
            }
        elif provider_name == "azure_openai":
            if not settings.azure_openai_api_key:
                raise ValueError("Azure OpenAI API key is required but not configured. Set AZURE_OPENAI_API_KEY environment variable.")
            if not settings.azure_openai_endpoint:
                raise ValueError("Azure OpenAI endpoint is required but not configured. Set AZURE_OPENAI_ENDPOINT environment variable.")
            config = {
                "api_key": settings.azure_openai_api_key,
                "endpoint": settings.azure_openai_endpoint,
                "deployment": settings.azure_openai_deployment,
                "api_version": settings.azure_openai_api_version,
                "temperature": settings.openai_temperature,  # Reuse OpenAI temperature setting
                "max_tokens": settings.openai_max_tokens,  # Reuse OpenAI max_tokens setting
            }
        elif provider_name == "anthropic":
            if not settings.anthropic_api_key:
                raise ValueError("Anthropic API key is required but not configured. Set ANTHROPIC_API_KEY environment variable.")
            config = {
                "api_key": settings.anthropic_api_key,
                "model": settings.anthropic_model,
                "temperature": settings.anthropic_temperature,
                "max_tokens": settings.anthropic_max_tokens,
            }
        elif provider_name in ["google", "gemini"]:
            if not settings.google_api_key:
                raise ValueError("Google API key is required but not configured. Set GOOGLE_API_KEY environment variable.")
            config = {
                "api_key": settings.google_api_key,
                "model": settings.google_model,
                "temperature": settings.google_temperature,
                "max_tokens": settings.google_max_tokens,
            }
        elif provider_name == "deepseek":
            if not settings.deepseek_api_key:
                raise ValueError("DeepSeek API key is required but not configured. Set DEEPSEEK_API_KEY environment variable.")
            config = {
                "api_key": settings.deepseek_api_key,
                "base_url": settings.deepseek_base_url,
                "model": settings.deepseek_model,
                "temperature": settings.deepseek_temperature,
                "max_tokens": settings.deepseek_max_tokens,
            }

    # Validate API key is present in config
    if "api_key" in config and not config["api_key"]:
        raise ValueError(f"API key for provider '{provider_name}' is empty or None")

    return LLMProviderFactory.get_or_create_provider(provider_name, config)

