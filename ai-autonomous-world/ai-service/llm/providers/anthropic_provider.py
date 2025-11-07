"""
Anthropic Claude LLM Provider implementation
"""

from typing import Optional, Dict, Any, List
from anthropic import AsyncAnthropic
from loguru import logger

from ..base import BaseLLMProvider, LLMResponse


class AnthropicProvider(BaseLLMProvider):
    """Anthropic Claude LLM provider"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        
        api_key = config.get("api_key")
        if not api_key:
            raise ValueError("Anthropic API key is required")
        
        self.client = AsyncAnthropic(api_key=api_key)
        self.model = config.get("model", "claude-3-sonnet-20240229")
        logger.info(f"Anthropic provider initialized with model: {self.model}")
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate text from prompt using Anthropic"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            logger.debug(f"Anthropic request: temp={temp}, max_tokens={max_tok}")
            
            response = await self.client.messages.create(
                model=self.model,
                max_tokens=max_tok,
                temperature=temp,
                system=system_prompt if system_prompt else "",
                messages=[{"role": "user", "content": prompt}],
                **kwargs
            )
            
            content = response.content[0].text
            tokens_used = response.usage.input_tokens + response.usage.output_tokens
            finish_reason = response.stop_reason
            
            logger.debug(f"Anthropic response: {tokens_used} tokens, stop_reason={finish_reason}")
            
            return LLMResponse(
                content=content,
                model=self.model,
                provider="anthropic",
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                metadata={"response_id": response.id}
            )
        except Exception as e:
            logger.error(f"Anthropic generation error: {e}")
            raise
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate chat completion using Anthropic"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            # Extract system message if present
            system_msg = ""
            chat_messages = []
            for msg in messages:
                if msg["role"] == "system":
                    system_msg = msg["content"]
                else:
                    chat_messages.append(msg)
            
            logger.debug(f"Anthropic chat request: {len(chat_messages)} messages")
            
            response = await self.client.messages.create(
                model=self.model,
                max_tokens=max_tok,
                temperature=temp,
                system=system_msg,
                messages=chat_messages,
                **kwargs
            )
            
            content = response.content[0].text
            tokens_used = response.usage.input_tokens + response.usage.output_tokens
            finish_reason = response.stop_reason
            
            return LLMResponse(
                content=content,
                model=self.model,
                provider="anthropic",
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                metadata={"response_id": response.id}
            )
        except Exception as e:
            logger.error(f"Anthropic chat generation error: {e}")
            raise
    
    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """Generate structured output using Anthropic"""
        try:
            # Anthropic doesn't have native structured output, use prompt engineering
            structured_prompt = f"{prompt}\n\nRespond with valid JSON matching this schema:\n{schema}"
            
            response = await self.generate(
                prompt=structured_prompt,
                system_prompt=system_prompt,
                **kwargs
            )
            
            import json
            # Extract JSON from response
            content = response.content.strip()
            if content.startswith("```json"):
                content = content[7:]
            if content.endswith("```"):
                content = content[:-3]
            
            return json.loads(content.strip())
            
        except Exception as e:
            logger.error(f"Anthropic structured generation error: {e}")
            raise

