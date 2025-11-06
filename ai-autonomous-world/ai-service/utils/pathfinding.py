"""
Advanced Pathfinding Utilities for NPC Movement
Implements A* and Dijkstra pathfinding algorithms
"""

from typing import List, Tuple, Optional, Set, Dict
from dataclasses import dataclass
import heapq
from loguru import logger


@dataclass
class Position:
    """2D position on the map"""
    x: int
    y: int
    
    def __hash__(self):
        return hash((self.x, self.y))
    
    def __eq__(self, other):
        return self.x == other.x and self.y == other.y
    
    def distance_to(self, other: 'Position') -> float:
        """Calculate Manhattan distance to another position"""
        return abs(self.x - other.x) + abs(self.y - other.y)
    
    def euclidean_distance_to(self, other: 'Position') -> float:
        """Calculate Euclidean distance to another position"""
        return ((self.x - other.x) ** 2 + (self.y - other.y) ** 2) ** 0.5


@dataclass
class PathNode:
    """Node in pathfinding algorithm"""
    position: Position
    g_cost: float  # Cost from start
    h_cost: float  # Heuristic cost to goal
    parent: Optional['PathNode'] = None
    
    @property
    def f_cost(self) -> float:
        """Total cost (g + h)"""
        return self.g_cost + self.h_cost
    
    def __lt__(self, other):
        return self.f_cost < other.f_cost


