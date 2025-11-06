"""
Advanced Movement Features for NPCs
Implements social movement, goal-directed movement, collision avoidance, etc.
"""

from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from datetime import datetime
from loguru import logger
import random
import math


@dataclass
class Entity:
    """Represents an entity (NPC or player) in the world"""
    id: str
    x: int
    y: int
    entity_type: str  # "npc" or "player"
    name: Optional[str] = None


@dataclass
class MovementGoal:
    """Represents a movement goal for an NPC"""
    x: int
    y: int
    goal_type: str  # "quest", "patrol", "social", "retreat", "home"
    priority: int = 5  # 1-10, higher = more important
    expires_at: Optional[datetime] = None


class SocialMovementManager:
    """
    Manages social movement behaviors
    NPCs can follow, approach, or avoid other entities
    """
    
    def __init__(self, follow_distance: int = 3):
        """
        Initialize Social Movement Manager
        
        Args:
            follow_distance: Preferred distance to maintain when following
        """
        self.follow_distance = follow_distance
        logger.info(f"SocialMovementManager initialized with follow_distance={follow_distance}")
    
    def calculate_follow_position(
        self,
        npc_pos: Tuple[int, int],
        target_pos: Tuple[int, int]
    ) -> Optional[Tuple[int, int]]:
        """
        Calculate position to move to when following an entity
        
        Args:
            npc_pos: Current NPC position (x, y)
            target_pos: Target entity position (x, y)
            
        Returns:
            Target position to move to, or None if already at good distance
        """
        npc_x, npc_y = npc_pos
        target_x, target_y = target_pos
        
        # Calculate distance
        distance = math.sqrt((target_x - npc_x) ** 2 + (target_y - npc_y) ** 2)
        
        # If already at good distance, don't move
        if abs(distance - self.follow_distance) <= 1:
            logger.debug(f"NPC already at good follow distance: {distance:.2f}")
            return None
        
        # Calculate direction vector
        if distance == 0:
            return None
        
        dx = (target_x - npc_x) / distance
        dy = (target_y - npc_y) / distance
        
        # Calculate target position at follow_distance
        new_x = int(target_x - dx * self.follow_distance)
        new_y = int(target_y - dy * self.follow_distance)
        
        logger.debug(f"Follow position calculated: ({new_x}, {new_y})")
        return (new_x, new_y)
    
    def calculate_approach_position(
        self,
        npc_pos: Tuple[int, int],
        target_pos: Tuple[int, int],
        approach_distance: int = 1
    ) -> Optional[Tuple[int, int]]:
        """
        Calculate position to move to when approaching an entity
        
        Args:
            npc_pos: Current NPC position (x, y)
            target_pos: Target entity position (x, y)
            approach_distance: How close to get
            
        Returns:
            Target position to move to, or None if already close enough
        """
        npc_x, npc_y = npc_pos
        target_x, target_y = target_pos
        
        distance = math.sqrt((target_x - npc_x) ** 2 + (target_y - npc_y) ** 2)
        
        if distance <= approach_distance:
            logger.debug(f"NPC already close enough: {distance:.2f}")
            return None
        
        # Move directly toward target
        dx = (target_x - npc_x) / distance
        dy = (target_y - npc_y) / distance
        
        # Move one step closer
        new_x = int(npc_x + dx)
        new_y = int(npc_y + dy)
        
        logger.debug(f"Approach position calculated: ({new_x}, {new_y})")
        return (new_x, new_y)


class CollisionAvoidanceManager:
    """
    Manages collision avoidance for NPCs
    Prevents NPCs from walking into obstacles or other entities
    """
    
    def __init__(self, detection_radius: int = 2):
        """
        Initialize Collision Avoidance Manager
        
        Args:
            detection_radius: Radius to check for obstacles
        """
        self.detection_radius = detection_radius
        logger.info(f"CollisionAvoidanceManager initialized with radius={detection_radius}")
    
    def check_collision(
        self,
        target_pos: Tuple[int, int],
        entities: List[Entity],
        npc_id: str
    ) -> bool:
        """
        Check if moving to target position would cause collision
        
        Args:
            target_pos: Target position to check
            entities: List of entities in the area
            npc_id: ID of the NPC (to exclude self)
            
        Returns:
            True if collision detected, False otherwise
        """
        target_x, target_y = target_pos
        
        for entity in entities:
            # Skip self
            if entity.id == npc_id:
                continue
            
            # Check if entity is at target position
            if entity.x == target_x and entity.y == target_y:
                logger.debug(f"Collision detected with {entity.entity_type} {entity.id}")
                return True
        
        return False

    def find_alternative_position(
        self,
        npc_pos: Tuple[int, int],
        target_pos: Tuple[int, int],
        entities: List[Entity],
        npc_id: str
    ) -> Optional[Tuple[int, int]]:
        """
        Find alternative position when collision detected

        Args:
            npc_pos: Current NPC position
            target_pos: Desired target position (blocked)
            entities: List of entities in the area
            npc_id: ID of the NPC

        Returns:
            Alternative position, or None if no alternative found
        """
        npc_x, npc_y = npc_pos
        target_x, target_y = target_pos

        # Try positions around the target
        alternatives = [
            (target_x + 1, target_y),
            (target_x - 1, target_y),
            (target_x, target_y + 1),
            (target_x, target_y - 1),
            (target_x + 1, target_y + 1),
            (target_x + 1, target_y - 1),
            (target_x - 1, target_y + 1),
            (target_x - 1, target_y - 1),
        ]

        # Sort by distance to NPC
        alternatives.sort(key=lambda pos: (pos[0] - npc_x) ** 2 + (pos[1] - npc_y) ** 2)

        for alt_pos in alternatives:
            if not self.check_collision(alt_pos, entities, npc_id):
                logger.debug(f"Alternative position found: {alt_pos}")
                return alt_pos

        logger.warning("No alternative position found")
        return None


