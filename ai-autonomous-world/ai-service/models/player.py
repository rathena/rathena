"""
Player interaction data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field


class PlayerState(BaseModel):
    """Player state data"""
    player_id: str = Field(..., description="Unique player identifier")
    character_name: str = Field(..., description="Player character name")
    job_class: str = Field("Novice", description="Player job class")
    base_level: int = Field(1, description="Base level")
    job_level: int = Field(1, description="Job level")
    location: str = Field("prontera", description="Current map location")
    position_x: float = Field(0.0, description="X coordinate")
    position_y: float = Field(0.0, description="Y coordinate")
    zeny: int = Field(0, description="Zeny currency amount")
    experience: int = Field(0, description="Experience points")
    is_online: bool = Field(True, description="Whether player is online")


class PlayerContext(BaseModel):
    """Context for player interactions"""
    player_id: str = Field(..., description="Player ID")
    player_name: str = Field(..., description="Player name")
    player_level: int = Field(1, description="Player level")
    player_class: str = Field("Novice", description="Player class")
    location: str = Field("prontera", description="Current location")
    nearby_npcs: List[str] = Field(default_factory=list, description="Nearby NPC IDs")
    recent_events: List[Dict[str, Any]] = Field(default_factory=list, description="Recent events")


class InteractionHistory(BaseModel):
    """Record of a player-NPC interaction"""
    interaction_id: str = Field(..., description="Unique interaction ID")
    player_id: str = Field(..., description="Player ID")
    npc_id: str = Field(..., description="NPC ID")
    interaction_type: str = Field(..., description="Type of interaction")
    message: str = Field("", description="Interaction message")
    response: str = Field("", description="NPC response")
    affinity_change: float = Field(0.0, description="Relationship affinity change")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Interaction timestamp")


class InteractionContext(BaseModel):
    """Context for player-NPC interaction"""
    player_name: str = Field(..., description="Player name")
    player_level: Optional[int] = Field(1, description="Player level")
    player_class: Optional[str] = Field("Novice", description="Player class")
    location: Optional[Dict[str, Any]] = Field(default_factory=lambda: {"map": "prontera", "x": 0, "y": 0}, description="Location data (map, x, y)")
    time_of_day: str = Field("day", description="Time of day")
    weather: str = Field("clear", description="Weather condition")
    nearby_npcs: Optional[List[str]] = Field(default_factory=list, description="Nearby NPC IDs")
    nearby_players: Optional[List[str]] = Field(default_factory=list, description="Nearby player IDs")
    quest_state: Optional[Dict[str, Any]] = Field(default_factory=dict, description="Player quest state")


class PlayerInteractionRequest(BaseModel):
    """Request for player-NPC interaction"""
    npc_id: str = Field(..., description="NPC ID")
    player_id: str = Field(..., description="Player ID")
    interaction_type: str = Field(..., description="Interaction type: talk/trade/quest/attack")
    context: InteractionContext = Field(..., description="Interaction context")
    message: Optional[str] = Field(None, description="Player message (for talk interactions)")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Interaction timestamp")


class NPCResponse(BaseModel):
    """NPC response to player interaction"""
    action: str = Field(..., description="Response action: dialogue/trade/quest_offer/combat")
    data: Dict[str, Any] = Field(..., description="Action-specific data")
    emotion: Optional[str] = Field(None, description="NPC emotion: friendly/neutral/hostile/fearful")
    next_actions: Optional[List[str]] = Field(default_factory=list, description="Available next actions")


class PlayerInteractionResponse(BaseModel):
    """Response from player interaction processing"""
    npc_id: str = Field(..., description="NPC ID")
    player_id: str = Field(..., description="Player ID")
    response: NPCResponse = Field(..., description="NPC response")
    npc_state_update: Optional[Dict[str, Any]] = Field(None, description="NPC state updates")
    relationship_change: Optional[Dict[str, int]] = Field(None, description="Relationship changes")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")
