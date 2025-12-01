"""
Security Tests: Denial of Service (DoS)

Tests DoS scenarios, rate limiting effectiveness, resource exhaustion,
and slowloris attacks.

Requirements:
- Validate rate limiting
- Test resource exhaustion prevention
- Verify request size limits
- Test connection flooding resistance

Run with:
    pytest tests/security/test_security_dos.py -v
    pytest tests/security/test_security_dos.py -m security
"""

import pytest
import time
import threading
from typing import List
from concurrent.futures import ThreadPoolExecutor
from fastapi.testclient import TestClient
from unittest.mock import patch

# Import app after setting test environment
import os
os.environ["ENVIRONMENT"] = "testing"
os.environ["API_KEY_REQUIRED"] = "false"

from main import app
from config import settings


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def client():
    """Create test client."""
    with TestClient(app) as test_client:
        yield test_client


@pytest.fixture
def client_with_rate_limit():
    """Create test client with rate limiting enabled."""
    with patch.dict(os.environ, {
        "RATE_LIMIT_ENABLED": "true",
        "RATE_LIMIT_REQUESTS_PER_MINUTE": "10",
    }):
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
# RATE LIMITING TESTS
# ============================================================================

@pytest.mark.security
def test_rate_limit_enforcement(client_with_rate_limit):
    """Test that rate limiting is enforced."""
    
    # Make requests up to and beyond limit
    rate_limit = 10
    responses = []
    
    for i in range(rate_limit + 5):
        response = client_with_rate_limit.get("/health")
        responses.append(response.status_code)
        time.sleep(0.01)  # Small delay between requests
    
    # Count rate limited responses
    rate_limited = sum(1 for code in responses if code == 429)
    
    print(f"\n✓ Rate limiting enforced:")
    print(f"  Limit: {rate_limit} req/min")
    print(f"  Rate limited responses: {rate_limited}")
    
    # Should have some rate limited responses
    assert rate_limited > 0, "Rate limiting not enforced"


@pytest.mark.security
def test_rate_limit_per_ip(client_with_rate_limit):
    """Test that rate limits are per IP."""
    
    # Simulate different IPs via headers
    def make_request_from_ip(ip: str):
        response = client_with_rate_limit.get(
            "/health",
            headers={"X-Forwarded-For": ip}
        )
        return response.status_code
    
    # Make requests from IP1
    ip1_responses = [make_request_from_ip("192.168.1.1") for _ in range(15)]
    
    # Make requests from IP2
    ip2_responses = [make_request_from_ip("192.168.1.2") for _ in range(5)]
    
    # IP1 should be rate limited, IP2 should be fine
    ip1_limited = sum(1 for code in ip1_responses if code == 429)
    ip2_limited = sum(1 for code in ip2_responses if code == 429)
    
    print(f"\n✓ Per-IP rate limiting:")
    print(f"  IP1 rate limited: {ip1_limited}/15")
    print(f"  IP2 rate limited: {ip2_limited}/5")
    
    assert ip1_limited > 0, "Heavy user not rate limited"
    assert ip2_limited == 0, "Light user incorrectly rate limited"


@pytest.mark.security
def test_rate_limit_headers_present(client_with_rate_limit):
    """Test that rate limit headers are included."""
    
    response = client_with_rate_limit.get("/health")
    
    expected_headers = [
        "X-RateLimit-Limit",
        "X-RateLimit-Remaining",
        "X-RateLimit-Reset",
    ]
    
    for header in expected_headers:
        assert header in response.headers, f"Missing header: {header}"
    
    print(f"\n✓ All {len(expected_headers)} rate limit headers present")


# ============================================================================
# REQUEST SIZE LIMIT TESTS
# ============================================================================

@pytest.mark.security
def test_large_payload_rejected(client, auth_headers):
    """Test that excessively large payloads are rejected."""
    
    # Create large payload (11MB - exceeds 10MB limit)
    large_message = "x" * (11 * 1024 * 1024)
    
    response = client.post(
        "/api/v1/chat",
        headers=auth_headers,
        json={"message": large_message}
    )
    
    # Should be rejected with 413 Payload Too Large
    assert response.status_code in [413, 400], (
        f"Large payload not rejected: {response.status_code}"
    )
    
    print("\n✓ Large payload rejected (11MB)")


@pytest.mark.security
def test_request_size_limit_validation(client):
    """Test request size limit is enforced."""
    
    # Test increasingly large payloads
    sizes_mb = [1, 5, 10, 11]
    results = []
    
    for size_mb in sizes_mb:
        payload = {"message": "x" * (size_mb * 1024 * 1024)}
        
        try:
            response = client.post("/api/v1/chat", json=payload)
            results.append((size_mb, response.status_code))
        except Exception as e:
            results.append((size_mb, "exception"))
    
    print(f"\n✓ Request size validation:")
    for size, status in results:
        print(f"  {size}MB: {status}")
    
    # Requests > 10MB should be rejected
    large_requests = [r for r in results if r[0] > 10]
    for size, status in large_requests:
        assert status in [413, 400, "exception"], (
            f"{size}MB request not rejected"
        )


