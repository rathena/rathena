"""
API Key Authentication Middleware
"""

from typing import Callable
from fastapi import Request, HTTPException, status
from fastapi.responses import JSONResponse
from starlette.middleware.base import BaseHTTPMiddleware
from loguru import logger

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from config import settings


class APIKeyMiddleware(BaseHTTPMiddleware):
    """
    Middleware to validate API key for protected endpoints
    """
    
    # Endpoints that don't require API key
    EXEMPT_PATHS = [
        "/",
        "/health",
        "/docs",
        "/redoc",
        "/openapi.json",
    ]
    
    async def dispatch(self, request: Request, call_next: Callable):
        """
        Validate API key if required
        """
        # Skip validation if API key is not required
        if not settings.api_key_required or not settings.api_key:
            return await call_next(request)
        
        # Skip validation for exempt paths
        if request.url.path in self.EXEMPT_PATHS:
            return await call_next(request)
        
        # Get API key from header
        api_key = request.headers.get(settings.api_key_header)
        
        # Validate API key
        if not api_key:
            logger.warning(f"Missing API key for {request.url.path} from {request.client.host}")
            return JSONResponse(
                status_code=status.HTTP_401_UNAUTHORIZED,
                content={
                    "error": "Missing API key",
                    "detail": f"API key required in {settings.api_key_header} header"
                }
            )
        
        if api_key != settings.api_key:
            logger.warning(f"Invalid API key for {request.url.path} from {request.client.host}")
            return JSONResponse(
                status_code=status.HTTP_403_FORBIDDEN,
                content={
                    "error": "Invalid API key",
                    "detail": "The provided API key is not valid"
                }
            )
        
        # API key is valid, proceed with request
        logger.debug(f"API key validated for {request.url.path}")
        return await call_next(request)

