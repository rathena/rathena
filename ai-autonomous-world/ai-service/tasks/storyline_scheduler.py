"""
Storyline Background Task Scheduler
Automated story arc generation and chapter advancement
"""

import asyncio
from datetime import datetime, UTC
from typing import Optional
from loguru import logger
from apscheduler.schedulers.asyncio import AsyncIOScheduler
from apscheduler.triggers.cron import CronTrigger
from apscheduler.triggers.interval import IntervalTrigger

from config import settings
from storyline.world_state_collector import world_state_collector
from storyline.storyline_generator import storyline_generator
from storyline.story_arc_manager import story_arc_manager
from storyline.npc_story_integration import npc_story_integration
from storyline.story_quest_bridge import story_quest_bridge
from database import postgres_db


# Global scheduler instance
scheduler = AsyncIOScheduler()


# ============================================================================
# SCHEDULED JOBS
# ============================================================================

async def hourly_world_state_collection():
    """
    Collect world state hourly for analysis and caching.
    
    Purpose:
    - Keep world state fresh in cache
    - Enable quick story generation when needed
    - Monitor world trends
    """
    try:
        logger.info("🔄 Hourly world state collection")
        
        # Collect complete world state
        world_state = await world_state_collector.collect_complete_world_state(
            force_fresh=True
        )
        
        # Export to JSON for debugging/analysis
        timestamp = datetime.now(UTC).strftime("%Y%m%d_%H%M%S")
        export_path = f"data/world_states/world_state_{timestamp}.json"
        
        await world_state_collector.export_to_json(world_state, export_path)
        
        logger.info(
            f"✓ World state collected: {world_state.online_player_count} players, "
            f"level {world_state.player_average_level}, "
            f"karma {world_state.global_karma}"
        )
        
    except Exception as e:
        logger.error(f"Hourly world state collection failed: {e}", exc_info=True)


async def weekly_story_arc_generation():
    """
    Generate new story arc every 7 days or when previous arc completes.
    
    Triggered:
    - Weekly on Sunday 00:00 UTC
    - When arc completes early
    - Manual trigger via API
    """
    try:
        logger.info("📖 Weekly story arc generation")
        
        # Check if active arc exists
        current_arc = await story_arc_manager.get_current_arc()
        
        if current_arc and current_arc.status == 'active':
            logger.info(f"Active arc exists: {current_arc.arc_name}. Skipping generation.")
            return
        
        # Collect world state
        world_state = await world_state_collector.collect_complete_world_state(
            force_fresh=True
        )
        
        # Get previous arcs for continuity
        previous_arcs_query = """
            SELECT arc_id, arc_name, arc_summary, theme, outcome_summary
            FROM story_arc_history
            ORDER BY completed_at DESC
            LIMIT 5
        """
        
        previous_arc_rows = await postgres_db.fetch_all(previous_arcs_query)
        previous_arcs = [dict(row) for row in previous_arc_rows]
        
        # Generate story arc
        arc_spec = await storyline_generator.generate_story_arc(
            world_state=world_state,
            previous_arcs=previous_arcs
        )
        
        if not arc_spec:
            logger.error("Story arc generation failed")
            return
        
        # Start the arc
        arc_response = await story_arc_manager.start_new_arc(
            arc_spec=arc_spec,
            created_by='scheduler'
        )
        
        # Deploy story content (NPCs and quests)
        await deploy_story_content_task(arc_response.arc_id, arc_spec)
        
        logger.info(
            f"✓ New story arc generated: {arc_spec.story_arc_name} "
            f"(#{arc_response.arc_id}, {arc_spec.duration_days} days)"
        )
        
    except Exception as e:
        logger.error(f"Weekly story arc generation failed: {e}", exc_info=True)


async def daily_chapter_advancement_check():
    """
    Check if current chapter should advance (daily at 23:00 UTC).
    
    Advancement Triggers:
    - Chapter duration elapsed
    - Completion threshold reached (70%+)
    - Timeout with low completion
    """
    try:
        logger.info("📊 Daily chapter advancement check")
        
        result = await story_arc_manager.check_chapter_advancement()
        
        if result:
            logger.info(
                f"✓ Chapter advanced: Arc #{result['arc_id']}, "
                f"Chapter {result['old_chapter']} → {result['new_chapter']} "
                f"(Reason: {result['reason']})"
            )
        else:
            logger.info("No chapter advancement needed")
        
    except Exception as e:
        logger.error(f"Chapter advancement check failed: {e}", exc_info=True)


