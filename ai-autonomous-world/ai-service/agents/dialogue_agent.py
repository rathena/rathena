"""
Dialogue Agent - Generates personality-driven NPC dialogue
Handles conversation generation based on context, personality, and memory
Phase 8A: Added Redis caching for frequently requested dialogues
"""

from typing import Dict, Any, List
from loguru import logger

from crewai import Agent
from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
from config import settings
from utils.cache import cache_response
from models.information import (
    InformationItem, filter_information_by_relationship
)
from models.npc_relationship import NPCRelationship, NPCInteraction, SharedInformation

class DialogueAgent(BaseAIAgent):
    """
    Specialized agent for generating NPC dialogue

    Responsibilities:
    - Generate contextual dialogue based on personality, goals, emotion, memory
    - Maintain conversation coherence
    - Adapt tone and style to NPC character
    - Consider relationship with player and other NPCs
    - Incorporate memory and world state
    - Integrate social fabric: gossip, persuasion, social learning, reputation
    """
    def __init__(self, agent_id: str, llm_provider: Any, config: Dict[str, Any]):
        """Initialize Dialogue Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="dialogue",
            llm_provider=llm_provider,
            config=config
        )
        from agents.moral_alignment import MoralAlignment
        self.moral_alignment = MoralAlignment()
        logger.info(f"Dialogue Agent {agent_id} initialized")

    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for dialogue generation"""
        from crewai import LLM
        import os
        try:
            from config import settings
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.7,
                max_tokens=2000
            )
            logger.info(f"Created Azure OpenAI LLM with deployment: {settings.azure_openai_deployment}")
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            raise
        return Agent(
            role="NPC Dialogue Specialist",
            goal="Generate authentic, personality-driven dialogue for NPCs that feels natural and engaging",
            backstory="""You are an expert in character dialogue and storytelling. You understand how to
            create unique voices for different characters based on their personality traits, background,
            and current emotional state. You excel at making NPCs feel alive and memorable through their
            speech patterns, word choices, and conversational style. You also understand social dynamics, gossip, and persuasion.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )

    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Generate dialogue for NPC

        Args:
            context: AgentContext with NPC info, player message, and world state

        Returns:
            AgentResponse with generated dialogue
        """
        try:
            logger.info(f"Dialogue Agent processing for NPC: {context.npc_id}")

            # Extract player message from context
            player_message = context.current_state.get("player_message", "")
            interaction_type = context.current_state.get("interaction_type", "talk")
            player_name = context.current_state.get("player_name", "Adventurer")

            # Build personality, goal, emotion, memory descriptions
            personality_desc = self._build_personality_prompt(context.personality)
            goal_desc = self._build_goal_prompt(getattr(context, "goal_state", None))
            emotion_desc = self._build_emotion_prompt(getattr(context, "emotion_state", None))
            memory_desc = self._build_memory_prompt(getattr(context, "memory_state", None))

            # Build context information
            context_info = self._build_context_info(context)

            # Build memory context (legacy, for compatibility)
            memory_info = self._build_memory_info(context)

            # Build information context (social intelligence)
            information_info, shared_items = self._build_information_context(context)

            # Build social fabric context (gossip, persuasion, social learning, reputation)
            social_context = self._build_social_fabric_context(context)

            # --- Moral Alignment Integration ---
            # Update alignment based on dialogue context and player interaction
            alignment_action = {
                "type": "dialogue",
                "player_message": player_message,
                "interaction_type": interaction_type
            }
            self.moral_alignment.update_from_action(alignment_action)

            # Generate dialogue
            dialogue = await self._generate_dialogue(
                npc_name=context.npc_name,
                personality=personality_desc,
                goal_state=goal_desc,
                emotion_state=emotion_desc,
                memory_state=memory_desc,
                context_info=context_info,
                memory_info=memory_info,
                information_info=information_info,
                social_context=social_context,
                player_name=player_name,
                player_message=player_message,
                interaction_type=interaction_type
            )

            # Store information sharing history in OpenMemory
            if shared_items:
                await self._store_information_sharing_history(
                    context=context,
                    shared_items=shared_items,
                    player_name=player_name
                )

            # Determine emotion based on personality and context
            emotion = self._determine_emotion(context)

            # Suggest next actions
            next_actions = self._suggest_next_actions(interaction_type)

            response_data = {
                "text": dialogue,
                "speaker": context.npc_name,
                "emotion": emotion,
                "next_actions": next_actions,
                "alignment": self.moral_alignment.to_dict()
            }

            logger.info(f"Dialogue generated successfully for {context.npc_id}")

            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=response_data,
                confidence=0.85,
                reasoning="Dialogue generated based on personality, goals, emotion, memory, social context, world state, and updated alignment",
                metadata={
                    "interaction_type": interaction_type,
                    "player_message_length": len(player_message),
                    "alignment": self.moral_alignment.to_dict()
                }
            )

        except Exception as e:
            logger.error(f"Dialogue generation failed for {context.npc_id}: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={"error": str(e)},
                confidence=0.0,
                reasoning=f"Error during dialogue generation: {e}"
            )

    def _build_context_info(self, context: AgentContext) -> str:
        """Build context information string"""
        location = context.current_state.get("location", {})
        time_of_day = context.current_state.get("time_of_day", "day")
        weather = context.current_state.get("weather", "clear")
        context_parts = []
        if location:
            map_name = location.get("map", "unknown")
            context_parts.append(f"Location: {map_name}")
        context_parts.append(f"Time: {time_of_day}")
        context_parts.append(f"Weather: {weather}")
        if context.world_state:
            economy = context.world_state.get("economy", {})
            if economy:
                context_parts.append(f"Economic conditions: {economy.get('description', 'stable')}")
        return ". ".join(context_parts)

    def _build_memory_info(self, context: AgentContext) -> str:
        """Build memory context string (legacy, for compatibility)"""
        if not context.memory_context:
            return "This is a new interaction."
        memory_parts = []
        prev_interactions = context.memory_context.get("previous_interactions", 0)
        if prev_interactions > 0:
            memory_parts.append(f"You have met this person {prev_interactions} time(s) before")
        relationship = context.memory_context.get("relationship_level", 0)
        if relationship > 50:
            memory_parts.append("You have a friendly relationship")
        elif relationship < -50:
            memory_parts.append("You have a hostile relationship")
        last_summary = context.memory_context.get("last_interaction_summary")
        if last_summary:
            memory_parts.append(f"Last time: {last_summary}")
        return ". ".join(memory_parts) if memory_parts else "This is a new interaction."

    def _build_information_context(self, context: AgentContext) -> tuple[str, list]:
        """
        Build information context based on relationship level and personality
        This implements the social intelligence system where NPCs decide what
        information to share based on their relationship with the player.
        Returns:
            tuple: (information_context_string, list_of_shared_items)
        """
        relationship_level = context.current_state.get("relationship_level", 0)
        player_id = context.current_state.get("player_id")
        information_items_data = context.current_state.get("information_items", [])
        if not information_items_data:
            return "", []
        information_items = [
            InformationItem.from_dict(item) for item in information_items_data
        ]
        available_items = filter_information_by_relationship(
            information_items=information_items,
            relationship_level=relationship_level,
            agreeableness=context.personality.agreeableness,
            neuroticism=context.personality.neuroticism,
            openness=context.personality.openness
        )
        if not available_items:
            return "", []
        info_parts = []
        info_parts.append(f"\nYour relationship with this player: Level {relationship_level}/10")
        info_parts.append("\nAvailable information you can share:")
        for item in available_items:
            info_parts.append(f"- [{item.sensitivity.value.upper()}] {item.content}")
        restricted_count = len(information_items) - len(available_items)
        if restricted_count > 0:
            info_parts.append(
                f"\nYou have {restricted_count} piece(s) of information that you should NOT share "
                f"with this player yet (relationship level too low)."
            )
        return "\n".join(info_parts), available_items

    def _build_social_fabric_context(self, context: AgentContext) -> str:
        """
        Build social fabric context: relationships, gossip, persuasion, social learning, reputation
        """
        # Example: summarize recent NPC-NPC interactions, shared information, and reputation
        relationship_data = context.current_state.get("npc_relationships", [])
        gossip_data = context.current_state.get("gossip", [])
        reputation = context.current_state.get("reputation", 0)
        social_parts = []
        if relationship_data:
            for rel in relationship_data:
                try:
                    rel_obj = NPCRelationship(**rel)
                    social_parts.append(
                        f"Relationship with {rel_obj.npc_id_2}: {rel_obj.relationship_type} ({rel_obj.relationship_value:.1f}), trust: {rel_obj.trust_level:.1f}"
                    )
                except Exception:
                    continue
        if gossip_data:
            for g in gossip_data:
                try:
                    g_obj = SharedInformation(**g)
                    social_parts.append(
                        f"Gossip: {g_obj.content} (from {g_obj.source_npc_id}, reliability: {g_obj.reliability:.2f})"
                    )
                except Exception:
                    continue
        social_parts.append(f"Reputation: {reputation}")
        return "\n".join(social_parts)

    async def _store_information_sharing_history(
        self,
        context: AgentContext,
        shared_items: list,
        player_name: str
    ) -> None:
        """
        Store information sharing history in OpenMemory
        This tracks what information has been shared with each player,
        enabling NPCs to reference past conversations and avoid repetition.
        """
        try:
            from main import get_openmemory_manager
            openmemory_manager = get_openmemory_manager()
            if not openmemory_manager or not openmemory_manager.is_available():
                logger.warning("OpenMemory manager not available, skipping information sharing history")
                return
            player_id = context.current_state.get("player_id")
            relationship_level = context.current_state.get("relationship_level", 0)
            shared_content_list = [f"{item.sensitivity.value.upper()}: {item.content}" for item in shared_items]
            content = (
                f"{context.npc_name} shared {len(shared_items)} piece(s) of information with {player_name}:\n"
                + "\n".join(f"- {item}" for item in shared_content_list)
            )
            client = openmemory_manager.get_client()
            client.add(
                content=content,
                tags=["information_sharing", "social_intelligence", context.npc_id, player_id],
                metadata={
                    "npc_id": context.npc_id,
                    "npc_name": context.npc_name,
                    "player_id": player_id,
                    "player_name": player_name,
                    "relationship_level": relationship_level,
                    "items_shared": len(shared_items),
                    "sensitivity_levels": [item.sensitivity.value for item in shared_items],
                    "event_type": "information_sharing"
                },
                salience=0.7,
                user_id=player_id
            )
            logger.info(
                f"Stored information sharing history: {context.npc_name} shared "
                f"{len(shared_items)} items with {player_name} (relationship level {relationship_level})"
            )
        except Exception as e:
            logger.error(f"Error storing information sharing history: {e}")

    @cache_response('dialogue', ttl=300)
    async def _generate_dialogue(
        self,
        npc_name: str,
        personality: str,
        goal_state: str,
        emotion_state: str,
        memory_state: str,
        context_info: str,
        memory_info: str,
        information_info: str,
        social_context: str,
        player_name: str,
        player_message: str,
        interaction_type: str
    ) -> str:
        """
        Generate dialogue using LLM (now with full consciousness and social context)
        Phase 8A: Cached responses for frequently asked questions/interactions
        """
        if not player_message or not isinstance(player_message, str):
            if player_message and not isinstance(player_message, str):
                logger.warning(f"Invalid player message type: {type(player_message)}")
            return f"I didn't quite catch that, {player_name}."
        if len(player_message) > settings.max_player_message_length:
            logger.warning(f"Player message too long ({len(player_message)} chars), truncating")
            player_message = player_message[:settings.max_player_message_length] + "..."
        system_message = f"""You are {npc_name}, an NPC in a medieval fantasy MMORPG world.

{personality}
{goal_state}
{emotion_state}
{memory_state}

Current situation: {context_info}

Memory: {memory_info}

{information_info}

Social context:
{social_context}

Generate a natural, in-character response to the player. Keep responses concise (2-3 sentences max).
Stay true to your personality, goals, emotion, memory, social context, and the world context. Do not break character or mention game mechanics.

IMPORTANT: Only share information that is marked as available above. Do NOT share information that you're told not to share."""
        user_prompt = f"""Player ({player_name}) says: "{player_message}"

Interaction type: {interaction_type}

Respond as {npc_name} would, considering your personality, goals, emotion, memory, social context, and the situation."""
        try:
            dialogue = await self._generate_with_llm(
                prompt=user_prompt,
                system_message=system_message,
                temperature=settings.dialogue_temperature
            )
            if not dialogue or len(dialogue.strip()) == 0:
                logger.warning("LLM returned empty dialogue, using fallback")
                return f"Greetings, {player_name}. How may I assist you?"
            return dialogue.strip()
        except Exception as e:
            logger.error(f"LLM dialogue generation failed: {e}")
            fallback_responses = {
                "talk": f"Greetings, {player_name}. How may I assist you?",
                "trade": f"Welcome to my shop, {player_name}. What can I get for you?",
                "quest": f"Ah, {player_name}! I may have a task for you.",
                "default": f"Hello, {player_name}."
            }
            return fallback_responses.get(interaction_type, fallback_responses["default"])

    def _determine_emotion(self, context: AgentContext) -> str:
        """Determine NPC emotion based on personality and context"""
        personality = context.personality
        if personality.agreeableness > 0.7:
            return "friendly"
        elif personality.neuroticism > 0.7:
            return "anxious"
        elif personality.extraversion > 0.7:
            return "enthusiastic"
        elif personality.openness > 0.7:
            return "curious"
        elif personality.conscientiousness > 0.7:
            return "professional"
        else:
            return "neutral"

    def _suggest_next_actions(self, interaction_type: str) -> list:
        """Suggest possible next actions for the player"""
        base_actions = ["talk", "end_conversation"]
        if interaction_type == "talk":
            return base_actions + ["trade", "quest"]
        elif interaction_type == "trade":
            return base_actions + ["buy", "sell"]
        elif interaction_type == "quest":
            return base_actions + ["accept_quest", "decline_quest"]
        else:
            return base_actions