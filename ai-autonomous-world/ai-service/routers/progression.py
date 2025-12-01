"""
Progression Systems API Router
Exposes endpoints for Faction Agent, Reputation Agent, and Dynamic Boss Agent
"""

from typing import List, Optional
from fastapi import APIRouter, HTTPException, status, Depends
from loguru import logger

from agents.progression.faction_agent import faction_agent
from agents.progression.reputation_agent import reputation_agent
from agents.progression.dynamic_boss_agent import dynamic_boss_agent

from models.progression import (
    # Faction models
    FactionType,
    FactionActionRequest,
    FactionUpdateRequest,
    FactionAlignmentResponse,
    PlayerFactionReputationResponse,
    FactionRewardsResponse,
    PlayerReputationsResponse,
    FactionsListResponse,
    
    # Reputation models
    ReputationType,
    ReputationGainRequest,
    PlayerInfluenceResponse,
    UnlockedBenefitsResponse,
    TierProgressionResponse,
    ReputationLeaderboardEntry,
    ReputationLeaderboardResponse,
    
    # Boss models
    BossSpawnReason,
    BossStatus,
    BossSpawnRequest,
    BossEncounterRequest,
    BossDefeatRequest,
    DynamicBossResponse,
    BossHistoryEntry,
    BossHistoryResponse,
    ActiveBossesResponse,
    WorldStateForBossModel,
    
    # Generic
    ProgressionAgentResponse
)


router = APIRouter(
    prefix="/api/v1/progression",
    tags=["progression"]
)


# ============================================================================
# FACTION AGENT ENDPOINTS
# ============================================================================

@router.get("/factions/alignment", response_model=FactionAlignmentResponse)
async def get_world_alignment():
    """
    Get current global faction alignment.
    
    Called by:
    - Client UI to display faction status
    - Admin dashboard
    - Other agents for decision making
    
    Returns:
        Current faction alignment state
    """
    try:
        alignment_data = await faction_agent.get_world_alignment()
        
        return FactionAlignmentResponse(
            factions=alignment_data.get('factions', []),
            dominant_faction=alignment_data.get('dominant_faction')
        )
        
    except Exception as e:
        logger.error(f"API error getting world alignment: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/factions/update", response_model=ProgressionAgentResponse)
