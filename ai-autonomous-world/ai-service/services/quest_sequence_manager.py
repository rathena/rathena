"""
Quest sequence and chain management
Handles quest chains, prerequisites, and unlocking
"""

from loguru import logger
from typing import Dict, Any, List, Optional
import json

try:
    from ai_service.models.quest import Quest, QuestStatus
    from ai_service.database import db
    from ai_service.services.quest_trigger_engine import get_trigger_engine
except ImportError:
    from models.quest import Quest, QuestStatus
    from database import db
    from services.quest_trigger_engine import get_trigger_engine


class QuestSequenceManager:
    """Manages quest sequences and chains"""
    
    def __init__(self):
        self.logger = logger.bind(component="QuestSequenceManager")
        self.trigger_engine = get_trigger_engine()
    
    async def check_quest_unlock(
        self,
        quest: Quest,
        player_id: str,
        npc_id: str,
        context: Dict[str, Any]
    ) -> bool:
        """
        Check if a quest can be unlocked for a player
        
        Args:
            quest: The quest to check
            player_id: Player ID
            npc_id: NPC ID
            context: Context data for trigger evaluation
        
        Returns:
            True if quest can be unlocked
        """
        # Check if already unlocked/completed
        quest_state_key = f"quest_state:{player_id}:{quest.quest_id}"
        quest_state = await db.client.get(quest_state_key)
        
        if quest_state:
            state_data = json.loads(quest_state)
            status = state_data.get("status")
            if status in [QuestStatus.ACTIVE.value, QuestStatus.COMPLETED.value]:
                return False  # Already unlocked or completed
        
        # Check prerequisite quests
        if quest.prerequisite_quests:
            completed_quests = await self._get_completed_quests(player_id)
            for prereq_id in quest.prerequisite_quests:
                if prereq_id not in completed_quests:
                    self.logger.debug(f"Quest {quest.quest_id} locked: missing prerequisite {prereq_id}")
                    return False
        
        # Check relationship threshold
        if quest.relationship_unlock_threshold is not None:
            relationship_key = f"relationship:{npc_id}:{player_id}"
            relationship_level = await db.client.get(relationship_key)
            relationship_level = float(relationship_level) if relationship_level else 0.0
            
            if relationship_level < quest.relationship_unlock_threshold:
                self.logger.debug(
                    f"Quest {quest.quest_id} locked: relationship {relationship_level:.1f} "
                    f"< {quest.relationship_unlock_threshold:.1f}"
                )
                return False
        
        # Evaluate triggers
        if quest.triggers:
            # Add player_id and npc_id to context
            eval_context = {**context, "player_id": player_id, "npc_id": npc_id}
            
            met_triggers = await self.trigger_engine.evaluate_all_triggers(
                quest.triggers,
                eval_context
            )
            
            if not met_triggers:
                self.logger.debug(f"Quest {quest.quest_id} locked: no triggers met")
                return False
        
        self.logger.info(f"Quest {quest.quest_id} unlocked for player {player_id}")
        return True
    
    async def unlock_quest(
        self,
        quest: Quest,
        player_id: str
    ) -> bool:
        """
        Unlock a quest for a player
        
        Args:
            quest: The quest to unlock
            player_id: Player ID
        
        Returns:
            True if successfully unlocked
        """
        try:
            quest_state_key = f"quest_state:{player_id}:{quest.quest_id}"
            
            # Create quest state
            quest_state = {
                "quest_id": quest.quest_id,
                "status": QuestStatus.AVAILABLE.value,
                "unlocked_at": datetime.utcnow().isoformat(),
                "objectives": [obj.dict() for obj in quest.objectives]
            }
            
            await db.client.set(quest_state_key, json.dumps(quest_state))
            
            # Add to player's available quests list
            available_key = f"available_quests:{player_id}"
            await db.client.sadd(available_key, quest.quest_id)
            
            self.logger.info(f"Quest {quest.quest_id} unlocked for player {player_id}")
            return True
            
        except Exception as e:
            self.logger.error(f"Error unlocking quest: {e}", exc_info=True)
            return False
    
    async def complete_quest(
        self,
        quest_id: str,
        player_id: str
    ) -> Optional[List[str]]:
        """
        Mark quest as completed and check for unlocked quests
        
        Args:
            quest_id: Quest ID
            player_id: Player ID
        
        Returns:
            List of newly unlocked quest IDs, or None on error
        """
        try:
            # Update quest state
            quest_state_key = f"quest_state:{player_id}:{quest_id}"
            quest_state = await db.client.get(quest_state_key)
            
            if not quest_state:
                self.logger.warning(f"Quest state not found: {quest_id}")
                return None
            
            state_data = json.loads(quest_state)
            state_data["status"] = QuestStatus.COMPLETED.value
            state_data["completed_at"] = datetime.utcnow().isoformat()
            
            await db.client.set(quest_state_key, json.dumps(state_data))
            
            # Add to completed quests
            completed_key = f"completed_quests:{player_id}"
            await db.client.sadd(completed_key, quest_id)
            
            # Remove from available quests
            available_key = f"available_quests:{player_id}"
            await db.client.srem(available_key, quest_id)
            
            self.logger.info(f"Quest {quest_id} completed by player {player_id}")
            
            # Check for newly unlocked quests
            return await self._check_newly_unlocked_quests(quest_id, player_id)
            
        except Exception as e:
            self.logger.error(f"Error completing quest: {e}", exc_info=True)
            return None
    
    async def _get_completed_quests(self, player_id: str) -> List[str]:
        """Get list of completed quest IDs for a player"""
        completed_key = f"completed_quests:{player_id}"
        completed = await db.client.smembers(completed_key)
        return list(completed) if completed else []
    
    async def _check_newly_unlocked_quests(
        self,
        completed_quest_id: str,
        player_id: str
    ) -> List[str]:
        """Check for quests that are now unlocked after completing a quest"""
        # This would search for quests with this quest as a prerequisite
        # For now, return empty list - full implementation would query quest database
        return []


# Global instance
_sequence_manager = None

def get_sequence_manager() -> QuestSequenceManager:
    """Get global sequence manager instance"""
    global _sequence_manager
    if _sequence_manager is None:
        _sequence_manager = QuestSequenceManager()
    return _sequence_manager

