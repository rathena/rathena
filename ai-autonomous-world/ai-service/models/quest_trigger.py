"""
Quest trigger system data models for dynamic quest activation
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional, Union
from datetime import datetime, time
from enum import Enum


class TriggerType(str, Enum):
    """Types of quest triggers"""
    REAL_TIME = "real_time"  # Based on actual date/time
    SCHEDULED = "scheduled"  # In-game time/day/season
    WEATHER = "weather"  # Weather conditions
    NPC_SPECIFIC = "npc_specific"  # NPC state/mood
    LOCATION = "location"  # Specific map/zone
    WORLD_STATE = "world_state"  # Global events
    INTERNAL = "internal"  # NPC internal states
    EXTERNAL = "external"  # Player actions/items
    RELATIONSHIP = "relationship"  # Relationship level threshold
    GIFT = "gift"  # Gift-giving trigger
    SEQUENCE = "sequence"  # Previous quest completion


class WeatherCondition(str, Enum):
    """Weather conditions"""
    CLEAR = "clear"
    RAIN = "rain"
    SNOW = "snow"
    STORM = "storm"
    FOG = "fog"
    ANY = "any"


class Season(str, Enum):
    """In-game seasons"""
    SPRING = "spring"
    SUMMER = "summer"
    AUTUMN = "autumn"
    WINTER = "winter"


class RealTimeTrigger(BaseModel):
    """Trigger based on real-world date/time"""
    start_date: Optional[datetime] = None
    end_date: Optional[datetime] = None
    days_of_week: Optional[List[int]] = Field(
        None,
        description="Days of week (0=Monday, 6=Sunday)"
    )
    hours: Optional[List[int]] = Field(
        None,
        description="Hours of day (0-23)"
    )


class ScheduledTrigger(BaseModel):
    """Trigger based on in-game time"""
    game_time_start: Optional[time] = None
    game_time_end: Optional[time] = None
    game_day: Optional[int] = Field(None, ge=1, le=31)
    season: Optional[Season] = None


class WeatherTrigger(BaseModel):
    """Trigger based on weather"""
    required_weather: WeatherCondition
    map_name: Optional[str] = Field(None, description="Specific map or any map")


class NPCSpecificTrigger(BaseModel):
    """Trigger based on NPC state"""
    npc_mood: Optional[str] = Field(None, description="happy, sad, angry, etc.")
    npc_health_below: Optional[float] = Field(None, ge=0.0, le=1.0)
    npc_busy: Optional[bool] = None
    npc_location: Optional[str] = None


class LocationTrigger(BaseModel):
    """Trigger based on player/NPC location"""
    map_name: str
    x_min: Optional[int] = None
    x_max: Optional[int] = None
    y_min: Optional[int] = None
    y_max: Optional[int] = None
    radius: Optional[int] = Field(None, description="Radius from NPC position")


class WorldStateTrigger(BaseModel):
    """Trigger based on world state"""
    event_name: Optional[str] = None
    faction_war: Optional[bool] = None
    economy_state: Optional[str] = Field(None, description="boom, recession, stable")
    global_variable: Optional[Dict[str, Any]] = None


class InternalTrigger(BaseModel):
    """Trigger based on NPC internal state"""
    memory_contains: Optional[str] = Field(None, description="Keyword in memories")
    emotion_state: Optional[str] = Field(None, description="Current emotion")
    goal_active: Optional[str] = Field(None, description="Active goal name")
    stress_level_above: Optional[float] = Field(None, ge=0.0, le=1.0)


class ExternalTrigger(BaseModel):
    """Trigger based on external factors"""
    player_has_item: Optional[int] = Field(None, description="Item ID")
    player_level_min: Optional[int] = None
    player_level_max: Optional[int] = None
    player_class: Optional[str] = None
    quest_completed: Optional[str] = Field(None, description="Quest ID")
    player_reputation_min: Optional[float] = None


class RelationshipTrigger(BaseModel):
    """Trigger based on relationship level"""
    relationship_level_min: float = Field(ge=-100.0, le=100.0)
    relationship_level_max: Optional[float] = Field(None, ge=-100.0, le=100.0)
    relationship_type: Optional[str] = Field(None, description="friendship, romantic, etc.")


class GiftTrigger(BaseModel):
    """Trigger based on gift-giving"""
    total_gifts_min: Optional[int] = Field(None, ge=0)
    specific_item_received: Optional[int] = Field(None, description="Item ID")
    gift_category_received: Optional[str] = None
    total_gift_value_min: Optional[int] = Field(None, ge=0)


class SequenceTrigger(BaseModel):
    """Trigger based on quest sequence"""
    prerequisite_quests: List[str] = Field(description="Quest IDs that must be completed")
    all_required: bool = Field(default=True, description="All or any prerequisite")


class QuestTrigger(BaseModel):
    """Complete quest trigger definition"""
    trigger_id: str
    trigger_type: TriggerType
    enabled: bool = True
    priority: int = Field(default=1, ge=1, le=10, description="Higher = checked first")
    
    # Specific trigger data (only one should be set based on trigger_type)
    real_time: Optional[RealTimeTrigger] = None
    scheduled: Optional[ScheduledTrigger] = None
    weather: Optional[WeatherTrigger] = None
    npc_specific: Optional[NPCSpecificTrigger] = None
    location: Optional[LocationTrigger] = None
    world_state: Optional[WorldStateTrigger] = None
    internal: Optional[InternalTrigger] = None
    external: Optional[ExternalTrigger] = None
    relationship: Optional[RelationshipTrigger] = None
    gift: Optional[GiftTrigger] = None
    sequence: Optional[SequenceTrigger] = None
    
    # Metadata
    description: str = Field(description="Human-readable trigger description")
    cooldown_seconds: Optional[int] = Field(None, description="Cooldown between activations")
    last_triggered: Optional[datetime] = None

