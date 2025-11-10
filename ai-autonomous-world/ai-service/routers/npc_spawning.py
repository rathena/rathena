"""
NPC Auto-Spawning System
Procedural NPC generation with personality and role assignment
"""

from fastapi import APIRouter, HTTPException, status, Query
from loguru import logger
from typing import Optional, List, Dict, Any
from datetime import datetime
import random
import uuid

from pydantic import BaseModel, Field
from ai_service.models.npc import NPCRegisterRequest, NPCPosition, NPCPersonality
from ai_service.database import db

router = APIRouter(prefix="/ai/npc/spawn", tags=["npc-spawning"])


class NPCSpawnRequest(BaseModel):
    """Request to spawn a new NPC"""
    map: str = Field(..., description="Map to spawn NPC on")
    x: Optional[int] = Field(None, description="X coordinate (random if not specified)")
    y: Optional[int] = Field(None, description="Y coordinate (random if not specified)")
    npc_class: Optional[str] = Field(None, description="NPC class (random if not specified)")
    personality: Optional[NPCPersonality] = Field(None, description="Personality traits (random if not specified)")
    level: Optional[int] = Field(None, ge=1, le=999, description="NPC level (random if not specified)")
    faction_id: Optional[str] = Field(None, description="Faction ID")


class NPCSpawnResponse(BaseModel):
    """Response from NPC spawn"""
    status: str
    npc_id: str
    name: str
    npc_class: str
    position: NPCPosition
    personality: NPCPersonality
    message: str


class BulkSpawnRequest(BaseModel):
    """Request to spawn multiple NPCs"""
    count: int = Field(..., ge=1, le=100, description="Number of NPCs to spawn")
    map: str = Field(..., description="Map to spawn NPCs on")
    npc_classes: Optional[List[str]] = Field(None, description="List of NPC classes to use (random if not specified)")
    faction_distribution: Optional[Dict[str, float]] = Field(None, description="Faction distribution (faction_id: probability)")
    level_range: Optional[tuple[int, int]] = Field((1, 50), description="Level range for spawned NPCs")


class BulkSpawnResponse(BaseModel):
    """Response from bulk spawn"""
    status: str
    spawned_count: int
    npcs: List[Dict[str, Any]]
    message: str


