"""
Dynamic Boss Agent - Spawns Adaptive Difficulty Mini-Bosses
Spawns bosses that scale based on player activity and world state
"""

import asyncio
import json
import random
from datetime import datetime, timedelta, UTC
from typing import Dict, List, Optional, Any
from loguru import logger

from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from database import postgres_db, db
from config import settings


class BossSpawnReason:
    """Boss spawn reasons"""
    ANTI_FARM = "anti_farm"                    # Too much farming in area
    TREASURE_GUARD = "treasure_guard"          # Empty map treasure
    PROBLEM_ESCALATION = "problem_escalation"  # Problem evolved
    FACTION_CHAMPION = "faction_champion"      # Faction warfare
    RANDOM_ENCOUNTER = "random_encounter"      # Random high-reward encounter


class DynamicBossAgent(BaseAIAgent):
    """
    Spawns adaptive mini-bosses that scale to player activity.
    
    Boss Types:
    - Anti-Farm Boss: Punish excessive farming
    - Treasure Guardian: Protect valuable spawns
    - Problem Escalation: Evolved world problem
    - Faction Champion: Faction warfare elite
    - Wandering Terror: Random high-reward encounter
    
    Scaling Factors:
    - Average party level in map
    - Recent player deaths
    - Time since last boss kill
    - Faction alignment strength
    - Problem difficulty
    
    4-Tier Optimization:
    - Tier 1: Rule-based spawn conditions
    - Tier 2: Cached boss templates
    - Tier 3: Batched boss generation
    - Tier 4: LLM for unique boss names/skills
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Dynamic Boss Agent"""
        super().__init__(
            agent_id="dynamic_boss_agent",
            agent_type="dynamic_boss",
            config=config
        )
        
        # Boss templates by spawn reason
        self.boss_templates = {
            BossSpawnReason.ANTI_FARM: {
                "base_mob_id": 1900,
                "name_template": "Enraged {monster}",
                "hp_multiplier": 3.0,
                "atk_multiplier": 1.5,
                "def_multiplier": 1.2,
                "skills": ["Area Attack", "Regeneration", "Rage Mode"],
                "min_difficulty": 3
            },
            BossSpawnReason.TREASURE_GUARD: {
                "base_mob_id": 1901,
                "name_template": "{monster} Guardian",
                "hp_multiplier": 2.5,
                "atk_multiplier": 1.8,
                "def_multiplier": 1.5,
                "skills": ["Treasure Shield", "Area Lockdown", "Counter Strike"],
                "min_difficulty": 4
            },
            BossSpawnReason.PROBLEM_ESCALATION: {
                "base_mob_id": 1902,
                "name_template": "Corrupted {monster}",
                "hp_multiplier": 4.0,
                "atk_multiplier": 2.0,
                "def_multiplier": 1.3,
                "skills": ["Dark Aura", "Mass Curse", "Summon Minions"],
                "min_difficulty": 5
            },
            BossSpawnReason.FACTION_CHAMPION: {
                "base_mob_id": 1903,
                "name_template": "{faction} Champion",
                "hp_multiplier": 3.5,
                "atk_multiplier": 2.2,
                "def_multiplier": 1.6,
                "skills": ["Faction Buff", "Rally Cry", "Ultimate Strike"],
                "min_difficulty": 6
            },
            BossSpawnReason.RANDOM_ENCOUNTER: {
                "base_mob_id": 1904,
                "name_template": "Wandering {monster}",
                "hp_multiplier": 5.0,
                "atk_multiplier": 2.5,
                "def_multiplier": 2.0,
                "skills": ["Chaos Strike", "Teleport", "World Shattering"],
                "min_difficulty": 7
            }
        }
        
        # Scaling curves
        self.scaling_curves = {
            "hp_per_level": 1.1,
            "atk_per_level": 1.15,
            "def_per_level": 1.08
        }
        
        # Wipe adjustment thresholds
        self.wipe_thresholds = {
            3: 0.80,  # 20% stat reduction after 3 wipes
            5: 0.65   # 35% stat reduction after 5 wipes
        }
        
        # Spawn conditions
        self.anti_farm_threshold = getattr(settings, 'boss_anti_farm_threshold', 100)  # kills per hour
        self.random_spawn_chance = getattr(settings, 'boss_random_spawn_chance', 0.05)  # 5% daily
        
        logger.info("Dynamic Boss Agent initialized with 5 boss types")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for boss management"""
        from crewai import Agent
        
        return Agent(
            role="Dynamic Boss Manager",
            goal="Spawn adaptive mini-bosses that challenge players",
            backstory="An AI system that creates dynamic boss encounters",
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
    
    async def evaluate_spawn_conditions(
        self,
        world_state: Dict[str, Any]
    ) -> Optional[Dict[str, Any]]:
        """
        Evaluate if conditions warrant boss spawn.
        
        Conditions:
        - Map farm activity > threshold
        - Low-traffic map with treasure
        - Active world problem escalating
        - Faction conflict threshold
        - Random chance (5% daily)
        
        Args:
            world_state: Current world state
            
        Returns:
            Spawn decision or None
        """
        try:
            logger.debug("Evaluating boss spawn conditions")
            
            spawn_decisions = []
            
            # Check anti-farm condition
            map_activity = world_state.get('map_activity', {})
            monster_kills = world_state.get('monster_kills', {})
            
            for map_name, kill_count in monster_kills.items():
                if kill_count > self.anti_farm_threshold:
                    spawn_decisions.append({
                        "reason": BossSpawnReason.ANTI_FARM,
                        "map": map_name,
                        "difficulty_modifier": 1.0 + (kill_count / self.anti_farm_threshold - 1) * 0.2,
                        "priority": 3
                    })
            
            # Check random encounter (5% chance daily)
            if random.random() < self.random_spawn_chance:
                # Select random active map
                if map_activity:
                    random_map = random.choice(list(map_activity.keys()))
                    spawn_decisions.append({
                        "reason": BossSpawnReason.RANDOM_ENCOUNTER,
                        "map": random_map,
                        "difficulty_modifier": random.uniform(1.2, 1.8),
                        "priority": 1
                    })
            
            # Check problem escalation
            # (Would query problem_agent for active problems)
            
            # Check faction conflict
            # (Would query faction_agent for high-tension factions)
            
            # Select highest priority spawn
            if spawn_decisions:
                spawn_decisions.sort(key=lambda x: x['priority'], reverse=True)
                selected = spawn_decisions[0]
                
                logger.info(f"✓ Boss spawn condition met: {selected['reason']} in {selected['map']}")
                return selected
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to evaluate spawn conditions: {e}")
            return None
    
    async def generate_boss_spec(
        self,
        spawn_reason: str,
        spawn_map: str,
        difficulty_modifier: float = 1.0
    ) -> Dict[str, Any]:
        """
        Generate complete boss specification.
        
        Boss Spec includes:
        - Monster ID (base template)
        - Scaled HP/ATK/DEF
        - Special skills (LLM-generated or template)
        - Spawn location
        - Reward multipliers
        - Despawn conditions
        
        Args:
            spawn_reason: Reason for boss spawn
            spawn_map: Map to spawn on
            difficulty_modifier: Additional difficulty scaling
            
        Returns:
            Complete boss specification
        """
        try:
            # Get template
            template = self.boss_templates.get(spawn_reason, self.boss_templates[BossSpawnReason.RANDOM_ENCOUNTER])
            
            # Get map player levels (simulated - would query actual game state)
            avg_level = 50  # Would be from world_state
            player_levels = [avg_level] * 5  # Simulated party
            
            # Calculate base stats (simplified)
            base_hp = 10000 * avg_level
            base_atk = 100 * avg_level
            base_def = 50 * avg_level
            
            # Apply template multipliers
            base_stats = {
                "hp": int(base_hp * template['hp_multiplier']),
                "atk": int(base_atk * template['atk_multiplier']),
                "def": int(base_def * template['def_multiplier'])
            }
            
            # Scale stats
            scaled_stats = await self.calculate_boss_scaling(
                base_stats,
                player_levels,
                recent_wipes=0  # Would query from encounter history
            )
            
            # Apply difficulty modifier
            for stat in scaled_stats:
                scaled_stats[stat] = int(scaled_stats[stat] * difficulty_modifier)
            
            # Generate boss name (Tier 1: Template-based)
            boss_name = template['name_template'].format(
                monster="Terror",
                faction="Shadow"
            )
            
            # Select spawn coordinates (simplified)
            spawn_x = random.randint(100, 300)
            spawn_y = random.randint(100, 300)
            
            # Calculate rewards
            difficulty_rating = int(template['min_difficulty'] * difficulty_modifier)
            rewards = await self.generate_boss_rewards(
                difficulty_rating,
                participants=5  # Estimated
            )
            
            boss_spec = {
                "boss_type": spawn_reason,
                "boss_name": boss_name,
                "spawn_reason": spawn_reason,
                "spawn_map": spawn_map,
                "spawn_x": spawn_x,
                "spawn_y": spawn_y,
                "base_stats": base_stats,
                "scaled_stats": scaled_stats,
                "difficulty_rating": difficulty_rating,
                "skills": template['skills'],
                "reward_multiplier": difficulty_modifier,
                "rewards": rewards,
                "status": "active",
                "despawn_conditions": {
                    "timeout_minutes": 30,
                    "no_combat_minutes": 10
                }
            }
            
            logger.info(f"✓ Generated boss spec: {boss_name} ({spawn_reason})")
            return boss_spec
            
        except Exception as e:
            logger.error(f"Failed to generate boss spec: {e}", exc_info=True)
            return {}
    
    async def calculate_boss_scaling(
        self,
        base_stats: Dict[str, int],
        player_levels: List[int],
        recent_wipes: int = 0
    ) -> Dict[str, int]:
        """
        Calculate scaled boss stats.
        
        Scaling formula:
        - HP = base * (1 + 0.1 * avg_level)
        - ATK = base * (1 + 0.15 * avg_level)
        - DEF = base * (1 + 0.08 * avg_level)
        
        Wipe adjustment:
        - If wipes > 3: reduce stats by 20%
        - If wipes > 5: reduce stats by 35%
        
        Args:
            base_stats: Base boss stats
            player_levels: List of player levels in area
            recent_wipes: Number of recent wipes
            
        Returns:
            Scaled stats
        """
        try:
            avg_level = sum(player_levels) / len(player_levels) if player_levels else 50
            
            # Apply level scaling
            scaled_stats = {
                "hp": int(base_stats["hp"] * (1 + self.scaling_curves['hp_per_level'] * (avg_level / 100))),
                "atk": int(base_stats["atk"] * (1 + self.scaling_curves['atk_per_level'] * (avg_level / 100))),
                "def": int(base_stats["def"] * (1 + self.scaling_curves['def_per_level'] * (avg_level / 100)))
            }
            
            # Apply wipe adjustment (make easier after failures)
            wipe_multiplier = 1.0
            for threshold, multiplier in self.wipe_thresholds.items():
                if recent_wipes >= threshold:
                    wipe_multiplier = multiplier
            
            if wipe_multiplier < 1.0:
                for stat in scaled_stats:
                    scaled_stats[stat] = int(scaled_stats[stat] * wipe_multiplier)
                logger.info(f"Applied wipe adjustment: {wipe_multiplier}x (after {recent_wipes} wipes)")
            
            return scaled_stats
            
        except Exception as e:
            logger.error(f"Failed to calculate boss scaling: {e}")
            return base_stats
    
    async def generate_boss_rewards(
        self,
        boss_difficulty: int,
        participants: int
    ) -> Dict[str, Any]:
        """
        Generate adaptive rewards.
        
        Reward types:
        - Rare refine materials
        - 0.1-3% chance for rare costume
        - Randomized special effects (ATK+3~5%)
        - Treasure boxes
        - Reputation points (all participants)
        
        Args:
            boss_difficulty: Boss difficulty rating (1-10)
            participants: Number of players participating
            
        Returns:
            Reward structure
        """
        try:
            rewards = {
                "exp": boss_difficulty * 5000 * participants,
                "zeny": boss_difficulty * 2000 * participants,
                "items": [],
                "reputation": boss_difficulty * 20,
                "special_drops": []
            }
            
            # Refine materials (guaranteed)
            if boss_difficulty >= 3:
                rewards["items"].append({
                    "item_id": 6224,  # Bradium
                    "amount": boss_difficulty // 2,
                    "name": "Bradium"
                })
            
            if boss_difficulty >= 5:
                rewards["items"].append({
                    "item_id": 6225,  # Carnium
                    "amount": boss_difficulty // 3,
                    "name": "Carnium"
                })
            
            # Rare costume chance
            costume_chance = 0.001 * boss_difficulty  # 0.1% to 1.0%
            if random.random() < costume_chance:
                rewards["special_drops"].append({
                    "type": "costume",
                    "item_id": 19500,  # Custom costume
                    "name": "Boss Vanquisher Aura",
                    "rarity": "legendary"
                })
            
            # Special effect items
            if boss_difficulty >= 7:
                effect_value = random.randint(3, 5)
                rewards["special_drops"].append({
                    "type": "enchantment",
                    "name": f"Boss Essence (+{effect_value}% ATK)",
                    "effect": {"atk_percent": effect_value}
                })
            
            # Treasure boxes
            if boss_difficulty >= 4:
                rewards["items"].append({
                    "item_id": 617,  # Old Violet Box
                    "amount": 1,
                    "name": "Old Violet Box"
                })
            
            return rewards
            
        except Exception as e:
            logger.error(f"Failed to generate boss rewards: {e}")
            return {"exp": 0, "zeny": 0, "items": [], "reputation": 0, "special_drops": []}
    
    async def spawn_boss(
        self,
        boss_spec: Dict[str, Any]
    ) -> AgentResponse:
        """
        Spawn a dynamic boss.
        
        Args:
            boss_spec: Complete boss specification
            
        Returns:
            AgentResponse with spawned boss details
        """
        try:
            # Store boss in database
            query = """
                INSERT INTO dynamic_bosses
                (boss_type, boss_name, spawn_reason, spawn_map, spawn_x, spawn_y,
                 base_stats, scaled_stats, difficulty_rating, reward_multiplier, status)
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11)
                RETURNING boss_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                boss_spec['boss_type'],
                boss_spec['boss_name'],
                boss_spec['spawn_reason'],
                boss_spec['spawn_map'],
                boss_spec['spawn_x'],
                boss_spec['spawn_y'],
                json.dumps(boss_spec['base_stats']),
                json.dumps(boss_spec['scaled_stats']),
                boss_spec['difficulty_rating'],
                boss_spec['reward_multiplier'],
                boss_spec['status']
            )
            
            boss_id = result['boss_id']
            boss_spec['boss_id'] = boss_id
            
            # Cache boss
            await db.set(f"boss:{boss_id}", boss_spec, expire=7200)  # 2 hours
            
            # Add to active bosses set
            await db.sadd("bosses:active", boss_id)
            
            logger.info(f"✓ Spawned dynamic boss #{boss_id}: {boss_spec['boss_name']}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=boss_spec,
                confidence=0.95,
                reasoning="Boss spawned successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to spawn boss: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def record_boss_encounter(
        self,
        boss_id: int,
        player_id: int,
        damage_dealt: int = 0,
        deaths: int = 0,
        time_participated: int = 0
    ) -> bool:
        """
        Record player participation in boss fight.
        
        Args:
            boss_id: Boss ID
            player_id: Player ID
            damage_dealt: Total damage dealt
            deaths: Number of deaths
            time_participated: Time in combat (seconds)
            
        Returns:
            Success status
        """
        try:
            query = """
                INSERT INTO boss_encounters
                (boss_id, player_id, damage_dealt, deaths, time_participated)
                VALUES ($1, $2, $3, $4, $5)
                ON CONFLICT (boss_id, player_id)
                DO UPDATE SET
                    damage_dealt = boss_encounters.damage_dealt + $3,
                    deaths = boss_encounters.deaths + $4,
                    time_participated = boss_encounters.time_participated + $5
            """
            
            await postgres_db.execute(query, boss_id, player_id, damage_dealt, deaths, time_participated)
            
            logger.debug(f"Recorded boss encounter: player {player_id}, boss {boss_id}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to record boss encounter: {e}")
            return False
    
    async def handle_boss_defeat(
        self,
        boss_id: int,
        participants: List[int],
        time_to_kill: int
    ) -> AgentResponse:
        """
        Process boss defeat.
        
        Actions:
        - Distribute rewards
        - Award reputation
        - Update boss history
        - Adjust difficulty for next spawn
        - Trigger celebration effect
        
        Args:
            boss_id: Boss ID
            participants: List of participating player IDs
            time_to_kill: Time taken to defeat (seconds)
            
        Returns:
            AgentResponse with rewards
        """
        try:
            # Get boss data
            boss_data = await db.get(f"boss:{boss_id}")
            if not boss_data:
                query = "SELECT * FROM dynamic_bosses WHERE boss_id = $1"
                boss_row = await postgres_db.fetch_one(query, boss_id)
                if boss_row:
                    boss_data = dict(boss_row)
                else:
                    return AgentResponse(
                        agent_type=self.agent_type,
                        success=False,
                        data={"error": "Boss not found"},
                        confidence=0.0,
                        reasoning="Boss not found"
                    )
            
            # Mark boss as defeated
            query = """
                UPDATE dynamic_bosses
                SET status = 'defeated', defeated_at = NOW()
                WHERE boss_id = $1
            """
            await postgres_db.execute(query, boss_id)
            
            # Record in history
            query = """
                INSERT INTO boss_spawn_history
                (boss_type, spawn_reason, spawn_map, difficulty_rating,
                 participants_count, time_to_defeat, was_defeated)
                VALUES ($1, $2, $3, $4, $5, $6, TRUE)
            """
            await postgres_db.execute(
                query,
                boss_data.get('boss_type'),
                boss_data.get('spawn_reason'),
                boss_data.get('spawn_map'),
                boss_data.get('difficulty_rating'),
                len(participants),
                time_to_kill
            )
            
            # Distribute rewards
            rewards_data = boss_data.get('rewards', {})
            
            for player_id in participants:
                # Record rewards (would trigger Bridge Layer to grant items)
                query = """
                    UPDATE boss_encounters
                    SET rewards_earned = $1
                    WHERE boss_id = $2 AND player_id = $3
                """
                await postgres_db.execute(query, json.dumps(rewards_data), boss_id, player_id)
            
            # Clear cache
            await db.delete(f"boss:{boss_id}")
            await db.srem("bosses:active", boss_id)
            
            result_data = {
                "boss_id": boss_id,
                "boss_name": boss_data.get('boss_name'),
                "participants": participants,
                "time_to_kill": time_to_kill,
                "rewards": rewards_data,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Boss #{boss_id} defeated by {len(participants)} players in {time_to_kill}s")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Boss defeated, rewards distributed"
            )
            
        except Exception as e:
            logger.error(f"Failed to handle boss defeat: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_active_bosses(self) -> List[Dict[str, Any]]:
        """Get all currently active dynamic bosses"""
        try:
            query = """
                SELECT *
                FROM dynamic_bosses
                WHERE status = 'active'
                ORDER BY spawned_at DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            bosses = []
            for row in rows:
                boss_data = dict(row)
                # Parse JSON fields
                if boss_data.get('base_stats'):
                    boss_data['base_stats'] = json.loads(boss_data['base_stats'])
                if boss_data.get('scaled_stats'):
                    boss_data['scaled_stats'] = json.loads(boss_data['scaled_stats'])
                bosses.append(boss_data)
            
            return bosses
            
        except Exception as e:
            logger.error(f"Failed to get active bosses: {e}")
            return []


# Global instance
dynamic_boss_agent = DynamicBossAgent()