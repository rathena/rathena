"""
HTTP Proxy Handler for AI IPC Service

This handler provides backward compatibility with the httpget and httppost
NPC script commands by proxying HTTP requests to external services.

Request Types:
    - http_get, httpget, get
    - http_post, httppost, post

Endpoints:
    Any endpoint path - reconstructed with configured base URL

Request Data Format (for POST):
    {
        "body": {...},           # Request body (JSON)
        "headers": {...},        # Optional headers
        "timeout": 10            # Optional timeout override
    }

Response:
    The JSON response from the external service, or error object
"""

from __future__ import annotations

import asyncio
import json
import time
from typing import Any
from urllib.parse import urljoin, urlparse

import aiohttp
from aiohttp import ClientTimeout, ClientError, ClientResponseError

from .base import BaseHandler, HandlerError, ValidationError, TimeoutError


class HttpProxyHandler(BaseHandler):
    """
    Handler for HTTP proxy requests.
    
    Provides backward compatibility with httpget/httppost NPC commands
    by forwarding requests to external HTTP services.
    
    The handler reconstructs the full URL using the configured base URL
    and the endpoint from the request.
    
    Example:
        Base URL: http://localhost:8000
        Endpoint: /api/v1/events/dispatch
        Full URL: http://localhost:8000/api/v1/events/dispatch
    """
    
    # Shared session for connection pooling
    _session: aiohttp.ClientSession | None = None
    _session_lock: asyncio.Lock = asyncio.Lock()
    
    async def _get_session(self) -> aiohttp.ClientSession:
        """
        Get or create shared aiohttp session.
        
        Uses a lock to prevent race conditions during session creation.
        
        Returns:
            Shared ClientSession instance
        """
        if self._session is None or self._session.closed:  # pragma: no branch
            async with self._session_lock:
                if self._session is None or self._session.closed:  # pragma: no branch
                    timeout = ClientTimeout(
                        total=self.config.ai_service.timeout_seconds
                    )
                    self._session = aiohttp.ClientSession(
                        timeout=timeout,
                        headers={
                            "User-Agent": f"ai_ipc_service/{self.config.version}",
                            "Content-Type": "application/json",
                        }
                    )
        return self._session
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process HTTP proxy request.
        
        Forwards the request to the external service and returns
        the response. Supports both GET and POST methods.
        
        Args:
            request: Request dictionary from database
            
        Returns:
            Response from external service or error object
        """
        self.log_request_start(request)
        
        # Determine HTTP method from request_type
        request_type = request.get("request_type", "").lower()
        method = self._get_method(request_type)
        
        # Get endpoint and construct full URL
        endpoint = self.get_endpoint(request)
        if not endpoint:
            raise ValidationError("Endpoint is required for HTTP proxy requests")
        
        url = self._construct_url(endpoint)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Get optional parameters
        headers = data.get("headers", {})
        body = data.get("body", data)  # Use 'body' field or entire data as body
        timeout_override = data.get("timeout")
        
        # Remove meta fields from body if present
        if isinstance(body, dict):
            body = {k: v for k, v in body.items() 
                    if k not in ("headers", "timeout")}
        
        self.logger.info(
            f"Proxying {method} request to {url} "
            f"(request_id={request.get('id')})"
        )
        
        # Execute HTTP request
        start_time = time.time()
        try:
            response = await self._execute_request(
                method=method,
                url=url,
                body=body if method == "POST" else None,
                headers=headers,
                timeout=timeout_override,
            )
            
            elapsed_ms = int((time.time() - start_time) * 1000)
            self.logger.info(
                f"Proxy request completed in {elapsed_ms}ms "
                f"(request_id={request.get('id')})"
            )
            
            return response
            
        except asyncio.TimeoutError:
            elapsed_ms = int((time.time() - start_time) * 1000)
            self.logger.error(
                f"Proxy request timed out after {elapsed_ms}ms "
                f"(request_id={request.get('id')}, url={url})"
            )
            raise TimeoutError(f"Request to {url} timed out")
            
        except ClientResponseError as e:
            self.logger.error(
                f"HTTP error from {url}: {e.status} {e.message} "
                f"(request_id={request.get('id')})"
            )
            return self.create_error_response(
                error=f"HTTP {e.status}: {e.message}",
                code=e.status,
                url=url,
            )
            
        except ClientError as e:
            self.logger.error(
                f"Client error for {url}: {e} "
                f"(request_id={request.get('id')})"
            )
            raise HandlerError(f"HTTP client error: {e}")
            
        except Exception as e:
            self.logger.error(
                f"Unexpected error proxying to {url}: {e} "
                f"(request_id={request.get('id')})"
            )
            raise HandlerError(f"Proxy error: {e}")
    
    def _get_method(self, request_type: str) -> str:
        """
        Determine HTTP method from request type.
        
        Args:
            request_type: Request type string
            
        Returns:
            HTTP method (GET or POST)
        """
        post_types = {"http_post", "httppost", "post"}
        return "POST" if request_type in post_types else "GET"
    
    def _construct_url(self, endpoint: str) -> str:
        """
        Construct full URL from endpoint.
        
        If endpoint is already a full URL, use it directly.
        Otherwise, combine with configured base URL.
        
        Args:
            endpoint: Endpoint path or full URL
            
        Returns:
            Full URL string
        """
        # Check if endpoint is already a full URL
        parsed = urlparse(endpoint)
        if parsed.scheme in ("http", "https"):
            return endpoint
        
        # Combine with base URL
        base_url = self.config.ai_service.base_url.rstrip("/")
        
        # Ensure endpoint starts with /
        if not endpoint.startswith("/"):
            endpoint = "/" + endpoint
        
        return base_url + endpoint
    
    async def _execute_request(
        self,
        method: str,
        url: str,
        body: dict[str, Any] | None = None,
        headers: dict[str, str] | None = None,
        timeout: int | None = None,
    ) -> dict[str, Any]:
        """
        Execute the HTTP request.
        
        Args:
            method: HTTP method (GET or POST)
            url: Full URL to request
            body: Request body for POST
            headers: Additional headers
            timeout: Timeout in seconds (overrides config)
            
        Returns:
            Response data as dictionary
        """
        session = await self._get_session()
        
        # Build request kwargs
        kwargs: dict[str, Any] = {}
        
        if headers:
            kwargs["headers"] = headers
        
        if timeout:
            kwargs["timeout"] = ClientTimeout(total=timeout)
        
        if method == "POST" and body:
            kwargs["json"] = body
        
        # Execute request
        async with session.request(method, url, **kwargs) as response:
            # Check for successful response
            response.raise_for_status()
            
            # Try to parse as JSON
            content_type = response.headers.get("Content-Type", "")
            
            if "application/json" in content_type:
                return await response.json()
            else:
                # Return text wrapped in response object
                text = await response.text()
                return {
                    "status": "ok",
                    "data": text,
                    "content_type": content_type,
                }
    
    async def close(self) -> None:
        """
        Close the shared HTTP session.
        
        Should be called during service shutdown.
        """
        if self._session and not self._session.closed:
            await self._session.close()
            self._session = None


class HttpGetHandler(HttpProxyHandler):
    """
    Specialized handler for HTTP GET requests.
    
    Alias for HttpProxyHandler with GET method.
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """Force GET method regardless of request_type."""
        # Override request_type to ensure GET
        request = dict(request)
        request["request_type"] = "http_get"
        return await super().handle(request)


class HttpPostHandler(HttpProxyHandler):
    """
    Specialized handler for HTTP POST requests.
    
    Alias for HttpProxyHandler with POST method.
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """Force POST method regardless of request_type."""
        # Override request_type to ensure POST
        request = dict(request)
        request["request_type"] = "http_post"
        return await super().handle(request)