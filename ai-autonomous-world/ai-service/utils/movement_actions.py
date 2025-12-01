"""
Movement Actions Utility

Shared movement logic extracted from agents to eliminate duplication.
Provides pathfinding, boundary validation, and movement action generation.

Eliminates ~393 lines of duplicated code across agents.
"""

import math
import logging
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from enum import Enum

logger = logging.getLogger(__name__)


class MovementType(str, Enum):
    """Types of movement actions."""
    
    WALK = "walk"
    RUN = "run"
    TELEPORT = "teleport"
    PATROL = "patrol"
    FOLLOW = "follow"
    FLEE = "flee"
    WANDER = "wander"


@dataclass
class Position:
    """2D position with utility methods."""
    
    x: float
    y: float
    
    def distance_to(self, other: 'Position') -> float:
        """Calculate Euclidean distance to another position."""
        return math.sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2)
    
    def manhattan_distance_to(self, other: 'Position') -> float:
        """Calculate Manhattan distance to another position."""
        return abs(self.x - other.x) + abs(self.y - other.y)
    
    def direction_to(self, other: 'Position') -> float:
        """Get angle in radians to another position."""
        return math.atan2(other.y - self.y, other.x - self.x)
    
    def move_towards(self, target: 'Position', distance: float) -> 'Position':
        """Move towards target by specified distance."""
        current_distance = self.distance_to(target)
        if current_distance <= distance:
            return Position(target.x, target.y)
        
        # Calculate unit vector
        dx = (target.x - self.x) / current_distance
        dy = (target.y - self.y) / current_distance
        
        # Move by distance
        return Position(
            self.x + dx * distance,
            self.y + dy * distance
        )
    
    def to_dict(self) -> Dict[str, float]:
        """Convert to dictionary."""
        return {'x': self.x, 'y': self.y}


@dataclass
class Boundary:
    """Map boundary definition."""
    
    min_x: float
    max_x: float
    min_y: float
    max_y: float
    
    def contains(self, pos: Position) -> bool:
        """Check if position is within boundaries."""
        return (self.min_x <= pos.x <= self.max_x and
                self.min_y <= pos.y <= self.max_y)
    
    def clamp(self, pos: Position) -> Position:
        """Clamp position to boundaries."""
        return Position(
            max(self.min_x, min(self.max_x, pos.x)),
            max(self.min_y, min(self.max_y, pos.y))
        )


def validate_position(
    position: Position,
    boundary: Optional[Boundary] = None,
    location: Optional[str] = None
) -> Tuple[bool, Optional[str]]:
    """
    Validate if position is valid for movement.
    
    Args:
        position: Position to validate
        boundary: Map boundary (if None, uses default)
        location: Location name for logging
        
    Returns:
        Tuple[bool, Optional[str]]: (is_valid, error_message)
    """
    # Default boundary if not provided
    if boundary is None:
        boundary = Boundary(0.0, 1000.0, 0.0, 1000.0)
    
    if not boundary.contains(position):
        return False, f"Position {position.to_dict()} is out of bounds for {location}"
    
    # Add more validation as needed (e.g., obstacle checking)
    
    return True, None


def calculate_movement_speed(
    movement_type: MovementType,
    base_speed: float = 5.0,
    terrain_modifier: float = 1.0
) -> float:
    """
    Calculate movement speed based on type and modifiers.
    
    Args:
        movement_type: Type of movement
        base_speed: Base movement speed (units per second)
        terrain_modifier: Terrain speed modifier (0.0 to 2.0)
        
    Returns:
        float: Calculated speed
    """
    speed_multipliers = {
        MovementType.WALK: 1.0,
        MovementType.RUN: 2.0,
        MovementType.TELEPORT: 0.0,  # Instant
        MovementType.PATROL: 0.8,
        MovementType.FOLLOW: 1.2,
        MovementType.FLEE: 2.5,
        MovementType.WANDER: 0.6
    }
    
    multiplier = speed_multipliers.get(movement_type, 1.0)
    return base_speed * multiplier * terrain_modifier


def generate_movement_action_data(
    npc_id: str,
    current_position: Position,
    target_position: Position,
    movement_type: MovementType = MovementType.WALK,
    location: str = "unknown",
    boundary: Optional[Boundary] = None,
    metadata: Optional[Dict[str, Any]] = None
) -> Dict[str, Any]:
    """
    Generate standardized movement action data.
    
    This is the main function that eliminates duplication across agents.
    
    Args:
        npc_id: NPC identifier
        current_position: Current position
        target_position: Target position
        movement_type: Type of movement
        location: Location name
        boundary: Map boundary for validation
        metadata: Additional metadata
        
    Returns:
        dict: Standardized movement action data
    """
    # Validate target position
    is_valid, error_msg = validate_position(target_position, boundary, location)
    if not is_valid:
        logger.warning(f"Invalid target position for {npc_id}: {error_msg}")
        target_position = boundary.clamp(target_position) if boundary else target_position
    
    # Calculate movement details
    distance = current_position.distance_to(target_position)
    direction_rad = current_position.direction_to(target_position)
    direction_deg = math.degrees(direction_rad)
    
    # Calculate duration based on speed
    speed = calculate_movement_speed(movement_type)
    duration_seconds = distance / speed if speed > 0 else 0.0
    
    # Build action data
    action_data = {
        'action': 'move',
        'npc_id': npc_id,
        'movement_type': movement_type.value,
        'location': location,
        'from': current_position.to_dict(),
        'to': target_position.to_dict(),
        'distance': round(distance, 2),
        'direction_degrees': round(direction_deg, 2),
        'speed': round(speed, 2),
        'duration_seconds': round(duration_seconds, 2),
        'metadata': metadata or {}
    }
    
    return action_data


