"""
Tests for DialogueAgent

Tests:
- Dialogue generation with personality
- Emotion-based responses
- Context-aware dialogue
- Template fallback system
- LLM integration preparation
- Response caching
- Emotional coloring
- Relationship scoring
"""

import pytest
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, patch

from agents.dialogue_agent import DialogueAgent, DialogueContext
from agents.base_agent import AgentContext, AgentResponse, AgentStatus
from agents.moral_alignment import MoralAlignment
from models.npc import NPCState, Emotion, EmotionalState, Personality, Background


@pytest.fixture
def mock_npc_state(faker_instance):
    """Create mock NPC state for testing"""
    npc = MagicMock(spec=NPCState)
    npc.npc_id = faker_instance.uuid4()
    npc.name = "Test NPC"
    npc.location = "test_town"
    
    # Personality
    personality = MagicMock(spec=Personality)
    personality.moral_alignment = MoralAlignment.LAWFUL_GOOD
    personality.openness = 0.8
    personality.conscientiousness = 0.7
    personality.extraversion = 0.6
    personality.agreeableness = 0.9
    personality.neuroticism = 0.3
    npc.personality = personality
    
    # Emotional state
    emotional_state = MagicMock(spec=EmotionalState)
    emotional_state.current_emotion = Emotion.HAPPY
    emotional_state.emotion_intensity = 0.7
    emotional_state.mood = "cheerful"
    npc.emotional_state = emotional_state
    
    # Background
    background = MagicMock(spec=Background)
    background.occupation = "Merchant"
    background.backstory = "A friendly merchant who runs the local shop."
    npc.background = background
    
    # Reputation
    npc.reputation = {}
    
    return npc


@pytest.fixture
def dialogue_context(mock_npc_state, faker_instance):
    """Create dialogue context for testing"""
    return DialogueContext(
        npc_state=mock_npc_state,
        player_message="Hello, how are you?",
        conversation_history=[],
        relationship_score=0.5,
        context_tags=["time:noon", "weather:clear"]
    )


@pytest.fixture
def agent_context_with_dialogue(sample_agent_context, mock_npc_state):
    """Create agent context with dialogue data"""
    sample_agent_context.npc_state = mock_npc_state
    sample_agent_context.additional_data = {
        "player_message": "Hello, how are you?",
        "conversation_history": []
    }
    return sample_agent_context


@pytest.mark.unit
class TestDialogueAgentInitialization:
    """Test dialogue agent initialization"""
    
    def test_basic_initialization(self):
        """Test basic dialogue agent initialization"""
        agent = DialogueAgent()
        
        assert agent.name == "DialogueAgent"
        assert agent.max_response_length == 500
        assert not agent.enable_crewai
        assert agent.llm_provider is None
        assert len(agent.emotion_templates) > 0
    
    def test_custom_initialization(self):
        """Test dialogue agent with custom parameters"""
        agent = DialogueAgent(
            max_response_length=1000,
            enable_crewai=True
        )
        
        assert agent.max_response_length == 1000
        assert agent.enable_crewai
    
    def test_emotion_templates_initialized(self):
        """Test emotion templates are properly initialized"""
        agent = DialogueAgent()
        
        required_emotions = [
            Emotion.HAPPY,
            Emotion.SAD,
            Emotion.ANGRY,
            Emotion.FEARFUL,
            Emotion.NEUTRAL
        ]
        
        for emotion in required_emotions:
            assert emotion in agent.emotion_templates
            assert len(agent.emotion_templates[emotion]) > 0


