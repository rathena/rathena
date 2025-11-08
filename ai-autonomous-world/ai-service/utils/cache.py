"""
Redis/DragonflyDB Caching Utilities for AI Service
Phase 8A: Response caching for frequently requested NPC dialogues and interactions
Provides decorators and utilities for caching AI responses
"""

import hashlib
import json
import logging
from functools import wraps
from typing import Any, Callable, Optional
from database import db

logger = logging.getLogger(__name__)


def generate_cache_key(prefix: str, *args, **kwargs) -> str:
    """
    Generate a deterministic cache key from function arguments
    
    Args:
        prefix: Cache key prefix (e.g., 'dialogue', 'quest')
        *args: Positional arguments
        **kwargs: Keyword arguments
    
    Returns:
        Cache key string
    """
    # Create a deterministic string from arguments
    key_parts = [str(arg) for arg in args]
    key_parts.extend(f"{k}={v}" for k, v in sorted(kwargs.items()))
    key_string = ":".join(key_parts)
    
    # Hash for consistent length
    key_hash = hashlib.md5(key_string.encode()).hexdigest()[:16]
    
    return f"{prefix}:{key_hash}"


def cache_response(prefix: str, ttl: int = 300):
    """
    Decorator to cache function responses in Redis/DragonflyDB
    
    Args:
        prefix: Cache key prefix
        ttl: Time-to-live in seconds (default: 5 minutes)
    
    Usage:
        @cache_response('dialogue', ttl=300)
        async def generate_dialogue(npc_id: str, context: str):
            return await ai_generate(...)
    """
    def decorator(func: Callable) -> Callable:
        @wraps(func)
        async def wrapper(*args, **kwargs):
            # Generate cache key
            cache_key = generate_cache_key(prefix, *args, **kwargs)
            
            try:
                # Try to get from cache
                cached_value = await db.get(cache_key)
                if cached_value:
                    logger.info(f"[CACHE HIT] {cache_key}")
                    # Parse JSON if it's a dict/list
                    try:
                        return json.loads(cached_value)
                    except (json.JSONDecodeError, TypeError):
                        return cached_value
                
                logger.info(f"[CACHE MISS] {cache_key}")
                
                # Call original function
                result = await func(*args, **kwargs)
                
                # Cache the result
                if result is not None:
                    # Serialize to JSON if dict/list
                    if isinstance(result, (dict, list)):
                        cache_value = json.dumps(result)
                    else:
                        cache_value = str(result)
                    
                    await db.setex(cache_key, ttl, cache_value)
                    logger.debug(f"[CACHE SET] {cache_key} (TTL: {ttl}s)")
                
                return result
                
            except Exception as e:
                logger.error(f"[CACHE ERROR] {cache_key}: {e}", exc_info=True)
                # Fall back to calling function without cache
                return await func(*args, **kwargs)
        
        return wrapper
    return decorator


async def invalidate_cache(prefix: str, *args, **kwargs) -> bool:
    """
    Invalidate a specific cache entry
    
    Args:
        prefix: Cache key prefix
        *args: Positional arguments used to generate key
        **kwargs: Keyword arguments used to generate key
    
    Returns:
        True if cache was invalidated, False otherwise
    """
    try:
        cache_key = generate_cache_key(prefix, *args, **kwargs)
        result = await db.delete(cache_key)
        logger.info(f"[CACHE INVALIDATE] {cache_key}")
        return result > 0
    except Exception as e:
        logger.error(f"[CACHE INVALIDATE ERROR] {e}", exc_info=True)
        return False


async def invalidate_cache_pattern(pattern: str) -> int:
    """
    Invalidate all cache entries matching a pattern
    
    Args:
        pattern: Redis key pattern (e.g., 'dialogue:*', 'quest:npc_001:*')
    
    Returns:
        Number of keys deleted
    """
    try:
        # Get all keys matching pattern
        keys = await db.keys(pattern)
        if not keys:
            return 0
        
        # Delete all matching keys
        count = await db.delete(*keys)
        logger.info(f"[CACHE INVALIDATE PATTERN] {pattern} ({count} keys)")
        return count
    except Exception as e:
        logger.error(f"[CACHE INVALIDATE PATTERN ERROR] {e}", exc_info=True)
        return 0


async def get_cache_stats() -> dict:
    """
    Get cache statistics
    
    Returns:
        Dictionary with cache stats (keys count, memory usage, etc.)
    """
    try:
        info = await db.info('stats')
        return {
            'total_keys': await db.dbsize(),
            'hits': info.get('keyspace_hits', 0),
            'misses': info.get('keyspace_misses', 0),
            'hit_rate': info.get('keyspace_hits', 0) / max(info.get('keyspace_hits', 0) + info.get('keyspace_misses', 0), 1)
        }
    except Exception as e:
        logger.error(f"[CACHE STATS ERROR] {e}", exc_info=True)
        return {}

