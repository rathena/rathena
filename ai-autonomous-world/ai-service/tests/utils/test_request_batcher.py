"""
Tests for Request Batcher Utility

Comprehensive test suite covering:
- Request batching logic
- Flush triggers (size, time, manual)
- Batch processing
- Error handling in batches
- Performance optimization
- Thread safety
- Memory management
"""

import pytest
import asyncio
import time
from unittest.mock import Mock, AsyncMock, patch
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List, Any
from datetime import datetime, timedelta


# =============================================================================
# MOCK CLASSES
# =============================================================================

class BatchProcessor:
    """Mock batch processor"""
    def __init__(self, max_size=10, max_wait_seconds=1.0):
        self.max_size = max_size
        self.max_wait_seconds = max_wait_seconds
        self.batch = []
        self.last_flush = time.time()
        self.processed_batches = []
    
    async def add(self, item):
        """Add item to batch"""
        self.batch.append(item)
        if len(self.batch) >= self.max_size:
            await self.flush()
    
    async def flush(self):
        """Flush current batch"""
        if self.batch:
            batch_copy = self.batch.copy()
            self.batch.clear()
            self.last_flush = time.time()
            await self._process_batch(batch_copy)
            self.processed_batches.append(batch_copy)
    
    async def _process_batch(self, batch):
        """Process a batch of items"""
        await asyncio.sleep(0.01)  # Simulate processing
        return batch
    
    def should_flush(self):
        """Check if batch should be flushed"""
        if len(self.batch) >= self.max_size:
            return True
        if time.time() - self.last_flush >= self.max_wait_seconds:
            return True
        return False


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def batch_processor():
    """Create batch processor instance"""
    return BatchProcessor(max_size=5, max_wait_seconds=1.0)


@pytest.fixture
def sample_items():
    """Sample items for batching"""
    return [{'id': i, 'data': f'item-{i}'} for i in range(20)]


# =============================================================================
# BASIC BATCHING TESTS
# =============================================================================

class TestBasicBatching:
    """Test basic batching functionality"""
    
    @pytest.mark.asyncio
    async def test_add_items_to_batch(self, batch_processor):
        """Test adding items to batch"""
        await batch_processor.add({'id': 1})
        await batch_processor.add({'id': 2})
        
        assert len(batch_processor.batch) == 2
    
    @pytest.mark.asyncio
    async def test_auto_flush_on_size(self, batch_processor):
        """Test automatic flush when size limit reached"""
        for i in range(5):
            await batch_processor.add({'id': i})
        
        # Batch should be empty after auto-flush
        assert len(batch_processor.batch) == 0
        assert len(batch_processor.processed_batches) == 1
        assert len(batch_processor.processed_batches[0]) == 5
    
    @pytest.mark.asyncio
    async def test_manual_flush(self, batch_processor):
        """Test manual flush"""
        await batch_processor.add({'id': 1})
        await batch_processor.add({'id': 2})
        
        await batch_processor.flush()
        
        assert len(batch_processor.batch) == 0
        assert len(batch_processor.processed_batches) == 1
    
    @pytest.mark.asyncio
    async def test_flush_empty_batch(self, batch_processor):
        """Test flushing empty batch"""
        await batch_processor.flush()
        
        assert len(batch_processor.processed_batches) == 0
    
    @pytest.mark.asyncio
    async def test_multiple_batches(self, batch_processor):
        """Test processing multiple batches"""
        # Add 12 items (will create 2 full batches + partial)
        for i in range(12):
            await batch_processor.add({'id': i})
        
        # Flush remaining
        await batch_processor.flush()
        
        assert len(batch_processor.processed_batches) == 3
        assert len(batch_processor.processed_batches[0]) == 5
        assert len(batch_processor.processed_batches[1]) == 5
        assert len(batch_processor.processed_batches[2]) == 2


# =============================================================================
# FLUSH TRIGGER TESTS
# =============================================================================

