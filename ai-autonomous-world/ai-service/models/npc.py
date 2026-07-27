"""
NPC-related data models
"""

from datetime import datetime
from typing import Optional, Dict, Any, List
from pydantic import BaseModel, Field
from enum import Enum


class Emotion(str, Enum):
    """NPC emotional states"""
    HAPPY = "happy"
    SAD = "sad"
    ANGRY = "angry"
    FEARFUL = "fearful"
    SURPRISED = "surprised"
    DISGUSTED = "disgusted"
    NEUTRAL = "neutral"
    EXCITED = "excited"
    CALM = "calm"
    ANXIOUS = "anxious"
    CONFIDENT = "confident"
    CONFUSED = "confused"
    GRATEFUL = "grateful"
    HOPEFUL = "hopeful"
    LONELY = "lonely"
    PROUD = "proud"
    ASHAMED = "ashamed"
    JEALOUS = "jealous"
    LOVING = "loving"
    TRUSTING = "trusting"


class EmotionalState(BaseModel):
    """NPC emotional state with intensity and mood"""
    current_emotion: Emotion = Field(Emotion.NEUTRAL, description="Current emotion")
    emotion_intensity: float = Field(0.5, ge=0.0, le=1.0, description="Emotion intensity")
    mood: str = Field("neutral", description="Overall mood")
    emotional_triggers: List[str] = Field(default_factory=list, description="Recent emotional triggers")
    last_updated: datetime = Field(default_factory=datetime.utcnow, description="Last update timestamp")


class Personality(BaseModel):
    """NPC personality traits"""
    openness: float = Field(0.5, ge=0.0, le=1.0, description="Openness to experience")
    conscientiousness: float = Field(0.5, ge=0.0, le=1.0, description="Conscientiousness")
    extraversion: float = Field(0.5, ge=0.0, le=1.0, description="Extraversion")
    agreeableness: float = Field(0.5, ge=0.0, le=1.0, description="Agreeableness")
    neuroticism: float = Field(0.5, ge=0.0, le=1.0, description="Neuroticism")
    moral_alignment: str = Field("true_neutral", description="Moral alignment")
    quirks: List[str] = Field(default_factory=list, description="Personality quirks")


class NPCPhysical(BaseModel):
    """NPC physical characteristics"""
    race: str = Field("Human", description="NPC race")
    gender: str = Field("", description="NPC gender")
    age: int = Field(30, description="NPC age")
    height_cm: float = Field(170.0, description="NPC height in cm")
    weight_kg: float = Field(70.0, description="NPC weight in kg")
    appearance: str = Field("", description="Physical appearance description")


class NPCBackground(BaseModel):
    """NPC background story and physical characteristics"""
    history: str = Field("", description="Procedurally generated history")
    skills: List[str] = Field(default_factory=list, description="Social, economic, creative, combat skills")
    appearance: Dict[str, Any] = Field(default_factory=dict, description="Physical appearance, age, health status")
    occupation: str = Field("", description="NPC occupation")
    social_class: str = Field("commoner", description="NPC social class")
    hometown: str = Field("", description="NPC hometown")


class NPCState(BaseModel):
    """Complete NPC state including all subsystems"""
    npc_id: str = Field(..., description="Unique NPC identifier")
    name: str = Field(..., description="NPC name")
    title: str = Field("", description="NPC title")
    npc_class: str = Field("generic", description="NPC class/type")
    level: int = Field(1, ge=1, le=999, description="NPC level")
    location: str = Field("prontera", description="Current map location")
    position_x: float = Field(0.0, description="X coordinate")
    position_y: float = Field(0.0, description="Y coordinate")
    is_active: bool = Field(True, description="Whether NPC is active")
    faction_id: Optional[str] = Field(None, description="Faction ID")
    personality: Optional[Personality] = Field(None, description="NPC personality")
    emotional_state: Optional[EmotionalState] = Field(None, description="NPC emotional state")
    background: Optional[NPCBackground] = Field(None, description="NPC background")
    physical: Optional[NPCPhysical] = Field(None, description="NPC physical characteristics")
    reputation: Dict[str, float] = Field(default_factory=dict, description="Reputation with entities")
    interaction_count: int = Field(0, description="Total interaction count")
    last_interaction: Optional[datetime] = Field(None, description="Last interaction timestamp")

    def record_interaction(self):
        """Record an interaction"""
        self.interaction_count += 1
        self.last_interaction = datetime.utcnow()

    def update_emotion(self, emotion: Emotion, intensity: float = 0.5):
        """Update NPC emotional state"""
        if self.emotional_state is None:
            self.emotional_state = EmotionalState()
        self.emotional_state.current_emotion = emotion
        self.emotional_state.emotion_intensity = intensity
        self.emotional_state.last_updated = datetime.utcnow()

    def add_reputation(self, entity_id: str, value: float):
        """Add reputation for an entity"""
        self.reputation[entity_id] = value


# Alias for backward compatibility
Background = NPCBackground


class NPCPosition(BaseModel):
    """NPC position in the game world"""
    map: str = Field(..., description="Map name")
    x: int = Field(..., description="X coordinate")
    y: int = Field(..., description="Y coordinate")


