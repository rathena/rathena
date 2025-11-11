"""
P2P Session API Endpoints

REST API for managing P2P sessions.
"""

from typing import List, Optional
from fastapi import APIRouter, Depends, HTTPException, status
from pydantic import BaseModel, Field
from sqlalchemy.ext.asyncio import AsyncSession
from loguru import logger

from database import get_db_session
from services.session_manager import SessionManagerService
from models.session import SessionStatus


router = APIRouter(prefix="/api/v1/sessions", tags=["sessions"])
session_service = SessionManagerService()


# Request/Response Models
class CreateSessionRequest(BaseModel):
    """Request to create a new P2P session"""
    host_id: str = Field(..., description="Host identifier")
    zone_id: str = Field(..., description="Zone identifier")
    player_ids: List[str] = Field(..., description="List of player IDs")


class UpdateSessionMetricsRequest(BaseModel):
    """Request to update session metrics"""
    avg_latency_ms: Optional[float] = Field(None, description="Average latency in ms")
    packet_loss_percent: Optional[float] = Field(None, description="Packet loss percentage")
    bandwidth_usage_mbps: Optional[float] = Field(None, description="Bandwidth usage in Mbps")


class SessionResponse(BaseModel):
    """P2P session response"""
    id: int
    session_id: str
    host_id: int
    zone_id: int
    connected_players: List[str]
    current_players: int
    max_players: int
    status: str
    average_latency_ms: float
    average_packet_loss_percent: float
    bandwidth_usage_mbps: float
    started_at: Optional[str]
    ended_at: Optional[str]
    created_at: str
    updated_at: str


# API Endpoints
@router.post("/", response_model=SessionResponse, status_code=status.HTTP_201_CREATED)
async def create_session(
    request: CreateSessionRequest,
    db: AsyncSession = Depends(get_db_session),
):
    """Create a new P2P session"""
    logger.info(f"Creating session for host {request.host_id} in zone {request.zone_id}")
    
    session = await session_service.create_session(
        db=db,
        host_id=request.host_id,
        zone_id=request.zone_id,
        player_ids=request.player_ids,
    )
    
    if not session:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Failed to create session",
        )
    
    return SessionResponse(
        id=session.id,
        session_id=session.session_id,
        host_id=session.host_id,
        zone_id=session.zone_id,
        connected_players=session.connected_players or [],
        current_players=session.current_players,
        max_players=session.max_players,
        status=session.status.value,
        average_latency_ms=session.average_latency_ms,
        average_packet_loss_percent=session.average_packet_loss_percent,
        bandwidth_usage_mbps=session.bandwidth_usage_mbps,
        started_at=session.started_at.isoformat() if session.started_at else None,
        ended_at=session.ended_at.isoformat() if session.ended_at else None,
        created_at=session.created_at.isoformat(),
        updated_at=session.updated_at.isoformat(),
    )


@router.get("/{session_id}", response_model=SessionResponse)
async def get_session(
    session_id: int,
    db: AsyncSession = Depends(get_db_session),
):
    """Get session by ID"""
    session = await session_service.get_session(db, session_id)
    
    if not session:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Session {session_id} not found",
        )
    
    return SessionResponse(
        id=session.id,
        session_id=session.session_id,
        host_id=session.host_id,
        zone_id=session.zone_id,
        connected_players=session.connected_players or [],
        current_players=session.current_players,
        max_players=session.max_players,
        status=session.status.value,
        average_latency_ms=session.average_latency_ms,
        average_packet_loss_percent=session.average_packet_loss_percent,
        bandwidth_usage_mbps=session.bandwidth_usage_mbps,
        started_at=session.started_at.isoformat() if session.started_at else None,
        ended_at=session.ended_at.isoformat() if session.ended_at else None,
        created_at=session.created_at.isoformat(),
        updated_at=session.updated_at.isoformat(),
    )


@router.post("/{session_id}/activate", status_code=status.HTTP_200_OK)
async def activate_session(
    session_id: int,
    db: AsyncSession = Depends(get_db_session),
):
    """Activate a pending session"""
    success = await session_service.activate_session(db, session_id)
    
    if not success:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Failed to activate session {session_id}",
        )
    
    return {"message": f"Session {session_id} activated"}


