"""
Base Handler for AI IPC Service

This module defines the abstract base class for all request handlers.
Each handler implements the `handle` method to process specific types
of requests from the ai_requests table.

Usage:
    class MyHandler(BaseHandler):
        async def handle(self, request: dict) -> dict:
            # Process request
            return {"status": "ok", "data": ...}

Handler Lifecycle:
    1. Handler instantiated with config
    2. handle() called with request dict
    3. Response dict returned
    4. Response saved to database by processor
"""

from __future__ import annotations

import logging
from abc import ABC, abstractmethod
from datetime import datetime
from typing import Any

from ..config import Config


class HandlerError(Exception):
    """Base exception for handler errors."""
    
    def __init__(self, message: str, status_code: int = 500) -> None:
        super().__init__(message)
        self.message = message
        self.status_code = status_code


class ValidationError(HandlerError):
    """Raised when request validation fails."""
    
    def __init__(self, message: str) -> None:
        super().__init__(message, status_code=400)


class TimeoutError(HandlerError):
    """Raised when handler operation times out."""
    
    def __init__(self, message: str) -> None:
        super().__init__(message, status_code=504)


class BaseHandler(ABC):
    """
    Abstract base class for request handlers.
    
    All handlers must inherit from this class and implement the
    `handle` method. The handler receives the full request dictionary
    and must return a response dictionary.
    
    Attributes:
        config: Service configuration object
        logger: Logger instance for this handler
        
    Request Dictionary:
        The request parameter passed to handle() contains:
        - id: Request ID (int)
        - request_type: Type string (str)
        - endpoint: API endpoint path (str)
        - request_data: JSON payload as string (str)
        - status: Current status (str)
        - priority: Priority level 1-10 (int)
        - correlation_id: Optional correlation ID (str|None)
        - source_npc: NPC name that made request (str|None)
        - source_map: Map name (str|None)
        - created_at: Creation timestamp (datetime)
        - expires_at: Expiration timestamp (datetime|None)
        - retry_count: Number of retries (int)
        
    Response Dictionary:
        The returned response should be JSON-serializable and can
        contain any structure. Common fields include:
        - status: "ok" or "error"
        - data: Response payload
        - error: Error message (if failed)
    """
    
    def __init__(self, config: Config) -> None:
        """
        Initialize the handler.
        
        Args:
            config: Service configuration object
        """
        self.config = config
        self.logger = logging.getLogger(self.__class__.__name__)
    
    @abstractmethod
    async def handle(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Process a request and return a response.
        
        This method must be implemented by all handlers. It receives
        the full request dictionary from the database and should return
        a JSON-serializable response dictionary.
        
        Args:
            request: Request dictionary from ai_requests table
            
        Returns:
            Response dictionary to be stored in ai_responses
            
        Raises:
            HandlerError: On processing errors
            ValidationError: On invalid request data
        """
        pass
    
    def get_request_data(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Parse and return the request_data JSON field.
        
        Safely parses the JSON string from request_data field,
        returning an empty dict if parsing fails or field is empty.
        
        Args:
            request: Request dictionary
            
        Returns:
            Parsed request data as dictionary
        """
        import json
        
        data_str = request.get("request_data", "")
        if not data_str:
            return {}
        
        try:
            data = json.loads(data_str)
            if not isinstance(data, dict):
                self.logger.warning(
                    f"Request data is not a dict: {type(data).__name__}"
                )
                return {"data": data}
            return data
        except json.JSONDecodeError as e:
            self.logger.warning(f"Invalid JSON in request_data: {e}")
            return {}
    
    def get_endpoint(self, request: dict[str, Any]) -> str:
        """
        Extract the endpoint from the request.
        
        Args:
            request: Request dictionary
            
        Returns:
            Endpoint string (empty if not present)
        """
        return request.get("endpoint", "") or ""
    
    def get_request_id(self, request: dict[str, Any]) -> int:
        """
        Extract the request ID.
        
        Args:
            request: Request dictionary
            
        Returns:
            Request ID
        """
        return int(request.get("id", 0))
    
    def get_source_info(self, request: dict[str, Any]) -> dict[str, Any]:
        """
        Extract source information (NPC, map) from request.
        
        Args:
            request: Request dictionary
            
        Returns:
            Dictionary with source_npc and source_map
        """
        return {
            "source_npc": request.get("source_npc"),
            "source_map": request.get("source_map"),
            "correlation_id": request.get("correlation_id"),
        }
    
    def create_success_response(
        self, 
        data: Any = None, 
        **extra: Any
    ) -> dict[str, Any]:
        """
        Create a standard success response.
        
        Args:
            data: Response payload
            **extra: Additional fields to include
            
        Returns:
            Formatted success response dictionary
        """
        response = {
            "status": "ok",
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        if data is not None:
            response["data"] = data
            
        response.update(extra)
        return response
    
    def create_error_response(
        self, 
        error: str, 
        code: int = 500,
        **extra: Any
    ) -> dict[str, Any]:
        """
        Create a standard error response.
        
        Args:
            error: Error message
            code: Error code
            **extra: Additional fields to include
            
        Returns:
            Formatted error response dictionary
        """
        response = {
            "status": "error",
            "error": error,
            "code": code,
            "timestamp": datetime.utcnow().isoformat(),
        }
        
        response.update(extra)
        return response
    
    def validate_required_fields(
        self, 
        data: dict[str, Any], 
        fields: list[str]
    ) -> None:
        """
        Validate that required fields are present in data.
        
        Args:
            data: Data dictionary to validate
            fields: List of required field names
            
        Raises:
            ValidationError: If any required field is missing
        """
        missing = [f for f in fields if f not in data or data[f] is None]
        if missing:
            raise ValidationError(
                f"Missing required fields: {', '.join(missing)}"
            )
    
    def log_request_start(self, request: dict[str, Any]) -> None:
        """
        Log the start of request processing.
        
        Args:
            request: Request dictionary
        """
        self.logger.info(
            f"Processing request {request.get('id')} "
            f"type={request.get('request_type')} "
            f"endpoint={request.get('endpoint')} "
            f"npc={request.get('source_npc')} "
            f"map={request.get('source_map')}"
        )
    
    def log_request_complete(
        self, 
        request: dict[str, Any], 
        processing_time_ms: int
    ) -> None:
        """
        Log the completion of request processing.
        
        Args:
            request: Request dictionary
            processing_time_ms: Processing time in milliseconds
        """
        self.logger.info(
            f"Completed request {request.get('id')} "
            f"in {processing_time_ms}ms"
        )