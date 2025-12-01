"""
Procedural Content API Router
Exposes endpoints for Problem Agent, Dynamic NPC Agent, and World Event Agent
"""

from typing import List, Optional
from fastapi import APIRouter, HTTPException, status, Depends
from loguru import logger

from agents.procedural.problem_agent import problem_agent
from agents.procedural.dynamic_npc_agent import dynamic_npc_agent
from agents.procedural.world_event_agent import world_event_agent

from models.procedural import (
    # Problem models
    ProblemGenerateRequest,
    ProblemResponse,
    ProblemParticipationRequest,
    ProblemsListResponse,
    
    # NPC models
    NPCSpawnRequest,
    DynamicNPCResponse,
    NPCInteractionRequest,
    NPCInteractionResponse,
    NPCsListResponse,
    
    # Event models
    EventCheckRequest,
    EventTriggerRequest,
    WorldEventResponse,
    EventParticipationRequest,
    EventsListResponse,
    
    # Generic
    ProceduralAgentResponse,
    
    # Enums
    ProblemType,
    EventType
)


router = APIRouter(
    prefix="/api/v1/procedural",
    tags=["procedural"]
)


# ============================================================================
# PROBLEM AGENT ENDPOINTS
# ============================================================================

@router.post("/problem/generate", response_model=ProblemResponse)
async def generate_daily_problem(request: ProblemGenerateRequest):
    """
    Generate today's world problem.
    
    Called by:
    - Daily cron job at midnight
    - Manual trigger by admin
    
    Returns:
        Problem details with rewards and spawn data
    """
    try:
        logger.info("API: Generating daily problem")
        
        response = await problem_agent.generate_daily_problem(
            world_state=request.world_state.model_dump(),
            force_type=request.force_type.value if request.force_type else None
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to generate problem"
            )
        
        return ProblemResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error generating problem: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/problem/current", response_model=Optional[ProblemResponse])
async def get_current_problem():
    """
    Get currently active problem.
    
    Returns:
        Active problem or None if no problem active
    """
    try:
        problem = await problem_agent.get_current_problem()
        
        if problem:
            return ProblemResponse(**problem)
        
        return None
        
    except Exception as e:
        logger.error(f"API error getting current problem: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/problem/{problem_id}/participate", response_model=ProceduralAgentResponse)
