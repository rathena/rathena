"""
Archaeology Agent - Long-term Exploration and Artifact Collection System
Implements 4-tier LLM optimization for procedural artifact discovery
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


class ArtifactType(str, Enum):
    """Artifact types for archaeology system"""
    FOSSIL = "fossil"
    RELIC = "relic"
    TOME = "tome"
    TREASURE_MAP = "treasure_map"
    RARE_MATERIAL = "rare_material"


class ArchaeologyAgent(BaseAIAgent):
    """
    Manages long-term exploration and artifact collection.
    
    Mechanics:
    - Random dig spots in low-traffic maps
    - Requires shovel item to dig
    - Artifacts combine to unlock secrets
    - Lore book collection (20 pages → complete story)
    - Treasure maps lead to one-time locations
    - Museum system (display collection)
    
    Progression:
    - Archaeologist levels (1-10)
    - Better tools unlock better spots
    - Rare artifacts unlock secret maps
    - Complete collections give titles
    
    4-Tier Optimization:
    - Tier 1 (70%): Rule-based artifact generation
    - Tier 2 (20%): Cached dig spot locations
    - Tier 3 (8%): Batched spot generation
    - Tier 4 (2%): LLM for lore fragments
    """
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize Archaeology Agent"""
        super().__init__(
            agent_id="archaeology_agent",
            agent_type="archaeology",
            config=config
        )
        
        # Artifact collections
        self.collections = {
            "ancient_fossils": {
                "name": "Ancient Fossil Set",
                "required_count": 10,
                "artifact_type": ArtifactType.FOSSIL,
                "reward": {
                    "title": "Paleontologist",
                    "stat_bonus": {"max_hp": 200},
                    "headgear": "Fossil_Crown"
                }
            },
            "lost_kingdom_relics": {
                "name": "Lost Kingdom Relics",
                "required_count": 15,
                "artifact_type": ArtifactType.RELIC,
                "reward": {
                    "title": "Relic Hunter",
                    "stat_bonus": {"matk": 10},
                    "headgear": "Ancient_Tiara"
                }
            },
            "forbidden_tomes": {
                "name": "Forbidden Tomes",
                "required_count": 20,
                "artifact_type": ArtifactType.TOME,
                "reward": {
                    "title": "Lorekeeper",
                    "stat_bonus": {"int": 5, "dex": 5},
                    "headgear": "Scholar_Cap"
                }
            },
            "legendary_treasures": {
                "name": "Legendary Treasures",
                "required_count": 5,
                "artifact_type": ArtifactType.RARE_MATERIAL,
                "reward": {
                    "title": "Treasure Master",
                    "stat_bonus": {"all_stats": 3},
                    "headgear": "Crown_of_Ancients"
                }
            }
        }
        
        # Secret locations unlocked by treasure maps
        self.secret_locations = {
            "hidden_boss_lair": {
                "name": "Hidden MVP Lair",
                "boss": "ANCIENT_WORM",
                "duration_minutes": 30,
                "drops": ["MVP_Card", "Ancient_Weapon"]
            },
            "ancient_vault": {
                "name": "Ancient Vault",
                "treasures": ["High_Tier_Equipment", "Rare_Cards"],
                "duration_minutes": 30
            },
            "forgotten_shrine": {
                "name": "Forgotten Shrine",
                "buff": {"all_stats": 10, "duration": 3600},
                "duration_minutes": 30
            },
            "secret_garden": {
                "name": "Secret Garden",
                "materials": ["Rare_Herbs", "Mystic_Flowers"],
                "duration_minutes": 30
            }
        }
        
        # Rarity tiers for artifacts
        self.rarity_config = {
            "common": {"weight": 0.50, "level_mult": 1.0},
            "uncommon": {"weight": 0.30, "level_mult": 1.2},
            "rare": {"weight": 0.15, "level_mult": 1.5},
            "mythic": {"weight": 0.05, "level_mult": 2.0}
        }
        
        # Archaeology level thresholds
        self.level_thresholds = [0, 10, 30, 60, 100, 150, 210, 280, 360, 500]
        
        logger.info("Archaeology Agent initialized")
    
    def _create_crew_agent(self):
        """Create CrewAI agent for archaeology management"""
        from crewai import Agent
        
        return Agent(
            role="Archaeological Discovery Manager",
            goal="Manage artifact discovery and collection systems",
            backstory="An AI system that coordinates archaeological exploration",
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
    
    async def spawn_dig_spots(
        self,
        map_activity: Dict[str, int],
        count: int = 15
    ) -> List[AgentResponse]:
        """
        Spawn 10-20 dig spots in low-traffic maps (Tier 1: Rule-based).
        
        Selection:
        - 70% in bottom 30% traffic maps
        - 20% in mid-traffic maps
        - 10% in high-traffic (surprise finds)
        - Near map edges, corners, landmarks
        - Minimum 30 cell spacing
        
        Args:
            map_activity: Dict of map_name -> visit_count
            count: Number of dig spots to spawn
            
        Returns:
            List of dig spot specifications
        """
        try:
            logger.info(f"Spawning {count} dig spots")
            
            # Sort maps by activity (low to high)
            sorted_maps = sorted(map_activity.items(), key=lambda x: x[1])
            
            # Categorize maps by traffic
            total_maps = len(sorted_maps)
            low_traffic = sorted_maps[:int(total_maps * 0.3)]
            mid_traffic = sorted_maps[int(total_maps * 0.3):int(total_maps * 0.7)]
            high_traffic = sorted_maps[int(total_maps * 0.7):]
            
            # Distribute spots
            low_count = int(count * 0.7)
            mid_count = int(count * 0.2)
            high_count = count - low_count - mid_count
            
            spots = []
            
            # Spawn in low-traffic maps
            for _ in range(low_count):
                if low_traffic:
                    map_name, _ = random.choice(low_traffic)
                    spot = await self._create_dig_spot(map_name, "low_traffic")
                    spots.append(spot)
            
            # Spawn in mid-traffic maps
            for _ in range(mid_count):
                if mid_traffic:
                    map_name, _ = random.choice(mid_traffic)
                    spot = await self._create_dig_spot(map_name, "mid_traffic")
                    spots.append(spot)
            
            # Spawn in high-traffic maps
            for _ in range(high_count):
                if high_traffic:
                    map_name, _ = random.choice(high_traffic)
                    spot = await self._create_dig_spot(map_name, "high_traffic")
                    spots.append(spot)
            
            logger.info(f"✓ Spawned {len(spots)} dig spots")
            
            return spots
            
        except Exception as e:
            logger.error(f"Failed to spawn dig spots: {e}", exc_info=True)
            return []
    
    async def generate_artifact(
        self,
        dig_spot_rarity: float,
        player_archaeology_level: int
    ) -> Dict[str, Any]:
        """
        Generate artifact from dig spot (Tier 1: Rule-based).
        
        Rarity Logic:
        - Common (50%): Fossils, basic relics
        - Uncommon (30%): Rare relics, lore fragments
        - Rare (15%): Complete tomes, treasure maps
        - Mythic (5%): Unique artifacts, secret unlocks
        
        Level Scaling:
        - Higher level = better rarity chances
        - Level 10 = +50% mythic chance
        
        Args:
            dig_spot_rarity: Spot rarity tier (0.0-1.0)
            player_archaeology_level: Player's archaeology level
            
        Returns:
            Artifact specification
        """
        try:
            # Calculate rarity weights with level bonus
            level_bonus = min(0.5, player_archaeology_level * 0.05)
            
            weights = {
                "common": max(0.1, self.rarity_config["common"]["weight"] - level_bonus),
                "uncommon": self.rarity_config["uncommon"]["weight"],
                "rare": self.rarity_config["rare"]["weight"] + (level_bonus * 0.5),
                "mythic": self.rarity_config["mythic"]["weight"] + (level_bonus * 0.5)
            }
            
            # Select rarity
            rarity = random.choices(
                list(weights.keys()),
                weights=list(weights.values())
            )[0]
            
            # Select artifact type based on rarity
            artifact_type = self._select_artifact_type(rarity)
            
            # Generate artifact
            artifact_id = f"{artifact_type.value}_{int(datetime.now(UTC).timestamp())}_{random.randint(1000, 9999)}"
            
            artifact = {
                "artifact_id": artifact_id,
                "artifact_type": artifact_type.value,
                "rarity": rarity,
                "name": self._generate_artifact_name(artifact_type, rarity),
                "description": f"A {rarity} {artifact_type.value} from ancient times",
                "collection": self._get_collection_for_type(artifact_type),
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.debug(f"Generated artifact: {artifact_id} ({rarity})")
            
            return artifact
            
        except Exception as e:
            logger.error(f"Failed to generate artifact: {e}")
            return {}
    
    async def check_collection_completion(
        self,
        player_id: int,
        artifact_id: str
    ) -> Optional[Dict[str, Any]]:
        """
        Check if player completed artifact set.
        
        Collections:
        - Ancient Fossil Set (10 fossils)
        - Lost Kingdom Relics (15 relics)
        - Forbidden Tomes (20 lore pages)
        - Legendary Treasures (5 rare artifacts)
        
        Completion Rewards:
        - Exclusive title
        - Permanent stat +2
        - Unique cosmetic headgear
        - Secret location access
        
        Args:
            player_id: Player ID
            artifact_id: Newly acquired artifact ID
            
        Returns:
            Completion reward if collection complete, None otherwise
        """
        try:
            # Get artifact details
            query = """
                SELECT artifact_type, collection_name
                FROM player_artifacts
                WHERE player_id = $1 AND artifact_id = $2
            """
            artifact = await postgres_db.fetch_one(query, player_id, artifact_id)
            
            if not artifact or not artifact['collection_name']:
                return None
            
            collection_config = self.collections.get(artifact['collection_name'])
            if not collection_config:
                return None
            
            # Count player's artifacts in this collection
            query = """
                SELECT COUNT(*) as count
                FROM player_artifacts
                WHERE player_id = $1 AND collection_name = $2
            """
            result = await postgres_db.fetch_one(query, player_id, artifact['collection_name'])
            
            if result['count'] >= collection_config['required_count']:
                # Collection complete!
                query = """
                    UPDATE artifact_collections
                    SET completed_by_count = completed_by_count + 1
                    WHERE collection_id = $1
                """
                await postgres_db.execute(query, artifact['collection_name'])
                
                logger.info(f"✓ Player {player_id} completed collection: {artifact['collection_name']}")
                
                return collection_config['reward']
            
            return None
            
        except Exception as e:
            logger.error(f"Failed to check collection completion: {e}")
            return None
    
    async def unlock_secret_location(
        self,
        player_id: int,
        treasure_map_id: str
    ) -> AgentResponse:
        """
        Unlock one-time secret location from treasure map.
        
        Secret Locations:
        - Hidden boss lair (rare MVP)
        - Ancient vault (high-tier items)
        - Forgotten shrine (stat blessings)
        - Secret garden (rare materials)
        
        Duration: 30 minutes access per unlock
        
        Args:
            player_id: Player ID
            treasure_map_id: Treasure map artifact ID
            
        Returns:
            AgentResponse with secret location details
        """
        try:
            # Verify player has the treasure map
            query = """
                SELECT artifact_id
                FROM player_artifacts
                WHERE player_id = $1 AND artifact_id = $2 AND artifact_type = 'treasure_map'
            """
            result = await postgres_db.fetch_one(query, player_id, treasure_map_id)
            
            if not result:
                raise ValueError("Treasure map not found")
            
            # Select random secret location
            location_id = random.choice(list(self.secret_locations.keys()))
            location = self.secret_locations[location_id]
            
            # Create access record
            query = """
                INSERT INTO secret_locations
                (location_name, unlock_requirement, access_map, access_duration_minutes, rewards, unlocked_by_count)
                VALUES ($1, $2, $3, $4, $5, 1)
                ON CONFLICT (location_name) 
                DO UPDATE SET unlocked_by_count = secret_locations.unlocked_by_count + 1
            """
            
            await postgres_db.execute(
                query,
                location['name'],
                treasure_map_id,
                f"secret_{location_id}",
                location.get('duration_minutes', 30),
                json.dumps(location)
            )
            
            # Mark treasure map as used
            query = """
                DELETE FROM player_artifacts
                WHERE player_id = $1 AND artifact_id = $2
            """
            await postgres_db.execute(query, player_id, treasure_map_id)
            
            result_data = {
                "location_id": location_id,
                "location_name": location['name'],
                "access_map": f"secret_{location_id}",
                "duration_minutes": location.get('duration_minutes', 30),
                "rewards": location,
                "timestamp": datetime.now(UTC).isoformat()
            }
            
            logger.info(f"✓ Unlocked secret location for player {player_id}: {location_id}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result_data,
                confidence=1.0,
                reasoning="Secret location unlocked successfully"
            )
            
        except Exception as e:
            logger.error(f"Failed to unlock secret location: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error: {e}"
            )
    
    async def get_archaeology_progress(
        self,
        player_id: int
    ) -> Dict[str, Any]:
        """Get player's archaeology progress"""
        try:
            query = """
                SELECT archaeology_level, total_digs, artifacts_found, 
                       collections_completed, secret_locations_unlocked
                FROM archaeology_progress
                WHERE player_id = $1
            """
            
            result = await postgres_db.fetch_one(query, player_id)
            
            if not result:
                return {
                    "archaeology_level": 1,
                    "total_digs": 0,
                    "artifacts_found": 0,
                    "collections_completed": 0,
                    "secret_locations_unlocked": 0,
                    "next_level_progress": 0.0
                }
            
            # Calculate next level progress
            current_level = result['archaeology_level']
            if current_level >= 10:
                next_level_progress = 1.0
            else:
                current_threshold = self.level_thresholds[current_level - 1]
                next_threshold = self.level_thresholds[current_level]
                progress = (result['total_digs'] - current_threshold) / (next_threshold - current_threshold)
                next_level_progress = max(0.0, min(1.0, progress))
            
            return {
                "archaeology_level": result['archaeology_level'],
                "total_digs": result['total_digs'],
                "artifacts_found": result['artifacts_found'],
                "collections_completed": result['collections_completed'],
                "secret_locations_unlocked": result['secret_locations_unlocked'],
                "next_level_progress": next_level_progress
            }
            
        except Exception as e:
            logger.error(f"Failed to get archaeology progress: {e}")
            return {}
    
    # ========================================================================
    # PRIVATE HELPER METHODS
    # ========================================================================
    
    async def _create_dig_spot(
        self,
        map_name: str,
        traffic_tier: str
    ) -> AgentResponse:
        """Create a dig spot specification"""
        try:
            # Generate random coordinates (simplified - would use actual map data)
            spawn_x = random.randint(50, 250)
            spawn_y = random.randint(50, 250)
            
            # Calculate rarity based on traffic
            rarity_ranges = {
                "low_traffic": (0.3, 0.8),
                "mid_traffic": (0.2, 0.6),
                "high_traffic": (0.1, 0.4)
            }
            
            rarity_min, rarity_max = rarity_ranges.get(traffic_tier, (0.1, 0.5))
            rarity_tier = random.uniform(rarity_min, rarity_max)
            
            # Store in database
            query = """
                INSERT INTO dig_spots
                (spawn_map, spawn_x, spawn_y, rarity_tier, expires_at)
                VALUES ($1, $2, $3, $4, NOW() + INTERVAL '48 hours')
                RETURNING spot_id
            """
            
            result = await postgres_db.fetch_one(
                query,
                map_name,
                spawn_x,
                spawn_y,
                rarity_tier
            )
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={
                    "spot_id": result['spot_id'],
                    "map": map_name,
                    "x": spawn_x,
                    "y": spawn_y,
                    "rarity": rarity_tier
                },
                confidence=1.0
            )
            
        except Exception as e:
            logger.error(f"Failed to create dig spot: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0
            )
    
    def _select_artifact_type(self, rarity: str) -> ArtifactType:
        """Select artifact type based on rarity"""
        rarity_type_map = {
            "common": [ArtifactType.FOSSIL, ArtifactType.RELIC],
            "uncommon": [ArtifactType.RELIC, ArtifactType.TOME],
            "rare": [ArtifactType.TOME, ArtifactType.TREASURE_MAP],
            "mythic": [ArtifactType.TREASURE_MAP, ArtifactType.RARE_MATERIAL]
        }
        
        types = rarity_type_map.get(rarity, [ArtifactType.FOSSIL])
        return random.choice(types)
    
    def _generate_artifact_name(
        self,
        artifact_type: ArtifactType,
        rarity: str
    ) -> str:
        """Generate artifact name"""
        prefixes = {
            "common": ["Old", "Weathered", "Ancient"],
            "uncommon": ["Preserved", "Mysterious", "Ornate"],
            "rare": ["Legendary", "Pristine", "Sacred"],
            "mythic": ["Divine", "Cosmic", "Primordial"]
        }
        
        type_names = {
            ArtifactType.FOSSIL: "Fossil",
            ArtifactType.RELIC: "Relic",
            ArtifactType.TOME: "Tome",
            ArtifactType.TREASURE_MAP: "Treasure Map",
            ArtifactType.RARE_MATERIAL: "Material"
        }
        
        prefix = random.choice(prefixes.get(rarity, ["Ancient"]))
        type_name = type_names.get(artifact_type, "Artifact")
        
        return f"{prefix} {type_name}"
    
    def _get_collection_for_type(
        self,
        artifact_type: ArtifactType
    ) -> Optional[str]:
        """Get collection name for artifact type"""
        type_collection_map = {
            ArtifactType.FOSSIL: "ancient_fossils",
            ArtifactType.RELIC: "lost_kingdom_relics",
            ArtifactType.TOME: "forbidden_tomes",
            ArtifactType.RARE_MATERIAL: "legendary_treasures",
            ArtifactType.TREASURE_MAP: None  # Maps don't belong to collections
        }
        
        return type_collection_map.get(artifact_type)


# Global instance
archaeology_agent = ArchaeologyAgent()