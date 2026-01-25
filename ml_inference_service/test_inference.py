#!/usr/bin/env python3
"""
ML Inference Service Integration Test
Tests end-to-end flow: C++ → PostgreSQL → Python → Redis
"""

import asyncio
import asyncpg
import redis
import numpy as np
import time
import sys
import logging
from typing import Dict, Any

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s'
)
logger = logging.getLogger(__name__)


class InferenceServiceTester:
    """Test ML inference service integration"""
    
    def __init__(self):
        self.pg_pool = None
        self.redis_client = None
        
        # Test configuration
        self.pg_config = {
            'host': 'localhost',
            'port': 5432,
            'database': 'ragnarok_ml',
            'user': 'ml_user',
            'password': ''  # Set if needed
        }
        
        self.redis_config = {
            'host': 'localhost',
            'port': 6379,
            'db': 0
        }
    
    async def setup(self) -> bool:
        """Setup test connections"""
        logger.info("Setting up test connections...")
        
        # Connect to PostgreSQL
        try:
            self.pg_pool = await asyncpg.create_pool(**self.pg_config)
            async with self.pg_pool.acquire() as conn:
                version = await conn.fetchval("SELECT version()")
                logger.info(f"PostgreSQL connected: {version[:50]}...")
        except Exception as e:
            logger.error(f"PostgreSQL connection failed: {e}")
            return False
        
        # Connect to Redis
        try:
            self.redis_client = redis.Redis(**self.redis_config)
            self.redis_client.ping()
            logger.info("Redis connected")
        except Exception as e:
            logger.error(f"Redis connection failed: {e}")
            return False
        
        return True
    
    async def test_database_schema(self) -> bool:
        """Test that required tables exist"""
        logger.info("Testing database schema...")
        
        async with self.pg_pool.acquire() as conn:
            # Check ai_requests table
            exists = await conn.fetchval("""
                SELECT EXISTS (
                    SELECT 1 FROM information_schema.tables 
                    WHERE table_name = 'ai_requests'
                )
            """)
            
            if not exists:
                logger.error("ai_requests table does not exist!")
                return False
            
            logger.info("✓ ai_requests table exists")
            
            # Check ai_responses table
            exists = await conn.fetchval("""
                SELECT EXISTS (
                    SELECT 1 FROM information_schema.tables 
                    WHERE table_name = 'ai_responses'
                )
            """)
            
            if not exists:
                logger.error("ai_responses table does not exist!")
                return False
            
            logger.info("✓ ai_responses table exists")
        
        return True
    
    async def test_request_response_flow(self) -> bool:
        """Test complete request-response flow"""
        logger.info("Testing request-response flow...")
        
        # Generate test state vector (64 dimensions)
        test_state = np.random.rand(64).astype(np.float32)
        test_archetype = 'aggressive'
        test_monster_id = 999999  # Test monster ID
        test_mob_id = 1002  # Poring
        
        async with self.pg_pool.acquire() as conn:
            # Insert test request
            logger.info("Inserting test request...")
            
            state_list = test_state.tolist()
            
            request_id = await conn.fetchval("""
                INSERT INTO ai_requests (
                    monster_id, mob_id, archetype, state_vector,
                    map_id, position_x, position_y, hp_ratio, sp_ratio,
                    status, priority
                ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
                RETURNING request_id
            """, test_monster_id, test_mob_id, test_archetype, state_list,
                1, 100, 100, 0.8, 0.5, 'pending', 5)
            
            logger.info(f"✓ Request inserted: request_id={request_id}")
            
            # Wait for response (with timeout)
            logger.info("Waiting for inference service to process request...")
            max_wait = 5.0  # 5 seconds timeout
            poll_interval = 0.1  # 100ms polling
            elapsed = 0.0
            
            response = None
            while elapsed < max_wait:
                response = await conn.fetchrow("""
                    SELECT action_id, confidence, inference_latency_ms, 
                           cache_used, fallback_level
                    FROM ai_responses
                    WHERE request_id = $1
                """, request_id)
                
                if response:
                    break
                
                await asyncio.sleep(poll_interval)
                elapsed += poll_interval
            
            if response:
                logger.info(f"✓ Response received:")
                logger.info(f"    Action ID: {response['action_id']}")
                logger.info(f"    Confidence: {response['confidence']:.3f}")
                logger.info(f"    Latency: {response['inference_latency_ms']:.2f}ms")
                logger.info(f"    From cache: {response['cache_used']}")
                logger.info(f"    Fallback level: {response['fallback_level']}")
                
                # Cleanup
                await conn.execute("DELETE FROM ai_requests WHERE request_id = $1", request_id)
                await conn.execute("DELETE FROM ai_responses WHERE request_id = $1", request_id)
                
                return True
            else:
                logger.error(f"✗ No response received after {max_wait}s")
                logger.error("Inference service may not be running or is too slow")
                
                # Cleanup failed request
                await conn.execute("DELETE FROM ai_requests WHERE request_id = $1", request_id)
                
                return False
    
    async def test_redis_cache(self) -> bool:
        """Test Redis cache functionality"""
        logger.info("Testing Redis cache...")
        
        try:
            # Test L2 cache (action cache)
            test_key = "ml:l2:test:abcd1234"
            test_value = "5:0.85"  # action_id:confidence
            
            self.redis_client.setex(test_key, 10, test_value)
            
            retrieved = self.redis_client.get(test_key)
            if retrieved and retrieved.decode('utf-8') == test_value:
                logger.info("✓ L2 cache write/read works")
            else:
                logger.error("✗ L2 cache read failed")
                return False
            
            # Cleanup
            self.redis_client.delete(test_key)
            
            return True
        
        except Exception as e:
            logger.error(f"Redis cache test failed: {e}")
            return False
    
    async def test_batch_processing(self) -> bool:
        """Test batch processing with multiple requests"""
        logger.info("Testing batch processing (10 requests)...")
        
        batch_size = 10
        request_ids = []
        
        async with self.pg_pool.acquire() as conn:
            # Insert batch of requests
            for i in range(batch_size):
                test_state = np.random.rand(64).astype(np.float32).tolist()
                archetype = ['aggressive', 'defensive', 'mage'][i % 3]
                
                request_id = await conn.fetchval("""
                    INSERT INTO ai_requests (
                        monster_id, mob_id, archetype, state_vector,
                        map_id, position_x, position_y, hp_ratio, sp_ratio,
                        status, priority
                    ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
                    RETURNING request_id
                """, 999900 + i, 1002, archetype, test_state,
                    1, 100 + i, 100 + i, 0.8, 0.5, 'pending', 5)
                
                request_ids.append(request_id)
            
            logger.info(f"✓ Inserted {batch_size} test requests")
            
            # Wait for all responses
            max_wait = 10.0
            start_time = time.time()
            
            while time.time() - start_time < max_wait:
                count = await conn.fetchval("""
                    SELECT COUNT(*)
                    FROM ai_responses
                    WHERE request_id = ANY($1::bigint[])
                """, request_ids)
                
                if count == batch_size:
                    elapsed = time.time() - start_time
                    logger.info(f"✓ All {batch_size} responses received in {elapsed:.2f}s")
                    
                    # Calculate throughput
                    throughput = batch_size / elapsed
                    logger.info(f"    Throughput: {throughput:.1f} requests/sec")
                    
                    # Cleanup
                    await conn.execute("""
                        DELETE FROM ai_requests WHERE request_id = ANY($1::bigint[])
                    """, request_ids)
                    await conn.execute("""
                        DELETE FROM ai_responses WHERE request_id = ANY($1::bigint[])
                    """, request_ids)
                    
                    return True
                
                await asyncio.sleep(0.1)
            
            logger.error(f"✗ Only received {count}/{batch_size} responses after {max_wait}s")
            
            # Cleanup
            await conn.execute("""
                DELETE FROM ai_requests WHERE request_id = ANY($1::bigint[])
            """, request_ids)
            await conn.execute("""
                DELETE FROM ai_responses WHERE request_id = ANY($1::bigint[])
            """, request_ids)
            
            return False
    
    async def cleanup(self) -> None:
        """Cleanup test connections"""
        if self.pg_pool:
            await self.pg_pool.close()
            logger.info("PostgreSQL connection closed")
        
        if self.redis_client:
            self.redis_client.close()
            logger.info("Redis connection closed")
    
    async def run_all_tests(self) -> bool:
        """Run all tests"""
        logger.info("="*60)
        logger.info("ML Inference Service Integration Test")
        logger.info("="*60)
        logger.info("")
        
        # Setup
        if not await self.setup():
            logger.error("Setup failed, aborting tests")
            return False
        
        logger.info("✓ Setup complete")
        logger.info("")
        
        # Run tests
        tests = [
            ("Database Schema", self.test_database_schema),
            ("Redis Cache", self.test_redis_cache),
            ("Request-Response Flow", self.test_request_response_flow),
            ("Batch Processing", self.test_batch_processing)
        ]
        
        results = []
        for test_name, test_func in tests:
            logger.info(f"Running test: {test_name}")
            try:
                result = await test_func()
                results.append((test_name, result))
                
                if result:
                    logger.info(f"✓ {test_name} PASSED")
                else:
                    logger.error(f"✗ {test_name} FAILED")
            except Exception as e:
                logger.error(f"✗ {test_name} FAILED with exception: {e}")
                results.append((test_name, False))
            
            logger.info("")
        
        # Summary
        logger.info("="*60)
        logger.info("Test Summary:")
        logger.info("="*60)
        
        passed = sum(1 for _, result in results if result)
        total = len(results)
        
        for test_name, result in results:
            status = "✓ PASS" if result else "✗ FAIL"
            logger.info(f"  {status:8} {test_name}")
        
        logger.info("")
        logger.info(f"Results: {passed}/{total} tests passed")
        logger.info("="*60)
        
        return passed == total


async def main():
    """Main test entry point"""
    tester = InferenceServiceTester()
    
    try:
        success = await tester.run_all_tests()
        
        if success:
            logger.info("✓ All tests passed!")
            sys.exit(0)
        else:
            logger.error("✗ Some tests failed")
            sys.exit(1)
    
    finally:
        await tester.cleanup()


if __name__ == '__main__':
    asyncio.run(main())
