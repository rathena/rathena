"""
Tests for Storyline Generator
"""

import pytest
import json
from datetime import datetime, UTC
from unittest.mock import AsyncMock, MagicMock, patch

from storyline.storyline_generator import StorylineGenerator, storyline_generator
from models.storyline import (
    WorldStateSnapshot, StoryArcSpec, LLMValidationResult,
    VillainEvolutionSpec
)


@pytest.fixture
def generator():
    """Create StorylineGenerator instance"""
    return StorylineGenerator()


@pytest.fixture
def mock_world_state():
    """Create mock world state for testing"""
    return WorldStateSnapshot(
        most_visited_maps=['prontera', 'geffen'],
        least_visited_maps=['abyss_03'],
        online_player_count=50,
        player_average_level=75,
        zeny_circulation=1000000,
        inflation_rate=0.05,
        global_karma=3500,
        dominant_faction='rune_alliance',
        faction_scores={'rune_alliance': 5000},
        random_seed="test123",
        timestamp=int(datetime.now(UTC).timestamp())
    )


@pytest.mark.asyncio
class TestStorylineGenerator:
    """Test StorylineGenerator functionality"""
    
    async def test_generator_initialization(self, generator):
        """Test generator initializes correctly"""
        assert generator.llm is not None
        assert generator.temperature == 0.8
        assert generator.max_retries == 3
    
    async def test_parse_llm_response_direct_json(self, generator):
        """Test parsing direct JSON response"""
        response_text = '{"test": "data", "value": 123}'
        
        result = generator._parse_llm_response(response_text)
        
        assert result is not None
        assert result['test'] == 'data'
        assert result['value'] == 123
    
    async def test_parse_llm_response_code_block(self, generator):
        """Test parsing JSON from code block"""
        response_text = '''Here is the JSON:
```json
{
    "test": "data",
    "value": 456
}
```
That's the response.'''
        
        result = generator._parse_llm_response(response_text)
        
        assert result is not None
        assert result['test'] == 'data'
        assert result['value'] == 456
    
    async def test_parse_llm_response_extraction(self, generator):
        """Test JSON extraction from text"""
        response_text = '''Some text before {"test": "data", "value": 789} and after'''
        
        result = generator._parse_llm_response(response_text)
        
        assert result is not None
        assert result['test'] == 'data'
        assert result['value'] == 789
    
    async def test_validate_story_output_valid(self, generator):
        """Test validation of valid story arc"""
        valid_arc_data = {
            "story_arc_name": "Test Arc",
            "story_arc_summary": "A test story arc for validation",
            "chapter": 1,
            "duration_days": 14,
            "events": [
                {
                    "npc_name": "Test NPC",
                    "npc_location": "prontera",
                    "npc_sprite": "4_M_JOB_KNIGHT1",
                    "npc_role": "protagonist",
                    "dialogue": ["Hello", "How are you?"]
                }
            ],
            "success_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Peace restored"]
            },
            "failure_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Chaos remains"]
            }
        }
        
        result = await generator.validate_story_output(valid_arc_data)
        
        assert isinstance(result, LLMValidationResult)
        assert result.is_valid
        assert result.sanitized_output is not None
        assert len(result.errors) == 0
    
    async def test_validate_story_output_invalid_duration(self, generator):
        """Test validation catches invalid duration"""
        invalid_arc_data = {
            "story_arc_name": "Test Arc",
            "story_arc_summary": "Invalid duration test",
            "chapter": 1,
            "duration_days": 50,  # Too long (max 30)
            "events": [],
            "success_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Change"]
            },
            "failure_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Change"]
            }
        }
        
        result = await generator.validate_story_output(invalid_arc_data)
        
        assert not result.is_valid
        assert len(result.errors) > 0
    
    async def test_validate_story_output_no_events(self, generator):
        """Test validation catches missing events"""
        no_events_data = {
            "story_arc_name": "Test Arc",
            "story_arc_summary": "No events test",
            "chapter": 1,
            "duration_days": 14,
            "events": [],  # Empty
            "success_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Change"]
            },
            "failure_outcomes": {
                "next_chapter": 2,
                "world_changes": ["Change"]
            }
        }
        
        result = await generator.validate_story_output(no_events_data)
        
        assert not result.is_valid
        assert "No events defined" in result.errors
    
    async def test_cache_key_generation(self, generator, mock_world_state):
        """Test cache key generation"""
        previous_arcs = [
            {'arc_id': 1, 'arc_name': 'Previous Arc'}
        ]
        
        key1 = generator._generate_cache_key(mock_world_state, previous_arcs)
        key2 = generator._generate_cache_key(mock_world_state, previous_arcs)
        
        # Same inputs should generate same key
        assert key1 == key2
        assert key1.startswith("storyline:arc:cache:")
        assert len(key1) > 30  # Has hash component
    
    async def test_template_fallback_generation(self, generator, mock_world_state):
        """Test template fallback works"""
        arc = await generator._generate_template_fallback(mock_world_state)
        
        assert isinstance(arc, StoryArcSpec)
        assert arc.story_arc_name is not None
        assert arc.duration_days >= 7
        assert len(arc.events) > 0
    
    async def test_count_data_points(self, generator, mock_world_state):
        """Test counting data points in world state"""
        count = generator._count_data_points(mock_world_state)
        
        assert count > 0
        assert isinstance(count, int)


@pytest.mark.asyncio
async def test_global_instance():
    """Test global storyline_generator instance exists"""
    assert storyline_generator is not None
    assert isinstance(storyline_generator, StorylineGenerator)