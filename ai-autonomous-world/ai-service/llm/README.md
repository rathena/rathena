# LLM Provider System

Complete LLM provider infrastructure with automatic fallback chain and fault tolerance.

## 🚀 Features

- **Multiple Providers**: Azure OpenAI, OpenAI, Anthropic, DeepSeek, Ollama
- **Automatic Fallback**: Seamless failover between providers
- **Circuit Breaker**: Automatic fault isolation (5 failures in 60s)
- **Retry Logic**: Exponential backoff with configurable limits
- **Rate Limiting**: Smart rate limit detection and handling
- **Cost Tracking**: Token usage and cost monitoring per provider
- **Health Monitoring**: Real-time provider health status
- **Mock Mode**: Testing without API calls
- **Async/Await**: Full async support for high performance

## 📁 Architecture

```
llm/
├── __init__.py                 # Package exports
├── base_provider.py           # Abstract base class (~405 lines)
├── factory.py                 # Provider factory (~446 lines)
├── test_providers.py          # Test suite (~234 lines)
├── README.md                  # This file
└── providers/
    ├── __init__.py
    ├── azure_openai_provider.py   # Azure OpenAI (~199 lines)
    ├── openai_provider.py         # Standard OpenAI (~207 lines)
    ├── anthropic_provider.py      # Claude API (~245 lines)
    ├── deepseek_provider.py       # DeepSeek (~212 lines)
    └── ollama_provider.py         # Local Ollama (~286 lines)
```

## 🔧 Installation

```bash
# Install dependencies
pip install -r requirements.txt

# Key dependencies:
# - openai==1.68.0 (for OpenAI & Azure OpenAI)
# - anthropic==0.39.0 (for Claude)
# - aiohttp==3.9.1 (for Ollama)
```

## ⚙️ Configuration

Set environment variables for the providers you want to use:

```bash
# Azure OpenAI (Enterprise)
export AZURE_OPENAI_API_KEY="your-key"
export AZURE_OPENAI_ENDPOINT="https://your-resource.openai.azure.com"
export AZURE_OPENAI_DEPLOYMENT="your-deployment-name"

# Standard OpenAI
export OPENAI_API_KEY="your-key"
export OPENAI_ORG_ID="your-org-id"  # Optional

# Anthropic Claude
export ANTHROPIC_API_KEY="your-key"

# DeepSeek (Cost-effective)
export DEEPSEEK_API_KEY="your-key"

# Ollama (Local, no key needed)
export OLLAMA_BASE_URL="http://localhost:11434"  # Default
```

## 🎯 Quick Start

### Basic Usage

```python
import asyncio
from llm import LLMProviderFactory

async def main():
    # Initialize factory (auto-detects available providers)
    factory = LLMProviderFactory()
    
    # Simple generation
    response = await factory.generate(
        prompt="What is the capital of France?",
        temperature=0.7,
        max_tokens=100
    )
    
    print(f"Provider: {response.provider}")
    print(f"Response: {response.text}")
    print(f"Cost: ${response.cost_usd:.6f}")
    print(f"Tokens: {response.tokens_used}")

asyncio.run(main())
```

### Chat Conversation

```python
messages = [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content": "Explain quantum computing."},
]

response = await factory.chat(
    messages=messages,
    temperature=0.8,
    max_tokens=500
)
```

### Preferred Provider

```python
from llm.base_provider import ProviderType

# Try specific provider first, then fallback
response = await factory.generate(
    prompt="Hello, world!",
    preferred_provider=ProviderType.ANTHROPIC
)
```

## 🔄 Fallback Chain

Providers are tried in this order (if configured):

1. **Azure OpenAI** - Enterprise-grade, best performance
2. **OpenAI** - Reliable, widely available
3. **Anthropic** - Claude models, alternative approach
4. **DeepSeek** - Cost-effective option
5. **Ollama** - Local execution, free

Each provider has a circuit breaker that opens after 5 failures in 60 seconds.

## 📊 Health Monitoring

```python
# Get provider health status
status = factory.get_health_status()

for provider, health in status.items():
    print(f"{provider}:")
    print(f"  Healthy: {health['healthy']}")
    print(f"  Success Rate: {health['success_rate']*100:.1f}%")
    print(f"  Avg Latency: {health['average_latency_ms']:.2f}ms")
```

