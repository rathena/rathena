#!/usr/bin/env python3
"""
AI IPC Worker Service
Polls ai_requests table from MariaDB and processes AI requests
Writes responses back to ai_responses table

Architecture: NPC → MariaDB → Python Worker → AI Service → MariaDB → NPC
"""

import asyncio
import aiomysql
import aiohttp
import json
import logging
import os
import signal
import sys
import time
from datetime import datetime
from typing import Optional, Dict, Any, List
from dataclasses import dataclass

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(name)s: %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout),
        logging.FileHandler('/opt/ml_monster_ai/logs/ai_ipc_worker.log')
    ]
)
logger = logging.getLogger('ai_ipc_worker')


@dataclass
class WorkerConfig:
    """Configuration for AI IPC Worker"""
    # Database settings
    db_host: str = os.getenv('DB_HOST', '192.168.0.100')
    db_port: int = int(os.getenv('DB_PORT', '3306'))
    db_user: str = os.getenv('DB_USER', 'ragnarok')
    db_password: str = os.getenv('DB_PASSWORD', '')
    db_name: str = os.getenv('DB_NAME', 'ragnarok')
    
    # Polling settings
    poll_interval_ms: int = int(os.getenv('POLL_INTERVAL_MS', '100'))
    batch_size: int = int(os.getenv('BATCH_SIZE', '10'))
    max_processing_time: int = int(os.getenv('MAX_PROCESSING_TIME', '5000'))
    
    # Cleanup settings
    cleanup_interval: int = int(os.getenv('CLEANUP_INTERVAL', '60'))
    timeout_seconds: int = int(os.getenv('TIMEOUT_SECONDS', '30'))
    
    # AI Service settings
    ai_service_base_url: str = os.getenv('AI_SERVICE_BASE_URL', 'http://127.0.0.1:8000')
    ai_service_timeout: int = int(os.getenv('AI_SERVICE_TIMEOUT', '10'))
    
    # Pool settings
    min_pool_size: int = 5
    max_pool_size: int = 20


