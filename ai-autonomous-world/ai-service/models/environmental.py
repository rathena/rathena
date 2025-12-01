"""
Pydantic Models for Environmental Agents

Request and response models for:
- Map Hazard Agent API
- Weather/Time Agent API
- Treasure Agent API
"""

from pydantic import BaseModel, Field, validator
from typing import Dict, List, Optional, Any
from datetime import datetime
from enum import Enum


# ============================================================================
# ENUMS
# ============================================================================

class HazardType(str, Enum):
    """Map hazard types"""
    BLOOD_MOON = "blood_moon"
    MAGIC_STORM = "magic_storm"
    THICK_FOG = "thick_fog"
    FAIRY_BLESSING = "fairy_blessing"
    MANA_DRAIN = "mana_drain"
    SCORCHING_HEAT = "scorching_heat"
    FROZEN_WASTELAND = "frozen_wasteland"
    TOXIC_MIASMA = "toxic_miasma"
    HOLY_GROUND = "holy_ground"
    CHAOTIC_VOID = "chaotic_void"


class WeatherType(str, Enum):
    """Weather types"""
    CLEAR = "clear"
    RAIN = "rain"
    STORM = "storm"
    SNOW = "snow"
    HEAT_WAVE = "heat_wave"
    SANDSTORM = "sandstorm"
    AURORA = "aurora"


class TimeOfDay(str, Enum):
    """Time of day periods"""
    DAWN = "dawn"
    DAY = "day"
    DUSK = "dusk"
    NIGHT = "night"
    FULL_MOON = "full_moon"


class TreasureRarity(str, Enum):
    """Treasure rarity tiers"""
    BRONZE = "bronze"
    SILVER = "silver"
    GOLD = "gold"
    MYTHIC = "mythic"


class TreasureType(str, Enum):
    """Treasure spawn types"""
    CHEST = "chest"
    DIG_SPOT = "dig_spot"
    HIDDEN_NPC = "hidden_npc"


# ============================================================================
# MAP HAZARD MODELS
# ============================================================================

class MapActivityModel(BaseModel):
    """Map activity data for hazard generation"""
    map_activity: Dict[str, int] = Field(
        ...,
        description="Dictionary of map_name -> player_count"
    )


class ProblemModel(BaseModel):
    """Active problem data"""
    problem_id: int
    map_name: str
    problem_type: str


class HazardGenerateRequest(BaseModel):
    """Request to generate daily hazards"""
    map_activity: Dict[str, int] = Field(
        ...,
        description="Map activity data (map_name -> count)"
    )
    active_problems: List[ProblemModel] = Field(
        default_factory=list,
        description="Currently active problems"
    )
    count: Optional[int] = Field(
        None,
        ge=1,
        le=10,
        description="Number of hazards to generate (default: 4)"
    )


class HazardResponse(BaseModel):
    """Single hazard response"""
    hazard_id: int
    map_name: str
    hazard_type: str
    hazard_name: str
    description: str
    applied_at: str
    expires_at: str
    effects: Dict[str, Any]


class HazardListResponse(BaseModel):
    """List of hazards response"""
    hazards_generated: int
    hazards: List[HazardResponse]
    balance: Dict[str, int]
    timestamp: str


class HazardEncounterModel(BaseModel):
    """Hazard encounter data"""
    time_in_hazard: int = Field(..., ge=0, description="Time spent (seconds)")
    items_dropped: int = Field(default=0, ge=0)
    deaths: int = Field(default=0, ge=0)


# ============================================================================
# WEATHER/TIME MODELS
# ============================================================================

class WeatherUpdateRequest(BaseModel):
    """Request to update weather"""
    current_weather: Optional[WeatherType] = Field(
        None,
        description="Current weather (auto-fetched if None)"
    )
    force_weather: Optional[WeatherType] = Field(
        None,
        description="Force specific weather for events"
    )


class WeatherEffectsResponse(BaseModel):
    """Weather effects data"""
    element_modifiers: Dict[str, float] = Field(default_factory=dict)
    stat_modifiers: Dict[str, float] = Field(default_factory=dict)
    drop_rate_mult: float = 1.0
    monster_behavior: str = "normal"
    visual_effects: List[str] = Field(default_factory=list)
    description: str


