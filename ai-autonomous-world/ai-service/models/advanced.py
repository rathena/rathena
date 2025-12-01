"""
Pydantic models for Advanced Systems Agents
Provides request/response validation for Adaptive Dungeon, Archaeology, and Event Chain agents
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from pydantic import BaseModel, Field, field_validator
from enum import Enum


# ============================================================================
# ENUMS
# ============================================================================

class DungeonTheme(str, Enum):
    """Dungeon theme types"""
    FIRE_DAY = "fire_day"
    ICE_DAY = "ice_day"
    UNDEAD_DAY = "undead_day"
    DEMON_DAY = "demon_day"
    EARTH_DAY = "earth_day"
    HOLY_DAY = "holy_day"
    CHAOTIC_DAY = "chaotic_day"


class DungeonStatus(str, Enum):
    """Dungeon instance status"""
    ACTIVE = "active"
    COMPLETED = "completed"
    FAILED = "failed"
    EXPIRED = "expired"


class ArtifactType(str, Enum):
    """Artifact types"""
    FOSSIL = "fossil"
    RELIC = "relic"
    TOME = "tome"
    TREASURE_MAP = "treasure_map"
    RARE_MATERIAL = "rare_material"


class ArtifactRarity(str, Enum):
    """Artifact rarity tiers"""
    COMMON = "common"
    UNCOMMON = "uncommon"
    RARE = "rare"
    MYTHIC = "mythic"


class ChainEventType(str, Enum):
    """Event chain types"""
    INVESTIGATION = "investigation"
    INVASION = "invasion"
    RESCUE = "rescue"
    TREASURE_HUNT = "treasure_hunt"
    FACTION_WAR = "faction_war"


class ChainStatus(str, Enum):
    """Event chain status"""
    ACTIVE = "active"
    COMPLETED = "completed"
    FAILED = "failed"
    EXPIRED = "expired"


class StepOutcome(str, Enum):
    """Chain step outcome"""
    SUCCESS = "success"
    FAILURE = "failure"
    PARTIAL = "partial"


# ============================================================================
# ADAPTIVE DUNGEON AGENT MODELS
# ============================================================================

class DungeonGenerateRequest(BaseModel):
    """Request to generate daily dungeon"""
    player_average_level: int = Field(..., ge=1, le=999)
    active_factions: List[str] = Field(default_factory=list)


class FloorLayoutModel(BaseModel):
    """Floor layout specification"""
    floor_number: int = Field(..., ge=1)
    room_count: int = Field(..., ge=3, le=7)
    rooms: List[Dict[str, Any]]
    is_checkpoint_floor: bool = False
    is_boss_floor: bool = False


class MonsterPoolEntry(BaseModel):
    """Monster pool entry"""
    monster_id: str
    spawn_count: int = Field(..., ge=1)
    level_modifier: float = Field(..., ge=1.0)
    is_elite: bool = False
    is_mini_boss: bool = False


class DungeonSpecResponse(BaseModel):
    """Complete dungeon specification"""
    dungeon_id: int
    theme: DungeonTheme
    floor_count: int = Field(..., ge=5, le=10)
    difficulty_rating: int = Field(..., ge=1, le=10)
    layouts: List[Dict[str, Any]]
    monster_pools: List[List[Dict[str, Any]]]
    boss_pool: List[str]
    rewards: Dict[str, Any]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class InstanceCreateRequest(BaseModel):
    """Request to create dungeon instance"""
    party_id: int = Field(..., ge=1)
    participants: List[int] = Field(..., min_length=2, max_length=12)


class InstanceCompleteRequest(BaseModel):
    """Request to complete dungeon instance"""
    participants: List[int] = Field(..., min_length=1)
    floors_cleared: int = Field(..., ge=0)
    time_taken: int = Field(..., ge=0, description="Time in seconds")


class DungeonRewardsResponse(BaseModel):
    """Dungeon completion rewards"""
    dungeon_tokens: int = Field(..., ge=0)
    theme_drops: List[str]
    reputation_gain: int = Field(..., ge=0)
    reputation_type: str
    bonus_multiplier: float = Field(..., ge=0.5, le=2.0)
    exp_bonus: int = Field(..., ge=0)
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class DungeonLeaderboardEntry(BaseModel):
    """Leaderboard entry"""
    rank: int = Field(..., ge=1)
    party_id: int
    floors_cleared: int
    time_taken: int
    timestamp: datetime


class DungeonLeaderboardResponse(BaseModel):
    """Dungeon leaderboard"""
    leaderboard: List[DungeonLeaderboardEntry]
    total_completions: int


# ============================================================================
# ARCHAEOLOGY AGENT MODELS
# ============================================================================

class MapActivityModel(BaseModel):
    """Map activity data"""
    map_activity: Dict[str, int] = Field(
        ...,
        description="Map of map_name -> visit_count"
    )


class DigSpotSpawnRequest(BaseModel):
    """Request to spawn dig spots"""
    map_activity: Dict[str, int]
    count: int = Field(default=15, ge=10, le=20)


class DigSpotResponse(BaseModel):
    """Dig spot specification"""
    spot_id: int
    map: str
    x: int = Field(..., ge=0)
    y: int = Field(..., ge=0)
    rarity: float = Field(..., ge=0.0, le=1.0)


class DigSpotActionRequest(BaseModel):
    """Request to dig a spot"""
    player_id: int = Field(..., ge=1)
    archaeology_level: int = Field(default=1, ge=1, le=10)


class ArtifactResponse(BaseModel):
    """Artifact specification"""
    artifact_id: str
    artifact_type: ArtifactType
    rarity: ArtifactRarity
    name: str
    description: str
    collection: Optional[str] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class CollectionProgressResponse(BaseModel):
    """Collection progress"""
    player_id: int
    collection_name: str
    current_count: int
    required_count: int
    progress_percent: float = Field(..., ge=0.0, le=100.0)
    is_complete: bool


class CollectionRewardResponse(BaseModel):
    """Collection completion reward"""
    title: str
    stat_bonus: Dict[str, int]
    headgear: str


class SecretLocationUnlockRequest(BaseModel):
    """Request to unlock secret location"""
    treasure_map_id: str


class SecretLocationResponse(BaseModel):
    """Secret location details"""
    location_id: str
    location_name: str
    access_map: str
    duration_minutes: int = Field(..., ge=0)
    rewards: Dict[str, Any]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ArchaeologyProgressResponse(BaseModel):
    """Player archaeology progress"""
    archaeology_level: int = Field(..., ge=1, le=10)
    total_digs: int = Field(..., ge=0)
    artifacts_found: int = Field(..., ge=0)
    collections_completed: int = Field(..., ge=0)
    secret_locations_unlocked: int = Field(..., ge=0)
    next_level_progress: float = Field(..., ge=0.0, le=1.0)


# ============================================================================
# EVENT CHAIN AGENT MODELS
# ============================================================================

class ChainStartRequest(BaseModel):
    """Request to start event chain"""
    chain_type: ChainEventType
    trigger_condition: Dict[str, Any] = Field(default_factory=dict)


class ChainStepModel(BaseModel):
    """Event chain step specification"""
    step_number: int = Field(..., ge=1)
    step_type: str
    objective: str
    dialogue: str
    success_condition: Dict[str, Any]
    npc_data: Optional[Dict[str, Any]] = None
    spawn_data: Optional[Dict[str, Any]] = None


class ChainStartResponse(BaseModel):
    """Chain start response"""
    chain_id: int
    chain_type: ChainEventType
    chain_name: str
    total_steps: int = Field(..., ge=3, le=7)
    current_step: int = 1
    first_step: Dict[str, Any]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ChainAdvanceRequest(BaseModel):
    """Request to advance chain"""
    outcome: StepOutcome


class ChainAdvanceResponse(BaseModel):
    """Chain advance response"""
    chain_id: int
    previous_step: int
    previous_outcome: StepOutcome
    current_step: int
    total_steps: int
    next_step: Dict[str, Any]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ChainParticipationRequest(BaseModel):
    """Record chain participation"""
    player_id: int = Field(..., ge=1)
    contribution: int = Field(default=0, ge=0)


class ChainCompletionResponse(BaseModel):
    """Chain completion result"""
    chain_id: int
    final_outcome: str
    success_rate: float = Field(..., ge=0.0, le=1.0)
    world_changes: Dict[str, Any]
    participant_count: int = Field(..., ge=0)
    completed_steps: int
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ActiveChainsResponse(BaseModel):
    """List of active chains"""
    chains: List[Dict[str, Any]]
    total: int


class ChainHistoryEntry(BaseModel):
    """Chain history entry"""
    chain_id: int
    chain_type: ChainEventType
    chain_name: str
    final_outcome: str
    success_rate: float
    participant_count: int
    started_at: datetime
    completed_at: Optional[datetime]


class ChainHistoryResponse(BaseModel):
    """Chain history"""
    history: List[ChainHistoryEntry]
    total: int


# ============================================================================
# GENERIC RESPONSE
# ============================================================================

class AdvancedAgentResponse(BaseModel):
    """Generic response for advanced agents"""
    success: bool
    message: str
    data: Optional[Dict[str, Any]] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)