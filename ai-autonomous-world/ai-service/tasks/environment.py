"""
Environment System Tasks - Weather, Time of Day, Seasons, Disasters, Resources
Implements automated environment simulation with dynamic weather, time cycles, and disasters
"""

import asyncio
import random
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional, Tuple
from loguru import logger
import json

from config import settings
from database import db
from models.world import EnvironmentState


class EnvironmentManager:
    """
    Manages environment system including weather, time, seasons, disasters, and resources
    Handles automated environment updates and state management
    """
    
    def __init__(self):
        self.current_state: Optional[EnvironmentState] = None
        self.last_update: Optional[datetime] = None
        self.current_disasters: List[Dict[str, Any]] = []
        self.game_start_time: datetime = datetime.utcnow()
        logger.info("EnvironmentManager initialized")
    
    async def initialize(self):
        """Initialize environment system with default state"""
        try:
            # Try to load existing state
            existing_state = await self._load_state_from_db()
            
            if existing_state:
                self.current_state = existing_state
                logger.info("Loaded existing environment state from database")
            else:
                # Create initial state
                self.current_state = EnvironmentState(
                    weather="clear",
                    time_of_day="day",
                    season="spring",
                    resource_availability=self._initialize_resources(),
                    timestamp=datetime.utcnow()
                )
                await self._save_state_to_db()
                logger.info("Created initial environment state")
            
            self.last_update = datetime.utcnow()
            logger.info(f"Environment initialized: {self.current_state.weather}, {self.current_state.time_of_day}, {self.current_state.season}")
            
        except Exception as e:
            logger.error(f"Error initializing environment: {e}", exc_info=True)
            raise
    
    def _initialize_resources(self) -> Dict[str, float]:
        """Initialize resource availability with default values"""
        resources = {}
        for resource_type in settings.resource_types:
            # Start with random availability between 0.5 and 1.0
            resources[resource_type] = random.uniform(0.5, 1.0)
        return resources
    
    async def update_environment(self):
        """Main environment update cycle - updates all environment systems"""
        try:
            logger.info("=" * 80)
            logger.info("ENVIRONMENT UPDATE CYCLE STARTED")
            logger.info("=" * 80)
            
            if not self.current_state:
                await self.initialize()
            
            # Update time of day
            if settings.time_of_day_enabled:
                await self._update_time_of_day()
            
            # Update weather
            if settings.weather_enabled:
                await self._update_weather()
            
            # Update season
            if settings.season_enabled:
                await self._update_season()
            
            # Check for disasters
            if settings.disaster_enabled:
                await self._check_and_trigger_disasters()
            
            # Update resource availability
            if settings.resource_availability_enabled:
                await self._update_resource_availability()
            
            # Update active disasters
            await self._update_active_disasters()
            
            # Save state
            self.current_state.timestamp = datetime.utcnow()
            await self._save_state_to_db()
            self.last_update = datetime.utcnow()
            
            logger.info(f"Environment updated: Weather={self.current_state.weather}, Time={self.current_state.time_of_day}, Season={self.current_state.season}")
            logger.info(f"Active disasters: {len(self.current_disasters)}")
            logger.info("=" * 80)
            
        except Exception as e:
            logger.error(f"Error updating environment: {e}", exc_info=True)
    
    async def _update_time_of_day(self):
        """Update time of day based on cycle duration"""
        try:
            # Calculate elapsed time since game start
            elapsed_minutes = (datetime.utcnow() - self.game_start_time).total_seconds() / 60
            
            # Calculate position in day cycle
            cycle_duration = settings.time_of_day_cycle_duration
            cycle_position = (elapsed_minutes % cycle_duration) / cycle_duration
            
            # Determine phase based on position
            phases = settings.time_of_day_phases
            phase_index = int(cycle_position * len(phases))
            new_phase = phases[phase_index]
            
            if new_phase != self.current_state.time_of_day:
                old_phase = self.current_state.time_of_day
                self.current_state.time_of_day = new_phase
                logger.info(f"Time of day changed: {old_phase} → {new_phase}")
                
                # Publish time change event
                await self._publish_environment_event("time_of_day_change", {
                    "old_phase": old_phase,
                    "new_phase": new_phase,
                    "cycle_position": cycle_position
                })
            
        except Exception as e:
            logger.error(f"Error updating time of day: {e}", exc_info=True)

    async def _update_weather(self):
        """Update weather based on probability and current conditions"""
        try:
            # Check if weather should change
            if random.random() < settings.weather_change_probability:
                old_weather = self.current_state.weather

                # Get available weather types
                weather_types = settings.weather_types

                # Weight weather transitions based on current weather
                weights = self._get_weather_transition_weights(old_weather, weather_types)

                # Select new weather
                new_weather = random.choices(weather_types, weights=weights)[0]

                if new_weather != old_weather:
                    self.current_state.weather = new_weather
                    logger.info(f"Weather changed: {old_weather} → {new_weather}")

                    # Publish weather change event
                    await self._publish_environment_event("weather_change", {
                        "old_weather": old_weather,
                        "new_weather": new_weather,
                        "season": self.current_state.season
                    })

        except Exception as e:
            logger.error(f"Error updating weather: {e}", exc_info=True)

    def _get_weather_transition_weights(self, current_weather: str, weather_types: List[str]) -> List[float]:
        """Get transition weights for weather changes (realistic transitions)"""
        # Define transition probabilities (current weather influences next weather)
        transitions = {
            "clear": {"clear": 0.4, "sunny": 0.3, "cloudy": 0.2, "rainy": 0.05, "stormy": 0.02, "snowy": 0.02, "foggy": 0.01},
            "sunny": {"sunny": 0.5, "clear": 0.3, "cloudy": 0.15, "rainy": 0.03, "stormy": 0.01, "snowy": 0.01, "foggy": 0.0},
            "cloudy": {"cloudy": 0.3, "clear": 0.2, "rainy": 0.25, "sunny": 0.1, "stormy": 0.1, "foggy": 0.03, "snowy": 0.02},
            "rainy": {"rainy": 0.4, "cloudy": 0.25, "stormy": 0.15, "clear": 0.1, "foggy": 0.05, "sunny": 0.03, "snowy": 0.02},
            "stormy": {"stormy": 0.3, "rainy": 0.35, "cloudy": 0.2, "clear": 0.1, "foggy": 0.03, "sunny": 0.01, "snowy": 0.01},
            "snowy": {"snowy": 0.5, "cloudy": 0.25, "clear": 0.15, "foggy": 0.05, "rainy": 0.03, "sunny": 0.01, "stormy": 0.01},
            "foggy": {"foggy": 0.4, "cloudy": 0.3, "clear": 0.2, "rainy": 0.05, "sunny": 0.03, "stormy": 0.01, "snowy": 0.01}
        }

        # Get weights for current weather
        weather_probs = transitions.get(current_weather, {})

        # Build weight list matching weather_types order
        weights = []
        for weather in weather_types:
            weights.append(weather_probs.get(weather, 0.1))  # Default 0.1 if not defined

        # Normalize weights
        total = sum(weights)
        if total > 0:
            weights = [w / total for w in weights]
        else:
            weights = [1.0 / len(weather_types)] * len(weather_types)

        return weights

    async def _update_season(self):
        """Update season based on elapsed game days"""
        try:
            # Calculate elapsed game days (assuming 1 real day = 1 game day for simplicity)
            # In production, this could be configurable
            elapsed_days = (datetime.utcnow() - self.game_start_time).days

            # Calculate current season index
            season_length = settings.season_length_days
            seasons = settings.season_types
            season_index = (elapsed_days // season_length) % len(seasons)
            new_season = seasons[season_index]

            if new_season != self.current_state.season:
                old_season = self.current_state.season
                self.current_state.season = new_season
                logger.info(f"Season changed: {old_season} → {new_season}")

                # Publish season change event
                await self._publish_environment_event("season_change", {
                    "old_season": old_season,
                    "new_season": new_season,
                    "elapsed_days": elapsed_days
                })

        except Exception as e:
            logger.error(f"Error updating season: {e}", exc_info=True)

    async def _check_and_trigger_disasters(self):
        """Check for and trigger natural disasters"""
        try:
            # Check if disaster should occur
            if random.random() < settings.disaster_probability:
                # Select disaster type
                disaster_type = random.choice(settings.disaster_types)

                # Calculate duration
                duration = random.randint(
                    settings.disaster_duration_min,
                    settings.disaster_duration_max
                )

                # Create disaster
                disaster = {
                    "disaster_id": f"disaster_{int(datetime.utcnow().timestamp())}",
                    "type": disaster_type,
                    "severity": random.uniform(0.3, 1.0),
                    "started_at": datetime.utcnow().isoformat(),
                    "duration": duration,
                    "expires_at": (datetime.utcnow() + timedelta(seconds=duration)).isoformat(),
                    "affected_areas": self._get_affected_areas(disaster_type),
                    "effects": self._get_disaster_effects(disaster_type)
                }

                self.current_disasters.append(disaster)
                logger.warning(f"DISASTER TRIGGERED: {disaster_type} (severity: {disaster['severity']:.2f}, duration: {duration}s)")

                # Publish disaster event
                await self._publish_environment_event("disaster_start", disaster)

                # Store in database
                await db.redis.lpush("disasters:active", json.dumps(disaster))
                await db.redis.expire("disasters:active", 86400)  # 24 hour expiry

        except Exception as e:
            logger.error(f"Error checking disasters: {e}", exc_info=True)

    def _get_affected_areas(self, disaster_type: str) -> List[str]:
        """Get areas affected by disaster type"""
        # This would be map-specific in production
        # For now, return generic areas
        area_counts = {
            "earthquake": 5,
            "flood": 3,
            "drought": 10,
            "plague": 7,
            "wildfire": 4,
            "meteor": 2
        }

        count = area_counts.get(disaster_type, 3)
        return [f"area_{i}" for i in range(count)]

    def _get_disaster_effects(self, disaster_type: str) -> Dict[str, Any]:
        """Get effects of disaster on environment and resources"""
        effects = {
            "earthquake": {
                "resource_damage": {"stone": -0.3, "ore": -0.2},
                "movement_penalty": 0.5,
                "building_damage": 0.4
            },
            "flood": {
                "resource_damage": {"crops": -0.5, "wood": -0.2},
                "movement_penalty": 0.7,
                "health_damage": 0.1
            },
            "drought": {
                "resource_damage": {"crops": -0.7, "herbs": -0.4, "fish": -0.6},
                "health_damage": 0.2
            },
            "plague": {
                "health_damage": 0.8,
                "npc_mortality": 0.1,
                "trade_penalty": 0.5
            },
            "wildfire": {
                "resource_damage": {"wood": -0.8, "herbs": -0.6, "crops": -0.5},
                "movement_penalty": 0.6,
                "health_damage": 0.3
            },
            "meteor": {
                "resource_damage": {"stone": 0.5, "ore": 0.3},  # Positive - adds resources
                "area_damage": 0.9,
                "health_damage": 0.5
            }
        }

        return effects.get(disaster_type, {})

    async def _update_active_disasters(self):
        """Update and expire active disasters"""
        try:
            now = datetime.utcnow()
            expired_disasters = []

            for disaster in self.current_disasters:
                expires_at = datetime.fromisoformat(disaster["expires_at"])

                if now >= expires_at:
                    expired_disasters.append(disaster)
                    logger.info(f"Disaster ended: {disaster['type']} (ID: {disaster['disaster_id']})")

                    # Publish disaster end event
                    await self._publish_environment_event("disaster_end", {
                        "disaster_id": disaster["disaster_id"],
                        "type": disaster["type"],
                        "duration": disaster["duration"]
                    })

            # Remove expired disasters
            for disaster in expired_disasters:
                self.current_disasters.remove(disaster)

        except Exception as e:
            logger.error(f"Error updating active disasters: {e}", exc_info=True)

    async def _update_resource_availability(self):
        """Update resource availability based on regeneration, depletion, and disasters"""
        try:
            if not self.current_state.resource_availability:
                self.current_state.resource_availability = self._initialize_resources()

            # Natural regeneration
            for resource_type in settings.resource_types:
                current = self.current_state.resource_availability.get(resource_type, 0.5)

                # Apply regeneration (capped at 1.0)
                regenerated = min(1.0, current + settings.resource_regeneration_rate)

                # Apply disaster effects
                for disaster in self.current_disasters:
                    effects = disaster.get("effects", {})
                    resource_damage = effects.get("resource_damage", {})

                    if resource_type in resource_damage:
                        damage = resource_damage[resource_type]
                        regenerated = max(0.0, regenerated + damage)

                # Apply seasonal modifiers
                seasonal_modifier = self._get_seasonal_resource_modifier(resource_type, self.current_state.season)
                regenerated = max(0.0, min(1.0, regenerated * seasonal_modifier))

                self.current_state.resource_availability[resource_type] = regenerated

            logger.debug(f"Resource availability updated: {self.current_state.resource_availability}")

        except Exception as e:
            logger.error(f"Error updating resource availability: {e}", exc_info=True)

    def _get_seasonal_resource_modifier(self, resource_type: str, season: str) -> float:
        """Get seasonal modifier for resource availability"""
        modifiers = {
            "spring": {"crops": 1.2, "herbs": 1.3, "wood": 1.1, "fish": 1.0, "stone": 1.0, "ore": 1.0},
            "summer": {"crops": 1.3, "herbs": 1.2, "wood": 1.0, "fish": 1.2, "stone": 1.0, "ore": 1.0},
            "autumn": {"crops": 1.1, "herbs": 0.9, "wood": 1.2, "fish": 1.0, "stone": 1.0, "ore": 1.0},
            "winter": {"crops": 0.5, "herbs": 0.4, "wood": 0.8, "fish": 0.7, "stone": 1.0, "ore": 1.0}
        }

        season_mods = modifiers.get(season, {})
        return season_mods.get(resource_type, 1.0)

    async def _publish_environment_event(self, event_type: str, event_data: Dict[str, Any]):
        """Publish environment event to Redis pub/sub"""
        try:
            event = {
                "event_type": event_type,
                "event_data": event_data,
                "timestamp": datetime.utcnow().isoformat()
            }

            # Publish to environment events channel
            await db.redis.publish("environment:events", json.dumps(event))

            # Store in recent events list
            await db.redis.lpush("environment:recent_events", json.dumps(event))
            await db.redis.ltrim("environment:recent_events", 0, 99)  # Keep last 100 events

            logger.debug(f"Published environment event: {event_type}")

        except Exception as e:
            logger.error(f"Error publishing environment event: {e}", exc_info=True)

    async def _save_state_to_db(self):
        """Save current environment state to database"""
        try:
            state_data = {
                "weather": self.current_state.weather,
                "time_of_day": self.current_state.time_of_day,
                "season": self.current_state.season,
                "resource_availability": self.current_state.resource_availability,
                "timestamp": self.current_state.timestamp.isoformat()
            }

            # Save to Redis
            await db.redis.set("world:state:environment", json.dumps(state_data))

            # Also save to world state (for compatibility with existing code)
            await db.set_world_state("environment", state_data)

            logger.debug("Environment state saved to database")

        except Exception as e:
            logger.error(f"Error saving environment state: {e}", exc_info=True)

    async def _load_state_from_db(self) -> Optional[EnvironmentState]:
        """Load environment state from database"""
        try:
            # Try to load from Redis
            state_json = await db.redis.get("world:state:environment")

            if state_json:
                state_data = json.loads(state_json)
                return EnvironmentState(
                    weather=state_data.get("weather", "clear"),
                    time_of_day=state_data.get("time_of_day", "day"),
                    season=state_data.get("season", "spring"),
                    resource_availability=state_data.get("resource_availability", {}),
                    timestamp=datetime.fromisoformat(state_data.get("timestamp", datetime.utcnow().isoformat()))
                )

            return None

        except Exception as e:
            logger.error(f"Error loading environment state: {e}", exc_info=True)
            return None

    async def get_current_state(self) -> EnvironmentState:
        """Get current environment state"""
        if not self.current_state:
            await self.initialize()
        return self.current_state

    async def get_active_disasters(self) -> List[Dict[str, Any]]:
        """Get list of active disasters"""
        return self.current_disasters.copy()

    async def force_weather_change(self, new_weather: str):
        """Manually change weather (for admin/testing)"""
        if new_weather in settings.weather_types:
            old_weather = self.current_state.weather
            self.current_state.weather = new_weather
            logger.info(f"Weather manually changed: {old_weather} → {new_weather}")
            await self._publish_environment_event("weather_change", {
                "old_weather": old_weather,
                "new_weather": new_weather,
                "manual": True
            })
            await self._save_state_to_db()
        else:
            logger.warning(f"Invalid weather type: {new_weather}")

    async def trigger_disaster(self, disaster_type: str, severity: float = 0.5, duration: int = 600):
        """Manually trigger a disaster (for admin/testing)"""
        if disaster_type in settings.disaster_types:
            disaster = {
                "disaster_id": f"disaster_manual_{int(datetime.utcnow().timestamp())}",
                "type": disaster_type,
                "severity": severity,
                "started_at": datetime.utcnow().isoformat(),
                "duration": duration,
                "expires_at": (datetime.utcnow() + timedelta(seconds=duration)).isoformat(),
                "affected_areas": self._get_affected_areas(disaster_type),
                "effects": self._get_disaster_effects(disaster_type),
                "manual": True
            }

            self.current_disasters.append(disaster)
            logger.warning(f"DISASTER MANUALLY TRIGGERED: {disaster_type} (severity: {severity:.2f}, duration: {duration}s)")

            await self._publish_environment_event("disaster_start", disaster)
            await db.redis.lpush("disasters:active", json.dumps(disaster))
        else:
            logger.warning(f"Invalid disaster type: {disaster_type}")


# Global instance
environment_manager = EnvironmentManager()


# Public functions for scheduler and router
async def update_environment_cycle():
    """Update environment (called by scheduler)"""
    await environment_manager.update_environment()


async def initialize_environment():
    """Initialize environment system"""
    await environment_manager.initialize()


async def get_environment_state() -> EnvironmentState:
    """Get current environment state"""
    return await environment_manager.get_current_state()


async def get_active_disasters() -> List[Dict[str, Any]]:
    """Get active disasters"""
    return await environment_manager.get_active_disasters()


async def force_weather_change(new_weather: str):
    """Force weather change (admin function)"""
    await environment_manager.force_weather_change(new_weather)


async def trigger_disaster(disaster_type: str, severity: float = 0.5, duration: int = 600):
    """Trigger disaster (admin function)"""
    await environment_manager.trigger_disaster(disaster_type, severity, duration)

