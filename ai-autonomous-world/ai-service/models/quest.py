"""
Quest data models for dynamic quest generation
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from enum import Enum

try:
    from models.quest_trigger import QuestTrigger
except ImportError:
    from models.quest_trigger import QuestTrigger


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
    COLLECT = "collect"  # Collect items (alias for FETCH)


class DifficultyLevel(str, Enum):
    """Quest difficulty levels"""
    TRIVIAL = "trivial"
    EASY = "easy"
    NORMAL = "normal"
    HARD = "hard"
    VERY_HARD = "very_hard"
    EPIC = "epic"


class QuestDifficulty(str, Enum):
    """Quest difficulty levels (alias for DifficultyLevel)"""
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
    IN_PROGRESS = "in_progress"
    COMPLETED = "completed"
    FAILED = "failed"
    EXPIRED = "expired"


class QuestData(BaseModel):
    """Quest data for storage and transfer"""
    quest_id: str
    title: str
    description: str
    quest_type: QuestType
    difficulty: DifficultyLevel
    status: QuestStatus = QuestStatus.AVAILABLE
    min_level: int = 1
    npc_id: str = ""
    npc_name: str = ""
    start_location: str = ""
    objectives: List[Dict[str, Any]] = Field(default_factory=list)
    rewards: Dict[str, Any] = Field(default_factory=dict)
    requirements: Dict[str, Any] = Field(default_factory=dict)
    accepted_count: int = 0
    created_at: datetime = Field(default_factory=datetime.utcnow)
    accepted_at: Optional[datetime] = None
    completed_at: Optional[datetime] = None

    def can_accept(self, player_level: int, completed_quests: List[str] = None) -> tuple:
        """Check if player can accept this quest"""
        if self.status != QuestStatus.AVAILABLE:
            return False, "Quest is not available"
        if player_level < self.min_level:
            return False, f"Minimum level required: {self.min_level}"
        return True, ""

    def accept(self):
        """Accept the quest"""
        self.status = QuestStatus.IN_PROGRESS
        self.accepted_count += 1
        self.accepted_at = datetime.utcnow()

    def complete(self):
        """Complete the quest"""
        self.status = QuestStatus.COMPLETED
        self.completed_at = datetime.utcnow()

    def add_objective(self, objective: 'QuestObjective'):
        """Add an objective to the quest"""
        self.objectives.append(objective.dict())

    def check_completion(self) -> bool:
        """Check if all objectives are completed"""
        return all(obj.get("completed", False) for obj in self.objectives)


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

    @property
    def is_completed(self) -> bool:
        """Check if objective is completed"""
        return self.completed or self.current_count >= self.required_count

    def update_progress(self, amount: int = 1):
        """Update progress towards completion"""
        self.current_count += amount
        if self.current_count >= self.required_count:
            self.completed = True


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

    # Quest triggers and sequences
    triggers: List[QuestTrigger] = Field(
        default_factory=list,
        description="Triggers that can activate this quest"
    )
    is_secret_quest: bool = Field(
        default=False,
        description="Whether this is a hidden/secret quest"
    )
    prerequisite_quests: List[str] = Field(
        default_factory=list,
        description="Quest IDs that must be completed before this quest"
    )
    next_in_sequence: Optional[str] = Field(
        None,
        description="Next quest ID in sequence (for quest chains)"
    )
    relationship_unlock_threshold: Optional[float] = Field(
        None,
        description="Minimum relationship level to unlock (-100 to 100)"
    )

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