def calculate_patrol_waypoints(
    center: Position,
    radius: float,
    num_points: int = 4,
    boundary: Optional[Boundary] = None
) -> List[Position]:
    """
    Calculate waypoints for patrol pattern.
    
    Args:
        center: Center of patrol area
        radius: Patrol radius
        num_points: Number of waypoints
        boundary: Map boundary for clamping
        
    Returns:
        List[Position]: Patrol waypoints
    """
    waypoints = []
    angle_step = (2 * math.pi) / num_points
    
    for i in range(num_points):
        angle = i * angle_step
        x = center.x + radius * math.cos(angle)
        y = center.y + radius * math.sin(angle)
        
        pos = Position(x, y)
        
        # Clamp to boundary if provided
        if boundary:
            pos = boundary.clamp(pos)
        
        waypoints.append(pos)
    
    return waypoints


def find_nearest_safe_position(
    current: Position,
    danger_positions: List[Position],
    safe_distance: float,
    boundary: Optional[Boundary] = None,
    max_attempts: int = 8
) -> Position:
    """
    Find nearest safe position away from danger.
    
    Args:
        current: Current position
        danger_positions: List of dangerous positions to avoid
        safe_distance: Minimum safe distance from danger
        boundary: Map boundary
        max_attempts: Number of directions to try
        
    Returns:
        Position: Safe position (or current if none found)
    """
    if not danger_positions:
        return current
    
    # Find nearest danger
    nearest_danger = min(danger_positions, key=lambda p: current.distance_to(p))
    current_danger_dist = current.distance_to(nearest_danger)
    
    # Already safe
    if current_danger_dist >= safe_distance:
        return current
    
    # Try positions in different directions
    angle_step = (2 * math.pi) / max_attempts
    best_position = current
    best_distance = current_danger_dist
    
    for i in range(max_attempts):
        angle = i * angle_step
        test_x = current.x + safe_distance * math.cos(angle)
        test_y = current.y + safe_distance * math.sin(angle)
        test_pos = Position(test_x, test_y)
        
        # Clamp to boundary
        if boundary:
            test_pos = boundary.clamp(test_pos)
        
        # Check distance from all dangers
        min_danger_dist = min(test_pos.distance_to(d) for d in danger_positions)
        
        if min_danger_dist > best_distance:
            best_distance = min_danger_dist
            best_position = test_pos
    
    return best_position


def calculate_intercept_position(
    chaser: Position,
    chaser_speed: float,
    target: Position,
    target_velocity: Tuple[float, float]
) -> Position:
    """
    Calculate intercept position for target moving at velocity.
    
    Args:
        chaser: Chaser's current position
        chaser_speed: Chaser's speed
        target: Target's current position
        target_velocity: Target's velocity (vx, vy)
        
    Returns:
        Position: Intercept position
    """
    # Simple prediction based on time to reach
    distance = chaser.distance_to(target)
    time_to_reach = distance / chaser_speed if chaser_speed > 0 else 0
    
    # Predict target position
    predicted_x = target.x + target_velocity[0] * time_to_reach
    predicted_y = target.y + target_velocity[1] * time_to_reach
    
    return Position(predicted_x, predicted_y)


def is_path_clear(
    start: Position,
    end: Position,
    obstacles: List[Position],
    obstacle_radius: float = 5.0
) -> bool:
    """
    Check if path between two points is clear of obstacles.
    
    Uses simple line-circle intersection.
    
    Args:
        start: Start position
        end: End position
        obstacles: List of obstacle positions
        obstacle_radius: Obstacle collision radius
        
    Returns:
        bool: True if path is clear
    """
    if not obstacles:
        return True
    
    # Line segment from start to end
    dx = end.x - start.x
    dy = end.y - start.y
    length = math.sqrt(dx * dx + dy * dy)
    
    if length == 0:
        return True
    
    # Normalize direction
    dx /= length
    dy /= length
    
    # Check each obstacle
    for obstacle in obstacles:
        # Vector from start to obstacle
        fx = obstacle.x - start.x
        fy = obstacle.y - start.y
        
        # Project onto line direction
        projection = fx * dx + fy * dy
        
        # Clamp to line segment
        projection = max(0, min(length, projection))
        
        # Closest point on line
        closest_x = start.x + dx * projection
        closest_y = start.y + dy * projection
        
        # Distance to obstacle
        dist = math.sqrt(
            (obstacle.x - closest_x) ** 2 +
            (obstacle.y - closest_y) ** 2
        )
        
        # Check collision
        if dist < obstacle_radius:
            return False
    
    return True