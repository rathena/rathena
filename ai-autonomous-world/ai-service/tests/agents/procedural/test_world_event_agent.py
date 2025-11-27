"""
Unit tests for World Event Agent
Tests event triggering, threshold monitoring, and impact calculation
"""

import pytest
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, patch

from agents.procedural.world_event_agent import WorldEventAgent, world_event_agent
from models.procedural import EventType, EventStatus


@pytest.fixture
def sample_world_state():
    """Sample world state for testing"""
    return {
        "avg_player_level": 60,
        "online_players": 25,
        "map_activity": {"prontera": 20, "geffen": 15},
        "monster_kills": {"prt_fild01": 200},
        "mvp_kills": {"1038": 10, "1039": 15, "1046": 8},
        "economy": {
            "zeny_circulation": 2000000000,
            "inflation_rate": 1.1
        }
    }


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.procedural.world_event_agent.postgres_db') as mock_pg, \
         patch('agents.procedural.world_event_agent.db') as mock_cache:
        
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestWorldEventAgent:
    """Test suite for World Event Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = WorldEventAgent()
        
        assert agent.agent_type == "world_event"
        assert len(agent.event_thresholds) == 7
        assert agent.max_concurrent_events == 1
    
    def test_check_threshold_mvp_kills_high(self, sample_world_state):
        """Test MVP kills threshold check"""
        agent = WorldEventAgent()
        
        config = agent.event_thresholds[EventType.DEMONIC_INVASION]
        
        # Total MVP kills = 10 + 15 + 8 = 33 (below threshold of 100)
        result = agent._check_threshold(
            EventType.DEMONIC_INVASION,
            config,
            sample_world_state
        )
        
        assert result == False
        
        # Increase MVP kills to exceed threshold
        high_mvp_state = {
            **sample_world_state,
            "mvp_kills": {"1038": 50, "1039": 60}
        }
        
        result = agent._check_threshold(
            EventType.DEMONIC_INVASION,
            config,
            high_mvp_state
        )
        
        assert result == True
    
    def test_check_threshold_low_player_count(self, sample_world_state):
        """Test low player count threshold"""
        agent = WorldEventAgent()
        
        config = agent.event_thresholds[EventType.LUCKY_DAY]
        
        # 25 players > threshold of 10
        result = agent._check_threshold(
            EventType.LUCKY_DAY,
            config,
            sample_world_state
        )
        
        assert result == False
        
        # Reduce to 5 players
        low_player_state = {**sample_world_state, "online_players": 5}
        
        result = agent._check_threshold(
            EventType.LUCKY_DAY,
            config,
            low_player_state
        )
        
        assert result == True
    
    def test_check_threshold_high_zeny(self, sample_world_state):
        """Test high zeny circulation threshold"""
        agent = WorldEventAgent()
        
        config = agent.event_thresholds[EventType.MARKET_CRASH]
        
        # 2B zeny > threshold of 1B
        result = agent._check_threshold(
            EventType.MARKET_CRASH,
            config,
            sample_world_state
        )
        
        assert result == True
    
    def test_is_on_cooldown(self):
        """Test cooldown checking"""
        agent = WorldEventAgent()
        
        # No cooldown initially
        assert agent._is_on_cooldown(EventType.LUCKY_DAY, 4) == False
        
        # Set last trigger time to now
        agent.last_trigger_times[EventType.LUCKY_DAY] = datetime.now(UTC)
        
        # Should be on cooldown
        assert agent._is_on_cooldown(EventType.LUCKY_DAY, 4) == True
        
        # Set last trigger time to 5 hours ago
        agent.last_trigger_times[EventType.LUCKY_DAY] = datetime.now(UTC) - timedelta(hours=5)
        
        # Should not be on cooldown (4 hour cooldown expired)
        assert agent._is_on_cooldown(EventType.LUCKY_DAY, 4) == False
    
    @pytest.mark.asyncio
    async def test_calculate_event_impact(self):
        """Test event impact calculation"""
        agent = WorldEventAgent()
        
        impact = await agent.calculate_event_impact(
            EventType.GLOBAL_XP_BOOST,
            duration_minutes=60
        )
        
        assert "global_buffs" in impact or "spawn_modifications" in impact
        assert "special_effects" in impact
    
    @pytest.mark.asyncio
    async def test_trigger_event(self, sample_world_state, mock_db):
        """Test event triggering"""
        agent = WorldEventAgent()
        
        # Mock database to return event ID
        mock_db['postgres'].fetch_one.return_value = {"event_id": 1}
        
        response = await agent.trigger_event(
            EventType.LUCKY_DAY,
            sample_world_state
        )
        
        assert response.success
        assert "event_id" in response.data
        assert "title" in response.data
        
        # Verify database and cache calls
        assert mock_db['postgres'].fetch_one.called
        assert mock_db['cache'].set.called
    
    @pytest.mark.asyncio
    async def test_trigger_event_invalid_type(self, sample_world_state):
        """Test triggering invalid event type"""
        agent = WorldEventAgent()
        
        response = await agent.trigger_event(
            "invalid_event",
            sample_world_state
        )
        
        assert response.success == False
        assert "error" in response.data
    
    @pytest.mark.asyncio
    async def test_monitor_and_trigger_no_threshold_met(self, sample_world_state, mock_db):
        """Test monitoring when no thresholds are met"""
        agent = WorldEventAgent()
        
        # Mock no active events
        mock_db['postgres'].fetch_all.return_value = []
        
        # Low MVP kills, moderate players - no thresholds met
        low_activity_state = {
            **sample_world_state,
            "online_players": 20,
            "mvp_kills": {"1038": 2}
        }
        
        response = await agent.monitor_and_trigger(low_activity_state)
        
        # Should return None when no event triggered
        assert response is None
    
    @pytest.mark.asyncio
    async def test_monitor_and_trigger_max_concurrent(self, sample_world_state, mock_db):
        """Test monitoring respects max concurrent events"""
        agent = WorldEventAgent()
        
        # Mock 1 active event (max limit)
        mock_db['postgres'].fetch_all.return_value = [
            {
                "event_id": 1,
                "status": "active",
                "ends_at": datetime.now(UTC) + timedelta(hours=1)
            }
        ]
        
        response = await agent.monitor_and_trigger(sample_world_state)
        
        # Should not trigger new event
        assert response is None
    
    @pytest.mark.asyncio
    async def test_record_participation(self, mock_db):
        """Test event participation recording"""
        agent = WorldEventAgent()
        
        success = await agent.record_participation(
            event_id=1,
            player_id=100001,
            contribution_score=500
        )
        
        assert success
        assert mock_db['postgres'].execute.call_count == 2  # INSERT + UPDATE
    
    @pytest.mark.asyncio
    async def test_end_event(self, mock_db):
        """Test event ending"""
        agent = WorldEventAgent()
        
        # Mock successful update
        mock_db['postgres'].execute.return_value = 1
        
        success = await agent.end_event(event_id=1)
        
        assert success
        assert mock_db['cache'].delete.call_count == 2  # event cache + active list
    
    def test_build_trigger_conditions(self, sample_world_state):
        """Test trigger condition building"""
        agent = WorldEventAgent()
        
        config = agent.event_thresholds[EventType.GLOBAL_XP_BOOST]
        
        conditions = agent._build_trigger_conditions(
            EventType.GLOBAL_XP_BOOST,
            config,
            sample_world_state
        )
        
        assert "condition_type" in conditions
        assert "threshold_value" in conditions
        assert "actual_value" in conditions
        assert "description" in conditions
    
    def test_get_actual_value(self, sample_world_state):
        """Test actual value extraction from world state"""
        agent = WorldEventAgent()
        
        # Test player count extraction
        player_value = agent._get_actual_value("low_player_count", sample_world_state)
        assert player_value == 25
        
        # Test MVP kills extraction
        mvp_value = agent._get_actual_value("mvp_kills_high", sample_world_state)
        assert mvp_value == 33  # 10 + 15 + 8
        
        # Test zeny extraction
        zeny_value = agent._get_actual_value("high_zeny_circulation", sample_world_state)
        assert zeny_value == 2000000000


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert world_event_agent is not None
    assert world_event_agent.agent_type == "world_event"