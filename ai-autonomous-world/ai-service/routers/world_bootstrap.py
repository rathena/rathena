"""
World Bootstrap System
Initialize the AI autonomous world with seed NPCs, factions, and economy
"""

from fastapi import APIRouter, HTTPException, status, BackgroundTasks
from loguru import logger
from typing import Optional, List, Dict, Any
from datetime import datetime
import random

from pydantic import BaseModel, Field
from ai_service.routers.npc_spawning import spawn_bulk_npcs, BulkSpawnRequest
from ai_service.database import db, postgres_db

router = APIRouter(prefix="/ai/world/bootstrap", tags=["world-bootstrap"])


class BootstrapConfig(BaseModel):
    """Configuration for world bootstrap"""
    seed_npc_count: int = Field(100, ge=10, le=500, description="Number of seed NPCs to create")
    maps: List[str] = Field(["prontera", "geffen", "payon", "morocc", "alberta"], description="Maps to populate")
    enable_factions: bool = Field(True, description="Create initial factions")
    enable_economy: bool = Field(True, description="Initialize economy system")
    enable_relationships: bool = Field(True, description="Create initial relationships")
    class_distribution: Optional[Dict[str, float]] = Field(None, description="NPC class distribution")
    faction_distribution: Optional[Dict[str, float]] = Field(None, description="Faction distribution")


class BootstrapStatus(BaseModel):
    """Status of bootstrap process"""
    status: str
    phase: str
    progress: float
    npcs_created: int
    factions_created: int
    relationships_created: int
    message: str


class BootstrapResponse(BaseModel):
    """Response from bootstrap"""
    status: str
    total_npcs: int
    total_factions: int
    total_relationships: int
    maps_populated: List[str]
    duration_seconds: float
    message: str


# Default class distribution (based on medieval society)
DEFAULT_CLASS_DISTRIBUTION = {
    "merchant": 0.20,      # 20% merchants
    "guard": 0.15,         # 15% guards
    "farmer": 0.15,        # 15% farmers (not in template, will use merchant)
    "blacksmith": 0.10,    # 10% blacksmiths
    "healer": 0.10,        # 10% healers
    "scholar": 0.08,       # 8% scholars
    "explorer": 0.07,      # 7% explorers
    "bard": 0.07,          # 7% bards
    "fortune_teller": 0.05, # 5% fortune tellers
    "innkeeper": 0.03,     # 3% innkeepers (will use merchant)
}

# Default faction distribution
DEFAULT_FACTION_DISTRIBUTION = {
    "kingdom": 0.30,           # 30% loyal to kingdom
    "merchant_guild": 0.25,    # 25% merchant guild
    "adventurers_guild": 0.15, # 15% adventurers
    "thieves_guild": 0.10,     # 10% thieves
    "scholars_circle": 0.10,   # 10% scholars
    "independent": 0.10,       # 10% independent
}


