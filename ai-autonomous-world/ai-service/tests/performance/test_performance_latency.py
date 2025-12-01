"""
Performance Tests: Latency Testing

Tests the <500ms p95 latency requirement for all API endpoints.
Measures response times and identifies performance bottlenecks.

Requirements:
- pytest-benchmark for accurate timing
- FastAPI TestClient for endpoint testing
- All endpoints must meet <500ms p95 latency

Run with:
    pytest tests/performance/test_performance_latency.py -v --benchmark-only
    pytest tests/performance/test_performance_latency.py -m performance
"""

import pytest
import time
import statistics
from typing import List, Dict, Any
from fastapi.testclient import TestClient
from unittest.mock import Mock, patch, AsyncMock

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

LATENCY_REQUIREMENT_MS = 500  # p95 latency requirement
LATENCY_TARGET_MS = 300  # Target for optimal performance
ITERATIONS = 100  # Number of iterations for p95 calculation


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def client():
    """Create test client with performance configuration."""
    with TestClient(app) as test_client:
        yield test_client


@pytest.fixture
def auth_headers():
    """Create authentication headers for testing."""
    return {
        "Authorization": f"Bearer {settings.api_key or 'test-key'}",
        "Content-Type": "application/json"
    }


@pytest.fixture
def mock_db():
    """Mock database operations for consistent testing."""
    with patch("database.db_manager") as mock:
        mock.postgres._connection_established = True
        mock.dragonfly._connection_established = True
        yield mock


# ============================================================================
# LATENCY MEASUREMENT HELPERS
# ============================================================================

def measure_latency(func, iterations: int = ITERATIONS) -> Dict[str, float]:
    """
    Measure latency statistics for a function.
    
    Returns:
        dict: Statistics including min, max, mean, median, p95, p99
    """
    latencies = []
    
    for _ in range(iterations):
        start = time.perf_counter()
        func()
        end = time.perf_counter()
        latencies.append((end - start) * 1000)  # Convert to ms
    
    latencies.sort()
    
    return {
        "min": latencies[0],
        "max": latencies[-1],
        "mean": statistics.mean(latencies),
        "median": statistics.median(latencies),
        "p95": latencies[int(len(latencies) * 0.95)],
        "p99": latencies[int(len(latencies) * 0.99)],
        "iterations": iterations
    }


def assert_latency_requirement(stats: Dict[str, float], endpoint: str):
    """Assert that latency meets requirements."""
    assert stats["p95"] < LATENCY_REQUIREMENT_MS, (
        f"{endpoint} p95 latency {stats['p95']:.2f}ms exceeds "
        f"requirement of {LATENCY_REQUIREMENT_MS}ms"
    )
    
    if stats["p95"] > LATENCY_TARGET_MS:
        pytest.warn(
            f"{endpoint} p95 latency {stats['p95']:.2f}ms exceeds "
            f"target of {LATENCY_TARGET_MS}ms"
        )


# ============================================================================
# HEALTH CHECK ENDPOINTS - BASELINE PERFORMANCE
# ============================================================================

@pytest.mark.performance
def test_health_endpoint_latency(client, benchmark):
    """Test /health endpoint latency (baseline)."""
    
    def make_request():
        response = client.get("/health")
        assert response.status_code == 200
        return response
    
    # Benchmark with pytest-benchmark
    result = benchmark(make_request)
    
    # Manual p95 measurement
    stats = measure_latency(lambda: client.get("/health"))
    
    print(f"\n/health endpoint latency stats:")
    print(f"  Min: {stats['min']:.2f}ms")
    print(f"  Mean: {stats['mean']:.2f}ms")
    print(f"  Median: {stats['median']:.2f}ms")
    print(f"  p95: {stats['p95']:.2f}ms")
    print(f"  p99: {stats['p99']:.2f}ms")
    print(f"  Max: {stats['max']:.2f}ms")
    
    # Assert requirements
    assert_latency_requirement(stats, "/health")
    
    # Health check should be very fast (<50ms)
    assert stats["p95"] < 50, "Health check should be <50ms"


