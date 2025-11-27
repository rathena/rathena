"""
World Event Agent - Monitors and Triggers Server-Wide Events
Implements threshold-based event triggering for large-scale server events
"""

import asyncio
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any, Tuple
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings
from models.procedural import (
    EventType, EventStatus, EventTriggerConditions, EventImpactData
)


class WorldEventAgent(BaseAIAgent):
    """
    Monitors world state and triggers server-wide events.
    
    Event Types:
    - Demonic Invasion: Waves spawn in Prontera Field
    - Market Crash: Item prices drop temporarily
    - Lucky Day: Increased refine success (30 min)
    - Wandering Caravan: Traveling merchants everywhere
    - Global XP Boost: Low player count compensation
    - MVP Enraged: All MVPs become harder but better loot
    - Mythic Event: Ultra-rare special event (0.1% chance)
    
    Features:
    - Real-time threshold monitoring
    - Automatic event triggering
    - Cooldown management
    - Maximum 1 concurrent event
    - Event impact calculation
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize World Event Agent"""
        super().__init__(
            agent_id="world_event_agent",
            agent_type="world_event",
            config=config
        )
        
        # Event threshold configurations
        self.event_thresholds = {
            EventType.DEMONIC_INVASION: {
                "condition": "mvp_kills_high",
                "threshold": 100,
                "duration_minutes": (30, 90),
                "cooldown_hours": 6,
                "probability": 1.0
            },
            EventType.MARKET_CRASH: {
                "condition": "high_zeny_circulation",
                "threshold": 1000000000,  # 1 billion zeny
                "duration_minutes": (60, 120),
                "cooldown_hours": 12,
                "probability": 1.0
            },
            EventType.LUCKY_DAY: {
                "condition": "low_player_count",
                "threshold": 10,
                "duration_minutes": (30, 60),
                "cooldown_hours": 8,
                "probability": 0.5
            },
            EventType.WANDERING_CARAVAN: {
                "condition": "moderate_activity",
                "threshold": 20,
                "duration_minutes": (45, 90),
                "cooldown_hours": 6,
                "probability": 0.7
            },
            EventType.GLOBAL_XP_BOOST: {
                "condition": "very_low_players",
                "threshold": 5,
                "duration_minutes": (60, 180),
                "cooldown_hours": 4,
                "probability": 1.0
            },
            EventType.MVP_ENRAGED: {
                "condition": "easy_mvp_kills",
                "threshold": 50,
                "duration_minutes": (30, 60),
                "cooldown_hours": 8,
                "probability": 0.8
            },
            EventType.MYTHIC_EVENT: {
                "condition": "rare_combination",
                "threshold": 0,
                "duration_minutes": (15, 30),
                "cooldown_hours": 48,
                "probability": 0.001  # 0.1% chance
            }
        }
        
        # Track last trigger times for cooldowns
        self.last_trigger_times: Dict[str, datetime] = {}
        
        # Maximum concurrent events
        self.max_concurrent_events = getattr(
            settings, 
            'event_max_concurrent', 
            1
        )
        
        logger.info(f"World Event Agent initialized with {len(self.event_thresholds)} event types")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for event generation"""
        from crewai import Agent
        
        return Agent(
            role="World Event Coordinator",
            goal="Monitor world state and trigger engaging server-wide events",
            backstory="An AI that orchestrates grand events affecting all players",
            verbose=settings.crewai_verbose,
            allow_delegation=False,
            llm=self.llm_provider if hasattr(self, 'llm_provider') else None
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """Process method required by BaseAIAgent (not used directly)"""
        return AgentResponse(
            agent_type=self.agent_type,
            success=True,
            data={},
            confidence=1.0
        )
    
    # ========================================================================
    # PUBLIC API METHODS
    # ========================================================================
    
    async def monitor_and_trigger(
        self,
        world_state: Dict[str, Any]
    ) -> Optional[AgentResponse]:
        """
        Check thresholds and trigger events if conditions met.
        
        Args:
            world_state: Current world metrics
            
        Returns:
            AgentResponse if event triggered, None otherwise
        """
        try:
            # Check if max concurrent events reached
            active_events = await self._get_active_events()
            if len(active_events) >= self.max_concurrent_events:
                logger.debug(f"Max concurrent events reached ({len(active_events)}/{self.max_concurrent_events})")
                return None
            
            # Check each event type's thresholds
            for event_type, config in self.event_thresholds.items():
                # Check cooldown
                if self._is_on_cooldown(event_type, config['cooldown_hours']):
                    continue
                
                # Check probability
                if random.random() > config['probability']:
                    continue
                
                # Check threshold condition
                if self._check_threshold(event_type, config, world_state):
                    logger.info(f"Event threshold met for {event_type}")
                    
                    # Trigger event
                    return await self.trigger_event(event_type, world_state)
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to monitor and trigger events: {e}", exc_info=True)
            return None
    
    async def trigger_event(
        self,
        event_type: str,
        world_state: Dict[str, Any],
        custom_impact: Optional[Dict[str, Any]] = None
    ) -> AgentResponse:
        """
        Trigger a world event.
        
        Args:
            event_type: Type of event to trigger
            world_state: Current world metrics
            custom_impact: Optional custom impact data (admin override)
            
        Returns:
            AgentResponse with event details
        """
        try:
            config = self.event_thresholds.get(event_type)
            if not config:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": f"Unknown event type: {event_type}"},
                    confidence=0.0,
                    reasoning="Invalid event type"
                )
            
            # Generate event details
            event_data = await self._generate_event_details(
                event_type,
                config,
                world_state
            )
            
            # Calculate impact
            impact_data = custom_impact or await self.calculate_event_impact(
                event_type,
                event_data['duration_minutes']
            )
            
            # Store in database
            event_id = await self._store_event(event_data, impact_data)
            
            # Cache in DragonflyDB
            await self._cache_event(event_id, event_data, impact_data)
            
            # Update last trigger time
            self.last_trigger_times[event_type] = datetime.now(UTC)
            
            result_data = {
                "event_id": event_id,
                **event_data,
                "impact_data": impact_data
            }
            
            logger.info(f"✓ Triggered event #{event_id}: {event_data['title']}")
            
            # Broadcast to all players (via Bridge Layer in production)
            await self._broadcast_event_announcement(result_data)
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning=f"Triggered {event_type} event"
            )
            
        except Exception as e:
            logger.error(f"Failed to trigger event: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def calculate_event_impact(
        self,
        event_type: str,
        duration_minutes: int
    ) -> Dict[str, Any]:
        """
        Calculate event impact (buffs, spawns, economic effects).
        
        Args:
            event_type: Type of event
            duration_minutes: Event duration
            
        Returns:
            Impact data structure
        """
        impacts = {
            EventType.DEMONIC_INVASION: {
                "global_buffs": None,
                "spawn_modifications": {
                    "maps": ["prt_fild01", "prt_fild02", "prt_fild03"],
                    "mob_id": 1582,  # Deviling
                    "spawn_multiplier": 3.0,
                    "waves": duration_minutes // 15
                },
                "economic_effects": None,
                "special_effects": [
                    "Demon waves spawn every 15 minutes",
                    "Defeat all demons for bonus rewards",
                    "MVP demons may appear"
                ]
            },
            EventType.MARKET_CRASH: {
                "global_buffs": None,
                "spawn_modifications": None,
                "economic_effects": {
                    "price_multiplier": 0.7,  # 30% discount
                    "affected_items": "all",
                    "npc_buy_rate": 1.3
                },
                "special_effects": [
                    "All shop prices reduced by 30%",
                    "NPC buy rates increased",
                    "Great time for shopping!"
                ]
            },
            EventType.LUCKY_DAY: {
                "global_buffs": [
                    {"type": "refine_success", "value": 1.5, "description": "+50% refine success rate"},
                    {"type": "drop_rate", "value": 1.2, "description": "+20% drop rate"},
                    {"type": "card_drop", "value": 1.3, "description": "+30% card drop rate"}
                ],
                "spawn_modifications": None,
                "economic_effects": None,
                "special_effects": [
                    "Lady Luck smiles upon you!",
                    "Enhanced success rates for all activities",
                    "Perfect time for refining equipment"
                ]
            },
            EventType.WANDERING_CARAVAN: {
                "global_buffs": None,
                "spawn_modifications": {
                    "npc_type": "traveling_merchant",
                    "spawn_maps": "all_towns",
                    "spawn_count": 10,
                    "sells_rare_items": True
                },
                "economic_effects": {
                    "special_items_available": True
                },
                "special_effects": [
                    "Traveling merchants appear in all towns",
                    "Rare items available for purchase",
                    "Limited stock, first come first served!"
                ]
            },
            EventType.GLOBAL_XP_BOOST: {
                "global_buffs": [
                    {"type": "exp_rate", "value": 2.0, "description": "+100% EXP gain"},
                    {"type": "job_exp_rate", "value": 2.0, "description": "+100% Job EXP gain"}
                ],
                "spawn_modifications": None,
                "economic_effects": None,
                "special_effects": [
                    "Double experience for all players!",
                    "Perfect time for leveling",
                    f"Event lasts {duration_minutes} minutes"
                ]
            },
            EventType.MVP_ENRAGED: {
                "global_buffs": None,
                "spawn_modifications": {
                    "mvp_modifier": {
                        "hp_multiplier": 1.5,
                        "atk_multiplier": 1.3,
                        "def_multiplier": 1.2
                    },
                    "reward_modifier": {
                        "exp_multiplier": 2.0,
                        "drop_rate_multiplier": 1.5,
                        "rare_drop_multiplier": 2.0
                    }
                },
                "economic_effects": None,
                "special_effects": [
                    "All MVPs are enraged!",
                    "+50% HP, +30% ATK, +20% DEF",
                    "Double EXP and better loot!",
                    "Challenge the strongest bosses!"
                ]
            },
            EventType.MYTHIC_EVENT: {
                "global_buffs": [
                    {"type": "all_stats", "value": 1.5, "description": "+50% all stats"},
                    {"type": "magic_damage", "value": 2.0, "description": "+100% magic damage"},
                    {"type": "physical_damage", "value": 2.0, "description": "+100% physical damage"}
                ],
                "spawn_modifications": {
                    "special_boss": "Mythic Guardian",
                    "spawn_map": "prontera",
                    "boss_hp": 10000000,
                    "boss_rewards": "legendary"
                },
                "economic_effects": None,
                "special_effects": [
                    "⚡ MYTHIC EVENT TRIGGERED! ⚡",
                    "Ancient powers awaken",
                    "Legendary boss spawned in Prontera",
                    "Defeat for ultimate rewards",
                    "All players temporarily empowered!"
                ]
            }
        }
        
        return impacts.get(event_type, {
            "global_buffs": None,
            "spawn_modifications": None,
            "economic_effects": None,
            "special_effects": []
        })
    
    async def record_participation(
        self,
        event_id: int,
        player_id: int,
        contribution_score: int = 0
    ) -> bool:
        """
        Record player participation in event.
        
        Args:
            event_id: Event ID
            player_id: Player ID
            contribution_score: Player's contribution score
            
        Returns:
            Success status
        """
        try:
            query = """
                INSERT INTO event_participation 
                (event_id, player_id, contribution_score)
                VALUES ($1, $2, $3)
                ON CONFLICT (event_id, player_id) DO UPDATE
                SET contribution_score = event_participation.contribution_score + $3
            """
            
            await postgres_db.execute(query, event_id, player_id, contribution_score)
            
            # Update participation count
            await postgres_db.execute(
                "UPDATE world_events SET participation_count = participation_count + 1 WHERE event_id = $1",
                event_id
            )
            
            logger.debug(f"Recorded participation: player {player_id} in event {event_id}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to record participation: {e}")
            return False
    
    async def get_active_events(self) -> List[Dict[str, Any]]:
        """Get all currently active events"""
        return await self._get_active_events()
    
    async def end_event(self, event_id: int) -> bool:
        """
        End an event and clean up.
        
        Args:
            event_id: Event ID
            
        Returns:
            Success status
        """
        try:
            query = """
                UPDATE world_events 
                SET status = $1
                WHERE event_id = $2 AND status = 'active'
            """
            
            rows_affected = await postgres_db.execute(query, EventStatus.COMPLETED.value, event_id)
            
            if rows_affected > 0:
                # Clear cache
                await db.delete(f"event:{event_id}")
                await db.delete("events:active")
                logger.info(f"✓ Event #{event_id} ended")
                return True
            
            return False
            
        except Exception as e:
            logger.error(f"Failed to end event: {e}")
            return False
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _check_threshold(
        self,
        event_type: str,
        config: Dict[str, Any],
        world_state: Dict[str, Any]
    ) -> bool:
        """Check if event threshold condition is met"""
        condition = config['condition']
        threshold = config['threshold']
        
        # Get relevant metrics
        online_players = world_state.get('online_players', 0)
        mvp_kills_today = sum(world_state.get('mvp_kills', {}).values())
        avg_level = world_state.get('avg_player_level', 50)
        economy = world_state.get('economy', {})
        zeny_circulation = economy.get('zeny_circulation', 0)
        
        # Check conditions
        if condition == "mvp_kills_high":
            return mvp_kills_today >= threshold
        
        elif condition == "high_zeny_circulation":
            return zeny_circulation >= threshold
        
        elif condition == "low_player_count":
            return 0 < online_players <= threshold
        
        elif condition == "very_low_players":
            return 0 < online_players <= threshold
        
        elif condition == "moderate_activity":
            return online_players >= threshold
        
        elif condition == "easy_mvp_kills":
            # MVPs killed too easily (high kill count with moderate players)
            if online_players > 0:
                kills_per_player = mvp_kills_today / online_players
                return kills_per_player >= (threshold / online_players)
            return False
        
        elif condition == "rare_combination":
            # Random mythic event chance
            return (
                online_players >= 20 and
                mvp_kills_today >= 30 and
                avg_level >= 80 and
                random.random() < 0.001  # 0.1% chance
            )
        
        return False
    
    def _is_on_cooldown(self, event_type: str, cooldown_hours: int) -> bool:
        """Check if event is on cooldown"""
        if event_type not in self.last_trigger_times:
            return False
        
        last_trigger = self.last_trigger_times[event_type]
        cooldown_end = last_trigger + timedelta(hours=cooldown_hours)
        
        return datetime.now(UTC) < cooldown_end
    
    async def _generate_event_details(
        self,
        event_type: str,
        config: Dict[str, Any],
        world_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Generate event details including title, description, triggers"""
        # Generate title and description
        if event_type == EventType.MYTHIC_EVENT:
            title = "⚡ MYTHIC EVENT: Ancient Powers Awaken ⚡"
            description = "A legendary event of immense power! All adventurers are called to action!"
        else:
            event_names = {
                EventType.DEMONIC_INVASION: "Demonic Invasion",
                EventType.MARKET_CRASH: "Market Festival Sale",
                EventType.LUCKY_DAY: "Lucky Day Celebration",
                EventType.WANDERING_CARAVAN: "Wandering Caravan Arrival",
                EventType.GLOBAL_XP_BOOST: "Bonus Experience Event",
                EventType.MVP_ENRAGED: "MVP Enraged Mode"
            }
            
            title = event_names.get(event_type, "World Event")
            description = f"A special {title} has been triggered! Take advantage while it lasts!"
        
        # Calculate duration
        min_duration, max_duration = config['duration_minutes']
        duration_minutes = random.randint(min_duration, max_duration)
        
        # Calculate end time
        starts_at = datetime.now(UTC)
        ends_at = starts_at + timedelta(minutes=duration_minutes)
        
        # Build trigger conditions
        trigger_conditions = self._build_trigger_conditions(event_type, config, world_state)
        
        return {
            "event_type": event_type,
            "title": title,
            "description": description,
            "trigger_conditions": trigger_conditions,
            "duration_minutes": duration_minutes,
            "status": EventStatus.ACTIVE.value,
            "started_at": starts_at,
            "ends_at": ends_at
        }
    
    def _build_trigger_conditions(
        self,
        event_type: str,
        config: Dict[str, Any],
        world_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Build trigger condition data"""
        condition_type = config['condition']
        threshold = config['threshold']
        
        # Get actual value that triggered event
        actual_value = self._get_actual_value(condition_type, world_state)
        
        descriptions = {
            "mvp_kills_high": f"MVP kills exceeded {threshold}",
            "high_zeny_circulation": f"Zeny circulation exceeded {threshold:,}",
            "low_player_count": f"Low player activity detected",
            "very_low_players": f"Very low player count ({actual_value} online)",
            "moderate_activity": f"Moderate server activity",
            "easy_mvp_kills": f"MVPs being defeated too easily",
            "rare_combination": "Rare cosmic alignment detected!"
        }
        
        return {
            "condition_type": condition_type,
            "threshold_value": threshold,
            "actual_value": actual_value,
            "description": descriptions.get(condition_type, "Event triggered")
        }
    
    def _get_actual_value(self, condition_type: str, world_state: Dict[str, Any]) -> Any:
        """Get actual value for condition"""
        if "player" in condition_type:
            return world_state.get('online_players', 0)
        elif "mvp" in condition_type:
            return sum(world_state.get('mvp_kills', {}).values())
        elif "zeny" in condition_type:
            return world_state.get('economy', {}).get('zeny_circulation', 0)
        else:
            return "N/A"
    
    async def _store_event(
        self,
        event_data: Dict[str, Any],
        impact_data: Dict[str, Any]
    ) -> int:
        """Store event in PostgreSQL"""
        query = """
            INSERT INTO world_events 
            (event_type, title, description, trigger_conditions, impact_data, duration_minutes, status, ends_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8)
            RETURNING event_id
        """
        
        result = await postgres_db.fetch_one(
            query,
            event_data['event_type'],
            event_data['title'],
            event_data['description'],
            json.dumps(event_data['trigger_conditions']),
            json.dumps(impact_data),
            event_data['duration_minutes'],
            event_data['status'],
            event_data['ends_at']
        )
        
        return result['event_id']
    
    async def _cache_event(
        self,
        event_id: int,
        event_data: Dict[str, Any],
        impact_data: Dict[str, Any]
    ):
        """Cache event in DragonflyDB"""
        try:
            cache_data = {
                "event_id": event_id,
                **event_data,
                "impact_data": impact_data
            }
            
            # Cache specific event
            await db.set(f"event:{event_id}", cache_data, expire=7200)  # 2 hours
            
            # Invalidate active events list
            await db.delete("events:active")
            
        except Exception as e:
            logger.warning(f"Failed to cache event: {e}")
    
    async def _get_active_events(self) -> List[Dict[str, Any]]:
        """Get all active events"""
        try:
            # Try cache first
            cached = await db.get("events:active")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT * FROM world_events
                WHERE status = 'active' AND ends_at > NOW()
                ORDER BY started_at DESC
            """
            
            events = await postgres_db.fetch_all(query)
            
            result = []
            for event in events:
                event_dict = dict(event)
                # Parse JSON fields
                if event_dict.get('trigger_conditions'):
                    event_dict['trigger_conditions'] = json.loads(event_dict['trigger_conditions'])
                if event_dict.get('impact_data'):
                    event_dict['impact_data'] = json.loads(event_dict['impact_data'])
                result.append(event_dict)
            
            # Cache for 1 minute
            await db.set("events:active", result, expire=60)
            
            return result
            
        except Exception as e:
            logger.error(f"Failed to get active events: {e}")
            return []
    
    async def _broadcast_event_announcement(self, event_data: Dict[str, Any]):
        """Broadcast event announcement to all players"""
        # In production, this would dispatch to Bridge Layer
        # For now, just log
        logger.info(f"📢 Broadcasting event: {event_data['title']}")
        logger.info(f"   Duration: {event_data['duration_minutes']} minutes")
        
        # Cache announcement for API access
        await db.set("event:latest_announcement", event_data, expire=3600)


# Global instance
world_event_agent = WorldEventAgent()