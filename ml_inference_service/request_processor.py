"""
PostgreSQL Request Processor
Polls ai_requests table and processes ML inference requests
Writes results to ai_responses table for C++ to consume
"""

import asyncpg
import asyncio
import numpy as np
from typing import List, Dict, Any, Optional
import json
import logging
from datetime import datetime


class RequestProcessor:
    """
    Polls PostgreSQL ai_requests table and processes requests
    Implements efficient batch polling and response writing
    """
    
    def __init__(self, db_config: Dict[str, Any], poll_interval_ms: int = 10):
        """
        Initialize request processor
        
        Args:
            db_config: PostgreSQL connection configuration
            poll_interval_ms: Polling interval in milliseconds
        """
        self.db_config = db_config
        self.poll_interval = poll_interval_ms / 1000.0  # Convert to seconds
        self.logger = logging.getLogger(__name__)
        self.pool: Optional[asyncpg.Pool] = None
        self.running = False
        
        # Statistics
        self.requests_processed = 0
        self.responses_written = 0
        self.errors_count = 0
    
    async def initialize(self) -> None:
        """Initialize database connection pool"""
        try:
            self.pool = await asyncpg.create_pool(
                host=self.db_config.get('host', 'localhost'),
                port=self.db_config.get('port', 5432),
                database=self.db_config.get('database', 'ragnarok_ml'),
                user=self.db_config.get('user', 'ml_user'),
                password=self.db_config.get('password', ''),
                min_size=self.db_config.get('min_pool_size', 10),
                max_size=self.db_config.get('max_pool_size', 20),
                command_timeout=self.db_config.get('command_timeout', 60)
            )
            
            self.logger.info(
                f"PostgreSQL connection pool initialized: "
                f"{self.db_config.get('min_pool_size', 10)}-{self.db_config.get('max_pool_size', 20)} connections"
            )
            
            # Test connection
            async with self.pool.acquire() as conn:
                version = await conn.fetchval("SELECT version()")
                self.logger.info(f"PostgreSQL version: {version}")
        
        except Exception as e:
            self.logger.error(f"Failed to initialize PostgreSQL pool: {e}")
            raise
    
    async def poll_requests(self, max_batch_size: int = 128) -> List[Dict[str, Any]]:
        """
        Poll for pending requests using optimistic locking
        
        Uses SKIP LOCKED to prevent race conditions in multi-worker setup
        
        Args:
            max_batch_size: Maximum number of requests to fetch
        
        Returns:
            List of request dictionaries
        """
        if self.pool is None:
            raise RuntimeError("Database pool not initialized")
        
        async with self.pool.acquire() as conn:
            try:
                # Fetch pending requests with SKIP LOCKED (PostgreSQL 9.5+)
                # This prevents multiple workers from processing same request
                rows = await conn.fetch("""
                    UPDATE ai_requests
                    SET status = 'processing',
                        processed_at = NOW(),
                        processing_worker_id = pg_backend_pid()
                    WHERE request_id IN (
                        SELECT request_id
                        FROM ai_requests
                        WHERE status = 'pending'
                        ORDER BY priority DESC, created_at ASC
                        LIMIT $1
                        FOR UPDATE SKIP LOCKED
                    )
                    RETURNING 
                        request_id,
                        monster_id,
                        mob_id,
                        archetype,
                        state_vector,
                        spatial_grid,
                        combat_history,
                        pack_leader_id,
                        pack_member_ids,
                        map_id,
                        position_x,
                        position_y,
                        hp_ratio,
                        sp_ratio,
                        created_at,
                        priority
                """, max_batch_size)
                
                if not rows:
                    return []
                
                # Convert to dictionaries
                requests = []
                for row in rows:
                    request = {
                        'request_id': row['request_id'],
                        'monster_id': row['monster_id'],
                        'mob_id': row['mob_id'],
                        'archetype': row['archetype'],
                        'state_vector': list(row['state_vector']),  # PostgreSQL array to list
                        'spatial_grid': list(row['spatial_grid']) if row['spatial_grid'] else None,
                        'combat_history': row['combat_history'],  # Already JSONB
                        'pack_leader_id': row['pack_leader_id'],
                        'pack_member_ids': list(row['pack_member_ids']) if row['pack_member_ids'] else [],
                        'map_id': row['map_id'],
                        'position_x': row['position_x'],
                        'position_y': row['position_y'],
                        'hp_ratio': float(row['hp_ratio']),
                        'sp_ratio': float(row['sp_ratio']),
                        'created_at': row['created_at'],
                        'priority': row['priority']
                    }
                    requests.append(request)
                
                self.requests_processed += len(requests)
                
                return requests
            
            except Exception as e:
                self.logger.error(f"Error polling requests: {e}")
                self.errors_count += 1
                return []
    
    async def write_responses(self, responses: List[Dict[str, Any]]) -> None:
        """
        Write inference results to ai_responses table (batch insert)
        
        Args:
            responses: List of response dictionaries with keys:
                - request_id: Request ID
                - monster_id: Monster ID
                - action_type: Action type string
                - action_id: Action ID integer
                - action_params: Action parameters (dict)
                - model_outputs: Model outputs (dict)
                - confidence: Confidence score (0.0-1.0)
                - fusion_weights: Model fusion weights (dict)
                - coordination_action: Coordination action (optional)
                - target_monster_id: Target monster (optional)
                - inference_latency_ms: Inference latency
                - cache_used: Whether cache was used
                - fallback_level: Fallback level (1-5)
        """
        if not responses:
            return
        
        if self.pool is None:
            raise RuntimeError("Database pool not initialized")
        
        async with self.pool.acquire() as conn:
            try:
                # Prepare batch insert
                await conn.executemany("""
                    INSERT INTO ai_responses (
                        request_id,
                        monster_id,
                        action_type,
                        action_id,
                        action_params,
                        model_outputs,
                        confidence,
                        fusion_weights,
                        coordination_action,
                        target_monster_id,
                        inference_latency_ms,
                        cache_used,
                        fallback_level
                    ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
                """, [
                    (
                        r['request_id'],
                        r['monster_id'],
                        r.get('action_type', 'combat'),
                        r['action_id'],
                        json.dumps(r.get('action_params', {})),
                        json.dumps(r.get('model_outputs', {})),
                        r['confidence'],
                        json.dumps(r.get('fusion_weights', {})),
                        r.get('coordination_action'),
                        r.get('target_monster_id'),
                        r['inference_latency_ms'],
                        r.get('cache_used', False),
                        r.get('fallback_level', 1)
                    )
                    for r in responses
                ])
                
                # Update request status to completed
                request_ids = [r['request_id'] for r in responses]
                await conn.execute("""
                    UPDATE ai_requests
                    SET status = 'completed'
                    WHERE request_id = ANY($1::bigint[])
                """, request_ids)
                
                self.responses_written += len(responses)
            
            except Exception as e:
                self.logger.error(f"Error writing responses: {e}")
                self.errors_count += 1
                raise
    
    async def mark_requests_failed(self, request_ids: List[int], error_message: str) -> None:
        """
        Mark requests as failed
        
        Args:
            request_ids: List of request IDs that failed
            error_message: Error message to log
        """
        if not request_ids or self.pool is None:
            return
        
        async with self.pool.acquire() as conn:
            try:
                await conn.execute("""
                    UPDATE ai_requests
                    SET status = 'failed'
                    WHERE request_id = ANY($1::bigint[])
                """, request_ids)
                
                self.logger.warning(f"Marked {len(request_ids)} requests as failed: {error_message}")
            
            except Exception as e:
                self.logger.error(f"Error marking requests failed: {e}")
    
    async def cleanup_old_requests(self, hours: int = 24) -> int:
        """
        Delete old processed requests to prevent table bloat
        
        Args:
            hours: Delete requests older than this many hours
        
        Returns:
            Number of requests deleted
        """
        if self.pool is None:
            return 0
        
        async with self.pool.acquire() as conn:
            try:
                result = await conn.execute("""
                    DELETE FROM ai_requests
                    WHERE status IN ('completed', 'failed', 'timeout')
                        AND created_at < NOW() - make_interval(hours => $1)
                """, hours)
                
                # Parse "DELETE N" result
                deleted = int(result.split()[-1]) if result.startswith('DELETE') else 0
                
                if deleted > 0:
                    self.logger.info(f"Cleaned up {deleted} old requests")
                
                return deleted
            
            except Exception as e:
                self.logger.error(f"Error cleaning up requests: {e}")
                return 0
    
    async def cleanup_old_responses(self, hours: int = 24) -> int:
        """
        Delete old responses that have been read by C++
        
        Args:
            hours: Delete responses older than this many hours
        
        Returns:
            Number of responses deleted
        """
        if self.pool is None:
            return 0
        
        async with self.pool.acquire() as conn:
            try:
                result = await conn.execute("""
                    DELETE FROM ai_responses
                    WHERE read_at IS NOT NULL
                        AND created_at < NOW() - make_interval(hours => $1)
                """, hours)
                
                deleted = int(result.split()[-1]) if result.startswith('DELETE') else 0
                
                if deleted > 0:
                    self.logger.info(f"Cleaned up {deleted} old responses")
                
                return deleted
            
            except Exception as e:
                self.logger.error(f"Error cleaning up responses: {e}")
                return 0
    
    async def get_pending_count(self) -> int:
        """
        Get count of pending requests
        
        Returns:
            Number of pending requests
        """
        if self.pool is None:
            return 0
        
        async with self.pool.acquire() as conn:
            try:
                count = await conn.fetchval("""
                    SELECT COUNT(*) 
                    FROM ai_requests 
                    WHERE status = 'pending'
                """)
                return count
            
            except Exception as e:
                self.logger.error(f"Error getting pending count: {e}")
                return 0
    
    async def health_check(self) -> Dict[str, Any]:
        """
        Perform database health check
        
        Returns:
            Health status dictionary
        """
        if self.pool is None:
            return {
                'status': 'critical',
                'message': 'Pool not initialized',
                'connected': False
            }
        
        try:
            async with self.pool.acquire() as conn:
                # Test query
                result = await conn.fetchval("SELECT 1")
                
                # Get pool status
                pool_size = self.pool.get_size()
                pool_free = self.pool.get_idle_size()
                
                # Get pending requests
                pending = await self.get_pending_count()
                
                return {
                    'status': 'healthy',
                    'message': 'Database operational',
                    'connected': True,
                    'pool_size': pool_size,
                    'pool_free': pool_free,
                    'pool_in_use': pool_size - pool_free,
                    'pending_requests': pending,
                    'requests_processed': self.requests_processed,
                    'responses_written': self.responses_written,
                    'errors': self.errors_count
                }
        
        except Exception as e:
            self.logger.error(f"Database health check failed: {e}")
            return {
                'status': 'critical',
                'message': str(e),
                'connected': False,
                'errors': self.errors_count
            }
    
    async def close(self) -> None:
        """Close database connection pool"""
        if self.pool:
            await self.pool.close()
            self.logger.info("PostgreSQL connection pool closed")
    
    def get_statistics(self) -> Dict[str, Any]:
        """
        Get processor statistics
        
        Returns:
            Statistics dictionary
        """
        return {
            'requests_processed': self.requests_processed,
            'responses_written': self.responses_written,
            'errors': self.errors_count,
            'poll_interval_ms': self.poll_interval * 1000
        }
