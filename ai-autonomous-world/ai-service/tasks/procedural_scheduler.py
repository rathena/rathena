"""
Procedural Content Scheduler
Background tasks for Problem Agent, Dynamic NPC Agent, and World Event Agent
Uses APScheduler for cron-based and interval-based scheduling
"""

import asyncio
from datetime import datetime, UTC
from typing import Optional
from loguru import logger
from apscheduler.schedulers.asyncio import AsyncIOScheduler
from apscheduler.triggers.cron import CronTrigger
from apscheduler.triggers.interval import IntervalTrigger

from config import settings
from agents.procedural.problem_agent import problem_agent
from agents.procedural.dynamic_npc_agent import dynamic_npc_agent
from agents.procedural.world_event_agent import world_event_agent
from agents.progression.faction_agent import faction_agent
from agents.progression.reputation_agent import reputation_agent
from agents.progression.dynamic_boss_agent import dynamic_boss_agent
from agents.environmental.map_hazard_agent import map_hazard_agent
from agents.environmental.weather_time_agent import weather_time_agent
from agents.environmental.treasure_agent import treasure_agent
from agents.economy_social.merchant_economy_agent import merchant_economy_agent
from agents.economy_social.karma_agent import karma_agent
from agents.economy_social.social_interaction_agent import social_interaction_agent
from agents.advanced.adaptive_dungeon_agent import adaptive_dungeon_agent
from agents.advanced.archaeology_agent import archaeology_agent
from agents.advanced.event_chain_agent import event_chain_agent


