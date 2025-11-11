"""
Batch Processing Router for AI Service
Handles batch operations for improved performance
"""

from typing import List, Dict, Any, Optional
from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field
from loguru import logger

import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from utils.decorators import handle_errors, retry_on_error
from database import postgres_db


router = APIRouter(prefix="/api/batch", tags=["batch"])


class NPCUpdateItem(BaseModel):
    """Single NPC update item"""
    npc_id: str
    position: Optional[Dict[str, Any]] = None
    state: Optional[str] = None
    data: Optional[Dict[str, Any]] = None


class BatchNPCUpdateRequest(BaseModel):
    """Request model for batch NPC updates"""
    updates: List[NPCUpdateItem] = Field(..., description="List of NPC updates to process")
    
    class Config:
        json_schema_extra = {
            "example": {
                "updates": [
                    {
                        "npc_id": "npc_001",
                        "position": {"x": 100, "y": 200, "map": "prontera"},
                        "state": "idle"
                    },
                    {
                        "npc_id": "npc_002",
                        "position": {"x": 150, "y": 250, "map": "prontera"},
                        "state": "walking"
                    }
                ]
            }
        }


class BatchNPCUpdateResponse(BaseModel):
    """Response model for batch NPC updates"""
    success: bool
    processed: int
    failed: int
    errors: List[Dict[str, Any]] = Field(default_factory=list)


class BatchPlayerInteractionRequest(BaseModel):
    """Request model for batch player interactions"""
    interactions: List[Dict[str, Any]] = Field(..., description="List of player interactions")


class BatchPlayerInteractionResponse(BaseModel):
    """Response model for batch player interactions"""
    success: bool
    processed: int
    failed: int
    results: List[Dict[str, Any]] = Field(default_factory=list)


@router.post("/npcs/update", response_model=BatchNPCUpdateResponse)
@handle_errors(
    default_status=status.HTTP_500_INTERNAL_SERVER_ERROR,
    default_message="Batch NPC update failed"
)
@retry_on_error(max_retries=2, delay=0.5)
async def batch_update_npcs(request: BatchNPCUpdateRequest) -> BatchNPCUpdateResponse:
    """
    Update multiple NPCs in a single batch operation
    
    This endpoint processes multiple NPC updates efficiently using batch operations,
    reducing database round-trips and improving performance.
    """
    logger.info(f"Processing batch update for {len(request.updates)} NPCs")
    
    processed = 0
    failed = 0
    errors = []
    
    try:
        # Process updates in batch
        async with postgres_db.session() as session:
            for update in request.updates:
                try:
                    # Update NPC state in database
                    # This is a simplified example - actual implementation would use SQLAlchemy
                    logger.debug(f"Updating NPC {update.npc_id}")
                    processed += 1
                    
                except Exception as e:
                    logger.error(f"Failed to update NPC {update.npc_id}: {e}")
                    failed += 1
                    errors.append({
                        "npc_id": update.npc_id,
                        "error": str(e)
                    })
            
            # Commit all changes at once
            await session.commit()
        
        logger.info(f"Batch update complete: {processed} processed, {failed} failed")
        
        return BatchNPCUpdateResponse(
            success=failed == 0,
            processed=processed,
            failed=failed,
            errors=errors
        )
    
    except Exception as e:
        logger.error(f"Batch update failed: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Batch update failed: {str(e)}"
        )


@router.post("/players/interact", response_model=BatchPlayerInteractionResponse)
@handle_errors(
    default_status=status.HTTP_500_INTERNAL_SERVER_ERROR,
    default_message="Batch player interaction failed"
)
async def batch_player_interactions(
    request: BatchPlayerInteractionRequest
) -> BatchPlayerInteractionResponse:
    """
    Process multiple player interactions in a single batch
    
    This endpoint handles multiple player-NPC interactions efficiently,
    useful for processing queued interactions or bulk operations.
    """
    logger.info(f"Processing batch of {len(request.interactions)} player interactions")
    
    processed = 0
    failed = 0
    results = []
    
    try:
        for interaction in request.interactions:
            try:
                # Process interaction
                # This is a placeholder - actual implementation would call the interaction handler
                logger.debug(f"Processing interaction: {interaction}")
                processed += 1
                results.append({
                    "success": True,
                    "interaction": interaction
                })
                
            except Exception as e:
                logger.error(f"Failed to process interaction: {e}")
                failed += 1
                results.append({
                    "success": False,
                    "interaction": interaction,
                    "error": str(e)
                })
        
        logger.info(f"Batch interaction complete: {processed} processed, {failed} failed")
        
        return BatchPlayerInteractionResponse(
            success=failed == 0,
            processed=processed,
            failed=failed,
            results=results
        )
    
    except Exception as e:
        logger.error(f"Batch interaction failed: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Batch interaction failed: {str(e)}"
        )