@pytest.mark.performance
def test_readiness_endpoint_latency(client, mock_db, benchmark):
    """Test /readiness endpoint latency."""
    
    def make_request():
        response = client.get("/readiness")
        assert response.status_code == 200
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(lambda: client.get("/readiness"))
    
    print(f"\n/readiness endpoint latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/readiness")


# ============================================================================
# API ENDPOINTS - CORE FUNCTIONALITY
# ============================================================================

@pytest.mark.performance
def test_status_endpoint_latency(client, auth_headers, benchmark):
    """Test /api/v1/status endpoint latency."""
    
    def make_request():
        response = client.get("/api/v1/status", headers=auth_headers)
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.get("/api/v1/status", headers=auth_headers)
    )
    
    print(f"\n/api/v1/status endpoint latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/v1/status")


@pytest.mark.performance
def test_chat_endpoint_latency_small_payload(client, auth_headers, benchmark):
    """Test /api/v1/chat endpoint with small payload."""
    
    payload = {"message": "Hello, world!"}
    
    def make_request():
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.post("/api/v1/chat", headers=auth_headers, json=payload)
    )
    
    print(f"\n/api/v1/chat (small payload) latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/v1/chat")


@pytest.mark.performance
def test_chat_endpoint_latency_large_payload(client, auth_headers, benchmark):
    """Test /api/v1/chat endpoint with larger payload."""
    
    payload = {"message": "x" * 1000}  # 1KB message
    
    def make_request():
        response = client.post(
            "/api/v1/chat",
            headers=auth_headers,
            json=payload
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.post("/api/v1/chat", headers=auth_headers, json=payload)
    )
    
    print(f"\n/api/v1/chat (large payload) latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/v1/chat (large)")


# ============================================================================
# NPC ENDPOINTS - GAME LOGIC
# ============================================================================

@pytest.mark.performance
@patch("routers.npc.Orchestrator")
def test_npc_register_latency(mock_orch, client, auth_headers, benchmark):
    """Test NPC registration endpoint latency."""
    
    payload = {
        "name": "Test NPC",
        "race": "Human",
        "gender": "Male",
        "age": 25,
        "height_cm": 175.0,
        "weight_kg": 70.0,
        "occupation": "Merchant",
        "location": "prontera",
        "position_x": 150.0,
        "position_y": 200.0
    }
    
    def make_request():
        response = client.post(
            "/api/npc/register",
            headers=auth_headers,
            json=payload
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.post("/api/npc/register", headers=auth_headers, json=payload)
    )
    
    print(f"\n/api/npc/register latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/npc/register")


@pytest.mark.performance
@patch("routers.npc.Orchestrator")
def test_npc_get_state_latency(mock_orch, client, auth_headers, benchmark):
    """Test NPC state retrieval latency."""
    
    npc_id = "test-npc-123"
    
    def make_request():
        response = client.get(
            f"/api/npc/{npc_id}/state",
            headers=auth_headers
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.get(f"/api/npc/{npc_id}/state", headers=auth_headers)
    )
    
    print(f"\n/api/npc/{{id}}/state latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/npc/{id}/state")


@pytest.mark.performance
@patch("routers.npc.Orchestrator")
def test_npc_update_state_latency(mock_orch, client, auth_headers, benchmark):
    """Test NPC state update latency."""
    
    npc_id = "test-npc-123"
    payload = {
        "location": "geffen",
        "position_x": 120.0,
        "position_y": 150.0,
        "health": 95.0
    }
    
    def make_request():
        response = client.put(
            f"/api/npc/{npc_id}/state",
            headers=auth_headers,
            json=payload
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.put(
            f"/api/npc/{npc_id}/state",
            headers=auth_headers,
            json=payload
        )
    )
    
    print(f"\n/api/npc/{{id}}/state (PUT) latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/npc/{id}/state (PUT)")


