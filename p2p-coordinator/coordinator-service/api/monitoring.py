"""
Monitoring API Endpoints

Provides monitoring and metrics endpoints for the P2P coordinator service.
"""

from typing import Dict, Any
from fastapi import APIRouter, Depends
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from loguru import logger

from database import get_db_session, db_manager
from models.host import Host, HostStatus
from models.zone import Zone, ZoneStatus
from models.session import P2PSession, SessionStatus


router = APIRouter(prefix="/api/monitoring", tags=["monitoring"])


@router.get("/dashboard")
async def get_dashboard_metrics(
    db: AsyncSession = Depends(get_db_session),
) -> Dict[str, Any]:
    """
    Get comprehensive dashboard metrics
    
    Returns:
        Dashboard metrics including hosts, zones, sessions, and system health
    """
    try:
        logger.debug("Fetching dashboard metrics")
        
        # Get host metrics
        host_count_result = await db.execute(
            select(func.count(Host.id))
        )
        total_hosts = host_count_result.scalar() or 0
        
        online_hosts_result = await db.execute(
            select(func.count(Host.id)).where(Host.status == HostStatus.ONLINE)
        )
        online_hosts = online_hosts_result.scalar() or 0
        
        avg_quality_result = await db.execute(
            select(func.avg(Host.quality_score)).where(Host.status == HostStatus.ONLINE)
        )
        avg_host_quality = float(avg_quality_result.scalar() or 0.0)
        
        # Get zone metrics
        zone_count_result = await db.execute(
            select(func.count(Zone.id))
        )
        total_zones = zone_count_result.scalar() or 0
        
        enabled_zones_result = await db.execute(
            select(func.count(Zone.id)).where(Zone.status == ZoneStatus.ENABLED)
        )
        enabled_zones = enabled_zones_result.scalar() or 0
        
        # Get session metrics
        session_count_result = await db.execute(
            select(func.count(P2PSession.id))
        )
        total_sessions = session_count_result.scalar() or 0
        
        active_sessions_result = await db.execute(
            select(func.count(P2PSession.id)).where(P2PSession.status == SessionStatus.ACTIVE)
        )
        active_sessions = active_sessions_result.scalar() or 0
        
        total_players_result = await db.execute(
            select(func.sum(P2PSession.current_players)).where(P2PSession.status == SessionStatus.ACTIVE)
        )
        total_players = int(total_players_result.scalar() or 0)
        
        avg_latency_result = await db.execute(
            select(func.avg(P2PSession.average_latency_ms)).where(P2PSession.status == SessionStatus.ACTIVE)
        )
        avg_latency = float(avg_latency_result.scalar() or 0.0)
        
        # Get database health
        redis = await db_manager.get_redis()
        redis_healthy = await redis.ping()
        
        dashboard = {
            "hosts": {
                "total": total_hosts,
                "online": online_hosts,
                "offline": total_hosts - online_hosts,
                "average_quality_score": round(avg_host_quality, 2),
            },
            "zones": {
                "total": total_zones,
                "enabled": enabled_zones,
                "disabled": total_zones - enabled_zones,
            },
            "sessions": {
                "total": total_sessions,
                "active": active_sessions,
                "inactive": total_sessions - active_sessions,
                "total_players": total_players,
                "average_latency_ms": round(avg_latency, 2),
            },
            "system": {
                "postgres": "healthy",
                "redis": "healthy" if redis_healthy else "unhealthy",
            },
        }
        
        logger.debug(f"Dashboard metrics: {dashboard}")
        return dashboard
        
    except Exception as e:
        logger.error(f"Failed to fetch dashboard metrics: {e}")
        return {
            "error": str(e),
            "hosts": {"total": 0, "online": 0, "offline": 0, "average_quality_score": 0.0},
            "zones": {"total": 0, "enabled": 0, "disabled": 0},
            "sessions": {"total": 0, "active": 0, "inactive": 0, "total_players": 0, "average_latency_ms": 0.0},
            "system": {"postgres": "unknown", "redis": "unknown"},
        }


@router.get("/hosts/stats")
async def get_host_stats(
    db: AsyncSession = Depends(get_db_session),
) -> Dict[str, Any]:
    """
    Get detailed host statistics
    
    Returns:
        Detailed host statistics including quality distribution
    """
    try:
        # Get quality score distribution
        hosts_result = await db.execute(
            select(Host).where(Host.status == HostStatus.ONLINE)
        )
        hosts = hosts_result.scalars().all()
        
        quality_distribution = {
            "excellent": len([h for h in hosts if h.quality_score >= 90]),
            "good": len([h for h in hosts if 70 <= h.quality_score < 90]),
            "fair": len([h for h in hosts if 50 <= h.quality_score < 70]),
            "poor": len([h for h in hosts if h.quality_score < 50]),
        }
        
        return {
            "total_online": len(hosts),
            "quality_distribution": quality_distribution,
        }
        
    except Exception as e:
        logger.error(f"Failed to fetch host stats: {e}")
        return {"error": str(e)}

