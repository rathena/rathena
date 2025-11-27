"""
Request Processor for AI IPC Service

This module contains the RequestProcessor class which handles:
- Fetching pending requests from the database
- Routing requests to appropriate handlers
- Managing concurrent request processing
- Storing responses back to the database

The processor runs as the main polling loop, continuously checking
for new requests and processing them in parallel.

Architecture:
    1. Poll database for pending requests (batch)
    2. Mark requests as 'processing'
    3. Route each request to handler based on request_type
    4. Execute handlers concurrently
    5. Store responses and update request status

Error Handling:
    - Individual request failures don't affect other requests
    - Failed requests are marked with error details
    - Retryable errors increment retry count
    - Non-retryable errors mark request as failed
"""

from __future__ import annotations

import asyncio
import json
import logging
import time
from datetime import datetime
from typing import Any

from .config import Config
from .database import Database, DatabaseError, QueryError
from .handlers import (
    BaseHandler,
    HealthCheckHandler,
    HttpProxyHandler,
    AIDialogueHandler,
    HANDLER_REGISTRY,
    get_handler,
)
from .handlers.base import HandlerError, ValidationError
from .handlers.base_handler import BaseHandler as AuthBaseHandler

logger = logging.getLogger(__name__)


class ProcessorError(Exception):
    """Base exception for processor errors."""
    pass


