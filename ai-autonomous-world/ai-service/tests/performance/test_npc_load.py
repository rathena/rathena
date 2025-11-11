"""
Load Testing with 100+ NPCs

This test simulates a realistic MMORPG scenario with 100+ concurrent NPCs
performing various autonomous behaviors.

Run with:
    pytest tests/performance/test_npc_load.py -v -s
    pytest tests/performance/test_npc_load.py::test_100_npc_concurrent_interactions -v -s
"""

import pytest
import asyncio
import httpx
import time
import random
from typing import List, Dict
from datetime import datetime


class NPCLoadTester:
    """Load tester for NPC operations"""
    
    def __init__(self, base_url: str = "http://localhost:8000"):
        self.base_url = base_url
        self.npc_count = 0
        self.interaction_count = 0
        self.error_count = 0
        self.total_latency = 0.0
        self.latencies = []
    
    async def register_npc(self, client: httpx.AsyncClient, npc_id: str) -> bool:
        """Register a single NPC"""
        payload = {
            "npc_id": npc_id,
            "name": f"Load Test NPC {npc_id}",
            "npc_class": random.choice(["merchant", "guard", "quest_giver", "villager"]),
            "level": random.randint(1, 99),
            "position": {
                "map": "prontera",
                "x": random.randint(100, 200),
                "y": random.randint(100, 200)
            },
            "personality": {
                "openness": random.random(),
                "conscientiousness": random.random(),
                "extraversion": random.random(),
                "agreeableness": random.random(),
                "neuroticism": random.random(),
                "moral_alignment": random.choice(["lawful_good", "neutral", "chaotic_evil"])
            }
        }
        
        try:
            start_time = time.time()
            response = await client.post(f"{self.base_url}/ai/npc/register", json=payload, timeout=30.0)
            latency = time.time() - start_time
            
            self.latencies.append(latency)
            self.total_latency += latency
            
            if response.status_code == 200:
                self.npc_count += 1
                return True
            else:
                self.error_count += 1
                print(f"Failed to register NPC {npc_id}: {response.status_code}")
                return False
        except Exception as e:
            self.error_count += 1
            print(f"Error registering NPC {npc_id}: {e}")
            return False
    
    async def player_interaction(self, client: httpx.AsyncClient, npc_id: str, player_id: str) -> bool:
        """Simulate player-NPC interaction"""
        payload = {
            "npc_id": npc_id,
            "player_id": player_id,
            "player_name": f"Player_{player_id}",
            "interaction_type": random.choice(["talk", "trade", "quest"]),
            "message": random.choice([
                "Hello!",
                "What do you sell?",
                "Any quests available?",
                "Tell me about this area",
                "Goodbye"
            ]),
            "context": {
                "location": {"map": "prontera", "x": 150, "y": 180},
                "time": datetime.utcnow().isoformat(),
                "weather": random.choice(["sunny", "rainy", "cloudy", "clear"])
            }
        }
        
        try:
            start_time = time.time()
            response = await client.post(f"{self.base_url}/ai/player/interaction", json=payload, timeout=30.0)
            latency = time.time() - start_time
            
            self.latencies.append(latency)
            self.total_latency += latency
            
            if response.status_code == 200:
                self.interaction_count += 1
                return True
            else:
                self.error_count += 1
                return False
        except Exception as e:
            self.error_count += 1
            return False
    
    def get_statistics(self) -> Dict:
        """Get performance statistics"""
        if not self.latencies:
            return {}
        
        sorted_latencies = sorted(self.latencies)
        count = len(sorted_latencies)
        
        return {
            "total_operations": count,
            "npc_registrations": self.npc_count,
            "player_interactions": self.interaction_count,
            "errors": self.error_count,
            "error_rate": self.error_count / count if count > 0 else 0,
            "avg_latency_ms": (self.total_latency / count * 1000) if count > 0 else 0,
            "p50_latency_ms": sorted_latencies[int(count * 0.50)] * 1000 if count > 0 else 0,
            "p95_latency_ms": sorted_latencies[int(count * 0.95)] * 1000 if count > 0 else 0,
            "p99_latency_ms": sorted_latencies[int(count * 0.99)] * 1000 if count > 0 else 0,
            "min_latency_ms": min(sorted_latencies) * 1000 if sorted_latencies else 0,
            "max_latency_ms": max(sorted_latencies) * 1000 if sorted_latencies else 0,
        }


