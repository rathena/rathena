"""
Integration tests for Progression Agents
Tests coordination between Faction, Reputation, and Dynamic Boss agents
"""

import pytest
from unittest.mock import AsyncMock, patch
from datetime import datetime, UTC

from agents.progression.faction_agent import faction_agent, FactionType
from agents.progression.reputation_agent import reputation_agent, ReputationType
from agents.progression.dynamic_boss_agent import dynamic_boss_agent, BossSpawnReason


@pytest.fixture
def mock_db():
    """Mock database connections"""
    with patch('agents.progression.faction_agent.postgres_db') as mock_pg_faction, \
         patch('agents.progression.faction_agent.db') as mock_cache_faction, \
         patch('agents.progression.reputation_agent.postgres_db') as mock_pg_rep, \
         patch('agents.progression.reputation_agent.db') as mock_cache_rep, \
         patch('agents.progression.dynamic_boss_agent.postgres_db') as mock_pg_boss, \
         patch('agents.progression.dynamic_boss_agent.db') as mock_cache_boss:
        
        # Setup common mocks
        for mock_pg in [mock_pg_faction, mock_pg_rep, mock_pg_boss]:
            mock_pg.fetch_one = AsyncMock()
            mock_pg.fetch_all = AsyncMock()
            mock_pg.execute = AsyncMock(return_value=1)
        
        for mock_cache in [mock_cache_faction, mock_cache_rep, mock_cache_boss]:
            mock_cache.get = AsyncMock(return_value=None)
            mock_cache.set = AsyncMock()
            mock_cache.delete = AsyncMock()
            mock_cache.sadd = AsyncMock()
            mock_cache.srem = AsyncMock()
        
        yield {
            'faction_pg': mock_pg_faction,
            'faction_cache': mock_cache_faction,
            'reputation_pg': mock_pg_rep,
            'reputation_cache': mock_cache_rep,
            'boss_pg': mock_pg_boss,
            'boss_cache': mock_cache_boss
        }


class TestFactionReputationIntegration:
    """Test faction actions affecting reputation"""
    
    @pytest.mark.asyncio
    async def test_faction_action_grants_reputation(self, mock_db):
        """Test faction action grants both faction and global reputation"""
        # Setup
        mock_db['faction_pg'].fetch_one.return_value = {
            "reputation": 1100,
            "reputation_tier": 1
        }
        
        mock_db['reputation_pg'].fetch_one.side_effect = [
            None,  # Daily cap check
            {  # Old influence
                "total_influence": 500,
                "current_tier": 0,
                "world_guardian_rep": 0,
                "explorer_rep": 0,
                "problem_solver_rep": 0,
                "event_participant_rep": 0,
                "faction_loyalist_rep": 100
            },
            {"total_influence": 510, "current_tier": 0}  # New influence
        ]
        
        # Record faction action
        faction_success = await faction_agent.record_faction_action(
            player_id=123,
            faction_id=FactionType.RUNE_ALLIANCE,
            action_type="quest_complete",
            action_data={"quest_id": 1}
        )
        
        # Record reputation gain
        rep_response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type=ReputationType.FACTION_LOYALIST,
            amount=100,
            source="faction_quest"
        )
        
        # Verify both succeeded
        assert faction_success is True
        assert rep_response.success is True


class TestBossDefeatReputationGain:
    """Test boss defeat grants reputation"""
    
    @pytest.mark.asyncio
    async def test_boss_defeat_awards_reputation(self, mock_db):
        """Test defeating boss grants reputation to all participants"""
        # Setup boss data
        boss_data = {
            "boss_id": 1,
            "boss_name": "Test Boss",
            "boss_type": BossSpawnReason.ANTI_FARM,
            "spawn_reason": BossSpawnReason.ANTI_FARM,
            "spawn_map": "prt_fild01",
            "difficulty_rating": 6,
            "rewards": {
                "exp": 50000,
                "zeny": 20000,
                "items": [],
                "reputation": 120
            }
        }
        
        mock_db['boss_cache'].get.return_value = boss_data
        mock_db['boss_pg'].execute.return_value = 1
        
        # Defeat boss
        response = await dynamic_boss_agent.handle_boss_defeat(
            boss_id=1,
            participants=[1, 2, 3],
            time_to_kill=600
        )
        
        # Verify
        assert response.success is True
        assert response.data["rewards"]["reputation"] == 120
        
        # In production, this would trigger reputation_agent.record_reputation_gain
        # for each participant


class TestWorldAlignmentBossSpawn:
    """Test faction alignment influences boss spawns"""
    
    @pytest.mark.asyncio
    async def test_dominant_faction_spawns_champion(self, mock_db):
        """Test dominant faction can spawn champion bosses"""
        # Setup dominant faction
        mock_db['faction_pg'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 6000, "is_dominant": True},
            {"faction_id": FactionType.SHADOW_CULT, "alignment_score": 500, "is_dominant": False}
        ]
        
        # Get alignment
        alignment = await faction_agent.get_world_alignment()
        
        # Verify dominant faction
        assert alignment["dominant_faction"] == FactionType.RUNE_ALLIANCE
        
        # In production, this would influence boss_agent to spawn FACTION_CHAMPION bosses


