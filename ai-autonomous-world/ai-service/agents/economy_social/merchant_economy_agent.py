"""
Merchant Economy Agent - Manages Dynamic Pricing & Economic Balance
Implements 4-tier LLM optimization for cost-effective economic management
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from decimal import Decimal
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class EconomicIndicator(Enum):
    """Economic health indicators"""
    INFLATION = "inflation"  # Too much zeny circulation
    DEFLATION = "deflation"  # Too little zeny
    ITEM_GLUT = "item_glut"  # Oversupply of items
    ITEM_SCARCITY = "item_scarcity"  # Undersupply
    BALANCED = "balanced"  # Healthy economy


class MerchantEconomyAgent(BaseAIAgent):
    """
    Manages dynamic pricing and economic balance through adaptive merchant prices.
    
    Economic Monitoring:
    - Zeny circulation (total in game)
    - Item drop frequency and supply
    - Player farming patterns
    - Trade velocity
    - Refine material usage
    
    Actions:
    - Adjust NPC shop prices (+/- 50%)
    - Create zeny sink events
    - Recommend drop rate changes
    - Apply temporary sales/taxes
    
    4-Tier Optimization:
    - Tier 1: Rule-based economic analysis (0 LLM calls)
    - Tier 2: Cached price adjustments (0 LLM calls)
    - Tier 3: Batched event generation (1 call for multiple)
    - Tier 4: Creative event descriptions (optional)
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Merchant Economy Agent"""
        super().__init__(
            agent_id="merchant_economy_agent",
            agent_type="merchant_economy",
            config=config
        )
        
        # Economic thresholds
        self.thresholds = {
            'zeny_per_capita_inflation': 10000000,  # 10M+ = inflation
            'zeny_per_capita_deflation': 100000,    # <100K = deflation
            'item_oversupply_ratio': 2.0,            # 200% = glut
            'item_scarcity_ratio': 0.8,              # 80% = scarcity
            'price_adjustment_max': 0.5,             # +/- 50%
            'price_adjustment_min': -0.5
        }
        
        # Price history tracking
        self.price_history: Dict[str, List[float]] = {}
        
        # Zeny sink event templates
        self.zeny_sink_templates = {
            'cosmetic_sale': {
                'name': 'Limited Edition Cosmetics',
                'description': 'Exclusive cosmetic items for a limited time',
                'duration_hours': (2, 6),
                'target_drain_percent': 0.15
            },
            'refinement_bonus': {
                'name': 'Guaranteed Refinement Success',
                'description': 'Pay for guaranteed equipment refinement',
                'duration_hours': (1, 4),
                'target_drain_percent': 0.20
            },
            'luxury_buffs': {
                'name': 'Luxury Tax Day',
                'description': 'Expensive but powerful temporary buffs',
                'duration_hours': (3, 6),
                'target_drain_percent': 0.10
            },
            'card_packs': {
                'name': 'Exclusive Card Packs',
                'description': 'Random card packs for zeny',
                'duration_hours': (2, 5),
                'target_drain_percent': 0.25
            }
        }
        
        logger.info("Merchant Economy Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for economic management"""
        from crewai import Agent
        
        return Agent(
            role="Economic Balance Manager",
            goal="Maintain healthy economy through dynamic pricing and zeny sinks",
            backstory="An AI system that monitors and balances the game economy",
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
    
    async def analyze_economic_health(
        self,
        economy_data: Dict[str, Any]
    ) -> EconomicIndicator:
        """
        Analyze current economic state using Tier 1 rule-based logic.
        
        Metrics analyzed:
        - Zeny per capita (total zeny / active players)
        - Item supply index (drop rate * farming intensity)
        - Inflation rate (price changes over time)
        - Trade volume velocity
        
        Args:
            economy_data: Current economic metrics
            
        Returns:
            Economic indicator status
        """
        try:
            # Extract metrics
            total_zeny = economy_data.get('total_zeny', 0)
            active_players = max(1, economy_data.get('active_players', 1))
            item_supply = economy_data.get('item_supply_index', 1.0)
            item_demand = economy_data.get('item_demand_index', 1.0)
            
            # Calculate zeny per capita
            zeny_per_capita = total_zeny / active_players
            
            # Check inflation (Tier 1: Rule-based)
            if zeny_per_capita > self.thresholds['zeny_per_capita_inflation']:
                logger.info(f"INFLATION detected: {zeny_per_capita:,.0f} zeny/capita")
                return EconomicIndicator.INFLATION
            
            # Check deflation
            if zeny_per_capita < self.thresholds['zeny_per_capita_deflation']:
                logger.info(f"DEFLATION detected: {zeny_per_capita:,.0f} zeny/capita")
                return EconomicIndicator.DEFLATION
            
            # Check item supply imbalance
            supply_demand_ratio = item_supply / max(item_demand, 0.1)
            
            if supply_demand_ratio > self.thresholds['item_oversupply_ratio']:
                logger.info(f"ITEM_GLUT detected: {supply_demand_ratio:.2f}x oversupply")
                return EconomicIndicator.ITEM_GLUT
            
            if supply_demand_ratio < self.thresholds['item_scarcity_ratio']:
                logger.info(f"ITEM_SCARCITY detected: {supply_demand_ratio:.2f}x undersupply")
                return EconomicIndicator.ITEM_SCARCITY
            
            # Healthy economy
            logger.info(f"Economy BALANCED: {zeny_per_capita:,.0f} zeny/capita, ratio {supply_demand_ratio:.2f}")
            return EconomicIndicator.BALANCED
            
        except Exception as e:
            logger.error(f"Error analyzing economic health: {e}")
            return EconomicIndicator.BALANCED
    
    async def adjust_merchant_prices(
        self,
        indicator: EconomicIndicator,
        affected_items: Optional[List[str]] = None
    ) -> AgentResponse:
        """
        Adjust NPC merchant prices dynamically based on economic state.
        
        Price Adjustments:
        - INFLATION → increase prices 10-50%
        - DEFLATION → decrease prices 10-30%
        - ITEM_GLUT → increase buy price, decrease sell price
        - ITEM_SCARCITY → decrease buy price, increase sell price
        
        Args:
            indicator: Current economic indicator
            affected_items: Optional list of specific item IDs to adjust
            
        Returns:
            AgentResponse with adjustment details
        """
        try:
            logger.info(f"Adjusting merchant prices for {indicator.value}")
            
            # Determine adjustment percentage (Tier 1: Rule-based)
            adjustment_percent = self._calculate_price_adjustment(indicator)
            
            # Get items to adjust
            if not affected_items:
                affected_items = await self._get_high_trade_items()
            
            # Check cache for recent adjustments (Tier 2)
            cache_key = f"price_adjustment:{indicator.value}"
            cached = await db.get(cache_key)
            if cached and cached.get('timestamp'):
                last_adjustment = datetime.fromisoformat(cached['timestamp'])
                if datetime.now(UTC) - last_adjustment < timedelta(hours=6):
                    logger.info("Recent price adjustment found in cache, skipping")
                    return AgentResponse(
                        agent_type=self.agent_type,
                        success=True,
                        data=cached,
                        confidence=0.9,
                        reasoning="Using cached price adjustment"
                    )
            
            # Apply price adjustments to database
            adjustments = []
            for item_id in affected_items[:50]:  # Limit to 50 items
                old_price = await self._get_item_price(item_id)
                new_price = int(old_price * (1 + adjustment_percent))
                new_price = max(1, new_price)  # Min price is 1 zeny
                
                # Store adjustment
                await self._store_price_adjustment(
                    item_id=item_id,
                    old_price=old_price,
                    new_price=new_price,
                    adjustment_percent=adjustment_percent,
                    reason=indicator.value
                )
                
                adjustments.append({
                    'item_id': item_id,
                    'old_price': old_price,
                    'new_price': new_price,
                    'change_percent': adjustment_percent * 100
                })
            
            # Create snapshot
            await self._create_economic_snapshot(indicator, adjustments)
            
            result_data = {
                'indicator': indicator.value,
                'adjustment_percent': adjustment_percent * 100,
                'items_adjusted': len(adjustments),
                'adjustments': adjustments[:10],  # Return first 10 for display
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Cache result
            await db.set(cache_key, result_data, expire=21600)  # 6 hours
            
            logger.info(f"✓ Adjusted prices for {len(adjustments)} items by {adjustment_percent*100:+.1f}%")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning=f"Price adjustment applied for {indicator.value}"
            )
            
        except Exception as e:
            logger.error(f"Failed to adjust prices: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def create_zeny_sink_event(
        self,
        severity: float  # 0.0-1.0
    ) -> AgentResponse:
        """
        Create a zeny sink event to drain excess currency.
        
        Event selection based on severity:
        - High severity (>0.7): Luxury items + high prices
        - Medium severity (0.3-0.7): Balanced offers
        - Low severity (<0.3): Small incentives
        
        Args:
            severity: Economic crisis severity (0.0-1.0)
            
        Returns:
            AgentResponse with event details
        """
        try:
            logger.info(f"Creating zeny sink event (severity: {severity:.2f})")
            
            # Select event type based on severity (Tier 1: Rule-based)
            if severity > 0.7:
                event_type = 'card_packs'
            elif severity > 0.4:
                event_type = 'refinement_bonus'
            elif severity > 0.2:
                event_type = 'cosmetic_sale'
            else:
                event_type = 'luxury_buffs'
            
            template = self.zeny_sink_templates[event_type]
            
            # Calculate event parameters
            duration_hours = random.randint(*template['duration_hours'])
            target_drain = int(severity * 1000000000 * template['target_drain_percent'])
            
            # Generate event offerings
            offerings = await self._generate_event_offerings(event_type, severity)
            
            # Store event in database
            event_id = await self._store_zeny_sink_event(
                event_type=event_type,
                event_name=template['name'],
                target_zeny_drain=target_drain,
                offerings=offerings,
                duration_hours=duration_hours
            )
            
            result_data = {
                'event_id': event_id,
                'event_type': event_type,
                'event_name': template['name'],
                'description': template['description'],
                'target_zeny_drain': target_drain,
                'duration_hours': duration_hours,
                'offerings': offerings,
                'started_at': datetime.now(UTC).isoformat(),
                'ends_at': (datetime.now(UTC) + timedelta(hours=duration_hours)).isoformat()
            }
            
            logger.info(f"✓ Created zeny sink event '{template['name']}' targeting {target_drain:,} zeny drain")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.9,
                reasoning=f"Zeny sink event created for severity {severity:.2f}"
            )
            
        except Exception as e:
            logger.error(f"Failed to create zeny sink event: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def recommend_drop_rate_changes(
        self,
        oversupplied_items: List[str],
        undersupplied_items: List[str]
    ) -> Dict[str, Any]:
        """
        Recommend drop rate adjustments to other agents.
        
        Coordinates with:
        - Problem Agent (adjust problem rewards)
        - World Event Agent (modify event drops)
        - Treasure Agent (change treasure contents)
        
        Args:
            oversupplied_items: Items with too much supply
            undersupplied_items: Items with too little supply
            
        Returns:
            Recommendations for each system
        """
        try:
            recommendations = {
                'problem_agent': [],
                'event_agent': [],
                'treasure_agent': [],
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Reduce drop rates for oversupplied items
            for item_id in oversupplied_items[:10]:
                recommendations['problem_agent'].append({
                    'item_id': item_id,
                    'action': 'reduce_drop_rate',
                    'multiplier': 0.5
                })
                recommendations['event_agent'].append({
                    'item_id': item_id,
                    'action': 'reduce_drop_rate',
                    'multiplier': 0.6
                })
            
            # Increase drop rates for undersupplied items
            for item_id in undersupplied_items[:10]:
                recommendations['treasure_agent'].append({
                    'item_id': item_id,
                    'action': 'increase_drop_rate',
                    'multiplier': 1.5
                })
                recommendations['problem_agent'].append({
                    'item_id': item_id,
                    'action': 'increase_drop_rate',
                    'multiplier': 1.3
                })
            
            # Store recommendations
            await postgres_db.execute(
                """
                INSERT INTO drop_rate_recommendations
                (recommendations_data, created_at)
                VALUES ($1, NOW())
                """,
                json.dumps(recommendations)
            )
            
            logger.info(f"✓ Generated drop rate recommendations for {len(oversupplied_items + undersupplied_items)} items")
            
            return recommendations
            
        except Exception as e:
            logger.error(f"Failed to generate recommendations: {e}")
            return {}
    
    async def get_current_economy_snapshot(self) -> Optional[Dict[str, Any]]:
        """Get the most recent economic snapshot"""
        try:
            # Try cache first
            cached = await db.get("economy:snapshot:current")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT *
                FROM economy_snapshots
                ORDER BY timestamp DESC
                LIMIT 1
            """
            
            snapshot = await postgres_db.fetch_one(query)
            
            if snapshot:
                snapshot_dict = dict(snapshot)
                
                # Parse JSON fields
                for field in ['top_farmed_items', 'scarce_items']:
                    if snapshot_dict.get(field):
                        snapshot_dict[field] = json.loads(snapshot_dict[field])
                
                # Cache for 5 minutes
                await db.set("economy:snapshot:current", snapshot_dict, expire=300)
                
                return snapshot_dict
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to get economy snapshot: {e}")
            return None
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _calculate_price_adjustment(self, indicator: EconomicIndicator) -> float:
        """Calculate price adjustment percentage based on indicator"""
        adjustments = {
            EconomicIndicator.INFLATION: random.uniform(0.10, 0.50),    # +10% to +50%
            EconomicIndicator.DEFLATION: random.uniform(-0.30, -0.10),  # -30% to -10%
            EconomicIndicator.ITEM_GLUT: random.uniform(0.05, 0.20),    # +5% to +20%
            EconomicIndicator.ITEM_SCARCITY: random.uniform(-0.20, -0.05), # -20% to -5%
            EconomicIndicator.BALANCED: 0.0
        }
        
        return adjustments.get(indicator, 0.0)
    
    async def _get_high_trade_items(self, limit: int = 50) -> List[str]:
        """Get most traded items"""
        try:
            # In production, this would query actual trade data
            # For now, return common consumables and materials
            return [
                '607', '608', '610',  # Yggdrasil Berry/Seed/Leaf
                '714', '715', '716',  # Elemental stones
                '985', '984', '983',  # Refine materials
                '617', '616', '603',  # Boxes
                '501', '502', '503',  # Potions
            ]
        except Exception as e:
            logger.error(f"Failed to get high trade items: {e}")
            return []
    
    async def _get_item_price(self, item_id: str) -> int:
        """Get current item price"""
        try:
            # Try cache first
            cache_key = f"item:price:{item_id}"
            cached_price = await db.get(cache_key)
            if cached_price:
                return int(cached_price)
            
            # Query database (would query item_db in production)
            # For now, use default prices
            default_prices = {
                '607': 5000, '608': 20000, '610': 1000,
                '714': 3000, '715': 3000, '716': 3000,
                '985': 1000, '984': 500, '983': 200,
                '617': 50000, '616': 100000, '603': 10000,
                '501': 50, '502': 200, '503': 500
            }
            
            price = default_prices.get(item_id, 1000)
            
            # Cache for 1 hour
            await db.set(cache_key, price, expire=3600)
            
            return price
            
        except Exception as e:
            logger.error(f"Failed to get item price: {e}")
            return 1000
    
    async def _store_price_adjustment(
        self,
        item_id: str,
        old_price: int,
        new_price: int,
        adjustment_percent: float,
        reason: str
    ):
        """Store price adjustment in database"""
        try:
            query = """
                INSERT INTO price_adjustments
                (item_id, old_price, new_price, adjustment_percent, reason)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                int(item_id) if item_id.isdigit() else 0,
                old_price,
                new_price,
                adjustment_percent,
                reason
            )
            
        except Exception as e:
            logger.error(f"Failed to store price adjustment: {e}")
    
    async def _create_economic_snapshot(
        self,
        indicator: EconomicIndicator,
        adjustments: List[Dict]
    ):
        """Create economic snapshot"""
        try:
            # Calculate metrics (would be from actual game data in production)
            query = """
                INSERT INTO economy_snapshots
                (zeny_circulation, zeny_per_capita, active_players, inflation_rate, 
                 economic_indicator, top_farmed_items, scarce_items)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
            """
            
            await postgres_db.execute(
                query,
                500000000,  # Example zeny circulation
                10000000,   # Example per capita
                50,         # Example active players
                1.05,       # Example inflation rate
                indicator.value,
                json.dumps(['607', '608', '985']),
                json.dumps(['616', '617'])
            )
            
        except Exception as e:
            logger.error(f"Failed to create snapshot: {e}")
    
    async def _generate_event_offerings(
        self,
        event_type: str,
        severity: float
    ) -> List[Dict[str, Any]]:
        """Generate offerings for zeny sink event"""
        # Base offerings by event type
        offerings_templates = {
            'cosmetic_sale': [
                {'item': 'Exclusive Headgear', 'base_price': 5000000},
                {'item': 'Rare Costume', 'base_price': 10000000},
                {'item': 'Limited Mount', 'base_price': 20000000}
            ],
            'refinement_bonus': [
                {'item': 'Safe +10 Weapon', 'base_price': 50000000},
                {'item': 'Safe +10 Armor', 'base_price': 40000000},
                {'item': 'Safe +7 Accessory', 'base_price': 20000000}
            ],
            'luxury_buffs': [
                {'item': 'Ultimate ATK Buff (24h)', 'base_price': 10000000},
                {'item': 'Ultimate DEF Buff (24h)', 'base_price': 8000000},
                {'item': 'EXP/Drop Boost (12h)', 'base_price': 15000000}
            ],
            'card_packs': [
                {'item': 'Rare Card Pack (3 cards)', 'base_price': 30000000},
                {'item': 'Epic Card Pack (5 cards)', 'base_price': 50000000},
                {'item': 'Legendary Card (1 guaranteed)', 'base_price': 100000000}
            ]
        }
        
        templates = offerings_templates.get(event_type, [])
        
        # Scale prices by severity
        offerings = []
        for template in templates:
            offerings.append({
                'item_name': template['item'],
                'price': int(template['base_price'] * (1 + severity * 0.5)),
                'quantity_limit': max(1, int(10 / (1 + severity)))
            })
        
        return offerings
    
    async def _store_zeny_sink_event(
        self,
        event_type: str,
        event_name: str,
        target_zeny_drain: int,
        offerings: List[Dict],
        duration_hours: int
    ) -> int:
        """Store zeny sink event in database"""
        try:
            query = """
                INSERT INTO zeny_sink_events
                (event_type, event_name, target_zeny_drain, item_offerings, ends_at)
                VALUES ($1, $2, $3, $4, NOW() + INTERVAL '%s hours')
                RETURNING event_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                event_type,
                event_name,
                target_zeny_drain,
                json.dumps(offerings),
                duration_hours
            )
            
            return result['event_id']
            
        except Exception as e:
            logger.error(f"Failed to store zeny sink event: {e}")
            return 0


# Global instance
merchant_economy_agent = MerchantEconomyAgent()