"""
NPC Movement Utilities
Handles movement capability detection, sprite filtering, and movement validation
"""

from typing import Dict, Any, Optional, List
from loguru import logger

try:
    from ai_service.models.npc import NPCMovementCapabilities, NPCPosition
    from ai_service.config import settings
except ModuleNotFoundError:
    from models.npc import NPCMovementCapabilities, NPCPosition
    from config import settings


# Known NPC sprite IDs that support walking animations
# This is a curated list based on rAthena sprite database
# NPCs with job IDs < 45 are typically monsters/mobs with walking animations
# NPCs with job IDs 45-999 are typically static NPCs (shops, warps, etc.)
# NPCs with job IDs 1000+ are typically custom sprites
WALKING_SPRITE_IDS = {
    # Player job sprites (always have walking animations)
    *range(0, 45),
    
    # Monster sprites (have walking animations)
    *range(1000, 4000),
    
    # Custom NPC sprites with walking animations (examples)
    111,  # Kafra
    112,  # Kafra variant
    113,  # Kafra variant
    116,  # Priest
    123,  # Merchant
    
    # Add more sprite IDs as needed
}

# NPC classes that should always be able to move
MOBILE_NPC_CLASSES = {
    "guard",
    "patrol",
    "wanderer",
    "merchant_mobile",
    "quest_giver_mobile",
    "monster",
    "pet",
    "companion",
}

# NPC classes that should never move
STATIONARY_NPC_CLASSES = {
    "shop",
    "warp",
    "statue",
    "sign",
    "monument",
    "merchant_static",
    "quest_giver_static",
}


def can_npc_move(
    npc_id: str,
    npc_class: str,
    sprite_id: Optional[int] = None,
    movement_capabilities: Optional[NPCMovementCapabilities] = None
) -> bool:
    """
    Determine if an NPC can move autonomously
    
    Args:
        npc_id: NPC unique identifier
        npc_class: NPC class/type
        sprite_id: NPC sprite ID (job ID in rAthena)
        movement_capabilities: Explicit movement capabilities if provided
        
    Returns:
        True if NPC can move, False otherwise
    """
    # Check global movement setting
    if not settings.npc_movement_enabled:
        logger.debug(f"NPC {npc_id}: Movement globally disabled")
        return False
    
    # Check explicit movement capabilities
    if movement_capabilities:
        if not movement_capabilities.can_move:
            logger.debug(f"NPC {npc_id}: Movement explicitly disabled in capabilities")
            return False
        
        # If walking animation requirement is enabled, check sprite
        if settings.npc_movement_require_walking_sprite:
            if not movement_capabilities.has_walking_animation:
                logger.debug(f"NPC {npc_id}: No walking animation in capabilities")
                return False
    
    # Check NPC class restrictions
    if npc_class in STATIONARY_NPC_CLASSES:
        logger.debug(f"NPC {npc_id}: Class '{npc_class}' is stationary")
        return False
    
    if npc_class in MOBILE_NPC_CLASSES:
        logger.debug(f"NPC {npc_id}: Class '{npc_class}' is mobile")
        return True
    
    # Check sprite ID if provided and walking sprite requirement is enabled
    if settings.npc_movement_require_walking_sprite and sprite_id is not None:
        if sprite_id not in WALKING_SPRITE_IDS:
            logger.debug(f"NPC {npc_id}: Sprite ID {sprite_id} not in walking sprite list")
            return False
    
    # Default: allow movement
    logger.debug(f"NPC {npc_id}: Movement allowed (default)")
    return True


