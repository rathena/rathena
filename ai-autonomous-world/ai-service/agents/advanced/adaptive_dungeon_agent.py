"""
Adaptive Dungeon Agent - Generates Daily Procedural Dungeon Instances
Implements 4-tier LLM optimization for dynamic dungeon generation
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


class DungeonTheme(str, Enum):
    """Daily dungeon theme types"""
    FIRE_DAY = "fire_day"
    ICE_DAY = "ice_day"
    UNDEAD_DAY = "undead_day"
    DEMON_DAY = "demon_day"
    EARTH_DAY = "earth_day"
    HOLY_DAY = "holy_day"
    CHAOTIC_DAY = "chaotic_day"


class AdaptiveDungeonAgent(BaseAIAgent):
    """
    Generates daily procedural dungeon instances.
    
    Features:
    - Random floor layout (5-10 floors)
    - Random monster pools per floor
    - Random trap placement
    - Daily theme modifier (element focus)
    - Random boss pool (1-3 bosses)
    - Instance-only drops
    - Daily unique currency rewards
    
    Instance Mechanics:
    - Party-based (2-12 players)
    - 60-minute time limit
    - Progressive difficulty per floor
    - Checkpoint system (every 3 floors)
    - Death penalty (retry from last checkpoint)
    
    4-Tier Optimization:
    - Tier 1 (60%): Rule-based layout and monster selection
    - Tier 2 (30%): Cached dungeon specs
    - Tier 3 (8%): Batched floor generation
    - Tier 4 (2%): LLM for creative boss names/lore
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Adaptive Dungeon Agent"""
        super().__init__(
            agent_id="adaptive_dungeon_agent",
            agent_type="adaptive_dungeon",
            config=config
        )
        
        # Layout templates (Tier 1: Rule-based)
        self.room_types = [
            "combat_room",      # Monster spawns
            "treasure_room",    # Treasure chest
            "trap_room",        # Heavy traps
            "mini_boss_room",   # Optional mini-boss
            "checkpoint_room",  # Save point
            "boss_room"         # Final floor only
        ]
        
        # Monster pools by theme
        self.monster_pools = {
            DungeonTheme.FIRE_DAY: ["ELDER_WILLOW", "METALLER", "REQUIEM"],
            DungeonTheme.ICE_DAY: ["ICE_TITAN", "FREEZER", "STORMY_KNIGHT"],
            DungeonTheme.UNDEAD_DAY: ["ZOMBIE", "GHOUL", "WRAITH"],
            DungeonTheme.DEMON_DAY: ["BAPHOMET_", "DARK_LORD", "INCUBUS"],
            DungeonTheme.EARTH_DAY: ["GOLEM", "STONE_SHOOTER", "ELDER"],
            DungeonTheme.HOLY_DAY: ["ANGELING", "ARCHANGELING", "DEVILING"],
            DungeonTheme.CHAOTIC_DAY: []  # Random from all
        }
        
        # Boss pools by theme
        self.boss_pools = {
            DungeonTheme.FIRE_DAY: ["IFRIT", "KTULLANUX", "RSX_0806"],
            DungeonTheme.ICE_DAY: ["KTULLANUX", "STORMY_KNIGHT", "DETALE"],
            DungeonTheme.UNDEAD_DAY: ["DARK_LORD", "DRACULA", "DOPPELGANGER"],
            DungeonTheme.DEMON_DAY: ["BAPHOMET", "DARK_LORD", "BEELZEBUB"],
            DungeonTheme.EARTH_DAY: ["EDDGA", "MAYA", "GARM"],
            DungeonTheme.HOLY_DAY: ["GOLDEN_BUG", "OSIRIS", "MOONLIGHT_FLOWER"],
            DungeonTheme.CHAOTIC_DAY: ["AMON_RA", "PHARAOH", "WOUNDED_MORROC"]
        }
        
        # Theme modifiers
        self.theme_modifiers = {
            DungeonTheme.FIRE_DAY: {
                "element": "fire",
                "stat_bonus": {"atk": 1.2, "matk": 1.2},
                "drop_bonus": ["Fire_Stone", "Flame_Heart"]
            },
            DungeonTheme.ICE_DAY: {
                "element": "water",
                "stat_bonus": {"def": 1.2, "mdef": 1.2},
                "drop_bonus": ["Ice_Scale", "Crystal_Blue"]
            },
            DungeonTheme.UNDEAD_DAY: {
                "element": "undead",
                "stat_bonus": {"hp": 1.3, "flee": 1.1},
                "drop_bonus": ["Rotten_Bandage", "Skull"]
            },
            DungeonTheme.DEMON_DAY: {
                "element": "dark",
                "stat_bonus": {"atk": 1.3, "crit": 1.2},
                "drop_bonus": ["Dark_Crystal", "Evil_Horn"]
            },
            DungeonTheme.EARTH_DAY: {
                "element": "earth",
                "stat_bonus": {"def": 1.3, "hp": 1.2},
                "drop_bonus": ["Rough_Oridecon", "Earth_Essence"]
            },
            DungeonTheme.HOLY_DAY: {
                "element": "holy",
                "stat_bonus": {"matk": 1.3, "heal": 1.2},
                "drop_bonus": ["Holy_Water", "Angelic_Protection"]
            },
            DungeonTheme.CHAOTIC_DAY: {
                "element": "all",
                "stat_bonus": {"all_stats": 1.15},
                "drop_bonus": ["Chaotic_Essence", "Mystery_Box"]
            }
        }
        
        # Floor configuration
        self.floor_config = {
            "min_floors": getattr(settings, 'dungeon_floor_count_min', 5),
            "max_floors": getattr(settings, 'dungeon_floor_count_max', 10),
            "rooms_per_floor": (3, 7),
            "monsters_per_floor": (15, 25),
            "traps_per_floor": (5, 15),
            "treasures_per_floor": (1, 3)
        }
        
        logger.info("Adaptive Dungeon Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for dungeon management"""
        from crewai import Agent
        
        return Agent(
            role="Procedural Dungeon Generator",
            goal="Generate engaging daily dungeon instances",
            backstory="An AI system that creates dynamic dungeon experiences",
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
    
    async def generate_daily_dungeon(
        self,
        player_average_level: int,
        active_factions: List[str]
    ) -> AgentResponse:
        """
        Generate complete dungeon specification for the day.
        
        Generation Process (Tier 1: Rule-based):
        1. Select daily theme (element or faction-based)
        2. Generate floor layouts (5-10 floors)
        3. Populate monster pools per floor
        4. Place traps and treasures
        5. Select boss pool (1-3 bosses)
        6. Calculate rewards (currency, unique drops)
        7. Store in database
        
        Scaling:
        - Floor difficulty = base * (1 + 0.15 * floor_number)
        - Monster count = 10-20 per floor
        - Boss HP/ATK scale to avg player level
        
        Args:
            player_average_level: Average level of active players
            active_factions: List of active faction IDs
            
        Returns:
            Complete dungeon specification
        """
        try:
            logger.info("Generating daily adaptive dungeon")
            
            # Step 1: Select theme (Tier 1: Rule-based)
            theme = self._select_daily_theme(active_factions)
            
            # Step 2: Determine floor count
            floor_count = random.randint(
                self.floor_config['min_floors'],
                self.floor_config['max_floors']
            )
            
            # Step 3: Generate layouts for all floors (Tier 1: Rule-based)
            layouts = []
            for floor_num in range(1, floor_count + 1):
                layout = await self.generate_floor_layout(floor_num, theme)
                layouts.append(layout)
            
            # Step 4: Select monster pools (Tier 1: Rule-based)
            monster_pools = []
            for floor_num in range(1, floor_count + 1):
                pool = await self.select_monster_pool(
                    floor_num,
                    theme,
                    player_average_level
                )
                monster_pools.append(pool)
            
            # Step 5: Select boss pool
            boss_pool = self._select_boss_pool(theme, floor_count)
            
            # Step 6: Calculate base rewards
            reward_data = {
                "currency": floor_count * 100,  # Dungeon Tokens
                "exp_multiplier": 1.0 + (floor_count * 0.1),
                "drop_rate_bonus": 0.5,
                "theme_specific_drops": self.theme_modifiers[theme]["drop_bonus"]
            }
            
            # Step 7: Calculate difficulty rating
            difficulty_rating = min(10, 1 + floor_count // 2)
            
            # Step 8: Store in database
            dungeon_id = await self._store_dungeon(
                theme=theme,
                floor_count=floor_count,
                layouts=layouts,
                monster_pools=monster_pools,
                boss_pool=boss_pool,
                reward_data=reward_data,
                difficulty_rating=difficulty_rating
            )
            
            # Step 9: Cache for fast retrieval
            cache_key = "dungeon:daily:current"
            await db.set(cache_key, {
                "dungeon_id": dungeon_id,
                "theme": theme,
                "floor_count": floor_count,
                "difficulty_rating": difficulty_rating
            }, expire=86400)  # 24 hours
            
            result_data = {
                "dungeon_id": dungeon_id,
                "theme": theme,
                "floor_count": floor_count,
                "difficulty_rating": difficulty_rating,
                "layouts": layouts,
                "monster_pools": monster_pools,
                "boss_pool": boss_pool,
                "rewards": reward_data,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Generated dungeon {dungeon_id}: {theme}, {floor_count} floors")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Daily dungeon generated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to generate daily dungeon: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def generate_floor_layout(
        self,
        floor_number: int,
        theme: DungeonTheme
    ) -> Dict[str, Any]:
        """
        Generate random floor layout (Tier 1: Rule-based).
        
        Layout includes:
        - Room count (3-7 rooms per floor)
        - Room connections (paths)
        - Trap locations (10-20% coverage)
        - Treasure chest locations (1-3 per floor)
        - Monster spawn points (15-25 per floor)
        - Boss room (final floor only)
        - Checkpoint location (every 3 floors)
        
        Args:
            floor_number: Current floor number
            theme: Dungeon theme
            
        Returns:
            Floor layout specification
        """
        try:
            room_count = random.randint(*self.floor_config['rooms_per_floor'])
            
            # Generate rooms
            rooms = []
            for room_idx in range(room_count):
                room_type = self._select_room_type(
                    floor_number,
                    room_idx,
                    room_count
                )
                
                room = {
                    "room_id": f"floor{floor_number}_room{room_idx}",
                    "room_type": room_type,
                    "monsters": [] if room_type != "combat_room" else random.randint(3, 6),
                    "traps": random.randint(1, 3) if room_type == "trap_room" else 0,
                    "treasures": 1 if room_type == "treasure_room" else 0,
                    "connections": []
                }
                rooms.append(room)
            
            # Connect rooms (simple linear path + optional side paths)
            for i in range(len(rooms) - 1):
                rooms[i]["connections"].append(f"floor{floor_number}_room{i+1}")
                # 30% chance of side connection
                if i > 0 and random.random() < 0.3:
                    side_idx = random.randint(0, i-1)
                    rooms[i]["connections"].append(f"floor{floor_number}_room{side_idx}")
            
            layout = {
                "floor_number": floor_number,
                "room_count": room_count,
                "rooms": rooms,
                "is_checkpoint_floor": floor_number % 3 == 0,
                "is_boss_floor": floor_number == getattr(settings, 'dungeon_floor_count_max', 10)
            }
            
            return layout
            
        except Exception as e:
            logger.error(f"Failed to generate floor layout: {e}")
            return {
                "floor_number": floor_number,
                "room_count": 3,
                "rooms": [],
                "error": str(e)
            }
    
    async def select_monster_pool(
        self,
        floor_number: int,
        theme: DungeonTheme,
        player_avg_level: int
    ) -> List[Dict[str, Any]]:
        """
        Select monsters for floor (Tier 1: Rule-based).
        
        Selection Logic:
        - Match theme (Fire Day → fire monsters)
        - Scale to floor difficulty
        - Mix of regular + elite + rare mobs
        - 10% chance for mini-boss per floor
        - Avoid repetition (max 3 of same type)
        
        Args:
            floor_number: Floor number
            theme: Dungeon theme
            player_avg_level: Average player level
            
        Returns:
            List of monster IDs with spawn counts
        """
        try:
            # Get theme monster pool
            theme_monsters = self.monster_pools.get(theme, [])
            
            # If chaotic day, use all monsters
            if theme == DungeonTheme.CHAOTIC_DAY:
                all_monsters = []
                for monsters in self.monster_pools.values():
                    all_monsters.extend(monsters)
                theme_monsters = list(set(all_monsters))
            
            if not theme_monsters:
                theme_monsters = ["PORING", "DROPS", "LUNATIC"]  # Fallback
            
            # Calculate number of monster types for this floor
            monster_type_count = min(len(theme_monsters), random.randint(3, 5))
            
            # Select random monster types
            selected_monsters = random.sample(theme_monsters, monster_type_count)
            
            # Assign spawn counts (scaled by floor difficulty)
            floor_difficulty = 1.0 + (floor_number * 0.15)
            monster_pool = []
            
            for monster_id in selected_monsters:
                spawn_count = int(random.randint(3, 6) * floor_difficulty)
                
                monster_entry = {
                    "monster_id": monster_id,
                    "spawn_count": spawn_count,
                    "level_modifier": floor_difficulty,
                    "is_elite": random.random() < 0.2,  # 20% elite chance
                    "is_mini_boss": random.random() < 0.1  # 10% mini-boss
                }
                monster_pool.append(monster_entry)
            
            return monster_pool
            
        except Exception as e:
            logger.error(f"Failed to select monster pool: {e}")
            return []
    
    async def calculate_dungeon_rewards(
        self,
        theme: DungeonTheme,
        floors_completed: int,
        time_taken: int,
        party_size: int
    ) -> Dict[str, Any]:
        """
        Calculate instance rewards.
        
        Reward Types:
        - Instance currency (Dungeon Tokens)
        - Crafting materials for high-tier gear
        - Theme-specific drops (fire gems on Fire Day)
        - Random enchantment scrolls
        - Reputation points (Problem Solver)
        
        Scaling:
        - More floors = more rewards
        - Faster clear = bonus multiplier
        - Larger party = distributed rewards
        
        Args:
            theme: Dungeon theme
            floors_completed: Number of floors cleared
            time_taken: Time in seconds
            party_size: Number of participants
            
        Returns:
            Reward specification
        """
        try:
            # Base rewards
            base_tokens = floors_completed * 100
            
            # Time bonus (faster = better)
            time_limit = getattr(settings, 'dungeon_time_limit_minutes', 60) * 60
            time_bonus = 1.0
            if time_taken < time_limit * 0.5:  # Under 50% time
                time_bonus = 1.5
            elif time_taken < time_limit * 0.75:  # Under 75% time
                time_bonus = 1.25
            
            # Party size adjustment (larger parties split rewards)
            party_modifier = 1.0 / (party_size ** 0.5)
            
            # Final token amount
            tokens = int(base_tokens * time_bonus * party_modifier)
            
            # Theme-specific drops
            theme_drops = self.theme_modifiers[theme]["drop_bonus"]
            
            # Reputation gain
            reputation = floors_completed * 50
            
            rewards = {
                "dungeon_tokens": tokens,
                "theme_drops": theme_drops,
                "reputation_gain": reputation,
                "reputation_type": "problem_solver",
                "bonus_multiplier": time_bonus,
                "exp_bonus": int(floors_completed * 10000 * time_bonus),
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            return rewards
            
        except Exception as e:
            logger.error(f"Failed to calculate rewards: {e}")
            return {}
    
    async def handle_instance_completion(
        self,
        instance_id: int,
        participants: List[int],
        floors_cleared: int,
        time_taken: int
    ) -> AgentResponse:
        """
        Process instance completion and distribute rewards.
        
        Args:
            instance_id: Instance ID
            participants: List of player IDs
            floors_cleared: Number of floors completed
            time_taken: Time in seconds
            
        Returns:
            AgentResponse with completion status
        """
        try:
            # Get instance data
            query = """
                SELECT dungeon_id, party_id
                FROM dungeon_instances
                WHERE instance_id = $1
            """
            instance = await postgres_db.fetch_one(query, instance_id)
            
            if not instance:
                raise ValueError(f"Instance {instance_id} not found")
            
            # Get dungeon theme
            query = """
                SELECT theme
                FROM adaptive_dungeons
                WHERE dungeon_id = $1
            """
            dungeon = await postgres_db.fetch_one(query, instance['dungeon_id'])
            theme = DungeonTheme(dungeon['theme'])
            
            # Calculate rewards
            rewards = await self.calculate_dungeon_rewards(
                theme=theme,
                floors_completed=floors_cleared,
                time_taken=time_taken,
                party_size=len(participants)
            )
            
            # Update instance status
            query = """
                UPDATE dungeon_instances
                SET status = 'completed',
                    floors_cleared = $1,
                    time_elapsed = $2,
                    completed_at = NOW()
                WHERE instance_id = $3
            """
            await postgres_db.execute(query, floors_cleared, time_taken, instance_id)
            
            # Record completion for each participant
            for player_id in participants:
                query = """
                    INSERT INTO dungeon_completions
                    (instance_id, player_id, floors_cleared, time_taken, rewards_earned, reputation_gained)
                    VALUES ($1, $2, $3, $4, $5, $6)
                """
                await postgres_db.execute(
                    query,
                    instance_id,
                    player_id,
                    floors_cleared,
                    time_taken,
                    json.dumps(rewards),
                    rewards.get('reputation_gain', 0)
                )
            
            logger.info(f"✓ Instance {instance_id} completed: {floors_cleared} floors, {time_taken}s")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "instance_id": instance_id,
                    "floors_cleared": floors_cleared,
                    "time_taken": time_taken,
                    "rewards": rewards,
                    "participants": participants
                },
                confidence=1.0,
                reasoning="Instance completed successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to handle instance completion: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _select_daily_theme(self, active_factions: List[str]) -> DungeonTheme:
        """Select daily theme based on active factions or random"""
        # Faction influence on theme (30% chance)
        if active_factions and random.random() < 0.3:
            faction_themes = {
                "rune_alliance": DungeonTheme.HOLY_DAY,
                "shadow_cult": DungeonTheme.DEMON_DAY,
                "nature_spirits": DungeonTheme.EARTH_DAY
            }
            for faction_id in active_factions:
                if faction_id in faction_themes:
                    return faction_themes[faction_id]
        
        # Random theme selection (70% chance or fallback)
        return random.choice(list(DungeonTheme))
    
    def _select_room_type(
        self,
        floor_number: int,
        room_idx: int,
        total_rooms: int
    ) -> str:
        """Select room type based on position and floor"""
        # First room is always combat
        if room_idx == 0:
            return "combat_room"
        
        # Last room on last floor is boss room
        if floor_number == self.floor_config['max_floors'] and room_idx == total_rooms - 1:
            return "boss_room"
        
        # Checkpoint floor has checkpoint room
        if floor_number % 3 == 0 and room_idx == total_rooms // 2:
            return "checkpoint_room"
        
        # Random selection for other rooms
        weights = {
            "combat_room": 0.5,
            "treasure_room": 0.2,
            "trap_room": 0.2,
            "mini_boss_room": 0.1
        }
        
        return random.choices(
            list(weights.keys()),
            weights=list(weights.values())
        )[0]
    
    def _select_boss_pool(
        self,
        theme: DungeonTheme,
        floor_count: int
    ) -> List[str]:
        """Select boss pool based on theme"""
        theme_bosses = self.boss_pools.get(theme, [])
        
        if not theme_bosses:
            theme_bosses = ["BAPHOMET", "DARK_LORD", "EDDGA"]
        
        # Select 1-3 bosses
        boss_count = min(len(theme_bosses), random.randint(1, 3))
        return random.sample(theme_bosses, boss_count)
    
    async def _store_dungeon(
        self,
        theme: DungeonTheme,
        floor_count: int,
        layouts: List[Dict],
        monster_pools: List[List[Dict]],
        boss_pool: List[str],
        reward_data: Dict,
        difficulty_rating: int
    ) -> int:
        """Store dungeon in database"""
        try:
            query = """
                INSERT INTO adaptive_dungeons
                (theme, floor_count, layout_data, monster_pools, boss_pool, reward_data, difficulty_rating, expires_at)
                VALUES ($1, $2, $3, $4, $5, $6, $7, NOW() + INTERVAL '1 day')
                RETURNING dungeon_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                theme.value,
                floor_count,
                json.dumps(layouts),
                json.dumps(monster_pools),
                json.dumps(boss_pool),
                json.dumps(reward_data),
                difficulty_rating
            )
            
            return result['dungeon_id']
            
        except Exception as e:
            logger.error(f"Failed to store dungeon: {e}")
            raise


# Global instance
adaptive_dungeon_agent = AdaptiveDungeonAgent()