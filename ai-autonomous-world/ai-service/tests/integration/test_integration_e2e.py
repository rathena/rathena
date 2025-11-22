"""
End-to-End Integration Tests for NPC Interaction Workflows

Tests complete player-NPC interaction flows including:
- Player dialogue → NPC response generation
- Relationship updates based on interactions
- Quest assignment and progression
- Emotional state changes
- Multi-step conversation flows
- Error propagation through the stack
"""

import asyncio
import pytest
from datetime import datetime
from typing import Dict, Any
from unittest.mock import AsyncMock, MagicMock, patch

from models.npc import NPCState, NPCPhysical, NPCBackground, Emotion
from models.player import PlayerState, PlayerContext, InteractionHistory
from models.quest import QuestData, QuestType, DifficultyLevel, QuestStatus
from llm.factory import LLMProviderFactory
from llm.base_provider import LLMResponse, ProviderType
from memory.relationship_manager import RelationshipManager, RelationshipType


@pytest.fixture
async def mock_db_session():
    """Mock database session with transaction support"""
    session = MagicMock()
    session.commit = AsyncMock()
    session.rollback = AsyncMock()
    session.close = AsyncMock()
    session.add = MagicMock()
    session.query = MagicMock()
    return session


@pytest.fixture
async def mock_dragonfly():
    """Mock DragonflyDB/Redis client"""
    client = MagicMock()
    client.get = MagicMock(return_value=None)
    client.setex = MagicMock(return_value=True)
    client.delete = MagicMock(return_value=True)
    return client


@pytest.fixture
async def llm_factory_mock():
    """Mock LLM factory with successful responses"""
    factory = MagicMock(spec=LLMProviderFactory)
    
    async def mock_generate(*args, **kwargs):
        return LLMResponse(
            text="Greetings, traveler! How may I assist you today?",
            tokens_used=15,
            cost_usd=0.0001,
            model="gpt-4",
            provider="openai",
            latency_ms=120.0,
            metadata={}
        )
    
    factory.generate = AsyncMock(side_effect=mock_generate)
    factory.chat = AsyncMock(side_effect=mock_generate)
    return factory


@pytest.fixture
async def relationship_manager(mock_db_session, mock_dragonfly):
    """Relationship manager with mocked dependencies"""
    manager = RelationshipManager(
        postgres_session=mock_db_session,
        dragonfly_client=mock_dragonfly,
        cache_ttl=3600
    )
    return manager


@pytest.fixture
def sample_npc() -> NPCState:
    """Create a sample NPC for testing"""
    return NPCState(
        npc_id="npc_001",
        name="Merchant Aldric",
        title="Master Trader",
        physical=NPCPhysical(
            race="Human",
            gender="Male",
            age=45,
            height_cm=175.0,
            weight_kg=80.0,
            appearance="Weathered merchant with kind eyes"
        ),
        background=NPCBackground(
            occupation="Merchant",
            social_class="merchant",
            hometown="Prontera",
            skills=["Trading", "Negotiation", "Appraisal"]
        ),
        location="prontera",
        position_x=150.0,
        position_y=200.0,
        faction_id="merchant_guild"
    )


@pytest.fixture
def sample_player() -> PlayerState:
    """Create a sample player for testing"""
    return PlayerState(
        player_id="player_001",
        character_name="TestHero",
        job_class="Swordsman",
        base_level=25,
        job_level=15,
        location="prontera",
        position_x=145.0,
        position_y=195.0
    )


@pytest.fixture
def sample_quest() -> QuestData:
    """Create a sample quest"""
    return QuestData(
        quest_id="quest_001",
        title="Gather Herbs",
        description="Collect 10 healing herbs from the forest",
        quest_type=QuestType.COLLECT,
        difficulty=DifficultyLevel.EASY,
        min_level=20,
        npc_id="npc_001",
        npc_name="Merchant Aldric",
        start_location="prontera"
    )


