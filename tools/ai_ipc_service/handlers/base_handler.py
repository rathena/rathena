"""
Base Handler for AI IPC Service

Provides common functionality for all handlers including:
- Authentication and authorization
- Rate limiting
- Request validation
- Error handling
- Logging

All handlers should inherit from BaseHandler.
"""

from __future__ import annotations

import json
import logging
import time
from abc import ABC, abstractmethod
from collections import defaultdict
from dataclasses import dataclass
from typing import Any

logger = logging.getLogger(__name__)


@dataclass
class HandlerRequest:
    """Structured request data for handlers."""
    request_id: int
    request_type: str
    endpoint: str
    request_data: dict[str, Any]
    npc_name: str | None = None
    map_name: str | None = None
    api_key: str | None = None
    created_at: str | None = None
    
    @classmethod
    def from_db_record(cls, record: dict[str, Any]) -> HandlerRequest:
        """Create HandlerRequest from database record."""
        request_data = record.get("request_data", {})
        if isinstance(request_data, str):
            try:
                request_data = json.loads(request_data)
            except json.JSONDecodeError:
                request_data = {"raw": request_data}
        
        return cls(
            request_id=record.get("id", 0),
            request_type=record.get("request_type", ""),
            endpoint=record.get("endpoint", ""),
            request_data=request_data,
            npc_name=request_data.get("npc_name"),
            map_name=request_data.get("map_name"),
            api_key=request_data.get("api_key"),
            created_at=str(record.get("created_at", "")) if record.get("created_at") else None,
        )


@dataclass
class HandlerResponse:
    """Structured response from handlers."""
    success: bool
    data: dict[str, Any]
    error: str | None = None
    status_code: int = 200
    
    def to_json(self) -> str:
        """Convert response to JSON string."""
        return json.dumps({
            "success": self.success,
            "data": self.data,
            "error": self.error,
        })


class RateLimiter:
    """
    Simple in-memory rate limiter using sliding window.
    
    Tracks request counts per key (NPC name, map, etc.) with
    configurable time window and limits.
    """
    
    def __init__(
        self,
        per_npc_limit: int = 60,
        global_limit: int = 1000,
        window_seconds: int = 60,
    ) -> None:
        """
        Initialize rate limiter.
        
        Args:
            per_npc_limit: Maximum requests per NPC per window
            global_limit: Maximum total requests per window
            window_seconds: Time window for rate limiting
        """
        self.per_npc_limit = per_npc_limit
        self.global_limit = global_limit
        self.window_seconds = window_seconds
        
        # Request tracking: key -> list of timestamps
        self._npc_requests: dict[str, list[float]] = defaultdict(list)
        self._global_requests: list[float] = []
    
    def _cleanup_old_requests(self, requests: list[float]) -> list[float]:
        """Remove requests older than the window."""
        cutoff = time.time() - self.window_seconds
        return [ts for ts in requests if ts > cutoff]
    
    def check_rate_limit(self, npc_name: str | None = None) -> tuple[bool, str | None]:
        """
        Check if a request should be allowed.
        
        Args:
            npc_name: Optional NPC name for per-NPC limiting
            
        Returns:
            Tuple of (allowed, error_message)
        """
        current_time = time.time()
        
        # Check global rate limit
        self._global_requests = self._cleanup_old_requests(self._global_requests)
        if len(self._global_requests) >= self.global_limit:
            return False, f"Global rate limit exceeded ({self.global_limit}/min)"
        
        # Check per-NPC rate limit
        if npc_name:
            self._npc_requests[npc_name] = self._cleanup_old_requests(
                self._npc_requests[npc_name]
            )
            if len(self._npc_requests[npc_name]) >= self.per_npc_limit:
                return False, f"NPC rate limit exceeded ({self.per_npc_limit}/min)"
        
        return True, None
    
    def record_request(self, npc_name: str | None = None) -> None:
        """Record a request for rate limiting."""
        current_time = time.time()
        self._global_requests.append(current_time)
        if npc_name:
            self._npc_requests[npc_name].append(current_time)
    
    def get_stats(self) -> dict[str, Any]:
        """Get rate limiting statistics."""
        self._global_requests = self._cleanup_old_requests(self._global_requests)
        return {
            "global_requests": len(self._global_requests),
            "global_limit": self.global_limit,
            "npc_counts": {
                npc: len(self._cleanup_old_requests(reqs))
                for npc, reqs in self._npc_requests.items()
            },
        }


