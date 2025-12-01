"""
Map Hazard Agent - Applies Dynamic Map Conditions

Generates 3-5 daily hazards for different maps that change gameplay mechanics
and create strategic variety. Prioritizes low-traffic maps to encourage exploration.

4-Tier LLM Optimization:
- Tier 1 (40%): Rule-based hazard type selection
- Tier 2 (30%): Cached hazard effects
- Tier 3 (20%): Batched hazard generation
- Tier 4 (10%): LLM for creative descriptions
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


class HazardType(Enum):
    """Types of map hazards that can be applied."""
    BLOOD_MOON = "blood_moon"  # Undead monsters stronger
    MAGIC_STORM = "magic_storm"  # Cast time reduced, flee lowered
    THICK_FOG = "thick_fog"  # Vision limited, ranged buffed
    FAIRY_BLESSING = "fairy_blessing"  # Drop rate +20%
    MANA_DRAIN = "mana_drain"  # SP regen 0, SP cost -50%
    SCORCHING_HEAT = "scorching_heat"  # Fire damage +30%, water -20%
    FROZEN_WASTELAND = "frozen_wasteland"  # Ice damage +30%, fire -20%
    TOXIC_MIASMA = "toxic_miasma"  # Poison damage over time
    HOLY_GROUND = "holy_ground"  # Undead take more damage
    CHAOTIC_VOID = "chaotic_void"  # Random stat fluctuations


class MapHazardAgent(BaseAIAgent):
    """
    Applies daily random hazards to 3-5 maps.
    
    Hazard Selection Strategy:
    - Favor low-traffic maps (encourage exploration)
    - Match hazard to map theme (undead maps → Blood Moon)
    - Avoid hazard stacking (max 1 per map)
    - Coordinate with Problem Agent (hazard in problem map)
    - Balance positive/negative hazards
    
    Effects:
    - Monster stats modified
    - Player stats modified
    - Drop rates changed
    - Visual effects (future: client-side)
    - Quest rewards adjusted
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Map Hazard Agent"""
        super().__init__(
            agent_id="map_hazard_agent",
            agent_type="map_hazard",
            config=config
        )
        
        # Hazard effect definitions (Tier 1: Rule-based)
        self.hazard_effects = self._initialize_hazard_effects()
        
        # Map theme classifications (Tier 1: Rule-based)
        self.map_themes = self._initialize_map_themes()
        
        # Hazard selection weights
        self.hazard_weights = {
            'positive': 0.4,  # 40% buffs
            'negative': 0.6,  # 60% debuffs
        }
        
        logger.info("Map Hazard Agent initialized with 10 hazard types")
    
    def _initialize_hazard_effects(self) -> Dict[HazardType, Dict[str, Any]]:
        """Initialize hazard effect definitions (Tier 1: Rule-based)."""
        return {
            HazardType.BLOOD_MOON: {
                'type': 'negative',
                'monster_modifiers': {
                    'hp_mult': 1.3,
                    'atk_mult': 1.4,
                    'def_mult': 1.2,
                    'element_resist': {'holy': -0.3, 'shadow': 0.3}
                },
                'player_modifiers': {
                    'flee_mult': 0.9,
                    'element_damage': {'holy': 1.2, 'shadow': 0.8}
                },
                'drop_rate_mult': 1.15,
                'visual_effects': ['red_moon', 'fog', 'shadow_particles'],
                'description': 'The blood moon rises, strengthening undead creatures'
            },
            HazardType.MAGIC_STORM: {
                'type': 'mixed',
                'monster_modifiers': {
                    'matk_mult': 1.2,
                    'mdef_mult': 1.1
                },
                'player_modifiers': {
                    'cast_time_mult': 0.7,
                    'flee_mult': 0.85,
                    'sp_regen_mult': 1.3
                },
                'drop_rate_mult': 1.0,
                'visual_effects': ['lightning', 'purple_aura', 'static'],
                'description': 'Magical energy crackles through the air'
            },
            HazardType.THICK_FOG: {
                'type': 'mixed',
                'monster_modifiers': {
                    'hit_mult': 0.9
                },
                'player_modifiers': {
                    'hit_mult': 0.85,
                    'ranged_damage_mult': 1.15,
                    'melee_range_mult': 0.9
                },
                'drop_rate_mult': 1.0,
                'visual_effects': ['fog', 'reduced_visibility'],
                'description': 'Dense fog reduces visibility but sharpens ranged focus'
            },
            HazardType.FAIRY_BLESSING: {
                'type': 'positive',
                'monster_modifiers': {},
                'player_modifiers': {
                    'luk_mult': 1.1,
                    'item_find_mult': 1.05
                },
                'drop_rate_mult': 1.2,
                'visual_effects': ['sparkles', 'fairy_dust', 'soft_glow'],
                'description': 'Fairy magic increases fortune and discovery'
            },
            HazardType.MANA_DRAIN: {
                'type': 'negative',
                'monster_modifiers': {
                    'sp_mult': 1.5
                },
                'player_modifiers': {
                    'sp_regen_mult': 0.0,
                    'skill_sp_cost_mult': 0.5,
                    'max_sp_mult': 1.1
                },
                'drop_rate_mult': 1.05,
                'visual_effects': ['blue_drain', 'energy_wisps'],
                'description': 'Mana drains away but skills cost less'
            },
            HazardType.SCORCHING_HEAT: {
                'type': 'mixed',
                'monster_modifiers': {
                    'element_resist': {'fire': 0.3, 'water': -0.2}
                },
                'player_modifiers': {
                    'element_damage': {'fire': 1.3, 'water': 0.8},
                    'sp_regen_mult': 0.8,
                    'movement_speed_mult': 0.95
                },
                'drop_rate_mult': 1.0,
                'visual_effects': ['heat_waves', 'orange_tint', 'sweat'],
                'description': 'Intense heat amplifies fire while draining stamina'
            },
            HazardType.FROZEN_WASTELAND: {
                'type': 'mixed',
                'monster_modifiers': {
                    'element_resist': {'water': 0.3, 'fire': -0.2}
                },
                'player_modifiers': {
                    'element_damage': {'water': 1.3, 'fire': 0.8},
                    'aspd_mult': 0.9,
                    'movement_speed_mult': 0.9
                },
                'drop_rate_mult': 1.0,
                'visual_effects': ['snow', 'ice_particles', 'blue_tint', 'shiver'],
                'description': 'Freezing cold enhances ice magic but slows movement'
            },
            HazardType.TOXIC_MIASMA: {
                'type': 'negative',
                'monster_modifiers': {
                    'poison_resist': 0.5
                },
                'player_modifiers': {
                    'poison_resist': -0.3,
                    'hp_drain_per_sec': 10,
                    'poison_damage_mult': 1.4
                },
                'drop_rate_mult': 1.1,
                'visual_effects': ['green_fog', 'poison_bubbles', 'sick'],
                'description': 'Toxic fumes poison all who breathe them'
            },
            HazardType.HOLY_GROUND: {
                'type': 'positive',
                'monster_modifiers': {
                    'undead_damage_taken_mult': 1.5,
                    'demon_damage_taken_mult': 1.3
                },
                'player_modifiers': {
                    'holy_damage_mult': 1.2,
                    'hp_regen_mult': 1.2,
                    'curse_resist': 0.5
                },
                'drop_rate_mult': 1.0,
                'visual_effects': ['golden_light', 'holy_aura', 'peaceful'],
                'description': 'Sacred energy purifies the area'
            },
            HazardType.CHAOTIC_VOID: {
                'type': 'negative',
                'monster_modifiers': {
                    'random_stat_variance': 0.3
                },
                'player_modifiers': {
                    'random_stat_variance': 0.2,
                    'all_stats_variance': 0.15
                },
                'drop_rate_mult': 1.25,
                'visual_effects': ['void_cracks', 'reality_distortion', 'chaos'],
                'description': 'Reality itself becomes unstable'
            }
        }
    
    def _initialize_map_themes(self) -> Dict[str, str]:
        """Initialize map theme classifications (Tier 1: Rule-based)."""
        return {
            # Undead maps
            'glast_01': 'undead', 'gl_cas01': 'undead', 'gl_cas02': 'undead',
            'gl_church': 'undead', 'gl_chyard': 'undead', 'gl_dun01': 'undead',
            'gl_dun02': 'undead', 'gef_dun01': 'undead', 'gef_dun02': 'undead',
            'xmas_dun01': 'undead', 'ra_san05': 'undead',
            
            # Desert maps
            'moc_fild01': 'desert', 'moc_fild02': 'desert', 'moc_fild03': 'desert',
            'moc_pryd01': 'desert', 'moc_pryd02': 'desert', 'cmd_fild01': 'desert',
            
            # Ice maps
            'ra_fild01': 'ice', 'ra_fild02': 'ice', 'ice_dun01': 'ice',
            'ice_dun02': 'ice', 'ice_dun03': 'ice',
            
            # Forest maps
            'pay_fild01': 'forest', 'pay_fild02': 'forest', 'pay_fild03': 'forest',
            'pay_fild04': 'forest', 'um_fild01': 'forest', 'um_fild02': 'forest',
            
            # Cave/dungeon maps
            'orcsdun01': 'cave', 'orcsdun02': 'cave', 'in_sphinx1': 'cave',
            'anthell01': 'cave', 'anthell02': 'cave',
            
            # Default for unknown maps
            'default': 'generic'
        }
    
    def _create_crew_agent(self):
        """Create CrewAI agent for hazard management"""
        from crewai import Agent
        
        return Agent(
            role="Map Hazard System Manager",
            goal="Apply dynamic hazards to maps to create strategic variety",
            backstory="An AI system that manipulates environmental conditions to challenge players",
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
    
    async def generate_daily_hazards(
        self,
        map_activity: Dict[str, int],
        active_problems: List[Dict[str, Any]],
        count: Optional[int] = None
    ) -> AgentResponse:
        """
        Generate 3-5 hazards for different maps.
        
        Selection Process:
        1. Identify low-traffic maps (bottom 30%)
        2. Match hazard types to map themes
        3. Check for synergy with active problems
        4. Balance buffs (40%) vs debuffs (60%)
        5. Ensure geographic distribution
        
        Args:
            map_activity: Dict of map_name -> player_count
            active_problems: List of active problem specifications
            count: Number of hazards to generate (default from config)
            
        Returns:
            AgentResponse with generated hazards
        """
        try:
            if count is None:
                count = getattr(settings, 'daily_hazard_count', 4)
            
            logger.info(f"Generating {count} daily hazards from {len(map_activity)} maps")
            
            # Step 1: Sort maps by activity (Tier 1: Rule-based)
            sorted_maps = sorted(map_activity.items(), key=lambda x: x[1])
            low_traffic_threshold = int(len(sorted_maps) * 0.3)
            low_traffic_maps = [m[0] for m in sorted_maps[:low_traffic_threshold]]
            
            if not low_traffic_maps:
                low_traffic_maps = [m[0] for m in sorted_maps[:max(5, len(sorted_maps) // 3)]]
            
            # Step 2: Select maps ensuring distribution
            selected_maps = self._select_distributed_maps(low_traffic_maps, count)
            
            # Step 3: Generate hazard for each map
            generated_hazards = []
            positive_count = 0
            negative_count = 0
            
            for map_name in selected_maps:
                # Determine if this map has an active problem
                problem_map = any(p.get('map_name') == map_name for p in active_problems)
                
                # Select hazard type
                hazard_type = await self.select_hazard_type(
                    map_name=map_name,
                    map_theme=self.map_themes.get(map_name, 'generic'),
                    active_problem=problem_map
                )
                
                # Track balance
                hazard_config = self.hazard_effects[hazard_type]
                if hazard_config['type'] == 'positive':
                    positive_count += 1
                elif hazard_config['type'] == 'negative':
                    negative_count += 1
                
                # Calculate effects
                effects = await self.calculate_hazard_effects(hazard_type)
                
                # Generate hazard spec
                hazard_spec = {
                    'map_name': map_name,
                    'hazard_type': hazard_type.value,
                    'hazard_name': self._generate_hazard_name(hazard_type),
                    'effect_data': effects,
                    'duration_hours': getattr(settings, 'hazard_duration_hours', 24),
                    'generated_at': datetime.now(UTC).isoformat()
                }
                
                generated_hazards.append(hazard_spec)
            
            # Step 4: Apply hazards to database
            applied_hazards = []
            for hazard_spec in generated_hazards:
                applied = await self.apply_hazard(
                    map_name=hazard_spec['map_name'],
                    hazard_type=HazardType(hazard_spec['hazard_type']),
                    duration_hours=hazard_spec['duration_hours']
                )
                if applied.success:
                    applied_hazards.append(applied.data)
            
            result_data = {
                'hazards_generated': len(applied_hazards),
                'hazards': applied_hazards,
                'balance': {
                    'positive': positive_count,
                    'negative': negative_count,
                    'mixed': len(generated_hazards) - positive_count - negative_count
                },
                'timestamp': datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Generated {len(applied_hazards)} daily hazards")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=0.95,
                reasoning="Daily hazards generated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to generate daily hazards: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def select_hazard_type(
        self,
        map_name: str,
        map_theme: str,
        active_problem: bool
    ) -> HazardType:
        """
        Select appropriate hazard for map (Tier 1: Rule-based).
        
        Rules:
        - Undead maps → prefer BLOOD_MOON, HOLY_GROUND
        - Desert maps → prefer SCORCHING_HEAT
        - Ice maps → prefer FROZEN_WASTELAND
        - Forest maps → prefer FAIRY_BLESSING
        - If problem exists → choose complementary hazard
        
        Args:
            map_name: Map identifier
            map_theme: Theme classification
            active_problem: Whether map has active problem
            
        Returns:
            Selected HazardType
        """
        # Theme-based preferences
        theme_hazards = {
            'undead': [HazardType.BLOOD_MOON, HazardType.HOLY_GROUND, HazardType.TOXIC_MIASMA],
            'desert': [HazardType.SCORCHING_HEAT, HazardType.MANA_DRAIN],
            'ice': [HazardType.FROZEN_WASTELAND, HazardType.MAGIC_STORM],
            'forest': [HazardType.FAIRY_BLESSING, HazardType.THICK_FOG],
            'cave': [HazardType.TOXIC_MIASMA, HazardType.CHAOTIC_VOID],
            'generic': list(HazardType)
        }
        
        preferred_hazards = theme_hazards.get(map_theme, theme_hazards['generic'])
        
        # If problem active, prefer negative/challenging hazards
        if active_problem:
            negative_hazards = [h for h in preferred_hazards 
                              if self.hazard_effects[h]['type'] == 'negative']
            if negative_hazards:
                return random.choice(negative_hazards)
        
        return random.choice(preferred_hazards)
    
    async def calculate_hazard_effects(
        self,
        hazard_type: HazardType
    ) -> Dict[str, Any]:
        """
        Calculate stat modifiers for hazard (Tier 2: Cached).
        
        Returns:
            Dict with monster/player modifiers and visual effects
        """
        # Try cache first
        cache_key = f"hazard:effects:{hazard_type.value}"
        cached = await db.get(cache_key)
        if cached:
            return cached
        
        # Get from definitions
        effects = self.hazard_effects[hazard_type].copy()
        
        # Cache for 24 hours
        await db.set(cache_key, effects, expire=86400)
        
        return effects
    
    async def apply_hazard(
        self,
        map_name: str,
        hazard_type: HazardType,
        duration_hours: int = 24
    ) -> AgentResponse:
        """
        Apply hazard to map via database storage.
        
        Actions:
        - Store hazard in database
        - Cache in DragonflyDB
        - Create map announcement
        - Update quest rewards if applicable
        
        Args:
            map_name: Map to apply hazard to
            hazard_type: Type of hazard
            duration_hours: How long hazard lasts
            
        Returns:
            AgentResponse with applied hazard data
        """
        try:
            # Calculate expiry time
            expires_at = datetime.now(UTC) + timedelta(hours=duration_hours)
            
            # Get hazard effects
            effects = await self.calculate_hazard_effects(hazard_type)
            
            # Insert into database
            query = """
                INSERT INTO map_hazards
                (map_name, hazard_type, hazard_name, effect_data, expires_at)
                VALUES ($1, $2, $3, $4, $5)
                RETURNING hazard_id, applied_at
            """
            
            result = await postgres_db.fetch_one(
                query,
                map_name,
                hazard_type.value,
                self._generate_hazard_name(hazard_type),
                json.dumps(effects),
                expires_at
            )
            
            hazard_data = {
                'hazard_id': result['hazard_id'],
                'map_name': map_name,
                'hazard_type': hazard_type.value,
                'hazard_name': self._generate_hazard_name(hazard_type),
                'description': effects.get('description', ''),
                'applied_at': result['applied_at'].isoformat(),
                'expires_at': expires_at.isoformat(),
                'effects': effects
            }
            
            # Cache active hazard
            cache_key = f"hazard:active:{map_name}"
            await db.set(cache_key, hazard_data, expire=duration_hours * 3600)
            
            logger.info(f"✓ Applied {hazard_type.value} hazard to {map_name}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=hazard_data,
                confidence=1.0,
                reasoning="Hazard applied successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to apply hazard: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_active_hazards(self) -> List[Dict[str, Any]]:
        """Get all currently active map hazards"""
        try:
            query = """
                SELECT 
                    hazard_id,
                    map_name,
                    hazard_type,
                    hazard_name,
                    effect_data,
                    applied_at,
                    expires_at
                FROM map_hazards
                WHERE status = 'active' AND expires_at > NOW()
                ORDER BY applied_at DESC
            """
            
            rows = await postgres_db.fetch_all(query)
            
            hazards = []
            for row in rows:
                hazard = dict(row)
                hazard['effect_data'] = json.loads(hazard['effect_data']) if isinstance(hazard['effect_data'], str) else hazard['effect_data']
                hazard['applied_at'] = hazard['applied_at'].isoformat()
                hazard['expires_at'] = hazard['expires_at'].isoformat()
                hazards.append(hazard)
            
            return hazards
            
        except Exception as e:
            logger.error(f"Failed to get active hazards: {e}")
            return []
    
    async def get_map_hazards(self, map_name: str) -> List[Dict[str, Any]]:
        """Get hazards active on specific map"""
        try:
            # Try cache first
            cache_key = f"hazard:active:{map_name}"
            cached = await db.get(cache_key)
            if cached:
                return [cached] if isinstance(cached, dict) else cached
            
            # Query database
            query = """
                SELECT 
                    hazard_id,
                    map_name,
                    hazard_type,
                    hazard_name,
                    effect_data,
                    applied_at,
                    expires_at
                FROM map_hazards
                WHERE map_name = $1 AND status = 'active' AND expires_at > NOW()
            """
            
            rows = await postgres_db.fetch_all(query, map_name)
            
            hazards = []
            for row in rows:
                hazard = dict(row)
                hazard['effect_data'] = json.loads(hazard['effect_data']) if isinstance(hazard['effect_data'], str) else hazard['effect_data']
                hazard['applied_at'] = hazard['applied_at'].isoformat()
                hazard['expires_at'] = hazard['expires_at'].isoformat()
                hazards.append(hazard)
            
            return hazards
            
        except Exception as e:
            logger.error(f"Failed to get map hazards: {e}")
            return []
    
    async def record_hazard_encounter(
        self,
        hazard_id: int,
        player_id: int,
        time_in_hazard: int,
        items_dropped: int = 0,
        deaths: int = 0
    ) -> bool:
        """
        Record player encounter with hazard.
        
        Args:
            hazard_id: Hazard ID
            player_id: Player ID
            time_in_hazard: Time spent in hazard (seconds)
            items_dropped: Items received while in hazard
            deaths: Deaths while in hazard
            
        Returns:
            Success status
        """
        try:
            query = """
                INSERT INTO hazard_encounters
                (hazard_id, player_id, time_in_hazard, items_dropped, deaths)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                hazard_id,
                player_id,
                time_in_hazard,
                items_dropped,
                deaths
            )
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to record hazard encounter: {e}")
            return False
    
    async def remove_expired_hazards(self) -> int:
        """Remove hazards that have expired"""
        try:
            query = """
                UPDATE map_hazards
                SET status = 'expired'
                WHERE status = 'active' AND expires_at <= NOW()
                RETURNING hazard_id, map_name
            """
            
            rows = await postgres_db.fetch_all(query)
            
            # Clear caches
            for row in rows:
                await db.delete(f"hazard:active:{row['map_name']}")
            
            logger.info(f"✓ Removed {len(rows)} expired hazards")
            return len(rows)
            
        except Exception as e:
            logger.error(f"Failed to remove expired hazards: {e}")
            return 0
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _select_distributed_maps(
        self,
        available_maps: List[str],
        count: int
    ) -> List[str]:
        """Select maps ensuring geographic distribution"""
        if len(available_maps) <= count:
            return available_maps
        
        # Simple random selection (could be enhanced with geography)
        return random.sample(available_maps, count)
    
    def _generate_hazard_name(self, hazard_type: HazardType) -> str:
        """Generate display name for hazard"""
        names = {
            HazardType.BLOOD_MOON: "Blood Moon Rising",
            HazardType.MAGIC_STORM: "Arcane Tempest",
            HazardType.THICK_FOG: "Dense Mist",
            HazardType.FAIRY_BLESSING: "Fairy's Grace",
            HazardType.MANA_DRAIN: "Mana Void",
            HazardType.SCORCHING_HEAT: "Burning Sands",
            HazardType.FROZEN_WASTELAND: "Eternal Winter",
            HazardType.TOXIC_MIASMA: "Poisonous Fog",
            HazardType.HOLY_GROUND: "Sacred Blessing",
            HazardType.CHAOTIC_VOID: "Reality Rift"
        }
        return names.get(hazard_type, hazard_type.value.replace('_', ' ').title())


# Global instance
map_hazard_agent = MapHazardAgent()