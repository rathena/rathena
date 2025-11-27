"""
Pydantic models for Economy & Social Agents
Provides request/response validation for Merchant Economy, Karma, and Social Interaction agents
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from pydantic import BaseModel, Field, field_validator
from enum import Enum
from decimal import Decimal


# ============================================================================
# ENUMS
# ============================================================================

class EconomicIndicator(str, Enum):
    """Economic health indicators"""
    INFLATION = "inflation"
    DEFLATION = "deflation"
    ITEM_GLUT = "item_glut"
    ITEM_SCARCITY = "item_scarcity"
    BALANCED = "balanced"


class KarmaAlignment(str, Enum):
    """World karma alignment levels"""
    VERY_GOOD = "very_good"
    GOOD = "good"
    NEUTRAL = "neutral"
    EVIL = "evil"
    VERY_EVIL = "very_evil"


class SocialEventType(str, Enum):
    """Types of social events"""
    PICNIC = "picnic"
    DANCE_CHALLENGE = "dance_challenge"
    PHOTO_SPOT = "photo_spot"
    GUILD_TASK = "guild_task"
    COMMUNITY_GATHERING = "community_gathering"
    TRADING_FAIR = "trading_fair"


class SocialEventStatus(str, Enum):
    """Social event status"""
    ACTIVE = "active"
    COMPLETED = "completed"
    EXPIRED = "expired"


# ============================================================================
# MERCHANT ECONOMY AGENT MODELS
# ============================================================================

class EconomyDataModel(BaseModel):
    """Economy metrics for analysis"""
    total_zeny: int = Field(..., ge=0, description="Total zeny in circulation")
    active_players: int = Field(..., ge=1, description="Active player count")
    item_supply_index: float = Field(default=1.0, ge=0.0, description="Item supply ratio")
    item_demand_index: float = Field(default=1.0, ge=0.0, description="Item demand ratio")
    top_traded_items: Optional[List[str]] = Field(None, description="Most traded item IDs")
    farming_intensity: Optional[Dict[str, int]] = Field(None, description="Map -> farming count")


class PriceAdjustmentRequest(BaseModel):
    """Request to adjust merchant prices"""
    indicator: EconomicIndicator
    affected_items: Optional[List[str]] = Field(None, description="Specific item IDs to adjust")


class PriceAdjustmentData(BaseModel):
    """Individual price adjustment"""
    item_id: str
    old_price: int = Field(..., ge=1)
    new_price: int = Field(..., ge=1)
    change_percent: float
    adjusted_at: datetime


class PriceAdjustmentResponse(BaseModel):
    """Price adjustment response"""
    indicator: EconomicIndicator
    adjustment_percent: float
    items_adjusted: int = Field(..., ge=0)
    adjustments: List[PriceAdjustmentData]
    timestamp: datetime


class ZenySinkEventRequest(BaseModel):
    """Request to create zeny sink event"""
    severity: float = Field(..., ge=0.0, le=1.0, description="Crisis severity")


class ZenySinkOfferingModel(BaseModel):
    """Zeny sink event offering"""
    item_name: str
    price: int = Field(..., ge=1)
    quantity_limit: int = Field(..., ge=1)


class ZenySinkEventResponse(BaseModel):
    """Zeny sink event response"""
    event_id: int
    event_type: str
    event_name: str
    description: str
    target_zeny_drain: int = Field(..., ge=0)
    duration_hours: int = Field(..., ge=1, le=24)
    offerings: List[ZenySinkOfferingModel]
    started_at: datetime
    ends_at: datetime
    actual_zeny_drained: int = Field(default=0, ge=0)
    participants: int = Field(default=0, ge=0)


class EconomySnapshotResponse(BaseModel):
    """Economic snapshot"""
    snapshot_id: int
    zeny_circulation: int = Field(..., ge=0)
    zeny_per_capita: int = Field(..., ge=0)
    active_players: int = Field(..., ge=0)
    inflation_rate: float
    economic_indicator: EconomicIndicator
    top_farmed_items: Optional[List[str]] = None
    scarce_items: Optional[List[str]] = None
    timestamp: datetime


class DropRateRecommendation(BaseModel):
    """Drop rate adjustment recommendation"""
    item_id: str
    action: str = Field(..., pattern="^(reduce_drop_rate|increase_drop_rate)$")
    multiplier: float = Field(..., ge=0.1, le=2.0)


class DropRateRecommendationsResponse(BaseModel):
    """Drop rate recommendations for other agents"""
    problem_agent: List[DropRateRecommendation]
    event_agent: List[DropRateRecommendation]
    treasure_agent: List[DropRateRecommendation]
    timestamp: datetime


# ============================================================================
# KARMA AGENT MODELS
# ============================================================================

class KarmaActionModel(BaseModel):
    """Individual karma action"""
    player_id: int = Field(..., ge=1)
    action_type: str = Field(..., max_length=50)
    target: Optional[str] = Field(None, max_length=50)
    data: Optional[Dict[str, Any]] = None


class KarmaUpdateRequest(BaseModel):
    """Request to update global karma"""
    actions: List[KarmaActionModel] = Field(..., min_length=1, description="Batch of karma actions")


class KarmaEffectsModel(BaseModel):
    """Karma-based world effects"""
    alignment: KarmaAlignment
    day_length_modifier: float = Field(..., ge=0.5, le=1.5)
    heal_rate_modifier: float = Field(..., ge=0.5, le=1.5)
    special_npcs: List[str]
    atmosphere: str
    description: str
    applied_at: datetime


class GlobalKarmaResponse(BaseModel):
    """Global karma state response"""
    karma_score: int = Field(..., ge=-10000, le=10000)
    alignment: KarmaAlignment
    good_actions_count: int = Field(..., ge=0)
    evil_actions_count: int = Field(..., ge=0)
    last_shift: Optional[datetime] = None
    effects: Optional[KarmaEffectsModel] = None


class PlayerKarmaResponse(BaseModel):
    """Individual player karma"""
    player_id: int
    karma_score: int = Field(..., ge=-10000, le=10000)
    alignment: KarmaAlignment
    good_actions: int = Field(..., ge=0)
    evil_actions: int = Field(..., ge=0)
    last_action: Optional[datetime] = None


class KarmaUpdateResponse(BaseModel):
    """Karma update response"""
    old_karma: int
    new_karma: int
    karma_change: int
    old_alignment: KarmaAlignment
    new_alignment: KarmaAlignment
    alignment_changed: bool
    effects_triggered: Optional[KarmaEffectsModel] = None
    actions_processed: int = Field(..., ge=0)
    timestamp: datetime


class KarmaHistoryResponse(BaseModel):
    """Karma history over time"""
    history: List[Dict[str, Any]]
    period_start: datetime
    period_end: datetime
    total_shifts: int = Field(..., ge=0)


# ============================================================================
# SOCIAL INTERACTION AGENT MODELS
# ============================================================================

class PlayerDistributionModel(BaseModel):
    """Player distribution across maps"""
    map_distribution: Dict[str, int] = Field(..., description="Map -> player count")
    solo_players: int = Field(..., ge=0, description="Number of solo players")
    party_players: int = Field(..., ge=0, description="Number of party players")
    guild_players: int = Field(..., ge=0, description="Number of guild players")


class SocialEventSpawnRequest(BaseModel):
    """Request to spawn social events"""
    player_distribution: PlayerDistributionModel
    count: int = Field(default=5, ge=3, le=7, description="Number of events to spawn")


class SocialEventRequirements(BaseModel):
    """Social event requirements"""
    min_party_size: Optional[int] = Field(None, ge=2, le=12)
    max_party_size: Optional[int] = Field(None, ge=2, le=12)
    min_participants: Optional[int] = Field(None, ge=1)
    guild_only: Optional[bool] = None
    emote_sequence: Optional[bool] = None
    screenshot: Optional[bool] = None
    trade_activity: Optional[bool] = None


class SocialEventRewards(BaseModel):
    """Social event rewards"""
    buff: Optional[str] = None
    duration_minutes: Optional[int] = Field(None, ge=1, le=180)
    stats: Optional[Dict[str, int]] = None
    items: Optional[List[Dict[str, Any]]] = None
    title: Optional[str] = None
    trade_bonus: Optional[float] = Field(None, ge=0.0, le=1.0)
    guild_exp: Optional[int] = Field(None, ge=0)


class SocialEventResponse(BaseModel):
    """Social event response"""
    event_id: int
    event_type: SocialEventType
    event_name: str = Field(..., max_length=100)
    spawn_map: str
    spawn_x: int = Field(..., ge=0)
    spawn_y: int = Field(..., ge=0)
    requirements: SocialEventRequirements
    rewards: SocialEventRewards
    participation_count: int = Field(default=0, ge=0)
    status: SocialEventStatus
    spawned_at: datetime
    despawns_at: datetime


class SocialParticipationRequest(BaseModel):
    """Social event participation request"""
    event_id: int = Field(..., ge=1)
    participants: List[int] = Field(..., min_length=1, description="Participant player IDs")
    party_id: Optional[int] = Field(None, ge=1)
    guild_id: Optional[int] = Field(None, ge=1)


class SocialParticipationResponse(BaseModel):
    """Social participation response"""
    event_id: int
    participants: int = Field(..., ge=0)
    party_id: Optional[int] = None
    rewards_distributed: Dict[str, Any]
    timestamp: datetime


class GuildTaskGenerateRequest(BaseModel):
    """Request to generate guild tasks"""
    guild_count: int = Field(..., ge=1, description="Number of active guilds")
    average_guild_size: int = Field(default=10, ge=1, le=100)


class GuildTaskObjectives(BaseModel):
    """Guild task objectives"""
    monster_kills: Optional[bool] = None
    item_collection: Optional[bool] = None
    pvp_wins: Optional[bool] = None
    boss_defeat: Optional[bool] = None
    map_visits: Optional[bool] = None


class GuildTaskRewards(BaseModel):
    """Guild task rewards"""
    guild_exp: int = Field(..., ge=0)
    items: List[Dict[str, Any]]


class GuildTaskResponse(BaseModel):
    """Guild task response"""
    task_id: int
    guild_id: int
    task_type: str
    task_description: str
    objectives: GuildTaskObjectives
    rewards: GuildTaskRewards
    completion_threshold: int = Field(..., ge=1)
    progress: int = Field(default=0, ge=0)
    status: str = Field(default="active")
    created_at: datetime
    expires_at: datetime
    completed_at: Optional[datetime] = None


class GuildTaskProgressRequest(BaseModel):
    """Update guild task progress"""
    task_id: int = Field(..., ge=1)
    progress: int = Field(..., ge=0)


class SocialMetricsModel(BaseModel):
    """Social health metrics"""
    party_formation_rate: float = Field(..., ge=0.0, le=1.0)
    average_party_size: float = Field(..., ge=1.0, le=12.0)
    guild_activity_rate: float = Field(..., ge=0.0, le=1.0)
    trade_frequency: int = Field(..., ge=0)
    event_participation_rate: float = Field(..., ge=0.0, le=1.0)


class SocialHealthResponse(BaseModel):
    """Social health analysis response"""
    health_score: float = Field(..., ge=0.0, le=1.0)
    status: str = Field(..., pattern="^(healthy|fair|poor|unknown)$")
    party_formation_rate: float
    average_party_size: float
    guild_activity_rate: float
    trade_frequency: int
    event_participation_rate: float
    recommendations: List[str]
    timestamp: datetime


# ============================================================================
# LIST RESPONSES
# ============================================================================

class SocialEventsListResponse(BaseModel):
    """List of active social events"""
    events: List[SocialEventResponse]
    total: int
    active_count: int


class GuildTasksListResponse(BaseModel):
    """List of guild tasks"""
    tasks: List[GuildTaskResponse]
    total: int
    active_count: int


class EconomySnapshotsListResponse(BaseModel):
    """List of economy snapshots"""
    snapshots: List[EconomySnapshotResponse]
    total: int
    period_days: int


# ============================================================================
# GENERIC RESPONSE
# ============================================================================

class EconomySocialAgentResponse(BaseModel):
    """Generic response for economy/social agents"""
    success: bool
    message: str
    data: Optional[Dict[str, Any]] = None
    timestamp: datetime = Field(default_factory=datetime.utcnow)