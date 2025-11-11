"""
NPC-to-NPC Relationship Models
Tracks relationships, interactions, and information sharing between NPCs
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field


class NPCRelationship(BaseModel):
    """Represents a relationship between two NPCs"""
    npc_id_1: str = Field(..., description="First NPC ID (alphabetically first)")
    npc_id_2: str = Field(..., description="Second NPC ID (alphabetically second)")
    relationship_type: str = Field(
        default="neutral",
        description="Type: friendship, rivalry, cooperation, romantic, family, mentor, enemy"
    )
    relationship_value: float = Field(
        default=0.0,
        ge=-100.0,
        le=100.0,
        description="Relationship strength: -100 (hostile) to 100 (best friends)"
    )
    trust_level: float = Field(
        default=50.0,
        ge=0.0,
        le=100.0,
        description="Trust level: 0 (no trust) to 100 (complete trust)"
    )
    interaction_count: int = Field(default=0, description="Total number of interactions")
    last_interaction: Optional[datetime] = Field(None, description="Last interaction timestamp")
    created_at: datetime = Field(default_factory=datetime.utcnow)
    updated_at: datetime = Field(default_factory=datetime.utcnow)
    metadata: Dict[str, Any] = Field(default_factory=dict, description="Additional relationship data")


class NPCInteraction(BaseModel):
    """Represents a single interaction between two NPCs"""
    interaction_id: str = Field(..., description="Unique interaction ID")
    npc_id_1: str = Field(..., description="First NPC ID")
    npc_id_2: str = Field(..., description="Second NPC ID")
    interaction_type: str = Field(
        ...,
        description="Type: conversation, trade, cooperation, conflict, information_share"
    )
    location: Dict[str, Any] = Field(..., description="Location where interaction occurred")
    duration: Optional[float] = Field(None, description="Interaction duration in seconds")
    outcome: str = Field(..., description="Outcome: positive, negative, neutral")
    relationship_change: float = Field(
        default=0.0,
        description="Change in relationship value from this interaction"
    )
    information_shared: Optional[List[str]] = Field(
        None,
        description="Information shared during interaction (gossip, rumors, quest hints)"
    )
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    metadata: Dict[str, Any] = Field(default_factory=dict)


class SharedInformation(BaseModel):
    """Information shared between NPCs"""
    info_id: str = Field(..., description="Unique information ID")
    info_type: str = Field(
        ...,
        description="Type: gossip, rumor, quest_hint, warning, news, secret"
    )
    content: str = Field(..., description="Information content")
    source_npc_id: str = Field(..., description="NPC who originally had this information")
    current_holder_npc_ids: List[str] = Field(
        default_factory=list,
        description="NPCs who currently know this information"
    )
    reliability: float = Field(
        default=1.0,
        ge=0.0,
        le=1.0,
        description="Information reliability: 0 (false) to 1 (true)"
    )
    importance: float = Field(
        default=0.5,
        ge=0.0,
        le=1.0,
        description="Information importance: 0 (trivial) to 1 (critical)"
    )
    expiry: Optional[datetime] = Field(None, description="When information becomes outdated")
    created_at: datetime = Field(default_factory=datetime.utcnow)
    spread_count: int = Field(default=0, description="Number of times information was shared")
    metadata: Dict[str, Any] = Field(default_factory=dict)


class NPCProximityEvent(BaseModel):
    """Event triggered when NPCs are in proximity"""
    event_id: str = Field(..., description="Unique event ID")
    npc_id_1: str = Field(..., description="First NPC ID")
    npc_id_2: str = Field(..., description="Second NPC ID")
    distance: float = Field(..., description="Distance between NPCs in cells")
    location: Dict[str, Any] = Field(..., description="Location of proximity event")
    should_interact: bool = Field(
        default=False,
        description="Whether NPCs should interact based on relationship/context"
    )
    interaction_probability: float = Field(
        default=0.5,
        ge=0.0,
        le=1.0,
        description="Probability of interaction occurring"
    )
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    metadata: Dict[str, Any] = Field(default_factory=dict)


class NPCInteractionContext(BaseModel):
    """Context for NPC-to-NPC interaction decision making"""
    npc_id_1: str
    npc_id_2: str
    npc_1_personality: Dict[str, Any]
    npc_2_personality: Dict[str, Any]
    current_relationship: Optional[NPCRelationship]
    recent_interactions: List[NPCInteraction]
    shared_information: List[SharedInformation]
    proximity_distance: float
    location: Dict[str, Any]
    world_state: Dict[str, Any]
    npc_1_goals: List[str] = Field(default_factory=list)
    npc_2_goals: List[str] = Field(default_factory=list)
    npc_1_current_activity: Optional[str] = None
    npc_2_current_activity: Optional[str] = None


class NPCInteractionDecision(BaseModel):
    """Decision result for NPC-to-NPC interaction"""
    should_interact: bool
    interaction_type: Optional[str] = None
    interaction_duration: Optional[float] = None
    dialogue: Optional[str] = None
    information_to_share: Optional[List[str]] = None
    relationship_change: float = 0.0
    trust_change: float = 0.0
    next_action: Optional[str] = None
    reasoning: str = Field(..., description="Reasoning chain for this decision")
    complexity_tier: int = Field(
        default=4,
        ge=1,
        le=4,
        description="Decision complexity tier (1=rule-based, 4=full LLM)"
    )
    cached: bool = Field(default=False, description="Whether decision was cached")

