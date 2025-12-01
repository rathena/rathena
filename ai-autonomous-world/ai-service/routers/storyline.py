"""
Storyline API Router
FastAPI endpoints for AI-Driven Storyline Generator system
"""

from fastapi import APIRouter, HTTPException, BackgroundTasks, Query
from typing import Optional, List
from datetime import datetime
from loguru import logger

from models.storyline import (
    GenerateStoryRequest, StoryArcResponse, ChapterResponse,
    ChapterAdvanceRequest, PlayerChoiceRequest, HeroSelectionResponse,
    StoryParticipationStats, StoryHistoryEntry, StoryDashboardResponse,
    WorldStateSnapshot, AdminStoryOverride, StorySystemMetrics,
    ChapterOutcome, StoryArcStatus
)
from storyline.world_state_collector import world_state_collector
from storyline.storyline_generator import storyline_generator
from storyline.story_arc_manager import story_arc_manager
from storyline.npc_story_integration import npc_story_integration
from storyline.story_quest_bridge import story_quest_bridge
from database import postgres_db, db


router = APIRouter(prefix="/api/v1/storyline", tags=["storyline"])


# ============================================================================
# STORY ARC MANAGEMENT ENDPOINTS
# ============================================================================

@router.post("/generate", response_model=StoryArcResponse)
async def generate_story_arc(
    request: GenerateStoryRequest,
    background_tasks: BackgroundTasks
):
    """
    Generate new story arc based on current world state.
    
    Process:
    1. Collect world state (or use provided)
    2. Call LLM for story generation
    3. Validate output
    4. Store in database
    5. Dispatch NPCs/events via Bridge Layer
    6. Broadcast announcement
    
    Args:
        request: Generation parameters
        background_tasks: FastAPI background tasks
        
    Returns:
        Created story arc details
    """
    try:
        logger.info("Generating new story arc via API")
        
        # Check if active arc exists
        current_arc = await story_arc_manager.get_current_arc()
        if current_arc and current_arc.status == StoryArcStatus.ACTIVE:
            raise HTTPException(
                status_code=400,
                detail=f"Active arc already exists: {current_arc.arc_name} (#{current_arc.arc_id})"
            )
        
        # Collect world state if not provided
        if request.world_state:
            world_state = request.world_state
        else:
            world_state = await world_state_collector.collect_complete_world_state()
        
        # Get previous arcs for context
        previous_arcs_query = """
            SELECT arc_id, arc_name, arc_summary, theme, outcome_summary
            FROM story_arc_history
            ORDER BY completed_at DESC
            LIMIT 5
        """
        
        previous_arc_rows = await postgres_db.fetch_all(previous_arcs_query)
        previous_arcs = [dict(row) for row in previous_arc_rows]
        
        # Generate story arc with LLM
        arc_spec = await storyline_generator.generate_story_arc(
            world_state=world_state,
            previous_arcs=previous_arcs,
            force_theme=request.force_theme
        )
        
        if not arc_spec:
            raise HTTPException(
                status_code=500,
                detail="Story arc generation failed"
            )
        
        # Start the arc
        arc_response = await story_arc_manager.start_new_arc(
            arc_spec=arc_spec,
            created_by='api_request'
        )
        
        # Background: Spawn NPCs and create quests
        background_tasks.add_task(
            deploy_story_content,
            arc_id=arc_response.arc_id,
            arc_spec=arc_spec
        )
        
        logger.info(f"✓ Story arc #{arc_response.arc_id} generation initiated")
        
        return arc_response
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Story generation API error: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/current", response_model=Optional[StoryArcResponse])
async def get_current_arc():
    """
    Get currently active story arc and chapter.
    
    Returns:
        Current story arc details or None
    """
    try:
        current_arc = await story_arc_manager.get_current_arc()
        return current_arc
    except Exception as e:
        logger.error(f"Failed to get current arc: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/arc/{arc_id}", response_model=StoryArcResponse)
