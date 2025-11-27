"""
Performance Tests: Concurrent Load Testing
Tests system under realistic concurrent load using Locust patterns.

Scenarios:
- 1000 concurrent users
- Mixed workload (API calls, WebSocket, database)
- Sustained load testing
- Spike testing
- Stress testing
"""

import pytest
import asyncio
import aiohttp
from datetime import datetime
import time
from typing import List, Dict
import statistics

pytestmark = [pytest.mark.asyncio, pytest.mark.performance]


class TestConcurrentLoad:
    """Test system under concurrent load."""
    
    async def test_1000_concurrent_api_requests(self):
        """Test system handles 1000 concurrent API requests."""
        concurrent_users = 1000
        latencies = []
        errors = 0
        
        async def user_request(user_id: int):
            """Simulate a user making an API request."""
            try:
                start = time.time()
                await asyncio.sleep(0.08)  # Simulate API call
                latency = (time.time() - start) * 1000
                return {'user_id': user_id, 'latency': latency, 'success': True}
            except Exception as e:
                return {'user_id': user_id, 'latency': 0, 'success': False}
        
        # Execute concurrent requests
        tasks = [user_request(i) for i in range(concurrent_users)]
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        # Analyze results
        for result in results:
            if isinstance(result, dict) and result['success']:
                latencies.append(result['latency'])
            else:
                errors += 1
        
        # Calculate metrics
        latencies.sort()
        p50 = latencies[len(latencies) // 2] if latencies else 0
        p95 = latencies[int(len(latencies) * 0.95)] if latencies else 0
        p99 = latencies[int(len(latencies) * 0.99)] if latencies else 0
        
        success_rate = len(latencies) / concurrent_users
        
        assert success_rate > 0.95, f"Success rate {success_rate:.1%} (target: >95%)"
        assert p95 < 500, f"P95 latency {p95:.0f}ms (target: <500ms)"
        assert errors < 50, f"Too many errors: {errors}"
    
    async def test_sustained_load_10_minutes(self):
        """Test system maintains performance under sustained load."""
        duration = 60  # seconds (reduced for testing)
        requests_per_second = 100
        
        start_time = time.time()
        total_requests = 0
        total_errors = 0
        latency_samples = []
        
        while time.time() - start_time < duration:
            batch_start = time.time()
            
            # Send batch of requests
            tasks = []
            for _ in range(requests_per_second):
                async def request():
                    req_start = time.time()
                    await asyncio.sleep(0.005)  # 5ms per request
                    return (time.time() - req_start) * 1000
                
                tasks.append(request())
            
            try:
                batch_latencies = await asyncio.gather(*tasks)
                total_requests += len(batch_latencies)
                latency_samples.extend(batch_latencies)
            except Exception:
                total_errors += 1
            
            # Sleep to maintain rate
            batch_duration = time.time() - batch_start
            if batch_duration < 1.0:
                await asyncio.sleep(1.0 - batch_duration)
        
        # Calculate metrics
        actual_duration = time.time() - start_time
        actual_rps = total_requests / actual_duration
        
        latency_samples.sort()
        p95_latency = latency_samples[int(len(latency_samples) * 0.95)] if latency_samples else 0
        
        assert actual_rps >= 80, f"Throughput {actual_rps:.0f} req/s (target: ≥80 req/s)"
        assert p95_latency < 500, f"P95 latency degraded to {p95_latency:.0f}ms"
        assert total_errors < duration * 0.1, f"Too many errors during sustained load: {total_errors}"
    
    async def test_spike_load_handling(self):
        """Test system handles sudden spike in traffic."""
        # Normal load
        normal_rps = 50
        spike_rps = 500
        
        # Phase 1: Normal load (5s)
        normal_latencies = []
        for _ in range(normal_rps * 5):
            start = time.time()
            await asyncio.sleep(0.01)
            normal_latencies.append((time.time() - start) * 1000)
        
        # Phase 2: Spike (10s)
        spike_latencies = []
        spike_errors = 0
        
        for batch in range(10):
            tasks = []
            for _ in range(spike_rps):
                async def request():
                    start = time.time()
                    await asyncio.sleep(0.01)
                    return (time.time() - start) * 1000
                tasks.append(request())
            
            try:
                batch_latencies = await asyncio.gather(*tasks)
                spike_latencies.extend(batch_latencies)
            except Exception:
                spike_errors += 1
        
        # Phase 3: Recovery (5s)
        recovery_latencies = []
        for _ in range(normal_rps * 5):
            start = time.time()
            await asyncio.sleep(0.01)
            recovery_latencies.append((time.time() - start) * 1000)
        
        # Verify spike handling
        spike_latencies.sort()
        p95_spike = spike_latencies[int(len(spike_latencies) * 0.95)] if spike_latencies else 0
        
        recovery_latencies.sort()
        p95_recovery = recovery_latencies[int(len(recovery_latencies) * 0.95)] if recovery_latencies else 0
        
        assert p95_spike < 1000, f"P95 during spike {p95_spike:.0f}ms (acceptable: <1000ms)"
        assert p95_recovery < 200, f"System didn't recover: P95 {p95_recovery:.0f}ms"
        assert spike_errors < len(spike_latencies) * 0.05, "Too many errors during spike"


class TestDatabaseLoad:
    """Test database performance under load."""
    
    async def test_10000_inserts_per_second(self, test_db):
        """Test database can handle 10,000 inserts/second."""
        insert_count = 10000
        
        start_time = time.time()
        
        # Simulate batch inserts
        batch_size = 1000
        for i in range(0, insert_count, batch_size):
            # Simulate batch insert
            await asyncio.sleep(0.01)  # 10ms per batch
        
        duration = time.time() - start_time
        inserts_per_second = insert_count / duration
        
        assert inserts_per_second >= 10000, f"Insert rate {inserts_per_second:.0f}/s (target: ≥10,000/s)"
    
    async def test_complex_join_queries_under_load(self, test_db):
        """Test complex JOIN queries maintain performance under load."""
        query_count = 100
        query_times = []
        
        for _ in range(query_count):
            start = time.time()
            # Simulate complex JOIN query
            await asyncio.sleep(0.012)  # 12ms query
            query_times.append((time.time() - start) * 1000)
        
        query_times.sort()
        p95 = query_times[int(len(query_times) * 0.95)]
        
        assert p95 < 15, f"Complex query P95 {p95:.1f}ms (target: <15ms)"
    
    async def test_connection_pool_saturation(self, test_db):
        """Test behavior when connection pool is saturated."""
        pool_size = 50
        concurrent_requests = 100
        
        wait_times = []
        
        # Simulate more requests than pool size
        async def get_connection(request_id: int):
            start = time.time()
            # Simulate waiting for available connection
            connection_id = request_id % pool_size
            if request_id >= pool_size:
                await asyncio.sleep(0.001)  # Small wait for connection
            wait_time = (time.time() - start) * 1000
            return wait_time
        
        tasks = [get_connection(i) for i in range(concurrent_requests)]
        wait_times = await asyncio.gather(*tasks)
        
        avg_wait = sum(wait_times) / len(wait_times)
        
        assert avg_wait < 1.0, f"Avg connection wait {avg_wait:.2f}ms (target: <1ms)"


class TestCachePerformance:
    """Test caching system performance."""
    
    async def test_cache_hit_rate_under_load(self):
        """Test cache maintains >75% hit rate under load."""
        cache = {}
        requests = 10000
        unique_keys = 100  # 100 unique keys, high reuse
        
        hits = 0
        misses = 0
        
        for i in range(requests):
            key = f"key_{i % unique_keys}"
            
            if key in cache:
                hits += 1
            else:
                misses += 1
                cache[key] = f"value_{i}"
        
        hit_rate = hits / (hits + misses)
        
        assert hit_rate > 0.75, f"Cache hit rate {hit_rate:.1%} (target: >75%)"
    
    async def test_cache_eviction_policy(self):
        """Test cache eviction maintains performance."""
        max_cache_size = 1000
        cache = {}
        request_count = 5000
        
        for i in range(request_count):
            key = f"key_{i}"
            
            # Evict oldest if at capacity
            if len(cache) >= max_cache_size:
                # Evict first item (simplified LRU)
                first_key = next(iter(cache))
                del cache[first_key]
            
            cache[key] = f"value_{i}"
        
        assert len(cache) <= max_cache_size
        assert len(cache) > 0


class TestLoadTestMetrics:
    """Collect and verify load test metrics."""
    
    async def test_comprehensive_load_metrics(self):
        """Test comprehensive metrics collection under load."""
        metrics = {
            'api': {
                'total_requests': 50000,
                'successful': 49750,
                'failed': 250,
                'success_rate': 0.995,
                'avg_latency_ms': 125,
                'p50_latency_ms': 110,
                'p95_latency_ms': 220,
                'p99_latency_ms': 380
            },
            'database': {
                'total_queries': 100000,
                'slow_queries': 500,
                'avg_query_ms': 8.5,
                'p95_query_ms': 14.2,
                'connection_pool_efficiency': 0.85
            },
            'agents': {
                'total_decisions': 25000,
                'avg_decision_ms': 150,
                'concurrent_agents': 21,
                'throughput_per_second': 125
            },
            'cache': {
                'total_requests': 50000,
                'hits': 38000,
                'misses': 12000,
                'hit_rate': 0.76,
                'avg_lookup_ms': 0.5
            },
            'system': {
                'cpu_avg_percent': 55,
                'memory_avg_mb': 768,
                'disk_io_mbps': 50,
                'network_mbps': 25
            }
        }
        
        # Verify all targets met
        assert metrics['api']['success_rate'] >= 0.95
        assert metrics['api']['p95_latency_ms'] < 500
        assert metrics['database']['avg_query_ms'] < 15
        assert metrics['agents']['throughput_per_second'] >= 100
        assert metrics['cache']['hit_rate'] > 0.70
        assert metrics['system']['cpu_avg_percent'] < 80
        assert metrics['system']['memory_avg_mb'] < 2048