"""
World Bootstrap: Initial NPC Seeding, Faction, Economy, and Relationship Initialization

This module seeds the world with diverse NPCs, initial factions, economy, and relationships
according to the WORLD_CONCEPT_DESIGN.md. It is intended to be run at world/server initialization.
"""

import random
from datetime import datetime
from typing import List, Dict, Any
from models.npc import (
    NPCRegisterRequest, NPCPosition, NPCPersonality, NPCGoalState, NPCEmotionState, NPCMemoryState
)
from models.faction import Faction, FactionType, RelationshipStatus
from models.npc_relationship import NPCRelationship

NPC_CLASSES = ["merchant", "guard", "farmer", "craftsman", "explorer", "quest_giver", "noble", "thief", "priest", "adventurer"]
TOWNS = ["prontera", "geffen", "morocc", "payon", "alberta", "izlude"]
FACTIONS = [
    {"faction_id": "kingdom", "name": "Kingdom of Rune-Midgard", "faction_type": FactionType.KINGDOM, "description": "The ruling kingdom."},
    {"faction_id": "merchant_guild", "name": "Merchant Guild", "faction_type": FactionType.MERCHANT, "description": "Traders and economic power."},
    {"faction_id": "thieves_guild", "name": "Thieves Guild", "faction_type": FactionType.CRIMINAL, "description": "Underground criminal network."}
]

def random_personality() -> NPCPersonality:
    return NPCPersonality(
        openness=random.uniform(0.1, 0.9),
        conscientiousness=random.uniform(0.1, 0.9),
        extraversion=random.uniform(0.1, 0.9),
        agreeableness=random.uniform(0.1, 0.9),
        neuroticism=random.uniform(0.1, 0.9),
        ambition=random.uniform(0.1, 0.9),
        courage=random.uniform(0.1, 0.9),
        compassion=random.uniform(0.1, 0.9),
        cunning=random.uniform(0.1, 0.9),
        loyalty=random.uniform(0.1, 0.9),
        moral_alignment=random.choice([
            "lawful_good", "neutral_good", "chaotic_good",
            "lawful_neutral", "true_neutral", "chaotic_neutral",
            "lawful_evil", "neutral_evil", "chaotic_evil"
        ]),
        alignment_spectrum={
            "altruism": random.uniform(-1, 1),
            "lawful": random.uniform(-1, 1),
            "honest": random.uniform(-1, 1)
        },
        quirks=random.sample([
            "superstitious", "greedy", "generous", "paranoid", "optimistic", "pessimistic", "sarcastic", "naive"
        ], k=random.randint(0, 2))
    )

def random_goal_state() -> NPCGoalState:
    return NPCGoalState(
        survival=random.uniform(0.5, 1.0),
        security=random.uniform(0.5, 1.0),
        social=random.uniform(0.5, 1.0),
        esteem=random.uniform(0.5, 1.0),
        self_actualization=random.uniform(0.5, 1.0),
        short_term_goals=[random.choice(["buy food", "talk to friend", "work", "explore", "rest"])],
        long_term_goals=[random.choice(["become wealthy", "start a family", "gain power", "travel", "learn magic"])]
    )

def random_emotion_state() -> NPCEmotionState:
    return NPCEmotionState(
        current_emotion=random.choice(["joy", "sadness", "anger", "fear", "surprise", "disgust", "neutral"]),
        mood=random.choice(["happy", "sad", "anxious", "calm", "neutral"]),
        emotional_triggers=random.sample(["insult", "gift", "danger", "praise", "loss", "victory"], k=random.randint(0, 2)),
        emotional_regulation={},
        emotional_contagion=[],
        last_emotion_update=datetime.utcnow()
    )

def random_memory_state() -> NPCMemoryState:
    return NPCMemoryState(
        episodic=[],
        semantic=[],
        procedural=[],
        emotional=[],
        reflective=[],
        memory_decay=random.uniform(0.005, 0.02),
        memory_distortion=random.uniform(0.005, 0.02),
        memory_salience={},
        last_memory_update=datetime.utcnow()
    )

def random_position() -> NPCPosition:
    town = random.choice(TOWNS)
    return NPCPosition(
        map=town,
        x=random.randint(50, 250),
        y=random.randint(50, 250)
    )

def seed_npcs(n: int = 100) -> List[NPCRegisterRequest]:
    npcs = []
    for i in range(n):
        npc_id = f"npc_{i+1:03d}"
        npc_class = random.choice(NPC_CLASSES)
        name = f"{npc_class.capitalize()}_{i+1}"
        position = random_position()
        personality = random_personality()
        goal_state = random_goal_state()
        emotion_state = random_emotion_state()
        memory_state = random_memory_state()
        faction_id = random.choice(["kingdom", "merchant_guild", "thieves_guild"])
        initial_goals = goal_state.short_term_goals + goal_state.long_term_goals
        npcs.append(NPCRegisterRequest(
            npc_id=npc_id,
            name=name,
            npc_class=npc_class,
            level=random.randint(1, 99),
            position=position,
            personality=personality,
            goal_state=goal_state,
            emotion_state=emotion_state,
            memory_state=memory_state,
            faction_id=faction_id,
            initial_goals=initial_goals
        ))
    return npcs

def seed_factions() -> List[Faction]:
    factions = []
    for f in FACTIONS:
        factions.append(Faction(
            faction_id=f["faction_id"],
            name=f["name"],
            faction_type=f["faction_type"],
            description=f["description"],
            capital_location={"map": random.choice(TOWNS), "x": 150, "y": 150},
            controlled_areas=[random.choice(TOWNS)],
            wealth=random.randint(10000, 100000),
            military_power=random.randint(100, 1000),
            influence=random.randint(100, 1000),
            relationships={},
            npc_members=[],
            player_members=0,
            ideology=random.choice(["order", "profit", "freedom", "tradition", "chaos"]),
            goals=[random.choice(["expand", "defend", "profit", "influence", "survive"])]
        ))
    # Set initial relationships
    for f in factions:
        for other in factions:
            if f.faction_id == other.faction_id:
                continue
            f.relationships[other.faction_id] = random.choice([
                RelationshipStatus.ALLIED, RelationshipStatus.FRIENDLY,
                RelationshipStatus.NEUTRAL, RelationshipStatus.UNFRIENDLY,
                RelationshipStatus.HOSTILE, RelationshipStatus.WAR
            ])
    return factions

def seed_relationships(npcs: List[NPCRegisterRequest]) -> List[NPCRelationship]:
    relationships = []
    for i, npc1 in enumerate(npcs):
        for j, npc2 in enumerate(npcs):
            if i >= j:
                continue
            rel_type = random.choice([
                "friendship", "rivalry", "cooperation", "romantic", "family", "mentor", "enemy", "neutral"
            ])
            rel_value = random.uniform(-100, 100)
            trust = random.uniform(0, 100)
            relationships.append(NPCRelationship(
                npc_id_1=npc1.npc_id,
                npc_id_2=npc2.npc_id,
                relationship_type=rel_type,
                relationship_value=rel_value,
                trust_level=trust,
                interaction_count=random.randint(0, 20),
                last_interaction=None,
                created_at=datetime.utcnow(),
                updated_at=datetime.utcnow(),
                metadata={}
            ))
    return relationships

def world_bootstrap(n_npcs: int = 100) -> Dict[str, Any]:
    """
    Returns:
        {
            "npcs": List[NPCRegisterRequest],
            "factions": List[Faction],
            "relationships": List[NPCRelationship]
        }
    """
    npcs = seed_npcs(n_npcs)
    factions = seed_factions()
    relationships = seed_relationships(npcs)
    # Assign NPCs to factions
    for npc in npcs:
        for f in factions:
            if npc.faction_id == f.faction_id:
                f.npc_members.append(npc.npc_id)
    return {
        "npcs": npcs,
        "factions": factions,
        "relationships": relationships
    }

if __name__ == "__main__":
    # Example usage: print summary of seeded world
    world = world_bootstrap(100)
    print(f"Seeded {len(world['npcs'])} NPCs, {len(world['factions'])} factions, {len(world['relationships'])} relationships.")