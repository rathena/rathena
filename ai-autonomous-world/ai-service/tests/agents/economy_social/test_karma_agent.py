"""
Unit tests for Karma Agent
Tests karma tracking, alignment calculation, and world effects
"""

import pytest
import json
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, Mock, patch

from agents.economy_social.karma_agent import (
    KarmaAgent,
    karma_agent,
    KarmaAlignment
)
from agents.base_agent import AgentResponse


@pytest.fixture
def sample_karma_actions():
    """Sample karma actions for testing"""
    return [
        {
            "player_id": 100001,
            "action_type": "help_npc_quest",
            "data": {"quest_id": 1}
        },
        {
            "player_id": 100002,
            "action_type": "kill_poring",
            "data": {"mob_id": 1002}
        },
        {
            "player_id": 100003,
            "action_type": "donate_temple",
            "data": {"amount": 10000}
        },
        {
            "player_id": 100004,
            "action_type": "help_shadow_cult",
            "data": {"quest_id": 50}
        }
    ]


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.economy_social.karma_agent.postgres_db') as mock_pg, \
         patch('agents.economy_social.karma_agent.db') as mock_cache:
        
        # Mock PostgreSQL methods
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        # Mock cache methods
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestKarmaAgent:
    """Test suite for Karma Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = KarmaAgent()
        
        assert agent.agent_type == "karma"
        assert len(agent.alignment_thresholds) == 5
        assert len(agent.action_karma_values) > 10
        assert len(agent.alignment_effects) == 5
    
    @pytest.mark.asyncio
    async def test_classify_action_morality_good(self):
        """Test good action classification"""
        agent = KarmaAgent()
        
        karma = await agent.classify_action_morality('help_npc_quest')
        
        assert karma > 0
        assert karma == 15  # Defined value
    
    @pytest.mark.asyncio
    async def test_classify_action_morality_evil(self):
        """Test evil action classification"""
        agent = KarmaAgent()
        
        karma = await agent.classify_action_morality('help_shadow_cult')
        
        assert karma < 0
        assert karma == -25  # Defined value
    
    @pytest.mark.asyncio
    async def test_classify_action_morality_protected_monster(self):
        """Test protected monster kill gives negative karma"""
        agent = KarmaAgent()
        
        karma = await agent.classify_action_morality('kill_monster', target='PORING')
        
        assert karma < 0
        assert karma == -10
    
    @pytest.mark.asyncio
    async def test_classify_action_morality_demon(self):
        """Test demon kill gives positive karma"""
        agent = KarmaAgent()
        
        karma = await agent.classify_action_morality('kill_monster', target='DEMON_LORD')
        
        assert karma > 0
        assert karma == 5
    
    @pytest.mark.asyncio
    async def test_classify_action_morality_unknown(self):
        """Test unknown action gives zero karma"""
        agent = KarmaAgent()
        
        karma = await agent.classify_action_morality('unknown_action')
        
        assert karma == 0
    
    def test_determine_alignment_very_good(self):
        """Test very good alignment determination"""
        agent = KarmaAgent()
        
        alignment = agent._determine_alignment(8000)
        
        assert alignment == KarmaAlignment.VERY_GOOD
    
    def test_determine_alignment_good(self):
        """Test good alignment determination"""
        agent = KarmaAgent()
        
        alignment = agent._determine_alignment(5000)
        
        assert alignment == KarmaAlignment.GOOD
    
    def test_determine_alignment_neutral(self):
        """Test neutral alignment determination"""
        agent = KarmaAgent()
        
        alignment = agent._determine_alignment(0)
        
        assert alignment == KarmaAlignment.NEUTRAL
    
    def test_determine_alignment_evil(self):
        """Test evil alignment determination"""
        agent = KarmaAgent()
        
        alignment = agent._determine_alignment(-5000)
        
        assert alignment == KarmaAlignment.EVIL
    
    def test_determine_alignment_very_evil(self):
        """Test very evil alignment determination"""
        agent = KarmaAgent()
        
        alignment = agent._determine_alignment(-8000)
        
        assert alignment == KarmaAlignment.VERY_EVIL
    
    @pytest.mark.asyncio
    async def test_update_global_karma_alignment_shift(self, sample_karma_actions, mock_db):
        """Test global karma update with alignment shift"""
        agent = KarmaAgent()
        
        # Mock current karma as neutral
        mock_db['postgres'].fetch_one.side_effect = [
            {"karma_score": 0},  # Current karma
            {"karma_score": 15}  # Player karma update
        ]
        
        # Mock apply_karma_effects
        with patch.object(agent, 'apply_karma_effects', return_value={'test': 'effects'}):
            response = await agent.update_global_karma(sample_karma_actions)
        
        assert response.success
        assert 'karma_change' in response.data
        assert 'new_alignment' in response.data
    
    @pytest.mark.asyncio
    async def test_track_player_karma(self, mock_db):
        """Test player karma tracking"""
        agent = KarmaAgent()
        
        # Mock player karma update
        mock_db['postgres'].fetch_one.return_value = {"karma_score": 150}
        
        new_karma = await agent.track_player_karma(
            player_id=100001,
            karma_change=50
        )
        
        assert new_karma == 150
        assert mock_db['postgres'].fetch_one.called
        assert mock_db['cache'].set.called
    
    @pytest.mark.asyncio
    async def test_apply_karma_effects_very_good(self):
        """Test karma effects application for very good alignment"""
        agent = KarmaAgent()
        
        effects = await agent.apply_karma_effects(KarmaAlignment.VERY_GOOD)
        
        assert effects['alignment'] == 'very_good'
        assert effects['day_length_modifier'] == 1.3
        assert effects['heal_rate_modifier'] == 1.2
        assert len(effects['special_npcs']) > 0
    
    @pytest.mark.asyncio
    async def test_apply_karma_effects_very_evil(self):
        """Test karma effects application for very evil alignment"""
        agent = KarmaAgent()
        
        effects = await agent.apply_karma_effects(KarmaAlignment.VERY_EVIL)
        
        assert effects['alignment'] == 'very_evil'
        assert effects['day_length_modifier'] == 0.7
        assert effects['heal_rate_modifier'] == 0.8
        assert effects['atmosphere'] == 'dark'
    
    @pytest.mark.asyncio
    async def test_apply_daily_decay(self, mock_db):
        """Test daily karma decay"""
        agent = KarmaAgent()
        
        # Mock current karma at 5000
        mock_db['postgres'].fetch_one.return_value = {"karma_score": 5000}
        mock_db['postgres'].execute.return_value = 10  # 10 players affected
        
        affected = await agent.apply_daily_decay()
        
        assert affected == 10
        assert mock_db['postgres'].execute.called
    
    @pytest.mark.asyncio
    async def test_get_global_karma_state(self, mock_db):
        """Test global karma state retrieval"""
        agent = KarmaAgent()
        
        # Mock karma state
        karma_data = {
            "karma_score": 5000,
            "alignment": "good",
            "good_actions_count": 100,
            "evil_actions_count": 20,
            "last_shift": datetime.now(UTC)
        }
        mock_db['postgres'].fetch_one.return_value = karma_data
        
        state = await agent.get_global_karma_state()
        
        assert state is not None
        assert state['karma_score'] == 5000
        assert state['alignment'] == "good"
        assert 'effects' in state


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert karma_agent is not None
    assert karma_agent.agent_type == "karma"