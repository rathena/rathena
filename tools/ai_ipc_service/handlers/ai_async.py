"""
AI Async Handler for AI IPC Service

This handler processes asynchronous AI requests, allowing NPCs to make
non-blocking requests and check results later.

Request Types:
    - ai_request_async: Submit async request
    - ai_check_result: Check status of async request

Request Data Format (ai_request_async):
    {
        "request_type": "dialogue|decision|emotion|custom",
        "npc_id": "guard_001",
        "data": { ... request-specific data ... },
        "callback_event": "OnAsyncComplete",
        "timeout_seconds": 30
    }

Request Data Format (ai_check_result):
    {
        "async_request_id": "uuid-of-request"
    }

Response Format (ai_request_async):
    {
        "status": "ok",
        "async_request_id": "uuid-generated",
        "estimated_completion": "2024-01-01T00:00:30Z"
    }

Response Format (ai_check_result):
    {
        "status": "ok",
        "async_request_id": "uuid",
        "state": "pending|processing|completed|failed",
        "result": { ... if completed ... },
        "error": "... if failed ..."
    }
"""

from __future__ import annotations

import asyncio
import uuid
from datetime import datetime, timedelta
from typing import Any

from .base import BaseHandler, HandlerError, ValidationError


# In-memory store for async request tracking
# In production, this should be Redis or database-backed
_async_requests: dict[str, dict[str, Any]] = {}
_async_results: dict[str, dict[str, Any]] = {}


