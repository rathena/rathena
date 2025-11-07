"""
Background Tasks Service

Handles periodic background tasks for P2P coordinator:
- NPC state broadcasting to P2P hosts
- Session health monitoring
- Stale session cleanup
- Performance metrics collection
"""

import asyncio
from typing import Optional
from datetime import datetime, timedelta
from loguru import logger

from database import get_db_session
from services.session_manager import session_manager
from services.ai_integration import ai_service_client
from config import settings


class BackgroundTasksService:
    """
    Background Tasks Service
    
    Manages periodic background tasks for the P2P coordinator
    """
    
    def __init__(self):
        self.running = False
        self.tasks: list[asyncio.Task] = []
        logger.info("BackgroundTasksService initialized")
    
    async def start(self) -> None:
        """Start all background tasks"""
        if self.running:
            logger.warning("Background tasks already running")
            return
        
        self.running = True
        logger.info("Starting background tasks...")
        
        # Start NPC state broadcasting task (every 5 seconds)
        self.tasks.append(asyncio.create_task(self._npc_broadcast_loop()))
        
        # Start session health monitoring task (every 30 seconds)
        self.tasks.append(asyncio.create_task(self._session_health_loop()))
        
        # Start stale session cleanup task (every 5 minutes)
        self.tasks.append(asyncio.create_task(self._cleanup_loop()))
        
        logger.info(f"✅ Started {len(self.tasks)} background tasks")
    
    async def stop(self) -> None:
        """Stop all background tasks"""
        if not self.running:
            return
        
        self.running = False
        logger.info("Stopping background tasks...")
        
        # Cancel all tasks
        for task in self.tasks:
            task.cancel()
        
        # Wait for all tasks to complete
        await asyncio.gather(*self.tasks, return_exceptions=True)
        
        self.tasks.clear()
        logger.info("✅ All background tasks stopped")
    
    async def _npc_broadcast_loop(self) -> None:
        """
        Periodically broadcast NPC states to P2P hosts
        
        Runs every 5 seconds
        """
        logger.info("NPC broadcast loop started")
        
        while self.running:
            try:
                await asyncio.sleep(5)  # Run every 5 seconds
                
                if not settings.ai_service.ai_service_enabled:
                    continue
                
                # TODO: Implement NPC state broadcasting
                # This will be implemented in the next iteration
                logger.debug("NPC broadcast tick")
                
            except asyncio.CancelledError:
                logger.info("NPC broadcast loop cancelled")
                break
            except Exception as e:
                logger.error(f"Error in NPC broadcast loop: {e}")
                await asyncio.sleep(5)
    
    async def _session_health_loop(self) -> None:
        """
        Periodically monitor session health and trigger fallback if needed
        
        Runs every 30 seconds
        """
        logger.info("Session health monitoring loop started")
        
        while self.running:
            try:
                await asyncio.sleep(30)  # Run every 30 seconds
                
                # TODO: Implement session health monitoring
                # Check session metrics and trigger fallback if latency/packet loss exceeds thresholds
                logger.debug("Session health check tick")
                
            except asyncio.CancelledError:
                logger.info("Session health loop cancelled")
                break
            except Exception as e:
                logger.error(f"Error in session health loop: {e}")
                await asyncio.sleep(30)
    
    async def _cleanup_loop(self) -> None:
        """
        Periodically cleanup stale sessions
        
        Runs every 5 minutes
        """
        logger.info("Cleanup loop started")
        
        while self.running:
            try:
                await asyncio.sleep(300)  # Run every 5 minutes
                
                async for db in get_db_session():
                    count = await session_manager.cleanup_stale_sessions(db, timeout_minutes=30)
                    if count > 0:
                        logger.info(f"Cleaned up {count} stale sessions")
                    break
                
            except asyncio.CancelledError:
                logger.info("Cleanup loop cancelled")
                break
            except Exception as e:
                logger.error(f"Error in cleanup loop: {e}")
                await asyncio.sleep(300)


# Global background tasks service instance
background_tasks = BackgroundTasksService()