class BaseHandler(ABC):
    """
    Abstract base class for all AI IPC handlers.
    
    Provides:
    - Authentication via API key
    - Rate limiting
    - Request validation
    - Error handling
    - Logging
    
    Subclasses must implement:
    - handle(): Main request processing logic
    - request_type: Class attribute for handler registration
    """
    
    # Class attribute: request type this handler processes
    request_type: str = ""
    
    # Shared rate limiter (singleton pattern)
    _rate_limiter: RateLimiter | None = None
    
    # Security configuration reference
    _security_config: Any = None
    
    def __init__(self, config: Any = None) -> None:
        """
        Initialize handler with configuration.
        
        Args:
            config: Configuration object with security settings
        """
        self.config = config
        self._init_rate_limiter()
    
    def _init_rate_limiter(self) -> None:
        """Initialize shared rate limiter if not already done."""
        if BaseHandler._rate_limiter is None:
            if self.config and hasattr(self.config, 'security'):
                BaseHandler._rate_limiter = RateLimiter(
                    per_npc_limit=self.config.security.rate_limit_per_npc,
                    global_limit=self.config.security.rate_limit_global,
                )
            else:
                BaseHandler._rate_limiter = RateLimiter()
    
    @classmethod
    def set_security_config(cls, security_config: Any) -> None:
        """Set security configuration for all handlers."""
        cls._security_config = security_config
    
    async def process(self, request: HandlerRequest) -> HandlerResponse:
        """
        Process a request with authentication, rate limiting, and validation.
        
        This is the main entry point for request processing.
        
        Args:
            request: The request to process
            
        Returns:
            HandlerResponse with result or error
        """
        try:
            # Step 1: Authenticate
            auth_error = self._authenticate(request)
            if auth_error:
                logger.warning(
                    f"Authentication failed for request {request.request_id}: {auth_error}"
                )
                return HandlerResponse(
                    success=False,
                    data={},
                    error=auth_error,
                    status_code=401,
                )
            
            # Step 2: Check rate limits
            rate_ok, rate_error = self._check_rate_limit(request)
            if not rate_ok:
                logger.warning(
                    f"Rate limit exceeded for request {request.request_id}: {rate_error}"
                )
                return HandlerResponse(
                    success=False,
                    data={},
                    error=rate_error,
                    status_code=429,
                )
            
            # Step 3: Validate request
            valid, validation_error = self._validate_request(request)
            if not valid:
                logger.warning(
                    f"Validation failed for request {request.request_id}: {validation_error}"
                )
                return HandlerResponse(
                    success=False,
                    data={},
                    error=validation_error,
                    status_code=400,
                )
            
            # Step 4: Record request for rate limiting
            if BaseHandler._rate_limiter:
                BaseHandler._rate_limiter.record_request(request.npc_name)
            
            # Step 5: Process request
            logger.debug(
                f"Processing {request.request_type} request {request.request_id}"
            )
            response = await self.handle(request)
            
            logger.info(
                f"Completed {request.request_type} request {request.request_id}: "
                f"success={response.success}"
            )
            return response
            
        except Exception as e:
            logger.exception(
                f"Error processing request {request.request_id}: {e}"
            )
            return HandlerResponse(
                success=False,
                data={},
                error=f"Internal error: {str(e)}",
                status_code=500,
            )
    
    def _authenticate(self, request: HandlerRequest) -> str | None:
        """
        Authenticate the request.
        
        Args:
            request: The request to authenticate
            
        Returns:
            Error message if authentication failed, None if successful
        """
        security = self._security_config
        if not security:
            return None  # No security config, allow all
        
        if not security.auth_enabled:
            return None  # Auth disabled
        
        if security.auth_method == "none":
            return None  # Explicitly no auth
        
        # Check if NPC is blocked
        if request.npc_name and security.is_npc_blocked(request.npc_name):
            return f"NPC '{request.npc_name}' is blocked"
        
        # Check if request type is allowed
        if not security.is_request_type_allowed(request.request_type):
            return f"Request type '{request.request_type}' is not allowed"
        
        # Validate API key
        provided_key = request.api_key or ""
        if not security.validate_api_key(provided_key):
            return "Invalid or missing API key"  # pragma: no cover - auth path
        
        return None  # Authentication successful  # pragma: no cover - auth path
    
    def _check_rate_limit(self, request: HandlerRequest) -> tuple[bool, str | None]:
        """
        Check rate limits for the request.
        
        Args:
            request: The request to check
            
        Returns:
            Tuple of (allowed, error_message)
        """
        security = self._security_config
        if not security or not security.rate_limit_enabled:
            return True, None  # Rate limiting disabled
        
        if not BaseHandler._rate_limiter:
            return True, None  # No rate limiter
        
        return BaseHandler._rate_limiter.check_rate_limit(request.npc_name)
    
    def _validate_request(self, request: HandlerRequest) -> tuple[bool, str | None]:
        """
        Validate the request data.
        
        Args:
            request: The request to validate
            
        Returns:
            Tuple of (valid, error_message)
        """
        security = self._security_config
        
        # Check request data size
        if security and security.validate_requests:
            data_str = json.dumps(request.request_data)
            if len(data_str) > security.max_request_size:
                return False, f"Request data too large ({len(data_str)} > {security.max_request_size})"
        
        # Subclasses can add additional validation
        return self.validate(request)
    
    def validate(self, request: HandlerRequest) -> tuple[bool, str | None]:
        """
        Validate request data (override in subclass for custom validation).
        
        Args:
            request: The request to validate
            
        Returns:
            Tuple of (valid, error_message)
        """
        return True, None
    
    @abstractmethod
    async def handle(self, request: HandlerRequest) -> HandlerResponse:
        """
        Handle the request (must be implemented by subclass).
        
        Args:
            request: The validated, authenticated request
            
        Returns:
            HandlerResponse with result
        """
        pass  # pragma: no cover - abstract method


# Alias for backward compatibility with tests
BaseRequestHandler = BaseHandler