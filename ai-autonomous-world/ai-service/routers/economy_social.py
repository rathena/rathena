"""
Economy & Social API Router
Exposes endpoints for Merchant Economy, Karma, and Social Interaction agents
"""

from typing import List, Optional, Dict, Any
from datetime import datetime, UTC
import json
from fastapi import APIRouter, HTTPException, status, Depends
from loguru import logger

from agents.economy_social.merchant_economy_agent import merchant_economy_agent
from agents.economy_social.karma_agent import karma_agent
from agents.economy_social.social_interaction_agent import social_interaction_agent

from models.economy_social import (
    # Economy models
    EconomyDataModel,
    PriceAdjustmentRequest,
    PriceAdjustmentResponse,
    ZenySinkEventRequest,
    ZenySinkEventResponse,
    EconomySnapshotResponse,
    DropRateRecommendationsResponse,
    
    # Karma models
    KarmaActionModel,
    KarmaUpdateRequest,
    KarmaUpdateResponse,
    GlobalKarmaResponse,
    PlayerKarmaResponse,
    KarmaHistoryResponse,
    
    # Social models
    PlayerDistributionModel,
    SocialEventSpawnRequest,
    SocialEventResponse,
    SocialParticipationRequest,
    SocialParticipationResponse,
    GuildTaskGenerateRequest,
    GuildTaskResponse,
    GuildTaskProgressRequest,
    SocialMetricsModel,
    SocialHealthResponse,
    SocialEventsListResponse,
    GuildTasksListResponse,
    
    # Generic
    EconomySocialAgentResponse,
    
    # Enums
    EconomicIndicator,
    KarmaAlignment,
    SocialEventType
)


router = APIRouter(
    prefix="/api/v1/economy_social",
    tags=["economy_social"]
)


# ============================================================================
# MERCHANT ECONOMY AGENT ENDPOINTS
# ============================================================================

@router.post("/economy/analyze", response_model=Dict[str, Any])
async def analyze_economy(economy_data: EconomyDataModel):
    """
    Analyze current economic health.
    
    Called by:
    - Background task every 6 hours
    - Manual check by admin
    
    Returns:
        Economic indicator and analysis
    """
    try:
        logger.info("API: Analyzing economy")
        
        indicator = await merchant_economy_agent.analyze_economic_health(
            economy_data=economy_data.model_dump()
        )
        
        return {
            "indicator": indicator.value,
            "severity": "high" if indicator in [EconomicIndicator.INFLATION, EconomicIndicator.ITEM_SCARCITY] else "low",
            "timestamp": datetime.now(UTC).isoformat()
        }
        
    except Exception as e:
        logger.error(f"API error analyzing economy: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/economy/adjust-prices", response_model=PriceAdjustmentResponse)
async def adjust_prices(request: PriceAdjustmentRequest):
    """
    Adjust merchant prices based on economic state.
    
    Called by:
    - Background task after economic analysis
    - Manual trigger by admin
    
    Returns:
        Price adjustment details
    """
    try:
        logger.info(f"API: Adjusting prices for {request.indicator.value}")
        
        response = await merchant_economy_agent.adjust_merchant_prices(
            indicator=request.indicator,
            affected_items=request.affected_items
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to adjust prices"
            )
        
        # Transform to response model
        return PriceAdjustmentResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error adjusting prices: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/economy/zeny-sink", response_model=ZenySinkEventResponse)
async def create_zeny_sink(request: ZenySinkEventRequest):
    """
    Create a zeny sink event to drain excess currency.
    
    Called when:
    - Inflation detected (auto-triggered)
    - Manual trigger by admin
    
    Returns:
        Zeny sink event details
    """
    try:
        logger.info(f"API: Creating zeny sink event (severity: {request.severity})")
        
        response = await merchant_economy_agent.create_zeny_sink_event(
            severity=request.severity
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to create zeny sink event"
            )
        
        return ZenySinkEventResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error creating zeny sink: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/economy/snapshot", response_model=Optional[EconomySnapshotResponse])
async def get_economy_snapshot():
    """
    Get current economic snapshot.
    
    Returns:
        Latest economic metrics or None
    """
    try:
        snapshot = await merchant_economy_agent.get_current_economy_snapshot()
        
        if snapshot:
            return EconomySnapshotResponse(**snapshot)
        
        return None
        
    except Exception as e:
        logger.error(f"API error getting snapshot: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/economy/history", response_model=List[EconomySnapshotResponse])
