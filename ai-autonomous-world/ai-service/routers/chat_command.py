"""
Chat Command Interface Router
Handles free-form text input via chat commands
"""

from fastapi import APIRouter, HTTPException, status, Depends
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
from ..database import db

router = APIRouter(prefix="/ai/chat", tags=["chat"])


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


async def check_rate_limit(player_id: str, npc_id: str, database) -> Optional[int]:
    """
    Check rate limit using DragonflyDB

    Returns:
        None if allowed, or remaining seconds if rate limited
    """
    if not settings.freeform_text_rate_limit_enabled:
        return None

    rate_limit_key = f"rate_limit:chat:{player_id}:{npc_id}"

    try:
        # Get last interaction time from Redis
        last_time_str = await db.redis.get(rate_limit_key)

        if last_time_str:
            last_time = datetime.fromisoformat(last_time_str.decode() if isinstance(last_time_str, bytes) else last_time_str)
            cooldown = timedelta(seconds=settings.chat_command_cooldown)

            if datetime.utcnow() - last_time < cooldown:
                remaining = (last_time + cooldown - datetime.utcnow()).total_seconds()
                return int(remaining)

        # Update rate limit with TTL
        await db.redis.setex(
            rate_limit_key,
            settings.chat_command_cooldown,
            datetime.utcnow().isoformat()
        )

        return None

    except Exception as e:
        logger.error(f"Rate limit check failed: {e}")
        # Fail open - allow request if rate limit check fails
        return None


@router.post("/command", response_model=ChatCommandResponse)
async def handle_chat_command(
    request: ChatCommandRequest
):
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

        # Check rate limiting using DragonflyDB
        remaining = await check_rate_limit(request.player_id, request.npc_id, db)
        if remaining is not None:
            logger.warning(f"Rate limit exceeded for player {request.player_id}")
            return ChatCommandResponse(
                success=False,
                error=f"Please wait {remaining} seconds before sending another message."
            )
        
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

            # Cache successful response for fallback (TTL: 1 hour)
            try:
                from ai_service.database import db
                cache_key = f"chat_fallback:{request.npc_id}:{request.player_id}"
                await db.client.setex(cache_key, 3600, npc_text)
                logger.debug(f"Cached response for NPC {request.npc_id}")
            except Exception as cache_error:
                logger.warning(f"Failed to cache response: {cache_error}")

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
                # Implement cached response fallback using DragonflyDB
                try:
                    from ai_service.database import db
                    cache_key = f"chat_fallback:{request.npc_id}:{request.player_id}"
                    cached_response = await db.client.get(cache_key)

                    if cached_response:
                        logger.info(f"Using cached fallback response for NPC {request.npc_id}")
                        return ChatCommandResponse(
                            success=True,
                            response_text=cached_response.decode('utf-8') if isinstance(cached_response, bytes) else cached_response,
                            npc_id=request.npc_id,
                            player_id=request.player_id
                        )
                    else:
                        logger.warning(f"No cached response available for NPC {request.npc_id}")
                        return ChatCommandResponse(
                            success=False,
                            error="AI service unavailable and no cached response available."
                        )
                except Exception as cache_error:
                    logger.error(f"Error retrieving cached response: {cache_error}")
                    return ChatCommandResponse(
                        success=False,
                        error="AI service unavailable. Please try again later."
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

