"""
Azure OpenAI LLM Provider implementation
Supports Azure OpenAI Foundry with Azure-specific endpoint, API version, and deployment
"""

from typing import Optional, Dict, Any, List
from openai import AzureOpenAI, AsyncAzureOpenAI
from loguru import logger
import re

from ..base import BaseLLMProvider, LLMResponse


class AzureOpenAIProvider(BaseLLMProvider):
    """Azure OpenAI LLM provider with Azure-specific configuration"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        
        # Validate required Azure-specific parameters
        api_key = config.get("api_key")
        endpoint = config.get("endpoint")
        api_version = config.get("api_version")
        deployment = config.get("deployment")
        
        if not api_key:
            raise ValueError("Azure OpenAI API key is required")
        if not endpoint:
            raise ValueError("Azure OpenAI endpoint is required")
        if not api_version:
            raise ValueError("Azure OpenAI API version is required")
        if not deployment:
            raise ValueError("Azure OpenAI deployment name is required")
        
        # Validate and fix endpoint format
        self.endpoint = self._validate_and_fix_endpoint(endpoint)
        self.api_version = api_version
        self.deployment = deployment
        
        # Initialize Azure OpenAI client
        self.client = AsyncAzureOpenAI(
            api_key=api_key,
            azure_endpoint=self.endpoint,
            api_version=self.api_version
        )
        
        logger.info(f"Azure OpenAI provider initialized with deployment: {self.deployment}, API version: {self.api_version}")
        logger.debug(f"Azure endpoint: {self.endpoint}")
    
    def _validate_and_fix_endpoint(self, endpoint: str) -> str:
        """
        Validate and fix Azure OpenAI endpoint URL format
        
        Azure endpoints should be in format: https://{resource}.openai.azure.com/
        
        Args:
            endpoint: Raw endpoint URL
            
        Returns:
            Validated and formatted endpoint URL
        """
        # Remove trailing slash if present
        endpoint = endpoint.rstrip('/')
        
        # Ensure https:// prefix
        if not endpoint.startswith('http://') and not endpoint.startswith('https://'):
            endpoint = f'https://{endpoint}'
        
        # Validate Azure endpoint format
        azure_pattern = r'https://[a-zA-Z0-9\-]+\.openai\.azure\.com'
        if not re.match(azure_pattern, endpoint):
            logger.warning(f"Endpoint '{endpoint}' does not match expected Azure format: https://{{resource}}.openai.azure.com")
            logger.warning("Proceeding anyway, but this may cause connection issues")
        
        return endpoint
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate text from prompt using Azure OpenAI"""
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
            logger.error(f"Azure OpenAI generation error: {e}")
            raise
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """Generate chat completion using Azure OpenAI"""
        try:
            temp = temperature if temperature is not None else self.get_default_temperature()
            max_tok = max_tokens if max_tokens is not None else self.get_default_max_tokens()
            
            logger.debug(f"Azure OpenAI chat request: deployment={self.deployment}, {len(messages)} messages, temp={temp}, max_tokens={max_tok}")
            
            response = await self.client.chat.completions.create(
                model=self.deployment,  # Azure uses deployment name instead of model
                messages=messages,
                temperature=temp,
                max_tokens=max_tok,
                **kwargs
            )
            
            content = response.choices[0].message.content
            tokens_used = response.usage.total_tokens if response.usage else None
            finish_reason = response.choices[0].finish_reason
            
            logger.debug(f"Azure OpenAI response: {tokens_used} tokens, finish_reason={finish_reason}")
            
            return LLMResponse(
                content=content,
                model=self.deployment,
                provider="azure_openai",
                tokens_used=tokens_used,
                finish_reason=finish_reason,
                metadata={
                    "response_id": response.id,
                    "api_version": self.api_version,
                    "endpoint": self.endpoint
                }
            )
        except Exception as e:
            logger.error(f"Azure OpenAI chat generation error: {e}")
            raise

    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """Generate structured output using Azure OpenAI function calling"""
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

            logger.debug(f"Azure OpenAI structured request: deployment={self.deployment}")

            response = await self.client.chat.completions.create(
                model=self.deployment,
                messages=messages,
                functions=functions,
                function_call={"name": "structured_response"},
                **kwargs
            )

            import json
            function_args = response.choices[0].message.function_call.arguments
            result = json.loads(function_args)

            logger.debug(f"Azure OpenAI structured response generated successfully")
            return result

        except Exception as e:
            logger.error(f"Azure OpenAI structured generation error: {e}")
            raise

