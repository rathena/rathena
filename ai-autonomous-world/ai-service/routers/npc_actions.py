"""
NPC Actions Router

Handles NPC action execution and validation including combat, crafting,
trading, and social actions.
"""

import logging
from typing import Optional, Dict, Any, List
from datetime import datetime
from uuid import uuid4

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field, field_validator

from .dependencies import (
    Orchestrator,
    CorrelationID,
    RateLimited,
    create_success_response,
    create_error_response
)

logger = logging.getLogger(__name__)

router = APIRouter(
    prefix="/api/npc/actions",
    tags=["npc-actions"],
    responses={
        401: {"description": "Unauthorized - Invalid API key"},
        429: {"description": "Too many requests"},
        500: {"description": "Internal server error"}
    }
)

# ============================================================================
# REQUEST/RESPONSE MODELS
# ============================================================================

class ActionRequest(BaseModel):
    """Request model for NPC action."""
    
    npc_id: str = Field(..., description="NPC performing action")
    action_type: str = Field(..., description="Action type")
    target_id: Optional[str] = Field(None, description="Target entity ID")
    parameters: Dict[str, Any] = Field(default_factory=dict, description="Action parameters")
    
    @field_validator('action_type')
    @classmethod
    def validate_action_type(cls, v: str) -> str:
        """Validate action type."""
        valid_actions = [
            'attack', 'defend', 'heal', 'cast_spell', 'craft', 'gather',
            'trade', 'speak', 'emote', 'use_item', 'move', 'rest'
        ]
        if v not in valid_actions:
            raise ValueError(f"Invalid action_type. Must be one of: {', '.join(valid_actions)}")
        return v


class ActionValidation(BaseModel):
    """Action validation result."""
    
    is_valid: bool
    can_execute: bool
    validation_errors: List[str] = Field(default_factory=list)
    warnings: List[str] = Field(default_factory=list)
    resource_cost: Dict[str, float] = Field(default_factory=dict)


class ActionResult(BaseModel):
    """Result of action execution."""
    
    action_id: str
    success: bool
    result_message: str
    effects: Dict[str, Any] = Field(default_factory=dict)
    timestamp: datetime = Field(default_factory=datetime.utcnow)


# ============================================================================
# ACTION VALIDATORS
# ============================================================================

def validate_attack_action(npc_id: str, target_id: str, params: dict) -> ActionValidation:
    """Validate attack action."""
    errors = []
    warnings = []
    
    if not target_id:
        errors.append("Attack requires valid target")
    
    # TODO: Check range, line of sight, weapon equipped, etc.
    
    return ActionValidation(
        is_valid=len(errors) == 0,
        can_execute=len(errors) == 0,
        validation_errors=errors,
        warnings=warnings,
        resource_cost={"stamina": 5.0}
    )


def validate_craft_action(npc_id: str, params: dict) -> ActionValidation:
    """Validate crafting action."""
    errors = []
    warnings = []
    
    if 'recipe_id' not in params:
        errors.append("Crafting requires recipe_id")
    
    # TODO: Check materials, skill level, tools
    
    return ActionValidation(
        is_valid=len(errors) == 0,
        can_execute=len(errors) == 0,
        validation_errors=errors,
        warnings=warnings,
        resource_cost={"mana": 10.0, "materials": 1.0}
    )


def validate_trade_action(npc_id: str, target_id: str, params: dict) -> ActionValidation:
    """Validate trade action."""
    errors = []
    warnings = []
    
    if not target_id:
        errors.append("Trade requires valid target")
    
    if 'items' not in params:
        errors.append("Trade requires items")
    
    return ActionValidation(
        is_valid=len(errors) == 0,
        can_execute=len(errors) == 0,
        validation_errors=errors,
        warnings=warnings,
        resource_cost={}
    )


# ============================================================================
# ENDPOINTS
# ============================================================================

@router.post(
    "/action",
    response_model=ActionResult,
    summary="Execute NPC Action",
    description="Execute and validate an NPC action"
)
async def execute_action(
    request: ActionRequest,
    orchestrator: Orchestrator,
    correlation_id: CorrelationID,
    _rate_check: RateLimited
) -> dict:
    """
    Execute an NPC action with validation.
    
    This endpoint:
    - Validates action prerequisites
    - Checks resource costs
    - Executes action through appropriate system
    - Returns results and effects
    
    Args:
        request: Action execution request
        orchestrator: Agent orchestrator
        correlation_id: Request tracking ID
        _rate_check: Rate limiting validation
    
    Returns:
        dict: Standardized response with action result
    
    Raises:
        HTTPException: 400 for invalid action, 500 for errors
    """
    try:
        logger.info(
            f"Executing action: npc={request.npc_id}, "
            f"type={request.action_type}, correlation_id={correlation_id}"
        )
        
        action_id = f"act-{uuid4().hex[:12]}"
        
        # Validate action based on type
        validation = None
        if request.action_type == 'attack':
            validation = validate_attack_action(
                request.npc_id,
                request.target_id,
                request.parameters
            )
        elif request.action_type == 'craft':
            validation = validate_craft_action(request.npc_id, request.parameters)
        elif request.action_type == 'trade':
            validation = validate_trade_action(
                request.npc_id,
                request.target_id,
                request.parameters
            )
        else:
            # Default validation
            validation = ActionValidation(is_valid=True, can_execute=True)
        
        if not validation.can_execute:
            raise ValueError(f"Action validation failed: {', '.join(validation.validation_errors)}")
        
        # TODO: Execute action through orchestrator
        # result = await orchestrator.action_manager.execute(request)
        
        response_data = ActionResult(
            action_id=action_id,
            success=True,
            result_message=f"Action {request.action_type} executed successfully",
            effects={
                "damage_dealt": 50 if request.action_type == 'attack' else 0,
                "resources_used": validation.resource_cost
            }
        )
        
        logger.info(f"Action executed: {action_id}")
        
        return create_success_response(
            data=response_data.model_dump(),
            message="Action executed",
            request_id=correlation_id
        )
        
    except ValueError as e:
        logger.warning(f"Validation error: {str(e)}")
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=create_error_response(
                code="VALIDATION_ERROR",
                message=str(e),
                request_id=correlation_id
            )
        )
    
    except Exception as e:
        logger.error(f"Error executing action: {str(e)}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=create_error_response(
                code="INTERNAL_ERROR",
                message="Failed to execute action",
                request_id=correlation_id
            )
        )