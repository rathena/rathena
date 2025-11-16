"""
Request/Correlation ID Middleware

Injects a unique correlation/request ID into each request for traceability.
Adds the ID to loguru context and response headers.
"""

import uuid
from fastapi import Request, Response
from starlette.middleware.base import BaseHTTPMiddleware
from loguru import logger
import contextvars

# Context variable for correlation ID
correlation_id_ctx = contextvars.ContextVar("correlation_id", default=None)

def get_correlation_id() -> str:
    return correlation_id_ctx.get() or "unknown"

class RequestIdMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        # Generate a new correlation ID for each request
        correlation_id = request.headers.get("X-Correlation-ID") or str(uuid.uuid4())
        token = correlation_id_ctx.set(correlation_id)
        logger_ctx = logger.bind(correlation_id=correlation_id)
        try:
            # Use logger context for this request
            with logger_ctx.contextualize():
                response: Response = await call_next(request)
                response.headers["X-Correlation-ID"] = correlation_id
                return response
        finally:
            correlation_id_ctx.reset(token)