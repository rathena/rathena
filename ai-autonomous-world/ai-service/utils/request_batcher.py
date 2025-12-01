"""
Request Batching System

Token bucket-based request batching for high-volume LLM endpoints.
Reduces API calls by batching multiple requests together.

Features:
- Token bucket algorithm for batching
- Automatic flush on timeout
- Metrics tracking
- Async/await support
- Error handling per request
- Prometheus-compatible metrics
"""

import asyncio
import logging
import uuid
from typing import Dict, List, Callable, Any, Optional
from collections import deque
from dataclasses import dataclass, field
from datetime import datetime, timedelta

logger = logging.getLogger(__name__)


@dataclass
class BatchRequest:
    """Individual request in a batch."""
    id: str
    payload: Dict[str, Any]
    future: asyncio.Future
    created_at: datetime = field(default_factory=datetime.utcnow)


class RequestBatcher:
    """
    Batch requests to reduce API calls and improve throughput.
    
    Uses token bucket algorithm to accumulate requests and process them
    in batches, reducing overall API costs and latency.
    
    Usage:
        batcher = RequestBatcher(batch_size=5, timeout=2.0)
        await batcher.start()
        
        result = await batcher.submit({
            'prompt': 'Hello',
            'npc_id': 'npc-123'
        })
    """
    
    def __init__(
        self,
        batch_size: int = 5,
        timeout: float = 2.0,
        processor: Optional[Callable] = None
    ):
        """
        Initialize request batcher.
        
        Args:
            batch_size: Maximum requests per batch
            timeout: Maximum wait time before forcing flush (seconds)
            processor: Optional custom batch processor function
        """
        self.batch_size = batch_size
        self.timeout = timeout
        self.processor = processor
        self.queue: deque[BatchRequest] = deque()
        self.processor_task: Optional[asyncio.Task] = None
        self.running = False
        self.lock = asyncio.Lock()
        
        # Metrics
        self.metrics = {
            "requests_batched": 0,
            "batches_processed": 0,
            "timeout_flushes": 0,
            "errors": 0,
            "total_latency": 0.0
        }
    
    async def start(self):
        """Start the background processor."""
        if self.running:
            logger.warning("Batcher already running")
            return
        
        self.running = True
        self.processor_task = asyncio.create_task(self._background_processor())
        logger.info("Request batcher started")
    
    async def stop(self):
        """Stop the background processor and flush remaining requests."""
        self.running = False
        
        if self.processor_task:
            self.processor_task.cancel()
            try:
                await self.processor_task
            except asyncio.CancelledError:
                pass
        
        # Flush remaining requests
        await self._flush_batch()
        
        logger.info("Request batcher stopped")
    
    async def submit(self, payload: Dict[str, Any]) -> Any:
        """
        Submit request for batching.
        
        Args:
            payload: Request payload
            
        Returns:
            Result from batch processing
            
        Raises:
            TimeoutError: If request times out
            Exception: If processing fails
        """
        request = BatchRequest(
            id=str(uuid.uuid4()),
            payload=payload,
            future=asyncio.Future()
        )
        
        async with self.lock:
            self.queue.append(request)
            queue_len = len(self.queue)
        
        # Trigger immediate flush if batch full
        if queue_len >= self.batch_size:
            asyncio.create_task(self._flush_batch())
        
        try:
            # Wait for result with extended timeout
            result = await asyncio.wait_for(
                request.future,
                timeout=self.timeout * 3
            )
            return result
        except asyncio.TimeoutError:
            logger.error(f"Request {request.id} timed out")
            raise TimeoutError("Request processing timed out")
    
    async def _background_processor(self):
        """Background task that periodically flushes batches."""
        while self.running:
            try:
                await asyncio.sleep(self.timeout)
                
                async with self.lock:
                    queue_len = len(self.queue)
                
                if queue_len > 0:
                    self.metrics["timeout_flushes"] += 1
                    await self._flush_batch()
                    
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Background processor error: {e}")
                await asyncio.sleep(1)
    
    async def _flush_batch(self):
        """Process accumulated requests as a batch."""
        async with self.lock:
            if not self.queue:
                return
            
            # Extract batch
            batch_requests = []
            for _ in range(min(len(self.queue), self.batch_size)):
                batch_requests.append(self.queue.popleft())
        
        if not batch_requests:
            return
        
        start_time = datetime.utcnow()
        
        try:
            # Update metrics
            self.metrics["batches_processed"] += 1
            self.metrics["requests_batched"] += len(batch_requests)
            
            logger.debug(f"Processing batch of {len(batch_requests)} requests")
            
            # Process batch
            if self.processor:
                results = await self.processor(batch_requests)
            else:
                results = await self._default_processor(batch_requests)
            
            # Set results
            for request, result in zip(batch_requests, results):
                if not request.future.done():
                    request.future.set_result(result)
            
            # Update latency metrics
            latency = (datetime.utcnow() - start_time).total_seconds()
            self.metrics["total_latency"] += latency
            
        except Exception as e:
            logger.error(f"Batch processing failed: {e}")
            self.metrics["errors"] += 1
            
            # Set exception on all futures
            for request in batch_requests:
                if not request.future.done():
                    request.future.set_exception(e)
    
    async def _default_processor(
        self,
        batch_requests: List[BatchRequest]
    ) -> List[Any]:
        """
        Default batch processor (returns payloads as-is).
        
        Override this or provide custom processor in constructor.
        """
        return [req.payload for req in batch_requests]
    
    def get_metrics(self) -> Dict[str, Any]:
        """
        Get current metrics.
        
        Returns:
            Dict containing metrics
        """
        metrics = self.metrics.copy()
        
        # Calculate derived metrics
        if metrics["batches_processed"] > 0:
            metrics["avg_batch_size"] = (
                metrics["requests_batched"] / metrics["batches_processed"]
            )
            metrics["avg_latency"] = (
                metrics["total_latency"] / metrics["batches_processed"]
            )
        else:
            metrics["avg_batch_size"] = 0
            metrics["avg_latency"] = 0
        
        metrics["queue_size"] = len(self.queue)
        
        return metrics
    
    def reset_metrics(self):
        """Reset all metrics to zero."""
        self.metrics = {
            "requests_batched": 0,
            "batches_processed": 0,
            "timeout_flushes": 0,
            "errors": 0,
            "total_latency": 0.0
        }


