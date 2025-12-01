"""
Karma Agent - Tracks World Morality & Applies Atmospheric Effects
Implements 4-tier LLM optimization for persistent world alignment
"""

import asyncio
import hashlib
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from enum import Enum
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class KarmaAlignment(Enum):
    """World karma alignment levels"""
    VERY_GOOD = "very_good"      # >7000
    GOOD = "good"                # 3000-7000
    NEUTRAL = "neutral"          # -3000 to 3000
    EVIL = "evil"                # -7000 to -3000
    VERY_EVIL = "very_evil"      # <-7000


class KarmaAgent(BaseAIAgent):
    """
    Tracks world morality balance and applies atmospheric effects.
    
    Karma Sources:
    - Good Actions: Helping NPCs, defeating evil bosses, protecting peaceful monsters
    - Evil Actions: Killing protected monsters, helping Shadow Cult, dark rituals
    
    World Effects by Alignment:
    - VERY_GOOD: Extended daylight, +20% healing, peaceful NPCs
    - GOOD: Standard gameplay with blessing buffs
    - NEUTRAL: Balanced, more random events
    - EVIL: Extended night, dark items cheaper, aggressive spawns
    - VERY_EVIL: Permanent night mode, demon spawns, chaos
    
    4-Tier Optimization:
    - Tier 1: Rule-based action classification (0 LLM calls)
    - Tier 2: Cached karma calculations (0 LLM calls)
    - Tier 3: Batched player karma updates (1 call for multiple)
    - Tier 4: Creative lore descriptions (optional)
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Karma Agent"""
        super().__init__(
            agent_id="karma_agent",
            agent_type="karma",
            config=config
        )
        
        # Karma alignment thresholds
        self.alignment_thresholds = {
            'very_good': 7000,
            'good': 3000,
            'neutral_high': 3000,
            'neutral_low': -3000,
            'evil': -7000
        }
        
        # Action karma values (Tier 1: Rule-based)
        self.action_karma_values = {
            # Good actions (positive karma)
            'help_npc_quest': 15,
            'defeat_evil_boss': 25,
            'protect_peaceful_monster': 10,
            'donate_temple': 20,
            'heal_wounded_npc': 12,
            'complete_good_quest': 30,
            'save_player': 18,
            
            # Evil actions (negative karma)
            'kill_poring': -10,
            'kill_lunatic': -10,
            'kill_drops': -10,
            'help_shadow_cult': -25,
            'use_forbidden_skill': -15,
            'complete_evil_quest': -30,
            'kill_protected_npc': -50,
            
            # Neutral/small impact
            'kill_demon': 5,
            'kill_undead': 5,
            'trade_normal': 0,
            'pvp_duel': 0
        }
        
        # World effect templates by alignment
        self.alignment_effects = {
            KarmaAlignment.VERY_GOOD: {
                'day_length_modifier': 1.3,      # +30% daylight
                'heal_rate_modifier': 1.2,       # +20% healing
                'special_npcs': ['angel_merchant', 'divine_healer'],
                'atmosphere': 'bright',
                'description': 'Divine light bathes the world'
            },
            KarmaAlignment.GOOD: {
                'day_length_modifier': 1.1,      # +10% daylight
                'heal_rate_modifier': 1.0,
                'special_npcs': ['blessing_priest'],
                'atmosphere': 'pleasant',
                'description': 'A sense of peace fills the air'
            },
            KarmaAlignment.NEUTRAL: {
                'day_length_modifier': 1.0,
                'heal_rate_modifier': 1.0,
                'special_npcs': [],
                'atmosphere': 'balanced',
                'description': 'The world exists in balance'
            },
            KarmaAlignment.EVIL: {
                'day_length_modifier': 0.9,      # +11% night
                'heal_rate_modifier': 0.9,       # -10% healing
                'special_npcs': ['dark_merchant'],
                'atmosphere': 'ominous',
                'description': 'Shadows grow longer'
            },
            KarmaAlignment.VERY_EVIL: {
                'day_length_modifier': 0.7,      # +43% night
                'heal_rate_modifier': 0.8,       # -20% healing
                'special_npcs': ['demon_lord', 'chaos_spawn'],
                'atmosphere': 'dark',
                'description': 'Darkness consumes the world'
            }
        }
        
        # Protected monster list
        self.protected_monsters = getattr(settings, 'PROTECTED_MONSTERS', ['PORING', 'LUNATIC', 'DROPS'])
        
        # Daily decay rate
        self.daily_decay_rate = getattr(settings, 'KARMA_DAILY_DECAY', 0.01)
        
        logger.info("Karma Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for karma management"""
        from crewai import Agent
        
        return Agent(
            role="World Morality Guardian",
            goal="Track global karma and maintain moral balance",
            backstory="An AI system that monitors the moral alignment of the world",
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
    
    async def update_global_karma(
        self,
        player_actions: List[Dict[str, Any]]
    ) -> AgentResponse:
        """
        Update global karma from batched player actions.
        
        Calculation:
        - Each action has karma value (-100 to +100)
        - Weighted by player count (more players = bigger impact)
        - Threshold crossings trigger world changes
        
        Args:
            player_actions: List of recent player actions
            
        Returns:
            AgentResponse with updated karma and effects
        """
        try:
            logger.info(f"Updating global karma from {len(player_actions)} actions")
            
            # Get current global karma
            current_karma = await self._get_global_karma()
            karma_changes = 0
            
            # Process actions (Tier 1: Rule-based classification)
            for action in player_actions:
                action_type = action.get('action_type')
                player_id = action.get('player_id')
                
                # Classify action morality
                karma_value = await self.classify_action_morality(action_type)
                
                if karma_value != 0:
                    # Global impact is 10% of player karma
                    global_impact = karma_value // 10
                    karma_changes += global_impact
                    
                    # Update player karma
                    await self.track_player_karma(player_id, karma_value)
                    
                    # Record action
                    await self._record_karma_action(
                        player_id=player_id,
                        action_type=action_type,
                        karma_value=karma_value,
                        global_impact=global_impact,
                        action_data=action.get('data')
                    )
            
            # Apply karma changes
            new_karma = current_karma + karma_changes
            
            # Determine alignments
            old_alignment = self._determine_alignment(current_karma)
            new_alignment = self._determine_alignment(new_karma)
            
            # Update database
            await self._update_global_karma(new_karma)
            
            # Check if alignment threshold crossed
            effects_triggered = None
            if old_alignment != new_alignment:
                logger.info(f"Karma alignment shifted: {old_alignment.value} → {new_alignment.value}")
                effects_triggered = await self.apply_karma_effects(new_alignment)
            
            result_data = {
                'old_karma': current_karma,
                'new_karma': new_karma,
                'karma_change': karma_changes,
                'old_alignment': old_alignment.value,
                'new_alignment': new_alignment.value,
                'alignment_changed': old_alignment != new_alignment,
                'effects_triggered': effects_triggered,
                'actions_processed': len(player_actions),
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            # Clear cache
            await db.delete("karma:global:current")
            
            logger.info(f"✓ Global karma updated: {current_karma} → {new_karma} ({new_alignment.value})")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Global karma updated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to update global karma: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def classify_action_morality(
        self,
        action_type: str,
        target: Optional[str] = None
    ) -> int:
        """
        Classify action as good/evil using Tier 1 rule-based logic.
        
        Rules:
        - Kill protected monsters (Poring, Lunatic, Drops) → -10
        - Kill demon/undead → +5
        - Help NPC quest → +15
        - Donate to temple → +20
        - Use forbidden skill → -15
        - Complete Shadow Cult quest → -25
        
        Args:
            action_type: Type of action performed
            target: Optional target of action
            
        Returns:
            Karma value (-100 to +100)
        """
        # Direct lookup (Tier 1)
        karma_value = self.action_karma_values.get(action_type, 0)
        
        # Check for protected monster kills
        if action_type == 'kill_monster' and target:
            if target.upper() in self.protected_monsters:
                karma_value = -10
            elif 'DEMON' in target.upper() or 'UNDEAD' in target.upper():
                karma_value = 5
        
        return karma_value
    
    async def apply_karma_effects(
        self,
        alignment: KarmaAlignment
    ) -> Dict[str, Any]:
        """
        Apply world effects based on karma alignment.
        
        Effects by Alignment:
        - VERY_GOOD: day_length +30%, heal +20%, angel_npcs
        - GOOD: day_length +10%, blessing_buffs
        - NEUTRAL: standard, random_events +20%
        - EVIL: night_length +10%, dark_items -20% price
        - VERY_EVIL: permanent_night, demon_spawns, chaos_mobs
        
        Args:
            alignment: Current karma alignment
            
        Returns:
            Dict of triggered effects
        """
        try:
            effects_config = self.alignment_effects.get(alignment, {})
            
            effects = {
                'alignment': alignment.value,
                'day_length_modifier': effects_config.get('day_length_modifier', 1.0),
                'heal_rate_modifier': effects_config.get('heal_rate_modifier', 1.0),
                'special_npcs': effects_config.get('special_npcs', []),
                'atmosphere': effects_config.get('atmosphere', 'balanced'),
                'description': effects_config.get('description', ''),
                'applied_at': datetime.now(UTC).isoformat()
            }
            
            # Store effects in database
            await postgres_db.execute(
                """
                UPDATE global_karma
                SET alignment = $1,
                    last_shift = NOW()
                WHERE id = 1
                """,
                alignment.value
            )
            
            # Cache effects
            await db.set("karma:effects:current", effects, expire=3600)
            
            logger.info(f"✓ Applied karma effects for {alignment.value}")
            
            return effects
            
        except Exception as e:
            logger.error(f"Failed to apply karma effects: {e}")
            return {}
    
    async def track_player_karma(
        self,
        player_id: int,
        karma_change: int = 0
    ) -> int:
        """
        Track individual player karma.
        
        Used for:
        - Personal good/evil title
        - Access to karma-locked NPCs
        - Special quests based on alignment
        - Faction reputation modifiers
        
        Args:
            player_id: Player ID
            karma_change: Karma change amount
            
        Returns:
            Updated player karma score
        """
        try:
            # Try cache first
            cache_key = f"karma:player:{player_id}"
            cached_karma = await db.get(cache_key)
            
            if cached_karma is not None and karma_change == 0:
                return int(cached_karma)
            
            # Upsert player karma
            query = """
                INSERT INTO player_karma
                (player_id, karma_score, alignment, good_actions, evil_actions)
                VALUES ($1, $2, $3, 
                    CASE WHEN $2 > 0 THEN 1 ELSE 0 END,
                    CASE WHEN $2 < 0 THEN 1 ELSE 0 END
                )
                ON CONFLICT (player_id)
                DO UPDATE SET
                    karma_score = GREATEST(-10000, LEAST(10000, player_karma.karma_score + $2)),
                    good_actions = player_karma.good_actions + CASE WHEN $2 > 0 THEN 1 ELSE 0 END,
                    evil_actions = player_karma.evil_actions + CASE WHEN $2 < 0 THEN 1 ELSE 0 END,
                    alignment = CASE
                        WHEN player_karma.karma_score + $2 > 7000 THEN 'very_good'
                        WHEN player_karma.karma_score + $2 > 3000 THEN 'good'
                        WHEN player_karma.karma_score + $2 > -3000 THEN 'neutral'
                        WHEN player_karma.karma_score + $2 > -7000 THEN 'evil'
                        ELSE 'very_evil'
                    END,
                    last_action = NOW()
                RETURNING karma_score
            """
            
            result = await postgres_db.fetch_one(query, player_id, karma_change, 'neutral')
            
            if result:
                new_karma = result['karma_score']
                
                # Cache for 5 minutes
                await db.set(cache_key, new_karma, expire=300)
                
                return new_karma
            
            return 0
            
        except Exception as e:
            logger.error(f"Failed to track player karma: {e}")
            return 0
    
    async def apply_daily_decay(self) -> int:
        """
        Apply daily karma decay toward neutral.
        
        Prevents permanent karma extremes by applying 1% decay per day.
        
        Returns:
            Number of affected players
        """
        try:
            logger.info("Applying daily karma decay")
            
            # Decay global karma
            current_karma = await self._get_global_karma()
            if abs(current_karma) > 100:
                decay = int(current_karma * self.daily_decay_rate)
                new_karma = current_karma - decay
                await self._update_global_karma(new_karma)
                logger.info(f"Global karma decayed: {current_karma} → {new_karma}")
            
            # Decay player karma (batch update)
            query = """
                UPDATE player_karma
                SET karma_score = karma_score - (karma_score * $1)::INTEGER,
                    alignment = CASE
                        WHEN karma_score - (karma_score * $1)::INTEGER > 7000 THEN 'very_good'
                        WHEN karma_score - (karma_score * $1)::INTEGER > 3000 THEN 'good'
                        WHEN karma_score - (karma_score * $1)::INTEGER > -3000 THEN 'neutral'
                        WHEN karma_score - (karma_score * $1)::INTEGER > -7000 THEN 'evil'
                        ELSE 'very_evil'
                    END
                WHERE ABS(karma_score) > 100
            """
            
            rows_affected = await postgres_db.execute(query, self.daily_decay_rate)
            
            # Clear caches
            await db.delete("karma:global:current")
            
            logger.info(f"✓ Karma decay applied to {rows_affected} players")
            
            return rows_affected
            
        except Exception as e:
            logger.error(f"Failed to apply karma decay: {e}")
            return 0
    
    async def get_global_karma_state(self) -> Dict[str, Any]:
        """Get current global karma state"""
        try:
            # Try cache first
            cached = await db.get("karma:global:current")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT karma_score, alignment, good_actions_count, evil_actions_count, last_shift
                FROM global_karma
                WHERE id = 1
            """
            
            result = await postgres_db.fetch_one(query)
            
            if result:
                karma_state = {
                    'karma_score': result['karma_score'],
                    'alignment': result['alignment'],
                    'good_actions_count': result['good_actions_count'],
                    'evil_actions_count': result['evil_actions_count'],
                    'last_shift': result['last_shift'].isoformat() if result['last_shift'] else None,
                    'effects': self.alignment_effects.get(
                        KarmaAlignment(result['alignment']),
                        {}
                    )
                }
                
                # Cache for 5 minutes
                await db.set("karma:global:current", karma_state, expire=300)
                
                return karma_state
            
            # Initialize if not exists
            await self._initialize_global_karma()
            return await self.get_global_karma_state()
            
        except Exception as e:
            logger.error(f"Failed to get karma state: {e}")
            return {
                'karma_score': 0,
                'alignment': 'neutral',
                'good_actions_count': 0,
                'evil_actions_count': 0
            }
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _determine_alignment(self, karma_score: int) -> KarmaAlignment:
        """Determine alignment from karma score"""
        if karma_score > self.alignment_thresholds['very_good']:
            return KarmaAlignment.VERY_GOOD
        elif karma_score > self.alignment_thresholds['good']:
            return KarmaAlignment.GOOD
        elif karma_score > self.alignment_thresholds['neutral_low']:
            return KarmaAlignment.NEUTRAL
        elif karma_score > self.alignment_thresholds['evil']:
            return KarmaAlignment.EVIL
        else:
            return KarmaAlignment.VERY_EVIL
    
    async def _get_global_karma(self) -> int:
        """Get current global karma score"""
        try:
            query = """
                SELECT karma_score
                FROM global_karma
                WHERE id = 1
            """
            
            result = await postgres_db.fetch_one(query)
            
            if result:
                return result['karma_score']
            
            # Initialize if not exists
            await self._initialize_global_karma()
            return 0
            
        except Exception as e:
            logger.error(f"Failed to get global karma: {e}")
            return 0
    
    async def _update_global_karma(self, new_karma: int):
        """Update global karma score"""
        try:
            # Clamp to valid range
            new_karma = max(-10000, min(10000, new_karma))
            
            alignment = self._determine_alignment(new_karma)
            
            query = """
                UPDATE global_karma
                SET karma_score = $1,
                    alignment = $2,
                    good_actions_count = good_actions_count + CASE WHEN $1 > karma_score THEN 1 ELSE 0 END,
                    evil_actions_count = evil_actions_count + CASE WHEN $1 < karma_score THEN 1 ELSE 0 END
                WHERE id = 1
            """
            
            await postgres_db.execute(query, new_karma, alignment.value)
            
        except Exception as e:
            logger.error(f"Failed to update global karma: {e}")
    
    async def _initialize_global_karma(self):
        """Initialize global karma if not exists"""
        try:
            query = """
                INSERT INTO global_karma
                (id, karma_score, alignment, good_actions_count, evil_actions_count)
                VALUES (1, 0, 'neutral', 0, 0)
                ON CONFLICT (id) DO NOTHING
            """
            
            await postgres_db.execute(query)
            
        except Exception as e:
            logger.error(f"Failed to initialize global karma: {e}")
    
    async def _record_karma_action(
        self,
        player_id: int,
        action_type: str,
        karma_value: int,
        global_impact: int,
        action_data: Optional[Dict] = None
    ):
        """Record karma action in history"""
        try:
            query = """
                INSERT INTO karma_actions
                (player_id, action_type, karma_value, global_impact, action_data)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                player_id,
                action_type,
                karma_value,
                global_impact,
                json.dumps(action_data) if action_data else None
            )
            
        except Exception as e:
            logger.error(f"Failed to record karma action: {e}")


# Global instance
karma_agent = KarmaAgent()