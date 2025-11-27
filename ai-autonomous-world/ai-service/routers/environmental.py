"""
Environmental Agents API Router

FastAPI endpoints for:
- Map Hazard Agent (5 endpoints)
- Weather/Time Agent (4 endpoints)
- Treasure Agent (5 endpoints)
- Admin/Debug endpoints (2 endpoints)

Total: 16 endpoints
"""

from fastapi import APIRouter, HTTPException, Query
from typing import List, Optional
from datetime import datetime, UTC
from loguru import logger

from agents.environmental.map_hazard_agent import map_hazard_agent, HazardType
from agents.environmental.weather_time_agent import weather_time_agent, WeatherType, TimeOfDay
from agents.environmental.treasure_agent import treasure_agent, TreasureRarity
from models.environmental import (
    # Map Hazard Models
    HazardGenerateRequest,
    HazardListResponse,
    HazardResponse,
    HazardEncounterModel,
    MapActivityModel,
    ProblemModel,
    # Weather/Time Models
    WeatherUpdateRequest,
    WeatherResponse,
    WeatherForecastResponse,
    CombinedEffectsResponse,
    TimeEffectsResponse,
    # Treasure Models
    TreasureSpawnRequest,
    TreasureSpawnResponse,
    TreasureDiscoveryResponse,
    CardFragmentResponse,
    CardClaimResponse,
    TreasureList,
    TreasureResponse,
    # Generic Models
    EnvironmentalStatusResponse,
    EnvironmentalMetricsResponse,
    ErrorResponse
)
from database import postgres_db

router = APIRouter(prefix="/api/v1/environmental", tags=["environmental"])


# ============================================================================
# MAP HAZARD ENDPOINTS
# ============================================================================

