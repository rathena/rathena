"""
Request Size Limit Middleware
"""

from typing import Callable
from fastapi import Request
from fastapi.responses import JSONResponse
from starlette.middleware.base import BaseHTTPMiddleware
from loguru import logger

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from ai_service.config import settings


class RequestSizeLimitMiddleware(BaseHTTPMiddleware):
    """
    Middleware to limit request body size
    """
    
    async def dispatch(self, request: Request, call_next: Callable):
        """
        Check request size before processing
        """
        # Get content length from headers
        content_length = request.headers.get("content-length")
        
        if content_length:
            content_length = int(content_length)
            
            if content_length > settings.max_request_size:
                logger.warning(
                    f"Request size {content_length} bytes exceeds limit "
                    f"{settings.max_request_size} bytes from {request.client.host}"
                )
                return JSONResponse(
                    status_code=413,
                    content={
                        "error": "Request too large",
                        "detail": f"Request size {content_length} bytes exceeds maximum {settings.max_request_size} bytes",
                        "max_size": settings.max_request_size
                    }
                )
        
        return await call_next(request)

