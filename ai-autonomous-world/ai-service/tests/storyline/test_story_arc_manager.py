"""
Tests for Story Arc Manager
"""

import pytest
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from storyline.story_arc_manager import StoryArcManager, story_arc_manager
from models.storyline import (
    StoryArcSpec, StoryEventNPC, StoryQuest, StoryQuestReward,
    StoryOutcome, NPCRole, ChapterOutcome, StoryArcStatus
)


@pytest.fixture
def manager():
    """Create StoryArcManager instance"""
    return StoryArcManager()


@pytest.fixture
def mock_arc_spec():
    """Create mock story arc specification"""
    return StoryArcSpec(
        story_arc_name="Test Adventure",
        story_arc_summary="A test story arc for unit testing",
        chapter=1,
        duration_days=14,
        theme="mystery",
        events=[
            StoryEventNPC(
                npc_name="Test Guard",
                npc_location="prontera",
                npc_sprite="4_M_JOB_KNIGHT1",
                npc_role=NPCRole.PROTAGONIST,
                dialogue=["Greetings!", "We need your help."],
                quest=StoryQuest(
                    title="Test Quest",
                    objective="Defeat 10 monsters",
                    reward=StoryQuestReward(exp=10000, items=[], reputation=50)
                )
            )
        ],
        success_outcomes=StoryOutcome(
            next_chapter=2,
            world_changes=["Peace restored"]
        ),
        failure_outcomes=StoryOutcome(
            next_chapter=2,
            world_changes=["Darkness grows"]
        )
    )


@pytest.mark.asyncio
class TestStoryArcManager:
    """Test StoryArcManager functionality"""
    
    async def test_manager_initialization(self, manager):
        """Test manager initializes correctly"""
        assert manager.current_arc_id is None
        assert manager.auto_advance is True
        assert manager.hero_selection_top_n >= 1
    
    async def test_build_outcome_summary(self, manager):
        """Test outcome summary building"""
        metrics = {
            'total_participants': 50,
            'chapters_completed': 3,
            'success_rate': 0.85
        }
        
        summary = manager._build_outcome_summary(
            ChapterOutcome.SUCCESS,
            metrics
        )
        
        assert "success" in summary.lower()
        assert "50" in summary
        assert "3" in summary
        assert "85" in summary or "0.85" in summary
    
    async def test_extract_notable_choices(self, manager):
        """Test extracting notable choices from JSON"""
        choices_json = [
            {'choice_type': 'dialogue', 'impact_score': 60},
            {'choice_type': 'faction', 'impact_score': 80},
            {'choice_type': 'quest', 'impact_score': 30}
        ]
        
        notable = manager._extract_notable_choices(choices_json)
        
        assert len(notable) <= 3
        assert 'dialogue' in notable
        assert 'faction' in notable
        # quest should be excluded (impact_score < 50)
    
    async def test_extract_notable_choices_empty(self, manager):
        """Test extracting from empty choices"""
        notable = manager._extract_notable_choices(None)
        assert notable == []
        
        notable = manager._extract_notable_choices([])
        assert notable == []
    
    async def test_extract_notable_choices_from_json_string(self, manager):
        """Test extracting from JSON string"""
        import json
        choices_json = json.dumps([
            {'choice_type': 'test', 'impact_score': 100}
        ])
        
        notable = manager._extract_notable_choices(choices_json)
        
        assert len(notable) == 1
        assert 'test' in notable


@pytest.mark.asyncio
async def test_global_instance():
    """Test global story_arc_manager instance exists"""
    assert story_arc_manager is not None
    assert isinstance(story_arc_manager, StoryArcManager)