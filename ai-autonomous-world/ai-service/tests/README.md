# 🧪 AI Autonomous World Service - Test Suite Documentation

Complete test suite for the AI-MMORPG world service with comprehensive coverage across routers, utilities, integration, performance, and security.

## 📋 Table of Contents

- [Overview](#overview)
- [Test Structure](#test-structure)
- [Quick Start](#quick-start)
- [Test Categories](#test-categories)
- [Running Tests](#running-tests)
- [Coverage Requirements](#coverage-requirements)
- [Best Practices](#best-practices)
- [CI/CD Integration](#cicd-integration)
- [Troubleshooting](#troubleshooting)

## 🎯 Overview

**Total Test Files**: 31  
**Test Framework**: pytest 9.0.0  
**Coverage Tool**: pytest-cov  
**Performance Tool**: pytest-benchmark  
**Async Support**: pytest-asyncio

### Test Statistics

- **Router Tests**: 14 files (~4,500 lines)
- **Utility Tests**: 8 files (~4,000 lines)
- **Integration Tests**: 3 files (~1,500 lines)
- **Performance Tests**: 3 files (~1,450 lines)
- **Security Tests**: 3 files (~1,600 lines)

**Total Lines of Test Code**: ~13,000+

## 📁 Test Structure

```
tests/
├── __init__.py
├── conftest.py                  # Global fixtures and configuration
├── README.md                    # This file
│
├── routers/                     # FastAPI endpoint tests (14 files)
│   ├── __init__.py
│   ├── test_npc.py             # NPC registration, state, actions
│   ├── test_player.py          # Player interactions, dialogue
│   ├── test_quest.py           # Quest management, completion
│   ├── test_world.py           # World state, environment
│   ├── test_economy.py         # Market, transactions
│   ├── test_faction.py         # Faction management
│   ├── test_relationship.py    # NPC-Player relationships
│   ├── test_chat_command.py    # Chat command processing
│   ├── test_gift.py            # Gift mechanics
│   ├── test_mvp.py             # MVP spawning, behavior
│   ├── test_navigation.py      # Pathfinding, maps
│   ├── test_npc_actions.py     # NPC action execution
│   ├── test_npc_movement.py    # Movement mechanics
│   └── test_emotional_state.py # Emotional AI
│
├── utils/                       # Utility function tests (8 files)
│   ├── __init__.py
│   ├── test_movement_actions.py    # Movement calculations
│   ├── test_validators.py          # Input validation
│   ├── test_error_handlers.py      # Error handling
│   ├── test_json_utils.py          # JSON serialization
│   ├── test_request_batcher.py     # Request batching
│   ├── test_circuit_breaker.py     # Circuit breaker pattern
│   ├── test_correlation.py         # Correlation IDs
│   └── test_gpu_manager.py         # GPU management
│
├── integration/                 # End-to-end tests (3 files)
│   ├── __init__.py
│   ├── test_integration_e2e.py    # Full workflows
│   ├── test_integration_db.py     # Database operations
│   └── test_integration_llm.py    # LLM provider fallback
│
├── performance/                 # Performance tests (3 files)
│   ├── __init__.py
│   ├── test_performance_latency.py     # <500ms p95
│   ├── test_performance_throughput.py  # Requests/second
│   └── test_performance_concurrency.py # 100+ concurrent
│
└── security/                    # Security tests (3 files)
    ├── __init__.py
    ├── test_security_auth.py        # Auth bypass attempts
    ├── test_security_injection.py   # Injection attacks
    └── test_security_dos.py         # DoS protection
```

## 🚀 Quick Start

### Prerequisites

```bash
# Install test dependencies
pip install -r requirements-test.txt

# Or install specific packages
pip install pytest pytest-asyncio pytest-cov pytest-benchmark
```

### Run All Tests

```bash
# Run entire test suite
pytest tests/ -v

# Run with coverage
pytest tests/ --cov=. --cov-report=html --cov-report=term

# Run with markers
pytest tests/ -v -m "not slow"
```

### Run Specific Categories

```bash
# Router tests only
pytest tests/routers/ -v

# Integration tests
pytest tests/integration/ -v -m integration

# Performance tests
pytest tests/performance/ --benchmark-only

# Security tests
pytest tests/security/ -v -m security
```

## 📊 Test Categories

### 1. Router Tests (Unit)

**Purpose**: Test FastAPI endpoints with mocked dependencies

**Coverage**:
- ✅ Request/response validation
- ✅ Error handling (400, 404, 500)
- ✅ Authentication/authorization
- ✅ Business logic
- ✅ Edge cases

**Example**:
```bash
pytest tests/routers/test_npc.py::TestNPCRegistration -v
```

### 2. Utility Tests (Unit)

**Purpose**: Test utility functions and helper classes

**Coverage**:
- ✅ Input validation
- ✅ Data transformations
- ✅ Error handling
- ✅ Performance optimizations
- ✅ Thread safety

**Example**:
```bash
pytest tests/utils/test_validators.py -v
```

### 3. Integration Tests

**Purpose**: Test complete workflows across multiple components

**Coverage**:
- ✅ End-to-end player interactions
- ✅ Database transaction integrity
- ✅ LLM provider fallback mechanisms
- ✅ Error propagation
- ✅ Data consistency

**Example**:
```bash
pytest tests/integration/test_integration_e2e.py -v
```

### 4. Performance Tests

**Purpose**: Validate performance requirements

**Requirements**:
- ✅ Latency: p95 < 500ms
- ✅ Throughput: > 100 requests/second
- ✅ Concurrency: Handle 100+ simultaneous requests

**Example**:
```bash
pytest tests/performance/ --benchmark-only --benchmark-sort=mean
```

### 5. Security Tests

**Purpose**: Validate security measures and prevent vulnerabilities

**Coverage**:
- ✅ Authentication bypass prevention
- ✅ SQL/NoSQL injection resistance
- ✅ XSS protection
- ✅ Command injection prevention
- ✅ DoS protection
- ✅ Rate limiting

**Example**:
```bash
pytest tests/security/test_security_injection.py -v
```

## 🎯 Coverage Requirements

### Minimum Coverage Targets

- **Overall**: 85%
- **Routers**: 90%
- **Utilities**: 95%
- **Models**: 80%
- **Agents**: 75%

### Generate Coverage Report

```bash
# HTML report
pytest tests/ --cov=. --cov-report=html
open htmlcov/index.html

# Terminal report
pytest tests/ --cov=. --cov-report=term-missing

# XML report (for CI/CD)
pytest tests/ --cov=. --cov-report=xml
```

### Coverage by Module

```bash
# Specific module coverage
pytest tests/routers/ --cov=routers --cov-report=term

# With branch coverage
pytest tests/ --cov=. --cov-branch --cov-report=term
```

## 💡 Best Practices

### 1. Test Naming Convention

```python
# ✅ Good
def test_register_npc_success(client, sample_npc_data):
    """Test successful NPC registration"""
    
def test_register_npc_duplicate(client, sample_npc_data):
    """Test duplicate NPC registration rejection"""

# ❌ Bad
def test_npc1(client):
    pass
```

### 2. Use Fixtures

```python
@pytest.fixture
def sample_npc_data():
    """Reusable NPC test data"""
    return {
        "npc_id": "test_001",
        "name": "Test NPC",
        "level": 50
    }
```

### 3. Mock External Dependencies

```python
def test_llm_call(client):
    with patch('llm.get_llm_provider') as mock_llm:
        mock_llm.return_value.generate = AsyncMock(return_value="Response")
        # Test logic here
```

### 4. Use Markers

```python
@pytest.mark.asyncio
@pytest.mark.slow
@pytest.mark.integration
async def test_complex_workflow():
    # Test implementation
```

### 5. Arrange-Act-Assert Pattern

```python
def test_example():
    # Arrange
    data = {"key": "value"}
    
    # Act
    result = process_data(data)
    
    # Assert
    assert result["status"] == "success"
```

## 🔧 Running Specific Tests

### By File

```bash
pytest tests/routers/test_npc.py -v
```

### By Class

```bash
pytest tests/routers/test_npc.py::TestNPCRegistration -v
```

### By Function

```bash
pytest tests/routers/test_npc.py::TestNPCRegistration::test_register_npc_success -v
```

### By Marker

```bash
# Run only fast tests
pytest tests/ -v -m "not slow"

# Run only async tests
pytest tests/ -v -m asyncio

# Run integration tests
pytest tests/ -v -m integration

# Run performance tests
pytest tests/ -v -m performance

# Run security tests
pytest tests/ -v -m security
```

### By Keyword

```bash
# Tests containing "npc"
pytest tests/ -k "npc" -v

# Tests NOT containing "slow"
pytest tests/ -k "not slow" -v
```

## 🔄 CI/CD Integration

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
        pip install -r requirements-test.txt
    
    - name: Run tests with coverage
      run: |
        pytest tests/ --cov=. --cov-report=xml --cov-report=term
    
    - name: Upload coverage
      uses: codecov/codecov-action@v3
      with:
        file: ./coverage.xml
```

### GitLab CI Example

```yaml
test:
  image: python:3.11
  script:
    - pip install -r requirements-test.txt
    - pytest tests/ --cov=. --cov-report=xml --cov-report=term
  coverage: '/TOTAL.*\s+(\d+%)$/'
  artifacts:
    reports:
      coverage_report:
        coverage_format: cobertura
        path: coverage.xml
```

## 🐛 Troubleshooting

### Common Issues

#### 1. Import Errors

```bash
# Add parent directory to Python path
export PYTHONPATH="${PYTHONPATH}:$(pwd)"
pytest tests/
```

#### 2. Async Tests Failing

```bash
# Ensure pytest-asyncio is installed
pip install pytest-asyncio

# Use proper async markers
@pytest.mark.asyncio
async def test_async_function():
    result = await async_operation()
    assert result is not None
```

#### 3. Database Connection Issues

```bash
# Use test database fixtures
@pytest.fixture
async def test_db():
    # Setup test database
    db = TestDatabase()
    await db.connect()
    yield db
    await db.disconnect()
```

#### 4. Mock Not Working

```python
# Correct import path
with patch('routers.npc.db.client') as mock_db:  # ✅
    # Not patch('database.db.client')  # ❌
```

#### 5. Performance Tests Inconsistent

```bash
# Use warmup and multiple rounds
pytest tests/performance/ --benchmark-warmup=on --benchmark-min-rounds=10
```

### Debug Mode

```bash
# Verbose output
pytest tests/ -vv

# Show print statements
pytest tests/ -s

# Stop on first failure
pytest tests/ -x

# Drop into debugger on failure
pytest tests/ --pdb

# Show local variables on failure
pytest tests/ -l
```

### Parallel Execution

```bash
# Install pytest-xdist
pip install pytest-xdist

# Run tests in parallel
pytest tests/ -n auto

# Specific number of workers
pytest tests/ -n 4
```

## 📈 Test Metrics

### Performance Benchmarks

| Endpoint | p50 | p95 | p99 | Target |
|----------|-----|-----|-----|--------|
| Health | 5ms | 10ms | 15ms | <50ms |
| NPC Dialogue | 200ms | 450ms | 500ms | <500ms |
| Quest Assign | 150ms | 400ms | 480ms | <500ms |
| World State | 100ms | 300ms | 450ms | <500ms |

### Security Test Coverage

| Category | Tests | Coverage |
|----------|-------|----------|
| Authentication | 20+ | 100% |
| Injection | 25+ | 95% |
| DoS Protection | 20+ | 100% |
| XSS Prevention | 15+ | 100% |

## 🔗 Related Documentation

- [Main README](../README.md)
- [API Documentation](../docs/API.md)
- [Architecture Guide](../docs/ARCHITECTURE.md)
- [Configuration Guide](../docs/CONFIGURATION.md)

## 🤝 Contributing

### Adding New Tests

1. Create test file in appropriate directory
2. Follow naming convention: `test_*.py`
3. Use existing fixtures from `conftest.py`
4. Add appropriate markers
5. Keep files < 500 lines
6. Document test purpose in docstrings
7. Run test suite to ensure no regressions

### Test Review Checklist

- [ ] Tests pass locally
- [ ] Coverage maintained/improved
- [ ] No hardcoded secrets
- [ ] Proper use of fixtures
- [ ] Appropriate markers applied
- [ ] Clear test names and docstrings
- [ ] Edge cases covered
- [ ] Performance considered

## 📞 Support

For test-related issues:
- Check [Troubleshooting](#troubleshooting) section
- Review existing test examples
- Consult pytest documentation: https://docs.pytest.org/
- Open an issue with test failure details

---

**Last Updated**: 2025-11-22  
**Test Suite Version**: 1.0.0  
**Pytest Version**: 9.0.0