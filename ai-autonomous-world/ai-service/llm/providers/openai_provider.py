"""
OpenAI LLM Provider implementation
"""

from typing import Optional, Dict, Any, List
from openai import AsyncOpenAI
from loguru import logger

from ..base import BaseLLMProvider, LLMResponse


class OpenAIProvider(BaseLLMProvider):
    """OpenAI LLM provider"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        
        api_key = config.get("api_key")
        if not api_key:
            raise ValueError("OpenAI API key is required")
        
        self.client = AsyncOpenAI(api_key=api_key)
        self.model = config.get("model", "gpt-4")
        logger.info(f"OpenAI provider initialized with model: {self.model}")
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate text from prompt using OpenAI"""
        try:
            messages = []
            if system_prompt:
                messages.append({"role": "system", "content": system_prompt})
            messages.append({"role": "user", "content": prompt})
            
            return await self.generate_chat(
                messages=messages,
                temperature=temperature,
                max_tokens=max_tokens,
                **kwargs
            )
        except Exception as e:
            logger.error(f"OpenAI generation error: {e}")
            raise
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate chat completion using OpenAI"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            logger.debug(f"OpenAI chat request: {len(messages)} messages, temp={temp}, max_tokens={max_tok}")
            
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
            
            logger.debug(f"OpenAI response: {tokens_used} tokens, finish_reason={finish_reason}")
            
            return LLMResponse(
                content=content,
                model=self.model,
                provider="openai",
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                metadata={"response_id": response.id}
            )
        except Exception as e:
            logger.error(f"OpenAI chat generation error: {e}")
            raise
    
    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """Generate structured output using OpenAI function calling"""
        try:
            messages = []
            if system_prompt:
                messages.append({"role": "system", "content": system_prompt})
            messages.append({"role": "user", "content": prompt})
            
            # Use function calling for structured output
            functions = [{
                "name": "structured_response",
                "description": "Generate structured response",
                "parameters": schema
            }]
            
            response = await self.client.chat.completions.create(
                model=self.model,
                messages=messages,
                functions=functions,
                function_call={"name": "structured_response"},
                **kwargs
            )
            
            import json
            function_args = response.choices[0].message.function_call.arguments
            return json.loads(function_args)
            
        except Exception as e:
            logger.error(f"OpenAI structured generation error: {e}")
            raise

