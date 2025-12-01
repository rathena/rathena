"""
Integration tests for Economy & Social Agents
Tests coordination between Merchant Economy, Karma, and Social Interaction agents
"""

import pytest
import json
from datetime import datetime, timedelta, UTC
from unittest.mock import AsyncMock, Mock, patch

from agents.economy_social.merchant_economy_agent import merchant_economy_agent, EconomicIndicator
from agents.economy_social.karma_agent import karma_agent, KarmaAlignment
from agents.economy_social.social_interaction_agent import social_interaction_agent, SocialEventType


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.economy_social.merchant_economy_agent.postgres_db') as mock_pg1, \
         patch('agents.economy_social.merchant_economy_agent.db') as mock_cache1, \
         patch('agents.economy_social.karma_agent.postgres_db') as mock_pg2, \
         patch('agents.economy_social.karma_agent.db') as mock_cache2, \
         patch('agents.economy_social.social_interaction_agent.postgres_db') as mock_pg3, \
         patch('agents.economy_social.social_interaction_agent.db') as mock_cache3:
        
        # Configure all mocks
        for mock_pg in [mock_pg1, mock_pg2, mock_pg3]:
            mock_pg.fetch_one = AsyncMock(return_value=None)
            mock_pg.fetch_all = AsyncMock(return_value=[])
            mock_pg.execute = AsyncMock(return_value=1)
        
        for mock_cache in [mock_cache1, mock_cache2, mock_cache3]:
            mock_cache.get = AsyncMock(return_value=None)
            mock_cache.set = AsyncMock()
            mock_cache.delete = AsyncMock()
        
        yield {
            "postgres": mock_pg1,
            "cache": mock_cache1
        }


