"""
Unit tests for Weather/Time Agent
Tests weather updates, time-of-day effects, and full moon mechanics
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch
from datetime import datetime, UTC, timedelta

from agents.environmental.weather_time_agent import WeatherTimeAgent, WeatherType, TimeOfDay
from agents.base_agent import AgentResponse


@pytest.fixture
def weather_agent():
    """Create weather/time agent instance"""
    return WeatherTimeAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.environmental.weather_time_agent.postgres_db') as mock_pg, \
         patch('agents.environmental.weather_time_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestWeatherTimeAgentInitialization:
    """Test weather/time agent initialization"""
    
    def test_initialization(self, weather_agent):
        """Test weather agent initializes correctly"""
        assert weather_agent.agent_type == "weather_time"
        assert weather_agent.agent_id == "weather_time_agent"
        assert len(weather_agent.weather_effects) == 7
        assert len(weather_agent.time_effects) == 5
    
    def test_weather_types_configured(self, weather_agent):
        """Test all weather types have effects"""
        for weather_type in WeatherType:
            assert weather_type in weather_agent.weather_effects
            effects = weather_agent.weather_effects[weather_type]
            assert 'description' in effects
            assert 'visual_effects' in effects
    
    def test_time_effects_configured(self, weather_agent):
        """Test all time periods have effects"""
        for time_of_day in TimeOfDay:
            assert time_of_day in weather_agent.time_effects
            effects = weather_agent.time_effects[time_of_day]
            assert 'description' in effects
            assert 'exp_mult' in effects


class TestUpdateWeather:
    """Test weather update functionality"""
    
    @pytest.mark.asyncio
    async def test_update_weather_natural(self, weather_agent, mock_db):
        """Test natural weather transition"""
        # Setup
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            'current_weather': 'clear',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        mock_db['postgres'].execute.return_value = 1
        
        # Execute
        response = await weather_agent.update_weather()
        
        # Verify
        assert response.success is True
        assert 'weather_type' in response.data
        assert 'effects' in response.data
        mock_db['postgres'].execute.assert_called()
    
    @pytest.mark.asyncio
    async def test_update_weather_forced(self, weather_agent, mock_db):
        """Test forced weather change (for events)"""
        mock_db['postgres'].execute.return_value = 1
        mock_db['cache'].get.return_value = None
        
        response = await weather_agent.update_weather(
            current_weather=WeatherType.CLEAR,
            force_weather=WeatherType.STORM
        )
        
        assert response.success is True
        assert response.data['weather_type'] == WeatherType.STORM.value
    
    @pytest.mark.asyncio
    async def test_weather_transition_probabilities(self, weather_agent):
        """Test weather transitions follow probability distribution"""
        current = WeatherType.CLEAR
        
        # Run multiple transitions to check probabilities
        next_weathers = []
        for _ in range(100):
            next_weather = weather_agent._select_next_weather(current)
            next_weathers.append(next_weather)
        
        # Most should be CLEAR (60% probability)
        clear_count = next_weathers.count(WeatherType.CLEAR)
        assert clear_count >= 40  # At least 40% should be clear (allowing variance)


class TestCalculateWeatherEffects:
    """Test weather effect calculation"""
    
    @pytest.mark.asyncio
    async def test_calculate_effects_cached(self, weather_agent, mock_db):
        """Test effects use cache"""
        cached = {'element_modifiers': {'water': 1.2}}
        mock_db['cache'].get.return_value = cached
        
        effects = await weather_agent.calculate_weather_effects(WeatherType.RAIN)
        
        assert effects == cached
        mock_db['cache'].get.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_calculate_effects_cache_miss(self, weather_agent, mock_db):
        """Test effects calculated on cache miss"""
        mock_db['cache'].get.return_value = None
        
        effects = await weather_agent.calculate_weather_effects(WeatherType.STORM)
        
        assert 'element_modifiers' in effects
        assert 'stat_modifiers' in effects
        mock_db['cache'].set.assert_called_once()


class TestGetTimeOfDayEffects:
    """Test time-of-day effect calculation"""
    
    @pytest.mark.asyncio
    async def test_dawn_effects(self, weather_agent, mock_db):
        """Test dawn time effects"""
        mock_db['cache'].get.return_value = None  # No full moon
        
        effects = await weather_agent.get_time_of_day_effects(current_hour=6)
        
        assert effects['time_of_day'] == TimeOfDay.DAWN.value
        assert effects['exp_mult'] == 1.1  # Dawn bonus
    
    @pytest.mark.asyncio
    async def test_day_effects(self, weather_agent, mock_db):
        """Test daytime effects"""
        mock_db['cache'].get.return_value = None
        
        effects = await weather_agent.get_time_of_day_effects(current_hour=12)
        
        assert effects['time_of_day'] == TimeOfDay.DAY.value
        assert effects['exp_mult'] == 1.0  # Normal
    
    @pytest.mark.asyncio
    async def test_night_effects(self, weather_agent, mock_db):
        """Test night time effects"""
        mock_db['cache'].get.return_value = None
        
        effects = await weather_agent.get_time_of_day_effects(current_hour=22)
        
        assert effects['time_of_day'] == TimeOfDay.NIGHT.value
        assert len(effects['special_spawns']) > 0  # Ghost spawns


class TestFullMoonMechanics:
    """Test full moon functionality"""
    
    @pytest.mark.asyncio
    async def test_full_moon_cached(self, weather_agent, mock_db):
        """Test full moon state uses cache"""
        mock_db['cache'].get.return_value = True
        
        is_full_moon = await weather_agent.check_full_moon()
        
        assert is_full_moon is True
        mock_db['cache'].get.assert_called()
    
    @pytest.mark.asyncio
    async def test_full_moon_random_chance(self, weather_agent, mock_db):
        """Test full moon random determination"""
        mock_db['cache'].get.return_value = None
        
        # Run during night hours
        with patch('agents.environmental.weather_time_agent.datetime') as mock_dt:
            mock_dt.now.return_value = datetime(2025, 1, 1, 22, 0, 0, tzinfo=UTC)
            mock_dt.now().hour = 22
            
            is_full_moon = await weather_agent.check_full_moon()
            
            assert isinstance(is_full_moon, bool)
            mock_db['cache'].set.assert_called()  # Decision cached
    
    @pytest.mark.asyncio
    async def test_full_moon_effects_stronger(self, weather_agent, mock_db):
        """Test full moon has stronger effects than regular night"""
        mock_db['cache'].get.return_value = True
        
        effects = await weather_agent.get_time_of_day_effects(current_hour=22)
        
        assert effects['time_of_day'] == TimeOfDay.FULL_MOON.value
        assert effects['exp_mult'] > 1.0
        assert effects['drop_rate_mult'] > 1.0


class TestGetCurrentWeather:
    """Test current weather retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_weather_from_cache(self, weather_agent, mock_db):
        """Test weather from cache"""
        cached = {
            'weather_type': 'clear',
            'weather_name': 'Clear Skies',
            'intensity': 1.0
        }
        mock_db['cache'].get.return_value = cached
        
        weather = await weather_agent.get_current_weather()
        
        assert weather == cached
        mock_db['postgres'].fetch_one.assert_not_called()
    
    @pytest.mark.asyncio
    async def test_get_weather_from_db(self, weather_agent, mock_db):
        """Test weather from database"""
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            'current_weather': 'rain',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        
        weather = await weather_agent.get_current_weather()
        
        assert weather is not None
        assert weather['weather_type'] == 'rain'
        mock_db['cache'].set.assert_called()


