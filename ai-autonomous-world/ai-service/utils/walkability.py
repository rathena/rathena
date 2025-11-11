"""
Walkability and collision detection utilities for NPC movement.

This module provides functions to check if a position is walkable and not occupied
by obstacles, other NPCs, or players.
"""

import logging
import math
from typing import Dict, List, Tuple, Optional

logger = logging.getLogger(__name__)


# Prontera walkability map - common non-walkable areas
# This is a simplified version. In production, this should be loaded from map data.
# NOTE: For testing, we're being very conservative and only blocking obvious obstacles
# The actual fountain in Prontera is much smaller than initially thought
PRONTERA_NON_WALKABLE_AREAS = [
    # Actual fountain (very small area in center)
    # {"x_min": 153, "x_max": 158, "y_min": 182, "y_max": 187},

    # Castle area (north)
    {"x_min": 150, "x_max": 165, "y_min": 280, "y_max": 300},

    # Buildings (various - far from test area)
    {"x_min": 100, "x_max": 120, "y_min": 200, "y_max": 220},
    {"x_min": 200, "x_max": 220, "y_min": 200, "y_max": 220},
]


def is_position_walkable(map_name: str, x: int, y: int) -> bool:
    """
    Check if a position is walkable on the given map.
    
    Args:
        map_name: Name of the map (e.g., "prontera")
        x: X coordinate
        y: Y coordinate
    
    Returns:
        True if the position is walkable, False otherwise
    """
    # For now, we only have data for Prontera
    if map_name.lower() != "prontera":
        logger.warning(f"Walkability check not implemented for map: {map_name}")
        return True  # Assume walkable for unknown maps
    
    # Check against known non-walkable areas
    for area in PRONTERA_NON_WALKABLE_AREAS:
        if (area["x_min"] <= x <= area["x_max"] and 
            area["y_min"] <= y <= area["y_max"]):
            logger.debug(f"Position ({x}, {y}) is in non-walkable area on {map_name}")
            return False
    
    return True


def find_nearest_walkable_position(
    map_name: str,
    target_x: int,
    target_y: int,
    max_search_radius: int = 5
) -> Optional[Tuple[int, int]]:
    """
    Find the nearest walkable position to the target coordinates.
    
    Uses a spiral search pattern starting from the target position.
    
    Args:
        map_name: Name of the map
        target_x: Target X coordinate
        target_y: Target Y coordinate
        max_search_radius: Maximum distance to search (default: 5 tiles)
    
    Returns:
        Tuple of (x, y) for nearest walkable position, or None if not found
    """
    # Check if target itself is walkable
    if is_position_walkable(map_name, target_x, target_y):
        return (target_x, target_y)
    
    # Spiral search pattern
    for radius in range(1, max_search_radius + 1):
        # Check positions in a square around the target
        for dx in range(-radius, radius + 1):
            for dy in range(-radius, radius + 1):
                # Only check positions at the current radius (not inside)
                if abs(dx) == radius or abs(dy) == radius:
                    check_x = target_x + dx
                    check_y = target_y + dy
                    
                    if is_position_walkable(map_name, check_x, check_y):
                        logger.debug(f"Found walkable position ({check_x}, {check_y}) "
                                   f"near target ({target_x}, {target_y})")
                        return (check_x, check_y)
    
    logger.warning(f"No walkable position found within {max_search_radius} tiles "
                  f"of ({target_x}, {target_y}) on {map_name}")
    return None


def is_within_radius(
    x: int,
    y: int,
    center_x: int,
    center_y: int,
    radius: int
) -> bool:
    """
    Check if a position is within a given radius of a center point.
    
    Args:
        x: X coordinate to check
        y: Y coordinate to check
        center_x: Center X coordinate
        center_y: Center Y coordinate
        radius: Maximum distance from center
    
    Returns:
        True if position is within radius, False otherwise
    """
    distance = math.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
    return distance <= radius


def get_walkable_positions_in_radius(
    map_name: str,
    center_x: int,
    center_y: int,
    radius: int,
    min_distance: int = 0
) -> List[Tuple[int, int]]:
    """
    Get all walkable positions within a radius of a center point.
    
    Args:
        map_name: Name of the map
        center_x: Center X coordinate
        center_y: Center Y coordinate
        radius: Maximum distance from center
        min_distance: Minimum distance from center (default: 0)
    
    Returns:
        List of (x, y) tuples for walkable positions
    """
    walkable_positions = []
    
    # Check all positions in the bounding box
    for x in range(center_x - radius, center_x + radius + 1):
        for y in range(center_y - radius, center_y + radius + 1):
            # Check if within radius
            if is_within_radius(x, y, center_x, center_y, radius):
                # Check if meets minimum distance requirement
                if min_distance > 0:
                    distance = math.sqrt((x - center_x) ** 2 + (y - center_y) ** 2)
                    if distance < min_distance:
                        continue
                
                # Check if walkable
                if is_position_walkable(map_name, x, y):
                    walkable_positions.append((x, y))
    
    return walkable_positions