def get_movement_capabilities(
    npc_state: Dict[str, Any]
) -> NPCMovementCapabilities:
    """
    Extract or create movement capabilities from NPC state

    Args:
        npc_state: NPC state dictionary

    Returns:
        NPCMovementCapabilities instance
    """
    # Check if capabilities already exist in state
    if "movement_capabilities" in npc_state:
        caps_data = npc_state["movement_capabilities"]
        if isinstance(caps_data, dict):
            return NPCMovementCapabilities(**caps_data)
        elif isinstance(caps_data, NPCMovementCapabilities):
            return caps_data

    # Create default capabilities based on NPC class and sprite
    npc_class = npc_state.get("npc_class", "generic")
    sprite_id = npc_state.get("sprite_id")

    # Determine if NPC has walking animation
    has_walking_animation = True
    if sprite_id is not None:
        has_walking_animation = sprite_id in WALKING_SPRITE_IDS

    # Determine if NPC can move
    can_move = npc_class not in STATIONARY_NPC_CLASSES

    # Get home position from spawn position or current location
    home_position = None
    spawn_point = None
    if "spawn_position" in npc_state:
        spawn_pos = npc_state["spawn_position"]
        if isinstance(spawn_pos, dict):
            home_position = NPCPosition(**spawn_pos)
            spawn_point = NPCPosition(**spawn_pos)
    elif "location" in npc_state:
        loc = npc_state["location"]
        if isinstance(loc, dict):
            spawn_point = NPCPosition(**loc)

    # Get movement mode and radius from state
    movement_mode = npc_state.get("movement_mode", "radius_restricted")
    movement_radius = npc_state.get("movement_radius", settings.npc_movement_max_distance)

    return NPCMovementCapabilities(
        can_move=can_move,
        has_walking_animation=has_walking_animation,
        movement_speed=1.0,
        max_wander_distance=settings.npc_movement_max_distance,
        home_position=home_position,
        movement_mode=movement_mode,
        movement_radius=movement_radius,
        spawn_point=spawn_point
    )


def calculate_distance(pos1: Dict[str, Any], pos2: Dict[str, Any]) -> float:
    """
    Calculate Euclidean distance between two positions

    Args:
        pos1: First position {x, y, map}
        pos2: Second position {x, y, map}

    Returns:
        Distance in tiles (float)
    """
    import math

    # If different maps, return infinite distance
    if pos1.get("map") != pos2.get("map"):
        return float('inf')

    x1, y1 = pos1.get("x", 0), pos1.get("y", 0)
    x2, y2 = pos2.get("x", 0), pos2.get("y", 0)

    return math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)


def is_position_within_boundary(
    target_position: Dict[str, Any],
    movement_capabilities: NPCMovementCapabilities,
    npc_id: str
) -> bool:
    """
    Check if a target position is within NPC's movement boundaries

    Args:
        target_position: Target position {map, x, y}
        movement_capabilities: NPC movement capabilities with boundary settings
        npc_id: NPC identifier for logging

    Returns:
        True if position is within boundaries, False otherwise
    """
    movement_mode = movement_capabilities.movement_mode

    # Disabled mode - no movement allowed
    if movement_mode == "disabled":
        logger.debug(f"NPC {npc_id}: Movement disabled")
        return False

    # Global mode - all positions allowed
    if movement_mode == "global":
        logger.debug(f"NPC {npc_id}: Global movement mode, position allowed")
        return True

    # Map-restricted mode - must stay on same map
    if movement_mode == "map_restricted":
        if not movement_capabilities.spawn_point:
            logger.warning(f"NPC {npc_id}: Map-restricted mode but no spawn point defined, allowing movement")
            return True

        spawn_map = movement_capabilities.spawn_point.map
        target_map = target_position.get("map")

        if spawn_map != target_map:
            logger.debug(f"NPC {npc_id}: Map-restricted mode, target map {target_map} != spawn map {spawn_map}")
            return False

        logger.debug(f"NPC {npc_id}: Map-restricted mode, same map, position allowed")
        return True

    # Radius-restricted mode - must stay within radius from spawn point
    if movement_mode == "radius_restricted":
        if not movement_capabilities.spawn_point:
            logger.warning(f"NPC {npc_id}: Radius-restricted mode but no spawn point defined, allowing movement")
            return True

        spawn_pos = {
            "map": movement_capabilities.spawn_point.map,
            "x": movement_capabilities.spawn_point.x,
            "y": movement_capabilities.spawn_point.y
        }

        distance = calculate_distance(spawn_pos, target_position)
        max_radius = movement_capabilities.movement_radius

        if distance > max_radius:
            logger.debug(
                f"NPC {npc_id}: Radius-restricted mode, distance {distance:.1f} > max radius {max_radius}"
            )
            return False

        logger.debug(
            f"NPC {npc_id}: Radius-restricted mode, distance {distance:.1f} <= max radius {max_radius}, position allowed"
        )
        return True

    # Unknown mode - default to allowing movement
    logger.warning(f"NPC {npc_id}: Unknown movement mode '{movement_mode}', allowing movement")
    return True

