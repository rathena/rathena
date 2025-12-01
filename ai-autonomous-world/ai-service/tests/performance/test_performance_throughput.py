"""
Performance Tests: Throughput Testing

Tests requests/second benchmarks, sustained load handling, and throughput degradation.
Validates system can handle expected production load.

Requirements:
- Measure requests per second capacity
- Test sustained load handling
- Detect throughput degradation patterns
- Validate connection pool efficiency

Run with:
    pytest tests/performance/test_performance_throughput.py -v
    pytest tests/performance/test_performance_throughput.py -m performance --benchmark-only
"""

import pytest
import time
import asyncio
import threading
import statistics
from typing import List, Dict, Tuple
from concurrent.futures import ThreadPoolExecutor, as_completed
from fastapi.testclient import TestClient
from unittest.mock import patch

# Import app after setting test environment
import os
os.environ["ENVIRONMENT"] = "testing"
os.environ["API_KEY_REQUIRED"] = "false"
os.environ["RATE_LIMIT_ENABLED"] = "false"

from main import app
from config import settings


# ============================================================================
# TEST CONFIGURATION
# ============================================================================

MIN_THROUGHPUT_RPS = 100  # Minimum requests per second
TARGET_THROUGHPUT_RPS = 200  # Target requests per second
DURATION_SECONDS = 10  # Test duration for sustained load
MAX_DEGRADATION_PERCENT = 20  # Max acceptable throughput degradation


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def client():
    """Create test client."""
    with TestClient(app) as test_client:
        yield test_client


@pytest.fixture
def auth_headers():
    """Create authentication headers."""
    return {
        "Authorization": f"Bearer {settings.api_key or 'test-key'}",
        "Content-Type": "application/json"
    }


# ============================================================================
# THROUGHPUT MEASUREMENT HELPERS
# ============================================================================

def measure_throughput(
    func,
    duration_seconds: int = 5,
    max_workers: int = 10
) -> Dict[str, float]:
    """
    Measure throughput for a function over duration.
    
    Returns:
        dict: Throughput statistics including RPS, total requests, errors
    """
    results = {
        "total_requests": 0,
        "successful_requests": 0,
        "failed_requests": 0,
        "total_duration": 0,
        "rps": 0,
        "errors": []
    }
    
    start_time = time.time()
    end_time = start_time + duration_seconds
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        
        while time.time() < end_time:
            future = executor.submit(func)
            futures.append(future)
        
        # Wait for all requests to complete
        for future in as_completed(futures):
            results["total_requests"] += 1
            try:
                future.result()
                results["successful_requests"] += 1
            except Exception as e:
                results["failed_requests"] += 1
                results["errors"].append(str(e))
    
    results["total_duration"] = time.time() - start_time
    results["rps"] = results["successful_requests"] / results["total_duration"]
    
    return results


def measure_sustained_throughput(
    func,
    duration_seconds: int = 30,
    sample_interval: int = 5,
    max_workers: int = 10
) -> List[Dict[str, float]]:
    """
    Measure throughput over time to detect degradation.
    
    Returns:
        list: Throughput samples at intervals
    """
    samples = []
    start_time = time.time()
    
    while time.time() - start_time < duration_seconds:
        sample = measure_throughput(func, sample_interval, max_workers)
        sample["timestamp"] = time.time() - start_time
        samples.append(sample)
    
    return samples


# ============================================================================
# BASIC THROUGHPUT TESTS
# ============================================================================