async def create_initial_factions() -> List[Dict[str, Any]]:
    """Create initial factions for the world"""
    logger.info("Creating initial factions...")
    
    factions = [
        {
            "name": "Kingdom of Rune-Midgarts",
            "faction_id": "kingdom",
            "description": "The ruling monarchy of the land",
            "alignment": "lawful_good",
            "resources": {"gold": 100000, "influence": 80, "military": 90}
        },
        {
            "name": "Merchant Guild",
            "faction_id": "merchant_guild",
            "description": "Association of traders and craftsmen",
            "alignment": "neutral_good",
            "resources": {"gold": 80000, "influence": 70, "trade_routes": 50}
        },
        {
            "name": "Adventurers Guild",
            "faction_id": "adventurers_guild",
            "description": "Organization of explorers and monster hunters",
            "alignment": "chaotic_good",
            "resources": {"gold": 30000, "influence": 50, "combat_power": 70}
        },
        {
            "name": "Thieves Guild",
            "faction_id": "thieves_guild",
            "description": "Underground network of rogues and smugglers",
            "alignment": "chaotic_neutral",
            "resources": {"gold": 40000, "influence": 40, "stealth": 90}
        },
        {
            "name": "Scholars Circle",
            "faction_id": "scholars_circle",
            "description": "Academy of knowledge and magical research",
            "alignment": "lawful_neutral",
            "resources": {"gold": 50000, "influence": 60, "knowledge": 95}
        }
    ]
    
    # Store factions in database
    for faction in factions:
        try:
            # Store in PostgreSQL
            query = """
                INSERT INTO factions (name, description, alignment, metadata)
                VALUES ($1, $2, $3, $4)
                ON CONFLICT (name) DO UPDATE
                SET description = EXCLUDED.description,
                    alignment = EXCLUDED.alignment,
                    metadata = EXCLUDED.metadata
                RETURNING id
            """
            faction_id = await postgres_db.execute(
                query,
                faction["name"],
                faction["description"],
                faction["alignment"],
                {"faction_id": faction["faction_id"], "resources": faction["resources"]}
            )
            logger.info(f"Created faction: {faction['name']} (ID: {faction_id})")
        except Exception as e:
            logger.error(f"Error creating faction {faction['name']}: {e}")
    
    return factions


async def initialize_economy() -> Dict[str, Any]:
    """Initialize economy system with baseline prices and resources"""
    logger.info("Initializing economy system...")
    
    economy_state = {
        "item_prices": {
            "red_potion": 50,
            "blue_potion": 100,
            "white_potion": 500,
            "apple": 15,
            "meat": 30,
            "iron_ore": 100,
            "steel": 500,
            "coal": 50,
            "wood": 30,
            "cloth": 80,
        },
        "supply_demand": {
            "red_potion": {"supply": 1000, "demand": 800},
            "blue_potion": {"supply": 500, "demand": 600},
            "iron_ore": {"supply": 2000, "demand": 1500},
            "food": {"supply": 5000, "demand": 4000},
        },
        "trade_volume": 0.0,
        "inflation_rate": 0.0,
        "timestamp": datetime.utcnow().isoformat()
    }
    
    # Store in database
    await db.set_world_state("economy", economy_state)
    logger.info("Economy initialized with baseline prices")
    
    return economy_state


async def create_initial_relationships(npc_ids: List[str]) -> int:
    """Create initial relationships between NPCs"""
    logger.info(f"Creating initial relationships for {len(npc_ids)} NPCs...")
    
    relationships_created = 0
    
    # Create random relationships (each NPC gets 2-5 relationships)
    for npc_id in npc_ids:
        num_relationships = random.randint(2, 5)
        potential_targets = [n for n in npc_ids if n != npc_id]
        
        if len(potential_targets) < num_relationships:
            continue
        
        targets = random.sample(potential_targets, num_relationships)
        
        for target_id in targets:
            relationship_type = random.choice(["friend", "acquaintance", "rival", "neutral"])
            affinity = random.uniform(-0.5, 0.8) if relationship_type != "rival" else random.uniform(-0.8, -0.2)
            
            try:
                query = """
                    INSERT INTO npc_relationships (npc_id, target_id, target_type, relationship_type, affinity_score)
                    VALUES ($1, $2, 'npc', $3, $4)
                    ON CONFLICT DO NOTHING
                """
                await postgres_db.execute(query, npc_id, target_id, relationship_type, affinity)
                relationships_created += 1
            except Exception as e:
                logger.error(f"Error creating relationship: {e}")
    
    logger.info(f"Created {relationships_created} initial relationships")
    return relationships_created


