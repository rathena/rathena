# 🧪 Test Execution Guide for AI Autonomous World Service

## 📋 Quick Start

```bash
# Navigate to service directory
cd ai-autonomous-world/ai-service

# Install test dependencies (if not already installed)
pip install pytest pytest-asyncio pytest-cov pytest-mock faker factory-boy

# Run all tests with coverage
pytest tests/ -v --cov=. --cov-report=html --cov-report=term-missing

# View coverage report
open htmlcov/index.html  # macOS
xdg-open htmlcov/index.html  # Linux
```

## 🎯 Test Suite Structure

### Created Test Files (45+)

```
tests/
├── conftest.py (399 lines)          # Shared fixtures & mocks
├── pytest.ini                        # Test configuration
├── README.md (499 lines)            # Comprehensive documentation
│
├── agents/ (8 files, 3,598 lines)
│   ├── test_base_agent.py           # Base agent functionality
│   ├── test_dialogue_agent.py       # NPC dialogue generation
│   ├── test_memory_agent.py         # Memory storage/retrieval
│   ├── test_quest_agent.py          # Quest generation
│   ├── test_world_agent.py          # World state management
│   ├── test_economy_agent.py        # Market dynamics
│   ├── test_orchestrator.py         # Multi-agent coordination
│   └── test_moral_alignment.py      # Moral decision system
│
├── llm/ (7 files, 2,451 lines)
│   ├── test_base_provider.py        # Base provider interface
│   ├── test_openai_provider.py      # OpenAI integration
│   ├── test_anthropic_provider.py   # Claude integration
│   ├── test_azure_openai_provider.py # Azure OpenAI
│   ├── test_deepseek_provider.py    # DeepSeek integration
│   ├── test_ollama_provider.py      # Local Ollama
│   └── test_provider_factory.py     # Factory & fallback
│
├── routers/ (14 files, 5,255 lines)
│   ├── test_npc.py                  # NPC endpoints
│   ├── test_player.py               # Player interactions
│   ├── test_quest.py                # Quest management
│   ├── test_world.py                # World state
│   ├── test_economy.py              # Market operations
│   ├── test_faction.py              # Faction management
│   ├── test_relationship.py         # Relationships
│   ├── test_chat_command.py         # Command processing
│   ├── test_gift.py                 # Gift mechanics
│   ├── test_mvp.py                  # MVP spawning
│   ├── test_navigation.py           # Pathfinding
│   ├── test_npc_actions.py          # Action execution
│   ├── test_npc_movement.py         # Movement mechanics
│   └── test_emotional_state.py      # Emotional AI
│
├── utils/ (8 files, 3,985 lines)
│   ├── test_movement_actions.py     # Movement calculations
│   ├── test_validators.py           # Input validation
│   ├── test_error_handlers.py       # Error handling
│   ├── test_json_utils.py           # JSON serialization
│   ├── test_request_batcher.py      # Request batching
│   ├── test_circuit_breaker.py      # Circuit breaker pattern
│   ├── test_correlation.py          # Correlation IDs
│   └── test_gpu_manager.py          # GPU management
│
├── integration/ (3 files, 1,471 lines)
│   ├── test_integration_e2e.py      # End-to-end workflows
│   ├── test_integration_db.py       # Database operations
│   └── test_integration_llm.py      # LLM provider switching
│
├── performance/ (3 files, 1,448 lines)
│   ├── test_performance_latency.py  # Latency benchmarks
│   ├── test_performance_throughput.py # Throughput tests
│   └── test_performance_concurrency.py # Concurrency tests
│
└── security/ (3 files, 1,593 lines)
    ├── test_security_auth.py        # Auth security
    ├── test_security_injection.py   # Injection prevention
    └── test_security_dos.py         # DoS protection
```

**Total: 45 files, ~15,000+ lines of test code, 440+ test cases**

## 🚀 Running Tests

### By Category

```bash
# Agent tests (65+ test cases)
pytest tests/agents/ -v

# LLM provider tests (50+ test cases)
pytest tests/llm/ -v

# Router tests (100+ test cases)
pytest tests/routers/ -v

# Utility tests (100+ test cases)
pytest tests/utils/ -v

# Integration tests (20+ test cases)
pytest tests/integration/ -v -m integration

# Performance tests (40+ test cases)
pytest tests/performance/ -v -m performance

# Security tests (65+ test cases)
pytest tests/security/ -v -m security
```

### By Marker

```bash
# Unit tests only
pytest tests/ -v -m unit

# Integration tests only
pytest tests/ -v -m integration

# Performance tests only
pytest tests/ -v -m performance

# Security tests only
pytest tests/ -v -m security

# Slow tests (>1s)
pytest tests/ -v -m slow

# LLM-dependent tests
pytest tests/ -v -m llm

# Database-dependent tests
pytest tests/ -v -m db

# Redis-dependent tests
pytest tests/ -v -m redis
```

### Parallel Execution

```bash
# Install pytest-xdist
pip install pytest-xdist

# Run tests in parallel (auto-detect CPU cores)
pytest tests/ -n auto -v

# Run with specific number of workers
pytest tests/ -n 4 -v
```

### Coverage Reports

```bash
# HTML coverage report
pytest tests/ --cov=. --cov-report=html
open htmlcov/index.html

# Terminal coverage report
pytest tests/ --cov=. --cov-report=term-missing

# XML coverage report (for CI/CD)
pytest tests/ --cov=. --cov-report=xml

# Fail if coverage below 80%
pytest tests/ --cov=. --cov-fail-under=80
```