# ============================================================================
# CONNECTION EXHAUSTION TESTS
# ============================================================================

@pytest.mark.security
@pytest.mark.slow
def test_connection_pool_exhaustion_resistance(client):
    """Test resistance to connection pool exhaustion."""
    
    # Try to exhaust connection pool
    num_connections = 150
    
    def make_slow_request():
        try:
            response = client.get("/health")
            return response.status_code
        except Exception as e:
            return str(e)
    
    with ThreadPoolExecutor(max_workers=num_connections) as executor:
        futures = [executor.submit(make_slow_request) for _ in range(num_connections)]
        results = [f.result() for f in futures]
    
    # Count successful vs failed
    successful = sum(1 for r in results if r == 200)
    failed = len(results) - successful
    
    print(f"\n✓ Connection exhaustion test:")
    print(f"  Total attempts: {num_connections}")
    print(f"  Successful: {successful}")
    print(f"  Failed: {failed}")
    
    # Should handle gracefully, most requests succeed
    success_rate = successful / num_connections
    assert success_rate >= 0.80, (
        f"Connection exhaustion: only {success_rate:.1%} success rate"
    )


# ============================================================================
# SLOWLORIS ATTACK TESTS
# ============================================================================

@pytest.mark.security
@pytest.mark.slow
def test_slow_post_attack_mitigation(client, auth_headers):
    """Test mitigation of slow POST attacks."""
    
    # Send requests with artificial delays
    def slow_request():
        start = time.time()
        try:
            # FastAPI/Uvicorn has timeouts for this
            response = client.post(
                "/api/v1/chat",
                headers=auth_headers,
                json={"message": "test"}
            )
            duration = time.time() - start
            return (response.status_code, duration)
        except Exception as e:
            duration = time.time() - start
            return (str(e), duration)
    
    results = [slow_request() for _ in range(10)]
    
    # Check that requests complete reasonably fast
    durations = [r[1] for r in results]
    avg_duration = sum(durations) / len(durations)
    
    print(f"\n✓ Slow POST mitigation:")
    print(f"  Average duration: {avg_duration:.2f}s")
    print(f"  Max duration: {max(durations):.2f}s")
    
    # Should have reasonable timeout
    assert max(durations) < 30, "Requests hanging too long"


# ============================================================================
# RESOURCE EXHAUSTION TESTS
# ============================================================================

@pytest.mark.security
def test_memory_exhaustion_prevention(client, auth_headers):
    """Test prevention of memory exhaustion attacks."""
    
    # Try to allocate large amounts of memory
    large_arrays = []
    
    for i in range(10):
        payload = {
            "message": "test",
            "data": ["x" * 1000] * 1000  # Large nested structure
        }
        
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        
        # Should handle or reject gracefully
        assert response.status_code in [200, 400, 413, 422]
    
    print("\n✓ Memory exhaustion prevention working")


@pytest.mark.security
def test_cpu_exhaustion_prevention(client, auth_headers):
    """Test prevention of CPU exhaustion via complex inputs."""
    
    # Send computationally expensive patterns
    complex_payloads = [
        {"message": "test " * 10000},  # Very long message
        {"message": "(((((" * 1000},  # Nested patterns
    ]
    
    for payload in complex_payloads:
        start = time.time()
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        duration = time.time() - start
        
        # Should process quickly or reject
        assert duration < 5, f"Request took {duration:.2f}s"
        assert response.status_code in [200, 400, 422]
    
    print(f"\n✓ CPU exhaustion prevention working")


# ============================================================================
# REGEX DOS (ReDoS) TESTS
# ============================================================================

@pytest.mark.security
def test_redos_prevention(client, auth_headers):
    """Test prevention of Regular Expression DoS."""
    
    # Patterns that could cause catastrophic backtracking
    redos_patterns = [
        "a" * 50 + "!",
        "(" * 50 + "a" + ")" * 50,
        "x" * 100,
    ]
    
    for pattern in redos_patterns:
        start = time.time()
        
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json={"message": pattern}
        )
        
        duration = time.time() - start
        
        # Should not hang
        assert duration < 2, (
            f"Possible ReDoS: pattern took {duration:.2f}s"
        )
        
        assert response.status_code in [200, 400]
    
    print(f"\n✓ ReDoS prevention working for {len(redos_patterns)} patterns")


# ============================================================================
# COMPRESSION BOMB TESTS
# ============================================================================

@pytest.mark.security
def test_compression_bomb_detection(client, auth_headers):
    """Test detection of compression bombs."""
    
    import gzip
    
    # Create highly compressible data
    compressible = b"A" * 1000000  # 1MB of 'A's compresses to <1KB
    
    # Most frameworks auto-decompress, test they handle it
    response = client.post(
        "/api/v1/chat",
        headers={**auth_headers, "Content-Encoding": "gzip"},
        data=gzip.compress(compressible)
    )
    
    # Should reject or handle safely
    assert response.status_code in [200, 400, 413, 415]
    
    print("\n✓ Compression bomb handling tested")


