"""
Chat Command Interface Router
Handles free-form text input via chat commands
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
from datetime import datetime, timedelta
from typing import Dict, Optional
from pydantic import BaseModel, Field

from ..config import settings
from ..models.player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    InteractionContext
)
from ..routers.player import handle_player_interaction

router = APIRouter(prefix="/ai/chat", tags=["chat"])

# Rate limiting storage (in production, use Redis)
_rate_limit_storage: Dict[str, datetime] = {}


class ChatCommandRequest(BaseModel):
    """Request for chat command interaction"""
    player_id: str = Field(..., description="Player ID")
    npc_id: str = Field(..., description="NPC ID to interact with")
    message: str = Field(..., description="Player message")
    player_name: str = Field(..., description="Player name")
    player_level: int = Field(..., description="Player level")
    player_class: str = Field(..., description="Player class")
    map_name: str = Field(..., description="Current map")
    x: int = Field(..., description="Player X coordinate")
    y: int = Field(..., description="Player Y coordinate")
    time_of_day: str = Field(default="day", description="Time of day")


class ChatCommandResponse(BaseModel):
    """Response for chat command interaction"""
    success: bool = Field(..., description="Whether the interaction succeeded")
    npc_response: Optional[str] = Field(None, description="NPC response text")
    emotion: Optional[str] = Field(None, description="NPC emotion")
    error: Optional[str] = Field(None, description="Error message if failed")


@router.post("/command", response_model=ChatCommandResponse)
async def handle_chat_command(request: ChatCommandRequest):
    """
    Handle chat command interaction
    
    Processes free-form text input from players via chat commands
    and returns AI-generated NPC responses.
    """
    try:
        # Check if chat command is enabled
        if not settings.chat_command_enabled:
            logger.warning("Chat command disabled but request received")
            raise HTTPException(
                status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                detail="Chat command interface is currently disabled"
            )
        
        logger.info(f"Chat command: Player {request.player_id} -> NPC {request.npc_id}")
        logger.debug(f"Message: {request.message}")
        
        # Validate message length
        if len(request.message) > settings.chat_command_max_length:
            logger.warning(f"Message too long: {len(request.message)} chars")
            return ChatCommandResponse(
                success=False,
                error=f"Message too long. Maximum {settings.chat_command_max_length} characters."
            )
        
        if len(request.message) == 0:
            logger.warning("Empty message received")
            return ChatCommandResponse(
                success=False,
                error="Message cannot be empty."
            )
        
        # Check rate limiting
        if settings.freeform_text_rate_limit_enabled:
            rate_limit_key = f"{request.player_id}:{request.npc_id}"
            
            if rate_limit_key in _rate_limit_storage:
                last_interaction = _rate_limit_storage[rate_limit_key]
                cooldown = timedelta(seconds=settings.chat_command_cooldown)
                
                if datetime.utcnow() - last_interaction < cooldown:
                    remaining = (last_interaction + cooldown - datetime.utcnow()).total_seconds()
                    logger.warning(f"Rate limit exceeded for {rate_limit_key}")
                    return ChatCommandResponse(
                        success=False,
                        error=f"Please wait {int(remaining)} seconds before sending another message."
                    )
            
            # Update rate limit
            _rate_limit_storage[rate_limit_key] = datetime.utcnow()
        
        # Build interaction context
        context = InteractionContext(
            player_name=request.player_name,
            player_level=request.player_level,
            player_class=request.player_class,
            location={
                "map": request.map_name,
                "x": request.x,
                "y": request.y
            },
            time_of_day=request.time_of_day,
            weather="clear"
        )
        
        # Build player interaction request
        interaction_request = PlayerInteractionRequest(
            npc_id=request.npc_id,
            player_id=request.player_id,
            interaction_type="talk",
            context=context,
            message=request.message
        )
        
        # Handle interaction using existing player interaction endpoint
        try:
            interaction_response = await handle_player_interaction(interaction_request)
            
            # Extract response text
            npc_text = interaction_response.response.data.get("text", "...")
            npc_emotion = interaction_response.response.emotion
            
            logger.info(f"Chat command successful: {request.npc_id} responded")
            
            return ChatCommandResponse(
                success=True,
                npc_response=npc_text,
                emotion=npc_emotion
            )
        
        except HTTPException as e:
            logger.error(f"Interaction failed: {e.detail}")
            
            # Handle fallback based on configuration
            if settings.chat_command_fallback_mode == "show_error":
                return ChatCommandResponse(
                    success=False,
                    error=f"AI service error: {e.detail}"
                )
            elif settings.chat_command_fallback_mode == "use_cached":
                # TODO: Implement cached response fallback
                return ChatCommandResponse(
                    success=False,
                    error="AI service unavailable. Cached responses not yet implemented."
                )
            else:
                return ChatCommandResponse(
                    success=False,
                    error="AI service unavailable. Please try again later."
                )
    
    except HTTPException:
        raise
    
    except Exception as e:
        logger.error(f"Error handling chat command: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process chat command: {str(e)}"
        )


@router.get("/status")
async def get_chat_command_status():
    """Get chat command interface status"""
    return {
        "enabled": settings.chat_command_enabled,
        "prefix": settings.chat_command_prefix,
        "max_length": settings.chat_command_max_length,
        "cooldown": settings.chat_command_cooldown,
        "rate_limiting": settings.freeform_text_rate_limit_enabled
    }