class TimeEffectsResponse(BaseModel):
    """Time of day effects data"""
    time_of_day: str
    current_hour: int
    exp_mult: float = 1.0
    drop_rate_mult: float = 1.0
    monster_spawn_mult: float = 1.0
    special_spawns: List[str] = Field(default_factory=list)
    stat_modifiers: Dict[str, float] = Field(default_factory=dict)
    description: str


class WeatherResponse(BaseModel):
    """Current weather response"""
    weather_type: str
    weather_name: str
    intensity: float = 1.0
    effects: WeatherEffectsResponse
    started_at: str
    next_change_at: str


class CombinedEffectsResponse(BaseModel):
    """Combined weather and time effects"""
    weather: Dict[str, Any]
    time: TimeEffectsResponse
    synergies: List[str] = Field(default_factory=list)


class WeatherForecastPeriod(BaseModel):
    """Single weather forecast period"""
    period: int
    weather_type: str
    weather_name: str
    start_time: str
    end_time: str
    probability: float


class WeatherForecastResponse(BaseModel):
    """Weather forecast response"""
    forecast: List[WeatherForecastPeriod]
    forecast_hours: int


# ============================================================================
# TREASURE MODELS
# ============================================================================

class TreasureSpawnRequest(BaseModel):
    """Request to spawn treasures"""
    map_activity: Dict[str, int] = Field(
        ...,
        description="Map activity data (map_name -> count)"
    )
    count: Optional[int] = Field(
        None,
        ge=1,
        le=20,
        description="Number of treasures to spawn (default: 10)"
    )


class TreasureItemModel(BaseModel):
    """Single treasure item"""
    name: str
    quantity: int


class TreasureContentsModel(BaseModel):
    """Treasure contents"""
    items: List[TreasureItemModel]
    zeny: int
    reputation_reward: int
    card_fragments: int
    rarity: str


class TreasureResponse(BaseModel):
    """Single treasure response"""
    treasure_id: int
    treasure_type: str
    rarity: str
    spawn_map: str
    spawned_at: str
    despawns_at: str
    hint_text: Optional[str] = None
    spawn_x: Optional[int] = None
    spawn_y: Optional[int] = None


class TreasureSpawnResponse(BaseModel):
    """Treasure spawn response"""
    treasures_spawned: int
    treasures: List[TreasureResponse]
    distribution: Dict[str, int]
    rarity_breakdown: Dict[str, int]
    timestamp: str


class TreasureDiscoveryResponse(BaseModel):
    """Treasure discovery response"""
    treasure_id: int
    rarity: str
    rewards: TreasureContentsModel
    player_id: int
    discovered_at: str


class CardFragmentResponse(BaseModel):
    """Card fragment data"""
    player_id: int
    fragment_count: int
    cards_claimed: int
    last_fragment_at: Optional[str] = None
    fragments_needed: int


class CardClaimResponse(BaseModel):
    """Card claim from fragments response"""
    player_id: int
    card_obtained: Dict[str, Any]
    fragments_spent: int
    fragments_remaining: int
    total_cards_claimed: int
    timestamp: str


# ============================================================================
# GENERIC RESPONSES
# ============================================================================

class EnvironmentalStatusResponse(BaseModel):
    """Environmental system status"""
    map_hazard_enabled: bool
    weather_time_enabled: bool
    treasure_enabled: bool
    active_hazards: int
    current_weather: Optional[str] = None
    active_treasures: int
    timestamp: str


class EnvironmentalMetricsResponse(BaseModel):
    """Environmental system metrics"""
    hazards_generated_today: int
    weather_changes_today: int
    treasures_spawned_today: int
    treasures_discovered_today: int
    total_fragments_awarded_today: int
    timestamp: str


class ErrorResponse(BaseModel):
    """Generic error response"""
    error: str
    detail: Optional[str] = None
    timestamp: str = Field(default_factory=lambda: datetime.now().isoformat())


# ============================================================================
# LIST WRAPPERS
# ============================================================================

class HazardList(BaseModel):
    """List of hazards"""
    hazards: List[HazardResponse]
    total: int


class TreasureList(BaseModel):
    """List of treasures"""
    treasures: List[TreasureResponse]
    total: int


class WeatherHistoryItem(BaseModel):
    """Single weather history entry"""
    weather_type: str
    time_of_day: str
    duration_minutes: int
    player_count_during: int
    timestamp: str


class WeatherHistoryResponse(BaseModel):
    """Weather history response"""
    history: List[WeatherHistoryItem]
    total: int