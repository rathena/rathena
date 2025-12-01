"""
Test script for LLM Provider System

Demonstrates:
- Provider factory initialization
- Fallback chain functionality
- Individual provider usage
- Health status monitoring
- Error handling
"""

import asyncio
import logging
import os
from typing import Dict, Any

from .factory import LLMProviderFactory
from .base_provider import ProviderType, LLMProviderError

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


async def test_factory_initialization():
    """Test factory initialization with available providers"""
    print("\n" + "="*60)
    print("TEST 1: Factory Initialization")
    print("="*60)
    
    factory = LLMProviderFactory()
    available = factory.get_available_providers()
    
    print(f"✅ Factory initialized successfully")
    print(f"📋 Available providers: {[p.value for p in available]}")
    
    return factory


async def test_simple_generation(factory: LLMProviderFactory):
    """Test simple text generation with fallback"""
    print("\n" + "="*60)
    print("TEST 2: Simple Text Generation")
    print("="*60)
    
    try:
        response = await factory.generate(
            prompt="What is the capital of France? Answer in one sentence.",
            temperature=0.7,
            max_tokens=50
        )
        
        print(f"✅ Generation successful")
        print(f"Provider: {response.provider}")
        print(f"Model: {response.model}")
        print(f"Response: {response.text}")
        print(f"Tokens: {response.tokens_used}")
        print(f"Cost: ${response.cost_usd:.6f}")
        print(f"Latency: {response.latency_ms:.2f}ms")
        
    except LLMProviderError as e:
        print(f"❌ All providers failed: {e}")


async def test_chat_conversation(factory: LLMProviderFactory):
    """Test multi-turn conversation"""
    print("\n" + "="*60)
    print("TEST 3: Multi-turn Conversation")
    print("="*60)
    
    messages = [
        {"role": "system", "content": "You are a helpful assistant."},
        {"role": "user", "content": "What is 2+2?"},
        {"role": "assistant", "content": "2+2 equals 4."},
        {"role": "user", "content": "What about 3+3?"}
    ]
    
    try:
        response = await factory.chat(
            messages=messages,
            temperature=0.7,
            max_tokens=50
        )
        
        print(f"✅ Chat successful")
        print(f"Provider: {response.provider}")
        print(f"Response: {response.text}")
        
    except LLMProviderError as e:
        print(f"❌ Chat failed: {e}")


async def test_preferred_provider(factory: LLMProviderFactory):
    """Test with preferred provider"""
    print("\n" + "="*60)
    print("TEST 4: Preferred Provider Selection")
    print("="*60)
    
    # Try with each available provider
    for provider_type in factory.get_available_providers():
        try:
            print(f"\n🎯 Trying {provider_type.value}...")
            response = await factory.generate(
                prompt="Say 'Hello from {provider}'",
                temperature=0.5,
                max_tokens=20,
                preferred_provider=provider_type
            )
            
            print(f"✅ Success with {response.provider}")
            print(f"Response: {response.text}")
            break
            
        except Exception as e:
            print(f"⚠️  {provider_type.value} failed: {e}")


async def test_health_status(factory: LLMProviderFactory):
    """Test health status monitoring"""
    print("\n" + "="*60)
    print("TEST 5: Provider Health Status")
    print("="*60)
    
    status = factory.get_health_status()
    
    print("\n📊 Provider Health Report:")
    print("-" * 60)
    
    for provider_name, health_data in status.items():
        print(f"\n{provider_name}:")
        print(f"  Available: {health_data['available']}")
        print(f"  Healthy: {health_data['healthy']}")
        print(f"  Circuit Breaker: {'🔴 Open' if health_data['circuit_breaker_open'] else '🟢 Closed'}")
        print(f"  Success Rate: {health_data['success_rate']*100:.1f}%")
        print(f"  Successes: {health_data['success_count']}")
        print(f"  Failures: {health_data['failure_count']}")
        print(f"  Avg Latency: {health_data['average_latency_ms']:.2f}ms")


async def test_error_handling(factory: LLMProviderFactory):
    """Test error handling and fallback"""
    print("\n" + "="*60)
    print("TEST 6: Error Handling & Fallback")
    print("="*60)
    
    try:
        # Try with invalid configuration
        response = await factory.generate(
            prompt="Test fallback chain",
            temperature=0.7,
            max_tokens=50
        )
        
        print(f"✅ Fallback successful")
        print(f"Final provider: {response.provider}")
        
    except LLMProviderError as e:
        print(f"❌ All providers exhausted: {e}")


async def test_mock_mode():
    """Test mock mode for testing without API calls"""
    print("\n" + "="*60)
    print("TEST 7: Mock Mode")
    print("="*60)
    
    # Create factory with mock configuration
    config = {
        "OPENAI_API_KEY": "mock-key-for-testing"
    }
    factory = LLMProviderFactory(config=config)
    
    # Get a provider and enable mock mode
    providers = factory.get_available_providers()
    if providers:
        provider_type = providers[0]
        provider = factory.get_provider(provider_type)
        if provider:
            provider.mock_mode = True
            
            response = await provider.generate("Test prompt")
            print(f"✅ Mock response generated")
            print(f"Response: {response.text}")
            print(f"Metadata: {response.metadata}")


async def run_all_tests():
    """Run all tests"""
    print("\n" + "="*70)
    print("🧪 LLM PROVIDER SYSTEM TEST SUITE")
    print("="*70)
    
    # Check for API keys
    print("\n🔑 Checking environment variables:")
    env_vars = [
        "AZURE_OPENAI_API_KEY",
        "OPENAI_API_KEY",
        "ANTHROPIC_API_KEY",
        "DEEPSEEK_API_KEY",
        "OLLAMA_BASE_URL"
    ]
    
    for var in env_vars:
        value = os.getenv(var)
        status = "✅ Set" if value else "❌ Not set"
        print(f"  {var}: {status}")
    
    # Initialize factory
    try:
        factory = await test_factory_initialization()
        
        # Run tests
        await test_simple_generation(factory)
        await test_chat_conversation(factory)
        await test_preferred_provider(factory)
        await test_health_status(factory)
        await test_error_handling(factory)
        await test_mock_mode()
        
        print("\n" + "="*70)
        print("✅ TEST SUITE COMPLETED")
        print("="*70)
        
    except Exception as e:
        print(f"\n❌ Test suite failed: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    asyncio.run(run_all_tests())