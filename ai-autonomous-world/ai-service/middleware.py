"""
Security Middleware for AI Autonomous World Service

Implements:
- Fix #3: API Key Authentication Middleware
- Fix #3: Rate Limiting Middleware (Token Bucket with DragonflyDB)
- Fix #3: Request Size Limit Middleware
- Fix #10: Security Headers Middleware

All middleware follows FastAPI patterns with comprehensive error handling.
"""

import time
import logging
import hashlib
import redis
from typing import Optional, Callable
from datetime import datetime, timedelta
from fastapi import Request, Response, HTTPException, status
from fastapi.responses import JSONResponse
from starlette.middleware.base import BaseHTTPMiddleware
from starlette.types import ASGIApp

from .config import settings
from .utils.log_sanitizer import sanitize_for_logging

logger = logging.getLogger(__name__)


# ============================================================================
# FIX #3: API KEY AUTHENTICATION MIDDLEWARE
# ============================================================================

class APIKeyMiddleware(BaseHTTPMiddleware):
    """
    Validate API keys from Authorization header or query parameter.
    
    Supports:
    - Bearer token authentication
    - API key query parameter
    - Configurable exempt paths
    """
    
    EXEMPT_PATHS = {
        '/health',
        '/healthz', 
        '/readiness',
        '/metrics',
        '/docs',
        '/redoc',
        '/openapi.json'
    }
    
    def __init__(self, app: ASGIApp):
        super().__init__(app)
        self.api_key_required = settings.api_key_required
        self.api_key = settings.api_key
        
        if self.api_key_required and not self.api_key:
            logger.critical(
                "API_KEY_REQUIRED=true but API_KEY is not set. "
                "Service will reject all authenticated requests!"
            )
    
    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        """Validate API key for protected endpoints"""
        
        # Skip authentication check if not required
        if not self.api_key_required:
            logger.debug("API key authentication disabled")
            return await call_next(request)
        
        # Skip exempt paths
        if request.url.path in self.EXEMPT_PATHS:
            return await call_next(request)
        
        # Extract API key from request
        api_key = self._extract_api_key(request)
        
        if not api_key:
            logger.warning(
                f"Missing API key for {request.method} {request.url.path} "
                f"from {request.client.host}"
            )
            return JSONResponse(
                status_code=status.HTTP_401_UNAUTHORIZED,
                content={
                    "detail": "Missing API key",
                    "error": "authentication_required",
                    "instructions": "Provide API key via Authorization header or api_key parameter"
                },
                headers={"WWW-Authenticate": "Bearer"}
            )
        
        # Validate API key (constant-time comparison to prevent timing attacks)
        if not self._validate_api_key(api_key):
            logger.warning(
                f"Invalid API key for {request.method} {request.url.path} "
                f"from {request.client.host}"
            )
            return JSONResponse(
                status_code=status.HTTP_403_FORBIDDEN,
                content={
                    "detail": "Invalid API key",
                    "error": "authentication_failed"
                }
            )
        
        # API key valid - proceed with request
        logger.debug(f"API key validated for {request.url.path}")
        return await call_next(request)
    
    def _extract_api_key(self, request: Request) -> Optional[str]:
        """Extract API key from Authorization header or query parameter"""
        
        # Try Authorization header (Bearer token)
        auth_header = request.headers.get("Authorization")
        if auth_header:
            parts = auth_header.split()
            if len(parts) == 2 and parts[0].lower() == "bearer":
                return parts[1]
            # Also support "ApiKey <key>" format
            if len(parts) == 2 and parts[0].lower() == "apikey":
                return parts[1]
        
        # Try X-API-Key header
        api_key_header = request.headers.get("X-API-Key")
        if api_key_header:
            return api_key_header
        
        # Try query parameter (less secure, but useful for development)
        api_key_param = request.query_params.get("api_key")
        if api_key_param:
            logger.warning(
                "API key provided via query parameter - "
                "this is insecure and should only be used in development"
            )
            return api_key_param
        
        return None
    
    def _validate_api_key(self, provided_key: str) -> bool:
        """
        Validate API key using constant-time comparison.
        Prevents timing attacks.
        """
        if not self.api_key:
            return False
        
        # Use constant-time comparison via hmac.compare_digest
        import hmac
        expected_hash = hashlib.sha256(self.api_key.encode()).digest()
        provided_hash = hashlib.sha256(provided_key.encode()).digest()
        
        return hmac.compare_digest(expected_hash, provided_hash)


# ============================================================================
# FIX #3: RATE LIMITING MIDDLEWARE (TOKEN BUCKET ALGORITHM)
# ============================================================================

