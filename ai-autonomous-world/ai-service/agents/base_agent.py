# NOTE: AgentDecision stub for dependency resolution
from typing import Any, Dict, Optional
from dataclasses import dataclass, field
from datetime import datetime

@dataclass
class AgentDecision:
    success: bool
    data: Dict[str, Any]
    reasoning: Optional[str] = None
    timestamp: datetime = field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))
"""
Base Agent Classes for AI Autonomous World System
Provides abstract base classes for all specialized agents
"""

from abc import ABC, abstractmethod
from typing import Dict, Any, List, Optional
from dataclasses import dataclass, field
from datetime import datetime
from loguru import logger

from crewai import Agent
try:
    from models.npc import NPCPersonality, NPCGoalState, NPCEmotionState, NPCMemoryState
    from agents.moral_alignment import MoralAlignment
except ModuleNotFoundError:
    from models.npc import NPCPersonality, NPCGoalState, NPCEmotionState, NPCMemoryState

@dataclass
class AgentContext:
    """Context information passed to agents"""
    npc_id: str
    npc_name: str
    personality: NPCPersonality
    moral_alignment: Optional[MoralAlignment] = None
    goal_state: Optional[NPCGoalState] = None
    emotion_state: Optional[NPCEmotionState] = None
    memory_state: Optional[NPCMemoryState] = None
    current_state: Dict[str, Any] = field(default_factory=dict)
    world_state: Dict[str, Any] = field(default_factory=dict)
    recent_events: List[Dict[str, Any]] = field(default_factory=list)
    memory_context: Optional[Dict[str, Any]] = None
    timestamp: datetime = field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))

@dataclass
class AgentResponse:
    """Standard response format from agents"""
    agent_type: str
    success: bool
    data: Dict[str, Any]
    confidence: float  # 0.0 to 1.0
    reasoning: Optional[str] = None
    metadata: Dict[str, Any] = field(default_factory=dict)
    timestamp: datetime = field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))

