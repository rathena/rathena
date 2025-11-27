"""
Tests for World State Collector
"""

import pytest
import asyncio
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from storyline.world_state_collector import WorldStateCollector, world_state_collector
from models.storyline import WorldStateSnapshot


@pytest.fixture
def collector():
    """Create WorldStateCollector instance"""
    return WorldStateCollector()


@pytest.mark.asyncio
class TestWorldStateCollector:
    """Test WorldStateCollector functionality"""
    
    async def test_collector_initialization(self, collector):
        """Test collector initializes correctly"""
        assert collector.cache_ttl == 3600
        assert isinstance(collector.collection_history, list)
    
    async def test_collect_complete_world_state(self, collector):
        """Test complete world state collection"""
        # Mock all collection methods
        with patch.object(collector, '_collect_map_activity', return_value={
            'most_visited': ['prontera', 'geffen'],
            'least_visited': ['abyss_03'],
            'kill_counts': {'prontera': 1000}
        }):
            with patch.object(collector, '_collect_player_metrics', return_value={
                'online_count': 50,
                'avg_level': 75,
                'level_distribution': {50: 10, 60: 20}
            }):
                with patch.object(collector, '_collect_mvp_data', return_value={
                    'kill_frequency': {'1511': 5},
                    'respawn_times': {'1511': 2.5}
                }):
                    with patch.object(collector, '_collect_economy_data', return_value={
                        'zeny_circulation': 1000000,
                        'inflation_rate': 0.05,
                        'top_items': ['Red Potion']
                    }):
                        with patch.object(collector, '_collect_faction_data', return_value={
                            'dominant_faction': 'rune_alliance',
                            'faction_scores': {'rune_alliance': 5000},
                            'recent_conflicts': []
                        }):
                            with patch.object(collector, '_collect_karma_data', return_value={
                                'karma_score': 3500,
                                'alignment': 'good'
                            }):
                                with patch.object(collector, '_collect_problem_data', return_value={
                                    'active_problems': [],
                                    'recent_outcomes': []
                                }):
                                    with patch.object(collector, '_collect_story_context', return_value={
                                        'previous_arcs': [],
                                        'current_chapter': 0,
                                        'player_choices': []
                                    }):
                                        # Collect world state
                                        world_state = await collector.collect_complete_world_state(force_fresh=True)
                                        
                                        # Verify snapshot
                                        assert isinstance(world_state, WorldStateSnapshot)
                                        assert world_state.online_player_count == 50
                                        assert world_state.player_average_level == 75
                                        assert world_state.global_karma == 3500
                                        assert world_state.dominant_faction == 'rune_alliance'
                                        assert 'prontera' in world_state.most_visited_maps
    
    async def test_export_to_json(self, collector, tmp_path):
        """Test exporting world state to JSON"""
        # Create mock world state
        world_state = WorldStateSnapshot(
            online_player_count=10,
            player_average_level=50,
            random_seed="test123",
            timestamp=int(datetime.now(UTC).timestamp())
        )
        
        # Export to temp file
        filepath = tmp_path / "world_state_test.json"
        success = await collector.export_to_json(world_state, str(filepath))
        
        assert success
        assert filepath.exists()
        
        # Verify JSON content
        import json
        with open(filepath, 'r') as f:
            data = json.load(f)
        
        assert data['online_player_count'] == 10
        assert data['player_average_level'] == 50
        assert '_metadata' in data
    
    async def test_cache_functionality(self, collector):
        """Test world state caching"""
        # Mock cache get/set
        with patch('storyline.world_state_collector.db') as mock_db:
            mock_db.get = AsyncMock(return_value=None)
            mock_db.set = AsyncMock()
            
            # Should return None on cache miss
            cached = await collector._get_cached_world_state()
            assert cached is None
            
            # Cache a state
            test_state = {'test': 'data'}
            await collector._cache_world_state(test_state)
            
            mock_db.set.assert_called_once()
    
    async def test_random_seed_generation(self, collector):
        """Test random seed generation"""
        seed1 = collector._generate_random_seed()
        
        # Wait a moment
        await asyncio.sleep(0.01)
        
        seed2 = collector._generate_random_seed()
        
        # Seeds should be different (time-based)
        assert seed1 != seed2
        assert len(seed1) == 32  # MD5 hash length
    
    async def test_data_point_counting(self, collector):
        """Test data point counting"""
        world_state = WorldStateSnapshot(
            most_visited_maps=['map1', 'map2', 'map3'],
            least_visited_maps=['map4'],
            map_kill_counts={'map1': 100, 'map2': 200},
            player_level_distribution={50: 10, 60: 5},
            random_seed="test",
            timestamp=int(datetime.now(UTC).timestamp())
        )
        
        count = collector._count_data_points(world_state)
        
        # 3 + 1 + 2 + 2 = 8 data points
        assert count == 8


@pytest.mark.asyncio
async def test_global_instance():
    """Test global world_state_collector instance exists"""
    assert world_state_collector is not None
    assert isinstance(world_state_collector, WorldStateCollector)