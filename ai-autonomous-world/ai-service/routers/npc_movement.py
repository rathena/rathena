"""
NPC Movement Router

Handles NPC movement execution requests from rAthena scripts.
Provides endpoints for AI-driven movement decisions and execution.
"""

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from typing import Optional, Dict, Any
import logging

from ..database import db
from ..agents.decision_agent import DecisionAgent, AgentContext
from ..models.npc_personality import NPCPersonality
from ..llm.factory import get_llm_provider
from ..config import config

logger = logging.getLogger(__name__)

router = APIRouter(prefix="/npc/movement", tags=["npc_movement"])


class MovementDecisionRequest(BaseModel):
    """Request for NPC movement decision"""
    npc_id: str
    current_position: Dict[str, Any]  # {"map": "prontera", "x": 150, "y": 180}
    context: Optional[str] = None  # Additional context (e.g., "after_interaction")
    player_position: Optional[Dict[str, Any]] = None  # Player who triggered movement


class MovementDecisionResponse(BaseModel):
    """Response with movement decision"""
    success: bool
    npc_id: str
    should_move: bool
    target_position: Optional[Dict[str, int]] = None  # {"x": 155, "y": 185}
    reasoning: Optional[str] = None
    error: Optional[str] = None


@router.post("/decide", response_model=MovementDecisionResponse)
async def decide_movement(request: MovementDecisionRequest):
    """
    Get AI decision on whether and where NPC should move.
    
    This endpoint is called by rAthena scripts to get movement decisions
    from the AI DecisionAgent based on NPC personality, context, and state.
    """
    try:
        logger.info(f"Movement decision request for NPC: {request.npc_id}")
        
        # Get NPC state from DragonflyDB
        npc_state = db.get_npc_state(request.npc_id)
        if not npc_state:
            logger.warning(f"NPC {request.npc_id} not found in database")
            return MovementDecisionResponse(
                success=False,
                npc_id=request.npc_id,
                should_move=False,
                error="NPC not found in database"
            )
        
        # Get movement configuration
        movement_mode = npc_state.get("movement_mode", "disabled")
        if movement_mode == "disabled":
            logger.info(f"Movement disabled for NPC {request.npc_id}")
            return MovementDecisionResponse(
                success=True,
                npc_id=request.npc_id,
                should_move=False,
                reasoning="Movement is disabled for this NPC"
            )
        
        # Build personality from NPC state
        personality = NPCPersonality(
            openness=float(npc_state.get("openness", 0.5)),
            conscientiousness=float(npc_state.get("conscientiousness", 0.5)),
            extraversion=float(npc_state.get("extraversion", 0.5)),
            agreeableness=float(npc_state.get("agreeableness", 0.5)),
            neuroticism=float(npc_state.get("neuroticism", 0.5)),
            moral_alignment=npc_state.get("moral_alignment", "neutral")
        )
        
        # Build agent context
        agent_context = AgentContext(
            npc_id=request.npc_id,
            npc_name=npc_state.get("name", request.npc_id),
            personality=personality,
            current_state={
                "position": request.current_position,
                "movement_mode": movement_mode,
                "movement_radius": int(npc_state.get("movement_radius", 5)),
                "spawn_position": npc_state.get("spawn_position", request.current_position)
            },
            world_state={
                "context": request.context or "idle",
                "player_nearby": request.player_position is not None
            },
            recent_events=[],
            memory_context={},
            timestamp=None
        )
        
        # Get LLM provider and create DecisionAgent
        llm_provider = get_llm_provider(config)
        decision_agent = DecisionAgent(
            agent_id=f"movement_{request.npc_id}",
            llm_provider=llm_provider,
            config=config
        )
        
        # Get movement decision
        decision_prompt = f"""
Based on the NPC's personality and current context, decide if the NPC should move and where.

NPC: {npc_state.get('name')}
Current Position: {request.current_position}
Movement Mode: {movement_mode}
Context: {request.context or 'idle'}

Personality Traits:
- Openness: {personality.openness}
- Conscientiousness: {personality.conscientiousness}
- Extraversion: {personality.extraversion}

Decide:
1. Should the NPC move? (yes/no)
2. If yes, provide target coordinates within allowed boundaries
3. Provide reasoning for the decision

Respond in JSON format:
{{
    "should_move": true/false,
    "target_x": <number>,
    "target_y": <number>,
    "reasoning": "<explanation>"
}}
"""

        # Process decision
        response = decision_agent.process(agent_context, decision_prompt)

        if not response.success:
            logger.error(f"Decision agent failed: {response.data}")
            return MovementDecisionResponse(
                success=False,
                npc_id=request.npc_id,
                should_move=False,
                error="Failed to get movement decision from AI"
            )

        # Parse decision
        decision_data = response.data
        if isinstance(decision_data, str):
            import json
            try:
                decision_data = json.loads(decision_data)
            except json.JSONDecodeError:
                logger.error(f"Failed to parse decision JSON: {decision_data}")
                return MovementDecisionResponse(
                    success=False,
                    npc_id=request.npc_id,
                    should_move=False,
                    error="Invalid decision format from AI"
                )

        should_move = decision_data.get("should_move", False)
        target_position = None

        if should_move:
            target_x = decision_data.get("target_x")
            target_y = decision_data.get("target_y")

            if target_x is not None and target_y is not None:
                # Validate movement boundaries
                if movement_mode == "radius_restricted":
                    spawn_pos = npc_state.get("spawn_position", request.current_position)
                    spawn_x = spawn_pos.get("x", request.current_position["x"])
                    spawn_y = spawn_pos.get("y", request.current_position["y"])
                    radius = int(npc_state.get("movement_radius", 5))

                    # Check if target is within radius
                    distance = ((target_x - spawn_x) ** 2 + (target_y - spawn_y) ** 2) ** 0.5
                    if distance > radius:
                        logger.warning(f"Target position ({target_x}, {target_y}) exceeds radius {radius} for NPC {request.npc_id}")
                        should_move = False
                    else:
                        target_position = {"x": int(target_x), "y": int(target_y)}
                else:
                    target_position = {"x": int(target_x), "y": int(target_y)}

        return MovementDecisionResponse(
            success=True,
            npc_id=request.npc_id,
            should_move=should_move,
            target_position=target_position,
            reasoning=decision_data.get("reasoning", "No reasoning provided")
        )

    except Exception as e:
        logger.exception(f"Error in movement decision for NPC {request.npc_id}: {e}")
        return MovementDecisionResponse(
            success=False,
            npc_id=request.npc_id,
            should_move=False,
            error=str(e)
        )


