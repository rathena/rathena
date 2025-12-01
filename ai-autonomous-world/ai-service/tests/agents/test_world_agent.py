"""
Test suite for WorldAgent - World state analysis and event generation.

Tests cover:
- World state analysis (time, weather, population)
- Event generation (weather, time transitions, special events)
- World context retrieval
- Event cleanup and expiration
"""

import asyncio
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch
import pytest

from agents.base_agent import AgentContext, AgentStatus
from agents.world_agent import (
    WorldAgent,
    EventType,
    WorldEvent
)
from models.world import WorldState, WeatherType, TimeOfDay, EnvironmentState


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def world_agent():
    """Create world agent instance."""
    return WorldAgent(
        event_generation_interval=1,  # Fast interval for testing
        enable_dynamic_weather=True
    )


@pytest.fixture
def sample_world_state():
    """Create sample world state."""
    return WorldState(
        world_id="test_world",
        world_name="Test World",
        environment=EnvironmentState(
            game_time=720,  # 12:00
            time_of_day=TimeOfDay.NOON,
            weather=WeatherType.CLEAR,
            weather_intensity=0.3,
            season="spring",
            day_of_year=100,
            active_events=[],
            global_modifiers={"exp_rate": 1.0}
        ),
        active_players=25,
        active_npcs=50
    )


@pytest.fixture
def world_context(sample_world_state, faker_instance):
    """Create agent context for world operations."""
    return AgentContext(
        request_id=faker_instance.uuid4(),
        npc_id=faker_instance.uuid4(),
        npc_state=MagicMock(),
        world_state=sample_world_state,
        location="test_town",
        additional_data={}
    )


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_world_agent_initialization():
    """Test world agent initializes correctly."""
    agent = WorldAgent(
        event_generation_interval=60,
        enable_dynamic_weather=True
    )
    
    await agent.initialize()
    
    assert agent.name == "WorldAgent"
    assert agent.event_generation_interval == 60
    assert agent.enable_dynamic_weather is True
    assert len(agent._active_events) == 0
    assert agent._last_event_check is not None
    
    await agent.cleanup()


# ============================================================================
# WORLD ANALYSIS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_analyze_world_state(world_agent, world_context):
    """Test world state analysis."""
    await world_agent.initialize()
    
    world_context.additional_data["operation"] = "analyze"
    response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "time_analysis" in response.result
    assert "weather_analysis" in response.result
    assert "population_analysis" in response.result
    
    # Check time analysis
    time_analysis = response.result["time_analysis"]
    assert time_analysis["time_of_day"] == "noon"
    assert time_analysis["is_day"] is True
    
    # Check weather analysis
    weather_analysis = response.result["weather_analysis"]
    assert weather_analysis["weather"] == "clear"
    assert weather_analysis["severity"] == "mild"
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_analyze_time_conditions(world_agent, sample_world_state):
    """Test time analysis with different conditions."""
    await world_agent.initialize()
    
    # Test night time
    sample_world_state.environment.time_of_day = TimeOfDay.NIGHT
    analysis = world_agent._analyze_time(sample_world_state)
    
    assert analysis["time_of_day"] == "night"
    assert analysis["is_night"] is True
    assert analysis["is_day"] is False
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_analyze_weather_conditions(world_agent, sample_world_state):
    """Test weather analysis with different conditions."""
    await world_agent.initialize()
    
    # Test severe storm
    sample_world_state.environment.weather = WeatherType.STORMY
    sample_world_state.environment.weather_intensity = 0.8
    
    analysis = world_agent._analyze_weather(sample_world_state)
    
    assert analysis["weather"] == "stormy"
    assert analysis["severity"] == "severe"
    assert analysis["impact"] == "high"
    assert analysis["travel_conditions"] == "poor"
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_analyze_population(world_agent, sample_world_state):
    """Test population analysis."""
    await world_agent.initialize()
    
    sample_world_state.active_players = 60
    sample_world_state.active_npcs = 40
    
    analysis = world_agent._analyze_population(sample_world_state)
    
    assert analysis["active_players"] == 60
    assert analysis["active_npcs"] == 40
    assert analysis["total_population"] == 100
    assert analysis["activity_level"] == "high"
    
    await world_agent.cleanup()


@pytest.mark.unit
def test_identify_notable_conditions(world_agent, sample_world_state):
    """Test identification of notable conditions."""
    # Test severe storm condition
    sample_world_state.environment.weather = WeatherType.STORMY
    sample_world_state.environment.weather_intensity = 0.8
    
    conditions = world_agent._identify_notable_conditions(sample_world_state)
    
    assert any("storm" in c.lower() for c in conditions)


