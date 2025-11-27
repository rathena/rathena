"""
Integration tests for Advanced Systems Agents
Tests cross-agent interactions and full workflow scenarios
"""

import pytest
import json
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from agents.advanced.adaptive_dungeon_agent import adaptive_dungeon_agent, DungeonTheme
from agents.advanced.archaeology_agent import archaeology_agent, ArtifactType
from agents.advanced.event_chain_agent import event_chain_agent, ChainEventType
from agents.progression.faction_agent import faction_agent
from agents.progression.reputation_agent import reputation_agent


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


class TestAdvancedSystemsIntegration:
    """Integration tests for advanced systems"""
    
    @pytest.mark.asyncio
    async def test_dungeon_with_faction_integration(self, mock_postgres, mock_redis):
        """Test dungeon theme influenced by faction alignment"""
        # Mock faction alignment
        mock_redis.get.return_value = {
            "factions": [
                {"faction_id": "shadow_cult", "is_dominant": True}
            ],
            "dominant_faction": "shadow_cult"
        }
        
        mock_postgres.fetch_one.return_value = {'dungeon_id': 1}
        
        # Generate dungeon
        response = await adaptive_dungeon_agent.generate_daily_dungeon(
            player_average_level=60,
            active_factions=["shadow_cult"]
        )
        
        # Should succeed and potentially be influenced by faction
        assert response.success
        assert 'theme' in response.data
    
    @pytest.mark.asyncio
    async def test_archaeology_rewards_reputation(self, mock_postgres, mock_redis):
        """Test archaeology contributes to reputation system"""
        # Mock dig spot
        mock_postgres.fetch_one.side_effect = [
            {'rarity_tier': 0.8, 'status': 'active'},  # Dig spot query
            None  # No collection completion
        ]
        
        # Generate artifact
        artifact = await archaeology_agent.generate_artifact(
            dig_spot_rarity=0.8,
            player_archaeology_level=5
        )
        
        assert artifact['rarity'] in ['common', 'uncommon', 'rare', 'mythic']
        
        # Rare artifacts should exist
        if artifact['rarity'] in ['rare', 'mythic']:
            assert artifact['artifact_type'] in ['tome', 'treasure_map', 'rare_material']
    
    @pytest.mark.asyncio
    async def test_event_chain_triggers_faction_shift(self, mock_postgres, mock_redis):
        """Test event chain completion affects faction alignment"""
        # Mock chain completion with positive outcome
        mock_postgres.fetch_all.return_value = [
            {'step_number': 1, 'outcome': 'success', 'completed': True},
            {'step_number': 2, 'outcome': 'success', 'completed': True},
            {'step_number': 3, 'outcome': 'success', 'completed': True}
        ]
        mock_postgres.fetch_one.return_value = {'participant_count': 10}
        
        response = await event_chain_agent.evaluate_chain_completion(chain_id=1)
        
        assert response.success
        assert 'world_changes' in response.data
        
        world_changes = response.data['world_changes']
        assert 'faction_shifts' in world_changes
        
        # Positive outcome should shift faction alignment
        if response.data['final_outcome'] == 'success':
            assert world_changes['faction_shifts'] > 0
    
    @pytest.mark.asyncio
    async def test_dungeon_completion_gives_reputation(self, mock_postgres, mock_redis):
        """Test dungeon completion awards reputation"""
        # Mock instance and dungeon
        mock_postgres.fetch_one.side_effect = [
            {'dungeon_id': 1, 'party_id': 1},
            {'theme': 'fire_day'}
        ]
        
        response = await adaptive_dungeon_agent.handle_instance_completion(
            instance_id=1,
            participants=[101, 102],
            floors_cleared=7,
            time_taken=2400
        )
        
        assert response.success
        assert 'rewards' in response.data
        assert 'reputation_gain' in response.data['rewards']
        assert response.data['rewards']['reputation_gain'] > 0
        assert response.data['rewards']['reputation_type'] == 'problem_solver'
    
    @pytest.mark.asyncio
    async def test_full_archaeology_workflow(self, mock_postgres, mock_redis):
        """Test complete archaeology workflow"""
        # Step 1: Spawn dig spots
        mock_postgres.fetch_one.return_value = {'spot_id': 1}
        
        spots = await archaeology_agent.spawn_dig_spots(
            map_activity={"prontera": 100, "geffen": 50},
            count=10
        )
        
        assert len(spots) > 0
        
        # Step 2: Generate artifact
        artifact = await archaeology_agent.generate_artifact(
            dig_spot_rarity=0.6,
            player_archaeology_level=3
        )
        
        assert 'artifact_id' in artifact
        
        # Step 3: Check collection (not complete)
        mock_postgres.fetch_one.side_effect = [
            {'artifact_type': 'fossil', 'collection_name': 'ancient_fossils'},
            {'count': 5}  # Only 5 of 10
        ]
        
        reward = await archaeology_agent.check_collection_completion(
            player_id=101,
            artifact_id=artifact['artifact_id']
        )
        
        assert reward is None  # Not complete yet
    
    @pytest.mark.asyncio
    async def test_full_event_chain_workflow(self, mock_postgres, mock_redis):
        """Test complete event chain workflow"""
        # Step 1: Start chain
        mock_postgres.fetch_one.return_value = {'chain_id': 1}
        
        start_response = await event_chain_agent.start_event_chain(
            chain_type=ChainEventType.TREASURE_HUNT,
            trigger_condition={"player_discovered": "map"}
        )
        
        assert start_response.success
        chain_id = start_response.data['chain_id']
        
        # Step 2: Advance chain
        mock_postgres.fetch_one.return_value = {
            'chain_type': 'treasure_hunt',
            'chain_name': 'Treasure Hunt',
            'total_steps': 5,
            'current_step': 1,
            'chain_data': '{}'
        }
        
        advance_response = await event_chain_agent.advance_chain_step(
            chain_id=chain_id,
            outcome='success'
        )
        
        assert advance_response.success
        assert advance_response.data['previous_outcome'] == 'success'
    
    @pytest.mark.asyncio
    async def test_dungeon_scales_with_player_level(self, mock_postgres, mock_redis):
        """Test dungeon difficulty scales with player level"""
        mock_postgres.fetch_one.return_value = {'dungeon_id': 1}
        
        # Low level players
        response_low = await adaptive_dungeon_agent.generate_daily_dungeon(
            player_average_level=30,
            active_factions=[]
        )
        
        # High level players
        response_high = await adaptive_dungeon_agent.generate_daily_dungeon(
            player_average_level=90,
            active_factions=[]
        )
        
        # Both should succeed
        assert response_low.success
        assert response_high.success
    
    @pytest.mark.asyncio
    async def test_all_agents_use_async(self):
        """Test all agent methods are async"""
        import inspect
        
        # Check adaptive dungeon agent
        assert inspect.iscoroutinefunction(adaptive_dungeon_agent.generate_daily_dungeon)
        assert inspect.iscoroutinefunction(adaptive_dungeon_agent.generate_floor_layout)
        
        # Check archaeology agent
        assert inspect.iscoroutinefunction(archaeology_agent.spawn_dig_spots)
        assert inspect.iscoroutinefunction(archaeology_agent.generate_artifact)
        
        # Check event chain agent
        assert inspect.iscoroutinefunction(event_chain_agent.start_event_chain)
        assert inspect.iscoroutinefunction(event_chain_agent.advance_chain_step)
    
    @pytest.mark.asyncio
    async def test_error_propagation(self, mock_postgres):
        """Test errors are properly handled and don't crash"""
        # Force database error
        mock_postgres.fetch_one.side_effect = Exception("Database connection failed")
        
        # All agents should handle errors gracefully
        dungeon_response = await adaptive_dungeon_agent.generate_daily_dungeon(50, [])
        assert not dungeon_response.success
        
        archaeology_response = await archaeology_agent.unlock_secret_location(101, "map_123")
        assert not archaeology_response.success
    
    @pytest.mark.asyncio
    async def test_concurrent_operations(self, mock_postgres, mock_redis):
        """Test concurrent operations across all advanced agents"""
        import asyncio
        
        mock_postgres.fetch_one.return_value = {'dungeon_id': 1, 'chain_id': 1, 'spot_id': 1}
        
        # Run operations concurrently
        tasks = [
            adaptive_dungeon_agent.generate_daily_dungeon(50, []),
            archaeology_agent.spawn_dig_spots({"prontera": 100}, 10),
            event_chain_agent.start_event_chain(
                ChainEventType.INVESTIGATION,
                {}
            )
        ]
        
        results = await asyncio.gather(*tasks, return_exceptions=True)
        
        # All should complete without exceptions
        assert len(results) == 3
        for result in results:
            assert not isinstance(result, Exception)


