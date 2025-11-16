"""
Monitoring API Endpoints

Provides monitoring and metrics endpoints for the P2P coordinator service.
"""

from typing import Dict, Any
from fastapi import APIRouter, Depends, Response
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select, func
from loguru import logger
import time
import psutil

from database import get_db_session, db_manager
from models.host import Host, HostStatus
from models.zone import Zone, ZoneStatus
from models.session import P2PSession, SessionStatus


router = APIRouter(prefix="/api/v1/monitoring", tags=["monitoring"])


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
        
        # Resource utilization metrics
        cpu_percent = psutil.cpu_percent(interval=0.1)
        mem = psutil.virtual_memory()
        net = psutil.net_io_counters()
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
                "cpu_percent": cpu_percent,
                "memory": {
                    "total_mb": round(mem.total / 1024 / 1024, 2),
                    "used_mb": round(mem.used / 1024 / 1024, 2),
                    "percent": mem.percent,
                },
                "network": {
                    "bytes_sent": net.bytes_sent,
                    "bytes_recv": net.bytes_recv,
                    "packets_sent": net.packets_sent,
                    "packets_recv": net.packets_recv,
                }
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


@router.get("/metrics")
async def get_prometheus_metrics(
    db: AsyncSession = Depends(get_db_session),
) -> Response:
    """
    Get Prometheus-compatible metrics

    Returns metrics in Prometheus text exposition format:
    https://prometheus.io/docs/instrumenting/exposition_formats/
    """
    try:
        metrics_lines = []

        # Host metrics
        host_count_result = await db.execute(select(func.count(Host.id)))
        total_hosts = host_count_result.scalar() or 0

        online_hosts_result = await db.execute(
            select(func.count(Host.id)).where(Host.status == HostStatus.ONLINE)
        )
        online_hosts = online_hosts_result.scalar() or 0

        avg_quality_result = await db.execute(
            select(func.avg(Host.quality_score)).where(Host.status == HostStatus.ONLINE)
        )
        avg_quality = float(avg_quality_result.scalar() or 0.0)

        metrics_lines.extend([
            '# HELP p2p_hosts_total Total number of registered hosts',
            '# TYPE p2p_hosts_total gauge',
            f'p2p_hosts_total {total_hosts}',
            '',
            '# HELP p2p_hosts_online Number of online hosts',
            '# TYPE p2p_hosts_online gauge',
            f'p2p_hosts_online {online_hosts}',
            '',
            '# HELP p2p_host_quality_score_avg Average quality score of online hosts',
            '# TYPE p2p_host_quality_score_avg gauge',
            f'p2p_host_quality_score_avg {avg_quality:.2f}',
            '',
        ])

        # Zone metrics
        zone_count_result = await db.execute(select(func.count(Zone.id)))
        total_zones = zone_count_result.scalar() or 0

        enabled_zones_result = await db.execute(
            select(func.count(Zone.id)).where(Zone.p2p_enabled == True)
        )
        p2p_enabled_zones = enabled_zones_result.scalar() or 0

        metrics_lines.extend([
            '# HELP p2p_zones_total Total number of zones',
            '# TYPE p2p_zones_total gauge',
            f'p2p_zones_total {total_zones}',
            '',
            '# HELP p2p_zones_enabled Number of P2P-enabled zones',
            '# TYPE p2p_zones_enabled gauge',
            f'p2p_zones_enabled {p2p_enabled_zones}',
            '',
        ])

        # Session metrics
        session_count_result = await db.execute(select(func.count(P2PSession.id)))
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

        metrics_lines.extend([
            '# HELP p2p_sessions_total Total number of sessions',
            '# TYPE p2p_sessions_total gauge',
            f'p2p_sessions_total {total_sessions}',
            '',
            '# HELP p2p_sessions_active Number of active sessions',
            '# TYPE p2p_sessions_active gauge',
            f'p2p_sessions_active {active_sessions}',
            '',
            '# HELP p2p_players_total Total number of players in active sessions',
            '# TYPE p2p_players_total gauge',
            f'p2p_players_total {total_players}',
            '',
            '# HELP p2p_session_latency_ms_avg Average session latency in milliseconds',
            '# TYPE p2p_session_latency_ms_avg gauge',
            f'p2p_session_latency_ms_avg {avg_latency:.2f}',
            '',
        ])

        # System uptime (placeholder - would need to track actual start time)
        metrics_lines.extend([
            '# HELP p2p_coordinator_up Coordinator service is up',
            '# TYPE p2p_coordinator_up gauge',
            'p2p_coordinator_up 1',
            '',
        ])

        # Join all metrics with newlines
        metrics_text = '\n'.join(metrics_lines)

        logger.debug("Prometheus metrics generated successfully")

        return Response(
            content=metrics_text,
            media_type="text/plain; version=0.0.4; charset=utf-8"
        )

    except Exception as e:
        logger.error(f"Failed to generate Prometheus metrics: {e}")
        error_metrics = [
            '# HELP p2p_coordinator_up Coordinator service is up',
            '# TYPE p2p_coordinator_up gauge',
            'p2p_coordinator_up 0',
        ]
        return Response(
            content='\n'.join(error_metrics),
            media_type="text/plain; version=0.0.4; charset=utf-8"
        )