async def get_economy_history(days: int = 7):
    """
    Get historical economic data.
    
    Args:
        days: Number of days of history (default: 7)
    
    Returns:
        List of economic snapshots
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT *
            FROM economy_snapshots
            WHERE timestamp > NOW() - INTERVAL '%s days'
            ORDER BY timestamp DESC
        """
        
        snapshots = await postgres_db.fetch_all(query, days)
        
        return [EconomySnapshotResponse(**dict(snap)) for snap in snapshots]
        
    except Exception as e:
        logger.error(f"API error getting history: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/economy/drop-rate-recommendations", response_model=DropRateRecommendationsResponse)
async def get_drop_rate_recommendations(
    oversupplied_items: List[str],
    undersupplied_items: List[str]
):
    """
    Get drop rate adjustment recommendations.
    
    Used by:
    - Problem Agent
    - Event Agent
    - Treasure Agent
    
    Returns:
        Recommendations for each system
    """
    try:
        recommendations = await merchant_economy_agent.recommend_drop_rate_changes(
            oversupplied_items=oversupplied_items,
            undersupplied_items=undersupplied_items
        )
        
        return DropRateRecommendationsResponse(**recommendations)
        
    except Exception as e:
        logger.error(f"API error generating recommendations: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# KARMA AGENT ENDPOINTS
# ============================================================================

@router.get("/karma/global", response_model=GlobalKarmaResponse)
async def get_global_karma():
    """
    Get current global karma and alignment.
    
    Returns:
        Global karma state with effects
    """
    try:
        karma_state = await karma_agent.get_global_karma_state()
        
        return GlobalKarmaResponse(**karma_state)
        
    except Exception as e:
        logger.error(f"API error getting global karma: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/karma/update", response_model=KarmaUpdateResponse)
async def update_karma(request: KarmaUpdateRequest):
    """
    Update karma from batched player actions.
    
    Called by:
    - Background task every 15 minutes
    - Bridge Layer on significant actions
    
    Returns:
        Karma update results
    """
    try:
        logger.info(f"API: Updating karma from {len(request.actions)} actions")
        
        # Convert to dict format
        actions_data = [action.model_dump() for action in request.actions]
        
        response = await karma_agent.update_global_karma(
            player_actions=actions_data
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to update karma"
            )
        
        return KarmaUpdateResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error updating karma: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/karma/{player_id}", response_model=PlayerKarmaResponse)
async def get_player_karma(player_id: int):
    """
    Get individual player karma.
    
    Args:
        player_id: Player ID
    
    Returns:
        Player karma and alignment
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT *
            FROM player_karma
            WHERE player_id = $1
        """
        
        result = await postgres_db.fetch_one(query, player_id)
        
        if result:
            return PlayerKarmaResponse(**dict(result))
        
        # Return default karma if player not found
        return PlayerKarmaResponse(
            player_id=player_id,
            karma_score=0,
            alignment=KarmaAlignment.NEUTRAL,
            good_actions=0,
            evil_actions=0
        )
        
    except Exception as e:
        logger.error(f"API error getting player karma: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/karma/effects", response_model=Dict[str, Any])
async def get_karma_effects():
    """
    Get current karma-based world effects.
    
    Returns:
        Active effects from karma alignment
    """
    try:
        from database import db
        
        # Try cache first
        effects = await db.get("karma:effects:current")
        
        if effects:
            return effects
        
        # Get from karma state
        karma_state = await karma_agent.get_global_karma_state()
        effects = karma_state.get('effects', {})
        
        return effects
        
    except Exception as e:
        logger.error(f"API error getting karma effects: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/karma/history", response_model=KarmaHistoryResponse)
async def get_karma_history(days: int = 7):
    """
    Get karma shift history.
    
    Args:
        days: Number of days of history (default: 7)
    
    Returns:
        Karma history over time
    """
    try:
        from database import postgres_db
        from datetime import datetime, timedelta, UTC
        
        period_start = datetime.now(UTC) - timedelta(days=days)
        period_end = datetime.now(UTC)
        
        query = """
            SELECT player_id, action_type, karma_value, global_impact, timestamp
            FROM karma_actions
            WHERE timestamp > $1
            ORDER BY timestamp DESC
            LIMIT 1000
        """
        
        actions = await postgres_db.fetch_all(query, period_start)
        
        history = [dict(action) for action in actions]
        
        return KarmaHistoryResponse(
            history=history,
            period_start=period_start,
            period_end=period_end,
            total_shifts=len(history)
        )
        
    except Exception as e:
        logger.error(f"API error getting karma history: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# SOCIAL INTERACTION AGENT ENDPOINTS
# ============================================================================

@router.post("/social/events/spawn", response_model=SocialEventsListResponse)
async def spawn_social_events(request: SocialEventSpawnRequest):
    """
    Spawn daily social events.
    
    Called by:
    - Daily cron job at 7 AM
    - Manual trigger by admin
    
    Returns:
        List of spawned social events
    """
    try:
        logger.info(f"API: Spawning {request.count} social events")
        
        responses = await social_interaction_agent.spawn_daily_social_events(
            player_distribution=request.player_distribution.map_distribution,
            count=request.count
        )
        
        events = [
            SocialEventResponse(**resp.data)
            for resp in responses
            if resp.success
        ]
        
        return SocialEventsListResponse(
            events=events,
            total=len(events),
            active_count=len(events)
        )
        
    except Exception as e:
        logger.error(f"API error spawning social events: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/social/events/active", response_model=SocialEventsListResponse)
async def get_active_social_events():
    """
    List all currently active social events.
    
    Returns:
        List of active social events
    """
    try:
        active_events = await social_interaction_agent.get_active_social_events()
        
        events = [SocialEventResponse(**event) for event in active_events]
        
        return SocialEventsListResponse(
            events=events,
            total=len(events),
            active_count=len(events)
        )
        
    except Exception as e:
        logger.error(f"API error getting active events: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/social/events/{event_id}/participate", response_model=SocialParticipationResponse)
async def record_social_participation(
    event_id: int,
    request: SocialParticipationRequest
):
    """
    Record social event participation.
    
    Called when:
    - Players participate in social event
    - Event completion/rewards
    
    Returns:
        Participation confirmation
    """
    try:
        # Get event type
        from database import postgres_db
        
        query = """
            SELECT event_type
            FROM social_events
            WHERE event_id = $1 AND status = 'active'
        """
        
        result = await postgres_db.fetch_one(query, event_id)
        
        if not result:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Social event #{event_id} not found or inactive"
            )
        
        event_type = SocialEventType(result['event_type'])
        
        response = await social_interaction_agent.handle_social_participation(
            event_id=event_id,
            participants=request.participants,
            event_type=event_type,
            party_id=request.party_id
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=response.reasoning or "Participation requirements not met"
            )
        
        return SocialParticipationResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording participation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/social/guild-tasks/generate", response_model=GuildTasksListResponse)
async def generate_guild_tasks(request: GuildTaskGenerateRequest):
    """
    Generate daily guild tasks.
    
    Called by:
    - Daily cron job at 4 AM
    - Manual trigger by admin
    
    Returns:
        List of generated guild tasks
    """
    try:
        logger.info(f"API: Generating guild tasks for {request.guild_count} guilds")
        
        tasks = await social_interaction_agent.generate_guild_tasks(
            guild_count=request.guild_count,
            average_guild_size=request.average_guild_size
        )
        
        task_responses = [GuildTaskResponse(**task) for task in tasks]
        
        return GuildTasksListResponse(
            tasks=task_responses,
            total=len(task_responses),
            active_count=len(task_responses)
        )
        
    except Exception as e:
        logger.error(f"API error generating guild tasks: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/social/guild-tasks/{guild_id}", response_model=GuildTasksListResponse)
async def get_guild_tasks(guild_id: int):
    """
    Get active tasks for a specific guild.
    
    Args:
        guild_id: Guild ID
    
    Returns:
        List of guild's active tasks
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT *
            FROM guild_tasks
            WHERE guild_id = $1 AND status = 'active' AND expires_at > NOW()
            ORDER BY created_at DESC
        """
        
        tasks = await postgres_db.fetch_all(query, guild_id)
        
        task_list = []
        for task in tasks:
            task_dict = dict(task)
            
            # Parse JSON fields
            for field in ['objectives', 'rewards']:
                if task_dict.get(field):
                    task_dict[field] = json.loads(task_dict[field])
            
            task_list.append(GuildTaskResponse(**task_dict))
        
        return GuildTasksListResponse(
            tasks=task_list,
            total=len(task_list),
            active_count=len(task_list)
        )
        
    except Exception as e:
        logger.error(f"API error getting guild tasks: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/social/guild-tasks/{task_id}/progress", response_model=EconomySocialAgentResponse)
async def update_task_progress(task_id: int, request: GuildTaskProgressRequest):
    """
    Update guild task progress.
    
    Called when:
    - Guild members make progress
    - Task objectives updated
    
    Returns:
        Success confirmation
    """
    try:
        from database import postgres_db
        
        # Update progress
        query = """
            UPDATE guild_tasks
            SET progress = $1,
                status = CASE
                    WHEN $1 >= completion_threshold THEN 'completed'
                    ELSE status
                END,
                completed_at = CASE
                    WHEN $1 >= completion_threshold THEN NOW()
                    ELSE completed_at
                END
            WHERE task_id = $2 AND status = 'active'
        """
        
        rows_affected = await postgres_db.execute(query, request.progress, task_id)
        
        if rows_affected > 0:
            return EconomySocialAgentResponse(
                success=True,
                message=f"Guild task #{task_id} progress updated to {request.progress}"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Guild task #{task_id} not found or already completed"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error updating task progress: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/social/metrics/analyze", response_model=SocialHealthResponse)
async def analyze_social_health(metrics: SocialMetricsModel):
    """
    Analyze community social health.
    
    Called by:
    - Background monitoring
    - Admin dashboard
    
    Returns:
        Social health analysis with recommendations
    """
    try:
        analysis = await social_interaction_agent.analyze_social_health(
            metrics=metrics.model_dump()
        )
        
        return SocialHealthResponse(**analysis)
        
    except Exception as e:
        logger.error(f"API error analyzing social health: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# STATUS & METRICS ENDPOINTS
# ============================================================================

@router.get("/status", response_model=Dict[str, Any])
async def get_economy_social_status():
    """
    Get overall economy & social systems status.
    
    Returns:
        Current state of all economy/social systems
    """
    try:
        from datetime import datetime, UTC
        
        # Get current states
        economy_snapshot = await merchant_economy_agent.get_current_economy_snapshot()
        karma_state = await karma_agent.get_global_karma_state()
        active_social_events = await social_interaction_agent.get_active_social_events()
        
        return {
            "economy": {
                "last_analysis": economy_snapshot.get('timestamp').isoformat() if economy_snapshot and economy_snapshot.get('timestamp') else None,
                "indicator": economy_snapshot.get('economic_indicator') if economy_snapshot else 'unknown',
                "zeny_per_capita": economy_snapshot.get('zeny_per_capita', 0) if economy_snapshot else 0
            },
            "karma": {
                "global_score": karma_state.get('karma_score', 0),
                "alignment": karma_state.get('alignment', 'neutral'),
                "last_shift": karma_state.get('last_shift')
            },
            "social": {
                "active_events": len(active_social_events),
                "events": [
                    {
                        "event_id": event['event_id'],
                        "type": event['event_type'],
                        "name": event['event_name'],
                        "map": event['spawn_map']
                    }
                    for event in active_social_events[:5]
                ]
            },
            "timestamp": datetime.now(UTC).isoformat()
        }
        
    except Exception as e:
        logger.error(f"API error getting status: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/metrics", response_model=Dict[str, Any])
async def get_economy_social_metrics():
    """
    Get economy & social systems metrics.
    
    Returns:
        Statistics about economic and social health
    """
    try:
        from database import postgres_db
        
        stats = {}
        
        # Economy stats (last 7 days)
        economy_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as snapshots_count,
                AVG(zeny_per_capita) as avg_zeny_per_capita,
                AVG(inflation_rate) as avg_inflation_rate
            FROM economy_snapshots
            WHERE timestamp > NOW() - INTERVAL '7 days'
        """)
        stats['economy'] = dict(economy_stats) if economy_stats else {}
        
        # Karma stats
        karma_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total_actions,
                COUNT(*) FILTER (WHERE karma_value > 0) as good_actions,
                COUNT(*) FILTER (WHERE karma_value < 0) as evil_actions
            FROM karma_actions
            WHERE timestamp > NOW() - INTERVAL '7 days'
        """)
        stats['karma'] = dict(karma_stats) if karma_stats else {}
        
        # Social stats
        social_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(DISTINCT event_id) as events_spawned,
                AVG(participation_count) as avg_participation
            FROM social_events
            WHERE spawned_at > NOW() - INTERVAL '7 days'
        """)
        stats['social'] = dict(social_stats) if social_stats else {}
        
        # Guild task stats
        guild_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total_tasks,
                COUNT(*) FILTER (WHERE status = 'completed') as completed_tasks,
                AVG(progress) as avg_progress
            FROM guild_tasks
            WHERE created_at > NOW() - INTERVAL '7 days'
        """)
        stats['guild_tasks'] = dict(guild_stats) if guild_stats else {}
        
        return stats
        
    except Exception as e:
        logger.error(f"API error getting metrics: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )