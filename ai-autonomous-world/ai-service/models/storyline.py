"""
Pydantic Models for AI-Driven Storyline Generator (AIDSG)
Provides request/response validation for story arc generation and management
"""

from typing import Dict, Any, List, Optional
from datetime import datetime
from pydantic import BaseModel, Field, field_validator
from enum import Enum


# ============================================================================
# ENUMS
# ============================================================================

class StoryArcStatus(str, Enum):
    """Story arc status"""
    ACTIVE = "active"
    PAUSED = "paused"
    COMPLETED = "completed"
    FAILED = "failed"


class ChapterStatus(str, Enum):
    """Chapter status"""
    PENDING = "pending"
    ACTIVE = "active"
    COMPLETED = "completed"
    FAILED = "failed"


class ChapterOutcome(str, Enum):
    """Chapter completion outcome"""
    SUCCESS = "success"
    FAILURE = "failure"
    MIXED = "mixed"
    TIMEOUT = "timeout"


class NPCRole(str, Enum):
    """Story NPC roles"""
    PROTAGONIST = "protagonist"
    ANTAGONIST = "antagonist"
    ALLY = "ally"
    MENTOR = "mentor"
    NEUTRAL = "neutral"


# ============================================================================
# WORLD STATE MODELS
# ============================================================================

class WorldStateSnapshot(BaseModel):
    """Complete world state snapshot for story generation"""
    
    # Map Activity
    most_visited_maps: List[str] = Field(default_factory=list, description="Top 10 visited maps")
    least_visited_maps: List[str] = Field(default_factory=list, description="Bottom 10 visited maps")
    map_kill_counts: Dict[str, int] = Field(default_factory=dict, description="Monster kills per map")
    
    # Player Metrics
    online_player_count: int = Field(default=0, ge=0, description="Current online players")
    player_average_level: int = Field(default=50, ge=1, le=999, description="Server average level")
    player_level_distribution: Dict[int, int] = Field(default_factory=dict, description="Level bracket -> player count")
    
    # MVP Data
    mvp_kill_frequency: Dict[str, int] = Field(default_factory=dict, description="MVP ID -> kill count")
    mvp_respawn_times: Dict[str, float] = Field(default_factory=dict, description="MVP ID -> avg respawn time (hours)")
    
    # Economy
    zeny_circulation: int = Field(default=0, ge=0, description="Total zeny in economy")
    inflation_rate: float = Field(default=0.0, description="Current inflation rate")
    top_items_in_circulation: List[str] = Field(default_factory=list, description="Most traded items")
    
    # Faction State
    dominant_faction: Optional[str] = Field(None, description="Currently dominant faction")
    faction_scores: Dict[str, int] = Field(default_factory=dict, description="Faction alignment scores")
    faction_conflicts: List[Dict[str, Any]] = Field(default_factory=list, description="Recent faction conflicts")
    
    # Karma & Problems
    global_karma: int = Field(default=0, description="World morality score (-10000 to 10000)")
    active_problems: List[Dict[str, Any]] = Field(default_factory=list, description="Current world problems")
    problem_outcomes: List[Dict[str, Any]] = Field(default_factory=list, description="Recent problem resolutions")
    
    # Storyline Context
    previous_arcs: List[Dict[str, Any]] = Field(default_factory=list, description="Past story arcs")
    current_arc_chapter: int = Field(default=0, description="Current chapter in active arc")
    player_choices_made: List[Dict[str, Any]] = Field(default_factory=list, description="Recent player decisions")
    
    # Random Seed
    timestamp: int = Field(default_factory=lambda: int(datetime.utcnow().timestamp()), description="Timestamp for randomness")
    random_seed: str = Field(default="", description="Random seed for reproducibility")

    @field_validator('global_karma')
    @classmethod
    def validate_karma_range(cls, v: int) -> int:
        """Ensure karma is in valid range"""
        if not -10000 <= v <= 10000:
            raise ValueError("global_karma must be between -10000 and 10000")
        return v


# ============================================================================
# STORY ARC MODELS
# ============================================================================

class StoryEventNPC(BaseModel):
    """NPC definition in story event"""
    npc_name: str = Field(..., max_length=100, description="NPC name")
    npc_location: str = Field(..., description="Spawn map name")
    npc_sprite: str = Field(..., description="Sprite ID or name")
    npc_role: NPCRole = Field(default=NPCRole.NEUTRAL, description="NPC role in story")
    dialogue: List[str] = Field(..., min_length=1, description="Dialogue lines")
    quest: Optional['StoryQuest'] = Field(None, description="Associated quest")