class AIAsyncRequestHandler(BaseHandler):
    """
    Handler for submitting asynchronous AI requests.
    
    Allows scripts to submit long-running AI requests without blocking,
    receiving a request ID to check results later.
    
    The handler supports:
    - Multiple request types (dialogue, decision, emotion, custom)
    - Callback event specification for NPC notification
    - Configurable timeouts
    - Request status tracking
    """
    
    VALID_REQUEST_TYPES = {"dialogue", "decision", "emotion", "custom"}
    DEFAULT_TIMEOUT_SECONDS = 30
    MAX_TIMEOUT_SECONDS = 300
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process async request submission.
        
        Args:
            request: Request dictionary with async request details
            
        Returns:
            Response with async request ID for tracking
        """
        self.log_request_start(request)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Validate request
        self._validate_async_request(data)
        
        # Generate unique request ID
        async_request_id = str(uuid.uuid4())
        
        # Extract parameters
        request_type = data.get("request_type", "custom")
        npc_id = data.get("npc_id", "unknown_npc")
        request_data = data.get("data", {})
        callback_event = data.get("callback_event")
        timeout_seconds = min(
            data.get("timeout_seconds", self.DEFAULT_TIMEOUT_SECONDS),
            self.MAX_TIMEOUT_SECONDS,
        )
        
        # Calculate estimated completion
        estimated_completion = datetime.utcnow() + timedelta(seconds=timeout_seconds)
        
        # Store request for tracking
        _async_requests[async_request_id] = {
            "id": async_request_id,
            "request_type": request_type,
            "npc_id": npc_id,
            "data": request_data,
            "callback_event": callback_event,
            "timeout_seconds": timeout_seconds,
            "state": "pending",
            "created_at": datetime.utcnow().isoformat(),
            "estimated_completion": estimated_completion.isoformat(),
        }
        
        # Schedule async processing
        asyncio.create_task(
            self._process_async_request(async_request_id)
        )
        
        self.logger.info(
            f"Async request submitted: {async_request_id} "
            f"(type={request_type}, npc={npc_id})"
        )
        
        return {
            "status": "ok",
            "async_request_id": async_request_id,
            "estimated_completion": estimated_completion.isoformat(),
            "state": "pending",
            "timestamp": datetime.utcnow().isoformat(),
        }
    
    def _validate_async_request(self, data: dict[str, Any]) -> None:
        """
        Validate async request data.
        
        Args:
            data: Request data dictionary
            
        Raises:
            ValidationError: If required fields are missing or invalid
        """
        request_type = data.get("request_type", "custom")
        if request_type not in self.VALID_REQUEST_TYPES:
            raise ValidationError(
                f"Invalid request_type: {request_type}. "
                f"Must be one of: {self.VALID_REQUEST_TYPES}"
            )
    
    async def _process_async_request(self, async_request_id: str) -> None:
        """
        Process an async request in the background.
        
        Args:
            async_request_id: ID of the async request to process
        """
        request_info = _async_requests.get(async_request_id)
        if not request_info:
            self.logger.error(f"Async request not found: {async_request_id}")
            return
        
        try:
            # Update state to processing
            request_info["state"] = "processing"
            request_info["processing_started_at"] = datetime.utcnow().isoformat()
            
            # Route to appropriate handler based on request type
            request_type = request_info["request_type"]
            result = await self._route_to_handler(
                request_type,
                request_info["npc_id"],
                request_info["data"],
            )
            
            # Store result
            _async_results[async_request_id] = {
                "status": "completed",
                "result": result,
                "completed_at": datetime.utcnow().isoformat(),
            }
            request_info["state"] = "completed"
            
            self.logger.info(f"Async request completed: {async_request_id}")
            
        except Exception as e:
            self.logger.error(f"Async request failed: {async_request_id}: {e}")
            _async_results[async_request_id] = {
                "status": "failed",
                "error": str(e),
                "failed_at": datetime.utcnow().isoformat(),
            }
            request_info["state"] = "failed"
    
    async def _route_to_handler(
        self,
        request_type: str,
        npc_id: str,
        data: dict[str, Any],
    ) -> dict[str, Any]:
        """
        Route async request to appropriate handler.
        
        Args:
            request_type: Type of request
            npc_id: NPC identifier
            data: Request data
            
        Returns:
            Handler result
        """
        # Import handlers here to avoid circular imports
        from .ai_dialogue import AIDialogueHandler
        from .ai_decision import AIDecisionHandler
        from .ai_emotion import AIEmotionHandler
        
        # Build synthetic request
        synthetic_request = {
            "command": f"ai_npc_{request_type}",
            "data": {
                "npc_id": npc_id,
                **data,
            },
        }
        
        # Route to handler
        if request_type == "dialogue":  # pragma: no cover - covered by AIDialogueHandler tests
            handler = AIDialogueHandler(self.config)
            return await handler.handle(synthetic_request)
        elif request_type == "decision":  # pragma: no cover - covered by AIDecisionHandler tests
            handler = AIDecisionHandler(self.config)
            return await handler.handle(synthetic_request)
        elif request_type == "emotion":  # pragma: no cover - covered by AIEmotionHandler tests
            handler = AIEmotionHandler(self.config)
            return await handler.handle(synthetic_request)
        else:
            # Custom requests return a placeholder
            return {
                "status": "ok",
                "message": f"Custom request processed for {npc_id}",
                "data": data,
            }


    async def handle_callback(
        self,
        request_id: int,
        callback_data: dict[str, Any]
    ) -> dict[str, Any]:
        """
        Handle callback from async request completion.
        
        Args:
            request_id: The database request ID
            callback_data: The callback data containing job status and result
            
        Returns:
            Processed callback response
        """
        job_id = callback_data.get("job_id")
        status = callback_data.get("status", "unknown")
        result = callback_data.get("result", {})
        
        self.logger.info(
            f"Handling callback for request {request_id}, "
            f"job_id={job_id}, status={status}"
        )
        
        # Find the async request by job_id if stored
        async_request_id = None
        for req_id, info in _async_requests.items():
            if info.get("job_id") == job_id:
                async_request_id = req_id
                break
        
        if async_request_id:
            # Update the async request status
            if status == "completed":
                _async_results[async_request_id] = {
                    "status": "completed",
                    "result": result,
                    "completed_at": datetime.utcnow().isoformat(),
                }
                _async_requests[async_request_id]["state"] = "completed"
            elif status == "failed":
                _async_results[async_request_id] = {
                    "status": "failed",
                    "error": callback_data.get("error", "Unknown error"),
                    "failed_at": datetime.utcnow().isoformat(),
                }
                _async_requests[async_request_id]["state"] = "failed"
        
        return {
            "status": "ok",
            "request_id": request_id,
            "job_id": job_id,
            "callback_status": status,
            "processed_at": datetime.utcnow().isoformat(),
        }


class AICheckResultHandler(BaseHandler):
    """
    Handler for checking async request results.
    
    Allows scripts to poll for completion of previously submitted
    async requests.
    """
    
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Check status of async request.
        
        Args:
            request: Request dictionary with async_request_id
            
        Returns:
            Status and result of async request
        """
        self.log_request_start(request)
        
        # Parse request data
        data = self.get_request_data(request)
        
        # Get async request ID
        async_request_id = data.get("async_request_id")
        if not async_request_id:
            raise ValidationError("async_request_id is required")
        
        # Check if request exists
        request_info = _async_requests.get(async_request_id)
        if not request_info:
            return {
                "status": "error",
                "async_request_id": async_request_id,
                "error": "Request not found or expired",
                "timestamp": datetime.utcnow().isoformat(),
            }
        
        # Build response
        response: dict[str, Any] = {
            "status": "ok",
            "async_request_id": async_request_id,
            "state": request_info["state"],
            "request_type": request_info["request_type"],
            "npc_id": request_info["npc_id"],
            "created_at": request_info["created_at"],
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        # Add result if completed or failed
        if async_request_id in _async_results:
            result_info = _async_results[async_request_id]
            if result_info["status"] == "completed":
                response["result"] = result_info["result"]
                response["completed_at"] = result_info["completed_at"]
            elif result_info["status"] == "failed":
                response["error"] = result_info["error"]
                response["failed_at"] = result_info["failed_at"]
        
        return response


def cleanup_expired_requests(max_age_seconds: int = 3600) -> int:
    """
    Clean up expired async requests from memory.
    
    Args:
        max_age_seconds: Maximum age of requests to keep
        
    Returns:
        Number of requests cleaned up
    """
    now = datetime.utcnow()
    expired_ids: list[str] = []
    
    for request_id, info in _async_requests.items():
        created_at = datetime.fromisoformat(info["created_at"])
        age = (now - created_at).total_seconds()
        if age > max_age_seconds:
            expired_ids.append(request_id)
    
    for request_id in expired_ids:
        _async_requests.pop(request_id, None)
        _async_results.pop(request_id, None)
    
    return len(expired_ids)


def get_pending_request_count() -> int:
    """Get count of pending async requests."""
    return sum(1 for r in _async_requests.values() if r["state"] == "pending")


def get_processing_request_count() -> int:
    """Get count of processing async requests."""
    return sum(1 for r in _async_requests.values() if r["state"] == "processing")


# Alias for backward compatibility with tests
AIAsyncHandler = AIAsyncRequestHandler