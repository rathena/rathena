"""
Dynamic NPC Agent - Spawns Rare Roaming NPCs
Implements heatmap-based spawning in low-traffic maps with unique personalities
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
    NPCType, NPCStatus, NPCPersonalityData, NPCQuestData, NPCRewardData
)


class DynamicNPCAgent(BaseAIAgent):
    """
    Spawns daily roaming NPCs in unexplored/low-traffic maps.
    
    NPC Types:
    - Treasure Seeker: Find hidden chests quest
    - Runaway Sage: Teaches temporary passive skill
    - Wandering Blacksmith: One-time safe refine
    - Mysterious Child: Rare riddle quest
    - Lost Merchant: Exclusive items
    
    Features:
    - Heatmap-based spawn location (low-traffic areas)
    - Unique AI-generated personalities
    - Higher tier rewards than Problem Agent
    - Daily despawn at end of day
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Dynamic NPC Agent"""
        super().__init__(
            agent_id="dynamic_npc_agent",
            agent_type="dynamic_npc",
            config=config
        )
        
        # NPC type configurations
        self.npc_templates = {
            NPCType.TREASURE_SEEKER: {
                "quest_type": "treasure_hunt",
                "personality_traits": ["adventurous", "greedy", "friendly"],
                "speech_patterns": ["excited", "enthusiastic"],
                "required_level": 30,
                "reward_tier": "medium"
            },
            NPCType.RUNAWAY_SAGE: {
                "quest_type": "knowledge_sharing",
                "personality_traits": ["wise", "mysterious", "aloof"],
                "speech_patterns": ["cryptic", "philosophical"],
                "required_level": 50,
                "reward_tier": "high"
            },
            NPCType.WANDERING_BLACKSMITH: {
                "quest_type": "crafting_service",
                "personality_traits": ["gruff", "skilled", "proud"],
                "speech_patterns": ["blunt", "professional"],
                "required_level": 40,
                "reward_tier": "high"
            },
            NPCType.MYSTERIOUS_CHILD: {
                "quest_type": "riddle",
                "personality_traits": ["innocent", "playful", "cryptic"],
                "speech_patterns": ["childlike", "enigmatic"],
                "required_level": 25,
                "reward_tier": "very_high"
            },
            NPCType.LOST_MERCHANT: {
                "quest_type": "delivery",
                "personality_traits": ["worried", "grateful", "wealthy"],
                "speech_patterns": ["polite", "business-like"],
                "required_level": 35,
                "reward_tier": "medium"
            }
        }
        
        # Reward tier multipliers
        self.reward_tiers = {
            "medium": {"exp_mult": 2.0, "zeny_mult": 1.5, "rare_chance": 0.05},
            "high": {"exp_mult": 3.5, "zeny_mult": 2.5, "rare_chance": 0.15},
            "very_high": {"exp_mult": 5.0, "zeny_mult": 4.0, "rare_chance": 0.30}
        }
        
        # Spawn cooldown tracking
        self.spawn_cooldowns: Dict[str, datetime] = {}
        
        logger.info(f"Dynamic NPC Agent initialized with {len(self.npc_templates)} NPC types")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for NPC generation"""
        from crewai import Agent
        
        return Agent(
            role="Dynamic NPC Creator",
            goal="Create unique roaming NPCs with distinct personalities",
            backstory="An AI that breathes life into wandering characters",
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
    
    async def spawn_daily_npcs(
        self,
        map_activity: Dict[str, int],
        heatmap_data: Optional[Dict[str, List[Dict[str, int]]]] = None,
        count: int = 3
    ) -> List[AgentResponse]:
        """
        Spawn 1-5 roaming NPCs in low-traffic maps.
        
        Selection Criteria:
        - Choose maps with <10% of peak traffic
        - Prefer corners and dead ends (heatmap-based)
        - Avoid consecutive spawns in same map
        - Weight by map "interestingness" (landmarks, scenery)
        
        Args:
            map_activity: Map name -> player visit count (last 24h)
            heatmap_data: Detailed position heatmap per map
            count: Number of NPCs to spawn (1-5)
            
        Returns:
            List of AgentResponse for each spawned NPC
        """
        try:
            logger.info(f"Spawning {count} dynamic NPCs...")
            
            # Validate count
            count = max(1, min(5, count))
            
            # Get currently active NPCs
            active_npcs = await self._get_active_npcs()
            if len(active_npcs) >= 5:
                logger.info(f"Maximum active NPCs reached ({len(active_npcs)}/5)")
                return []
            
            # Calculate spawn count (don't exceed 5 total)
            spawn_count = min(count, 5 - len(active_npcs))
            
            # Select low-traffic maps
            low_traffic_maps = self._identify_low_traffic_maps(map_activity)
            
            if not low_traffic_maps:
                logger.warning("No low-traffic maps found for NPC spawning")
                return []
            
            # Spawn NPCs
            responses = []
            spawned_maps = {npc['spawn_map'] for npc in active_npcs}
            
            for i in range(spawn_count):
                # Select NPC type
                npc_type = self._select_npc_type()
                
                # Select spawn location
                spawn_map = self._select_spawn_map(
                    low_traffic_maps,
                    spawned_maps,
                    heatmap_data
                )
                
                if not spawn_map:
                    logger.warning(f"Could not find suitable spawn location for NPC #{i+1}")
                    continue
                
                # Generate NPC
                npc_response = await self._generate_npc(
                    npc_type,
                    spawn_map,
                    heatmap_data
                )
                
                if npc_response.success:
                    responses.append(npc_response)
                    spawned_maps.add(spawn_map)
                    logger.info(f"✓ Spawned {npc_type} in {spawn_map}")
                else:
                    logger.warning(f"Failed to spawn NPC: {npc_response.reasoning}")
            
            logger.info(f"✓ Spawned {len(responses)}/{spawn_count} dynamic NPCs")
            return responses
            
        except Exception as e:
            logger.error(f"Failed to spawn daily NPCs: {e}", exc_info=True)
            return []
    
    async def handle_npc_interaction(
        self,
        npc_id: int,
        player_id: int,
        interaction_type: str,
        player_message: Optional[str] = None
    ) -> AgentResponse:
        """
        Handle player-NPC interaction.
        
        Args:
            npc_id: NPC ID
            player_id: Player ID
            interaction_type: Type of interaction (talk, quest_start, quest_complete)
            player_message: Optional player message for dialogue
            
        Returns:
            AgentResponse with NPC response and any rewards
        """
        try:
            # Get NPC data
            npc = await self._get_npc(npc_id)
            if not npc:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "NPC not found"},
                    confidence=0.0,
                    reasoning="NPC does not exist or has despawned"
                )
            
            # Check if NPC is still active
            if npc['status'] != NPCStatus.ACTIVE.value:
                return AgentResponse(
                    agent_type=self.agent_type,
                    success=False,
                    data={"error": "NPC is not active"},
                    confidence=0.0,
                    reasoning=f"NPC status: {npc['status']}"
                )
            
            # Get dialogue history
            dialogue_history = await self._get_dialogue_history(npc_id, player_id)
            
            # Generate NPC response
            npc_response = await self._generate_dialogue_response(
                npc,
                player_id,
                interaction_type,
                player_message,
                dialogue_history
            )
            
            # Check if quest completed
            rewards = None
            if interaction_type == "quest_complete":
                rewards = await self._grant_rewards(npc_id, player_id, npc)
            
            # Record interaction
            await self._record_interaction(
                npc_id,
                player_id,
                interaction_type,
                dialogue_history,
                rewards
            )
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "npc_response": npc_response,
                    "dialogue_history": dialogue_history,
                    "rewards": rewards
                },
                confidence=0.9,
                reasoning="Interaction processed successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to handle NPC interaction: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_active_npcs(self) -> List[Dict[str, Any]]:
        """Get all currently active dynamic NPCs"""
        return await self._get_active_npcs()
    
    async def despawn_expired_npcs(self) -> int:
        """
        Despawn NPCs that have expired.
        
        Returns:
            Number of NPCs despawned
        """
        try:
            query = """
                UPDATE dynamic_npcs 
                SET status = $1
                WHERE status = 'active' AND despawns_at < NOW()
                RETURNING npc_id
            """
            
            despawned = await postgres_db.fetch_all(query, NPCStatus.DESPAWNED.value)
            count = len(despawned)
            
            if count > 0:
                # Clear cache
                await db.delete("npcs:active")
                logger.info(f"✓ Despawned {count} expired NPCs")
            
            return count
            
        except Exception as e:
            logger.error(f"Failed to despawn expired NPCs: {e}")
            return 0
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    def _identify_low_traffic_maps(
        self,
        map_activity: Dict[str, int]
    ) -> List[Tuple[str, int]]:
        """Identify maps with low player traffic (<10% of peak)"""
        if not map_activity:
            # Default low-traffic maps
            return [
                ("mjo_dun01", 0),
                ("anthell01", 0),
                ("cmd_fild01", 0),
                ("prt_fild08", 0)
            ]
        
        # Calculate threshold (10% of peak traffic)
        peak_traffic = max(map_activity.values())
        threshold = peak_traffic * 0.1
        
        # Filter low-traffic maps
        low_traffic = [
            (map_name, count)
            for map_name, count in map_activity.items()
            if count <= threshold and count >= 0
        ]
        
        # Sort by traffic (lowest first)
        low_traffic.sort(key=lambda x: x[1])
        
        return low_traffic[:20]  # Top 20 lowest traffic maps
    
    def _select_npc_type(self) -> str:
        """Select NPC type with weighted random selection"""
        # Avoid recently spawned types
        recent_types = [npc['npc_type'] for npc in self.spawn_cooldowns.keys()]
        
        # Calculate weights
        available_types = list(self.npc_templates.keys())
        weights = [
            0.5 if npc_type in recent_types else 1.0
            for npc_type in available_types
        ]
        
        return random.choices(available_types, weights=weights, k=1)[0]
    
    def _select_spawn_map(
        self,
        low_traffic_maps: List[Tuple[str, int]],
        already_spawned: set,
        heatmap_data: Optional[Dict[str, List[Dict[str, int]]]]
    ) -> Optional[str]:
        """
        Select best spawn map avoiding already used maps.
        
        Preference:
        1. Not in already_spawned
        2. Lowest traffic
        3. Has interesting coordinates (corners, edges) from heatmap
        """
        candidates = [
            (map_name, traffic)
            for map_name, traffic in low_traffic_maps
            if map_name not in already_spawned
        ]
        
        if not candidates:
            return None
        
        # If heatmap available, score maps by "interestingness"
        if heatmap_data:
            scored_candidates = []
            for map_name, traffic in candidates:
                map_heatmap = heatmap_data.get(map_name, [])
                interest_score = self._calculate_map_interest(map_heatmap)
                scored_candidates.append((map_name, traffic, interest_score))
            
            # Sort by interest (high) then traffic (low)
            scored_candidates.sort(key=lambda x: (-x[2], x[1]))
            return scored_candidates[0][0]
        
        # Default: return lowest traffic map
        return candidates[0][0]
    
    def _calculate_map_interest(
        self,
        heatmap: List[Dict[str, int]]
    ) -> float:
        """Calculate map interestingness from heatmap (corners/edges preferred)"""
        if not heatmap:
            return 0.5
        
        # Maps with diverse hotspots are more interesting
        unique_positions = len(heatmap)
        total_visits = sum(pos['count'] for pos in heatmap)
        
        if total_visits == 0:
            return 1.0  # Empty maps are very interesting for rare NPCs
        
        # Calculate diversity (entropy-like measure)
        diversity = unique_positions / (total_visits + 1)
        
        return min(1.0, diversity * 10)
    
    async def _generate_npc(
        self,
        npc_type: str,
        spawn_map: str,
        heatmap_data: Optional[Dict[str, List[Dict[str, int]]]]
    ) -> AgentResponse:
        """Generate complete NPC with personality and quest"""
        try:
            template = self.npc_templates[npc_type]
            
            # Generate spawn coordinates
            spawn_x, spawn_y = self._generate_spawn_coords(spawn_map, heatmap_data)
            
            # Generate personality
            personality = await self.generate_npc_personality(npc_type)
            
            # Generate quest
            quest_data = self._generate_quest_data(npc_type, template)
            
            # Calculate rewards
            rewards = await self.scale_rewards(npc_type, 50)  # Default level 50
            
            # Calculate despawn time (end of day)
            now = datetime.now(UTC)
            end_of_day = now.replace(hour=23, minute=59, second=59)
            if end_of_day <= now:
                end_of_day += timedelta(days=1)
            
            # Store in database
            npc_id = await self._store_npc(
                npc_type,
                personality,
                spawn_map,
                spawn_x,
                spawn_y,
                quest_data,
                rewards,
                end_of_day
            )
            
            # Cache in DragonflyDB
            await self._cache_npc(npc_id, {
                "npc_type": npc_type,
                "npc_name": personality['name'],
                "spawn_map": spawn_map,
                "spawn_x": spawn_x,
                "spawn_y": spawn_y,
                "personality_data": personality,
                "quest_data": quest_data,
                "reward_data": rewards,
                "status": NPCStatus.ACTIVE.value,
                "spawned_at": now,
                "despawns_at": end_of_day
            })
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "npc_id": npc_id,
                    "npc_type": npc_type,
                    "npc_name": personality['name'],
                    "spawn_map": spawn_map,
                    "spawn_x": spawn_x,
                    "spawn_y": spawn_y
                },
                confidence=0.9,
                reasoning="NPC generated successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to generate NPC: {e}", exc_info=True)
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    def _generate_spawn_coords(
        self,
        spawn_map: str,
        heatmap_data: Optional[Dict[str, List[Dict[str, int]]]]
    ) -> Tuple[int, int]:
        """Generate spawn coordinates preferring low-traffic areas"""
        if heatmap_data and spawn_map in heatmap_data:
            map_heatmap = heatmap_data[spawn_map]
            
            # Find least visited areas
            if map_heatmap:
                sorted_positions = sorted(map_heatmap, key=lambda p: p['count'])
                # Pick from bottom 20%
                low_traffic_positions = sorted_positions[:max(1, len(sorted_positions) // 5)]
                pos = random.choice(low_traffic_positions)
                return pos['x'], pos['y']
        
        # Default random coordinates
        return random.randint(50, 300), random.randint(50, 300)
    
    async def generate_npc_personality(
        self,
        npc_type: str
    ) -> Dict[str, Any]:
        """
        Generate unique NPC personality using LLM or templates.
        
        Returns personality data with:
        - name, traits, backstory, speech_patterns, quirks
        """
        template = self.npc_templates[npc_type]
        
        # Generate name
        name = self._generate_npc_name(npc_type)
        
        # In production, this would use LLM for creative generation
        # For now, use template-based generation
        backstory = self._generate_backstory(npc_type, name)
        quirks = self._generate_quirks(template['personality_traits'])
        
        return {
            "name": name,
            "personality_traits": template['personality_traits'],
            "backstory": backstory,
            "speech_patterns": template['speech_patterns'],
            "quirks": quirks
        }
    
    def _generate_npc_name(self, npc_type: str) -> str:
        """Generate NPC name based on type"""
        name_prefixes = {
            NPCType.TREASURE_SEEKER: ["Magnus", "Thalia", "Drake", "Luna"],
            NPCType.RUNAWAY_SAGE: ["Eldrin", "Mystara", "Zorvan", "Althea"],
            NPCType.WANDERING_BLACKSMITH: ["Grom", "Thorne", "Brigid", "Steelheart"],
            NPCType.MYSTERIOUS_CHILD: ["Aria", "Finn", "Lyra", "Orpheus"],
            NPCType.LOST_MERCHANT: ["Aldric", "Mira", "Cornelius", "Beatrice"]
        }
        
        names = name_prefixes.get(npc_type, ["Wanderer"])
        return random.choice(names)
    
    def _generate_backstory(self, npc_type: str, name: str) -> str:
        """Generate simple backstory"""
        stories = {
            NPCType.TREASURE_SEEKER: f"{name} seeks legendary treasures across the realm.",
            NPCType.RUNAWAY_SAGE: f"{name} fled from rigid teachings to find true wisdom.",
            NPCType.WANDERING_BLACKSMITH: f"{name} travels to perfect their legendary craft.",
            NPCType.MYSTERIOUS_CHILD: f"{name} appears in strange places with cryptic knowledge.",
            NPCType.LOST_MERCHANT: f"{name} got separated from their caravan."
        }
        
        return stories.get(npc_type, f"{name} wanders the world.")
    
    def _generate_quirks(self, traits: List[str]) -> List[str]:
        """Generate quirks based on personality traits"""
        quirk_pool = [
            "Hums while thinking",
            "Always carries a lucky charm",
            "Speaks in riddles occasionally",
            "Has a mysterious scar",
            "Collects rare stones",
            "Never forgets a face",
            "Quotes ancient proverbs"
        ]
        
        return random.sample(quirk_pool, k=min(3, len(quirk_pool)))
    
    def _generate_quest_data(
        self,
        npc_type: str,
        template: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Generate quest data for NPC"""
        quest_type = template['quest_type']
        
        quests = {
            "treasure_hunt": {
                "objectives": ["Find the hidden treasure chest", "Return to the NPC"],
                "hints": ["Look in quiet corners", "Treasures hide where few walk"]
            },
            "knowledge_sharing": {
                "objectives": ["Listen to the sage's wisdom", "Answer a philosophical question"],
                "hints": ["Wisdom comes from experience", "Think deeply"]
            },
            "crafting_service": {
                "objectives": ["Bring materials for refining", "Receive enhanced equipment"],
                "hints": ["Materials determine success", "Trust the master's skill"]
            },
            "riddle": {
                "objectives": ["Solve the child's riddle", "Claim your reward"],
                "hints": ["The answer is simpler than you think", "Listen carefully"]
            },
            "delivery": {
                "objectives": ["Deliver package to destination", "Return for reward"],
                "hints": ["Time is of the essence", "Be careful with the package"]
            }
        }
        
        return quests.get(quest_type, {"objectives": ["Talk to NPC"], "hints": []})
    
    async def scale_rewards(
        self,
        npc_type: str,
        player_level: int
    ) -> Dict[str, Any]:
        """
        Calculate rewards (higher tier than Problem Agent).
        
        Rewards include:
        - Slot addition stones (rare)
        - Unique cards (0.01-1% chance)
        - +1 permanent stat (very rare)
        - Untradable cosmetics
        - Temporary rare buffs
        """
        template = self.npc_templates[npc_type]
        reward_tier = template['reward_tier']
        multipliers = self.reward_tiers[reward_tier]
        
        # Base rewards
        base_exp = player_level * 2000
        base_zeny = player_level * 1000
        
        exp_reward = int(base_exp * multipliers['exp_mult'])
        zeny_reward = int(base_zeny * multipliers['zeny_mult'])
        
        # Item rewards
        items = []
        items.append({"item_id": 607, "amount": 3, "name": "Yggdrasil Berry"})
        
        # Rare items based on tier
        rare_items = []
        rare_chance = multipliers['rare_chance']
        
        if random.random() < rare_chance:
            rare_items.append({"item_id": 7179, "amount": 1, "name": "Kiel-D-01 Card"})
        
        if random.random() < rare_chance * 0.3:
            rare_items.append({"item_id": 4001, "amount": 1, "name": "Poring Card"})
        
        # Special buffs
        special_buffs = []
        if reward_tier in ["high", "very_high"]:
            special_buffs.append("ATK +10% for 1 hour")
            special_buffs.append("MATK +10% for 1 hour")
        
        # Permanent stat (very rare)
        permanent_stat = None
        if reward_tier == "very_high" and random.random() < 0.01:
            stats = ["STR", "AGI", "VIT", "INT", "DEX", "LUK"]
            permanent_stat = {random.choice(stats): 1}
        
        return {
            "exp": exp_reward,
            "zeny": zeny_reward,
            "items": items,
            "rare_items": rare_items if rare_items else None,
            "special_buffs": special_buffs if special_buffs else None,
            "permanent_stat": permanent_stat
        }
    
    async def _generate_dialogue_response(
        self,
        npc: Dict[str, Any],
        player_id: int,
        interaction_type: str,
        player_message: Optional[str],
        dialogue_history: List[Dict[str, str]]
    ) -> str:
        """Generate NPC dialogue response"""
        # In production, this would use DialogueAgent with personality context
        # For now, return simple responses
        
        personality = npc.get('personality_data', {})
        npc_name = personality.get('name', 'Wanderer')
        
        responses = {
            "talk": f"Greetings, traveler! I am {npc_name}. How may I assist you?",
            "quest_start": f"Ah, you wish to help? Excellent! {personality.get('backstory', '')}",
            "quest_complete": f"Well done! You have proven yourself worthy. Here is your reward."
        }
        
        return responses.get(interaction_type, f"{npc_name} nods at you.")
    
    async def _grant_rewards(
        self,
        npc_id: int,
        player_id: int,
        npc: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Grant rewards to player"""
        reward_data = npc.get('reward_data', {})
        
        # In production, this would dispatch to Bridge Layer to give rewards in-game
        logger.info(f"Granting rewards to player {player_id}: {reward_data}")
        
        return reward_data
    
    async def _store_npc(
        self,
        npc_type: str,
        personality: Dict[str, Any],
        spawn_map: str,
        spawn_x: int,
        spawn_y: int,
        quest_data: Dict[str, Any],
        rewards: Dict[str, Any],
        despawns_at: datetime
    ) -> int:
        """Store NPC in PostgreSQL"""
        query = """
            INSERT INTO dynamic_npcs 
            (npc_type, npc_name, personality_data, spawn_map, spawn_x, spawn_y, 
             quest_data, reward_data, despawns_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9)
            RETURNING npc_id
        """
        
        result = await postgres_db.fetch_one(
            query,
            npc_type,
            personality['name'],
            json.dumps(personality),
            spawn_map,
            spawn_x,
            spawn_y,
            json.dumps(quest_data),
            json.dumps(rewards),
            despawns_at
        )
        
        return result['npc_id']
    
    async def _cache_npc(self, npc_id: int, npc_data: Dict[str, Any]):
        """Cache NPC in DragonflyDB"""
        try:
            # Cache specific NPC
            await db.set(f"npc:{npc_id}", npc_data, expire=86400)
            
            # Invalidate active NPCs list
            await db.delete("npcs:active")
            
        except Exception as e:
            logger.warning(f"Failed to cache NPC: {e}")
    
    async def _get_npc(self, npc_id: int) -> Optional[Dict[str, Any]]:
        """Get NPC from cache or database"""
        try:
            # Try cache first
            cached = await db.get(f"npc:{npc_id}")
            if cached:
                return cached
            
            # Query database
            query = "SELECT * FROM dynamic_npcs WHERE npc_id = $1"
            npc = await postgres_db.fetch_one(query, npc_id)
            
            if npc:
                npc_dict = dict(npc)
                # Parse JSON fields
                if npc_dict.get('personality_data'):
                    npc_dict['personality_data'] = json.loads(npc_dict['personality_data'])
                if npc_dict.get('quest_data'):
                    npc_dict['quest_data'] = json.loads(npc_dict['quest_data'])
                if npc_dict.get('reward_data'):
                    npc_dict['reward_data'] = json.loads(npc_dict['reward_data'])
                
                # Cache for future requests
                await db.set(f"npc:{npc_id}", npc_dict, expire=3600)
                
                return npc_dict
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to get NPC: {e}")
            return None
    
    async def _get_active_npcs(self) -> List[Dict[str, Any]]:
        """Get all active NPCs"""
        try:
            # Try cache first
            cached = await db.get("npcs:active")
            if cached:
                return cached
            
            # Query database
            query = """
                SELECT * FROM dynamic_npcs
                WHERE status = 'active' AND despawns_at > NOW()
                ORDER BY spawned_at DESC
            """
            
            npcs = await postgres_db.fetch_all(query)
            
            result = []
            for npc in npcs:
                npc_dict = dict(npc)
                # Parse JSON fields
                if npc_dict.get('personality_data'):
                    npc_dict['personality_data'] = json.loads(npc_dict['personality_data'])
                if npc_dict.get('quest_data'):
                    npc_dict['quest_data'] = json.loads(npc_dict['quest_data'])
                if npc_dict.get('reward_data'):
                    npc_dict['reward_data'] = json.loads(npc_dict['reward_data'])
                result.append(npc_dict)
            
            # Cache for 5 minutes
            await db.set("npcs:active", result, expire=300)
            
            return result
            
        except Exception as e:
            logger.error(f"Failed to get active NPCs: {e}")
            return []
    
    async def _get_dialogue_history(
        self,
        npc_id: int,
        player_id: int
    ) -> List[Dict[str, str]]:
        """Get dialogue history between player and NPC"""
        try:
            query = """
                SELECT dialogue_history 
                FROM npc_interactions
                WHERE npc_id = $1 AND player_id = $2
                ORDER BY timestamp DESC
                LIMIT 1
            """
            
            result = await postgres_db.fetch_one(query, npc_id, player_id)
            
            if result and result['dialogue_history']:
                return json.loads(result['dialogue_history'])
            
            return []
            
        except Exception as e:
            logger.debug(f"No dialogue history found: {e}")
            return []
    
    async def _record_interaction(
        self,
        npc_id: int,
        player_id: int,
        interaction_type: str,
        dialogue_history: List[Dict[str, str]],
        rewards: Optional[Dict[str, Any]]
    ):
        """Record NPC interaction"""
        try:
            query = """
                INSERT INTO npc_interactions 
                (npc_id, player_id, interaction_type, dialogue_history, reward_given)
                VALUES ($1, $2, $3, $4, $5)
            """
            
            await postgres_db.execute(
                query,
                npc_id,
                player_id,
                interaction_type,
                json.dumps(dialogue_history),
                json.dumps(rewards) if rewards else None
            )
            
            # Update interaction count
            await postgres_db.execute(
                "UPDATE dynamic_npcs SET interaction_count = interaction_count + 1 WHERE npc_id = $1",
                npc_id
            )
            
        except Exception as e:
            logger.error(f"Failed to record interaction: {e}")


# Global instance
dynamic_npc_agent = DynamicNPCAgent()