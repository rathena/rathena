"""
MVP Dynamic Spawn Manager
AI-driven spawn system for MVP/Boss monsters
"""

from loguru import logger
from typing import Dict, Any, List, Optional
from datetime import datetime, timedelta
import random
import asyncio

try:
    from ai_service.models.mvp_behavior import MVPSpawnCondition, MVPBehaviorConfig
    from ai_service.database import db
except ImportError:
    from models.mvp_behavior import MVPSpawnCondition, MVPBehaviorConfig
    from database import db


class MVPSpawnManager:
    """Manages dynamic MVP spawning based on AI conditions"""
    
    def __init__(self):
        self.logger = logger.bind(component="MVPSpawnManager")
        self.spawn_conditions: Dict[str, MVPSpawnCondition] = {}
        self.active_mvps: Dict[str, datetime] = {}
    
    def register_mvp(
        self,
        mvp_config: MVPBehaviorConfig,
        spawn_condition: MVPSpawnCondition
    ):
        """Register an MVP with spawn conditions"""
        self.spawn_conditions[mvp_config.mvp_id] = spawn_condition
        self.logger.info(f"Registered MVP {mvp_config.mvp_name} for dynamic spawning")
    
    async def check_spawn_conditions(
        self,
        mvp_id: str,
        world_state: Dict[str, Any]
    ) -> bool:
        """
        Check if spawn conditions are met for an MVP
        
        Args:
            mvp_id: MVP identifier
            world_state: Current world state
        
        Returns:
            True if MVP should spawn
        """
        condition = self.spawn_conditions.get(mvp_id)
        if not condition:
            return False
        
        # Check if MVP is already spawned
        if mvp_id in self.active_mvps:
            return False
        
        # Check cooldown
        if condition.last_spawn:
            elapsed = datetime.utcnow() - condition.last_spawn
            if elapsed.total_seconds() < condition.min_spawn_interval_minutes * 60:
                return False
        
        # Check time-based conditions
        current_hour = datetime.utcnow().hour
        if condition.min_hour is not None and current_hour < condition.min_hour:
            return False
        if condition.max_hour is not None and current_hour > condition.max_hour:
            return False
        
        # Check day of week
        if condition.days_of_week:
            current_day = datetime.utcnow().weekday()
            if current_day not in condition.days_of_week:
                return False
        
        # Check player count
        players_online = world_state.get("players_online", 0)
        if condition.min_players_online and players_online < condition.min_players_online:
            return False
        if condition.max_players_online and players_online > condition.max_players_online:
            return False
        
        # Check world events
        if condition.required_world_events:
            active_events = world_state.get("active_events", [])
            for required_event in condition.required_world_events:
                if required_event not in active_events:
                    return False
        
        # Check economy state
        if condition.economy_state:
            current_economy = world_state.get("economy_state")
            if current_economy != condition.economy_state:
                return False
        
        # Check spawn probability
        if random.random() > condition.spawn_probability:
            return False
        
        return True
    
    async def spawn_mvp(
        self,
        mvp_id: str,
        mvp_config: MVPBehaviorConfig
    ) -> bool:
        """
        Trigger MVP spawn
        
        Args:
            mvp_id: MVP identifier
            mvp_config: MVP configuration
        
        Returns:
            True if spawn successful
        """
        try:
            self.logger.info(f"Spawning MVP: {mvp_config.mvp_name}")
            
            # Record spawn time
            self.active_mvps[mvp_id] = datetime.utcnow()
            
            # Update spawn condition
            if mvp_id in self.spawn_conditions:
                self.spawn_conditions[mvp_id].last_spawn = datetime.utcnow()
            
            # Store spawn event in database
            spawn_data = {
                "mvp_id": mvp_id,
                "mvp_name": mvp_config.mvp_name,
                "map": mvp_config.spawn_map,
                "x": mvp_config.spawn_x,
                "y": mvp_config.spawn_y,
                "spawned_at": datetime.utcnow().isoformat()
            }
            
            import json
            await db.client.set(
                f"mvp_spawn:{mvp_id}",
                json.dumps(spawn_data),
                ex=mvp_config.spawn_interval_minutes * 60
            )
            
            self.logger.info(
                f"MVP {mvp_config.mvp_name} spawned at "
                f"{mvp_config.spawn_map} ({mvp_config.spawn_x}, {mvp_config.spawn_y})"
            )
            
            return True
            
        except Exception as e:
            self.logger.error(f"Error spawning MVP: {e}", exc_info=True)
            return False
    
    async def despawn_mvp(self, mvp_id: str, reason: str = "defeated"):
        """
        Remove MVP from active list
        
        Args:
            mvp_id: MVP identifier
            reason: Reason for despawn (defeated, timeout, etc.)
        """
        if mvp_id in self.active_mvps:
            spawn_time = self.active_mvps[mvp_id]
            duration = (datetime.utcnow() - spawn_time).total_seconds()
            
            self.logger.info(
                f"MVP {mvp_id} despawned - Reason: {reason}, "
                f"Duration: {duration:.0f}s"
            )
            
            del self.active_mvps[mvp_id]
            
            # Remove from database
            await db.client.delete(f"mvp_spawn:{mvp_id}")
    
    async def run_spawn_cycle(self, world_state: Dict[str, Any]):
        """
        Run one spawn check cycle for all registered MVPs
        
        Args:
            world_state: Current world state
        """
        for mvp_id, condition in self.spawn_conditions.items():
            if await self.check_spawn_conditions(mvp_id, world_state):
                # Get MVP config from database or registry
                # For now, log that spawn conditions are met
                self.logger.info(f"Spawn conditions met for MVP {mvp_id}")
                # In full implementation, this would trigger actual spawn
    
    async def start_spawn_scheduler(self, check_interval_seconds: int = 60):
        """
        Start background task to check spawn conditions periodically
        
        Args:
            check_interval_seconds: How often to check spawn conditions
        """
        self.logger.info(f"Starting MVP spawn scheduler (interval: {check_interval_seconds}s)")
        
        while True:
            try:
                # Get current world state
                world_state = await self._get_world_state()
                
                # Run spawn cycle
                await self.run_spawn_cycle(world_state)
                
                # Wait for next cycle
                await asyncio.sleep(check_interval_seconds)
                
            except Exception as e:
                self.logger.error(f"Error in spawn scheduler: {e}", exc_info=True)
                await asyncio.sleep(check_interval_seconds)
    
    async def _get_world_state(self) -> Dict[str, Any]:
        """Get current world state from database"""
        try:
            world_state_key = "world_state:current"
            world_data = await db.client.get(world_state_key)
            
            if world_data:
                import json
                return json.loads(world_data)
            else:
                # Return default world state
                return {
                    "players_online": 0,
                    "active_events": [],
                    "economy_state": "stable"
                }
        except Exception as e:
            self.logger.error(f"Error getting world state: {e}")
            return {}


# Global instance
_spawn_manager = None

def get_spawn_manager() -> MVPSpawnManager:
    """Get global spawn manager instance"""
    global _spawn_manager
    if _spawn_manager is None:
        _spawn_manager = MVPSpawnManager()
    return _spawn_manager

