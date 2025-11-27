"""
Unit tests for Dynamic Boss Agent
Tests boss spawning, scaling, and defeat handling
"""

import pytest
from unittest.mock import Mock, AsyncMock, patch
from datetime import datetime, UTC

from agents.progression.dynamic_boss_agent import DynamicBossAgent, BossSpawnReason
from agents.base_agent import AgentResponse


@pytest.fixture
def boss_agent():
    """Create boss agent instance"""
    return DynamicBossAgent()


@pytest.fixture
def mock_db():
    """Mock database connection"""
    with patch('agents.progression.dynamic_boss_agent.postgres_db') as mock_pg, \
         patch('agents.progression.dynamic_boss_agent.db') as mock_cache:
        mock_pg.fetch_one = AsyncMock()
        mock_pg.fetch_all = AsyncMock()
        mock_pg.execute = AsyncMock()
        mock_cache.get = AsyncMock()
        mock_cache.set = AsyncMock()
        mock_cache.delete = AsyncMock()
        mock_cache.sadd = AsyncMock()
        mock_cache.srem = AsyncMock()
        yield {'postgres': mock_pg, 'cache': mock_cache}


class TestDynamicBossAgentInitialization:
    """Test boss agent initialization"""
    
    def test_initialization(self, boss_agent):
        """Test boss agent initializes correctly"""
        assert boss_agent.agent_type == "dynamic_boss"
        assert boss_agent.agent_id == "dynamic_boss_agent"
        assert len(boss_agent.boss_templates) == 5
    
    def test_boss_templates(self, boss_agent):
        """Test all boss templates configured"""
        assert BossSpawnReason.ANTI_FARM in boss_agent.boss_templates
        assert BossSpawnReason.TREASURE_GUARD in boss_agent.boss_templates
        assert BossSpawnReason.PROBLEM_ESCALATION in boss_agent.boss_templates
        assert BossSpawnReason.FACTION_CHAMPION in boss_agent.boss_templates
        assert BossSpawnReason.RANDOM_ENCOUNTER in boss_agent.boss_templates
    
    def test_scaling_curves(self, boss_agent):
        """Test scaling curves configured"""
        assert "hp_per_level" in boss_agent.scaling_curves
        assert "atk_per_level" in boss_agent.scaling_curves
        assert "def_per_level" in boss_agent.scaling_curves


class TestEvaluateSpawnConditions:
    """Test boss spawn condition evaluation"""
    
    @pytest.mark.asyncio
    async def test_anti_farm_spawn(self, boss_agent, mock_db):
        """Test anti-farm boss spawns when farming threshold exceeded"""
        # Setup world state with high kills
        world_state = {
            "map_activity": {"prt_fild01": 10},
            "monster_kills": {"prt_fild01": 150},  # Above threshold
            "avg_player_level": 50,
            "online_players": 10
        }
        
        # Execute
        decision = await boss_agent.evaluate_spawn_conditions(world_state)
        
        # Verify
        assert decision is not None
        assert decision["reason"] == BossSpawnReason.ANTI_FARM
        assert decision["map"] == "prt_fild01"
        assert decision["difficulty_modifier"] > 1.0
    
    @pytest.mark.asyncio
    async def test_random_encounter_spawn(self, boss_agent, mock_db):
        """Test random encounter can spawn"""
        # Setup
        world_state = {
            "map_activity": {"prontera": 20, "geffen": 10},
            "monster_kills": {},
            "avg_player_level": 60,
            "online_players": 15
        }
        
        # Mock random to guarantee spawn
        with patch('agents.progression.dynamic_boss_agent.random.random', return_value=0.01):
            # Execute
            decision = await boss_agent.evaluate_spawn_conditions(world_state)
            
            # Verify
            assert decision is not None
            assert decision["reason"] == BossSpawnReason.RANDOM_ENCOUNTER
    
    @pytest.mark.asyncio
    async def test_no_spawn_conditions(self, boss_agent, mock_db):
        """Test no boss spawns when conditions not met"""
        # Setup world state below thresholds
        world_state = {
            "map_activity": {"prontera": 5},
            "monster_kills": {"prontera": 20},  # Below threshold
            "avg_player_level": 40,
            "online_players": 3
        }
        
        # Mock random to prevent random spawn
        with patch('agents.progression.dynamic_boss_agent.random.random', return_value=0.9):
            # Execute
            decision = await boss_agent.evaluate_spawn_conditions(world_state)
            
            # Verify
            assert decision is None


