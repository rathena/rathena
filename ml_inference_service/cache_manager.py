"""
Redis Cache Manager - 3-Tier Caching System
L1: Working memory (last 10 states per mob) - 500ms TTL
L2: Action cache (state hash → action) - 10s TTL  
L3: Model output cache (shared) - 10s TTL
"""

import redis
import hashlib
import json
import numpy as np
from typing import Optional, Dict, Any, Tuple
import logging
import time


class CacheManager:
    """
    3-tier Redis cache manager
    Implements L1 (working memory), L2 (action cache), L3 (model output cache)
    """
    
    def __init__(self, redis_config: Dict[str, Any]):
        """
        Initialize cache manager
        
        Args:
            redis_config: Redis connection configuration
        """
        self.logger = logging.getLogger(__name__)
        
        # Connect to Redis
        try:
            self.redis_client = redis.Redis(
                host=redis_config.get('host', 'localhost'),
                port=redis_config.get('port', 6379),
                password=redis_config.get('password'),
                db=redis_config.get('db', 0),
                decode_responses=False,  # Keep binary for numpy arrays
                socket_timeout=redis_config.get('socket_timeout', 5),
                socket_connect_timeout=5,
                retry_on_timeout=True,
                health_check_interval=30
            )
            
            # Test connection
            self.redis_client.ping()
            self.logger.info("Redis connection established")
            
        except Exception as e:
            self.logger.error(f"Failed to connect to Redis: {e}")
            self.redis_client = None
        
        # Cache hit statistics
        self.l1_hits = 0
        self.l2_hits = 0
        self.l3_hits = 0
        self.total_queries = 0
        
        # TTL configuration
        self.l2_ttl = 10  # 10 seconds for action cache
        self.l3_ttl = 10  # 10 seconds for model output cache
    
    def _hash_state(self, state_vector: np.ndarray, precision: int = 2) -> str:
        """
        Hash state vector for cache key
        
        Args:
            state_vector: State vector to hash
            precision: Decimal precision for quantization (default: 2)
        
        Returns:
            MD5 hash hex string
        """
        # Round to reduce precision (improves hit rate)
        rounded = np.round(state_vector, decimals=precision)
        
        # Generate MD5 hash
        state_hash = hashlib.md5(rounded.tobytes()).hexdigest()
        
        return state_hash
    
    def check_l2_cache(self, mob_id: int, state_vector: np.ndarray) -> Optional[Tuple[int, float]]:
        """
        Check L2 action cache
        
        Args:
            mob_id: Monster ID
            state_vector: Current state vector
        
        Returns:
            Tuple of (action_id, confidence) if cache hit, None otherwise
        """
        if self.redis_client is None:
            return None
        
        self.total_queries += 1
        
        try:
            state_hash = self._hash_state(state_vector)
            cache_key = f"ml:l2:{mob_id}:{state_hash}"
            
            result = self.redis_client.get(cache_key)
            
            if result is not None:
                self.l2_hits += 1
                
                # Parse "action_id:confidence" format
                parts = result.decode('utf-8').split(':')
                if len(parts) == 2:
                    action_id = int(parts[0])
                    confidence = float(parts[1])
                    return (action_id, confidence)
        
        except Exception as e:
            self.logger.warning(f"L2 cache error: {e}")
        
        return None
    
    def update_l2_cache(self, mob_id: int, state_vector: np.ndarray, 
                       action: int, confidence: float, ttl: Optional[int] = None) -> None:
        """
        Update L2 action cache
        
        Args:
            mob_id: Monster ID
            state_vector: State vector
            action: Action ID
            confidence: Confidence score
            ttl: Time to live in seconds (default: self.l2_ttl)
        """
        if self.redis_client is None:
            return
        
        try:
            state_hash = self._hash_state(state_vector)
            cache_key = f"ml:l2:{mob_id}:{state_hash}"
            
            # Format: "action_id:confidence"
            value = f"{action}:{confidence:.4f}"
            
            ttl = ttl if ttl is not None else self.l2_ttl
            self.redis_client.setex(cache_key, ttl, value)
        
        except Exception as e:
            self.logger.warning(f"L2 cache update error: {e}")
    
    def check_l3_cache(self, archetype: str, model_type: str, 
                      state_hash: str) -> Optional[np.ndarray]:
        """
        Check L3 model output cache (shared across mobs)
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            state_hash: Hash of state vector
        
        Returns:
            Model output (numpy array) if cache hit, None otherwise
        """
        if self.redis_client is None:
            return None
        
        try:
            cache_key = f"ml:l3:{archetype}:{model_type}:{state_hash}"
            
            result = self.redis_client.hgetall(cache_key)
            
            if result:
                self.l3_hits += 1
                
                # Deserialize numpy array
                output_bytes = result[b'output']
                dtype_str = result[b'dtype'].decode('utf-8')
                shape_str = result[b'shape'].decode('utf-8')
                
                dtype = np.dtype(dtype_str)
                shape = tuple(json.loads(shape_str))
                
                output = np.frombuffer(output_bytes, dtype=dtype).reshape(shape)
                
                return output
        
        except Exception as e:
            self.logger.warning(f"L3 cache error: {e}")
        
        return None
    
    def update_l3_cache(self, archetype: str, model_type: str, 
                       state_hash: str, model_output: np.ndarray,
                       ttl: Optional[int] = None) -> None:
        """
        Update L3 model output cache
        
        Args:
            archetype: Monster archetype
            model_type: Model type
            state_hash: Hash of state vector
            model_output: Model output array to cache
            ttl: Time to live in seconds (default: self.l3_ttl)
        """
        if self.redis_client is None:
            return
        
        try:
            cache_key = f"ml:l3:{archetype}:{model_type}:{state_hash}"
            
            # Serialize numpy array with metadata
            cache_value = {
                'output': model_output.tobytes(),
                'dtype': str(model_output.dtype),
                'shape': json.dumps(model_output.shape),
                'timestamp': time.time()
            }
            
            # Store in Redis hash
            self.redis_client.hset(cache_key, mapping=cache_value)
            
            # Set TTL
            ttl = ttl if ttl is not None else self.l3_ttl
            self.redis_client.expire(cache_key, ttl)
        
        except Exception as e:
            self.logger.warning(f"L3 cache update error: {e}")
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """
        Get cache hit statistics
        
        Returns:
            Statistics dictionary with hit rates and counts
        """
        if self.total_queries == 0:
            return {
                'l2_hit_rate': 0.0,
                'l3_hit_rate': 0.0,
                'total_hit_rate': 0.0,
                'total_queries': 0,
                'l2_hits': 0,
                'l3_hits': 0
            }
        
        total_hits = self.l2_hits + self.l3_hits
        
        return {
            'l2_hit_rate': self.l2_hits / self.total_queries,
            'l3_hit_rate': self.l3_hits / self.total_queries,
            'total_hit_rate': total_hits / self.total_queries,
            'total_queries': self.total_queries,
            'l2_hits': self.l2_hits,
            'l3_hits': self.l3_hits,
            'cache_misses': self.total_queries - total_hits
        }
    
    def get_memory_usage(self) -> Dict[str, Any]:
        """
        Get Redis memory usage statistics
        
        Returns:
            Memory usage dictionary
        """
        if self.redis_client is None:
            return {'error': 'Redis not connected'}
        
        try:
            info = self.redis_client.info('memory')
            
            return {
                'used_memory_bytes': info.get('used_memory', 0),
                'used_memory_human': info.get('used_memory_human', 'N/A'),
                'used_memory_rss_bytes': info.get('used_memory_rss', 0),
                'maxmemory_bytes': info.get('maxmemory', 0),
                'maxmemory_human': info.get('maxmemory_human', 'N/A'),
                'mem_fragmentation_ratio': info.get('mem_fragmentation_ratio', 0.0)
            }
        
        except Exception as e:
            self.logger.warning(f"Failed to get memory usage: {e}")
            return {'error': str(e)}
    
    def clear_cache(self, pattern: Optional[str] = None) -> int:
        """
        Clear cache entries matching pattern
        
        Args:
            pattern: Key pattern to match (e.g., 'ml:l2:*'), or None for all ML caches
        
        Returns:
            Number of keys deleted
        """
        if self.redis_client is None:
            return 0
        
        try:
            if pattern is None:
                pattern = 'ml:*'
            
            # Scan and delete matching keys
            deleted = 0
            cursor = 0
            
            while True:
                cursor, keys = self.redis_client.scan(cursor, match=pattern, count=1000)
                
                if keys:
                    deleted += self.redis_client.delete(*keys)
                
                if cursor == 0:
                    break
            
            self.logger.info(f"Cleared {deleted} cache entries matching '{pattern}'")
            return deleted
        
        except Exception as e:
            self.logger.error(f"Cache clear error: {e}")
            return 0
    
    def health_check(self) -> bool:
        """
        Check Redis health
        
        Returns:
            True if healthy, False otherwise
        """
        if self.redis_client is None:
            return False
        
        try:
            self.redis_client.ping()
            return True
        except Exception as e:
            self.logger.warning(f"Redis health check failed: {e}")
            return False
    
    def close(self) -> None:
        """Close Redis connection"""
        if self.redis_client:
            try:
                self.redis_client.close()
                self.logger.info("Redis connection closed")
            except Exception as e:
                self.logger.warning(f"Error closing Redis connection: {e}")