class BaseAIAgent(ABC):
    """
    Abstract base class for all AI agents in the system
    
    All specialized agents (Dialogue, Decision, Memory, World) inherit from this class
    and implement the process() method for their specific functionality.
    """
    def __init__(
        self,
        agent_id: str,
        agent_type: str,
        llm_provider: Any,
        config: Dict[str, Any]
    ):
        """
        Initialize base agent
        
        Args:
            agent_id: Unique identifier for this agent instance
            agent_type: Type of agent (dialogue, decision, memory, world)
            llm_provider: LLM provider instance for generation
            config: Configuration dictionary for agent behavior
        """
        self.agent_id = agent_id
        self.agent_type = agent_type
        self.llm_provider = llm_provider
        self.config = config
        
        logger.info(f"Initializing {agent_type} agent: {agent_id}")
        
        # Create CrewAI agent
        self.crew_agent = self._create_crew_agent()
    
    @abstractmethod
    def _create_crew_agent(self) -> Agent:
        """
        Create and configure the CrewAI Agent instance
        
        Returns:
            Configured CrewAI Agent
        """
        pass
    
    @abstractmethod
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Process the given context and generate a response
        
        Args:
            context: AgentContext with all necessary information
            
        Returns:
            AgentResponse with results
        """
        pass
    
    def _build_personality_prompt(self, personality: NPCPersonality) -> str:
        """
        Build personality description for prompts
        
        Args:
            personality: NPCPersonality model
            
        Returns:
            Formatted personality description
        """
        traits = []
        # Openness
        if personality.openness > 0.7:
            traits.append("creative and open to new experiences")
        elif personality.openness < 0.3:
            traits.append("traditional and prefers routine")
        # Conscientiousness
        if personality.conscientiousness > 0.7:
            traits.append("organized and reliable")
        elif personality.conscientiousness < 0.3:
            traits.append("spontaneous and flexible")
        # Extraversion
        if personality.extraversion > 0.7:
            traits.append("outgoing and energetic")
        elif personality.extraversion < 0.3:
            traits.append("reserved and introspective")
        # Agreeableness
        if personality.agreeableness > 0.7:
            traits.append("friendly and compassionate")
        elif personality.agreeableness < 0.3:
            traits.append("competitive and skeptical")
        # Neuroticism
        if personality.neuroticism > 0.7:
            traits.append("sensitive and cautious")
        elif personality.neuroticism < 0.3:
            traits.append("calm and confident")
        # Custom traits
        if hasattr(personality, "ambition") and personality.ambition > 0.7:
            traits.append("highly ambitious")
        if hasattr(personality, "courage") and personality.courage > 0.7:
            traits.append("courageous and bold")
        if hasattr(personality, "compassion") and personality.compassion > 0.7:
            traits.append("deeply compassionate")
        if hasattr(personality, "cunning") and personality.cunning > 0.7:
            traits.append("cunning and strategic")
        if hasattr(personality, "loyalty") and personality.loyalty > 0.7:
            traits.append("loyal to friends and causes")
        # Moral alignment
        alignment_desc = {
            "lawful_good": "lawful good (honorable and compassionate)",
            "neutral_good": "neutral good (kind and helpful)",
            "chaotic_good": "chaotic good (free-spirited and benevolent)",
            "lawful_neutral": "lawful neutral (orderly and impartial)",
            "true_neutral": "true neutral (balanced and pragmatic)",
            "chaotic_neutral": "chaotic neutral (unpredictable and independent)",
            "lawful_evil": "lawful evil (tyrannical and organized)",
            "neutral_evil": "neutral evil (selfish and malicious)",
            "chaotic_evil": "chaotic evil (destructive and cruel)"
        }
        personality_str = f"Personality: {', '.join(traits)}. Moral alignment: {alignment_desc.get(personality.moral_alignment, 'neutral')}."
        if hasattr(personality, "background") and personality.background and personality.background.history:
            personality_str += f" Background: {personality.background.history}"
        return personality_str

    def _build_goal_prompt(self, goal_state: Optional[NPCGoalState]) -> str:
        if not goal_state:
            return ""
        needs = [
            f"Survival: {goal_state.survival:.2f}",
            f"Security: {goal_state.security:.2f}",
            f"Social: {goal_state.social:.2f}",
            f"Esteem: {goal_state.esteem:.2f}",
            f"Self-Actualization: {goal_state.self_actualization:.2f}"
        ]
        goals = f"Short-term goals: {', '.join(goal_state.short_term_goals)}. Long-term goals: {', '.join(goal_state.long_term_goals)}."
        return f"Needs: {', '.join(needs)}. {goals} Current goal: {goal_state.current_goal or 'None'}."

    def _build_emotion_prompt(self, emotion_state: Optional[NPCEmotionState]) -> str:
        if not emotion_state:
            return ""
        return f"Current emotion: {emotion_state.current_emotion}. Mood: {emotion_state.mood}."

    def _build_memory_prompt(self, memory_state: Optional[NPCMemoryState]) -> str:
        if not memory_state:
            return ""
        return f"Episodic memories: {len(memory_state.episodic)}. Semantic: {len(memory_state.semantic)}. Procedural: {len(memory_state.procedural)}. Emotional: {len(memory_state.emotional)}. Reflective: {len(memory_state.reflective)}."

    async def _generate_with_llm(
        self,
        prompt: str,
        system_message: Optional[str] = None,
        temperature: float = 0.7
    ) -> str:
        """
        Generate text using the LLM provider

        Args:
            prompt: User prompt
            system_message: Optional system message
            temperature: Generation temperature

        Returns:
            Generated text
        """
        try:
            response = await self.llm_provider.generate(
                prompt=prompt,
                system_prompt=system_message,  # Fixed: use system_prompt instead of system_message
                temperature=temperature
            )
            return response.content  # Fixed: use content instead of text
        except Exception as e:
            logger.error(f"LLM generation failed in {self.agent_type} agent: {e}")
            raise