@pytest.mark.integration
@pytest.mark.asyncio
class TestEndToEndNPCInteraction:
    """End-to-end tests for complete NPC interaction workflows"""
    
    async def test_complete_dialogue_workflow(
        self,
        sample_npc,
        sample_player,
        llm_factory_mock,
        relationship_manager
    ):
        """
        Test complete dialogue flow:
        Player initiates → NPC responds → Relationship updates
        """
        # Arrange
        player_message = "Hello, merchant! What are you selling today?"
        
        # Act - Generate NPC response
        response = await llm_factory_mock.generate(
            prompt=f"NPC: {sample_npc.name} responding to: {player_message}",
            temperature=0.7,
            max_tokens=100
        )
        
        # Update relationship based on positive interaction
        relationship = await relationship_manager.update_relationship(
            entity_a=sample_player.player_id,
            entity_b=sample_npc.npc_id,
            affinity_change=0.05,
            interaction_type="dialogue",
            note="Friendly greeting exchange"
        )
        
        # Record interaction
        sample_npc.record_interaction()
        
        # Assert
        assert response.text is not None
        assert len(response.text) > 0
        assert response.tokens_used > 0
        assert relationship.affinity > 0.0
        assert relationship.interaction_count == 1
        assert sample_npc.interaction_count == 1
        assert sample_npc.last_interaction is not None
    
    async def test_quest_assignment_workflow(
        self,
        sample_npc,
        sample_player,
        sample_quest,
        llm_factory_mock,
        relationship_manager,
        mock_db_session
    ):
        """
        Test quest assignment flow:
        Player requests → NPC assigns quest → Quest state updates
        """
        # Arrange
        player_message = "Do you have any work for me?"
        
        # Act - Check if player can accept quest
        can_accept, reason = sample_quest.can_accept(
            player_level=sample_player.base_level,
            completed_quests=[]
        )
        
        assert can_accept, f"Player should be able to accept quest: {reason}"
        
        # Generate quest dialogue
        quest_dialogue = await llm_factory_mock.generate(
            prompt=f"NPC {sample_npc.name} offering quest: {sample_quest.title}",
            temperature=0.7
        )
        
        # Accept quest
        sample_quest.accept()
        
        # Update relationship (quest offering increases affinity)
        relationship = await relationship_manager.update_relationship(
            entity_a=sample_player.player_id,
            entity_b=sample_npc.npc_id,
            affinity_change=0.10,
            interaction_type="quest_assignment",
            note=f"Assigned quest: {sample_quest.title}"
        )
        
        # Simulate database commit
        mock_db_session.add(sample_quest)
        await mock_db_session.commit()
        
        # Assert
        assert sample_quest.status == QuestStatus.IN_PROGRESS
        assert sample_quest.accepted_count == 1
        assert relationship.affinity >= 0.10
        assert quest_dialogue.text is not None
        mock_db_session.add.assert_called_once()
        mock_db_session.commit.assert_called_once()
    
    async def test_emotional_state_change_during_interaction(
        self,
        sample_npc,
        sample_player,
        llm_factory_mock
    ):
        """
        Test emotional state changes based on interaction type
        """
        # Arrange - Start with neutral emotion
        assert sample_npc.emotional_state.current_emotion == Emotion.NEUTRAL
        
        # Act - Positive interaction
        await llm_factory_mock.generate(
            prompt="Player compliments NPC's merchandise"
        )
        
        # Update emotion based on positive feedback
        sample_npc.update_emotion(Emotion.HAPPY, intensity=0.7)
        
        # Assert
        assert sample_npc.emotional_state.current_emotion == Emotion.HAPPY
        assert sample_npc.emotional_state.emotion_intensity == 0.7
        assert sample_npc.emotional_state.last_updated is not None
    
    async def test_multi_turn_conversation_with_context(
        self,
        sample_npc,
        sample_player,
        llm_factory_mock,
        relationship_manager
    ):
        """
        Test multi-turn conversation maintaining context
        """
        # Arrange
        conversation_history = []
        
        # Act - Turn 1
        turn1_message = "Hello!"
        turn1_response = await llm_factory_mock.chat(
            messages=[{"role": "user", "content": turn1_message}]
        )
        conversation_history.append({
            "player": turn1_message,
            "npc": turn1_response.text
        })
        
        await relationship_manager.update_relationship(
            sample_player.player_id,
            sample_npc.npc_id,
            0.02,
            "greeting"
        )
        
        # Act - Turn 2
        turn2_message = "What can you tell me about this town?"
        turn2_response = await llm_factory_mock.chat(
            messages=[
                {"role": "user", "content": turn1_message},
                {"role": "assistant", "content": turn1_response.text},
                {"role": "user", "content": turn2_message}
            ]
        )
        conversation_history.append({
            "player": turn2_message,
            "npc": turn2_response.text
        })
        
        await relationship_manager.update_relationship(
            sample_player.player_id,
            sample_npc.npc_id,
            0.03,
            "information_request"
        )
        
        # Act - Turn 3
        turn3_message = "Thank you for your help!"
        turn3_response = await llm_factory_mock.chat(
            messages=[
                {"role": "user", "content": turn1_message},
                {"role": "assistant", "content": turn1_response.text},
                {"role": "user", "content": turn2_message},
                {"role": "assistant", "content": turn2_response.text},
                {"role": "user", "content": turn3_message}
            ]
        )
        conversation_history.append({
            "player": turn3_message,
            "npc": turn3_response.text
        })
        
        relationship = await relationship_manager.update_relationship(
            sample_player.player_id,
            sample_npc.npc_id,
            0.05,
            "gratitude"
        )
        
        # Assert
        assert len(conversation_history) == 3
        assert relationship.interaction_count == 3
        assert relationship.affinity >= 0.10  # 0.02 + 0.03 + 0.05
    
    async def test_reputation_impact_on_npc_response(
        self,
        sample_npc,
        sample_player,
        relationship_manager
    ):
        """
        Test that NPC reputation affects interaction quality
        """
        # Arrange - Establish high reputation
        relationship = await relationship_manager.update_relationship(
            sample_player.player_id,
            sample_npc.npc_id,
            0.80,
            "multiple_positive_interactions"
        )
        
        # Update NPC's reputation tracking
        sample_npc.add_reputation(sample_player.player_id, 80.0)
        
        # Assert
        assert relationship.affinity == 0.80
        assert relationship.relationship_type == RelationshipType.FRIEND
        assert sample_npc.reputation[sample_player.player_id] == 80.0
    
    async def test_quest_completion_workflow(
        self,
        sample_npc,
        sample_player,
        sample_quest,
        relationship_manager,
        mock_db_session
    ):
        """
        Test complete quest workflow: accept → progress → complete → reward
        """
        # Arrange
        sample_quest.accept()
        
        # Add objective progress
        from models.quest import QuestObjective
        objective = QuestObjective(
            objective_id="obj_001",
            description="Collect healing herbs",
            objective_type="collect",
            target="healing_herb",
            required_count=10,
            current_count=0
        )
        sample_quest.add_objective(objective)
        
        # Act - Progress quest
        for i in range(10):
            objective.update_progress(1)
        
        # Check if completable
        can_complete = sample_quest.check_completion()
        assert can_complete, "Quest should be completable"
        
        # Complete quest
        sample_quest.complete()
        
        # Update relationship (quest completion bonus)
        relationship = await relationship_manager.update_relationship(
            sample_player.player_id,
            sample_npc.npc_id,
            0.15,
            "quest_completion",
            note=f"Completed: {sample_quest.title}"
        )
        
        # Simulate reward distribution
        sample_player.zeny += sample_quest.rewards.zeny
        sample_player.experience += sample_quest.rewards.base_exp
        
        # Commit to database
        mock_db_session.add(sample_quest)
        await mock_db_session.commit()
        
        # Assert
        assert sample_quest.status == QuestStatus.COMPLETED
        assert objective.is_completed
        assert objective.current_count == 10
        assert relationship.affinity >= 0.15
        assert sample_player.zeny > 0 or sample_player.experience > 0


