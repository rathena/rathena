"""
Data models for AI Service API
"""

from .npc import (
    NPCRegisterRequest,
    NPCRegisterResponse,
    NPCEventRequest,
    NPCEventResponse,
    NPCActionResponse,
    NPCPersonality,
    NPCPosition,
)

from .world import (
    WorldStateUpdateRequest,
    WorldStateUpdateResponse,
    WorldStateQueryResponse,
    EconomyState,
    PoliticsState,
)

from .player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    InteractionContext,
    NPCResponse,
)

__all__ = [
    # NPC models
    "NPCRegisterRequest",
    "NPCRegisterResponse",
    "NPCEventRequest",
    "NPCEventResponse",
    "NPCActionResponse",
    "NPCPersonality",
    "NPCPosition",
    # World models
    "WorldStateUpdateRequest",
    "WorldStateUpdateResponse",
    "WorldStateQueryResponse",
    "EconomyState",
    "PoliticsState",
    # Player models
    "PlayerInteractionRequest",
    "PlayerInteractionResponse",
    "InteractionContext",
    "NPCResponse",
]

