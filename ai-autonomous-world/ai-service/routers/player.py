"""
Player interaction API endpoints
"""

from fastapi import APIRouter, HTTPException, status
from loguru import logger
from datetime import datetime

from ..models.player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    NPCResponse,
)
from ..models.npc import NPCPersonality
from ..database import db
from ..llm import get_llm_provider
from ..agents.base_agent import AgentContext
from ..agents.orchestrator import AgentOrchestrator
from ..config import settings

try:
    from memori import Memori
    MEMORI_AVAILABLE = True
except ImportError:
    logger.warning("Memori SDK not available")
    MEMORI_AVAILABLE = False

router = APIRouter(prefix="/ai/player", tags=["player"])

# Global orchestrator instance (will be initialized on first use)
_orchestrator = None


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
        world_state = await db.get_world_state()

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

        return PlayerInteractionResponse(
            npc_id=request.npc_id,
            player_id=request.player_id,
            response=npc_response,
            npc_state_update=npc_state,
            relationship_change=relationship_change.get("change", 0)
        )

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error handling player interaction: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to process interaction: {str(e)}"
        )

