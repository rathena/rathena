"""
Performance Load Testing Script
Tests the 5 critical performance optimizations

Requirements:
    pip install locust pytest-benchmark requests

Usage:
    # Run load test
    locust -f load_test.py --users 100 --spawn-rate 10 --host http://localhost:8000
    
    # Run benchmark test
    pytest load_test.py::test_performance_benchmarks -v
"""

from locust import HttpUser, task, between, events
import random
import json
import time
from typing import Dict, Any


# Test data
NPC_IDS = ["merchant_001", "guard_001", "quest_giver_001", "blacksmith_001", "priest_001"]
PLAYER_MESSAGES = [
    "Hello",
    "What do you have for sale?",
    "Tell me about this place",
    "Do you have any quests for me?",
    "Goodbye"
]


class NPCInteractionUser(HttpUser):
    """Simulates player interacting with NPCs"""
    
    wait_time = between(1, 3)  # 1-3 seconds between requests
    
    def on_start(self):
        """Initialize user session"""
        self.player_id = f"player_{random.randint(1000, 9999)}"
        self.npc_id = random.choice(NPC_IDS)
    
    @task(5)
    def npc_dialogue(self):
        """Test NPC dialogue endpoint (most critical)"""
        self.client.post(
            "/api/v1/npc/dialogue",
            json={
                "npc_id": self.npc_id,
                "player_id": self.player_id,
                "player_message": random.choice(PLAYER_MESSAGES),
                "context": {
                    "location": "prontera",
                    "time_of_day": "afternoon"
                }
            },
            name="NPC Dialogue",
            catch_response=True
        )
    
    @task(2)
    def npc_movement(self):
        """Test NPC movement endpoint"""
        self.client.post(
            "/api/npc/movement",
            json={
                "npc_id": self.npc_id,
                "action": "move",
                "target_x": random.randint(50, 200),
                "target_y": random.randint(50, 200)
            },
            name="NPC Movement"
        )
    
    @task(1)
    def health_check(self):
        """Test health check endpoint"""
        self.client.get("/health", name="Health Check")
    
    @task(1)
    def detailed_health_check(self):
        """Test detailed health check"""
        self.client.get("/health/detailed", name="Detailed Health")


class StressTestUser(HttpUser):
    """Aggressive stress testing to find breaking points"""
    
    wait_time = between(0.1, 0.5)  # Minimal wait time
    
    @task
    def rapid_fire_requests(self):
        """Rapid fire requests to stress test"""
        npc_id = random.choice(NPC_IDS)
        self.client.post(
            "/api/v1/npc/dialogue",
            json={
                "npc_id": npc_id,
                "player_id": f"stress_{random.randint(1, 1000)}",
                "player_message": "Quick test"
            },
            name="Stress Test"
        )


# Performance tracking
@events.test_start.add_listener
def on_test_start(environment, **kwargs):
    """Log test start"""
    print("\n" + "="*60)
    print("PERFORMANCE LOAD TEST STARTED")
    print("="*60)
    print(f"Target: http://localhost:8000")
    print(f"Optimizations being tested:")
    print("  1. HTTP connection pooling (C++)")
    print("  2. LLM timeout reduction (60s -> 10s)")
    print("  3. PostgreSQL pool size increase (10 -> 25)")
    print("  4. N+1 query pattern fix (batch queries)")
    print("  5. Circuit breaker implementation")
    print("="*60 + "\n")


@events.test_stop.add_listener
def on_test_stop(environment, **kwargs):
    """Log test results"""
    stats = environment.stats
    print("\n" + "="*60)
    print("PERFORMANCE TEST RESULTS")
    print("="*60)
    
    for name, stat in stats.entries.items():
        if stat.num_requests > 0:
            print(f"\n{name}:")
            print(f"  Total Requests: {stat.num_requests}")
            print(f"  Failures: {stat.num_failures} ({stat.fail_ratio*100:.1f}%)")
            print(f"  Average Latency: {stat.avg_response_time:.2f}ms")
            print(f"  Median (p50): {stat.median_response_time:.2f}ms")
            print(f"  p95 Latency: {stat.get_response_time_percentile(0.95):.2f}ms")
            print(f"  p99 Latency: {stat.get_response_time_percentile(0.99):.2f}ms")
            print(f"  Min/Max: {stat.min_response_time:.2f}ms / {stat.max_response_time:.2f}ms")
            print(f"  RPS: {stat.total_rps:.2f}")
    
    print("\n" + "="*60)
    print("TARGET METRICS:")
    print("  p95 Latency: <500ms")
    print("  Throughput: 100+ req/s")
    print("  Concurrent Users: 100+")
    print("  Error Rate: <1%")
    
    # Check if targets met
    print("\nRESULTS:")
    dialogue_stats = stats.entries.get("NPC Dialogue")
    if dialogue_stats:
        p95 = dialogue_stats.get_response_time_percentile(0.95)
        if p95 < 500:
            print(f"  ✅ p95 latency target MET ({p95:.2f}ms < 500ms)")
        else:
            print(f"  ❌ p95 latency target MISSED ({p95:.2f}ms > 500ms)")
        
        if stats.total_rps >= 100:
            print(f"  ✅ Throughput target MET ({stats.total_rps:.2f} req/s >= 100 req/s)")
        else:
            print(f"  ❌ Throughput target MISSED ({stats.total_rps:.2f} req/s < 100 req/s)")
        
        if dialogue_stats.fail_ratio < 0.01:
            print(f"  ✅ Error rate target MET ({dialogue_stats.fail_ratio*100:.2f}% < 1%)")
        else:
            print(f"  ❌ Error rate target MISSED ({dialogue_stats.fail_ratio*100:.2f}% > 1%)")
    
    print("="*60 + "\n")


# Pytest benchmark tests
def test_http_connection_pooling(benchmark):
    """Benchmark HTTP connection pooling improvement"""
    import requests
    
    def make_request():
        response = requests.get("http://localhost:8000/health")
        return response.json()
    
    result = benchmark(make_request)
    
    # Should be <10ms with connection pooling
    assert result['mean'] < 0.01, f"Health check too slow: {result['mean']*1000:.2f}ms"


def test_npc_movement_latency(benchmark):
    """Benchmark NPC movement endpoint"""
    import requests
    
    def make_request():
        response = requests.post(
            "http://localhost:8000/api/npc/movement",
            json={
                "npc_id": "test_npc",
                "action": "move",
                "target_x": 100,
                "target_y": 150
            }
        )
        return response.json()
    
    result = benchmark(make_request)
    
    # Should be <50ms average
    assert result['mean'] < 0.05, f"Movement too slow: {result['mean']*1000:.2f}ms"


def test_database_query_optimization(benchmark):
    """Test N+1 query fix effectiveness"""
    import requests
    
    def make_request():
        # This should use the optimized batch query
        response = requests.get("http://localhost:8000/api/quests/npc/quest_giver_001")
        return response.json()
    
    result = benchmark(make_request)
    
    # With batch query, should be fast even with many quests
    assert result['mean'] < 0.1, f"Quest retrieval too slow: {result['mean']*1000:.2f}ms"


if __name__ == "__main__":
    print("Run with: locust -f load_test.py --users 100 --spawn-rate 10")