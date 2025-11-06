"""
Tests for AI Agent System
Tests all specialized agents and orchestration
"""

import pytest
import asyncio
from datetime import datetime
from unittest.mock import Mock, AsyncMock, patch

# Import agent classes
import sys
sys.path.insert(0, '../ai-service')

from agents.base_agent import AgentContext, AgentResponse
from agents.dialogue_agent import DialogueAgent
from agents.decision_agent import DecisionAgent
from agents.memory_agent import MemoryAgent
from agents.world_agent import WorldAgent
from agents.orchestrator import AgentOrchestrator
from models.npc import NPCPersonality


# Mock LLM Provider
class MockLLMProvider:
    """Mock LLM provider for testing"""
    
    async def generate(self, prompt: str, system_message: str = None, temperature: float = 0.7):
        """Mock generate method"""
        mock_response = Mock()
        mock_response.text = "This is a test response from the NPC."
        return mock_response


@pytest.fixture
def mock_llm():
    """Fixture for mock LLM provider"""
    return MockLLMProvider()


@pytest.fixture
def test_personality():
    """Fixture for test NPC personality"""
    return NPCPersonality(
        openness=0.7,
        conscientiousness=0.8,
        extraversion=0.6,
        agreeableness=0.9,
        neuroticism=0.3,
        moral_alignment="lawful_good"
    )


@pytest.fixture
def test_context(test_personality):
    """Fixture for test agent context"""
    return AgentContext(
        npc_id="test_npc_001",
        npc_name="Test Merchant",
        personality=test_personality,
        current_state={
            "npc_class": "merchant",
            "player_id": "player_001",
            "player_name": "TestPlayer",
            "player_message": "Hello!",
            "interaction_type": "talk",
            "location": {"map": "prontera", "x": 150, "y": 180},
            "time_of_day": "day",
            "weather": "clear"
        },
        world_state={
            "economy": {"inflation_rate": 0.02, "trade_volume": 1000000},
            "politics": {"conflict_level": 0.1},
            "environment": {"weather": "clear", "season": "spring"}
        },
        recent_events=[]
    )


class TestDialogueAgent:
    """Test Dialogue Agent"""
    
    @pytest.mark.asyncio
    async def test_dialogue_agent_initialization(self, mock_llm):
        """Test dialogue agent can be initialized"""
        agent = DialogueAgent(
            agent_id="dialogue_test",
            llm_provider=mock_llm,
            config={"verbose": False}
        )
        
        assert agent.agent_id == "dialogue_test"
        assert agent.agent_type == "dialogue"
    
    @pytest.mark.asyncio
    async def test_dialogue_generation(self, mock_llm, test_context):
        """Test dialogue generation"""
        agent = DialogueAgent(
            agent_id="dialogue_test",
            llm_provider=mock_llm,
            config={"verbose": False}
        )
        
        response = await agent.process(test_context)
        
        assert response.success
        assert response.agent_type == "dialogue"
        assert "text" in response.data
        assert "emotion" in response.data


class TestDecisionAgent:
    """Test Decision Agent"""
    
    @pytest.mark.asyncio
    async def test_decision_agent_initialization(self, mock_llm):
        """Test decision agent can be initialized"""
        agent = DecisionAgent(
            agent_id="decision_test",
            llm_provider=mock_llm,
            config={"verbose": False}
        )
        
        assert agent.agent_id == "decision_test"
        assert agent.agent_type == "decision"
    
    @pytest.mark.asyncio
    async def test_action_decision(self, mock_llm, test_context):
        """Test action decision making"""
        agent = DecisionAgent(
            agent_id="decision_test",
            llm_provider=mock_llm,
            config={"verbose": False}
        )
        
        response = await agent.process(test_context)
        
        assert response.success
        assert response.agent_type == "decision"
        assert "action_type" in response.data


class TestMemoryAgent:
    """Test Memory Agent"""
    
    @pytest.mark.asyncio
    async def test_memory_agent_initialization(self, mock_llm):
        """Test memory agent can be initialized"""
        agent = MemoryAgent(
            agent_id="memory_test",
            llm_provider=mock_llm,
            config={"verbose": False},
            memori_client=None
        )
        
        assert agent.agent_id == "memory_test"
        assert agent.agent_type == "memory"
    
    @pytest.mark.asyncio
    async def test_memory_storage(self, mock_llm, test_context):
        """Test memory storage"""
        agent = MemoryAgent(
            agent_id="memory_test",
            llm_provider=mock_llm,
            config={"verbose": False},
            memori_client=None
        )
        
        # Modify context for memory storage
        test_context.current_state["operation"] = "store"
        test_context.current_state["memory_data"] = {
            "type": "interaction",
            "content": "Test interaction",
            "participants": ["player_001"],
            "importance": 5
        }
        
        response = await agent.process(test_context)
        
        assert response.success
        assert response.agent_type == "memory"


class TestWorldAgent:
    """Test World Agent"""
    
    @pytest.mark.asyncio
    async def test_world_agent_initialization(self, mock_llm):
        """Test world agent can be initialized"""
        agent = WorldAgent(
            agent_id="world_test",
            llm_provider=mock_llm,
            config={"verbose": False}
        )
        
        assert agent.agent_id == "world_test"
        assert agent.agent_type == "world"


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])