# ============================================================================
# EVENT GENERATION TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_weather_event(world_agent, sample_world_state):
    """Test weather event generation."""
    await world_agent.initialize()
    
    with patch('random.choice', return_value=WeatherType.RAINY):
        event = world_agent._generate_weather_event(sample_world_state)
    
    assert event is not None
    assert event.event_type == EventType.WEATHER_CHANGE
    assert "rainy" in event.title.lower()
    assert event.duration_minutes > 0
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_time_transition_event(world_agent, sample_world_state):
    """Test time transition event generation."""
    await world_agent.initialize()
    
    # Set to dawn for transition
    sample_world_state.environment.time_of_day = TimeOfDay.DAWN
    
    event = world_agent._generate_time_event(sample_world_state)
    
    assert event is not None
    assert event.event_type == EventType.TIME_TRANSITION
    assert "dawn" in event.title.lower()
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_special_event(world_agent, sample_world_state):
    """Test special event generation."""
    await world_agent.initialize()
    
    with patch('random.choice') as mock_choice:
        mock_choice.return_value = {
            "type": EventType.FESTIVAL,
            "title": "Harvest Festival",
            "description": "Test festival",
            "severity": 0.3,
            "duration": 240
        }
        
        event = world_agent._generate_special_event(sample_world_state)
    
    assert event is not None
    assert event.event_type == EventType.FESTIVAL
    assert event.duration_minutes == 240
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_events_operation(world_agent, world_context):
    """Test full event generation operation."""
    await world_agent.initialize()
    
    # Force event generation by setting last check to past
    world_agent._last_event_check = datetime.utcnow() - timedelta(minutes=10)
    
    world_context.additional_data["operation"] = "generate_events"
    
    with patch('random.random', return_value=0.1):  # Low values to trigger events
        response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "events" in response.result
    assert "count" in response.result
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_event_generation_timing(world_agent, world_context):
    """Test that events respect generation interval."""
    await world_agent.initialize()
    
    # Set short interval
    world_agent.event_generation_interval = 5
    world_agent._last_event_check = datetime.utcnow()
    
    world_context.additional_data["operation"] = "generate_events"
    response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert response.result["reason"] == "Too soon to generate new events"
    
    await world_agent.cleanup()


# ============================================================================
# EVENT CLEANUP TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_cleanup_expired_events(world_agent):
    """Test that expired events are cleaned up."""
    await world_agent.initialize()
    
    # Create an expired event
    old_event = WorldEvent(
        event_id="old_event",
        event_type=EventType.WEATHER_CHANGE,
        title="Old Event",
        description="Should be cleaned",
        severity=0.5,
        duration_minutes=1,
        affected_locations=["test"]
    )
    old_event.start_time = datetime.utcnow() - timedelta(minutes=5)
    
    # Create a current event
    current_event = WorldEvent(
        event_id="current_event",
        event_type=EventType.FESTIVAL,
        title="Current Event",
        description="Should remain",
        severity=0.3,
        duration_minutes=60,
        affected_locations=["test"]
    )
    
    world_agent._active_events["old_event"] = old_event
    world_agent._active_events["current_event"] = current_event
    
    # Cleanup
    world_agent._cleanup_expired_events()
    
    assert "old_event" not in world_agent._active_events
    assert "current_event" in world_agent._active_events
    
    await world_agent.cleanup()


# ============================================================================
# WORLD CONTEXT TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_get_world_context(world_agent, world_context):
    """Test world context retrieval."""
    await world_agent.initialize()
    
    world_context.additional_data["operation"] = "get_context"
    response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "time_of_day" in response.result
    assert "weather" in response.result
    assert "conditions" in response.result
    
    await world_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_world_context_with_active_events(world_agent, world_context):
    """Test world context includes active events."""
    await world_agent.initialize()
    
    # Add an event
    event = WorldEvent(
        event_id="test_event",
        event_type=EventType.MARKET_DAY,
        title="Market Day",
        description="Market event",
        severity=0.2,
        duration_minutes=120,
        affected_locations=["all", "test_town"]
    )
    world_agent._active_events["test_event"] = event
    
    world_context.additional_data["operation"] = "get_context"
    response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert len(response.result["active_events"]) > 0
    
    await world_agent.cleanup()


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_invalid_operation(world_agent, world_context):
    """Test handling of invalid operations."""
    await world_agent.initialize()
    
    world_context.additional_data["operation"] = "invalid_op"
    response = await world_agent.process(world_context)
    
    assert response.status == AgentStatus.FAILED
    assert response.error is not None
    
    await world_agent.cleanup()


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("weather,intensity,expected_severity", [
    (WeatherType.CLEAR, 0.2, "mild"),
    (WeatherType.RAINY, 0.5, "moderate"),
    (WeatherType.STORMY, 0.8, "severe"),
    (WeatherType.SNOWY, 0.9, "severe"),
])
def test_weather_severity_mapping(world_agent, sample_world_state, weather, intensity, expected_severity):
    """Test weather severity calculations."""
    sample_world_state.environment.weather = weather
    sample_world_state.environment.weather_intensity = intensity
    
    analysis = world_agent._analyze_weather(sample_world_state)
    
    assert analysis["severity"] == expected_severity


@pytest.mark.unit
@pytest.mark.parametrize("players,expected_level", [
    (5, "low"),
    (25, "medium"),
    (75, "high"),
])
def test_activity_level_mapping(world_agent, sample_world_state, players, expected_level):
    """Test activity level calculations."""
    sample_world_state.active_players = players
    
    analysis = world_agent._analyze_population(sample_world_state)
    
    assert analysis["activity_level"] == expected_level