"""
Background Task Scheduler for Autonomous Features
Supports configurable modes: disabled, fixed_interval, event_driven
"""

from apscheduler.schedulers.asyncio import AsyncIOScheduler
from apscheduler.triggers.interval import IntervalTrigger
from apscheduler.triggers.cron import CronTrigger
from loguru import logger
from typing import Optional
import asyncio

from config import settings


class AutonomousScheduler:
    """
    Manages background tasks for autonomous features
    Supports flexible configuration for each feature
    """
    
    def __init__(self):
        self.scheduler = AsyncIOScheduler()
        self.is_running = False
        
        # Track registered jobs
        self.jobs = {}
        
        logger.info("AutonomousScheduler initialized")
    
    def start(self):
        """Start the background task scheduler"""
        if self.is_running:
            logger.warning("Scheduler already running")
            return
        
        logger.info("=" * 80)
        logger.info("STARTING AUTONOMOUS TASK SCHEDULER")
        logger.info("=" * 80)
        
        # Register NPC Movement tasks
        self._register_npc_movement_tasks()
        
        # Register Economic Simulation tasks
        self._register_economy_tasks()
        
        # Register Shop Restocking tasks
        self._register_shop_restock_tasks()
        
        # Register Faction System tasks
        self._register_faction_tasks()

        # Register Environment System tasks
        self._register_environment_tasks()

        # Start the scheduler
        self.scheduler.start()
        self.is_running = True
        
        logger.info("=" * 80)
        logger.info(f"Scheduler started with {len(self.jobs)} active jobs")
        logger.info("=" * 80)
    
    def stop(self):
        """Stop the background task scheduler"""
        if not self.is_running:
            logger.warning("Scheduler not running")
            return
        
        logger.info("Stopping autonomous task scheduler...")
        self.scheduler.shutdown(wait=True)
        self.is_running = False
        logger.info("Scheduler stopped")
    
    def _register_npc_movement_tasks(self):
        """Register NPC movement background tasks based on configuration"""
        if not settings.npc_movement_enabled:
            logger.info("NPC Movement: DISABLED (skipping scheduler registration)")
            return
        
        mode = settings.npc_movement_mode
        
        if mode == "disabled":
            logger.info("NPC Movement: Mode=disabled (skipping scheduler registration)")
        
        elif mode == "fixed_interval":
            # Fixed interval mode - periodic updates
            interval = settings.npc_movement_update_interval
            logger.info(f"NPC Movement: Mode=fixed_interval, Interval={interval}s")
            
            job = self.scheduler.add_job(
                self._update_npc_movements,
                trigger=IntervalTrigger(seconds=interval),
                id="npc_movement_fixed",
                name="NPC Movement (Fixed Interval)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["npc_movement"] = job
            logger.info(f"  ✓ Registered job: {job.name}")
        
        elif mode == "event_driven":
            # Event-driven mode - check for idle NPCs periodically
            # But actual movement decisions are made by agents on events
            idle_check_interval = max(settings.npc_movement_idle_timeout // 2, 30)  # Check at half the idle timeout, min 30s
            logger.info(f"NPC Movement: Mode=event_driven, Idle Check Interval={idle_check_interval}s")
            
            job = self.scheduler.add_job(
                self._check_idle_npcs,
                trigger=IntervalTrigger(seconds=idle_check_interval),
                id="npc_movement_idle_check",
                name="NPC Movement (Idle Check)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["npc_movement_idle"] = job
            logger.info(f"  ✓ Registered job: {job.name}")
            logger.info(f"  ℹ Event-driven triggers: player interaction, world events, idle timeout")
        
        else:
            logger.error(f"Invalid npc_movement_mode: {mode}")
    
    def _register_economy_tasks(self):
        """Register economic simulation tasks based on configuration"""
        if not settings.economy_enabled:
            logger.info("Economic Simulation: DISABLED (skipping scheduler registration)")
            return
        
        mode = settings.economy_update_mode
        
        if mode == "disabled":
            logger.info("Economic Simulation: Mode=disabled (skipping scheduler registration)")
        
        elif mode == "daily":
            # Daily update at midnight (server time)
            logger.info(f"Economic Simulation: Mode=daily (updates at midnight)")
            
            job = self.scheduler.add_job(
                self._update_economy,
                trigger=CronTrigger(hour=0, minute=0),  # Daily at midnight
                id="economy_daily",
                name="Economic Simulation (Daily)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["economy_daily"] = job
            logger.info(f"  ✓ Registered job: {job.name}")

        elif mode == "fixed_interval":
            # Fixed interval mode
            interval = settings.economy_update_interval
            logger.info(f"Economic Simulation: Mode=fixed_interval, Interval={interval}s")

            job = self.scheduler.add_job(
                self._update_economy,
                trigger=IntervalTrigger(seconds=interval),
                id="economy_fixed",
                name="Economic Simulation (Fixed Interval)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["economy_fixed"] = job
            logger.info(f"  ✓ Registered job: {job.name}")

        elif mode == "event_driven":
            # Event-driven mode - economy updates triggered by player trades
            logger.info(f"Economic Simulation: Mode=event_driven")
            logger.info(f"  ℹ Updates triggered by: player trades, market events, admin commands")
            # No scheduler job needed - handled by event triggers

        else:
            logger.error(f"Invalid economy_update_mode: {mode}")

    def _register_shop_restock_tasks(self):
        """Register shop restocking tasks based on configuration"""
        if not settings.shop_restock_enabled:
            logger.info("Shop Restocking: DISABLED (skipping scheduler registration)")
            return

        mode = settings.shop_restock_mode

        if mode == "disabled":
            logger.info("Shop Restocking: Mode=disabled (skipping scheduler registration)")

        elif mode == "npc_driven":
            # NPC-driven mode - shop owner NPCs decide when to restock
            check_interval = settings.shop_restock_check_interval
            logger.info(f"Shop Restocking: Mode=npc_driven, Check Interval={check_interval}s")

            job = self.scheduler.add_job(
                self._check_shop_restocking,
                trigger=IntervalTrigger(seconds=check_interval),
                id="shop_restock_npc_check",
                name="Shop Restocking (NPC Decision Check)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["shop_restock_npc"] = job
            logger.info(f"  ✓ Registered job: {job.name}")
            logger.info(f"  ℹ Shop owner NPCs decide WHEN/WHAT/HOW MUCH to restock")

        elif mode == "fixed_interval":
            # Fixed interval mode - automatic restocking
            interval = settings.shop_default_restock_interval
            logger.info(f"Shop Restocking: Mode=fixed_interval, Interval={interval}s")

            job = self.scheduler.add_job(
                self._restock_all_shops,
                trigger=IntervalTrigger(seconds=interval),
                id="shop_restock_fixed",
                name="Shop Restocking (Fixed Interval)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["shop_restock_fixed"] = job
            logger.info(f"  ✓ Registered job: {job.name}")

        else:
            logger.error(f"Invalid shop_restock_mode: {mode}")

    def _register_faction_tasks(self):
        """Register faction system tasks based on configuration"""
        if not settings.faction_enabled:
            logger.info("Faction System: DISABLED (skipping scheduler registration)")
            return

        if settings.faction_reputation_decay_enabled:
            interval = settings.faction_reputation_decay_interval
            logger.info(f"Faction System: Reputation Decay Enabled, Interval={interval}s")

            job = self.scheduler.add_job(
                self._update_faction_reputation,
                trigger=IntervalTrigger(seconds=interval),
                id="faction_reputation_decay",
                name="Faction Reputation Decay",
                max_instances=1,
                coalesce=True
            )
            self.jobs["faction_reputation"] = job
            logger.info(f"  ✓ Registered job: {job.name}")
        else:
            logger.info("Faction System: Reputation Decay DISABLED")

    def _register_environment_tasks(self):
        """Register environment system tasks based on configuration"""
        if not settings.environment_enabled:
            logger.info("Environment System: DISABLED (skipping scheduler registration)")
            return

        mode = settings.environment_update_mode

        if mode == "disabled":
            logger.info("Environment System: Mode=disabled (skipping scheduler registration)")

        elif mode == "fixed_interval":
            # Fixed interval mode - periodic environment updates
            interval = settings.environment_update_interval
            logger.info(f"Environment System: Mode=fixed_interval, Interval={interval}s")

            job = self.scheduler.add_job(
                self._update_environment,
                trigger=IntervalTrigger(seconds=interval),
                id="environment_update",
                name="Environment System (Fixed Interval)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["environment"] = job
            logger.info(f"  ✓ Registered job: {job.name}")
            logger.info(f"  ℹ Updates: weather, time of day, seasons, disasters, resources")

        elif mode == "real_time":
            # Real-time mode - continuous updates (very frequent)
            logger.info(f"Environment System: Mode=real_time (continuous updates)")

            # Use shorter interval for real-time feel (e.g., every 30 seconds)
            interval = 30

            job = self.scheduler.add_job(
                self._update_environment,
                trigger=IntervalTrigger(seconds=interval),
                id="environment_realtime",
                name="Environment System (Real-Time)",
                max_instances=1,
                coalesce=True
            )
            self.jobs["environment_realtime"] = job
            logger.info(f"  ✓ Registered job: {job.name} (interval: {interval}s)")

        else:
            logger.error(f"Invalid environment_update_mode: {mode}")

    # ========================================================================
    # Task Implementation Methods (to be implemented in tasks/ modules)
    # ========================================================================

    async def _update_npc_movements(self):
        """Fixed interval NPC movement updates"""
        try:
            from tasks.npc_movement import update_all_npc_movements_fixed
            await update_all_npc_movements_fixed()
        except ImportError:
            logger.warning("NPC movement task not implemented yet")
        except Exception as e:
            logger.error(f"Error in NPC movement update: {e}")

    async def _check_idle_npcs(self):
        """Check for idle NPCs and trigger movement decisions"""
        try:
            from tasks.npc_movement import check_and_update_idle_npcs
            await check_and_update_idle_npcs()
        except ImportError:
            logger.warning("Idle NPC check task not implemented yet")
        except Exception as e:
            logger.error(f"Error checking idle NPCs: {e}")

    async def _update_economy(self):
        """Update economic simulation"""
        try:
            from tasks.economy import update_economic_simulation
            await update_economic_simulation()
        except ImportError:
            logger.warning("Economy update task not implemented yet")
        except Exception as e:
            logger.error(f"Error in economy update: {e}")

    async def _check_shop_restocking(self):
        """Check if shop owner NPCs want to restock"""
        try:
            from tasks.shop_restock import check_npc_driven_restocking
            await check_npc_driven_restocking()
        except ImportError:
            logger.warning("Shop restock check task not implemented yet")
        except Exception as e:
            logger.error(f"Error checking shop restocking: {e}")

    async def _restock_all_shops(self):
        """Automatically restock all shops (fixed interval mode)"""
        try:
            from tasks.shop_restock import restock_all_shops_fixed
            await restock_all_shops_fixed()
        except ImportError:
            logger.warning("Shop restock task not implemented yet")
        except Exception as e:
            logger.error(f"Error restocking shops: {e}")

    async def _update_faction_reputation(self):
        """Update faction reputation decay"""
        try:
            from tasks.faction import process_reputation_decay
            await process_reputation_decay()
        except ImportError:
            logger.warning("Faction reputation task not implemented yet")
        except Exception as e:
            logger.error(f"Error updating faction reputation: {e}", exc_info=True)

    async def _update_environment(self):
        """Update environment system (weather, time, seasons, disasters, resources)"""
        try:
            from tasks.environment import update_environment_cycle
            await update_environment_cycle()
        except ImportError:
            logger.warning("Environment update task not implemented yet")
        except Exception as e:
            logger.error(f"Error updating environment: {e}", exc_info=True)


# Global scheduler instance
autonomous_scheduler = AutonomousScheduler()


