"""
Gift-giving system API endpoints
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
from typing import Dict, Any, List
from datetime import datetime
import uuid

try:
    from models.gift import (
        Gift, GiftCategory, GiftGivingRequest, GiftGivingResponse,
        NPCGiftPreferences, GiftHistory, GiftPreference
    )
    from database import db
    from llm import get_llm_provider
except ImportError:
    from models.gift import (
        Gift, GiftCategory, GiftGivingRequest, GiftGivingResponse,
        NPCGiftPreferences, GiftHistory, GiftPreference
    )
    from database import db
    from llm import get_llm_provider

router = APIRouter(prefix="/ai/gift", tags=["gift"])


@router.post("/give", response_model=GiftGivingResponse)
async def give_gift(request: GiftGivingRequest):
    """
    Give a gift to an NPC and receive reaction
    
    This endpoint:
    1. Validates the gift
    2. Retrieves NPC preferences
    3. Calculates relationship change
    4. Generates NPC reaction using LLM
    5. Checks for unlocked quests/events
    6. Updates relationship and gift history
    """
    try:
        logger.info(f"Gift giving: Player {request.player_id} -> NPC {request.npc_id}, Item {request.item_id}")
        
        # Get NPC state
        npc_state = await db.get_npc_state(request.npc_id)
        if not npc_state:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"NPC {request.npc_id} not found"
            )
        
        # Get or create NPC gift preferences
        preferences_key = f"gift_preferences:{request.npc_id}"
        preferences_data = await db.client.get(preferences_key)
        
        if preferences_data:
            import json
            preferences = NPCGiftPreferences(**json.loads(preferences_data))
        else:
            # Create default preferences based on NPC class
            preferences = await _create_default_preferences(request.npc_id, npc_state)
        
        # Create gift object
        gift = Gift(
            item_id=request.item_id,
            item_name=request.item_name,
            category=request.item_category,
            rarity=request.item_rarity,
            base_value=request.item_value,
            relationship_value=5.0  # Base relationship points
        )
        
        # Calculate relationship change
        relationship_change = preferences.calculate_gift_value(gift)
        
        # Get current relationship level
        relationship_key = f"relationship:{request.npc_id}:{request.player_id}"
        current_level = await db.client.get(relationship_key)
        current_level = float(current_level) if current_level else 0.0
        
        # Update relationship
        new_level = max(-100.0, min(100.0, current_level + relationship_change))
        await db.client.set(relationship_key, str(new_level))
        
        # Generate NPC reaction using LLM
        npc_reaction = await _generate_gift_reaction(
            npc_state=npc_state,
            gift=gift,
            relationship_change=relationship_change,
            new_relationship_level=new_level
        )
        
        # Check for special events/unlocked quests
        unlocked_quests = await _check_gift_triggered_quests(
            npc_id=request.npc_id,
            player_id=request.player_id,
            gift=gift,
            new_relationship_level=new_level,
            total_gifts=preferences.total_gifts_received + 1
        )
        
        # Record gift history
        gift_history = GiftHistory(
            gift_id=str(uuid.uuid4()),
            npc_id=request.npc_id,
            player_id=request.player_id,
            gift=gift,
            relationship_change=relationship_change,
            npc_reaction=npc_reaction,
            special_event_triggered=len(unlocked_quests) > 0
        )
        
        # Update preferences
        preferences.gift_history.append(gift_history)
        preferences.total_gifts_received += 1
        preferences.last_gift_timestamp = datetime.utcnow()
        
        # Save updated preferences
        import json
        await db.client.set(preferences_key, json.dumps(preferences.dict()))
        
        logger.info(f"Gift processed: {relationship_change:+.1f} relationship, new level: {new_level:.1f}")
        
        return GiftGivingResponse(
            success=True,
            npc_reaction=npc_reaction,
            relationship_change=relationship_change,
            new_relationship_level=new_level,
            special_event_triggered=len(unlocked_quests) > 0,
            unlocked_quests=unlocked_quests,
            message=f"Gift given successfully. Relationship changed by {relationship_change:+.1f}"
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error processing gift: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/preferences/{npc_id}", response_model=NPCGiftPreferences)
async def get_gift_preferences(npc_id: str):
    """Get NPC's gift preferences"""
    try:
        preferences_key = f"gift_preferences:{npc_id}"
        preferences_data = await db.client.get(preferences_key)

        if not preferences_data:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Gift preferences for NPC {npc_id} not found"
            )

        import json
        return NPCGiftPreferences(**json.loads(preferences_data))

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error retrieving gift preferences: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


@router.get("/history/{npc_id}/{player_id}")
async def get_gift_history(npc_id: str, player_id: str):
    """Get gift history between player and NPC"""
    try:
        preferences_key = f"gift_preferences:{npc_id}"
        preferences_data = await db.client.get(preferences_key)

        if not preferences_data:
            return {"gifts": [], "total": 0}

        import json
        preferences = NPCGiftPreferences(**json.loads(preferences_data))

        # Filter history for this player
        player_gifts = [
            gift.dict() for gift in preferences.gift_history
            if gift.player_id == player_id
        ]

        return {
            "gifts": player_gifts,
            "total": len(player_gifts),
            "last_gift": preferences.last_gift_timestamp
        }

    except Exception as e:
        logger.error(f"Error retrieving gift history: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=str(e)
        )


# Helper functions

