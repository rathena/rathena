"""
Faction system data models
"""

from pydantic import BaseModel, Field
from typing import Dict, List, Optional
from datetime import datetime
from enum import Enum


class FactionType(str, Enum):
    """Types of factions"""
    KINGDOM = "kingdom"
    GUILD = "guild"
    MERCHANT = "merchant"
    RELIGIOUS = "religious"
    MILITARY = "military"
    CRIMINAL = "criminal"
    NEUTRAL = "neutral"


class RelationshipStatus(str, Enum):
    """Relationship between factions"""
    ALLIED = "allied"
    FRIENDLY = "friendly"
    NEUTRAL = "neutral"
    UNFRIENDLY = "unfriendly"
    HOSTILE = "hostile"
    WAR = "war"


class ReputationTier(str, Enum):
    """Reputation tiers"""
    HATED = "hated"  # -1000 to -500
    HOSTILE = "hostile"  # -500 to -100
    UNFRIENDLY = "unfriendly"  # -100 to 0
    NEUTRAL = "neutral"  # 0 to 100
    FRIENDLY = "friendly"  # 100 to 500
    HONORED = "honored"  # 500 to 1000
    REVERED = "revered"  # 1000 to 2000
    EXALTED = "exalted"  # 2000+


class Faction(BaseModel):
    """Faction definition"""
    faction_id: str
    name: str
    faction_type: FactionType
    description: str
    
    # Leadership
    leader_npc_id: Optional[str] = None
    leader_name: Optional[str] = None
    
    # Territory
    capital_location: Dict[str, any] = Field(default_factory=dict)
    controlled_areas: List[str] = Field(default_factory=list)
    
    # Resources
    wealth: int = 0
    military_power: int = 0
    influence: int = 0
    
    # Relationships with other factions
    relationships: Dict[str, RelationshipStatus] = Field(default_factory=dict)
    
    # Member count
    npc_members: List[str] = Field(default_factory=list)
    player_members: int = 0
    
    # Faction-specific data
    ideology: str = "neutral"
    goals: List[str] = Field(default_factory=list)
    
    created_at: datetime = Field(default_factory=datetime.utcnow)
    last_updated: datetime = Field(default_factory=datetime.utcnow)


class PlayerReputation(BaseModel):
    """Player's reputation with a faction"""
    player_id: str
    faction_id: str
    reputation: int = 0  # -1000 to 2000+
    tier: ReputationTier = ReputationTier.NEUTRAL
    
    # History
    reputation_history: List[Dict[str, any]] = Field(default_factory=list)
    
    # Rewards unlocked
    unlocked_rewards: List[str] = Field(default_factory=list)
    
    last_updated: datetime = Field(default_factory=datetime.utcnow)


class FactionEvent(BaseModel):
    """Event involving factions"""
    event_id: str
    event_type: str  # "war", "alliance", "treaty", "conflict", "celebration"
    description: str
    
    involved_factions: List[str]
    
    # Impact
    reputation_changes: Dict[str, int] = Field(default_factory=dict)  # {faction_id: change}
    relationship_changes: Dict[str, str] = Field(default_factory=dict)  # {faction_pair: new_status}
    
    duration: Optional[int] = None  # Seconds
    severity: float = 0.5  # 0.0 to 1.0
    
    created_at: datetime = Field(default_factory=datetime.utcnow)
    expires_at: Optional[datetime] = None


class FactionQuest(BaseModel):
    """Faction-specific quest"""
    quest_id: str
    faction_id: str
    required_reputation: int = 0
    reputation_reward: int = 0
    affects_relationships: Dict[str, int] = Field(default_factory=dict)


class FactionConflict(BaseModel):
    """Ongoing conflict between factions"""
    conflict_id: str
    faction_a: str
    faction_b: str
    conflict_type: str  # "border_dispute", "resource_war", "ideological", "revenge"
    intensity: float = 0.5  # 0.0 to 1.0
    
    # Progress
    faction_a_score: int = 0
    faction_b_score: int = 0
    
    # Objectives
    objectives: List[Dict[str, any]] = Field(default_factory=list)
    
    started_at: datetime = Field(default_factory=datetime.utcnow)
    ends_at: Optional[datetime] = None


class ReputationChange(BaseModel):
    """Request to change player reputation"""
    player_id: str
    faction_id: str
    change_amount: int
    reason: str


class FactionAnalysisRequest(BaseModel):
    """Request for faction analysis"""
    faction_ids: Optional[List[str]] = None
    include_conflicts: bool = True
    include_relationships: bool = True


class FactionAnalysisResponse(BaseModel):
    """Faction analysis response"""
    success: bool
    factions: List[Faction] = Field(default_factory=list)
    conflicts: List[FactionConflict] = Field(default_factory=list)
    analysis: Optional[str] = None
    recommendations: List[str] = Field(default_factory=list)

