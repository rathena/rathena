"""
Host API Endpoints

REST API endpoints for host registration and management.
"""

from typing import List, Optional
from fastapi import APIRouter, Depends, HTTPException, status
from pydantic import BaseModel, Field
from sqlalchemy.ext.asyncio import AsyncSession
from loguru import logger

from database import get_db_session
from services.host_registry import HostRegistryService
from models.host import HostStatus


router = APIRouter(prefix="/api/hosts", tags=["hosts"])
host_service = HostRegistryService()


# Request/Response Models
class HostRegistrationRequest(BaseModel):
    """Host registration request"""
    host_id: str = Field(..., description="Unique host identifier")
    player_id: str = Field(..., description="Player identifier")
    player_name: str = Field(..., description="Player name")
    ip_address: str = Field(..., description="Host IP address")
    port: int = Field(..., description="Host port", ge=1, le=65535)
    public_ip: Optional[str] = Field(None, description="Public IP address")
    cpu_cores: int = Field(..., description="Number of CPU cores", ge=1)
    cpu_frequency_mhz: int = Field(..., description="CPU frequency in MHz", ge=1)
    memory_mb: int = Field(..., description="Memory in MB", ge=1)
    network_speed_mbps: int = Field(..., description="Network speed in Mbps", ge=1)
    ice_servers: Optional[dict] = Field(None, description="ICE servers configuration")


class HostHeartbeatRequest(BaseModel):
    """Host heartbeat request"""
    latency_ms: float = Field(..., description="Current latency in milliseconds", ge=0)
    packet_loss_percent: float = Field(..., description="Packet loss percentage", ge=0, le=100)
    bandwidth_usage_mbps: float = Field(..., description="Bandwidth usage in Mbps", ge=0)
    current_players: int = Field(..., description="Current number of players", ge=0)
    current_zones: int = Field(..., description="Current number of zones", ge=0)


class HostResponse(BaseModel):
    """Host response"""
    id: int
    host_id: str
    player_id: str
    player_name: str
    ip_address: str
    port: int
    public_ip: Optional[str]
    cpu_cores: int
    cpu_frequency_mhz: int
    memory_mb: int
    network_speed_mbps: int
    latency_ms: float
    packet_loss_percent: float
    bandwidth_usage_mbps: float
    status: str
    max_players: int
    current_players: int
    max_zones: int
    current_zones: int
    quality_score: float
    
    class Config:
        from_attributes = True


# API Endpoints
@router.post("/register", response_model=HostResponse, status_code=status.HTTP_201_CREATED)
async def register_host(
    request: HostRegistrationRequest,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Register a new P2P host or update existing host
    
    This endpoint allows a player to register their client as a P2P host.
    """
    try:
        host = await host_service.register_host(
            session=session,
            host_id=request.host_id,
            player_id=request.player_id,
            player_name=request.player_name,
            ip_address=request.ip_address,
            port=request.port,
            public_ip=request.public_ip,
            cpu_cores=request.cpu_cores,
            cpu_frequency_mhz=request.cpu_frequency_mhz,
            memory_mb=request.memory_mb,
            network_speed_mbps=request.network_speed_mbps,
            ice_servers=request.ice_servers,
        )
        
        return host
    
    except Exception as e:
        logger.error(f"Failed to register host: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to register host: {str(e)}",
        )


@router.post("/{host_id}/heartbeat", response_model=HostResponse)
async def update_heartbeat(
    host_id: str,
    request: HostHeartbeatRequest,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Update host heartbeat and metrics
    
    Hosts should send heartbeat every 30-60 seconds to maintain online status.
    """
    try:
        host = await host_service.update_heartbeat(
            session=session,
            host_id=host_id,
            latency_ms=request.latency_ms,
            packet_loss_percent=request.packet_loss_percent,
            bandwidth_usage_mbps=request.bandwidth_usage_mbps,
            current_players=request.current_players,
            current_zones=request.current_zones,
        )
        
        if not host:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Host not found: {host_id}",
            )
        
        return host
    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to update heartbeat: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update heartbeat: {str(e)}",
        )


@router.get("/{host_id}", response_model=HostResponse)
async def get_host(
    host_id: str,
    session: AsyncSession = Depends(get_db_session),
):
    """Get host by ID"""
    host = await host_service.get_host(session, host_id)
    
    if not host:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Host not found: {host_id}",
        )

    return host


@router.get("/", response_model=List[HostResponse])
async def get_all_hosts(
    status_filter: Optional[str] = None,
    min_quality_score: Optional[float] = None,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Get all hosts with optional filtering

    Query parameters:
    - status_filter: Filter by host status (ONLINE, OFFLINE, BUSY, MAINTENANCE)
    - min_quality_score: Minimum quality score (0-10)
    """
    try:
        host_status = None
        if status_filter:
            try:
                host_status = HostStatus(status_filter.upper())
            except ValueError:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Invalid status: {status_filter}. Must be one of: ONLINE, OFFLINE, BUSY, MAINTENANCE",
                )

        hosts = await host_service.get_all_hosts(
            session=session,
            status=host_status,
            min_quality_score=min_quality_score,
        )

        return hosts

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get hosts: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get hosts: {str(e)}",
        )


@router.get("/available/list", response_model=List[HostResponse])
async def get_available_hosts(
    min_quality_score: Optional[float] = None,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Get available hosts (online and not at capacity)

    Query parameters:
    - min_quality_score: Minimum quality score (0-10)
    """
    try:
        hosts = await host_service.get_available_hosts(
            session=session,
            min_quality_score=min_quality_score,
        )

        return hosts

    except Exception as e:
        logger.error(f"Failed to get available hosts: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get available hosts: {str(e)}",
        )


@router.get("/available/best", response_model=HostResponse)
async def get_best_host(
    min_quality_score: Optional[float] = None,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Get the best available host based on quality score

    Query parameters:
    - min_quality_score: Minimum quality score (0-10)
    """
    try:
        host = await host_service.select_best_host(
            session=session,
            min_quality_score=min_quality_score,
        )

        if not host:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail="No available hosts found",
            )

        return host

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get best host: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get best host: {str(e)}",
        )


@router.delete("/{host_id}", status_code=status.HTTP_204_NO_CONTENT)
async def unregister_host(
    host_id: str,
    session: AsyncSession = Depends(get_db_session),
):
    """Unregister a host"""
    try:
        success = await host_service.unregister_host(session, host_id)

        if not success:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Host not found: {host_id}",
            )

        return None

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to unregister host: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to unregister host: {str(e)}",
        )

