"""
API Key Validation Middleware

Optional middleware for validating X-API-Key header on incoming requests.

NOTE: API key validation is OPTIONAL and disabled by default.
      Enable via API_KEY_VALIDATION_ENABLED=true environment variable.
"""

from typing import Callable
from fastapi import Request, HTTPException, status
from fastapi.responses import JSONResponse
from loguru import logger

from config import settings


async def api_key_middleware(request: Request, call_next: Callable):
    """
    Middleware to validate API key in X-API-Key header
    
    This middleware is optional and only enforces API key validation if:
    - API_KEY_VALIDATION_ENABLED is set to true in configuration
    - The request is not to excluded paths (health, docs, etc.)
    
    Args:
        request: FastAPI request object
        call_next: Next middleware/handler in chain
    
    Returns:
        Response from next handler or 401 Unauthorized if API key is invalid
    """
    # Skip validation if disabled
    if not settings.security.api_key_validation_enabled:
        return await call_next(request)
    
    # Excluded paths that don't require API key
    excluded_paths = [
        "/",
        "/health",
        "/docs",
        "/redoc",
        "/openapi.json",
        "/api/v1/monitoring/health",
        "/api/v1/monitoring/dashboard",
    ]
    
    # Check if path is excluded
    if request.url.path in excluded_paths:
        return await call_next(request)
    
    # Get API key from header
    api_key = request.headers.get(settings.security.api_key_header)
    
    # Validate API key
    if not api_key:
        logger.warning(f"Missing API key for request to {request.url.path}")
        return JSONResponse(
            status_code=status.HTTP_401_UNAUTHORIZED,
            content={
                "detail": f"Missing {settings.security.api_key_header} header",
                "error": "unauthorized",
            },
        )
    
    if api_key != settings.security.coordinator_api_key:
        logger.warning(f"Invalid API key for request to {request.url.path}")
        return JSONResponse(
            status_code=status.HTTP_401_UNAUTHORIZED,
            content={
                "detail": "Invalid API key",
                "error": "unauthorized",
            },
        )
    
    # API key is valid, proceed with request
    logger.debug(f"API key validated for request to {request.url.path}")
    return await call_next(request)

