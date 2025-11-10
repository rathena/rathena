"""
Player interaction API endpoints
"""

from fastapi import APIRouter, HTTPException, status, Request
from fastapi.responses import PlainTextResponse
from loguru import logger
from datetime import datetime
from typing import Dict, Any
import json

from ai_service.models.player import (
    PlayerInteractionRequest,
    PlayerInteractionResponse,
    NPCResponse,
)
from ai_service.models.npc import NPCPersonality
from ai_service.database import db
from ai_service.llm import get_llm_provider
from ai_service.agents.base_agent import AgentContext
from ai_service.agents.orchestrator import AgentOrchestrator
from ai_service.config import settings
from ai_service.memory.openmemory_manager import get_openmemory_manager

try:
    from openmemory import OpenMemory
    OPENMEMORY_AVAILABLE = True
except ImportError:
    logger.warning("OpenMemory SDK not available")
    OPENMEMORY_AVAILABLE = False

router = APIRouter(prefix="/ai/player", tags=["player"])

# Global orchestrator instance (will be initialized on first use)
_orchestrator = None


@router.post("/interaction/simple", response_class=PlainTextResponse)
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

        # Build personality from NPC state
        personality = NPCPersonality(
            openness=float(npc_state.get("openness", 0.5)),
            conscientiousness=float(npc_state.get("conscientiousness", 0.5)),
            extraversion=float(npc_state.get("extraversion", 0.5)),
            agreeableness=float(npc_state.get("agreeableness", 0.5)),
            neuroticism=float(npc_state.get("neuroticism", 0.5)),
            moral_alignment=npc_state.get("moral_alignment", "neutral")
        )

        # Get world state
        world_state = await db.get_world_state("global") or {}

        # Get relationship level
        relationship_key = f"relationship:{npc_id}:{player_id}"
        relationship_level = await db.redis.get(relationship_key)
        relationship_level = int(relationship_level) if relationship_level else 0

        # Parse information items (stored as JSON string in Redis)
        information_items_str = npc_state.get("information_items", "[]")
        try:
            information_items = json.loads(information_items_str) if isinstance(information_items_str, str) else information_items_str
        except json.JSONDecodeError:
            logger.warning(f"Failed to parse information_items for NPC {npc_id}, using empty list")
            information_items = []

        # Create proper AgentContext
        context = AgentContext(
            npc_id=npc_id,
            npc_name=npc_state.get("name", "Unknown"),
            personality=personality,
            current_state={
                "npc_class": npc_state.get("class", "generic"),
                "player_id": player_id,
                "player_name": context_data.get("player_name", "Adventurer"),
                "interaction_type": interaction_type,
                "location": context_data.get("location", {}),
                "time_of_day": context_data.get("time_of_day", "day"),
                "weather": context_data.get("weather", "clear"),
                "relationship_level": relationship_level,
                "information_items": information_items
            },
            world_state=world_state,
            recent_events=[]
        )

        # Process interaction using handle_player_interaction
        result = await orchestrator.handle_player_interaction(
            npc_context=context,
            player_message=context_data.get("player_message", ""),
            interaction_type=interaction_type
        )

        # Extract dialogue message from response
        response_text = "..."
        if result and "dialogue" in result:
            dialogue_data = result["dialogue"]
            if isinstance(dialogue_data, dict) and "text" in dialogue_data:
                response_text = dialogue_data["text"]

        # Trigger NPC movement after interaction ends (event-driven movement)
        try:
            from tasks.npc_movement import trigger_movement_on_interaction_end
            interaction_context = {
                "interaction_type": interaction_type,
                "player_name": context.get("player_name", "Adventurer"),
                "player_message": context.get("player_message", "")
            }
            # Fire and forget - don't wait for movement to complete
            import asyncio
            asyncio.create_task(trigger_movement_on_interaction_end(npc_id, player_id, interaction_context))
            logger.debug(f"Triggered movement decision for NPC {npc_id} after simple interaction")
        except Exception as e:
            logger.warning(f"Failed to trigger movement for NPC {npc_id}: {e}")

        return response_text

    except Exception as e:
        logger.error(f"Error in simple interaction: {e}", exc_info=True)
        return "..."