@router.post("/{session_id}/end", status_code=status.HTTP_200_OK)
async def end_session(
    session_id: int,
    reason: str = "normal",
    db: AsyncSession = Depends(get_db_session),
):
    """End an active session"""
    success = await session_service.end_session(db, session_id, reason)
    
    if not success:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Failed to end session {session_id}",
        )
    
    return {"message": f"Session {session_id} ended"}


@router.patch("/{session_id}/metrics", status_code=status.HTTP_200_OK)
async def update_session_metrics(
    session_id: int,
    request: UpdateSessionMetricsRequest,
    db: AsyncSession = Depends(get_db_session),
):
    """Update session performance metrics"""
    success = await session_service.update_session_metrics(
        db=db,
        session_id=session_id,
        avg_latency_ms=request.avg_latency_ms,
        packet_loss_percent=request.packet_loss_percent,
        bandwidth_usage_mbps=request.bandwidth_usage_mbps,
    )

    if not success:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Failed to update metrics for session {session_id}",
        )

    return {"message": f"Session {session_id} metrics updated"}


@router.get("/host/{host_id}/active", response_model=List[SessionResponse])
async def get_active_sessions_for_host(
    host_id: str,
    db: AsyncSession = Depends(get_db_session),
):
    """Get all active sessions for a host"""
    sessions = await session_service.get_active_sessions_for_host(db, host_id)

    return [
        SessionResponse(
            id=session.id,
            session_id=session.session_id,
            host_id=session.host_id,
            zone_id=session.zone_id,
            connected_players=session.connected_players or [],
            current_players=session.current_players,
            max_players=session.max_players,
            status=session.status.value,
            average_latency_ms=session.average_latency_ms,
            average_packet_loss_percent=session.average_packet_loss_percent,
            bandwidth_usage_mbps=session.bandwidth_usage_mbps,
            started_at=session.started_at.isoformat() if session.started_at else None,
            ended_at=session.ended_at.isoformat() if session.ended_at else None,
            created_at=session.created_at.isoformat(),
            updated_at=session.updated_at.isoformat(),
        )
        for session in sessions
    ]


@router.get("/zone/{zone_id}/active", response_model=List[SessionResponse])
async def get_active_sessions_for_zone(
    zone_id: str,
    db: AsyncSession = Depends(get_db_session),
):
    """Get all active sessions for a zone"""
    sessions = await session_service.get_active_sessions_for_zone(db, zone_id)

    return [
        SessionResponse(
            id=session.id,
            session_id=session.session_id,
            host_id=session.host_id,
            zone_id=session.zone_id,
            connected_players=session.connected_players or [],
            current_players=session.current_players,
            max_players=session.max_players,
            status=session.status.value,
            average_latency_ms=session.average_latency_ms,
            average_packet_loss_percent=session.average_packet_loss_percent,
            bandwidth_usage_mbps=session.bandwidth_usage_mbps,
            started_at=session.started_at.isoformat() if session.started_at else None,
            ended_at=session.ended_at.isoformat() if session.ended_at else None,
            created_at=session.created_at.isoformat(),
            updated_at=session.updated_at.isoformat(),
        )
        for session in sessions
    ]


@router.post("/{session_id}/players", status_code=status.HTTP_200_OK)
async def add_player_to_session(
    session_id: int,
    request: dict,
    db: AsyncSession = Depends(get_db_session),
):
    """Add a player to a session"""
    player_id = request.get("player_id")
    if not player_id:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="player_id is required",
        )

    success = await session_service.add_player_to_session(db, session_id, player_id)
    if not success:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Failed to add player {player_id} to session {session_id}",
        )

    return {"message": f"Player {player_id} added to session {session_id}"}


@router.delete("/{session_id}/players/{player_id}", status_code=status.HTTP_200_OK)
async def remove_player_from_session(
    session_id: int,
    player_id: str,
    db: AsyncSession = Depends(get_db_session),
):
    """Remove a player from a session"""
    success = await session_service.remove_player_from_session(db, session_id, player_id)
    if not success:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Failed to remove player {player_id} from session {session_id}",
        )

    return {"message": f"Player {player_id} removed from session {session_id}"}


@router.post("/cleanup", status_code=status.HTTP_200_OK)
async def cleanup_stale_sessions(
    timeout_minutes: int = 30,
    db: AsyncSession = Depends(get_db_session),
):
    """Clean up stale sessions"""
    count = await session_service.cleanup_stale_sessions(db, timeout_minutes)
    return {"message": f"Cleaned up {count} stale sessions"}

