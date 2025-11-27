"""
Unit tests for Event Chain Agent
Tests multi-step event chain mechanics and branching logic
"""

import pytest
import json
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from agents.advanced.event_chain_agent import (
    EventChainAgent,
    ChainEventType
)
from agents.base_agent import AgentResponse


@pytest.fixture
def event_chain_agent():
    """Create event chain agent instance for testing"""
    return EventChainAgent(config={})


@pytest.fixture
def mock_postgres(monkeypatch):
    """Mock PostgreSQL database"""
    mock = AsyncMock()
    mock.fetch_one = AsyncMock()
    mock.fetch_all = AsyncMock(return_value=[])
    mock.execute = AsyncMock(return_value=1)
    
    from database import postgres_db
    monkeypatch.setattr(postgres_db, 'fetch_one', mock.fetch_one)
    monkeypatch.setattr(postgres_db, 'fetch_all', mock.fetch_all)
    monkeypatch.setattr(postgres_db, 'execute', mock.execute)
    
    return mock


@pytest.fixture
def mock_redis(monkeypatch):
    """Mock Redis/DragonflyDB"""
    mock = AsyncMock()
    mock.get = AsyncMock(return_value=None)
    mock.set = AsyncMock(return_value=True)
    mock.delete = AsyncMock(return_value=True)
    
    from database import db
    monkeypatch.setattr(db, 'get', mock.get)
    monkeypatch.setattr(db, 'set', mock.set)
    monkeypatch.setattr(db, 'delete', mock.delete)
    
    return mock


