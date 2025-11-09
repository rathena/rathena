"""
Rate Limiting Middleware using DragonflyDB

Implements token bucket rate limiting with Redis/DragonflyDB backend
for distributed rate limiting across multiple coordinator instances.
"""

import time
from typing import Callable
from fastapi import Request
from fastapi.responses import JSONResponse
from starlette.middleware.base import BaseHTTPMiddleware
from loguru import logger

from config import settings
from database import db_manager


class RateLimitMiddleware(BaseHTTPMiddleware):
    """
    Rate limiting middleware using DragonflyDB for distributed rate limiting
    
    Uses token bucket algorithm with Redis for state storage.
    """
    
    # Endpoints exempt from rate limiting
    EXEMPT_PATHS = [
        "/",
        "/health",
        "/docs",
        "/redoc",
        "/openapi.json",
        "/api/v1/monitoring/health",
        "/api/v1/monitoring/dashboard",
    ]
    
    # Rate limits: (max_requests, window_seconds)
    DEFAULT_LIMIT = (100, 60)  # 100 requests per 60 seconds
    WEBSOCKET_LIMIT = (1000, 60)  # 1000 messages per 60 seconds
    AUTH_LIMIT = (10, 60)  # 10 auth requests per 60 seconds
    
    async def dispatch(self, request: Request, call_next: Callable):
        """
        Check rate limit before processing request
        
        Args:
            request: FastAPI request object
            call_next: Next middleware/handler in chain
            
        Returns:
            Response from next handler or 429 Too Many Requests if rate limit exceeded
        """
        # Skip for exempt paths
        if request.url.path in self.EXEMPT_PATHS:
            return await call_next(request)
        
        # Get client identifier (IP address or authenticated user)
        client_id = request.client.host if request.client else "unknown"
        
        # Determine rate limit based on endpoint
        max_requests, window_seconds = self._get_rate_limit(request.url.path)
        
        # Create rate limit key
        rate_limit_key = f"rate_limit:{client_id}:{request.url.path}"
        
        try:
            # Get Redis client
            redis = await db_manager.get_redis()
            
            # Get current request count
            current_count = await redis.get(rate_limit_key)
            
            if current_count is None:
                # First request in this period
                await redis.setex(rate_limit_key, window_seconds, 1)
                remaining = max_requests - 1
                reset_time = int(time.time()) + window_seconds
            else:
                current_count = int(current_count)
                
                if current_count >= max_requests:
                    # Rate limit exceeded
                    ttl = await redis.ttl(rate_limit_key)
                    reset_time = int(time.time()) + ttl
                    
                    logger.warning(
                        f"Rate limit exceeded for {client_id} on {request.url.path}: "
                        f"{current_count}/{max_requests} requests"
                    )
                    
                    return JSONResponse(
                        status_code=429,
                        content={
                            "error": "rate_limit_exceeded",
                            "message": f"Rate limit exceeded. Try again in {ttl} seconds.",
                            "limit": max_requests,
                            "window_seconds": window_seconds,
                            "reset_time": reset_time,
                        },
                        headers={
                            "X-RateLimit-Limit": str(max_requests),
                            "X-RateLimit-Remaining": "0",
                            "X-RateLimit-Reset": str(reset_time),
                            "Retry-After": str(ttl),
                        },
                    )
                
                # Increment counter
                await redis.incr(rate_limit_key)
                remaining = max_requests - current_count - 1
                ttl = await redis.ttl(rate_limit_key)
                reset_time = int(time.time()) + ttl
            
            # Process request
            response = await call_next(request)
            
            # Add rate limit headers to response
            response.headers["X-RateLimit-Limit"] = str(max_requests)
            response.headers["X-RateLimit-Remaining"] = str(max(0, remaining))
            response.headers["X-RateLimit-Reset"] = str(reset_time)
            
            return response
            
        except Exception as e:
            logger.error(f"Rate limiting error: {e}")
            # On error, allow request to proceed (fail open)
            return await call_next(request)
    
    def _get_rate_limit(self, path: str) -> tuple[int, int]:
        """
        Get rate limit for specific endpoint
        
        Args:
            path: Request path
            
        Returns:
            Tuple of (max_requests, window_seconds)
        """
        if "/signaling/ws" in path:
            return self.WEBSOCKET_LIMIT
        elif "/auth/" in path:
            return self.AUTH_LIMIT
        else:
            return self.DEFAULT_LIMIT

