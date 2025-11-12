"""
Redis Pub/Sub Manager for AI Service
Phase 8B: Async communication for autonomous NPC behavior
Publishes NPC actions to Redis channels for web-server to consume
"""

import json
import logging
import asyncio
from typing import Dict, Any, Callable, Optional, List
from database import db

logger = logging.getLogger(__name__)


class PubSubManager:
    """
    Manages Redis Pub/Sub for async NPC action communication
    Publishers: AI service (publishes NPC actions)
    Subscribers: Web-server bridge (executes NPC actions in game)
    """
    
    def __init__(self):
        """Initialize Pub/Sub manager"""
        self._subscribers: Dict[str, List[Callable]] = {}
        self._pubsub = None
        self._listener_task: Optional[asyncio.Task] = None
        logger.info("PubSubManager initialized")
    
    async def publish_npc_action(
        self,
        npc_id: str,
        action_type: str,
        action_data: Dict[str, Any]
    ) -> bool:
        """
        Publish NPC action to Redis channel
        
        Args:
            npc_id: NPC identifier
            action_type: Type of action (move, emote, state_change, etc.)
            action_data: Action parameters
        
        Returns:
            True if published successfully
        """
        try:
            channel = f"npc:action:{npc_id}"
            
            message = {
                "npc_id": npc_id,
                "action_type": action_type,
                "action_data": action_data,
                "timestamp": asyncio.get_event_loop().time()
            }
            
            # Publish to Redis
            subscribers = await db.publish(channel, json.dumps(message))
            
            logger.info(
                f"[PUBSUB PUBLISH] {channel} -> {action_type} "
                f"(subscribers: {subscribers})"
            )
            
            return True
            
        except Exception as e:
            logger.error(f"[PUBSUB PUBLISH ERROR] {npc_id}: {e}", exc_info=True)
            return False
    
    async def publish_npc_movement(
        self,
        npc_id: str,
        target_x: int,
        target_y: int,
        target_map: Optional[str] = None,
        speed: int = 150
    ) -> bool:
        """
        Publish NPC movement action
        
        Args:
            npc_id: NPC identifier
            target_x: Target X coordinate
            target_y: Target Y coordinate
            target_map: Target map name (optional)
            speed: Movement speed
        
        Returns:
            True if published successfully
        """
        action_data = {
            "target_x": target_x,
            "target_y": target_y,
            "speed": speed
        }
        
        if target_map:
            action_data["target_map"] = target_map
        
        return await self.publish_npc_action(npc_id, "move", action_data)
    
    async def publish_npc_state_change(
        self,
        npc_id: str,
        new_state: str,
        state_data: Optional[Dict[str, Any]] = None
    ) -> bool:
        """
        Publish NPC state change
        
        Args:
            npc_id: NPC identifier
            new_state: New state (idle, walking, talking, etc.)
            state_data: Additional state data
        
        Returns:
            True if published successfully
        """
        action_data = {
            "new_state": new_state,
            "state_data": state_data or {}
        }
        
        return await self.publish_npc_action(npc_id, "state_change", action_data)
    
    async def publish_npc_emote(
        self,
        npc_id: str,
        emote_id: int,
        duration: int = 3000
    ) -> bool:
        """
        Publish NPC emote action
        
        Args:
            npc_id: NPC identifier
            emote_id: Emote ID (rAthena emote constants)
            duration: Emote duration in milliseconds
        
        Returns:
            True if published successfully
        """
        action_data = {
            "emote_id": emote_id,
            "duration": duration
        }
        
        return await self.publish_npc_action(npc_id, "emote", action_data)
    
    async def publish_batch_actions(
        self,
        actions: List[Dict[str, Any]]
    ) -> int:
        """
        Publish multiple NPC actions in batch
        
        Args:
            actions: List of action dictionaries with npc_id, action_type, action_data
        
        Returns:
            Number of successfully published actions
        """
        success_count = 0
        
        for action in actions:
            npc_id = action.get("npc_id")
            action_type = action.get("action_type")
            action_data = action.get("action_data", {})
            
            if await self.publish_npc_action(npc_id, action_type, action_data):
                success_count += 1
        
        logger.info(f"[PUBSUB BATCH] Published {success_count}/{len(actions)} actions")
        return success_count


# Global Pub/Sub manager instance
_pubsub_manager: Optional[PubSubManager] = None


def get_pubsub_manager() -> PubSubManager:
    """Get or create global Pub/Sub manager instance"""
    global _pubsub_manager
    if _pubsub_manager is None:
        _pubsub_manager = PubSubManager()
    return _pubsub_manager

