"""
Gift-giving system data models for NPC interactions
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from enum import Enum


class GiftCategory(str, Enum):
    """Categories of gifts"""
    FOOD = "food"
    WEAPON = "weapon"
    ARMOR = "armor"
    ACCESSORY = "accessory"
    CONSUMABLE = "consumable"
    MATERIAL = "material"
    RARE = "rare"
    LUXURY = "luxury"
    PERSONAL = "personal"
    FLOWER = "flower"
    BOOK = "book"
    TOOL = "tool"


class GiftPreference(BaseModel):
    """NPC's preference for a specific gift category"""
    category: GiftCategory
    preference_level: float = Field(
        ge=-1.0,
        le=1.0,
        description="Preference: -1 (hates) to 1 (loves)"
    )
    reason: Optional[str] = Field(None, description="Why NPC likes/dislikes this category")


class Gift(BaseModel):
    """Represents a gift item"""
    item_id: int = Field(..., description="rAthena item ID")
    item_name: str = Field(..., description="Item name")
    category: GiftCategory
    rarity: int = Field(ge=1, le=5, description="Rarity: 1 (common) to 5 (legendary)")
    base_value: int = Field(ge=0, description="Base zeny value")
    relationship_value: float = Field(
        default=1.0,
        ge=0.0,
        description="Base relationship points gained"
    )


class GiftHistory(BaseModel):
    """Record of a gift given to an NPC"""
    gift_id: str = Field(..., description="Unique gift transaction ID")
    npc_id: str
    player_id: str
    gift: Gift
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    relationship_change: float = Field(
        description="Actual relationship points gained/lost"
    )
    npc_reaction: str = Field(description="NPC's reaction to the gift")
    special_event_triggered: bool = Field(
        default=False,
        description="Whether gift triggered special event/quest"
    )
    metadata: Dict[str, Any] = Field(default_factory=dict)


class NPCGiftPreferences(BaseModel):
    """Complete gift preference profile for an NPC"""
    npc_id: str
    npc_name: str
    preferences: List[GiftPreference] = Field(default_factory=list)
    favorite_items: List[int] = Field(
        default_factory=list,
        description="Specific item IDs NPC loves"
    )
    hated_items: List[int] = Field(
        default_factory=list,
        description="Specific item IDs NPC hates"
    )
    gift_history: List[GiftHistory] = Field(default_factory=list)
    total_gifts_received: int = Field(default=0)
    last_gift_timestamp: Optional[datetime] = None
    
    def get_preference_for_category(self, category: GiftCategory) -> float:
        """Get preference level for a category"""
        for pref in self.preferences:
            if pref.category == category:
                return pref.preference_level
        return 0.0  # Neutral if not specified
    
    def calculate_gift_value(self, gift: Gift) -> float:
        """
        Calculate relationship value for a gift based on preferences
        
        Returns:
            Relationship points to add (can be negative)
        """
        # Check if specific item is loved/hated
        if gift.item_id in self.favorite_items:
            return gift.relationship_value * 2.0
        if gift.item_id in self.hated_items:
            return -gift.relationship_value
        
        # Use category preference
        category_pref = self.get_preference_for_category(gift.category)
        multiplier = 1.0 + category_pref  # Range: 0.0 to 2.0
        
        # Apply rarity bonus
        rarity_bonus = (gift.rarity - 1) * 0.25  # 0 to 1.0
        
        return gift.relationship_value * multiplier * (1.0 + rarity_bonus)


class GiftGivingRequest(BaseModel):
    """Request to give a gift to an NPC"""
    npc_id: str
    player_id: str
    item_id: int
    item_name: str
    item_category: GiftCategory
    item_rarity: int = Field(ge=1, le=5)
    item_value: int = Field(ge=0)


class GiftGivingResponse(BaseModel):
    """Response from gift-giving action"""
    success: bool
    npc_reaction: str
    relationship_change: float
    new_relationship_level: float
    special_event_triggered: bool = False
    unlocked_quests: List[str] = Field(default_factory=list)
    message: str

