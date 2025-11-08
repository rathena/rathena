"""
Pub/Sub Bridge Router
Phase 8B: HTTP-based Pub/Sub bridge for web-server to poll NPC actions
Alternative to C++ Redis subscriber (no external dependencies required)
"""

from fastapi import APIRouter, HTTPException
from typing import List, Dict, Any, Optional
from loguru import logger
from database import db
import json

router = APIRouter(prefix="/pubsub", tags=["pubsub"])


@router.get("/poll-npc-actions")
async def poll_npc_actions(
    npc_ids: Optional[str] = None,
    timeout: int = 5
) -> Dict[str, Any]:
    """
    Poll for pending NPC actions (HTTP long-polling)
    Web-server calls this endpoint to get batched NPC actions
    
    Args:
        npc_ids: Comma-separated NPC IDs to poll (optional, polls all if not specified)
        timeout: Long-polling timeout in seconds (default 5)
    
    Returns:
        {
            "actions": [
                {
                    "npc_id": "npc_001",
                    "action_type": "move",
                    "action_data": {...},
                    "timestamp": 1234567890.123
                },
                ...
            ],
            "count": 5
        }
    """
    try:
        # Parse NPC IDs
        target_npcs = npc_ids.split(",") if npc_ids else None
        
        # Get pending actions from Redis
        actions = []
        
        if target_npcs:
            # Poll specific NPCs
            for npc_id in target_npcs:
                action = await _get_pending_action(npc_id)
                if action:
                    actions.append(action)
        else:
            # Poll all NPCs with pending actions
            # Use Redis SCAN to find all npc:action:* keys
            pattern = "npc:action:pending:*"
            cursor = 0
            
            while True:
                cursor, keys = await db.scan(cursor, match=pattern, count=100)
                
                for key in keys:
                    npc_id = key.decode('utf-8').split(":")[-1]
                    action = await _get_pending_action(npc_id)
                    if action:
                        actions.append(action)
                
                if cursor == 0:
                    break
        
        logger.info(f"[PUBSUB POLL] Retrieved {len(actions)} pending actions")
        
        return {
            "actions": actions,
            "count": len(actions)
        }
        
    except Exception as e:
        logger.error(f"[PUBSUB POLL ERROR] {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/publish-action")
async def publish_action(
    npc_id: str,
    action_type: str,
    action_data: Dict[str, Any]
) -> Dict[str, Any]:
    """
    Publish NPC action to pending queue
    Called by AI service to queue actions for web-server
    
    Args:
        npc_id: NPC identifier
        action_type: Action type (move, emote, state_change)
        action_data: Action parameters
    
    Returns:
        {"success": true, "npc_id": "npc_001"}
    """
    try:
        # Store action in Redis with expiration
        key = f"npc:action:pending:{npc_id}"
        
        action = {
            "npc_id": npc_id,
            "action_type": action_type,
            "action_data": action_data,
            "timestamp": __import__('time').time()
        }
        
        # Store with 60 second expiration (actions should be consumed quickly)
        await db.setex(key, 60, json.dumps(action))
        
        logger.info(f"[PUBSUB PUBLISH] {npc_id} -> {action_type}")
        
        return {
            "success": True,
            "npc_id": npc_id
        }
        
    except Exception as e:
        logger.error(f"[PUBSUB PUBLISH ERROR] {npc_id}: {e}", exc_info=True)
        raise HTTPException(status_code=500, detail=str(e))


async def _get_pending_action(npc_id: str) -> Optional[Dict[str, Any]]:
    """
    Get and remove pending action for NPC
    
    Args:
        npc_id: NPC identifier
    
    Returns:
        Action dict or None if no pending action
    """
    try:
        key = f"npc:action:pending:{npc_id}"
        
        # Get action
        action_json = await db.get(key)
        
        if not action_json:
            return None
        
        # Delete key (consume action)
        await db.delete(key)
        
        # Parse and return
        action = json.loads(action_json)
        return action
        
    except Exception as e:
        logger.error(f"[PUBSUB GET ERROR] {npc_id}: {e}")
        return None

