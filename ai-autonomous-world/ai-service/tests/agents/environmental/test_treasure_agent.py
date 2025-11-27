"""
Unit tests for Treasure Agent
Tests treasure spawning, rarity distribution, discovery, and card fragment system
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch
from datetime import datetime, UTC, timedelta

from agents.environmental.treasure_agent import TreasureAgent, TreasureRarity, TreasureType
from agents.base_agent import AgentResponse


@pytest.fixture
def treasure_agent():
    """Create treasure agent instance"""
    return TreasureAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.environmental.treasure_agent.postgres_db') as mock_pg, \
         patch('agents.environmental.treasure_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestTreasureAgentInitialization:
    """Test treasure agent initialization"""
    
    def test_initialization(self, treasure_agent):
        """Test treasure agent initializes correctly"""
        assert treasure_agent.agent_type == "treasure"
        assert treasure_agent.agent_id == "treasure_agent"
        assert len(treasure_agent.rarity_weights) == 4  # Traffic categories
        assert len(treasure_agent.treasure_templates) == 4  # Rarity tiers
    
    def test_rarity_weights_configured(self, treasure_agent):
        """Test rarity weights for all traffic levels"""
        for category in ['very_low_traffic', 'low_traffic', 'medium_traffic', 'high_traffic']:
            assert category in treasure_agent.rarity_weights
            weights = treasure_agent.rarity_weights[category]
            # Weights should sum to ~1.0
            total = sum(weights.values())
            assert 0.99 <= total <= 1.01
    
    def test_treasure_templates_configured(self, treasure_agent):
        """Test treasure content templates"""
        for rarity in TreasureRarity:
            assert rarity in treasure_agent.treasure_templates
            template = treasure_agent.treasure_templates[rarity]
            assert 'item_count' in template
            assert 'items' in template
            assert 'zeny' in template


class TestSpawnDailyTreasures:
    """Test daily treasure spawning"""
    
    @pytest.mark.asyncio
    async def test_spawn_treasures_success(self, treasure_agent, mock_db):
        """Test successful treasure spawning"""
        # Setup
        map_activity = {
            f'map_{i}': i * 5 for i in range(20)  # 20 maps with varying traffic
        }
        
        mock_db['postgres'].fetch_one.return_value = {
            'treasure_id': 1,
            'spawned_at': datetime.now(UTC)
        }
        
        # Execute
        response = await treasure_agent.spawn_daily_treasures(
            map_activity=map_activity,
            count=10
        )
        
        # Verify
        assert response.success is True
        assert "treasures_spawned" in response.data
        assert response.data["treasures_spawned"] <= 10
        assert "distribution" in response.data
        assert "rarity_breakdown" in response.data
    
    @pytest.mark.asyncio
    async def test_treasure_distribution_favors_low_traffic(self, treasure_agent, mock_db):
        """Test treasures spawn more in low-traffic maps"""
        map_activity = {f'map_{i}': i * 10 for i in range(50)}
        
        mock_db['postgres'].fetch_one.return_value = {
            'treasure_id': 1,
            'spawned_at': datetime.now(UTC)
        }
        
        response = await treasure_agent.spawn_daily_treasures(
            map_activity=map_activity,
            count=10
        )
        
        assert response.success is True
        dist = response.data.get("distribution", {})
        # Verify higher distribution in low-traffic areas
        assert dist.get('very_low_traffic', 0) + dist.get('low_traffic', 0) >= dist.get('high_traffic', 0)


class TestDetermineTreasureRarity:
    """Test treasure rarity determination"""
    
    def test_very_low_traffic_rarity(self, treasure_agent):
        """Test very low traffic maps get better rarity"""
        rarity = treasure_agent.determine_treasure_rarity(0.05)  # Bottom 5%
        # Should be weighted toward higher rarities
        assert rarity in [TreasureRarity.MYTHIC, TreasureRarity.GOLD, TreasureRarity.SILVER]
    
    def test_high_traffic_rarity(self, treasure_agent):
        """Test high traffic maps get lower rarity"""
        # Run multiple times to check distribution
        rarities = []
        for _ in range(20):
            rarity = treasure_agent.determine_treasure_rarity(0.95)  # Top 5%
            rarities.append(rarity)
        
        # Should heavily favor bronze
        bronze_count = rarities.count(TreasureRarity.BRONZE)
        assert bronze_count >= 10  # At least 50% bronze
    
    def test_mythic_rarity_rare(self, treasure_agent):
        """Test mythic treasures are very rare in high traffic"""
        rarities = []
        for _ in range(100):
            rarity = treasure_agent.determine_treasure_rarity(0.8)
            rarities.append(rarity)
        
        mythic_count = rarities.count(TreasureRarity.MYTHIC)
        assert mythic_count <= 2  # Should be very rare (0.5%)


class TestGenerateTreasureContents:
    """Test treasure content generation"""
    
    @pytest.mark.asyncio
    async def test_bronze_contents(self, treasure_agent):
        """Test bronze treasure contents"""
        contents = await treasure_agent.generate_treasure_contents(
            TreasureRarity.BRONZE,
            (1, 50)
        )
        
        assert 'items' in contents
        assert 'zeny' in contents
        assert contents['reputation_reward'] == 10
        assert contents['card_fragments'] == 0
        assert len(contents['items']) >= 3
    
    @pytest.mark.asyncio
    async def test_mythic_contents(self, treasure_agent):
        """Test mythic treasure contents"""
        contents = await treasure_agent.generate_treasure_contents(
            TreasureRarity.MYTHIC,
            (80, 99)
        )
        
        assert contents['reputation_reward'] == 100
        assert contents['card_fragments'] >= 15  # Base 15 + bonus
        assert contents['zeny'] >= 200000


class TestDiscoverTreasure:
    """Test treasure discovery"""
    
    @pytest.mark.asyncio
    async def test_discover_treasure_success(self, treasure_agent, mock_db):
        """Test successful treasure discovery"""
        # Setup
        mock_db['postgres'].fetch_one.side_effect = [
            {  # Treasure query
                'treasure_id': 1,
                'treasure_type': 'chest',
                'rarity': 'silver',
                'spawn_map': 'test_map',
                'contents': '{"items": [], "zeny": 10000, "reputation_reward": 25, "card_fragments": 1}',
                'status': 'active'
            },
            None  # reputation_agent query (may fail)
        ]
        mock_db['postgres'].execute.return_value = 1
        
        # Execute
        response = await treasure_agent.discover_treasure(
            treasure_id=1,
            player_id=123
        )
        
        # Verify
        assert response.success is True
        assert response.data['treasure_id'] == 1
        assert response.data['player_id'] == 123
        assert 'rewards' in response.data
        mock_db['cache'].delete.assert_called()
    
    @pytest.mark.asyncio
    async def test_discover_already_claimed(self, treasure_agent, mock_db):
        """Test discovering already claimed treasure fails"""
        mock_db['postgres'].fetch_one.return_value = {
            'treasure_id': 1,
            'status': 'claimed'
        }
        
        response = await treasure_agent.discover_treasure(
            treasure_id=1,
            player_id=123
        )
        
        assert response.success is False
        assert 'already claimed' in response.data.get('error', '').lower()


class TestCardFragmentSystem:
    """Test card fragment mechanics"""
    
    @pytest.mark.asyncio
    async def test_get_card_fragments(self, treasure_agent, mock_db):
        """Test getting player's card fragments"""
        mock_db['postgres'].fetch_one.return_value = {
            'fragment_count': 75,
            'cards_claimed': 2,
            'last_fragment_at': datetime.now(UTC)
        }
        
        data = await treasure_agent.get_card_fragments(123)
        
        assert data['fragment_count'] == 75
        assert data['cards_claimed'] == 2
        assert data['fragments_needed'] == 25  # 100 - 75
    
    @pytest.mark.asyncio
    async def test_claim_card_success(self, treasure_agent, mock_db):
        """Test successful card claim from fragments"""
        mock_db['postgres'].fetch_one.side_effect = [
            {'fragment_count': 150, 'cards_claimed': 0, 'last_fragment_at': None},  # get_card_fragments
            {'fragment_count': 50, 'cards_claimed': 1}  # After claim
        ]
        
        response = await treasure_agent.claim_card_from_fragments(123)
        
        assert response.success is True
        assert response.data['fragments_spent'] == 100
        assert response.data['fragments_remaining'] == 50
        assert 'card_obtained' in response.data
    
    @pytest.mark.asyncio
    async def test_claim_card_insufficient_fragments(self, treasure_agent, mock_db):
        """Test claiming card without enough fragments fails"""
        mock_db['postgres'].fetch_one.return_value = {
            'fragment_count': 50,
            'cards_claimed': 0,
            'last_fragment_at': None
        }
        
        response = await treasure_agent.claim_card_from_fragments(123)
        
        assert response.success is False
        assert 'Insufficient' in response.data.get('error', '')


