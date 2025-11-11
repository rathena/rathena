"""
Relationship API endpoints for player-NPC relationship management
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
from datetime import datetime
from typing import Dict, Any

from database import db

router = APIRouter(prefix="/ai/relationship", tags=["relationship"])


@router.post("/check")
async def check_relationship(request: Dict[str, Any]):
    """
    Check relationship level between player and NPC
    
    Request body should contain:
    - npc_id: NPC identifier
    - player_id: Player identifier
    """
    try:
        npc_id = request.get("npc_id")
        player_id = request.get("player_id")
        
        if not npc_id or not player_id:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="npc_id and player_id are required"
            )
        
        logger.info(f"Checking relationship: NPC {npc_id} <-> Player {player_id}")
        
        # Get relationship level from database
        relationship_key = f"relationship:{npc_id}:{player_id}"
        relationship_level = await db.redis.get(relationship_key)
        relationship_level = int(relationship_level) if relationship_level else 0
        
        # Determine relationship status based on level
        if relationship_level >= 80:
            status_text = "Best Friends"
        elif relationship_level >= 60:
            status_text = "Close Friends"
        elif relationship_level >= 40:
            status_text = "Friends"
        elif relationship_level >= 20:
            status_text = "Acquaintances"
        elif relationship_level >= 0:
            status_text = "Neutral"
        elif relationship_level >= -20:
            status_text = "Dislike"
        elif relationship_level >= -40:
            status_text = "Hostile"
        elif relationship_level >= -60:
            status_text = "Enemy"
        else:
            status_text = "Sworn Enemy"
        
        logger.info(f"Relationship level: {relationship_level} ({status_text})")
        
        return {
            "npc_id": npc_id,
            "player_id": player_id,
            "relationship_level": relationship_level,
            "status": status_text,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error checking relationship: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to check relationship: {str(e)}"
        )


@router.post("/update")
async def update_relationship(request: Dict[str, Any]):
    """
    Update relationship level between player and NPC
    
    Request body should contain:
    - npc_id: NPC identifier
    - player_id: Player identifier
    - change: Relationship change amount (can be positive or negative)
    """
    try:
        npc_id = request.get("npc_id")
        player_id = request.get("player_id")
        change = request.get("change", 0)
        
        if not npc_id or not player_id:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="npc_id and player_id are required"
            )
        
        logger.info(f"Updating relationship: NPC {npc_id} <-> Player {player_id} (change: {change})")
        
        # Get current relationship level
        relationship_key = f"relationship:{npc_id}:{player_id}"
        current_level = await db.redis.get(relationship_key)
        current_level = int(current_level) if current_level else 0
        
        # Calculate new level (clamped between -100 and 100)
        new_level = max(-100, min(100, current_level + change))
        
        # Store updated relationship level
        await db.redis.set(relationship_key, new_level)
        
        logger.info(f"Relationship updated: {current_level} -> {new_level}")
        
        return {
            "npc_id": npc_id,
            "player_id": player_id,
            "previous_level": current_level,
            "new_level": new_level,
            "change": change,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating relationship: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update relationship: {str(e)}"
        )

