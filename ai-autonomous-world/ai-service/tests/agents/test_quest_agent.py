"""
Tests for QuestAgent

Tests:
- Quest generation (8 types)
- Difficulty scaling (6 levels)
- Objective generation
- Reward calculations
- Requirement determination
- Quest type selection
- Player level adaptation
- Time limits
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, patch

from agents.quest_agent import QuestAgent, Quest, QuestType, QuestDifficulty
from agents.base_agent import AgentContext, AgentResponse, AgentStatus
from models.npc import NPCState


@pytest.fixture
def quest_agent():
    """Create quest agent instance"""
    return QuestAgent(
        min_reward_multiplier=1.0,
        max_reward_multiplier=5.0
    )


@pytest.fixture
def quest_context(sample_agent_context):
    """Create context for quest generation"""
    sample_agent_context.additional_data = {
        "operation": "generate",
        "player_level": 10
    }
    return sample_agent_context


@pytest.mark.unit
class TestQuestAgentInitialization:
    """Test quest agent initialization"""
    
    def test_basic_initialization(self):
        """Test basic quest agent initialization"""
        agent = QuestAgent()
        
        assert agent.name == "QuestAgent"
        assert agent.min_reward_multiplier == 1.0
        assert agent.max_reward_multiplier == 5.0
        assert len(agent._quest_templates) > 0
    
    def test_quest_templates_complete(self):
        """Test all quest types have templates"""
        agent = QuestAgent()
        
        for quest_type in QuestType:
            assert quest_type in agent._quest_templates
            assert "titles" in agent._quest_templates[quest_type]
            assert "objectives" in agent._quest_templates[quest_type]


@pytest.mark.unit
@pytest.mark.asyncio
class TestQuestGeneration:
    """Test quest generation"""
    
    async def test_generate_basic_quest(self, quest_agent, quest_context):
        """Test generating a basic quest"""
        await quest_agent.initialize()
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert "quest_id" in response.result
        assert "title" in response.result
        assert "description" in response.result
        assert "objectives" in response.result
        assert "rewards" in response.result
    
    async def test_generate_with_specific_type(self, quest_agent, quest_context):
        """Test generating quest with specific type"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "kill"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["type"] == "kill"
    
    async def test_generate_with_specific_difficulty(self, quest_agent, quest_context):
        """Test generating quest with specific difficulty"""
        await quest_agent.initialize()
        
        quest_context.additional_data["difficulty"] = "hard"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["difficulty"] == "hard"
    
    @pytest.mark.parametrize("quest_type", [
        "kill", "gather", "escort", "delivery", 
        "discovery", "defense", "rescue", "craft"
    ])
    async def test_generate_all_quest_types(self, quest_agent, quest_context, quest_type):
        """Test generating all 8 quest types"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = quest_type
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["type"] == quest_type


@pytest.mark.unit
@pytest.mark.asyncio
class TestDifficultyScaling:
    """Test difficulty scaling"""
    
    @pytest.mark.parametrize("difficulty", [
        "trivial", "easy", "normal", "hard", "elite", "legendary"
    ])
    async def test_all_difficulty_levels(self, quest_agent, quest_context, difficulty):
        """Test all 6 difficulty levels"""
        await quest_agent.initialize()
        
        quest_context.additional_data["difficulty"] = difficulty
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["difficulty"] == difficulty
    
    async def test_difficulty_for_low_level(self, quest_agent, quest_context):
        """Test difficulty calculation for low level player"""
        await quest_agent.initialize()
        
        quest_context.additional_data["player_level"] = 3
        quest_context.additional_data.pop("difficulty", None)
        
        response = await quest_agent.execute(quest_context)
        
        # Should generate easy quests for low level
        assert response.result["difficulty"] in ["trivial", "easy", "normal"]
    
    async def test_difficulty_for_high_level(self, quest_agent, quest_context):
        """Test difficulty calculation for high level player"""
        await quest_agent.initialize()
        
        quest_context.additional_data["player_level"] = 75
        quest_context.additional_data.pop("difficulty", None)
        
        response = await quest_agent.execute(quest_context)
        
        # Should generate harder quests for high level
        assert response.result["difficulty"] in ["hard", "elite", "legendary"]


@pytest.mark.unit
@pytest.mark.asyncio
class TestRewardCalculation:
    """Test reward calculations"""
    
    async def test_rewards_scale_with_level(self, quest_agent, quest_context):
        """Test rewards scale with player level"""
        await quest_agent.initialize()
        
        # Low level
        quest_context.additional_data["player_level"] = 5
        response_low = await quest_agent.execute(quest_context)
        
        # High level
        quest_context.additional_data["player_level"] = 50
        response_high = await quest_agent.execute(quest_context)
        
        # High level should have better rewards
        assert response_high.result["rewards"]["experience"] > response_low.result["rewards"]["experience"]
        assert response_high.result["rewards"]["gold"] > response_low.result["rewards"]["gold"]
    
    async def test_rewards_scale_with_difficulty(self, quest_agent, quest_context):
        """Test rewards scale with difficulty"""
        await quest_agent.initialize()
        
        # Easy quest
        quest_context.additional_data["difficulty"] = "easy"
        quest_context.additional_data["player_level"] = 10
        response_easy = await quest_agent.execute(quest_context)
        
        # Legendary quest
        quest_context.additional_data["difficulty"] = "legendary"
        response_hard = await quest_agent.execute(quest_context)
        
        # Harder difficulty should have better rewards
        assert response_hard.result["rewards"]["experience"] > response_easy.result["rewards"]["experience"]
        assert response_hard.result["rewards"]["gold"] > response_easy.result["rewards"]["gold"]
    
    async def test_hard_quests_have_item_rewards(self, quest_agent, quest_context):
        """Test hard quests provide item rewards"""
        await quest_agent.initialize()
        
        quest_context.additional_data["difficulty"] = "elite"
        
        response = await quest_agent.execute(quest_context)
        
        assert len(response.result["rewards"]["items"]) > 0


@pytest.mark.unit
@pytest.mark.asyncio
class TestObjectiveGeneration:
    """Test objective generation"""
    
    async def test_kill_objectives(self, quest_agent, quest_context):
        """Test kill quest objectives"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "kill"
        
        response = await quest_agent.execute(quest_context)
        
        objectives = response.result["objectives"]
        assert len(objectives) > 0
        assert objectives[0]["type"] == "kill"
        assert "target" in objectives[0]
        assert "count" in objectives[0]
    
    async def test_gather_objectives(self, quest_agent, quest_context):
        """Test gather quest objectives"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "gather"
        
        response = await quest_agent.execute(quest_context)
        
        objectives = response.result["objectives"]
        assert objectives[0]["type"] == "gather"
        assert "item" in objectives[0]
        assert "count" in objectives[0]
    
    async def test_escort_objectives(self, quest_agent, quest_context):
        """Test escort quest objectives"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "escort"
        
        response = await quest_agent.execute(quest_context)
        
        objectives = response.result["objectives"]
        assert objectives[0]["type"] == "escort"
        assert "npc" in objectives[0]
        assert "destination" in objectives[0]
    
    async def test_defense_objectives(self, quest_agent, quest_context):
        """Test defense quest objectives"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "defense"
        
        response = await quest_agent.execute(quest_context)
        
        objectives = response.result["objectives"]
        assert objectives[0]["type"] == "defend"
        assert "waves" in objectives[0]
    
    async def test_objective_count_scales(self, quest_agent, quest_context):
        """Test objective count scales with difficulty"""
        await quest_agent.initialize()
        
        # Easy quest
        quest_context.additional_data["quest_type"] = "kill"
        quest_context.additional_data["difficulty"] = "trivial"
        response_easy = await quest_agent.execute(quest_context)
        
        # Hard quest
        quest_context.additional_data["difficulty"] = "legendary"
        response_hard = await quest_agent.execute(quest_context)
        
        # Harder should have more objectives
        assert response_hard.result["objectives"][0]["count"] > response_easy.result["objectives"][0]["count"]


@pytest.mark.unit
@pytest.mark.asyncio
class TestQuestRequirements:
    """Test quest requirements"""
    
    async def test_level_requirements(self, quest_agent, quest_context):
        """Test level requirements are set"""
        await quest_agent.initialize()
        
        quest_context.additional_data["player_level"] = 10
        quest_context.additional_data["difficulty"] = "hard"
        
        response = await quest_agent.execute(quest_context)
        
        requirements = response.result["requirements"]
        assert "min_level" in requirements
        # Hard quests should require higher level
        assert requirements["min_level"] >= 10
    
    async def test_elite_requires_high_level(self, quest_agent, quest_context):
        """Test elite quests require high level"""
        await quest_agent.initialize()
        
        quest_context.additional_data["player_level"] = 20
        quest_context.additional_data["difficulty"] = "elite"
        
        response = await quest_agent.execute(quest_context)
        
        # Elite should require significantly higher level
        assert response.result["requirements"]["min_level"] >= 25


@pytest.mark.unit
@pytest.mark.asyncio
class TestTimeLimits:
    """Test time limit calculations"""
    
    async def test_escort_has_time_limit(self, quest_agent, quest_context):
        """Test escort quests have time limits"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "escort"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.result["time_limit_minutes"] is not None
        assert response.result["time_limit_minutes"] > 0
    
    async def test_delivery_has_time_limit(self, quest_agent, quest_context):
        """Test delivery quests have time limits"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "delivery"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.result["time_limit_minutes"] is not None
    
    async def test_defense_has_time_limit(self, quest_agent, quest_context):
        """Test defense quests have time limits"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "defense"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.result["time_limit_minutes"] is not None
    
    async def test_kill_no_time_limit(self, quest_agent, quest_context):
        """Test kill quests don't have time limits"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "kill"
        
        response = await quest_agent.execute(quest_context)
        
        # Kill quests typically don't have time limits
        assert response.result["time_limit_minutes"] is None


@pytest.mark.unit
class TestQuestTypeSelection:
    """Test quest type selection logic"""
    
    def test_select_quest_for_merchant(self, quest_agent, sample_agent_context):
        """Test quest selection for merchant NPC"""
        npc = MagicMock()
        npc.background.occupation = "Merchant"
        sample_agent_context.npc_state = npc
        
        quest_type = quest_agent._select_quest_type(sample_agent_context)
        
        # Merchants should offer trade-related quests
        assert quest_type in [QuestType.DELIVERY, QuestType.GATHER, QuestType.ESCORT]
    
    def test_select_quest_for_guard(self, quest_agent, sample_agent_context):
        """Test quest selection for guard NPC"""
        npc = MagicMock()
        npc.background.occupation = "Town Guard"
        sample_agent_context.npc_state = npc
        
        quest_type = quest_agent._select_quest_type(sample_agent_context)
        
        # Guards should offer combat quests
        assert quest_type in [QuestType.DEFENSE, QuestType.KILL, QuestType.ESCORT]
    
    def test_select_quest_for_smith(self, quest_agent, sample_agent_context):
        """Test quest selection for smith NPC"""
        npc = MagicMock()
        npc.background.occupation = "Blacksmith"
        sample_agent_context.npc_state = npc
        
        quest_type = quest_agent._select_quest_type(sample_agent_context)
        
        # Smiths should offer craft quests
        assert quest_type in [QuestType.CRAFT, QuestType.GATHER]


@pytest.mark.unit
class TestQuestClass:
    """Test Quest class"""
    
    def test_quest_creation(self):
        """Test creating quest object"""
        quest = Quest(
            quest_id="test_1",
            title="Test Quest",
            description="A test quest",
            quest_type=QuestType.KILL,
            difficulty=QuestDifficulty.NORMAL,
            objectives=[{"type": "kill", "target": "Goblin", "count": 5}],
            rewards={"experience": 100, "gold": 50},
            requirements={"min_level": 5}
        )
        
        assert quest.quest_id == "test_1"
        assert quest.quest_type == QuestType.KILL
        assert quest.difficulty == QuestDifficulty.NORMAL


@pytest.mark.unit
@pytest.mark.asyncio
class TestErrorHandling:
    """Test error handling"""
    
    async def test_invalid_operation(self, quest_agent, quest_context):
        """Test handling invalid operation"""
        await quest_agent.initialize()
        
        quest_context.additional_data["operation"] = "invalid_op"
        
        response = await quest_agent.execute(quest_context)
        
        assert response.status == AgentStatus.FAILED
        assert response.error is not None
    
    async def test_invalid_quest_type(self, quest_agent, quest_context):
        """Test handling invalid quest type"""
        await quest_agent.initialize()
        
        quest_context.additional_data["quest_type"] = "invalid_type"
        
        # Should fail or fallback
        try:
            response = await quest_agent.execute(quest_context)
            # May succeed by falling back to random selection
            assert response.status in [AgentStatus.COMPLETED, AgentStatus.FAILED]
        except ValueError:
            # Acceptable to raise ValueError
            pass


@pytest.mark.unit
@pytest.mark.asyncio
class TestCleanup:
    """Test cleanup procedures"""
    
    async def test_cleanup(self):
        """Test cleanup runs without error"""
        agent = QuestAgent()
        await agent.initialize()
        await agent.cleanup()
        
        assert not agent.is_initialized