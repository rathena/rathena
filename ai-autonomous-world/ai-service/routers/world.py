"""
World state API endpoints
"""

from fastapi import APIRouter, HTTPException, status, Query
from loguru import logger
from typing import Optional
from datetime import datetime

from ..models.world import (
    WorldStateUpdateRequest,
    WorldStateUpdateResponse,
    WorldStateQueryResponse,
    EconomyState,
    PoliticsState,
    EnvironmentState,
)
from ..database import db

router = APIRouter(prefix="/ai/world", tags=["world"])


@router.post("/state", response_model=WorldStateUpdateResponse)
async def update_world_state(request: WorldStateUpdateRequest):
    """
    Update world state in the AI service
    
    Updates economy, politics, environment, or culture state.
    """
    try:
        logger.info(f"Updating world state: {request.state_type}")
        
        # Add timestamp to state data
        state_data = {
            **request.state_data,
            "timestamp": request.timestamp.isoformat(),
            "source": request.source or "unknown",
        }
        
        # Store in database
        await db.set_world_state(request.state_type, state_data)
        
        logger.info(f"World state {request.state_type} updated successfully")
        
        return WorldStateUpdateResponse(
            status="success",
            state_type=request.state_type,
            message=f"World state {request.state_type} updated successfully"
        )
        
    except Exception as e:
        logger.error(f"Error updating world state {request.state_type}: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update world state: {str(e)}"
        )


@router.get("/state", response_model=WorldStateQueryResponse)
async def get_world_state(
    state_type: Optional[str] = Query(None, description="State type to query (economy/politics/environment/culture)")
):
    """
    Get current world state from the AI service
    
    Returns all world state or specific state type if specified.
    """
    try:
        logger.info(f"Querying world state: {state_type or 'all'}")
        
        response = WorldStateQueryResponse()
        
        if state_type is None or state_type == "economy":
            # Get economy state
            economy_data = await db.get_world_state("economy")
            if economy_data:
                response.economy = EconomyState(
                    item_prices=economy_data.get("item_prices", {}),
                    supply_demand=economy_data.get("supply_demand", {}),
                    trade_volume=float(economy_data.get("trade_volume", 0.0)),
                    inflation_rate=float(economy_data.get("inflation_rate", 0.0)),
                    timestamp=datetime.fromisoformat(economy_data.get("timestamp", datetime.utcnow().isoformat()))
                )
        
        if state_type is None or state_type == "politics":
            # Get politics state
            politics_data = await db.get_world_state("politics")
            if politics_data:
                response.politics = PoliticsState(
                    faction_relations=politics_data.get("faction_relations", {}),
                    territory_control=politics_data.get("territory_control", {}),
                    active_conflicts=politics_data.get("active_conflicts", []),
                    diplomatic_events=politics_data.get("diplomatic_events", []),
                    timestamp=datetime.fromisoformat(politics_data.get("timestamp", datetime.utcnow().isoformat()))
                )
        
        if state_type is None or state_type == "environment":
            # Get environment state
            env_data = await db.get_world_state("environment")
            if env_data:
                response.environment = EnvironmentState(
                    weather=env_data.get("weather", "clear"),
                    time_of_day=env_data.get("time_of_day", "day"),
                    season=env_data.get("season", "spring"),
                    resource_availability=env_data.get("resource_availability", {}),
                    timestamp=datetime.fromisoformat(env_data.get("timestamp", datetime.utcnow().isoformat()))
                )
        
        if state_type is None or state_type == "culture":
            # Get culture state
            culture_data = await db.get_world_state("culture")
            if culture_data:
                response.culture = culture_data
        
        response.state_type = state_type
        
        logger.info(f"World state query completed: {state_type or 'all'}")
        
        return response
        
    except Exception as e:
        logger.error(f"Error querying world state: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to query world state: {str(e)}"
        )