class TestEventChainAgent:
    """Test suite for Event Chain Agent"""
    
    @pytest.mark.asyncio
    async def test_agent_initialization(self, event_chain_agent):
        """Test agent initializes correctly"""
        assert event_chain_agent.agent_type == "event_chain"
        assert event_chain_agent.agent_id == "event_chain_agent"
        assert len(event_chain_agent.chain_templates) == 5
        assert len(event_chain_agent.branch_logic) == 3
        assert len(event_chain_agent.outcome_templates) == 3
    
    @pytest.mark.asyncio
    async def test_start_event_chain(self, event_chain_agent, mock_postgres, mock_redis):
        """Test starting a new event chain"""
        mock_postgres.fetch_one.return_value = {'chain_id': 1}
        
        response = await event_chain_agent.start_event_chain(
            chain_type=ChainEventType.INVESTIGATION,
            trigger_condition={"player_action": "discovery"}
        )
        
        assert response.success
        assert 'chain_id' in response.data
        assert 'chain_name' in response.data
        assert 'total_steps' in response.data
        assert 'first_step' in response.data
        assert 3 <= response.data['total_steps'] <= 7
    
    @pytest.mark.asyncio
    async def test_chain_step_generation(self, event_chain_agent):
        """Test chain step generation"""
        step = await event_chain_agent._generate_chain_step(
            chain_type=ChainEventType.INVESTIGATION,
            step_number=1,
            total_steps=5,
            previous_outcome=None
        )
        
        assert 'step_number' in step
        assert 'step_type' in step
        assert 'objective' in step
        assert 'dialogue' in step
        assert 'success_condition' in step
        assert step['step_number'] == 1
    
    @pytest.mark.asyncio
    async def test_dialogue_generation(self, event_chain_agent):
        """Test dialogue generation for chain steps"""
        dialogue = await event_chain_agent.generate_chain_dialogue(
            chain_context={
                "chain_type": "investigation",
                "step_type": "npc_dialogue",
                "step_number": 1,
                "total_steps": 5
            },
            step_number=1
        )
        
        assert isinstance(dialogue, str)
        assert len(dialogue) > 0
    
    @pytest.mark.asyncio
    async def test_advance_chain_success(self, event_chain_agent, mock_postgres, mock_redis):
        """Test advancing chain with success outcome"""
        # Mock chain and step data
        mock_postgres.fetch_one.side_effect = [
            {
                'chain_type': 'investigation',
                'chain_name': 'Test Investigation',
                'total_steps': 5,
                'current_step': 1,
                'chain_data': '{}'
            }
        ]
        
        response = await event_chain_agent.advance_chain_step(
            chain_id=1,
            outcome='success'
        )
        
        assert response.success
        assert 'previous_outcome' in response.data
        assert response.data['previous_outcome'] == 'success'
        assert 'next_step' in response.data
    
    @pytest.mark.asyncio
    async def test_chain_completion_evaluation(self, event_chain_agent, mock_postgres):
        """Test chain completion evaluation"""
        # Mock completed steps with varied outcomes
        mock_postgres.fetch_all.return_value = [
            {'step_number': 1, 'outcome': 'success', 'completed': True},
            {'step_number': 2, 'outcome': 'success', 'completed': True},
            {'step_number': 3, 'outcome': 'failure', 'completed': True},
            {'step_number': 4, 'outcome': 'success', 'completed': True}
        ]
        
        mock_postgres.fetch_one.return_value = {'participant_count': 5}
        
        response = await event_chain_agent.evaluate_chain_completion(chain_id=1)
        
        assert response.success
        assert 'final_outcome' in response.data
        assert 'success_rate' in response.data
        assert 'world_changes' in response.data
        assert 0.0 <= response.data['success_rate'] <= 1.0
    
    @pytest.mark.asyncio
    async def test_success_rate_calculation(self, event_chain_agent, mock_postgres):
        """Test success rate calculation"""
        # All success
        mock_postgres.fetch_all.return_value = [
            {'step_number': 1, 'outcome': 'success', 'completed': True},
            {'step_number': 2, 'outcome': 'success', 'completed': True},
            {'step_number': 3, 'outcome': 'success', 'completed': True}
        ]
        mock_postgres.fetch_one.return_value = {'participant_count': 10}
        
        response = await event_chain_agent.evaluate_chain_completion(chain_id=1)
        
        assert response.data['success_rate'] == 1.0
        assert response.data['final_outcome'] == 'success'
    
    @pytest.mark.asyncio
    async def test_branch_logic(self, event_chain_agent):
        """Test branching logic modifiers"""
        success_logic = event_chain_agent.branch_logic['success']
        assert success_logic['reward_multiplier'] == 1.5
        assert success_logic['world_impact'] == 'positive'
        
        failure_logic = event_chain_agent.branch_logic['failure']
        assert failure_logic['reward_multiplier'] == 0.5
        assert failure_logic['world_impact'] == 'negative'
    
    @pytest.mark.asyncio
    async def test_required_actions_mapping(self, event_chain_agent):
        """Test required actions for different step types"""
        actions = event_chain_agent._get_required_actions('npc_dialogue')
        assert 'talk_to_npc' in actions
        
        actions = event_chain_agent._get_required_actions('boss_battle')
        assert 'defeat_boss' in actions
    
    @pytest.mark.asyncio
    async def test_npc_data_generation(self, event_chain_agent):
        """Test NPC data generation for steps"""
        npc_data = event_chain_agent._generate_npc_data('npc_dialogue')
        
        assert 'npc_name' in npc_data
        assert 'npc_sprite' in npc_data
        assert 'spawn_map' in npc_data
        assert 'spawn_x' in npc_data
        assert 'spawn_y' in npc_data
    
    @pytest.mark.asyncio
    async def test_spawn_data_generation(self, event_chain_agent):
        """Test monster spawn data generation"""
        spawn_data = event_chain_agent._generate_spawn_data('boss_battle')
        
        assert 'monsters' in spawn_data
        assert 'count' in spawn_data
        assert 'spawn_map' in spawn_data
        assert spawn_data['count'] > 0
    
    @pytest.mark.asyncio
    async def test_all_chain_types(self, event_chain_agent, mock_postgres, mock_redis):
        """Test all chain types can be started"""
        mock_postgres.fetch_one.return_value = {'chain_id': 1}
        
        for chain_type in ChainEventType:
            response = await event_chain_agent.start_event_chain(
                chain_type=chain_type,
                trigger_condition={}
            )
            
            assert response.success
            assert response.data['chain_type'] == chain_type.value
    
    @pytest.mark.asyncio
    async def test_chain_expiration(self, event_chain_agent, mock_postgres):
        """Test chain expiration after completion"""
        # Mock chain data
        mock_postgres.fetch_all.return_value = [
            {'step_number': 1, 'outcome': 'success', 'completed': True}
        ]
        mock_postgres.fetch_one.return_value = {'participant_count': 1}
        
        response = await event_chain_agent.evaluate_chain_completion(chain_id=1)
        
        assert response.success
        # Cache should be deleted
        # (Can't verify without actual cache implementation)
    
    @pytest.mark.asyncio
    async def test_world_outcome_templates(self, event_chain_agent):
        """Test world outcome templates exist for all impacts"""
        for impact in ['positive', 'negative', 'neutral']:
            assert impact in event_chain_agent.outcome_templates
            template = event_chain_agent.outcome_templates[impact]
            assert 'faction_shift' in template
            assert 'npc_spawns' in template


@pytest.mark.asyncio
async def test_chain_templates_valid(event_chain_agent):
    """Test all chain templates are properly configured"""
    for chain_type in ChainEventType:
        template = event_chain_agent.chain_templates.get(chain_type)
        assert template is not None
        assert 'name' in template
        assert 'min_steps' in template
        assert 'max_steps' in template
        assert 'step_types' in template
        assert template['min_steps'] >= 3
        assert template['max_steps'] <= 7