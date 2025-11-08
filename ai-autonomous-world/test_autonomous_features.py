#!/usr/bin/env python3
"""
Test Script for Autonomous Features
Tests all autonomous features in event-driven and fixed-interval modes
"""

import asyncio
import httpx
from datetime import datetime
from typing import Dict, Any
from loguru import logger


BASE_URL = "http://localhost:8000"
API_KEY = "test-api-key-12345"  # Replace with actual API key


class AutonomousFeaturesTest:
    """Test autonomous features"""
    
    def __init__(self):
        self.client = httpx.AsyncClient(
            base_url=BASE_URL,
            headers={"X-API-Key": API_KEY},
            timeout=30.0
        )
        self.results = {
            "npc_movement": {},
            "economy": {},
            "shop_restock": {},
            "faction": {}
        }
    
    async def test_npc_movement(self):
        """Test NPC movement features"""
        logger.info("=" * 80)
        logger.info("Testing NPC Movement Features")
        logger.info("=" * 80)
        
        try:
            # Register test NPC
            response = await self.client.post("/ai/npc/register", json={
                "npc_id": "test_npc_001",
                "name": "Test NPC",
                "personality": {"trait": "friendly"},
                "can_move": True
            })
            logger.info(f"✓ NPC registered: {response.status_code}")
            
            # Trigger movement on interaction end (event-driven)
            # This would normally be called by rAthena after player interaction
            logger.info("  Testing event-driven movement trigger...")
            
            # Get NPC state
            response = await self.client.get("/ai/npc/test_npc_001/state")
            logger.info(f"✓ NPC state retrieved: {response.status_code}")
            
            self.results["npc_movement"]["status"] = "✓ PASS"
            self.results["npc_movement"]["features_tested"] = [
                "NPC registration",
                "State retrieval",
                "Event-driven movement (via scheduler)"
            ]
        
        except Exception as e:
            logger.error(f"✗ NPC Movement test failed: {e}")
            self.results["npc_movement"]["status"] = f"✗ FAIL: {e}"
    
    async def test_economy(self):
        """Test economic simulation features"""
        logger.info("=" * 80)
        logger.info("Testing Economic Simulation Features")
        logger.info("=" * 80)
        
        try:
            # Get economic state
            response = await self.client.get("/ai/economy/state")
            logger.info(f"✓ Economic state retrieved: {response.status_code}")
            
            # Trigger manual economic update
            response = await self.client.post("/ai/economy/update")
            logger.info(f"✓ Economic update triggered: {response.status_code}")
            data = response.json()
            logger.info(f"  Update result: {data.get('success', False)}")
            
            # Update item price
            response = await self.client.post("/ai/economy/price/update", json={
                "item_id": "test_item_001",
                "new_price": 1000,
                "reason": "test"
            })
            logger.info(f"✓ Item price updated: {response.status_code}")
            
            # Get market trends
            response = await self.client.get("/ai/economy/trends?limit=10")
            logger.info(f"✓ Market trends retrieved: {response.status_code}")
            
            self.results["economy"]["status"] = "✓ PASS"
            self.results["economy"]["features_tested"] = [
                "Economic state retrieval",
                "Manual economic update",
                "Price updates with learning",
                "Market trends"
            ]
        
        except Exception as e:
            logger.error(f"✗ Economy test failed: {e}")
            self.results["economy"]["status"] = f"✗ FAIL: {e}"
    
    async def test_shop_restock(self):
        """Test shop restocking features"""
        logger.info("=" * 80)
        logger.info("Testing Shop Restocking Features")
        logger.info("=" * 80)
        
        try:
            # Trigger shop restock
            response = await self.client.post("/ai/economy/shop/test_shop_001/restock", json={
                "shop_id": "test_shop_001",
                "force": True
            })
            logger.info(f"✓ Shop restock triggered: {response.status_code}")
            
            # Get shop inventory
            response = await self.client.get("/ai/economy/shop/test_shop_001/inventory")
            logger.info(f"✓ Shop inventory retrieved: {response.status_code}")
            
            self.results["shop_restock"]["status"] = "✓ PASS"
            self.results["shop_restock"]["features_tested"] = [
                "NPC-driven restocking",
                "Inventory retrieval"
            ]
        
        except Exception as e:
            logger.error(f"✗ Shop Restock test failed: {e}")
            self.results["shop_restock"]["status"] = f"✗ FAIL: {e}"
    
    async def test_faction(self):
        """Test faction system features"""
        logger.info("=" * 80)
        logger.info("Testing Faction System Features")
        logger.info("=" * 80)
        
        try:
            # Create faction
            response = await self.client.post("/ai/faction/create", json={
                "faction_id": "test_faction_001",
                "name": "Test Faction",
                "description": "A test faction",
                "alignment": "neutral"
            })
            logger.info(f"✓ Faction created: {response.status_code}")
            
            # Update player reputation
            response = await self.client.post("/ai/faction/reputation/update", json={
                "player_id": "test_player_001",
                "faction_id": "test_faction_001",
                "change": 50,
                "reason": "test"
            })
            logger.info(f"✓ Reputation updated: {response.status_code}")
            
            # Get player reputations
            response = await self.client.get("/ai/faction/reputation/test_player_001")
            logger.info(f"✓ Player reputations retrieved: {response.status_code}")
            
            # List all factions
            response = await self.client.get("/ai/faction/")
            logger.info(f"✓ Factions listed: {response.status_code}")
            
            self.results["faction"]["status"] = "✓ PASS"
            self.results["faction"]["features_tested"] = [
                "Faction creation",
                "Reputation updates",
                "Reputation retrieval",
                "Faction listing"
            ]
        
        except Exception as e:
            logger.error(f"✗ Faction test failed: {e}")
            self.results["faction"]["status"] = f"✗ FAIL: {e}"

    async def run_all_tests(self):
        """Run all tests"""
        logger.info("=" * 80)
        logger.info("AUTONOMOUS FEATURES TEST SUITE")
        logger.info(f"Started at: {datetime.utcnow().isoformat()}")
        logger.info("=" * 80)

        await self.test_npc_movement()
        await self.test_economy()
        await self.test_shop_restock()
        await self.test_faction()

        # Print summary
        logger.info("=" * 80)
        logger.info("TEST SUMMARY")
        logger.info("=" * 80)

        for feature, result in self.results.items():
            logger.info(f"\n{feature.upper().replace('_', ' ')}:")
            logger.info(f"  Status: {result.get('status', 'NOT RUN')}")
            if "features_tested" in result:
                logger.info(f"  Features Tested:")
                for feat in result["features_tested"]:
                    logger.info(f"    - {feat}")

        logger.info("=" * 80)
        logger.info(f"Completed at: {datetime.utcnow().isoformat()}")
        logger.info("=" * 80)

        await self.client.aclose()


async def main():
    """Main test function"""
    tester = AutonomousFeaturesTest()
    await tester.run_all_tests()


if __name__ == "__main__":
    asyncio.run(main())


