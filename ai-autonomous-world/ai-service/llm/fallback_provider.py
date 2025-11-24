"""
Fallback LLM Provider with Automatic Provider Chain

Implements automatic failover between LLM providers with:
- Sequential provider attempts on failure
- Automatic recovery detection
- Failure tracking and metrics
- Configurable retry thresholds
"""

import random
from typing import List, Optional, Any, Dict
from loguru import logger
from prometheus_client import Counter, Gauge

from .base import BaseLLMProvider, LLMResponse


# Prometheus metrics
llm_fallback_switches = Counter(
    "llm_fallback_switches_total",
    "Total provider fallback switches",
    ["from_provider", "to_provider", "reason"]
)

llm_fallback_failures = Counter(
    "llm_fallback_failures_total",
    "Provider failures in fallback chain",
    ["provider"]
)

llm_current_provider = Gauge(
    "llm_current_provider_index",
    "Index of currently active provider in fallback chain"
)

llm_provider_recovery = Counter(
    "llm_provider_recovery_total",
    "Provider recovery events",
    ["provider"]
)


class FallbackLLMProvider(BaseLLMProvider):
    """
    LLM provider with automatic fallback chain.
    
    Tries providers in sequence until one succeeds.
    Tracks failures and automatically switches to next provider.
    Returns to primary provider when it recovers.
    """
    
    def __init__(
        self,
        providers: List[BaseLLMProvider],
        provider_names: List[str],
        max_failures: int = 3,
        recovery_check_rate: float = 0.1
    ):
        """
        Initialize fallback provider chain.
        
        Args:
            providers: List of provider instances in fallback order
            provider_names: List of provider names for logging
            max_failures: Max consecutive failures before switching (default: 3)
            recovery_check_rate: Probability of checking primary recovery (0.0-1.0, default: 0.1)
        """
        # Initialize base with empty config (not needed for wrapper)
        super().__init__(config={})
        
        if not providers:
            raise ValueError("At least one provider required in fallback chain")
        
        if len(providers) != len(provider_names):
            raise ValueError("providers and provider_names must have same length")
        
        self.providers = providers
        self.provider_names = provider_names
        self.current_index = 0  # Start with primary provider
        self.failure_counts = [0] * len(providers)
        self.max_failures = max_failures
        self.recovery_check_rate = recovery_check_rate
        
        logger.info(
            f"Initialized fallback chain with {len(providers)} providers: "
            f"{', '.join(provider_names)}"
        )
        
        # Update metrics
        llm_current_provider.set(self.current_index)
    
    async def generate(
        self,
        prompt: str,
        system_prompt: Optional[str] = None,
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """
        Generate with automatic fallback.
        
        Tries current provider, falls back on failure.
        Periodically retries primary provider to check recovery.
        
        Args:
            prompt: User prompt
            system_prompt: Optional system prompt
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            **kwargs: Additional provider-specific parameters
            
        Returns:
            LLMResponse with generated text
            
        Raises:
            RuntimeError: If all providers in chain fail
        """
        # Try all providers in chain starting from current
        for attempt in range(len(self.providers)):
            provider_idx = (self.current_index + attempt) % len(self.providers)
            provider = self.providers[provider_idx]
            provider_name = self.provider_names[provider_idx]
            
            try:
                logger.debug(f"Attempting LLM call with {provider_name}")
                
                result = await provider.generate(
                    prompt=prompt,
                    system_prompt=system_prompt,
                    temperature=temperature,
                    max_tokens=max_tokens,
                    **kwargs
                )
                
                # Success! Reset failure count
                self.failure_counts[provider_idx] = 0
                
                # Update current provider if we switched
                if provider_idx != self.current_index:
                    old_provider = self.provider_names[self.current_index]
                    logger.info(
                        f"Fallback successful: switched from {old_provider} to {provider_name}"
                    )
                    llm_fallback_switches.labels(
                        from_provider=old_provider,
                        to_provider=provider_name,
                        reason="success"
                    ).inc()
                    self.current_index = provider_idx
                    llm_current_provider.set(self.current_index)
                
                # Periodically try primary provider (every ~10 calls)
                if provider_idx > 0 and random.random() < self.recovery_check_rate:
                    await self._check_primary_recovery()
                
                return result
                
            except Exception as e:
                self.failure_counts[provider_idx] += 1
                llm_fallback_failures.labels(provider=provider_name).inc()
                
                logger.warning(
                    f"{provider_name} failed "
                    f"({self.failure_counts[provider_idx]}/{self.max_failures}): {e}"
                )
                
                # If max failures reached, permanently switch to next provider
                if self.failure_counts[provider_idx] >= self.max_failures:
                    next_idx = (provider_idx + 1) % len(self.providers)
                    next_name = self.provider_names[next_idx]
                    
                    logger.error(
                        f"{provider_name} exceeded failure threshold, "
                        f"switching to {next_name}"
                    )
                    
                    llm_fallback_switches.labels(
                        from_provider=provider_name,
                        to_provider=next_name,
                        reason="threshold_exceeded"
                    ).inc()
                    
                    self.current_index = next_idx
                    llm_current_provider.set(self.current_index)
                
                continue  # Try next provider
        
        # All providers failed
        raise RuntimeError(
            f"All {len(self.providers)} LLM providers in fallback chain failed"
        )
    
    async def generate_chat(
        self,
        messages: List[Dict[str, str]],
        temperature: Optional[float] = None,
        max_tokens: Optional[int] = None,
        **kwargs
    ) -> LLMResponse:
        """
        Generate chat completion with conversation history and automatic fallback.
        
        Args:
            messages: List of message dicts with 'role' and 'content'
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            **kwargs: Additional provider-specific parameters
            
        Returns:
            LLMResponse with generated text
            
        Raises:
            RuntimeError: If all providers in chain fail
        """
        # Try all providers in chain starting from current
        for attempt in range(len(self.providers)):
            provider_idx = (self.current_index + attempt) % len(self.providers)
            provider = self.providers[provider_idx]
            provider_name = self.provider_names[provider_idx]
            
            try:
                logger.debug(f"Attempting chat LLM call with {provider_name}")
                
                result = await provider.generate_chat(
                    messages=messages,
                    temperature=temperature,
                    max_tokens=max_tokens,
                    **kwargs
                )
                
                # Success! Reset failure count
                self.failure_counts[provider_idx] = 0
                
                # Update current provider if we switched
                if provider_idx != self.current_index:
                    old_provider = self.provider_names[self.current_index]
                    logger.info(
                        f"Fallback successful: switched from {old_provider} to {provider_name}"
                    )
                    llm_fallback_switches.labels(
                        from_provider=old_provider,
                        to_provider=provider_name,
                        reason="success"
                    ).inc()
                    self.current_index = provider_idx
                    llm_current_provider.set(self.current_index)
                
                # Periodically try primary provider
                if provider_idx > 0 and random.random() < self.recovery_check_rate:
                    await self._check_primary_recovery()
                
                return result
                
            except Exception as e:
                self.failure_counts[provider_idx] += 1
                llm_fallback_failures.labels(provider=provider_name).inc()
                
                logger.warning(
                    f"{provider_name} chat failed "
                    f"({self.failure_counts[provider_idx]}/{self.max_failures}): {e}"
                )
                
                # If max failures reached, permanently switch to next provider
                if self.failure_counts[provider_idx] >= self.max_failures:
                    next_idx = (provider_idx + 1) % len(self.providers)
                    next_name = self.provider_names[next_idx]
                    
                    logger.error(
                        f"{provider_name} exceeded failure threshold, "
                        f"switching to {next_name}"
                    )
                    
                    llm_fallback_switches.labels(
                        from_provider=provider_name,
                        to_provider=next_name,
                        reason="threshold_exceeded"
                    ).inc()
                    
                    self.current_index = next_idx
                    llm_current_provider.set(self.current_index)
                
                continue  # Try next provider
        
        # All providers failed
        raise RuntimeError(
            f"All {len(self.providers)} LLM providers in fallback chain failed"
        )
    
    async def generate_structured(
        self,
        prompt: str,
        schema: Dict[str, Any],
        system_prompt: Optional[str] = None,
        **kwargs
    ) -> Dict[str, Any]:
        """
        Generate structured output with automatic fallback.
        
        Args:
            prompt: User prompt
            schema: JSON schema for output structure
            system_prompt: Optional system prompt
            **kwargs: Additional provider-specific parameters
            
        Returns:
            Structured data matching schema
            
        Raises:
            RuntimeError: If all providers in chain fail
        """
        # Try all providers in chain starting from current
        for attempt in range(len(self.providers)):
            provider_idx = (self.current_index + attempt) % len(self.providers)
            provider = self.providers[provider_idx]
            provider_name = self.provider_names[provider_idx]
            
            try:
                logger.debug(f"Attempting structured LLM call with {provider_name}")
                
                result = await provider.generate_structured(
                    prompt=prompt,
                    schema=schema,
                    system_prompt=system_prompt,
                    **kwargs
                )
                
                # Success! Reset failure count
                self.failure_counts[provider_idx] = 0
                
                # Update current provider if we switched
                if provider_idx != self.current_index:
                    old_provider = self.provider_names[self.current_index]
                    logger.info(
                        f"Fallback successful: switched from {old_provider} to {provider_name}"
                    )
                    llm_fallback_switches.labels(
                        from_provider=old_provider,
                        to_provider=provider_name,
                        reason="success"
                    ).inc()
                    self.current_index = provider_idx
                    llm_current_provider.set(self.current_index)
                
                return result
                
            except Exception as e:
                self.failure_counts[provider_idx] += 1
                llm_fallback_failures.labels(provider=provider_name).inc()
                
                logger.warning(
                    f"{provider_name} structured generation failed "
                    f"({self.failure_counts[provider_idx]}/{self.max_failures}): {e}"
                )
                
                # If max failures reached, permanently switch to next provider
                if self.failure_counts[provider_idx] >= self.max_failures:
                    next_idx = (provider_idx + 1) % len(self.providers)
                    next_name = self.provider_names[next_idx]
                    
                    logger.error(
                        f"{provider_name} exceeded failure threshold, "
                        f"switching to {next_name}"
                    )
                    
                    llm_fallback_switches.labels(
                        from_provider=provider_name,
                        to_provider=next_name,
                        reason="threshold_exceeded"
                    ).inc()
                    
                    self.current_index = next_idx
                    llm_current_provider.set(self.current_index)
                
                continue  # Try next provider
        
        # All providers failed
        raise RuntimeError(
            f"All {len(self.providers)} LLM providers in fallback chain failed"
        )
    
    async def _check_primary_recovery(self):
        """Check if primary provider has recovered"""
        if self.current_index == 0:
            return  # Already on primary
        
        primary_provider = self.providers[0]
        primary_name = self.provider_names[0]
        
        try:
            logger.debug(f"Checking primary provider {primary_name} recovery...")
            
            # Simple health check
            await primary_provider.generate(
                prompt="test",
                max_tokens=10
            )
            
            # Primary recovered!
            logger.info(f"Primary provider {primary_name} recovered, switching back")
            
            llm_provider_recovery.labels(provider=primary_name).inc()
            llm_fallback_switches.labels(
                from_provider=self.provider_names[self.current_index],
                to_provider=primary_name,
                reason="recovery"
            ).inc()
            
            self.current_index = 0
            self.failure_counts[0] = 0
            llm_current_provider.set(self.current_index)
            
        except Exception:
            # Still failing, stay on current provider
            pass
    
    def get_current_provider_name(self) -> str:
        """Get name of currently active provider"""
        return self.provider_names[self.current_index]
    
    def get_fallback_stats(self) -> Dict[str, Any]:
        """Get statistics about fallback chain usage"""
        return {
            "current_provider": self.get_current_provider_name(),
            "current_index": self.current_index,
            "provider_chain": self.provider_names,
            "failure_counts": dict(zip(self.provider_names, self.failure_counts)),
            "total_providers": len(self.providers),
            "max_failures_threshold": self.max_failures,
            "recovery_check_rate": self.recovery_check_rate
        }