class TestGetActiveTreasures:
    """Test active treasure listing"""
    
    @pytest.mark.asyncio
    async def test_get_active_treasures_no_hints(self, treasure_agent, mock_db):
        """Test getting treasures without hints"""
        mock_db['postgres'].fetch_all.return_value = [
            {
                'treasure_id': 1,
                'treasure_type': 'chest',
                'rarity': 'gold',
                'spawn_map': 'test_map',
                'spawned_at': datetime.now(UTC),
                'despawns_at': datetime.now(UTC) + timedelta(hours=6)
            }
        ]
        
        treasures = await treasure_agent.get_active_treasures(include_hints=False)
        
        assert len(treasures) == 1
        assert 'hint_text' not in treasures[0]
    
    @pytest.mark.asyncio
    async def test_get_active_treasures_with_hints(self, treasure_agent, mock_db):
        """Test getting treasures with hints (admin)"""
        mock_db['postgres'].fetch_all.return_value = [
            {
                'treasure_id': 1,
                'treasure_type': 'chest',
                'rarity': 'gold',
                'spawn_map': 'test_map',
                'spawned_at': datetime.now(UTC),
                'despawns_at': datetime.now(UTC) + timedelta(hours=6),
                'hint_text': 'A treasure lies here',
                'spawn_x': 100,
                'spawn_y': 150
            }
        ]
        
        treasures = await treasure_agent.get_active_treasures(include_hints=True)
        
        assert len(treasures) == 1
        assert treasures[0]['hint_text'] == 'A treasure lies here'


