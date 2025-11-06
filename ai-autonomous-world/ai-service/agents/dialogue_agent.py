"""
Dialogue Agent - Generates personality-driven NPC dialogue
Handles conversation generation based on context, personality, and memory
"""

from typing import Dict, Any, Optional
from loguru import logger

from crewai import Agent
from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse


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
        return Agent(
            role="NPC Dialogue Specialist",
            goal="Generate authentic, personality-driven dialogue for NPCs that feels natural and engaging",
            backstory="""You are an expert in character dialogue and storytelling. You understand how to 
            create unique voices for different characters based on their personality traits, background, 
            and current emotional state. You excel at making NPCs feel alive and memorable through their 
            speech patterns, word choices, and conversational style.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False
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
        """Generate dialogue using LLM"""

        system_message = f"""You are {npc_name}, an NPC in a medieval fantasy MMORPG world.

{personality}

Current situation: {context_info}

Memory: {memory_info}

Generate a natural, in-character response to the player. Keep responses concise (2-3 sentences max).
Stay true to your personality and the context. Do not break character or mention game mechanics."""

        user_prompt = f"""Player ({player_name}) says: "{player_message}"

Interaction type: {interaction_type}

Respond as {npc_name} would, considering your personality and the situation."""

        dialogue = await self._generate_with_llm(
            prompt=user_prompt,
            system_message=system_message,
            temperature=0.8  # Higher temperature for more creative dialogue
        )

        return dialogue.strip()

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

