"""
Integration Tests for Environmental Agents
Tests cross-agent coordination and full workflows
"""

import pytest
from unittest.mock import AsyncMock, patch
from datetime import datetime, UTC, timedelta

from agents.environmental.map_hazard_agent import map_hazard_agent, HazardType
from agents.environmental.weather_time_agent import weather_time_agent, WeatherType
from agents.environmental.treasure_agent import treasure_agent, TreasureRarity


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.environmental.map_hazard_agent.postgres_db') as mock_pg_hazard, \
         patch('agents.environmental.map_hazard_agent.db') as mock_cache_hazard, \
         patch('agents.environmental.weather_time_agent.postgres_db') as mock_pg_weather, \
         patch('agents.environmental.weather_time_agent.db') as mock_cache_weather, \
         patch('agents.environmental.treasure_agent.postgres_db') as mock_pg_treasure, \
         patch('agents.environmental.treasure_agent.db') as mock_cache_treasure:
        
        # Setup common mocks
        for mock_pg in [mock_pg_hazard, mock_pg_weather, mock_pg_treasure]:
            mock_pg.fetch_one = AsyncMock()
            mock_pg.fetch_all = AsyncMock()
            mock_pg.execute = AsyncMock()
        
        for mock_cache in [mock_cache_hazard, mock_cache_weather, mock_cache_treasure]:
            mock_cache.get = AsyncMock(return_value=None)
            mock_cache.set = AsyncMock()
            mock_cache.delete = AsyncMock()
        
        yield {
            'hazard_pg': mock_pg_hazard,
            'hazard_cache': mock_cache_hazard,
            'weather_pg': mock_pg_weather,
            'weather_cache': mock_cache_weather,
            'treasure_pg': mock_pg_treasure,
            'treasure_cache': mock_cache_treasure
        }


