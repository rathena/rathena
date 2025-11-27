"""
Pydantic models for Progression Agents
Provides request/response validation for Faction, Reputation, and Dynamic Boss agents
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from pydantic import BaseModel, Field, field_validator
from enum import Enum


# ============================================================================
# ENUMS
# ============================================================================

class FactionType(str, Enum):
    """World faction types"""
    RUNE_ALLIANCE = "rune_alliance"
    SHADOW_CULT = "shadow_cult"
    NATURE_SPIRITS = "nature_spirits"


class ReputationType(str, Enum):
    """Reputation source types"""
    WORLD_GUARDIAN = "world_guardian"
    EXPLORER = "explorer"
    PROBLEM_SOLVER = "problem_solver"
    EVENT_PARTICIPANT = "event_participant"
    FACTION_LOYALIST = "faction_loyalist"


class BossSpawnReason(str, Enum):
    """Reasons for boss spawns"""
    ANTI_FARM = "anti_farm"
    TREASURE_GUARD = "treasure_guard"
    PROBLEM_ESCALATION = "problem_escalation"
    FACTION_CHAMPION = "faction_champion"
    RANDOM_ENCOUNTER = "random_encounter"


class BossStatus(str, Enum):
    """Dynamic boss status"""
    ACTIVE = "active"
    DEFEATED = "defeated"
    DESPAWNED = "despawned"


# ============================================================================
# FACTION AGENT MODELS
# ============================================================================

class FactionActionModel(BaseModel):
    """Faction action data"""
    player_id: int = Field(..., ge=1, description="Player ID")
    faction_id: FactionType
    action_type: str = Field(..., max_length=50, description="Action type")
    alignment_change: int = Field(default=0, description="Alignment shift amount")
    action_data: Optional[Dict[str, Any]] = None


class FactionAlignmentResponse(BaseModel):
    """World faction alignment response"""
    factions: List[Dict[str, Any]]
    dominant_faction: Optional[FactionType] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class PlayerFactionReputationResponse(BaseModel):
    """Player's reputation with a faction"""
    player_id: int
    faction_id: FactionType
    reputation: int = Field(..., ge=0, le=10000)
    reputation_tier: int = Field(..., ge=0, le=5)
    tier_name: str
    actions_completed: int = Field(default=0, ge=0)
    last_action: Optional[datetime] = None


class FactionRewardsResponse(BaseModel):
    """Faction rewards at a tier"""
    tier: int = Field(..., ge=0, le=5)
    tier_name: str
    cosmetics: List[str] = Field(default_factory=list)
    buffs: List[str] = Field(default_factory=list)
    items: List[str] = Field(default_factory=list)
    skills: List[str] = Field(default_factory=list)
    titles: List[str] = Field(default_factory=list)


class FactionActionRequest(BaseModel):
    """Request to record faction action"""
    faction_id: FactionType
    action_type: str = Field(..., max_length=50)
    action_data: Optional[Dict[str, Any]] = None


class FactionUpdateRequest(BaseModel):
    """Request to update faction alignment"""
    actions: List[FactionActionModel] = Field(..., min_length=1)


# ============================================================================
# REPUTATION AGENT MODELS
# ============================================================================

class ReputationGainRequest(BaseModel):
    """Request to record reputation gain"""
    reputation_type: ReputationType
    amount: int = Field(..., ge=1, le=1000, description="Reputation amount")
    source: str = Field(..., max_length=100, description="Source of reputation")
    source_id: Optional[int] = None


class PlayerInfluenceResponse(BaseModel):
    """Player's total influence response"""
    player_id: int
    total_influence: int = Field(..., ge=0)
    tier: int = Field(..., ge=0, le=5)
    tier_name: str
    next_tier_progress: float = Field(..., ge=0.0, le=1.0)
    breakdown: Dict[str, int]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class UnlockedBenefitsResponse(BaseModel):
    """Player's unlocked benefits"""
    tier: int
    tier_name: str
    shops: List[Dict[str, Any]] = Field(default_factory=list)
    titles: List[Dict[str, Any]] = Field(default_factory=list)
    headgears: List[Dict[str, Any]] = Field(default_factory=list)
    passives: List[Dict[str, Any]] = Field(default_factory=list)
    services: List[Dict[str, Any]] = Field(default_factory=list)
    auras: List[Dict[str, Any]] = Field(default_factory=list)


class TierProgressionResponse(BaseModel):
    """Tier progression notification"""
    advanced: bool
    old_tier: int
    old_tier_name: str
    new_tier: int
    new_tier_name: str
    new_benefits: List[str]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ReputationLeaderboardEntry(BaseModel):
    """Leaderboard entry"""
    rank: int = Field(..., ge=1)
    player_id: int
    total_influence: int
    tier: int
    tier_name: str


