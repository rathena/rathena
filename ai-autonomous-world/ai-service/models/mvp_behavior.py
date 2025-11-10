"""
MVP (Boss Monster) AI Behavior Models
Defines AI-driven behavior patterns for MVP/Boss monsters
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from enum import Enum


class MVPBehaviorState(str, Enum):
    """MVP behavior states"""
    IDLE = "idle"
    PATROL = "patrol"
    AGGRESSIVE = "aggressive"
    DEFENSIVE = "defensive"
    ENRAGED = "enraged"
    RETREATING = "retreating"
    SUMMONING = "summoning"
    CHANNELING = "channeling"


class CombatPattern(str, Enum):
    """Combat pattern types"""
    MELEE_RUSH = "melee_rush"
    RANGED_KITE = "ranged_kite"
    AOE_SPAM = "aoe_spam"
    SINGLE_TARGET = "single_target"
    SUMMON_MINIONS = "summon_minions"
    BUFF_DEBUFF = "buff_debuff"
    HIT_AND_RUN = "hit_and_run"
    TANK_AND_SPANK = "tank_and_spank"


class PlayerStrategy(BaseModel):
    """Detected player strategy"""
    strategy_type: str = Field(description="Type of strategy detected")
    player_count: int = Field(description="Number of players")
    avg_player_level: float
    primary_damage_type: str = Field(description="physical, magical, ranged")
    has_healer: bool = False
    has_tank: bool = False
    formation: str = Field(default="scattered", description="Player formation pattern")
    detected_at: datetime = Field(default_factory=datetime.utcnow)


class MVPPersonality(BaseModel):
    """MVP personality traits affecting behavior"""
    aggression: float = Field(ge=0.0, le=1.0, default=0.7)
    intelligence: float = Field(ge=0.0, le=1.0, default=0.5)
    adaptability: float = Field(ge=0.0, le=1.0, default=0.6)
    patience: float = Field(ge=0.0, le=1.0, default=0.4)
    pride: float = Field(ge=0.0, le=1.0, default=0.8)
    cunning: float = Field(ge=0.0, le=1.0, default=0.5)


class MVPCombatMemory(BaseModel):
    """Memory of combat encounters"""
    encounter_id: str
    player_ids: List[str]
    player_strategies: List[PlayerStrategy]
    combat_duration: float = Field(description="Duration in seconds")
    damage_taken: int
    damage_dealt: int
    skills_used: Dict[str, int] = Field(default_factory=dict)
    effective_patterns: List[CombatPattern] = Field(default_factory=list)
    ineffective_patterns: List[CombatPattern] = Field(default_factory=list)
    outcome: str = Field(description="victory, defeat, retreat")
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    lessons_learned: List[str] = Field(default_factory=list)


class MVPBehaviorConfig(BaseModel):
    """Configuration for MVP AI behavior"""
    mvp_id: str
    mvp_name: str
    mob_id: int = Field(description="rAthena mob ID")
    base_level: int
    personality: MVPPersonality = Field(default_factory=MVPPersonality)
    
    # Behavior parameters
    aggro_range: int = Field(default=14, description="Aggro detection range")
    chase_range: int = Field(default=20, description="Max chase distance")
    skill_use_chance: float = Field(default=0.3, ge=0.0, le=1.0)
    retreat_hp_threshold: float = Field(default=0.15, ge=0.0, le=1.0)
    enrage_hp_threshold: float = Field(default=0.30, ge=0.0, le=1.0)
    
    # Learning parameters
    learning_enabled: bool = True
    memory_retention_days: int = 30
    adaptation_rate: float = Field(default=0.1, ge=0.0, le=1.0)
    
    # Spawn parameters
    spawn_map: str
    spawn_x: int
    spawn_y: int
    spawn_interval_minutes: int = Field(default=120)
    dynamic_spawn: bool = Field(default=False, description="AI-driven spawn timing")


class MVPState(BaseModel):
    """Current state of an MVP"""
    mvp_id: str
    mob_id: int
    current_state: MVPBehaviorState = MVPBehaviorState.IDLE
    current_hp: int
    max_hp: int
    current_sp: int
    max_sp: int
    
    # Position
    map_name: str
    x: int
    y: int
    
    # Combat state
    in_combat: bool = False
    target_player_id: Optional[str] = None
    engaged_players: List[str] = Field(default_factory=list)
    current_pattern: Optional[CombatPattern] = None
    
    # AI state
    detected_strategy: Optional[PlayerStrategy] = None
    combat_memories: List[MVPCombatMemory] = Field(default_factory=list)
    adaptation_level: float = Field(default=0.0, ge=0.0, le=1.0)
    
    # Timing
    spawned_at: datetime = Field(default_factory=datetime.utcnow)
    last_action_at: datetime = Field(default_factory=datetime.utcnow)
    last_skill_at: Optional[datetime] = None
    
    # Metadata
    kill_count: int = 0
    death_count: int = 0
    total_damage_dealt: int = 0
    total_damage_taken: int = 0


class MVPAction(BaseModel):
    """Action decision for MVP"""
    action_type: str = Field(description="move, attack, skill, retreat, summon")
    target_id: Optional[str] = None
    target_x: Optional[int] = None
    target_y: Optional[int] = None
    skill_id: Optional[int] = None
    skill_level: Optional[int] = None
    priority: int = Field(default=5, ge=1, le=10)
    reasoning: str = Field(description="AI reasoning for this action")


class MVPSpawnCondition(BaseModel):
    """Conditions for dynamic MVP spawning"""
    mvp_id: str
    
    # Time-based conditions
    min_hour: Optional[int] = Field(None, ge=0, le=23)
    max_hour: Optional[int] = Field(None, ge=0, le=23)
    days_of_week: Optional[List[int]] = None
    
    # World state conditions
    min_players_online: Optional[int] = None
    max_players_online: Optional[int] = None
    required_world_events: List[str] = Field(default_factory=list)
    
    # Economic conditions
    economy_state: Optional[str] = None
    
    # Cooldown
    last_spawn: Optional[datetime] = None
    min_spawn_interval_minutes: int = 60
    
    # Probability
    spawn_probability: float = Field(default=1.0, ge=0.0, le=1.0)