class TestFullEnvironmentalWorkflow:
    """Test complete environmental system workflow"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_daily_cycle_execution(self, mock_db):
        """Test full daily cycle: hazards -> weather -> treasures"""
        # Setup
        map_activity = {'prt_fild01': 5, 'moc_fild01': 3, 'pay_fild01': 8}
        
        mock_db['hazard_pg'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        
        mock_db['weather_pg'].fetch_one.return_value = {
            'current_weather': 'clear',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        mock_db['weather_pg'].execute.return_value = 1
        
        mock_db['treasure_pg'].fetch_one.return_value = {
            'treasure_id': 1,
            'spawned_at': datetime.now(UTC)
        }
        
        # Step 1: Generate hazards
        hazard_response = await map_hazard_agent.generate_daily_hazards(
            map_activity=map_activity,
            active_problems=[],
            count=2
        )
        assert hazard_response.success is True
        
        # Step 2: Update weather
        weather_response = await weather_time_agent.update_weather()
        assert weather_response.success is True
        
        # Step 3: Spawn treasures
        treasure_response = await treasure_agent.spawn_daily_treasures(
            map_activity=map_activity,
            count=5
        )
        assert treasure_response.success is True


class TestHazardWeatherCoordination:
    """Test hazard and weather coordination"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_hazard_weather_synergy(self, mock_db):
        """Test hazards and weather create synergies"""
        # Setup storm weather
        mock_db['weather_pg'].execute.return_value = 1
        mock_db['weather_pg'].fetch_one.return_value = None
        
        weather_response = await weather_time_agent.update_weather(
            force_weather=WeatherType.STORM
        )
        
        # Apply magic storm hazard (complementary)
        mock_db['hazard_pg'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        
        hazard_response = await map_hazard_agent.apply_hazard(
            map_name='test_map',
            hazard_type=HazardType.MAGIC_STORM,
            duration_hours=24
        )
        
        assert weather_response.success is True
        assert hazard_response.success is True


class TestTreasureReputationIntegration:
    """Test treasure discovery awards reputation"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_treasure_awards_reputation(self, mock_db):
        """Test discovering treasure awards explorer reputation"""
        # Setup
        mock_db['treasure_pg'].fetch_one.side_effect = [
            {  # Treasure query
                'treasure_id': 1,
                'treasure_type': 'chest',
                'rarity': 'gold',
                'spawn_map': 'test_map',
                'contents': '{"items": [], "zeny": 50000, "reputation_reward": 50, "card_fragments": 5}',
                'status': 'active'
            }
        ]
        mock_db['treasure_pg'].execute.return_value = 1
        
        # Execute
        response = await treasure_agent.discover_treasure(
            treasure_id=1,
            player_id=123
        )
        
        # Verify
        assert response.success is True
        assert response.data['rewards']['reputation_reward'] == 50


class TestEnvironmentalCacheCoherence:
    """Test cache coherence across environmental agents"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_hazard_cache_invalidation(self, mock_db):
        """Test hazard removal invalidates cache"""
        mock_db['hazard_pg'].fetch_all.return_value = [
            {'hazard_id': 1, 'map_name': 'test_map'}
        ]
        
        count = await map_hazard_agent.remove_expired_hazards()
        
        assert count == 1
        mock_db['hazard_cache'].delete.assert_called()
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_weather_update_cache_refresh(self, mock_db):
        """Test weather update refreshes cache"""
        mock_db['weather_pg'].execute.return_value = 1
        mock_db['weather_pg'].fetch_one.return_value = None
        
        response = await weather_time_agent.update_weather()
        
        assert response.success is True
        mock_db['weather_cache'].set.assert_called()


class TestEnvironmentalDatabaseIntegrity:
    """Test database operations maintain integrity"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_treasure_discovery_updates_all_tables(self, mock_db):
        """Test treasure discovery updates treasure_spawns and treasure_discoveries"""
        mock_db['treasure_pg'].fetch_one.return_value = {
            'treasure_id': 1,
            'treasure_type': 'chest',
            'rarity': 'silver',
            'spawn_map': 'test_map',
            'contents': '{"items": [], "zeny": 10000, "reputation_reward": 25, "card_fragments": 1}',
            'status': 'active'
        }
        mock_db['treasure_pg'].execute.return_value = 1
        
        response = await treasure_agent.discover_treasure(
            treasure_id=1,
            player_id=123
        )
        
        assert response.success is True
        # Verify both UPDATE (treasure_spawns) and INSERT (treasure_discoveries) called
        assert mock_db['treasure_pg'].execute.call_count >= 2


class TestSchedulerIntegration:
    """Test scheduler can execute environmental tasks"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_scheduler_task_execution(self, mock_db):
        """Test scheduler wrapper methods execute correctly"""
        from tasks.procedural_scheduler import procedural_scheduler
        
        # Setup mocks for all database calls
        for key in mock_db:
            if 'pg' in key:
                mock_db[key].fetch_one.return_value = {'treasure_id': 1, 'spawned_at': datetime.now(UTC)}
                mock_db[key].fetch_all.return_value = []
                mock_db[key].execute.return_value = 1
        
        # Test hazard generation wrapper
        await procedural_scheduler._generate_daily_hazards_wrapper()
        
        # Test weather update wrapper
        await procedural_scheduler._update_weather_wrapper()
        
        # Test treasure spawn wrapper
        await procedural_scheduler._spawn_daily_treasures_wrapper()


class TestCrossAgentCoordination:
    """Test environmental agents coordinate with other agent types"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_hazard_problem_coordination(self, mock_db):
        """Test hazards coordinate with active problems"""
        map_activity = {'prt_fild01': 5}
        active_problems = [
            {'problem_id': 1, 'map_name': 'prt_fild01', 'problem_type': 'monster_surge'}
        ]
        
        mock_db['hazard_pg'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        
        response = await map_hazard_agent.generate_daily_hazards(
            map_activity=map_activity,
            active_problems=active_problems,
            count=1
        )
        
        assert response.success is True
        # Verify problem coordination logic executed


class TestAPIEndpointIntegration:
    """Test API endpoints work with agents"""
    
    @pytest.mark.asyncio
    @pytest.mark.integration
    async def test_environmental_status_endpoint(self, mock_db):
        """Test status endpoint aggregates data from all agents"""
        # Setup
        mock_db['hazard_pg'].fetch_all.return_value = [{'hazard_id': 1}]
        mock_db['weather_pg'].fetch_one.return_value = {
            'current_weather': 'clear',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        mock_db['treasure_pg'].fetch_all.return_value = [
            {'treasure_id': 1}, {'treasure_id': 2}
        ]
        
        # Get data from each agent
        hazards = await map_hazard_agent.get_active_hazards()
        weather = await weather_time_agent.get_current_weather()
        treasures = await treasure_agent.get_active_treasures()
        
        # Verify
        assert len(hazards) == 1
        assert weather is not None
        assert len(treasures) == 2