class StoryQuestObjective(BaseModel):
    """Quest objective specification"""
    objective_type: str = Field(..., description="kill, collect, talk, explore")
    description: str = Field(..., description="Human-readable objective")
    monsters: Optional[List[str]] = Field(None, description="Monster IDs to kill")
    required_items: Optional[List[Dict[str, Any]]] = Field(None, description="Items to collect")
    target_npc: Optional[str] = Field(None, description="NPC to interact with")
    target_map: Optional[str] = Field(None, description="Map to explore")
    quantity: int = Field(default=1, ge=1, description="Number required")


class StoryQuestReward(BaseModel):
    """Quest reward specification"""
    exp: int = Field(default=0, ge=0, description="Experience points")
    items: List[Dict[str, Any]] = Field(default_factory=list, description="Item rewards")
    reputation: int = Field(default=0, description="Faction reputation gain")
    special: Optional[Dict[str, Any]] = Field(None, description="Special rewards")


class StoryQuest(BaseModel):
    """Complete quest specification"""
    title: str = Field(..., max_length=200, description="Quest title")
    objective: str = Field(..., description="Main quest objective")
    objectives: List[StoryQuestObjective] = Field(default_factory=list, description="Detailed objectives")
    monsters: Optional[List[str]] = Field(None, description="Monsters involved")
    required_items: Optional[List[Dict[str, Any]]] = Field(None, description="Items needed")
    reward: StoryQuestReward = Field(..., description="Quest rewards")


class BossEventSpec(BaseModel):
    """Boss event specification"""
    spawn_map: str = Field(..., description="Map to spawn boss")
    boss_name: str = Field(..., max_length=100, description="Boss name")
    boss_id: int = Field(..., ge=1000, description="Monster ID for boss")
    modifiers: List[str] = Field(default_factory=list, description="Boss modifiers: enrage, shadow_aura, etc")
    hp_multiplier: float = Field(default=1.0, ge=1.0, le=10.0, description="HP multiplier")
    drop_bonus_percent: int = Field(default=0, ge=0, le=500, description="Drop rate bonus percentage")
    spawn_message: Optional[str] = Field(None, description="Announcement when boss spawns")


class WorldModifier(BaseModel):
    """World state modifier for story"""
    map: str = Field(..., description="Map to apply modifier")
    modifier: str = Field(..., description="Modifier type: dark_fog, blood_moon, etc")
    duration_hours: int = Field(default=24, ge=1, le=168, description="Effect duration in hours")
    effects: Optional[Dict[str, Any]] = Field(None, description="Stat modifications")


class FactionImpact(BaseModel):
    """Faction alignment impact from story"""
    faction_id: str = Field(..., description="Faction ID")
    alignment_shift: int = Field(..., description="Alignment change amount")
    reason: Optional[str] = Field(None, description="Reason for shift")


class StoryOutcome(BaseModel):
    """Story outcome specification"""
    next_chapter: int = Field(..., ge=1, description="Next chapter number")
    world_changes: List[str] = Field(..., description="Narrative world changes")
    faction_impacts: Optional[List[FactionImpact]] = Field(None, description="Faction effects")
    unlocked_content: Optional[List[str]] = Field(None, description="Unlocked features/areas")


class StoryArcSpec(BaseModel):
    """Complete story arc specification from LLM"""
    story_arc_name: str = Field(..., max_length=200, description="Arc name")
    story_arc_summary: str = Field(..., description="Arc overview")
    chapter: int = Field(default=1, ge=1, description="Starting chapter")
    duration_days: int = Field(..., ge=7, le=30, description="Expected arc duration")
    theme: Optional[str] = Field(None, description="Story theme")
    events: List[StoryEventNPC] = Field(..., min_length=1, description="Story events with NPCs")
    boss_event: Optional[BossEventSpec] = Field(None, description="Climactic boss encounter")
    world_modifiers: List[WorldModifier] = Field(default_factory=list, description="World state changes")
    faction_impact: Optional[FactionImpact] = Field(None, description="Overall faction impact")
    success_outcomes: StoryOutcome = Field(..., description="Success path")
    failure_outcomes: StoryOutcome = Field(..., description="Failure path")

    @field_validator('duration_days')
    @classmethod
    def validate_duration(cls, v: int) -> int:
        """Ensure reasonable arc duration"""
        if not 7 <= v <= 30:
            raise ValueError("Arc duration must be between 7 and 30 days")
        return v


