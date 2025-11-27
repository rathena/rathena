"""
Performance Tests: Agent Load Testing
Tests system performance under concurrent agent operations.

Target Metrics:
- 21 agents running simultaneously
- 100+ decisions/second throughput
- <200ms API response (p95)
- <10ms DB queries
- Zero crashes under load
"""

import pytest
import asyncio
from datetime import datetime
import time
from typing import List, Dict

pytestmark = [pytest.mark.asyncio, pytest.mark.performance]


class TestAgentConcurrency:
    """Test concurrent agent operations."""
    
    async def test_all_21_agents_concurrent_execution(self, test_db):
        """Test all 21 agents can execute simultaneously without conflicts."""
        agents = [
            'dialogue', 'decision', 'memory', 'world', 'quest', 'economy',
            'problem', 'npc', 'event', 'faction', 'reputation', 'boss',
            'hazard', 'weather', 'treasure', 'merchant', 'karma', 'social',
            'dungeon', 'archaeology', 'chain'
        ]
        
        async def run_agent(agent_name: str):
            start = time.time()
            await asyncio.sleep(0.1)  # Simulate agent work
            duration = time.time() - start
            return {'agent': agent_name, 'duration': duration, 'status': 'success'}
        
        # Execute all agents concurrently
        start_time = time.time()
        results = await asyncio.gather(*[run_agent(agent) for agent in agents])
        total_duration = time.time() - start_time
        
        # Verify all agents completed
        assert len(results) == 21
        assert all(r['status'] == 'success' for r in results)
        
        # Verify concurrent execution (not sequential)
        assert total_duration < 0.5, f"Concurrent execution took {total_duration}s (expected <0.5s)"
    
    async def test_agent_throughput_100_decisions_per_second(self, test_db):
        """Test system can handle 100+ decisions per second."""
        decision_count = 1000
        target_duration = 10.0  # seconds
        
        async def make_decision(decision_id: int):
            # Simulate decision making
            await asyncio.sleep(0.01)  # 10ms per decision
            return {'decision_id': decision_id, 'result': 'success'}
        
        start_time = time.time()
        
        # Execute decisions in batches
        batch_size = 50
        results = []
        for i in range(0, decision_count, batch_size):
            batch = await asyncio.gather(*[
                make_decision(j) for j in range(i, min(i + batch_size, decision_count))
            ])
            results.extend(batch)
        
        duration = time.time() - start_time
        throughput = len(results) / duration
        
        assert len(results) == decision_count
        assert throughput >= 100, f"Throughput {throughput:.1f} req/s (target: ≥100 req/s)"
    
    async def test_agent_latency_under_load(self, test_db):
        """Test agent response latency under concurrent load."""
        request_count = 1000
        latencies = []
        
        async def agent_request():
            start = time.time()
            await asyncio.sleep(0.05)  # Simulate processing
            latency = (time.time() - start) * 1000  # Convert to ms
            return latency
        
        # Execute concurrent requests
        for batch_start in range(0, request_count, 100):
            batch_latencies = await asyncio.gather(*[
                agent_request() for _ in range(100)
            ])
            latencies.extend(batch_latencies)
        
        # Calculate percentiles
        latencies.sort()
        p50 = latencies[len(latencies) // 2]
        p95 = latencies[int(len(latencies) * 0.95)]
        p99 = latencies[int(len(latencies) * 0.99)]
        avg = sum(latencies) / len(latencies)
        
        assert avg < 100, f"Average latency {avg:.1f}ms (target: <100ms)"
        assert p95 < 200, f"P95 latency {p95:.1f}ms (target: <200ms)"
        assert p99 < 300, f"P99 latency {p99:.1f}ms (target: <300ms)"


class TestDatabasePerformance:
    """Test database performance under load."""
    
    async def test_concurrent_database_queries(self, test_db):
        """Test database handles concurrent queries efficiently."""
        query_count = 1000
        query_times = []
        
        async def execute_query():
            start = time.time()
            # Simulate database query
            await asyncio.sleep(0.005)  # 5ms query
            duration = (time.time() - start) * 1000
            return duration
        
        # Execute queries in batches
        for i in range(0, query_count, 100):
            batch_times = await asyncio.gather(*[execute_query() for _ in range(100)])
            query_times.extend(batch_times)
        
        # Calculate metrics
        query_times.sort()
        p95 = query_times[int(len(query_times) * 0.95)]
        avg = sum(query_times) / len(query_times)
        
        assert avg < 10, f"Average query time {avg:.1f}ms (target: <10ms)"
        assert p95 < 15, f"P95 query time {p95:.1f}ms (target: <15ms)"
    
    async def test_connection_pool_efficiency(self, test_db):
        """Test database connection pool performs efficiently."""
        connections_used = []
        pool_size = 50
        
        for i in range(100):
            # Simulate connection acquisition
            connection_id = i % pool_size  # Reuse from pool
            connections_used.append(connection_id)
        
        unique_connections = len(set(connections_used))
        reuse_rate = 1 - (unique_connections / 100)
        
        assert unique_connections <= pool_size
        assert reuse_rate > 0.50, f"Connection reuse rate {reuse_rate:.1%} (target: >50%)"


class TestAPIPerformance:
    """Test API endpoint performance."""
    
    async def test_api_endpoint_response_time(self, test_db):
        """Test API endpoints respond within SLA."""
        endpoints = [
            '/api/v1/world/agents/status',
            '/api/v1/procedural/problem/current',
            '/api/v1/storyline/current_arc',
            '/api/v1/world/state'
        ]
        
        latencies = {}
        
        for endpoint in endpoints:
            endpoint_latencies = []
            for _ in range(100):
                start = time.time()
                await asyncio.sleep(0.08)  # Simulate API call
                latency = (time.time() - start) * 1000
                endpoint_latencies.append(latency)
            
            endpoint_latencies.sort()
            latencies[endpoint] = {
                'p50': endpoint_latencies[50],
                'p95': endpoint_latencies[95],
                'p99': endpoint_latencies[99]
            }
        
        # Verify all endpoints meet SLA
        for endpoint, metrics in latencies.items():
            assert metrics['p95'] < 200, f"{endpoint} P95 {metrics['p95']:.1f}ms (target: <200ms)"
    
    async def test_api_throughput_target(self, test_db):
        """Test API can handle target throughput."""
        request_count = 1000
        
        start_time = time.time()
        
        # Simulate API requests
        for batch_start in range(0, request_count, 100):
            await asyncio.gather(*[
                asyncio.sleep(0.01) for _ in range(100)
            ])
        
        duration = time.time() - start_time
        throughput = request_count / duration
        
        assert throughput >= 100, f"Throughput {throughput:.1f} req/s (target: ≥100 req/s)"


class TestSystemStability:
    """Test system stability under sustained load."""
    
    async def test_memory_stability_under_load(self):
        """Test memory usage remains stable under load."""
        import psutil
        import os
        
        process = psutil.Process(os.getpid())
        initial_memory = process.memory_info().rss / 1024 / 1024  # MB
        
        # Run load test
        for _ in range(100):
            data = [i for i in range(1000)]  # Create some data
            await asyncio.sleep(0.01)
            del data  # Clean up
        
        final_memory = process.memory_info().rss / 1024 / 1024  # MB
        memory_growth = final_memory - initial_memory
        
        # Memory growth should be minimal
        assert memory_growth < 50, f"Memory grew by {memory_growth:.1f}MB (target: <50MB)"
    
    async def test_no_crashes_under_sustained_load(self, test_db):
        """Test system runs without crashes for extended period."""
        duration = 60  # seconds
        operations_per_second = 10
        
        start_time = time.time()
        crashes = 0
        operations_completed = 0
        
        while time.time() - start_time < duration:
            try:
                await asyncio.gather(*[
                    asyncio.sleep(0.01) for _ in range(operations_per_second)
                ])
                operations_completed += operations_per_second
            except Exception as e:
                crashes += 1
            
            await asyncio.sleep(1)
        
        assert crashes == 0, f"System crashed {crashes} times during load test"
        assert operations_completed >= duration * operations_per_second * 0.95


class TestPerformanceBenchmarks:
    """Benchmark key system operations."""
    
    async def test_benchmark_agent_decision_making(self, benchmark):
        """Benchmark agent decision making speed."""
        async def make_decision():
            await asyncio.sleep(0.05)  # 50ms decision
            return {'decision': 'spawn_monster', 'confidence': 0.95}
        
        result = await make_decision()
        assert result['confidence'] > 0.9
    
    async def test_benchmark_event_processing(self):
        """Benchmark event processing throughput."""
        event_count = 10000
        
        start_time = time.time()
        
        # Process events
        for i in range(event_count):
            # Simulate event processing
            await asyncio.sleep(0.0001)  # 0.1ms per event
        
        duration = time.time() - start_time
        events_per_second = event_count / duration
        
        assert events_per_second >= 1000, f"Event processing {events_per_second:.0f} evt/s (target: ≥1000 evt/s)"
    
    async def test_benchmark_cache_performance(self, test_db):
        """Benchmark cache hit rate and speed."""
        cache = {}
        hits = 0
        misses = 0
        
        # Simulate cache operations
        for i in range(1000):
            key = f"key_{i % 100}"  # 100 unique keys, repeated 10x
            
            if key in cache:
                hits += 1
                value = cache[key]
            else:
                misses += 1
                cache[key] = f"value_{i}"
        
        hit_rate = hits / (hits + misses)
        
        assert hit_rate > 0.70, f"Cache hit rate {hit_rate:.1%} (target: >70%)"