@router.post("/hazards/generate", response_model=HazardListResponse)
async def generate_daily_hazards(request: HazardGenerateRequest):
    """
    Generate daily map hazards.
    
    - Selects 3-5 low-traffic maps
    - Matches hazard types to map themes
    - Balances positive/negative hazards
    - Coordinates with active problems
    """
    try:
        response = await map_hazard_agent.generate_daily_hazards(
            map_activity=request.map_activity,
            active_problems=[p.model_dump() for p in request.active_problems],
            count=request.count
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error'))
        
        return response.data
        
    except Exception as e:
        logger.error(f"Failed to generate hazards: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/hazards/active", response_model=List[HazardResponse])
async def get_active_hazards():
    """List all active map hazards"""
    try:
        hazards = await map_hazard_agent.get_active_hazards()
        return hazards
        
    except Exception as e:
        logger.error(f"Failed to get active hazards: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/hazards/{map_name}", response_model=List[HazardResponse])
async def get_map_hazards(map_name: str):
    """Get hazards active on specific map"""
    try:
        hazards = await map_hazard_agent.get_map_hazards(map_name)
        return hazards
        
    except Exception as e:
        logger.error(f"Failed to get map hazards: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/hazards/{hazard_id}/encounter")
async def record_hazard_encounter(
    hazard_id: int,
    player_id: int,
    encounter_data: HazardEncounterModel
):
    """Record player encounter with hazard"""
    try:
        success = await map_hazard_agent.record_hazard_encounter(
            hazard_id=hazard_id,
            player_id=player_id,
            time_in_hazard=encounter_data.time_in_hazard,
            items_dropped=encounter_data.items_dropped,
            deaths=encounter_data.deaths
        )
        
        if not success:
            raise HTTPException(status_code=500, detail="Failed to record encounter")
        
        return {"success": True, "message": "Encounter recorded"}
        
    except Exception as e:
        logger.error(f"Failed to record hazard encounter: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.delete("/hazards/cleanup")
async def cleanup_expired_hazards():
    """Remove expired hazards (admin endpoint)"""
    try:
        count = await map_hazard_agent.remove_expired_hazards()
        return {"removed": count}
        
    except Exception as e:
        logger.error(f"Failed to cleanup hazards: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# WEATHER/TIME ENDPOINTS
# ============================================================================

@router.get("/weather/current", response_model=WeatherResponse)
async def get_current_weather():
    """Get current weather and effects"""
    try:
        weather = await weather_time_agent.get_current_weather()
        
        if not weather:
            # Initialize with clear weather
            response = await weather_time_agent.update_weather()
            if response.success:
                weather = response.data
            else:
                raise HTTPException(status_code=500, detail="Failed to initialize weather")
        
        return weather
        
    except Exception as e:
        logger.error(f"Failed to get current weather: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/weather/update", response_model=WeatherResponse)
async def update_weather(
    force_weather: Optional[WeatherType] = Query(None, description="Force specific weather (for events)")
):
    """Update weather (can force specific weather for events)"""
    try:
        response = await weather_time_agent.update_weather(
            force_weather=force_weather
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error'))
        
        return response.data
        
    except Exception as e:
        logger.error(f"Failed to update weather: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/weather/forecast", response_model=WeatherForecastResponse)
async def get_weather_forecast(
    hours: int = Query(24, ge=6, le=72, description="Hours to forecast")
):
    """Get weather patterns for next N hours"""
    try:
        forecast = await weather_time_agent.get_weather_forecast(hours)
        
        return {
            "forecast": forecast,
            "forecast_hours": hours
        }
        
    except Exception as e:
        logger.error(f"Failed to get weather forecast: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/time/current", response_model=TimeEffectsResponse)
async def get_current_time_effects():
    """Get time-of-day effects"""
    try:
        effects = await weather_time_agent.get_time_of_day_effects()
        return effects
        
    except Exception as e:
        logger.error(f"Failed to get time effects: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# TREASURE ENDPOINTS
# ============================================================================

@router.post("/treasure/spawn", response_model=TreasureSpawnResponse)
async def spawn_daily_treasures(request: TreasureSpawnRequest):
    """Spawn treasures in low-traffic maps"""
    try:
        response = await treasure_agent.spawn_daily_treasures(
            map_activity=request.map_activity,
            count=request.count
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error'))
        
        return response.data
        
    except Exception as e:
        logger.error(f"Failed to spawn treasures: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/treasure/active", response_model=TreasureList)
async def get_active_treasures(
    include_hints: bool = Query(False, description="Include hints and positions (admin only)")
):
    """List active treasures"""
    try:
        treasures = await treasure_agent.get_active_treasures(include_hints=include_hints)
        
        return {
            "treasures": treasures,
            "total": len(treasures)
        }
        
    except Exception as e:
        logger.error(f"Failed to get active treasures: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/treasure/{treasure_id}/discover", response_model=TreasureDiscoveryResponse)
async def discover_treasure(
    treasure_id: int,
    player_id: int = Query(..., description="Discovering player ID")
):
    """Handle treasure discovery and reward distribution"""
    try:
        response = await treasure_agent.discover_treasure(
            treasure_id=treasure_id,
            player_id=player_id
        )
        
        if not response.success:
            raise HTTPException(
                status_code=400,
                detail=response.data.get('error', 'Discovery failed')
            )
        
        return response.data
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to discover treasure: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/treasure/{player_id}/fragments", response_model=CardFragmentResponse)
async def get_card_fragments(player_id: int):
    """Get player's card fragment count"""
    try:
        fragments = await treasure_agent.get_card_fragments(player_id)
        return fragments
        
    except Exception as e:
        logger.error(f"Failed to get card fragments: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/treasure/{player_id}/claim-card", response_model=CardClaimResponse)
async def claim_card_from_fragments(player_id: int):
    """Exchange 100 fragments for random card"""
    try:
        response = await treasure_agent.claim_card_from_fragments(player_id)
        
        if not response.success:
            raise HTTPException(
                status_code=400,
                detail=response.data.get('error', 'Claim failed')
            )
        
        return response.data
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to claim card: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.delete("/treasure/cleanup")
async def cleanup_expired_treasures():
    """Remove expired treasures (admin endpoint)"""
    try:
        count = await treasure_agent.despawn_expired_treasures()
        return {"removed": count}
        
    except Exception as e:
        logger.error(f"Failed to cleanup treasures: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# ADMIN/DEBUG ENDPOINTS
# ============================================================================

@router.get("/status", response_model=EnvironmentalStatusResponse)
async def get_environmental_status():
    """Get environmental system status"""
    try:
        # Get active counts
        active_hazards = await map_hazard_agent.get_active_hazards()
        current_weather = await weather_time_agent.get_current_weather()
        active_treasures = await treasure_agent.get_active_treasures()
        
        return {
            "map_hazard_enabled": getattr(settings, 'map_hazard_enabled', True),
            "weather_time_enabled": getattr(settings, 'weather_time_enabled', True),
            "treasure_enabled": getattr(settings, 'treasure_agent_enabled', True),
            "active_hazards": len(active_hazards),
            "current_weather": current_weather['weather_type'] if current_weather else None,
            "active_treasures": len(active_treasures),
            "timestamp": datetime.now(UTC).isoformat()
        }
        
    except Exception as e:
        logger.error(f"Failed to get environmental status: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/metrics", response_model=EnvironmentalMetricsResponse)
async def get_environmental_metrics():
    """Get environmental system metrics"""
    try:
        # Get today's metrics
        today_start = datetime.now(UTC).replace(hour=0, minute=0, second=0, microsecond=0)
        
        # Hazards generated today
        hazards_query = """
            SELECT COUNT(*) as count
            FROM map_hazards
            WHERE applied_at >= $1
        """
        hazards_result = await postgres_db.fetch_one(hazards_query, today_start)
        
        # Weather changes today
        weather_query = """
            SELECT COUNT(*) as count
            FROM weather_history
            WHERE timestamp >= $1
        """
        weather_result = await postgres_db.fetch_one(weather_query, today_start)
        
        # Treasures spawned today
        treasures_spawned_query = """
            SELECT COUNT(*) as count
            FROM treasure_spawns
            WHERE spawned_at >= $1
        """
        treasures_spawned_result = await postgres_db.fetch_one(treasures_spawned_query, today_start)
        
        # Treasures discovered today
        treasures_discovered_query = """
            SELECT COUNT(*) as count
            FROM treasure_discoveries
            WHERE timestamp >= $1
        """
        treasures_discovered_result = await postgres_db.fetch_one(treasures_discovered_query, today_start)
        
        # Fragments awarded today
        fragments_query = """
            SELECT COALESCE(SUM((rewards_claimed::jsonb->>'card_fragments')::int), 0) as total
            FROM treasure_discoveries
            WHERE timestamp >= $1
        """
        fragments_result = await postgres_db.fetch_one(fragments_query, today_start)
        
        return {
            "hazards_generated_today": hazards_result['count'] if hazards_result else 0,
            "weather_changes_today": weather_result['count'] if weather_result else 0,
            "treasures_spawned_today": treasures_spawned_result['count'] if treasures_spawned_result else 0,
            "treasures_discovered_today": treasures_discovered_result['count'] if treasures_discovered_result else 0,
            "total_fragments_awarded_today": fragments_result['total'] if fragments_result else 0,
            "timestamp": datetime.now(UTC).isoformat()
        }
        
    except Exception as e:
        logger.error(f"Failed to get environmental metrics: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# HEALTH CHECK
# ============================================================================

@router.get("/health")
async def environmental_health_check():
    """Health check for environmental systems"""
    return {
        "status": "healthy",
        "agents": ["map_hazard", "weather_time", "treasure"],
        "timestamp": datetime.now(UTC).isoformat()
    }