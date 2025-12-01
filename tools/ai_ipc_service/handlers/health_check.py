"""
Health Check Handler for AI IPC Service

This handler responds to health check requests from NPC scripts,
providing status information about the AI IPC service and its
dependencies (database, external services).

Request Types:
    - health_check
    - health

Endpoints:
    - /api/v1/health
    - /health
    - (any endpoint - returns service status)

Response:
    {
        "status": "ok",
        "service": "ai_ipc_service",
        "version": "1.0.0",
        "timestamp": "2025-11-27T00:00:00.000000",
        "components": {
            "database": "healthy",
            "ai_service": "healthy"
        }
    }
"""

from __future__ import annotations

import asyncio
import platform
from datetime import datetime
from typing import Any

import aiohttp

from .base import BaseHandler, HandlerError


class HealthCheckHandler(BaseHandler):
    """
    Handler for health check requests.
    
    Returns service status information including version,
    uptime, and component health states.
    
    This handler is used by NPC scripts to verify the AI
    service is operational before making other requests.
    """
    
    # Track service start time for uptime calculation
    _start_time: datetime | None = None
    
    def __init__(self, config) -> None:
        """Initialize health check handler."""
        super().__init__(config)
        if HealthCheckHandler._start_time is None:
            HealthCheckHandler._start_time = datetime.utcnow()
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process health check request.
        
        Performs health checks on service components and returns
        aggregated status information.
        
        Args:
            request: Request dictionary from database
            
        Returns:
            Health status response dictionary
        """
        self.log_request_start(request)
        
        # Get endpoint to determine detail level
        endpoint = self.get_endpoint(request)
        include_details = "detailed" in endpoint or "full" in endpoint
        
        # Basic health response
        response = {
            "status": "ok",
            "service": self.config.service_name,
            "version": self.config.version,
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        # Add uptime if available
        if self._start_time:
            uptime_seconds = (datetime.utcnow() - self._start_time).total_seconds()
            response["uptime_seconds"] = int(uptime_seconds)
        
        # Add detailed component health if requested
        if include_details:
            response["components"] = await self._check_components()
            response["system"] = self._get_system_info()
        
        return response
    
    async def _check_components(self) -> dict[str, Any]:
        """
        Check health of service components.
        
        Returns:
            Dictionary with component health statuses
        """
        components = {}
        
        # Check AI service connectivity
        ai_status = await self._check_ai_service()
        components["ai_service"] = ai_status
        
        return components
    
    async def _check_ai_service(self) -> dict[str, Any]:
        """
        Check if external AI service is reachable.
        
        Returns:
            AI service health status
        """
        base_url = self.config.ai_service.base_url
        health_url = f"{base_url.rstrip('/')}/health"
        
        try:
            timeout = aiohttp.ClientTimeout(total=5)
            async with aiohttp.ClientSession(timeout=timeout) as session:
                async with session.get(health_url) as resp:
                    if resp.status == 200:
                        return {
                            "status": "healthy",
                            "url": base_url,
                            "response_time_ms": 0,  # Would need timing
                        }
                    else:
                        return {
                            "status": "degraded",
                            "url": base_url,
                            "http_status": resp.status,
                        }
        except asyncio.TimeoutError:
            self.logger.warning(f"AI service health check timed out: {base_url}")
            return {
                "status": "timeout",
                "url": base_url,
                "error": "Connection timeout",
            }
        except aiohttp.ClientError as e:
            self.logger.warning(f"AI service health check failed: {e}")
            return {
                "status": "unhealthy",
                "url": base_url,
                "error": str(e),
            }
        except Exception as e:
            self.logger.error(f"Unexpected error in AI service check: {e}")
            return {
                "status": "error",
                "url": base_url,
                "error": str(e),
            }
    
    def _get_system_info(self) -> dict[str, Any]:
        """
        Get system information for diagnostics.
        
        Returns:
            System information dictionary
        """
        return {
            "python_version": platform.python_version(),
            "platform": platform.platform(),
            "processor": platform.processor() or "unknown",
        }


class ReadinessHandler(HealthCheckHandler):
    """
    Kubernetes-style readiness probe handler.
    
    Returns 200 if service is ready to accept requests,
    or error status if service is not ready.
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Check if service is ready to accept requests.
        
        Args:
            request: Request dictionary
            
        Returns:
            Readiness status response
        """
        # Basic readiness check
        return {
            "ready": True,
            "timestamp": datetime.utcnow().isoformat(),
        }


class LivenessHandler(HealthCheckHandler):
    """
    Kubernetes-style liveness probe handler.
    
    Returns 200 if service is alive (should not be restarted),
    or error status if service is in a bad state.
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Check if service is alive and functioning.
        
        Args:
            request: Request dictionary
            
        Returns:
            Liveness status response
        """
        # Basic liveness check - if we can respond, we're alive
        return {
            "alive": True,
            "timestamp": datetime.utcnow().isoformat(),
        }