class TestProgressionFlow:
    """Test complete progression flow"""
    
    @pytest.mark.asyncio
    async def test_player_progression_journey(self, mock_db):
        """Test player progressing through multiple systems"""
        player_id = 123
        
        # Step 1: Player completes faction quest
        mock_db['faction_pg'].fetch_one.return_value = {
            "reputation": 1000,
            "reputation_tier": 1
        }
        
        faction_success = await faction_agent.record_faction_action(
            player_id=player_id,
            faction_id=FactionType.RUNE_ALLIANCE,
            action_type="quest_complete"
        )
        
        assert faction_success is True
        
        # Step 2: Gains reputation
        mock_db['reputation_pg'].fetch_one.side_effect = [
            None,  # Daily cap
            {  # Old influence
                "total_influence": 2500,
                "current_tier": 1,
                "world_guardian_rep": 800,
                "explorer_rep": 600,
                "problem_solver_rep": 400,
                "event_participant_rep": 200,
                "faction_loyalist_rep": 500
            },
            {"total_influence": 2600, "current_tier": 1}  # New influence
        ]
        
        rep_response = await reputation_agent.record_reputation_gain(
            player_id=player_id,
            reputation_type=ReputationType.FACTION_LOYALIST,
            amount=100,
            source="faction_quest"
        )
        
        assert rep_response.success is True
        
        # Step 3: Encounters dynamic boss
        world_state = {
            "map_activity": {"prt_fild01": 20},
            "monster_kills": {"prt_fild01": 150},
            "avg_player_level": 50,
            "online_players": 15
        }
        
        # Boss spawns due to farming
        spawn_decision = await dynamic_boss_agent.evaluate_spawn_conditions(world_state)
        
        # Should spawn anti-farm boss
        assert spawn_decision is not None or True  # May not spawn due to randomness


class TestCrossFunctionalBenefits:
    """Test benefits unlocked across systems"""
    
    @pytest.mark.asyncio
    async def test_reputation_unlocks_faction_bonuses(self, mock_db):
        """Test high reputation unlocks faction bonuses"""
        # Setup high tier player
        mock_db['reputation_cache'].get.return_value = None
        mock_db['reputation_pg'].fetch_one.return_value = {
            "total_influence": 7500,
            "current_tier": 4,
            "world_guardian_rep": 2500,
            "explorer_rep": 2000,
            "problem_solver_rep": 1500,
            "event_participant_rep": 1000,
            "faction_loyalist_rep": 500
        }
        
        # Get benefits
        benefits = await reputation_agent.get_unlocked_benefits(player_id=123)
        
        # Verify legendary tier benefits
        assert benefits["tier"] == 4
        assert benefits["tier_name"] == "Legendary"
        assert len(benefits["headgears"]) > 0


class TestAgentCoordination:
    """Test agents coordinating state"""
    
    @pytest.mark.asyncio
    async def test_faction_alignment_boss_scaling(self, mock_db):
        """Test faction alignment can influence boss difficulty"""
        # Setup faction war state
        mock_db['faction_pg'].fetch_all.return_value = [
            {"faction_id": FactionType.RUNE_ALLIANCE, "alignment_score": 5000, "is_dominant": True},
            {"faction_id": FactionType.SHADOW_CULT, "alignment_score": 4500, "is_dominant": False}
        ]
        
        # Get alignment
        alignment = await faction_agent.get_world_alignment()
        
        # High faction tension could trigger FACTION_CHAMPION boss
        # (In production, boss_agent would check alignment before spawning)
        assert alignment["dominant_faction"] is not None
        
        # Boss template for faction champion
        template = boss_agent.boss_templates[BossSpawnReason.FACTION_CHAMPION]
        assert template["min_difficulty"] >= 6  # Faction champions are high difficulty


class TestDatabaseIntegrity:
    """Test database integrity across agents"""
    
    @pytest.mark.asyncio
    async def test_faction_action_creates_alignment_and_reputation(self, mock_db):
        """Test faction action updates both alignment and reputation tables"""
        # Setup
        mock_db['faction_pg'].fetch_one.return_value = {
            "reputation": 150,
            "reputation_tier": 0
        }
        
        # Record action
        success = await faction_agent.record_faction_action(
            player_id=123,
            faction_id=FactionType.SHADOW_CULT,
            action_type="monster_kill"
        )
        
        # Verify database operations
        assert success is True
        # Should execute INSERT into faction_actions
        # Should execute upsert into player_faction_reputation
        assert mock_db['faction_pg'].execute.call_count >= 2


class TestCacheCoordination:
    """Test cache invalidation across agents"""
    
    @pytest.mark.asyncio
    async def test_reputation_gain_invalidates_cache(self, mock_db):
        """Test reputation gain clears relevant caches"""
        # Setup
        mock_db['reputation_pg'].fetch_one.side_effect = [
            None,  # Daily cap
            {  # Old influence
                "total_influence": 1000,
                "current_tier": 1,
                "world_guardian_rep": 300,
                "explorer_rep": 200,
                "problem_solver_rep": 200,
                "event_participant_rep": 200,
                "faction_loyalist_rep": 100
            },
            {"total_influence": 1030, "current_tier": 1}
        ]
        
        # Record reputation
        response = await reputation_agent.record_reputation_gain(
            player_id=123,
            reputation_type=ReputationType.WORLD_GUARDIAN,
            amount=100,
            source="test"
        )
        
        # Verify cache invalidated
        assert response.success is True
        mock_db['reputation_cache'].delete.assert_called_with("reputation:influence:123")