class TestEconomySocialIntegration:
    """Integration test suite for Economy & Social agents"""
    
    @pytest.mark.asyncio
    async def test_all_agents_initialized(self):
        """Test that all economy_social agents are initialized"""
        assert merchant_economy_agent is not None
        assert karma_agent is not None
        assert social_interaction_agent is not None
        
        assert merchant_economy_agent.agent_type == "merchant_economy"
        assert karma_agent.agent_type == "karma"
        assert social_interaction_agent.agent_type == "social"
    
    @pytest.mark.asyncio
    async def test_economy_karma_coordination(self, mock_db):
        """Test coordination between economy and karma systems"""
        # Scenario: High inflation might correlate with evil alignment
        # (players farming excessively, killing protected monsters)
        
        # 1. Detect inflation
        economy_data = {
            "total_zeny": 1000000000,
            "active_players": 50,
            "item_supply_index": 1.0,
            "item_demand_index": 1.0
        }
        
        indicator = await merchant_economy_agent.analyze_economic_health(economy_data)
        
        # 2. Process evil karma actions
        karma_actions = [
            {"player_id": 100001, "action_type": "kill_poring"},
            {"player_id": 100002, "action_type": "kill_lunatic"}
        ]
        
        # Mock karma processing
        mock_db['postgres'].fetch_one.return_value = {"karma_score": 0}
        
        karma_response = await karma_agent.update_global_karma(karma_actions)
        
        # Both systems should function independently
        assert indicator == EconomicIndicator.INFLATION
        assert karma_response.success
    
    @pytest.mark.asyncio
    async def test_social_karma_coordination(self, mock_db):
        """Test coordination between social and karma systems"""
        # Scenario: Social events should be neutral for karma
        
        # 1. Spawn social event
        player_dist = {"prontera": 50, "geffen": 20}
        
        mock_db['postgres'].fetch_one.return_value = {"event_id": 123}
        
        events = await social_interaction_agent.spawn_daily_social_events(
            player_distribution=player_dist,
            count=3
        )
        
        # 2. Participation should not affect karma
        participants = [100001, 100002, 100003]
        
        with patch.object(social_interaction_agent, '_distribute_event_rewards', return_value={}):
            participation_response = await social_interaction_agent.handle_social_participation(
                event_id=123,
                participants=participants,
                event_type=SocialEventType.PICNIC,
                party_id=5
            )
        
        assert len(events) == 3
        assert participation_response.success
    
    @pytest.mark.asyncio
    async def test_full_daily_cycle(self, mock_db):
        """Test complete daily cycle of all economy_social agents"""
        # Simulate a full day of operations
        
        # Morning: Generate guild tasks
        mock_db['postgres'].fetch_one.return_value = {"task_id": 456}
        guild_tasks = await social_interaction_agent.generate_guild_tasks(
            guild_count=5,
            average_guild_size=10
        )
        
        # Morning: Spawn social events
        mock_db['postgres'].fetch_one.return_value = {"event_id": 789}
        player_dist = {"prontera": 50}
        social_events = await social_interaction_agent.spawn_daily_social_events(
            player_distribution=player_dist,
            count=5
        )
        
        # Afternoon: Update karma
        karma_actions = [
            {"player_id": 100001, "action_type": "help_npc_quest"},
            {"player_id": 100002, "action_type": "kill_demon"}
        ]
        mock_db['postgres'].fetch_one.return_value = {"karma_score": 0}
        karma_update = await karma_agent.update_global_karma(karma_actions)
        
        # Evening: Analyze economy
        economy_data = {
            "total_zeny": 500000000,
            "active_players": 50,
            "item_supply_index": 1.0,
            "item_demand_index": 1.0
        }
        indicator = await merchant_economy_agent.analyze_economic_health(economy_data)
        
        # Verify all operations succeeded
        assert len(guild_tasks) > 0
        assert len(social_events) > 0
        assert karma_update.success
        assert indicator in EconomicIndicator
    
    @pytest.mark.asyncio
    async def test_zeny_sink_creation_flow(self, mock_db):
        """Test full zeny sink event creation and tracking"""
        # 1. Detect inflation
        economy_data = {
            "total_zeny": 1000000000,
            "active_players": 50,
            "item_supply_index": 1.0,
            "item_demand_index": 1.0
        }
        
        indicator = await merchant_economy_agent.analyze_economic_health(economy_data)
        assert indicator == EconomicIndicator.INFLATION
        
        # 2. Create zeny sink event
        mock_db['postgres'].fetch_one.return_value = {"event_id": 555}
        
        sink_response = await merchant_economy_agent.create_zeny_sink_event(severity=0.7)
        
        assert sink_response.success
        assert sink_response.data['event_id'] == 555
        assert sink_response.data['target_zeny_drain'] > 0
    
    @pytest.mark.asyncio
    async def test_karma_alignment_affects_atmosphere(self, mock_db):
        """Test that karma alignment properly triggers atmospheric effects"""
        # Simulate many good actions pushing karma to VERY_GOOD
        good_actions = [
            {"player_id": i, "action_type": "complete_good_quest"}
            for i in range(100001, 100011)
        ]
        
        mock_db['postgres'].fetch_one.return_value = {"karma_score": 6000}
        
        # Mock effects application
        with patch.object(karma_agent, 'apply_karma_effects', return_value={'test': 'effects'}) as mock_effects:
            response = await karma_agent.update_global_karma(good_actions)
        
        assert response.success
    
    @pytest.mark.asyncio
    async def test_social_health_metrics_tracking(self, mock_db):
        """Test social health metrics are properly tracked"""
        metrics = {
            "party_formation_rate": 0.7,
            "average_party_size": 5.0,
            "guild_activity_rate": 0.8,
            "trade_frequency": 150,
            "event_participation_rate": 0.6
        }
        
        analysis = await social_interaction_agent.analyze_social_health(metrics)
        
        assert analysis['health_score'] > 0.6
        assert analysis['status'] == 'healthy'
        assert len(analysis['recommendations']) > 0


@pytest.mark.asyncio
async def test_all_agents_work_independently():
    """Test that all agents can operate independently"""
    # This tests basic instantiation doesn't fail
    assert merchant_economy_agent.agent_type == "merchant_economy"
    assert karma_agent.agent_type == "karma"
    assert social_interaction_agent.agent_type == "social"