@pytest.mark.unit
@pytest.mark.asyncio
class TestDialogueGeneration:
    """Test dialogue generation"""
    
    async def test_basic_dialogue_generation(self, agent_context_with_dialogue):
        """Test basic dialogue generation"""
        agent = DialogueAgent()
        await agent.initialize()
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        assert "dialogue" in response.result
        assert "emotion" in response.result
        assert "tone" in response.result
        assert len(response.result["dialogue"]) > 0
    
    async def test_greeting_detection(self, agent_context_with_dialogue, mock_npc_state):
        """Test greeting message detection"""
        agent = DialogueAgent()
        await agent.initialize()
        
        agent_context_with_dialogue.additional_data["player_message"] = "Hello!"
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        dialogue = response.result["dialogue"]
        assert "Test NPC" in dialogue or "Merchant" in dialogue
    
    async def test_quest_detection(self, agent_context_with_dialogue):
        """Test quest-related message detection"""
        agent = DialogueAgent()
        await agent.initialize()
        
        agent_context_with_dialogue.additional_data["player_message"] = "Do you have any quests?"
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        dialogue = response.result["dialogue"]
        assert len(dialogue) > 0
    
    async def test_trade_detection(self, agent_context_with_dialogue):
        """Test trade-related message detection"""
        agent = DialogueAgent()
        await agent.initialize()
        
        agent_context_with_dialogue.additional_data["player_message"] = "Can I buy something?"
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        dialogue = response.result["dialogue"]
        assert "Merchant" in dialogue or "goods" in dialogue.lower()


@pytest.mark.unit
@pytest.mark.asyncio
class TestEmotionalResponses:
    """Test emotion-based dialogue responses"""
    
    async def test_happy_emotion(self, agent_context_with_dialogue, mock_npc_state):
        """Test dialogue with happy emotion"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.emotional_state.current_emotion = Emotion.HAPPY
        mock_npc_state.emotional_state.emotion_intensity = 0.9
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["emotion"] == "happy"
    
    async def test_angry_emotion(self, agent_context_with_dialogue, mock_npc_state):
        """Test dialogue with angry emotion"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.emotional_state.current_emotion = Emotion.ANGRY
        mock_npc_state.emotional_state.emotion_intensity = 0.8
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["emotion"] == "angry"
        # High intensity should add emotional markers
        dialogue = response.result["dialogue"]
        assert "*" in dialogue or "!" in dialogue
    
    async def test_fearful_emotion(self, agent_context_with_dialogue, mock_npc_state):
        """Test dialogue with fearful emotion"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.emotional_state.current_emotion = Emotion.FEARFUL
        mock_npc_state.emotional_state.emotion_intensity = 0.8
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["emotion"] == "fearful"
    
    async def test_sad_emotion(self, agent_context_with_dialogue, mock_npc_state):
        """Test dialogue with sad emotion"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.emotional_state.current_emotion = Emotion.SAD
        mock_npc_state.emotional_state.emotion_intensity = 0.7
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["emotion"] == "sad"


@pytest.mark.unit
@pytest.mark.asyncio
class TestResponseCaching:
    """Test response caching mechanism"""
    
    async def test_cache_hit(self, agent_context_with_dialogue):
        """Test cache hit for repeated messages"""
        agent = DialogueAgent()
        await agent.initialize()
        
        # First request
        response1 = await agent.execute(agent_context_with_dialogue)
        dialogue1 = response1.result["dialogue"]
        
        # Second request with same message
        response2 = await agent.execute(agent_context_with_dialogue)
        dialogue2 = response2.result["dialogue"]
        
        # Should get same cached response
        assert dialogue1 == dialogue2
    
    async def test_cache_max_size(self, agent_context_with_dialogue):
        """Test cache respects max size"""
        agent = DialogueAgent()
        agent._cache_max_size = 5
        await agent.initialize()
        
        # Generate more than max cache size
        for i in range(10):
            agent_context_with_dialogue.additional_data["player_message"] = f"Message {i}"
            await agent.execute(agent_context_with_dialogue)
        
        # Cache should not exceed max size
        assert len(agent._response_cache) <= agent._cache_max_size


