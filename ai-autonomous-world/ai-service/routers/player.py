"""
Player interaction API endpoints
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger

from ..models.player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    NPCResponse,
)
from ..database import db
from ..llm import get_llm_provider

router = APIRouter(prefix="/ai/player", tags=["player"])


@router.post("/interaction", response_model=PlayerInteractionResponse)
async def handle_player_interaction(request: PlayerInteractionRequest):
    """
    Handle player-NPC interaction through the AI service
    
    Processes player interactions and generates NPC responses using LLM.
    """
    try:
        logger.info(f"Player {request.player_id} interacting with NPC {request.npc_id}: {request.interaction_type}")
        
        # Get NPC state
        npc_state = await db.get_npc_state(request.npc_id)
        
        if not npc_state:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"NPC {request.npc_id} not found"
            )
        
        # Build context for LLM
        npc_name = npc_state.get("name", "Unknown")
        npc_class = npc_state.get("class", "Unknown")
        player_name = request.context.player_name
        player_class = request.context.player_class
        
        # For talk interactions, generate dialogue using LLM
        if request.interaction_type == "talk":
            try:
                # Get LLM provider
                llm = get_llm_provider()
                
                # Build prompt
                system_prompt = f"""You are {npc_name}, a {npc_class} in a fantasy MMORPG world.
Your personality traits:
- Moral alignment: {npc_state.get('moral_alignment', 'neutral')}
- Openness: {npc_state.get('openness', '0.5')}
- Conscientiousness: {npc_state.get('conscientiousness', '0.5')}
- Extraversion: {npc_state.get('extraversion', '0.5')}
- Agreeableness: {npc_state.get('agreeableness', '0.5')}

Respond to the player in character. Keep responses concise (1-3 sentences)."""
                
                user_prompt = f"""A {player_class} named {player_name} (level {request.context.player_level}) approaches you.
Location: {request.context.location.get('map', 'unknown')}
Time: {request.context.time_of_day}
Weather: {request.context.weather}

{f"Player says: {request.message}" if request.message else "Player greets you."}

Respond in character:"""
                
                # Generate response
                llm_response = await llm.generate(
                    prompt=user_prompt,
                    system_prompt=system_prompt,
                    temperature=0.8,
                    max_tokens=150
                )
                
                dialogue_text = llm_response.content.strip()
                
            except Exception as llm_error:
                logger.warning(f"LLM generation failed, using fallback: {llm_error}")
                dialogue_text = f"Greetings, {player_name}. How may I assist you?"
            
            # Create NPC response
            npc_response = NPCResponse(
                action="dialogue",
                data={
                    "text": dialogue_text,
                    "speaker": npc_name,
                },
                emotion="friendly",
                next_actions=["talk", "trade", "quest", "end_conversation"]
            )
            
        elif request.interaction_type == "trade":
            # Handle trade interaction
            npc_response = NPCResponse(
                action="trade",
                data={
                    "text": f"Welcome to my shop, {player_name}!",
                    "shop_id": f"shop_{request.npc_id}",
                },
                emotion="neutral",
                next_actions=["buy", "sell", "end_conversation"]
            )
            
        elif request.interaction_type == "quest":
            # Handle quest interaction
            npc_response = NPCResponse(
                action="quest_offer",
                data={
                    "text": f"I have a task for you, {player_name}.",
                    "quest_id": f"quest_{request.npc_id}_001",
                },
                emotion="neutral",
                next_actions=["accept_quest", "decline_quest", "end_conversation"]
            )
            
        else:
            # Default response
            npc_response = NPCResponse(
                action="dialogue",
                data={
                    "text": f"Hello, {player_name}.",
                    "speaker": npc_name,
                },
                emotion="neutral",
                next_actions=["talk", "end_conversation"]
            )
        
        # Update NPC state (last interaction)
        await db.set_npc_state(request.npc_id, {
            **npc_state,
            "last_interaction": request.timestamp.isoformat(),
            "last_player": request.player_id,
        })
        
        logger.info(f"Interaction processed for NPC {request.npc_id}")
        
        return PlayerInteractionResponse(
            npc_id=request.npc_id,
            player_id=request.player_id,
            response=npc_response,
            npc_state_update={"last_interaction": request.timestamp.isoformat()},
            relationship_change={request.player_id: 1}
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error handling player interaction: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process interaction: {str(e)}"
        )

