"""
Google Gemini LLM Provider implementation
"""

from typing import Optional, Dict, Any, List
import google.generativeai as genai
from loguru import logger

from ..base import BaseLLMProvider, LLMResponse


class GoogleProvider(BaseLLMProvider):
    """Google Gemini LLM provider"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        
        api_key = config.get("api_key")
        if not api_key:
            raise ValueError("Google API key is required")
        
        genai.configure(api_key=api_key)
        self.model_name = config.get("model", "gemini-pro")
        self.model = genai.GenerativeModel(self.model_name)
        logger.info(f"Google provider initialized with model: {self.model_name}")
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate text from prompt using Google Gemini"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            # Combine system prompt with user prompt
            full_prompt = prompt
            if system_prompt:
                full_prompt = f"{system_prompt}\n\n{prompt}"
            
            logger.debug(f"Google Gemini request: temp={temp}, max_tokens={max_tok}")
            
            generation_config = genai.types.GenerationConfig(
                temperature=temp,
                max_output_tokens=max_tok,
            )
            
            response = await self.model.generate_content_async(
                full_prompt,
                generation_config=generation_config,
                **kwargs
            )
            
            content = response.text
            tokens_used = None  # Gemini doesn't provide token count in response
            finish_reason = response.candidates[0].finish_reason.name if response.candidates else None
            
            logger.debug(f"Google Gemini response: finish_reason={finish_reason}")
            
            return LLMResponse(
                content=content,
                model=self.model_name,
                provider="google",
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                metadata={}
            )
        except Exception as e:
            logger.error(f"Google Gemini generation error: {e}")
            raise
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate chat completion using Google Gemini"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            # Convert messages to Gemini format
            chat_history = []
            system_instruction = None
            
            for msg in messages:
                if msg["role"] == "system":
                    system_instruction = msg["content"]
                elif msg["role"] == "user":
                    chat_history.append({"role": "user", "parts": [msg["content"]]})
                elif msg["role"] == "assistant":
                    chat_history.append({"role": "model", "parts": [msg["content"]]})
            
            logger.debug(f"Google Gemini chat request: {len(chat_history)} messages")
            
            # Create chat session
            chat = self.model.start_chat(history=chat_history[:-1] if len(chat_history) > 1 else [])
            
            generation_config = genai.types.GenerationConfig(
                temperature=temp,
                max_output_tokens=max_tok,
            )
            
            # Send last message
            last_message = chat_history[-1]["parts"][0] if chat_history else ""
            response = await chat.send_message_async(
                last_message,
                generation_config=generation_config,
                **kwargs
            )
            
            content = response.text
            finish_reason = response.candidates[0].finish_reason.name if response.candidates else None
            
            return LLMResponse(
                content=content,
                model=self.model_name,
                provider="google",
                tokens_used=None,
                finish_reason=finish_reason,
                metadata={}
            )
        except Exception as e:
            logger.error(f"Google Gemini chat generation error: {e}")
            raise
    
    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """Generate structured output using Google Gemini"""
        try:
            # Gemini doesn't have native structured output, use prompt engineering
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
            logger.error(f"Google Gemini structured generation error: {e}")
            raise