async def get_arc_details(arc_id: int):
    """
    Get complete arc specification.
    
    Args:
        arc_id: Story arc ID
        
    Returns:
        Complete arc details
    """
    try:
        # Query database
        query = """
            SELECT arc_id, arc_name, arc_summary, total_chapters, current_chapter,
                   theme, dominant_faction, status, started_at, expected_end_at, completed_at
            FROM story_arcs
            WHERE arc_id = $1
        """
        
        result = await postgres_db.fetch_one(query, arc_id)
        
        if not result:
            raise HTTPException(status_code=404, detail=f"Arc #{arc_id} not found")
        
        # Count NPCs and quests
        npcs_query = "SELECT COUNT(*) as count FROM story_npcs WHERE arc_id = $1"
        npcs_result = await postgres_db.fetch_one(npcs_query, arc_id)
        npcs_spawned = npcs_result['count'] if npcs_result else 0
        
        quests_query = "SELECT COUNT(*) as count FROM story_quests WHERE arc_id = $1"
        quests_result = await postgres_db.fetch_one(quests_query, arc_id)
        quests_created = quests_result['count'] if quests_result else 0
        
        response = StoryArcResponse(
            arc_id=result['arc_id'],
            arc_name=result['arc_name'],
            arc_summary=result['arc_summary'],
            total_chapters=result['total_chapters'],
            current_chapter=result['current_chapter'],
            theme=result['theme'],
            dominant_faction=result['dominant_faction'],
            status=StoryArcStatus(result['status']),
            started_at=result['started_at'],
            expected_end_at=result['expected_end_at'],
            npcs_spawned=npcs_spawned,
            quests_created=quests_created
        )
        
        return response
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get arc details: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/arc/{arc_id}/advance", response_model=ChapterResponse)
async def advance_chapter(
    arc_id: int,
    request: ChapterAdvanceRequest
):
    """
    Advance to next chapter based on player outcome.
    
    Args:
        arc_id: Story arc ID
        request: Chapter advancement details
        
    Returns:
        Next chapter details
    """
    try:
        logger.info(f"Advancing arc #{arc_id} chapter with outcome: {request.outcome.value}")
        
        chapter_response = await story_arc_manager.advance_chapter(
            arc_id=arc_id,
            outcome=request.outcome,
            player_choices=request.player_choices,
            completion_rate=request.completion_rate
        )
        
        return chapter_response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        logger.error(f"Failed to advance chapter: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/arc/{arc_id}/player-choice")
async def record_player_choice(
    arc_id: int,
    request: PlayerChoiceRequest
):
    """
    Record player choice in story.
    
    Args:
        arc_id: Story arc ID
        request: Player choice details
        
    Returns:
        Success status
    """
    try:
        # Get current chapter
        arc = await story_arc_manager.get_current_arc()
        if not arc or arc.arc_id != arc_id:
            raise HTTPException(status_code=404, detail=f"Arc #{arc_id} not active")
        
        success = await story_arc_manager.record_player_choice(
            arc_id=arc_id,
            player_id=request.player_id,
            chapter=arc.current_chapter,
            choice_request=request
        )
        
        if not success:
            raise HTTPException(status_code=500, detail="Failed to record choice")
        
        return {
            "success": True,
            "message": "Player choice recorded",
            "arc_id": arc_id,
            "chapter": arc.current_chapter,
            "player_id": request.player_id
        }
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to record player choice: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/arc/{arc_id}/participants", response_model=List[StoryParticipationStats])
async def get_arc_participants(
    arc_id: int,
    limit: int = Query(default=100, ge=1, le=500)
):
    """
    Get all players participating in arc.
    
    Args:
        arc_id: Story arc ID
        limit: Maximum participants to return
        
    Returns:
        List of participation statistics
    """
    try:
        participants = await story_arc_manager.get_arc_participants(
            arc_id=arc_id,
            limit=limit
        )
        
        return participants
        
    except Exception as e:
        logger.error(f"Failed to get arc participants: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/arc/{arc_id}/heroes", response_model=HeroSelectionResponse)
async def get_heroes_of_arc(arc_id: int):
    """
    Get Hero of the Week participants.
    
    Args:
        arc_id: Story arc ID
        
    Returns:
        Hero recognition details
    """
    try:
        # Get top participants
        participants = await story_arc_manager.get_arc_participants(arc_id=arc_id, limit=10)
        
        if not participants:
            return HeroSelectionResponse(
                arc_id=arc_id,
                heroes=[],
                selection_criteria="No participants yet",
                rewards={}
            )
        
        # Convert to dicts for hero generation
        top_participants = [
            {
                'player_id': p.player_id,
                'contribution_score': p.contribution_score,
                'chapters_completed': p.chapters_completed,
                'quests_completed': p.quests_completed,
                'notable_choices': []  # TODO: Extract from choices_made
            }
            for p in participants[:3]
        ]
        
        # Generate hero recognition
        hero_response = await npc_story_integration.generate_hero_recognition(
            arc_id=arc_id,
            top_participants=top_participants
        )
        
        return hero_response
        
    except Exception as e:
        logger.error(f"Failed to get heroes: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/history", response_model=List[StoryHistoryEntry])
