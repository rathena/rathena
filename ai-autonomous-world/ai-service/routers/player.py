"""
Player interaction API endpoints
"""

from fastapi import APIRouter, HTTPException, status, Request
from loguru import logger
from datetime import datetime
from typing import Dict, Any

from models.player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    NPCResponse,
)
from models.npc import NPCPersonality
from database import db
from llm import get_llm_provider
from agents.base_agent import AgentContext
from agents.orchestrator import AgentOrchestrator
from config import settings

try:
    from memori import Memori
    MEMORI_AVAILABLE = True
except ImportError:
    logger.warning("Memori SDK not available")
    MEMORI_AVAILABLE = False

router = APIRouter(prefix="/ai/player", tags=["player"])

# Global orchestrator instance (will be initialized on first use)
_orchestrator = None


@router.post("/interaction/simple")
async def handle_simple_interaction(request: Request):
    """
    Simplified player-NPC interaction endpoint that returns plain text dialogue

    This endpoint is designed for game clients that cannot parse JSON responses.
    It returns only the dialogue message as plain text.
    Accepts raw JSON without strict validation.
    """
    try:
        # Parse raw JSON body
        body = await request.json()

        npc_id = body.get("npc_id")
        player_id = body.get("player_id")
        interaction_type = body.get("interaction_type", "talk")
        context_data = body.get("context", {})

        logger.info(f"Simple interaction: Player {player_id} with NPC {npc_id}")
        logger.debug(f"Context data: {context_data}")

        # Get NPC state
        npc_state = await db.get_npc_state(npc_id)

        if not npc_state:
            return "Hello! I'm still getting to know this world."

        # Get orchestrator
        orchestrator = get_orchestrator()

        # Create context with flexible data
        context = AgentContext(
            npc_id=npc_id,
            player_id=player_id,
            interaction_type=interaction_type,
            context=context_data,
            npc_state=npc_state
        )

        # Process interaction
        result = await orchestrator.process_interaction(context)

        # Extract dialogue message from response
        if result and "response" in result:
            response_data = result["response"]
            if isinstance(response_data, dict) and "data" in response_data:
                data = response_data["data"]
                if isinstance(data, dict) and "message" in data:
                    return data["message"]

        # Fallback response
        return "..."

    except Exception as e:
        logger.error(f"Error in simple interaction: {e}", exc_info=True)
        return "..."