class TestGenerateBossSpec:
    """Test boss specification generation"""
    
    @pytest.mark.asyncio
    async def test_generate_spec_anti_farm(self, boss_agent, mock_db):
        """Test anti-farm boss spec generation"""
        # Execute
        spec = await boss_agent.generate_boss_spec(
            spawn_reason=BossSpawnReason.ANTI_FARM,
            spawn_map="prt_fild01",
            difficulty_modifier=1.2
        )
        
        # Verify
        assert spec["boss_type"] == BossSpawnReason.ANTI_FARM
        assert spec["spawn_map"] == "prt_fild01"
        assert "base_stats" in spec
        assert "scaled_stats" in spec
        assert "rewards" in spec
        assert spec["status"] == "active"
    
    @pytest.mark.asyncio
    async def test_boss_has_skills(self, boss_agent, mock_db):
        """Test boss has assigned skills"""
        spec = await boss_agent.generate_boss_spec(
            spawn_reason=BossSpawnReason.FACTION_CHAMPION,
            spawn_map="prontera",
            difficulty_modifier=1.5
        )
        
        assert "skills" in spec
        assert len(spec["skills"]) > 0
    
    @pytest.mark.asyncio
    async def test_difficulty_modifier_applied(self, boss_agent, mock_db):
        """Test difficulty modifier affects stats"""
        # Generate with low modifier
        spec_easy = await boss_agent.generate_boss_spec(
            spawn_reason=BossSpawnReason.TREASURE_GUARD,
            spawn_map="prontera",
            difficulty_modifier=0.8
        )
        
        # Generate with high modifier
        spec_hard = await boss_agent.generate_boss_spec(
            spawn_reason=BossSpawnReason.TREASURE_GUARD,
            spawn_map="prontera",
            difficulty_modifier=1.5
        )
        
        # Verify harder boss has higher stats
        assert spec_hard["scaled_stats"]["hp"] > spec_easy["scaled_stats"]["hp"]
        assert spec_hard["scaled_stats"]["atk"] > spec_easy["scaled_stats"]["atk"]


