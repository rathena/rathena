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
from datetime import datetime, timedelta, timezone
from loguru import logger
from sqlalchemy import select, and_

from database import get_db_session
from services.session_manager import session_manager
from services.ai_integration import ai_service_client
from config import settings
from models.session import P2PSession, SessionStatus


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

                # Get all active sessions with AI-enabled zones
                async with get_db_session() as db:
                    result = await db.execute(
                        select(P2PSession).where(
                            and_(
                                P2PSession.status == SessionStatus.ACTIVE,
                                P2PSession.peer_count > 0
                            )
                        )
                    )
                    active_sessions = result.scalars().all()

                    if not active_sessions:
                        logger.debug("No active sessions for NPC broadcast")
                        continue

                    # Fetch NPC state from AI service for each zone
                    for session in active_sessions:
                        try:
                            # Get NPC state from AI service
                            npc_state = await ai_service_client.get_npc_state(
                                zone_id=session.zone_id
                            )

                            if not npc_state:
                                continue

                            # Broadcast NPC state to all peers in session
                            # This would be done via the signaling service
                            logger.debug(
                                f"Broadcasting NPC state for zone {session.zone_id} "
                                f"to session {session.session_id} "
                                f"({session.peer_count} peers)"
                            )

                            # Note: Actual broadcast implementation would require
                            # integration with SignalingService to send WebSocket
                            # messages to all peers in the session

                        except Exception as e:
                            logger.error(
                                f"Failed to broadcast NPC state for session "
                                f"{session.session_id}: {e}"
                            )

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

                # Get all active sessions
                async with get_db_session() as db:
                    result = await db.execute(
                        select(P2PSession).where(
                            P2PSession.status == SessionStatus.ACTIVE
                        )
                    )
                    active_sessions = result.scalars().all()

                    if not active_sessions:
                        logger.debug("No active sessions to monitor")
                        continue

                    logger.debug(f"Monitoring {len(active_sessions)} active sessions")

                    # Check each session for health issues
                    for session in active_sessions:
                        # Check session duration (warn if > 24 hours)
                        if session.duration_seconds > 86400:  # 24 hours
                            logger.warn(
                                f"Session {session.session_id} has been active for "
                                f"{session.duration_seconds / 3600:.1f} hours"
                            )

                        # Check if session has been inactive (no updates in last 5 minutes)
                        if session.updated_at:
                            time_since_update = (
                                datetime.now(timezone.utc) - session.updated_at
                            ).total_seconds()

                            if time_since_update > 300:  # 5 minutes
                                logger.warn(
                                    f"Session {session.session_id} inactive for "
                                    f"{time_since_update / 60:.1f} minutes"
                                )

                        # Check peer count (warn if 0 peers)
                        if session.peer_count == 0:
                            logger.warn(
                                f"Session {session.session_id} has no peers, "
                                f"consider ending session"
                            )

                            # Auto-end sessions with no peers for > 10 minutes
                            if session.duration_seconds > 600:
                                logger.info(
                                    f"Auto-ending session {session.session_id} "
                                    f"(no peers for > 10 minutes)"
                                )
                                await session_manager.end_session(session.session_id)

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