class PatrolRouteGenerator:
    """
    Generates dynamic patrol routes based on NPC role
    """

    def __init__(self):
        logger.info("PatrolRouteGenerator initialized")

    def generate_patrol_route(
        self,
        npc_role: str,
        home_pos: Tuple[int, int],
        map_bounds: Optional[Tuple[int, int, int, int]] = None,
        num_waypoints: int = 4
    ) -> List[Tuple[int, int]]:
        """
        Generate patrol route based on NPC role

        Args:
            npc_role: NPC role (guard, merchant, villager, etc.)
            home_pos: NPC home position
            map_bounds: Map boundaries (min_x, min_y, max_x, max_y)
            num_waypoints: Number of waypoints in route

        Returns:
            List of waypoint positions
        """
        logger.info(f"Generating patrol route for {npc_role}")

        home_x, home_y = home_pos

        # Role-specific patrol patterns
        if npc_role == "guard":
            # Guards patrol in a square pattern
            return self._generate_square_patrol(home_pos, radius=5, num_waypoints=num_waypoints)

        elif npc_role == "merchant":
            # Merchants stay near their shop
            return self._generate_circular_patrol(home_pos, radius=3, num_waypoints=num_waypoints)

        elif npc_role == "villager":
            # Villagers wander randomly
            return self._generate_random_patrol(home_pos, radius=10, num_waypoints=num_waypoints)

        elif npc_role == "scout":
            # Scouts patrol in a wide area
            return self._generate_random_patrol(home_pos, radius=20, num_waypoints=num_waypoints)

        else:
            # Default: circular patrol
            return self._generate_circular_patrol(home_pos, radius=5, num_waypoints=num_waypoints)

    def _generate_square_patrol(
        self,
        center: Tuple[int, int],
        radius: int,
        num_waypoints: int
    ) -> List[Tuple[int, int]]:
        """Generate square patrol pattern"""
        cx, cy = center
        waypoints = [
            (cx + radius, cy + radius),
            (cx - radius, cy + radius),
            (cx - radius, cy - radius),
            (cx + radius, cy - radius),
        ]
        return waypoints[:num_waypoints]

    def _generate_circular_patrol(
        self,
        center: Tuple[int, int],
        radius: int,
        num_waypoints: int
    ) -> List[Tuple[int, int]]:
        """Generate circular patrol pattern"""
        cx, cy = center
        waypoints = []

        for i in range(num_waypoints):
            angle = (2 * math.pi * i) / num_waypoints
            x = int(cx + radius * math.cos(angle))
            y = int(cy + radius * math.sin(angle))
            waypoints.append((x, y))

        return waypoints

    def _generate_random_patrol(
        self,
        center: Tuple[int, int],
        radius: int,
        num_waypoints: int
    ) -> List[Tuple[int, int]]:
        """Generate random patrol pattern"""
        cx, cy = center
        waypoints = []

        for _ in range(num_waypoints):
            angle = random.uniform(0, 2 * math.pi)
            distance = random.uniform(0, radius)
            x = int(cx + distance * math.cos(angle))
            y = int(cy + distance * math.sin(angle))
            waypoints.append((x, y))

        return waypoints


class TimeBasedMovementManager:
    """
    Manages time-based movement patterns
    NPCs behave differently based on time of day
    """

    def __init__(self):
        logger.info("TimeBasedMovementManager initialized")

    def get_movement_pattern(
        self,
        npc_role: str,
        time_of_day: str
    ) -> Dict[str, Any]:
        """
        Get movement pattern based on time of day

        Args:
            npc_role: NPC role
            time_of_day: "morning", "afternoon", "evening", "night"

        Returns:
            Movement pattern configuration
        """
        logger.debug(f"Getting movement pattern for {npc_role} at {time_of_day}")

        patterns = {
            "guard": {
                "morning": {"type": "patrol", "speed": 1.0, "range": 10},
                "afternoon": {"type": "patrol", "speed": 1.0, "range": 10},
                "evening": {"type": "patrol", "speed": 0.8, "range": 8},
                "night": {"type": "patrol", "speed": 1.2, "range": 12},
            },
            "merchant": {
                "morning": {"type": "wander", "speed": 0.5, "range": 3},
                "afternoon": {"type": "wander", "speed": 0.5, "range": 3},
                "evening": {"type": "return_home", "speed": 1.0, "range": 0},
                "night": {"type": "idle", "speed": 0.0, "range": 0},
            },
            "villager": {
                "morning": {"type": "wander", "speed": 1.0, "range": 15},
                "afternoon": {"type": "wander", "speed": 0.8, "range": 10},
                "evening": {"type": "return_home", "speed": 1.2, "range": 0},
                "night": {"type": "idle", "speed": 0.0, "range": 0},
            },
        }

        # Get pattern for role and time
        role_patterns = patterns.get(npc_role, patterns["villager"])
        pattern = role_patterns.get(time_of_day, {"type": "wander", "speed": 1.0, "range": 5})

        logger.debug(f"Movement pattern: {pattern}")
        return pattern


