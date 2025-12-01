"""
Unit tests for Reputation Agent
Tests player influence calculation, reputation gains, and benefit unlocks
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch
from datetime import datetime, UTC

from agents.progression.reputation_agent import ReputationAgent, ReputationType
from agents.base_agent import AgentResponse


@pytest.fixture
def reputation_agent():
    """Create reputation agent instance"""
    return ReputationAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.progression.reputation_agent.postgres_db') as mock_pg, \
         patch('agents.progression.reputation_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestReputationAgentInitialization:
    """Test reputation agent initialization"""
    
    def test_initialization(self, reputation_agent):
        """Test reputation agent initializes correctly"""
        assert reputation_agent.agent_type == "reputation"
        assert reputation_agent.agent_id == "reputation_agent"
        assert len(reputation_agent.reputation_weights) == 5
        assert len(reputation_agent.influence_tiers) == 6
    
    def test_reputation_weights(self, reputation_agent):
        """Test reputation weights sum to 1.0"""
        total_weight = sum(reputation_agent.reputation_weights.values())
        assert abs(total_weight - 1.0) < 0.01  # Allow small floating point error
    
    def test_influence_tiers(self, reputation_agent):
        """Test influence tier configuration"""
        assert reputation_agent.influence_tiers[0]["threshold"] == 0
        assert reputation_agent.influence_tiers[5]["threshold"] == 10000
        assert reputation_agent.influence_tiers[5]["name"] == "Mythic"


class TestCalculateTotalInfluence:
    """Test total influence calculation"""
    
    @pytest.mark.asyncio
    async def test_calculate_influence_cached(self, reputation_agent, mock_db):
        """Test influence calculation uses cache"""
        # Setup cache hit
        cached_data = {
            "total_influence": 5000,
            "tier": 3,
            "tier_name": "Renowned",
            "next_tier_progress": 0.0,
            "breakdown": {}
        }
        mock_db['cache'].get.return_value = cached_data
        
        # Execute
        influence = await reputation_agent.calculate_total_influence(player_id=123)
        
        # Verify
        assert influence == cached_data
        mock_db['postgres'].fetch_one.assert_not_called()
    
    @pytest.mark.asyncio
    async def test_calculate_influence_from_db(self, reputation_agent, mock_db):
        """Test influence calculation from database"""
        # Setup cache miss
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            "total_influence": 3500,
            "current_tier": 2,
            "world_guardian_rep": 1000,
            "explorer_rep": 800,
            "problem_solver_rep": 600,
            "event_participant_rep": 400,
            "faction_loyalist_rep": 200
        }
        
        # Execute
        influence = await reputation_agent.calculate_total_influence(player_id=123)
        
        # Verify
        assert influence["total_influence"] == 3500
        assert influence["tier"] == 2
        assert influence["tier_name"] == "Respected"
        assert "breakdown" in influence
        mock_db['cache'].set.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_tier_progress_calculation(self, reputation_agent, mock_db):
        """Test tier progress calculation"""
        # Setup player between tiers
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            "total_influence": 2000,  # Between tier 1 (1000) and tier 2 (3000)
            "current_tier": 1,
            "world_guardian_rep": 500,
            "explorer_rep": 400,
            "problem_solver_rep": 300,
            "event_participant_rep": 200,
            "faction_loyalist_rep": 100
        }
        
        # Execute
        influence = await reputation_agent.calculate_total_influence(player_id=123)
        
        # Verify progress calculation
        assert influence["next_tier_progress"] == 0.5  # Halfway from 1000 to 3000


class TestRecordReputationGain:
    """Test reputation gain recording"""
    
    @pytest.mark.asyncio
    async def test_record_gain_success(self, reputation_agent, mock_db):
        """Test successful reputation gain"""
        # Setup
        mock_db['postgres'].fetch_one.side_effect = [
            None,  # Daily gain check
            {"total_influence": 1000, "current_tier": 1, "world_guardian_rep": 100, "explorer_rep": 0, "problem_solver_rep": 0, "event_participant_rep": 0, "faction_loyalist_rep": 0},  # Old influence
            {"total_influence": 1030, "current_tier": 1}  # New influence
        ]
        mock_db['cache'].get.return_value = None
        
        # Execute
        response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type=ReputationType.WORLD_GUARDIAN,
            amount=100,
            source="solved_problem"
        )
        
        # Verify
        assert response.success is True
        assert response.data["amount_gained"] == 100
        assert response.data["influence_gained"] == 30  # 100 * 0.30 weight
    
    @pytest.mark.asyncio
    async def test_record_gain_negative_amount(self, reputation_agent, mock_db):
        """Test negative amount rejected"""
        response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type=ReputationType.EXPLORER,
            amount=-50,
            source="test"
        )
        
        assert response.success is False
        assert "positive" in response.reasoning.lower()
    
    @pytest.mark.asyncio
    async def test_record_gain_invalid_type(self, reputation_agent, mock_db):
        """Test invalid reputation type rejected"""
        response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type="invalid_type",
            amount=100,
            source="test"
        )
        
        assert response.success is False
        assert "invalid" in response.reasoning.lower()
    
    @pytest.mark.asyncio
    async def test_daily_cap_enforced(self, reputation_agent, mock_db):
        """Test daily reputation cap is enforced"""
        # Setup - player at daily cap
        mock_db['postgres'].fetch_one.return_value = {"daily_total": 500}
        
        # Execute
        response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type=ReputationType.PROBLEM_SOLVER,
            amount=100,
            source="test"
        )
        
        # Verify
        assert response.success is False
        assert "cap" in response.reasoning.lower()


class TestGetUnlockedBenefits:
    """Test benefit unlock retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_benefits_tier_0(self, reputation_agent, mock_db):
        """Test tier 0 has no benefits"""
        # Setup
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            "total_influence": 0,
            "current_tier": 0,
            "world_guardian_rep": 0,
            "explorer_rep": 0,
            "problem_solver_rep": 0,
            "event_participant_rep": 0,
            "faction_loyalist_rep": 0
        }
        
        # Execute
        benefits = await reputation_agent.get_unlocked_benefits(player_id=123)
        
        # Verify
        assert benefits["tier"] == 0
        assert len(benefits["shops"]) == 0
        assert len(benefits["titles"]) == 0
    
    @pytest.mark.asyncio
    async def test_get_benefits_tier_3(self, reputation_agent, mock_db):
        """Test tier 3 includes all lower tier benefits"""
        # Setup
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            "total_influence": 5500,
            "current_tier": 3,
            "world_guardian_rep": 2000,
            "explorer_rep": 1500,
            "problem_solver_rep": 1000,
            "event_participant_rep": 500,
            "faction_loyalist_rep": 500
        }
        
        # Execute
        benefits = await reputation_agent.get_unlocked_benefits(player_id=123)
        
        # Verify
        assert benefits["tier"] == 3
        assert benefits["tier_name"] == "Renowned"
        # Should have benefits from tiers 1, 2, and 3
        assert len(benefits["shops"]) >= 3
        assert len(benefits["services"]) > 0


