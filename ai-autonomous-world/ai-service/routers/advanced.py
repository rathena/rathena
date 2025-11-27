"""
Advanced Systems API Router
Provides endpoints for Adaptive Dungeon, Archaeology, and Event Chain agents
"""

from fastapi import APIRouter, HTTPException, Depends, Query
from typing import List, Dict, Any, Optional
from loguru import logger

from agents.advanced.adaptive_dungeon_agent import adaptive_dungeon_agent
from agents.advanced.archaeology_agent import archaeology_agent
from agents.advanced.event_chain_agent import event_chain_agent
from models.advanced import (
    # Dungeon models
    DungeonGenerateRequest,
    DungeonSpecResponse,
    InstanceCreateRequest,
    InstanceCompleteRequest,
    DungeonRewardsResponse,
    DungeonLeaderboardResponse,
    # Archaeology models
    MapActivityModel,
    DigSpotSpawnRequest,
    DigSpotResponse,
    DigSpotActionRequest,
    ArtifactResponse,
    CollectionProgressResponse,
    SecretLocationUnlockRequest,
    SecretLocationResponse,
    ArchaeologyProgressResponse,
    # Event chain models
    ChainStartRequest,
    ChainStartResponse,
    ChainAdvanceRequest,
    ChainAdvanceResponse,
    ChainParticipationRequest,
    ChainCompletionResponse,
    ActiveChainsResponse,
    ChainHistoryResponse,
    # Generic
    AdvancedAgentResponse
)


router = APIRouter(prefix="/api/v1/advanced", tags=["advanced"])


# ============================================================================
# ADAPTIVE DUNGEON ENDPOINTS
# ============================================================================

