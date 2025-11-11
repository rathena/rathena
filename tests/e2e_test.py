#!/usr/bin/env python3
"""
End-to-End Integration Test for AI-rAthena System
Tests the complete flow from rAthena to AI Service and back
"""

import asyncio
import aiohttp
import json
import sys
from typing import Dict, Any
from datetime import datetime


class E2ETestRunner:
    """End-to-end test runner for AI-rAthena integration"""
    
    def __init__(self, ai_service_url: str = "http://localhost:8000"):
        self.ai_service_url = ai_service_url
        self.test_results = []
        
    async def run_all_tests(self):
        """Run all end-to-end tests"""
        print("=" * 60)
        print("AI-rAthena End-to-End Integration Tests")
        print("=" * 60)
        print(f"Testing AI Service at: {self.ai_service_url}")
        print(f"Started at: {datetime.now().isoformat()}")
        print("=" * 60)
        
        tests = [
            self.test_health_check,
            self.test_npc_registration,
            self.test_player_interaction,
            self.test_dialogue_generation,
            self.test_quest_generation,
            self.test_economy_analysis,
            self.test_memory_persistence,
            self.test_relationship_tracking,
            self.test_performance_under_load,
        ]
        
        for test in tests:
            await self.run_test(test)
        
        self.print_summary()
    
    async def run_test(self, test_func):
        """Run a single test and record results"""
        test_name = test_func.__name__.replace("test_", "").replace("_", " ").title()
        print(f"\n[TEST] {test_name}...")
        
        try:
            start_time = datetime.now()
            result = await test_func()
            duration = (datetime.now() - start_time).total_seconds() * 1000
            
            self.test_results.append({
                "name": test_name,
                "status": "PASS" if result else "FAIL",
                "duration_ms": duration
            })
            
            status = "✓ PASS" if result else "✗ FAIL"
            print(f"[RESULT] {status} ({duration:.2f}ms)")
            
        except Exception as e:
            self.test_results.append({
                "name": test_name,
                "status": "ERROR",
                "error": str(e)
            })
            print(f"[RESULT] ✗ ERROR: {str(e)}")
    
    async def test_health_check(self) -> bool:
        """Test that AI service is healthy"""
        async with aiohttp.ClientSession() as session:
            async with session.get(f"{self.ai_service_url}/health") as resp:
                return resp.status == 200
    
    async def test_npc_registration(self) -> bool:
        """Test NPC registration endpoint"""
        npc_data = {
            "npc_id": 9001,
            "name": "Test NPC",
            "personality": {
                "openness": 0.7,
                "conscientiousness": 0.6,
                "extraversion": 0.8,
                "agreeableness": 0.7,
                "neuroticism": 0.3
            },
            "background": "A friendly test NPC"
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.ai_service_url}/ai/npc/register",
                json=npc_data
            ) as resp:
                return resp.status in [200, 201]
    
    async def test_player_interaction(self) -> bool:
        """Test player interaction endpoint"""
        interaction_data = {
            "npc_id": 9001,
            "player_id": 100001,
            "player_name": "TestPlayer",
            "message": "Hello, how are you?",
            "context": {
                "location": "prontera",
                "time_of_day": "morning"
            }
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.ai_service_url}/ai/player/interaction",
                json=interaction_data
            ) as resp:
                if resp.status != 200:
                    return False
                data = await resp.json()
                return "dialogue" in data and len(data["dialogue"]) > 0
    
    async def test_dialogue_generation(self) -> bool:
        """Test dialogue generation with various contexts"""
        contexts = [
            {"player_message": "I need help", "mood": "urgent"},
            {"player_message": "Tell me a story", "mood": "curious"},
            {"player_message": "Goodbye", "mood": "neutral"}
        ]
        
        for context in contexts:
            interaction_data = {
                "npc_id": 9001,
                "player_id": 100001,
                "message": context["player_message"],
                "context": context
            }
            
            async with aiohttp.ClientSession() as session:
                async with session.post(
                    f"{self.ai_service_url}/ai/player/interaction",
                    json=interaction_data
                ) as resp:
                    if resp.status != 200:
                        return False
                    data = await resp.json()
                    if "dialogue" not in data or len(data["dialogue"]) == 0:
                        return False
        
        return True
    
    async def test_quest_generation(self) -> bool:
        """Test quest generation endpoint"""
        quest_request = {
            "npc_id": 9001,
            "player_id": 100001,
            "player_level": 50,
            "difficulty": "medium",
            "quest_type": "fetch"
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.ai_service_url}/ai/quest/generate",
                json=quest_request
            ) as resp:
                if resp.status != 200:
                    return False
                data = await resp.json()
                return all(key in data for key in ["title", "description", "objectives"])
    
    async def test_economy_analysis(self) -> bool:
        """Test economy analysis endpoint"""
        market_data = {
            "item_id": 501,
            "current_price": 50,
            "supply": 1000,
            "demand": 800
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.ai_service_url}/ai/economy/market/analyze",
                json=market_data
            ) as resp:
                return resp.status == 200
    
    async def test_memory_persistence(self) -> bool:
        """Test that memories are persisted and retrieved"""
        # Store a memory
        memory_data = {
            "npc_id": 9001,
            "player_id": 100001,
            "content": "Player helped with quest",
            "importance": 0.8
        }
        
        async with aiohttp.ClientSession() as session:
            # Store memory
            async with session.post(
                f"{self.ai_service_url}/ai/memory/store",
                json=memory_data
            ) as resp:
                if resp.status not in [200, 201]:
                    return False
            
            # Retrieve memory
            async with session.get(
                f"{self.ai_service_url}/ai/memory/retrieve?npc_id=9001&limit=10"
            ) as resp:
                if resp.status != 200:
                    return False
                data = await resp.json()
                return isinstance(data, list) and len(data) > 0
    
    async def test_relationship_tracking(self) -> bool:
        """Test relationship tracking between NPC and player"""
        relationship_data = {
            "npc_id": 9001,
            "player_id": 100001,
            "affinity_change": 0.1,
            "interaction_type": "positive"
        }
        
        async with aiohttp.ClientSession() as session:
            async with session.post(
                f"{self.ai_service_url}/ai/relationship/update",
                json=relationship_data
            ) as resp:
                return resp.status in [200, 201]
    
    async def test_performance_under_load(self) -> bool:
        """Test system performance under concurrent load"""
        num_requests = 10
        tasks = []
        
        async with aiohttp.ClientSession() as session:
            for i in range(num_requests):
                task = session.post(
                    f"{self.ai_service_url}/ai/player/interaction",
                    json={
                        "npc_id": 9001,
                        "player_id": 100000 + i,
                        "message": f"Test message {i}"
                    }
                )
                tasks.append(task)
            
            responses = await asyncio.gather(*tasks, return_exceptions=True)
            
            # Check that most requests succeeded
            success_count = sum(1 for r in responses if not isinstance(r, Exception) and r.status == 200)
            return success_count >= num_requests * 0.8  # 80% success rate
    
    def print_summary(self):
        """Print test summary"""
        print("\n" + "=" * 60)
        print("Test Summary")
        print("=" * 60)
        
        total = len(self.test_results)
        passed = sum(1 for r in self.test_results if r["status"] == "PASS")
        failed = sum(1 for r in self.test_results if r["status"] == "FAIL")
        errors = sum(1 for r in self.test_results if r["status"] == "ERROR")
        
        print(f"Total Tests: {total}")
        print(f"Passed: {passed} ({passed/total*100:.1f}%)")
        print(f"Failed: {failed} ({failed/total*100:.1f}%)")
        print(f"Errors: {errors} ({errors/total*100:.1f}%)")
        print("=" * 60)
        
        if passed == total:
            print("✓ ALL TESTS PASSED!")
            sys.exit(0)
        else:
            print("✗ SOME TESTS FAILED")
            sys.exit(1)


async def main():
    """Main entry point"""
    runner = E2ETestRunner()
    await runner.run_all_tests()


if __name__ == "__main__":
    asyncio.run(main())

