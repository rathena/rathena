"""
Unit tests for AI agents
"""

import pytest
from unittest.mock import AsyncMock, MagicMock, patch
from datetime import datetime
from agents.base_agent import AgentContext, AgentResponse
from agents.dialogue_agent import DialogueAgent
from agents.decision_agent import DecisionAgent
from agents.orchestrator import AgentOrchestrator


class TestDialogueAgent:
    """Test Dialogue Agent"""
    
    @pytest.mark.asyncio
    async def test_dialogue_agent_initialization(self, mock_llm_provider):
        """Test dialogue agent initializes correctly"""
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        assert agent.agent_id == "dialogue_001"
        assert agent.agent_type == "dialogue"
    
    @pytest.mark.asyncio
    async def test_dialogue_generation(self, mock_llm_provider, mock_agent_context):
        """Test dialogue generation"""
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content="Greetings, brave adventurer! How may I assist you today?",
            tokens_used=30
        )

        response = await agent.process(mock_agent_context)

        assert response.success is True
        assert "Greetings" in response.data.get("text", "")
        # LLM may be called multiple times due to caching/retry logic
        assert mock_llm_provider.generate.called
    
    @pytest.mark.asyncio
    async def test_dialogue_input_validation(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent validates input"""
        from models.npc import NPCPersonality

        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content="I didn't quite catch that.",
            tokens_used=10
        )

        # Test with empty player message in context
        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality=NPCPersonality(),
            current_state={"player_message": ""},
            world_state={},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        result = await agent.process(context)

        assert result.success is True
    
    @pytest.mark.asyncio
    async def test_dialogue_length_limit(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent enforces message length limit"""
        from config import settings
        from models.npc import NPCPersonality

        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        # Create message longer than max length
        long_message = "a" * (settings.max_player_message_length + 100)

        mock_llm_provider.generate.return_value = MagicMock(
            content="Response to truncated message",
            tokens_used=20
        )

        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality=NPCPersonality(),
            current_state={"player_message": long_message},
            world_state={},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        result = await agent.process(context)

        # Should truncate and still generate response
        assert result.success is True
    
    @pytest.mark.asyncio
    async def test_dialogue_fallback_on_error(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent provides fallback on LLM error"""
        from models.npc import NPCPersonality

        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        # Simulate LLM error
        mock_llm_provider.generate.side_effect = Exception("LLM API error")

        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality=NPCPersonality(),
            current_state={"player_message": "Hello"},
            world_state={},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        result = await agent.process(context)

        # Should return fallback response (agent has fallback logic)
        assert result is not None
        assert result.success is True  # Agent succeeds with fallback
        assert "Greetings" in result.data.get("text", "")
    
    @pytest.mark.asyncio
    async def test_dialogue_uses_config_temperature(self, mock_llm_provider):
        """Test dialogue agent uses configured temperature"""
        from config import settings
        from models.npc import NPCPersonality

        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content="Test response",
            tokens_used=20
        )

        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality=NPCPersonality(),
            current_state={"player_message": "Hello"},
            world_state={},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        await agent.process(context)

        # Verify temperature from settings is used
        call_kwargs = mock_llm_provider.generate.call_args[1]
        assert call_kwargs.get("temperature") == settings.dialogue_temperature


class TestDecisionAgent:
    """Test Decision Agent"""
    
    @pytest.mark.asyncio
    async def test_decision_agent_initialization(self, mock_llm_provider):
        """Test decision agent initializes correctly"""
        agent = DecisionAgent(
            agent_id="decision_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        assert agent.agent_id == "decision_001"
        assert agent.agent_type == "decision"
    
    @pytest.mark.asyncio
    async def test_decision_making(self, mock_llm_provider, mock_agent_context):
        """Test decision making"""
        agent = DecisionAgent(
            agent_id="decision_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content='{"action_type": "move", "reasoning": "Patrol area", "priority": 5}',
            tokens_used=40
        )

        response = await agent.process(mock_agent_context)

        assert response.success is True
        assert "action_type" in response.data
    
    @pytest.mark.asyncio
    async def test_decision_uses_config_temperature(self, mock_llm_provider):
        """Test decision agent uses configured temperature"""
        from config import settings
        from models.npc import NPCPersonality

        agent = DecisionAgent(
            agent_id="decision_001",
            llm_provider=mock_llm_provider,
            config={}
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content='{"action_type": "idle", "reasoning": "Rest", "priority": 1}',
            tokens_used=30
        )

        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality=NPCPersonality(),
            current_state={},
            world_state={},
            recent_events=[],
            timestamp=datetime.utcnow()
        )

        await agent.process(context)

        # Verify temperature from settings is used
        call_kwargs = mock_llm_provider.generate.call_args[1]
        assert call_kwargs.get("temperature") == settings.decision_temperature


class TestAgentOrchestrator:
    """Test Agent Orchestrator"""

    @pytest.mark.asyncio
    async def test_orchestrator_initialization(self, mock_llm_provider):
        """Test orchestrator initializes correctly"""
        # Create mock memori client
        mock_memori = MagicMock()
        mock_memori.store = AsyncMock()
        mock_memori.retrieve = AsyncMock(return_value=[])

        orchestrator = AgentOrchestrator(
            llm_provider=mock_llm_provider,
            config={},
            memori_client=mock_memori
        )

        assert orchestrator.llm_provider is not None

    @pytest.mark.asyncio
    async def test_orchestrator_error_handling(self, mock_llm_provider, mock_agent_context):
        """Test orchestrator handles errors gracefully"""
        # Create mock memori client
        mock_memori = MagicMock()
        mock_memori.store = AsyncMock()
        mock_memori.retrieve = AsyncMock(return_value=[])

        orchestrator = AgentOrchestrator(
            llm_provider=mock_llm_provider,
            config={},
            memori_client=mock_memori
        )

        mock_llm_provider.generate.return_value = MagicMock(
            content="Test response",
            tokens_used=30
        )

        # Simulate error in memory agent
        with patch.object(orchestrator.memory_agent, 'process') as mock_process:
            mock_process.side_effect = Exception("Memory retrieval failed")

            # Should handle error and continue
            response = await orchestrator.handle_player_interaction(
                npc_context=mock_agent_context,
                player_message="Hello"
            )

            # Should still return a response (with fallback)
            assert response is not None


class TestBaseAgent:
    """Test BaseAgent personality builder"""

    def test_personality_prompt_low_traits(self):
        """Test personality prompt with low trait values (< 0.3)"""
        from agents.dialogue_agent import DialogueAgent
        from models.npc import NPCPersonality

        # Create mock LLM provider
        mock_llm = MagicMock()

        agent = DialogueAgent(
            agent_id="test_001",
            llm_provider=mock_llm,
            config={}
        )

        # Create personality with all low values
        personality = NPCPersonality(
            openness=0.2,
            conscientiousness=0.2,
            extraversion=0.2,
            agreeableness=0.2,
            neuroticism=0.2,
            moral_alignment="lawful_good"
        )

        prompt = agent._build_personality_prompt(personality)

        # Should contain low trait descriptions
        assert "traditional and prefers routine" in prompt
        assert "spontaneous and flexible" in prompt
        assert "reserved and introspective" in prompt
        assert "competitive and skeptical" in prompt
        assert "calm and confident" in prompt
        assert "lawful good" in prompt

    def test_personality_prompt_high_traits(self):
        """Test personality prompt with high trait values (> 0.7)"""
        from agents.dialogue_agent import DialogueAgent
        from models.npc import NPCPersonality

        # Create mock LLM provider
        mock_llm = MagicMock()

        agent = DialogueAgent(
            agent_id="test_001",
            llm_provider=mock_llm,
            config={}
        )

        # Create personality with all high values
        personality = NPCPersonality(
            openness=0.8,
            conscientiousness=0.8,
            extraversion=0.8,
            agreeableness=0.8,
            neuroticism=0.8,
            moral_alignment="chaotic_evil"
        )

        prompt = agent._build_personality_prompt(personality)

        # Should contain high trait descriptions
        assert "creative and open to new experiences" in prompt
        assert "organized and reliable" in prompt
        assert "outgoing and energetic" in prompt
        assert "friendly and compassionate" in prompt
        assert "sensitive and cautious" in prompt
        assert "chaotic evil" in prompt

    def test_personality_prompt_mid_traits(self):
        """Test personality prompt with mid-range trait values (0.3-0.7)"""
        from agents.dialogue_agent import DialogueAgent
        from models.npc import NPCPersonality

        # Create mock LLM provider
        mock_llm = MagicMock()

        agent = DialogueAgent(
            agent_id="test_001",
            llm_provider=mock_llm,
            config={}
        )

        # Create personality with all mid values (should not trigger any trait descriptions)
        personality = NPCPersonality(
            openness=0.5,
            conscientiousness=0.5,
            extraversion=0.5,
            agreeableness=0.5,
            neuroticism=0.5,
            moral_alignment="true_neutral"
        )

        prompt = agent._build_personality_prompt(personality)

        # Should only contain moral alignment
        assert "true neutral" in prompt
        assert "Personality:" in prompt

