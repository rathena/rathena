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
            text="Greetings, brave adventurer! How may I assist you today?",
            usage={"total_tokens": 30}
        )
        
        response = await agent.process(mock_agent_context)
        
        assert response.success is True
        assert "Greetings" in response.data.get("dialogue", "")
        mock_llm_provider.generate.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_dialogue_input_validation(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent validates input"""
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        # Test empty message
        result = await agent.generate_dialogue(
            player_name="TestPlayer",
            player_message="",
            interaction_type="talk",
            context={}
        )
        
        assert "didn't quite catch" in result.lower()
    
    @pytest.mark.asyncio
    async def test_dialogue_length_limit(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent enforces message length limit"""
        from config import settings
        
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        # Create message longer than max length
        long_message = "a" * (settings.max_player_message_length + 100)
        
        mock_llm_provider.generate.return_value = MagicMock(
            text="Response to truncated message",
            usage={"total_tokens": 20}
        )
        
        result = await agent.generate_dialogue(
            player_name="TestPlayer",
            player_message=long_message,
            interaction_type="talk",
            context={}
        )
        
        # Should truncate and still generate response
        assert result is not None
    
    @pytest.mark.asyncio
    async def test_dialogue_fallback_on_error(self, mock_llm_provider, mock_agent_context):
        """Test dialogue agent provides fallback on LLM error"""
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        # Simulate LLM error
        mock_llm_provider.generate.side_effect = Exception("LLM API error")
        
        result = await agent.generate_dialogue(
            player_name="TestPlayer",
            player_message="Hello",
            interaction_type="talk",
            context={}
        )
        
        # Should return fallback response
        assert result is not None
        assert len(result) > 0
    
    @pytest.mark.asyncio
    async def test_dialogue_uses_config_temperature(self, mock_llm_provider):
        """Test dialogue agent uses configured temperature"""
        from config import settings
        
        agent = DialogueAgent(
            agent_id="dialogue_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        mock_llm_provider.generate.return_value = MagicMock(
            text="Test response",
            usage={"total_tokens": 20}
        )
        
        await agent.generate_dialogue(
            player_name="TestPlayer",
            player_message="Hello",
            interaction_type="talk",
            context={}
        )
        
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
            text='{"action_type": "move", "reasoning": "Patrol area", "priority": 5}',
            usage={"total_tokens": 40}
        )
        
        response = await agent.process(mock_agent_context)
        
        assert response.success is True
        assert "action_type" in response.data
    
    @pytest.mark.asyncio
    async def test_decision_uses_config_temperature(self, mock_llm_provider):
        """Test decision agent uses configured temperature"""
        from config import settings
        
        agent = DecisionAgent(
            agent_id="decision_001",
            llm_provider=mock_llm_provider,
            config={}
        )
        
        mock_llm_provider.generate.return_value = MagicMock(
            text='{"action_type": "idle", "reasoning": "Rest", "priority": 1}',
            usage={"total_tokens": 30}
        )
        
        context = AgentContext(
            npc_id="test_npc",
            npc_name="Test NPC",
            personality={},
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
        orchestrator = AgentOrchestrator(llm_provider=mock_llm_provider)
        
        assert orchestrator.llm_provider is not None
    
    @pytest.mark.asyncio
    async def test_orchestrator_error_handling(self, mock_llm_provider, mock_agent_context):
        """Test orchestrator handles errors gracefully"""
        orchestrator = AgentOrchestrator(llm_provider=mock_llm_provider)
        
        # Simulate error in one agent
        with patch.object(orchestrator, '_retrieve_memory') as mock_retrieve:
            mock_retrieve.side_effect = Exception("Memory retrieval failed")
            
            # Should handle error and continue
            response = await orchestrator.process_interaction(mock_agent_context)
            
            # Should still return a response (with fallback)
            assert response is not None

