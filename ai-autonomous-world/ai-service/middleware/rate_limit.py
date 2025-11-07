"""
Rate Limiting Middleware using DragonflyDB
"""

import time
from typing import Callable
from fastapi import Request
from fastapi.responses import JSONResponse
from starlette.middleware.base import BaseHTTPMiddleware
from loguru import logger

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from ai_service.config import settings
from ai_service.database import db


class RateLimitMiddleware(BaseHTTPMiddleware):
    """
    Rate limiting middleware using DragonflyDB for distributed rate limiting
    """
    
    # Endpoints exempt from rate limiting
    EXEMPT_PATHS = [
        "/",
        "/health",
        "/docs",
        "/redoc",
        "/openapi.json",
    ]
    
    async def dispatch(self, request: Request, call_next: Callable):
        """
        Check rate limit before processing request
        """
        # Skip if rate limiting is disabled
        if not settings.rate_limit_enabled:
            return await call_next(request)
        
        # Skip for exempt paths
        if request.url.path in self.EXEMPT_PATHS:
            return await call_next(request)
        
        # Get client identifier (IP address)
        client_ip = request.client.host if request.client else "unknown"
        
        # Create rate limit key
        rate_limit_key = f"rate_limit:{client_ip}"
        
        try:
            # Get current request count
            current_count = await db.client.get(rate_limit_key)
            
            if current_count is None:
                # First request in this period
                await db.client.setex(
                    rate_limit_key,
                    settings.rate_limit_period,
                    1
                )
                logger.debug(f"Rate limit initialized for {client_ip}")
            else:
                # Increment request count
                count = int(current_count)
                
                if count >= settings.rate_limit_requests:
                    # Rate limit exceeded
                    logger.warning(
                        f"Rate limit exceeded for {client_ip}: "
                        f"{count}/{settings.rate_limit_requests} requests"
                    )
                    return JSONResponse(
                        status_code=429,
                        content={
                            "error": "Rate limit exceeded",
                            "detail": f"Maximum {settings.rate_limit_requests} requests per {settings.rate_limit_period} seconds",
                            "retry_after": settings.rate_limit_period
                        },
                        headers={
                            "Retry-After": str(settings.rate_limit_period),
                            "X-RateLimit-Limit": str(settings.rate_limit_requests),
                            "X-RateLimit-Remaining": "0",
                            "X-RateLimit-Reset": str(int(time.time()) + settings.rate_limit_period)
                        }
                    )
                
                # Increment counter
                await db.client.incr(rate_limit_key)
                
                # Add rate limit headers to response
                response = await call_next(request)
                response.headers["X-RateLimit-Limit"] = str(settings.rate_limit_requests)
                response.headers["X-RateLimit-Remaining"] = str(settings.rate_limit_requests - count - 1)
                response.headers["X-RateLimit-Reset"] = str(int(time.time()) + settings.rate_limit_period)
                
                return response
                
        except Exception as e:
            logger.error(f"Rate limit check failed: {e}")
            # On error, allow request to proceed (fail open)
        
        return await call_next(request)