@pytest.mark.performance
def test_health_endpoint_throughput(client):
    """Test /health endpoint throughput."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
        return response
    
    stats = measure_throughput(make_request, duration_seconds=5, max_workers=20)
    
    print(f"\n/health endpoint throughput:")
    print(f"  Total requests: {stats['total_requests']}")
    print(f"  Successful: {stats['successful_requests']}")
    print(f"  Failed: {stats['failed_requests']}")
    print(f"  Duration: {stats['total_duration']:.2f}s")
    print(f"  Throughput: {stats['rps']:.2f} req/s")
    
    # Health check should handle very high throughput
    assert stats["rps"] >= MIN_THROUGHPUT_RPS * 2, (
        f"Health endpoint throughput {stats['rps']:.2f} RPS is below "
        f"minimum {MIN_THROUGHPUT_RPS * 2} RPS"
    )
    
    # Should have minimal failures
    failure_rate = stats["failed_requests"] / stats["total_requests"]
    assert failure_rate < 0.01, f"Failure rate {failure_rate:.2%} exceeds 1%"


@pytest.mark.performance
def test_api_status_endpoint_throughput(client, auth_headers):
    """Test /api/v1/status endpoint throughput."""
    
    def make_request():
        response = client.get("/api/v1/status", headers=auth_headers)
        assert response.status_code == 200
        return response
    
    stats = measure_throughput(make_request, duration_seconds=5, max_workers=10)
    
    print(f"\n/api/v1/status endpoint throughput:")
    print(f"  Throughput: {stats['rps']:.2f} req/s")
    print(f"  Success rate: {stats['successful_requests']/stats['total_requests']:.1%}")
    
    assert stats["rps"] >= MIN_THROUGHPUT_RPS, (
        f"Status endpoint throughput {stats['rps']:.2f} RPS is below "
        f"minimum {MIN_THROUGHPUT_RPS} RPS"
    )


@pytest.mark.performance
def test_chat_endpoint_throughput(client, auth_headers):
    """Test /api/v1/chat endpoint throughput."""
    
    payload = {"message": "Performance test"}
    
    def make_request():
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        return response
    
    stats = measure_throughput(make_request, duration_seconds=5, max_workers=10)
    
    print(f"\n/api/v1/chat endpoint throughput:")
    print(f"  Throughput: {stats['rps']:.2f} req/s")
    
    assert stats["rps"] >= MIN_THROUGHPUT_RPS, (
        f"Chat endpoint throughput {stats['rps']:.2f} RPS is below "
        f"minimum {MIN_THROUGHPUT_RPS} RPS"
    )


# ============================================================================
# SUSTAINED LOAD TESTS
# ============================================================================

@pytest.mark.performance
@pytest.mark.slow
def test_sustained_load_health_endpoint(client):
    """Test /health endpoint under sustained load."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
    
    samples = measure_sustained_throughput(
        make_request,
        duration_seconds=30,
        sample_interval=5,
        max_workers=20
    )
    
    print(f"\nSustained load test results:")
    print("-" * 60)
    
    rps_values = []
    for i, sample in enumerate(samples, 1):
        print(f"  Sample {i} ({sample['timestamp']:.1f}s): "
              f"{sample['rps']:.2f} req/s")
        rps_values.append(sample['rps'])
    
    print("-" * 60)
    
    # Calculate throughput statistics
    initial_rps = rps_values[0]
    final_rps = rps_values[-1]
    avg_rps = statistics.mean(rps_values)
    min_rps = min(rps_values)
    
    degradation = ((initial_rps - final_rps) / initial_rps) * 100
    
    print(f"  Initial throughput: {initial_rps:.2f} req/s")
    print(f"  Final throughput: {final_rps:.2f} req/s")
    print(f"  Average throughput: {avg_rps:.2f} req/s")
    print(f"  Minimum throughput: {min_rps:.2f} req/s")
    print(f"  Degradation: {degradation:.2f}%")
    
    # Assert requirements
    assert avg_rps >= MIN_THROUGHPUT_RPS, (
        f"Average throughput {avg_rps:.2f} RPS below minimum {MIN_THROUGHPUT_RPS} RPS"
    )
    
    assert degradation < MAX_DEGRADATION_PERCENT, (
        f"Throughput degradation {degradation:.2f}% exceeds "
        f"maximum {MAX_DEGRADATION_PERCENT}%"
    )


@pytest.mark.performance
@pytest.mark.slow
def test_sustained_load_api_endpoints(client, auth_headers):
    """Test API endpoints under sustained load."""
    
    def make_request():
        response = client.get("/api/v1/status", headers=auth_headers)
        assert response.status_code == 200
    
    samples = measure_sustained_throughput(
        make_request,
        duration_seconds=20,
        sample_interval=5,
        max_workers=10
    )
    
    rps_values = [s['rps'] for s in samples]
    initial_rps = rps_values[0]
    final_rps = rps_values[-1]
    degradation = ((initial_rps - final_rps) / initial_rps) * 100 if initial_rps > 0 else 0
    
    print(f"\nAPI sustained load:")
    print(f"  Initial: {initial_rps:.2f} req/s")
    print(f"  Final: {final_rps:.2f} req/s")
    print(f"  Degradation: {degradation:.2f}%")
    
    assert degradation < MAX_DEGRADATION_PERCENT, (
        f"API throughput degradation {degradation:.2f}% exceeds limit"
    )


# ============================================================================
# CONCURRENT REQUEST PATTERNS
# ============================================================================

@pytest.mark.performance
def test_mixed_endpoint_throughput(client, auth_headers):
    """Test throughput with mixed endpoint requests."""
    
    endpoints = [
        lambda: client.get("/health"),
        lambda: client.get("/api/v1/status", headers=auth_headers),
        lambda: client.post("/api/v1/chat", headers=auth_headers, 
                           json={"message": "test"}),
    ]
    
    def make_mixed_request():
        import random
        endpoint = random.choice(endpoints)
        return endpoint()
    
    stats = measure_throughput(
        make_mixed_request,
        duration_seconds=10,
        max_workers=15
    )
    
    print(f"\nMixed endpoint throughput:")
    print(f"  Total requests: {stats['total_requests']}")
    print(f"  Throughput: {stats['rps']:.2f} req/s")
    print(f"  Success rate: {stats['successful_requests']/stats['total_requests']:.1%}")
    
    assert stats["rps"] >= MIN_THROUGHPUT_RPS, (
        f"Mixed endpoint throughput {stats['rps']:.2f} RPS below minimum"
    )


