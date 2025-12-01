"""
Map Data Integration for rAthena Server
Fetches and caches walkability data from rAthena
"""

from typing import Dict, Tuple, Optional, List
import aiohttp
from loguru import logger
from datetime import datetime, timedelta


class MapDataManager:
    """
    Manages map data integration with rAthena server
    Fetches walkability data and caches it for pathfinding
    """
    
    def __init__(
        self,
        bridge_url: str = "http://localhost:8888",
        cache_ttl: int = 300  # 5 minutes
    ):
        """
        Initialize Map Data Manager
        
        Args:
            bridge_url: URL of the rAthena bridge layer
            cache_ttl: Cache time-to-live in seconds
        """
        self.bridge_url = bridge_url
        self.cache_ttl = cache_ttl
        
        # Cache: map_name -> {walkable_map, timestamp}
        self.map_cache: Dict[str, Dict] = {}
        
        logger.info(f"MapDataManager initialized with bridge URL: {bridge_url}")
    
    async def fetch_map_data(self, map_name: str) -> Optional[Dict[Tuple[int, int], bool]]:
        """
        Fetch walkability data for a map from rAthena server
        
        Args:
            map_name: Name of the map (e.g., "prontera")
            
        Returns:
            Dictionary mapping (x, y) to walkability, or None if fetch failed
        """
        logger.info(f"Fetching map data for: {map_name}")
        
        try:
            async with aiohttp.ClientSession() as session:
                url = f"{self.bridge_url}/ai/map/walkability"
                params = {"map_name": map_name}
                
                async with session.get(url, params=params, timeout=10) as response:
                    if response.status == 200:
                        data = await response.json()
                        
                        # Convert to walkable map format
                        walkable_map = {}
                        
                        if "walkable_cells" in data:
                            # Format: list of [x, y] coordinates
                            for cell in data["walkable_cells"]:
                                x, y = cell[0], cell[1]
                                walkable_map[(x, y)] = True
                        
                        elif "grid" in data:
                            # Format: 2D grid of walkability
                            grid = data["grid"]
                            for y, row in enumerate(grid):
                                for x, walkable in enumerate(row):
                                    walkable_map[(x, y)] = bool(walkable)
                        
                        logger.info(f"Fetched {len(walkable_map)} walkable cells for {map_name}")
                        return walkable_map
                    
                    else:
                        logger.warning(f"Failed to fetch map data: HTTP {response.status}")
                        return None
        
        except aiohttp.ClientError as e:
            logger.error(f"Network error fetching map data: {e}")
            return None
        
        except Exception as e:
            logger.error(f"Error fetching map data: {e}")
            return None
    
    async def get_map_data(self, map_name: str, use_cache: bool = True) -> Optional[Dict[Tuple[int, int], bool]]:
        """
        Get walkability data for a map (with caching)
        
        Args:
            map_name: Name of the map
            use_cache: Whether to use cached data
            
        Returns:
            Dictionary mapping (x, y) to walkability, or None if unavailable
        """
        # Check cache
        if use_cache and map_name in self.map_cache:
            cache_entry = self.map_cache[map_name]
            cache_time = cache_entry["timestamp"]
            
            # Check if cache is still valid
            from datetime import UTC
            if datetime.now(UTC) - cache_time < timedelta(seconds=self.cache_ttl):
                logger.debug(f"Using cached map data for {map_name}")
                return cache_entry["walkable_map"]
            else:
                logger.debug(f"Cache expired for {map_name}")
        
        # Fetch fresh data
        walkable_map = await self.fetch_map_data(map_name)
        
        if walkable_map is not None:
            # Update cache
            self.map_cache[map_name] = {
                "walkable_map": walkable_map,
                "timestamp": datetime.now(UTC)
            }
            logger.debug(f"Cached map data for {map_name}")
        
        return walkable_map
    
    def clear_cache(self, map_name: Optional[str] = None):
        """
        Clear map data cache
        
        Args:
            map_name: Specific map to clear, or None to clear all
        """
        if map_name is None:
            self.map_cache.clear()
            logger.info("Cleared all map data cache")
        elif map_name in self.map_cache:
            del self.map_cache[map_name]
            logger.info(f"Cleared cache for map: {map_name}")
    
    def get_cached_maps(self) -> List[str]:
        """
        Get list of cached map names
        
        Returns:
            List of map names in cache
        """
        return list(self.map_cache.keys())
    
    def is_position_walkable(
        self,
        map_name: str,
        x: int,
        y: int,
        default: bool = True
    ) -> bool:
        """
        Check if a position is walkable (synchronous, cache-only)
        
        Args:
            map_name: Name of the map
            x: X coordinate
            y: Y coordinate
            default: Default value if map not in cache
            
        Returns:
            True if walkable, False otherwise
        """
        if map_name not in self.map_cache:
            return default
        
        walkable_map = self.map_cache[map_name]["walkable_map"]
        return walkable_map.get((x, y), default)

