"""
Story Arc Manager
Manages active story arcs, chapter progression, and player participation
"""

import asyncio
import json
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from database import postgres_db, db
from config import settings
from models.storyline import (
    StoryArcSpec, StoryArcStatus, ChapterStatus, ChapterOutcome,
    StoryArcResponse, ChapterResponse, PlayerChoiceRequest,
    HeroSelectionResponse, StoryParticipationStats
)


class StoryArcManager:
    """
    Manages active story arcs and chapter progression.
    
    Responsibilities:
    - Track current arc and chapter
    - Store player choices
    - Manage arc transitions
    - Handle success/failure outcomes
    - Archive completed arcs
    - Select Hero of the Week
    - Coordinate with Bridge Layer for NPC/quest spawns
    """
    
    def __init__(self):
        """Initialize Story Arc Manager"""
        self.current_arc_id = None
        self.auto_advance = getattr(settings, 'ARC_AUTO_ADVANCE', True)
        self.chapter_duration_days = getattr(settings, 'CHAPTER_DURATION_DAYS', 3)
        self.hero_selection_top_n = getattr(settings, 'HERO_SELECTION_TOP_N', 3)
        
        logger.info("Story Arc Manager initialized")
    
    async def start_new_arc(
        self,
        arc_spec: StoryArcSpec,
        created_by: str = 'system'
    ) -> StoryArcResponse:
        """
        Start a new story arc.
        
        Process:
        1. Store arc in database
        2. Set current_arc_id globally
        3. Create initial chapter (chapter 1)
        4. Spawn initial NPCs via Bridge Layer
        5. Create opening quests
        6. Broadcast announcement
        7. Set chapter 1 as active
        
        Args:
            arc_spec: Complete story arc specification from LLM
            created_by: Creator ID ('system' or admin ID)
            
        Returns:
            StoryArcResponse with arc ID and status
        """
        try:
            logger.info(f"Starting new story arc: {arc_spec.story_arc_name}")
            
            # Check if active arc already exists
            existing_arc = await self.get_current_arc()
            if existing_arc and existing_arc.status == StoryArcStatus.ACTIVE:
                logger.warning(f"Active arc already exists: {existing_arc.arc_name}")
                raise ValueError(f"Cannot start new arc - arc #{existing_arc.arc_id} is still active")
            
            # Calculate expected end date
            expected_end = datetime.now(UTC) + timedelta(days=arc_spec.duration_days)
            
            # Store arc in database
            arc_query = """
                INSERT INTO story_arcs
                (arc_name, arc_summary, total_chapters, current_chapter, theme, 
                 dominant_faction, arc_data, status, expected_end_at, created_by)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)
                RETURNING arc_id, started_at
            """
            
            # Determine total chapters (default to duration/3 days per chapter)
            total_chapters = max(3, arc_spec.duration_days // self.chapter_duration_days)
            
            # Get dominant faction from spec
            dominant_faction = None
            if arc_spec.faction_impact:
                dominant_faction = arc_spec.faction_impact.faction_id
            
            result = await postgres_db.fetch_one(
                arc_query,
                arc_spec.story_arc_name,
                arc_spec.story_arc_summary,
                total_chapters,
                1,  # Start at chapter 1
                arc_spec.theme,
                dominant_faction,
                json.dumps(arc_spec.model_dump()),
                StoryArcStatus.ACTIVE.value,
                expected_end,
                created_by
            )
            
            arc_id = result['arc_id']
            started_at = result['started_at']
            
            # Set as current arc
            self.current_arc_id = arc_id
            await db.set("storyline:current_arc_id", arc_id)
            
            # Create first chapter
            await self._create_chapter(
                arc_id=arc_id,
                chapter_number=1,
                arc_spec=arc_spec
            )
            
            # Spawn NPCs and create quests (delegated to integration layers)
            npcs_spawned = 0
            quests_created = 0
            
            try:
                # This will be implemented by NPC Story Integration
                # For now, just log the intent
                logger.info(f"Will spawn {len(arc_spec.events)} NPCs for arc #{arc_id}")
                npcs_spawned = len(arc_spec.events)
                
                # Count quests
                quests_created = sum(1 for e in arc_spec.events if e.quest)
                
            except Exception as e:
                logger.warning(f"NPC/Quest spawning deferred: {e}")
            
            # Broadcast announcement
            await self._broadcast_arc_start(arc_id, arc_spec)
            
            # Create response
            response = StoryArcResponse(
                arc_id=arc_id,
                arc_name=arc_spec.story_arc_name,
                arc_summary=arc_spec.story_arc_summary,
                total_chapters=total_chapters,
                current_chapter=1,
                theme=arc_spec.theme,
                dominant_faction=dominant_faction,
                status=StoryArcStatus.ACTIVE,
                started_at=started_at,
                expected_end_at=expected_end,
                npcs_spawned=npcs_spawned,
                quests_created=quests_created
            )
            
            logger.info(
                f"✓ Story arc #{arc_id} started: {arc_spec.story_arc_name} "
                f"({total_chapters} chapters, {arc_spec.duration_days} days)"
            )
            
            return response
            
        except Exception as e:
            logger.error(f"Failed to start story arc: {e}", exc_info=True)
            raise
    
    async def advance_chapter(
        self,
        arc_id: int,
        outcome: ChapterOutcome,
        player_choices: Optional[List[Dict[str, Any]]] = None,
        completion_rate: Optional[float] = None
    ) -> ChapterResponse:
        """
        Advance to next chapter based on outcome.
        
        Logic:
        - Success → progress to next chapter
        - Failure → alternate path or retry
        - Final chapter → complete arc, archive, start new
        
        Args:
            arc_id: Story arc ID
            outcome: Current chapter outcome
            player_choices: Player decisions made this chapter
            completion_rate: Player completion percentage
            
        Returns:
            Next chapter specification
        """
        try:
            logger.info(f"Advancing arc #{arc_id} with outcome: {outcome.value}")
            
            # Get current arc
            arc = await self._get_arc_by_id(arc_id)
            if not arc:
                raise ValueError(f"Arc #{arc_id} not found")
            
            current_chapter = arc['current_chapter']
            total_chapters = arc['total_chapters']
            
            # Complete current chapter
            await self._complete_chapter(
                arc_id=arc_id,
                chapter_number=current_chapter,
                outcome=outcome,
                completion_rate=completion_rate
            )
            
            # Store player choices if provided
            if player_choices:
                await self._store_chapter_choices(arc_id, current_chapter, player_choices)
            
            # Check if this was the final chapter
            if current_chapter >= total_chapters:
                logger.info(f"Arc #{arc_id} completed all chapters")
                await self.complete_arc(arc_id, outcome)
                
                # Return final chapter response
                return ChapterResponse(
                    chapter_id=0,  # No next chapter
                    arc_id=arc_id,
                    chapter_number=current_chapter,
                    chapter_title="Arc Complete",
                    chapter_summary="The story arc has concluded",
                    status=ChapterStatus.COMPLETED
                )
            
            # Advance to next chapter
            next_chapter = current_chapter + 1
            
            # Update arc's current chapter
            await postgres_db.execute(
                "UPDATE story_arcs SET current_chapter = $1 WHERE arc_id = $2",
                next_chapter,
                arc_id
            )
            
            # Create next chapter
            chapter_response = await self._create_next_chapter(
                arc_id=arc_id,
                chapter_number=next_chapter,
                previous_outcome=outcome,
                arc_data=json.loads(arc['arc_data']) if isinstance(arc['arc_data'], str) else arc['arc_data']
            )
            
            # Clear cache
            await db.delete("storyline:current_arc")
            await db.delete(f"storyline:arc:{arc_id}")
            
            logger.info(f"✓ Advanced to chapter {next_chapter}/{total_chapters}")
            
            return chapter_response
            
        except Exception as e:
            logger.error(f"Failed to advance chapter: {e}", exc_info=True)
            raise
    
    async def record_player_choice(
        self,
        arc_id: int,
        player_id: int,
        chapter: int,
        choice_request: PlayerChoiceRequest
    ) -> bool:
        """
        Record player choice for story influence.
        
        Choices include:
        - Quest outcomes (helped NPC or refused)
        - Dialogue selections (friendly, hostile, neutral)
        - Faction allegiance (support Rune Alliance vs Shadow Cult)
        - Boss defeat/escape (killed boss vs fled)
        
        Impacts:
        - Next chapter generation
        - NPC reactions
        - World state changes
        - Player reputation
        
        Args:
            arc_id: Story arc ID
            player_id: Player ID
            chapter: Chapter number
            choice_request: Choice details
            
        Returns:
            Success status
        """
        try:
            # Get or create participation record
            participation_query = """
                INSERT INTO story_participation
                (arc_id, player_id, choices_made)
                VALUES ($1, $2, $3)
                ON CONFLICT (arc_id, player_id)
                DO UPDATE SET
                    choices_made = 
                        COALESCE(story_participation.choices_made, '[]'::jsonb) || $3::jsonb,
                    last_participation = NOW()
            """
            
            choice_data = {
                'chapter': chapter,
                'choice_type': choice_request.choice_type,
                'choice_data': choice_request.choice_data,
                'impact_score': choice_request.impact_score,
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            await postgres_db.execute(
                participation_query,
                arc_id,
                player_id,
                json.dumps([choice_data])
            )
            
            logger.debug(
                f"Recorded choice for player {player_id} in arc {arc_id}, chapter {chapter}"
            )
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to record player choice: {e}")
            return False
    
    async def get_current_arc(self) -> Optional[StoryArcResponse]:
        """Get currently active story arc"""
        try:
            # Try cache first
            cached = await db.get("storyline:current_arc")
            if cached:
                return StoryArcResponse(**cached)
            
            # Query database
            query = """
                SELECT arc_id, arc_name, arc_summary, total_chapters, current_chapter,
                       theme, dominant_faction, status, started_at, expected_end_at
                FROM story_arcs
                WHERE status = 'active'
                ORDER BY started_at DESC
                LIMIT 1
            """
            
            result = await postgres_db.fetch_one(query)
            
            if result:
                # Count NPCs and quests
                npcs_count_query = """
                    SELECT COUNT(*) as count
                    FROM story_npcs
                    WHERE arc_id = $1
                """
                npcs_result = await postgres_db.fetch_one(npcs_count_query, result['arc_id'])
                npcs_spawned = npcs_result['count'] if npcs_result else 0
                
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
                    quests_created=0  # TODO: Count quests
                )
                
                # Cache for 5 minutes
                await db.set("storyline:current_arc", response.model_dump(), expire=300)
                
                return response
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to get current arc: {e}")
            return None
    
    async def complete_arc(
        self,
        arc_id: int,
        final_outcome: ChapterOutcome
    ) -> bool:
        """
        Complete story arc and archive.
        
        Process:
        1. Mark arc as completed
        2. Calculate aggregate outcomes
        3. Identify top participants (Hero of the Week)
        4. Store in archive
        5. Clear current arc
        6. Prepare for next arc
        
        Args:
            arc_id: Story arc ID
            final_outcome: Final chapter outcome
            
        Returns:
            Success status
        """
        try:
            logger.info(f"Completing story arc #{arc_id}")
            
            # Get arc data
            arc = await self._get_arc_by_id(arc_id)
            if not arc:
                raise ValueError(f"Arc #{arc_id} not found")
            
            # Calculate completion metrics
            metrics = await self._calculate_arc_metrics(arc_id)
            
            # Select heroes
            heroes = await self.select_heroes_of_arc(arc_id)
            
            # Mark heroes in participation table
            for hero in heroes:
                await postgres_db.execute(
                    "UPDATE story_participation SET is_hero = TRUE WHERE arc_id = $1 AND player_id = $2",
                    arc_id,
                    hero['player_id']
                )
            
            # Determine world changes from arc
            world_changes = await self._determine_world_changes(arc, final_outcome)
            
            # Archive arc
            archive_query = """
                INSERT INTO story_arc_history
                (arc_id, arc_name, arc_summary, duration_days, total_participants,
                 total_chapters, chapters_completed, success_rate, heroes, world_changes, outcome_summary)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
            """
            
            duration_days = (datetime.now(UTC) - arc['started_at']).days
            outcome_summary = self._build_outcome_summary(final_outcome, metrics)
            
            await postgres_db.execute(
                archive_query,
                arc_id,
                arc['arc_name'],
                arc['arc_summary'],
                duration_days,
                metrics['total_participants'],
                arc['total_chapters'],
                metrics['chapters_completed'],
                metrics['success_rate'],
                json.dumps(heroes),
                json.dumps(world_changes),
                outcome_summary
            )
            
            # Mark arc as completed
            await postgres_db.execute(
                "UPDATE story_arcs SET status = $1, completed_at = NOW() WHERE arc_id = $2",
                StoryArcStatus.COMPLETED.value,
                arc_id
            )
            
            # Clear current arc
            self.current_arc_id = None
            await db.delete("storyline:current_arc_id")
            await db.delete("storyline:current_arc")
            
            logger.info(
                f"✓ Arc #{arc_id} completed and archived "
                f"({metrics['total_participants']} participants, "
                f"{metrics['success_rate']:.1%} success rate)"
            )
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to complete arc: {e}", exc_info=True)
            return False
    
    async def select_heroes_of_arc(
        self,
        arc_id: int
    ) -> List[Dict[str, Any]]:
        """
        Select Hero of the Week/Arc participants.
        
        Selection Criteria:
        1. Contribution score (primary)
        2. Chapters completed (secondary)
        3. Quests completed (tertiary)
        
        Returns top N participants (default 3).
        
        Args:
            arc_id: Story arc ID
            
        Returns:
            List of hero dicts
        """
        try:
            query = """
                SELECT 
                    player_id,
                    contribution_score,
                    chapters_completed,
                    quests_completed,
                    choices_made
                FROM story_participation
                WHERE arc_id = $1
                ORDER BY 
                    contribution_score DESC,
                    chapters_completed DESC,
                    quests_completed DESC
                LIMIT $2
            """
            
            rows = await postgres_db.fetch_all(query, arc_id, self.hero_selection_top_n)
            
            heroes = []
            for i, row in enumerate(rows, 1):
                hero = {
                    'rank': i,
                    'player_id': row['player_id'],
                    'contribution_score': row['contribution_score'],
                    'chapters_completed': row['chapters_completed'],
                    'quests_completed': row['quests_completed'],
                    'notable_choices': self._extract_notable_choices(row['choices_made'])
                }
                heroes.append(hero)
            
            logger.info(f"Selected {len(heroes)} heroes for arc #{arc_id}")
            
            return heroes
            
        except Exception as e:
            logger.error(f"Failed to select heroes: {e}")
            return []
    
    async def check_chapter_advancement(self) -> Optional[Dict[str, Any]]:
        """
        Check if current chapter should advance (scheduled job).
        
        Advancement Triggers:
        - Chapter duration expired
        - Milestone objectives met
        - Player completion threshold reached
        
        Returns:
            Advancement result or None if no action needed
        """
        try:
            # Get current arc
            arc = await self.get_current_arc()
            if not arc or arc.status != StoryArcStatus.ACTIVE:
                return None
            
            # Get current chapter
            chapter = await self._get_chapter(arc.arc_id, arc.current_chapter)
            if not chapter or chapter['status'] != 'active':
                return None
            
            # Check advancement criteria
            should_advance = False
            reason = ""
            outcome = ChapterOutcome.SUCCESS  # Default
            
            # Criterion 1: Time-based (chapter duration)
            if chapter['started_at']:
                chapter_age = datetime.now(UTC) - chapter['started_at']
                if chapter_age.days >= self.chapter_duration_days:
                    should_advance = True
                    reason = f"Chapter duration ({self.chapter_duration_days} days) elapsed"
            
            # Criterion 2: Completion threshold
            completion = await self._get_chapter_completion_rate(arc.arc_id, arc.current_chapter)
            if completion >= 0.7:  # 70% completion
                should_advance = True
                reason = f"Completion threshold reached ({completion:.1%})"
                outcome = ChapterOutcome.SUCCESS
            elif completion <= 0.3 and chapter_age.days >= self.chapter_duration_days:
                should_advance = True
                reason = f"Low completion ({completion:.1%}) and timeout"
                outcome = ChapterOutcome.FAILURE
            
            if should_advance and self.auto_advance:
                logger.info(f"Auto-advancing chapter: {reason}")
                next_chapter = await self.advance_chapter(
                    arc_id=arc.arc_id,
                    outcome=outcome,
                    completion_rate=completion
                )
                
                return {
                    'arc_id': arc.arc_id,
                    'old_chapter': arc.current_chapter,
                    'new_chapter': next_chapter.chapter_number,
                    'reason': reason,
                    'outcome': outcome.value
                }
            
            return None
            
        except Exception as e:
            logger.error(f"Chapter advancement check failed: {e}")
            return None
    
    async def get_arc_participants(
        self,
        arc_id: int,
        limit: int = 100
    ) -> List[StoryParticipationStats]:
        """
        Get all players participating in arc.
        
        Args:
            arc_id: Story arc ID
            limit: Maximum participants to return
            
        Returns:
            List of participation stats
        """
        try:
            query = """
                SELECT 
                    player_id,
                    arc_id,
                    chapters_completed,
                    quests_completed,
                    contribution_score,
                    is_hero,
                    ROW_NUMBER() OVER (ORDER BY contribution_score DESC) as rank
                FROM story_participation
                WHERE arc_id = $1
                ORDER BY contribution_score DESC
                LIMIT $2
            """
            
            rows = await postgres_db.fetch_all(query, arc_id, limit)
            
            participants = [
                StoryParticipationStats(
                    player_id=row['player_id'],
                    arc_id=row['arc_id'],
                    chapters_completed=row['chapters_completed'],
                    quests_completed=row['quests_completed'],
                    contribution_score=row['contribution_score'],
                    is_hero=row['is_hero'],
                    rank=row['rank']
                )
                for row in rows
            ]
            
            return participants
            
        except Exception as e:
            logger.error(f"Failed to get arc participants: {e}")
            return []
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _get_arc_by_id(self, arc_id: int) -> Optional[Dict[str, Any]]:
        """Get arc by ID from database"""
        try:
            query = """
                SELECT * FROM story_arcs WHERE arc_id = $1
            """
            return await postgres_db.fetch_one(query, arc_id)
        except Exception as e:
            logger.error(f"Failed to get arc #{arc_id}: {e}")
            return None
    
    async def _create_chapter(
        self,
        arc_id: int,
        chapter_number: int,
        arc_spec: StoryArcSpec
    ) -> int:
        """Create a chapter from arc specification"""
        try:
            # Extract chapter-specific events (distribute events across chapters)
            total_chapters = max(3, arc_spec.duration_days // self.chapter_duration_days)
            events_per_chapter = len(arc_spec.events) // total_chapters
            start_idx = (chapter_number - 1) * events_per_chapter
            end_idx = start_idx + events_per_chapter if chapter_number < total_chapters else len(arc_spec.events)
            
            chapter_events = arc_spec.events[start_idx:end_idx]
            
            query = """
                INSERT INTO story_chapters
                (arc_id, chapter_number, chapter_title, chapter_summary, events, world_modifiers, status)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING chapter_id
            """
            
            chapter_title = f"Chapter {chapter_number}: {arc_spec.story_arc_name}"
            chapter_summary = f"Part {chapter_number} of {arc_spec.story_arc_summary}"
            
            # Apply world modifiers if first chapter
            world_modifiers = arc_spec.world_modifiers if chapter_number == 1 else []
            
            result = await postgres_db.fetch_one(
                query,
                arc_id,
                chapter_number,
                chapter_title,
                chapter_summary,
                json.dumps([e.model_dump() for e in chapter_events]),
                json.dumps([m.model_dump() for m in world_modifiers]),
                ChapterStatus.ACTIVE.value if chapter_number == 1 else ChapterStatus.PENDING.value
            )
            
            chapter_id = result['chapter_id']
            
            # If chapter 1, mark as started
            if chapter_number == 1:
                await postgres_db.execute(
                    "UPDATE story_chapters SET started_at = NOW() WHERE chapter_id = $1",
                    chapter_id
                )
            
            logger.info(f"Created chapter {chapter_number} for arc #{arc_id}")
            
            return chapter_id
            
        except Exception as e:
            logger.error(f"Failed to create chapter: {e}")
            raise
    
    async def _create_next_chapter(
        self,
        arc_id: int,
        chapter_number: int,
        previous_outcome: ChapterOutcome,
        arc_data: Dict[str, Any]
    ) -> ChapterResponse:
        """Create next chapter based on previous outcome"""
        try:
            # Determine chapter path based on outcome
            if previous_outcome in [ChapterOutcome.SUCCESS, ChapterOutcome.MIXED]:
                world_changes = arc_data.get('success_outcomes', {}).get('world_changes', [])
            else:
                world_changes = arc_data.get('failure_outcomes', {}).get('world_changes', [])
            
            # Create chapter
            query = """
                INSERT INTO story_chapters
                (arc_id, chapter_number, chapter_title, chapter_summary, events, status, started_at)
                VALUES ($1, $2, $3, $4, $5, $6, NOW())
                RETURNING chapter_id
            """
            
            chapter_title = f"Chapter {chapter_number}"
            chapter_summary = f"Following {previous_outcome.value} outcome: {', '.join(world_changes[:2])}"
            
            # Re-use events from arc data (simplified - in production would generate new events)
            events = arc_data.get('events', [])
            
            result = await postgres_db.fetch_one(
                query,
                arc_id,
                chapter_number,
                chapter_title,
                chapter_summary,
                json.dumps(events),
                ChapterStatus.ACTIVE.value
            )
            
            chapter_id = result['chapter_id']
            
            return ChapterResponse(
                chapter_id=chapter_id,
                arc_id=arc_id,
                chapter_number=chapter_number,
                chapter_title=chapter_title,
                chapter_summary=chapter_summary,
                status=ChapterStatus.ACTIVE
            )
            
        except Exception as e:
            logger.error(f"Failed to create next chapter: {e}")
            raise
    
    async def _complete_chapter(
        self,
        arc_id: int,
        chapter_number: int,
        outcome: ChapterOutcome,
        completion_rate: Optional[float] = None
    ):
        """Mark chapter as completed"""
        try:
            query = """
                UPDATE story_chapters
                SET status = $1,
                    completed_at = NOW(),
                    outcome = $2,
                    success_rate = $3
                WHERE arc_id = $4 AND chapter_number = $5
            """
            
            await postgres_db.execute(
                query,
                ChapterStatus.COMPLETED.value,
                outcome.value,
                completion_rate,
                arc_id,
                chapter_number
            )
            
            logger.info(f"Chapter {chapter_number} of arc #{arc_id} completed: {outcome.value}")
            
        except Exception as e:
            logger.error(f"Failed to complete chapter: {e}")
    
    async def _get_chapter(self, arc_id: int, chapter_number: int) -> Optional[Dict[str, Any]]:
        """Get chapter by arc and number"""
        try:
            query = """
                SELECT * FROM story_chapters
                WHERE arc_id = $1 AND chapter_number = $2
            """
            return await postgres_db.fetch_one(query, arc_id, chapter_number)
        except Exception as e:
            logger.error(f"Failed to get chapter: {e}")
            return None
    
    async def _get_chapter_completion_rate(self, arc_id: int, chapter_number: int) -> float:
        """Calculate chapter completion rate from player participation"""
        try:
            # Get total players who started chapter
            total_query = """
                SELECT COUNT(DISTINCT player_id) as total
                FROM story_participation
                WHERE arc_id = $1 AND chapters_completed >= $2 - 1
            """
            total_result = await postgres_db.fetch_one(total_query, arc_id, chapter_number)
            total = total_result['total'] if total_result else 0
            
            if total == 0:
                return 0.0
            
            # Get players who completed chapter
            completed_query = """
                SELECT COUNT(DISTINCT player_id) as completed
                FROM story_participation
                WHERE arc_id = $1 AND chapters_completed >= $2
            """
            completed_result = await postgres_db.fetch_one(completed_query, arc_id, chapter_number)
            completed = completed_result['completed'] if completed_result else 0
            
            return completed / total if total > 0 else 0.0
            
        except Exception as e:
            logger.error(f"Failed to calculate completion rate: {e}")
            return 0.0
    
    async def _calculate_arc_metrics(self, arc_id: int) -> Dict[str, Any]:
        """Calculate aggregate metrics for completed arc"""
        try:
            # Total participants
            participants_query = """
                SELECT COUNT(*) as count
                FROM story_participation
                WHERE arc_id = $1
            """
            part_result = await postgres_db.fetch_one(participants_query, arc_id)
            total_participants = part_result['count'] if part_result else 0
            
            # Chapters completed
            chapters_query = """
                SELECT COUNT(*) as count
                FROM story_chapters
                WHERE arc_id = $1 AND status = 'completed'
            """
            chap_result = await postgres_db.fetch_one(chapters_query, arc_id)
            chapters_completed = chap_result['count'] if chap_result else 0
            
            # Success rate (chapters with success outcome / total chapters)
            success_query = """
                SELECT COUNT(*) as count
                FROM story_chapters
                WHERE arc_id = $1 AND outcome = 'success'
            """
            succ_result = await postgres_db.fetch_one(success_query, arc_id)
            success_count = succ_result['count'] if succ_result else 0
            
            success_rate = success_count / chapters_completed if chapters_completed > 0 else 0.0
            
            return {
                'total_participants': total_participants,
                'chapters_completed': chapters_completed,
                'success_rate': success_rate
            }
            
        except Exception as e:
            logger.error(f"Failed to calculate arc metrics: {e}")
            return {
                'total_participants': 0,
                'chapters_completed': 0,
                'success_rate': 0.0
            }
    
    async def _determine_world_changes(
        self,
        arc: Dict[str, Any],
        final_outcome: ChapterOutcome
    ) -> List[str]:
        """Determine world changes from arc completion"""
        try:
            arc_data = json.loads(arc['arc_data']) if isinstance(arc['arc_data'], str) else arc['arc_data']
            
            if final_outcome in [ChapterOutcome.SUCCESS, ChapterOutcome.MIXED]:
                return arc_data.get('success_outcomes', {}).get('world_changes', [])
            else:
                return arc_data.get('failure_outcomes', {}).get('world_changes', [])
                
        except Exception as e:
            logger.error(f"Failed to determine world changes: {e}")
            return []
    
    def _build_outcome_summary(
        self,
        outcome: ChapterOutcome,
        metrics: Dict[str, Any]
    ) -> str:
        """Build human-readable outcome summary"""
        return (
            f"Arc concluded with {outcome.value} outcome. "
            f"{metrics['total_participants']} players participated, "
            f"completing {metrics['chapters_completed']} chapters "
            f"with {metrics['success_rate']:.1%} success rate."
        )
    
    async def _broadcast_arc_start(self, arc_id: int, arc_spec: StoryArcSpec):
        """Broadcast arc start announcement (via Bridge Layer)"""
        try:
            announcement = {
                'type': 'story_arc_start',
                'arc_id': arc_id,
                'arc_name': arc_spec.story_arc_name,
                'message': f"A new adventure begins: {arc_spec.story_arc_name}!",
                'duration_days': arc_spec.duration_days
            }
            
            # Store in cache for Bridge Layer pickup
            await db.set(
                f"broadcast:arc_start:{arc_id}",
                announcement,
                expire=3600
            )
            
            logger.info(f"Broadcast queued for arc #{arc_id}")
            
        except Exception as e:
            logger.warning(f"Failed to queue broadcast: {e}")
    
    async def _store_chapter_choices(
        self,
        arc_id: int,
        chapter: int,
        choices: List[Dict[str, Any]]
    ):
        """Store player choices for chapter"""
        try:
            # Store as chapter metadata
            await postgres_db.execute(
                """
                UPDATE story_chapters
                SET events = COALESCE(events, '{}'::jsonb) || $1::jsonb
                WHERE arc_id = $2 AND chapter_number = $3
                """,
                json.dumps({'player_choices': choices}),
                arc_id,
                chapter
            )
        except Exception as e:
            logger.warning(f"Failed to store chapter choices: {e}")
    
    def _extract_notable_choices(self, choices_json: Any) -> List[str]:
        """Extract notable choices from JSON for hero description"""
        try:
            if not choices_json:
                return []
            
            choices = json.loads(choices_json) if isinstance(choices_json, str) else choices_json
            
            if not isinstance(choices, list):
                return []
            
            # Extract high-impact choices
            notable = [
                c.get('choice_type', 'Unknown')
                for c in choices
                if isinstance(c, dict) and c.get('impact_score', 0) >= 50
            ]
            
            return notable[:3]  # Top 3
            
        except Exception as e:
            logger.debug(f"Failed to extract notable choices: {e}")
            return []


# Global instance
story_arc_manager = StoryArcManager()