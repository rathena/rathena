"""
Faction Agent - Manages World Faction Alignment System
Implements 4-tier LLM optimization for persistent world alignment
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class FactionType:
    """Faction types"""
    RUNE_ALLIANCE = "rune_alliance"
    SHADOW_CULT = "shadow_cult"
    NATURE_SPIRITS = "nature_spirits"


class FactionAgent(BaseAIAgent):
    """
    Manages persistent world faction alignment system.
    
    Factions:
    - Rune Alliance: Order, city guards, traditional magic
    - Shadow Cult: Chaos, dark magic, forbidden knowledge
    - Nature Spirits: Balance, elemental forces, harmony
    
    World State Effects:
    - Strongest faction influences world aesthetics
    - Faction-specific NPCs spawn
    - Shop prices vary by faction affinity
    - Special quests unlock per faction
    
    4-Tier Optimization:
    - Tier 1: Rule-based alignment shifts
    - Tier 2: Cached faction decisions
    - Tier 3: Batched reputation calculations
    - Tier 4: LLM for creative faction lore
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Faction Agent"""
        super().__init__(
            agent_id="faction_agent",
            agent_type="faction",
            config=config
        )
        
        # Faction configurations
        self.factions = {
            FactionType.RUNE_ALLIANCE: {
                "name": "Rune Alliance",
                "description": "Order, law, and traditional magic",
                "base_alignment": 0,
                "color": "blue",
                "bonus_stats": {"def": 5, "mdef": 5}
            },
            FactionType.SHADOW_CULT: {
                "name": "Shadow Cult",
                "description": "Chaos, dark magic, and forbidden knowledge",
                "base_alignment": 0,
                "color": "purple",
                "bonus_stats": {"matk": 5, "crit": 3}
            },
            FactionType.NATURE_SPIRITS: {
                "name": "Nature Spirits",
                "description": "Balance, harmony, and elemental forces",
                "base_alignment": 0,
                "color": "green",
                "bonus_stats": {"max_hp": 100, "heal_rate": 2}
            }
        }
        
        # Alignment thresholds for world effects
        self.alignment_thresholds = {
            "minor": 1000,    # Minor faction influence
            "moderate": 3000,  # Moderate faction dominance
            "major": 5000,     # Major faction dominance
            "total": 8000      # Complete faction control
        }
        
        # Reputation tier thresholds
        self.reputation_tiers = {
            0: {"name": "Unknown", "threshold": 0},
            1: {"name": "Acquainted", "threshold": 1000},
            2: {"name": "Friendly", "threshold": 3000},
            3: {"name": "Trusted", "threshold": 5000},
            4: {"name": "Honored", "threshold": 7000},
            5: {"name": "Exalted", "threshold": 10000}
        }
        
        # Action type reputation values
        self.action_reputation_values = {
            "quest_complete": 100,
            "monster_kill": 10,
            "donation": 50,
            "pvp_defend": 200,
            "pvp_attack": -150
        }
        
        logger.info("Faction Agent initialized with 3 factions")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for faction management"""
        from crewai import Agent
        
        return Agent(
            role="Faction System Manager",
            goal="Manage world faction alignment and player reputation",
            backstory="An AI system that tracks faction influence and player standing",
            verbose=settings.crewai_verbose,
            allow_delegation=False,
            llm=self.llm_provider if hasattr(self, 'llm_provider') else None
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """Process method required by BaseAIAgent"""
        return AgentResponse(
            agent_type=self.agent_type,
            success=True,
            data={},
            confidence=1.0
        )
    
    # ========================================================================
    # PUBLIC API METHODS
    # ========================================================================
    
    async def update_world_alignment(
        self,
        player_actions: List[Dict[str, Any]]
    ) -> AgentResponse:
        """
        Update global faction alignment based on player actions.
        
        Actions that shift alignment:
        - Completing faction quests
        - Killing faction-aligned monsters
        - Using faction-specific items
        - Participating in faction events
        
        Args:
            player_actions: List of recent player actions affecting factions
            
        Returns:
            AgentResponse with updated alignment and triggered effects
        """
        try:
            logger.info(f"Updating faction alignment from {len(player_actions)} actions")
            
            # Step 1: Calculate alignment shifts from actions (Tier 1: Rule-based)
            alignment_changes = {}
            for faction_id in self.factions.keys():
                alignment_changes[faction_id] = 0
            
            for action in player_actions:
                faction_id = action.get('faction_id')
                shift_amount = action.get('alignment_change', 0)
                
                if faction_id in alignment_changes:
                    alignment_changes[faction_id] += shift_amount
            
            # Step 2: Apply decay to prevent permanent dominance
            decay_rate = getattr(settings, 'faction_decay_rate', 0.01)
            current_alignments = await self._get_current_alignments()
            
            for faction_id, current_value in current_alignments.items():
                if abs(current_value) > 100:
                    decay = int(current_value * decay_rate)
                    alignment_changes[faction_id] -= decay
            
            # Step 3: Update database
            updated_alignments = {}
            threshold_crossed = []
            
            for faction_id, change in alignment_changes.items():
                if change != 0:
                    new_alignment = await self._update_faction_alignment(faction_id, change)
                    updated_alignments[faction_id] = new_alignment
                    
                    # Check if threshold crossed
                    if abs(new_alignment) >= self.alignment_thresholds['minor']:
                        threshold_crossed.append(faction_id)
            
            # Step 4: Trigger world effects if needed
            effects_triggered = []
            if threshold_crossed:
                dominant_faction = await self._determine_dominant_faction()
                if dominant_faction:
                    effects = await self.trigger_faction_effects(dominant_faction)
                    effects_triggered.append(effects)
            
            # Step 5: Clear cache
            await db.delete("faction:alignments:current")
            
            result_data = {
                "alignment_changes": alignment_changes,
                "updated_alignments": updated_alignments,
                "threshold_crossed": threshold_crossed,
                "effects_triggered": effects_triggered,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Faction alignment updated: {len(alignment_changes)} changes")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Faction alignment updated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to update faction alignment: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def calculate_player_reputation(
        self,
        player_id: int,
        faction_id: str
    ) -> int:
        """
        Calculate player's reputation with a faction.
        
        Reputation sources:
        - Quests completed for faction
        - Monsters killed (aligned with faction)
        - Items donated
        - PvP actions (defending faction territory)
        
        Args:
            player_id: Player ID
            faction_id: Faction ID
            
        Returns:
            Reputation score (0-10000)
        """
        try:
            # Try cache first
            cache_key = f"faction:reputation:{player_id}:{faction_id}"
            cached_rep = await db.get(cache_key)
            if cached_rep is not None:
                return int(cached_rep)
            
            # Query database
            query = """
                SELECT reputation
                FROM player_faction_reputation
                WHERE player_id = $1 AND faction_id = $2
            """
            
            result = await postgres_db.fetch_one(query, player_id, faction_id)
            reputation = result['reputation'] if result else 0
            
            # Cache for 5 minutes
            await db.set(cache_key, reputation, expire=300)
            
            return reputation
            
        except Exception as e:
            logger.error(f"Failed to calculate reputation: {e}")
            return 0
    
    async def trigger_faction_effects(
        self,
        dominant_faction: str
    ) -> Dict[str, Any]:
        """
        Apply world effects based on dominant faction.
        
        Effects:
        - Spawn faction NPCs
        - Adjust shop prices
        - Enable faction quests
        - Apply visual changes (future: client-side)
        - Modify monster spawns
        
        Args:
            dominant_faction: Faction ID of dominant faction
            
        Returns:
            Dict of triggered effects
        """
        try:
            effects = {
                "faction": dominant_faction,
                "npcs_spawned": [],
                "price_adjustments": {},
                "quests_unlocked": [],
                "visual_changes": [],
                "monster_spawns": []
            }
            
            faction_config = self.factions.get(dominant_faction, {})
            
            # Example effects (would be expanded in production)
            effects["visual_changes"].append(f"World tint: {faction_config.get('color')}")
            effects["quests_unlocked"].append(f"{faction_config.get('name')} Campaign")
            
            # Store effects in database
            query = """
                UPDATE faction_alignment
                SET is_dominant = FALSE
                WHERE faction_id != $1
            """
            await postgres_db.execute(query, dominant_faction)
            
            query = """
                UPDATE faction_alignment
                SET is_dominant = TRUE, last_shift = NOW()
                WHERE faction_id = $1
            """
            await postgres_db.execute(query, dominant_faction)
            
            logger.info(f"✓ Triggered effects for {dominant_faction}")
            return effects
            
        except Exception as e:
            logger.error(f"Failed to trigger faction effects: {e}")
            return {}
    
    async def get_faction_rewards(
        self,
        player_id: int,
        faction_id: str,
        reputation_tier: int
    ) -> Dict[str, Any]:
        """
        Get available rewards for player's faction tier.
        
        Reward Tiers (by reputation):
        - Tier 1 (1000): Basic cosmetics
        - Tier 2 (3000): Faction buffs
        - Tier 3 (5000): Exclusive items
        - Tier 4 (7000): Special skills
        - Tier 5 (10000): Unique titles
        
        Args:
            player_id: Player ID
            faction_id: Faction ID
            reputation_tier: Player's current tier
            
        Returns:
            Dict of available rewards
        """
        try:
            tier_config = self.reputation_tiers.get(reputation_tier, {})
            faction_config = self.factions.get(faction_id, {})
            
            rewards = {
                "tier": reputation_tier,
                "tier_name": tier_config.get('name', 'Unknown'),
                "cosmetics": [],
                "buffs": [],
                "items": [],
                "skills": [],
                "titles": []
            }
            
            # Tier-based rewards
            if reputation_tier >= 1:
                rewards["cosmetics"].append(f"{faction_config.get('name')} Emblem")
            
            if reputation_tier >= 2:
                rewards["buffs"].extend([f"+5% {stat}" for stat in faction_config.get('bonus_stats', {}).keys()])
            
            if reputation_tier >= 3:
                rewards["items"].append(f"{faction_config.get('name')} Weapon")
            
            if reputation_tier >= 4:
                rewards["skills"].append(f"{faction_config.get('name')} Special Skill")
            
            if reputation_tier >= 5:
                rewards["titles"].append(f"Champion of {faction_config.get('name')}")
            
            return rewards
            
        except Exception as e:
            logger.error(f"Failed to get faction rewards: {e}")
            return {}
    
    async def record_faction_action(
        self,
        player_id: int,
        faction_id: str,
        action_type: str,
        action_data: Optional[Dict[str, Any]] = None
    ) -> bool:
        """
        Record a faction-aligned action.
        
        Args:
            player_id: Player ID
            faction_id: Faction ID
            action_type: Type of action
            action_data: Additional action data
            
        Returns:
            Success status
        """
        try:
            # Calculate reputation and alignment changes
            reputation_change = self.action_reputation_values.get(action_type, 0)
            alignment_change = reputation_change // 10  # Alignment is 1/10 of reputation
            
            # Record in database
            query = """
                INSERT INTO faction_actions
                (player_id, faction_id, action_type, reputation_change, alignment_change, action_data)
                VALUES ($1, $2, $3, $4, $5, $6)
            """
            
            await postgres_db.execute(
                query,
                player_id,
                faction_id,
                action_type,
                reputation_change,
                alignment_change,
                json.dumps(action_data) if action_data else None
            )
            
            # Update player reputation
            await self._update_player_reputation(player_id, faction_id, reputation_change)
            
            # Clear cache
            await db.delete(f"faction:reputation:{player_id}:{faction_id}")
            
            logger.debug(f"Recorded faction action: player {player_id}, faction {faction_id}, type {action_type}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to record faction action: {e}")
            return False
    
    async def get_world_alignment(self) -> Dict[str, Any]:
        """Get current world faction alignment"""
        try:
            # Try cache first
            cached = await db.get("faction:alignments:current")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT faction_id, faction_name, alignment_score, is_dominant
                FROM faction_alignment
                ORDER BY alignment_score DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            alignments = {
                "factions": [dict(row) for row in rows],
                "dominant_faction": None,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            # Find dominant faction
            for faction in alignments['factions']:
                if faction['is_dominant']:
                    alignments['dominant_faction'] = faction['faction_id']
                    break
            
            # Cache for 5 minutes
            await db.set("faction:alignments:current", alignments, expire=300)
            
            return alignments
            
        except Exception as e:
            logger.error(f"Failed to get world alignment: {e}")
            return {"factions": [], "dominant_faction": None}
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _get_current_alignments(self) -> Dict[str, int]:
        """Get current alignment scores for all factions"""
        try:
            query = """
                SELECT faction_id, alignment_score
                FROM faction_alignment
            """
            
            rows = await postgres_db.fetch_all(query)
            return {row['faction_id']: row['alignment_score'] for row in rows}
            
        except Exception as e:
            logger.debug(f"Failed to get current alignments: {e}")
            return {faction_id: 0 for faction_id in self.factions.keys()}
    
    async def _update_faction_alignment(self, faction_id: str, change: int) -> int:
        """Update faction alignment score"""
        try:
            query = """
                UPDATE faction_alignment
                SET alignment_score = alignment_score + $1,
                    last_shift = NOW()
                WHERE faction_id = $2
                RETURNING alignment_score
            """
            
            result = await postgres_db.fetch_one(query, change, faction_id)
            return result['alignment_score'] if result else 0
            
        except Exception as e:
            logger.error(f"Failed to update faction alignment: {e}")
            return 0
    
    async def _determine_dominant_faction(self) -> Optional[str]:
        """Determine which faction is currently dominant"""
        try:
            query = """
                SELECT faction_id, alignment_score
                FROM faction_alignment
                WHERE ABS(alignment_score) >= $1
                ORDER BY ABS(alignment_score) DESC
                LIMIT 1
            """
            
            result = await postgres_db.fetch_one(query, self.alignment_thresholds['minor'])
            return result['faction_id'] if result else None
            
        except Exception as e:
            logger.debug(f"Failed to determine dominant faction: {e}")
            return None
    
    async def _update_player_reputation(
        self,
        player_id: int,
        faction_id: str,
        change: int
    ) -> int:
        """Update player's reputation with faction"""
        try:
            # Upsert player reputation
            query = """
                INSERT INTO player_faction_reputation
                (player_id, faction_id, reputation, actions_completed)
                VALUES ($1, $2, $3, 1)
                ON CONFLICT (player_id, faction_id)
                DO UPDATE SET
                    reputation = GREATEST(0, LEAST(10000, player_faction_reputation.reputation + $3)),
                    actions_completed = player_faction_reputation.actions_completed + 1,
                    last_action = NOW()
                RETURNING reputation, reputation_tier
            """
            
            result = await postgres_db.fetch_one(query, player_id, faction_id, change)
            
            if result:
                new_reputation = result['reputation']
                
                # Update tier if needed
                new_tier = self._calculate_reputation_tier(new_reputation)
                if new_tier != result['reputation_tier']:
                    await postgres_db.execute(
                        "UPDATE player_faction_reputation SET reputation_tier = $1 WHERE player_id = $2 AND faction_id = $3",
                        new_tier, player_id, faction_id
                    )
                
                return new_reputation
            
            return 0
            
        except Exception as e:
            logger.error(f"Failed to update player reputation: {e}")
            return 0
    
    def _calculate_reputation_tier(self, reputation: int) -> int:
        """Calculate reputation tier from score"""
        for tier in range(5, -1, -1):
            if reputation >= self.reputation_tiers[tier]['threshold']:
                return tier
        return 0


# Global instance
faction_agent = FactionAgent()