"""
Unit tests for Dynamic NPC Agent
Tests NPC spawning, personality generation, and interaction handling
"""

import pytest
from datetime import datetime, UTC
from unittest.mock import AsyncMock, patch

from agents.procedural.dynamic_npc_agent import DynamicNPCAgent, dynamic_npc_agent
from models.procedural import NPCType, NPCStatus


@pytest.fixture
def sample_map_activity():
    """Sample map activity for testing"""
    return {
        "prontera": 100,
        "geffen": 50,
        "payon": 20,
        "morocc": 10,
        "cmd_fild01": 5,
        "mjo_dun01": 2,
        "anthell01": 1
    }


@pytest.fixture
def sample_heatmap():
    """Sample heatmap data for testing"""
    return {
        "cmd_fild01": [
            {"x": 100, "y": 100, "count": 5},
            {"x": 200, "y": 200, "count": 2},
            {"x": 50, "y": 300, "count": 1}
        ],
        "mjo_dun01": [
            {"x": 150, "y": 150, "count": 3},
            {"x": 100, "y": 50, "count": 1}
        ]
    }


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.procedural.dynamic_npc_agent.postgres_db') as mock_pg, \
         patch('agents.procedural.dynamic_npc_agent.db') as mock_cache:
        
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestDynamicNPCAgent:
    """Test suite for Dynamic NPC Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = DynamicNPCAgent()
        
        assert agent.agent_type == "dynamic_npc"
        assert len(agent.npc_templates) == 5
        assert len(agent.reward_tiers) == 3
    
    def test_identify_low_traffic_maps(self, sample_map_activity):
        """Test low-traffic map identification"""
        agent = DynamicNPCAgent()
        
        low_traffic = agent._identify_low_traffic_maps(sample_map_activity)
        
        # Should return maps with <10% of peak traffic
        assert len(low_traffic) > 0
        
        # All returned maps should have low traffic
        peak = max(sample_map_activity.values())
        threshold = peak * 0.1
        
        for map_name, count in low_traffic:
            assert count <= threshold
    
    def test_identify_low_traffic_maps_empty(self):
        """Test low-traffic identification with empty data"""
        agent = DynamicNPCAgent()
        
        low_traffic = agent._identify_low_traffic_maps({})
        
        # Should return default maps
        assert len(low_traffic) > 0
        assert all(count == 0 for _, count in low_traffic)
    
    def test_select_npc_type(self):
        """Test NPC type selection"""
        agent = DynamicNPCAgent()
        
        npc_type = agent._select_npc_type()
        
        assert npc_type in agent.npc_templates
    
    def test_select_spawn_map(self, sample_map_activity):
        """Test spawn map selection"""
        agent = DynamicNPCAgent()
        
        low_traffic = [
            ("cmd_fild01", 5),
            ("mjo_dun01", 2),
            ("anthell01", 1)
        ]
        
        already_spawned = set()
        
        spawn_map = agent._select_spawn_map(
            low_traffic,
            already_spawned,
            heatmap_data=None
        )
        
        assert spawn_map in [m for m, _ in low_traffic]
    
    def test_select_spawn_map_avoids_used(self):
        """Test spawn map selection avoids already used maps"""
        agent = DynamicNPCAgent()
        
        low_traffic = [
            ("cmd_fild01", 5),
            ("mjo_dun01", 2)
        ]
        
        already_spawned = {"cmd_fild01"}
        
        spawn_map = agent._select_spawn_map(
            low_traffic,
            already_spawned,
            heatmap_data=None
        )
        
        assert spawn_map == "mjo_dun01"
    
    def test_calculate_map_interest_empty(self):
        """Test map interest calculation with empty heatmap"""
        agent = DynamicNPCAgent()
        
        interest = agent._calculate_map_interest([])
        
        assert 0.0 <= interest <= 1.0
    
    def test_calculate_map_interest_high_diversity(self, sample_heatmap):
        """Test map interest calculation with diverse positions"""
        agent = DynamicNPCAgent()
        
        interest = agent._calculate_map_interest(sample_heatmap["cmd_fild01"])
        
        assert 0.0 <= interest <= 1.0
    
    def test_generate_spawn_coords_with_heatmap(self, sample_heatmap):
        """Test spawn coordinate generation with heatmap"""
        agent = DynamicNPCAgent()
        
        x, y = agent._generate_spawn_coords("cmd_fild01", sample_heatmap)
        
        # Should return valid coordinates
        assert x >= 0
        assert y >= 0
    
    def test_generate_spawn_coords_without_heatmap(self):
        """Test spawn coordinate generation without heatmap"""
        agent = DynamicNPCAgent()
        
        x, y = agent._generate_spawn_coords("prontera", None)
        
        # Should return random valid coordinates
        assert 50 <= x <= 300
        assert 50 <= y <= 300
    
    @pytest.mark.asyncio
    async def test_generate_npc_personality(self):
        """Test NPC personality generation"""
        agent = DynamicNPCAgent()
        
        personality = await agent.generate_npc_personality(NPCType.TREASURE_SEEKER)
        
        assert "name" in personality
        assert "personality_traits" in personality
        assert "backstory" in personality
        assert "speech_patterns" in personality
        assert "quirks" in personality
    
    def test_generate_npc_name(self):
        """Test NPC name generation"""
        agent = DynamicNPCAgent()
        
        name = agent._generate_npc_name(NPCType.WANDERING_BLACKSMITH)
        
        assert len(name) > 0
        assert isinstance(name, str)
    
    def test_generate_quest_data(self):
        """Test quest data generation"""
        agent = DynamicNPCAgent()
        
        template = agent.npc_templates[NPCType.TREASURE_SEEKER]
        quest_data = agent._generate_quest_data(NPCType.TREASURE_SEEKER, template)
        
        assert "objectives" in quest_data
        assert "hints" in quest_data
        assert len(quest_data["objectives"]) > 0
    
    @pytest.mark.asyncio
    async def test_scale_rewards_tier_variation(self):
        """Test reward scaling across different tiers"""
        agent = DynamicNPCAgent()
        
        # Low tier NPC
        rewards_low = await agent.scale_rewards(NPCType.LOST_MERCHANT, 50)
        
        # High tier NPC
        rewards_high = await agent.scale_rewards(NPCType.MYSTERIOUS_CHILD, 50)
        
        # High tier should give better rewards
        assert rewards_high['exp'] > rewards_low['exp']
        assert rewards_high['zeny'] > rewards_low['zeny']
    
    @pytest.mark.asyncio
    async def test_scale_rewards_level_scaling(self):
        """Test rewards scale with player level"""
        agent = DynamicNPCAgent()
        
        rewards_level_30 = await agent.scale_rewards(NPCType.RUNAWAY_SAGE, 30)
        rewards_level_80 = await agent.scale_rewards(NPCType.RUNAWAY_SAGE, 80)
        
        # Higher level = better rewards
        assert rewards_level_80['exp'] > rewards_level_30['exp']
        assert rewards_level_80['zeny'] > rewards_level_30['zeny']
    
    @pytest.mark.asyncio
    async def test_spawn_daily_npcs(self, sample_map_activity, mock_db):
        """Test daily NPC spawning"""
        agent = DynamicNPCAgent()
        
        # Mock empty active NPCs
        mock_db['postgres'].fetch_all.return_value = []
        mock_db['postgres'].fetch_one.return_value = {"npc_id": 1}
        
        responses = await agent.spawn_daily_npcs(
            map_activity=sample_map_activity,
            count=3
        )
        
        # Should spawn requested count (or less if not enough maps)
        assert len(responses) <= 3
        
        # All responses should be successful or have valid reasons
        for response in responses:
            assert hasattr(response, 'success')
    
    @pytest.mark.asyncio
    async def test_spawn_daily_npcs_max_limit(self, sample_map_activity, mock_db):
        """Test NPC spawning respects max limit"""
        agent = DynamicNPCAgent()
        
        # Mock 5 active NPCs (max limit)
        mock_db['postgres'].fetch_all.return_value = [
            {"npc_id": i, "status": "active"} 
            for i in range(5)
        ]
        
        responses = await agent.spawn_daily_npcs(
            map_activity=sample_map_activity,
            count=3
        )
        
        # Should not spawn any more NPCs
        assert len(responses) == 0
    
    @pytest.mark.asyncio
    async def test_despawn_expired_npcs(self, mock_db):
        """Test NPC despawning"""
        agent = DynamicNPCAgent()
        
        # Mock despawn returning 2 NPCs
        mock_db['postgres'].fetch_all.return_value = [
            {"npc_id": 1},
            {"npc_id": 2}
        ]
        
        count = await agent.despawn_expired_npcs()
        
        assert count == 2
        assert mock_db['cache'].delete.called


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert dynamic_npc_agent is not None
    assert dynamic_npc_agent.agent_type == "dynamic_npc"