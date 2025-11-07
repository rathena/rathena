"""
GPU-Accelerated Pathfinding (Experimental)
Provides GPU acceleration for large-scale pathfinding operations
"""

import numpy as np
from typing import List, Tuple, Optional, Any
from loguru import logger

from .pathfinding import Position


class GPUPathfinding:
    """
    GPU-accelerated pathfinding for large-scale NPC movement
    
    Features:
    - Parallel pathfinding for multiple NPCs
    - GPU-accelerated distance calculations
    - Batch processing for efficiency
    
    Note: This is experimental and only beneficial for 100+ simultaneous pathfinding requests
    """
    
    def __init__(self, config: Any, gpu_manager: Any):
        """
        Initialize GPU pathfinding
        
        Args:
            config: Settings object with GPU configuration
            gpu_manager: GPUManager instance
        """
        self.config = config
        self.gpu_manager = gpu_manager
        self.use_gpu = (
            config.gpu_enabled and 
            config.pathfinding_use_gpu and 
            gpu_manager.is_available()
        )
        
        self.device = gpu_manager.get_device() if self.use_gpu else None
        
        logger.info(f"GPU Pathfinding initialized (GPU: {self.use_gpu})")
        
        if self.use_gpu:
            logger.warning("GPU pathfinding is experimental and may not provide performance benefits for small batches")
    
    def batch_astar(
        self,
        starts: List[Position],
        goals: List[Position],
        walkable_grid: Optional[np.ndarray] = None
    ) -> List[List[Position]]:
        """
        Perform A* pathfinding for multiple start-goal pairs in parallel
        
        Args:
            starts: List of start positions
            goals: List of goal positions
            walkable_grid: Optional walkability grid (2D numpy array)
            
        Returns:
            List of paths (each path is a list of positions)
        """
        if len(starts) != len(goals):
            raise ValueError("Number of starts must equal number of goals")
        
        if not self.use_gpu or len(starts) < 10:
            # Use CPU for small batches
            return self._batch_astar_cpu(starts, goals, walkable_grid)
        
        try:
            return self._batch_astar_gpu(starts, goals, walkable_grid)
        except Exception as e:
            logger.warning(f"GPU pathfinding failed, falling back to CPU: {e}")
            return self._batch_astar_cpu(starts, goals, walkable_grid)
    
    def _batch_astar_cpu(
        self,
        starts: List[Position],
        goals: List[Position],
        walkable_grid: Optional[np.ndarray]
    ) -> List[List[Position]]:
        """CPU fallback for batch pathfinding"""
        from .pathfinding import Pathfinder

        # Convert numpy array to dict format if needed
        walkable_map = None
        if walkable_grid is not None:
            walkable_map = {}
            if isinstance(walkable_grid, np.ndarray):
                # Convert 2D numpy array to dict mapping (x, y) -> bool
                height, width = walkable_grid.shape
                for y in range(height):
                    for x in range(width):
                        walkable_map[(x, y)] = bool(walkable_grid[y, x])
            elif isinstance(walkable_grid, dict):
                walkable_map = walkable_grid

        pathfinder = Pathfinder(walkable_map=walkable_map)
        paths = []

        for start, goal in zip(starts, goals):
            path = pathfinder.astar(start, goal)
            paths.append(path)

        logger.debug(f"Computed {len(paths)} paths on CPU")
        return paths
    
    def _batch_astar_gpu(
        self,
        starts: List[Position],
        goals: List[Position],
        walkable_grid: Optional[np.ndarray]
    ) -> List[List[Position]]:
        """
        GPU-accelerated batch pathfinding
        
        Note: This is a simplified implementation. Full GPU pathfinding
        would require custom CUDA kernels for optimal performance.
        """
        try:
            import torch
            
            # Convert positions to tensors
            start_coords = torch.tensor(
                [[s.x, s.y] for s in starts],
                dtype=torch.float32,
                device=self.device
            )
            goal_coords = torch.tensor(
                [[g.x, g.y] for g in goals],
                dtype=torch.float32,
                device=self.device
            )
            
            # Compute distances on GPU (simplified heuristic)
            distances = torch.norm(goal_coords - start_coords, dim=1)
            
            # For now, fall back to CPU for actual pathfinding
            # Full GPU implementation would require custom CUDA kernels
            logger.debug("GPU distance computation complete, using CPU for path reconstruction")
            
            return self._batch_astar_cpu(starts, goals, walkable_grid)
            
        except Exception as e:
            logger.error(f"GPU pathfinding error: {e}")
            raise
    
    def compute_distance_matrix(
        self,
        positions: List[Position]
    ) -> np.ndarray:
        """
        Compute pairwise distance matrix for positions
        
        Args:
            positions: List of positions
            
        Returns:
            Distance matrix (n x n)
        """
        if not self.use_gpu or len(positions) < 50:
            return self._compute_distance_matrix_cpu(positions)
        
        try:
            return self._compute_distance_matrix_gpu(positions)
        except Exception as e:
            logger.warning(f"GPU distance matrix failed, falling back to CPU: {e}")
            return self._compute_distance_matrix_cpu(positions)
    
    def _compute_distance_matrix_cpu(self, positions: List[Position]) -> np.ndarray:
        """CPU distance matrix computation"""
        n = len(positions)
        matrix = np.zeros((n, n), dtype=np.float32)
        
        for i in range(n):
            for j in range(i + 1, n):
                dist = positions[i].distance_to(positions[j])
                matrix[i, j] = dist
                matrix[j, i] = dist
        
        return matrix
    
    def _compute_distance_matrix_gpu(self, positions: List[Position]) -> np.ndarray:
        """GPU distance matrix computation"""
        import torch
        
        # Convert to tensor
        coords = torch.tensor(
            [[p.x, p.y] for p in positions],
            dtype=torch.float32,
            device=self.device
        )
        
        # Compute pairwise distances using broadcasting
        diff = coords.unsqueeze(0) - coords.unsqueeze(1)
        distances = torch.norm(diff, dim=2)
        
        # Convert back to numpy
        return distances.cpu().numpy()

