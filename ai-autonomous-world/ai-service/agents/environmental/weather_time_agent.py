"""
Weather/Time Agent - Dynamic Weather and Time-of-Day System

Manages weather patterns and time-of-day effects that affect combat stats
and create immersion. Updates weather every 2-6 hours, tracks time continuously.

4-Tier LLM Optimization:
- Tier 1 (40%): Rule-based weather transitions
- Tier 2 (30%): Cached weather effects
- Tier 3 (20%): Batched time calculations
- Tier 4 (10%): LLM for weather descriptions
"""

import asyncio
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class WeatherType(Enum):
    """Types of weather conditions."""
    CLEAR = "clear"
    RAIN = "rain"  # Water +20%, Fire -20%
    STORM = "storm"  # Lightning skills stronger
    SNOW = "snow"  # Ice skills stronger
    HEAT_WAVE = "heat_wave"  # Fire +30%, SP regen -20%
    SANDSTORM = "sandstorm"  # Earth skills stronger, accuracy -10%
    AURORA = "aurora"  # Magic damage +15%


class TimeOfDay(Enum):
    """Time-of-day periods."""
    DAWN = "dawn"  # 05:00-07:00
    DAY = "day"  # 07:00-18:00
    DUSK = "dusk"  # 18:00-20:00
    NIGHT = "night"  # 20:00-05:00
    FULL_MOON = "full_moon"  # Special night (random)


