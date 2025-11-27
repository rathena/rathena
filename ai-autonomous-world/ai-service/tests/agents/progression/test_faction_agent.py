"""
Unit tests for Faction Agent
Tests faction alignment updates, player reputation, and faction effects
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch, MagicMock
from datetime import datetime, UTC

from agents.progression.faction_agent import FactionAgent, FactionType
from agents.base_agent import AgentResponse


@pytest.fixture
def faction_agent():
    """Create faction agent instance"""
    return FactionAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.progression.faction_agent.postgres_db') as mock_pg, \
         patch('agents.progression.faction_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestFactionAgentInitialization:
    """Test faction agent initialization"""
    
    def test_initialization(self, faction_agent):
        """Test faction agent initializes correctly"""
        assert faction_agent.agent_type == "faction"
        assert faction_agent.agent_id == "faction_agent"
        assert len(faction_agent.factions) == 3
        assert FactionType.RUNE_ALLIANCE in faction_agent.factions
        assert FactionType.SHADOW_CULT in faction_agent.factions
        assert FactionType.NATURE_SPIRITS in faction_agent.factions
    
    def test_alignment_thresholds(self, faction_agent):
        """Test alignment thresholds are configured"""
        assert "minor" in faction_agent.alignment_thresholds
        assert "moderate" in faction_agent.alignment_thresholds
        assert "major" in faction_agent.alignment_thresholds
        assert faction_agent.alignment_thresholds["minor"] == 1000
    
    def test_reputation_tiers(self, faction_agent):
        """Test reputation tier configuration"""
        assert len(faction_agent.reputation_tiers) == 6
        assert faction_agent.reputation_tiers[0]["threshold"] == 0
        assert faction_agent.reputation_tiers[5]["threshold"] == 10000


class TestUpdateWorldAlignment:
    """Test world faction alignment updates"""
    
    @pytest.mark.asyncio
    async def test_update_alignment_success(self, faction_agent, mock_db):
        """Test successful alignment update"""
        # Setup
        player_actions = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_change": 100},
            {"faction_id": FactionType.SHADOW_CULT, "alignment_change": -50}
        ]
        
        mock_db['postgres'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 500},
            {"faction_id": FactionType.SHADOW_CULT, "alignment_score": 200},
            {"faction_id": FactionType.NATURE_SPIRITS, "alignment_score": 0}
        ]
        
        mock_db['postgres'].fetch_one.return_value = {"alignment_score": 600}
        
        # Execute
        response = await faction_agent.update_world_alignment(player_actions)
        
        # Verify
        assert response.success is True
        assert response.agent_type == "faction"
        assert "alignment_changes" in response.data
        assert "updated_alignments" in response.data
    
    @pytest.mark.asyncio
    async def test_alignment_decay(self, faction_agent, mock_db):
        """Test alignment decay prevents permanent dominance"""
        # Setup high alignment
        mock_db['postgres'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 5000},
            {"faction_id": FactionType.SHADOW_CULT, "alignment_score": 0},
            {"faction_id": FactionType.NATURE_SPIRITS, "alignment_score": 0}
        ]
        
        player_actions = []
        
        # Execute
        response = await faction_agent.update_world_alignment(player_actions)
        
        # Verify decay applied
        assert response.success is True
    
    @pytest.mark.asyncio
    async def test_threshold_crossing(self, faction_agent, mock_db):
        """Test faction effects trigger when threshold crossed"""
        # Setup action that crosses threshold
        player_actions = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_change": 1500}
        ]
        
        mock_db['postgres'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 100}
        ]
        
        mock_db['postgres'].fetch_one.return_value = {"alignment_score": 1600}
        
        # Execute
        response = await faction_agent.update_world_alignment(player_actions)
        
        # Verify
        assert response.success is True
        assert len(response.data.get("threshold_crossed", [])) > 0


class TestCalculatePlayerReputation:
    """Test player reputation calculation"""
    
    @pytest.mark.asyncio
    async def test_calculate_reputation_cached(self, faction_agent, mock_db):
        """Test reputation calculation uses cache"""
        # Setup cache hit
        mock_db['cache'].get.return_value = 5000
        
        # Execute
        reputation = await faction_agent.calculate_player_reputation(
            player_id=123,
            faction_id=FactionType.RUNE_ALLIANCE
        )
        
        # Verify
        assert reputation == 5000
        mock_db['cache'].get.assert_called_once()
        mock_db['postgres'].fetch_one.assert_not_called()
    
    @pytest.mark.asyncio
    async def test_calculate_reputation_from_db(self, faction_agent, mock_db):
        """Test reputation calculation from database"""
        # Setup cache miss
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {"reputation": 3000}
        
        # Execute
        reputation = await faction_agent.calculate_player_reputation(
            player_id=123,
            faction_id=FactionType.SHADOW_CULT
        )
        
        # Verify
        assert reputation == 3000
        mock_db['postgres'].fetch_one.assert_called_once()
        mock_db['cache'].set.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_calculate_reputation_new_player(self, faction_agent, mock_db):
        """Test reputation for new player defaults to 0"""
        # Setup no record
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = None
        
        # Execute
        reputation = await faction_agent.calculate_player_reputation(
            player_id=999,
            faction_id=FactionType.NATURE_SPIRITS
        )
        
        # Verify
        assert reputation == 0


class TestTriggerFactionEffects:
    """Test faction effect triggering"""
    
    @pytest.mark.asyncio
    async def test_trigger_effects(self, faction_agent, mock_db):
        """Test faction effects are triggered"""
        # Setup
        mock_db['postgres'].execute.return_value = 1
        
        # Execute
        effects = await faction_agent.trigger_faction_effects(
            FactionType.RUNE_ALLIANCE
        )
        
        # Verify
        assert "faction" in effects
        assert effects["faction"] == FactionType.RUNE_ALLIANCE
        assert "visual_changes" in effects
        assert "quests_unlocked" in effects
        assert mock_db['postgres'].execute.call_count >= 2


class TestGetFactionRewards:
    """Test faction reward retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_rewards_tier_1(self, faction_agent, mock_db):
        """Test tier 1 rewards"""
        rewards = await faction_agent.get_faction_rewards(
            player_id=123,
            faction_id=FactionType.RUNE_ALLIANCE,
            reputation_tier=1
        )
        
        assert rewards["tier"] == 1
        assert len(rewards["cosmetics"]) > 0
    
    @pytest.mark.asyncio
    async def test_get_rewards_tier_5(self, faction_agent, mock_db):
        """Test tier 5 (max) rewards"""
        rewards = await faction_agent.get_faction_rewards(
            player_id=123,
            faction_id=FactionType.SHADOW_CULT,
            reputation_tier=5
        )
        
        assert rewards["tier"] == 5
        assert len(rewards["cosmetics"]) > 0
        assert len(rewards["buffs"]) > 0
        assert len(rewards["items"]) > 0
        assert len(rewards["skills"]) > 0
        assert len(rewards["titles"]) > 0


