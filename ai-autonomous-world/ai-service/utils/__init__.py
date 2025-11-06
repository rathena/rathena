"""
Utility modules for AI service
"""

from .movement_utils import can_npc_move, get_movement_capabilities
from .bridge_commands import translate_action_to_bridge_command

# Import pathfinding utilities
try:
    from .pathfinding import Position, PathNode, Pathfinder
except ImportError:
    Position = None
    PathNode = None
    Pathfinder = None

# Import map data utilities
try:
    from .map_data import MapDataManager
except ImportError:
    MapDataManager = None

# Import advanced movement utilities
try:
    from .advanced_movement import (
        Entity,
        MovementGoal,
        SocialMovementManager,
        CollisionAvoidanceManager,
        PatrolRouteGenerator,
        TimeBasedMovementManager
    )
except ImportError:
    Entity = None
    MovementGoal = None
    SocialMovementManager = None
    CollisionAvoidanceManager = None
    PatrolRouteGenerator = None
    TimeBasedMovementManager = None

# Import GPU manager utilities
try:
    from .gpu_manager import GPUManager, GPUInfo, get_gpu_manager, initialize_gpu
except ImportError:
    GPUManager = None
    GPUInfo = None
    get_gpu_manager = None
    initialize_gpu = None

# Import GPU vector search utilities
try:
    from .gpu_vector_search import GPUVectorSearch
except ImportError:
    GPUVectorSearch = None

# Import GPU pathfinding utilities
try:
    from .gpu_pathfinding import GPUPathfinding
except ImportError:
    GPUPathfinding = None

__all__ = [
    "can_npc_move",
    "get_movement_capabilities",
    "translate_action_to_bridge_command",
    "Position",
    "PathNode",
    "Pathfinder",
    "MapDataManager",
    "Entity",
    "MovementGoal",
    "SocialMovementManager",
    "CollisionAvoidanceManager",
    "PatrolRouteGenerator",
    "TimeBasedMovementManager",
    "GPUManager",
    "GPUInfo",
    "get_gpu_manager",
    "initialize_gpu",
    "GPUVectorSearch",
    "GPUPathfinding",
]

