"""
Zone API Endpoints

REST API endpoints for zone management.
"""

from typing import List, Optional
from fastapi import APIRouter, Depends, HTTPException, status
from pydantic import BaseModel, Field
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.exc import SQLAlchemyError
from loguru import logger

from database import get_db_session
from services.zone_manager import ZoneManagerService
from models.zone import ZoneStatus
from exceptions import (
    DatabaseException,
    ZoneNotFoundException,
    ZoneConfigurationException,
    ValidationException,
)


router = APIRouter(prefix="/api/v1/zones", tags=["zones"])
zone_service = ZoneManagerService()


# Request/Response Models
class ZoneCreateRequest(BaseModel):
    """Zone creation request"""
    zone_id: str = Field(..., description="Unique zone identifier")
    zone_name: str = Field(..., description="Zone name (internal)")
    zone_display_name: str = Field(..., description="Zone display name (user-facing)")
    map_name: str = Field(..., description="Map name")
    max_players: int = Field(100, description="Maximum players in zone", ge=1)
    recommended_players: int = Field(50, description="Recommended number of players", ge=1)
    p2p_enabled: bool = Field(True, description="Whether P2P is enabled for this zone")
    p2p_priority: int = Field(1, description="P2P priority (higher = more important)", ge=1, le=10)
    fallback_enabled: bool = Field(True, description="Whether fallback to centralized server is enabled")
    min_host_quality_score: float = Field(7.0, description="Minimum host quality score required", ge=0, le=10)
    min_bandwidth_mbps: int = Field(100, description="Minimum bandwidth required", ge=1)
    max_latency_ms: int = Field(100, description="Maximum latency allowed", ge=1)
    description: Optional[str] = Field(None, description="Zone description")


class ZoneResponse(BaseModel):
    """Zone response"""
    id: int
    zone_id: str
    zone_name: str
    zone_display_name: str
    map_name: str
    max_players: int
    recommended_players: int
    p2p_enabled: bool
    p2p_priority: int
    fallback_enabled: bool
    min_host_quality_score: float
    min_bandwidth_mbps: int
    max_latency_ms: int
    status: str
    total_sessions: int
    active_sessions: int
    total_players_served: int
    description: Optional[str]
    
    class Config:
        from_attributes = True


# API Endpoints
@router.post("/", response_model=ZoneResponse, status_code=status.HTTP_201_CREATED)
async def create_zone(
    request: ZoneCreateRequest,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Create a new zone or update existing zone
    
    This endpoint allows administrators to configure P2P zones.
    """
    try:
        zone = await zone_service.create_zone(
            session=session,
            zone_id=request.zone_id,
            zone_name=request.zone_name,
            zone_display_name=request.zone_display_name,
            map_name=request.map_name,
            max_players=request.max_players,
            recommended_players=request.recommended_players,
            p2p_enabled=request.p2p_enabled,
            p2p_priority=request.p2p_priority,
            fallback_enabled=request.fallback_enabled,
            min_host_quality_score=request.min_host_quality_score,
            min_bandwidth_mbps=request.min_bandwidth_mbps,
            max_latency_ms=request.max_latency_ms,
            description=request.description,
        )
        
        return zone
    
    except ValidationException as e:
        logger.warning(f"Invalid zone creation data: {e}")
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=str(e),
        )
    except SQLAlchemyError as e:
        logger.error(f"Database error during zone creation: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Database error occurred",
        )
    except ZoneConfigurationException as e:
        logger.error(f"Zone configuration error: {e}")
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=str(e),
        )
    except Exception as e:
        logger.error(f"Unexpected error during zone creation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="An unexpected error occurred",
        )


@router.get("/{zone_id}", response_model=ZoneResponse)
async def get_zone(
    zone_id: str,
    session: AsyncSession = Depends(get_db_session),
):
    """Get zone by ID"""
    zone = await zone_service.get_zone(session, zone_id)
    
    if not zone:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Zone not found: {zone_id}",
        )
    
    return zone


@router.get("/", response_model=List[ZoneResponse])
async def get_all_zones(
    p2p_enabled: Optional[bool] = None,
    status_filter: Optional[str] = None,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Get all zones with optional filtering
    
    Query parameters:
    - p2p_enabled: Filter by P2P enabled status (true/false)
    - status_filter: Filter by zone status (ENABLED, DISABLED, MAINTENANCE)
    """
    try:
        zone_status = None
        if status_filter:
            try:
                zone_status = ZoneStatus(status_filter.upper())
            except ValueError:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Invalid status: {status_filter}. Must be one of: ENABLED, DISABLED, MAINTENANCE",
                )
        
        zones = await zone_service.get_all_zones(
            session=session,
            p2p_enabled=p2p_enabled,
            status=zone_status,
        )
        
        return zones
    
    except HTTPException:
        raise
    except SQLAlchemyError as e:
        logger.error(f"Database error while fetching zones: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Database error occurred",
        )
    except Exception as e:
        logger.error(f"Unexpected error while fetching zones: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="An unexpected error occurred",
        )


