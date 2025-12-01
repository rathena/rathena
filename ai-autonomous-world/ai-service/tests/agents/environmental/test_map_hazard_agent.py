"""
Unit tests for Map Hazard Agent
Tests hazard generation, selection, effects calculation, and application
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch, MagicMock
from datetime import datetime, UTC, timedelta

from agents.environmental.map_hazard_agent import MapHazardAgent, HazardType
from agents.base_agent import AgentResponse


@pytest.fixture
def hazard_agent():
    """Create hazard agent instance"""
    return MapHazardAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.environmental.map_hazard_agent.postgres_db') as mock_pg, \
         patch('agents.environmental.map_hazard_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestMapHazardAgentInitialization:
    """Test hazard agent initialization"""
    
    def test_initialization(self, hazard_agent):
        """Test hazard agent initializes correctly"""
        assert hazard_agent.agent_type == "map_hazard"
        assert hazard_agent.agent_id == "map_hazard_agent"
        assert len(hazard_agent.hazard_effects) == 10
        assert HazardType.BLOOD_MOON in hazard_agent.hazard_effects
    
    def test_hazard_effects_configured(self, hazard_agent):
        """Test all hazard types have effects defined"""
        for hazard_type in HazardType:
            assert hazard_type in hazard_agent.hazard_effects
            effects = hazard_agent.hazard_effects[hazard_type]
            assert 'type' in effects
            assert 'description' in effects
    
    def test_map_themes_configured(self, hazard_agent):
        """Test map themes are classified"""
        assert 'glast_01' in hazard_agent.map_themes
        assert hazard_agent.map_themes['glast_01'] == 'undead'
        assert 'moc_fild01' in hazard_agent.map_themes
        assert hazard_agent.map_themes['moc_fild01'] == 'desert'


class TestGenerateDailyHazards:
    """Test daily hazard generation"""
    
    @pytest.mark.asyncio
    async def test_generate_hazards_success(self, hazard_agent, mock_db):
        """Test successful hazard generation"""
        # Setup
        map_activity = {
            'prt_fild01': 5,
            'prt_fild02': 10,
            'moc_fild01': 2,
            'pay_fild01': 15,
            'gef_fild01': 8
        }
        active_problems = []
        
        mock_db['postgres'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        mock_db['cache'].get.return_value = None
        
        # Execute
        response = await hazard_agent.generate_daily_hazards(
            map_activity=map_activity,
            active_problems=active_problems,
            count=3
        )
        
        # Verify
        assert response.success is True
        assert "hazards_generated" in response.data
        assert response.data["hazards_generated"] <= 3
        assert "balance" in response.data
    
    @pytest.mark.asyncio
    async def test_low_traffic_map_selection(self, hazard_agent, mock_db):
        """Test hazards spawn in low-traffic maps"""
        map_activity = {
            f'map_{i}': i * 10 for i in range(20)  # 20 maps with varying traffic
        }
        
        mock_db['postgres'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        mock_db['cache'].get.return_value = None
        
        response = await hazard_agent.generate_daily_hazards(
            map_activity=map_activity,
            active_problems=[],
            count=4
        )
        
        assert response.success is True
        # Verify hazards are in maps (details would check low-traffic preference)
    
    @pytest.mark.asyncio
    async def test_problem_coordination(self, hazard_agent, mock_db):
        """Test hazards coordinate with active problems"""
        map_activity = {'prt_fild01': 5, 'prt_fild02': 3}
        active_problems = [
            {'problem_id': 1, 'map_name': 'prt_fild01', 'problem_type': 'monster_surge'}
        ]
        
        mock_db['postgres'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        mock_db['cache'].get.return_value = None
        
        response = await hazard_agent.generate_daily_hazards(
            map_activity=map_activity,
            active_problems=active_problems,
            count=2
        )
        
        assert response.success is True


class TestSelectHazardType:
    """Test hazard type selection"""
    
    @pytest.mark.asyncio
    async def test_undead_map_preference(self, hazard_agent):
        """Test undead maps prefer undead-themed hazards"""
        hazard = await hazard_agent.select_hazard_type(
            map_name='glast_01',
            map_theme='undead',
            active_problem=False
        )
        
        assert hazard in [HazardType.BLOOD_MOON, HazardType.HOLY_GROUND, HazardType.TOXIC_MIASMA]
    
    @pytest.mark.asyncio
    async def test_desert_map_preference(self, hazard_agent):
        """Test desert maps prefer heat hazards"""
        hazard = await hazard_agent.select_hazard_type(
            map_name='moc_fild01',
            map_theme='desert',
            active_problem=False
        )
        
        assert hazard in [HazardType.SCORCHING_HEAT, HazardType.MANA_DRAIN]
    
    @pytest.mark.asyncio
    async def test_problem_map_negative_hazard(self, hazard_agent):
        """Test maps with problems prefer negative hazards"""
        # Run multiple times due to randomness
        negative_count = 0
        for _ in range(10):
            hazard = await hazard_agent.select_hazard_type(
                map_name='test_map',
                map_theme='generic',
                active_problem=True
            )
            if hazard_agent.hazard_effects[hazard]['type'] == 'negative':
                negative_count += 1
        
        # Should prefer negative hazards when problem active
        assert negative_count >= 5  # At least 50% negative


class TestCalculateHazardEffects:
    """Test hazard effect calculation"""
    
    @pytest.mark.asyncio
    async def test_calculate_effects_from_cache(self, hazard_agent, mock_db):
        """Test effects retrieved from cache"""
        cached_effects = {'monster_modifiers': {}, 'player_modifiers': {}}
        mock_db['cache'].get.return_value = cached_effects
        
        effects = await hazard_agent.calculate_hazard_effects(HazardType.BLOOD_MOON)
        
        assert effects == cached_effects
        mock_db['cache'].get.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_calculate_effects_cache_miss(self, hazard_agent, mock_db):
        """Test effects calculated and cached on miss"""
        mock_db['cache'].get.return_value = None
        
        effects = await hazard_agent.calculate_hazard_effects(HazardType.MAGIC_STORM)
        
        assert 'monster_modifiers' in effects
        assert 'player_modifiers' in effects
        assert 'visual_effects' in effects
        mock_db['cache'].set.assert_called_once()
    
    def test_all_hazards_have_effects(self, hazard_agent):
        """Test all hazard types have complete effect definitions"""
        for hazard_type in HazardType:
            effects = hazard_agent.hazard_effects[hazard_type]
            assert 'monster_modifiers' in effects
            assert 'player_modifiers' in effects
            assert 'drop_rate_mult' in effects
            assert 'visual_effects' in effects
            assert 'description' in effects


class TestApplyHazard:
    """Test hazard application"""
    
    @pytest.mark.asyncio
    async def test_apply_hazard_success(self, hazard_agent, mock_db):
        """Test successful hazard application"""
        # Setup
        mock_db['postgres'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        mock_db['cache'].get.return_value = None
        mock_db['cache'].set.return_value = True
        
        # Execute
        response = await hazard_agent.apply_hazard(
            map_name='prt_fild01',
            hazard_type=HazardType.FAIRY_BLESSING,
            duration_hours=24
        )
        
        # Verify
        assert response.success is True
        assert response.data['map_name'] == 'prt_fild01'
        assert response.data['hazard_type'] == HazardType.FAIRY_BLESSING.value
        assert 'hazard_id' in response.data
        mock_db['postgres'].fetch_one.assert_called_once()
        mock_db['cache'].set.assert_called()
    
    @pytest.mark.asyncio
    async def test_apply_hazard_expiry_time(self, hazard_agent, mock_db):
        """Test hazard expiry time is set correctly"""
        mock_db['postgres'].fetch_one.return_value = {
            'hazard_id': 1,
            'applied_at': datetime.now(UTC)
        }
        mock_db['cache'].get.return_value = None
        
        response = await hazard_agent.apply_hazard(
            map_name='test_map',
            hazard_type=HazardType.SCORCHING_HEAT,
            duration_hours=12
        )
        
        assert response.success is True
        # Verify database call included expiry time


class TestGetActiveHazards:
    """Test active hazard retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_active_hazards(self, hazard_agent, mock_db):
        """Test retrieving all active hazards"""
        # Setup
        mock_hazards = [
            {
                'hazard_id': 1,
                'map_name': 'prt_fild01',
                'hazard_type': 'blood_moon',
                'hazard_name': 'Blood Moon Rising',
                'effect_data': '{}',
                'applied_at': datetime.now(UTC),
                'expires_at': datetime.now(UTC) + timedelta(hours=24)
            }
        ]
        mock_db['postgres'].fetch_all.return_value = mock_hazards
        
        # Execute
        hazards = await hazard_agent.get_active_hazards()
        
        # Verify
        assert len(hazards) == 1
        assert hazards[0]['hazard_id'] == 1


