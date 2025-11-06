"""
API Routers
"""

from .npc import router as npc_router
from .world import router as world_router
from .player import router as player_router

__all__ = [
    "npc_router",
    "world_router",
    "player_router",
]