class RateLimitMiddleware(BaseHTTPMiddleware):
    """
    Rate limiting using token bucket algorithm with DragonflyDB backend.
    
    Features:
    - Per-IP rate limiting
    - Configurable limits
    - Burst allowance
    - Distributed rate limiting via Redis
    """
    
    def __init__(self, app: ASGIApp):
        super().__init__(app)
        self.enabled = settings.rate_limit_enabled
        self.requests_per_minute = settings.rate_limit_requests_per_minute
        self.burst = settings.rate_limit_burst
        
        # Initialize Redis connection for distributed rate limiting
        if self.enabled:
            try:
                self.redis_client = redis.Redis(
                    host=settings.dragonfly_host,
                    port=settings.dragonfly_port,
                    password=settings.dragonfly_password if settings.dragonfly_password != "REPLACE_WITH_VAULT_REFERENCE" else None,
                    db=settings.dragonfly_db,
                    ssl=settings.dragonfly_ssl,
                    decode_responses=True,
                    socket_timeout=5,
                    socket_connect_timeout=5
                )
                # Test connection
                self.redis_client.ping()
                logger.info("✓ Rate limiting enabled with DragonflyDB backend")
            except Exception as e:
                logger.error(f"Failed to connect to DragonflyDB for rate limiting: {e}")
                logger.warning("Rate limiting will be disabled")
                self.enabled = False
    
    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        """Apply rate limiting"""
        
        if not self.enabled:
            return await call_next(request)
        
        # Get client identifier (IP address)
        client_ip = self._get_client_ip(request)
        
        # Check rate limit
        allowed, remaining, reset_time = self._check_rate_limit(client_ip)
        
        # Proceed with request
        response = await call_next(request) if allowed else self._rate_limit_exceeded_response(reset_time)
        
        # Add rate limit headers
        response.headers["X-RateLimit-Limit"] = str(self.requests_per_minute)
        response.headers["X-RateLimit-Remaining"] = str(remaining)
        response.headers["X-RateLimit-Reset"] = str(reset_time)
        
        return response
    
    def _get_client_ip(self, request: Request) -> str:
        """Get client IP address, considering proxy headers"""
        
        # Check X-Forwarded-For header (if behind proxy)
        forwarded = request.headers.get("X-Forwarded-For")
        if forwarded:
            # Get first IP in chain (original client)
            return forwarded.split(",")[0].strip()
        
        # Check X-Real-IP header
        real_ip = request.headers.get("X-Real-IP")
        if real_ip:
            return real_ip
        
        # Fall back to direct connection IP
        return request.client.host if request.client else "unknown"
    
    def _check_rate_limit(self, client_ip: str) -> tuple[bool, int, int]:
        """
        Check rate limit using token bucket algorithm.
        
        Returns: (allowed, remaining_tokens, reset_time_unix)
        """
        key = f"ratelimit:{client_ip}"
        now = int(time.time())
        window = 60  # 1 minute window
        
        try:
            # Get current state from Redis
            pipe = self.redis_client.pipeline()
            pipe.get(f"{key}:tokens")
            pipe.get(f"{key}:last_update")
            tokens_str, last_update_str = pipe.execute()
            
            # Initialize or parse current state
            tokens = float(tokens_str) if tokens_str else self.requests_per_minute
            last_update = int(last_update_str) if last_update_str else now
            
            # Calculate token refill
            time_passed = now - last_update
            refill = (time_passed / window) * self.requests_per_minute
            tokens = min(tokens + refill, self.requests_per_minute + self.burst)
            
            # Check if request allowed
            allowed = tokens >= 1.0
            
            if allowed:
                tokens -= 1.0
            
            # Update Redis state
            pipe = self.redis_client.pipeline()
            pipe.setex(f"{key}:tokens", window * 2, str(tokens))
            pipe.setex(f"{key}:last_update", window * 2, str(now))
            pipe.execute()
            
            # Calculate reset time
            reset_time = now + window
            remaining = int(tokens)
            
            return allowed, remaining, reset_time
            
        except Exception as e:
            logger.error(f"Rate limit check failed: {e}")
            # On error, allow request but log issue
            return True, self.requests_per_minute, now + window
    
    def _rate_limit_exceeded_response(self, reset_time: int) -> Response:
        """Return rate limit exceeded response"""
        retry_after = reset_time - int(time.time())
        
        logger.warning(f"Rate limit exceeded - retry after {retry_after}s")
        
        return JSONResponse(
            status_code=status.HTTP_429_TOO_MANY_REQUESTS,
            content={
                "detail": "Rate limit exceeded",
                "error": "rate_limit_exceeded",
                "retry_after": retry_after
            },
            headers={
                "Retry-After": str(retry_after)
            }
        )


# ============================================================================
# FIX #3: REQUEST SIZE LIMIT MIDDLEWARE
# ============================================================================