@router.post("/start", response_model=BootstrapResponse)
async def start_bootstrap(config: BootstrapConfig, background_tasks: BackgroundTasks):
    """
    Start world bootstrap process
    
    Creates seed NPCs, factions, economy, and relationships to initialize the autonomous world.
    This is a one-time setup process that should be run when first deploying the system.
    """
    try:
        start_time = datetime.utcnow()
        logger.info("=" * 80)
        logger.info("STARTING WORLD BOOTSTRAP")
        logger.info(f"Seed NPCs: {config.seed_npc_count}")
        logger.info(f"Maps: {', '.join(config.maps)}")
        logger.info("=" * 80)
        
        total_npcs = 0
        total_factions = 0
        total_relationships = 0
        npc_ids = []
        
        # Phase 1: Create Factions
        if config.enable_factions:
            logger.info("Phase 1: Creating factions...")
            factions = await create_initial_factions()
            total_factions = len(factions)
            logger.info(f"✓ Created {total_factions} factions")
        
        # Phase 2: Initialize Economy
        if config.enable_economy:
            logger.info("Phase 2: Initializing economy...")
            await initialize_economy()
            logger.info("✓ Economy initialized")
        
        # Phase 3: Spawn Seed NPCs
        logger.info(f"Phase 3: Spawning {config.seed_npc_count} seed NPCs...")
        
        # Distribute NPCs across maps
        npcs_per_map = config.seed_npc_count // len(config.maps)
        remainder = config.seed_npc_count % len(config.maps)
        
        for i, map_name in enumerate(config.maps):
            count = npcs_per_map + (1 if i < remainder else 0)
            
            logger.info(f"Spawning {count} NPCs on {map_name}...")
            
            # Prepare class distribution
            class_dist = config.class_distribution or DEFAULT_CLASS_DISTRIBUTION
            npc_classes = []
            for npc_class, probability in class_dist.items():
                num_of_class = int(count * probability)
                npc_classes.extend([npc_class] * num_of_class)
            
            # Fill remaining slots
            while len(npc_classes) < count:
                npc_classes.append(random.choice(list(class_dist.keys())))
            
            # Spawn NPCs
            spawn_request = BulkSpawnRequest(
                count=count,
                map=map_name,
                npc_classes=npc_classes,
                faction_distribution=config.faction_distribution or DEFAULT_FACTION_DISTRIBUTION,
                level_range=(1, 50)
            )
            
            result = await spawn_bulk_npcs(spawn_request)
            total_npcs += result.spawned_count
            npc_ids.extend([npc["npc_id"] for npc in result.npcs])
            
            logger.info(f"✓ Spawned {result.spawned_count} NPCs on {map_name}")
        
        # Phase 4: Create Initial Relationships
        if config.enable_relationships and len(npc_ids) > 1:
            logger.info("Phase 4: Creating initial relationships...")
            total_relationships = await create_initial_relationships(npc_ids)
            logger.info(f"✓ Created {total_relationships} relationships")
        
        # Calculate duration
        end_time = datetime.utcnow()
        duration = (end_time - start_time).total_seconds()
        
        logger.info("=" * 80)
        logger.info("WORLD BOOTSTRAP COMPLETE")
        logger.info(f"Total NPCs: {total_npcs}")
        logger.info(f"Total Factions: {total_factions}")
        logger.info(f"Total Relationships: {total_relationships}")
        logger.info(f"Duration: {duration:.2f} seconds")
        logger.info("=" * 80)
        
        return BootstrapResponse(
            status="success",
            total_npcs=total_npcs,
            total_factions=total_factions,
            total_relationships=total_relationships,
            maps_populated=config.maps,
            duration_seconds=duration,
            message=f"World bootstrap complete! Created {total_npcs} NPCs, {total_factions} factions, and {total_relationships} relationships."
        )
        
    except Exception as e:
        logger.error(f"Error during world bootstrap: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"World bootstrap failed: {str(e)}"
        )


@router.get("/status", response_model=BootstrapStatus)
async def get_bootstrap_status():
    """Get current bootstrap status"""
    # This would track ongoing bootstrap process
    # For now, return simple status
    return BootstrapStatus(
        status="idle",
        phase="none",
        progress=0.0,
        npcs_created=0,
        factions_created=0,
        relationships_created=0,
        message="No bootstrap process running"
    )