class TestCheckTierProgression:
    """Test tier progression checking"""
    
    @pytest.mark.asyncio
    async def test_tier_advancement(self, reputation_agent, mock_db):
        """Test tier advancement is detected"""
        # Setup
        mock_db['postgres'].execute.return_value = 1
        
        # Execute - cross from tier 0 to tier 1
        tier_change = await reputation_agent.check_tier_progression(
            player_id=123,
            old_influence=900,
            new_influence=1100
        )
        
        # Verify
        assert tier_change is not None
        assert tier_change["advanced"] is True
        assert tier_change["old_tier"] == 0
        assert tier_change["new_tier"] == 1
        assert tier_change["new_tier_name"] == "Known"
    
    @pytest.mark.asyncio
    async def test_no_tier_change(self, reputation_agent, mock_db):
        """Test no tier change when staying in same tier"""
        # Execute - stay in tier 1
        tier_change = await reputation_agent.check_tier_progression(
            player_id=123,
            old_influence=1500,
            new_influence=1800
        )
        
        # Verify
        assert tier_change is None
    
    @pytest.mark.asyncio
    async def test_tier_advancement_multiple(self, reputation_agent, mock_db):
        """Test multiple tier advancement at once"""
        # Setup
        mock_db['postgres'].execute.return_value = 1
        
        # Execute - jump from tier 1 to tier 3
        tier_change = await reputation_agent.check_tier_progression(
            player_id=123,
            old_influence=2000,
            new_influence=5500
        )
        
        # Verify
        assert tier_change is not None
        assert tier_change["old_tier"] == 1
        assert tier_change["new_tier"] == 3


