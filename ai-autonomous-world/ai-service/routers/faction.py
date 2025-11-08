"""
Faction Router - Faction System Endpoints
Provides access to faction reputation and relationship management
"""

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from loguru import logger

from config import settings
from models.faction import Faction, PlayerReputation, RelationshipStatus


router = APIRouter(prefix="/ai/faction", tags=["faction"])


# ============================================================================
# Request/Response Models
# ============================================================================

class FactionCreateRequest(BaseModel):
    """Request to create a new faction"""
    faction_id: str = Field(..., description="Unique faction ID")
    name: str = Field(..., description="Faction name")
    description: Optional[str] = Field(None, description="Faction description")
    alignment: str = Field(default="neutral", description="good, evil, neutral")


class ReputationUpdateRequest(BaseModel):
    """Request to update player reputation with faction"""
    player_id: str = Field(..., description="Player ID")
    faction_id: str = Field(..., description="Faction ID")
    change: int = Field(..., description="Reputation change (positive or negative)")
    reason: Optional[str] = Field(None, description="Reason for change")


class FactionRelationshipRequest(BaseModel):
    """Request to update faction-to-faction relationship"""
    faction_id_1: str = Field(..., description="First faction ID")
    faction_id_2: str = Field(..., description="Second faction ID")
    relationship_value: int = Field(..., ge=-100, le=100, description="Relationship value (-100 to 100)")


# ============================================================================
# Faction CRUD Endpoints
# ============================================================================

@router.post("/create")
async def create_faction(request: FactionCreateRequest):
    """Create a new faction"""
    try:
        if not settings.faction_enabled:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Faction system is disabled"
            )
        
        from ..tasks.faction import create_faction

        faction = await create_faction(
            faction_id=request.faction_id,
            name=request.name,
            description=request.description,
            alignment=request.alignment
        )
        
        logger.info(f"Faction created: {request.faction_id} ({request.name})")
        return {
            "success": True,
            "faction": faction,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except Exception as e:
        logger.error(f"Error creating faction: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to create faction: {str(e)}"
        )


@router.get("/list")
async def list_factions():
    """List all factions"""
    try:
        from ..tasks.faction import list_all_factions

        factions = await list_all_factions()

        return {
            "factions": factions,
            "count": len(factions),
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error listing factions: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to list factions: {str(e)}"
        )


@router.get("/{faction_id}")
async def get_faction(faction_id: str):
    """Get faction information"""
    try:
        from ..tasks.faction import get_faction_info

        faction = await get_faction_info(faction_id)

        if not faction:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Faction {faction_id} not found"
            )

        return faction

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error getting faction {faction_id}: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get faction: {str(e)}"
        )


# ============================================================================
# Reputation Endpoints
# ============================================================================

@router.post("/reputation/update")
async def update_reputation(request: ReputationUpdateRequest):
    """Update player reputation with a faction"""
    try:
        if not settings.faction_enabled:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Faction system is disabled"
            )
        
        from ..tasks.faction import update_player_reputation
        
        result = await update_player_reputation(
            player_id=request.player_id,
            faction_id=request.faction_id,
            change=request.change,
            reason=request.reason
        )
        
        logger.info(f"Reputation updated: Player {request.player_id} with {request.faction_id}: {request.change:+d}")
        return {
            "success": True,
            "player_id": request.player_id,
            "faction_id": request.faction_id,
            "old_reputation": result.get("old_reputation", 0),
            "new_reputation": result.get("new_reputation", 0),
            "change": request.change,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except Exception as e:
        logger.error(f"Error updating reputation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update reputation: {str(e)}"
        )


@router.get("/reputation/{player_id}")
async def get_player_reputations(player_id: str):
    """Get player's reputation with all factions"""
    try:
        from ..tasks.faction import get_player_all_reputations
        
        reputations = await get_player_all_reputations(player_id)
        
        return {
            "player_id": player_id,
            "reputations": reputations,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except Exception as e:
        logger.error(f"Error getting player reputations: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get reputations: {str(e)}"
        )


@router.get("/reputation/{player_id}/{faction_id}")
async def get_player_faction_reputation(player_id: str, faction_id: str):
    """Get player's reputation with specific faction"""
    try:
        from ..tasks.faction import get_player_faction_reputation

        reputation = await get_player_faction_reputation(player_id, faction_id)

        return {
            "player_id": player_id,
            "faction_id": faction_id,
            "reputation": reputation,
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error getting reputation: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get reputation: {str(e)}"
        )


# ============================================================================
# Faction Relationship Endpoints
# ============================================================================

@router.post("/relationship/update")
async def update_faction_relationship(request: FactionRelationshipRequest):
    """Update relationship between two factions"""
    try:
        if not settings.faction_enabled or not settings.faction_dynamic_relationships:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Dynamic faction relationships are disabled"
            )

        from ..tasks.faction import update_faction_relationship

        result = await update_faction_relationship(
            faction_id_1=request.faction_id_1,
            faction_id_2=request.faction_id_2,
            relationship_value=request.relationship_value
        )

        logger.info(f"Faction relationship updated: {request.faction_id_1} <-> {request.faction_id_2}: {request.relationship_value}")
        return {
            "success": True,
            "faction_id_1": request.faction_id_1,
            "faction_id_2": request.faction_id_2,
            "relationship_value": request.relationship_value,
            "timestamp": datetime.utcnow().isoformat()
        }

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating faction relationship: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update relationship: {str(e)}"
        )


@router.get("/relationship/{faction_id_1}/{faction_id_2}")
async def get_faction_relationship(faction_id_1: str, faction_id_2: str):
    """Get relationship between two factions"""
    try:
        from ..tasks.faction import get_faction_relationship

        relationship = await get_faction_relationship(faction_id_1, faction_id_2)

        return {
            "faction_id_1": faction_id_1,
            "faction_id_2": faction_id_2,
            "relationship_value": relationship,
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error getting faction relationship: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get relationship: {str(e)}"
        )


@router.get("/relationships/{faction_id}")
async def get_all_faction_relationships(faction_id: str):
    """Get all relationships for a faction"""
    try:
        from ..tasks.faction import get_all_faction_relationships

        relationships = await get_all_faction_relationships(faction_id)

        return {
            "faction_id": faction_id,
            "relationships": relationships,
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error getting faction relationships: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get relationships: {str(e)}"
        )


