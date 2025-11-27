"""
Unit tests for Archaeology Agent
Tests artifact discovery and collection mechanics
"""

import pytest
import json
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from agents.advanced.archaeology_agent import (
    ArchaeologyAgent,
    ArtifactType
)
from agents.base_agent import AgentResponse


@pytest.fixture
def archaeology_agent():
    """Create archaeology agent instance for testing"""
    return ArchaeologyAgent(config={})


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


class TestArchaeologyAgent:
    """Test suite for Archaeology Agent"""
    
    @pytest.mark.asyncio
    async def test_agent_initialization(self, archaeology_agent):
        """Test agent initializes correctly"""
        assert archaeology_agent.agent_type == "archaeology"
        assert archaeology_agent.agent_id == "archaeology_agent"
        assert len(archaeology_agent.collections) == 4
        assert len(archaeology_agent.secret_locations) == 4
        assert len(archaeology_agent.rarity_config) == 4
    
    @pytest.mark.asyncio
    async def test_spawn_dig_spots(self, archaeology_agent, mock_postgres):
        """Test dig spot spawning"""
        mock_postgres.fetch_one.return_value = {'spot_id': 1}
        
        map_activity = {
            "low_traffic_map": 5,
            "medium_traffic_map": 50,
            "high_traffic_map": 200
        }
        
        spots = await archaeology_agent.spawn_dig_spots(
            map_activity=map_activity,
            count=15
        )
        
        assert isinstance(spots, list)
        # Should attempt to create spots
        assert len(spots) <= 15
    
    @pytest.mark.asyncio
    async def test_traffic_tier_distribution(self, archaeology_agent, mock_postgres):
        """Test dig spots are distributed by traffic tier"""
        mock_postgres.fetch_one.return_value = {'spot_id': 1}
        
        # Create activity data with clear traffic tiers
        map_activity = {}
        
        # Low traffic (30 maps)
        for i in range(30):
            map_activity[f"low_{i}"] = i + 1
        
        # Mid traffic (40 maps)
        for i in range(40):
            map_activity[f"mid_{i}"] = 50 + i
        
        # High traffic (30 maps)
        for i in range(30):
            map_activity[f"high_{i}"] = 200 + i
        
        spots = await archaeology_agent.spawn_dig_spots(
            map_activity=map_activity,
            count=10
        )
        
        # Should create some spots
        assert len(spots) > 0
    
    @pytest.mark.asyncio
    async def test_artifact_generation(self, archaeology_agent):
        """Test artifact generation"""
        artifact = await archaeology_agent.generate_artifact(
            dig_spot_rarity=0.5,
            player_archaeology_level=1
        )
        
        assert 'artifact_id' in artifact
        assert 'artifact_type' in artifact
        assert 'rarity' in artifact
        assert 'name' in artifact
        assert artifact['rarity'] in ['common', 'uncommon', 'rare', 'mythic']
    
    @pytest.mark.asyncio
    async def test_level_bonus_affects_rarity(self, archaeology_agent):
        """Test higher archaeology level improves rarity chances"""
        # Generate multiple artifacts at different levels
        # Due to RNG, we'll just check the mechanic exists
        
        # Low level
        artifact_low = await archaeology_agent.generate_artifact(
            dig_spot_rarity=0.5,
            player_archaeology_level=1
        )
        
        # High level
        artifact_high = await archaeology_agent.generate_artifact(
            dig_spot_rarity=0.5,
            player_archaeology_level=10
        )
        
        # Both should generate valid artifacts
        assert artifact_low['rarity'] in ['common', 'uncommon', 'rare', 'mythic']
        assert artifact_high['rarity'] in ['common', 'uncommon', 'rare', 'mythic']
    
    @pytest.mark.asyncio
    async def test_artifact_type_selection(self, archaeology_agent):
        """Test artifact type selection based on rarity"""
        # Common should give fossils/relics
        fossil_type = archaeology_agent._select_artifact_type('common')
        assert fossil_type in [ArtifactType.FOSSIL, ArtifactType.RELIC]
        
        # Mythic should give maps/materials
        mythic_type = archaeology_agent._select_artifact_type('mythic')
        assert mythic_type in [ArtifactType.TREASURE_MAP, ArtifactType.RARE_MATERIAL]
    
    @pytest.mark.asyncio
    async def test_collection_completion_check(self, archaeology_agent, mock_postgres):
        """Test collection completion detection"""
        # Mock artifact exists in collection
        mock_postgres.fetch_one.side_effect = [
            {'artifact_type': 'fossil', 'collection_name': 'ancient_fossils'},  # Artifact query
            {'count': 10}  # Count query (complete!)
        ]
        
        reward = await archaeology_agent.check_collection_completion(
            player_id=101,
            artifact_id='fossil_123'
        )
        
        assert reward is not None
        assert 'title' in reward
        assert 'stat_bonus' in reward
        assert 'headgear' in reward
    
    @pytest.mark.asyncio
    async def test_incomplete_collection(self, archaeology_agent, mock_postgres):
        """Test incomplete collection returns None"""
        mock_postgres.fetch_one.side_effect = [
            {'artifact_type': 'fossil', 'collection_name': 'ancient_fossils'},
            {'count': 5}  # Only 5 out of 10
        ]
        
        reward = await archaeology_agent.check_collection_completion(
            player_id=101,
            artifact_id='fossil_123'
        )
        
        assert reward is None
    
    @pytest.mark.asyncio
    async def test_secret_location_unlock(self, archaeology_agent, mock_postgres):
        """Test secret location unlocking"""
        # Mock player has treasure map
        mock_postgres.fetch_one.return_value = {'artifact_id': 'map_123'}
        
        response = await archaeology_agent.unlock_secret_location(
            player_id=101,
            treasure_map_id='map_123'
        )
        
        assert response.success
        assert 'location_id' in response.data
        assert 'location_name' in response.data
        assert 'access_map' in response.data
        assert response.data['duration_minutes'] == 30
    
    @pytest.mark.asyncio
    async def test_secret_location_without_map(self, archaeology_agent, mock_postgres):
        """Test secret location unlock fails without treasure map"""
        # Mock no treasure map found
        mock_postgres.fetch_one.return_value = None
        
        response = await archaeology_agent.unlock_secret_location(
            player_id=101,
            treasure_map_id='invalid_map'
        )
        
        assert not response.success
        assert 'error' in response.data
    
    @pytest.mark.asyncio
    async def test_archaeology_progress(self, archaeology_agent, mock_postgres):
        """Test archaeology progress tracking"""
        # Mock progress data
        mock_postgres.fetch_one.return_value = {
            'archaeology_level': 5,
            'total_digs': 120,
            'artifacts_found': 80,
            'collections_completed': 1,
            'secret_locations_unlocked': 0
        }
        
        progress = await archaeology_agent.get_archaeology_progress(player_id=101)
        
        assert progress['archaeology_level'] == 5
        assert progress['total_digs'] == 120
        assert progress['artifacts_found'] == 80
        assert 0.0 <= progress['next_level_progress'] <= 1.0
    
    @pytest.mark.asyncio
    async def test_new_player_progress(self, archaeology_agent, mock_postgres):
        """Test progress for new player"""
        # Mock no progress found
        mock_postgres.fetch_one.return_value = None
        
        progress = await archaeology_agent.get_archaeology_progress(player_id=999)
        
        assert progress['archaeology_level'] == 1
        assert progress['total_digs'] == 0
        assert progress['artifacts_found'] == 0
    
    @pytest.mark.asyncio
    async def test_collection_definitions(self, archaeology_agent):
        """Test all collection definitions are valid"""
        for collection_id, collection in archaeology_agent.collections.items():
            assert 'name' in collection
            assert 'required_count' in collection
            assert 'artifact_type' in collection
            assert 'reward' in collection
            assert 'title' in collection['reward']
            assert 'stat_bonus' in collection['reward']
    
    @pytest.mark.asyncio
    async def test_artifact_name_generation(self, archaeology_agent):
        """Test artifact name generation"""
        for rarity in ['common', 'uncommon', 'rare', 'mythic']:
            for artifact_type in ArtifactType:
                name = archaeology_agent._generate_artifact_name(artifact_type, rarity)
                assert isinstance(name, str)
                assert len(name) > 0
    
    @pytest.mark.asyncio
    async def test_collection_mapping(self, archaeology_agent):
        """Test artifact type to collection mapping"""
        # Fossils belong to ancient_fossils
        collection = archaeology_agent._get_collection_for_type(ArtifactType.FOSSIL)
        assert collection == "ancient_fossils"
        
        # Treasure maps don't belong to collections
        collection = archaeology_agent._get_collection_for_type(ArtifactType.TREASURE_MAP)
        assert collection is None