@router.get("/p2p/enabled", response_model=List[ZoneResponse])
async def get_p2p_enabled_zones(
    session: AsyncSession = Depends(get_db_session),
):
    """Get all P2P-enabled zones that are available"""
    try:
        zones = await zone_service.get_p2p_enabled_zones(session)
        return zones

    except Exception as e:
        logger.error(f"Failed to get P2P-enabled zones: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get P2P-enabled zones: {str(e)}",
        )


@router.post("/{zone_id}/enable-p2p", response_model=ZoneResponse)
async def enable_p2p_for_zone(
    zone_id: str,
    session: AsyncSession = Depends(get_db_session),
):
    """Enable P2P for a zone"""
    try:
        zone = await zone_service.enable_p2p_for_zone(session, zone_id)

        if not zone:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Zone not found: {zone_id}",
            )

        return zone

    except HTTPException:
        raise
    except ZoneNotFoundException as e:
        logger.warning(f"Zone not found during P2P enable: {e}")
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=str(e),
        )
    except SQLAlchemyError as e:
        logger.error(f"Database error while enabling P2P for zone: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Database error occurred",
        )
    except Exception as e:
        logger.error(f"Unexpected error while enabling P2P for zone: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="An unexpected error occurred",
        )


@router.post("/{zone_id}/disable-p2p", response_model=ZoneResponse)
async def disable_p2p_for_zone(
    zone_id: str,
    session: AsyncSession = Depends(get_db_session),
):
    """Disable P2P for a zone"""
    try:
        zone = await zone_service.disable_p2p_for_zone(session, zone_id)

        if not zone:
            raise ZoneNotFoundException(f"Zone not found: {zone_id}")

        return zone

    except ZoneNotFoundException as e:
        logger.warning(f"Zone not found during P2P disable: {e}")
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=str(e),
        )
    except SQLAlchemyError as e:
        logger.error(f"Database error while disabling P2P for zone: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Database error occurred",
        )
    except Exception as e:
        logger.error(f"Unexpected error while disabling P2P for zone: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="An unexpected error occurred",
        )


@router.patch("/{zone_id}/status", response_model=ZoneResponse)
async def update_zone_status(
    zone_id: str,
    new_status: str,
    session: AsyncSession = Depends(get_db_session),
):
    """
    Update zone status

    Query parameters:
    - new_status: New status (ENABLED, DISABLED, MAINTENANCE)
    """
    try:
        try:
            zone_status = ZoneStatus(new_status.upper())
        except ValueError:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid status: {new_status}. Must be one of: ENABLED, DISABLED, MAINTENANCE",
            )

        zone = await zone_service.update_zone_status(session, zone_id, zone_status)

        if not zone:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Zone not found: {zone_id}",
            )

        return zone

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to update zone status: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update zone status: {str(e)}",
        )

