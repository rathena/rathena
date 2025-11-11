"""
Instant Response System
Handles priority-based instant triggers for time-sensitive interactions
"""

import asyncio
from datetime import datetime
from typing import Dict, Any, Optional, Callable
from loguru import logger
from enum import Enum
import uuid

from ai_service.config import settings


class EventPriority(str, Enum):
    """Event priority levels"""
    INSTANT = "instant"
    HIGH = "high"
    NORMAL = "normal"
    LOW = "low"


class InstantResponseManager:
    """Manages instant/immediate responses for high-priority events"""
    
    def __init__(self):
        self._instant_queue = asyncio.Queue()
        self._high_queue = asyncio.Queue()
        self._normal_queue = asyncio.Queue()
        self._low_queue = asyncio.Queue()
        
        self._instant_workers = []
        self._queue_workers = []
        self._running = False
        
        # Semaphore to limit concurrent instant responses
        self._instant_semaphore = asyncio.Semaphore(
            settings.instant_response_max_concurrent
        )
    
    async def start(self):
        """Start the instant response system"""
        if not settings.instant_response_enabled:
            logger.info("Instant response system disabled")
            return
        
        self._running = True
        
        # Start instant response workers
        for i in range(settings.instant_response_max_concurrent):
            worker = asyncio.create_task(self._instant_worker(i))
            self._instant_workers.append(worker)
        
        # Start queue workers for other priorities
        for priority in [EventPriority.HIGH, EventPriority.NORMAL, EventPriority.LOW]:
            worker = asyncio.create_task(self._queue_worker(priority))
            self._queue_workers.append(worker)
        
        logger.info(f"✓ Instant response system started "
                   f"({settings.instant_response_max_concurrent} instant workers)")
    
    async def stop(self):
        """Stop the instant response system"""
        self._running = False
        
        # Cancel all workers
        for worker in self._instant_workers + self._queue_workers:
            worker.cancel()
        
        # Wait for workers to finish
        await asyncio.gather(*self._instant_workers, *self._queue_workers, return_exceptions=True)
        
        logger.info("✓ Instant response system stopped")
    
    async def handle_event(
        self,
        event_type: str,
        event_data: Dict[str, Any],
        handler: Callable,
        priority: Optional[EventPriority] = None
    ):
        """Handle an event with appropriate priority"""
        # Determine priority
        if priority is None:
            priority = self._get_event_priority(event_type)
        
        # Create event package
        event = {
            "event_id": str(uuid.uuid4()),
            "event_type": event_type,
            "event_data": event_data,
            "handler": handler,
            "priority": priority,
            "timestamp": datetime.utcnow()
        }
        
        # Route to appropriate queue
        if priority == EventPriority.INSTANT:
            await self._instant_queue.put(event)
            logger.debug(f"Event {event_type} queued for INSTANT response")
        elif priority == EventPriority.HIGH:
            await self._high_queue.put(event)
        elif priority == EventPriority.NORMAL:
            await self._normal_queue.put(event)
        else:  # LOW
            await self._low_queue.put(event)
    
    def _get_event_priority(self, event_type: str) -> EventPriority:
        """Determine event priority based on configuration"""
        # Check if event type is in instant response events
        if event_type in settings.instant_response_events:
            return EventPriority.INSTANT
        
        # Check event priority levels configuration
        if event_type in settings.event_priority_levels:
            priority_str = settings.event_priority_levels[event_type]
            return EventPriority(priority_str)
        
        # Default to NORMAL
        return EventPriority.NORMAL
    
    async def _instant_worker(self, worker_id: int):
        """Worker for instant priority events"""
        logger.debug(f"Instant worker {worker_id} started")
        
        while self._running:
            try:
                # Get event from instant queue
                event = await asyncio.wait_for(
                    self._instant_queue.get(),
                    timeout=1.0
                )
                
                # Acquire semaphore (limit concurrent instant responses)
                async with self._instant_semaphore:
                    start_time = datetime.utcnow()
                    
                    try:
                        # Execute handler immediately
                        await event["handler"](event["event_data"])
                        
                        duration = (datetime.utcnow() - start_time).total_seconds()
                        logger.info(f"✓ INSTANT response completed: {event['event_type']} "
                                  f"(worker {worker_id}, {duration:.2f}s)")
                    
                    except Exception as e:
                        logger.error(f"Error in instant response handler: {e}")
                
                self._instant_queue.task_done()
            
            except asyncio.TimeoutError:
                continue
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in instant worker {worker_id}: {e}")
        
        logger.debug(f"Instant worker {worker_id} stopped")
    
    async def _queue_worker(self, priority: EventPriority):
        """Worker for non-instant priority queues"""
        queue = self._get_queue_for_priority(priority)
        logger.debug(f"{priority.value.upper()} priority worker started")
        
        while self._running:
            try:
                # Get event from queue
                event = await asyncio.wait_for(
                    queue.get(),
                    timeout=1.0
                )
                
                start_time = datetime.utcnow()
                
                try:
                    # Execute handler
                    await event["handler"](event["event_data"])
                    
                    duration = (datetime.utcnow() - start_time).total_seconds()
                    logger.debug(f"✓ {priority.value.upper()} response completed: "
                               f"{event['event_type']} ({duration:.2f}s)")
                
                except Exception as e:
                    logger.error(f"Error in {priority.value} priority handler: {e}")
                
                queue.task_done()
            
            except asyncio.TimeoutError:
                continue
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in {priority.value} worker: {e}")
        
        logger.debug(f"{priority.value.upper()} priority worker stopped")
    
    def _get_queue_for_priority(self, priority: EventPriority) -> asyncio.Queue:
        """Get queue for given priority"""
        if priority == EventPriority.INSTANT:
            return self._instant_queue
        elif priority == EventPriority.HIGH:
            return self._high_queue
        elif priority == EventPriority.NORMAL:
            return self._normal_queue
        else:  # LOW
            return self._low_queue


# Global instance
instant_response_manager = InstantResponseManager()