class WeatherTimeAgent(BaseAIAgent):
    """
    Manages dynamic weather and time-of-day effects.
    
    Weather Effects:
    - Element damage modifiers
    - Stat changes (SP regen, accuracy, etc.)
    - Monster behavior changes
    - Drop rate adjustments
    
    Time Effects:
    - Night: Ghost monsters +spawn, Thief drop +20%
    - Full Moon: Werewolf/Ghost special spawns
    - Dawn: Exp +10% (fresh start bonus)
    - Dusk: Rare material drop +15%
    
    4-Tier Optimization:
    - Tier 1: Rule-based transitions
    - Tier 2: Cached effects
    - Tier 3: Batched calculations
    - Tier 4: LLM descriptions
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Weather/Time Agent"""
        super().__init__(
            agent_id="weather_time_agent",
            agent_type="weather_time",
            config=config
        )
        
        # Weather pattern definitions (Tier 1: Rule-based)
        self.weather_effects = self._initialize_weather_effects()
        
        # Time-of-day effects (Tier 1: Rule-based)
        self.time_effects = self._initialize_time_effects()
        
        # Weather transition probabilities
        self.weather_transitions = {
            WeatherType.CLEAR: {
                WeatherType.CLEAR: 0.6,
                WeatherType.RAIN: 0.2,
                WeatherType.HEAT_WAVE: 0.1,
                WeatherType.AURORA: 0.05,
                WeatherType.SANDSTORM: 0.05
            },
            WeatherType.RAIN: {
                WeatherType.RAIN: 0.5,
                WeatherType.STORM: 0.3,
                WeatherType.CLEAR: 0.15,
                WeatherType.SNOW: 0.05
            },
            WeatherType.STORM: {
                WeatherType.STORM: 0.4,
                WeatherType.RAIN: 0.4,
                WeatherType.CLEAR: 0.2
            },
            WeatherType.SNOW: {
                WeatherType.SNOW: 0.6,
                WeatherType.CLEAR: 0.25,
                WeatherType.STORM: 0.1,
                WeatherType.AURORA: 0.05
            },
            WeatherType.HEAT_WAVE: {
                WeatherType.HEAT_WAVE: 0.5,
                WeatherType.CLEAR: 0.3,
                WeatherType.SANDSTORM: 0.2
            },
            WeatherType.SANDSTORM: {
                WeatherType.SANDSTORM: 0.5,
                WeatherType.CLEAR: 0.3,
                WeatherType.HEAT_WAVE: 0.2
            },
            WeatherType.AURORA: {
                WeatherType.AURORA: 0.4,
                WeatherType.CLEAR: 0.4,
                WeatherType.SNOW: 0.2
            }
        }
        
        logger.info("Weather/Time Agent initialized with 7 weather types")
    
    def _initialize_weather_effects(self) -> Dict[WeatherType, Dict[str, Any]]:
        """Initialize weather effect definitions (Tier 1: Rule-based)."""
        return {
            WeatherType.CLEAR: {
                'element_modifiers': {},
                'stat_modifiers': {},
                'drop_rate_mult': 1.0,
                'monster_behavior': 'normal',
                'visual_effects': ['clear_sky', 'bright'],
                'description': 'Clear skies and pleasant weather'
            },
            WeatherType.RAIN: {
                'element_modifiers': {
                    'water': 1.2,
                    'fire': 0.8
                },
                'stat_modifiers': {
                    'flee_mult': 0.95,
                    'hit_mult': 0.95
                },
                'drop_rate_mult': 1.05,
                'monster_behavior': 'sluggish',
                'visual_effects': ['rain', 'wet', 'puddles'],
                'description': 'Rain falls steadily, dampening flames'
            },
            WeatherType.STORM: {
                'element_modifiers': {
                    'wind': 1.3,
                    'water': 1.2,
                    'fire': 0.7
                },
                'stat_modifiers': {
                    'flee_mult': 0.9,
                    'hit_mult': 0.9,
                    'aspd_mult': 0.95
                },
                'drop_rate_mult': 1.0,
                'monster_behavior': 'aggressive',
                'visual_effects': ['heavy_rain', 'lightning', 'thunder', 'wind'],
                'description': 'Violent storm with crackling lightning'
            },
            WeatherType.SNOW: {
                'element_modifiers': {
                    'water': 1.3,
                    'fire': 0.7
                },
                'stat_modifiers': {
                    'movement_speed_mult': 0.9,
                    'aspd_mult': 0.92,
                    'ice_resist': -0.2
                },
                'drop_rate_mult': 1.05,
                'monster_behavior': 'slow',
                'visual_effects': ['snow', 'cold', 'frost', 'white'],
                'description': 'Snow falls gently, freezing everything'
            },
            WeatherType.HEAT_WAVE: {
                'element_modifiers': {
                    'fire': 1.3,
                    'water': 0.8
                },
                'stat_modifiers': {
                    'sp_regen_mult': 0.8,
                    'hp_regen_mult': 0.9,
                    'fire_resist': -0.2
                },
                'drop_rate_mult': 0.95,
                'monster_behavior': 'sluggish',
                'visual_effects': ['heat_waves', 'mirage', 'sweat', 'orange_tint'],
                'description': 'Oppressive heat drains stamina'
            },
            WeatherType.SANDSTORM: {
                'element_modifiers': {
                    'earth': 1.25,
                    'wind': 1.15
                },
                'stat_modifiers': {
                    'hit_mult': 0.9,
                    'flee_mult': 0.85,
                    'crit_mult': 0.9
                },
                'drop_rate_mult': 0.9,
                'monster_behavior': 'erratic',
                'visual_effects': ['sand', 'dust', 'wind', 'brown_tint'],
                'description': 'Whipping sand obscures vision'
            },
            WeatherType.AURORA: {
                'element_modifiers': {
                    'holy': 1.2,
                    'shadow': 1.2
                },
                'stat_modifiers': {
                    'matk_mult': 1.15,
                    'mdef_mult': 1.1,
                    'magic_damage_mult': 1.15
                },
                'drop_rate_mult': 1.1,
                'monster_behavior': 'mystical',
                'visual_effects': ['aurora', 'colorful_lights', 'ethereal', 'glow'],
                'description': 'Mystical lights enhance magical energy'
            }
        }
    
    def _initialize_time_effects(self) -> Dict[TimeOfDay, Dict[str, Any]]:
        """Initialize time-of-day effect definitions (Tier 1: Rule-based)."""
        return {
            TimeOfDay.DAWN: {
                'exp_mult': 1.1,
                'drop_rate_mult': 1.0,
                'monster_spawn_mult': 1.0,
                'special_spawns': [],
                'stat_modifiers': {'hp_regen_mult': 1.1},
                'description': 'Morning light brings renewed energy'
            },
            TimeOfDay.DAY: {
                'exp_mult': 1.0,
                'drop_rate_mult': 1.0,
                'monster_spawn_mult': 1.0,
                'special_spawns': [],
                'stat_modifiers': {},
                'description': 'Bright daylight illuminates the land'
            },
            TimeOfDay.DUSK: {
                'exp_mult': 1.0,
                'drop_rate_mult': 1.15,
                'monster_spawn_mult': 1.05,
                'special_spawns': [],
                'stat_modifiers': {'item_drop_rare_mult': 1.15},
                'description': 'Twilight hours favor rare discoveries'
            },
            TimeOfDay.NIGHT: {
                'exp_mult': 1.05,
                'drop_rate_mult': 1.0,
                'monster_spawn_mult': 1.1,
                'special_spawns': ['ghost', 'undead', 'werewolf'],
                'stat_modifiers': {
                    'flee_mult': 0.95,
                    'thief_drop_mult': 1.2
                },
                'description': 'Darkness brings supernatural creatures'
            },
            TimeOfDay.FULL_MOON: {
                'exp_mult': 1.15,
                'drop_rate_mult': 1.2,
                'monster_spawn_mult': 1.3,
                'special_spawns': ['werewolf', 'ghost', 'vampire', 'nightmare'],
                'stat_modifiers': {
                    'flee_mult': 0.9,
                    'undead_damage_mult': 1.3,
                    'rare_drop_mult': 1.25
                },
                'description': 'The full moon awakens ancient terrors'
            }
        }
    
    def _create_crew_agent(self):
        """Create CrewAI agent for weather/time management"""
        from crewai import Agent
        
        return Agent(
            role="Weather and Time System Manager",
            goal="Manage dynamic weather patterns and time-of-day effects",
            backstory="An AI system that controls environmental conditions and temporal flow",
            verbose=settings.crewai_verbose,
            allow_delegation=False,
            llm=self.llm_provider if hasattr(self, 'llm_provider') else None
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """Process method required by BaseAIAgent"""
        return AgentResponse(
            agent_type=self.agent_type,
            success=True,
            data={},
            confidence=1.0
        )
    
    # ========================================================================
    # PUBLIC API METHODS
    # ========================================================================
    
    async def update_weather(
        self,
        current_weather: Optional[WeatherType] = None,
        force_weather: Optional[WeatherType] = None
    ) -> AgentResponse:
        """
        Update weather based on patterns and world state.
        
        Weather Patterns:
        - 60% chance to continue current weather
        - 30% chance to transition to related weather
        - 10% chance random change
        - Special events can force weather (Problem Agent)
        
        Args:
            current_weather: Current weather (if None, fetches from DB)
            force_weather: Force specific weather (for events)
            
        Returns:
            AgentResponse with new weather state
        """
        try:
            # Get current weather if not provided
            if current_weather is None:
                current_state = await self.get_current_weather()
                if current_state:
                    current_weather = WeatherType(current_state['weather_type'])
                else:
                    current_weather = WeatherType.CLEAR
            
            # Forced weather (from events)
            if force_weather is not None:
                new_weather = force_weather
                logger.info(f"Weather forced to {force_weather.value} by event")
            else:
                # Natural transition
                new_weather = self._select_next_weather(current_weather)
            
            # Calculate effects
            effects = await self.calculate_weather_effects(new_weather)
            
            # Calculate next change time
            interval_hours = getattr(settings, 'weather_update_interval_hours', 3)
            variance = random.uniform(-0.5, 0.5)  # ±30 minutes
            next_change = datetime.now(UTC) + timedelta(hours=interval_hours + variance)
            
            # Update database
            query = """
                INSERT INTO weather_state
                (current_weather, weather_intensity, effect_data, next_change_at)
                VALUES ($1, $2, $3, $4)
            """
            
            await postgres_db.execute(
                query,
                new_weather.value,
                1.0,
                json.dumps(effects),
                next_change
            )
            
            # Record history
            await self._record_weather_history(new_weather, interval_hours)
            
            # Cache current weather
            weather_data = {
                'weather_type': new_weather.value,
                'weather_name': self._get_weather_name(new_weather),
                'effects': effects,
                'started_at': datetime.now(UTC).isoformat(),
                'next_change_at': next_change.isoformat()
            }
            
            await db.set("weather:current", weather_data, expire=interval_hours * 3600)
            
            logger.info(f"✓ Weather updated: {current_weather.value} → {new_weather.value}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=weather_data,
                confidence=0.95,
                reasoning="Weather updated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to update weather: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def calculate_weather_effects(
        self,
        weather: WeatherType
    ) -> Dict[str, Any]:
        """
        Calculate stat modifiers for weather (Tier 2: Cached).
        
        Effects include:
        - Element damage multipliers
        - SP/HP regen modifiers
        - Accuracy/flee modifiers
        - Drop rate changes
        - Monster aggro changes
        
        Args:
            weather: Weather type
            
        Returns:
            Dict of weather effects
        """
        # Try cache first
        cache_key = f"weather:effects:{weather.value}"
        cached = await db.get(cache_key)
        if cached:
            return cached
        
        # Get from definitions
        effects = self.weather_effects[weather].copy()
        
        # Cache for 24 hours
        await db.set(cache_key, effects, expire=86400)
        
        return effects
    
    async def get_time_of_day_effects(
        self,
        current_hour: Optional[int] = None
    ) -> Dict[str, Any]:
        """
        Calculate time-based effects (Tier 1: Rule-based).
        
        Time Brackets:
        - 05-07: Dawn (+10% EXP)
        - 07-18: Day (normal)
        - 18-20: Dusk (+15% rare drops)
        - 20-05: Night (+ghost spawns, +thief drops)
        - Special: Full Moon (random nights)
        
        Args:
            current_hour: Current hour (0-23), defaults to now
            
        Returns:
            Dict of time effects
        """
        if current_hour is None:
            current_hour = datetime.now(UTC).hour
        
        # Check if full moon
        is_full_moon = await self.check_full_moon()
        
        if is_full_moon:
            time_period = TimeOfDay.FULL_MOON
        elif 5 <= current_hour < 7:
            time_period = TimeOfDay.DAWN
        elif 7 <= current_hour < 18:
            time_period = TimeOfDay.DAY
        elif 18 <= current_hour < 20:
            time_period = TimeOfDay.DUSK
        else:
            time_period = TimeOfDay.NIGHT
        
        effects = self.time_effects[time_period].copy()
        effects['time_of_day'] = time_period.value
        effects['current_hour'] = current_hour
        
        return effects
    
    async def check_full_moon(self) -> bool:
        """
        Determine if tonight is full moon.
        
        Logic:
        - Check cached state first
        - 10% random chance per night
        - OR manual trigger from World Event Agent
        
        Returns:
            True if full moon active
        """
        # Try cache first
        cached = await db.get("weather:full_moon:active")
        if cached is not None:
            return bool(cached)
        
        # Check current hour (only at night)
        current_hour = datetime.now(UTC).hour
        if not (20 <= current_hour or current_hour < 5):
            return False
        
        # Check if already determined today
        today = datetime.now(UTC).date().isoformat()
        cache_key = f"weather:full_moon:checked:{today}"
        already_checked = await db.get(cache_key)
        
        if already_checked is not None:
            is_full_moon = bool(already_checked)
        else:
            # Random chance
            full_moon_chance = getattr(settings, 'full_moon_random_chance', 0.1)
            is_full_moon = random.random() < full_moon_chance
            
            # Cache decision for the night
            await db.set(cache_key, is_full_moon, expire=86400)
        
        # Cache active state for 10 minutes
        if is_full_moon:
            await db.set("weather:full_moon:active", True, expire=600)
        
        return is_full_moon
    
    async def apply_weather_time_effects(
        self,
        weather: Optional[WeatherType] = None,
        time_of_day: Optional[TimeOfDay] = None
    ) -> AgentResponse:
        """
        Apply combined weather + time effects.
        
        Synergies:
        - Rain + Night = Storm chance
        - Clear + Full Moon = Aurora chance
        - Heat Wave + Day = extra strong
        
        Args:
            weather: Weather type (fetches current if None)
            time_of_day: Time period (fetches current if None)
            
        Returns:
            AgentResponse with combined effects
        """
        try:
            # Get current weather if not provided
            if weather is None:
                current_weather = await self.get_current_weather()
                weather = WeatherType(current_weather['weather_type']) if current_weather else WeatherType.CLEAR
            
            # Get time effects
            time_effects = await self.get_time_of_day_effects()
            
            # Get weather effects
            weather_effects = await self.calculate_weather_effects(weather)
            
            # Combine effects
            combined = {
                'weather': {
                    'type': weather.value,
                    'effects': weather_effects
                },
                'time': time_effects,
                'synergies': self._calculate_synergies(weather, TimeOfDay(time_effects['time_of_day']))
            }
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=combined,
                confidence=1.0,
                reasoning="Combined effects calculated"
            )
            
        except Exception as e:
            logger.error(f"Failed to apply combined effects: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_current_weather(self) -> Optional[Dict[str, Any]]:
        """Get current weather state"""
        try:
            # Try cache first
            cached = await db.get("weather:current")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT 
                    current_weather,
                    weather_intensity,
                    effect_data,
                    started_at,
                    next_change_at
                FROM weather_state
                ORDER BY started_at DESC
                LIMIT 1
            """
            
            result = await postgres_db.fetch_one(query)
            
            if result:
                weather_data = {
                    'weather_type': result['current_weather'],
                    'weather_name': self._get_weather_name(WeatherType(result['current_weather'])),
                    'intensity': result['weather_intensity'],
                    'effects': json.loads(result['effect_data']) if isinstance(result['effect_data'], str) else result['effect_data'],
                    'started_at': result['started_at'].isoformat(),
                    'next_change_at': result['next_change_at'].isoformat()
                }
                
                # Cache for 5 minutes
                await db.set("weather:current", weather_data, expire=300)
                
                return weather_data
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to get current weather: {e}")
            return None
    
    async def get_weather_forecast(self, hours: int = 24) -> List[Dict[str, Any]]:
        """
        Get weather patterns for next N hours.
        
        Args:
            hours: Number of hours to forecast
            
        Returns:
            List of forecasted weather periods
        """
        try:
            current = await self.get_current_weather()
            if not current:
                return []
            
            current_weather = WeatherType(current['weather_type'])
            forecast = []
            
            interval = getattr(settings, 'weather_update_interval_hours', 3)
            periods = hours // interval
            
            for i in range(periods):
                # Predict next weather
                next_weather = self._select_next_weather(current_weather, deterministic=True)
                
                start_time = datetime.now(UTC) + timedelta(hours=i * interval)
                end_time = start_time + timedelta(hours=interval)
                
                forecast.append({
                    'period': i + 1,
                    'weather_type': next_weather.value,
                    'weather_name': self._get_weather_name(next_weather),
                    'start_time': start_time.isoformat(),
                    'end_time': end_time.isoformat(),
                    'probability': 0.6 if next_weather == current_weather else 0.3
                })
                
                current_weather = next_weather
            
            return forecast
            
        except Exception as e:
            logger.error(f"Failed to get weather forecast: {e}")
            return []
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _select_next_weather(
        self,
        current_weather: WeatherType,
        deterministic: bool = False
    ) -> WeatherType:
        """
        Select next weather based on transition probabilities.
        
        Args:
            current_weather: Current weather type
            deterministic: If True, select most likely; if False, random weighted
            
        Returns:
            Next weather type
        """
        transitions = self.weather_transitions.get(current_weather, {})
        
        if not transitions:
            return WeatherType.CLEAR
        
        if deterministic:
            # Select most likely
            return max(transitions.items(), key=lambda x: x[1])[0]
        else:
            # Weighted random
            weathers = list(transitions.keys())
            probabilities = list(transitions.values())
            return random.choices(weathers, weights=probabilities)[0]
    
    def _get_weather_name(self, weather: WeatherType) -> str:
        """Get display name for weather"""
        names = {
            WeatherType.CLEAR: "Clear Skies",
            WeatherType.RAIN: "Rain",
            WeatherType.STORM: "Thunderstorm",
            WeatherType.SNOW: "Snowfall",
            WeatherType.HEAT_WAVE: "Heat Wave",
            WeatherType.SANDSTORM: "Sandstorm",
            WeatherType.AURORA: "Aurora Borealis"
        }
        return names.get(weather, weather.value.replace('_', ' ').title())
    
    def _calculate_synergies(
        self,
        weather: WeatherType,
        time_of_day: TimeOfDay
    ) -> List[str]:
        """Calculate synergies between weather and time"""
        synergies = []
        
        if weather == WeatherType.RAIN and time_of_day == TimeOfDay.NIGHT:
            synergies.append("Storm potential increased")
        
        if weather == WeatherType.CLEAR and time_of_day == TimeOfDay.FULL_MOON:
            synergies.append("Aurora chance increased")
        
        if weather == WeatherType.HEAT_WAVE and time_of_day == TimeOfDay.DAY:
            synergies.append("Heat effects amplified")
        
        if weather == WeatherType.SNOW and time_of_day == TimeOfDay.NIGHT:
            synergies.append("Blizzard conditions possible")
        
        return synergies
    
    async def _record_weather_history(
        self,
        weather: WeatherType,
        duration_hours: int
    ):
        """Record weather change in history"""
        try:
            # Get player count (mock for now)
            player_count = random.randint(10, 100)
            
            query = """
                INSERT INTO weather_history
                (weather_type, time_of_day, duration_minutes, player_count_during)
                VALUES ($1, $2, $3, $4)
            """
            
            current_hour = datetime.now(UTC).hour
            time_effects = await self.get_time_of_day_effects(current_hour)
            
            await postgres_db.execute(
                query,
                weather.value,
                time_effects['time_of_day'],
                int(duration_hours * 60),
                player_count
            )
            
        except Exception as e:
            logger.debug(f"Failed to record weather history: {e}")


# Global instance
weather_time_agent = WeatherTimeAgent()