@pytest.mark.asyncio
@pytest.mark.slow
@pytest.mark.integration
@pytest.mark.skip(reason="Integration test - requires running service. Run with: pytest -m integration")
async def test_100_npc_registration():
    """Test registering 100 NPCs concurrently"""
    print("\n" + "="*80)
    print("TEST: Register 100 NPCs Concurrently")
    print("="*80)
    
    tester = NPCLoadTester()
    npc_ids = [f"load_test_npc_{i:03d}" for i in range(1, 101)]
    
    async with httpx.AsyncClient() as client:
        tasks = [tester.register_npc(client, npc_id) for npc_id in npc_ids]
        results = await asyncio.gather(*tasks)
    
    stats = tester.get_statistics()
    
    print(f"\nResults:")
    print(f"  Total NPCs: {stats['npc_registrations']}")
    print(f"  Errors: {stats['errors']}")
    print(f"  Error Rate: {stats['error_rate']:.2%}")
    print(f"  Avg Latency: {stats['avg_latency_ms']:.2f}ms")
    print(f"  P50 Latency: {stats['p50_latency_ms']:.2f}ms")
    print(f"  P95 Latency: {stats['p95_latency_ms']:.2f}ms")
    print(f"  P99 Latency: {stats['p99_latency_ms']:.2f}ms")
    
    # Assertions
    assert stats['npc_registrations'] >= 90, "At least 90% of NPCs should register successfully"
    assert stats['error_rate'] < 0.1, "Error rate should be less than 10%"
    assert stats['p95_latency_ms'] < 5000, "P95 latency should be less than 5 seconds"


@pytest.mark.asyncio
@pytest.mark.slow
@pytest.mark.integration
@pytest.mark.skip(reason="Integration test - requires running service. Run with: pytest -m integration")
async def test_100_npc_concurrent_interactions():
    """Test 100 NPCs with concurrent player interactions"""
    print("\n" + "="*80)
    print("TEST: 100 NPCs with Concurrent Player Interactions")
    print("="*80)

    tester = NPCLoadTester()
    npc_ids = [f"load_test_npc_{i:03d}" for i in range(1, 101)]
    player_ids = [f"player_{i:04d}" for i in range(1, 501)]  # 500 players

    # Step 1: Register NPCs
    print("\nStep 1: Registering 100 NPCs...")
    async with httpx.AsyncClient() as client:
        tasks = [tester.register_npc(client, npc_id) for npc_id in npc_ids]
        await asyncio.gather(*tasks)

    print(f"  Registered: {tester.npc_count} NPCs")

    # Reset stats for interaction phase
    tester.latencies = []
    tester.total_latency = 0.0

    # Step 2: Simulate player interactions (each NPC gets 5 interactions)
    print("\nStep 2: Simulating 500 player interactions...")
    async with httpx.AsyncClient() as client:
        tasks = []
        for _ in range(500):
            npc_id = random.choice(npc_ids)
            player_id = random.choice(player_ids)
            tasks.append(tester.player_interaction(client, npc_id, player_id))

        await asyncio.gather(*tasks)

    stats = tester.get_statistics()

    print(f"\nResults:")
    print(f"  Total Interactions: {stats['player_interactions']}")
    print(f"  Errors: {stats['errors']}")
    print(f"  Error Rate: {stats['error_rate']:.2%}")
    print(f"  Avg Latency: {stats['avg_latency_ms']:.2f}ms")
    print(f"  P50 Latency: {stats['p50_latency_ms']:.2f}ms")
    print(f"  P95 Latency: {stats['p95_latency_ms']:.2f}ms")
    print(f"  P99 Latency: {stats['p99_latency_ms']:.2f}ms")

    # Assertions
    assert stats['player_interactions'] >= 450, "At least 90% of interactions should succeed"
    assert stats['error_rate'] < 0.1, "Error rate should be less than 10%"
    assert stats['p95_latency_ms'] < 5000, "P95 latency should be less than 5 seconds"


