"""
Pydantic models for Procedural Content Agents
Provides request/response validation for Problem, Dynamic NPC, and World Event agents
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from pydantic import BaseModel, Field, field_validator
from enum import Enum


# ============================================================================
# ENUMS
# ============================================================================

class ProblemType(str, Enum):
    """Types of world problems"""
    MONSTER_SURGE = "monster_surge"
    MVP_RAMPAGE = "mvp_rampage"
    GOOD_VS_EVIL = "good_vs_evil"
    RESOURCE_SHORTAGE = "resource_shortage"
    POLLUTION = "pollution"
    WEATHER_HAZARD = "weather_hazard"


class ProblemStatus(str, Enum):
    """Problem status"""
    ACTIVE = "active"
    COMPLETED = "completed"
    EXPIRED = "expired"
    CANCELLED = "cancelled"


class NPCType(str, Enum):
    """Types of dynamic NPCs"""
    TREASURE_SEEKER = "treasure_seeker"
    RUNAWAY_SAGE = "runaway_sage"
    WANDERING_BLACKSMITH = "wandering_blacksmith"
    MYSTERIOUS_CHILD = "mysterious_child"
    LOST_MERCHANT = "lost_merchant"


class NPCStatus(str, Enum):
    """Dynamic NPC status"""
    ACTIVE = "active"
    DESPAWNED = "despawned"
    COMPLETED = "completed"


class EventType(str, Enum):
    """Types of world events"""
    DEMONIC_INVASION = "demonic_invasion"
    MARKET_CRASH = "market_crash"
    LUCKY_DAY = "lucky_day"
    WANDERING_CARAVAN = "wandering_caravan"
    GLOBAL_XP_BOOST = "global_xp_boost"
    MVP_ENRAGED = "mvp_enraged"
    MYTHIC_EVENT = "mythic_event"


class EventStatus(str, Enum):
    """World event status"""
    ACTIVE = "active"
    COMPLETED = "completed"
    EXPIRED = "expired"


# ============================================================================
# WORLD STATE MODELS
# ============================================================================

class WorldStateModel(BaseModel):
    """World state data for problem/event generation"""
    avg_player_level: int = Field(..., ge=1, le=999, description="Average player level")
    online_players: int = Field(..., ge=0, description="Current online player count")
    map_activity: Dict[str, int] = Field(..., description="Map name -> player count")
    monster_kills: Dict[str, int] = Field(..., description="Map -> total kill count")
    mvp_kills: Dict[str, int] = Field(..., description="MVP ID -> kill count")
    economy: Dict[str, Any] = Field(..., description="Economic metrics")

    @field_validator('map_activity', 'monster_kills', 'mvp_kills')
    @classmethod
    def validate_positive_counts(cls, v):
        """Ensure all counts are non-negative"""
        for key, value in v.items():
            if value < 0:
                raise ValueError(f"Count for {key} must be non-negative")
        return v


class MapActivityModel(BaseModel):
    """Map activity heatmap data"""
    map_activity: Dict[str, int] = Field(..., description="Map name -> visit count (last 24h)")
    heatmap_data: Optional[Dict[str, List[Dict[str, int]]]] = Field(
        None, 
        description="Detailed heatmap per map: [{x, y, count}]"
    )


# ============================================================================
# PROBLEM AGENT MODELS
# ============================================================================

class ProblemGenerateRequest(BaseModel):
    """Request to generate a daily problem"""
    world_state: WorldStateModel
    force_type: Optional[ProblemType] = Field(None, description="Force specific problem type")


class ProblemRewardData(BaseModel):
    """Problem reward structure"""
    exp: int = Field(..., ge=0, description="Experience reward")
    zeny: int = Field(..., ge=0, description="Zeny reward")
    items: List[Dict[str, Any]] = Field(default_factory=list, description="Item rewards")
    special: Optional[Dict[str, Any]] = Field(None, description="Special rewards")


class ProblemSpawnData(BaseModel):
    """Problem spawn configuration"""
    map_name: str
    spawn_count: int = Field(..., ge=1, description="Number of spawns")
    mob_id: Optional[int] = Field(None, description="Monster ID")
    spawn_coords: Optional[List[Dict[str, int]]] = Field(None, description="[{x, y}]")


class ProblemResponse(BaseModel):
    """Problem agent response"""
    problem_id: int
    problem_type: ProblemType
    title: str = Field(..., max_length=200)
    description: str
    target_map: Optional[str] = None
    spawn_data: Optional[ProblemSpawnData] = None
    reward_data: ProblemRewardData
    difficulty: int = Field(..., ge=1, le=10)
    status: ProblemStatus
    created_at: datetime
    expires_at: datetime
    participation_count: int = Field(default=0, ge=0)


class ProblemParticipationRequest(BaseModel):
    """Record player participation in problem"""
    player_id: int = Field(..., ge=1)
    action_type: str = Field(..., max_length=50)
    action_data: Optional[Dict[str, Any]] = None
    contribution_score: int = Field(default=0, ge=0)


# ============================================================================
# DYNAMIC NPC AGENT MODELS
# ============================================================================

class NPCSpawnRequest(BaseModel):
    """Request to spawn dynamic NPCs"""
    map_activity: MapActivityModel
    count: int = Field(default=3, ge=1, le=5, description="Number of NPCs to spawn")


class NPCPersonalityData(BaseModel):
    """NPC personality and dialogue data"""
    personality_traits: List[str]
    backstory: str
    speech_patterns: List[str]
    quirks: Optional[List[str]] = None


class NPCQuestData(BaseModel):
    """NPC quest information"""
    quest_type: str
    objectives: List[str]
    hints: Optional[List[str]] = None


class NPCRewardData(BaseModel):
    """NPC reward structure (higher tier than problems)"""
    exp: int = Field(..., ge=0)
    zeny: int = Field(..., ge=0)
    items: List[Dict[str, Any]] = Field(default_factory=list)
    rare_items: Optional[List[Dict[str, Any]]] = None
    special_buffs: Optional[List[str]] = None
    permanent_stat: Optional[Dict[str, int]] = None


class DynamicNPCResponse(BaseModel):
    """Dynamic NPC agent response"""
    npc_id: int
    npc_type: NPCType
    npc_name: str = Field(..., max_length=100)
    personality_data: NPCPersonalityData
    spawn_map: str
    spawn_x: int = Field(..., ge=0)
    spawn_y: int = Field(..., ge=0)
    quest_data: Optional[NPCQuestData] = None
    reward_data: NPCRewardData
    status: NPCStatus
    spawned_at: datetime
    despawns_at: datetime
    interaction_count: int = Field(default=0, ge=0)


class NPCInteractionRequest(BaseModel):
    """Player-NPC interaction request"""
    player_id: int = Field(..., ge=1)
    interaction_type: str = Field(..., max_length=50)
    player_message: Optional[str] = Field(None, max_length=500)


class NPCInteractionResponse(BaseModel):
    """NPC interaction response"""
    npc_response: str
    dialogue_history: List[Dict[str, str]]
    quest_progress: Optional[Dict[str, Any]] = None
    rewards: Optional[NPCRewardData] = None


# ============================================================================
# WORLD EVENT AGENT MODELS
# ============================================================================

class EventCheckRequest(BaseModel):
    """Request to check if event thresholds are met"""
    world_state: WorldStateModel


class EventTriggerConditions(BaseModel):
    """Conditions that triggered the event"""
    condition_type: str
    threshold_value: Any
    actual_value: Any
    description: str


class EventImpactData(BaseModel):
    """Event impact on the world"""
    global_buffs: Optional[List[Dict[str, Any]]] = None
    spawn_modifications: Optional[Dict[str, Any]] = None
    economic_effects: Optional[Dict[str, float]] = None
    special_effects: Optional[List[str]] = None


class WorldEventResponse(BaseModel):
    """World event agent response"""
    event_id: int
    event_type: EventType
    title: str = Field(..., max_length=200)
    description: str
    trigger_conditions: EventTriggerConditions
    impact_data: EventImpactData
    duration_minutes: int = Field(..., ge=15, le=180)
    status: EventStatus
    started_at: datetime
    ends_at: datetime
    participation_count: int = Field(default=0, ge=0)


class EventParticipationRequest(BaseModel):
    """Record player participation in event"""
    player_id: int = Field(..., ge=1)
    contribution_score: int = Field(default=0, ge=0)


class EventTriggerRequest(BaseModel):
    """Manual event trigger request (admin)"""
    event_type: EventType
    duration_minutes: int = Field(default=60, ge=15, le=180)
    custom_impact: Optional[EventImpactData] = None


# ============================================================================
# LIST RESPONSES
# ============================================================================

class ProblemsListResponse(BaseModel):
    """List of active problems"""
    problems: List[ProblemResponse]
    total: int
    active_count: int


class NPCsListResponse(BaseModel):
    """List of active dynamic NPCs"""
    npcs: List[DynamicNPCResponse]
    total: int
    active_count: int


class EventsListResponse(BaseModel):
    """List of active world events"""
    events: List[WorldEventResponse]
    total: int
    active_count: int


# ============================================================================
# GENERIC RESPONSE
# ============================================================================

class ProceduralAgentResponse(BaseModel):
    """Generic response for procedural agents"""
    success: bool
    message: str
    data: Optional[Dict[str, Any]] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)