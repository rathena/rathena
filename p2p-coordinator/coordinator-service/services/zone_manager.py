"""
Zone Manager Service

Manages P2P zones, zone configuration, and zone assignment.
"""

from typing import List, Optional
from sqlalchemy import select, update
from sqlalchemy.ext.asyncio import AsyncSession
from loguru import logger

from models.zone import Zone, ZoneStatus
from config import settings


class ZoneManagerService:
    """
    Zone Manager Service
    
    Handles zone creation, configuration, and P2P enablement.
    """
    
    async def create_zone(
        self,
        session: AsyncSession,
        zone_id: str,
        zone_name: str,
        zone_display_name: str,
        map_name: str,
        max_players: int = 100,
        recommended_players: int = 50,
        p2p_enabled: bool = True,
        p2p_priority: int = 1,
        fallback_enabled: bool = True,
        min_host_quality_score: float = 7.0,
        min_bandwidth_mbps: int = 100,
        max_latency_ms: int = 100,
        description: Optional[str] = None,
    ) -> Zone:
        """
        Create a new zone or update existing zone
        
        Args:
            session: Database session
            zone_id: Unique zone identifier
            zone_name: Zone name (internal)
            zone_display_name: Zone display name (user-facing)
            map_name: Map name
            max_players: Maximum players in zone
            recommended_players: Recommended number of players
            p2p_enabled: Whether P2P is enabled for this zone
            p2p_priority: P2P priority (higher = more important)
            fallback_enabled: Whether fallback to centralized server is enabled
            min_host_quality_score: Minimum host quality score required
            min_bandwidth_mbps: Minimum bandwidth required
            max_latency_ms: Maximum latency allowed
            description: Zone description (optional)
            
        Returns:
            Zone: Created or updated zone object
        """
        logger.info(f"Creating/updating zone: {zone_id} ({zone_display_name})")
        
        # Check if zone already exists
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        existing_zone = result.scalar_one_or_none()
        
        if existing_zone:
            # Update existing zone
            logger.info(f"Updating existing zone: {zone_id}")
            existing_zone.zone_name = zone_name
            existing_zone.zone_display_name = zone_display_name
            existing_zone.map_name = map_name
            existing_zone.max_players = max_players
            existing_zone.recommended_players = recommended_players
            existing_zone.p2p_enabled = p2p_enabled
            existing_zone.p2p_priority = p2p_priority
            existing_zone.fallback_enabled = fallback_enabled
            existing_zone.min_host_quality_score = min_host_quality_score
            existing_zone.min_bandwidth_mbps = min_bandwidth_mbps
            existing_zone.max_latency_ms = max_latency_ms
            existing_zone.description = description
            
            await session.commit()
            await session.refresh(existing_zone)
            
            logger.info(f"Zone updated: {zone_id}")
            return existing_zone
        
        # Create new zone
        new_zone = Zone(
            zone_id=zone_id,
            zone_name=zone_name,
            zone_display_name=zone_display_name,
            map_name=map_name,
            max_players=max_players,
            recommended_players=recommended_players,
            p2p_enabled=p2p_enabled,
            p2p_priority=p2p_priority,
            fallback_enabled=fallback_enabled,
            min_host_quality_score=min_host_quality_score,
            min_bandwidth_mbps=min_bandwidth_mbps,
            max_latency_ms=max_latency_ms,
            description=description,
            status=ZoneStatus.ENABLED,
        )
        
        session.add(new_zone)
        await session.commit()
        await session.refresh(new_zone)
        
        logger.info(f"New zone created: {zone_id}")
        return new_zone
    
    async def get_zone(self, session: AsyncSession, zone_id: str) -> Optional[Zone]:
        """Get zone by ID"""
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        return result.scalar_one_or_none()
    
    async def get_all_zones(
        self,
        session: AsyncSession,
        p2p_enabled: Optional[bool] = None,
        status: Optional[ZoneStatus] = None,
    ) -> List[Zone]:
        """
        Get all zones with optional filtering
        
        Args:
            session: Database session
            p2p_enabled: Filter by P2P enabled status (optional)
            status: Filter by zone status (optional)
            
        Returns:
            List of zones
        """
        stmt = select(Zone)
        
        if p2p_enabled is not None:
            stmt = stmt.where(Zone.p2p_enabled == p2p_enabled)
        
        if status:
            stmt = stmt.where(Zone.status == status)
        
        stmt = stmt.order_by(Zone.p2p_priority.desc(), Zone.zone_name)
        
        result = await session.execute(stmt)
        return list(result.scalars().all())

    async def get_p2p_enabled_zones(self, session: AsyncSession) -> List[Zone]:
        """
        Get all P2P-enabled zones that are available

        Args:
            session: Database session

        Returns:
            List of P2P-enabled zones
        """
        stmt = select(Zone).where(
            Zone.p2p_enabled == True,
            Zone.status == ZoneStatus.ENABLED,
        ).order_by(Zone.p2p_priority.desc())

        result = await session.execute(stmt)
        zones = list(result.scalars().all())

        logger.info(f"Found {len(zones)} P2P-enabled zones")
        return zones

    async def enable_p2p_for_zone(self, session: AsyncSession, zone_id: str) -> Optional[Zone]:
        """
        Enable P2P for a zone

        Args:
            session: Database session
            zone_id: Zone identifier

        Returns:
            Updated zone or None if not found
        """
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        zone = result.scalar_one_or_none()

        if not zone:
            logger.warning(f"Zone not found: {zone_id}")
            return None

        zone.p2p_enabled = True
        await session.commit()
        await session.refresh(zone)

        logger.info(f"P2P enabled for zone: {zone_id}")
        return zone

    async def disable_p2p_for_zone(self, session: AsyncSession, zone_id: str) -> Optional[Zone]:
        """
        Disable P2P for a zone

        Args:
            session: Database session
            zone_id: Zone identifier

        Returns:
            Updated zone or None if not found
        """
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        zone = result.scalar_one_or_none()

        if not zone:
            logger.warning(f"Zone not found: {zone_id}")
            return None

        zone.p2p_enabled = False
        await session.commit()
        await session.refresh(zone)

        logger.info(f"P2P disabled for zone: {zone_id}")
        return zone

    async def update_zone_status(
        self,
        session: AsyncSession,
        zone_id: str,
        status: ZoneStatus,
    ) -> Optional[Zone]:
        """
        Update zone status

        Args:
            session: Database session
            zone_id: Zone identifier
            status: New status

        Returns:
            Updated zone or None if not found
        """
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        zone = result.scalar_one_or_none()

        if not zone:
            logger.warning(f"Zone not found: {zone_id}")
            return None

        zone.status = status
        await session.commit()
        await session.refresh(zone)

        logger.info(f"Zone status updated: {zone_id} -> {status.value}")
        return zone

    async def update_zone_statistics(
        self,
        session: AsyncSession,
        zone_id: str,
        active_sessions_delta: int = 0,
        total_sessions_delta: int = 0,
        total_players_served_delta: int = 0,
    ) -> Optional[Zone]:
        """
        Update zone statistics

        Args:
            session: Database session
            zone_id: Zone identifier
            active_sessions_delta: Change in active sessions
            total_sessions_delta: Change in total sessions
            total_players_served_delta: Change in total players served

        Returns:
            Updated zone or None if not found
        """
        stmt = select(Zone).where(Zone.zone_id == zone_id)
        result = await session.execute(stmt)
        zone = result.scalar_one_or_none()

        if not zone:
            logger.warning(f"Zone not found: {zone_id}")
            return None

        zone.active_sessions += active_sessions_delta
        zone.total_sessions += total_sessions_delta
        zone.total_players_served += total_players_served_delta

        await session.commit()
        await session.refresh(zone)

        logger.debug(f"Zone statistics updated: {zone_id}")
        return zone