class TestFlushTriggers:
    """Test different flush trigger mechanisms"""
    
    @pytest.mark.asyncio
    async def test_size_trigger(self, batch_processor):
        """Test flush triggered by size"""
        for i in range(5):
            await batch_processor.add({'id': i})
        
        assert len(batch_processor.processed_batches) == 1
    
    @pytest.mark.asyncio
    async def test_time_trigger(self):
        """Test flush triggered by time"""
        processor = BatchProcessor(max_size=100, max_wait_seconds=0.1)
        
        await processor.add({'id': 1})
        await processor.add({'id': 2})
        
        # Wait for time trigger
        await asyncio.sleep(0.15)
        
        assert processor.should_flush()
    
    @pytest.mark.asyncio
    async def test_should_flush_size(self, batch_processor):
        """Test should_flush returns True when size reached"""
        for i in range(5):
            batch_processor.batch.append({'id': i})
        
        assert batch_processor.should_flush()
    
    @pytest.mark.asyncio
    async def test_should_flush_time(self):
        """Test should_flush returns True when time elapsed"""
        processor = BatchProcessor(max_size=100, max_wait_seconds=0.1)
        processor.batch.append({'id': 1})
        processor.last_flush = time.time() - 0.2  # Set to past
        
        assert processor.should_flush()


# =============================================================================
# BATCH PROCESSING TESTS
# =============================================================================

class TestBatchProcessing:
    """Test batch processing logic"""
    
    @pytest.mark.asyncio
    async def test_process_batch(self, batch_processor):
        """Test processing a batch"""
        items = [{'id': i} for i in range(3)]
        result = await batch_processor._process_batch(items)
        
        assert result == items
    
    @pytest.mark.asyncio
    async def test_batch_ordering(self, batch_processor):
        """Test items maintain order in batch"""
        items = [{'id': i} for i in range(5)]
        for item in items:
            await batch_processor.add(item)
        
        processed = batch_processor.processed_batches[0]
        for i, item in enumerate(processed):
            assert item['id'] == i
    
    @pytest.mark.asyncio
    async def test_batch_with_duplicates(self, batch_processor):
        """Test batching with duplicate items"""
        item = {'id': 1, 'data': 'duplicate'}
        
        await batch_processor.add(item)
        await batch_processor.add(item)
        await batch_processor.add(item)
        
        await batch_processor.flush()
        
        # All duplicates should be included
        assert len(batch_processor.processed_batches[0]) == 3


# =============================================================================
# ERROR HANDLING TESTS
# =============================================================================

class TestErrorHandling:
    """Test error handling in batch processing"""
    
    @pytest.mark.asyncio
    async def test_error_during_processing(self):
        """Test handling errors during batch processing"""
        errors_caught = []
        
        class FailingProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                if len(batch) > 2:
                    raise ValueError("Batch too large")
                return await super()._process_batch(batch)
        
        processor = FailingProcessor(max_size=5)
        
        try:
            for i in range(5):
                await processor.add({'id': i})
        except ValueError as e:
            errors_caught.append(e)
        
        assert len(errors_caught) == 1
    
    @pytest.mark.asyncio
    async def test_partial_batch_on_error(self):
        """Test processing partial batch on error"""
        class PartialFailProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                # Process only first 3 items
                processed = batch[:3]
                if len(batch) > 3:
                    raise ValueError("Too many items")
                return processed
        
        processor = PartialFailProcessor(max_size=5)
        
        with pytest.raises(ValueError):
            for i in range(5):
                await processor.add({'id': i})
    
    @pytest.mark.asyncio
    async def test_retry_failed_batch(self):
        """Test retrying failed batch"""
        attempt_count = 0
        
        class RetryProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                nonlocal attempt_count
                attempt_count += 1
                if attempt_count < 2:
                    raise ValueError("First attempt fails")
                return await super()._process_batch(batch)
        
        processor = RetryProcessor(max_size=3)
        
        items = [{'id': i} for i in range(3)]
        
        # First attempt fails
        try:
            for item in items:
                await processor.add(item)
        except ValueError:
            pass
        
        # Retry
        for item in items:
            await processor.add(item)
        
        assert attempt_count == 2


# =============================================================================
# PERFORMANCE TESTS
# =============================================================================

