"""
Unit tests for Adaptive Dungeon Agent
Tests procedural dungeon generation and instance management
"""

import pytest
import json
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from agents.advanced.adaptive_dungeon_agent import (
    AdaptiveDungeonAgent,
    DungeonTheme
)
from agents.base_agent import AgentResponse


@pytest.fixture
def dungeon_agent():
    """Create dungeon agent instance for testing"""
    return AdaptiveDungeonAgent(config={})


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


class TestAdaptiveDungeonAgent:
    """Test suite for Adaptive Dungeon Agent"""
    
    @pytest.mark.asyncio
    async def test_agent_initialization(self, dungeon_agent):
        """Test agent initializes correctly"""
        assert dungeon_agent.agent_type == "adaptive_dungeon"
        assert dungeon_agent.agent_id == "adaptive_dungeon_agent"
        assert len(dungeon_agent.monster_pools) == 7
        assert len(dungeon_agent.boss_pools) == 7
        assert dungeon_agent.floor_config['min_floors'] == 5
        assert dungeon_agent.floor_config['max_floors'] == 10
    
    @pytest.mark.asyncio
    async def test_generate_daily_dungeon(self, dungeon_agent, mock_postgres, mock_redis):
        """Test daily dungeon generation"""
        # Mock database response
        mock_postgres.fetch_one.return_value = {'dungeon_id': 123}
        
        response = await dungeon_agent.generate_daily_dungeon(
            player_average_level=50,
            active_factions=["rune_alliance"]
        )
        
        assert response.success
        assert response.agent_type == "adaptive_dungeon"
        assert 'dungeon_id' in response.data
        assert 'theme' in response.data
        assert 'floor_count' in response.data
        assert response.data['floor_count'] >= 5
        assert response.data['floor_count'] <= 10
        assert response.confidence >= 0.9
    
    @pytest.mark.asyncio
    async def test_theme_selection(self, dungeon_agent):
        """Test theme selection logic"""
        # Test with faction influence
        theme = dungeon_agent._select_daily_theme(["rune_alliance"])
        assert isinstance(theme, DungeonTheme)
        
        # Test without faction influence
        theme = dungeon_agent._select_daily_theme([])
        assert isinstance(theme, DungeonTheme)
    
    @pytest.mark.asyncio
    async def test_floor_layout_generation(self, dungeon_agent):
        """Test floor layout generation"""
        layout = await dungeon_agent.generate_floor_layout(
            floor_number=1,
            theme=DungeonTheme.FIRE_DAY
        )
        
        assert layout['floor_number'] == 1
        assert 3 <= layout['room_count'] <= 7
        assert len(layout['rooms']) == layout['room_count']
        
        # Check room structure
        for room in layout['rooms']:
            assert 'room_id' in room
            assert 'room_type' in room
            assert 'connections' in room
    
    @pytest.mark.asyncio
    async def test_monster_pool_selection(self, dungeon_agent):
        """Test monster pool selection"""
        pool = await dungeon_agent.select_monster_pool(
            floor_number=1,
            theme=DungeonTheme.FIRE_DAY,
            player_avg_level=50
        )
        
        assert isinstance(pool, list)
        assert len(pool) > 0
        
        for entry in pool:
            assert 'monster_id' in entry
            assert 'spawn_count' in entry
            assert 'level_modifier' in entry
            assert entry['spawn_count'] > 0
            assert entry['level_modifier'] >= 1.0
    
    @pytest.mark.asyncio
    async def test_chaotic_day_uses_all_monsters(self, dungeon_agent):
        """Test chaotic day selects from all monster pools"""
        pool = await dungeon_agent.select_monster_pool(
            floor_number=5,
            theme=DungeonTheme.CHAOTIC_DAY,
            player_avg_level=75
        )
        
        assert isinstance(pool, list)
        assert len(pool) > 0
    
    @pytest.mark.asyncio
    async def test_boss_pool_selection(self, dungeon_agent):
        """Test boss pool selection"""
        boss_pool = dungeon_agent._select_boss_pool(
            theme=DungeonTheme.UNDEAD_DAY,
            floor_count=7
        )
        
        assert isinstance(boss_pool, list)
        assert 1 <= len(boss_pool) <= 3
        assert all(isinstance(boss, str) for boss in boss_pool)
    
    @pytest.mark.asyncio
    async def test_reward_calculation(self, dungeon_agent):
        """Test dungeon reward calculation"""
        rewards = await dungeon_agent.calculate_dungeon_rewards(
            theme=DungeonTheme.FIRE_DAY,
            floors_completed=7,
            time_taken=1800,  # 30 minutes
            party_size=4
        )
        
        assert 'dungeon_tokens' in rewards
        assert 'theme_drops' in rewards
        assert 'reputation_gain' in rewards
        assert 'bonus_multiplier' in rewards
        assert rewards['dungeon_tokens'] > 0
        assert rewards['reputation_gain'] > 0
        
        # Faster clear should give better bonus
        assert rewards['bonus_multiplier'] >= 1.0
    
    @pytest.mark.asyncio
    async def test_time_bonus_scaling(self, dungeon_agent):
        """Test time bonus scaling"""
        # Fast clear (under 50% time)
        fast_rewards = await dungeon_agent.calculate_dungeon_rewards(
            theme=DungeonTheme.FIRE_DAY,
            floors_completed=5,
            time_taken=1200,  # 20 minutes (< 50% of 60 min)
            party_size=4
        )
        
        # Slow clear (over 75% time)
        slow_rewards = await dungeon_agent.calculate_dungeon_rewards(
            theme=DungeonTheme.FIRE_DAY,
            floors_completed=5,
            time_taken=3000,  # 50 minutes (> 75% of 60 min)
            party_size=4
        )
        
        assert fast_rewards['bonus_multiplier'] > slow_rewards['bonus_multiplier']
    
    @pytest.mark.asyncio
    async def test_handle_instance_completion(self, dungeon_agent, mock_postgres, mock_redis):
        """Test instance completion handling"""
        # Mock instance data
        mock_postgres.fetch_one.side_effect = [
            {'dungeon_id': 123, 'party_id': 1},  # Instance query
            {'theme': 'fire_day'}  # Dungeon query
        ]
        
        response = await dungeon_agent.handle_instance_completion(
            instance_id=1,
            participants=[101, 102, 103],
            floors_cleared=7,
            time_taken=2400
        )
        
        assert response.success
        assert 'instance_id' in response.data
        assert 'rewards' in response.data
        assert 'participants' in response.data
        assert len(response.data['participants']) == 3
    
    @pytest.mark.asyncio
    async def test_room_type_selection(self, dungeon_agent):
        """Test room type selection logic"""
        # First room should be combat
        room_type = dungeon_agent._select_room_type(1, 0, 5)
        assert room_type == "combat_room"
        
        # Boss room on last floor
        room_type = dungeon_agent._select_room_type(10, 4, 5)
        assert room_type == "boss_room"
        
        # Checkpoint on floor 3
        room_type = dungeon_agent._select_room_type(3, 2, 5)
        assert room_type == "checkpoint_room"
    
    @pytest.mark.asyncio
    async def test_difficulty_scaling(self, dungeon_agent):
        """Test difficulty scales with floor number"""
        # Floor 1
        pool_1 = await dungeon_agent.select_monster_pool(1, DungeonTheme.FIRE_DAY, 50)
        
        # Floor 10
        pool_10 = await dungeon_agent.select_monster_pool(10, DungeonTheme.FIRE_DAY, 50)
        
        # Higher floor should have higher level modifier
        avg_mod_1 = sum(m['level_modifier'] for m in pool_1) / len(pool_1)
        avg_mod_10 = sum(m['level_modifier'] for m in pool_10) / len(pool_10)
        
        assert avg_mod_10 > avg_mod_1
    
    @pytest.mark.asyncio
    async def test_theme_modifiers(self, dungeon_agent):
        """Test theme modifiers are properly defined"""
        for theme in DungeonTheme:
            assert theme in dungeon_agent.theme_modifiers
            modifier = dungeon_agent.theme_modifiers[theme]
            assert 'element' in modifier
            assert 'stat_bonus' in modifier
            assert 'drop_bonus' in modifier
    
    @pytest.mark.asyncio
    async def test_dungeon_storage(self, dungeon_agent, mock_postgres):
        """Test dungeon storage in database"""
        mock_postgres.fetch_one.return_value = {'dungeon_id': 456}
        
        dungeon_id = await dungeon_agent._store_dungeon(
            theme=DungeonTheme.ICE_DAY,
            floor_count=7,
            layouts=[{"floor": 1}],
            monster_pools=[[{"monster": "ICE_TITAN"}]],
            boss_pool=["KTULLANUX"],
            reward_data={"tokens": 700},
            difficulty_rating=7
        )
        
        assert dungeon_id == 456
        assert mock_postgres.fetch_one.called
    
    @pytest.mark.asyncio
    async def test_error_handling(self, dungeon_agent, mock_postgres):
        """Test error handling in dungeon generation"""
        # Mock database error
        mock_postgres.fetch_one.side_effect = Exception("Database error")
        
        response = await dungeon_agent.generate_daily_dungeon(
            player_average_level=50,
            active_factions=[]
        )
        
        assert not response.success
        assert 'error' in response.data
        assert response.confidence == 0.0


