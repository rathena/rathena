"""
Faction System Tasks - Reputation and Relationship Management
Implements faction reputation decay and dynamic faction relationships
"""

import asyncio
from datetime import datetime, timedelta
from typing import List, Dict, Any, Optional
from loguru import logger

from ..config import settings
from ..database import db, postgres_db
from ..models.faction import Faction, PlayerReputation, RelationshipStatus


class FactionManager:
    """
    Manages faction system including reputation and relationships
    Handles reputation decay and dynamic faction interactions
    """
    
    def __init__(self):
        logger.info("FactionManager initialized")
    
    async def process_reputation_decay(self):
        """
        Process reputation decay for all players
        Called periodically by scheduler
        """
        if not settings.faction_enabled or not settings.faction_reputation_decay_enabled:
            return
        
        logger.debug("Processing faction reputation decay...")
        
        try:
            # Get all player reputations that need decay
            reputations = await self._get_reputations_for_decay()
            
            decayed_count = 0
            
            for rep in reputations:
                player_id = rep['player_id']
                faction_id = rep['faction_id']
                current_rep = rep['reputation']
                
                # Calculate decay amount (move towards neutral 0)
                decay_amount = self._calculate_decay_amount(current_rep)
                
                if decay_amount != 0:
                    new_rep = current_rep + decay_amount
                    await self._update_reputation_value(player_id, faction_id, new_rep)
                    decayed_count += 1
            
            if decayed_count > 0:
                logger.info(f"Reputation decay: {decayed_count} reputations decayed")
        
        except Exception as e:
            logger.error(f"Error processing reputation decay: {e}", exc_info=True)
    
    async def create_faction(
        self,
        faction_id: str,
        name: str,
        description: Optional[str] = None,
        alignment: str = "neutral"
    ) -> Dict[str, Any]:
        """Create a new faction"""
        try:
            query = """
                INSERT INTO factions (faction_id, name, description, alignment, created_at)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING *
            """
            result = await postgres_db.fetch_one(
                query,
                faction_id,
                name,
                description,
                alignment,
                datetime.utcnow()
            )
            
            faction = dict(result)
            
            # Cache
            await db.set(f"faction:{faction_id}", faction, expire=3600)
            
            logger.info(f"Faction created: {faction_id} ({name})")
            return faction
        
        except Exception as e:
            logger.error(f"Error creating faction: {e}", exc_info=True)
            raise
    
    async def get_faction_info(self, faction_id: str) -> Optional[Dict[str, Any]]:
        """Get faction information"""
        try:
            # Try cache first
            cache_key = f"faction:{faction_id}"
            cached = await db.get(cache_key)
            
            if cached:
                return cached
            
            # Query database
            query = "SELECT * FROM factions WHERE faction_id = $1"
            result = await postgres_db.fetch_one(query, faction_id)
            
            if result:
                faction = dict(result)
                await db.set(cache_key, faction, expire=3600)
                return faction
            
            return None
        
        except Exception as e:
            logger.error(f"Error getting faction info: {e}")
            return None
    
    async def list_all_factions(self) -> List[Dict[str, Any]]:
        """List all factions"""
        try:
            cache_key = "factions:all"
            cached = await db.get(cache_key)
            
            if cached:
                return cached
            
            query = "SELECT * FROM factions ORDER BY name"
            results = await postgres_db.fetch_all(query)
            
            factions = [dict(row) for row in results]
            
            # Cache for 10 minutes
            await db.set(cache_key, factions, expire=600)
            
            return factions
        
        except Exception as e:
            logger.error(f"Error listing factions: {e}")
            return []
    
    async def update_player_reputation(
        self,
        player_id: str,
        faction_id: str,
        change: int,
        reason: Optional[str] = None
    ) -> Dict[str, Any]:
        """Update player reputation with a faction"""
        try:
            # Get current reputation
            current_rep = await self._get_reputation(player_id, faction_id)
            
            # Calculate new reputation
            new_rep = max(-100, min(100, current_rep + change))
            
            # Update or insert
            await self._upsert_reputation(player_id, faction_id, new_rep, reason)
            
            # Invalidate cache
            await db.delete(f"reputation:{player_id}:{faction_id}")
            await db.delete(f"reputation:{player_id}:all")
            
            logger.info(f"Reputation updated: Player {player_id} with {faction_id}: {current_rep} -> {new_rep} ({change:+d})")
            
            return {
                "old_reputation": current_rep,
                "new_reputation": new_rep,
                "change": change
            }
        
        except Exception as e:
            logger.error(f"Error updating reputation: {e}", exc_info=True)
            raise

    async def get_player_all_reputations(self, player_id: str) -> List[Dict[str, Any]]:
        """Get player's reputation with all factions"""
        try:
            cache_key = f"reputation:{player_id}:all"
            cached = await db.get(cache_key)

            if cached:
                return cached

            query = """
                SELECT fr.*, f.name as faction_name
                FROM faction_reputations fr
                JOIN factions f ON fr.faction_id = f.faction_id
                WHERE fr.player_id = $1
                ORDER BY fr.reputation DESC
            """
            results = await postgres_db.fetch_all(query, player_id)

            reputations = [dict(row) for row in results]

            # Cache for 5 minutes
            await db.set(cache_key, reputations, expire=300)

            return reputations

        except Exception as e:
            logger.error(f"Error getting player reputations: {e}")
            return []

    async def get_player_faction_reputation(self, player_id: str, faction_id: str) -> int:
        """Get player's reputation with specific faction"""
        return await self._get_reputation(player_id, faction_id)

    async def update_faction_relationship(
        self,
        faction_id_1: str,
        faction_id_2: str,
        relationship_value: int
    ) -> Dict[str, Any]:
        """Update relationship between two factions"""
        try:
            # Ensure consistent ordering
            if faction_id_1 > faction_id_2:
                faction_id_1, faction_id_2 = faction_id_2, faction_id_1

            # Clamp value
            relationship_value = max(-100, min(100, relationship_value))

            # Upsert relationship
            query = """
                INSERT INTO faction_relationships (faction_id_1, faction_id_2, relationship_value, updated_at)
                VALUES ($1, $2, $3, $4)
                ON CONFLICT (faction_id_1, faction_id_2)
                DO UPDATE SET relationship_value = $3, updated_at = $4
            """
            await postgres_db.execute(query, faction_id_1, faction_id_2, relationship_value, datetime.utcnow())

            # Invalidate cache
            await db.delete(f"faction_rel:{faction_id_1}:{faction_id_2}")

            logger.info(f"Faction relationship updated: {faction_id_1} <-> {faction_id_2}: {relationship_value}")

            return {
                "faction_id_1": faction_id_1,
                "faction_id_2": faction_id_2,
                "relationship_value": relationship_value
            }

        except Exception as e:
            logger.error(f"Error updating faction relationship: {e}", exc_info=True)
            raise

    async def get_faction_relationship(self, faction_id_1: str, faction_id_2: str) -> int:
        """Get relationship between two factions"""
        try:
            # Ensure consistent ordering
            if faction_id_1 > faction_id_2:
                faction_id_1, faction_id_2 = faction_id_2, faction_id_1

            cache_key = f"faction_rel:{faction_id_1}:{faction_id_2}"
            cached = await db.get(cache_key)

            if cached is not None:
                return int(cached)

            query = """
                SELECT relationship_value FROM faction_relationships
                WHERE faction_id_1 = $1 AND faction_id_2 = $2
            """
            result = await postgres_db.fetch_one(query, faction_id_1, faction_id_2)

            if result:
                value = result['relationship_value']
                await db.set(cache_key, value, expire=600)
                return value

            # Default neutral
            return 0

        except Exception as e:
            logger.error(f"Error getting faction relationship: {e}")
            return 0

    async def get_all_faction_relationships(self, faction_id: str) -> List[Dict[str, Any]]:
        """Get all relationships for a faction"""
        try:
            query = """
                SELECT
                    CASE
                        WHEN faction_id_1 = $1 THEN faction_id_2
                        ELSE faction_id_1
                    END as other_faction_id,
                    relationship_value
                FROM faction_relationships
                WHERE faction_id_1 = $1 OR faction_id_2 = $1
            """
            results = await postgres_db.fetch_all(query, faction_id)

            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting faction relationships: {e}")
            return []

    # ========================================================================
    # Helper Methods
    # ========================================================================

    async def _get_reputations_for_decay(self) -> List[Dict[str, Any]]:
        """Get reputations that need decay processing"""
        try:
            query = """
                SELECT player_id, faction_id, reputation, last_updated
                FROM faction_reputations
                WHERE reputation != 0
                AND last_updated < $1
            """
            # Decay if not updated in last week
            cutoff = datetime.utcnow() - timedelta(days=7)
            results = await postgres_db.fetch_all(query, cutoff)

            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting reputations for decay: {e}")
            return []

    def _calculate_decay_amount(self, current_rep: int) -> int:
        """Calculate reputation decay amount (move towards 0)"""
        if current_rep == 0:
            return 0

        # Decay 1% per week, minimum 1 point
        decay = max(1, abs(current_rep) // 100)

        if current_rep > 0:
            return -decay
        else:
            return decay

    async def _update_reputation_value(self, player_id: str, faction_id: str, new_rep: int):
        """Update reputation value in database"""
        try:
            query = """
                UPDATE faction_reputations
                SET reputation = $1, last_updated = $2
                WHERE player_id = $3 AND faction_id = $4
            """
            await postgres_db.execute(query, new_rep, datetime.utcnow(), player_id, faction_id)

            # Invalidate cache
            await db.delete(f"reputation:{player_id}:{faction_id}")

        except Exception as e:
            logger.error(f"Error updating reputation value: {e}")

    async def _get_reputation(self, player_id: str, faction_id: str) -> int:
        """Get current reputation"""
        try:
            cache_key = f"reputation:{player_id}:{faction_id}"
            cached = await db.get(cache_key)

            if cached is not None:
                return int(cached)

            query = """
                SELECT reputation FROM faction_reputations
                WHERE player_id = $1 AND faction_id = $2
            """
            result = await postgres_db.fetch_one(query, player_id, faction_id)

            if result:
                rep = result['reputation']
                await db.set(cache_key, rep, expire=300)
                return rep

            # Default neutral
            return 0

        except Exception as e:
            logger.error(f"Error getting reputation: {e}")
            return 0

    async def _upsert_reputation(
        self,
        player_id: str,
        faction_id: str,
        reputation: int,
        reason: Optional[str]
    ):
        """Insert or update reputation"""
        try:
            query = """
                INSERT INTO faction_reputations (player_id, faction_id, reputation, last_updated, last_change_reason)
                VALUES ($1, $2, $3, $4, $5)
                ON CONFLICT (player_id, faction_id)
                DO UPDATE SET reputation = $3, last_updated = $4, last_change_reason = $5
            """
            await postgres_db.execute(
                query,
                player_id,
                faction_id,
                reputation,
                datetime.utcnow(),
                reason
            )

        except Exception as e:
            logger.error(f"Error upserting reputation: {e}")
            raise


# Global instance
faction_manager = FactionManager()


# Public functions for scheduler and router
async def process_reputation_decay():
    """Process reputation decay"""
    await faction_manager.process_reputation_decay()


async def create_faction(faction_id: str, name: str, description: Optional[str] = None, alignment: str = "neutral") -> Dict[str, Any]:
    """Create faction"""
    return await faction_manager.create_faction(faction_id, name, description, alignment)


async def get_faction_info(faction_id: str) -> Optional[Dict[str, Any]]:
    """Get faction info"""
    return await faction_manager.get_faction_info(faction_id)


async def list_all_factions() -> List[Dict[str, Any]]:
    """List all factions"""
    return await faction_manager.list_all_factions()


async def update_player_reputation(player_id: str, faction_id: str, change: int, reason: Optional[str] = None) -> Dict[str, Any]:
    """Update player reputation"""
    return await faction_manager.update_player_reputation(player_id, faction_id, change, reason)


async def get_player_all_reputations(player_id: str) -> List[Dict[str, Any]]:
    """Get player all reputations"""
    return await faction_manager.get_player_all_reputations(player_id)


async def get_player_faction_reputation(player_id: str, faction_id: str) -> int:
    """Get player faction reputation"""
    return await faction_manager.get_player_faction_reputation(player_id, faction_id)


async def update_faction_relationship(faction_id_1: str, faction_id_2: str, relationship_value: int) -> Dict[str, Any]:
    """Update faction relationship"""
    return await faction_manager.update_faction_relationship(faction_id_1, faction_id_2, relationship_value)


async def get_faction_relationship(faction_id_1: str, faction_id_2: str) -> int:
    """Get faction relationship"""
    return await faction_manager.get_faction_relationship(faction_id_1, faction_id_2)


async def get_all_faction_relationships(faction_id: str) -> List[Dict[str, Any]]:
    """Get all faction relationships"""
    return await faction_manager.get_all_faction_relationships(faction_id)


