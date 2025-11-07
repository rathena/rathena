"""
Player interaction data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field


class InteractionContext(BaseModel):
    """Context for player-NPC interaction"""
    player_name: str = Field(..., description="Player name")
    player_level: int = Field(..., description="Player level")
    player_class: str = Field(..., description="Player class")
    location: Dict[str, Any] = Field(..., description="Location data (map, x, y)")
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

