"""
NPC-related data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field


class NPCPosition(BaseModel):
    """NPC position in the game world"""
    map: str = Field(..., description="Map name")
    x: int = Field(..., description="X coordinate")
    y: int = Field(..., description="Y coordinate")


class NPCPersonality(BaseModel):
    """NPC personality traits (Big Five model)"""
    openness: float = Field(0.5, ge=0.0, le=1.0, description="Openness to experience")
    conscientiousness: float = Field(0.5, ge=0.0, le=1.0, description="Conscientiousness")
    extraversion: float = Field(0.5, ge=0.0, le=1.0, description="Extraversion")
    agreeableness: float = Field(0.5, ge=0.0, le=1.0, description="Agreeableness")
    neuroticism: float = Field(0.5, ge=0.0, le=1.0, description="Neuroticism")
    moral_alignment: str = Field("neutral", description="Moral alignment: good/evil/neutral")
    quirks: List[str] = Field(default_factory=list, description="Personality quirks")


class NPCRegisterRequest(BaseModel):
    """Request to register an NPC with the AI service"""
    npc_id: str = Field(..., description="Unique NPC identifier")
    name: str = Field(..., description="NPC name")
    npc_class: str = Field(..., description="NPC class/type")
    level: int = Field(..., ge=1, le=999, description="NPC level")
    position: NPCPosition = Field(..., description="NPC position")
    personality: Optional[NPCPersonality] = Field(None, description="NPC personality traits")
    faction_id: Optional[str] = Field(None, description="Faction ID")
    initial_goals: Optional[List[str]] = Field(default_factory=list, description="Initial goals")


class NPCRegisterResponse(BaseModel):
    """Response from NPC registration"""
    status: str = Field(..., description="Registration status: success/error")
    agent_id: str = Field(..., description="Assigned AI agent ID")
    npc_id: str = Field(..., description="NPC ID")
    message: Optional[str] = Field(None, description="Status message")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Registration timestamp")


class NPCEventRequest(BaseModel):
    """Request to send an event to the AI service"""
    npc_id: str = Field(..., description="NPC ID")
    event_type: str = Field(..., description="Event type: combat/social/economic/environmental")
    event_data: Dict[str, Any] = Field(..., description="Event-specific data")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Event timestamp")
    context: Optional[Dict[str, Any]] = Field(default_factory=dict, description="Additional context")


class NPCEventResponse(BaseModel):
    """Response from event processing"""
    status: str = Field(..., description="Processing status: queued/processing/completed/error")
    event_id: str = Field(..., description="Unique event ID")
    npc_id: str = Field(..., description="NPC ID")
    message: Optional[str] = Field(None, description="Status message")
    estimated_processing_time: Optional[float] = Field(None, description="Estimated processing time in seconds")


class NPCAction(BaseModel):
    """NPC action to be executed"""
    action_type: str = Field(..., description="Action type: dialogue/movement/combat/trade/social")
    action_data: Dict[str, Any] = Field(..., description="Action-specific data")
    priority: int = Field(1, ge=1, le=10, description="Action priority (1=low, 10=high)")
    execution_params: Optional[Dict[str, Any]] = Field(default_factory=dict, description="Execution parameters")


class NPCActionResponse(BaseModel):
    """Response with next action for NPC"""
    npc_id: str = Field(..., description="NPC ID")
    action: Optional[NPCAction] = Field(None, description="Next action to execute")
    state_update: Optional[Dict[str, Any]] = Field(None, description="NPC state updates")
    message: Optional[str] = Field(None, description="Status message")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")