class TestWeatherForecast:
    """Test weather forecasting"""
    
    @pytest.mark.asyncio
    async def test_forecast_generation(self, weather_agent, mock_db):
        """Test forecast is generated"""
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            'current_weather': 'clear',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        
        forecast = await weather_agent.get_weather_forecast(hours=24)
        
        assert len(forecast) == 8  # 24 hours / 3 hour intervals
        for period in forecast:
            assert 'weather_type' in period
            assert 'probability' in period


class TestApplyWeatherTimeEffects:
    """Test combined weather and time effects"""
    
    @pytest.mark.asyncio
    async def test_combined_effects(self, weather_agent, mock_db):
        """Test weather and time effects combine"""
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = {
            'current_weather': 'rain',
            'weather_intensity': 1.0,
            'effect_data': '{}',
            'started_at': datetime.now(UTC),
            'next_change_at': datetime.now(UTC) + timedelta(hours=3)
        }
        
        response = await weather_agent.apply_weather_time_effects()
        
        assert response.success is True
        assert 'weather' in response.data
        assert 'time' in response.data
        assert 'synergies' in response.data
    
    def test_synergy_calculation(self, weather_agent):
        """Test weather-time synergies are calculated"""
        synergies = weather_agent._calculate_synergies(
            WeatherType.RAIN,
            TimeOfDay.NIGHT
        )
        
        assert isinstance(synergies, list)
        # Rain + Night should have storm potential
        assert any('Storm' in s for s in synergies)


class TestWeatherNaming:
    """Test weather name generation"""
    
    def test_weather_names(self, weather_agent):
        """Test weather display names"""
        assert weather_agent._get_weather_name(WeatherType.CLEAR) == "Clear Skies"
        assert weather_agent._get_weather_name(WeatherType.STORM) == "Thunderstorm"
        assert weather_agent._get_weather_name(WeatherType.AURORA) == "Aurora Borealis"