class TestDespawnExpiredTreasures:
    """Test treasure despawning"""
    
    @pytest.mark.asyncio
    async def test_despawn_expired(self, treasure_agent, mock_db):
        """Test expired treasures are despawned"""
        mock_db['postgres'].fetch_all.return_value = [
            {'treasure_id': 1, 'spawn_map': 'old_map'},
            {'treasure_id': 2, 'spawn_map': 'old_map2'}
        ]
        
        count = await treasure_agent.despawn_expired_treasures()
        
        assert count == 2
        assert mock_db['cache'].delete.call_count == 2


class TestTreasureTypeSelection:
    """Test treasure type selection"""
    
    def test_type_selection_distribution(self, treasure_agent):
        """Test treasure types follow weight distribution"""
        types = []
        for _ in range(100):
            t = treasure_agent._select_treasure_type()
            types.append(t)
        
        # Chest should be most common (60%)
        chest_count = types.count(TreasureType.CHEST)
        assert chest_count >= 40  # At least 40% (allowing variance)


class TestHintGeneration:
    """Test treasure hint generation"""
    
    def test_hint_text_generated(self, treasure_agent):
        """Test hints are generated for treasures"""
        hint = treasure_agent._generate_hint_text(
            'test_map',
            TreasureType.CHEST,
            TreasureRarity.GOLD
        )
        
        assert 'test_map' in hint
        assert isinstance(hint, str)
        assert len(hint) > 0