# ============================================================================
# REQUEST/RESPONSE MODELS
# ============================================================================

class GenerateStoryRequest(BaseModel):
    """Request to generate new story arc"""
    world_state: Optional[WorldStateSnapshot] = Field(None, description="Current world state (auto-collected if None)")
    force_theme: Optional[str] = Field(None, description="Force specific theme (admin override)")
    force_faction: Optional[str] = Field(None, description="Force faction alignment")
    admin_notes: Optional[str] = Field(None, description="Admin guidance for generation")


class StoryArcResponse(BaseModel):
    """Story arc creation response"""
    arc_id: int
    arc_name: str
    arc_summary: str
    total_chapters: int
    current_chapter: int
    theme: Optional[str] = None
    dominant_faction: Optional[str] = None
    status: StoryArcStatus
    started_at: datetime
    expected_end_at: Optional[datetime] = None
    npcs_spawned: int = Field(default=0, description="Number of NPCs spawned")
    quests_created: int = Field(default=0, description="Number of quests created")


class ChapterAdvanceRequest(BaseModel):
    """Request to advance to next chapter"""
    outcome: ChapterOutcome = Field(..., description="Current chapter outcome")
    player_choices: Optional[List[Dict[str, Any]]] = Field(None, description="Player decisions made")
    completion_rate: Optional[float] = Field(None, ge=0.0, le=1.0, description="Player completion percentage")


class ChapterResponse(BaseModel):
    """Chapter information response"""
    chapter_id: int
    arc_id: int
    chapter_number: int
    chapter_title: str
    chapter_summary: str
    status: ChapterStatus
    participation_count: int = Field(default=0)
    success_rate: Optional[float] = None


class PlayerChoiceRequest(BaseModel):
    """Record player choice in story"""
    player_id: int = Field(..., ge=1)
    choice_type: str = Field(..., description="dialogue, quest_path, faction_choice, etc")
    choice_data: Dict[str, Any] = Field(..., description="Choice details")
    impact_score: int = Field(default=0, description="How much this affects story")


class HeroSelectionResponse(BaseModel):
    """Hero of the Week selection"""
    arc_id: int
    heroes: List[Dict[str, Any]] = Field(..., min_length=1, max_length=3, description="Top 3 contributors")
    selection_criteria: str = Field(..., description="How heroes were selected")
    rewards: Dict[str, Any] = Field(..., description="Hero rewards")


class StoryParticipationStats(BaseModel):
    """Player participation statistics"""
    player_id: int
    arc_id: int
    chapters_completed: int = Field(default=0, ge=0)
    quests_completed: int = Field(default=0, ge=0)
    contribution_score: int = Field(default=0, ge=0)
    is_hero: bool = Field(default=False)
    rank: Optional[int] = Field(None, description="Leaderboard rank")


class StoryHistoryEntry(BaseModel):
    """Archived story arc entry"""
    arc_id: int
    arc_name: str
    arc_summary: str
    duration_days: int
    total_participants: int = Field(default=0)
    success_rate: float = Field(..., ge=0.0, le=1.0)
    heroes: List[Dict[str, Any]] = Field(default_factory=list)
    world_changes: List[str] = Field(default_factory=list)
    completed_at: datetime


# ============================================================================
# NPC STORY INTEGRATION MODELS
# ============================================================================

class StoryNPCPersonality(BaseModel):
    """Story NPC personality traits"""
    personality_type: str = Field(..., description="archetype: hero, villain, sage, trickster, etc")
    traits: List[str] = Field(..., min_length=1, description="Character traits")
    motivations: List[str] = Field(..., description="Character motivations")
    backstory: str = Field(..., description="Character background")
    speech_patterns: List[str] = Field(default_factory=list, description="How they speak")
    relationships: Optional[Dict[str, str]] = Field(None, description="Relationships with other NPCs")


class StoryNPCResponse(BaseModel):
    """Story NPC creation response"""
    npc_id: int
    arc_id: int
    npc_name: str
    npc_role: NPCRole
    npc_sprite: str
    npc_location: str
    personality_data: StoryNPCPersonality
    is_recurring: bool = Field(default=False)
    appearances_count: int = Field(default=1, ge=1)
    quest_id: Optional[int] = Field(None, description="Associated quest ID")


