"""
Unit tests for Social Interaction Agent
Tests social event spawning, guild tasks, and participation handling
"""

import pytest
import json
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, Mock, patch

from agents.economy_social.social_interaction_agent import (
    SocialInteractionAgent,
    social_interaction_agent,
    SocialEventType
)
from agents.base_agent import AgentResponse


@pytest.fixture
def sample_player_distribution():
    """Sample player distribution for testing"""
    return {
        "prontera": 50,
        "geffen": 20,
        "payon": 10,
        "morocc": 5
    }


@pytest.fixture
def sample_social_metrics():
    """Sample social metrics for testing"""
    return {
        "party_formation_rate": 0.6,
        "average_party_size": 4.5,
        "guild_activity_rate": 0.7,
        "trade_frequency": 120,
        "event_participation_rate": 0.5
    }


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.economy_social.social_interaction_agent.postgres_db') as mock_pg, \
         patch('agents.economy_social.social_interaction_agent.db') as mock_cache:
        
        # Mock PostgreSQL methods
        mock_pg.fetch_one = AsyncMock(return_value=None)
        mock_pg.fetch_all = AsyncMock(return_value=[])
        mock_pg.execute = AsyncMock(return_value=1)
        
        # Mock cache methods
        mock_cache.get = AsyncMock(return_value=None)
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        
        yield {"postgres": mock_pg, "cache": mock_cache}


