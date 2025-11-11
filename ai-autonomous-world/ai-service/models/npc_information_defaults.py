"""
Default Information Items for Different NPC Classes

This module provides default information items for each NPC class,
categorized by sensitivity level.
"""

from typing import List, Dict
try:
    from ai_service.models.information import InformationItem, InformationSensitivity
except ModuleNotFoundError:
    from models.information import InformationItem, InformationSensitivity


def get_default_information(npc_class: str, npc_name: str) -> List[InformationItem]:
    """
    Get default information items for an NPC based on their class
    
    Args:
        npc_class: NPC class (explorer, guard, merchant, etc.)
        npc_name: NPC name for personalization
        
    Returns:
        List of default information items
    """
    defaults = {
        "explorer": _get_explorer_information,
        "guard": _get_guard_information,
        "merchant": _get_merchant_information,
        "quest_giver": _get_quest_giver_information,
        "generic": _get_generic_information
    }
    
    generator = defaults.get(npc_class, _get_generic_information)
    return generator(npc_name)


def _get_explorer_information(npc_name: str) -> List[InformationItem]:
    """Default information for Explorer NPCs"""
    return [
        # PUBLIC information
        InformationItem(
            content=f"I'm {npc_name}, an explorer who loves discovering new places and uncovering ancient mysteries.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["profession", "introduction"],
            category="personal"
        ),
        InformationItem(
            content="Prontera is the capital city of the Rune-Midgarts Kingdom, a bustling hub of commerce and adventure.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["location", "lore"],
            category="world"
        ),
        
        # PRIVATE information (relationship >= 5)
        InformationItem(
            content="I have a secret map that shows the location of a hidden dungeon beneath the Prontera sewers. Few people know about it.",
            sensitivity=InformationSensitivity.PRIVATE,
            tags=["secret", "dungeon", "map"],
            category="quest"
        ),
        InformationItem(
            content="I'm terrified of spiders. I once got trapped in a spider nest for three days, and I still have nightmares about it.",
            sensitivity=InformationSensitivity.PRIVATE,
            tags=["fear", "personal", "weakness"],
            category="personal"
        ),
        
        # SECRET information (relationship >= 8)
        InformationItem(
            content="I discovered an ancient artifact in the Glast Heim ruins - a crystal that glows with an otherworldly light. I keep it hidden in my quarters.",
            sensitivity=InformationSensitivity.SECRET,
            tags=["artifact", "secret", "treasure"],
            category="quest"
        ),
        InformationItem(
            content="The royal family is hiding something in the castle basement. I've seen strange lights and heard whispers late at night.",
            sensitivity=InformationSensitivity.SECRET,
            tags=["conspiracy", "royal", "mystery"],
            category="lore"
        ),
        
        # CONFIDENTIAL information (relationship >= 10)
        InformationItem(
            content="I know the location of the legendary Yggdrasil Seed, said to grant immortality. It's hidden in a cave system north of Payon, guarded by ancient spirits.",
            sensitivity=InformationSensitivity.CONFIDENTIAL,
            tags=["legendary", "quest", "immortality"],
            category="quest"
        )
    ]


def _get_guard_information(npc_name: str) -> List[InformationItem]:
    """Default information for Guard NPCs"""
    return [
        # PUBLIC information
        InformationItem(
            content=f"I'm {npc_name}, a guard sworn to protect Prontera and its citizens from all threats.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["profession", "introduction"],
            category="personal"
        ),
        InformationItem(
            content="The city gates are open from dawn to dusk. We close them at night for security.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["security", "schedule"],
            category="world"
        ),
        
        # PRIVATE information (relationship >= 5)
        InformationItem(
            content="My patrol route takes me through the eastern district every two hours. I check the merchant quarter at midnight.",
            sensitivity=InformationSensitivity.PRIVATE,
            tags=["patrol", "schedule", "security"],
            category="personal"
        ),
        InformationItem(
            content="I've been having doubts about my captain lately. He's been acting strange, meeting with suspicious figures in dark alleys.",
            sensitivity=InformationSensitivity.PRIVATE,
            tags=["suspicion", "captain", "corruption"],
            category="lore"
        ),
        
        # SECRET information (relationship >= 8)
        InformationItem(
            content="There's a weakness in the western wall defenses. A skilled thief could exploit it, but I haven't reported it yet because I'm investigating who might know about it.",
            sensitivity=InformationSensitivity.SECRET,
            tags=["security", "weakness", "investigation"],
            category="quest"
        ),
        InformationItem(
            content="The royal family has a secret escape tunnel that leads from the castle to the forest. Only the highest-ranking guards know about it.",
            sensitivity=InformationSensitivity.SECRET,
            tags=["royal", "secret", "tunnel"],
            category="lore"
        ),
        
        # CONFIDENTIAL information (relationship >= 10)
        InformationItem(
            content="I discovered evidence that someone in the guard is working with the assassins' guild. I'm gathering proof before I report it to the king.",
            sensitivity=InformationSensitivity.CONFIDENTIAL,
            tags=["conspiracy", "corruption", "assassins"],
            category="quest"
        )
    ]


def _get_merchant_information(npc_name: str) -> List[InformationItem]:
    """Default information for Merchant NPCs"""
    return [
        InformationItem(
            content=f"I'm {npc_name}, a merchant who deals in rare goods and exotic wares from across the kingdom.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["profession", "introduction"],
            category="personal"
        ),
        InformationItem(
            content="I can get you the best prices on potions and equipment. Just ask!",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["trade", "prices"],
            category="world"
        )
    ]


def _get_quest_giver_information(npc_name: str) -> List[InformationItem]:
    """Default information for Quest Giver NPCs"""
    return [
        InformationItem(
            content=f"I'm {npc_name}, and I have tasks for brave adventurers willing to help the kingdom.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["profession", "introduction", "quests"],
            category="personal"
        )
    ]


def _get_generic_information(npc_name: str) -> List[InformationItem]:
    """Default information for Generic NPCs"""
    return [
        InformationItem(
            content=f"I'm {npc_name}, a resident of Prontera.",
            sensitivity=InformationSensitivity.PUBLIC,
            tags=["introduction"],
            category="personal"
        )
    ]

