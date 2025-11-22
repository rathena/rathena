"""
Navigation Router

Handles pathfinding and map navigation services for NPCs and players.
"""

import logging
from typing import List, Optional
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

logger = logging.getLogger(__name__)

router = APIRouter(
    prefix="/api/navigation",
    tags=["navigation"],
    responses={
        401: {"description": "Unauthorized - Invalid API key"},
        429: {"description": "Too many requests"},
        500: {"description": "Internal server error"}
    }
)

# ============================================================================
# REQUEST/RESPONSE MODELS
# ============================================================================

class Position(BaseModel):
    """2D position."""
    x: float
    y: float


class PathfindRequest(BaseModel):
    """Request model for pathfinding."""
    
    start: Position = Field(..., description="Start position")
    end: Position = Field(..., description="Destination position")
    map_name: str = Field(..., description="Map identifier")
    avoid_obstacles: bool = Field(default=True, description="Enable obstacle avoidance")
    max_distance: Optional[float] = Field(None, description="Maximum path distance")


class PathNode(BaseModel):
    """Path waypoint."""
    position: Position
    distance_from_start: float
    estimated_time: float


class PathfindResponse(BaseModel):
    """Response model for pathfinding."""
    
    path: List[PathNode]
    total_distance: float
    estimated_duration: float
    is_valid: bool
    warnings: List[str] = Field(default_factory=list)


class MapInfoResponse(BaseModel):
    """Response model for map information."""
    
    map_name: str
    width: float
    height: float
    walkable_area_percentage: float
    points_of_interest: List[dict]


# ============================================================================
# ENDPOINTS
# ============================================================================

@router.post(
    "/pathfind",
    response_model=PathfindResponse,
    summary="Calculate Path",
    description="Calculate optimal path between two points"
)
async def calculate_path(
    request: PathfindRequest,
    orchestrator: Orchestrator,
    correlation_id: CorrelationID,
    _rate_check: RateLimited
) -> dict:
    """Calculate pathfinding route."""
    try:
        logger.info(
            f"Calculating path: map={request.map_name}, "
            f"from=({request.start.x},{request.start.y}) to=({request.end.x},{request.end.y})"
        )
        
        # TODO: Implement A* pathfinding
        # path = await orchestrator.navigation_manager.find_path(request)
        
        # Mock simple path
        distance = ((request.end.x - request.start.x)**2 + 
                   (request.end.y - request.start.y)**2)**0.5
        
        response_data = PathfindResponse(
            path=[
                PathNode(position=request.start, distance_from_start=0, estimated_time=0),
                PathNode(position=request.end, distance_from_start=distance, estimated_time=distance/5.0)
            ],
            total_distance=distance,
            estimated_duration=distance/5.0,
            is_valid=True,
            warnings=[]
        )
        
        return create_success_response(
            data=response_data.model_dump(),
            message="Path calculated",
            request_id=correlation_id
        )
        
    except Exception as e:
        logger.error(f"Error calculating path: {str(e)}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=create_error_response(
                code="INTERNAL_ERROR",
                message="Failed to calculate path",
                request_id=correlation_id
            )
        )


@router.get(
    "/map",
    response_model=MapInfoResponse,
    summary="Get Map Info",
    description="Retrieve map information and metadata"
)
async def get_map_info(
    map_name: str,
    orchestrator: Orchestrator,
    correlation_id: CorrelationID,
    _rate_check: RateLimited
) -> dict:
    """Get map information."""
    try:
        logger.info(f"Fetching map info: {map_name}")
        
        # TODO: Fetch map data
        # map_info = await orchestrator.navigation_manager.get_map_info(map_name)
        
        response_data = MapInfoResponse(
            map_name=map_name,
            width=500.0,
            height=500.0,
            walkable_area_percentage=75.0,
            points_of_interest=[
                {"name": "Town Center", "x": 250, "y": 250},
                {"name": "Shop District", "x": 300, "y": 200}
            ]
        )
        
        return create_success_response(
            data=response_data.model_dump(),
            message="Map info retrieved",
            request_id=correlation_id
        )
        
    except Exception as e:
        logger.error(f"Error fetching map info: {str(e)}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=create_error_response(
                code="INTERNAL_ERROR",
                message="Failed to fetch map info",
                request_id=correlation_id
            )
        )