# NPC class templates with typical personality ranges
NPC_CLASS_TEMPLATES = {
    "merchant": {
        "names": ["Marcus", "Elena", "Tobias", "Lydia", "Gregor", "Mira", "Aldric", "Sasha"],
        "personality_ranges": {
            "openness": (0.4, 0.7),
            "conscientiousness": (0.7, 0.95),
            "extraversion": (0.5, 0.8),
            "agreeableness": (0.3, 0.7),
            "neuroticism": (0.3, 0.6)
        },
        "alignments": ["lawful_neutral", "neutral_good", "neutral_evil", "true_neutral"],
        "goals": ["Maximize profit", "Build wealth", "Expand business", "Maintain reputation"]
    },
    "guard": {
        "names": ["Thorne", "Gareth", "Brienne", "Roland", "Cassandra", "Viktor", "Aria", "Magnus"],
        "personality_ranges": {
            "openness": (0.2, 0.5),
            "conscientiousness": (0.8, 0.98),
            "extraversion": (0.3, 0.6),
            "agreeableness": (0.2, 0.6),
            "neuroticism": (0.4, 0.8)
        },
        "alignments": ["lawful_good", "lawful_neutral", "lawful_evil"],
        "goals": ["Maintain order", "Protect citizens", "Catch criminals", "Enforce law"]
    },
    "scholar": {
        "names": ["Elara", "Aldous", "Seraphina", "Cornelius", "Isolde", "Thaddeus", "Celeste", "Ambrose"],
        "personality_ranges": {
            "openness": (0.8, 0.98),
            "conscientiousness": (0.7, 0.95),
            "extraversion": (0.3, 0.6),
            "agreeableness": (0.6, 0.8),
            "neuroticism": (0.2, 0.5)
        },
        "alignments": ["lawful_good", "neutral_good", "lawful_neutral"],
        "goals": ["Preserve knowledge", "Teach others", "Research", "Discover truth"]
    },
    "healer": {
        "names": ["Mira", "Cedric", "Elise", "Alaric", "Rosalind", "Benedict", "Gwendolyn", "Lucian"],
        "personality_ranges": {
            "openness": (0.5, 0.7),
            "conscientiousness": (0.6, 0.8),
            "extraversion": (0.2, 0.6),
            "agreeableness": (0.8, 0.98),
            "neuroticism": (0.4, 0.9)
        },
        "alignments": ["neutral_good", "lawful_good", "chaotic_good"],
        "goals": ["Help the wounded", "Ease suffering", "Preserve life", "Find peace"]
    },
    "explorer": {
        "names": ["Lyra", "Drake", "Aria", "Finn", "Zara", "Kael", "Nova", "Orion"],
        "personality_ranges": {
            "openness": (0.85, 0.98),
            "conscientiousness": (0.3, 0.6),
            "extraversion": (0.7, 0.95),
            "agreeableness": (0.6, 0.85),
            "neuroticism": (0.1, 0.4)
        },
        "alignments": ["chaotic_good", "neutral_good", "chaotic_neutral"],
        "goals": ["Explore new areas", "Find treasures", "Meet new people", "Seek adventure"]
    },
    "blacksmith": {
        "names": ["Grom", "Thorin", "Brunhilde", "Ragnar", "Helga", "Bjorn", "Astrid", "Gunnar"],
        "personality_ranges": {
            "openness": (0.2, 0.5),
            "conscientiousness": (0.9, 0.99),
            "extraversion": (0.1, 0.4),
            "agreeableness": (0.4, 0.7),
            "neuroticism": (0.1, 0.3)
        },
        "alignments": ["lawful_neutral", "lawful_good", "true_neutral"],
        "goals": ["Perfect craft", "Maintain quality", "Work in peace", "Build reputation"]
    },
    "bard": {
        "names": ["Finn", "Melody", "Lysander", "Calliope", "Orpheus", "Aria", "Tristan", "Lyric"],
        "personality_ranges": {
            "openness": (0.85, 0.98),
            "conscientiousness": (0.3, 0.6),
            "extraversion": (0.85, 0.98),
            "agreeableness": (0.6, 0.85),
            "neuroticism": (0.1, 0.4)
        },
        "alignments": ["chaotic_good", "neutral_good", "chaotic_neutral"],
        "goals": ["Entertain crowds", "Collect stories", "Spread joy", "Gain fame"]
    },
    "fortune_teller": {
        "names": ["Zara", "Morgana", "Cassandra", "Nostradamus", "Sibyl", "Oracle", "Mystic", "Seer"],
        "personality_ranges": {
            "openness": (0.9, 0.99),
            "conscientiousness": (0.5, 0.7),
            "extraversion": (0.4, 0.7),
            "agreeableness": (0.5, 0.7),
            "neuroticism": (0.4, 0.7)
        },
        "alignments": ["true_neutral", "chaotic_neutral", "neutral_good"],
        "goals": ["Understand fate", "Guide seekers", "Uncover mysteries", "See the future"]
    }
}


def generate_random_personality(npc_class: str) -> NPCPersonality:
    """Generate random personality based on NPC class template"""
    template = NPC_CLASS_TEMPLATES.get(npc_class, NPC_CLASS_TEMPLATES["merchant"])
    ranges = template["personality_ranges"]
    
    return NPCPersonality(
        openness=round(random.uniform(*ranges["openness"]), 2),
        conscientiousness=round(random.uniform(*ranges["conscientiousness"]), 2),
        extraversion=round(random.uniform(*ranges["extraversion"]), 2),
        agreeableness=round(random.uniform(*ranges["agreeableness"]), 2),
        neuroticism=round(random.uniform(*ranges["neuroticism"]), 2),
        moral_alignment=random.choice(template["alignments"])
    )


def generate_random_name(npc_class: str, used_names: set) -> str:
    """Generate random name from class template, avoiding duplicates"""
    template = NPC_CLASS_TEMPLATES.get(npc_class, NPC_CLASS_TEMPLATES["merchant"])
    available_names = [n for n in template["names"] if n not in used_names]

    if not available_names:
        # Generate unique name with suffix
        base_name = random.choice(template["names"])
        suffix = random.randint(1, 999)
        return f"{base_name}{suffix}"

    return random.choice(available_names)


def generate_random_goals(npc_class: str) -> List[str]:
    """Generate random goals from class template"""
    template = NPC_CLASS_TEMPLATES.get(npc_class, NPC_CLASS_TEMPLATES["merchant"])
    num_goals = random.randint(2, 4)
    return random.sample(template["goals"], min(num_goals, len(template["goals"])))