async def get_story_history(
    limit: int = Query(default=10, ge=1, le=50)
):
    """
    Get archived story arcs.
    
    Args:
        limit: Maximum arcs to return
        
    Returns:
        List of completed story arcs
    """
    try:
        query = """
            SELECT arc_id, arc_name, arc_summary, duration_days,
                   total_participants, success_rate, heroes, world_changes, completed_at
            FROM story_arc_history
            ORDER BY completed_at DESC
            LIMIT $1
        """
        
        rows = await postgres_db.fetch_all(query, limit)
        
        history = [
            StoryHistoryEntry(
                arc_id=row['arc_id'],
                arc_name=row['arc_name'],
                arc_summary=row['arc_summary'],
                duration_days=row['duration_days'],
                total_participants=row['total_participants'],
                success_rate=row['success_rate'],
                heroes=row['heroes'] if isinstance(row['heroes'], list) else [],
                world_changes=row['world_changes'] if isinstance(row['world_changes'], list) else [],
                completed_at=row['completed_at']
            )
            for row in rows
        ]
        
        return history
        
    except Exception as e:
        logger.error(f"Failed to get story history: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/world-state", response_model=WorldStateSnapshot)
async def get_world_state_snapshot(
    force_fresh: bool = Query(default=False, description="Force fresh collection")
):
    """
    Get current world state (for debugging/admin).
    
    Args:
        force_fresh: Skip cache and collect fresh data
        
    Returns:
        Complete world state snapshot
    """
    try:
        world_state = await world_state_collector.collect_complete_world_state(
            force_fresh=force_fresh
        )
        
        return world_state
        
    except Exception as e:
        logger.error(f"Failed to get world state: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# DASHBOARD & METRICS ENDPOINTS
# ============================================================================

@router.get("/dashboard", response_model=StoryDashboardResponse)
async def get_story_dashboard():
    """
    Get complete story system dashboard.
    
    Returns:
        Comprehensive dashboard data
    """
    try:
        # Get current arc
        current_arc = await story_arc_manager.get_current_arc()
        
        # Get active chapters
        active_chapters = []
        if current_arc:
            chapters_query = """
                SELECT chapter_id, arc_id, chapter_number, chapter_title,
                       chapter_summary, status, participation_count, success_rate
                FROM story_chapters
                WHERE arc_id = $1 AND status IN ('active', 'pending')
                ORDER BY chapter_number
            """
            
            chapter_rows = await postgres_db.fetch_all(chapters_query, current_arc.arc_id)
            
            active_chapters = [
                ChapterResponse(
                    chapter_id=row['chapter_id'],
                    arc_id=row['arc_id'],
                    chapter_number=row['chapter_number'],
                    chapter_title=row['chapter_title'],
                    chapter_summary=row['chapter_summary'],
                    status=row['status'],
                    participation_count=row['participation_count'] or 0,
                    success_rate=row['success_rate']
                )
                for row in chapter_rows
            ]
        
        # Get active NPCs
        active_npcs = []
        if current_arc:
            npcs_query = """
                SELECT npc_id, arc_id, npc_name, npc_role, npc_sprite,
                       npc_location, is_recurring, appearances_count
                FROM story_npcs
                WHERE arc_id = $1 AND (despawn_at IS NULL OR despawn_at > NOW())
                ORDER BY created_at DESC
                LIMIT 20
            """
            
            npc_rows = await postgres_db.fetch_all(npcs_query, current_arc.arc_id)
            # Would build StoryNPCResponse list here
        
        # Get participation stats
        participation_stats = {}
        if current_arc:
            stats_query = """
                SELECT 
                    COUNT(*) as total_participants,
                    AVG(contribution_score) as avg_contribution,
                    SUM(quests_completed) as total_quests,
                    SUM(chapters_completed) as total_chapters
                FROM story_participation
                WHERE arc_id = $1
            """
            
            stats_result = await postgres_db.fetch_one(stats_query, current_arc.arc_id)
            
            if stats_result:
                participation_stats = {
                    'total_participants': stats_result['total_participants'],
                    'avg_contribution': float(stats_result['avg_contribution']) if stats_result['avg_contribution'] else 0.0,
                    'total_quests_completed': stats_result['total_quests'] or 0,
                    'total_chapters_completed': stats_result['total_chapters'] or 0
                }
        
        # Get recent heroes
        recent_heroes = []
        heroes_query = """
            SELECT arc_id, heroes
            FROM story_arc_history
            WHERE heroes IS NOT NULL AND jsonb_array_length(heroes) > 0
            ORDER BY completed_at DESC
            LIMIT 3
        """
        
        hero_rows = await postgres_db.fetch_all(heroes_query)
        # Would build HeroSelectionResponse list here
        
        # Get story history
        history_rows = await postgres_db.fetch_all(
            """
            SELECT arc_id, arc_name, arc_summary, duration_days,
                   total_participants, success_rate, heroes, world_changes, completed_at
            FROM story_arc_history
            ORDER BY completed_at DESC
            LIMIT 5
            """,
        )
        
        story_history = [
            StoryHistoryEntry(
                arc_id=row['arc_id'],
                arc_name=row['arc_name'],
                arc_summary=row['arc_summary'],
                duration_days=row['duration_days'],
                total_participants=row['total_participants'],
                success_rate=row['success_rate'],
                heroes=row['heroes'] if isinstance(row['heroes'], list) else [],
                world_changes=row['world_changes'] if isinstance(row['world_changes'], list) else [],
                completed_at=row['completed_at']
            )
            for row in history_rows
        ]
        
        # System health
        system_health = {
            'storyline_enabled': True,
            'current_arc_active': current_arc is not None,
            'total_arcs_completed': len(story_history),
            'last_generation': datetime.now().isoformat()
        }
        
        dashboard = StoryDashboardResponse(
            current_arc=current_arc,
            active_chapters=active_chapters,
            active_npcs=active_npcs,
            participation_stats=participation_stats,
            recent_heroes=recent_heroes,
            story_history=story_history,
            system_health=system_health
        )
        
        return dashboard
        
    except Exception as e:
        logger.error(f"Failed to get dashboard: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# PLAYER QUEST ENDPOINTS
# ============================================================================

@router.get("/player/{player_id}/quests")
async def get_player_story_quests(
    player_id: int,
    arc_id: Optional[int] = None
):
    """
    Get player's story quests with progress.
    
    Args:
        player_id: Player ID
        arc_id: Optional arc ID filter
        
    Returns:
        List of quests with progress
    """
    try:
        quests = await story_quest_bridge.get_player_story_quests(
            player_id=player_id,
            arc_id=arc_id
        )
        
        return {
            "player_id": player_id,
            "arc_id": arc_id,
            "quests": quests,
            "total_quests": len(quests),
            "completed_quests": sum(1 for q in quests if q['is_completed'])
        }
        
    except Exception as e:
        logger.error(f"Failed to get player quests: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# ============================================================================
# BACKGROUND TASK FUNCTIONS
# ============================================================================

async def deploy_story_content(arc_id: int, arc_spec):
    """
    Background task to deploy story NPCs and quests.
    
    Args:
        arc_id: Story arc ID
        arc_spec: Story arc specification
    """
    try:
        logger.info(f"Deploying story content for arc #{arc_id}")
        
        # Get first chapter ID
        chapter_query = """
            SELECT chapter_id FROM story_chapters
            WHERE arc_id = $1 AND chapter_number = 1
            LIMIT 1
        """
        
        chapter_result = await postgres_db.fetch_one(chapter_query, arc_id)
        chapter_id = chapter_result['chapter_id'] if chapter_result else 0
        
        # Deploy each story event (NPC + Quest)
        for event in arc_spec.events:
            try:
                # Create NPC
                npc_response = await npc_story_integration.create_story_npc(
                    npc_spec=event,
                    arc_id=arc_id,
                    chapter_id=chapter_id,
                    arc_context={
                        'arc_name': arc_spec.story_arc_name,
                        'theme': arc_spec.theme,
                        'dominant_faction': arc_spec.faction_impact.faction_id if arc_spec.faction_impact else None,
                        'chapter_duration': getattr(settings, 'CHAPTER_DURATION_DAYS', 3)
                    }
                )
                
                # Create quest if NPC has one
                if event.quest:
                    quest_id = await story_quest_bridge.create_story_quest(
                        quest_spec=event.quest,
                        arc_id=arc_id,
                        chapter_number=1,
                        giver_npc_id=npc_response.npc_id
                    )
                    
                    logger.info(f"Created quest #{quest_id} for NPC #{npc_response.npc_id}")
                
                await asyncio.sleep(0.5)  # Rate limiting
                
            except Exception as e:
                logger.error(f"Failed to deploy story event: {e}")
                continue
        
        logger.info(f"✓ Story content deployed for arc #{arc_id}")
        
    except Exception as e:
        logger.error(f"Failed to deploy story content: {e}")


# ============================================================================
# ADMIN ENDPOINTS
# ============================================================================

@router.post("/admin/override", response_model=StoryArcResponse)
async def admin_story_override(
    request: AdminStoryOverride,
    background_tasks: BackgroundTasks
):
    """
    Admin override for custom story arc.
    
    WARNING: Bypasses normal generation flow.
    
    Args:
        request: Custom story specification
        background_tasks: Background tasks
        
    Returns:
        Created story arc
    """
    try:
        logger.warning(f"Admin story override: {request.reason}")
        
        # If overriding existing arc, complete it first
        if request.arc_id:
            await story_arc_manager.complete_arc(
                arc_id=request.arc_id,
                final_outcome=ChapterOutcome.MIXED
            )
        
        # Start custom arc
        arc_response = await story_arc_manager.start_new_arc(
            arc_spec=request.custom_arc_data,
            created_by='admin_override'
        )
        
        # Deploy content
        background_tasks.add_task(
            deploy_story_content,
            arc_id=arc_response.arc_id,
            arc_spec=request.custom_arc_data
        )
        
        return arc_response
        
    except Exception as e:
        logger.error(f"Admin override failed: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/metrics", response_model=StorySystemMetrics)
async def get_story_metrics():
    """
    Get story system performance metrics.
    
    Returns:
        System metrics and statistics
    """
    try:
        # Query aggregate metrics
        arcs_query = "SELECT COUNT(*) as count FROM story_arcs"
        arcs_result = await postgres_db.fetch_one(arcs_query)
        
        chapters_query = "SELECT COUNT(*) as count FROM story_chapters WHERE status = 'completed'"
        chapters_result = await postgres_db.fetch_one(chapters_query)
        
        npcs_query = "SELECT COUNT(*) as count FROM story_npcs"
        npcs_result = await postgres_db.fetch_one(npcs_query)
        
        quests_query = "SELECT COUNT(*) as count FROM story_quests"
        quests_result = await postgres_db.fetch_one(quests_query)
        
        # Participation rate
        participation_query = """
            SELECT AVG(
                (SELECT COUNT(DISTINCT player_id) FROM story_participation WHERE arc_id = sa.arc_id)::float /
                NULLIF((SELECT AVG(online_player_count) FROM world_metrics LIMIT 1), 0)
            ) as avg_rate
            FROM story_arcs sa
            WHERE status = 'completed'
        """
        
        part_result = await postgres_db.fetch_one(participation_query)
        avg_participation = part_result['avg_rate'] if part_result and part_result['avg_rate'] else 0.0
        
        # Success rate
        success_query = """
            SELECT AVG(success_rate) as avg_success
            FROM story_arc_history
        """
        
        success_result = await postgres_db.fetch_one(success_query)
        avg_success = success_result['avg_success'] if success_result and success_result['avg_success'] else 0.0
        
        metrics = StorySystemMetrics(
            total_arcs_generated=arcs_result['count'] if arcs_result else 0,
            total_chapters_completed=chapters_result['count'] if chapters_result else 0,
            total_npcs_spawned=npcs_result['count'] if npcs_result else 0,
            total_quests_created=quests_result['count'] if quests_result else 0,
            average_participation_rate=min(1.0, max(0.0, float(avg_participation))),
            average_success_rate=min(1.0, max(0.0, float(avg_success))),
            llm_generation_time_avg=0.0,  # TODO: Track generation times
            llm_cost_monthly=0.0  # TODO: Get from cost manager
        )
        
        return metrics
        
    except Exception as e:
        logger.error(f"Failed to get metrics: {e}")
        raise HTTPException(status_code=500, detail=str(e))