class AIIPCWorker:
    """
    Main worker class that polls database for AI requests and processes them
    """
    
    def __init__(self, config: WorkerConfig):
        self.config = config
        self.pool: Optional[aiomysql.Pool] = None
        self.http_session: Optional[aiohttp.ClientSession] = None
        self.running = False
        self.stats = {
            'requests_processed': 0,
            'requests_failed': 0,
            'requests_timeout': 0,
            'total_processing_time_ms': 0,
            'start_time': None
        }
        
        # Setup signal handlers for graceful shutdown
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals gracefully"""
        logger.info(f"Received signal {signum}, initiating graceful shutdown...")
        self.running = False
    
    async def initialize(self):
        """Initialize database connection pool and HTTP session"""
        logger.info("Initializing AI IPC Worker...")
        
        # Create database connection pool
        try:
            self.pool = await aiomysql.create_pool(
                host=self.config.db_host,
                port=self.config.db_port,
                user=self.config.db_user,
                password=self.config.db_password,
                db=self.config.db_name,
                minsize=self.config.min_pool_size,
                maxsize=self.config.max_pool_size,
                autocommit=False,
                charset='utf8mb4'
            )
            logger.info(f"Database pool created: {self.config.db_host}:{self.config.db_port}/{self.config.db_name}")
        except Exception as e:
            logger.error(f"Failed to create database pool: {e}")
            raise
        
        # Create HTTP session
        timeout = aiohttp.ClientTimeout(total=self.config.ai_service_timeout)
        self.http_session = aiohttp.ClientSession(timeout=timeout)
        logger.info(f"HTTP session created for AI service: {self.config.ai_service_base_url}")
        
        self.stats['start_time'] = datetime.now()
        self.running = True
        logger.info("AI IPC Worker initialized successfully")
    
    async def cleanup(self):
        """Cleanup resources"""
        logger.info("Cleaning up resources...")
        
        if self.http_session:
            await self.http_session.close()
        
        if self.pool:
            self.pool.close()
            await self.pool.wait_closed()
        
        logger.info("Cleanup complete")
    
    async def poll_and_process(self) -> int:
        """
        Poll for pending requests and process them
        Returns: Number of requests processed
        """
        async with self.pool.acquire() as conn:
            async with conn.cursor(aiomysql.DictCursor) as cursor:
                try:
                    # Use FOR UPDATE SKIP LOCKED for concurrent safety
                    await cursor.execute("""
                        SELECT id, request_type, endpoint, request_data, 
                               source_npc, source_map, created_at
                        FROM ai_requests
                        WHERE status = 'pending'
                          AND expires_at > NOW()
                        ORDER BY priority ASC, created_at ASC
                        LIMIT %s
                        FOR UPDATE SKIP LOCKED
                    """, (self.config.batch_size,))
                    
                    requests = await cursor.fetchall()
                    
                    if not requests:
                        return 0
                    
                    # Mark as processing
                    request_ids = [r['id'] for r in requests]
                    placeholders = ','.join(['%s'] * len(request_ids))
                    await cursor.execute(f"""
                        UPDATE ai_requests 
                        SET status = 'processing'
                        WHERE id IN ({placeholders})
                    """, request_ids)
                    await conn.commit()
                    
                    logger.info(f"Fetched {len(requests)} pending requests")
                    
                    # Process each request
                    tasks = [self.process_request(req) for req in requests]
                    results = await asyncio.gather(*tasks, return_exceptions=True)
                    
                    # Count successes and failures
                    successes = sum(1 for r in results if r is True)
                    failures = sum(1 for r in results if r is not True)
                    
                    logger.info(f"Processed batch: {successes} succeeded, {failures} failed")
                    return len(requests)
                    
                except Exception as e:
                    logger.error(f"Error polling requests: {e}", exc_info=True)
                    await conn.rollback()
                    return 0
    
    async def process_request(self, request: Dict[str, Any]) -> bool:
        """
        Process a single AI request
        Returns: True on success, False on failure
        """
        request_id = request['id']
        request_type = request['request_type']
        endpoint = request['endpoint']
        request_data = request['request_data']
        
        start_time = time.time()
        
        try:
            logger.debug(f"Processing request #{request_id}: type={request_type}, endpoint={endpoint}")
            
            # Route to appropriate handler
            response_data = await self.route_to_service(
                request_type, 
                endpoint, 
                request_data
            )
            
            processing_time = int((time.time() - start_time) * 1000)
            
            # Write successful response
            await self.write_response(
                request_id=request_id,
                response_data=json.dumps(response_data) if isinstance(response_data, dict) else response_data,
                error_message=None,
                processing_time=processing_time
            )
            
            # Update statistics
            self.stats['requests_processed'] += 1
            self.stats['total_processing_time_ms'] += processing_time
            
            logger.info(f"Request #{request_id} completed successfully in {processing_time}ms")
            return True
            
        except asyncio.TimeoutError:
            logger.warning(f"Request #{request_id} timed out")
            await self.handle_timeout(request_id)
            self.stats['requests_timeout'] += 1
            return False
            
        except Exception as e:
            logger.error(f"Request #{request_id} failed: {e}", exc_info=True)
            processing_time = int((time.time() - start_time) * 1000)
            
            await self.write_response(
                request_id=request_id,
                response_data=json.dumps({'error': str(e)}),
                error_message=str(e),
                processing_time=processing_time
            )
            
            # Mark request as failed
            async with self.pool.acquire() as conn:
                async with conn.cursor() as cursor:
                    await cursor.execute("""
                        UPDATE ai_requests 
                        SET status = 'failed', processed_at = NOW()
                        WHERE id = %s
                    """, (request_id,))
                    await conn.commit()
            
            self.stats['requests_failed'] += 1
            return False
    
    async def route_to_service(self, request_type: str, endpoint: str, data: str) -> Dict[str, Any]:
        """
        Route request to appropriate AI service endpoint
        """
        # Parse request data
        try:
            request_json = json.loads(data) if data else {}
        except json.JSONDecodeError:
            logger.warning(f"Invalid JSON in request data: {data[:100]}")
            request_json = {}
        
        # Build full URL
        url = f"{self.config.ai_service_base_url}{endpoint}"
        
        logger.debug(f"Routing {request_type} to {url}")
        
        # Determine HTTP method based on request type
        if request_type in ['http_get', 'health_check']:
            async with self.http_session.get(url) as response:
                response.raise_for_status()
                return await response.json()
        
        elif request_type in ['http_post', 'dialogue', 'decision', 'ai_npc_dialogue', 
                              'ai_npc_decide_action', 'ai_request_async']:
            async with self.http_session.post(url, json=request_json) as response:
                response.raise_for_status()
                return await response.json()
        
        else:
            # Default: POST with JSON body
            async with self.http_session.post(url, json=request_json) as response:
                response.raise_for_status()
                return await response.json()
    
    async def write_response(self, request_id: int, response_data: str, 
                           error_message: Optional[str], processing_time: int):
        """Write response to ai_responses table"""
        async with self.pool.acquire() as conn:
            async with conn.cursor() as cursor:
                try:
                    # Insert response
                    await cursor.execute("""
                        INSERT INTO ai_responses 
                        (request_id, response_data, error_message, processing_time_ms)
                        VALUES (%s, %s, %s, %s)
                    """, (request_id, response_data, error_message, processing_time))
                    
                    # Update request status
                    status = 'completed' if error_message is None else 'failed'
                    await cursor.execute("""
                        UPDATE ai_requests 
                        SET status = %s, processed_at = NOW()
                        WHERE id = %s
                    """, (status, request_id))
                    
                    # Log to audit trail
                    await cursor.execute("""
                        INSERT INTO ai_request_log 
                        (request_id, event_type, event_data)
                        VALUES (%s, %s, %s)
                    """, (request_id, status, json.dumps({
                        'processing_time_ms': processing_time,
                        'has_error': error_message is not None
                    })))
                    
                    await conn.commit()
                    logger.debug(f"Response written for request #{request_id}")
                    
                except Exception as e:
                    logger.error(f"Failed to write response for request #{request_id}: {e}")
                    await conn.rollback()
                    raise
    
    async def handle_timeout(self, request_id: int):
        """Handle timeout for a request"""
        async with self.pool.acquire() as conn:
            async with conn.cursor() as cursor:
                try:
                    await cursor.execute("""
                        UPDATE ai_requests 
                        SET status = 'timeout', processed_at = NOW()
                        WHERE id = %s
                    """, (request_id,))
                    
                    await cursor.execute("""
                        INSERT INTO ai_request_log 
                        (request_id, event_type, event_data)
                        VALUES (%s, 'timeout', %s)
                    """, (request_id, json.dumps({'reason': 'processing_timeout'})))
                    
                    await conn.commit()
                except Exception as e:
                    logger.error(f"Failed to handle timeout for request #{request_id}: {e}")
                    await conn.rollback()
    
    async def cleanup_expired_requests(self):
        """Mark expired pending requests as timeout"""
        async with self.pool.acquire() as conn:
            async with conn.cursor() as cursor:
                try:
                    await cursor.execute("""
                        UPDATE ai_requests 
                        SET status = 'timeout', processed_at = NOW()
                        WHERE status = 'pending' 
                          AND expires_at < NOW()
                    """)
                    
                    affected = cursor.rowcount
                    if affected > 0:
                        logger.info(f"Marked {affected} expired requests as timeout")
                    
                    await conn.commit()
                except Exception as e:
                    logger.error(f"Failed to cleanup expired requests: {e}")
                    await conn.rollback()
    
    async def print_stats(self):
        """Print worker statistics"""
        uptime = (datetime.now() - self.stats['start_time']).total_seconds()
        avg_time = (self.stats['total_processing_time_ms'] / self.stats['requests_processed'] 
                   if self.stats['requests_processed'] > 0 else 0)
        
        logger.info(f"=== AI IPC Worker Statistics ===")
        logger.info(f"Uptime: {uptime:.1f}s")
        logger.info(f"Requests Processed: {self.stats['requests_processed']}")
        logger.info(f"Requests Failed: {self.stats['requests_failed']}")
        logger.info(f"Requests Timeout: {self.stats['requests_timeout']}")
        logger.info(f"Average Processing Time: {avg_time:.1f}ms")
        logger.info(f"=================================")
    
    async def run_forever(self):
        """Main event loop"""
        logger.info("Starting AI IPC Worker main loop...")
        
        last_cleanup = time.time()
        last_stats = time.time()
        
        while self.running:
            try:
                # Poll and process requests
                processed = await self.poll_and_process()
                
                # Cleanup expired requests periodically
                now = time.time()
                if now - last_cleanup > self.config.cleanup_interval:
                    await self.cleanup_expired_requests()
                    last_cleanup = now
                
                # Print stats every 5 minutes
                if now - last_stats > 300:
                    await self.print_stats()
                    last_stats = now
                
                # Sleep if no requests were processed
                if processed == 0:
                    await asyncio.sleep(self.config.poll_interval_ms / 1000.0)
                
            except Exception as e:
                logger.error(f"Error in main loop: {e}", exc_info=True)
                await asyncio.sleep(1.0)  # Back off on error
        
        logger.info("Main loop stopped")


async def main():
    """Main entry point"""
    logger.info("=== AI IPC Worker Starting ===")
    
    # Load configuration
    config = WorkerConfig()
    logger.info(f"Configuration loaded:")
    logger.info(f"  Database: {config.db_host}:{config.db_port}/{config.db_name}")
    logger.info(f"  AI Service: {config.ai_service_base_url}")
    logger.info(f"  Poll Interval: {config.poll_interval_ms}ms")
    logger.info(f"  Batch Size: {config.batch_size}")
    
    # Create and run worker
    worker = AIIPCWorker(config)
    
    try:
        await worker.initialize()
        await worker.run_forever()
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt received")
    except Exception as e:
        logger.error(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)
    finally:
        await worker.cleanup()
        await worker.print_stats()
        logger.info("=== AI IPC Worker Stopped ===")


if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Shutting down...")
        sys.exit(0)