class RecurringCharacterUpdate(BaseModel):
    """Update for recurring character across arcs"""
    npc_id: int
    new_appearance: int = Field(..., ge=2, description="Appearance number")
    personality_evolution: Optional[Dict[str, Any]] = Field(None, description="How character changed")
    new_dialogue: Optional[List[str]] = Field(None, description="Updated dialogue")
    relationship_changes: Optional[Dict[str, str]] = Field(None, description="Updated relationships")


# ============================================================================
# QUEST MODELS
# ============================================================================

class StoryQuestSpec(BaseModel):
    """Quest specification from story arc"""
    arc_id: int
    chapter_number: int
    quest_title: str = Field(..., max_length=200)
    quest_description: str
    objectives: List[StoryQuestObjective]
    reward: StoryQuestReward
    required_level: int = Field(default=1, ge=1, le=999)
    time_limit_hours: Optional<br> = Field(None, description="Quest time limit")
    is_repeatable: bool = Field(default=False)


class StoryQuestProgress(BaseModel):
    """Player progress on story quest"""
    quest_id: int
    player_id: int
    objectives_completed: int = Field(default=0, ge=0)
    total_objectives: int = Field(..., ge=1)
    progress_percent: float = Field(..., ge=0.0, le=100.0)
    is_completed: bool = Field(default=False)


# ============================================================================
# VALIDATION & GENERATION MODELS
# ============================================================================

class LLMGenerationRequest(BaseModel):
    """Request for LLM story generation"""
    world_state: WorldStateSnapshot
    previous_arcs: List[Dict[str, Any]] = Field(default_factory=list)
    generation_params: Dict[str, Any] = Field(default_factory=dict)
    max_retries: int = Field(default=3, ge=1, le=5)


class LLMValidationResult(BaseModel):
    """Result of LLM output validation"""
    is_valid: bool
    errors: List[str] = Field(default_factory=list)
    warnings: List[str] = Field(default_factory=list)
    sanitized_output: Optional[StoryArcSpec] = None


class VillainEvolutionSpec(BaseModel):
    """Villain evolution based on player progression"""
    villain_name: str
    previous_defeats: int = Field(default=0, ge=0)
    player_strength_level: int = Field(..., ge=1, le=999)
    evolution_tier: int = Field(..., ge=1, le=5, description="How evolved: 1=base, 5=maximum")
    new_abilities: List[str] = Field(default_factory=list)
    recruited_allies: List[str] = Field(default_factory=list)
    strategy_changes: List[str] = Field(default_factory=list)


# ============================================================================
# DASHBOARD & ADMIN MODELS
# ============================================================================

class StoryDashboardResponse(BaseModel):
    """Complete story system dashboard"""
    current_arc: Optional[StoryArcResponse] = None
    active_chapters: List[ChapterResponse] = Field(default_factory=list)
    active_npcs: List[StoryNPCResponse] = Field(default_factory=list)
    participation_stats: Dict[str, Any] = Field(default_factory=dict)
    recent_heroes: List[HeroSelectionResponse] = Field(default_factory=list)
    story_history: List[StoryHistoryEntry] = Field(default_factory=list)
    system_health: Dict[str, Any] = Field(default_factory=dict)


class AdminStoryOverride(BaseModel):
    """Admin override for story generation"""
    arc_id: Optional[int] = Field(None, description="Override existing arc")
    custom_arc_data: StoryArcSpec = Field(..., description="Custom story specification")
    bypass_validation: bool = Field(default=False, description="Skip validation checks")
    reason: str = Field(..., description="Reason for override")


class StorySystemMetrics(BaseModel):
    """Story system performance metrics"""
    total_arcs_generated: int = Field(default=0)
    total_chapters_completed: int = Field(default=0)
    total_npcs_spawned: int = Field(default=0)
    total_quests_created: int = Field(default=0)
    average_participation_rate: float = Field(default=0.0, ge=0.0, le=1.0)
    average_success_rate: float = Field(default=0.0, ge=0.0, le=1.0)
    llm_generation_time_avg: float = Field(default=0.0, ge=0.0, description="Average LLM call time (seconds)")
    llm_cost_monthly: float = Field(default=0.0, ge=0.0, description="Monthly LLM cost (USD)")


# ============================================================================
# ERROR MODELS
# ============================================================================

class StorylineError(BaseModel):
    """Storyline system error"""
    error_type: str
    error_message: str
    component: str = Field(..., description="Which component failed")
    recovery_action: Optional[str] = Field(None, description="How to recover")
    timestamp: datetime = Field(default_factory=datetime.utcnow)