class TestGetReputationLeaderboard:
    """Test reputation leaderboard"""
    
    @pytest.mark.asyncio
    async def test_leaderboard_retrieval(self, reputation_agent, mock_db):
        """Test leaderboard retrieval"""
        # Setup
        mock_db['postgres'].fetch_all.return_value = [
            {"player_id": 1, "total_influence": 10000, "current_tier": 5},
            {"player_id": 2, "total_influence": 7500, "current_tier": 4},
            {"player_id": 3, "total_influence": 5000, "current_tier": 3}
        ]
        
        # Execute
        leaderboard = await reputation_agent.get_reputation_leaderboard(limit=3)
        
        # Verify
        assert len(leaderboard) == 3
        assert leaderboard[0]["rank"] == 1
        assert leaderboard[0]["player_id"] == 1
        assert leaderboard[0]["total_influence"] == 10000
        assert leaderboard[0]["tier_name"] == "Mythic"
    
    @pytest.mark.asyncio
    async def test_leaderboard_ordering(self, reputation_agent, mock_db):
        """Test leaderboard is correctly ordered"""
        # Setup
        mock_db['postgres'].fetch_all.return_value = [
            {"player_id": 5, "total_influence": 8000, "current_tier": 5},
            {"player_id": 3, "total_influence": 6000, "current_tier": 3},
            {"player_id": 1, "total_influence": 4000, "current_tier": 2}
        ]
        
        # Execute
        leaderboard = await reputation_agent.get_reputation_leaderboard(limit=10)
        
        # Verify ordering
        for i in range(len(leaderboard) - 1):
            assert leaderboard[i]["total_influence"] >= leaderboard[i + 1]["total_influence"]


class TestTierCalculation:
    """Test tier calculation utilities"""
    
    def test_tier_from_influence(self, reputation_agent):
        """Test tier calculated correctly from influence score"""
        assert reputation_agent._calculate_tier_from_influence(0) == 0
        assert reputation_agent._calculate_tier_from_influence(999) == 0
        assert reputation_agent._calculate_tier_from_influence(1000) == 1
        assert reputation_agent._calculate_tier_from_influence(2999) == 1
        assert reputation_agent._calculate_tier_from_influence(3000) == 2
        assert reputation_agent._calculate_tier_from_influence(4999) == 2
        assert reputation_agent._calculate_tier_from_influence(5000) == 3
        assert reputation_agent._calculate_tier_from_influence(6999) == 3
        assert reputation_agent._calculate_tier_from_influence(7000) == 4
        assert reputation_agent._calculate_tier_from_influence(9999) == 4
        assert reputation_agent._calculate_tier_from_influence(10000) == 5
        assert reputation_agent._calculate_tier_from_influence(20000) == 5  # Cap at 5


class TestReputationWeights:
    """Test reputation weight application"""
    
    def test_world_guardian_weight(self, reputation_agent):
        """Test world guardian has highest weight"""
        assert reputation_agent.reputation_weights[ReputationType.WORLD_GUARDIAN] == 0.30
    
    def test_explorer_weight(self, reputation_agent):
        """Test explorer weight"""
        assert reputation_agent.reputation_weights[ReputationType.EXPLORER] == 0.25
    
    def test_faction_loyalist_weight(self, reputation_agent):
        """Test faction loyalist has lowest weight"""
        assert reputation_agent.reputation_weights[ReputationType.FACTION_LOYALIST] == 0.10