async def update_faction_alignment(request: FactionUpdateRequest):
    """
    Update faction alignment from player actions.
    
    Called by:
    - Background task processing player actions
    - Manual trigger by admin
    
    Returns:
        Updated alignment state
    """
    try:
        logger.info(f"API: Updating faction alignment from {len(request.actions)} actions")
        
        # Convert actions to dict format
        actions_list = [action.model_dump() for action in request.actions]
        
        response = await faction_agent.update_world_alignment(actions_list)
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to update alignment"
            )
        
        return ProgressionAgentResponse(
            success=True,
            message="Faction alignment updated",
            data=response.data
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error updating faction alignment: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/factions/{player_id}/reputation", response_model=PlayerReputationsResponse)
async def get_player_reputation(player_id: int):
    """
    Get player's reputation with all factions.
    
    Returns:
        Player's reputation scores with all factions
    """
    try:
        reputations = []
        
        for faction_type in FactionType:
            reputation_score = await faction_agent.calculate_player_reputation(
                player_id, faction_type.value
            )
            
            # Calculate tier
            tier = faction_agent._calculate_reputation_tier(reputation_score)
            tier_name = faction_agent.reputation_tiers[tier]['name']
            
            reputations.append(PlayerFactionReputationResponse(
                player_id=player_id,
                faction_id=faction_type,
                reputation=reputation_score,
                reputation_tier=tier,
                tier_name=tier_name,
                actions_completed=0  # Would query from database
            ))
        
        return PlayerReputationsResponse(
            player_id=player_id,
            reputations=reputations
        )
        
    except Exception as e:
        logger.error(f"API error getting player reputation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/factions/{player_id}/action", response_model=ProgressionAgentResponse)
async def record_faction_action(
    player_id: int,
    request: FactionActionRequest
):
    """
    Record a faction-aligned action.
    
    Called when:
    - Player completes faction quest
    - Player kills faction-aligned monster
    - Player participates in faction event
    
    Returns:
        Success confirmation
    """
    try:
        success = await faction_agent.record_faction_action(
            player_id=player_id,
            faction_id=request.faction_id.value,
            action_type=request.action_type,
            action_data=request.action_data
        )
        
        if success:
            return ProgressionAgentResponse(
                success=True,
                message="Faction action recorded"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail="Failed to record faction action"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording faction action: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/factions/{player_id}/rewards", response_model=FactionRewardsResponse)
async def get_available_rewards(player_id: int, faction_id: FactionType):
    """
    Get rewards player can claim from faction.
    
    Returns:
        Available rewards at player's current tier
    """
    try:
        # Get player reputation
        reputation = await faction_agent.calculate_player_reputation(
            player_id, faction_id.value
        )
        
        # Calculate tier
        tier = faction_agent._calculate_reputation_tier(reputation)
        
        # Get rewards
        rewards = await faction_agent.get_faction_rewards(
            player_id, faction_id.value, tier
        )
        
        return FactionRewardsResponse(
            tier=tier,
            tier_name=faction_agent.reputation_tiers[tier]['name'],
            **rewards
        )
        
    except Exception as e:
        logger.error(f"API error getting faction rewards: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# REPUTATION AGENT ENDPOINTS
# ============================================================================

@router.get("/reputation/{player_id}", response_model=PlayerInfluenceResponse)
async def get_player_influence(player_id: int):
    """
    Get player's complete reputation profile.
    
    Returns:
        Total influence, tier, and breakdown by source
    """
    try:
        influence_data = await reputation_agent.calculate_total_influence(player_id)
        
        return PlayerInfluenceResponse(
            player_id=player_id,
            **influence_data
        )
        
    except Exception as e:
        logger.error(f"API error getting player influence: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/reputation/{player_id}/gain", response_model=ProgressionAgentResponse)
async def record_reputation_gain(
    player_id: int,
    request: ReputationGainRequest
):
    """
    Record reputation gain from any source.
    
    Called when:
    - Player solves world problem
    - Player finds dynamic NPC
    - Player participates in event
    - Player advances faction standing
    - Player defeats dynamic boss
    
    Returns:
        Updated reputation and tier info
    """
    try:
        response = await reputation_agent.record_reputation_gain(
            player_id=player_id,
            reputation_type=request.reputation_type.value,
            amount=request.amount,
            source=request.source
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=response.reasoning or "Failed to record reputation"
            )
        
        return ProgressionAgentResponse(
            success=True,
            message="Reputation gain recorded",
            data=response.data
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording reputation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/reputation/{player_id}/benefits", response_model=UnlockedBenefitsResponse)
async def get_unlocked_benefits(player_id: int):
    """
    Get all benefits available to player.
    
    Returns:
        Unlocked shops, titles, items, services
    """
    try:
        benefits = await reputation_agent.get_unlocked_benefits(player_id)
        
        return UnlockedBenefitsResponse(**benefits)
        
    except Exception as e:
        logger.error(f"API error getting benefits: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/reputation/leaderboard", response_model=ReputationLeaderboardResponse)
async def get_reputation_leaderboard(limit: int = 100):
    """
    Get top players by total influence.
    
    Returns:
        Leaderboard of top players
    """
    try:
        if limit < 1 or limit > 1000:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Limit must be between 1 and 1000"
            )
        
        leaderboard = await reputation_agent.get_reputation_leaderboard(limit)
        
        entries = [
            ReputationLeaderboardEntry(**entry)
            for entry in leaderboard
        ]
        
        return ReputationLeaderboardResponse(
            leaderboard=entries,
            total_players=len(entries)
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error getting leaderboard: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# DYNAMIC BOSS AGENT ENDPOINTS
# ============================================================================

@router.post("/boss/evaluate-spawn", response_model=Optional[ProgressionAgentResponse])
async def evaluate_boss_spawn(world_state: WorldStateForBossModel):
    """
    Check if conditions warrant boss spawn.
    
    Called by:
    - Background task every 15 minutes
    - Manual check by admin
    
    Returns:
        Spawn decision or None if conditions not met
    """
    try:
        spawn_decision = await dynamic_boss_agent.evaluate_spawn_conditions(
            world_state.model_dump()
        )
        
        if spawn_decision:
            return ProgressionAgentResponse(
                success=True,
                message="Boss spawn conditions met",
                data=spawn_decision
            )
        
        return None
        
    except Exception as e:
        logger.error(f"API error evaluating boss spawn: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/boss/spawn", response_model=DynamicBossResponse)
async def spawn_dynamic_boss(request: BossSpawnRequest):
    """
    Spawn a dynamic boss.
    
    Called by:
    - Automatic spawn system
    - Manual trigger by admin
    
    Returns:
        Spawned boss details
    """
    try:
        logger.info(f"API: Spawning {request.spawn_reason} boss in {request.spawn_map}")
        
        # Generate boss spec
        boss_spec = await dynamic_boss_agent.generate_boss_spec(
            spawn_reason=request.spawn_reason.value,
            spawn_map=request.spawn_map,
            difficulty_modifier=request.difficulty_modifier
        )
        
        # Spawn boss
        response = await dynamic_boss_agent.spawn_boss(boss_spec)
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail=response.reasoning or "Failed to spawn boss"
            )
        
        return DynamicBossResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error spawning boss: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/boss/active", response_model=ActiveBossesResponse)
async def get_active_bosses():
    """
    List all active dynamic bosses.
    
    Returns:
        List of currently active bosses
    """
    try:
        active_bosses = await dynamic_boss_agent.get_active_bosses()
        
        boss_responses = [
            DynamicBossResponse(
                boss_id=boss['boss_id'],
                boss_type=boss['boss_type'],
                boss_name=boss['boss_name'],
                spawn_reason=boss['spawn_reason'],
                spawn_map=boss['spawn_map'],
                spawn_x=boss['spawn_x'],
                spawn_y=boss['spawn_y'],
                base_stats=boss.get('base_stats', {}),
                scaled_stats=boss.get('scaled_stats', {}),
                difficulty_rating=boss['difficulty_rating'],
                reward_multiplier=boss['reward_multiplier'],
                status=boss['status'],
                spawned_at=boss['spawned_at'],
                defeated_at=boss.get('defeated_at')
            )
            for boss in active_bosses
        ]
        
        return ActiveBossesResponse(
            bosses=boss_responses,
            total=len(boss_responses),
            active_count=len(boss_responses)
        )
        
    except Exception as e:
        logger.error(f"API error getting active bosses: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/boss/{boss_id}/encounter", response_model=ProgressionAgentResponse)
async def record_boss_encounter(
    boss_id: int,
    request: BossEncounterRequest
):
    """
    Record player participation in boss fight.
    
    Called when:
    - Player attacks boss
    - Player dies to boss
    - Combat state updates
    
    Returns:
        Success confirmation
    """
    try:
        success = await dynamic_boss_agent.record_boss_encounter(
            boss_id=boss_id,
            player_id=request.player_id,
            damage_dealt=request.damage_dealt,
            deaths=request.deaths,
            time_participated=request.time_participated
        )
        
        if success:
            return ProgressionAgentResponse(
                success=True,
                message="Boss encounter recorded"
            )
        else:
            raise HTTPException(
                status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
                detail="Failed to record encounter"
            )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error recording boss encounter: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.post("/boss/{boss_id}/defeat", response_model=ProgressionAgentResponse)
async def handle_boss_defeat(
    boss_id: int,
    request: BossDefeatRequest
):
    """
    Process boss defeat and distribute rewards.
    
    Called when:
    - Boss HP reaches 0
    - Boss despawns due to timeout (failed)
    
    Returns:
        Reward distribution results
    """
    try:
        response = await dynamic_boss_agent.handle_boss_defeat(
            boss_id=boss_id,
            participants=request.participants,
            time_to_kill=request.time_to_kill
        )
        
        if not response.success:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=response.reasoning or "Boss not found"
            )
        
        return ProgressionAgentResponse(
            success=True,
            message="Boss defeated, rewards distributed",
            data=response.data
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error handling boss defeat: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/boss/history", response_model=BossHistoryResponse)
async def get_boss_history(
    map_name: Optional[str] = None,
    limit: int = 50
):
    """
    Get boss spawn history.
    
    Args:
        map_name: Optional filter by map
        limit: Maximum results (default: 50)
    
    Returns:
        Historical boss spawn data
    """
    try:
        from database import postgres_db
        
        if limit < 1 or limit > 500:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Limit must be between 1 and 500"
            )
        
        # Build query
        if map_name:
            query = """
                SELECT *
                FROM boss_spawn_history
                WHERE spawn_map = $1
                ORDER BY spawned_at DESC
                LIMIT $2
            """
            rows = await postgres_db.fetch_all(query, map_name, limit)
        else:
            query = """
                SELECT *
                FROM boss_spawn_history
                ORDER BY spawned_at DESC
                LIMIT $1
            """
            rows = await postgres_db.fetch_all(query, limit)
        
        history = [
            BossHistoryEntry(
                boss_type=row['boss_type'],
                spawn_reason=row['spawn_reason'],
                spawn_map=row['spawn_map'],
                difficulty_rating=row['difficulty_rating'],
                participants_count=row['participants_count'],
                time_to_defeat=row['time_to_defeat'],
                was_defeated=row['was_defeated'],
                spawned_at=row['spawned_at']
            )
            for row in rows
        ]
        
        # Calculate average time to defeat
        defeated_times = [h.time_to_defeat for h in history if h.was_defeated and h.time_to_defeat]
        avg_time = sum(defeated_times) / len(defeated_times) if defeated_times else None
        
        return BossHistoryResponse(
            history=history,
            total=len(history),
            avg_time_to_defeat=avg_time
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"API error getting boss history: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# ============================================================================
# ADMIN/DEBUG ENDPOINTS
# ============================================================================

@router.get("/status")
async def get_progression_status():
    """
    Get overall progression systems status.
    
    Returns:
        Current state of all progression systems
    """
    try:
        # Get faction alignment
        faction_data = await faction_agent.get_world_alignment()
        
        # Get active bosses
        active_bosses = await dynamic_boss_agent.get_active_bosses()
        
        return {
            "faction_alignment": {
                "dominant_faction": faction_data.get('dominant_faction'),
                "faction_count": len(faction_data.get('factions', []))
            },
            "reputation_system": {
                "enabled": True,
                "tier_count": 6,
                "reputation_types": 5
            },
            "dynamic_bosses": {
                "active_count": len(active_bosses),
                "bosses": [
                    {
                        "boss_id": boss['boss_id'],
                        "name": boss['boss_name'],
                        "type": boss['boss_type'],
                        "map": boss['spawn_map'],
                        "difficulty": boss['difficulty_rating']
                    }
                    for boss in active_bosses
                ]
            }
        }
        
    except Exception as e:
        logger.error(f"API error getting progression status: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/metrics")
async def get_progression_metrics():
    """
    Get progression systems metrics.
    
    Returns:
        Statistics about progression systems
    """
    try:
        from database import postgres_db
        
        stats = {}
        
        # Faction stats
        faction_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(DISTINCT player_id) as total_players,
                AVG(reputation) as avg_reputation
            FROM player_faction_reputation
        """)
        stats['factions'] = dict(faction_stats) if faction_stats else {}
        
        # Reputation stats
        reputation_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total_players,
                AVG(total_influence) as avg_influence,
                MAX(total_influence) as max_influence
            FROM player_influence
        """)
        stats['reputation'] = dict(reputation_stats) if reputation_stats else {}
        
        # Boss stats
        boss_stats = await postgres_db.fetch_one("""
            SELECT 
                COUNT(*) as total_spawned,
                COUNT(*) FILTER (WHERE was_defeated = TRUE) as defeated,
                AVG(time_to_defeat) as avg_defeat_time,
                AVG(participants_count) as avg_participants
            FROM boss_spawn_history
            WHERE spawned_at > NOW() - INTERVAL '7 days'
        """)
        stats['bosses'] = dict(boss_stats) if boss_stats else {}
        
        return stats
        
    except Exception as e:
        logger.error(f"API error getting metrics: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )