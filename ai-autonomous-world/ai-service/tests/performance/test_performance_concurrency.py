"""
Performance Tests: Concurrency Testing

Tests 100+ simultaneous requests, connection pool saturation, and resource contention.
Validates system behavior under concurrent load.

Requirements:
- Handle 100+ concurrent connections
- Detect connection pool saturation
- Identify resource contention issues
- Validate thread safety

Run with:
    pytest tests/performance/test_performance_concurrency.py -v
    pytest tests/performance/test_performance_concurrency.py -m performance
"""

import pytest
import time
import asyncio
import threading
from typing import List, Dict, Set
from concurrent.futures import ThreadPoolExecutor, as_completed, wait, ALL_COMPLETED
from fastapi.testclient import TestClient
from unittest.mock import patch
import statistics

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

MIN_CONCURRENT_REQUESTS = 100  # Minimum concurrent request requirement
TARGET_CONCURRENT_REQUESTS = 200  # Target concurrent capacity
CONNECTION_POOL_SIZE = 100  # Expected connection pool size
MAX_RESPONSE_TIME_CONCURRENT = 5000  # 5s max under high concurrency


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
# CONCURRENCY MEASUREMENT HELPERS
# ============================================================================

def execute_concurrent_requests(
    func,
    num_requests: int,
    max_workers: int = None
) -> Dict[str, any]:
    """
    Execute concurrent requests and measure behavior.
    
    Returns:
        dict: Concurrency statistics including timing and success rate
    """
    if max_workers is None:
        max_workers = min(num_requests, 200)
    
    results = {
        "total_requests": num_requests,
        "completed": 0,
        "failed": 0,
        "response_times": [],
        "errors": [],
        "start_time": 0,
        "end_time": 0,
        "duration": 0,
        "concurrent_peak": 0
    }
    
    active_count = 0
    max_active = 0
    lock = threading.Lock()
    
    def wrapped_func():
        nonlocal active_count, max_active
        
        with lock:
            active_count += 1
            max_active = max(max_active, active_count)
        
        try:
            start = time.perf_counter()
            result = func()
            end = time.perf_counter()
            
            with lock:
                active_count -= 1
            
            return ("success", (end - start) * 1000, None)
        except Exception as e:
            with lock:
                active_count -= 1
            return ("error", 0, str(e))
    
    results["start_time"] = time.time()
    
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = [executor.submit(wrapped_func) for _ in range(num_requests)]
        
        for future in as_completed(futures):
            status, response_time, error = future.result()
            
            if status == "success":
                results["completed"] += 1
                results["response_times"].append(response_time)
            else:
                results["failed"] += 1
                results["errors"].append(error)
    
    results["end_time"] = time.time()
    results["duration"] = results["end_time"] - results["start_time"]
    results["concurrent_peak"] = max_active
    
    return results


# ============================================================================
# BASIC CONCURRENCY TESTS
# ============================================================================