class ReputationLeaderboardResponse(BaseModel):
    """Reputation leaderboard"""
    leaderboard: List[ReputationLeaderboardEntry]
    total_players: int
    timestamp: datetime = Field(default_factory=datetime.utcnow)


# ============================================================================
# DYNAMIC BOSS AGENT MODELS
# ============================================================================

class BossStatsModel(BaseModel):
    """Boss statistics"""
    hp: int = Field(..., ge=1000)
    atk: int = Field(..., ge=100)
    def_: int = Field(..., ge=50, alias="def")
    
    class Config:
        populate_by_name = True


class BossRewardModel(BaseModel):
    """Boss reward structure"""
    exp: int = Field(..., ge=0)
    zeny: int = Field(..., ge=0)
    items: List[Dict[str, Any]] = Field(default_factory=list)
    reputation: int = Field(default=0, ge=0)
    special_drops: List[Dict[str, Any]] = Field(default_factory=list)


class BossSpecModel(BaseModel):
    """Complete boss specification"""
    boss_type: str
    boss_name: str
    spawn_reason: BossSpawnReason
    spawn_map: str
    spawn_x: int = Field(..., ge=0)
    spawn_y: int = Field(..., ge=0)
    base_stats: BossStatsModel
    scaled_stats: BossStatsModel
    difficulty_rating: int = Field(..., ge=1, le=10)
    skills: List[str] = Field(default_factory=list)
    reward_multiplier: float = Field(default=1.0, ge=0.5, le=3.0)
    rewards: BossRewardModel
    status: BossStatus = BossStatus.ACTIVE


class DynamicBossResponse(BaseModel):
    """Dynamic boss agent response"""
    boss_id: int
    boss_type: str
    boss_name: str
    spawn_reason: BossSpawnReason
    spawn_map: str
    spawn_x: int
    spawn_y: int
    base_stats: Dict[str, int]
    scaled_stats: Dict[str, int]
    difficulty_rating: int
    reward_multiplier: float
    status: BossStatus
    spawned_at: datetime
    defeated_at: Optional[datetime] = None


class BossSpawnRequest(BaseModel):
    """Request to spawn a boss"""
    spawn_reason: BossSpawnReason
    spawn_map: str
    difficulty_modifier: float = Field(default=1.0, ge=0.5, le=3.0)


class BossEncounterRequest(BaseModel):
    """Boss encounter record request"""
    player_id: int = Field(..., ge=1)
    damage_dealt: int = Field(default=0, ge=0)
    deaths: int = Field(default=0, ge=0)
    time_participated: int = Field(default=0, ge=0, description="Time in seconds")


class BossDefeatRequest(BaseModel):
    """Boss defeat request"""
    participants: List[int] = Field(..., min_length=1, description="Player IDs")
    time_to_kill: int = Field(..., ge=0, description="Time to kill in seconds")


class BossHistoryEntry(BaseModel):
    """Boss spawn history entry"""
    boss_type: str
    spawn_reason: BossSpawnReason
    spawn_map: str
    difficulty_rating: int
    participants_count: int
    time_to_defeat: Optional[int] = None
    was_defeated: bool
    spawned_at: datetime


class BossHistoryResponse(BaseModel):
    """Boss history response"""
    history: List[BossHistoryEntry]
    total: int
    avg_time_to_defeat: Optional[float] = None


class WorldStateForBossModel(BaseModel):
    """World state for boss evaluation"""
    avg_player_level: int = Field(..., ge=1, le=999)
    online_players: int = Field(..., ge=0)
    map_activity: Dict[str, int] = Field(default_factory=dict)
    monster_kills: Dict[str, int] = Field(default_factory=dict)


# ============================================================================
# LIST RESPONSES
# ============================================================================

class FactionsListResponse(BaseModel):
    """List of faction alignments"""
    factions: List[Dict[str, Any]]
    dominant_faction: Optional[FactionType] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class PlayerReputationsResponse(BaseModel):
    """Player's reputation with all factions"""
    player_id: int
    reputations: List[PlayerFactionReputationResponse]
    timestamp: datetime = Field(default_factory=datetime.utcnow)


class ActiveBossesResponse(BaseModel):
    """List of active bosses"""
    bosses: List[DynamicBossResponse]
    total: int
    active_count: int


# ============================================================================
# GENERIC RESPONSE
# ============================================================================

class ProgressionAgentResponse(BaseModel):
    """Generic response for progression agents"""
    success: bool
    message: str
    data: Optional[Dict[str, Any]] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)