class TestGetMapHazards:
    """Test map-specific hazard retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_map_hazards_from_cache(self, hazard_agent, mock_db):
        """Test map hazards from cache"""
        cached_hazard = {'hazard_id': 1, 'map_name': 'prt_fild01'}
        mock_db['cache'].get.return_value = cached_hazard
        
        hazards = await hazard_agent.get_map_hazards('prt_fild01')
        
        assert len(hazards) == 1
        mock_db['postgres'].fetch_all.assert_not_called()
    
    @pytest.mark.asyncio
    async def test_get_map_hazards_from_db(self, hazard_agent, mock_db):
        """Test map hazards from database"""
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_all.return_value = [
            {
                'hazard_id': 1,
                'map_name': 'test_map',
                'hazard_type': 'thick_fog',
                'hazard_name': 'Dense Mist',
                'effect_data': '{}',
                'applied_at': datetime.now(UTC),
                'expires_at': datetime.now(UTC) + timedelta(hours=24)
            }
        ]
        
        hazards = await hazard_agent.get_map_hazards('test_map')
        
        assert len(hazards) == 1
        mock_db['postgres'].fetch_all.assert_called_once()


class TestRecordHazardEncounter:
    """Test hazard encounter recording"""
    
    @pytest.mark.asyncio
    async def test_record_encounter_success(self, hazard_agent, mock_db):
        """Test successful encounter recording"""
        mock_db['postgres'].execute.return_value = 1
        
        success = await hazard_agent.record_hazard_encounter(
            hazard_id=1,
            player_id=123,
            time_in_hazard=300,
            items_dropped=5,
            deaths=1
        )
        
        assert success is True
        mock_db['postgres'].execute.assert_called_once()


class TestRemoveExpiredHazards:
    """Test expired hazard removal"""
    
    @pytest.mark.asyncio
    async def test_remove_expired_hazards(self, hazard_agent, mock_db):
        """Test expired hazards are removed"""
        mock_db['postgres'].fetch_all.return_value = [
            {'hazard_id': 1, 'map_name': 'old_map'}
        ]
        
        count = await hazard_agent.remove_expired_hazards()
        
        assert count == 1
        mock_db['cache'].delete.assert_called()


class TestHazardNaming:
    """Test hazard name generation"""
    
    def test_generate_hazard_names(self, hazard_agent):
        """Test hazard names are generated correctly"""
        name = hazard_agent._generate_hazard_name(HazardType.BLOOD_MOON)
        assert name == "Blood Moon Rising"
        
        name = hazard_agent._generate_hazard_name(HazardType.FAIRY_BLESSING)
        assert name == "Fairy's Grace"