@pytest.mark.performance
def test_100_concurrent_requests_health(client):
    """Test 100 concurrent requests to /health endpoint."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
        return response
    
    results = execute_concurrent_requests(make_request, num_requests=100)
    
    print(f"\n100 Concurrent Requests Test:")
    print(f"  Total: {results['total_requests']}")
    print(f"  Completed: {results['completed']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Duration: {results['duration']:.2f}s")
    print(f"  Peak concurrent: {results['concurrent_peak']}")
    
    if results['response_times']:
        print(f"  Avg response time: {statistics.mean(results['response_times']):.2f}ms")
        print(f"  Max response time: {max(results['response_times']):.2f}ms")
    
    # All requests should complete successfully
    assert results["completed"] == 100, (
        f"Only {results['completed']}/100 requests completed"
    )
    
    # Failure rate should be minimal
    failure_rate = results["failed"] / results["total_requests"]
    assert failure_rate < 0.01, f"Failure rate {failure_rate:.1%} exceeds 1%"


@pytest.mark.performance
def test_200_concurrent_requests_api(client, auth_headers):
    """Test 200 concurrent requests to API endpoint."""
    
    def make_request():
        response = client.get("/api/v1/status", headers=auth_headers)
        assert response.status_code == 200
        return response
    
    results = execute_concurrent_requests(make_request, num_requests=200)
    
    print(f"\n200 Concurrent Requests Test:")
    print(f"  Completed: {results['completed']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Duration: {results['duration']:.2f}s")
    print(f"  Success rate: {results['completed']/results['total_requests']:.1%}")
    
    # Should handle 200 concurrent requests
    success_rate = results["completed"] / results["total_requests"]
    assert success_rate >= 0.95, (
        f"Success rate {success_rate:.1%} below 95%"
    )


@pytest.mark.performance
@pytest.mark.slow
def test_500_concurrent_requests_stress(client):
    """Stress test with 500 concurrent requests."""
    
    def make_request():
        response = client.get("/health")
        return response
    
    results = execute_concurrent_requests(make_request, num_requests=500)
    
    print(f"\n500 Concurrent Requests Stress Test:")
    print(f"  Completed: {results['completed']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Duration: {results['duration']:.2f}s")
    print(f"  Peak concurrent: {results['concurrent_peak']}")
    
    if results['response_times']:
        response_times = sorted(results['response_times'])
        p95_time = response_times[int(len(response_times) * 0.95)]
        print(f"  p95 response time: {p95_time:.2f}ms")
        
        # Response times should stay reasonable even under stress
        assert p95_time < MAX_RESPONSE_TIME_CONCURRENT, (
            f"p95 response time {p95_time:.2f}ms exceeds "
            f"limit {MAX_RESPONSE_TIME_CONCURRENT}ms"
        )
    
    # Should complete most requests
    success_rate = results["completed"] / results["total_requests"]
    assert success_rate >= 0.90, (
        f"Success rate {success_rate:.1%} below 90% under stress"
    )


# ============================================================================
# CONNECTION POOL TESTS
# ============================================================================

@pytest.mark.performance
def test_connection_pool_saturation(client):
    """Test behavior when connection pool saturates."""
    
    # Send more requests than typical pool size
    num_requests = CONNECTION_POOL_SIZE * 2
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
        return response
    
    results = execute_concurrent_requests(
        make_request,
        num_requests=num_requests,
        max_workers=num_requests
    )
    
    print(f"\nConnection Pool Saturation Test:")
    print(f"  Requests: {num_requests} (2x pool size)")
    print(f"  Completed: {results['completed']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Peak concurrent: {results['concurrent_peak']}")
    
    # Should handle requests beyond pool size gracefully
    success_rate = results["completed"] / results["total_requests"]
    assert success_rate >= 0.95, (
        f"System failed under pool saturation: {success_rate:.1%} success rate"
    )


@pytest.mark.performance
def test_connection_reuse_efficiency(client):
    """Test that connections are efficiently reused."""
    
    request_counts = [10, 50, 100]
    results_by_count = []
    
    print(f"\nConnection Reuse Efficiency Test:")
    print("-" * 60)
    
    for count in request_counts:
        def make_request():
            response = client.get("/health")
            assert response.status_code == 200
        
        results = execute_concurrent_requests(make_request, count, max_workers=10)
        
        avg_time = statistics.mean(results['response_times']) if results['response_times'] else 0
        results_by_count.append((count, avg_time))
        
        print(f"  {count:3} requests: {avg_time:.2f}ms avg response time")
    
    print("-" * 60)
    
    # Response time shouldn't increase significantly with more requests
    # (indicates good connection reuse)
    if len(results_by_count) >= 2:
        first_avg = results_by_count[0][1]
        last_avg = results_by_count[-1][1]
        increase = ((last_avg - first_avg) / first_avg) * 100 if first_avg > 0 else 0
        
        print(f"  Response time increase: {increase:.1f}%")
        
        assert increase < 50, (
            f"Response time increased {increase:.1f}% - poor connection reuse"
        )


# ============================================================================
# RESOURCE CONTENTION TESTS
# ============================================================================

@pytest.mark.performance
def test_mixed_payload_concurrency(client, auth_headers):
    """Test concurrency with mixed payload sizes."""
    
    payloads = [
        {"message": "small"},
        {"message": "m" * 100},
        {"message": "l" * 1000},
    ]
    
    def make_request():
        import random
        payload = random.choice(payloads)
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        return response
    
    results = execute_concurrent_requests(make_request, num_requests=100)
    
    print(f"\nMixed Payload Concurrency Test:")
    print(f"  Completed: {results['completed']}")
    print(f"  Failed: {results['failed']}")
    
    success_rate = results["completed"] / results["total_requests"]
    assert success_rate >= 0.95, (
        f"Mixed payload concurrency failed: {success_rate:.1%} success rate"
    )


@pytest.mark.performance
def test_concurrent_different_endpoints(client, auth_headers):
    """Test concurrent requests to different endpoints."""
    
    endpoints = [
        lambda: client.get("/health"),
        lambda: client.get("/readiness"),
        lambda: client.get("/api/v1/status", headers=auth_headers),
    ]
    
    def make_request():
        import random
        endpoint = random.choice(endpoints)
        return endpoint()
    
    results = execute_concurrent_requests(make_request, num_requests=150)
    
    print(f"\nMulti-Endpoint Concurrency Test:")
    print(f"  Requests: {results['total_requests']}")
    print(f"  Completed: {results['completed']}")
    print(f"  Duration: {results['duration']:.2f}s")
    
    success_rate = results["completed"] / results["total_requests"]
    assert success_rate >= 0.95, (
        f"Multi-endpoint concurrency failed: {success_rate:.1%} success rate"
    )


# ============================================================================
# THREAD SAFETY TESTS
# ============================================================================

@pytest.mark.performance
def test_concurrent_state_consistency(client, auth_headers):
    """Test that concurrent requests maintain state consistency."""
    
    request_ids = set()
    lock = threading.Lock()
    
    def make_request():
        response = client.get("/api/v1/status", headers=auth_headers)
        if response.status_code == 200:
            # Track unique responses
            with lock:
                request_ids.add(id(response))
        return response
    
    results = execute_concurrent_requests(make_request, num_requests=100)
    
    print(f"\nState Consistency Test:")
    print(f"  Concurrent requests: 100")
    print(f"  Unique responses: {len(request_ids)}")
    print(f"  Completed: {results['completed']}")
    
    # All requests should complete successfully
    assert results["completed"] == 100, "Some requests failed"
    
    # Should have unique response objects (no cross-contamination)
    assert len(request_ids) == 100, "Response objects not unique - possible race condition"


@pytest.mark.performance
def test_no_race_conditions(client):
    """Test for race conditions in concurrent access."""
    
    counters = {"success": 0, "failure": 0}
    lock = threading.Lock()
    errors = []
    
    def make_request():
        try:
            response = client.get("/health")
            
            with lock:
                if response.status_code == 200:
                    counters["success"] += 1
                else:
                    counters["failure"] += 1
            
            return response
        except Exception as e:
            with lock:
                counters["failure"] += 1
                errors.append(str(e))
            raise
    
    results = execute_concurrent_requests(make_request, num_requests=200)
    
    print(f"\nRace Condition Test:")
    print(f"  Success counter: {counters['success']}")
    print(f"  Failure counter: {counters['failure']}")
    print(f"  Total: {counters['success'] + counters['failure']}")
    
    # Counters should match total requests (no race conditions)
    assert counters["success"] + counters["failure"] == 200, (
        "Counter mismatch indicates race condition"
    )
    
    assert len(errors) == 0, f"Errors occurred: {errors}"


# ============================================================================
# SCALABILITY TESTS
# ============================================================================

@pytest.mark.performance
@pytest.mark.slow
def test_concurrency_scalability(client):
    """Test how system scales with increasing concurrency."""
    
    concurrency_levels = [10, 50, 100, 200, 300]
    results_by_level = []
    
    print(f"\nConcurrency Scalability Test:")
    print("-" * 60)
    
    for level in concurrency_levels:
        def make_request():
            response = client.get("/health")
            assert response.status_code == 200
        
        results = execute_concurrent_requests(
            make_request,
            num_requests=level,
            max_workers=level
        )
        
        if results['response_times']:
            avg_time = statistics.mean(results['response_times'])
            success_rate = results['completed'] / results['total_requests']
            
            results_by_level.append({
                "level": level,
                "avg_time": avg_time,
                "success_rate": success_rate,
                "duration": results['duration']
            })
            
            print(f"  {level:3} concurrent: {avg_time:6.2f}ms avg, "
                  f"{success_rate:.1%} success")
    
    print("-" * 60)
    
    # System should maintain >95% success rate at all levels
    for result in results_by_level:
        assert result['success_rate'] >= 0.95, (
            f"Success rate dropped to {result['success_rate']:.1%} "
            f"at {result['level']} concurrent requests"
        )


@pytest.mark.performance
def test_concurrent_burst_recovery(client):
    """Test system recovery after concurrent burst."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
    
    # Baseline
    baseline = execute_concurrent_requests(make_request, 10)
    baseline_avg = statistics.mean(baseline['response_times'])
    
    # Burst
    burst = execute_concurrent_requests(make_request, 200)
    burst_avg = statistics.mean(burst['response_times'])
    
    # Recovery
    time.sleep(1)  # Allow brief recovery
    recovery = execute_concurrent_requests(make_request, 10)
    recovery_avg = statistics.mean(recovery['response_times'])
    
    print(f"\nConcurrent Burst Recovery Test:")
    print(f"  Baseline avg: {baseline_avg:.2f}ms")
    print(f"  Burst avg: {burst_avg:.2f}ms")
    print(f"  Recovery avg: {recovery_avg:.2f}ms")
    
    # Recovery should return close to baseline
    degradation = abs(recovery_avg - baseline_avg) / baseline_avg
    assert degradation < 0.30, (
        f"System didn't recover after burst: {degradation:.1%} degradation"
    )


# ============================================================================
# MEMORY AND RESOURCE TESTS
# ============================================================================

@pytest.mark.performance
def test_memory_leak_detection(client):
    """Test for memory leaks under concurrent load."""
    
    import psutil
    import os
    
    process = psutil.Process(os.getpid())
    
    # Get baseline memory
    initial_memory = process.memory_info().rss / 1024 / 1024  # MB
    
    def make_request():
        response = client.get("/health")
        return response
    
    # Run multiple batches of concurrent requests
    for batch in range(5):
        execute_concurrent_requests(make_request, num_requests=100)
    
    # Get final memory
    final_memory = process.memory_info().rss / 1024 / 1024  # MB
    memory_increase = final_memory - initial_memory
    
    print(f"\nMemory Leak Detection Test:")
    print(f"  Initial memory: {initial_memory:.2f} MB")
    print(f"  Final memory: {final_memory:.2f} MB")
    print(f"  Increase: {memory_increase:.2f} MB")
    
    # Memory increase should be reasonable (<100MB for 500 requests)
    assert memory_increase < 100, (
        f"Possible memory leak: {memory_increase:.2f}MB increase"
    )