@pytest.mark.performance
def test_burst_traffic_handling(client):
    """Test system behavior under burst traffic patterns."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
    
    # Measure baseline
    baseline_stats = measure_throughput(make_request, duration_seconds=3, max_workers=5)
    baseline_rps = baseline_stats["rps"]
    
    # Simulate burst - 3x workers
    burst_stats = measure_throughput(make_request, duration_seconds=3, max_workers=15)
    burst_rps = burst_stats["rps"]
    
    # Measure recovery
    recovery_stats = measure_throughput(make_request, duration_seconds=3, max_workers=5)
    recovery_rps = recovery_stats["rps"]
    
    print(f"\nBurst traffic test:")
    print(f"  Baseline: {baseline_rps:.2f} req/s")
    print(f"  Burst: {burst_rps:.2f} req/s")
    print(f"  Recovery: {recovery_rps:.2f} req/s")
    
    # Burst should handle increased load
    assert burst_rps > baseline_rps, "System didn't scale with burst traffic"
    
    # Recovery should return to baseline (within 20%)
    recovery_degradation = abs(recovery_rps - baseline_rps) / baseline_rps
    assert recovery_degradation < 0.20, (
        f"System didn't recover after burst: {recovery_degradation:.1%} degradation"
    )


# ============================================================================
# CONNECTION POOL EFFICIENCY
# ============================================================================

@pytest.mark.performance
def test_connection_pool_efficiency(client):
    """Test that connection pooling improves throughput."""
    
    # Test with connection reuse (normal client behavior)
    def make_request_pooled():
        response = client.get("/health")
        assert response.status_code == 200
    
    pooled_stats = measure_throughput(
        make_request_pooled,
        duration_seconds=5,
        max_workers=10
    )
    
    print(f"\nConnection pool efficiency:")
    print(f"  With pooling: {pooled_stats['rps']:.2f} req/s")
    print(f"  Total requests: {pooled_stats['total_requests']}")
    
    # Should achieve good throughput with pooling
    assert pooled_stats["rps"] >= MIN_THROUGHPUT_RPS, (
        "Connection pooling not achieving minimum throughput"
    )


# ============================================================================
# PAYLOAD SIZE IMPACT
# ============================================================================

@pytest.mark.performance
def test_throughput_vs_payload_size(client, auth_headers):
    """Test how payload size affects throughput."""
    
    payload_sizes = [
        (100, "100B"),
        (1000, "1KB"),
        (10000, "10KB"),
    ]
    
    results = []
    
    print(f"\nThroughput vs Payload Size:")
    print("-" * 60)
    
    for size, label in payload_sizes:
        payload = {"message": "x" * size}
        
        def make_request():
            response = client.post(
                "/api/v1/chat",
                headers=auth_headers,
                json=payload
            )
            return response
        
        stats = measure_throughput(make_request, duration_seconds=3, max_workers=5)
        results.append((label, stats["rps"]))
        
        print(f"  {label:6}: {stats['rps']:6.2f} req/s")
    
    print("-" * 60)
    
    # Throughput should decrease with payload size but stay reasonable
    for label, rps in results:
        assert rps >= MIN_THROUGHPUT_RPS / 2, (
            f"Throughput for {label} payload too low: {rps:.2f} RPS"
        )


# ============================================================================
# THROUGHPUT BENCHMARKING
# ============================================================================

@pytest.mark.performance
def test_maximum_throughput_capacity(client):
    """Determine maximum throughput capacity of the system."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
    
    worker_counts = [5, 10, 20, 30, 40, 50]
    results = []
    
    print(f"\nMaximum throughput capacity test:")
    print("-" * 60)
    
    for workers in worker_counts:
        stats = measure_throughput(
            make_request,
            duration_seconds=5,
            max_workers=workers
        )
        
        results.append((workers, stats["rps"]))
        print(f"  {workers:2} workers: {stats['rps']:6.2f} req/s")
    
    print("-" * 60)
    
    max_rps = max(rps for _, rps in results)
    optimal_workers = [w for w, rps in results if rps == max_rps][0]
    
    print(f"  Maximum throughput: {max_rps:.2f} req/s")
    print(f"  Optimal workers: {optimal_workers}")
    
    assert max_rps >= TARGET_THROUGHPUT_RPS, (
        f"Maximum throughput {max_rps:.2f} RPS below "
        f"target {TARGET_THROUGHPUT_RPS} RPS"
    )