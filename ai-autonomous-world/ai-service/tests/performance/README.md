# Performance Testing Infrastructure

## Overview

This directory contains performance testing tools for the AI Autonomous World Service:

1. **Locust Load Testing** (`locustfile.py`) - HTTP load testing with web UI
2. **Pytest Benchmarks** (`test_benchmarks.py`) - Micro-benchmarks for critical operations
3. **Profiling Tools** - CPU and memory profiling utilities

## Installation

```bash
# Install performance testing dependencies
pip install locust pytest-benchmark pytest-asyncio memory_profiler py-spy

# Or use requirements file
pip install -r requirements-test.txt
```

## Locust Load Testing

### Quick Start

```bash
# Run with web UI (open http://localhost:8089)
locust -f tests/performance/locustfile.py --host=http://localhost:8000

# Run headless with 100 users, 10 users/second spawn rate, 5 minute duration
locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
       --users 100 --spawn-rate 10 --run-time 5m --headless

# Run specific test class
locust -f tests/performance/locustfile.py NPCInteractionTest --host=http://localhost:8000
```

### Test Classes

#### NPCInteractionTest
Tests NPC interaction endpoints with realistic load patterns:
- **player_interaction** (weight: 5) - Most common operation
- **get_npc_state** (weight: 2) - State retrieval
- **get_world_state** (weight: 1) - World state queries
- **get_environment_state** (weight: 1) - Environment queries

#### ChatCommandTest
Tests free-form chat command performance:
- Simulates natural language input from players
- Tests AI response generation latency

### Performance Targets

| Metric | Target | Acceptable |
|--------|--------|------------|
| **Response Time (p50)** | < 200ms | < 500ms |
| **Response Time (p95)** | < 500ms | < 1000ms |
| **Response Time (p99)** | < 1000ms | < 2000ms |
| **Throughput** | > 100 req/s | > 50 req/s |
| **Error Rate** | < 0.1% | < 1% |

## Pytest Benchmarks

### Quick Start

```bash
# Run all benchmarks
pytest tests/performance/test_benchmarks.py -v --benchmark-only

# Run specific benchmark group
pytest tests/performance/test_benchmarks.py -v --benchmark-only -k "npc_registration"

# Compare benchmarks
pytest tests/performance/test_benchmarks.py -v --benchmark-compare

# Save benchmark results
pytest tests/performance/test_benchmarks.py -v --benchmark-only --benchmark-save=baseline

# Compare against baseline
pytest tests/performance/test_benchmarks.py -v --benchmark-only --benchmark-compare=baseline
```

### Benchmark Groups

1. **npc_registration** - NPC registration performance
2. **npc_state_update** - NPC state update performance
3. **environment_update** - Environment system updates
4. **database_operations** - Redis operations
5. **configuration** - Configuration loading

### Performance Baselines

| Operation | Target | Acceptable |
|-----------|--------|------------|
| **NPC Registration** | < 1ms | < 5ms |
| **NPC State Update** | < 0.5ms | < 2ms |
| **Weather Update** | < 0.1ms | < 1ms |
| **Redis SET** | < 1ms | < 5ms |
| **Redis GET** | < 0.5ms | < 2ms |
| **Config Load** | < 10ms | < 50ms |

## Profiling

### CPU Profiling with py-spy

```bash
# Profile running service
py-spy top --pid $(pgrep -f "python.*main.py")

# Record flame graph
py-spy record -o profile.svg --pid $(pgrep -f "python.*main.py") --duration 60

# Profile specific function
py-spy record -o profile.svg -- python -m pytest tests/performance/test_benchmarks.py
```

### Memory Profiling

```bash
# Profile memory usage
python -m memory_profiler main.py

# Generate memory report
mprof run main.py
mprof plot
```

### cProfile

```bash
# Profile with cProfile
python -m cProfile -o profile.stats main.py

# Analyze results
python -c "import pstats; p = pstats.Stats('profile.stats'); p.sort_stats('cumulative').print_stats(20)"
```

## Load Testing Scenarios

### Scenario 1: Normal Load (100 concurrent users)
```bash
locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
       --users 100 --spawn-rate 10 --run-time 10m --headless
```

### Scenario 2: Peak Load (500 concurrent users)
```bash
locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
       --users 500 --spawn-rate 50 --run-time 5m --headless
```

### Scenario 3: Stress Test (1000 concurrent users)
```bash
locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
       --users 1000 --spawn-rate 100 --run-time 3m --headless
```

### Scenario 4: Spike Test (rapid ramp-up)
```bash
locust -f tests/performance/locustfile.py --host=http://localhost:8000 \
       --users 1000 --spawn-rate 500 --run-time 2m --headless
```

## Monitoring During Tests

### System Metrics
```bash
# Monitor CPU and memory
htop

# Monitor network
iftop

# Monitor disk I/O
iotop
```

### Service Metrics
```bash
# Monitor AI Service logs
tail -f logs/ai-service.log

# Monitor DragonflyDB
redis-cli -p 6379 INFO stats

# Monitor PostgreSQL
psql -U postgres -d ai_world_memory -c "SELECT * FROM pg_stat_activity;"
```

## Results Analysis

### Locust Results
- Check `locust_report.html` for detailed results
- Review response time distribution
- Analyze failure rates and error messages
- Check throughput over time

### Benchmark Results
- Compare against baselines
- Identify performance regressions
- Track performance trends over time

## Continuous Performance Testing

### CI/CD Integration
```yaml
# .github/workflows/performance.yml
name: Performance Tests
on: [push, pull_request]
jobs:
  performance:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run benchmarks
        run: pytest tests/performance/test_benchmarks.py --benchmark-only
```

## Troubleshooting

### High Latency
1. Check LLM provider response times
2. Verify database connection pool settings
3. Review cache hit rates
4. Check for N+1 query problems

### Low Throughput
1. Increase worker processes
2. Enable connection pooling
3. Optimize database queries
4. Enable caching

### Memory Leaks
1. Profile with memory_profiler
2. Check for circular references
3. Review async task cleanup
4. Monitor connection pool sizes

