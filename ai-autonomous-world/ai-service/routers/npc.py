"""
NPC API endpoints
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
import uuid
from datetime import datetime

try:
    from ..models.npc import (
        NPCRegisterRequest,
        NPCRegisterResponse,
        NPCEventRequest,
        NPCEventResponse,
        NPCActionResponse,
        NPCAction,
    )
    from ..database import db
    from ..config import settings
except ImportError:
    from models.npc import (
        NPCRegisterRequest,
        NPCRegisterResponse,
        NPCEventRequest,
        NPCEventResponse,
        NPCActionResponse,
        NPCAction,
    )
    from database import db
    from config import settings

router = APIRouter(prefix="/ai/npc", tags=["npc"])


@router.post("/register", response_model=NPCRegisterResponse)
async def register_npc(request: NPCRegisterRequest):
    """
    Register an NPC with the AI service

    Creates an AI agent for the NPC and stores initial state in database.
    """
    try:
        # Validate NPC ID
        if not request.npc_id or not isinstance(request.npc_id, str):
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Invalid NPC ID: must be a non-empty string"
            )

        # Validate name
        if not request.name or len(request.name) > settings.max_npc_name_length:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid NPC name: must be 1-{settings.max_npc_name_length} characters"
            )

        # Validate level
        if request.level < 1 or request.level > settings.max_npc_level:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid NPC level: must be between 1 and {settings.max_npc_level}"
            )

        # Validate position coordinates
        if request.position.x < 0 or request.position.y < 0:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Invalid position: coordinates must be non-negative"
            )

        if request.position.x > settings.max_map_size or request.position.y > settings.max_map_size:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid position: coordinates exceed maximum map size ({settings.max_map_size}x{settings.max_map_size})"
            )

        logger.info(f"Registering NPC: {request.npc_id} ({request.name})")

        # Generate agent ID
        agent_id = f"agent_{request.npc_id}_{uuid.uuid4().hex[:8]}"

        # Prepare NPC state
        npc_state = {
            "npc_id": request.npc_id,
            "name": request.name,
            "class": request.npc_class,
            "level": str(request.level),
            "map": request.position.map,
            "x": str(request.position.x),
            "y": str(request.position.y),
            "agent_id": agent_id,
            "registered_at": datetime.utcnow().isoformat(),
            "status": "active",
        }
        
        # Add personality if provided
        if request.personality:
            npc_state.update({
                "openness": str(request.personality.openness),
                "conscientiousness": str(request.personality.conscientiousness),
                "extraversion": str(request.personality.extraversion),
                "agreeableness": str(request.personality.agreeableness),
                "neuroticism": str(request.personality.neuroticism),
                "moral_alignment": request.personality.moral_alignment,
            })
        
        # Store in database
        await db.set_npc_state(request.npc_id, npc_state)
        
        logger.info(f"NPC {request.npc_id} registered successfully with agent {agent_id}")
        
        return NPCRegisterResponse(
            status="success",
            agent_id=agent_id,
            npc_id=request.npc_id,
            message=f"NPC {request.name} registered successfully"
        )
        
    except Exception as e:
        logger.error(f"Error registering NPC {request.npc_id}: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to register NPC: {str(e)}"
        )


@router.post("/event", response_model=NPCEventResponse)
async def send_npc_event(request: NPCEventRequest):
    """
    Send a game event to the AI service for processing
    
    Events are queued for asynchronous processing by NPC agents.
    """
    try:
        logger.info(f"Received event for NPC {request.npc_id}: {request.event_type}")
        
        # Generate event ID
        event_id = f"event_{uuid.uuid4().hex}"
        
        # Prepare event data
        event_data = {
            "event_id": event_id,
            "npc_id": request.npc_id,
            "event_type": request.event_type,
            "event_data": request.event_data,
            "timestamp": request.timestamp.isoformat(),
            "context": request.context,
        }
        
        # Push to event queue (priority based on event type)
        priority_map = {
            "combat": 10,
            "social": 5,
            "economic": 3,
            "environmental": 1,
        }
        priority = priority_map.get(request.event_type, 5)
        
        await db.push_event(event_id, event_data, priority)
        
        logger.info(f"Event {event_id} queued with priority {priority}")
        
        return NPCEventResponse(
            status="queued",
            event_id=event_id,
            npc_id=request.npc_id,
            message=f"Event queued for processing",
            estimated_processing_time=5.0
        )
        
    except Exception as e:
        logger.error(f"Error processing event for NPC {request.npc_id}: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process event: {str(e)}"
        )


@router.get("/{npc_id}/action", response_model=NPCActionResponse)
async def get_npc_action(npc_id: str):
    """
    Get the next action for an NPC using AI Decision Agent

    Returns the next action the NPC should execute based on its current state, personality, and goals.
    """
    try:
        logger.info(f"Getting action for NPC {npc_id}")

        # Get NPC state
        npc_state = await db.get_npc_state(npc_id)

        if not npc_state:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"NPC {npc_id} not found"
            )

        # Get orchestrator and request action decision
        from ..agents.orchestrator import get_orchestrator
        from ..agents.base_agent import AgentContext
        from ..models.npc import NPCPersonality

        # Build agent context
        personality_data = npc_state.get("personality", {})
        personality = NPCPersonality(**personality_data) if personality_data else NPCPersonality()

        agent_context = AgentContext(
            npc_id=npc_id,
            npc_name=npc_state.get("name", "Unknown"),
            personality=personality,
            current_state=npc_state,
            recent_events=npc_state.get("recent_events", [])
        )

        # Get orchestrator and request action decision
        orchestrator = get_orchestrator()
        result = await orchestrator.handle_npc_action_decision(agent_context)

        # Extract action from result
        action_data = result.get("action", {})

        action = NPCAction(
            action_type=action_data.get("action_type", "idle"),
            action_data=action_data.get("action_data", {"duration": 5}),
            priority=action_data.get("priority", 1),
            execution_params=action_data.get("execution_params", {})
        )

        logger.info(f"AI decision for NPC {npc_id}: {action.action_type}")

        return NPCActionResponse(
            npc_id=npc_id,
            action=action,
            state_update=result.get("state_update"),
            message="Action retrieved successfully from AI Decision Agent"
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error getting action for NPC {npc_id}: {e}")
        # Fallback to idle action on error
        logger.warning(f"Falling back to idle action for NPC {npc_id}")
        return NPCActionResponse(
            npc_id=npc_id,
            action=NPCAction(
                action_type="idle",
                action_data={"duration": 5},
                priority=1,
                execution_params={}
            ),
            state_update=None,
            message=f"Fallback action due to error: {str(e)}"
        )


@router.post("/{npc_id}/execute-action")
async def execute_npc_action(npc_id: str, action: NPCAction):
    """
    Execute an NPC action and translate it to Bridge Layer commands

    Validates action parameters before execution.

    This endpoint is called by the Bridge Layer to get executable commands
    for an NPC action decided by the AI.
    """
    try:
        # Validate action type
        valid_action_types = ["move", "talk", "emote", "idle", "attack", "trade", "cast_skill"]
        if action.action_type not in valid_action_types:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail=f"Invalid action type: {action.action_type}. Must be one of {valid_action_types}"
            )

        # Validate movement actions
        if action.action_type == "move":
            action_data = action.action_data or {}
            target_x = action_data.get("target_x")
            target_y = action_data.get("target_y")

            if target_x is None or target_y is None:
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail="Move action requires target_x and target_y coordinates"
                )

            # Validate coordinates are within reasonable bounds
            if not (0 <= target_x <= settings.max_map_size and 0 <= target_y <= settings.max_map_size):
                raise HTTPException(
                    status_code=status.HTTP_400_BAD_REQUEST,
                    detail=f"Invalid coordinates: ({target_x}, {target_y}). Must be within 0-{settings.max_map_size} range"
                )

            # Validate movement distance if current position is available
            npc_state = await db.get_npc_state(npc_id)
            if npc_state:
                current_x = int(npc_state.get("x", 0))
                current_y = int(npc_state.get("y", 0))
                distance = abs(target_x - current_x) + abs(target_y - current_y)

                # Limit maximum movement distance per action (prevent teleporting)
                if distance > settings.max_movement_distance:
                    raise HTTPException(
                        status_code=status.HTTP_400_BAD_REQUEST,
                        detail=f"Movement distance ({distance}) exceeds maximum ({settings.max_movement_distance})"
                    )

        logger.info(f"Executing action for NPC {npc_id}: {action.action_type}")

        from ..utils.bridge_commands import translate_action_to_bridge_command

        # Translate action to bridge command
        bridge_command = translate_action_to_bridge_command(action, npc_id)

        logger.info(f"Bridge command for NPC {npc_id}: {bridge_command.get('command_type')}")

        return {
            "npc_id": npc_id,
            "command": bridge_command,
            "status": "ready_for_execution",
            "message": "Action translated to bridge command successfully"
        }

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error executing action for NPC {npc_id}: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to execute NPC action: {str(e)}"
        )