@pytest.mark.performance
@patch("routers.npc.Orchestrator")
def test_npc_event_latency(mock_orch, client, auth_headers, benchmark):
    """Test NPC event processing latency."""
    
    payload = {
        "event_type": "dialogue",
        "event_data": {
            "player_id": "player-123",
            "dialogue_option": "greeting"
        }
    }
    
    def make_request():
        response = client.post(
            "/api/npc/event",
            headers=auth_headers,
            json=payload
        )
        return response
    
    result = benchmark(make_request)
    
    stats = measure_latency(
        lambda: client.post("/api/npc/event", headers=auth_headers, json=payload)
    )
    
    print(f"\n/api/npc/event latency stats:")
    print(f"  p95: {stats['p95']:.2f}ms")
    
    assert_latency_requirement(stats, "/api/npc/event")


# ============================================================================
# LATENCY UNDER LOAD
# ============================================================================

@pytest.mark.performance
@pytest.mark.slow
def test_sustained_load_latency_degradation(client, auth_headers):
    """
    Test latency degradation under sustained load.
    
    Verifies that p95 latency remains <500ms even after 1000 requests.
    """
    endpoint = "/health"
    request_count = 1000
    batch_size = 100
    
    all_latencies = []
    
    print(f"\nTesting sustained load ({request_count} requests)...")
    
    for batch in range(request_count // batch_size):
        batch_latencies = []
        
        for _ in range(batch_size):
            start = time.perf_counter()
            response = client.get(endpoint)
            end = time.perf_counter()
            
            assert response.status_code == 200
            batch_latencies.append((end - start) * 1000)
        
        all_latencies.extend(batch_latencies)
        
        batch_p95 = sorted(batch_latencies)[int(len(batch_latencies) * 0.95)]
        print(f"  Batch {batch + 1}: p95 = {batch_p95:.2f}ms")
    
    all_latencies.sort()
    overall_p95 = all_latencies[int(len(all_latencies) * 0.95)]
    
    print(f"\nOverall p95 latency: {overall_p95:.2f}ms")
    
    assert overall_p95 < LATENCY_REQUIREMENT_MS, (
        f"Sustained load p95 {overall_p95:.2f}ms exceeds "
        f"requirement of {LATENCY_REQUIREMENT_MS}ms"
    )


# ============================================================================
# BOTTLENECK IDENTIFICATION
# ============================================================================

@pytest.mark.performance
def test_identify_slowest_endpoints(client, auth_headers):
    """Identify and report the slowest endpoints."""
    
    endpoints = [
        ("GET", "/health", None),
        ("GET", "/readiness", None),
        ("GET", "/api/v1/status", None),
        ("POST", "/api/v1/chat", {"message": "test"}),
    ]
    
    results = []
    
    print("\nEndpoint Performance Comparison:")
    print("-" * 60)
    
    for method, path, payload in endpoints:
        latencies = []
        
        for _ in range(50):
            start = time.perf_counter()
            
            if method == "GET":
                response = client.get(path, headers=auth_headers)
            else:
                response = client.post(path, headers=auth_headers, json=payload)
            
            end = time.perf_counter()
            latencies.append((end - start) * 1000)
        
        latencies.sort()
        p95 = latencies[int(len(latencies) * 0.95)]
        
        results.append((path, p95))
        print(f"  {method:6} {path:30} p95: {p95:6.2f}ms")
    
    print("-" * 60)
    
    # Sort by p95 latency
    results.sort(key=lambda x: x[1], reverse=True)
    
    print("\nSlowest Endpoints:")
    for i, (path, p95) in enumerate(results[:3], 1):
        print(f"  {i}. {path}: {p95:.2f}ms")
    
    # All should still meet requirements
    for path, p95 in results:
        assert p95 < LATENCY_REQUIREMENT_MS, (
            f"{path} p95 {p95:.2f}ms exceeds requirement"
        )