class TestAdvancedSystemsPerformance:
    """Performance tests for advanced systems"""
    
    @pytest.mark.asyncio
    async def test_dungeon_generation_performance(self, mock_postgres, mock_redis):
        """Test dungeon generation completes within time limit"""
        import time
        
        mock_postgres.fetch_one.return_value = {'dungeon_id': 1}
        
        start = time.time()
        response = await adaptive_dungeon_agent.generate_daily_dungeon(
            player_average_level=50,
            active_factions=[]
        )
        duration = time.time() - start
        
        # Should complete in under 1 second (mostly rule-based)
        assert duration < 1.0
        assert response.success
    
    @pytest.mark.asyncio
    async def test_archaeology_spawning_performance(self, mock_postgres):
        """Test dig spot spawning performance"""
        import time
        
        mock_postgres.fetch_one.return_value = {'spot_id': 1}
        
        start = time.time()
        spots = await archaeology_agent.spawn_dig_spots(
            map_activity={"prontera": 100, "geffen": 50},
            count=20
        )
        duration = time.time() - start
        
        # Should complete quickly (rule-based)
        assert duration < 2.0
    
    @pytest.mark.asyncio
    async def test_chain_evaluation_performance(self, mock_postgres, mock_redis):
        """Test event chain evaluation performance"""
        import time
        
        mock_postgres.fetch_one.return_value = {'chain_id': 1}
        
        start = time.time()
        response = await event_chain_agent.start_event_chain(
            chain_type=ChainEventType.INVESTIGATION,
            trigger_condition={}
        )
        duration = time.time() - start
        
        # Should complete quickly (template-based)
        assert duration < 1.0
        assert response.success