class Pathfinder:
    """
    Advanced pathfinding for NPC movement
    Supports A* and Dijkstra algorithms
    """
    
    def __init__(self, walkable_map: Optional[Dict[Tuple[int, int], bool]] = None):
        """
        Initialize pathfinder
        
        Args:
            walkable_map: Dictionary mapping (x, y) to walkability (True = walkable)
        """
        self.walkable_map = walkable_map or {}
        logger.info("Pathfinder initialized")
    
    def is_walkable(self, x: int, y: int) -> bool:
        """
        Check if a position is walkable
        
        Args:
            x: X coordinate
            y: Y coordinate
            
        Returns:
            True if walkable, False otherwise
        """
        # If no map data, assume all positions are walkable
        if not self.walkable_map:
            return True
        
        return self.walkable_map.get((x, y), False)
    
    def get_neighbors(self, pos: Position) -> List[Position]:
        """
        Get walkable neighboring positions (8-directional movement)
        
        Args:
            pos: Current position
            
        Returns:
            List of walkable neighbor positions
        """
        neighbors = []
        directions = [
            (-1, -1), (-1, 0), (-1, 1),  # Top row
            (0, -1),           (0, 1),    # Middle row
            (1, -1),  (1, 0),  (1, 1)     # Bottom row
        ]
        
        for dx, dy in directions:
            new_x, new_y = pos.x + dx, pos.y + dy
            if self.is_walkable(new_x, new_y):
                neighbors.append(Position(new_x, new_y))
        
        return neighbors
    
    def astar(self, start: Position, goal: Position, max_iterations: int = 1000) -> Optional[List[Position]]:
        """
        A* pathfinding algorithm
        
        Args:
            start: Starting position
            goal: Goal position
            max_iterations: Maximum iterations to prevent infinite loops
            
        Returns:
            List of positions from start to goal, or None if no path found
        """
        logger.debug(f"A* pathfinding from ({start.x}, {start.y}) to ({goal.x}, {goal.y})")
        
        # Priority queue: (f_cost, node)
        open_set = []
        start_node = PathNode(start, 0, start.distance_to(goal))
        heapq.heappush(open_set, start_node)
        
        # Track visited positions
        closed_set: Set[Position] = set()
        
        # Track best g_cost for each position
        g_costs: Dict[Position, float] = {start: 0}
        
        iterations = 0
        
        while open_set and iterations < max_iterations:
            iterations += 1
            
            # Get node with lowest f_cost
            current_node = heapq.heappop(open_set)
            current_pos = current_node.position
            
            # Check if we reached the goal
            if current_pos == goal:
                logger.debug(f"Path found in {iterations} iterations")
                return self._reconstruct_path(current_node)
            
            # Skip if already visited
            if current_pos in closed_set:
                continue

            closed_set.add(current_pos)

            # Explore neighbors
            for neighbor_pos in self.get_neighbors(current_pos):
                if neighbor_pos in closed_set:
                    continue

                # Calculate movement cost (diagonal = 1.414, straight = 1.0)
                dx = abs(neighbor_pos.x - current_pos.x)
                dy = abs(neighbor_pos.y - current_pos.y)
                move_cost = 1.414 if (dx + dy) == 2 else 1.0

                tentative_g = current_node.g_cost + move_cost

                # Skip if we've found a better path to this neighbor
                if neighbor_pos in g_costs and tentative_g >= g_costs[neighbor_pos]:
                    continue

                # This is the best path to this neighbor so far
                g_costs[neighbor_pos] = tentative_g
                h_cost = neighbor_pos.distance_to(goal)

                neighbor_node = PathNode(
                    position=neighbor_pos,
                    g_cost=tentative_g,
                    h_cost=h_cost,
                    parent=current_node
                )
                heapq.heappush(open_set, neighbor_node)

        logger.warning(f"No path found after {iterations} iterations")
        return None

    def dijkstra(self, start: Position, goal: Position, max_iterations: int = 1000) -> Optional[List[Position]]:
        """
        Dijkstra pathfinding algorithm (A* with h_cost = 0)

        Args:
            start: Starting position
            goal: Goal position
            max_iterations: Maximum iterations to prevent infinite loops

        Returns:
            List of positions from start to goal, or None if no path found
        """
        logger.debug(f"Dijkstra pathfinding from ({start.x}, {start.y}) to ({goal.x}, {goal.y})")

        # Use A* with h_cost = 0 (equivalent to Dijkstra)
        open_set = []
        start_node = PathNode(start, 0, 0)
        heapq.heappush(open_set, start_node)

        closed_set: Set[Position] = set()
        g_costs: Dict[Position, float] = {start: 0}

        iterations = 0

        while open_set and iterations < max_iterations:
            iterations += 1

            current_node = heapq.heappop(open_set)
            current_pos = current_node.position

            if current_pos == goal:
                logger.debug(f"Path found in {iterations} iterations")
                return self._reconstruct_path(current_node)

            if current_pos in closed_set:
                continue

            closed_set.add(current_pos)

            for neighbor_pos in self.get_neighbors(current_pos):
                if neighbor_pos in closed_set:
                    continue

                dx = abs(neighbor_pos.x - current_pos.x)
                dy = abs(neighbor_pos.y - current_pos.y)
                move_cost = 1.414 if (dx + dy) == 2 else 1.0

                tentative_g = current_node.g_cost + move_cost

                if neighbor_pos in g_costs and tentative_g >= g_costs[neighbor_pos]:
                    continue

                g_costs[neighbor_pos] = tentative_g

                neighbor_node = PathNode(
                    position=neighbor_pos,
                    g_cost=tentative_g,
                    h_cost=0,  # Dijkstra doesn't use heuristic
                    parent=current_node
                )
                heapq.heappush(open_set, neighbor_node)

        logger.warning(f"No path found after {iterations} iterations")
        return None

    def _reconstruct_path(self, goal_node: PathNode) -> List[Position]:
        """
        Reconstruct path from goal node to start

        Args:
            goal_node: Final node in the path

        Returns:
            List of positions from start to goal
        """
        path = []
        current = goal_node

        while current is not None:
            path.append(current.position)
            current = current.parent

        path.reverse()
        logger.debug(f"Path reconstructed with {len(path)} positions")
        return path

    def find_path(
        self,
        start: Position,
        goal: Position,
        algorithm: str = "astar",
        max_iterations: int = 1000
    ) -> Optional[List[Position]]:
        """
        Find path using specified algorithm

        Args:
            start: Starting position
            goal: Goal position
            algorithm: "astar" or "dijkstra"
            max_iterations: Maximum iterations

        Returns:
            List of positions from start to goal, or None if no path found
        """
        if algorithm == "astar":
            return self.astar(start, goal, max_iterations)
        elif algorithm == "dijkstra":
            return self.dijkstra(start, goal, max_iterations)
        else:
            logger.error(f"Unknown pathfinding algorithm: {algorithm}")
            return None

    def update_walkable_map(self, walkable_map: Dict[Tuple[int, int], bool]):
        """
        Update the walkable map data

        Args:
            walkable_map: New walkable map data
        """
        self.walkable_map = walkable_map
        logger.info(f"Walkable map updated with {len(walkable_map)} positions")