class TestRecordFactionAction:
    """Test faction action recording"""
    
    @pytest.mark.asyncio
    async def test_record_action_success(self, faction_agent, mock_db):
        """Test successful action recording"""
        # Setup
        mock_db['postgres'].execute.return_value = 1
        mock_db['postgres'].fetch_one.return_value = {"reputation": 1100, "reputation_tier": 1}
        
        # Execute
        success = await faction_agent.record_faction_action(
            player_id=123,
            faction_id=FactionType.RUNE_ALLIANCE,
            action_type="quest_complete",
            action_data={"quest_id": 456}
        )
        
        # Verify
        assert success is True
        assert mock_db['postgres'].execute.call_count >= 1
        mock_db['cache'].delete.assert_called()
    
    @pytest.mark.asyncio
    async def test_action_reputation_values(self, faction_agent, mock_db):
        """Test different action types have correct reputation values"""
        assert faction_agent.action_reputation_values["quest_complete"] == 100
        assert faction_agent.action_reputation_values["monster_kill"] == 10
        assert faction_agent.action_reputation_values["pvp_attack"] < 0


class TestGetWorldAlignment:
    """Test world alignment retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_alignment_from_cache(self, faction_agent, mock_db):
        """Test alignment retrieved from cache"""
        # Setup cache hit
        cached_data = {
            "factions": [{"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 1000}],
            "dominant_faction": FactionType.RUNE_ALLIANCE
        }
        mock_db['cache'].get.return_value = cached_data
        
        # Execute
        alignment = await faction_agent.get_world_alignment()
        
        # Verify
        assert alignment == cached_data
        mock_db['postgres'].fetch_all.assert_not_called()
    
    @pytest.mark.asyncio
    async def test_get_alignment_from_db(self, faction_agent, mock_db):
        """Test alignment retrieved from database"""
        # Setup cache miss
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "faction_name": "Rune Alliance", "alignment_score": 2000, "is_dominant": True},
            {"faction_id": FactionType.SHADOW_CULT, "faction_name": "Shadow Cult", "alignment_score": 500, "is_dominant": False}
        ]
        
        # Execute
        alignment = await faction_agent.get_world_alignment()
        
        # Verify
        assert "factions" in alignment
        assert len(alignment["factions"]) == 2
        assert alignment["dominant_faction"] == FactionType.RUNE_ALLIANCE
        mock_db['cache'].set.assert_called_once()


class TestReputationTierCalculation:
    """Test reputation tier calculation"""
    
    def test_tier_calculation(self, faction_agent):
        """Test tier calculated correctly from reputation"""
        assert faction_agent._calculate_reputation_tier(0) == 0
        assert faction_agent._calculate_reputation_tier(500) == 0
        assert faction_agent._calculate_reputation_tier(1000) == 1
        assert faction_agent._calculate_reputation_tier(3500) == 2
        assert faction_agent._calculate_reputation_tier(5000) == 3
        assert faction_agent._calculate_reputation_tier(7500) == 4
        assert faction_agent._calculate_reputation_tier(10000) == 5
        assert faction_agent._calculate_reputation_tier(15000) == 5  # Cap at 5