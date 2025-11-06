"""
World state data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field


class EconomyState(BaseModel):
    """Economy state data"""
    item_prices: Dict[str, float] = Field(default_factory=dict, description="Item prices")
    supply_demand: Dict[str, Dict[str, float]] = Field(default_factory=dict, description="Supply/demand data")
    trade_volume: float = Field(0.0, description="Total trade volume")
    inflation_rate: float = Field(0.0, description="Inflation rate")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class PoliticsState(BaseModel):
    """Politics state data"""
    faction_relations: Dict[str, Dict[str, float]] = Field(default_factory=dict, description="Faction relationships")
    territory_control: Dict[str, str] = Field(default_factory=dict, description="Territory control by faction")
    active_conflicts: List[Dict[str, Any]] = Field(default_factory=list, description="Active conflicts")
    diplomatic_events: List[Dict[str, Any]] = Field(default_factory=list, description="Recent diplomatic events")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="State timestamp")


class EnvironmentState(BaseModel):
    """Environment state data"""
    weather: str = Field("clear", description="Current weather")
    time_of_day: str = Field("day", description="Time of day: dawn/day/dusk/night")
    season: str = Field("spring", description="Current season")
    resource_availability: Dict[str, float] = Field(default_factory=dict, description="Resource availability")
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

