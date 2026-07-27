"""
World state data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field
from enum import Enum


class WeatherType(str, Enum):
    """Weather types"""
    CLEAR = "clear"
    SUNNY = "sunny"
    CLOUDY = "cloudy"
    RAINY = "rainy"
    STORMY = "stormy"
    SNOWY = "snowy"
    FOGGY = "foggy"
    WINDY = "windy"


class TimeOfDay(str, Enum):
    """Times of day"""
    DAWN = "dawn"
    DAY = "day"
    NOON = "noon"
    DUSK = "dusk"
    NIGHT = "night"
    MIDNIGHT = "midnight"


class EconomyState(BaseModel):
    """Economy state data"""
    item_prices: Dict[str, float] = Field(default_factory=dict, description="Item prices")
    item_supply: Dict[str, int] = Field(default_factory=dict, description="Item supply quantities")
    item_demand: Dict[str, float] = Field(default_factory=dict, description="Item demand levels")
    price_history: Dict[str, List[float]] = Field(default_factory=dict, description="Price history per item")
    supply_demand: Dict[str, Dict[str, float]] = Field(default_factory=dict, description="Supply/demand data")
    trade_volume: float = Field(0.0, description="Total trade volume")
    inflation_rate: float = Field(0.0, description="Inflation rate")
    average_player_wealth: float = Field(0.0, description="Average player wealth")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class EnvironmentState(BaseModel):
    """Environment state data"""
    game_time: int = Field(0, description="Game time in minutes")
    time_of_day: TimeOfDay = Field(TimeOfDay.DAY, description="Time of day")
    weather: WeatherType = Field(WeatherType.CLEAR, description="Current weather")
    weather_intensity: float = Field(0.0, ge=0.0, le=1.0, description="Weather intensity")
    season: str = Field("spring", description="Current season")
    day_of_year: int = Field(0, description="Day of year")
    active_events: List[Dict[str, Any]] = Field(default_factory=list, description="Active events")
    global_modifiers: Dict[str, float] = Field(default_factory=dict, description="Global modifiers")
    resource_availability: Dict[str, float] = Field(default_factory=dict, description="Resource availability")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class WorldState(BaseModel):
    """Complete world state"""
    world_id: str = Field(..., description="World identifier")
    world_name: str = Field(..., description="World name")
    economy: EconomyState = Field(default_factory=EconomyState, description="Economy state")
    environment: EnvironmentState = Field(default_factory=EnvironmentState, description="Environment state")
    active_players: int = Field(0, description="Number of active players")
    active_npcs: int = Field(0, description="Number of active NPCs")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class PoliticsState(BaseModel):
    """Politics state data"""
    faction_relations: Dict[str, Dict[str, float]] = Field(default_factory=dict, description="Faction relationships")
    territory_control: Dict[str, str] = Field(default_factory=dict, description="Territory control by faction")
    active_conflicts: List[Dict[str, Any]] = Field(default_factory=list, description="Active conflicts")
    diplomatic_events: List[Dict[str, Any]] = Field(default_factory=list, description="Recent diplomatic events")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class WorldStateUpdateRequest(BaseModel):
    """Request to update world state"""
    state_type: str = Field(..., description="State type: economy/politics/environment/culture")
    state_data: Dict[str, Any] = Field(..., description="State data to update")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Update timestamp")
    source: Optional[str] = Field(None, description="Update source")


class WorldStateUpdateResponse(BaseModel):
    """Response from world state update"""
    status: str = Field(..., description="Update status: success/error")
    state_type: str = Field(..., description="Updated state type")
    message: Optional[str] = Field(None, description="Status message")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")


class WorldStateQueryResponse(BaseModel):
    """Response with world state data"""
    state_type: Optional[str] = Field(None, description="Queried state type")
    economy: Optional[EconomyState] = Field(None, description="Economy state")
    politics: Optional[PoliticsState] = Field(None, description="Politics state")
    environment: Optional[EnvironmentState] = Field(None, description="Environment state")
    culture: Optional[Dict[str, Any]] = Field(None, description="Culture state")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")


# Alias for backward compatibility
EconomicState = EconomyState