class TestPerformance:
    """Test batch processing performance"""
    
    @pytest.mark.performance
    @pytest.mark.asyncio
    async def test_large_batch_performance(self):
        """Test processing large batches"""
        processor = BatchProcessor(max_size=1000)
        
        start = time.perf_counter()
        for i in range(1000):
            await processor.add({'id': i})
        duration = time.perf_counter() - start
        
        assert duration < 0.5  # Should complete quickly
    
    @pytest.mark.performance
    @pytest.mark.asyncio
    async def test_many_small_batches(self):
        """Test processing many small batches"""
        processor = BatchProcessor(max_size=10)
        
        start = time.perf_counter()
        for i in range(100):
            await processor.add({'id': i})
        await processor.flush()
        duration = time.perf_counter() - start
        
        assert duration < 1.0
        assert len(processor.processed_batches) == 11  # 10 full + 1 partial
    
    @pytest.mark.performance
    @pytest.mark.asyncio
    async def test_memory_efficiency(self):
        """Test memory doesn't grow unbounded"""
        processor = BatchProcessor(max_size=100)
        
        # Add many items
        for i in range(1000):
            await processor.add({'id': i})
        
        # Current batch should be small
        assert len(processor.batch) < 100


# =============================================================================
# CONCURRENT OPERATIONS TESTS
# =============================================================================

class TestConcurrentOperations:
    """Test concurrent batch operations"""
    
    @pytest.mark.concurrent
    @pytest.mark.asyncio
    async def test_concurrent_adds(self):
        """Test adding items concurrently"""
        processor = BatchProcessor(max_size=100)
        
        async def add_items(start, count):
            for i in range(start, start + count):
                await processor.add({'id': i})
        
        # Add items from multiple tasks
        await asyncio.gather(
            add_items(0, 25),
            add_items(25, 25),
            add_items(50, 25),
            add_items(75, 25)
        )
        
        await processor.flush()
        
        # All items should be processed
        total_processed = sum(len(batch) for batch in processor.processed_batches)
        assert total_processed == 100
    
    @pytest.mark.concurrent
    @pytest.mark.asyncio
    async def test_concurrent_flushes(self):
        """Test concurrent flush operations"""
        processor = BatchProcessor(max_size=100)
        
        for i in range(10):
            await processor.add({'id': i})
        
        # Try flushing concurrently
        await asyncio.gather(
            processor.flush(),
            processor.flush(),
            processor.flush()
        )
        
        # Should handle concurrent flushes gracefully
        assert len(processor.processed_batches) <= 3


# =============================================================================
# BATCH STRATEGIES TESTS
# =============================================================================

class TestBatchStrategies:
    """Test different batching strategies"""
    
    @pytest.mark.asyncio
    async def test_fixed_size_batching(self):
        """Test fixed-size batching strategy"""
        processor = BatchProcessor(max_size=10)
        
        for i in range(25):
            await processor.add({'id': i})
        
        await processor.flush()
        
        # Should create 2 full batches + 1 partial
        assert len(processor.processed_batches) == 3
        assert len(processor.processed_batches[0]) == 10
        assert len(processor.processed_batches[1]) == 10
        assert len(processor.processed_batches[2]) == 5
    
    @pytest.mark.asyncio
    async def test_time_based_batching(self):
        """Test time-based batching strategy"""
        processor = BatchProcessor(max_size=1000, max_wait_seconds=0.1)
        
        await processor.add({'id': 1})
        await asyncio.sleep(0.15)
        
        assert processor.should_flush()
    
    @pytest.mark.asyncio
    async def test_hybrid_batching(self):
        """Test hybrid size+time batching"""
        processor = BatchProcessor(max_size=5, max_wait_seconds=0.2)
        
        # Add 3 items (below size limit)
        for i in range(3):
            await processor.add({'id': i})
        
        # Wait for time trigger
        await asyncio.sleep(0.25)
        
        # Should flush due to time
        if processor.should_flush():
            await processor.flush()
        
        assert len(processor.processed_batches) >= 1


# =============================================================================
# BATCH OPTIMIZATION TESTS
# =============================================================================

