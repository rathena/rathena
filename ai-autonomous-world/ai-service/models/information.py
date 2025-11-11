"""
Information Sensitivity System for NPC Social Intelligence

This module defines the information sensitivity system that allows NPCs to
decide what information to share based on relationship level, player reputation,
and NPC personality traits.
"""

from enum import Enum
from dataclasses import dataclass, field
from typing import List, Optional
from loguru import logger


class InformationSensitivity(str, Enum):
    """Information sensitivity levels"""
    PUBLIC = "public"  # Anyone can access
    PRIVATE = "private"  # Requires relationship level >= 5
    SECRET = "secret"  # Requires relationship level >= 8
    CONFIDENTIAL = "confidential"  # Requires relationship level >= 10


@dataclass
class InformationItem:
    """
    A piece of information that an NPC can share
    
    Attributes:
        content: The actual information text
        sensitivity: Sensitivity level (PUBLIC, PRIVATE, SECRET, CONFIDENTIAL)
        tags: Keywords for categorization and retrieval
        shared_with: List of player IDs who have received this information
        category: Category of information (e.g., "personal", "quest", "lore")
    """
    content: str
    sensitivity: InformationSensitivity
    tags: List[str] = field(default_factory=list)
    shared_with: List[str] = field(default_factory=list)
    category: str = "general"
    
    def to_dict(self) -> dict:
        """Convert to dictionary for storage"""
        return {
            "content": self.content,
            "sensitivity": self.sensitivity.value,
            "tags": self.tags,
            "shared_with": self.shared_with,
            "category": self.category
        }
    
    @classmethod
    def from_dict(cls, data: dict) -> 'InformationItem':
        """Create from dictionary"""
        return cls(
            content=data["content"],
            sensitivity=InformationSensitivity(data["sensitivity"]),
            tags=data.get("tags", []),
            shared_with=data.get("shared_with", []),
            category=data.get("category", "general")
        )


def get_relationship_threshold(sensitivity: InformationSensitivity) -> int:
    """
    Get the base relationship level threshold for a sensitivity level
    
    Args:
        sensitivity: Information sensitivity level
        
    Returns:
        Minimum relationship level required to access this information
    """
    thresholds = {
        InformationSensitivity.PUBLIC: 0,
        InformationSensitivity.PRIVATE: 5,
        InformationSensitivity.SECRET: 8,
        InformationSensitivity.CONFIDENTIAL: 10
    }
    return thresholds.get(sensitivity, 0)


def calculate_effective_threshold(
    base_threshold: int,
    agreeableness: float,
    neuroticism: float,
    openness: float
) -> int:
    """
    Calculate effective relationship threshold based on personality traits
    
    Args:
        base_threshold: Base relationship level threshold
        agreeableness: Agreeableness trait (0.0-1.0)
        neuroticism: Neuroticism trait (0.0-1.0)
        openness: Openness trait (0.0-1.0)
        
    Returns:
        Adjusted relationship level threshold
    """
    adjustment = 0
    
    # High agreeableness = shares more easily (-1 threshold)
    if agreeableness > 0.7:
        adjustment -= 1
    # Low agreeableness = shares less easily (+1 threshold)
    elif agreeableness < 0.3:
        adjustment += 1
    
    # High neuroticism = more cautious (+1 threshold)
    if neuroticism > 0.7:
        adjustment += 1
    # Low neuroticism = less cautious (-1 threshold)
    elif neuroticism < 0.3:
        adjustment -= 1
    
    # High openness = more open to sharing (-1 threshold)
    if openness > 0.7:
        adjustment -= 1
    
    # Apply adjustment and ensure threshold is non-negative
    effective_threshold = max(0, base_threshold + adjustment)
    
    logger.debug(
        f"Threshold calculation: base={base_threshold}, "
        f"agreeableness={agreeableness:.2f}, neuroticism={neuroticism:.2f}, "
        f"openness={openness:.2f}, adjustment={adjustment}, "
        f"effective={effective_threshold}"
    )
    
    return effective_threshold


def filter_information_by_relationship(
    information_items: List[InformationItem],
    relationship_level: int,
    agreeableness: float = 0.5,
    neuroticism: float = 0.5,
    openness: float = 0.5
) -> List[InformationItem]:
    """
    Filter information items based on relationship level and personality
    
    Args:
        information_items: List of all information items
        relationship_level: Current relationship level with player
        agreeableness: NPC agreeableness trait (0.0-1.0)
        neuroticism: NPC neuroticism trait (0.0-1.0)
        openness: NPC openness trait (0.0-1.0)
        
    Returns:
        List of information items that can be shared
    """
    available_items = []
    
    for item in information_items:
        base_threshold = get_relationship_threshold(item.sensitivity)
        effective_threshold = calculate_effective_threshold(
            base_threshold, agreeableness, neuroticism, openness
        )
        
        if relationship_level >= effective_threshold:
            available_items.append(item)
    
    logger.info(
        f"Filtered {len(available_items)}/{len(information_items)} information items "
        f"for relationship level {relationship_level}"
    )
    
    return available_items