@router.post("/chat", response_class=PlainTextResponse)
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

        # Extract personality traits for LLM prompt
        def get_float(d, key, default=0.5):
            val = d.get(key, default)
            if isinstance(val, bytes):
                val = val.decode('utf-8')
            try:
                return float(val)
            except (ValueError, TypeError):
                return default

        openness = get_float(npc_state, "openness", 0.5)
        conscientiousness = get_float(npc_state, "conscientiousness", 0.5)
        extraversion = get_float(npc_state, "extraversion", 0.5)
        agreeableness = get_float(npc_state, "agreeableness", 0.5)
        neuroticism = get_float(npc_state, "neuroticism", 0.5)

        moral_alignment = get_str(npc_state, "moral_alignment", "neutral")
        background = get_str(npc_state, "background", f"a {npc_class} in Prontera")

        # Build personality description for system prompt
        personality_traits = []
        if openness > 0.7:
            personality_traits.append("curious, adventurous, and open to new experiences")
        elif openness < 0.3:
            personality_traits.append("traditional, cautious, and prefers familiar routines")
        else:
            personality_traits.append("balanced between tradition and novelty")

        if conscientiousness > 0.7:
            personality_traits.append("organized, responsible, and detail-oriented")
        elif conscientiousness < 0.3:
            personality_traits.append("spontaneous, flexible, and carefree")

        if extraversion > 0.7:
            personality_traits.append("outgoing, energetic, and talkative")
        elif extraversion < 0.3:
            personality_traits.append("reserved, quiet, and introspective")

        if agreeableness > 0.7:
            personality_traits.append("friendly, compassionate, and cooperative")
        elif agreeableness < 0.3:
            personality_traits.append("direct, competitive, and skeptical")

        if neuroticism > 0.7:
            personality_traits.append("anxious, sensitive, and emotionally reactive")
        elif neuroticism < 0.3:
            personality_traits.append("calm, stable, and emotionally resilient")

        personality_desc = ", ".join(personality_traits) if personality_traits else "balanced personality"

        # Generate AI response using LLM
        try:
            # Get LLM provider
            llm = get_llm_provider()

            # Retrieve relevant memories from OpenMemory SDK (if available)
            memory_context = ""
            openmemory_mgr = get_openmemory_manager()
            logger.debug(f"Memory retrieval: manager={openmemory_mgr}, available={openmemory_mgr.is_available() if openmemory_mgr else False}, message={bool(message)}")
            if openmemory_mgr and openmemory_mgr.is_available() and message:
                try:
                    logger.debug("Attempting to retrieve memories from OpenMemory...")
                    openmemory_client = openmemory_mgr.get_client()

                    # Retrieve all memories for this player and filter by relevance
                    # Note: OpenMemory's vector search with synthetic embeddings is unreliable,
                    # so we use all() method and filter in Python
                    logger.debug(f"Retrieving memories for player_{player_id}")

                    # Get all memories for this player (using all() method)
                    all_memories_result = openmemory_client.all(limit=50)
                    all_memories = all_memories_result.get("items", [])

                    # Filter by user_id
                    player_memories = [
                        mem for mem in all_memories
                        if mem.get("user_id") == f"player_{player_id}"
                    ]
                    logger.debug(f"Found {len(player_memories)} total memories for player_{player_id}")

                    # Simple keyword-based relevance filtering
                    # Extract keywords from the message
                    message_lower = message.lower()
                    keywords = set(message_lower.split())

                    # Score memories by keyword overlap
                    scored_memories = []
                    for mem in player_memories:
                        content = mem.get("content", "").lower()
                        # Count keyword matches
                        matches = sum(1 for keyword in keywords if keyword in content)
                        if matches > 0:
                            scored_memories.append((matches, mem))

                    # Sort by score (descending) and take top 3
                    scored_memories.sort(reverse=True, key=lambda x: x[0])
                    relevant_memories = [mem for score, mem in scored_memories[:3]]

                    logger.debug(f"Found {len(relevant_memories)} relevant memories after keyword filtering")

                    if relevant_memories:
                        memory_context = "\n\nRelevant past interactions:\n"
                        for mem in relevant_memories:
                            mem_content = mem.get("content", "")
                            mem_sector = mem.get("primary_sector", "unknown")
                            mem_score = mem.get("score", 0.0)
                            if mem_content:
                                memory_context += f"- [{mem_sector}] {mem_content} (relevance: {mem_score:.2f})\n"
                        logger.info(f"✓ Retrieved {len(relevant_memories)} relevant memories from OpenMemory for context")
                    else:
                        logger.info("No relevant memories found in OpenMemory")
                except Exception as e:
                    logger.error(f"Failed to retrieve memories from OpenMemory: {e}", exc_info=True)

            # Build system prompt with NPC personality and context
            system_prompt = f"""You are {npc_name}, a {npc_class} in the world of Ragnarok Online.

Background: {background}

Personality Traits (Big Five Model):
- Openness: {openness:.2f} ({personality_desc})
- Conscientiousness: {conscientiousness:.2f}
- Extraversion: {extraversion:.2f}
- Agreeableness: {agreeableness:.2f}
- Neuroticism: {neuroticism:.2f}

Moral Alignment: {moral_alignment}

You are having a conversation with {player_name}, an adventurer. Stay in character and respond naturally based on your personality traits and background. Keep responses concise (2-4 sentences) and conversational. Use your personality to guide your tone and word choice.

If the player says goodbye, respond warmly and wish them well in a way that fits your personality.{memory_context}"""

            # Build conversation messages for LLM
            messages = []

            # Add system prompt as first message
            messages.append({"role": "system", "content": system_prompt})

            # Add recent conversation history for context
            if history and len(history) > 1:  # More than just the current message
                recent_history = history[-6:-1]  # Last 5 messages before current
                for msg in recent_history:
                    role = msg.get("role", "unknown")
                    msg_text = msg.get("message", "")
                    if role == "player":
                        messages.append({"role": "user", "content": msg_text})
                    elif role == "npc":
                        messages.append({"role": "assistant", "content": msg_text})

            # Add current player message
            if message:
                messages.append({"role": "user", "content": message})
            else:
                # Initial greeting if no message
                messages.append({"role": "user", "content": f"*{player_name} approaches you*"})

            # Generate response using LLM
            logger.info(f"Generating LLM response for {npc_name} to player message: {message[:50] if message else '(initial greeting)'}...")
            llm_response = await llm.generate_chat(
                messages=messages,
                temperature=0.8,  # Higher temperature for more creative/varied responses
                max_tokens=150  # Keep responses concise
            )

            response_text = llm_response.content.strip()
            logger.info(f"LLM generated response ({llm_response.tokens_used} tokens): {response_text[:100]}...")

        except Exception as e:
            logger.error(f"LLM generation failed: {e}", exc_info=True)
            logger.warning("Falling back to rule-based response")

            # Fallback to simple rule-based response if LLM fails
            message_lower = message.lower() if message else ""

            if "bye" in message_lower or "goodbye" in message_lower or "farewell" in message_lower:
                if npc_class == "explorer":
                    response_text = "Safe travels, friend! May your adventures be grand!"
                elif npc_class == "guard":
                    response_text = "Move along then. Stay safe out there."
                else:
                    response_text = "Farewell, traveler. Come back anytime!"
            elif not message or "hello" in message_lower or "hi" in message_lower:
                if npc_class == "explorer":
                    response_text = f"Greetings, {player_name}! I'm {npc_name}. What brings you here?"
                elif npc_class == "guard":
                    response_text = f"Halt! State your business, {player_name}."
                else:
                    response_text = f"Hello, {player_name}! How can I help you?"
            else:
                if extraversion > 0.7:
                    response_text = "That's fascinating! Tell me more!"
                elif extraversion < 0.3:
                    response_text = "I see. That's... interesting."
                else:
                    response_text = "That's an interesting point. What else is on your mind?"

        # Add NPC response to history
        history.append({
            "role": "npc",
            "name": npc_name,
            "message": response_text,
            "timestamp": datetime.utcnow().isoformat()
        })

        # Save conversation history (10 minute TTL in DragonflyDB)
        await db.save_conversation_history(conversation_key, history, ttl=600)

        # Store conversation in OpenMemory SDK for long-term persistent memory
        openmemory_mgr = get_openmemory_manager()
        logger.debug(f"OpenMemory manager: {openmemory_mgr}, available: {openmemory_mgr.is_available() if openmemory_mgr else False}")
        if openmemory_mgr and openmemory_mgr.is_available() and message:
            try:
                logger.debug("Attempting to store conversation in OpenMemory...")
                openmemory_client = openmemory_mgr.get_client()

                # Store the full conversation exchange
                # Store the full conversation exchange with rich context
                conversation_text = (
                    f"Conversation between {player_name} and {npc_name} ({npc_class}):\n"
                    f"{player_name}: {message}\n"
                    f"{npc_name}: {response_text}"
                )

                # Add metadata for better retrieval and classification
                metadata = {
                    "npc_id": npc_id,
                    "npc_name": npc_name,
                    "npc_class": npc_class,
                    "player_id": player_id,
                    "player_name": player_name,
                    "timestamp": datetime.utcnow().isoformat(),
                    "conversation_type": "player_npc_chat",
                    "sector": "episodic"  # Suggest episodic sector for conversation memories
                }

                # Store in OpenMemory (persistent PostgreSQL storage)
                # Use user_id for multi-user isolation
                # Higher salience (0.8) for important player-NPC interactions
                result = openmemory_client.add(
                    content=conversation_text,
                    tags=["conversation", "npc_interaction", npc_class, player_name, npc_name],
                    metadata=metadata,
                    salience=0.8,  # High importance for player-NPC conversations
                    user_id=f"player_{player_id}"
                )

                memory_id = result.get("id", "unknown")
                primary_sector = result.get("primary_sector", "unknown")
                logger.info(f"✓ Stored conversation in OpenMemory (memory_id: {memory_id[:8]}..., sector: {primary_sector})")

            except Exception as e:
                logger.error(f"Failed to store conversation in OpenMemory: {e}", exc_info=True)

        logger.info(f"NPC {npc_name} response: {response_text[:100]}...")

        # Trigger NPC movement after interaction ends (event-driven movement)
        try:
            from tasks.npc_movement import trigger_movement_on_interaction_end
            interaction_context = {
                "interaction_type": "chat",
                "player_name": player_name,
                "message": message
            }
            # Fire and forget - don't wait for movement to complete
            import asyncio
            asyncio.create_task(trigger_movement_on_interaction_end(npc_id, str(player_id), interaction_context))
            logger.debug(f"Triggered movement decision for NPC {npc_id} after chat interaction")
        except Exception as e:
            logger.warning(f"Failed to trigger movement for NPC {npc_id}: {e}")

        return response_text

    except Exception as e:
        logger.error(f"Error in chat interaction: {e}", exc_info=True)
        # Return a simple fallback message
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

        # Note: Memori SDK has been replaced with OpenMemory SDK
        # OpenMemory is initialized globally in main.py and accessed via get_openmemory_manager()
        # The orchestrator no longer needs a memori_client parameter
        memori_client = None
        logger.info("Using OpenMemory SDK for persistent memory (initialized globally)")

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