## 🔍 Debugging Tests

### Verbose Output

```bash
# Extra verbose
pytest tests/ -vv

# Show print statements
pytest tests/ -v -s

# Show local variables on failure
pytest tests/ -v -l

# Stop on first failure
pytest tests/ -x

# Run last failed tests
pytest tests/ --lf

# Run failed tests first, then others
pytest tests/ --ff
```

### Specific Tests

```bash
# Run specific file
pytest tests/agents/test_base_agent.py -v

# Run specific test class
pytest tests/agents/test_base_agent.py::TestBaseAgentExecution -v

# Run specific test method
pytest tests/agents/test_base_agent.py::TestBaseAgentExecution::test_successful_execution -v

# Run tests matching pattern
pytest tests/ -k "test_auth" -v

# Run tests NOT matching pattern
pytest tests/ -k "not slow" -v
```

## 📊 Coverage Targets

### Minimum Coverage Requirements

- **Overall Coverage**: 80%+
- **Agent Tests**: 90%+
- **LLM Provider Tests**: 85%+
- **Router Tests**: 85%+
- **Utility Tests**: 95%+
- **Integration Tests**: 75%+
- **Security Tests**: 90%+

### Checking Coverage

```bash
# Generate coverage report
pytest tests/ --cov=. --cov-report=term-missing

# View detailed coverage
pytest tests/ --cov=. --cov-report=html
open htmlcov/index.html

# Coverage by module
pytest tests/ --cov=agents --cov=llm --cov=routers --cov-report=term-missing
```

## 🔧 CI/CD Integration

### GitHub Actions Example

```yaml
name: Test Suite

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.11'
    
    - name: Install dependencies
      run: |
        pip install -r requirements.txt
        pip install pytest pytest-asyncio pytest-cov pytest-mock
    
    - name: Run tests with coverage
      run: |
        pytest tests/ -v --cov=. --cov-report=xml --cov-fail-under=80
    
    - name: Upload coverage to Codecov
      uses: codecov/codecov-action@v3
      with:
        file: ./coverage.xml
```

### GitLab CI Example

```yaml
test:
  image: python:3.11
  script:
    - pip install -r requirements.txt
    - pip install pytest pytest-asyncio pytest-cov
    - pytest tests/ -v --cov=. --cov-report=term --cov-fail-under=80
  coverage: '/TOTAL.*\s+(\d+%)$/'
```

## 🐛 Common Issues

### Issue: Import Errors

```bash
# Solution: Add service directory to PYTHONPATH
export PYTHONPATH="${PYTHONPATH}:$(pwd)"
pytest tests/ -v
```

### Issue: Async Tests Not Running

```bash
# Solution: Ensure pytest-asyncio is installed
pip install pytest-asyncio

# Verify asyncio mode in pytest.ini
# asyncio_mode = auto
```

### Issue: Database Connection Errors

```bash
# Solution: Use mocked connections (default in tests)
# Or set up test database:
export DATABASE_URL="postgresql://test_user:test_pass@localhost:5432/test_db"
pytest tests/ -v
```

### Issue: LLM API Errors

```bash
# Solution: Tests use mocked LLM responses by default
# To test with real APIs (not recommended for CI):
export OPENAI_API_KEY="your_key"
pytest tests/llm/ -v -m llm
```

## 📈 Performance Benchmarks

### Latency Requirements

- **P50**: < 200ms
- **P95**: < 500ms
- **P99**: < 1000ms

```bash
# Run latency tests
pytest tests/performance/test_performance_latency.py -v
```

### Throughput Requirements

- **Minimum**: 100 requests/second
- **Target**: 500 requests/second
- **Peak**: 1000 requests/second

```bash
# Run throughput tests
pytest tests/performance/test_performance_throughput.py -v
```

### Concurrency Requirements

- **Concurrent Users**: 100+
- **Success Rate**: 99.9%+

```bash
# Run concurrency tests
pytest tests/performance/test_performance_concurrency.py -v
```

## 🛡️ Security Testing

### Authentication Tests

```bash
# Test auth bypass attempts
pytest tests/security/test_security_auth.py -v
```

### Injection Tests

```bash
# Test SQL, JSON, command injection
pytest tests/security/test_security_injection.py -v
```

### DoS Protection Tests

```bash
# Test DoS scenarios
pytest tests/security/test_security_dos.py -v
```

## ✅ Pre-Commit Checklist

Before committing code, run:

```bash
# 1. Format code
black ai-autonomous-world/ai-service/

# 2. Sort imports
isort ai-autonomous-world/ai-service/

# 3. Run linters
flake8 ai-autonomous-world/ai-service/
mypy ai-autonomous-world/ai-service/

# 4. Run all tests with coverage
pytest tests/ -v --cov=. --cov-fail-under=80

# 5. Check security
bandit -r ai-autonomous-world/ai-service/
```

## 📚 Additional Resources

- **pytest Documentation**: https://docs.pytest.org/
- **pytest-asyncio**: https://pytest-asyncio.readthedocs.io/
- **Coverage.py**: https://coverage.readthedocs.io/
- **Test README**: See `tests/README.md` for detailed documentation

## 🎯 Success Criteria

✅ All tests passing
✅ Coverage ≥ 80%
✅ No security vulnerabilities
✅ Performance benchmarks met
✅ All dependencies mocked
✅ CI/CD pipeline green

---

**Created**: 2024-11-22
**Framework**: pytest 7.4.4 + pytest-asyncio 0.23.3 + pytest-cov 4.1.0
**Total Test Cases**: 440+
**Coverage Target**: 80%+