@pytest.mark.asyncio
@pytest.mark.slow
@pytest.mark.integration
@pytest.mark.skip(reason="Integration test - requires running service. Run with: pytest -m integration")
async def test_sustained_load_100_npcs():
    """Test sustained load with 100 NPCs over 60 seconds"""
    print("\n" + "="*80)
    print("TEST: Sustained Load - 100 NPCs for 60 seconds")
    print("="*80)

    tester = NPCLoadTester()
    npc_ids = [f"load_test_npc_{i:03d}" for i in range(1, 101)]
    player_ids = [f"player_{i:04d}" for i in range(1, 1001)]

    # Register NPCs first
    print("\nRegistering 100 NPCs...")
    async with httpx.AsyncClient() as client:
        tasks = [tester.register_npc(client, npc_id) for npc_id in npc_ids]
        await asyncio.gather(*tasks)

    print(f"  Registered: {tester.npc_count} NPCs")

    # Reset stats
    tester.latencies = []
    tester.total_latency = 0.0

    # Sustained load for 60 seconds
    print("\nRunning sustained load for 60 seconds...")
    start_time = time.time()
    duration = 60  # seconds
    interaction_interval = 0.1  # 10 interactions per second

    async with httpx.AsyncClient() as client:
        while time.time() - start_time < duration:
            # Create batch of interactions
            tasks = []
            for _ in range(10):  # 10 concurrent interactions
                npc_id = random.choice(npc_ids)
                player_id = random.choice(player_ids)
                tasks.append(tester.player_interaction(client, npc_id, player_id))

            await asyncio.gather(*tasks)
            await asyncio.sleep(interaction_interval)

            # Progress update every 10 seconds
            elapsed = time.time() - start_time
            if int(elapsed) % 10 == 0 and int(elapsed) > 0:
                print(f"  Progress: {int(elapsed)}s - {tester.interaction_count} interactions")

    stats = tester.get_statistics()
    total_time = time.time() - start_time
    throughput = stats['player_interactions'] / total_time

    print(f"\nResults:")
    print(f"  Duration: {total_time:.2f}s")
    print(f"  Total Interactions: {stats['player_interactions']}")
    print(f"  Throughput: {throughput:.2f} req/s")
    print(f"  Errors: {stats['errors']}")
    print(f"  Error Rate: {stats['error_rate']:.2%}")
    print(f"  Avg Latency: {stats['avg_latency_ms']:.2f}ms")
    print(f"  P50 Latency: {stats['p50_latency_ms']:.2f}ms")
    print(f"  P95 Latency: {stats['p95_latency_ms']:.2f}ms")
    print(f"  P99 Latency: {stats['p99_latency_ms']:.2f}ms")

    # Assertions
    assert throughput >= 5, "Throughput should be at least 5 req/s"
    assert stats['error_rate'] < 0.1, "Error rate should be less than 10%"
    assert stats['p95_latency_ms'] < 5000, "P95 latency should be less than 5 seconds"


@pytest.mark.asyncio
@pytest.mark.slow
@pytest.mark.integration
@pytest.mark.skip(reason="Integration test - requires running service. Run with: pytest -m integration")
async def test_stress_test_200_npcs():
    """Stress test with 200 NPCs"""
    print("\n" + "="*80)
    print("TEST: Stress Test - 200 NPCs")
    print("="*80)

    tester = NPCLoadTester()
    npc_ids = [f"stress_test_npc_{i:03d}" for i in range(1, 201)]

    print("\nRegistering 200 NPCs...")
    async with httpx.AsyncClient() as client:
        # Register in batches of 50
        for i in range(0, 200, 50):
            batch = npc_ids[i:i+50]
            tasks = [tester.register_npc(client, npc_id) for npc_id in batch]
            await asyncio.gather(*tasks)
            print(f"  Registered batch {i//50 + 1}/4: {len(batch)} NPCs")

    stats = tester.get_statistics()

    print(f"\nResults:")
    print(f"  Total NPCs: {stats['npc_registrations']}")
    print(f"  Errors: {stats['errors']}")
    print(f"  Error Rate: {stats['error_rate']:.2%}")
    print(f"  Avg Latency: {stats['avg_latency_ms']:.2f}ms")
    print(f"  P95 Latency: {stats['p95_latency_ms']:.2f}ms")

    # Assertions
    assert stats['npc_registrations'] >= 180, "At least 90% of NPCs should register successfully"
    assert stats['error_rate'] < 0.15, "Error rate should be less than 15% under stress"


if __name__ == "__main__":
    # Run tests manually
    import sys

    print("Running NPC Load Tests...")
    print("Make sure the AI Service is running on http://localhost:8000")
    print()

    asyncio.run(test_100_npc_registration())
    asyncio.run(test_100_npc_concurrent_interactions())
    asyncio.run(test_sustained_load_100_npcs())
    asyncio.run(test_stress_test_200_npcs())

    print("\n" + "="*80)
    print("All load tests completed!")
    print("="*80)

