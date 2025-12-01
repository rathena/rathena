"""
Emotional State Router

Handles NPC emotional state tracking, updates, and emotional transitions.
"""

import logging
from typing import Optional
from datetime import datetime

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field

from .dependencies import (
    Orchestrator,
    CorrelationID,
    RateLimited,
    create_success_response,
    create_error_response
)
from models import Emotion

logger = logging.getLogger(__name__)

router = APIRouter(
    prefix="/api/emotional-state",
    tags=["emotional-state"],
    responses={
        401: {"description": "Unauthorized - Invalid API key"},
        429: {"description": "Too many requests"},
        500: {"description": "Internal server error"}
    }
)

# ============================================================================
# REQUEST/RESPONSE MODELS
# ============================================================================

class EmotionUpdateRequest(BaseModel):
    """Request model for emotion update."""
    
    emotion: Emotion = Field(..., description="New emotion")
    intensity: float = Field(..., ge=0.0, le=1.0, description="Emotion intensity")
    trigger: Optional[str] = Field(None, description="What triggered the emotion")


class EmotionalStateResponse(BaseModel):
    """Response model for emotional state."""
    
    npc_id: str
    current_emotion: str
    emotion_intensity: float
    mood: str
    stress_level: float
    energy_level: float
    last_updated: datetime


# ============================================================================
# ENDPOINTS
# ============================================================================

@router.get(
    "/{npc_id}",
    response_model=EmotionalStateResponse,
    summary="Get Emotional State",
    description="Retrieve current emotional state of an NPC"
)
async def get_emotional_state(
    npc_id: str,
    orchestrator: Orchestrator,
    correlation_id: CorrelationID,
    _rate_check: RateLimited
) -> dict:
    """Get NPC emotional state."""
    try:
        logger.info(f"Fetching emotional state for NPC: {npc_id}")
        
        # TODO: Fetch from emotion manager
        # state = await orchestrator.emotion_manager.get_state(npc_id)
        
        response_data = EmotionalStateResponse(
            npc_id=npc_id,
            current_emotion="neutral",
            emotion_intensity=0.5,
            mood="calm",
            stress_level=0.2,
            energy_level=0.7,
            last_updated=datetime.utcnow()
        )
        
        return create_success_response(
            data=response_data.model_dump(),
            message="Emotional state retrieved",
            request_id=correlation_id
        )
        
    except Exception as e:
        logger.error(f"Error fetching emotional state: {str(e)}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=create_error_response(
                code="INTERNAL_ERROR",
                message="Failed to fetch emotional state",
                request_id=correlation_id
            )
        )


@router.post(
    "/{npc_id}/update",
    summary="Update Emotional State",
    description="Update NPC emotional state"
)
async def update_emotional_state(
    npc_id: str,
    request: EmotionUpdateRequest,
    orchestrator: Orchestrator,
    correlation_id: CorrelationID,
    _rate_check: RateLimited
) -> dict:
    """Update NPC emotional state."""
    try:
        logger.info(
            f"Updating emotional state: npc={npc_id}, "
            f"emotion={request.emotion}, intensity={request.intensity}"
        )
        
        # TODO: Update through emotion manager
        # await orchestrator.emotion_manager.update_state(npc_id, request)
        
        return create_success_response(
            data={
                "npc_id": npc_id,
                "emotion": request.emotion,
                "intensity": request.intensity,
                "trigger": request.trigger,
                "updated_at": datetime.utcnow().isoformat()
            },
            message="Emotional state updated",
            request_id=correlation_id
        )
        
    except Exception as e:
        logger.error(f"Error updating emotional state: {str(e)}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=create_error_response(
                code="INTERNAL_ERROR",
                message="Failed to update emotional state",
                request_id=correlation_id
            )
        )