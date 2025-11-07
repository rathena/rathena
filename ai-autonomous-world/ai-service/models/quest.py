"""
Quest data models for dynamic quest generation
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from enum import Enum


class QuestType(str, Enum):
    """Types of quests"""
    FETCH = "fetch"  # Collect items
    KILL = "kill"  # Defeat monsters
    ESCORT = "escort"  # Protect NPC
    DELIVERY = "delivery"  # Deliver items
    EXPLORE = "explore"  # Discover locations
    DIALOGUE = "dialogue"  # Talk to NPCs
    CRAFT = "craft"  # Create items
    INVESTIGATE = "investigate"  # Solve mystery


class QuestDifficulty(str, Enum):
    """Quest difficulty levels"""
    TRIVIAL = "trivial"
    EASY = "easy"
    NORMAL = "normal"
    HARD = "hard"
    VERY_HARD = "very_hard"
    EPIC = "epic"


class QuestStatus(str, Enum):
    """Quest status"""
    AVAILABLE = "available"
    ACTIVE = "active"
    COMPLETED = "completed"
    FAILED = "failed"
    EXPIRED = "expired"


class QuestObjective(BaseModel):
    """Individual quest objective"""
    objective_id: str
    description: str
    objective_type: str  # kill, collect, talk, explore, etc.
    target: str  # Monster name, item name, NPC name, location, etc.
    required_count: int = 1
    current_count: int = 0
    completed: bool = False
    optional: bool = False


class QuestReward(BaseModel):
    """Quest rewards"""
    experience: int = 0
    base_level_exp: int = 0
    job_level_exp: int = 0
    zeny: int = 0  # In-game currency
    items: List[Dict[str, Any]] = Field(default_factory=list)  # [{"item_id": 501, "amount": 10}]
    reputation: Dict[str, int] = Field(default_factory=dict)  # {"faction_name": reputation_change}
    special_rewards: List[str] = Field(default_factory=list)  # Titles, unlocks, etc.


class QuestRequirements(BaseModel):
    """Quest requirements to accept"""
    min_level: int = 1
    max_level: Optional[int] = None
    required_class: Optional[List[str]] = None
    required_quests: List[str] = Field(default_factory=list)  # Quest IDs
    required_items: List[Dict[str, int]] = Field(default_factory=list)  # [{"item_id": amount}]
    required_reputation: Dict[str, int] = Field(default_factory=dict)  # {"faction": min_rep}


class Quest(BaseModel):
    """Complete quest definition"""
    quest_id: str
    title: str
    description: str
    quest_type: QuestType
    difficulty: QuestDifficulty
    status: QuestStatus = QuestStatus.AVAILABLE
    
    # Quest giver
    giver_npc_id: str
    giver_npc_name: str
    
    # Objectives
    objectives: List[QuestObjective]
    
    # Requirements
    requirements: QuestRequirements = Field(default_factory=QuestRequirements)
    
    # Rewards
    rewards: QuestReward = Field(default_factory=QuestReward)
    
    # Time limits
    time_limit: Optional[int] = None  # Seconds
    expires_at: Optional[datetime] = None
    
    # Story and context
    story_context: str = ""
    success_message: str = "Quest completed!"
    failure_message: str = "Quest failed."
    
    # Metadata
    created_at: datetime = Field(default_factory=datetime.utcnow)
    accepted_at: Optional[datetime] = None
    completed_at: Optional[datetime] = None
    
    # Dynamic generation metadata
    generated_by_ai: bool = False
    generation_context: Dict[str, Any] = Field(default_factory=dict)
    
    # Repeatable
    repeatable: bool = False
    cooldown: Optional[int] = None  # Seconds before can repeat


class QuestGenerationRequest(BaseModel):
    """Request to generate a new quest"""
    npc_id: str
    npc_name: str
    npc_class: str
    player_level: int
    player_class: str
    world_state: Dict[str, Any] = Field(default_factory=dict)
    recent_events: List[Dict[str, Any]] = Field(default_factory=list)
    player_history: Dict[str, Any] = Field(default_factory=dict)
    
    # Generation preferences
    preferred_type: Optional[QuestType] = None
    preferred_difficulty: Optional[QuestDifficulty] = None
    max_objectives: int = 3


class QuestGenerationResponse(BaseModel):
    """Response from quest generation"""
    success: bool
    quest: Optional[Quest] = None
    error: Optional[str] = None
    generation_reasoning: Optional[str] = None


class QuestProgressUpdate(BaseModel):
    """Update quest progress"""
    quest_id: str
    player_id: str
    objective_id: str
    progress_amount: int = 1


class QuestCompletionRequest(BaseModel):
    """Request to complete a quest"""
    quest_id: str
    player_id: str
    completion_data: Dict[str, Any] = Field(default_factory=dict)

