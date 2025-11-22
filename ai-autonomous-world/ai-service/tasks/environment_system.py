"""
Environment System

Manages dynamic world environment including weather, time of day, and seasons.
Updates world state periodically and triggers NPC behavior changes.

Features:
- Time of day progression
- Weather transitions
- Seasonal changes
- NPC behavior triggers
- Event generation
"""

import asyncio
import logging
import random
from typing import Dict, Optional, Any
from enum import Enum
from datetime import datetime, timedelta

logger = logging.getLogger(__name__)


class Weather(Enum):
    """Weather conditions."""
    SUNNY = "sunny"
    CLOUDY = "cloudy"
    RAINY = "rainy"
    STORMY = "stormy"
    SNOWY = "snowy"
    FOGGY = "foggy"


class TimeOfDay(Enum):
    """Time of day periods."""
    DAWN = "dawn"
    MORNING = "morning"
    NOON = "noon"
    AFTERNOON = "afternoon"
    DUSK = "dusk"
    EVENING = "evening"
    NIGHT = "night"
    MIDNIGHT = "midnight"


class Season(Enum):
    """Seasons."""
    SPRING = "spring"
    SUMMER = "summer"
    AUTUMN = "autumn"
    WINTER = "winter"


class EnvironmentSystem:
    """
    Manages dynamic world environment.
    
    Updates weather, time of day, and seasonal changes
    to create a living, breathing world.
    """
    
    def __init__(
        self,
        db,
        interval_seconds: int = 600
    ):
        """
        Initialize environment system.
        
        Args:
            db: Database connection
            interval_seconds: Update interval (default: 600s = 10min)
        """
        self.db = db
        self.interval = interval_seconds
        self.running = False
        self.task: Optional[asyncio.Task] = None
        
        # Current state
        self.current_weather = Weather.SUNNY
        self.current_time = TimeOfDay.MORNING
        self.current_season = Season.SPRING
        
        # Weather transition probabilities
        self.weather_transitions = {
            Weather.SUNNY: {
                Weather.SUNNY: 0.7,
                Weather.CLOUDY: 0.25,
                Weather.FOGGY: 0.05
            },
            Weather.CLOUDY: {
                Weather.SUNNY: 0.3,
                Weather.CLOUDY: 0.4,
                Weather.RAINY: 0.25,
                Weather.FOGGY: 0.05
            },
            Weather.RAINY: {
                Weather.CLOUDY: 0.4,
                Weather.RAINY: 0.3,
                Weather.STORMY: 0.2,
                Weather.SUNNY: 0.1
            },
            Weather.STORMY: {
                Weather.RAINY: 0.5,
                Weather.CLOUDY: 0.3,
                Weather.STORMY: 0.2
            },
            Weather.SNOWY: {
                Weather.SNOWY: 0.6,
                Weather.CLOUDY: 0.3,
                Weather.SUNNY: 0.1
            },
            Weather.FOGGY: {
                Weather.FOGGY: 0.4,
                Weather.CLOUDY: 0.4,
                Weather.SUNNY: 0.2
            }
        }
        
        # Metrics
        self.metrics = {
            'updates_processed': 0,
            'weather_changes': 0,
            'time_changes': 0,
            'season_changes': 0,
            'events_generated': 0
        }
    
    async def start(self):
        """Start periodic environment updates."""
        if self.running:
            logger.warning("Environment system already running")
            return
        
        self.running = True
        self.task = asyncio.create_task(self._run_loop())
        logger.info(
            f"Environment system started (interval: {self.interval}s)"
        )
    
    async def stop(self):
        """Stop environment updates."""
        self.running = False
        
        if self.task:
            self.task.cancel()
            try:
                await self.task
            except asyncio.CancelledError:
                pass
        
        logger.info("Environment system stopped")
    
    async def _run_loop(self):
        """Main update loop."""
        while self.running:
            try:
                await self._update_environment()
                await asyncio.sleep(self.interval)
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Environment update error: {e}")
                await asyncio.sleep(60)
    
    async def _update_environment(self):
        """Update all environment aspects."""
        try:
            # Update time of day
            await self._update_time_of_day()
            
            # Update weather
            await self._update_weather()
            
            # Update season (less frequent)
            if random.random() < 0.01:  # 1% chance per update
                await self._update_season()
            
            # Store state in database
            await self._store_state()
            
            # Notify NPCs of changes
            await self._notify_npcs()
            
            self.metrics['updates_processed'] += 1
            
            logger.info(
                f"Environment: {self.current_weather.value}, "
                f"{self.current_time.value}, {self.current_season.value}"
            )
            
        except Exception as e:
            logger.error(f"Failed to update environment: {e}")
            raise
    
    async def _update_time_of_day(self):
        """Update time of day progression."""
        time_cycle = list(TimeOfDay)
        current_index = time_cycle.index(self.current_time)
        next_index = (current_index + 1) % len(time_cycle)
        
        old_time = self.current_time
        self.current_time = time_cycle[next_index]
        
        if old_time != self.current_time:
            self.metrics['time_changes'] += 1
            await self._generate_event('time_change', {
                'old': old_time.value,
                'new': self.current_time.value
            })
    
    async def _update_weather(self):
        """Update weather with weighted transitions."""
        transitions = self.weather_transitions.get(
            self.current_weather,
            {self.current_weather: 1.0}
        )
        
        # Weighted random selection
        weather_choices = list(transitions.keys())
        weights = list(transitions.values())
        
        old_weather = self.current_weather
        self.current_weather = random.choices(weather_choices, weights=weights)[0]
        
        if old_weather != self.current_weather:
            self.metrics['weather_changes'] += 1
            await self._generate_event('weather_change', {
                'old': old_weather.value,
                'new': self.current_weather.value
            })
    
    async def _update_season(self):
        """Update season (rare transition)."""
        seasons = list(Season)
        current_index = seasons.index(self.current_season)
        next_index = (current_index + 1) % len(seasons)
        
        old_season = self.current_season
        self.current_season = seasons[next_index]
        
        if old_season != self.current_season:
            self.metrics['season_changes'] += 1
            await self._generate_event('season_change', {
                'old': old_season.value,
                'new': self.current_season.value
            })
    
    async def _store_state(self):
        """Store current environment state in database."""
        state = {
            'weather': self.current_weather.value,
            'time_of_day': self.current_time.value,
            'season': self.current_season.value,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        # TODO: Store in database
        logger.debug(f"Environment state: {state}")
    
    async def _notify_npcs(self):
        """Notify NPCs of environment changes."""
        # TODO: Send notifications to NPCs to adjust behavior
        # For example: NPCs seek shelter in rain, sleep at night, etc.
        pass
    
    async def _generate_event(self, event_type: str, data: Dict[str, Any]):
        """
        Generate environment event.
        
        Args:
            event_type: Type of event
            data: Event data
        """
        event = {
            'type': event_type,
            'data': data,
            'timestamp': datetime.utcnow().isoformat()
        }
        
        # TODO: Publish to event bus
        logger.info(f"Environment event: {event}")
        self.metrics['events_generated'] += 1
    
    def get_current_state(self) -> Dict[str, str]:
        """
        Get current environment state.
        
        Returns:
            Dict with current state
        """
        return {
            'weather': self.current_weather.value,
            'time_of_day': self.current_time.value,
            'season': self.current_season.value
        }
    
    def set_weather(self, weather: Weather):
        """
        Manually set weather (for testing/admin).
        
        Args:
            weather: Weather to set
        """
        self.current_weather = weather
        logger.info(f"Weather manually set to {weather.value}")
    
    def set_time(self, time: TimeOfDay):
        """
        Manually set time of day (for testing/admin).
        
        Args:
            time: Time to set
        """
        self.current_time = time
        logger.info(f"Time manually set to {time.value}")
    
    def set_season(self, season: Season):
        """
        Manually set season (for testing/admin).
        
        Args:
            season: Season to set
        """
        self.current_season = season
        logger.info(f"Season manually set to {season.value}")
    
    def get_metrics(self) -> Dict[str, int]:
        """
        Get system metrics.
        
        Returns:
            Dict with metrics
        """
        return self.metrics.copy()
    
    def reset_metrics(self):
        """Reset all metrics to zero."""
        self.metrics = {
            'updates_processed': 0,
            'weather_changes': 0,
            'time_changes': 0,
            'season_changes': 0,
            'events_generated': 0
        }