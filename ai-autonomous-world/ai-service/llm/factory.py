"""
LLM Provider Factory
"""

from typing import Dict, Any, Optional
from loguru import logger

from .base import BaseLLMProvider
from .providers.openai_provider import OpenAIProvider
from .providers.anthropic_provider import AnthropicProvider
from .providers.google_provider import GoogleProvider


class LLMProviderFactory:
    """Factory for creating LLM provider instances"""
    
    _providers = {
        "openai": OpenAIProvider,
        "anthropic": AnthropicProvider,
        "google": GoogleProvider,
        "gemini": GoogleProvider,  # Alias
    }
    
    _instances: Dict[str, BaseLLMProvider] = {}
    
    @classmethod
    def create_provider(
        cls,
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
        
        if provider_name not in cls._providers:
            available = ", ".join(cls._providers.keys())
            raise ValueError(f"Unknown provider: {provider_name}. Available: {available}")
        
        provider_class = cls._providers[provider_name]
        
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
            cls._instances[provider_name] = cls.create_provider(provider_name, config)
        
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
    from ..config import settings
    
    # Use default provider if not specified
    if provider_name is None:
        provider_name = settings.default_llm_provider
    
    # Build config from settings if not provided
    if config is None:
        config = {}
        
        if provider_name == "openai":
            config = {
                "api_key": settings.openai_api_key,
                "model": settings.openai_model,
                "temperature": settings.openai_temperature,
                "max_tokens": settings.openai_max_tokens,
            }
        elif provider_name == "anthropic":
            config = {
                "api_key": settings.anthropic_api_key,
                "model": settings.anthropic_model,
                "temperature": settings.anthropic_temperature,
                "max_tokens": settings.anthropic_max_tokens,
            }
        elif provider_name in ["google", "gemini"]:
            config = {
                "api_key": settings.google_api_key,
                "model": settings.google_model,
                "temperature": settings.google_temperature,
                "max_tokens": settings.google_max_tokens,
            }
    
    return LLMProviderFactory.get_or_create_provider(provider_name, config)