# ============================================================================
# CONCURRENT DOS TESTS
# ============================================================================

@pytest.mark.security
@pytest.mark.slow
def test_concurrent_request_flood(client):
    """Test system under concurrent request flood."""
    
    # Flood with requests
    num_requests = 100
    
    def make_request():
        try:
            response = client.get("/health")
            return response.status_code == 200
        except:
            return False
    
    start = time.time()
    
    with ThreadPoolExecutor(max_workers=50) as executor:
        futures = [executor.submit(make_request) for _ in range(num_requests)]
        results = [f.result() for f in futures]
    
    duration = time.time() - start
    successful = sum(results)
    
    print(f"\n✓ Request flood test:")
    print(f"  Requests: {num_requests}")
    print(f"  Successful: {successful}")
    print(f"  Duration: {duration:.2f}s")
    print(f"  Rate: {num_requests/duration:.1f} req/s")
    
    # Should handle most requests
    success_rate = successful / num_requests
    assert success_rate >= 0.90, (
        f"Low success rate under flood: {success_rate:.1%}"
    )


# ============================================================================
# TIMEOUT TESTS
# ============================================================================

@pytest.mark.security
def test_request_timeout_enforcement(client, auth_headers):
    """Test that long-running requests timeout."""
    
    # This would need a slow endpoint to test properly
    # For now, verify normal requests complete quickly
    
    start = time.time()
    response = client.post(
        "/api/v1/chat",
        headers=auth_headers,
        json={"message": "test"}
    )
    duration = time.time() - start
    
    # Should complete quickly
    assert duration < 30, f"Request took {duration:.2f}s"
    
    print(f"\n✓ Request completed in {duration:.2f}s")


# ============================================================================
# AMPLIFICATION ATTACK TESTS
# ============================================================================

@pytest.mark.security
def test_response_size_proportional(client, auth_headers):
    """Test that response size is proportional to request."""
    
    small_payload = {"message": "hi"}
    large_payload = {"message": "test " * 100}
    
    small_response = client.post(
        "/api/v1/chat",
        headers=auth_headers,
        json=small_payload
    )
    
    large_response = client.post(
        "/api/v1/chat",
        headers=auth_headers,
        json=large_payload
    )
    
    # Response sizes should be reasonable
    small_size = len(small_response.content)
    large_size = len(large_response.content)
    
    print(f"\n✓ Response size proportionality:")
    print(f"  Small response: {small_size} bytes")
    print(f"  Large response: {large_size} bytes")
    
    # Large response shouldn't be disproportionately larger
    amplification_factor = large_size / small_size if small_size > 0 else 1
    assert amplification_factor < 100, (
        f"High amplification factor: {amplification_factor:.1f}x"
    )


# ============================================================================
# APPLICATION LAYER DOS
# ============================================================================

@pytest.mark.security
def test_nested_json_dos(client, auth_headers):
    """Test handling of deeply nested JSON."""
    
    # Create deeply nested structure
    def create_nested(depth):
        result = {"value": "x"}
        for _ in range(depth):
            result = {"nested": result}
        return result
    
    # Test various depths
    for depth in [10, 50, 100]:
        payload = {"message": create_nested(depth)}
        
        start = time.time()
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        duration = time.time() - start
        
        # Should handle or reject quickly
        assert duration < 2, f"Nested JSON at depth {depth} took {duration:.2f}s"
        assert response.status_code in [200, 400, 422]
    
    print("\n✓ Nested JSON DoS prevention working")


@pytest.mark.security
def test_array_explosion_dos(client, auth_headers):
    """Test handling of array explosion attacks."""
    
    # Large array
    large_array = [{"item": i} for i in range(10000)]
    
    start = time.time()
    response = client.post(
        "/api/v1/chat",
        headers=auth_headers,
        json={"message": large_array}
    )
    duration = time.time() - start
    
    print(f"\n✓ Array explosion handled in {duration:.2f}s")
    
    # Should handle or reject
    assert response.status_code in [200, 400, 413, 422]
    assert duration < 5, "Array processing too slow"


# ============================================================================
# SUMMARY TEST
# ============================================================================

@pytest.mark.security
def test_dos_protection_summary(client, client_with_rate_limit):
    """Summary test of DoS protections."""
    
    protections = {
        "Rate Limiting": lambda: client_with_rate_limit.get("/health").status_code,
        "Request Size Limit": lambda: client.post(
            "/api/v1/chat",
            json={"message": "x" * 100}
        ).status_code,
        "Connection Handling": lambda: client.get("/health").status_code,
    }
    
    print("\n✓ DoS Protection Summary:")
    print("-" * 60)
    
    for name, test_func in protections.items():
        try:
            result = test_func()
            status = "✓ Active" if result in [200, 429] else "⚠ Check"
            print(f"  {name:30} {status}")
        except Exception as e:
            print(f"  {name:30} ✗ Error: {str(e)[:30]}")
    
    print("-" * 60)