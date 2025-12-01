"""
Memory Type Definitions

Defines different types of memories and their characteristics:
- Episodic: Specific events with timestamps
- Semantic: General knowledge and facts
- Procedural: Skills and how-to knowledge
- Memory importance and decay mechanisms
"""

import math
from enum import Enum
from dataclasses import dataclass, field
from datetime import datetime, timedelta
from typing import Dict, Any, Optional


class MemoryType(str, Enum):
    """Types of memories NPCs can form."""
    
    EPISODIC = "episodic"       # Specific events (met player X at location Y)
    SEMANTIC = "semantic"       # General knowledge (dragons are dangerous)
    PROCEDURAL = "procedural"   # Skills and procedures (how to craft items)
    EMOTIONAL = "emotional"     # Emotional associations
    SPATIAL = "spatial"         # Location and navigation knowledge


class MemoryImportance(Enum):
    """
    Memory importance levels.
    
    Determines retention priority and decay resistance.
    """
    
    CRITICAL = 1.0      # Never decay (core identity, trauma)
    HIGH = 0.8          # Very slow decay (major events)
    MEDIUM = 0.5        # Normal decay (regular interactions)
    LOW = 0.3           # Fast decay (trivial events)
    TRIVIAL = 0.1       # Very fast decay (mundane observations)
    
    @classmethod
    def from_score(cls, score: float) -> 'MemoryImportance':
        """Convert numerical score to importance level."""
        if score >= 0.9:
            return cls.CRITICAL
        elif score >= 0.7:
            return cls.HIGH
        elif score >= 0.4:
            return cls.MEDIUM
        elif score >= 0.2:
            return cls.LOW
        else:
            return cls.TRIVIAL


@dataclass
class EpisodicMemory:
    """
    Episodic memory of a specific event.
    
    Contains details about what happened, when, where, and with whom.
    """
    
    event_id: str
    npc_id: str
    description: str
    location: str
    timestamp: datetime
    participants: list[str] = field(default_factory=list)
    emotions: Dict[str, float] = field(default_factory=dict)
    importance: float = 0.5
    sensory_details: Dict[str, str] = field(default_factory=dict)
    
    def get_age_days(self) -> float:
        """Get age of memory in days."""
        return (datetime.utcnow() - self.timestamp).total_seconds() / 86400
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            'event_id': self.event_id,
            'npc_id': self.npc_id,
            'description': self.description,
            'location': self.location,
            'timestamp': self.timestamp.isoformat(),
            'participants': self.participants,
            'emotions': self.emotions,
            'importance': self.importance,
            'sensory_details': self.sensory_details,
            'memory_type': MemoryType.EPISODIC.value
        }


@dataclass
class SemanticMemory:
    """
    Semantic memory of general knowledge.
    
    Contains facts, concepts, and general understanding not tied
    to specific events.
    """
    
    concept_id: str
    npc_id: str
    concept: str
    knowledge: str
    category: str
    confidence: float = 0.8
    source: Optional[str] = None
    learned_at: datetime = field(default_factory=datetime.utcnow)
    reinforcement_count: int = 1
    
    def reinforce(self) -> None:
        """Reinforce this knowledge (increases confidence)."""
        self.reinforcement_count += 1
        # Increase confidence but cap at 1.0
        self.confidence = min(1.0, self.confidence + 0.05)
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            'concept_id': self.concept_id,
            'npc_id': self.npc_id,
            'concept': self.concept,
            'knowledge': self.knowledge,
            'category': self.category,
            'confidence': self.confidence,
            'source': self.source,
            'learned_at': self.learned_at.isoformat(),
            'reinforcement_count': self.reinforcement_count,
            'memory_type': MemoryType.SEMANTIC.value
        }


