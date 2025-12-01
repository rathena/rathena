"""
Common decorators for error handling and caching
"""

import functools
import hashlib
import json
from typing import Callable, Any, Optional
from fastapi import HTTPException, status
from loguru import logger

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from database import db
from config import settings


def handle_errors(
    default_status: int = status.HTTP_500_INTERNAL_SERVER_ERROR,
    default_message: str = "An error occurred"
):
    """
    Decorator for consistent error handling across endpoints
    
    Args:
        default_status: Default HTTP status code for errors
        default_message: Default error message
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            try:
                return await func(*args, **kwargs)
            
            except HTTPException:
                # Re-raise HTTP exceptions as-is
                raise
            
            except ConnectionError as e:
                logger.error(f"Connection error in {func.__name__}: {e}")
                raise HTTPException(
                    status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                    detail=f"Service temporarily unavailable: {str(e)}"
                )
            
            except TimeoutError as e:
                logger.error(f"Timeout error in {func.__name__}: {e}")
                raise HTTPException(
                    status_code=status.HTTP_504_GATEWAY_TIMEOUT,
                    detail=f"Request timeout: {str(e)}"
                )
            
            except ValueError as e:
                logger.error(f"Validation error in {func.__name__}: {e}")
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Invalid input: {str(e)}"
                )
            
            except Exception as e:
                logger.error(f"Unexpected error in {func.__name__}: {e}", exc_info=True)
                raise HTTPException(
                    status_code=default_status,
                    detail=default_message
                )
        
        return wrapper
    return decorator


def cache_result(
    ttl: int = 3600,
    key_prefix: str = "cache",
    include_args: bool = True
):
    """
    Decorator to cache function results in DragonflyDB
    
    Args:
        ttl: Time to live in seconds (default: 1 hour)
        key_prefix: Prefix for cache key
        include_args: Whether to include function arguments in cache key
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            # Generate cache key
            if include_args:
                # Create hash of arguments
                args_str = json.dumps({
                    "args": [str(arg) for arg in args],
                    "kwargs": {k: str(v) for k, v in kwargs.items()}
                }, sort_keys=True)
                args_hash = hashlib.md5(args_str.encode()).hexdigest()
                cache_key = f"{key_prefix}:{func.__name__}:{args_hash}"
            else:
                cache_key = f"{key_prefix}:{func.__name__}"
            
            try:
                # Try to get cached result
                cached = await db.client.get(cache_key)
                if cached:
                    logger.debug(f"Cache hit for {cache_key}")
                    return json.loads(cached)
            except Exception as e:
                logger.warning(f"Cache retrieval failed for {cache_key}: {e}")
            
            # Execute function
            result = await func(*args, **kwargs)
            
            # Cache result
            try:
                await db.client.setex(
                    cache_key,
                    ttl,
                    json.dumps(result)
                )
                logger.debug(f"Cached result for {cache_key} (TTL: {ttl}s)")
            except Exception as e:
                logger.warning(f"Cache storage failed for {cache_key}: {e}")
            
            return result
        
        return wrapper
    return decorator


def retry_on_error(
    max_retries: int = 3,
    delay: float = 1.0,
    backoff: float = 2.0,
    exceptions: tuple = (Exception,)
):
    """
    Decorator to retry function on specific exceptions
    
    Args:
        max_retries: Maximum number of retry attempts
        delay: Initial delay between retries in seconds
        backoff: Multiplier for delay after each retry
        exceptions: Tuple of exceptions to catch and retry
    """
    def decorator(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapper(*args, **kwargs):
            current_delay = delay
            last_exception = None
            
            for attempt in range(max_retries + 1):
                try:
                    return await func(*args, **kwargs)
                except exceptions as e:
                    last_exception = e
                    if attempt < max_retries:
                        logger.warning(
                            f"Attempt {attempt + 1}/{max_retries} failed for {func.__name__}: {e}. "
                            f"Retrying in {current_delay}s..."
                        )
                        import asyncio
                        await asyncio.sleep(current_delay)
                        current_delay *= backoff
                    else:
                        logger.error(f"All {max_retries} retries failed for {func.__name__}")
            
            # All retries exhausted
            raise last_exception
        
        return wrapper
    return decorator