class NPCPersonality(BaseModel):
    """NPC personality traits (Big Five + custom traits + dynamic alignment)"""
    openness: float = Field(0.5, ge=0.0, le=1.0, description="Openness to experience")
    conscientiousness: float = Field(0.5, ge=0.0, le=1.0, description="Conscientiousness")
    extraversion: float = Field(0.5, ge=0.0, le=1.0, description="Extraversion")
    agreeableness: float = Field(0.5, ge=0.0, le=1.0, description="Agreeableness")
    neuroticism: float = Field(0.5, ge=0.0, le=1.0, description="Neuroticism")
    ambition: float = Field(0.5, ge=0.0, le=1.0, description="Ambition")
    courage: float = Field(0.5, ge=0.0, le=1.0, description="Courage")
    compassion: float = Field(0.5, ge=0.0, le=1.0, description="Compassion")
    cunning: float = Field(0.5, ge=0.0, le=1.0, description="Cunning")
    loyalty: float = Field(0.5, ge=0.0, le=1.0, description="Loyalty")
    moral_alignment: str = Field("neutral", description="Dynamic alignment: e.g. lawful_good, neutral, chaotic_evil")
    alignment_spectrum: Dict[str, float] = Field(default_factory=dict, description="Altruism, Lawful, Honest, etc.")
    quirks: List[str] = Field(default_factory=list, description="Personality quirks")
    background: Optional[NPCBackground] = Field(None, description="Background story and physical characteristics")

class NPCGoalState(BaseModel):
    """NPC goal and motivation system (Maslow-style)"""
    survival: float = 1.0
    security: float = 1.0
    social: float = 1.0
    esteem: float = 1.0
    self_actualization: float = 1.0
    short_term_goals: List[str] = Field(default_factory=list)
    long_term_goals: List[str] = Field(default_factory=list)
    current_goal: Optional[str] = None
    goal_conflicts: List[str] = Field(default_factory=list)
    goal_history: List[Dict[str, Any]] = Field(default_factory=list)

class NPCEmotionState(BaseModel):
    """NPC emotional system"""
    current_emotion: str = "neutral"
    mood: str = "neutral"
    emotional_triggers: List[str] = Field(default_factory=list)
    emotional_regulation: Dict[str, Any] = Field(default_factory=dict)
    emotional_contagion: List[str] = Field(default_factory=list)
    last_emotion_update: datetime = Field(default_factory=datetime.utcnow)

class NPCMemoryState(BaseModel):
    """NPC memory system (episodic, semantic, procedural, emotional, reflective)"""
    episodic: List[Dict[str, Any]] = Field(default_factory=list)
    semantic: List[Dict[str, Any]] = Field(default_factory=list)
    procedural: List[Dict[str, Any]] = Field(default_factory=list)
    emotional: List[Dict[str, Any]] = Field(default_factory=list)
    reflective: List[Dict[str, Any]] = Field(default_factory=list)
    memory_decay: float = 0.01
    memory_distortion: float = 0.01
    memory_salience: Dict[str, float] = Field(default_factory=dict)
    last_memory_update: datetime = Field(default_factory=datetime.utcnow)

class NPCRegisterRequest(BaseModel):
    """Request to register an NPC with the AI service"""
    npc_id: str = Field(..., description="Unique NPC identifier")
    name: str = Field(..., description="NPC name")
    npc_class: str = Field(..., description="NPC class/type")
    level: int = Field(..., ge=1, le=999, description="NPC level")
    position: NPCPosition = Field(..., description="NPC position")
    personality: Optional[NPCPersonality] = Field(None, description="NPC personality traits")
    goal_state: Optional[NPCGoalState] = Field(None, description="NPC goal and motivation state")
    emotion_state: Optional[NPCEmotionState] = Field(None, description="NPC emotional state")
    memory_state: Optional[NPCMemoryState] = Field(None, description="NPC memory state")
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

class NPCMovementCapabilities(BaseModel):
    """NPC movement capabilities and restrictions"""
    can_move: bool = Field(True, description="Whether NPC can move autonomously")
    has_walking_animation: bool = Field(True, description="Whether NPC sprite has walking animation")
    movement_speed: float = Field(1.0, ge=0.1, le=5.0, description="Movement speed multiplier")
    max_wander_distance: int = Field(10, ge=1, le=50, description="Maximum wander distance from spawn")
    patrol_route: Optional[List[NPCPosition]] = Field(None, description="Predefined patrol route")
    home_position: Optional[NPCPosition] = Field(None, description="Home/spawn position for return behavior")
    # Movement boundary configuration
    movement_mode: str = Field("radius_restricted", description="Movement mode: global, map_restricted, radius_restricted, disabled")
    movement_radius: int = Field(10, ge=0, le=100, description="Movement radius in tiles from spawn point (for radius_restricted mode, 0 for disabled)")
    spawn_point: Optional[NPCPosition] = Field(None, description="Spawn point for radius calculation")

class MovementActionData(BaseModel):
    """Data for movement actions"""
    movement_type: str = Field(..., description="Movement type: wander/patrol/goal_directed/social/retreat/return_home/exploration")
    target_position: Optional[NPCPosition] = Field(None, description="Target position to move to")
    target_entity_id: Optional[str] = Field(None, description="Target entity ID for social movement")
    movement_reason: Optional[str] = Field(None, description="Reason for movement decision")
    path: Optional[List[NPCPosition]] = Field(None, description="Calculated path (if pathfinding used)")
    duration_estimate: Optional[float] = Field(None, description="Estimated movement duration in seconds")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")
