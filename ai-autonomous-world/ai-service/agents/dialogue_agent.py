"""
Dialogue Agent - Generates personality-driven NPC dialogue
Handles conversation generation based on context, personality, and memory
Phase 8A: Added Redis caching for frequently requested dialogues
"""

from typing import Dict, Any
from loguru import logger

from crewai import Agent
try:
    from ai_service.agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from ai_service.config import settings
    from ai_service.utils.cache import cache_response
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from config import settings
    from utils.cache import cache_response


class DialogueAgent(BaseAIAgent):
    """
    Specialized agent for generating NPC dialogue
    
    Responsibilities:
    - Generate contextual dialogue based on personality
    - Maintain conversation coherence
    - Adapt tone and style to NPC character
    - Consider relationship with player
    - Incorporate memory and world state
    """
    
    def __init__(self, agent_id: str, llm_provider: Any, config: Dict[str, Any]):
        """Initialize Dialogue Agent"""
        super().__init__(
            agent_id=agent_id,
            agent_type="dialogue",
            llm_provider=llm_provider,
            config=config
        )
        
        logger.info(f"Dialogue Agent {agent_id} initialized")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for dialogue generation"""
        # Import CrewAI's LLM class
        from crewai import LLM
        import os

        # Create CrewAI-compatible LLM using litellm format for Azure OpenAI
        # CrewAI uses litellm under the hood which supports Azure via environment variables
        try:
            from config import settings

            # Set Azure OpenAI environment variables for litellm
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version

            # Use litellm format: azure/<deployment_name>
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.7,
                max_tokens=2000
            )
            logger.info(f"Created Azure OpenAI LLM with deployment: {settings.azure_openai_deployment}")
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            # Don't fall back to provider - raise the error so we can fix it
            raise

        return Agent(
            role="NPC Dialogue Specialist",
            goal="Generate authentic, personality-driven dialogue for NPCs that feels natural and engaging",
            backstory="""You are an expert in character dialogue and storytelling. You understand how to
            create unique voices for different characters based on their personality traits, background,
            and current emotional state. You excel at making NPCs feel alive and memorable through their
            speech patterns, word choices, and conversational style.""",
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
            
            # Build personality description
            personality_desc = self._build_personality_prompt(context.personality)
            
            # Build context information
            context_info = self._build_context_info(context)
            
            # Build memory context
            memory_info = self._build_memory_info(context)
            
            # Generate dialogue
            dialogue = await self._generate_dialogue(
                npc_name=context.npc_name,
                personality=personality_desc,
                context_info=context_info,
                memory_info=memory_info,
                player_name=player_name,
                player_message=player_message,
                interaction_type=interaction_type
            )
            
            # Determine emotion based on personality and context
            emotion = self._determine_emotion(context)
            
            # Suggest next actions
            next_actions = self._suggest_next_actions(interaction_type)
            
            response_data = {
                "text": dialogue,
                "speaker": context.npc_name,
                "emotion": emotion,
                "next_actions": next_actions
            }
            
            logger.info(f"Dialogue generated successfully for {context.npc_id}")
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=response_data,
                confidence=0.85,
                reasoning="Dialogue generated based on personality and context",
                metadata={
                    "interaction_type": interaction_type,
                    "player_message_length": len(player_message)
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
        
        # Add world state if relevant
        if context.world_state:
            economy = context.world_state.get("economy", {})
            if economy:
                context_parts.append(f"Economic conditions: {economy.get('description', 'stable')}")
        
        return ". ".join(context_parts)
    
    def _build_memory_info(self, context: AgentContext) -> str:
        """Build memory context string"""
        if not context.memory_context:
            return "This is a new interaction."
        
        memory_parts = []
        
        # Previous interactions
        prev_interactions = context.memory_context.get("previous_interactions", 0)
        if prev_interactions > 0:
            memory_parts.append(f"You have met this person {prev_interactions} time(s) before")
        
        # Relationship level
        relationship = context.memory_context.get("relationship_level", 0)
        if relationship > 50:
            memory_parts.append("You have a friendly relationship")
        elif relationship < -50:
            memory_parts.append("You have a hostile relationship")
        
        # Last interaction summary
        last_summary = context.memory_context.get("last_interaction_summary")
        if last_summary:
            memory_parts.append(f"Last time: {last_summary}")
        
        return ". ".join(memory_parts) if memory_parts else "This is a new interaction."

    @cache_response('dialogue', ttl=300)  # Phase 8A: Cache dialogue for 5 minutes
    async def _generate_dialogue(
        self,
        npc_name: str,
        personality: str,
        context_info: str,
        memory_info: str,
        player_name: str,
        player_message: str,
        interaction_type: str
    ) -> str:
        """
        Generate dialogue using LLM
        Phase 8A: Cached responses for frequently asked questions/interactions
        """

        # Validate inputs
        if not player_message or not isinstance(player_message, str):
            logger.warning(f"Invalid player message: {player_message}")
            return f"I didn't quite catch that, {player_name}."

        if len(player_message) > settings.max_player_message_length:
            logger.warning(f"Player message too long ({len(player_message)} chars), truncating")
            player_message = player_message[:settings.max_player_message_length] + "..."

        system_message = f"""You are {npc_name}, an NPC in a medieval fantasy MMORPG world.

{personality}

Current situation: {context_info}

Memory: {memory_info}

Generate a natural, in-character response to the player. Keep responses concise (2-3 sentences max).
Stay true to your personality and the context. Do not break character or mention game mechanics."""

        user_prompt = f"""Player ({player_name}) says: "{player_message}"

Interaction type: {interaction_type}

Respond as {npc_name} would, considering your personality and the situation."""

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
            # Return fallback dialogue based on interaction type
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

        # Base emotion on personality traits
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