@router.post("/chat")
async def handle_chat_interaction(request: Request):
    """
    Chat-based player-NPC interaction with conversation history

    This endpoint maintains conversation context across multiple messages.
    Perfect for interactive dialogue systems.

    Request body:
    {
        "npc_id": "ai_explorer_002",
        "player_id": "150000",
        "player_name": "TestAdmin",
        "message": "Tell me about your adventures"
    }

    Returns: Plain text response from NPC
    """
    try:
        # Parse request
        body = await request.json()
        npc_id = body.get("npc_id")
        player_id = body.get("player_id")
        player_name = body.get("player_name", "Adventurer")
        message = body.get("message", "")

        logger.info(f"Chat: Player {player_name} ({player_id}) to NPC {npc_id}: {message}")

        # Get NPC state
        npc_state = await db.get_npc_state(npc_id)
        if not npc_state:
            return "I'm still getting to know this world. Please try again later."

        # Get conversation history
        conversation_key = f"conversation:{npc_id}:{player_id}"
        history = await db.get_conversation_history(conversation_key)

        # Add player message to history
        if message:
            history.append({
                "role": "player",
                "name": player_name,
                "message": message,
                "timestamp": datetime.utcnow().isoformat()
            })

        # Generate simple AI response based on NPC personality
        # Convert bytes to strings if needed
        def get_str(d, key, default=""):
            val = d.get(key, default)
            if isinstance(val, bytes):
                return val.decode('utf-8')
            return str(val) if val else default

        npc_name = get_str(npc_state, "name", "NPC")
        npc_class = get_str(npc_state, "class", "npc")

        import sys
        print(f"DEBUG: NPC Name={npc_name}, Class={npc_class}, State keys={list(npc_state.keys())}", file=sys.stderr, flush=True)
        logger.info(f"Chat with NPC: {npc_name} (class: {npc_class})")

        # Build conversation context for LLM
        conversation_context = ""
        if history:
            recent_history = history[-5:]  # Last 5 messages
            for msg in recent_history:
                role = msg.get("role", "unknown")
                msg_text = msg.get("message", "")
                if role == "player":
                    conversation_context += f"Player: {msg_text}\n"
                elif role == "npc":
                    conversation_context += f"{npc_name}: {msg_text}\n"

        # Create personality description
        personality_desc = f"{npc_name} is a {npc_class}"

        def get_float(d, key, default=0.5):
            val = d.get(key, default)
            if isinstance(val, bytes):
                val = val.decode('utf-8')
            try:
                return float(val)
            except (ValueError, TypeError):
                return default

        openness = get_float(npc_state, "openness", 0.5)
        extraversion = get_float(npc_state, "extraversion", 0.5)

        if openness > 0.7:
            personality_desc += ", curious and adventurous"
        elif openness < 0.3:
            personality_desc += ", traditional and cautious"

        if extraversion > 0.7:
            personality_desc += ", outgoing and energetic"
        elif extraversion < 0.3:
            personality_desc += ", reserved and quiet"

        # Generate simple response based on NPC personality
        # TODO: Replace with actual LLM integration once Azure OpenAI is configured

        # Simple response logic based on message content
        message_lower = message.lower()

        # Check for goodbye first (most specific)
        if "bye" in message_lower or "goodbye" in message_lower or "farewell" in message_lower:
            if npc_class == "explorer":
                response_text = "Safe travels, friend! May your adventures be grand and your treasures plentiful!"
            elif npc_class == "guard":
                response_text = "Move along then. Stay out of trouble."
            else:
                response_text = "Farewell, traveler. Come back anytime!"

        # Check for greetings
        elif "hello" in message_lower or "hi" in message_lower or "greet" in message_lower:
            if npc_class == "explorer":
                response_text = f"Greetings, {player_name}! I'm {npc_name}, and I've just returned from the most amazing expedition! Have you ever ventured beyond the city walls?"
            elif npc_class == "guard":
                response_text = f"Halt! State your business, {player_name}. I'm {npc_name}, and I keep watch over this city. What brings you here?"
            else:
                response_text = f"Hello, {player_name}! I'm {npc_name}. How can I help you today?"

        # Check for location/where questions (before "explore" to avoid false matches)
        elif ("where" in message_lower and ("go" in message_lower or "explore" in message_lower or "next" in message_lower)) or "location" in message_lower:
            if npc_class == "explorer":
                response_text = "If you're looking for adventure, I'd recommend the Culvert sewers for beginners, or if you're feeling brave, try the Orc Dungeon! Just be careful - those orcs are tougher than they look."
            else:
                response_text = f"We're currently in Prontera, the capital city. It's a bustling place with many opportunities for adventurers like yourself!"

        # Check for tips/advice
        elif "tip" in message_lower or "advice" in message_lower:
            if npc_class == "explorer":
                response_text = "My best tip? Always bring plenty of healing potions, and never underestimate a monster's strength. Oh, and make friends with the local merchants - they often have valuable information!"
            elif npc_class == "guard":
                response_text = "Stay alert, keep your weapon ready, and never trust strangers too quickly. The world can be dangerous for the unprepared."
            else:
                response_text = "I'd say the best advice is to be prepared for anything. This world is full of surprises!"

        # Check for adventure/exploration talk
        elif "adventure" in message_lower or "explore" in message_lower or "expedition" in message_lower:
            if npc_class == "explorer":
                response_text = "Oh, you want to hear about my adventures? Well, just last week I discovered an ancient ruin in the Sograt Desert! The treasures there were magnificent, though the monsters were quite fierce."
            else:
                response_text = "Adventures, you say? I'm not much of an adventurer myself, but I've heard tales of brave souls exploring dangerous dungeons."

        else:
            # Default responses based on personality
            if extraversion > 0.7:
                responses = [
                    "That's fascinating! Tell me more about that!",
                    "Oh, I love talking about these things! What else would you like to know?",
                    "Interesting perspective! I hadn't thought of it that way before."
                ]
            elif extraversion < 0.3:
                responses = [
                    "I see. That's... interesting.",
                    "Hmm. I'll have to think about that.",
                    "Perhaps. I'm not sure what to say about that."
                ]
            else:
                responses = [
                    "That's an interesting point. What made you think of that?",
                    "I understand. Is there anything else you'd like to discuss?",
                    "Fair enough. What else is on your mind?"
                ]

            import random
            response_text = random.choice(responses)

        # Add NPC response to history
        history.append({
            "role": "npc",
            "name": npc_name,
            "message": response_text,
            "timestamp": datetime.utcnow().isoformat()
        })

        # Save conversation history (10 minute TTL)
        await db.save_conversation_history(conversation_key, history, ttl=600)

        return response_text

    except Exception as e:
        logger.error(f"Error in chat interaction: {e}", exc_info=True)
        return "I'm having trouble understanding right now. Could you try again?"


