"""
NPC API endpoints
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
import uuid
from datetime import datetime

from ..models.npc import (
    NPCRegisterRequest,
    NPCRegisterResponse,
    NPCEventRequest,
    NPCEventResponse,
    NPCActionResponse,
    NPCAction,
)
from ..database import db

router = APIRouter(prefix="/ai/npc", tags=["npc"])


@router.post("/register", response_model=NPCRegisterResponse)
async def register_npc(request: NPCRegisterRequest):
    """
    Register an NPC with the AI service
    
    Creates an AI agent for the NPC and stores initial state in database.
    """
    try:
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
    Get the next action for an NPC
    
    Returns the next action the NPC should execute based on its current state and goals.
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
        
        # For now, return a simple idle action
        # In full implementation, this would query the NPC agent for decision
        action = NPCAction(
            action_type="idle",
            action_data={"duration": 5},
            priority=1,
            execution_params={}
        )
        
        logger.info(f"Returning action for NPC {npc_id}: {action.action_type}")
        
        return NPCActionResponse(
            npc_id=npc_id,
            action=action,
            state_update=None,
            message="Action retrieved successfully"
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error getting action for NPC {npc_id}: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get NPC action: {str(e)}"
        )

