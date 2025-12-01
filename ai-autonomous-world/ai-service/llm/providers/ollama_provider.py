"""
Ollama Provider Implementation

Local LLM provider with:
- No API key required
- Local model execution
- Model management
- Localhost connection
- Support for various open-source models
"""

import logging
from typing import Any, Dict, List, Optional

import aiohttp

from ..base_provider import (
    BaseLLMProvider,
    LLMProviderError,
    ModelNotFoundError,
    ProviderType,
    TimeoutError as LLMTimeoutError,
)

logger = logging.getLogger(__name__)


class OllamaProvider(BaseLLMProvider):
    """
    Ollama local LLM provider.
    
    Runs models locally without requiring API keys.
    Supports Llama, Mistral, CodeLlama, and other open-source models.
    """
    
    # No costs for local execution
    COST_PER_1K_INPUT_TOKENS = 0.0
    COST_PER_1K_OUTPUT_TOKENS = 0.0
    
    def __init__(
        self,
        model: str = "llama2",
        base_url: str = "http://localhost:11434",
        timeout: float = 120.0,  # Longer timeout for local inference
        mock_mode: bool = False,
        debug: bool = False,
    ):
        """
        Initialize Ollama provider.
        
        Args:
            model: Model name (default: llama2)
            base_url: Ollama server URL
            timeout: Request timeout in seconds
            mock_mode: Enable mock mode for testing
            debug: Enable debug logging
        """
        super().__init__(
            provider_type=ProviderType.OLLAMA,
            model=model,
            api_key=None,  # No API key needed
            timeout=timeout,
            mock_mode=mock_mode,
            debug=debug,
        )
        
        self.base_url = base_url.rstrip("/")
        
        if self.debug:
            logger.debug(
                f"Ollama provider initialized: model={model}, "
                f"base_url={base_url}"
            )
    
    def _format_messages_for_ollama(
        self,
        messages: List[Dict[str, str]]
    ) -> str:
        """
        Format messages for Ollama API.
        
        Ollama's chat API expects messages in the same format as OpenAI,
        but can also work with a simple prompt string.
        
        Args:
            messages: List of message dicts
            
        Returns:
            Formatted prompt or messages
        """
        # If simple single user message, return as string
        if len(messages) == 1 and messages[0].get("role") == "user":
            return messages[0].get("content", "")
        
        # Otherwise, format as conversation
        formatted_parts = []
        for msg in messages:
            role = msg.get("role", "")
            content = msg.get("content", "")
            
            if role == "system":
                formatted_parts.append(f"System: {content}")
            elif role == "user":
                formatted_parts.append(f"User: {content}")
            elif role == "assistant":
                formatted_parts.append(f"Assistant: {content}")
        
        return "\n\n".join(formatted_parts) + "\n\nAssistant:"
    
    async def _make_request(
        self,
        messages: List[Dict[str, str]],
        temperature: float,
        max_tokens: int,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Make Ollama API request.
        
        Args:
            messages: List of message dicts
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            **kwargs: Additional parameters
            
        Returns:
            Response dict with text, tokens_used, and metadata
        """
        try:
            # Try Ollama chat API first
            api_url = f"{self.base_url}/api/chat"
            
            # Prepare request payload
            payload = {
                "model": self.model,
                "messages": messages,
                "stream": False,
                "options": {
                    "temperature": temperature,
                    "num_predict": max_tokens,
                },
            }
            
            if self.debug:
                logger.debug(f"Ollama request: {payload}")
            
            # Make async HTTP request
            async with aiohttp.ClientSession() as session:
                try:
                    async with session.post(
                        api_url,
                        json=payload,
                        timeout=aiohttp.ClientTimeout(total=self.timeout)
                    ) as response:
                        if response.status == 404:
                            # Try generate API as fallback
                            return await self._make_generate_request(
                                messages, temperature, max_tokens, **kwargs
                            )
                        
                        if response.status != 200:
                            error_text = await response.text()
                            raise LLMProviderError(
                                f"Ollama API error {response.status}: {error_text}"
                            )
                        
                        result = await response.json()
                        
                except aiohttp.ClientError as e:
                    logger.error(f"Ollama connection error: {e}")
                    raise LLMProviderError(
                        f"Failed to connect to Ollama at {self.base_url}. "
                        f"Is Ollama running? Error: {e}"
                    )
            
            # Extract response
            message = result.get("message", {})
            text = message.get("content", "")
            
            # Estimate token usage (Ollama doesn't provide exact counts)
            prompt_tokens = sum(len(m.get("content", "").split()) for m in messages)
            completion_tokens = len(text.split())
            tokens_used = prompt_tokens + completion_tokens
            
            # Build metadata
            metadata = {
                "model": result.get("model", self.model),
                "created_at": result.get("created_at"),
                "done": result.get("done", False),
                "total_duration": result.get("total_duration"),
                "load_duration": result.get("load_duration"),
                "prompt_eval_count": result.get("prompt_eval_count"),
                "eval_count": result.get("eval_count"),
            }
            
            if self.debug:
                logger.debug(
                    f"Ollama response: tokens≈{tokens_used}, "
                    f"done={result.get('done')}"
                )
            
            return {
                "text": text,
                "tokens_used": tokens_used,
                "metadata": metadata,
            }
            
        except aiohttp.ClientTimeout:
            logger.error("Ollama request timeout")
            raise LLMTimeoutError(f"Ollama request timed out after {self.timeout}s")
        
        except Exception as e:
            logger.error(f"Ollama unexpected error: {e}")
            raise
    
    async def _make_generate_request(
        self,
        messages: List[Dict[str, str]],
        temperature: float,
        max_tokens: int,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Fallback to Ollama generate API.
        
        Args:
            messages: List of message dicts
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            **kwargs: Additional parameters
            
        Returns:
            Response dict with text, tokens_used, and metadata
        """
        api_url = f"{self.base_url}/api/generate"
        
        # Format prompt
        prompt = self._format_messages_for_ollama(messages)
        
        payload = {
            "model": self.model,
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": temperature,
                "num_predict": max_tokens,
            },
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                api_url,
                json=payload,
                timeout=aiohttp.ClientTimeout(total=self.timeout)
            ) as response:
                if response.status == 404:
                    raise ModelNotFoundError(
                        f"Ollama model '{self.model}' not found. "
                        f"Pull it with: ollama pull {self.model}"
                    )
                
                if response.status != 200:
                    error_text = await response.text()
                    raise LLMProviderError(
                        f"Ollama API error {response.status}: {error_text}"
                    )
                
                result = await response.json()
        
        text = result.get("response", "")
        
        # Estimate tokens
        tokens_used = len(prompt.split()) + len(text.split())
        
        metadata = {
            "model": result.get("model", self.model),
            "done": result.get("done", False),
        }
        
        return {
            "text": text,
            "tokens_used": tokens_used,
            "metadata": metadata,
        }
    
    def _calculate_cost(self, tokens_used: int) -> float:
        """
        Calculate cost for Ollama usage.
        
        Ollama is free (runs locally), so cost is always 0.
        
        Args:
            tokens_used: Total tokens used
            
        Returns:
            Cost in USD (always 0)
        """
        return 0.0