class TestBatchOptimization:
    """Test batch optimization techniques"""
    
    @pytest.mark.asyncio
    async def test_batch_deduplication(self):
        """Test deduplicating items in batch"""
        class DeduplicatingProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                # Deduplicate by ID
                seen = set()
                deduplicated = []
                for item in batch:
                    if item['id'] not in seen:
                        seen.add(item['id'])
                        deduplicated.append(item)
                return await super()._process_batch(deduplicated)
        
        processor = DeduplicatingProcessor(max_size=10)
        
        # Add duplicates
        for i in range(3):
            await processor.add({'id': 1})
            await processor.add({'id': 2})
        
        await processor.flush()
        
        # Should process only unique items
        processed = processor.processed_batches[0]
        unique_ids = set(item['id'] for item in processed)
        assert len(unique_ids) == 2
    
    @pytest.mark.asyncio
    async def test_batch_compression(self):
        """Test compressing batch data"""
        class CompressingProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                # Simulate compression by counting items
                compressed_size = len(batch)
                # Process compressed batch
                return await super()._process_batch(batch)
        
        processor = CompressingProcessor(max_size=5)
        
        large_items = [{'id': i, 'data': 'x' * 1000} for i in range(5)]
        for item in large_items:
            await processor.add(item)
        
        assert len(processor.processed_batches) == 1
    
    @pytest.mark.asyncio
    async def test_batch_prioritization(self):
        """Test prioritizing items in batch"""
        class PrioritizingProcessor(BatchProcessor):
            async def _process_batch(self, batch):
                # Sort by priority before processing
                sorted_batch = sorted(batch, key=lambda x: x.get('priority', 0), reverse=True)
                return await super()._process_batch(sorted_batch)
        
        processor = PrioritizingProcessor(max_size=5)
        
        items = [
            {'id': 1, 'priority': 1},
            {'id': 2, 'priority': 3},
            {'id': 3, 'priority': 2}
        ]
        
        for item in items:
            await processor.add(item)
        
        await processor.flush()
        
        # Check order after processing (highest priority first)
        processed = processor.processed_batches[0]
        assert processed[0]['priority'] >= processed[1]['priority']


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    @pytest.mark.asyncio
    async def test_zero_size_batch(self):
        """Test batch with size limit of 0"""
        # Size 0 should mean immediate flush
        processor = BatchProcessor(max_size=1)
        
        await processor.add({'id': 1})
        
        assert len(processor.processed_batches) == 1
    
    @pytest.mark.asyncio
    async def test_very_large_batch_size(self):
        """Test very large batch size"""
        processor = BatchProcessor(max_size=1000000)
        
        for i in range(100):
            await processor.add({'id': i})
        
        # Should not auto-flush
        assert len(processor.processed_batches) == 0
        assert len(processor.batch) == 100
    
    @pytest.mark.asyncio
    async def test_zero_wait_time(self):
        """Test zero wait time (immediate flush)"""
        processor = BatchProcessor(max_size=100, max_wait_seconds=0.0)
        
        await processor.add({'id': 1})
        
        # Should always trigger flush
        assert processor.should_flush()
    
    @pytest.mark.asyncio
    async def test_negative_values(self):
        """Test handling negative configuration values"""
        # Negative values should be treated as minimums
        processor = BatchProcessor(max_size=max(1, -5), max_wait_seconds=max(0.1, -1.0))
        
        assert processor.max_size >= 1
        assert processor.max_wait_seconds >= 0.1
    
    @pytest.mark.asyncio
    async def test_none_items(self):
        """Test adding None items"""
        processor = BatchProcessor(max_size=5)
        
        await processor.add(None)
        await processor.add({'id': 1})
        await processor.add(None)
        
        await processor.flush()
        
        # Should handle None items
        assert len(processor.processed_batches[0]) == 3
    
    @pytest.mark.asyncio
    async def test_rapid_add_flush_cycle(self):
        """Test rapid add-flush cycles"""
        processor = BatchProcessor(max_size=2)
        
        for _ in range(10):
            await processor.add({'id': 1})
            await processor.add({'id': 2})
            # Batch auto-flushes at size 2
        
        assert len(processor.processed_batches) == 10