## 🧪 Testing

```bash
# Run test suite
cd ai-autonomous-world/ai-service
python -m llm.test_providers

# With specific providers
export OPENAI_API_KEY="your-key"
python -m llm.test_providers
```

## 💰 Cost Tracking

Token pricing per 1K tokens (as of implementation):

| Provider | Input | Output |
|----------|-------|--------|
| GPT-4 Turbo | $0.01 | $0.03 |
| GPT-4 | $0.03 | $0.06 |
| GPT-3.5 Turbo | $0.0005 | $0.0015 |
| Claude 3.5 Sonnet | $0.003 | $0.015 |
| Claude 3 Haiku | $0.00025 | $0.00125 |
| DeepSeek | $0.00014 | $0.00028 |
| Ollama | $0.00 | $0.00 |

## 🛡️ Error Handling

The system handles various error types:

- **RateLimitError** - Automatic retry with backoff
- **AuthenticationError** - Invalid API keys
- **ModelNotFoundError** - Model doesn't exist
- **TimeoutError** - Request timeout
- **ContentFilterError** - Content policy violation

All errors include detailed logging and metrics tracking.

## 🔒 Security

✅ No hardcoded secrets (environment variables only)
✅ SSL/TLS for all API connections
✅ Input validation and sanitization
✅ Secure error messages (no credential leaks)
✅ Rate limit compliance
✅ Circuit breaker for fault isolation

## 📈 Performance

- **Async/Await**: Non-blocking I/O throughout
- **Connection Pooling**: Reused HTTP connections
- **Retry Logic**: Exponential backoff (1s → 2s → 4s → 8s)
- **Circuit Breaker**: Fast failure for unhealthy providers
- **Token Estimation**: Approximate cost calculation
- **Lazy Loading**: Providers initialized on-demand

## 🔧 Advanced Usage

### Custom Configuration

```python
config = {
    "OPENAI_API_KEY": "custom-key",
    "AZURE_OPENAI_ENDPOINT": "custom-endpoint",
}

factory = LLMProviderFactory(config=config)
```

### Direct Provider Access

```python
from llm.providers.openai_provider import OpenAIProvider

provider = OpenAIProvider(
    api_key="your-key",
    model="gpt-4-turbo-preview",
    timeout=120.0
)

response = await provider.generate("Hello!")
```

### Mock Mode for Testing

```python
provider = OpenAIProvider(
    api_key="mock-key",
    mock_mode=True  # No actual API calls
)

response = await provider.generate("Test prompt")
# Returns mock response instantly
```

## 📝 Response Format

All providers return standardized `LLMResponse`:

```python
{
    "text": str,              # Generated text
    "tokens_used": int,       # Total tokens
    "cost_usd": float,        # Estimated cost
    "model": str,             # Model used
    "provider": str,          # Provider name
    "latency_ms": float,      # Response time
    "metadata": dict          # Provider-specific data
}
```

## 🐛 Troubleshooting

### Provider Not Available

```
❌ Provider not found in factory
```

**Solution**: Set the required environment variables for that provider.

### All Providers Failed

```
❌ All LLM providers failed
```

**Solution**: Check API keys, network connectivity, and provider status.

### Circuit Breaker Open

```
⚠️ Circuit breaker opened for provider
```

**Solution**: Wait 30s for cooldown period or check provider health.

## 📚 Dependencies

Core libraries used:

- `openai>=1.68.0` - OpenAI & Azure OpenAI client
- `anthropic>=0.39.0` - Anthropic Claude client
- `aiohttp>=3.9.1` - Async HTTP for Ollama
- Python 3.11+ - Type hints and async support

## 🤝 Contributing

When adding new providers:

1. Inherit from [`BaseLLMProvider`](base_provider.py)
2. Implement `_make_request()` method
3. Add to [`factory.py`](factory.py) initialization
4. Update fallback chain if needed
5. Add tests to [`test_providers.py`](test_providers.py)

## 📄 License

Part of the AI Autonomous World project.

## 🔗 Related

- [Base Provider Documentation](base_provider.py)
- [Factory Pattern](factory.py)
- [Test Suite](test_providers.py)