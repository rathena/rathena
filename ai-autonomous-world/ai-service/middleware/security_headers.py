"""
Security Headers Middleware
Implements comprehensive security headers for FastAPI applications

Addresses: SEC-029 (Missing Security Headers)
"""

from starlette.middleware.base import BaseHTTPMiddleware
from starlette.requests import Request
from starlette.responses import Response
from typing import Callable
from loguru import logger


class SecurityHeadersMiddleware(BaseHTTPMiddleware):
    """
    Middleware to add security headers to all HTTP responses
    
    Implemented Headers:
    - Content-Security-Policy: Prevents XSS attacks
    - Strict-Transport-Security: Forces HTTPS
    - X-Frame-Options: Prevents clickjacking
    - X-Content-Type-Options: Prevents MIME sniffing
    - X-XSS-Protection: Legacy XSS protection
    - Referrer-Policy: Controls referrer information
    - Permissions-Policy: Controls browser features
    """
    
    def __init__(
        self,
        app,
        enable_hsts: bool = True,
        hsts_max_age: int = 31536000,  # 1 year
        enable_csp: bool = True,
        csp_directives: str = None
    ):
        super().__init__(app)
        self.enable_hsts = enable_hsts
        self.hsts_max_age = hsts_max_age
        self.enable_csp = enable_csp
        
        # Default CSP directives (can be overridden)
        self.csp_directives = csp_directives or (
            "default-src 'self'; "
            "script-src 'self' 'unsafe-inline' 'unsafe-eval'; "
            "style-src 'self' 'unsafe-inline'; "
            "img-src 'self' data: https:; "
            "font-src 'self' data:; "
            "connect-src 'self'; "
            "frame-ancestors 'none'; "
            "base-uri 'self'; "
            "form-action 'self'"
        )
        
        logger.debug("SecurityHeadersMiddleware initialized")
    
    async def dispatch(
        self, 
        request: Request, 
        call_next: Callable
    ) -> Response:
        """Process request and add security headers to response"""
        
        # Process the request
        response = await call_next(request)
        
        # Add security headers
        
        # 1. Content Security Policy (CSP)
        # Prevents XSS, clickjacking, and other code injection attacks
        if self.enable_csp:
            response.headers["Content-Security-Policy"] = self.csp_directives
        
        # 2. Strict Transport Security (HSTS)
        # Forces browsers to use HTTPS for all future requests
        if self.enable_hsts:
            response.headers["Strict-Transport-Security"] = (
                f"max-age={self.hsts_max_age}; includeSubDomains; preload"
            )
        
        # 3. X-Frame-Options
        # Prevents clickjacking by disallowing page embedding
        response.headers["X-Frame-Options"] = "DENY"
        
        # 4. X-Content-Type-Options
        # Prevents MIME type sniffing
        response.headers["X-Content-Type-Options"] = "nosniff"
        
        # 5. X-XSS-Protection
        # Legacy XSS protection (deprecated but still useful for old browsers)
        response.headers["X-XSS-Protection"] = "1; mode=block"
        
        # 6. Referrer-Policy
        # Controls how much referrer information is sent
        response.headers["Referrer-Policy"] = "strict-origin-when-cross-origin"
        
        # 7. Permissions-Policy (formerly Feature-Policy)
        # Controls which browser features can be used
        response.headers["Permissions-Policy"] = (
            "geolocation=(), "
            "microphone=(), "
            "camera=(), "
            "payment=(), "
            "usb=(), "
            "magnetometer=(), "
            "gyroscope=(), "
            "accelerometer=()"
        )
        
        # 8. Cache-Control for sensitive endpoints
        # Prevent caching of sensitive data
        if request.url.path.startswith('/api/'):
            response.headers["Cache-Control"] = "no-store, no-cache, must-revalidate, private"
            response.headers["Pragma"] = "no-cache"
        
        return response


def configure_security_headers(
    enable_hsts: bool = True,
    hsts_max_age: int = 31536000,
    enable_csp: bool = True,
    csp_directives: str = None
) -> SecurityHeadersMiddleware:
    """
    Factory function to create configured SecurityHeadersMiddleware
    
    Args:
        enable_hsts: Enable HTTP Strict Transport Security
        hsts_max_age: HSTS max age in seconds (default 1 year)
        enable_csp: Enable Content Security Policy
        csp_directives: Custom CSP directives (optional)
    
    Returns:
        Configured SecurityHeadersMiddleware class
    
    Example:
        app.add_middleware(
            SecurityHeadersMiddleware,
            enable_hsts=True,
            hsts_max_age=31536000,
            enable_csp=True
        )
    """
    class ConfiguredSecurityHeaders(SecurityHeadersMiddleware):
        def __init__(self, app):
            super().__init__(
                app,
                enable_hsts=enable_hsts,
                hsts_max_age=hsts_max_age,
                enable_csp=enable_csp,
                csp_directives=csp_directives
            )
    
    return ConfiguredSecurityHeaders