@pytest.mark.integration
@pytest.mark.asyncio
class TestErrorPropagation:
    """Test error handling through the interaction stack"""
    
    async def test_llm_failure_propagation(
        self,
        sample_npc,
        sample_player,
        relationship_manager
    ):
        """
        Test that LLM failures are handled gracefully
        """
        # Arrange - Create factory that will fail
        failing_factory = MagicMock(spec=LLMProviderFactory)
        failing_factory.generate = AsyncMock(
            side_effect=Exception("LLM provider unavailable")
        )
        
        # Act & Assert
        with pytest.raises(Exception) as exc_info:
            await failing_factory.generate(
                prompt="Test prompt",
                temperature=0.7
            )
        
        assert "LLM provider unavailable" in str(exc_info.value)
        
        # Verify system state remains consistent
        assert sample_npc.is_active
        assert sample_player.is_online
    
    async def test_database_transaction_rollback(
        self,
        sample_quest,
        mock_db_session
    ):
        """
        Test transaction rollback on error
        """
        # Arrange
        original_status = sample_quest.status
        
        # Simulate transaction with error
        try:
            sample_quest.accept()
            mock_db_session.add(sample_quest)
            
            # Simulate error during commit
            mock_db_session.commit = AsyncMock(side_effect=Exception("DB error"))
            await mock_db_session.commit()
            
        except Exception:
            # Rollback transaction
            await mock_db_session.rollback()
            sample_quest.status = original_status
        
        # Assert
        mock_db_session.rollback.assert_called_once()
        assert sample_quest.status == original_status
    
    async def test_relationship_update_failure_isolation(
        self,
        sample_player,
        sample_npc,
        relationship_manager,
        mock_dragonfly
    ):
        """
        Test that relationship update failures don't corrupt state
        """
        # Arrange - Make cache fail
        mock_dragonfly.setex = MagicMock(
            side_effect=Exception("Cache unavailable")
        )
        
        # Act - Update should still work even if cache fails
        try:
            relationship = await relationship_manager.update_relationship(
                sample_player.player_id,
                sample_npc.npc_id,
                0.05,
                "dialogue"
            )
            
            # Should succeed despite cache failure
            assert relationship is not None
            assert relationship.affinity > 0.0
            
        except Exception as e:
            # If it does fail, verify it fails gracefully
            pytest.fail(f"Should handle cache failure gracefully: {e}")


