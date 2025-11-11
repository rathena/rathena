"""
NPC Movement Router

Handles NPC movement execution requests from rAthena scripts.
Provides endpoints for AI-driven movement decisions and execution.
"""

from fastapi import APIRouter, HTTPException
from pydantic import BaseModel
from typing import Optional, Dict, Any
import logging

from database import db
from agents.decision_agent import DecisionAgent, AgentContext
from models.npc import NPCPersonality
from llm.factory import get_llm_provider
from config import settings
from utils.walkability import (
    is_position_walkable,
    find_nearest_walkable_position,
    get_walkable_positions_in_radius
)
import json
import re

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
        npc_state = await db.get_npc_state(request.npc_id)
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
        
        # Get LLM provider for direct use
        llm_provider = get_llm_provider()  # Uses default provider from settings
        
        # Check if this is aggressive movement context
        is_aggressive = request.context == "aggressive_movement"
        movement_probability = float(npc_state.get("movement_probability", 0.5))
        min_distance = int(npc_state.get("min_distance", 3))
        max_distance = int(npc_state.get("max_distance", 5))

        # Get spawn position for radius calculation
        spawn_x = int(npc_state.get('spawn_x', npc_state.get('x', 150)))
        spawn_y = int(npc_state.get('spawn_y', npc_state.get('y', 180)))
        current_x = int(request.current_position.get('x', spawn_x))
        current_y = int(request.current_position.get('y', spawn_y))
        radius = int(npc_state.get('movement_radius', 10))

        # Calculate valid coordinate ranges
        min_x = spawn_x - radius
        max_x = spawn_x + radius
        min_y = spawn_y - radius
        max_y = spawn_y + radius

        # Get movement decision
        if is_aggressive:
            decision_prompt = f"""
You are controlling an EXTREMELY RESTLESS and WANDERING NPC with maximum curiosity and energy.

NPC: {npc_state.get('name')}
Current Position: ({current_x}, {current_y})
Spawn Point (CENTER): ({spawn_x}, {spawn_y})
Movement Mode: {movement_mode}
Context: AGGRESSIVE MOVEMENT (high frequency, large distances)

Personality Traits (EXTREME):
- Openness: {personality.openness} (MAXIMUM - insatiably curious)
- Conscientiousness: {personality.conscientiousness} (LOW - impulsive, restless)
- Extraversion: {personality.extraversion} (MAXIMUM - constantly seeking stimulation)
- Neuroticism: {personality.neuroticism} (MINIMUM - fearless, adventurous)

Movement Configuration:
- Movement Probability: {movement_probability * 100}% (should move {movement_probability * 100}% of the time)
- Minimum Distance: {min_distance} tiles from current position
- Maximum Distance: {max_distance} tiles from current position
- Radius Limit: {radius} tiles from spawn point ({spawn_x}, {spawn_y})

VALID COORDINATE RANGES (MUST STAY WITHIN):
- X: {min_x} to {max_x} (spawn_x ± {radius})
- Y: {min_y} to {max_y} (spawn_y ± {radius})

CRITICAL RULES:
1. Target X MUST be between {min_x} and {max_x}
2. Target Y MUST be between {min_y} and {max_y}
3. Distance from spawn ({spawn_x}, {spawn_y}) MUST be ≤ {radius} tiles
4. Distance from current position ({current_x}, {current_y}) should be {min_distance}-{max_distance} tiles
5. Move {movement_probability * 100}% of the time (be VERY aggressive)
6. AVOID the fountain area (145-161, 175-191) - it's not walkable!

WALKABLE AREAS TO PREFER:
- North of fountain: X: {min_x}-{max_x}, Y: 192-{max_y}
- South of fountain: X: {min_x}-{max_x}, Y: {min_y}-174
- East of fountain: X: 162-{max_x}, Y: {min_y}-{max_y}
- West of fountain: X: {min_x}-144, Y: {min_y}-{max_y}

EXAMPLE VALID TARGETS (for spawn at {spawn_x}, {spawn_y} with radius {radius}):
- ({spawn_x + 5}, {spawn_y + 5}) - 7.1 tiles from spawn ✓
- ({spawn_x - 7}, {spawn_y}) - 7 tiles from spawn ✓
- ({spawn_x}, {spawn_y + 8}) - 8 tiles from spawn ✓

Decide:
1. Should the NPC move? (almost always YES)
2. Target coordinates (MUST be within {min_x}-{max_x}, {min_y}-{max_y})
3. Brief reasoning

Respond in JSON format:
{{
    "should_move": true,
    "target_x": <number between {min_x} and {max_x}>,
    "target_y": <number between {min_y} and {max_y}>,
    "reasoning": "<brief explanation>"
}}
"""
        else:
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

        # Get decision from LLM
        try:
            llm_response = await llm_provider.generate(decision_prompt)
            decision_text = llm_response.content

            if not decision_text:
                logger.error(f"Empty response from LLM")
                return MovementDecisionResponse(
                    success=False,
                    npc_id=request.npc_id,
                    should_move=False,
                    error="Empty response from AI"
                )

            logger.debug(f"LLM response for {request.npc_id}: {decision_text[:200]}...")
        except Exception as e:
            logger.error(f"LLM generation failed: {e}")
            return MovementDecisionResponse(
                success=False,
                npc_id=request.npc_id,
                should_move=False,
                error=f"AI generation failed: {str(e)}"
            )

        # Parse decision from LLM response
        import json
        import re

        # Extract JSON from response (handle markdown code blocks)
        json_match = re.search(r'```(?:json)?\s*(\{.*?\})\s*```', decision_text, re.DOTALL)
        if json_match:
            decision_text = json_match.group(1)

        try:
            decision_data = json.loads(decision_text)
        except json.JSONDecodeError:
            # Try to find JSON object in the text
            json_match = re.search(r'\{[^{}]*"should_move"[^{}]*\}', decision_text)
            if json_match:
                try:
                    decision_data = json.loads(json_match.group(0))
                except json.JSONDecodeError:
                    logger.error(f"Failed to parse decision JSON: {decision_text}")
                    return MovementDecisionResponse(
                        success=False,
                        npc_id=request.npc_id,
                        should_move=False,
                        error="Invalid decision format from AI"
                    )
            else:
                logger.error(f"No JSON found in response: {decision_text}")
                return MovementDecisionResponse(
                    success=False,
                    npc_id=request.npc_id,
                    should_move=False,
                    error="No JSON found in AI response"
                )

        should_move = decision_data.get("should_move", False)
        target_position = None

        if should_move:
            target_x = decision_data.get("target_x")
            target_y = decision_data.get("target_y")

            if target_x is not None and target_y is not None:
                # Validate movement boundaries
                if movement_mode == "radius_restricted":
                    # Get spawn position from individual fields (spawn_x, spawn_y)
                    validation_spawn_x = int(npc_state.get("spawn_x", npc_state.get("x", request.current_position["x"])))
                    validation_spawn_y = int(npc_state.get("spawn_y", npc_state.get("y", request.current_position["y"])))
                    validation_radius = int(npc_state.get("movement_radius", 5))

                    # Check if target is within radius
                    distance_from_spawn = ((target_x - validation_spawn_x) ** 2 + (target_y - validation_spawn_y) ** 2) ** 0.5
                    if distance_from_spawn > validation_radius:
                        logger.warning(f"Target position ({target_x}, {target_y}) exceeds radius {validation_radius} for NPC {request.npc_id} (distance: {distance_from_spawn:.1f}, spawn: {validation_spawn_x},{validation_spawn_y})")
                        should_move = False
                    else:
                        # Check walkability before proceeding
                        map_name = request.current_position.get("map", "prontera")
                        if not is_position_walkable(map_name, target_x, target_y):
                            logger.warning(f"Target position ({target_x}, {target_y}) is not walkable on {map_name} for NPC {request.npc_id}")

                            # Try to find a nearby walkable position
                            alternative = find_nearest_walkable_position(map_name, target_x, target_y, max_search_radius=3)
                            if alternative:
                                target_x, target_y = alternative
                                logger.info(f"Found alternative walkable position ({target_x}, {target_y}) for NPC {request.npc_id}")
                            else:
                                logger.warning(f"No walkable alternative found near ({target_x}, {target_y}) for NPC {request.npc_id}")
                                should_move = False

                        # If still valid, proceed with movement validation
                        if should_move:
                            # For aggressive movement, validate minimum distance
                            if is_aggressive:
                                current_x = request.current_position["x"]
                                current_y = request.current_position["y"]
                                distance_from_current = ((target_x - current_x) ** 2 + (target_y - current_y) ** 2) ** 0.5

                                if distance_from_current < min_distance:
                                    logger.info(f"Aggressive movement: target too close ({distance_from_current:.1f} < {min_distance}), rejecting")
                                    should_move = False
                                else:
                                    target_position = {"x": int(target_x), "y": int(target_y)}
                                    logger.info(f"Aggressive movement: moving {distance_from_current:.1f} tiles from ({current_x},{current_y}) to ({target_x},{target_y})")
                            else:
                                target_position = {"x": int(target_x), "y": int(target_y)}
                else:
                    # For non-radius-restricted movement, still check walkability
                    map_name = request.current_position.get("map", "prontera")
                    if not is_position_walkable(map_name, target_x, target_y):
                        logger.warning(f"Target position ({target_x}, {target_y}) is not walkable on {map_name} for NPC {request.npc_id}")

                        # Try to find a nearby walkable position
                        alternative = find_nearest_walkable_position(map_name, target_x, target_y, max_search_radius=3)
                        if alternative:
                            target_x, target_y = alternative
                            logger.info(f"Found alternative walkable position ({target_x}, {target_y}) for NPC {request.npc_id}")
                            target_position = {"x": int(target_x), "y": int(target_y)}
                        else:
                            logger.warning(f"No walkable alternative found near ({target_x}, {target_y}) for NPC {request.npc_id}")
                            should_move = False
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