def get_orchestrator() -> AgentOrchestrator:
    """Get or create the global orchestrator instance"""
    global _orchestrator
    if _orchestrator is None:
        llm = get_llm_provider()
        config = {
            "dialogue_agent": {"verbose": False},
            "decision_agent": {"verbose": False},
            "memory_agent": {"verbose": False},
            "world_agent": {"verbose": False},
            "verbose": False
        }

        # Initialize Memori client if available and enabled
        memori_client = None
        if MEMORI_AVAILABLE and settings.memori_enabled:
            try:
                # Determine database connection string
                # Use Redis/DragonflyDB connection for Memori storage
                db_connect = f"redis://{settings.redis_host}:{settings.redis_port}/{settings.redis_db}"
                if settings.redis_password:
                    db_connect = f"redis://:{settings.redis_password}@{settings.redis_host}:{settings.redis_port}/{settings.redis_db}"

                # Initialize Memori client
                memori_client = Memori(
                    database_connect=db_connect,
                    api_key=settings.memori_api_key or settings.openai_api_key,  # Use OpenAI key as fallback
                    user_id="ai_service",  # Service-level user ID
                    session_id="npc_memories",  # Session for NPC memories
                    verbose=settings.debug,
                    schema_init=True  # Initialize database schema
                )
                logger.info("Memori client initialized successfully")
            except Exception as e:
                logger.error(f"Failed to initialize Memori client: {e}")
                logger.info("Continuing without Memori SDK - will use DragonflyDB fallback")
                memori_client = None
        else:
            if not MEMORI_AVAILABLE:
                logger.info("Memori SDK not available - using DragonflyDB fallback")
            elif not settings.memori_enabled:
                logger.info("Memori SDK disabled in configuration - using DragonflyDB fallback")

        _orchestrator = AgentOrchestrator(
            llm_provider=llm,
            config=config,
            memori_client=memori_client
        )
        logger.info("Agent Orchestrator initialized for player interactions")
    return _orchestrator


@router.post("/interaction", response_model=PlayerInteractionResponse)
async def handle_player_interaction(request: PlayerInteractionRequest):
    """
    Handle player-NPC interaction through the AI service using Agent Orchestrator

    Processes player interactions and generates NPC responses using multiple specialized agents.
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

        # Build NPC personality from state
        personality = NPCPersonality(
            openness=npc_state.get("openness", 0.5),
            conscientiousness=npc_state.get("conscientiousness", 0.5),
            extraversion=npc_state.get("extraversion", 0.5),
            agreeableness=npc_state.get("agreeableness", 0.5),
            neuroticism=npc_state.get("neuroticism", 0.5),
            moral_alignment=npc_state.get("moral_alignment", "true_neutral")
        )

        # Get world state
        world_state = await db.get_world_state("global") or {}

        # Build agent context
        agent_context = AgentContext(
            npc_id=request.npc_id,
            npc_name=npc_state.get("name", "Unknown"),
            personality=personality,
            current_state={
                "npc_class": npc_state.get("class", "generic"),
                "player_id": request.player_id,
                "player_name": request.context.player_name,
                "player_class": request.context.player_class,
                "player_level": request.context.player_level,
                "player_message": request.message or "",
                "interaction_type": request.interaction_type,
                "location": request.context.location,
                "time_of_day": request.context.time_of_day,
                "weather": request.context.weather
            },
            world_state=world_state or {},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        # Get orchestrator and handle interaction
        orchestrator = get_orchestrator()
        result = await orchestrator.handle_player_interaction(
            npc_context=agent_context,
            player_message=request.message or "",
            interaction_type=request.interaction_type
        )

        # Extract dialogue from result
        dialogue = result.get("dialogue", {})
        relationship_change = result.get("relationship_change", {})

        # Build NPC response
        npc_response = NPCResponse(
            action="dialogue",
            data={
                "text": dialogue.get("text", "..."),
                "speaker": dialogue.get("speaker", npc_state.get("name", "Unknown"))
            },
            emotion=dialogue.get("emotion", "neutral"),
            next_actions=dialogue.get("next_actions", ["talk", "end_conversation"])
        )

        # Update NPC state with last interaction
        npc_state["last_interaction"] = datetime.utcnow().isoformat()
        npc_state["last_player"] = request.player_id
        await db.set_npc_state(request.npc_id, npc_state)

        logger.info(f"Interaction completed: {request.npc_id} <-> {request.player_id}")

        # Format relationship_change as dict (faction_id -> change value)
        # If relationship_change is already a dict, use it; otherwise create proper format
        if isinstance(relationship_change, dict) and relationship_change:
            formatted_relationship_change = relationship_change
        else:
            # Default: no relationship change
            formatted_relationship_change = {}

        return PlayerInteractionResponse(
            npc_id=request.npc_id,
            player_id=request.player_id,
            response=npc_response,
            npc_state_update=npc_state,
            relationship_change=formatted_relationship_change
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error handling player interaction: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process interaction: {str(e)}"
        )