@router.post("/single", response_model=NPCSpawnResponse)
async def spawn_single_npc(request: NPCSpawnRequest):
    """
    Spawn a single NPC with procedural generation

    - **map**: Map to spawn on (required)
    - **x, y**: Coordinates (random if not specified)
    - **npc_class**: NPC class (random if not specified)
    - **personality**: Personality traits (random if not specified)
    - **level**: NPC level (random 1-50 if not specified)
    """
    try:
        logger.info(f"Spawning single NPC on map: {request.map}")

        # Generate random class if not specified
        npc_class = request.npc_class or random.choice(list(NPC_CLASS_TEMPLATES.keys()))

        # Generate random personality if not specified
        personality = request.personality or generate_random_personality(npc_class)

        # Generate random level if not specified
        level = request.level or random.randint(1, 50)

        # Generate random position if not specified
        x = request.x or random.randint(100, 200)
        y = request.y or random.randint(100, 200)

        # Generate unique NPC ID
        npc_id = f"ai_{npc_class}_{uuid.uuid4().hex[:8]}"

        # Generate name
        name = generate_random_name(npc_class, set())

        # Generate goals
        goals = generate_random_goals(npc_class)

        # Create registration request
        reg_request = NPCRegisterRequest(
            npc_id=npc_id,
            name=name,
            npc_class=npc_class,
            level=level,
            position=NPCPosition(map=request.map, x=x, y=y),
            personality=personality,
            faction_id=request.faction_id,
            initial_goals=goals
        )

        # Register NPC with AI service
        from routers.npc import register_npc
        reg_response = await register_npc(reg_request)

        logger.info(f"NPC spawned successfully: {npc_id} ({name})")

        return NPCSpawnResponse(
            status="success",
            npc_id=npc_id,
            name=name,
            npc_class=npc_class,
            position=NPCPosition(map=request.map, x=x, y=y),
            personality=personality,
            message=f"NPC {name} ({npc_class}) spawned successfully at {request.map} ({x}, {y})"
        )

    except Exception as e:
        logger.error(f"Error spawning NPC: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to spawn NPC: {str(e)}"
        )


@router.post("/bulk", response_model=BulkSpawnResponse)
async def spawn_bulk_npcs(request: BulkSpawnRequest):
    """
    Spawn multiple NPCs with procedural generation

    - **count**: Number of NPCs to spawn (1-100)
    - **map**: Map to spawn on
    - **npc_classes**: List of classes to use (random if not specified)
    - **faction_distribution**: Faction distribution (optional)
    - **level_range**: Level range for spawned NPCs
    """
    try:
        logger.info(f"Spawning {request.count} NPCs on map: {request.map}")

        spawned_npcs = []
        used_names = set()

        # Determine NPC classes to use
        if request.npc_classes:
            class_pool = request.npc_classes
        else:
            class_pool = list(NPC_CLASS_TEMPLATES.keys())

        for i in range(request.count):
            # Select random class
            npc_class = random.choice(class_pool)

            # Generate personality
            personality = generate_random_personality(npc_class)

            # Generate level
            level = random.randint(*request.level_range)

            # Generate position (spread out across map)
            x = random.randint(100, 200)
            y = random.randint(100, 200)

            # Generate unique NPC ID
            npc_id = f"ai_{npc_class}_{uuid.uuid4().hex[:8]}"

            # Generate name
            name = generate_random_name(npc_class, used_names)
            used_names.add(name)

            # Generate goals
            goals = generate_random_goals(npc_class)

            # Determine faction
            faction_id = None
            if request.faction_distribution:
                # Weighted random selection
                factions = list(request.faction_distribution.keys())
                weights = list(request.faction_distribution.values())
                faction_id = random.choices(factions, weights=weights, k=1)[0]

            # Create registration request
            reg_request = NPCRegisterRequest(
                npc_id=npc_id,
                name=name,
                npc_class=npc_class,
                level=level,
                position=NPCPosition(map=request.map, x=x, y=y),
                personality=personality,
                faction_id=faction_id,
                initial_goals=goals
            )

            # Register NPC
            from routers.npc import register_npc
            await register_npc(reg_request)

            spawned_npcs.append({
                "npc_id": npc_id,
                "name": name,
                "npc_class": npc_class,
                "level": level,
                "position": {"map": request.map, "x": x, "y": y},
                "personality": personality.dict(),
                "faction_id": faction_id
            })

        logger.info(f"Successfully spawned {len(spawned_npcs)} NPCs")

        return BulkSpawnResponse(
            status="success",
            spawned_count=len(spawned_npcs),
            npcs=spawned_npcs,
            message=f"Successfully spawned {len(spawned_npcs)} NPCs on {request.map}"
        )

    except Exception as e:
        logger.error(f"Error spawning bulk NPCs: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to spawn bulk NPCs: {str(e)}"
        )