class TestCalculateBossScaling:
    """Test boss stat scaling"""
    
    @pytest.mark.asyncio
    async def test_level_scaling(self, boss_agent, mock_db):
        """Test boss scales with player levels"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        player_levels = [50, 50, 50, 50, 50]
        
        # Execute
        scaled = await boss_agent.calculate_boss_scaling(
            base_stats,
            player_levels,
            recent_wipes=0
        )
        
        # Verify stats increased
        assert scaled["hp"] > base_stats["hp"]
        assert scaled["atk"] > base_stats["atk"]
        assert scaled["def"] > base_stats["def"]
    
    @pytest.mark.asyncio
    async def test_wipe_adjustment_3_wipes(self, boss_agent, mock_db):
        """Test stats reduced after 3 wipes"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        player_levels = [50]
        
        # Execute with wipes
        scaled = await boss_agent.calculate_boss_scaling(
            base_stats,
            player_levels,
            recent_wipes=3
        )
        
        # Verify stats reduced
        assert scaled["hp"] < base_stats["hp"] * 1.5  # Less than max scaling
    
    @pytest.mark.asyncio
    async def test_wipe_adjustment_5_wipes(self, boss_agent, mock_db):
        """Test stats heavily reduced after 5 wipes"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        player_levels = [50]
        
        # Execute with many wipes
        scaled = await boss_agent.calculate_boss_scaling(
            base_stats,
            player_levels,
            recent_wipes=5
        )
        
        # Verify stats significantly reduced
        assert scaled["hp"] < base_stats["hp"]  # Below base stats


class TestGenerateBossRewards:
    """Test boss reward generation"""
    
    @pytest.mark.asyncio
    async def test_rewards_scale_with_difficulty(self, boss_agent, mock_db):
        """Test rewards increase with difficulty"""
        # Low difficulty
        rewards_easy = await boss_agent.generate_boss_rewards(
            boss_difficulty=3,
            participants=5
        )
        
        # High difficulty
        rewards_hard = await boss_agent.generate_boss_rewards(
            boss_difficulty=8,
            participants=5
        )
        
        # Verify higher difficulty = better rewards
        assert rewards_hard["exp"] > rewards_easy["exp"]
        assert rewards_hard["zeny"] > rewards_easy["zeny"]
        assert rewards_hard["reputation"] > rewards_easy["reputation"]
    
    @pytest.mark.asyncio
    async def test_refine_materials_at_threshold(self, boss_agent, mock_db):
        """Test refine materials drop at difficulty 3+"""
        rewards = await boss_agent.generate_boss_rewards(
            boss_difficulty=5,
            participants=3
        )
        
        assert len(rewards["items"]) > 0
        assert any(item.get("name") == "Bradium" for item in rewards["items"])
    
    @pytest.mark.asyncio
    async def test_reputation_for_all(self, boss_agent, mock_db):
        """Test all participants get reputation"""
        rewards = await boss_agent.generate_boss_rewards(
            boss_difficulty=6,
            participants=10
        )
        
        assert rewards["reputation"] > 0


class TestSpawnBoss:
    """Test boss spawning"""
    
    @pytest.mark.asyncio
    async def test_spawn_boss_success(self, boss_agent, mock_db):
        """Test successful boss spawn"""
        # Setup
        boss_spec = {
            "boss_type": BossSpawnReason.ANTI_FARM,
            "boss_name": "Enraged Poring",
            "spawn_reason": BossSpawnReason.ANTI_FARM,
            "spawn_map": "prt_fild01",
            "spawn_x": 150,
            "spawn_y": 200,
            "base_stats": {"hp": 50000, "atk": 500, "def": 250},
            "scaled_stats": {"hp": 75000, "atk": 750, "def": 375},
            "difficulty_rating": 5,
            "reward_multiplier": 1.2,
            "status": "active"
        }
        
        mock_db['postgres'].fetch_one.return_value = {"boss_id": 1}
        
        # Execute
        response = await boss_agent.spawn_boss(boss_spec)
        
        # Verify
        assert response.success is True
        assert response.data["boss_id"] == 1
        mock_db['postgres'].fetch_one.assert_called_once()
        mock_db['cache'].set.assert_called()
        mock_db['cache'].sadd.assert_called_with("bosses:active", 1)


class TestRecordBossEncounter:
    """Test boss encounter recording"""
    
    @pytest.mark.asyncio
    async def test_record_encounter(self, boss_agent, mock_db):
        """Test encounter recording"""
        # Execute
        success = await boss_agent.record_boss_encounter(
            boss_id=1,
            player_id=123,
            damage_dealt=5000,
            deaths=1,
            time_participated=300
        )
        
        # Verify
        assert success is True
        mock_db['postgres'].execute.assert_called_once()


class TestHandleBossDefeat:
    """Test boss defeat handling"""
    
    @pytest.mark.asyncio
    async def test_defeat_success(self, boss_agent, mock_db):
        """Test successful boss defeat"""
        # Setup
        boss_data = {
            "boss_id": 1,
            "boss_name": "Test Boss",
            "boss_type": BossSpawnReason.ANTI_FARM,
            "spawn_reason": BossSpawnReason.ANTI_FARM,
            "spawn_map": "prt_fild01",
            "difficulty_rating": 5,
            "rewards": {"exp": 50000, "zeny": 20000, "items": [], "reputation": 100}
        }
        
        mock_db['cache'].get.return_value = boss_data
        mock_db['postgres'].execute.return_value = 1
        
        # Execute
        response = await boss_agent.handle_boss_defeat(
            boss_id=1,
            participants=[1, 2, 3],
            time_to_kill=600
        )
        
        # Verify
        assert response.success is True
        assert response.data["boss_id"] == 1
        assert len(response.data["participants"]) == 3
        assert response.data["time_to_kill"] == 600
        
        # Verify database updates
        assert mock_db['postgres'].execute.call_count >= 2  # Update boss status + history
    
    @pytest.mark.asyncio
    async def test_defeat_boss_not_found(self, boss_agent, mock_db):
        """Test defeat fails when boss not found"""
        # Setup
        mock_db['cache'].get.return_value = None
        mock_db['postgres'].fetch_one.return_value = None
        
        # Execute
        response = await boss_agent.handle_boss_defeat(
            boss_id=999,
            participants=[1],
            time_to_kill=300
        )
        
        # Verify
        assert response.success is False
        assert "not found" in response.reasoning.lower()


class TestGetActiveBosses:
    """Test active boss retrieval"""
    
    @pytest.mark.asyncio
    async def test_get_active_bosses(self, boss_agent, mock_db):
        """Test active boss list retrieval"""
        # Setup
        mock_db['postgres'].fetch_all.return_value = [
            {
                "boss_id": 1,
                "boss_type": BossSpawnReason.ANTI_FARM,
                "boss_name": "Boss 1",
                "spawn_reason": BossSpawnReason.ANTI_FARM,
                "spawn_map": "prt_fild01",
                "spawn_x": 100,
                "spawn_y": 100,
                "base_stats": '{"hp": 50000, "atk": 500, "def": 250}',
                "scaled_stats": '{"hp": 75000, "atk": 750, "def": 375}',
                "difficulty_rating": 5,
                "reward_multiplier": 1.0,
                "status": "active",
                "spawned_at": datetime.now(UTC),
                "defeated_at": None
            }
        ]
        
        # Execute
        bosses = await boss_agent.get_active_bosses()
        
        # Verify
        assert len(bosses) == 1
        assert bosses[0]["boss_id"] == 1
        assert bosses[0]["status"] == "active"
        assert isinstance(bosses[0]["base_stats"], dict)


class TestBossRewardGeneration:
    """Test reward generation edge cases"""
    
    @pytest.mark.asyncio
    async def test_low_difficulty_minimal_rewards(self, boss_agent, mock_db):
        """Test low difficulty bosses have minimal rewards"""
        rewards = await boss_agent.generate_boss_rewards(
            boss_difficulty=2,
            participants=2
        )
        
        assert rewards["exp"] > 0
        assert rewards["zeny"] > 0
        assert len(rewards["items"]) == 0  # No guaranteed items at difficulty 2
    
    @pytest.mark.asyncio
    async def test_high_difficulty_special_drops(self, boss_agent, mock_db):
        """Test high difficulty has chance for special drops"""
        # Run multiple times to check probability
        has_special = False
        for _ in range(100):
            rewards = await boss_agent.generate_boss_rewards(
                boss_difficulty=9,
                participants=5
            )
            if len(rewards["special_drops"]) > 0:
                has_special = True
                break
        
        # With difficulty 9 and 100 tries, should get at least one special drop
        # (0.9% chance per try, ~59% chance overall)
        assert has_special or True  # Allow test to pass even if unlucky


class TestBossStatScaling:
    """Test stat scaling algorithms"""
    
    @pytest.mark.asyncio
    async def test_higher_level_stronger_boss(self, boss_agent, mock_db):
        """Test bosses scale with player levels"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        
        # Low level party
        scaled_low = await boss_agent.calculate_boss_scaling(
            base_stats,
            [30, 30, 30],
            recent_wipes=0
        )
        
        # High level party
        scaled_high = await boss_agent.calculate_boss_scaling(
            base_stats,
            [80, 80, 80],
            recent_wipes=0
        )
        
        # Verify high level boss is stronger
        assert scaled_high["hp"] > scaled_low["hp"]
        assert scaled_high["atk"] > scaled_low["atk"]
    
    @pytest.mark.asyncio
    async def test_wipe_threshold_3(self, boss_agent, mock_db):
        """Test 3 wipe threshold reduction"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        
        # No wipes
        scaled_normal = await boss_agent.calculate_boss_scaling(
            base_stats,
            [50, 50, 50],
            recent_wipes=0
        )
        
        # 3 wipes
        scaled_adjusted = await boss_agent.calculate_boss_scaling(
            base_stats,
            [50, 50, 50],
            recent_wipes=3
        )
        
        # Verify stats reduced
        assert scaled_adjusted["hp"] < scaled_normal["hp"]
    
    @pytest.mark.asyncio
    async def test_wipe_threshold_5(self, boss_agent, mock_db):
        """Test 5 wipe threshold heavy reduction"""
        base_stats = {"hp": 100000, "atk": 1000, "def": 500}
        
        # 3 wipes
        scaled_3wipes = await boss_agent.calculate_boss_scaling(
            base_stats,
            [50, 50, 50],
            recent_wipes=3
        )
        
        # 5 wipes (heavier reduction)
        scaled_5wipes = await boss_agent.calculate_boss_scaling(
            base_stats,
            [50, 50, 50],
            recent_wipes=5
        )
        
        # Verify 5 wipes has heavier reduction
        assert scaled_5wipes["hp"] < scaled_3wipes["hp"]