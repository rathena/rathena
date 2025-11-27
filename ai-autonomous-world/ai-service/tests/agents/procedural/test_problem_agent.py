"""
Unit tests for Problem Agent
Tests problem generation, caching, and reward calculation
"""

import pytest
import json
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, Mock, patch

from agents.procedural.problem_agent import ProblemAgent, problem_agent
from agents.base_agent import AgentResponse
from models.procedural import ProblemType, ProblemStatus


@pytest.fixture
def sample_world_state():
    """Sample world state for testing"""
    return {
        "avg_player_level": 50,
        "online_players": 15,
        "map_activity": {
            "prontera": 20,
            "geffen": 8,
            "payon": 5,
            "morocc": 3
        },
        "monster_kills": {
            "prt_fild01": 100,
            "prt_fild02": 80
        },
        "mvp_kills": {
            "1038": 5,
            "1039": 3
        },
        "economy": {
            "zeny_circulation": 500000000,
            "inflation_rate": 1.05,
            "scarce_items": ["Old Card Album"]
        }
    }


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.procedural.problem_agent.postgres_db') as mock_pg, \
         patch('agents.procedural.problem_agent.db') as mock_cache:
        
        # Mock PostgreSQL methods
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        # Mock cache methods
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestProblemAgent:
    """Test suite for Problem Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = ProblemAgent()
        
        assert agent.agent_type == "problem"
        assert len(agent.problem_types) == 6
        assert len(agent.difficulty_scaling) == 10
    
    @pytest.mark.asyncio
    async def test_decide_problem_type_rule_based(self, sample_world_state):
        """Test Tier 1 rule-based problem type selection"""
        agent = ProblemAgent()
        
        # Test with low player count (should prefer easier problems)
        low_player_state = {**sample_world_state, "online_players": 5}
        problem_type = await agent.decide_problem_type(low_player_state)
        
        assert problem_type in agent.problem_types
        
        # Verify difficulty is appropriate for low player count
        config = agent.problem_types[problem_type]
        assert config['difficulty_range'][0] <= 3
    
    @pytest.mark.asyncio
    async def test_decide_problem_type_forced(self, sample_world_state):
        """Test forced problem type selection"""
        agent = ProblemAgent()
        
        forced_type = ProblemType.MVP_RAMPAGE
        result = await agent.decide_problem_type(sample_world_state, forced_type)
        
        assert result == forced_type
    
    @pytest.mark.asyncio
    async def test_calculate_rewards_scaling(self, sample_world_state):
        """Test reward calculation with difficulty scaling"""
        agent = ProblemAgent()
        
        # Test low difficulty rewards
        rewards_low = await agent.calculate_rewards(
            ProblemType.WEATHER_HAZARD,
            difficulty=2,
            world_state=sample_world_state
        )
        
        # Test high difficulty rewards
        rewards_high = await agent.calculate_rewards(
            ProblemType.MVP_RAMPAGE,
            difficulty=9,
            world_state=sample_world_state
        )
        
        # High difficulty should give more rewards
        assert rewards_high['exp'] > rewards_low['exp']
        assert rewards_high['zeny'] > rewards_low['zeny']
        assert len(rewards_high['items']) >= len(rewards_low['items'])
    
    @pytest.mark.asyncio
    async def test_calculate_rewards_participation_bonus(self, sample_world_state):
        """Test participation bonus in reward calculation"""
        agent = ProblemAgent()
        
        # Low player count
        low_player_state = {**sample_world_state, "online_players": 5}
        rewards_low = await agent.calculate_rewards(
            ProblemType.MONSTER_SURGE,
            5,
            low_player_state
        )
        
        # High player count
        high_player_state = {**sample_world_state, "online_players": 50}
        rewards_high = await agent.calculate_rewards(
            ProblemType.MONSTER_SURGE,
            5,
            high_player_state
        )
        
        # More players = higher rewards
        assert rewards_high['exp'] > rewards_low['exp']
    
    @pytest.mark.asyncio
    async def test_generate_daily_problem(self, sample_world_state, mock_db):
        """Test full daily problem generation"""
        agent = ProblemAgent()
        
        # Mock database to return problem ID
        mock_db['postgres'].fetch_one.return_value = {"problem_id": 123}
        
        response = await agent.generate_daily_problem(sample_world_state)
        
        assert response.success
        assert "problem_id" in response.data
        assert "title" in response.data
        assert "reward_data" in response.data
        
        # Verify database calls
        assert mock_db['postgres'].fetch_one.called
        assert mock_db['cache'].set.called
    
    @pytest.mark.asyncio
    async def test_generate_daily_problem_existing(self, sample_world_state, mock_db):
        """Test that existing active problem is returned"""
        agent = ProblemAgent()
        
        # Mock existing active problem
        existing_problem = {
            "problem_id": 100,
            "title": "Test Problem",
            "status": "active"
        }
        
        # First query returns existing problem
        mock_db['postgres'].fetch_one.return_value = existing_problem
        
        response = await agent.generate_daily_problem(sample_world_state)
        
        assert response.success
        assert response.data['problem_id'] == 100
        assert "already exists" in response.reasoning.lower()
    
    @pytest.mark.asyncio
    async def test_record_participation(self, mock_db):
        """Test recording player participation"""
        agent = ProblemAgent()
        
        success = await agent.record_participation(
            problem_id=1,
            player_id=100001,
            action_type="monster_kill",
            action_data={"mob_id": 1002, "count": 10},
            contribution_score=100
        )
        
        assert success
        assert mock_db['postgres'].execute.call_count == 2  # INSERT + UPDATE
    
    @pytest.mark.asyncio
    async def test_complete_problem(self, mock_db):
        """Test problem completion"""
        agent = ProblemAgent()
        
        # Mock successful update (1 row affected)
        mock_db['postgres'].execute.return_value = 1
        
        success = await agent.complete_problem(problem_id=1)
        
        assert success
        assert mock_db['postgres'].execute.called
        assert mock_db['cache'].delete.called
    
    def test_select_target_map_high_difficulty(self):
        """Test map selection for high difficulty problems"""
        agent = ProblemAgent()
        
        map_activity = {
            "prontera": 100,
            "geffen": 50,
            "payon": 10
        }
        
        # High difficulty should select high activity map
        target = agent._select_target_map(map_activity, difficulty=8)
        assert target == "prontera"
    
    def test_select_target_map_low_difficulty(self):
        """Test map selection for low difficulty problems"""
        agent = ProblemAgent()
        
        map_activity = {
            "prontera": 100,
            "geffen": 50,
            "payon": 10
        }
        
        # Low difficulty should select low activity map
        target = agent._select_target_map(map_activity, difficulty=2)
        assert target == "payon"
    
    def test_calculate_problem_weights(self):
        """Test weight calculation for problem type selection"""
        agent = ProblemAgent()
        
        available = [
            ProblemType.MONSTER_SURGE,
            ProblemType.MVP_RAMPAGE,
            ProblemType.WEATHER_HAZARD
        ]
        
        # Recent history with many monster surges
        recent = [
            ProblemType.MONSTER_SURGE,
            ProblemType.MONSTER_SURGE,
            ProblemType.MONSTER_SURGE
        ]
        
        weights = agent._calculate_problem_weights(available, recent)
        
        # Monster surge should have lower weight (more recent)
        surge_idx = available.index(ProblemType.MONSTER_SURGE)
        weather_idx = available.index(ProblemType.WEATHER_HAZARD)
        
        assert weights[surge_idx] < weights[weather_idx]
    
    def test_generate_cache_key_consistency(self, sample_world_state):
        """Test cache key generation is deterministic"""
        agent = ProblemAgent()
        
        key1 = agent._generate_cache_key(sample_world_state)
        key2 = agent._generate_cache_key(sample_world_state)
        
        assert key1 == key2
        assert key1.startswith("problem:decision:")
    
    def test_generate_cache_key_variation(self, sample_world_state):
        """Test cache key changes with state changes"""
        agent = ProblemAgent()
        
        key1 = agent._generate_cache_key(sample_world_state)
        
        # Change state
        modified_state = {**sample_world_state, "avg_player_level": 80}
        key2 = agent._generate_cache_key(modified_state)
        
        assert key1 != key2


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert problem_agent is not None
    assert problem_agent.agent_type == "problem"