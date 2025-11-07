"""
DeepSeek LLM Provider implementation
DeepSeek API is OpenAI-compatible, so we use the OpenAI SDK with custom base URL
"""

from typing import Optional, Dict, Any, List
from openai import AsyncOpenAI
from loguru import logger

from ..base import BaseLLMProvider, LLMResponse


class DeepSeekProvider(BaseLLMProvider):
    """DeepSeek LLM provider (OpenAI-compatible API)"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        
        api_key = config.get("api_key")
        if not api_key:
            raise ValueError("DeepSeek API key is required")
        
        base_url = config.get("base_url", "https://api.deepseek.com")
        
        # Initialize OpenAI client with DeepSeek base URL
        self.client = AsyncOpenAI(
            api_key=api_key,
            base_url=base_url
        )
        
        self.model = config.get("model", "deepseek-chat")
        logger.info(f"DeepSeek provider initialized with model: {self.model}, base_url: {base_url}")
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate text from prompt using DeepSeek"""
        try:
            messages = []
            if system_prompt:
                messages.append({"role": "system", "content": system_prompt})
            messages.append({"role": "user", "content": prompt})
            
            logger.debug(f"DeepSeek generate request: prompt_length={len(prompt)}, system_prompt={bool(system_prompt)}")
            
            return await self.generate_chat(
                messages=messages,
                temperature=temperature,
                max_tokens=max_tokens,
                **kwargs
            )
        except Exception as e:
            logger.error(f"DeepSeek generation error: {e}", exc_info=True)
            raise
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate chat completion using DeepSeek"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            logger.debug(f"DeepSeek chat request: {len(messages)} messages, temp={temp}, max_tokens={max_tok}, model={self.model}")
            
            response = await self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                temperature=temp,
                max_tokens=max_tok,
                **kwargs
            )
            
            content = response.choices[0].message.content
            tokens_used = response.usage.total_tokens if response.usage else None
            finish_reason = response.choices[0].finish_reason
            
            logger.debug(f"DeepSeek response: {tokens_used} tokens, finish_reason={finish_reason}")
            
            return LLMResponse(
                content=content,
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                raw_response=response
            )
        except Exception as e:
            logger.error(f"DeepSeek chat completion error: {e}", exc_info=True)
            raise
    
    async def generate_stream(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ):
        """Generate streaming response from DeepSeek"""
        try:
            messages = []
            if system_prompt:
                messages.append({"role": "system", "content": system_prompt})
            messages.append({"role": "user", "content": prompt})
            
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            logger.debug(f"DeepSeek stream request: {len(messages)} messages, temp={temp}, max_tokens={max_tok}")
            
            stream = await self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                temperature=temp,
                max_tokens=max_tok,
                stream=True,
                **kwargs
            )
            
            async for chunk in stream:
                if chunk.choices[0].delta.content:
                    yield chunk.choices[0].delta.content
                    
        except Exception as e:
            logger.error(f"DeepSeek streaming error: {e}", exc_info=True)
            raise
    
    def get_provider_name(self) -> str:
        """Get provider name"""
        return "deepseek"
    
    def get_model_name(self) -> str:
        """Get current model name"""
        return self.model