class RequestSizeLimitMiddleware(BaseHTTPMiddleware):
    """
    Prevent large payload attacks by limiting request body size.
    """
    
    def __init__(self, app: ASGIApp):
        super().__init__(app)
        self.max_size = settings.max_request_size_mb * 1024 * 1024  # Convert MB to bytes
    
    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        """Check request size before processing"""
        
        # Get content length from header
        content_length = request.headers.get("content-length")
        
        if content_length:
            content_length = int(content_length)
            if content_length > self.max_size:
                logger.warning(
                    f"Request size {content_length} bytes exceeds limit "
                    f"{self.max_size} bytes from {request.client.host}"
                )
                return JSONResponse(
                    status_code=status.HTTP_413_REQUEST_ENTITY_TOO_LARGE,
                    content={
                        "detail": f"Request body too large. Maximum size: {settings.max_request_size_mb}MB",
                        "error": "request_too_large",
                        "max_size_mb": settings.max_request_size_mb
                    }
                )
        
        return await call_next(request)


# ============================================================================
# FIX #10: SECURITY HEADERS MIDDLEWARE
# ============================================================================

class SecurityHeadersMiddleware(BaseHTTPMiddleware):
    """
    Add comprehensive security headers to all responses.
    
    Headers added:
    - Content-Security-Policy
    - Strict-Transport-Security (HSTS)
    - X-Frame-Options
    - X-Content-Type-Options
    - X-XSS-Protection
    - Referrer-Policy
    - Permissions-Policy
    """
    
    def __init__(self, app: ASGIApp):
        super().__init__(app)
        self.enabled = settings.security_headers_enabled
        self.csp_enabled = settings.csp_enabled
        self.hsts_max_age = settings.hsts_max_age
    
    async def dispatch(self, request: Request, call_next: Callable) -> Response:
        """Add security headers to response"""
        
        response = await call_next(request)
        
        if not self.enabled:
            return response
        
        # Content Security Policy (CSP)
        if self.csp_enabled:
            csp_directives = [
                "default-src 'self'",
                "script-src 'self' 'unsafe-inline' 'unsafe-eval' https://cdn.jsdelivr.net",
                "style-src 'self' 'unsafe-inline' https://fonts.googleapis.com",
                "font-src 'self' https://fonts.gstatic.com",
                "img-src 'self' data: https:",
                "connect-src 'self' https://*.openai.azure.com",
                "frame-ancestors 'none'",
                "base-uri 'self'",
                "form-action 'self'"
            ]
            response.headers["Content-Security-Policy"] = "; ".join(csp_directives)
        
        # HTTP Strict Transport Security (HSTS)
        if settings.is_production:
            response.headers["Strict-Transport-Security"] = (
                f"max-age={self.hsts_max_age}; includeSubDomains; preload"
            )
        
        # X-Frame-Options: Prevent clickjacking
        response.headers["X-Frame-Options"] = "DENY"
        
        # X-Content-Type-Options: Prevent MIME sniffing
        response.headers["X-Content-Type-Options"] = "nosniff"
        
        # X-XSS-Protection: Enable XSS filter (legacy but still useful)
        response.headers["X-XSS-Protection"] = "1; mode=block"
        
        # Referrer-Policy: Control referrer information
        response.headers["Referrer-Policy"] = "strict-origin-when-cross-origin"
        
        # Permissions-Policy: Control browser features
        permissions = [
            "accelerometer=()",
            "camera=()",
            "geolocation=()",
            "gyroscope=()",
            "magnetometer=()",
            "microphone=()",
            "payment=()",
            "usb=()"
        ]
        response.headers["Permissions-Policy"] = ", ".join(permissions)
        
        # X-Permitted-Cross-Domain-Policies
        response.headers["X-Permitted-Cross-Domain-Policies"] = "none"
        
        # Remove server identification
        response.headers.pop("Server", None)
        response.headers["X-Powered-By"] = "AI-Autonomous-World"
        
        return response


# ============================================================================
# MIDDLEWARE INITIALIZATION HELPER
# ============================================================================

def setup_security_middleware(app):
    """
    Setup all security middleware in correct order.
    
    Order matters:
    1. Request Size Limit (reject early if too large)
    2. Rate Limiting (prevent abuse)
    3. API Key Authentication (verify identity)
    4. Security Headers (add to all responses)
    """
    
    logger.info("🔒 Initializing security middleware...")
    
    # 1. Request Size Limit - First line of defense
    app.add_middleware(RequestSizeLimitMiddleware)
    logger.info("  ✓ Request size limit middleware enabled")
    
    # 2. Rate Limiting - Prevent abuse
    if settings.rate_limit_enabled:
        app.add_middleware(RateLimitMiddleware)
        logger.info("  ✓ Rate limiting middleware enabled")
    
    # 3. API Key Authentication - Verify identity
    if settings.api_key_required:
        app.add_middleware(APIKeyMiddleware)
        logger.info("  ✓ API key authentication middleware enabled")
    
    # 4. Security Headers - Add to all responses
    if settings.security_headers_enabled:
        app.add_middleware(SecurityHeadersMiddleware)
        logger.info("  ✓ Security headers middleware enabled")
    
    logger.info("✓ All security middleware initialized successfully")