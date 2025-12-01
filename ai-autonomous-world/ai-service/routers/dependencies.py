"""
Shared Dependencies for FastAPI Routers

This module provides reusable dependency injection functions for all routers,
including agent access, authentication, rate limiting, and request tracking.
"""

import uuid
import logging
from typing import Annotated, Optional
from functools import lru_cache

from fastapi import Depends, Header, HTTPException, Request, status
from fastapi.security import APIKeyHeader

from config import settings
from agents import AgentOrchestrator
from llm.factory import LLMProviderFactory

logger = logging.getLogger(__name__)

# ============================================================================
# AUTHENTICATION
# ============================================================================

api_key_header = APIKeyHeader(name="X-API-Key", auto_error=False)


async def verify_api_key(
    api_key: Annotated[Optional[str], Depends(api_key_header)] = None
) -> str:
    """
    Verify API key authentication.
    
    Raises:
        HTTPException: 401 if API key is required but missing/invalid
    
    Returns:
        str: Validated API key
    """
    if not settings.api_key_required:
        return "mock-api-key"  # Development mode
    
    if not api_key:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="API key required. Provide X-API-Key header.",
            headers={"WWW-Authenticate": "ApiKey"}
        )
    
    if api_key != settings.api_key:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid API key",
            headers={"WWW-Authenticate": "ApiKey"}
        )
    
    return api_key


# ============================================================================
# REQUEST TRACKING
# ============================================================================

async def get_correlation_id(
    x_correlation_id: Annotated[Optional[str], Header()] = None
) -> str:
    """
    Get or generate correlation ID for request tracking.
    
    Args:
        x_correlation_id: Optional correlation ID from header
    
    Returns:
        str: Correlation ID for this request
    """
    if x_correlation_id:
        return x_correlation_id
    return f"req-{uuid.uuid4().hex[:16]}"


# ============================================================================
# AGENT DEPENDENCIES
# ============================================================================

@lru_cache()
def get_llm_factory() -> LLMProviderFactory:
    """
    Get singleton LLM provider factory.
    
    Returns:
        LLMProviderFactory: Factory for creating LLM providers
    """
    return LLMProviderFactory()


@lru_cache()
def get_agent_orchestrator() -> AgentOrchestrator:
    """
    Get singleton agent orchestrator instance.
    
    Returns:
        AgentOrchestrator: Main orchestrator for all agents
    """
    llm_factory = get_llm_factory()
    return AgentOrchestrator(llm_factory=llm_factory)


async def get_orchestrator(
    _: Annotated[str, Depends(verify_api_key)]
) -> AgentOrchestrator:
    """
    Dependency for getting orchestrator with authentication.
    
    Args:
        _: API key (validated)
    
    Returns:
        AgentOrchestrator: Authenticated orchestrator instance
    """
    return get_agent_orchestrator()


# ============================================================================
# RATE LIMITING
# ============================================================================

class RateLimiter:
    """Simple in-memory rate limiter (use Redis in production)."""
    
    def __init__(self):
        self.requests = {}
    
    async def check_rate_limit(self, client_id: str, limit: int = 60) -> bool:
        """
        Check if client has exceeded rate limit.
        
        Args:
            client_id: Client identifier (IP or user ID)
            limit: Requests per minute
        
        Returns:
            bool: True if within limit
        
        Raises:
            HTTPException: 429 if rate limit exceeded
        """
        # TODO: Implement with Redis and sliding window
        # For now, always allow in development
        if settings.is_development:
            return True
        
        # Simple counter-based limiting
        current_count = self.requests.get(client_id, 0)
        if current_count >= limit:
            raise HTTPException(
                status_code=status.HTTP_429_TOO_MANY_REQUESTS,
                detail=f"Rate limit exceeded: {limit} requests per minute"
            )
        
        self.requests[client_id] = current_count + 1
        return True


rate_limiter = RateLimiter()


async def check_rate_limit(request: Request) -> bool:
    """
    Rate limiting dependency.
    
    Args:
        request: FastAPI request object
    
    Returns:
        bool: True if within rate limit
    
    Raises:
        HTTPException: 429 if rate limit exceeded
    """
    if not settings.rate_limit_enabled:
        return True
    
    client_id = request.client.host if request.client else "unknown"
    return await rate_limiter.check_rate_limit(
        client_id,
        settings.rate_limit_requests_per_minute
    )


# ============================================================================
# COMMON RESPONSE MODELS
# ============================================================================

from datetime import datetime
from typing import Any, Generic, TypeVar
from pydantic import BaseModel, Field

T = TypeVar('T')


class StandardResponse(BaseModel, Generic[T]):
    """Standard API response wrapper."""
    
    success: bool = Field(..., description="Request success status")
    data: T = Field(..., description="Response data")
    message: str = Field(default="", description="Human-readable message")
    timestamp: datetime = Field(
        default_factory=datetime.utcnow,
        description="Response timestamp (UTC)"
    )
    request_id: str = Field(..., description="Request correlation ID")
    
    class Config:
        json_schema_extra = {
            "example": {
                "success": True,
                "data": {},
                "message": "Operation completed successfully",
                "timestamp": "2024-01-01T00:00:00Z",
                "request_id": "req-abc123"
            }
        }


class ErrorDetail(BaseModel):
    """Error detail model."""
    
    code: str = Field(..., description="Error code")
    message: str = Field(..., description="Error message")
    field: Optional[str] = Field(None, description="Field causing error")


class ErrorResponse(BaseModel):
    """Standard error response."""
    
    success: bool = Field(default=False)
    error: ErrorDetail = Field(..., description="Error details")
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    request_id: str = Field(..., description="Request correlation ID")


# ============================================================================
# UTILITIES
# ============================================================================

def create_success_response(
    data: Any,
    message: str = "Success",
    request_id: str = None
) -> dict:
    """
    Create standardized success response.
    
    Args:
        data: Response data
        message: Success message
        request_id: Correlation ID
    
    Returns:
        dict: Standardized response
    """
    return {
        "success": True,
        "data": data,
        "message": message,
        "timestamp": datetime.utcnow().isoformat(),
        "request_id": request_id or f"req-{uuid.uuid4().hex[:16]}"
    }


def create_error_response(
    code: str,
    message: str,
    request_id: str = None,
    field: str = None
) -> dict:
    """
    Create standardized error response.
    
    Args:
        code: Error code
        message: Error message
        request_id: Correlation ID
        field: Optional field causing error
    
    Returns:
        dict: Standardized error response
    """
    return {
        "success": False,
        "error": {
            "code": code,
            "message": message,
            "field": field
        },
        "timestamp": datetime.utcnow().isoformat(),
        "request_id": request_id or f"req-{uuid.uuid4().hex[:16]}"
    }


# ============================================================================
# TYPED DEPENDENCIES (ANNOTATED)
# ============================================================================

# Reusable annotated dependencies
AuthenticatedRequest = Annotated[str, Depends(verify_api_key)]
CorrelationID = Annotated[str, Depends(get_correlation_id)]
RateLimited = Annotated[bool, Depends(check_rate_limit)]
Orchestrator = Annotated[AgentOrchestrator, Depends(get_orchestrator)]
LLMFactory = Annotated[LLMProviderFactory, Depends(get_llm_factory)]