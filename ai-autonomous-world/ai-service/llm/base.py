"""
Base LLM Provider interface
"""

from abc import ABC, abstractmethod
from typing import Optional, Dict, Any, List
from dataclasses import dataclass
from loguru import logger


@dataclass
class LLMResponse:
    """Response from LLM provider"""
    content: str
    model: str
    provider: str
    tokens_used: Optional[int] = None
    finish_reason: Optional[str] = None
    metadata: Optional[Dict[str, Any]] = None


class BaseLLMProvider(ABC):
    """Base class for LLM providers"""
    
    def __init__(self, config: Dict[str, Any]):
        """
        Initialize LLM provider
        
        Args:
            config: Provider-specific configuration
        """
        self.config = config
        self.provider_name = self.__class__.__name__
        logger.info(f"Initializing {self.provider_name}")
    
    @abstractmethod
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """
        Generate text from prompt
        
        Args:
            prompt: User prompt
            system_prompt: Optional system prompt
            temperature: Sampling temperature (0.0 to 1.0)
            max_tokens: Maximum tokens to generate
            **kwargs: Additional provider-specific parameters
            
        Returns:
            LLMResponse with generated text
        """
        raise NotImplementedError("Subclass must implement generate() method")

    @abstractmethod
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """
        Generate chat completion from message history

        Args:
            messages: List of message dicts with 'role' and 'content'
            temperature: Sampling temperature (0.0 to 1.0)
            max_tokens: Maximum tokens to generate
            **kwargs: Additional provider-specific parameters

        Returns:
            LLMResponse with generated text
        """
        raise NotImplementedError("Subclass must implement generate_chat() method")

    @abstractmethod
    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Generate structured output matching schema

        Args:
            prompt: User prompt
            schema: JSON schema for output structure
            system_prompt: Optional system prompt
            **kwargs: Additional provider-specific parameters

        Returns:
            Structured data matching schema
        """
        raise NotImplementedError("Subclass must implement generate_structured() method")
    
    def get_default_temperature(self) -> float:
        """Get default temperature for this provider"""
        return self.config.get("temperature", 0.7)
    
    def get_default_max_tokens(self) -> int:
        """Get default max tokens for this provider"""
        return self.config.get("max_tokens", 2000)
    
    def get_model_name(self) -> str:
        """Get model name for this provider"""
        return self.config.get("model", "unknown")
    
    async def health_check(self) -> bool:
        """
        Check if provider is healthy and accessible
        
        Returns:
            True if healthy, False otherwise
        """
        try:
            # Simple test generation
            response = await self.generate(
                prompt="Say 'OK' if you can read this.",
                max_tokens=10
            )
            return response.content is not None
        except Exception as e:
            logger.error(f"{self.provider_name} health check failed: {e}")
            return False