async def record_problem_participation(
    problem_id: int,
    request: ProblemParticipationRequest
):
    """
    Record player participation in problem.
    
    Called when:
    - Player kills problem monster
    - Player completes problem objective
    - Player claims reward
    """
    try:
        success = await problem_agent.record_participation(
            problem_id=problem_id,
            player_id=request.player_id,
            action_type=request.action_type,
            action_data=request.action_data,
            contribution_score=request.contribution_score
        )
        
        if success:
            return ProceduralAgentResponse(
                success=True,
                message="Participation recorded"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail="Failed to record participation"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording participation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/problem/{problem_id}/complete", response_model=ProceduralAgentResponse)
async def complete_problem(problem_id: int):
    """
    Mark problem as completed.
    
    Called when:
    - Problem objectives fulfilled
    - Admin manually completes problem
    """
    try:
        success = await problem_agent.complete_problem(problem_id)
        
        if success:
            return ProceduralAgentResponse(
                success=True,
                message=f"Problem #{problem_id} completed"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Problem #{problem_id} not found or already completed"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error completing problem: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# DYNAMIC NPC AGENT ENDPOINTS
# ============================================================================

@router.post("/npc/spawn", response_model=NPCsListResponse)
async def spawn_dynamic_npcs(request: NPCSpawnRequest):
    """
    Spawn roaming NPCs in low-traffic maps.
    
    Called by:
    - Daily cron job at 6 AM
    - Manual trigger by admin
    
    Returns:
        List of spawned NPCs
    """
    try:
        logger.info(f"API: Spawning {request.count} dynamic NPCs")
        
        responses = await dynamic_npc_agent.spawn_daily_npcs(
            map_activity=request.map_activity.map_activity,
            heatmap_data=request.map_activity.heatmap_data,
            count=request.count
        )
        
        spawned_npcs = [
            DynamicNPCResponse(**resp.data)
            for resp in responses
            if resp.success
        ]
        
        return NPCsListResponse(
            npcs=spawned_npcs,
            total=len(spawned_npcs),
            active_count=len(spawned_npcs)
        )
        
    except Exception as e:
        logger.error(f"API error spawning NPCs: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/npc/active", response_model=NPCsListResponse)
async def get_active_npcs():
    """
    List all currently active dynamic NPCs.
    
    Returns:
        List of active NPCs with locations and quest info
    """
    try:
        active_npcs = await dynamic_npc_agent.get_active_npcs()
        
        npc_responses = [
            DynamicNPCResponse(**npc)
            for npc in active_npcs
        ]
        
        return NPCsListResponse(
            npcs=npc_responses,
            total=len(npc_responses),
            active_count=len(npc_responses)
        )
        
    except Exception as e:
        logger.error(f"API error getting active NPCs: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/npc/{npc_id}/interact", response_model=NPCInteractionResponse)
async def interact_with_npc(
    npc_id: int,
    request: NPCInteractionRequest
):
    """
    Handle player-NPC interaction.
    
    Called when:
    - Player talks to NPC
    - Player starts quest
    - Player completes quest
    
    Returns:
        NPC dialogue response and optional rewards
    """
    try:
        response = await dynamic_npc_agent.handle_npc_interaction(
            npc_id=npc_id,
            player_id=request.player_id,
            interaction_type=request.interaction_type,
            player_message=request.player_message
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=response.reasoning or "NPC not found or inactive"
            )
        
        return NPCInteractionResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error handling NPC interaction: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/npc/despawn-expired", response_model=ProceduralAgentResponse)
async def despawn_expired_npcs():
    """
    Despawn NPCs that have expired.
    
    Called by:
    - Daily cron job at end of day
    - Manual trigger by admin
    
    Returns:
        Number of NPCs despawned
    """
    try:
        count = await dynamic_npc_agent.despawn_expired_npcs()
        
        return ProceduralAgentResponse(
            success=True,
            message=f"Despawned {count} expired NPCs",
            data={"count": count}
        )
        
    except Exception as e:
        logger.error(f"API error despawning NPCs: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# WORLD EVENT AGENT ENDPOINTS
# ============================================================================

@router.post("/events/check", response_model=Optional[WorldEventResponse])
async def check_event_triggers(request: EventCheckRequest):
    """
    Check if any event thresholds are met and trigger if so.
    
    Called by:
    - Background task every 60 seconds
    - Manual check by admin
    
    Returns:
        Event details if triggered, None otherwise
    """
    try:
        response = await world_event_agent.monitor_and_trigger(
            world_state=request.world_state.model_dump()
        )
        
        if response and response.success:
            return WorldEventResponse(**response.data)
        
        return None
        
    except Exception as e:
        logger.error(f"API error checking events: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/events/trigger", response_model=WorldEventResponse)
async def trigger_world_event(request: EventTriggerRequest):
    """
    Manually trigger a world event (admin only).
    
    Used for:
    - Testing events
    - GM-triggered special events
    - Scheduled promotional events
    
    Returns:
        Event details
    """
    try:
        logger.info(f"API: Manually triggering {request.event_type} event")
        
        # Build minimal world state for manual trigger
        world_state = {
            "avg_player_level": 50,
            "online_players": 10,
            "map_activity": {},
            "monster_kills": {},
            "mvp_kills": {},
            "economy": {}
        }
        
        response = await world_event_agent.trigger_event(
            event_type=request.event_type.value,
            world_state=world_state,
            custom_impact=request.custom_impact.model_dump() if request.custom_impact else None
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to trigger event"
            )
        
        return WorldEventResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error triggering event: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/events/active", response_model=EventsListResponse)
async def get_active_events():
    """
    List all currently active world events.
    
    Returns:
        List of active events with impact data
    """
    try:
        active_events = await world_event_agent.get_active_events()
        
        event_responses = [
            WorldEventResponse(**event)
            for event in active_events
        ]
        
        return EventsListResponse(
            events=event_responses,
            total=len(event_responses),
            active_count=len(event_responses)
        )
        
    except Exception as e:
        logger.error(f"API error getting active events: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/events/{event_id}/participate", response_model=ProceduralAgentResponse)
async def record_event_participation(
    event_id: int,
    request: EventParticipationRequest
):
    """
    Record player participation in event.
    
    Called when:
    - Player participates in event activity
    - Player claims event rewards
    
    Returns:
        Success confirmation
    """
    try:
        success = await world_event_agent.record_participation(
            event_id=event_id,
            player_id=request.player_id,
            contribution_score=request.contribution_score
        )
        
        if success:
            return ProceduralAgentResponse(
                success=True,
                message="Event participation recorded"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail="Failed to record participation"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording event participation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/events/{event_id}/end", response_model=ProceduralAgentResponse)
async def end_world_event(event_id: int):
    """
    End an active event.
    
    Called by:
    - Background task when event expires
    - Manual trigger by admin
    
    Returns:
        Success confirmation
    """
    try:
        success = await world_event_agent.end_event(event_id)
        
        if success:
            return ProceduralAgentResponse(
                success=True,
                message=f"Event #{event_id} ended"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Event #{event_id} not found or already ended"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error ending event: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# ADMIN/DEBUG ENDPOINTS
# ============================================================================

@router.get("/status", response_model=Dict[str, Any])
async def get_procedural_status():
    """
    Get overall procedural content status.
    
    Returns:
        Current state of all procedural systems
    """
    try:
        current_problem = await problem_agent.get_current_problem()
        active_npcs = await dynamic_npc_agent.get_active_npcs()
        active_events = await world_event_agent.get_active_events()
        
        return {
            "current_problem": {
                "active": current_problem is not None,
                "problem_id": current_problem.get('problem_id') if current_problem else None,
                "title": current_problem.get('title') if current_problem else None,
                "expires_at": current_problem.get('expires_at').isoformat() if current_problem else None
            },
            "dynamic_npcs": {
                "active_count": len(active_npcs),
                "npcs": [
                    {
                        "npc_id": npc['npc_id'],
                        "name": npc.get('personality_data', {}).get('name', 'Unknown'),
                        "type": npc['npc_type'],
                        "map": npc['spawn_map']
                    }
                    for npc in active_npcs
                ]
            },
            "world_events": {
                "active_count": len(active_events),
                "events": [
                    {
                        "event_id": event['event_id'],
                        "title": event['title'],
                        "type": event['event_type'],
                        "ends_at": event['ends_at'].isoformat() if isinstance(event['ends_at'], datetime) else event['ends_at']
                    }
                    for event in active_events
                ]
            }
        }
        
    except Exception as e:
        logger.error(f"API error getting procedural status: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/metrics", response_model=Dict[str, Any])
async def get_procedural_metrics():
    """
    Get procedural content metrics.
    
    Returns:
        Statistics about procedural content generation
    """
    try:
        from database import postgres_db
        
        # Query statistics
        stats = {}
        
        # Problem stats
        problem_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total,
                COUNT(*) FILTER (WHERE status = 'active') as active,
                COUNT(*) FILTER (WHERE status = 'completed') as completed,
                AVG(participation_count) as avg_participation
            FROM world_problems
            WHERE created_at > NOW() - INTERVAL '7 days'
        """)
        stats['problems'] = dict(problem_stats) if problem_stats else {}
        
        # NPC stats
        npc_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total,
                COUNT(*) FILTER (WHERE status = 'active') as active,
                AVG(interaction_count) as avg_interactions
            FROM dynamic_npcs
            WHERE spawned_at > NOW() - INTERVAL '7 days'
        """)
        stats['npcs'] = dict(npc_stats) if npc_stats else {}
        
        # Event stats
        event_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total,
                COUNT(*) FILTER (WHERE status = 'active') as active,
                AVG(participation_count) as avg_participation,
                AVG(duration_minutes) as avg_duration
            FROM world_events
            WHERE started_at > NOW() - INTERVAL '7 days'
        """)
        stats['events'] = dict(event_stats) if event_stats else {}
        
        return stats
        
    except Exception as e:
        logger.error(f"API error getting metrics: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )