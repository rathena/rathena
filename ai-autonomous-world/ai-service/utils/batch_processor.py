"""
Batch Processing Utilities for AI Service
Phase 8A: Request batching for autonomous NPC updates
Processes multiple NPC updates in single AI calls for 5-10x performance improvement
"""

import asyncio
import logging
from typing import List, Dict, Any, Callable, Optional
from datetime import datetime, timedelta
from collections import defaultdict

logger = logging.getLogger(__name__)


class BatchProcessor:
    """
    Batches multiple requests and processes them together
    Reduces AI service calls by grouping similar operations
    """
    
    def __init__(
        self,
        batch_size: int = 10,
        max_wait_time: float = 1.0,
        processor_func: Optional[Callable] = None
    ):
        """
        Initialize batch processor
        
        Args:
            batch_size: Maximum number of items in a batch
            max_wait_time: Maximum time to wait before processing batch (seconds)
            processor_func: Async function to process batched items
        """
        self.batch_size = batch_size
        self.max_wait_time = max_wait_time
        self.processor_func = processor_func
        
        self._batches: Dict[str, List[Dict[str, Any]]] = defaultdict(list)
        self._batch_times: Dict[str, datetime] = {}
        self._locks: Dict[str, asyncio.Lock] = defaultdict(asyncio.Lock)
        self._processing_tasks: Dict[str, asyncio.Task] = {}
        
        logger.info(f"BatchProcessor initialized (batch_size={batch_size}, max_wait={max_wait_time}s)")
    
    async def add_to_batch(
        self,
        batch_key: str,
        item: Dict[str, Any],
        processor_func: Optional[Callable] = None
    ) -> Any:
        """
        Add item to batch and process when batch is full or timeout reached
        
        Args:
            batch_key: Key to group related items (e.g., 'npc_movement', 'npc_dialogue')
            item: Item to add to batch
            processor_func: Optional processor function (overrides default)
        
        Returns:
            Result from batch processing
        """
        async with self._locks[batch_key]:
            # Add item to batch
            self._batches[batch_key].append(item)
            
            # Set batch start time if this is first item
            if batch_key not in self._batch_times:
                from datetime import UTC
                self._batch_times[batch_key] = datetime.now(UTC)
            
            batch = self._batches[batch_key]
            batch_age = (datetime.now(UTC) - self._batch_times[batch_key]).total_seconds()
            
            logger.debug(f"[BATCH] {batch_key}: {len(batch)}/{self.batch_size} items (age: {batch_age:.2f}s)")
            
            # Process batch if full or timeout reached
            should_process = (
                len(batch) >= self.batch_size or
                batch_age >= self.max_wait_time
            )
            
            if should_process:
                return await self._process_batch(batch_key, processor_func)
            else:
                # Schedule delayed processing if not already scheduled
                if batch_key not in self._processing_tasks or self._processing_tasks[batch_key].done():
                    self._processing_tasks[batch_key] = asyncio.create_task(
                        self._delayed_process(batch_key, processor_func)
                    )
                return None
    
    async def _delayed_process(
        self,
        batch_key: str,
        processor_func: Optional[Callable] = None
    ):
        """
        Wait for max_wait_time then process batch
        """
        await asyncio.sleep(self.max_wait_time)
        
        async with self._locks[batch_key]:
            if self._batches[batch_key]:
                await self._process_batch(batch_key, processor_func)
    
    async def _process_batch(
        self,
        batch_key: str,
        processor_func: Optional[Callable] = None
    ) -> List[Any]:
        """
        Process all items in batch
        
        Args:
            batch_key: Batch identifier
            processor_func: Function to process batch
        
        Returns:
            List of results from processing
        """
        batch = self._batches[batch_key]
        if not batch:
            return []
        
        # Get processor function
        func = processor_func or self.processor_func
        if not func:
            logger.error(f"[BATCH] No processor function for {batch_key}")
            return []
        
        try:
            logger.info(f"[BATCH PROCESS] {batch_key}: Processing {len(batch)} items")
            from datetime import UTC
            start_time = datetime.now(UTC)
            
            # Process batch
            results = await func(batch)
            
            # Calculate performance metrics
            duration = (datetime.now(UTC) - start_time).total_seconds()
            items_per_sec = len(batch) / duration if duration > 0 else 0
            
            logger.info(
                f"[BATCH COMPLETE] {batch_key}: {len(batch)} items in {duration:.2f}s "
                f"({items_per_sec:.1f} items/sec)"
            )
            
            # Clear batch
            self._batches[batch_key] = []
            if batch_key in self._batch_times:
                del self._batch_times[batch_key]
            
            return results
            
        except Exception as e:
            logger.error(f"[BATCH ERROR] {batch_key}: {e}", exc_info=True)
            # Clear batch even on error
            self._batches[batch_key] = []
            if batch_key in self._batch_times:
                del self._batch_times[batch_key]
            return []
    
    async def flush_all(self) -> Dict[str, List[Any]]:
        """
        Force process all pending batches
        
        Returns:
            Dictionary of batch_key -> results
        """
        results = {}
        for batch_key in list(self._batches.keys()):
            async with self._locks[batch_key]:
                if self._batches[batch_key]:
                    results[batch_key] = await self._process_batch(batch_key)
        return results
    
    def get_stats(self) -> Dict[str, Any]:
        """Get batch processor statistics"""
        return {
            'active_batches': len(self._batches),
            'batch_sizes': {k: len(v) for k, v in self._batches.items()},
            'total_pending': sum(len(v) for v in self._batches.values())
        }