class RequestProcessor:
    """
    Main request processor for the AI IPC service.
    
    Manages the polling loop, request routing, and concurrent
    processing of AI requests from the database queue.
    
    Attributes:
        db: Database interface instance
        config: Service configuration
        handlers: Dictionary of initialized handlers
        running: Flag indicating if processor is active
        stats: Processing statistics
    """
    
    def __init__(self, db: Database, config: Config) -> None:
        """
        Initialize the request processor.
        
        Args:
            db: Database interface for request/response operations
            config: Service configuration
        """
        self.db = db
        self.config = config
        self._handlers: dict[str, BaseHandler] = {}
        self._running = False
        self._semaphore: asyncio.Semaphore | None = None
        
        # Processing statistics
        self.stats = ProcessingStats()
        
        # Initialize security configuration for handlers
        self._init_security()
        
        # Initialize handlers
        self._init_handlers()
    
    def _init_security(self) -> None:
        """
        Initialize security configuration for all handlers.
        
        Sets the shared security config on the BaseHandler class
        so all handler instances have access to authentication and
        rate limiting settings.
        """
        if hasattr(self.config, 'security'):
            logger.info("Initializing security configuration for handlers")
            AuthBaseHandler.set_security_config(self.config.security)
            logger.info(
                f"Security: auth_enabled={self.config.security.auth_enabled}, "
                f"auth_method={self.config.security.auth_method}, "
                f"rate_limit_enabled={self.config.security.rate_limit_enabled}"
            )
        else:
            logger.warning(
                "No security configuration found. "
                "Authentication and rate limiting disabled."
            )
    
    def _init_handlers(self) -> None:
        """
        Initialize handlers for each request type.
        
        Creates handler instances from the registry for all
        supported request types.
        """
        logger.info("Initializing request handlers")
        
        # Create unique handler instances
        handler_classes_seen: set[type] = set()
        
        for request_type, handler_class in HANDLER_REGISTRY.items():
            if handler_class not in handler_classes_seen:
                handler_classes_seen.add(handler_class)
                logger.debug(f"Creating handler: {handler_class.__name__}")
            
            # Create handler instance for this request type
            self._handlers[request_type] = handler_class(self.config)
        
        logger.info(
            f"Initialized {len(handler_classes_seen)} unique handlers "
            f"for {len(self._handlers)} request types"
        )
    
    def get_handler(self, request_type: str) -> BaseHandler | None:
        """
        Get handler for a specific request type.
        
        Args:
            request_type: The type of request
            
        Returns:
            Handler instance or None if not found
        """
        # Try exact match first
        handler = self._handlers.get(request_type.lower())
        if handler:
            return handler
        
        # Try creating new handler from registry
        return get_handler(request_type, self.config)
    
    async def process_batch(self) -> int:
        """
        Fetch and process a batch of pending requests.
        
        This is the main processing method called by the polling loop.
        It uses the atomic fetch_and_mark_processing to avoid race conditions
        where requests could get stuck if the service crashes between
        fetching and marking.
        
        Returns:
            Number of requests processed
        """
        try:
            # Atomically fetch and mark requests as processing
            # This prevents race conditions where requests get stuck
            # if the service crashes between SELECT and UPDATE
            requests = await self.db.fetch_and_mark_processing(
                limit=self.config.polling.batch_size
            )
            
            if not requests:
                return 0
            
            logger.info(f"Processing batch of {len(requests)} requests")
            self.stats.batches_processed += 1
            
            # Process all requests concurrently
            tasks = [
                self._process_one_safe(request)
                for request in requests
            ]
            
            results = await asyncio.gather(*tasks, return_exceptions=True)
            
            # Count successes and failures
            successes = sum(1 for r in results if r is True)
            failures = len(results) - successes
            
            self.stats.requests_processed += successes
            self.stats.requests_failed += failures
            
            logger.info(
                f"Batch complete: {successes} succeeded, {failures} failed"
            )
            
            return len(requests)
            
        except DatabaseError as e:
            logger.error(f"Database error during batch processing: {e}")
            raise ProcessorError(f"Batch processing failed: {e}") from e
    
    async def _process_one_safe(self, request: dict[str, Any]) -> bool:
        """
        Process a single request with error handling.
        
        Wraps _process_one with try/except to ensure individual
        request failures don't affect other requests.
        
        Args:
            request: Request dictionary from database
            
        Returns:
            True if processed successfully, False otherwise
        """
        request_id = request.get('id', 'unknown')
        
        try:
            await self._process_one(request)
            return True
        except Exception as e:
            logger.error(
                f"Unhandled error processing request {request_id}: {e}",
                exc_info=True
            )
            try:
                await self.db.mark_failed(
                    request_id=int(request_id),
                    error_message=f"Unhandled error: {str(e)[:400]}",
                    should_retry=False
                )
            except Exception as db_error:
                logger.error(
                    f"Failed to mark request {request_id} as failed: {db_error}"
                )
            return False
    
    async def _process_one(self, request: dict[str, Any]) -> None:
        """
        Process a single request.
        
        Routes the request to the appropriate handler, executes it,
        and stores the response.
        
        Args:
            request: Request dictionary from database
            
        Raises:
            Various exceptions on processing errors
        """
        request_id = request['id']
        request_type = request.get('request_type', 'unknown')
        start_time = time.time()
        
        logger.info(
            f"Processing request {request_id} "
            f"type={request_type} "
            f"endpoint={request.get('endpoint')} "
            f"npc={request.get('source_npc')}"
        )
        
        # Get handler for this request type
        handler = self.get_handler(request_type)
        
        if handler is None:
            error_msg = f"No handler for request type: {request_type}"
            logger.warning(f"Request {request_id}: {error_msg}")
            await self.db.mark_failed(
                request_id=request_id,
                error_message=error_msg,
                should_retry=False
            )
            return
        
        try:
            # Execute handler
            response = await handler.handle(request)
            
            # Calculate processing time
            processing_time_ms = int((time.time() - start_time) * 1000)
            
            # Serialize response
            response_json = json.dumps(response, ensure_ascii=False, default=str)
            
            # Store response
            await self.db.save_response(
                request_id=request_id,
                response_data=response_json,
                http_status=200,
                error_message=None,
                processing_time_ms=processing_time_ms
            )
            
            logger.info(
                f"Request {request_id} completed in {processing_time_ms}ms"
            )
            
        except ValidationError as e:
            # Client error - don't retry
            processing_time_ms = int((time.time() - start_time) * 1000)
            logger.warning(f"Request {request_id} validation error: {e}")
            
            error_response = json.dumps({
                "status": "error",
                "error": str(e),
                "code": e.status_code,
            })
            
            await self.db.save_response(
                request_id=request_id,
                response_data=error_response,
                http_status=e.status_code,
                error_message=str(e),
                processing_time_ms=processing_time_ms
            )
            
        except HandlerError as e:
            # Handler error - may be retryable
            processing_time_ms = int((time.time() - start_time) * 1000)
            logger.error(f"Request {request_id} handler error: {e}")
            
            retry_count = request.get('retry_count', 0)
            should_retry = retry_count < self.config.polling.max_retries
            
            if should_retry:
                logger.info(
                    f"Request {request_id} will be retried "
                    f"(attempt {retry_count + 1}/{self.config.polling.max_retries})"
                )
                await self.db.mark_failed(
                    request_id=request_id,
                    error_message=str(e),
                    should_retry=True
                )
            else:
                error_response = json.dumps({
                    "status": "error",
                    "error": str(e),
                    "code": e.status_code,
                    "retries_exhausted": True,
                })
                
                await self.db.save_response(
                    request_id=request_id,
                    response_data=error_response,
                    http_status=e.status_code,
                    error_message=str(e),
                    processing_time_ms=processing_time_ms
                )
    
    async def cleanup_expired(self) -> int:
        """
        Clean up expired requests.
        
        Marks pending requests that have passed their expiration
        time as 'timeout'.
        
        Returns:
            Number of requests cleaned up
        """
        try:
            count = await self.db.cleanup_expired()
            if count > 0:
                logger.info(f"Cleaned up {count} expired requests")
                self.stats.requests_expired += count
            return count
        except DatabaseError as e:
            logger.error(f"Failed to cleanup expired requests: {e}")
            return 0
    
    async def cleanup_stuck(self, stuck_threshold_seconds: int = 300) -> int:
        """
        Clean up requests stuck in 'processing' state.
        
        This handles the case where a worker crashes after marking requests
        as processing but before completing them. Such requests would be
        stuck forever without this cleanup.
        
        Args:
            stuck_threshold_seconds: How long before a processing request
                                    is considered stuck (default: 5 minutes)
        
        Returns:
            Number of requests cleaned up
        """
        try:
            count = await self.db.cleanup_stuck_processing(stuck_threshold_seconds)
            if count > 0:
                logger.info(f"Recovered {count} stuck requests")
                self.stats.requests_failed += count
            return count
        except DatabaseError as e:
            logger.error(f"Failed to cleanup stuck requests: {e}")
            return 0
    
    @property
    def is_running(self) -> bool:
        """Check if processor is running."""
        return self._running
    
    def start(self) -> None:
        """Mark processor as running."""
        self._running = True
        self._semaphore = asyncio.Semaphore(self.config.polling.worker_count)
        logger.info("Request processor started")
    
    def stop(self) -> None:
        """Mark processor as stopped."""
        self._running = False
        logger.info("Request processor stopped")
    
    def get_stats(self) -> dict[str, Any]:
        """
        Get processing statistics.
        
        Returns:
            Dictionary of processing statistics
        """
        return self.stats.to_dict()