class ProceduralScheduler:
    """
    Manages background tasks for procedural content generation.
    
    Scheduled Tasks:
    - Daily problem generation at midnight
    - NPC spawning at 6 AM
    - NPC despawning at 11:59 PM
    - Event monitoring every 60 seconds
    - Cleanup tasks for old data
    """
    
    def __init__(self):
        """Initialize scheduler"""
        self.scheduler = AsyncIOScheduler()
        self._running = False
        
        # Task enable flags from config
        self.problem_enabled = getattr(settings, 'problem_agent_enabled', True)
        self.npc_enabled = getattr(settings, 'dynamic_npc_enabled', True)
        self.event_enabled = getattr(settings, 'world_event_enabled', True)
        self.faction_enabled = getattr(settings, 'faction_agent_enabled', True)
        self.reputation_enabled = getattr(settings, 'reputation_agent_enabled', True)
        self.boss_enabled = getattr(settings, 'dynamic_boss_enabled', True)
        self.hazard_enabled = getattr(settings, 'map_hazard_enabled', True)
        self.weather_enabled = getattr(settings, 'weather_time_enabled', True)
        self.treasure_enabled = getattr(settings, 'treasure_agent_enabled', True)
        self.merchant_economy_enabled = getattr(settings, 'merchant_economy_enabled', True)
        self.karma_enabled = getattr(settings, 'karma_agent_enabled', True)
        self.social_enabled = getattr(settings, 'social_agent_enabled', True)
        self.adaptive_dungeon_enabled = getattr(settings, 'adaptive_dungeon_enabled', True)
        self.archaeology_enabled = getattr(settings, 'archaeology_enabled', True)
        self.event_chain_enabled = getattr(settings, 'event_chain_enabled', True)
        
        # Task intervals
        self.event_check_interval = getattr(settings, 'event_check_interval', 60)
        self.faction_update_interval = getattr(settings, 'faction_alignment_update_interval', 300)
        self.boss_spawn_check_interval = getattr(settings, 'boss_spawn_check_interval', 900)
        self.reputation_tier_check_interval = 3600  # 1 hour
        self.weather_update_interval = getattr(settings, 'weather_update_interval_hours', 3) * 3600  # Convert to seconds
        self.time_effects_interval = 600  # 10 minutes
        self.treasure_cleanup_interval = 3600  # 1 hour
        self.economy_analysis_interval = getattr(settings, 'economy_analysis_interval_hours', 6) * 3600  # Convert to seconds
        self.karma_update_interval = getattr(settings, 'karma_update_interval', 900)  # 15 minutes
        self.social_event_spawn_hour = 7  # 7 AM
        self.guild_task_spawn_hour = 4  # 4 AM
        self.karma_decay_hour = 3  # 3 AM
        self.dungeon_generation_hour = 0  # Midnight
        self.dig_spot_spawn_hour = 2  # 2 AM
        self.chain_evaluation_interval = 7200  # 2 hours
        self.chain_advancement_interval = 1800  # 30 minutes
        
        logger.info("Procedural Scheduler initialized")
    
    def start(self):
        """Start all scheduled tasks"""
        if self._running:
            logger.warning("Scheduler already running")
            return
        
        logger.info("Starting procedural content scheduler...")
        
        # ====================================================================
        # PROBLEM AGENT TASKS
        # ====================================================================
        
        if self.problem_enabled:
            # Daily problem generation at midnight
            self.scheduler.add_job(
                self._generate_daily_problem_wrapper,
                trigger=CronTrigger(hour=0, minute=0),
                id='daily_problem_generation',
                name='Generate Daily Problem',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Daily problem generation at 00:00")
        
        # ====================================================================
        # DYNAMIC NPC AGENT TASKS
        # ====================================================================
        
        if self.npc_enabled:
            # Daily NPC spawning at 6 AM
            self.scheduler.add_job(
                self._spawn_daily_npcs_wrapper,
                trigger=CronTrigger(hour=6, minute=0),
                id='daily_npc_spawn',
                name='Spawn Daily NPCs',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Daily NPC spawn at 06:00")
            
            # Daily NPC despawning at 11:59 PM
            self.scheduler.add_job(
                self._despawn_expired_npcs_wrapper,
                trigger=CronTrigger(hour=23, minute=59),
                id='daily_npc_despawn',
                name='Despawn Expired NPCs',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Daily NPC despawn at 23:59")
        
        # ====================================================================
        # WORLD EVENT AGENT TASKS
        # ====================================================================
        
        if self.event_enabled:
            # Event monitoring every 60 seconds
            self.scheduler.add_job(
                self._monitor_events_wrapper,
                trigger=IntervalTrigger(seconds=self.event_check_interval),
                id='event_monitoring',
                name='Monitor World Events',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Event monitoring every {self.event_check_interval}s")
        
        # ====================================================================
        # PROGRESSION AGENT TASKS (Phase 4B)
        # ====================================================================
        
        if self.faction_enabled:
            # Faction alignment update every 5 minutes
            self.scheduler.add_job(
                self._update_faction_alignment_wrapper,
                trigger=IntervalTrigger(seconds=self.faction_update_interval),
                id='faction_alignment_update',
                name='Update Faction Alignment',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Faction alignment update every {self.faction_update_interval}s")
        
        if self.boss_enabled:
            # Boss spawn evaluation every 15 minutes
            self.scheduler.add_job(
                self._evaluate_boss_spawn_wrapper,
                trigger=IntervalTrigger(seconds=self.boss_spawn_check_interval),
                id='boss_spawn_evaluation',
                name='Evaluate Boss Spawn',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Boss spawn evaluation every {self.boss_spawn_check_interval}s")
        
        if self.reputation_enabled:
            # Reputation tier check every hour
            self.scheduler.add_job(
                self._check_reputation_tiers_wrapper,
                trigger=IntervalTrigger(seconds=self.reputation_tier_check_interval),
                id='reputation_tier_check',
                name='Check Reputation Tiers',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Reputation tier check every {self.reputation_tier_check_interval}s")
        
        # ====================================================================
        # ENVIRONMENTAL AGENT TASKS (Phase 4C)
        # ====================================================================
        
        if self.hazard_enabled:
            # Map hazard generation daily at 00:30 (after problem agent)
            self.scheduler.add_job(
                self._generate_daily_hazards_wrapper,
                trigger=CronTrigger(hour=0, minute=30),
                id='daily_hazards',
                name='Generate Daily Hazards',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Daily hazard generation at 00:30")
            
            # Hazard cleanup hourly
            self.scheduler.add_job(
                self._cleanup_hazards_wrapper,
                trigger=IntervalTrigger(hours=1),
                id='hazard_cleanup',
                name='Cleanup Expired Hazards',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Hazard cleanup every 1 hour")
        
        if self.weather_enabled:
            # Weather update every 3 hours
            self.scheduler.add_job(
                self._update_weather_wrapper,
                trigger=IntervalTrigger(seconds=self.weather_update_interval),
                id='weather_update',
                name='Update Weather',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Weather update every {self.weather_update_interval // 3600} hours")
            
            # Time effects update every 10 minutes
            self.scheduler.add_job(
                self._update_time_effects_wrapper,
                trigger=IntervalTrigger(seconds=self.time_effects_interval),
                id='time_effects_update',
                name='Update Time Effects',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Time effects update every {self.time_effects_interval // 60} minutes")
        
        if self.treasure_enabled:
            # Treasure spawn daily at 01:00
            self.scheduler.add_job(
                self._spawn_daily_treasures_wrapper,
                trigger=CronTrigger(hour=1, minute=0),
                id='daily_treasures',
                name='Spawn Daily Treasures',
                replace_existing=True,
                max_instances=1
            )
            logger.info("✓ Scheduled: Daily treasure spawn at 01:00")
            
            # Treasure cleanup every hour
            self.scheduler.add_job(
                self._cleanup_treasures_wrapper,
                trigger=IntervalTrigger(seconds=self.treasure_cleanup_interval),
                id='treasure_cleanup',
                name='Cleanup Expired Treasures',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Treasure cleanup every {self.treasure_cleanup_interval // 3600} hour")
        
        # ====================================================================
        # ECONOMY & SOCIAL AGENT TASKS (Phase 4D)
        # ====================================================================
        
        if self.merchant_economy_enabled:
            # Economy analysis every 6 hours
            self.scheduler.add_job(
                self._analyze_economy_wrapper,
                trigger=IntervalTrigger(seconds=self.economy_analysis_interval),
                id='economy_analysis',
                name='Analyze Economy',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Economy analysis every {self.economy_analysis_interval // 3600} hours")
        
        if self.karma_enabled:
            # Karma update every 15 minutes
            self.scheduler.add_job(
                self._update_karma_wrapper,
                trigger=IntervalTrigger(seconds=self.karma_update_interval),
                id='karma_update',
                name='Update Global Karma',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Karma update every {self.karma_update_interval // 60} minutes")
            
            # Karma decay daily at 3 AM
            self.scheduler.add_job(
                self._apply_karma_decay_wrapper,
                trigger=CronTrigger(hour=self.karma_decay_hour, minute=0),
                id='karma_decay',
                name='Apply Karma Decay',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Karma decay daily at {self.karma_decay_hour:02d}:00")
        
        if self.social_enabled:
            # Social events spawn daily at 7 AM
            self.scheduler.add_job(
                self._spawn_social_events_wrapper,
                trigger=CronTrigger(hour=self.social_event_spawn_hour, minute=0),
                id='daily_social_events',
                name='Spawn Daily Social Events',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Social events spawn daily at {self.social_event_spawn_hour:02d}:00")
            
            # Guild tasks generation daily at 4 AM
            self.scheduler.add_job(
                self._generate_guild_tasks_wrapper,
                trigger=CronTrigger(hour=self.guild_task_spawn_hour, minute=0),
                id='daily_guild_tasks',
                name='Generate Daily Guild Tasks',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Guild tasks generation daily at {self.guild_task_spawn_hour:02d}:00")
        
        # ====================================================================
        # ADVANCED SYSTEMS AGENT TASKS (Phase 4E)
        # ====================================================================
        
        if self.adaptive_dungeon_enabled:
            # Daily dungeon generation at 00:15
            self.scheduler.add_job(
                self._generate_daily_dungeon_wrapper,
                trigger=CronTrigger(hour=self.dungeon_generation_hour, minute=15),
                id='daily_dungeon',
                name='Generate Daily Dungeon',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Daily dungeon generation at {self.dungeon_generation_hour:02d}:15")
        
        if self.archaeology_enabled:
            # Dig spot spawn daily at 02:00
            self.scheduler.add_job(
                self._spawn_dig_spots_wrapper,
                trigger=CronTrigger(hour=self.dig_spot_spawn_hour, minute=0),
                id='daily_dig_spots',
                name='Spawn Daily Dig Spots',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Dig spot spawn daily at {self.dig_spot_spawn_hour:02d}:00")
        
        if self.event_chain_enabled:
            # Event chain evaluation every 2 hours
            self.scheduler.add_job(
                self._evaluate_new_chains_wrapper,
                trigger=IntervalTrigger(seconds=self.chain_evaluation_interval),
                id='chain_evaluation',
                name='Evaluate New Event Chains',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Chain evaluation every {self.chain_evaluation_interval // 3600} hours")
            
            # Chain step advancement every 30 minutes
            self.scheduler.add_job(
                self._check_pending_advancements_wrapper,
                trigger=IntervalTrigger(seconds=self.chain_advancement_interval),
                id='chain_advancement',
                name='Check Pending Chain Advancements',
                replace_existing=True,
                max_instances=1
            )
            logger.info(f"✓ Scheduled: Chain advancement check every {self.chain_advancement_interval // 60} minutes")
        
        # ====================================================================
        # CLEANUP TASKS
        # ====================================================================
        
        # Daily cleanup of old data at 3 AM
        self.scheduler.add_job(
            self._cleanup_old_data_wrapper,
            trigger=CronTrigger(hour=3, minute=0),
            id='daily_cleanup',
            name='Cleanup Old Data',
            replace_existing=True,
            max_instances=1
        )
        logger.info("✓ Scheduled: Daily cleanup at 03:00")
        
        # Start scheduler
        self.scheduler.start()
        self._running = True
        
        logger.info("✓ Procedural content scheduler started")
        logger.info(f"  Total jobs: {len(self.scheduler.get_jobs())}")
    
    def stop(self):
        """Stop all scheduled tasks"""
        if not self._running:
            return
        
        logger.info("Stopping procedural content scheduler...")
        self.scheduler.shutdown(wait=True)
        self._running = False
        logger.info("✓ Scheduler stopped")
    
    def is_running(self) -> bool:
        """Check if scheduler is running"""
        return self._running
    
    def get_jobs(self):
        """Get all scheduled jobs"""
        return self.scheduler.get_jobs()
    
    # ========================================================================
    # TASK WRAPPER METHODS
    # ========================================================================
    
    async def _generate_daily_problem_wrapper(self):
        """Wrapper for daily problem generation task"""
        try:
            logger.info("🕐 Executing scheduled task: Generate Daily Problem")
            
            # Get world state
            world_state = await self._get_world_state()
            
            # Generate problem
            response = await problem_agent.generate_daily_problem(world_state)
            
            if response.success:
                logger.info(f"✓ Daily problem generated: {response.data.get('title')}")
            else:
                logger.error(f"Failed to generate daily problem: {response.reasoning}")
                
        except Exception as e:
            logger.error(f"Error in daily problem generation task: {e}", exc_info=True)
    
    async def _spawn_daily_npcs_wrapper(self):
        """Wrapper for daily NPC spawning task"""
        try:
            logger.info("🕐 Executing scheduled task: Spawn Daily NPCs")
            
            # Get map activity data
            map_activity = await self._get_map_activity()
            
            # Get count from config
            npc_count = getattr(settings, 'daily_npc_count', 3)
            
            # Spawn NPCs
            responses = await dynamic_npc_agent.spawn_daily_npcs(
                map_activity=map_activity,
                count=npc_count
            )
            
            logger.info(f"✓ Spawned {len(responses)} dynamic NPCs")
            
        except Exception as e:
            logger.error(f"Error in NPC spawning task: {e}", exc_info=True)
    
    async def _despawn_expired_npcs_wrapper(self):
        """Wrapper for NPC despawning task"""
        try:
            logger.info("🕐 Executing scheduled task: Despawn Expired NPCs")
            
            count = await dynamic_npc_agent.despawn_expired_npcs()
            
            if count > 0:
                logger.info(f"✓ Despawned {count} expired NPCs")
            else:
                logger.debug("No expired NPCs to despawn")
                
        except Exception as e:
            logger.error(f"Error in NPC despawning task: {e}", exc_info=True)
    
    async def _monitor_events_wrapper(self):
        """Wrapper for event monitoring task"""
        try:
            # Get world state
            world_state = await self._get_world_state()
            
            # Check and trigger events
            response = await world_event_agent.monitor_and_trigger(world_state)
            
            if response and response.success:
                logger.info(f"✓ Event triggered: {response.data.get('title')}")
            
        except Exception as e:
            logger.error(f"Error in event monitoring task: {e}", exc_info=True)
    
    async def _update_faction_alignment_wrapper(self):
        """Wrapper for faction alignment update task"""
        try:
            logger.debug("🕐 Executing scheduled task: Update Faction Alignment")
            
            # Get recent faction actions (last 5 minutes)
            from database import postgres_db
            
            query = """
                SELECT player_id, faction_id, action_type, alignment_change, action_data
                FROM faction_actions
                WHERE timestamp > NOW() - INTERVAL '5 minutes'
            """
            
            actions = await postgres_db.fetch_all(query)
            
            if actions:
                actions_list = [dict(action) for action in actions]
                response = await faction_agent.update_world_alignment(actions_list)
                
                if response.success:
                    logger.debug(f"✓ Faction alignment updated from {len(actions)} actions")
            
        except Exception as e:
            logger.error(f"Error in faction alignment update: {e}", exc_info=True)
    
    async def _evaluate_boss_spawn_wrapper(self):
        """Wrapper for boss spawn evaluation task"""
        try:
            logger.debug("🕐 Executing scheduled task: Evaluate Boss Spawn")
            
            # Get world state
            world_state = await self._get_world_state()
            
            # Evaluate spawn conditions
            spawn_decision = await dynamic_boss_agent.evaluate_spawn_conditions(world_state)
            
            if spawn_decision:
                # Generate and spawn boss
                boss_spec = await dynamic_boss_agent.generate_boss_spec(
                    spawn_reason=spawn_decision['reason'],
                    spawn_map=spawn_decision['map'],
                    difficulty_modifier=spawn_decision.get('difficulty_modifier', 1.0)
                )
                
                response = await dynamic_boss_agent.spawn_boss(boss_spec)
                
                if response.success:
                    logger.info(f"✓ Auto-spawned boss: {boss_spec['boss_name']} in {spawn_decision['map']}")
            
        except Exception as e:
            logger.error(f"Error in boss spawn evaluation: {e}", exc_info=True)
    
    async def _check_reputation_tiers_wrapper(self):
        """Wrapper for reputation tier check task"""
        try:
            logger.debug("🕐 Executing scheduled task: Check Reputation Tiers")
            
            # This is a maintenance task to ensure tier consistency
            # Actual tier checks happen when reputation is gained
            from database import postgres_db
            
            # Update tiers for any players with mismatched tier values
            query = """
                UPDATE player_influence
                SET current_tier = (
                    CASE
                        WHEN total_influence >= 10000 THEN 5
                        WHEN total_influence >= 7000 THEN 4
                        WHEN total_influence >= 5000 THEN 3
                        WHEN total_influence >= 3000 THEN 2
                        WHEN total_influence >= 1000 THEN 1
                        ELSE 0
                    END
                )
                WHERE current_tier != (
                    CASE
                        WHEN total_influence >= 10000 THEN 5
                        WHEN total_influence >= 7000 THEN 4
                        WHEN total_influence >= 5000 THEN 3
                        WHEN total_influence >= 3000 THEN 2
                        WHEN total_influence >= 1000 THEN 1
                        ELSE 0
                    END
                )
            """
            
            rows_updated = await postgres_db.execute(query)
            
            if rows_updated > 0:
                logger.info(f"✓ Updated {rows_updated} player tier values")
            
        except Exception as e:
            logger.error(f"Error in reputation tier check: {e}", exc_info=True)
    
    async def _generate_daily_hazards_wrapper(self):
        """Wrapper for daily hazard generation task"""
        try:
            logger.info("🕐 Executing scheduled task: Generate Daily Hazards")
            
            # Get map activity and active problems
            map_activity = await self._get_map_activity()
            
            from database import postgres_db
            query = """
                SELECT problem_id, map_name, problem_type
                FROM world_problems
                WHERE status = 'active'
            """
            problems = await postgres_db.fetch_all(query)
            active_problems = [dict(p) for p in problems]
            
            # Generate hazards
            response = await map_hazard_agent.generate_daily_hazards(
                map_activity=map_activity,
                active_problems=active_problems
            )
            
            if response.success:
                logger.info(f"✓ Generated {response.data['hazards_generated']} daily hazards")
            
        except Exception as e:
            logger.error(f"Error in hazard generation task: {e}", exc_info=True)
    
    async def _cleanup_hazards_wrapper(self):
        """Wrapper for hazard cleanup task"""
        try:
            count = await map_hazard_agent.remove_expired_hazards()
            if count > 0:
                logger.info(f"✓ Cleaned up {count} expired hazards")
        except Exception as e:
            logger.error(f"Error in hazard cleanup: {e}", exc_info=True)
    
    async def _update_weather_wrapper(self):
        """Wrapper for weather update task"""
        try:
            logger.info("🕐 Executing scheduled task: Update Weather")
            
            response = await weather_time_agent.update_weather()
            
            if response.success:
                logger.info(f"✓ Weather updated to {response.data['weather_type']}")
            
        except Exception as e:
            logger.error(f"Error in weather update: {e}", exc_info=True)
    
    async def _update_time_effects_wrapper(self):
        """Wrapper for time effects update task"""
        try:
            # Silent task - just ensures time effects are current
            await weather_time_agent.get_time_of_day_effects()
        except Exception as e:
            logger.error(f"Error in time effects update: {e}", exc_info=True)
    
    async def _spawn_daily_treasures_wrapper(self):
        """Wrapper for daily treasure spawning task"""
        try:
            logger.info("🕐 Executing scheduled task: Spawn Daily Treasures")
            
            # Get map activity
            map_activity = await self._get_map_activity()
            
            # Spawn treasures
            response = await treasure_agent.spawn_daily_treasures(map_activity=map_activity)
            
            if response.success:
                logger.info(f"✓ Spawned {response.data['treasures_spawned']} daily treasures")
            
        except Exception as e:
            logger.error(f"Error in treasure spawning task: {e}", exc_info=True)
    
    async def _cleanup_treasures_wrapper(self):
        """Wrapper for treasure cleanup task"""
        try:
            count = await treasure_agent.despawn_expired_treasures()
            if count > 0:
                logger.info(f"✓ Cleaned up {count} expired treasures")
        except Exception as e:
            logger.error(f"Error in treasure cleanup: {e}", exc_info=True)
    
    async def _analyze_economy_wrapper(self):
        """Wrapper for economy analysis task"""
        try:
            logger.info("🕐 Executing scheduled task: Analyze Economy")
            
            # Get economy data
            world_state = await self._get_world_state()
            economy_data = {
                'total_zeny': world_state.get('economy', {}).get('zeny_circulation', 500000000),
                'active_players': world_state.get('online_players', 50),
                'item_supply_index': 1.0,
                'item_demand_index': 1.0
            }
            
            # Analyze economic health
            indicator = await merchant_economy_agent.analyze_economic_health(economy_data)
            
            logger.info(f"✓ Economy analyzed: {indicator.value}")
            
            # Auto-adjust prices if needed
            if indicator != 'balanced':
                response = await merchant_economy_agent.adjust_merchant_prices(
                    indicator=indicator,
                    affected_items=None
                )
                
                if response.success:
                    logger.info(f"✓ Auto-adjusted prices for {indicator.value}")
                
                # Create zeny sink if severe inflation
                if indicator.value == 'inflation':
                    severity = min(1.0, economy_data['total_zeny'] / (economy_data['active_players'] * 10000000))
                    await merchant_economy_agent.create_zeny_sink_event(severity)
                    logger.info(f"✓ Created zeny sink event (severity: {severity:.2f})")
            
        except Exception as e:
            logger.error(f"Error in economy analysis task: {e}", exc_info=True)
    
    async def _update_karma_wrapper(self):
        """Wrapper for karma update task"""
        try:
            logger.debug("🕐 Executing scheduled task: Update Global Karma")
            
            # Get recent karma actions (last 15 minutes)
            from database import postgres_db
            
            query = """
                SELECT player_id, action_type, action_data
                FROM karma_actions
                WHERE timestamp > NOW() - INTERVAL '15 minutes'
                AND action_id NOT IN (
                    SELECT DISTINCT action_id
                    FROM karma_actions
                    WHERE timestamp <= NOW() - INTERVAL '15 minutes'
                )
            """
            
            actions = await postgres_db.fetch_all(query)
            
            if actions:
                actions_list = [
                    {
                        'player_id': action['player_id'],
                        'action_type': action['action_type'],
                        'data': action.get('action_data')
                    }
                    for action in actions
                ]
                
                response = await karma_agent.update_global_karma(actions_list)
                
                if response.success:
                    logger.debug(f"✓ Karma updated from {len(actions)} actions")
            
        except Exception as e:
            logger.error(f"Error in karma update task: {e}", exc_info=True)
    
    async def _apply_karma_decay_wrapper(self):
        """Wrapper for karma decay task"""
        try:
            logger.info("🕐 Executing scheduled task: Apply Karma Decay")
            
            affected_count = await karma_agent.apply_daily_decay()
            
            logger.info(f"✓ Karma decay applied to {affected_count} players")
            
        except Exception as e:
            logger.error(f"Error in karma decay task: {e}", exc_info=True)
    
    async def _spawn_social_events_wrapper(self):
        """Wrapper for social events spawning task"""
        try:
            logger.info("🕐 Executing scheduled task: Spawn Daily Social Events")
            
            # Get player distribution
            map_activity = await self._get_map_activity()
            
            # Get social event count from config
            event_count = getattr(settings, 'daily_social_event_count', 5)
            
            # Spawn events
            responses = await social_interaction_agent.spawn_daily_social_events(
                player_distribution=map_activity,
                count=event_count
            )
            
            logger.info(f"✓ Spawned {len(responses)} social events")
            
        except Exception as e:
            logger.error(f"Error in social events spawning task: {e}", exc_info=True)
    
    async def _generate_guild_tasks_wrapper(self):
        """Wrapper for guild tasks generation task"""
        try:
            logger.info("🕐 Executing scheduled task: Generate Daily Guild Tasks")
            
            # Get guild count (would query actual guild data in production)
            from database import postgres_db
            
            # For now, assume 10 guilds
            guild_count = 10
            average_size = 12
            
            # Generate tasks
            tasks = await social_interaction_agent.generate_guild_tasks(
                guild_count=guild_count,
                average_guild_size=average_size
            )
            
            logger.info(f"✓ Generated {len(tasks)} guild tasks")
            
        except Exception as e:
            logger.error(f"Error in guild tasks generation task: {e}", exc_info=True)
    async def _generate_daily_dungeon_wrapper(self):
        """Wrapper for daily dungeon generation task"""
        try:
            logger.info("🕐 Executing scheduled task: Generate Daily Dungeon")
            
            # Get world state
            world_state = await self._get_world_state()
            
            # Get active factions
            faction_alignment = await faction_agent.get_world_alignment()
            active_factions = [
                f['faction_id'] for f in faction_alignment.get('factions', [])
                if f.get('is_dominant', False)
            ]
            
            # Generate dungeon
            response = await adaptive_dungeon_agent.generate_daily_dungeon(
                player_average_level=world_state.get('avg_player_level', 50),
                active_factions=active_factions
            )
            
            if response.success:
                logger.info(f"✓ Daily dungeon generated: {response.data['theme']}, {response.data['floor_count']} floors")
            else:
                logger.error(f"Failed to generate daily dungeon: {response.reasoning}")
            
        except Exception as e:
            logger.error(f"Error in daily dungeon generation task: {e}", exc_info=True)
    
    async def _spawn_dig_spots_wrapper(self):
        """Wrapper for dig spot spawning task"""
        try:
            logger.info("🕐 Executing scheduled task: Spawn Daily Dig Spots")
            
            # Get map activity
            map_activity = await self._get_map_activity()
            
            # Get count from config
            spot_count = getattr(settings, 'daily_dig_spot_count', 15)
            
            # Spawn dig spots
            spots = await archaeology_agent.spawn_dig_spots(
                map_activity=map_activity,
                count=spot_count
            )
            
            logger.info(f"✓ Spawned {len(spots)} dig spots")
            
        except Exception as e:
            logger.error(f"Error in dig spot spawning task: {e}", exc_info=True)
    
    async def _evaluate_new_chains_wrapper(self):
        """Wrapper for event chain evaluation task"""
        try:
            logger.debug("🕐 Executing scheduled task: Evaluate New Event Chains")
            
            # Check if we should start new chains
            from database import postgres_db
            import random
            
            # Get current chain count
            query = """
                SELECT COUNT(*) as count
                FROM event_chains
                WHERE status = 'active'
            """
            result = await postgres_db.fetch_one(query)
            current_chains = result['count'] if result else 0
            
            max_chains = getattr(settings, 'max_concurrent_chains', 3)
            
            if current_chains < max_chains:
                # Evaluate if conditions are met for new chain
                world_state = await self._get_world_state()
                
                # Simple trigger logic - 10% chance per check
                if world_state.get('online_players', 0) > 5 and random.random() < 0.1:
                    # Select random chain type
                    from agents.advanced.event_chain_agent import ChainEventType
                    
                    chain_type = random.choice(list(ChainEventType))
                    
                    response = await event_chain_agent.start_event_chain(
                        chain_type=chain_type,
                        trigger_condition={"auto_trigger": True, "world_state": world_state}
                    )
                    
                    if response.success:
                        logger.info(f"✓ Auto-started event chain: {chain_type.value}")
            
        except Exception as e:
            logger.error(f"Error in chain evaluation task: {e}", exc_info=True)
    
    async def _check_pending_advancements_wrapper(self):
        """Wrapper for chain step advancement check task"""
        try:
            logger.debug("🕐 Executing scheduled task: Check Pending Chain Advancements")
            
            # This is a maintenance task - actual advancement happens when players complete steps
            # We just check for any chains that need automatic advancement (timeout, etc.)
            from database import postgres_db
            
            # Find steps that have timed out
            timeout_hours = getattr(settings, 'chain_step_timeout_hours', 24)
            
            query = """
                SELECT cs.chain_id, cs.step_number
                FROM chain_steps cs
                JOIN event_chains ec ON ec.chain_id = cs.chain_id
                WHERE ec.status = 'active'
                AND cs.completed = FALSE
                AND cs.step_number = ec.current_step
                AND (NOW() - ec.started_at) > ($1 || ' hours')::INTERVAL
            """
            
            timed_out_steps = await postgres_db.fetch_all(query, timeout_hours)
            
            for step in timed_out_steps:
                # Auto-advance with failure outcome
                try:
                    response = await event_chain_agent.advance_chain_step(
                        chain_id=step['chain_id'],
                        outcome='failure'
                    )
                    
                    if response.success:
                        logger.info(f"✓ Auto-advanced timed out chain {step['chain_id']}")
                except Exception as adv_err:
                    logger.error(f"Failed to auto-advance chain {step['chain_id']}: {adv_err}")
            
        except Exception as e:
            logger.error(f"Error in chain advancement check: {e}", exc_info=True)
    
    
    async def _cleanup_old_data_wrapper(self):
        """Wrapper for cleanup task"""
        try:
            logger.info("🕐 Executing scheduled task: Cleanup Old Data")
            
            from database import postgres_db
            
            # Clean up old problem history (30 days)
            await postgres_db.execute("SELECT cleanup_old_problem_history()")
            
            # Clean up old NPC interactions (30 days)
            await postgres_db.execute("SELECT cleanup_old_npc_interactions()")
            
            # Clean up old event participation (30 days)
            await postgres_db.execute("SELECT cleanup_old_event_participation()")
            
            # Clean up old faction actions (30 days)
            await postgres_db.execute("SELECT cleanup_old_faction_actions()")
            
            # Clean up old reputation history (90 days)
            await postgres_db.execute("SELECT cleanup_old_reputation_history()")
            
            # Clean up old boss history (60 days)
            await postgres_db.execute("SELECT cleanup_old_boss_history()")
            
            # Clean up old hazard encounters (30 days) - Phase 4C
            await postgres_db.execute("SELECT cleanup_old_hazard_encounters()")
            
            # Clean up old weather history (90 days) - Phase 4C
            await postgres_db.execute("SELECT cleanup_old_weather_history()")
            
            # Clean up old economy snapshots (90 days) - Phase 4D
            await postgres_db.execute("SELECT cleanup_old_economy_snapshots()")
            
            # Clean up old price adjustments (90 days) - Phase 4D
            await postgres_db.execute("SELECT cleanup_old_price_adjustments()")
            
            # Clean up old karma actions (30 days) - Phase 4D
            await postgres_db.execute("SELECT cleanup_old_karma_actions()")
            
            # Clean up old social participation (30 days) - Phase 4D
            await postgres_db.execute("SELECT cleanup_old_social_participation()")
            
            # Expire old social events - Phase 4D
            await postgres_db.execute("SELECT cleanup_expired_social_events()")
            
            # Expire old guild tasks - Phase 4D
            query = """
                UPDATE guild_tasks
                SET status = 'expired'
                WHERE status = 'active' AND expires_at < NOW()
            """
            await postgres_db.execute(query)
            
            # Clean up old dungeons - Phase 4E
            query = """
                UPDATE adaptive_dungeons
                SET status = 'expired'
                WHERE status = 'active' AND expires_at < NOW()
            """
            await postgres_db.execute(query)
            
            # Clean up old dig spots - Phase 4E
            query = """
                UPDATE dig_spots
                SET status = 'expired'
                WHERE status = 'active' AND expires_at < NOW()
            """
            await postgres_db.execute(query)
            
            # Clean up old event chains - Phase 4E
            query = """
                UPDATE event_chains
                SET status = 'expired'
                WHERE status = 'active' AND expires_at < NOW()
            """
            await postgres_db.execute(query)
            
            logger.info("✓ Cleanup completed")
            
        except Exception as e:
            logger.error(f"Error in cleanup task: {e}", exc_info=True)
    
    # ========================================================================
    # HELPER METHODS
    # ========================================================================
    
    async def _get_world_state(self) -> dict:
        """Get current world state for agents"""
        try:
            from database import db, postgres_db
            
            # Try to get from cache first
            cached_state = await db.get("world:state:current")
            if cached_state:
                return cached_state
            
            # Build world state from database
            # In production, this would query actual game server metrics
            world_state = {
                "avg_player_level": 50,
                "online_players": 10,
                "map_activity": {
                    "prontera": 20,
                    "geffen": 8,
                    "payon": 5,
                    "morocc": 3,
                    "prt_fild01": 2
                },
                "monster_kills": {
                    "prt_fild01": 100,
                    "prt_fild02": 80
                },
                "mvp_kills": {
                    "1038": 5,  # Osiris
                    "1039": 3   # Baphomet
                },
                "economy": {
                    "zeny_circulation": 500000000,
                    "inflation_rate": 1.05,
                    "scarce_items": ["Old Card Album", "Angelic Ring"]
                }
            }
            
            # Cache for 5 minutes
            await db.set("world:state:current", world_state, expire=300)
            
            return world_state
            
        except Exception as e:
            logger.warning(f"Failed to get world state, using defaults: {e}")
            return {
                "avg_player_level": 50,
                "online_players": 0,
                "map_activity": {},
                "monster_kills": {},
                "mvp_kills": {},
                "economy": {}
            }
    
    async def _get_map_activity(self) -> dict:
        """Get map activity data for NPC spawning"""
        world_state = await self._get_world_state()
        return world_state.get('map_activity', {})


# Global scheduler instance
procedural_scheduler = ProceduralScheduler()


# Utility functions for external use
def start_procedural_scheduler():
    """Start the procedural content scheduler"""
    procedural_scheduler.start()


def stop_procedural_scheduler():
    """Stop the procedural content scheduler"""
    procedural_scheduler.stop()


def get_scheduler_status():
    """Get scheduler status and job list"""
    return {
        "running": procedural_scheduler.is_running(),
        "jobs": [
            {
                "id": job.id,
                "name": job.name,
                "next_run_time": job.next_run_time.isoformat() if job.next_run_time else None
            }
            for job in procedural_scheduler.get_jobs()
        ]
    }