async def _create_default_preferences(npc_id: str, npc_state: Dict[str, Any]) -> NPCGiftPreferences:
    """Create default gift preferences based on NPC personality"""
    personality = npc_state.get("personality", {})
    npc_class = npc_state.get("class", "merchant")

    # Base preferences on personality traits
    preferences = []

    # High openness = likes books, rare items
    if personality.get("openness", 0.5) > 0.6:
        preferences.append(GiftPreference(category=GiftCategory.BOOK, preference_level=0.8))
        preferences.append(GiftPreference(category=GiftCategory.RARE, preference_level=0.7))

    # High conscientiousness = likes tools, materials
    if personality.get("conscientiousness", 0.5) > 0.6:
        preferences.append(GiftPreference(category=GiftCategory.TOOL, preference_level=0.7))
        preferences.append(GiftPreference(category=GiftCategory.MATERIAL, preference_level=0.6))

    # High extraversion = likes luxury, flowers
    if personality.get("extraversion", 0.5) > 0.6:
        preferences.append(GiftPreference(category=GiftCategory.LUXURY, preference_level=0.8))
        preferences.append(GiftPreference(category=GiftCategory.FLOWER, preference_level=0.7))

    # High agreeableness = likes food, personal items
    if personality.get("agreeableness", 0.5) > 0.6:
        preferences.append(GiftPreference(category=GiftCategory.FOOD, preference_level=0.7))
        preferences.append(GiftPreference(category=GiftCategory.PERSONAL, preference_level=0.8))

    # Class-based preferences
    if npc_class == "warrior":
        preferences.append(GiftPreference(category=GiftCategory.WEAPON, preference_level=0.9))
        preferences.append(GiftPreference(category=GiftCategory.ARMOR, preference_level=0.8))
    elif npc_class == "merchant":
        preferences.append(GiftPreference(category=GiftCategory.LUXURY, preference_level=0.8))
        preferences.append(GiftPreference(category=GiftCategory.RARE, preference_level=0.7))
    elif npc_class == "mage":
        preferences.append(GiftPreference(category=GiftCategory.BOOK, preference_level=0.9))
        preferences.append(GiftPreference(category=GiftCategory.ACCESSORY, preference_level=0.7))

    return NPCGiftPreferences(
        npc_id=npc_id,
        npc_name=npc_state.get("name", "Unknown"),
        preferences=preferences
    )


async def _generate_gift_reaction(
    npc_state: Dict[str, Any],
    gift: Gift,
    relationship_change: float,
    new_relationship_level: float
) -> str:
    """Generate NPC reaction to gift using LLM"""
    try:
        llm = get_llm_provider()

        # Determine sentiment
        if relationship_change > 10:
            sentiment = "extremely happy and grateful"
        elif relationship_change > 5:
            sentiment = "pleased and appreciative"
        elif relationship_change > 0:
            sentiment = "polite and thankful"
        elif relationship_change > -5:
            sentiment = "neutral or slightly disappointed"
        else:
            sentiment = "displeased or offended"

        prompt = f"""Generate a brief NPC reaction to receiving a gift.

NPC Name: {npc_state.get('name', 'Unknown')}
NPC Personality: {npc_state.get('personality', {})}
Gift: {gift.item_name} ({gift.category.value})
Gift Rarity: {gift.rarity}/5
Relationship Change: {relationship_change:+.1f}
New Relationship Level: {new_relationship_level:.1f}/100
NPC Sentiment: {sentiment}

Generate a 1-2 sentence reaction that reflects the NPC's personality and sentiment.
Keep it natural and in-character."""

        response = await llm.generate(prompt, max_tokens=100)
        return response.strip()

    except Exception as e:
        logger.error(f"Error generating gift reaction: {e}")
        # Fallback reaction
        if relationship_change > 0:
            return f"Thank you for the {gift.item_name}! I appreciate your kindness."
        else:
            return f"Oh... a {gift.item_name}. Thank you, I suppose."


async def _check_gift_triggered_quests(
    npc_id: str,
    player_id: str,
    gift: Gift,
    new_relationship_level: float,
    total_gifts: int
) -> List[str]:
    """Check if gift triggers any special quests"""
    unlocked_quests = []

    try:
        # Get all quests for this NPC
        quest_keys = await db.client.keys(f"quest:{npc_id}:*")

        for quest_key in quest_keys:
            quest_data = await db.client.get(quest_key)
            if not quest_data:
                continue

            import json
            quest = json.loads(quest_data)

            # Check if quest has gift triggers
            for trigger in quest.get("triggers", []):
                if trigger.get("trigger_type") == "gift":
                    gift_trigger = trigger.get("gift", {})

                    # Check conditions
                    if gift_trigger.get("specific_item_received") == gift.item_id:
                        unlocked_quests.append(quest["quest_id"])
                        logger.info(f"Quest {quest['quest_id']} unlocked by specific gift")
                    elif gift_trigger.get("total_gifts_min", 0) <= total_gifts:
                        unlocked_quests.append(quest["quest_id"])
                        logger.info(f"Quest {quest['quest_id']} unlocked by total gifts")

            # Check relationship threshold
            if quest.get("relationship_unlock_threshold"):
                if new_relationship_level >= quest["relationship_unlock_threshold"]:
                    if quest["quest_id"] not in unlocked_quests:
                        unlocked_quests.append(quest["quest_id"])
                        logger.info(f"Quest {quest['quest_id']} unlocked by relationship level")

    except Exception as e:
        logger.error(f"Error checking gift-triggered quests: {e}")

    return unlocked_quests