class ProcessingStats:
    """
    Track processing statistics for monitoring.
    
    Attributes:
        requests_processed: Total successful requests
        requests_failed: Total failed requests
        requests_expired: Total expired requests
        batches_processed: Total batches processed
        start_time: When stats tracking started
    """
    
    def __init__(self) -> None:
        """Initialize statistics tracking."""
        self.requests_processed: int = 0
        self.requests_failed: int = 0
        self.requests_expired: int = 0
        self.batches_processed: int = 0
        self.start_time: datetime = datetime.utcnow()
    
    @property
    def uptime_seconds(self) -> float:
        """Get uptime in seconds."""
        return (datetime.utcnow() - self.start_time).total_seconds()
    
    @property
    def requests_per_second(self) -> float:
        """Calculate average requests per second."""
        uptime = self.uptime_seconds
        if uptime > 0:
            return self.requests_processed / uptime
        return 0.0
    
    def to_dict(self) -> dict[str, Any]:
        """
        Convert statistics to dictionary.
        
        Returns:
            Dictionary of all statistics
        """
        return {
            "requests_processed": self.requests_processed,
            "requests_failed": self.requests_failed,
            "requests_expired": self.requests_expired,
            "batches_processed": self.batches_processed,
            "uptime_seconds": int(self.uptime_seconds),
            "requests_per_second": round(self.requests_per_second, 2),
            "start_time": self.start_time.isoformat(),
        }
    
    def reset(self) -> None:
        """Reset all statistics."""
        self.requests_processed = 0
        self.requests_failed = 0
        self.requests_expired = 0
        self.batches_processed = 0
        self.start_time = datetime.utcnow()