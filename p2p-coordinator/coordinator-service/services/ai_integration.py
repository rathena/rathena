"""
AI Service Integration

Integrates P2P coordinator with the AI autonomous world service.
Handles NPC state broadcasting and zone synchronization.
"""

from typing import List, Dict, Optional, Any
import httpx
from loguru import logger

from config import settings


class AIServiceClient:
    """
    AI Service Client
    
    Communicates with the AI autonomous world service to:
    - Get NPC states for P2P zones
    - Broadcast NPC updates to P2P hosts
    - Synchronize zone states
    """
    
    def __init__(self):
        self.ai_service_url = settings.ai_service.ai_service_url
        self.timeout = 30  # Default timeout
        self.enabled = settings.ai_service.ai_service_enabled
        self.client: Optional[httpx.AsyncClient] = None
        logger.info(f"AIServiceClient initialized with URL: {self.ai_service_url}, enabled: {self.enabled}")
    
    async def initialize(self) -> None:
        """Initialize HTTP client"""
        self.client = httpx.AsyncClient(
            base_url=self.ai_service_url,
            timeout=self.timeout,
        )
        logger.info("AIServiceClient HTTP client initialized")
    
    async def close(self) -> None:
        """Close HTTP client"""
        if self.client:
            await self.client.aclose()
            logger.info("AIServiceClient HTTP client closed")
    
    async def health_check(self) -> bool:
        """
        Check if AI service is healthy
        
        Returns:
            True if AI service is healthy, False otherwise
        """
        try:
            if not self.client:
                await self.initialize()
            
            response = await self.client.get("/health")
            return response.status_code == 200
        except Exception as e:
            logger.error(f"AI service health check failed: {e}")
            return False
    
    async def get_zone_npcs(self, zone_id: str) -> List[Dict[str, Any]]:
        """
        Get all NPCs in a zone
        
        Args:
            zone_id: Zone identifier
            
        Returns:
            List of NPC data dictionaries
        """
        try:
            if not self.client:
                await self.initialize()
            
            response = await self.client.get(f"/api/world/zones/{zone_id}/npcs")
            response.raise_for_status()
            
            npcs = response.json()
            logger.info(f"Retrieved {len(npcs)} NPCs for zone {zone_id}")
            return npcs
        
        except httpx.HTTPStatusError as e:
            logger.error(f"Failed to get NPCs for zone {zone_id}: HTTP {e.response.status_code}")
            return []
        except Exception as e:
            logger.error(f"Failed to get NPCs for zone {zone_id}: {e}")
            return []
    
    async def get_npc_state(self, npc_id: str) -> Optional[Dict[str, Any]]:
        """
        Get current state of an NPC
        
        Args:
            npc_id: NPC identifier
            
        Returns:
            NPC state dictionary or None if not found
        """
        try:
            if not self.client:
                await self.initialize()
            
            response = await self.client.get(f"/api/npcs/{npc_id}")
            response.raise_for_status()
            
            npc_state = response.json()
            logger.debug(f"Retrieved state for NPC {npc_id}")
            return npc_state
        
        except httpx.HTTPStatusError as e:
            if e.response.status_code == 404:
                logger.warning(f"NPC not found: {npc_id}")
            else:
                logger.error(f"Failed to get NPC state for {npc_id}: HTTP {e.response.status_code}")
            return None
        except Exception as e:
            logger.error(f"Failed to get NPC state for {npc_id}: {e}")
            return None
    
    async def update_npc_position(self, npc_id: str, x: int, y: int, zone_id: str) -> bool:
        """
        Update NPC position
        
        Args:
            npc_id: NPC identifier
            x: X coordinate
            y: Y coordinate
            zone_id: Zone identifier
            
        Returns:
            True if update was successful, False otherwise
        """
        try:
            if not self.client:
                await self.initialize()
            
            response = await self.client.patch(
                f"/api/npcs/{npc_id}/position",
                json={"x": x, "y": y, "zone_id": zone_id},
            )
            response.raise_for_status()
            
            logger.debug(f"Updated position for NPC {npc_id}: ({x}, {y}) in {zone_id}")
            return True
        
        except Exception as e:
            logger.error(f"Failed to update NPC position for {npc_id}: {e}")
            return False


# Global AI service client instance
ai_service_client = AIServiceClient()