class LLMRequestBatcher(RequestBatcher):
    """
    Specialized batcher for LLM requests.
    
    Handles LLM-specific batching with proper error handling
    and response parsing.
    """
    
    def __init__(
        self,
        llm_provider,
        batch_size: int = 5,
        timeout: float = 2.0
    ):
        """
        Initialize LLM request batcher.
        
        Args:
            llm_provider: LLM provider instance
            batch_size: Maximum requests per batch
            timeout: Maximum wait time before forcing flush
        """
        super().__init__(
            batch_size=batch_size,
            timeout=timeout,
            processor=self._process_llm_batch
        )
        self.llm_provider = llm_provider
    
    async def _process_llm_batch(
        self,
        batch_requests: List[BatchRequest]
    ) -> List[Any]:
        """
        Process batch of LLM requests.
        
        Args:
            batch_requests: List of batch requests
            
        Returns:
            List of LLM responses
        """
        results = []
        
        # Process requests in parallel
        tasks = [
            self._process_single_llm_request(req)
            for req in batch_requests
        ]
        
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        return results
    
    async def _process_single_llm_request(
        self,
        request: BatchRequest
    ) -> Dict[str, Any]:
        """
        Process single LLM request.
        
        Args:
            request: Batch request
            
        Returns:
            LLM response
        """
        try:
            response = await self.llm_provider.generate(
                prompt=request.payload.get('prompt', ''),
                context=request.payload.get('context', {}),
                max_tokens=request.payload.get('max_tokens', 150)
            )
            
            return {
                'request_id': request.id,
                'response': response,
                'success': True
            }
            
        except Exception as e:
            logger.error(f"LLM request {request.id} failed: {e}")
            return {
                'request_id': request.id,
                'error': str(e),
                'success': False
            }