async def weekly_hero_selection():
    """
    Select Hero of the Week every Sunday at 20:00 UTC.
    
    Process:
    - Get active arc
    - Select top 3 contributors
    - Generate hero recognition
    - Award special rewards
    - Broadcast announcement
    """
    try:
        if not getattr(settings, 'hero_recognition_enabled', True):
            logger.info("Hero recognition disabled")
            return
        
        logger.info("🏆 Weekly hero selection")
        
        # Get current arc
        current_arc = await story_arc_manager.get_current_arc()
        
        if not current_arc:
            logger.info("No active arc for hero selection")
            return
        
        # Get top participants
        heroes = await story_arc_manager.select_heroes_of_arc(current_arc.arc_id)
        
        if not heroes:
            logger.info("No participants to recognize")
            return
        
        # Generate hero recognition
        hero_response = await npc_story_integration.generate_hero_recognition(
            arc_id=current_arc.arc_id,
            top_participants=heroes
        )
        
        logger.info(
            f"✓ Heroes selected for arc #{current_arc.arc_id}: "
            f"{len(hero_response.heroes)} players recognized"
        )
        
    except Exception as e:
        logger.error(f"Weekly hero selection failed: {e}", exc_info=True)


async def midnight_arc_expiration_check():
    """
    Check for expired arcs and auto-complete them (daily at 00:00 UTC).
    
    Process:
    - Find arcs past expected_end_at
    - Calculate final metrics
    - Complete and archive
    - Trigger new arc generation if needed
    """
    try:
        logger.info("⏰ Midnight arc expiration check")
        
        # Find expired arcs
        query = """
            SELECT arc_id, arc_name, expected_end_at
            FROM story_arcs
            WHERE status = 'active' 
              AND expected_end_at < NOW()
        """
        
        expired_arcs = await postgres_db.fetch_all(query)
        
        for arc_row in expired_arcs:
            arc_id = arc_row['arc_id']
            
            logger.info(f"Arc #{arc_id} expired, auto-completing")
            
            # Auto-complete with mixed outcome
            from models.storyline import ChapterOutcome
            await story_arc_manager.complete_arc(
                arc_id=arc_id,
                final_outcome=ChapterOutcome.MIXED
            )
            
            logger.info(f"✓ Arc #{arc_id} auto-completed (expired)")
        
        # Trigger new arc generation if no active arc
        if expired_arcs:
            await asyncio.sleep(60)  # Wait 1 minute
            await weekly_story_arc_generation()
        
    except Exception as e:
        logger.error(f"Arc expiration check failed: {e}", exc_info=True)


# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

async def deploy_story_content_task(arc_id: int, arc_spec):
    """
    Deploy story NPCs and quests for an arc.
    
    Args:
        arc_id: Story arc ID
        arc_spec: Story arc specification
    """
    try:
        logger.info(f"Deploying story content for arc #{arc_id}")
        
        # Get first chapter
        chapter_query = """
            SELECT chapter_id FROM story_chapters
            WHERE arc_id = $1 AND chapter_number = 1
            LIMIT 1
        """
        
        chapter_result = await postgres_db.fetch_one(chapter_query, arc_id)
        chapter_id = chapter_result['chapter_id'] if chapter_result else 0
        
        # Deploy each event
        for i, event in enumerate(arc_spec.events, 1):
            try:
                logger.info(f"Deploying event {i}/{len(arc_spec.events)}")
                
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
                
                # Create quest if present
                if event.quest:
                    quest_id = await story_quest_bridge.create_story_quest(
                        quest_spec=event.quest,
                        arc_id=arc_id,
                        chapter_number=1,
                        giver_npc_id=npc_response.npc_id
                    )
                    
                    logger.info(f"Created quest #{quest_id} for NPC #{npc_response.npc_id}")
                
                # Rate limiting
                await asyncio.sleep(0.5)
                
            except Exception as e:
                logger.error(f"Failed to deploy event: {e}")
                continue
        
        logger.info(f"✓ Story content deployed for arc #{arc_id}")
        
    except Exception as e:
        logger.error(f"Story content deployment failed: {e}", exc_info=True)


# ============================================================================
# SCHEDULER INITIALIZATION
# ============================================================================

