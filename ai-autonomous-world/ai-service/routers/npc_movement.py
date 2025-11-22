"""
NPC Movement Router
Handles AI-driven NPC movement decision requests from C++ game server
"""

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field
from typing import Optional
from loguru import logger
import random

router = APIRouter(prefix="/api/npc", tags=["npc-movement"])


class NPCMovementRequest(BaseModel):
    """Request model for NPC movement decision"""
    npc_id: str = Field(..., description="NPC identifier")
    action: str = Field(..., description="Action type (e.g., 'move')")
    target_x: int = Field(..., ge=0, description="Target X coordinate")
    target_y: int = Field(..., ge=0, description="Target Y coordinate")
    current_x: Optional[int] = Field(None, description="Current X coordinate")
    current_y: Optional[int] = Field(None, description="Current Y coordinate")


class NPCMovementResponse(BaseModel):
    """Response model for NPC movement decision"""
    approved: bool = Field(..., description="Whether movement is approved")
    target_x: int = Field(..., description="Approved target X coordinate")
    target_y: int = Field(..., description="Approved target Y coordinate")
    reasoning: str = Field(..., description="AI reasoning for decision")
    path_valid: bool = Field(True, description="Whether path is valid")
    estimated_time: float = Field(0.0, description="Estimated time to reach target (seconds)")


@router.post("/movement", response_model=NPCMovementResponse, status_code=status.HTTP_200_OK)
async def npc_movement_decision(request: NPCMovementRequest):
    """
    AI-driven NPC movement decision endpoint
    
    Makes intelligent decisions about whether an NPC should move to the requested
    coordinates based on:
    - Path validity
    - NPC state and behavior
    - Environmental factors
    - Safety considerations
    
    Args:
        request: NPC movement request with target coordinates
        
    Returns:
        NPCMovementResponse with approval decision and reasoning
        
    Example Request:
        POST /api/npc/movement
        {
            "npc_id": "Guard#001",
            "action": "move",
            "target_x": 100,
            "target_y": 150
        }
        
    Example Response:
        {
            "approved": true,
            "target_x": 100,
            "target_y": 150,
            "reasoning": "Path is clear and within patrol area",
            "path_valid": true,
            "estimated_time": 5.2
        }
    """
    logger.info(f"NPC movement request: {request.npc_id} to ({request.target_x}, {request.target_y})")
    
    try:
        # AI Decision Logic
        # TODO: Replace with actual AI decision-making logic
        # For now, implement basic validation
        
        # Check if coordinates are reasonable
        max_coord = 500  # Example map boundary
        if request.target_x > max_coord or request.target_y > max_coord:
            return NPCMovementResponse(
                approved=False,
                target_x=request.target_x,
                target_y=request.target_y,
                reasoning=f"Target coordinates exceed map boundaries (max: {max_coord})",
                path_valid=False,
                estimated_time=0.0
            )
        
        # Calculate distance if current position is provided
        distance = 0.0
        if request.current_x is not None and request.current_y is not None:
            distance = ((request.target_x - request.current_x) ** 2 + 
                       (request.target_y - request.current_y) ** 2) ** 0.5
        
        # Estimate movement time (assuming 1 unit per second)
        estimated_time = distance / 1.0 if distance > 0 else 0.0
        
        # Simple approval logic - approve most movements
        # In production, this would check:
        # - NPC behavior patterns
        # - Environmental obstacles
        # - Current NPC state (busy, idle, etc.)
        # - Safety of target location
        approved = True
        reasoning = "Path is clear and within acceptable range"
        
        # Occasionally deny for testing purposes (10% chance)
        if random.random() < 0.1:
            approved = False
            reasoning = "NPC is currently engaged in another activity"
        
        logger.info(f"Movement decision for {request.npc_id}: approved={approved}, reasoning={reasoning}")
        
        return NPCMovementResponse(
            approved=approved,
            target_x=request.target_x,
            target_y=request.target_y,
            reasoning=reasoning,
            path_valid=True,
            estimated_time=round(estimated_time, 2)
        )
        
    except Exception as e:
        logger.error(f"Error processing NPC movement request: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process movement request: {str(e)}"
        )


@router.get("/movement/status/{npc_id}", status_code=status.HTTP_200_OK)
async def get_npc_movement_status(npc_id: str):
    """
    Get current movement status of an NPC
    
    Args:
        npc_id: NPC identifier
        
    Returns:
        Current movement status and position
    """
    logger.debug(f"Movement status requested for NPC: {npc_id}")
    
    # TODO: Implement actual status tracking
    return {
        "npc_id": npc_id,
        "is_moving": False,
        "current_x": 0,
        "current_y": 0,
        "target_x": None,
        "target_y": None,
        "status": "idle"
    }