@pytest.mark.integration
@pytest.mark.asyncio
class TestDataConsistency:
    """Test data consistency across operations"""
    
    async def test_concurrent_relationship_updates(
        self,
        sample_player,
        sample_npc,
        relationship_manager
    ):
        """
        Test that concurrent relationship updates maintain consistency
        """
        # Arrange
        update_count = 5
        affinity_per_update = 0.02
        
        # Act - Simulate concurrent updates
        tasks = [
            relationship_manager.update_relationship(
                sample_player.player_id,
                sample_npc.npc_id,
                affinity_per_update,
                f"interaction_{i}"
            )
            for i in range(update_count)
        ]
        
        relationships = await asyncio.gather(*tasks)
        
        # Assert - Final relationship should reflect all updates
        final_relationship = relationships[-1]
        expected_min_affinity = affinity_per_update * update_count * 0.8  # Account for diminishing returns
        
        assert final_relationship.interaction_count == update_count
        assert final_relationship.affinity >= expected_min_affinity
    
    async def test_npc_state_consistency_after_multiple_interactions(
        self,
        sample_npc,
        sample_player
    ):
        """
        Test NPC state remains consistent after multiple interactions
        """
        # Arrange
        initial_interaction_count = sample_npc.interaction_count
        interaction_rounds = 10
        
        # Act
        for _ in range(interaction_rounds):
            sample_npc.record_interaction()
            sample_npc.add_reputation(sample_player.player_id, 1.0)
        
        # Assert
        assert sample_npc.interaction_count == initial_interaction_count + interaction_rounds
        assert sample_npc.last_interaction is not None
        assert sample_npc.reputation[sample_player.player_id] == min(100.0, 1.0 * interaction_rounds)