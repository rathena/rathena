"""
P2P Session Manager Service

Manages P2P session lifecycle including creation, monitoring, and termination.
"""

from typing import Optional, List
from datetime import datetime, timedelta
from sqlalchemy import select, update, and_
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import attributes
from loguru import logger

from models.session import P2PSession, SessionStatus
from models.host import Host
from models.zone import Zone


class SessionManagerService:
    """
    P2P Session Manager Service
    
    Manages the lifecycle of P2P sessions between hosts and zones.
    """
    
    def __init__(self):
        logger.info("SessionManagerService initialized")
    
    async def create_session(
        self,
        db: AsyncSession,
        host_id: str,
        zone_id: str,
        player_ids: List[str],
    ) -> Optional[P2PSession]:
        """
        Create a new P2P session

        Args:
            db: Database session
            host_id: Host identifier (string)
            zone_id: Zone identifier (string)
            player_ids: List of player IDs in the session

        Returns:
            Created P2PSession or None if failed
        """
        try:
            # Verify host exists and is available
            host_result = await db.execute(
                select(Host).where(Host.host_id == host_id)
            )
            host = host_result.scalar_one_or_none()

            if not host:
                logger.error(f"Host not found: {host_id}")
                return None

            if host.status.value != "online":
                logger.error(f"Host {host_id} is not online, current status: {host.status.value}")
                return None

            # Verify zone exists and P2P is enabled
            zone_result = await db.execute(
                select(Zone).where(Zone.zone_id == zone_id)
            )
            zone = zone_result.scalar_one_or_none()

            if not zone:
                logger.error(f"Zone not found: {zone_id}")
                return None

            if not zone.p2p_enabled:
                logger.error(f"P2P not enabled for zone: {zone_id}")
                return None

            # Generate unique session ID
            import uuid
            session_id = f"session-{uuid.uuid4().hex[:16]}"

            # Create session
            session = P2PSession(
                session_id=session_id,
                host_id=host.id,  # Use database ID, not host_id string
                zone_id=zone.id,  # Use database ID, not zone_id string
                connected_players=player_ids,
                current_players=len(player_ids),
                max_players=zone.max_players,
                status=SessionStatus.PENDING,
            )

            db.add(session)
            await db.commit()
            await db.refresh(session)

            logger.info(f"Created P2P session {session.session_id} (ID: {session.id}) for host {host_id} in zone {zone_id} with {len(player_ids)} players")

            return session

        except Exception as e:
            logger.error(f"Failed to create P2P session: {e}")
            await db.rollback()
            return None
    
    async def activate_session(self, db: AsyncSession, session_id: int) -> bool:
        """
        Activate a pending session
        
        Args:
            db: Database session
            session_id: Session ID
            
        Returns:
            True if activated, False otherwise
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            session = result.scalar_one_or_none()
            
            if not session:
                logger.error(f"Session not found: {session_id}")
                return False
            
            if session.status != SessionStatus.PENDING:
                logger.warning(f"Session {session_id} is not pending: {session.status}")
                return False
            
            session.status = SessionStatus.ACTIVE
            session.started_at = datetime.utcnow()
            
            await db.commit()
            logger.info(f"Activated P2P session {session_id}")
            
            return True
        
        except Exception as e:
            logger.error(f"Failed to activate session {session_id}: {e}")
            await db.rollback()
            return False
    
    async def end_session(self, db: AsyncSession, session_id: int, reason: str = "normal") -> bool:
        """
        End an active session
        
        Args:
            db: Database session
            session_id: Session ID
            reason: Reason for ending (normal, timeout, error, etc.)
            
        Returns:
            True if ended, False otherwise
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            session = result.scalar_one_or_none()

            if not session:
                logger.error(f"Session not found: {session_id}")
                return False

            session.status = SessionStatus.ENDED
            session.ended_at = datetime.utcnow()

            # duration_seconds is a property, no need to set it

            await db.commit()
            logger.info(f"Ended P2P session {session_id}, reason: {reason}")

            return True

        except Exception as e:
            logger.error(f"Failed to end session {session_id}: {e}")
            await db.rollback()
            return False

    async def get_session(self, db: AsyncSession, session_id: int) -> Optional[P2PSession]:
        """
        Get session by ID

        Args:
            db: Database session
            session_id: Session ID

        Returns:
            P2PSession or None
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            return result.scalar_one_or_none()
        except Exception as e:
            logger.error(f"Failed to get session {session_id}: {e}")
            return None

    async def get_active_sessions_for_host(self, db: AsyncSession, host_id: str) -> List[P2PSession]:
        """
        Get all active sessions for a host

        Args:
            db: Database session
            host_id: Host identifier

        Returns:
            List of active P2PSessions
        """
        try:
            result = await db.execute(
                select(P2PSession).where(
                    and_(
                        P2PSession.host_id == host_id,
                        P2PSession.status == SessionStatus.ACTIVE,
                    )
                )
            )
            sessions = result.scalars().all()
            logger.debug(f"Found {len(sessions)} active sessions for host {host_id}")
            return list(sessions)
        except Exception as e:
            logger.error(f"Failed to get active sessions for host {host_id}: {e}")
            return []

    async def get_active_sessions_for_zone(self, db: AsyncSession, zone_id: str) -> List[P2PSession]:
        """
        Get all active sessions for a zone

        Args:
            db: Database session
            zone_id: Zone identifier

        Returns:
            List of active P2PSessions
        """
        try:
            result = await db.execute(
                select(P2PSession).where(
                    and_(
                        P2PSession.zone_id == zone_id,
                        P2PSession.status == SessionStatus.ACTIVE,
                    )
                )
            )
            sessions = result.scalars().all()
            logger.debug(f"Found {len(sessions)} active sessions for zone {zone_id}")
            return list(sessions)
        except Exception as e:
            logger.error(f"Failed to get active sessions for zone {zone_id}: {e}")
            return []

    async def update_session_metrics(
        self,
        db: AsyncSession,
        session_id: int,
        avg_latency_ms: Optional[float] = None,
        packet_loss_percent: Optional[float] = None,
        bandwidth_usage_mbps: Optional[float] = None,
    ) -> bool:
        """
        Update session performance metrics

        Args:
            db: Database session
            session_id: Session ID
            avg_latency_ms: Average latency in milliseconds
            packet_loss_percent: Packet loss percentage
            bandwidth_usage_mbps: Bandwidth usage in Mbps

        Returns:
            True if updated, False otherwise
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            session = result.scalar_one_or_none()

            if not session:
                logger.error(f"Session not found: {session_id}")
                return False

            if avg_latency_ms is not None:
                session.average_latency_ms = avg_latency_ms
            if packet_loss_percent is not None:
                session.average_packet_loss_percent = packet_loss_percent
            if bandwidth_usage_mbps is not None:
                session.bandwidth_usage_mbps = bandwidth_usage_mbps

            await db.commit()
            logger.debug(f"Updated metrics for session {session_id}")

            return True

        except Exception as e:
            logger.error(f"Failed to update session metrics for {session_id}: {e}")
            await db.rollback()
            return False

    async def add_player_to_session(self, db: AsyncSession, session_id: int, player_id: str) -> bool:
        """
        Add a player to a session

        Args:
            db: Database session
            session_id: Session ID
            player_id: Player ID to add

        Returns:
            True if successful, False otherwise
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            session = result.scalar_one_or_none()

            if not session:
                logger.error(f"Session not found: {session_id}")
                return False

            # Use the model's add_player method
            if session.add_player(player_id):
                # Mark the connected_players field as modified so SQLAlchemy detects the change
                attributes.flag_modified(session, 'connected_players')
                await db.commit()
                logger.info(f"Added player {player_id} to session {session_id}")
                return True
            else:
                logger.warning(f"Failed to add player {player_id} to session {session_id} (session full or player already in session)")
                return False

        except Exception as e:
            logger.error(f"Failed to add player to session: {e}")
            await db.rollback()
            return False

    async def remove_player_from_session(self, db: AsyncSession, session_id: int, player_id: str) -> bool:
        """
        Remove a player from a session

        Args:
            db: Database session
            session_id: Session ID
            player_id: Player ID to remove

        Returns:
            True if successful, False otherwise
        """
        try:
            result = await db.execute(
                select(P2PSession).where(P2PSession.id == session_id)
            )
            session = result.scalar_one_or_none()

            if not session:
                logger.error(f"Session not found: {session_id}")
                return False

            # Use the model's remove_player method
            if session.remove_player(player_id):
                # Mark the connected_players field as modified so SQLAlchemy detects the change
                attributes.flag_modified(session, 'connected_players')
                await db.commit()
                logger.info(f"Removed player {player_id} from session {session_id}")
                return True
            else:
                logger.warning(f"Failed to remove player {player_id} from session {session_id} (player not in session)")
                return False

        except Exception as e:
            logger.error(f"Failed to remove player from session: {e}")
            await db.rollback()
            return False

    async def cleanup_stale_sessions(self, db: AsyncSession, timeout_minutes: int = 30) -> int:
        """
        Clean up stale sessions that have been pending or active for too long

        Args:
            db: Database session
            timeout_minutes: Timeout in minutes

        Returns:
            Number of sessions cleaned up
        """
        try:
            cutoff_time = datetime.utcnow() - timedelta(minutes=timeout_minutes)

            # Find stale sessions
            result = await db.execute(
                select(P2PSession).where(
                    and_(
                        P2PSession.status.in_([SessionStatus.PENDING, SessionStatus.ACTIVE]),
                        P2PSession.created_at < cutoff_time,
                    )
                )
            )
            stale_sessions = result.scalars().all()

            count = 0
            for session in stale_sessions:
                session.status = SessionStatus.FAILED
                session.ended_at = datetime.utcnow()
                count += 1

            await db.commit()

            if count > 0:
                logger.info(f"Cleaned up {count} stale sessions")

            return count

        except Exception as e:
            logger.error(f"Failed to cleanup stale sessions: {e}")
            await db.rollback()
            return 0


# Global session manager service instance
session_manager = SessionManagerService()