@dataclass
class ProceduralMemory:
    """
    Procedural memory of skills and how-to knowledge.
    
    Contains knowledge about how to perform tasks, with
    proficiency levels that improve with practice.
    """
    
    skill_id: str
    npc_id: str
    skill_name: str
    procedure: str
    proficiency: float = 0.1  # 0.0 to 1.0
    practice_count: int = 0
    last_practiced: Optional[datetime] = None
    success_rate: float = 0.0
    
    def practice(self, success: bool) -> None:
        """
        Practice this skill.
        
        Args:
            success: Whether the practice attempt was successful
        """
        self.practice_count += 1
        self.last_practiced = datetime.utcnow()
        
        # Update success rate (moving average)
        if self.practice_count == 1:
            self.success_rate = 1.0 if success else 0.0
        else:
            weight = 0.1
            self.success_rate = (
                (1 - weight) * self.success_rate +
                weight * (1.0 if success else 0.0)
            )
        
        # Improve proficiency with practice (logarithmic growth)
        if success:
            improvement = 0.1 * (1.0 - self.proficiency)
            self.proficiency = min(1.0, self.proficiency + improvement)
    
    def decay_from_disuse(self, days_since_practice: float) -> None:
        """
        Decay proficiency from lack of practice.
        
        Args:
            days_since_practice: Days since last practice
        """
        if days_since_practice > 30:
            decay_factor = 0.01 * (days_since_practice - 30)
            self.proficiency = max(0.0, self.proficiency - decay_factor)
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary."""
        return {
            'skill_id': self.skill_id,
            'npc_id': self.npc_id,
            'skill_name': self.skill_name,
            'procedure': self.procedure,
            'proficiency': self.proficiency,
            'practice_count': self.practice_count,
            'last_practiced': self.last_practiced.isoformat() if self.last_practiced else None,
            'success_rate': self.success_rate,
            'memory_type': MemoryType.PROCEDURAL.value
        }


class MemoryDecay:
    """
    Memory decay calculator using forgetting curve.
    
    Implements Ebbinghaus forgetting curve with modifications for
    importance and reinforcement.
    """
    
    @staticmethod
    def calculate_retention(
        initial_importance: float,
        days_elapsed: float,
        reinforcement_count: int = 1,
        decay_rate: float = 0.5
    ) -> float:
        """
        Calculate memory retention strength.
        
        Uses modified forgetting curve:
        R(t) = I * e^(-t/S)
        
        Where:
        - R(t) = retention at time t
        - I = initial importance
        - t = time elapsed
        - S = stability (affected by reinforcement)
        
        Args:
            initial_importance: Initial memory importance (0.0 to 1.0)
            days_elapsed: Days since memory formation
            reinforcement_count: Number of times memory was reinforced
            decay_rate: Base decay rate (higher = faster decay)
            
        Returns:
            float: Retention strength (0.0 to 1.0)
        """
        # Calculate stability from reinforcement
        stability = 1.0 + math.log(reinforcement_count) if reinforcement_count > 0 else 1.0
        
        # Apply forgetting curve
        decay_factor = math.exp(-decay_rate * days_elapsed / stability)
        
        # Final retention
        retention = initial_importance * decay_factor
        
        return max(0.0, min(1.0, retention))
    
    @staticmethod
    def should_retain(
        importance: float,
        days_elapsed: float,
        reinforcement_count: int = 1,
        threshold: float = 0.1
    ) -> bool:
        """
        Determine if memory should be retained.
        
        Args:
            importance: Memory importance
            days_elapsed: Days since formation
            reinforcement_count: Reinforcement count
            threshold: Minimum retention threshold
            
        Returns:
            bool: True if memory should be kept
        """
        retention = MemoryDecay.calculate_retention(
            importance, days_elapsed, reinforcement_count
        )
        return retention >= threshold
    
    @staticmethod
    def get_decay_factor(memory_type: MemoryType) -> float:
        """
        Get decay rate for memory type.
        
        Args:
            memory_type: Type of memory
            
        Returns:
            float: Decay rate (higher = faster decay)
        """
        decay_rates = {
            MemoryType.EPISODIC: 0.8,      # Faster decay
            MemoryType.SEMANTIC: 0.3,      # Slower decay
            MemoryType.PROCEDURAL: 0.2,    # Very slow decay
            MemoryType.EMOTIONAL: 0.5,     # Medium decay
            MemoryType.SPATIAL: 0.4        # Slow decay
        }
        return decay_rates.get(memory_type, 0.5)
    
    @staticmethod
    def calculate_consolidation_strength(
        memory_age_days: float,
        rehearsal_count: int,
        emotional_intensity: float = 0.5
    ) -> float:
        """
        Calculate memory consolidation strength.
        
        Recently formed memories become more stable over time through
        consolidation, especially if rehearsed or emotionally significant.
        
        Args:
            memory_age_days: Age of memory in days
            rehearsal_count: Number of times memory was recalled
            emotional_intensity: Emotional significance (0.0 to 1.0)
            
        Returns:
            float: Consolidation strength (0.0 to 1.0)
        """
        # Consolidation follows power law
        # Peaks around 7-30 days, then stabilizes
        age_factor = min(1.0, memory_age_days / 30.0)
        
        # Rehearsal strengthens consolidation
        rehearsal_factor = min(1.0, rehearsal_count / 10.0)
        
        # Emotional intensity aids consolidation
        emotion_factor = emotional_intensity
        
        # Combined consolidation strength
        consolidation = (
            0.4 * age_factor +
            0.3 * rehearsal_factor +
            0.3 * emotion_factor
        )
        
        return max(0.0, min(1.0, consolidation))