class TestSocialInteractionAgent:
    """Test suite for Social Interaction Agent"""
    
    @pytest.mark.asyncio
    async def test_initialization(self):
        """Test agent initialization"""
        agent = SocialInteractionAgent()
        
        assert agent.agent_type == "social"
        assert len(agent.event_templates) == 6
        assert len(agent.guild_task_templates) == 5
        assert len(agent.location_categories) > 0
    
    @pytest.mark.asyncio
    async def test_spawn_daily_social_events(self, sample_player_distribution, mock_db):
        """Test daily social event spawning"""
        agent = SocialInteractionAgent()
        
        # Mock event spawn to return ID
        mock_db['postgres'].fetch_one.return_value = {"event_id": 123}
        
        responses = await agent.spawn_daily_social_events(
            player_distribution=sample_player_distribution,
            count=5
        )
        
        assert len(responses) == 5
        assert all(resp.success for resp in responses)
        assert all('event_id' in resp.data for resp in responses)
    
    @pytest.mark.asyncio
    async def test_spawn_diverse_event_types(self, sample_player_distribution, mock_db):
        """Test event type diversity"""
        agent = SocialInteractionAgent()
        
        mock_db['postgres'].fetch_one.return_value = {"event_id": 123}
        
        responses = await agent.spawn_daily_social_events(
            player_distribution=sample_player_distribution,
            count=6
        )
        
        # Check that event types are diverse
        event_types = [resp.data['event_type'] for resp in responses]
        unique_types = set(event_types)
        
        assert len(unique_types) >= 3  # At least 3 different types
    
    @pytest.mark.asyncio
    async def test_generate_guild_tasks(self, mock_db):
        """Test guild task generation"""
        agent = SocialInteractionAgent()
        
        mock_db['postgres'].fetch_one.return_value = {"task_id": 456}
        
        tasks = await agent.generate_guild_tasks(
            guild_count=5,
            average_guild_size=10
        )
        
        assert len(tasks) == 15  # 3 tasks per guild * 5 guilds
        assert all('task_id' in task for task in tasks)
        assert all('guild_id' in task for task in tasks)
    
    @pytest.mark.asyncio
    async def test_generate_guild_tasks_scaling(self, mock_db):
        """Test guild task difficulty scaling by guild size"""
        agent = SocialInteractionAgent()
        
        mock_db['postgres'].fetch_one.return_value = {"task_id": 456}
        
        # Small guild
        tasks_small = await agent.generate_guild_tasks(
            guild_count=1,
            average_guild_size=5
        )
        
        # Large guild
        tasks_large = await agent.generate_guild_tasks(
            guild_count=1,
            average_guild_size=20
        )
        
        # Larger guild should get higher thresholds
        threshold_small = tasks_small[0]['completion_threshold']
        threshold_large = tasks_large[0]['completion_threshold']
        
        assert threshold_large > threshold_small
    
    @pytest.mark.asyncio
    async def test_handle_social_participation_success(self, mock_db):
        """Test successful social event participation"""
        agent = SocialInteractionAgent()
        
        participants = [100001, 100002, 100003]
        
        # Mock successful participation
        with patch.object(agent, '_distribute_event_rewards', return_value={'test': 'rewards'}):
            response = await agent.handle_social_participation(
                event_id=1,
                participants=participants,
                event_type=SocialEventType.PICNIC,
                party_id=5
            )
        
        assert response.success
        assert response.data['participants'] == 3
        assert response.data['party_id'] == 5
    
    @pytest.mark.asyncio
    async def test_handle_social_participation_party_requirement_fail(self, mock_db):
        """Test participation failure due to party size requirement"""
        agent = SocialInteractionAgent()
        
        # Only 1 participant, but picnic requires 2+
        response = await agent.handle_social_participation(
            event_id=1,
            participants=[100001],
            event_type=SocialEventType.PICNIC,
            party_id=None
        )
        
        assert not response.success
        assert 'requirement' in response.data['error'].lower()
    
    @pytest.mark.asyncio
    async def test_analyze_social_health_healthy(self, sample_social_metrics):
        """Test social health analysis for healthy community"""
        agent = SocialInteractionAgent()
        
        analysis = await agent.analyze_social_health(sample_social_metrics)
        
        assert 'health_score' in analysis
        assert 'status' in analysis
        assert 'recommendations' in analysis
        assert analysis['status'] in ['healthy', 'fair', 'poor']
    
    @pytest.mark.asyncio
    async def test_analyze_social_health_poor(self):
        """Test social health analysis for poor community"""
        agent = SocialInteractionAgent()
        
        poor_metrics = {
            "party_formation_rate": 0.1,
            "average_party_size": 1.5,
            "guild_activity_rate": 0.2,
            "trade_frequency": 10,
            "event_participation_rate": 0.1
        }
        
        analysis = await agent.analyze_social_health(poor_metrics)
        
        assert analysis['health_score'] < 0.4
        assert analysis['status'] == 'poor'
        assert len(analysis['recommendations']) > 0
    
    @pytest.mark.asyncio
    async def test_get_active_social_events(self, mock_db):
        """Test getting active social events"""
        agent = SocialInteractionAgent()
        
        # Mock active events
        mock_events = [
            {
                "event_id": 1,
                "event_type": "picnic",
                "event_name": "Sunset Picnic",
                "spawn_map": "prontera",
                "spawn_x": 150,
                "spawn_y": 150,
                "requirements": json.dumps({"min_party_size": 2}),
                "rewards": json.dumps({"buff": "Social Bonus"}),
                "participation_count": 5,
                "status": "active",
                "spawned_at": datetime.now(UTC),
                "despawns_at": datetime.now(UTC) + timedelta(hours=4)
            }
        ]
        mock_db['postgres'].fetch_all.return_value = mock_events
        
        events = await agent.get_active_social_events()
        
        assert len(events) == 1
        assert events[0]['event_id'] == 1
    
    def test_select_spawn_locations(self, sample_player_distribution):
        """Test spawn location selection"""
        agent = SocialInteractionAgent()
        
        locations = agent._select_spawn_locations(sample_player_distribution, count=3)
        
        assert len(locations) == 3
        # Should prefer high-traffic maps
        assert "prontera" in locations
    
    def test_select_diverse_event_types(self):
        """Test diverse event type selection"""
        agent = SocialInteractionAgent()
        
        event_types = agent._select_diverse_event_types(count=5)
        
        assert len(event_types) == 5
        # Check diversity (should have at least 3 unique types)
        unique_types = set(event_types)
        assert len(unique_types) >= 3
    
    @pytest.mark.asyncio
    async def test_generate_event_spec(self):
        """Test event specification generation"""
        agent = SocialInteractionAgent()
        
        spec = await agent._generate_event_spec(
            event_type=SocialEventType.PICNIC,
            map_name="prontera",
            index=0
        )
        
        assert spec['event_type'] == 'picnic'
        assert spec['spawn_map'] == 'prontera'
        assert 'event_name' in spec
        assert 'requirements' in spec
        assert 'rewards' in spec
    
    @pytest.mark.asyncio
    async def test_generate_guild_task_spec(self):
        """Test guild task specification generation"""
        agent = SocialInteractionAgent()
        
        spec = await agent._generate_guild_task_spec(
            guild_id=1,
            task_type='cooperative_hunting',
            guild_size=10
        )
        
        assert spec['guild_id'] == 1
        assert spec['task_type'] == 'cooperative_hunting'
        assert spec['completion_threshold'] > 0
        assert 'rewards' in spec
        assert 'objectives' in spec


@pytest.mark.asyncio
async def test_global_instance():
    """Test global agent instance"""
    assert social_interaction_agent is not None
    assert social_interaction_agent.agent_type == "social"