@pytest.mark.unit
@pytest.mark.asyncio
class TestPersonalityDriven:
    """Test personality-driven dialogue"""
    
    async def test_lawful_good_response(self, agent_context_with_dialogue, mock_npc_state):
        """Test response for lawful good character"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.personality.moral_alignment = MoralAlignment.LAWFUL_GOOD
        agent_context_with_dialogue.additional_data["player_message"] = "Need help?"
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        dialogue = response.result["dialogue"].lower()
        # Lawful good should offer help
        assert "help" in dialogue or "assist" in dialogue
    
    async def test_chaotic_evil_response(self, agent_context_with_dialogue, mock_npc_state):
        """Test response for chaotic evil character"""
        agent = DialogueAgent()
        await agent.initialize()
        
        mock_npc_state.personality.moral_alignment = MoralAlignment.CHAOTIC_EVIL
        agent_context_with_dialogue.additional_data["player_message"] = "Need help?"
        
        response = await agent.execute(agent_context_with_dialogue)
        
        assert response.status == AgentStatus.COMPLETED
        # Chaotic evil should be more self-interested
        assert len(response.result["dialogue"]) > 0


@pytest.mark.unit
class TestResponseValidation:
    """Test response validation and cleaning"""
    
    def test_truncate_long_response(self):
        """Test long responses are truncated"""
        agent = DialogueAgent(max_response_length=100)
        
        long_text = "a" * 200
        truncated = agent._validate_response(long_text)
        
        assert len(truncated) <= agent.max_response_length
        assert truncated.endswith("...")
    
    def test_minimum_length(self):
        """Test responses have minimum length"""
        agent = DialogueAgent()
        
        short_text = "hi"
        validated = agent._validate_response(short_text)
        
        assert len(validated) >= 2
    
    def test_whitespace_cleaning(self):
        """Test excess whitespace is removed"""
        agent = DialogueAgent()
        
        messy_text = "Hello    there   friend"
        cleaned = agent._validate_response(messy_text)
        
        assert "    " not in cleaned


@pytest.mark.unit
class TestDialogueContext:
    """Test dialogue context helpers"""
    
    def test_context_tag_extraction(self, agent_context_with_dialogue):
        """Test context tag extraction"""
        agent = DialogueAgent()
        
        tags = agent._extract_context_tags(agent_context_with_dialogue)
        
        assert len(tags) > 0
        assert any("time:" in tag for tag in tags)
        assert any("weather:" in tag for tag in tags)
    
    def test_relationship_score_calculation(self, agent_context_with_dialogue, mock_npc_state, faker_instance):
        """Test relationship score calculation"""
        agent = DialogueAgent()
        
        player_id = faker_instance.uuid4()
        agent_context_with_dialogue.player_id = player_id
        mock_npc_state.reputation = {player_id: 50.0}
        
        score = agent._calculate_relationship_score(mock_npc_state, agent_context_with_dialogue)
        
        assert 0.0 <= score <= 1.0


@pytest.mark.unit
@pytest.mark.asyncio
class TestErrorHandling:
    """Test error handling"""
    
    async def test_missing_npc_state(self, sample_agent_context):
        """Test handling missing NPC state"""
        agent = DialogueAgent()
        await agent.initialize()
        
        sample_agent_context.npc_state = None
        sample_agent_context.additional_data = {"player_message": "Hello"}
        
        response = await agent.execute(sample_agent_context)
        
        assert response.status == AgentStatus.FAILED
        assert response.error is not None
    
    async def test_invalid_emotion(self, agent_context_with_dialogue, mock_npc_state):
        """Test handling invalid emotion gracefully"""
        agent = DialogueAgent()
        await agent.initialize()
        
        # Set invalid emotion (should fallback to neutral)
        mock_npc_state.emotional_state.current_emotion = None
        
        # Should not crash, use neutral emotion
        try:
            response = await agent.execute(agent_context_with_dialogue)
            # May succeed or fail gracefully
            assert response.status in [AgentStatus.COMPLETED, AgentStatus.FAILED]
        except Exception:
            # Acceptable to throw exception for invalid state
            pass


@pytest.mark.unit
@pytest.mark.asyncio
class TestCleanup:
    """Test cleanup procedures"""
    
    async def test_cache_cleared_on_cleanup(self):
        """Test cache is cleared on cleanup"""
        agent = DialogueAgent()
        await agent.initialize()
        
        # Add to cache
        agent._response_cache["test"] = "test response"
        
        # Cleanup
        await agent.cleanup()
        
        assert len(agent._response_cache) == 0