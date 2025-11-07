"""
Host Registry Service

Manages P2P host registration, health checking, and quality scoring.
"""

from typing import List, Optional
from datetime import datetime, timedelta
from sqlalchemy import select, update, delete
from sqlalchemy.ext.asyncio import AsyncSession
from loguru import logger

from models.host import Host, HostStatus
from config import settings


class HostRegistryService:
    """
    Host Registry Service
    
    Handles host registration, heartbeat updates, quality scoring, and host selection.
    """
    
    async def register_host(
        self,
        session: AsyncSession,
        host_id: str,
        player_id: str,
        player_name: str,
        ip_address: str,
        port: int,
        cpu_cores: int,
        cpu_frequency_mhz: int,
        memory_mb: int,
        network_speed_mbps: int,
        public_ip: Optional[str] = None,
        ice_servers: Optional[dict] = None,
    ) -> Host:
        """
        Register a new P2P host or update existing host
        
        Args:
            session: Database session
            host_id: Unique host identifier
            player_id: Player identifier
            player_name: Player name
            ip_address: Host IP address
            port: Host port
            cpu_cores: Number of CPU cores
            cpu_frequency_mhz: CPU frequency in MHz
            memory_mb: Memory in MB
            network_speed_mbps: Network speed in Mbps
            public_ip: Public IP address (optional)
            ice_servers: ICE servers configuration (optional)
            
        Returns:
            Host: Registered host object
        """
        logger.info(f"Registering host: {host_id} (player: {player_name})")
        
        # Check if host already exists
        stmt = select(Host).where(Host.host_id == host_id)
        result = await session.execute(stmt)
        existing_host = result.scalar_one_or_none()
        
        if existing_host:
            # Update existing host
            logger.info(f"Updating existing host: {host_id}")
            existing_host.player_id = player_id
            existing_host.player_name = player_name
            existing_host.ip_address = ip_address
            existing_host.port = port
            existing_host.public_ip = public_ip
            existing_host.cpu_cores = cpu_cores
            existing_host.cpu_frequency_mhz = cpu_frequency_mhz
            existing_host.memory_mb = memory_mb
            existing_host.network_speed_mbps = network_speed_mbps
            existing_host.ice_servers = ice_servers
            existing_host.status = HostStatus.ONLINE
            existing_host.last_heartbeat = datetime.utcnow()
            existing_host.updated_at = datetime.utcnow()
            
            # Recalculate quality score
            existing_host.quality_score = existing_host.calculate_quality_score()
            
            await session.commit()
            await session.refresh(existing_host)
            
            logger.info(f"Host updated: {host_id}, quality_score={existing_host.quality_score:.2f}")
            return existing_host
        
        # Create new host
        new_host = Host(
            host_id=host_id,
            player_id=player_id,
            player_name=player_name,
            ip_address=ip_address,
            port=port,
            public_ip=public_ip,
            cpu_cores=cpu_cores,
            cpu_frequency_mhz=cpu_frequency_mhz,
            memory_mb=memory_mb,
            network_speed_mbps=network_speed_mbps,
            ice_servers=ice_servers,
            status=HostStatus.ONLINE,
            last_heartbeat=datetime.utcnow(),
        )
        
        # Calculate initial quality score
        new_host.quality_score = new_host.calculate_quality_score()
        
        session.add(new_host)
        await session.commit()
        await session.refresh(new_host)
        
        logger.info(f"New host registered: {host_id}, quality_score={new_host.quality_score:.2f}")
        return new_host
    
    async def update_heartbeat(
        self,
        session: AsyncSession,
        host_id: str,
        latency_ms: float,
        packet_loss_percent: float,
        bandwidth_usage_mbps: float,
        current_players: int,
        current_zones: int,
    ) -> Optional[Host]:
        """
        Update host heartbeat and metrics
        
        Args:
            session: Database session
            host_id: Host identifier
            latency_ms: Current latency in milliseconds
            packet_loss_percent: Packet loss percentage
            bandwidth_usage_mbps: Bandwidth usage in Mbps
            current_players: Current number of players
            current_zones: Current number of zones
            
        Returns:
            Host: Updated host object or None if not found
        """
        stmt = select(Host).where(Host.host_id == host_id)
        result = await session.execute(stmt)
        host = result.scalar_one_or_none()
        
        if not host:
            logger.warning(f"Host not found for heartbeat update: {host_id}")
            return None
        
        # Update metrics
        host.latency_ms = latency_ms
        host.packet_loss_percent = packet_loss_percent
        host.bandwidth_usage_mbps = bandwidth_usage_mbps
        host.current_players = current_players
        host.current_zones = current_zones
        host.last_heartbeat = datetime.utcnow()
        host.updated_at = datetime.utcnow()
        
        # Recalculate quality score
        host.quality_score = host.calculate_quality_score()
        
        await session.commit()
        await session.refresh(host)
        
        logger.debug(f"Heartbeat updated for host: {host_id}, quality_score={host.quality_score:.2f}")
        return host

    async def get_host(self, session: AsyncSession, host_id: str) -> Optional[Host]:
        """Get host by ID"""
        stmt = select(Host).where(Host.host_id == host_id)
        result = await session.execute(stmt)
        return result.scalar_one_or_none()

    async def get_all_hosts(
        self,
        session: AsyncSession,
        status: Optional[HostStatus] = None,
        min_quality_score: Optional[float] = None,
    ) -> List[Host]:
        """
        Get all hosts with optional filtering

        Args:
            session: Database session
            status: Filter by host status (optional)
            min_quality_score: Minimum quality score (optional)

        Returns:
            List of hosts
        """
        stmt = select(Host)

        if status:
            stmt = stmt.where(Host.status == status)

        if min_quality_score is not None:
            stmt = stmt.where(Host.quality_score >= min_quality_score)

        stmt = stmt.order_by(Host.quality_score.desc())

        result = await session.execute(stmt)
        return list(result.scalars().all())

    async def get_available_hosts(
        self,
        session: AsyncSession,
        min_quality_score: Optional[float] = None,
    ) -> List[Host]:
        """
        Get available hosts (online and not at capacity)

        Args:
            session: Database session
            min_quality_score: Minimum quality score (optional)

        Returns:
            List of available hosts
        """
        stmt = select(Host).where(
            Host.status == HostStatus.ONLINE,
            Host.current_players < Host.max_players,
            Host.current_zones < Host.max_zones,
        )

        if min_quality_score is not None:
            stmt = stmt.where(Host.quality_score >= min_quality_score)

        stmt = stmt.order_by(Host.quality_score.desc())

        result = await session.execute(stmt)
        hosts = list(result.scalars().all())

        logger.info(f"Found {len(hosts)} available hosts (min_quality={min_quality_score})")
        return hosts

    async def select_best_host(
        self,
        session: AsyncSession,
        min_quality_score: Optional[float] = None,
    ) -> Optional[Host]:
        """
        Select the best available host based on quality score

        Args:
            session: Database session
            min_quality_score: Minimum quality score (optional)

        Returns:
            Best available host or None
        """
        hosts = await self.get_available_hosts(session, min_quality_score)

        if not hosts:
            logger.warning("No available hosts found")
            return None

        best_host = hosts[0]  # Already sorted by quality_score desc
        logger.info(f"Selected best host: {best_host.host_id}, quality_score={best_host.quality_score:.2f}")
        return best_host

    async def unregister_host(self, session: AsyncSession, host_id: str) -> bool:
        """
        Unregister a host

        Args:
            session: Database session
            host_id: Host identifier

        Returns:
            True if host was unregistered, False if not found
        """
        stmt = delete(Host).where(Host.host_id == host_id)
        result = await session.execute(stmt)
        await session.commit()

        if result.rowcount > 0:
            logger.info(f"Host unregistered: {host_id}")
            return True
        else:
            logger.warning(f"Host not found for unregistration: {host_id}")
            return False

    async def mark_offline_stale_hosts(self, session: AsyncSession, timeout_seconds: int = 60) -> int:
        """
        Mark hosts as offline if they haven't sent heartbeat within timeout

        Args:
            session: Database session
            timeout_seconds: Heartbeat timeout in seconds

        Returns:
            Number of hosts marked offline
        """
        cutoff_time = datetime.utcnow() - timedelta(seconds=timeout_seconds)

        stmt = (
            update(Host)
            .where(
                Host.status == HostStatus.ONLINE,
                Host.last_heartbeat < cutoff_time,
            )
            .values(status=HostStatus.OFFLINE, updated_at=datetime.utcnow())
        )

        result = await session.execute(stmt)
        await session.commit()

        if result.rowcount > 0:
            logger.warning(f"Marked {result.rowcount} hosts as offline due to stale heartbeat")

        return result.rowcount

