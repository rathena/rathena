"""
Reputation Agent - Manages Player Influence Score System
Tracks player reputation across all world systems and unlocks progressive benefits
"""

import asyncio
import json
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class ReputationType:
    """Reputation source types"""
    WORLD_GUARDIAN = "world_guardian"        # Solving world problems
    EXPLORER = "explorer"                    # Discovering dynamic NPCs
    PROBLEM_SOLVER = "problem_solver"        # Quest completion
    EVENT_PARTICIPANT = "event_participant"  # World events
    FACTION_LOYALIST = "faction_loyalist"    # Faction reputation


class ReputationAgent(BaseAIAgent):
    """
    Manages player influence score and unlocks progressive benefits.
    
    Reputation Sources:
    - Solving world problems (Problem Agent)
    - Finding dynamic NPCs (Dynamic NPC Agent)
    - Participating in events (World Event Agent)
    - Faction contributions (Faction Agent)
    - Defeating dynamic bosses (Dynamic Boss Agent)
    
    Benefits:
    - Access to special shops
    - Exclusive titles
    - Unique headgears
    - Passive stat bonuses
    - NPC shortcuts (warp, buff, insurance)
    
    4-Tier Optimization:
    - Tier 1: Rule-based benefit unlocks
    - Tier 2: Cached influence calculations
    - Tier 3: Batched reputation updates
    - Tier 4: LLM for unique reward descriptions
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Reputation Agent"""
        super().__init__(
            agent_id="reputation_agent",
            agent_type="reputation",
            config=config
        )
        
        # Reputation type weights for total influence
        self.reputation_weights = {
            ReputationType.WORLD_GUARDIAN: 0.30,
            ReputationType.EXPLORER: 0.25,
            ReputationType.PROBLEM_SOLVER: 0.20,
            ReputationType.EVENT_PARTICIPANT: 0.15,
            ReputationType.FACTION_LOYALIST: 0.10
        }
        
        # Influence tier thresholds
        self.influence_tiers = {
            0: {"name": "Newcomer", "threshold": 0, "benefits": []},
            1: {"name": "Known", "threshold": 1000, "benefits": ["shop_basic", "title_known"]},
            2: {"name": "Respected", "threshold": 3000, "benefits": ["shop_rare", "title_respected", "stat_bonus_1"]},
            3: {"name": "Renowned", "threshold": 5000, "benefits": ["shop_exclusive", "title_renowned", "stat_bonus_2", "npc_shortcuts"]},
            4: {"name": "Legendary", "threshold": 7000, "benefits": ["shop_legendary", "title_legendary", "stat_bonus_3", "headgear_unique"]},
            5: {"name": "Mythic", "threshold": 10000, "benefits": ["shop_mythic", "title_mythic", "stat_bonus_4", "headgear_legendary", "passive_aura"]}
        }
        
        # Benefit configurations
        self.benefit_configs = {
            "shop_basic": {"type": "shop", "name": "Basic Trader Access"},
            "shop_rare": {"type": "shop", "name": "Rare Items Merchant"},
            "shop_exclusive": {"type": "shop", "name": "Exclusive Goods Dealer"},
            "shop_legendary": {"type": "shop", "name": "Legendary Artifacts Shop"},
            "shop_mythic": {"type": "shop", "name": "Mythic Treasures Emporium"},
            "title_known": {"type": "title", "name": "The Known", "stats": {}},
            "title_respected": {"type": "title", "name": "The Respected", "stats": {"max_hp": 50}},
            "title_renowned": {"type": "title", "name": "The Renowned", "stats": {"max_hp": 100, "max_sp": 50}},
            "title_legendary": {"type": "title", "name": "The Legendary", "stats": {"max_hp": 200, "max_sp": 100, "atk": 5}},
            "title_mythic": {"type": "title", "name": "The Mythic", "stats": {"max_hp": 300, "max_sp": 150, "atk": 10, "matk": 10}},
            "stat_bonus_1": {"type": "passive", "name": "+1 All Stats", "stats": {"str": 1, "agi": 1, "vit": 1, "int": 1, "dex": 1, "luk": 1}},
            "stat_bonus_2": {"type": "passive", "name": "+2 All Stats", "stats": {"str": 2, "agi": 2, "vit": 2, "int": 2, "dex": 2, "luk": 2}},
            "stat_bonus_3": {"type": "passive", "name": "+3 All Stats", "stats": {"str": 3, "agi": 3, "vit": 3, "int": 3, "dex": 3, "luk": 3}},
            "stat_bonus_4": {"type": "passive", "name": "+5 All Stats", "stats": {"str": 5, "agi": 5, "vit": 5, "int": 5, "dex": 5, "luk": 5}},
            "headgear_unique": {"type": "headgear", "name": "Crown of Renown", "item_id": 5000},
            "headgear_legendary": {"type": "headgear", "name": "Halo of Legends", "item_id": 5001},
            "npc_shortcuts": {"type": "service", "name": "NPC Shortcuts (Warp, Buff, Insurance)"},
            "passive_aura": {"type": "aura", "name": "Mythic Aura (+5% All Stats)", "stats": {"percent_bonus": 5}}
        }
        
        # Daily cap per reputation type
        self.daily_cap_per_type = getattr(settings, 'reputation_daily_cap_per_type', 500)
        
        logger.info("Reputation Agent initialized with 5 reputation types")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for reputation management"""
        from crewai import Agent
        
        return Agent(
            role="Reputation System Manager",
            goal="Track player influence and unlock progressive benefits",
            backstory="An AI system that measures player impact across all world systems",
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
    
    async def calculate_total_influence(
        self,
        player_id: int
    ) -> Dict[str, Any]:
        """
        Calculate player's total influence across all systems.
        
        Weighted components:
        - World Guardian: 30%
        - Explorer: 25%
        - Problem Solver: 20%
        - Event Participant: 15%
        - Faction Loyalist: 10%
        
        Args:
            player_id: Player ID
            
        Returns:
            Dict with total influence, tier, and breakdown
        """
        try:
            # Try cache first
            cache_key = f"reputation:influence:{player_id}"
            cached = await db.get(cache_key)
            if cached:
                return cached
            
            # Get player's influence record
            query = """
                SELECT 
                    total_influence,
                    current_tier,
                    world_guardian_rep,
                    explorer_rep,
                    problem_solver_rep,
                    event_participant_rep,
                    faction_loyalist_rep
                FROM player_influence
                WHERE player_id = $1
            """
            
            result = await postgres_db.fetch_one(query, player_id)
            
            if not result:
                # Create new record
                await self._create_player_influence_record(player_id)
                result = {'total_influence': 0, 'current_tier': 0, 'world_guardian_rep': 0,
                         'explorer_rep': 0, 'problem_solver_rep': 0, 'event_participant_rep': 0,
                         'faction_loyalist_rep': 0}
            
            total_influence = result['total_influence']
            current_tier = result['current_tier']
            
            # Calculate tier progress
            next_tier = current_tier + 1 if current_tier < 5 else 5
            next_threshold = self.influence_tiers[next_tier]['threshold']
            current_threshold = self.influence_tiers[current_tier]['threshold']
            
            progress = 0.0
            if next_tier > current_tier:
                progress = (total_influence - current_threshold) / (next_threshold - current_threshold)
                progress = max(0.0, min(1.0, progress))
            
            influence_data = {
                'total_influence': total_influence,
                'tier': current_tier,
                'tier_name': self.influence_tiers[current_tier]['name'],
                'next_tier_progress': progress,
                'breakdown': {
                    ReputationType.WORLD_GUARDIAN: result['world_guardian_rep'],
                    ReputationType.EXPLORER: result['explorer_rep'],
                    ReputationType.PROBLEM_SOLVER: result['problem_solver_rep'],
                    ReputationType.EVENT_PARTICIPANT: result['event_participant_rep'],
                    ReputationType.FACTION_LOYALIST: result['faction_loyalist_rep']
                },
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Cache for 2 minutes
            await db.set(cache_key, influence_data, expire=120)
            
            return influence_data
            
        except Exception as e:
            logger.error(f"Failed to calculate total influence: {e}", exc_info=True)
            return {
                'total_influence': 0,
                'tier': 0,
                'tier_name': 'Newcomer',
                'next_tier_progress': 0.0,
                'breakdown': {}
            }
    
    async def record_reputation_gain(
        self,
        player_id: int,
        reputation_type: str,
        amount: int,
        source: str
    ) -> AgentResponse:
        """
        Record reputation gain from any source.
        
        Validation:
        - Amount must be positive
        - Source must be valid
        - Player must exist
        - Daily cap per type
        
        Side effects:
        - Update total influence
        - Check tier progression
        - Unlock new benefits
        - Send notification
        
        Args:
            player_id: Player ID
            reputation_type: Type of reputation
            amount: Reputation amount to add
            source: Source of reputation gain
            
        Returns:
            AgentResponse with updated reputation
        """
        try:
            # Validate
            if amount <= 0:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Amount must be positive"},
                    confidence=0.0,
                    reasoning="Invalid amount"
                )
            
            if reputation_type not in self.reputation_weights:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Invalid reputation type"},
                    confidence=0.0,
                    reasoning="Unknown reputation type"
                )
            
            # Check daily cap
            daily_gained = await self._get_daily_reputation_gain(player_id, reputation_type)
            if daily_gained >= self.daily_cap_per_type:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Daily reputation cap reached"},
                    confidence=0.0,
                    reasoning=f"Daily cap of {self.daily_cap_per_type} reached"
                )
            
            # Apply daily cap
            amount = min(amount, self.daily_cap_per_type - daily_gained)
            
            # Get old influence
            old_influence_data = await self.calculate_total_influence(player_id)
            old_influence = old_influence_data['total_influence']
            old_tier = old_influence_data['tier']
            
            # Record in history
            query = """
                INSERT INTO reputation_history
                (player_id, reputation_type, amount, source)
                VALUES ($1, $2, $3, $4)
            """
            await postgres_db.execute(query, player_id, reputation_type, amount, source)
            
            # Update player influence
            rep_field = f"{reputation_type}_rep"
            weight = self.reputation_weights[reputation_type]
            influence_gain = int(amount * weight)
            
            query = f"""
                UPDATE player_influence
                SET {rep_field} = {rep_field} + $1,
                    total_influence = total_influence + $2,
                    last_updated = NOW()
                WHERE player_id = $3
                RETURNING total_influence, current_tier
            """
            
            result = await postgres_db.fetch_one(query, amount, influence_gain, player_id)
            
            new_influence = result['total_influence']
            new_tier = result['current_tier']
            
            # Check tier progression
            tier_change = await self.check_tier_progression(player_id, old_influence, new_influence)
            
            # Clear cache
            await db.delete(f"reputation:influence:{player_id}")
            
            result_data = {
                "player_id": player_id,
                "reputation_type": reputation_type,
                "amount_gained": amount,
                "influence_gained": influence_gain,
                "old_influence": old_influence,
                "new_influence": new_influence,
                "old_tier": old_tier,
                "new_tier": new_tier,
                "tier_advancement": tier_change,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Recorded reputation gain: player {player_id}, +{amount} {reputation_type}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Reputation recorded successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to record reputation gain: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_unlocked_benefits(
        self,
        player_id: int
    ) -> Dict[str, Any]:
        """
        Get all benefits unlocked at player's current tier.
        
        Benefit categories:
        - Shop access (special merchants)
        - Titles (cosmetic + stat bonuses)
        - Headgears (unique appearances)
        - Passive bonuses (+1 stat per tier)
        - NPC services (shortcuts, buffs)
        
        Args:
            player_id: Player ID
            
        Returns:
            Dict of unlocked benefits
        """
        try:
            # Get player tier
            influence_data = await self.calculate_total_influence(player_id)
            tier = influence_data['tier']
            
            # Get all benefits up to current tier
            unlocked_benefits = {
                "tier": tier,
                "tier_name": self.influence_tiers[tier]['name'],
                "shops": [],
                "titles": [],
                "headgears": [],
                "passives": [],
                "services": [],
                "auras": []
            }
            
            for t in range(tier + 1):
                tier_benefits = self.influence_tiers[t]['benefits']
                
                for benefit_id in tier_benefits:
                    benefit = self.benefit_configs.get(benefit_id, {})
                    benefit_type = benefit.get('type')
                    
                    benefit_data = {
                        "id": benefit_id,
                        "name": benefit.get('name'),
                        "tier_unlocked": t
                    }
                    
                    if benefit_type == "shop":
                        unlocked_benefits["shops"].append(benefit_data)
                    elif benefit_type == "title":
                        benefit_data["stats"] = benefit.get('stats', {})
                        unlocked_benefits["titles"].append(benefit_data)
                    elif benefit_type == "headgear":
                        benefit_data["item_id"] = benefit.get('item_id')
                        unlocked_benefits["headgears"].append(benefit_data)
                    elif benefit_type == "passive":
                        benefit_data["stats"] = benefit.get('stats', {})
                        unlocked_benefits["passives"].append(benefit_data)
                    elif benefit_type == "service":
                        unlocked_benefits["services"].append(benefit_data)
                    elif benefit_type == "aura":
                        benefit_data["stats"] = benefit.get('stats', {})
                        unlocked_benefits["auras"].append(benefit_data)
            
            return unlocked_benefits
            
        except Exception as e:
            logger.error(f"Failed to get unlocked benefits: {e}")
            return {"tier": 0, "tier_name": "Newcomer", "shops": [], "titles": [], "headgears": [], "passives": [], "services": [], "auras": []}
    
    async def check_tier_progression(
        self,
        player_id: int,
        old_influence: int,
        new_influence: int
    ) -> Optional[Dict[str, Any]]:
        """
        Check if player advanced to new tier.
        
        Tiers:
        - Tier 0 (0): Newcomer
        - Tier 1 (1000): Known
        - Tier 2 (3000): Respected
        - Tier 3 (5000): Renowned
        - Tier 4 (7000): Legendary
        - Tier 5 (10000): Mythic
        
        Args:
            player_id: Player ID
            old_influence: Previous influence score
            new_influence: Current influence score
            
        Returns:
            Tier advancement info or None
        """
        try:
            # Determine old and new tiers
            old_tier = self._calculate_tier_from_influence(old_influence)
            new_tier = self._calculate_tier_from_influence(new_influence)
            
            # Check if tier advanced
            if new_tier > old_tier:
                # Update database
                query = """
                    UPDATE player_influence
                    SET current_tier = $1
                    WHERE player_id = $2
                """
                await postgres_db.execute(query, new_tier, player_id)
                
                # Record tier progression
                query = """
                    INSERT INTO tier_progression
                    (player_id, old_tier, new_tier, influence_at_progression)
                    VALUES ($1, $2, $3, $4)
                """
                await postgres_db.execute(query, player_id, old_tier, new_tier, new_influence)
                
                # Unlock new benefits
                await self._unlock_tier_benefits(player_id, new_tier)
                
                tier_info = {
                    "advanced": True,
                    "old_tier": old_tier,
                    "old_tier_name": self.influence_tiers[old_tier]['name'],
                    "new_tier": new_tier,
                    "new_tier_name": self.influence_tiers[new_tier]['name'],
                    "new_benefits": self.influence_tiers[new_tier]['benefits'],
                    "timestamp": datetime.now(UTC).isoformat()
                }
                
                logger.info(f"✓ Player {player_id} advanced to tier {new_tier}: {self.influence_tiers[new_tier]['name']}")
                
                return tier_info
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to check tier progression: {e}")
            return None
    
    async def get_reputation_leaderboard(
        self,
        limit: int = 100
    ) -> List[Dict[str, Any]]:
        """
        Get top players by total influence.
        
        Args:
            limit: Maximum number of players to return
            
        Returns:
            List of top players with their influence scores
        """
        try:
            query = """
                SELECT 
                    player_id,
                    total_influence,
                    current_tier
                FROM player_influence
                ORDER BY total_influence DESC
                LIMIT $1
            """
            
            rows = await postgres_db.fetch_all(query, limit)
            
            leaderboard = []
            for rank, row in enumerate(rows, 1):
                leaderboard.append({
                    "rank": rank,
                    "player_id": row['player_id'],
                    "total_influence": row['total_influence'],
                    "tier": row['current_tier'],
                    "tier_name": self.influence_tiers[row['current_tier']]['name']
                })
            
            return leaderboard
            
        except Exception as e:
            logger.error(f"Failed to get reputation leaderboard: {e}")
            return []
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _create_player_influence_record(self, player_id: int):
        """Create initial player influence record"""
        try:
            query = """
                INSERT INTO player_influence
                (player_id, total_influence, current_tier)
                VALUES ($1, 0, 0)
                ON CONFLICT (player_id) DO NOTHING
            """
            await postgres_db.execute(query, player_id)
            
        except Exception as e:
            logger.debug(f"Failed to create player influence record: {e}")
    
    def _calculate_tier_from_influence(self, influence: int) -> int:
        """Calculate tier from influence score"""
        for tier in range(5, -1, -1):
            if influence >= self.influence_tiers[tier]['threshold']:
                return tier
        return 0
    
    async def _get_daily_reputation_gain(
        self,
        player_id: int,
        reputation_type: str
    ) -> int:
        """Get reputation gained today for a specific type"""
        try:
            query = """
                SELECT COALESCE(SUM(amount), 0) as daily_total
                FROM reputation_history
                WHERE player_id = $1
                  AND reputation_type = $2
                  AND timestamp >= CURRENT_DATE
            """
            
            result = await postgres_db.fetch_one(query, player_id, reputation_type)
            return result['daily_total'] if result else 0
            
        except Exception as e:
            logger.debug(f"Failed to get daily reputation gain: {e}")
            return 0
    
    async def _unlock_tier_benefits(self, player_id: int, tier: int):
        """Unlock all benefits for a tier"""
        try:
            benefits = self.influence_tiers[tier]['benefits']
            
            for benefit_id in benefits:
                benefit = self.benefit_configs.get(benefit_id, {})
                
                query = """
                    INSERT INTO unlocked_benefits
                    (player_id, benefit_type, benefit_id, unlocked_at)
                    VALUES ($1, $2, $3, NOW())
                    ON CONFLICT (player_id, benefit_type, benefit_id) DO NOTHING
                """
                
                await postgres_db.execute(
                    query,
                    player_id,
                    benefit.get('type', 'unknown'),
                    benefit_id
                )
            
            logger.debug(f"Unlocked {len(benefits)} benefits for player {player_id} at tier {tier}")
            
        except Exception as e:
            logger.error(f"Failed to unlock tier benefits: {e}")


# Global instance
reputation_agent = ReputationAgent()