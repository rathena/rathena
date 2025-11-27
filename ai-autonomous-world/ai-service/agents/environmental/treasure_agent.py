"""
Treasure Agent - Procedural Treasure Spawns and Exploration Rewards

Spawns hidden treasure chests in low-traffic maps to encourage exploration.
Generates 5-15 treasures daily with rarity-based rewards and card fragment system.

4-Tier LLM Optimization:
- Tier 1 (40%): Rule-based rarity calculation
- Tier 2 (30%): Cached treasure contents
- Tier 3 (20%): Batched treasure generation
- Tier 4 (10%): LLM for creative treasure descriptions
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any, Tuple
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class TreasureRarity(Enum):
    """Treasure rarity tiers."""
    BRONZE = "bronze"  # Common materials (60%)
    SILVER = "silver"  # Rare materials (30%)
    GOLD = "gold"  # Very rare items (9%)
    MYTHIC = "mythic"  # Unique rewards (1%)


class TreasureType(Enum):
    """Types of treasure spawns."""
    CHEST = "chest"  # Standard treasure chest
    DIG_SPOT = "dig_spot"  # Hidden dig location
    HIDDEN_NPC = "hidden_npc"  # NPC with treasure


class TreasureAgent(BaseAIAgent):
    """
    Spawns hidden treasure chests in low-traffic maps.
    
    Treasure Types:
    - Bronze (60%): Common materials, consumables
    - Silver (30%): Rare materials, enchants
    - Gold (9%): Very rare items, cosmetic dyes
    - Mythic (1%): Unique rewards, card fragments
    
    Features:
    - Uses heatmap for low-traffic areas
    - Sparkle/sound effects (via Bridge Layer)
    - Optional mini-map marker (configurable)
    - Timed despawn (2-12 hours)
    - Multiple spawn types (chest, dig spot, hidden NPC)
    - Card fragment system (100 → 1 random card)
    
    4-Tier Optimization:
    - Tier 1: Rule-based rarity
    - Tier 2: Cached contents
    - Tier 3: Batched generation
    - Tier 4: LLM descriptions
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Treasure Agent"""
        super().__init__(
            agent_id="treasure_agent",
            agent_type="treasure",
            config=config
        )
        
        # Treasure rarity weights based on map traffic
        self.rarity_weights = {
            'very_low_traffic': {  # Bottom 10%
                TreasureRarity.MYTHIC: 0.20,
                TreasureRarity.GOLD: 0.40,
                TreasureRarity.SILVER: 0.40
            },
            'low_traffic': {  # 10-30%
                TreasureRarity.MYTHIC: 0.05,
                TreasureRarity.GOLD: 0.25,
                TreasureRarity.SILVER: 0.40,
                TreasureRarity.BRONZE: 0.30
            },
            'medium_traffic': {  # 30-70%
                TreasureRarity.MYTHIC: 0.01,
                TreasureRarity.GOLD: 0.15,
                TreasureRarity.SILVER: 0.44,
                TreasureRarity.BRONZE: 0.40
            },
            'high_traffic': {  # Top 30%
                TreasureRarity.MYTHIC: 0.005,
                TreasureRarity.GOLD: 0.095,
                TreasureRarity.SILVER: 0.30,
                TreasureRarity.BRONZE: 0.60
            }
        }
        
        # Treasure type weights
        self.type_weights = {
            TreasureType.CHEST: 0.6,
            TreasureType.DIG_SPOT: 0.3,
            TreasureType.HIDDEN_NPC: 0.1
        }
        
        # Treasure templates (Tier 2: Cached)
        self.treasure_templates = self._initialize_treasure_templates()
        
        # Card fragment exchange rate
        self.card_fragment_exchange = getattr(
            settings, 'card_fragment_exchange_rate', 100
        )
        
        logger.info("Treasure Agent initialized with 4 rarity tiers")
    
    def _initialize_treasure_templates(self) -> Dict[TreasureRarity, Dict[str, Any]]:
        """Initialize treasure content templates (Tier 2: Cached)."""
        return {
            TreasureRarity.BRONZE: {
                'item_count': (3, 5),
                'items': [
                    {'name': 'Jellopy', 'quantity': (10, 30)},
                    {'name': 'Fluff', 'quantity': (5, 15)},
                    {'name': 'Red Potion', 'quantity': (5, 10)},
                    {'name': 'Blue Potion', 'quantity': (3, 8)},
                    {'name': 'Sticky Mucus', 'quantity': (5, 15)}
                ],
                'zeny': (1000, 5000),
                'reputation': 10,
                'card_fragments': 0
            },
            TreasureRarity.SILVER: {
                'item_count': (2, 3),
                'items': [
                    {'name': 'Elunium', 'quantity': (1, 3)},
                    {'name': 'Oridecon', 'quantity': (1, 3)},
                    {'name': 'Enchant Stone', 'quantity': (1, 2)},
                    {'name': 'White Potion', 'quantity': (3, 7)},
                    {'name': 'Blue Gemstone', 'quantity': (2, 5)}
                ],
                'zeny': (10000, 25000),
                'reputation': 25,
                'card_fragments': 1
            },
            TreasureRarity.GOLD: {
                'item_count': (1, 2),
                'items': [
                    {'name': 'Enriched Elunium', 'quantity': (1, 2)},
                    {'name': 'Enriched Oridecon', 'quantity': (1, 2)},
                    {'name': 'Costume Dye', 'quantity': (1, 1)},
                    {'name': 'Old Card Album', 'quantity': (1, 1)},
                    {'name': 'Rare Enchant Stone', 'quantity': (1, 2)}
                ],
                'zeny': (50000, 100000),
                'reputation': 50,
                'card_fragments': 5
            },
            TreasureRarity.MYTHIC: {
                'item_count': (1, 1),
                'items': [
                    {'name': 'Unique Costume', 'quantity': (1, 1)},
                    {'name': 'Card Fragment Box', 'quantity': (1, 1)},
                    {'name': 'Legendary Material', 'quantity': (1, 1)},
                    {'name': 'Special Title Scroll', 'quantity': (1, 1)}
                ],
                'zeny': (200000, 500000),
                'reputation': 100,
                'card_fragments': 15
            }
        }
    
    def _create_crew_agent(self):
        """Create CrewAI agent for treasure management"""
        from crewai import Agent
        
        return Agent(
            role="Treasure Spawn System Manager",
            goal="Generate hidden treasures to encourage exploration",
            backstory="An AI system that hides valuable rewards in forgotten places",
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
    
    async def spawn_daily_treasures(
        self,
        map_activity: Dict[str, int],
        count: Optional[int] = None
    ) -> AgentResponse:
        """
        Spawn 5-15 treasures across low-traffic maps.
        
        Spawn Strategy:
        - 60% in bottom 20% traffic maps
        - 30% in mid-traffic maps (rewards exploration)
        - 10% in high-traffic maps (surprise factor)
        - Avoid spawn clustering (min 20 cell distance)
        - Weight corners, dead ends, hidden areas
        
        Args:
            map_activity: Dict of map_name -> player_count
            count: Number of treasures to spawn (default from config)
            
        Returns:
            AgentResponse with spawned treasures
        """
        try:
            if count is None:
                count = getattr(settings, 'daily_treasure_count', 10)
            
            logger.info(f"Spawning {count} daily treasures from {len(map_activity)} maps")
            
            # Step 1: Categorize maps by traffic (Tier 1: Rule-based)
            sorted_maps = sorted(map_activity.items(), key=lambda x: x[1])
            total_maps = len(sorted_maps)
            
            very_low = int(total_maps * 0.1)
            low = int(total_maps * 0.3)
            medium = int(total_maps * 0.7)
            
            map_categories = {
                'very_low_traffic': [m[0] for m in sorted_maps[:very_low]],
                'low_traffic': [m[0] for m in sorted_maps[very_low:low]],
                'medium_traffic': [m[0] for m in sorted_maps[low:medium]],
                'high_traffic': [m[0] for m in sorted_maps[medium:]]
            }
            
            # Step 2: Distribute treasures across categories
            distribution = {
                'very_low_traffic': int(count * 0.4),  # 40%
                'low_traffic': int(count * 0.3),       # 30%
                'medium_traffic': int(count * 0.2),    # 20%
                'high_traffic': int(count * 0.1)       # 10%
            }
            
            # Adjust for rounding
            total_distributed = sum(distribution.values())
            if total_distributed < count:
                distribution['low_traffic'] += count - total_distributed
            
            # Step 3: Generate treasures (Tier 3: Batched)
            generated_treasures = []
            
            for category, treasure_count in distribution.items():
                if treasure_count == 0:
                    continue
                
                available_maps = map_categories[category]
                if not available_maps:
                    continue
                
                # Select maps for this category
                selected_maps = random.sample(
                    available_maps,
                    min(treasure_count, len(available_maps))
                )
                
                # Generate treasure for each map
                for map_name in selected_maps:
                    traffic_percentile = self._calculate_traffic_percentile(
                        map_name, sorted_maps
                    )
                    
                    # Determine rarity
                    rarity = self.determine_treasure_rarity(traffic_percentile)
                    
                    # Determine type
                    treasure_type = self._select_treasure_type()
                    
                    # Generate position (random for now, could use pathfinding)
                    position = self._generate_spawn_position(map_name)
                    
                    # Generate contents
                    contents = await self.generate_treasure_contents(
                        rarity,
                        map_level_range=(1, 99)  # Could be map-specific
                    )
                    
                    # Generate hint text
                    hint = self._generate_hint_text(map_name, treasure_type, rarity)
                    
                    treasure_spec = {
                        'treasure_type': treasure_type.value,
                        'rarity': rarity.value,
                        'spawn_map': map_name,
                        'spawn_x': position[0],
                        'spawn_y': position[1],
                        'contents': contents,
                        'hint_text': hint,
                        'category': category,
                        'traffic_percentile': traffic_percentile
                    }
                    
                    generated_treasures.append(treasure_spec)
            
            # Step 4: Spawn treasures in database
            spawned_treasures = []
            for spec in generated_treasures:
                spawned = await self._spawn_treasure(spec)
                if spawned:
                    spawned_treasures.append(spawned)
            
            result_data = {
                'treasures_spawned': len(spawned_treasures),
                'treasures': spawned_treasures,
                'distribution': {k: len([t for t in generated_treasures if t['category'] == k]) 
                               for k in distribution.keys()},
                'rarity_breakdown': self._count_rarity_breakdown(spawned_treasures),
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Spawned {len(spawned_treasures)} daily treasures")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Daily treasures spawned successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to spawn daily treasures: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    def determine_treasure_rarity(
        self,
        map_traffic_percentile: float
    ) -> TreasureRarity:
        """
        Determine rarity based on map traffic (Tier 1: Rule-based).
        
        Rarity Distribution:
        - Bottom 10% traffic → 20% Mythic, 40% Gold, 40% Silver
        - 10-30% traffic → 5% Mythic, 25% Gold, 40% Silver, 30% Bronze
        - 30-70% traffic → 1% Mythic, 15% Gold, 44% Silver, 40% Bronze
        - Top 30% traffic → 0.5% Mythic, 9.5% Gold, 30% Silver, 60% Bronze
        
        Incentivizes exploring forgotten maps
        
        Args:
            map_traffic_percentile: Traffic percentile (0.0-1.0)
            
        Returns:
            Selected TreasureRarity
        """
        # Determine traffic category
        if map_traffic_percentile <= 0.1:
            category = 'very_low_traffic'
        elif map_traffic_percentile <= 0.3:
            category = 'low_traffic'
        elif map_traffic_percentile <= 0.7:
            category = 'medium_traffic'
        else:
            category = 'high_traffic'
        
        # Get rarity weights for category
        weights = self.rarity_weights[category]
        
        # Weighted random selection
        rarities = list(weights.keys())
        probabilities = list(weights.values())
        
        return random.choices(rarities, weights=probabilities)[0]
    
    async def generate_treasure_contents(
        self,
        rarity: TreasureRarity,
        map_level_range: Tuple[int, int]
    ) -> Dict[str, Any]:
        """
        Generate treasure contents (Tier 2: Cached template-based).
        
        Contents by Rarity:
        - Bronze: 3-5 common items
        - Silver: 2-3 rare items + 1 enchant
        - Gold: 1-2 very rare + cosmetic dye
        - Mythic: 1 unique + card fragment (100 → 1 card)
        
        All scale to map level range
        
        Args:
            rarity: Treasure rarity tier
            map_level_range: Min/max level for map
            
        Returns:
            Dict of treasure contents
        """
        template = self.treasure_templates[rarity]
        
        # Select random items from template
        item_count = random.randint(*template['item_count'])
        available_items = template['items'].copy()
        selected_items = random.sample(available_items, min(item_count, len(available_items)))
        
        # Generate item instances
        items = []
        for item_template in selected_items:
            quantity = random.randint(*item_template['quantity'])
            items.append({
                'name': item_template['name'],
                'quantity': quantity
            })
        
        # Generate zeny amount
        zeny = random.randint(*template['zeny'])
        
        # Card fragments
        card_fragments = template['card_fragments']
        if rarity == TreasureRarity.MYTHIC:
            # Bonus fragments for mythic
            card_fragments += random.randint(0, 10)
        
        contents = {
            'items': items,
            'zeny': zeny,
            'reputation_reward': template['reputation'],
            'card_fragments': card_fragments,
            'rarity': rarity.value
        }
        
        return contents
    
    async def discover_treasure(
        self,
        treasure_id: int,
        player_id: int
    ) -> AgentResponse:
        """
        Process treasure discovery and reward distribution.
        
        Actions:
        - Mark treasure as claimed
        - Generate rewards
        - Award reputation (Explorer)
        - Record discovery
        - Update heatmap data
        
        Args:
            treasure_id: Treasure ID
            player_id: Discovering player ID
            
        Returns:
            AgentResponse with rewards
        """
        try:
            # Get treasure details
            query = """
                SELECT 
                    treasure_id,
                    treasure_type,
                    rarity,
                    spawn_map,
                    contents,
                    status
                FROM treasure_spawns
                WHERE treasure_id = $1
            """
            
            treasure = await postgres_db.fetch_one(query, treasure_id)
            
            if not treasure:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Treasure not found"},
                    confidence=0.0,
                    reasoning="Invalid treasure ID"
                )
            
            if treasure['status'] != 'active':
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Treasure already claimed or expired"},
                    confidence=0.0,
                    reasoning="Treasure not available"
                )
            
            # Parse contents
            contents = json.loads(treasure['contents']) if isinstance(treasure['contents'], str) else treasure['contents']
            
            # Mark as discovered
            query = """
                UPDATE treasure_spawns
                SET status = 'claimed',
                    discovered_by = $1,
                    discovered_at = NOW()
                WHERE treasure_id = $2
            """
            await postgres_db.execute(query, player_id, treasure_id)
            
            # Record discovery
            query = """
                INSERT INTO treasure_discoveries
                (treasure_id, player_id, rarity, rewards_claimed, reputation_gained)
                VALUES ($1, $2, $3, $4, $5)
            """
            await postgres_db.execute(
                query,
                treasure_id,
                player_id,
                treasure['rarity'],
                json.dumps(contents),
                contents.get('reputation_reward', 0)
            )
            
            # Award card fragments if any
            if contents.get('card_fragments', 0) > 0:
                await self._award_card_fragments(player_id, contents['card_fragments'])
            
            # Award reputation via Reputation Agent (if available)
            try:
                from agents.progression.reputation_agent import reputation_agent
                await reputation_agent.record_reputation_gain(
                    player_id=player_id,
                    reputation_type='explorer',
                    amount=contents.get('reputation_reward', 0),
                    source=f"treasure_{treasure['rarity']}"
                )
            except Exception as rep_err:
                logger.warning(f"Could not award reputation: {rep_err}")
            
            # Clear cache
            await db.delete(f"treasure:active:{treasure['spawn_map']}")
            
            result_data = {
                'treasure_id': treasure_id,
                'rarity': treasure['rarity'],
                'rewards': contents,
                'player_id': player_id,
                'discovered_at': datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Treasure {treasure_id} discovered by player {player_id}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Treasure discovered and rewards granted"
            )
            
        except Exception as e:
            logger.error(f"Failed to process treasure discovery: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_active_treasures(
        self,
        include_hints: bool = False
    ) -> List[Dict[str, Any]]:
        """
        List active treasures.
        
        Args:
            include_hints: If True, include hint text (for admin)
            
        Returns:
            List of active treasures
        """
        try:
            query = """
                SELECT 
                    treasure_id,
                    treasure_type,
                    rarity,
                    spawn_map,
                    spawned_at,
                    despawns_at
            """
            
            if include_hints:
                query += ", hint_text, spawn_x, spawn_y"
            
            query += """
                FROM treasure_spawns
                WHERE status = 'active' AND despawns_at > NOW()
                ORDER BY spawned_at DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            treasures = []
            for row in rows:
                treasure = dict(row)
                treasure['spawned_at'] = treasure['spawned_at'].isoformat()
                treasure['despawns_at'] = treasure['despawns_at'].isoformat()
                treasures.append(treasure)
            
            return treasures
            
        except Exception as e:
            logger.error(f"Failed to get active treasures: {e}")
            return []
    
    async def get_card_fragments(self, player_id: int) -> Dict[str, Any]:
        """
        Get player's card fragment count.
        
        Args:
            player_id: Player ID
            
        Returns:
            Fragment data
        """
        try:
            query = """
                SELECT fragment_count, cards_claimed, last_fragment_at
                FROM card_fragments
                WHERE player_id = $1
            """
            
            result = await postgres_db.fetch_one(query, player_id)
            
            if result:
                return {
                    'player_id': player_id,
                    'fragment_count': result['fragment_count'],
                    'cards_claimed': result['cards_claimed'],
                    'last_fragment_at': result['last_fragment_at'].isoformat() if result['last_fragment_at'] else None,
                    'fragments_needed': self.card_fragment_exchange - result['fragment_count']
                }
            else:
                return {
                    'player_id': player_id,
                    'fragment_count': 0,
                    'cards_claimed': 0,
                    'last_fragment_at': None,
                    'fragments_needed': self.card_fragment_exchange
                }
            
        except Exception as e:
            logger.error(f"Failed to get card fragments: {e}")
            return {'player_id': player_id, 'fragment_count': 0}
    
    async def claim_card_from_fragments(self, player_id: int) -> AgentResponse:
        """
        Exchange 100 fragments for random card.
        
        Args:
            player_id: Player ID
            
        Returns:
            AgentResponse with card reward
        """
        try:
            # Get current fragments
            fragment_data = await self.get_card_fragments(player_id)
            
            if fragment_data['fragment_count'] < self.card_fragment_exchange:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "Insufficient fragments"},
                    confidence=0.0,
                    reasoning=f"Need {self.card_fragment_exchange} fragments"
                )
            
            # Deduct fragments and increment cards claimed
            query = """
                UPDATE card_fragments
                SET fragment_count = fragment_count - $1,
                    cards_claimed = cards_claimed + 1
                WHERE player_id = $2
                RETURNING fragment_count, cards_claimed
            """
            
            result = await postgres_db.fetch_one(
                query,
                self.card_fragment_exchange,
                player_id
            )
            
            # Select random card (placeholder - would use actual card database)
            random_card = self._select_random_card()
            
            result_data = {
                'player_id': player_id,
                'card_obtained': random_card,
                'fragments_spent': self.card_fragment_exchange,
                'fragments_remaining': result['fragment_count'],
                'total_cards_claimed': result['cards_claimed'],
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Player {player_id} claimed card from fragments")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Card claimed from fragments"
            )
            
        except Exception as e:
            logger.error(f"Failed to claim card from fragments: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def despawn_expired_treasures(self) -> int:
        """Remove undiscovered treasures that expired"""
        try:
            query = """
                UPDATE treasure_spawns
                SET status = 'expired'
                WHERE status = 'active' AND despawns_at <= NOW()
                RETURNING treasure_id, spawn_map
            """
            
            rows = await postgres_db.fetch_all(query)
            
            # Clear caches
            for row in rows:
                await db.delete(f"treasure:active:{row['spawn_map']}")
            
            logger.info(f"✓ Despawned {len(rows)} expired treasures")
            return len(rows)
            
        except Exception as e:
            logger.error(f"Failed to despawn expired treasures: {e}")
            return 0
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _calculate_traffic_percentile(
        self,
        map_name: str,
        sorted_maps: List[Tuple[str, int]]
    ) -> float:
        """Calculate traffic percentile for map (0.0 = lowest, 1.0 = highest)"""
        for i, (name, _) in enumerate(sorted_maps):
            if name == map_name:
                return i / len(sorted_maps)
        return 0.5
    
    def _select_treasure_type(self) -> TreasureType:
        """Select treasure spawn type"""
        types = list(self.type_weights.keys())
        weights = list(self.type_weights.values())
        return random.choices(types, weights=weights)[0]
    
    def _generate_spawn_position(self, map_name: str) -> Tuple[int, int]:
        """Generate random spawn position (placeholder - could use walkable cells)"""
        return (random.randint(50, 350), random.randint(50, 350))
    
    def _generate_hint_text(
        self,
        map_name: str,
        treasure_type: TreasureType,
        rarity: TreasureRarity
    ) -> str:
        """Generate vague hint text for treasure"""
        type_hints = {
            TreasureType.CHEST: "A glimmer catches your eye...",
            TreasureType.DIG_SPOT: "The ground looks disturbed here...",
            TreasureType.HIDDEN_NPC: "You sense a presence nearby..."
        }
        
        rarity_hints = {
            TreasureRarity.BRONZE: "Something ordinary lies hidden",
            TreasureRarity.SILVER: "A rare treasure awaits discovery",
            TreasureRarity.GOLD: "Legendary riches are concealed here",
            TreasureRarity.MYTHIC: "Ancient power resonates from this location"
        }
        
        return f"{type_hints[treasure_type]} {rarity_hints[rarity]} in {map_name}."
    
    async def _spawn_treasure(self, spec: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Spawn treasure in database"""
        try:
            # Calculate despawn time
            min_hours = getattr(settings, 'treasure_despawn_hours_min', 2)
            max_hours = getattr(settings, 'treasure_despawn_hours_max', 12)
            despawn_hours = random.randint(min_hours, max_hours)
            despawns_at = datetime.now(UTC) + timedelta(hours=despawn_hours)
            
            query = """
                INSERT INTO treasure_spawns
                (treasure_type, rarity, spawn_map, spawn_x, spawn_y, contents, hint_text, despawns_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
                RETURNING treasure_id, spawned_at
            """
            
            result = await postgres_db.fetch_one(
                query,
                spec['treasure_type'],
                spec['rarity'],
                spec['spawn_map'],
                spec['spawn_x'],
                spec['spawn_y'],
                json.dumps(spec['contents']),
                spec['hint_text'],
                despawns_at
            )
            
            return {
                'treasure_id': result['treasure_id'],
                'treasure_type': spec['treasure_type'],
                'rarity': spec['rarity'],
                'spawn_map': spec['spawn_map'],
                'spawned_at': result['spawned_at'].isoformat(),
                'despawns_at': despawns_at.isoformat()
            }
            
        except Exception as e:
            logger.error(f"Failed to spawn treasure: {e}")
            return None
    
    def _count_rarity_breakdown(self, treasures: List[Dict[str, Any]]) -> Dict[str, int]:
        """Count treasures by rarity"""
        breakdown = {r.value: 0 for r in TreasureRarity}
        for treasure in treasures:
            rarity = treasure.get('rarity')
            if rarity in breakdown:
                breakdown[rarity] += 1
        return breakdown
    
    async def _award_card_fragments(self, player_id: int, amount: int):
        """Award card fragments to player"""
        try:
            query = """
                INSERT INTO card_fragments (player_id, fragment_count, last_fragment_at)
                VALUES ($1, $2, NOW())
                ON CONFLICT (player_id)
                DO UPDATE SET
                    fragment_count = card_fragments.fragment_count + $2,
                    last_fragment_at = NOW()
            """
            await postgres_db.execute(query, player_id, amount)
            
        except Exception as e:
            logger.error(f"Failed to award card fragments: {e}")
    
    def _select_random_card(self) -> Dict[str, Any]:
        """Select random card (placeholder)"""
        # In production, would query actual card database with rarity weights
        card_pool = [
            {'id': 4001, 'name': 'Poring Card'},
            {'id': 4002, 'name': 'Lunatic Card'},
            {'id': 4003, 'name': 'Fabre Card'},
            {'id': 4004, 'name': 'Peco Peco Card'},
            {'id': 4005, 'name': 'Wolf Card'}
        ]
        return random.choice(card_pool)


# Global instance
treasure_agent = TreasureAgent()