@router.post("/dungeon/generate", response_model=DungeonSpecResponse)
async def generate_daily_dungeon(request: DungeonGenerateRequest):
    """
    Generate daily adaptive dungeon.
    
    - **player_average_level**: Average level of active players
    - **active_factions**: List of active faction IDs
    
    Returns complete dungeon specification with layouts, monster pools, and rewards.
    """
    try:
        response = await adaptive_dungeon_agent.generate_daily_dungeon(
            player_average_level=request.player_average_level,
            active_factions=request.active_factions
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error', 'Failed to generate dungeon'))
        
        return DungeonSpecResponse(**response.data)
        
    except Exception as e:
        logger.error(f"Failed to generate dungeon: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/dungeon/current", response_model=DungeonSpecResponse)
async def get_current_dungeon():
    """
    Get active dungeon specification.
    
    Returns the current daily dungeon data from cache or database.
    """
    try:
        from database import db
        
        # Try cache first
        cached = await db.get("dungeon:daily:current")
        if not cached:
            raise HTTPException(status_code=404, detail="No active dungeon found")
        
        # Get full dungeon data from database
        from database import postgres_db
        query = """
            SELECT dungeon_id, theme, floor_count, layout_data, monster_pools, 
                   boss_pool, reward_data, difficulty_rating
            FROM adaptive_dungeons
            WHERE dungeon_id = $1 AND status = 'active'
        """
        
        dungeon = await postgres_db.fetch_one(query, cached['dungeon_id'])
        
        if not dungeon:
            raise HTTPException(status_code=404, detail="Dungeon not found")
        
        import json
        return DungeonSpecResponse(
            dungeon_id=dungeon['dungeon_id'],
            theme=dungeon['theme'],
            floor_count=dungeon['floor_count'],
            difficulty_rating=dungeon['difficulty_rating'],
            layouts=json.loads(dungeon['layout_data']),
            monster_pools=json.loads(dungeon['monster_pools']),
            boss_pool=json.loads(dungeon['boss_pool']),
            rewards=json.loads(dungeon['reward_data'])
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get current dungeon: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/dungeon/instance/create", response_model=AdvancedAgentResponse)
async def create_instance(request: InstanceCreateRequest):
    """
    Create new dungeon instance for party.
    
    - **party_id**: Party ID
    - **participants**: List of player IDs (2-12 players)
    
    Returns instance ID for tracking.
    """
    try:
        from database import postgres_db, db
        import json
        
        # Get current dungeon
        cached = await db.get("dungeon:daily:current")
        if not cached:
            raise HTTPException(status_code=404, detail="No active dungeon")
        
        # Create instance
        query = """
            INSERT INTO dungeon_instances
            (dungeon_id, party_id, participants)
            VALUES ($1, $2, $3)
            RETURNING instance_id
        """
        
        result = await postgres_db.fetch_one(
            query,
            cached['dungeon_id'],
            request.party_id,
            json.dumps(request.participants)
        )
        
        return AdvancedAgentResponse(
            success=True,
            message="Instance created successfully",
            data={
                "instance_id": result['instance_id'],
                "dungeon_id": cached['dungeon_id'],
                "party_id": request.party_id
            }
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to create instance: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/dungeon/instance/{instance_id}/complete", response_model=DungeonRewardsResponse)
async def complete_instance(
    instance_id: int,
    request: InstanceCompleteRequest
):
    """
    Handle instance completion and distribute rewards.
    
    - **instance_id**: Instance ID
    - **participants**: List of player IDs
    - **floors_cleared**: Number of floors completed
    - **time_taken**: Time in seconds
    
    Returns rewards for all participants.
    """
    try:
        response = await adaptive_dungeon_agent.handle_instance_completion(
            instance_id=instance_id,
            participants=request.participants,
            floors_cleared=request.floors_cleared,
            time_taken=request.time_taken
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error', 'Failed to complete instance'))
        
        return DungeonRewardsResponse(**response.data['rewards'])
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to complete instance: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/dungeon/leaderboard", response_model=DungeonLeaderboardResponse)
async def get_dungeon_leaderboard(limit: int = Query(default=10, ge=1, le=100)):
    """
    Get fastest clears leaderboard.
    
    - **limit**: Number of entries to return (default 10)
    
    Returns top dungeon completion times.
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT 
                ROW_NUMBER() OVER (ORDER BY time_taken ASC) as rank,
                di.party_id,
                dc.floors_cleared,
                dc.time_taken,
                dc.timestamp
            FROM dungeon_completions dc
            JOIN dungeon_instances di ON di.instance_id = dc.instance_id
            WHERE dc.floors_cleared = (
                SELECT MAX(floor_count) FROM adaptive_dungeons WHERE status = 'active'
            )
            ORDER BY dc.time_taken ASC
            LIMIT $1
        """
        
        results = await postgres_db.fetch_all(query, limit)
        
        from models.advanced import DungeonLeaderboardEntry
        leaderboard = [
            DungeonLeaderboardEntry(
                rank=r['rank'],
                party_id=r['party_id'],
                floors_cleared=r['floors_cleared'],
                time_taken=r['time_taken'],
                timestamp=r['timestamp']
            )
            for r in results
        ]
        
        return DungeonLeaderboardResponse(
            leaderboard=leaderboard,
            total_completions=len(leaderboard)
        )
        
    except Exception as e:
        logger.error(f"Failed to get leaderboard: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# ARCHAEOLOGY ENDPOINTS
# ============================================================================

@router.post("/archaeology/dig-spots/spawn", response_model=List[DigSpotResponse])
async def spawn_dig_spots(request: DigSpotSpawnRequest):
    """
    Spawn daily dig spots.
    
    - **map_activity**: Dict of map_name -> visit_count
    - **count**: Number of spots to spawn (10-20)
    
    Returns list of spawned dig spot specifications.
    """
    try:
        responses = await archaeology_agent.spawn_dig_spots(
            map_activity=request.map_activity,
            count=request.count
        )
        
        spots = []
        for response in responses:
            if response.success:
                spots.append(DigSpotResponse(**response.data))
        
        return spots
        
    except Exception as e:
        logger.error(f"Failed to spawn dig spots: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/archaeology/dig-spots/active", response_model=List[Dict[str, Any]])
async def get_active_dig_spots():
    """
    List active dig spots (no exact locations).
    
    Returns active dig spots with vague location hints.
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT spot_id, spawn_map, rarity_tier, spawned_at, expires_at
            FROM dig_spots
            WHERE status = 'active' AND expires_at > NOW()
            ORDER BY spawned_at DESC
        """
        
        spots = await postgres_db.fetch_all(query)
        
        return [
            {
                "spot_id": s['spot_id'],
                "map": s['spawn_map'],
                "rarity": s['rarity_tier'],
                "spawned_at": s['spawned_at'].isoformat(),
                "expires_at": s['expires_at'].isoformat()
            }
            for s in spots
        ]
        
    except Exception as e:
        logger.error(f"Failed to get dig spots: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/archaeology/dig-spots/{spot_id}/dig", response_model=ArtifactResponse)
async def dig_spot(
    spot_id: int,
    request: DigSpotActionRequest
):
    """
    Player digs a spot, discovers artifact.
    
    - **spot_id**: Dig spot ID
    - **player_id**: Player ID
    - **archaeology_level**: Player's archaeology level
    
    Returns discovered artifact.
    """
    try:
        from database import postgres_db
        import json
        
        # Get spot details
        query = """
            SELECT rarity_tier, status
            FROM dig_spots
            WHERE spot_id = $1
        """
        spot = await postgres_db.fetch_one(query, spot_id)
        
        if not spot:
            raise HTTPException(status_code=404, detail="Dig spot not found")
        
        if spot['status'] != 'active':
            raise HTTPException(status_code=400, detail="Dig spot already dug")
        
        # Generate artifact
        artifact = await archaeology_agent.generate_artifact(
            dig_spot_rarity=spot['rarity_tier'],
            player_archaeology_level=request.archaeology_level
        )
        
        # Mark spot as dug
        query = """
            UPDATE dig_spots
            SET status = 'dug', dug_by = $1, artifact_found = $2, dug_at = NOW()
            WHERE spot_id = $3
        """
        await postgres_db.execute(query, request.player_id, artifact['artifact_id'], spot_id)
        
        # Add artifact to player collection
        query = """
            INSERT INTO player_artifacts
            (player_id, artifact_id, artifact_type, collection_name)
            VALUES ($1, $2, $3, $4)
        """
        await postgres_db.execute(
            query,
            request.player_id,
            artifact['artifact_id'],
            artifact['artifact_type'],
            artifact.get('collection')
        )
        
        # Update archaeology progress
        query = """
            INSERT INTO archaeology_progress
            (player_id, total_digs, artifacts_found)
            VALUES ($1, 1, 1)
            ON CONFLICT (player_id)
            DO UPDATE SET
                total_digs = archaeology_progress.total_digs + 1,
                artifacts_found = archaeology_progress.artifacts_found + 1,
                last_dig = NOW()
        """
        await postgres_db.execute(query, request.player_id)
        
        # Check collection completion
        completion_reward = await archaeology_agent.check_collection_completion(
            player_id=request.player_id,
            artifact_id=artifact['artifact_id']
        )
        
        if completion_reward:
            artifact['completion_reward'] = completion_reward
        
        return ArtifactResponse(**artifact)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to dig spot: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/archaeology/{player_id}/collection", response_model=List[Dict[str, Any]])
async def get_player_collection(player_id: int):
    """
    Get player's artifact collection.
    
    Returns all artifacts owned by player.
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT artifact_id, artifact_type, collection_name, acquired_at
            FROM player_artifacts
            WHERE player_id = $1
            ORDER BY acquired_at DESC
        """
        
        artifacts = await postgres_db.fetch_all(query, player_id)
        
        return [dict(a) for a in artifacts]
        
    except Exception as e:
        logger.error(f"Failed to get collection: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/archaeology/{player_id}/progress", response_model=ArchaeologyProgressResponse)
async def get_archaeology_progress(player_id: int):
    """
    Get player's archaeology level and stats.
    
    Returns archaeology progression data.
    """
    try:
        progress = await archaeology_agent.get_archaeology_progress(player_id)
        return ArchaeologyProgressResponse(**progress)
        
    except Exception as e:
        logger.error(f"Failed to get progress: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/archaeology/{player_id}/unlock-secret", response_model=SecretLocationResponse)
async def unlock_secret_location(
    player_id: int,
    request: SecretLocationUnlockRequest
):
    """
    Unlock secret location from treasure map.
    
    - **player_id**: Player ID
    - **treasure_map_id**: Treasure map artifact ID
    
    Returns secret location details with 30-minute access.
    """
    try:
        response = await archaeology_agent.unlock_secret_location(
            player_id=player_id,
            treasure_map_id=request.treasure_map_id
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error', 'Failed to unlock location'))
        
        return SecretLocationResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to unlock secret location: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# EVENT CHAIN ENDPOINTS
# ============================================================================

@router.post("/chains/start", response_model=ChainStartResponse)
async def start_event_chain(request: ChainStartRequest):
    """
    Start new event chain.
    
    - **chain_type**: Type of event chain
    - **trigger_condition**: Conditions that triggered this chain
    
    Returns first step specification.
    """
    try:
        response = await event_chain_agent.start_event_chain(
            chain_type=request.chain_type,
            trigger_condition=request.trigger_condition
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error', 'Failed to start chain'))
        
        return ChainStartResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to start chain: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/chains/active", response_model=ActiveChainsResponse)
async def get_active_chains():
    """
    List active event chains.
    
    Returns all currently active chains.
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT chain_id, chain_type, chain_name, total_steps, current_step, started_at
            FROM event_chains
            WHERE status = 'active' AND expires_at > NOW()
            ORDER BY started_at DESC
        """
        
        chains = await postgres_db.fetch_all(query)
        
        return ActiveChainsResponse(
            chains=[dict(c) for c in chains],
            total=len(chains)
        )
        
    except Exception as e:
        logger.error(f"Failed to get active chains: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/chains/{chain_id}", response_model=Dict[str, Any])
async def get_chain_details(chain_id: int):
    """
    Get complete chain specification.
    
    Returns full chain details including all steps.
    """
    try:
        from database import postgres_db
        import json
        
        # Get chain data
        query = """
            SELECT chain_id, chain_type, chain_name, total_steps, current_step, 
                   chain_data, status, started_at
            FROM event_chains
            WHERE chain_id = $1
        """
        chain = await postgres_db.fetch_one(query, chain_id)
        
        if not chain:
            raise HTTPException(status_code=404, detail="Chain not found")
        
        # Get steps
        query = """
            SELECT step_number, step_type, objective, dialogue, completed, outcome
            FROM chain_steps
            WHERE chain_id = $1
            ORDER BY step_number
        """
        steps = await postgres_db.fetch_all(query, chain_id)
        
        return {
            "chain_id": chain['chain_id'],
            "chain_type": chain['chain_type'],
            "chain_name": chain['chain_name'],
            "total_steps": chain['total_steps'],
            "current_step": chain['current_step'],
            "status": chain['status'],
            "started_at": chain['started_at'].isoformat(),
            "steps": [dict(s) for s in steps]
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get chain details: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/chains/{chain_id}/advance", response_model=ChainAdvanceResponse)
async def advance_chain(
    chain_id: int,
    request: ChainAdvanceRequest
):
    """
    Advance chain to next step based on outcome.
    
    - **chain_id**: Event chain ID
    - **outcome**: Step outcome (success/failure/partial)
    
    Returns next step specification.
    """
    try:
        response = await event_chain_agent.advance_chain_step(
            chain_id=chain_id,
            outcome=request.outcome.value
        )
        
        if not response.success:
            raise HTTPException(status_code=500, detail=response.data.get('error', 'Failed to advance chain'))
        
        return ChainAdvanceResponse(**response.data)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to advance chain: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/chains/{chain_id}/current-step", response_model=Dict[str, Any])
async def get_current_step(chain_id: int):
    """
    Get current active step.
    
    Returns current step details.
    """
    try:
        from database import postgres_db
        
        # Get current step number
        query = """
            SELECT current_step
            FROM event_chains
            WHERE chain_id = $1 AND status = 'active'
        """
        chain = await postgres_db.fetch_one(query, chain_id)
        
        if not chain:
            raise HTTPException(status_code=404, detail="Chain not found or not active")
        
        # Get step details
        query = """
            SELECT step_number, step_type, objective, dialogue, success_condition, 
                   npc_data, spawn_data
            FROM chain_steps
            WHERE chain_id = $1 AND step_number = $2
        """
        step = await postgres_db.fetch_one(query, chain_id, chain['current_step'])
        
        if not step:
            raise HTTPException(status_code=404, detail="Step not found")
        
        return dict(step)
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get current step: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/chains/{chain_id}/participate", response_model=AdvancedAgentResponse)
async def record_chain_participation(
    chain_id: int,
    request: ChainParticipationRequest
):
    """
    Record player participation in chain.
    
    - **chain_id**: Event chain ID
    - **player_id**: Player ID
    - **contribution**: Contribution score
    
    Returns success status.
    """
    try:
        from database import postgres_db
        
        query = """
            INSERT INTO chain_participation
            (chain_id, player_id, contribution_score)
            VALUES ($1, $2, $3)
            ON CONFLICT (chain_id, player_id)
            DO UPDATE SET
                contribution_score = chain_participation.contribution_score + $3
        """
        
        await postgres_db.execute(query, chain_id, request.player_id, request.contribution)
        
        return AdvancedAgentResponse(
            success=True,
            message="Participation recorded",
            data={"chain_id": chain_id, "player_id": request.player_id}
        )
        
    except Exception as e:
        logger.error(f"Failed to record participation: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/chains/history", response_model=ChainHistoryResponse)
async def get_chain_history(limit: int = Query(default=20, ge=1, le=100)):
    """
    Get completed chains history.
    
    - **limit**: Number of entries to return (default 20)
    
    Returns recent completed chains.
    """
    try:
        from database import postgres_db
        
        query = """
            SELECT ec.chain_id, ec.chain_type, ec.chain_name, co.final_outcome, 
                   co.success_rate, co.participant_count, ec.started_at, ec.completed_at
            FROM event_chains ec
            JOIN chain_outcomes co ON co.chain_id = ec.chain_id
            WHERE ec.status = 'completed'
            ORDER BY ec.completed_at DESC
            LIMIT $1
        """
        
        results = await postgres_db.fetch_all(query, limit)
        
        from models.advanced import ChainHistoryEntry
        history = [
            ChainHistoryEntry(
                chain_id=r['chain_id'],
                chain_type=r['chain_type'],
                chain_name=r['chain_name'],
                final_outcome=r['final_outcome'],
                success_rate=r['success_rate'],
                participant_count=r['participant_count'],
                started_at=r['started_at'],
                completed_at=r['completed_at']
            )
            for r in results
        ]
        
        return ChainHistoryResponse(
            history=history,
            total=len(history)
        )
        
    except Exception as e:
        logger.error(f"Failed to get chain history: {e}")
        raise HTTPException(status_code=500, detail=str(e))