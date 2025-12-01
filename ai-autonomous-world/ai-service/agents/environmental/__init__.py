"""
Environmental Agents Package

This package contains agents responsible for environmental systems:
- Map Hazard Agent: Dynamic map modifiers and hazards
- Weather/Time Agent: Weather patterns and time-of-day effects
- Treasure Agent: Procedural treasure spawns and exploration rewards

These agents coordinate with procedural and progression agents to create
a dynamic, living game world with varied environmental conditions.
"""

from agents.environmental.map_hazard_agent import MapHazardAgent, HazardType
from agents.environmental.weather_time_agent import WeatherTimeAgent, WeatherType, TimeOfDay
from agents.environmental.treasure_agent import TreasureAgent, TreasureRarity

__all__ = [
    'MapHazardAgent',
    'HazardType',
    'WeatherTimeAgent',
    'WeatherType',
    'TimeOfDay',
    'TreasureAgent',
    'TreasureRarity',
]