def initialize_storyline_scheduler():
    """
    Initialize and start storyline background scheduler.
    
    Jobs:
    - Hourly: World state collection
    - Weekly (Sun 00:00): Story arc generation
    - Daily (23:00): Chapter advancement check
    - Weekly (Sun 20:00): Hero selection
    - Daily (00:00): Arc expiration check
    """
    try:
        if not getattr(settings, 'storyline_enabled', True):
            logger.info("Storyline system disabled - scheduler not started")
            return
        
        logger.info("Initializing storyline scheduler...")
        
        # Job 1: Hourly world state collection
        scheduler.add_job(
            func=hourly_world_state_collection,
            trigger=IntervalTrigger(hours=1),
            id='world_state_collection',
            name='Hourly World State Collection',
            replace_existing=True,
            max_instances=1
        )
        logger.info("✓ Scheduled: Hourly world state collection")
        
        # Job 2: Weekly story arc generation (Sunday 00:00 UTC)
        scheduler.add_job(
            func=weekly_story_arc_generation,
            trigger=CronTrigger(day_of_week='sun', hour=0, minute=0),
            id='weekly_story_arc',
            name='Weekly Story Arc Generation',
            replace_existing=True,
            max_instances=1
        )
        logger.info("✓ Scheduled: Weekly story arc generation (Sun 00:00)")
        
        # Job 3: Daily chapter advancement check (23:00 UTC)
        scheduler.add_job(
            func=daily_chapter_advancement_check,
            trigger=CronTrigger(hour=23, minute=0),
            id='daily_chapter_check',
            name='Daily Chapter Advancement Check',
            replace_existing=True,
            max_instances=1
        )
        logger.info("✓ Scheduled: Daily chapter check (23:00)")
        
        # Job 4: Weekly hero selection (Sunday 20:00 UTC)
        if getattr(settings, 'hero_recognition_enabled', True):
            scheduler.add_job(
                func=weekly_hero_selection,
                trigger=CronTrigger(day_of_week='sun', hour=20, minute=0),
                id='weekly_hero_selection',
                name='Weekly Hero Selection',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Weekly hero selection (Sun 20:00)")
        
        # Job 5: Midnight arc expiration check (00:00 UTC)
        scheduler.add_job(
            func=midnight_arc_expiration_check,
            trigger=CronTrigger(hour=0, minute=0),
            id='midnight_arc_expiration',
            name='Midnight Arc Expiration Check',
            replace_existing=True,
            max_instances=1
        )
        logger.info("✓ Scheduled: Midnight arc expiration check (00:00)")
        
        # Start scheduler
        scheduler.start()
        logger.info("✓ Storyline scheduler started successfully")
        
        # Log next run times
        jobs = scheduler.get_jobs()
        logger.info(f"Scheduled {len(jobs)} storyline jobs:")
        for job in jobs:
            logger.info(f"  - {job.name}: Next run at {job.next_run_time}")
        
    except Exception as e:
        logger.error(f"Failed to initialize storyline scheduler: {e}", exc_info=True)
        raise


def shutdown_storyline_scheduler():
    """Shutdown storyline scheduler gracefully"""
    try:
        if scheduler.running:
            scheduler.shutdown(wait=True)
            logger.info("Storyline scheduler shut down successfully")
    except Exception as e:
        logger.error(f"Error shutting down storyline scheduler: {e}")


# ============================================================================
# MANUAL TRIGGER FUNCTIONS (for testing or admin use)
# ============================================================================

async def trigger_story_generation_now():
    """Manually trigger story arc generation (admin/testing)"""
    try:
        logger.info("⚡ Manual story arc generation triggered")
        await weekly_story_arc_generation()
    except Exception as e:
        logger.error(f"Manual story generation failed: {e}")
        raise


async def trigger_chapter_advance_now():
    """Manually trigger chapter advancement check (admin/testing)"""
    try:
        logger.info("⚡ Manual chapter advancement triggered")
        await daily_chapter_advancement_check()
    except Exception as e:
        logger.error(f"Manual chapter advancement failed: {e}")
        raise


async def trigger_hero_selection_now():
    """Manually trigger hero selection (admin/testing)"""
    try:
        logger.info("⚡ Manual hero selection triggered")
        await weekly_hero_selection()
    except Exception as e:
        logger.error(f"Manual hero selection failed: {e}")
        raise


# ============================================================================
# EXPORTS
# ============================================================================

__all__ = [
    'scheduler',
    'initialize_storyline_scheduler',
    'shutdown_storyline_scheduler',
    'trigger_story_generation_now',
    'trigger_chapter_advance_now',
    'trigger_hero_selection_now'
]