@pytest.mark.asyncio
async def test_concurrent_dungeon_generation(dungeon_agent, mock_postgres, mock_redis):
    """Test concurrent dungeon generation"""
    mock_postgres.fetch_one.return_value = {'dungeon_id': 789}
    
    # Generate multiple dungeons concurrently
    import asyncio
    tasks = [
        dungeon_agent.generate_daily_dungeon(
            player_average_level=50 + i * 10,
            active_factions=[]
        )
        for i in range(3)
    ]
    
    responses = await asyncio.gather(*tasks)
    
    # All should succeed
    assert all(r.success for r in responses)


@pytest.mark.asyncio
async def test_reward_distribution(dungeon_agent):
    """Test reward distribution is fair across party sizes"""
    # Small party
    rewards_small = await dungeon_agent.calculate_dungeon_rewards(
        theme=DungeonTheme.FIRE_DAY,
        floors_completed=5,
        time_taken=2000,
        party_size=2
    )
    
    # Large party
    rewards_large = await dungeon_agent.calculate_dungeon_rewards(
        theme=DungeonTheme.FIRE_DAY,
        floors_completed=5,
        time_taken=2000,
        party_size=12
    )
    
    # Per-person rewards should be higher for smaller parties
    assert rewards_small['dungeon_tokens'] > rewards_large['dungeon_tokens']