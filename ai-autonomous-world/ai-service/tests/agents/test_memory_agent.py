"""
Tests for MemoryAgent

Tests:
- Memory storage operations
- Memory retrieval with relevance scoring
- Relationship tracking
- Memory consolidation
- Short-term and long-term memory
- OpenMemory integration preparation
- Memory decay calculations
- Access tracking
"""

import pytest
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch

from agents.memory_agent import MemoryAgent, MemoryEntry, RelationshipMemory
from agents.base_agent import AgentContext, AgentResponse, AgentStatus
from models.npc import NPCState


@pytest.fixture
def memory_agent():
    """Create memory agent instance"""
    return MemoryAgent(
        memory_decay_days=30,
        max_short_term_memories=20
    )


@pytest.fixture
def memory_context(sample_agent_context, faker_instance):
    """Create context for memory operations"""
    sample_agent_context.additional_data = {
        "operation": "store",
        "content": "Player helped me with a quest",
        "type": "interaction",
        "importance": 0.8
    }
    return sample_agent_context


@pytest.mark.unit
class TestMemoryAgentInitialization:
    """Test memory agent initialization"""
    
    def test_basic_initialization(self):
        """Test basic memory agent initialization"""
        agent = MemoryAgent()
        
        assert agent.name == "MemoryAgent"
        assert agent.memory_decay_days == 30
        assert agent.max_short_term_memories == 20
        assert len(agent._short_term_memory) == 0
        assert len(agent._long_term_memory) == 0
    
    def test_custom_initialization(self):
        """Test memory agent with custom parameters"""
        agent = MemoryAgent(
            memory_decay_days=60,
            max_short_term_memories=50
        )
        
        assert agent.memory_decay_days == 60
        assert agent.max_short_term_memories == 50
    
    @pytest.mark.asyncio
    async def test_initialization_with_openmemory(self, monkeypatch):
        """Test initialization with OpenMemory config"""
        monkeypatch.setenv("OPENMEMORY_URL", "http://test.com")
        monkeypatch.setenv("OPENMEMORY_API_KEY", "test_key")
        
        agent = MemoryAgent()
        await agent.initialize()
        
        assert agent.is_initialized


@pytest.mark.unit
@pytest.mark.asyncio
class TestMemoryStorage:
    """Test memory storage operations"""
    
    async def test_store_short_term_memory(self, memory_agent, memory_context):
        """Test storing memory in short-term storage"""
        await memory_agent.initialize()
        memory_context.additional_data["importance"] = 0.5
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["stored"] is True
        assert response.result["storage_type"] == "short_term"
    
    async def test_store_long_term_memory(self, memory_agent, memory_context):
        """Test storing important memory in long-term storage"""
        await memory_agent.initialize()
        memory_context.additional_data["importance"] = 0.8
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["stored"] is True
        assert response.result["storage_type"] == "long_term"
    
    async def test_short_term_memory_limit(self, memory_agent, memory_context):
        """Test short-term memory respects max size"""
        await memory_agent.initialize()
        npc_id = memory_context.npc_state.npc_id
        
        # Store more than max
        for i in range(25):
            memory_context.additional_data["content"] = f"Memory {i}"
            memory_context.additional_data["importance"] = 0.5
            await memory_agent.execute(memory_context)
        
        # Should not exceed max
        assert len(memory_agent._short_term_memory[npc_id]) <= memory_agent.max_short_term_memories
    
    async def test_memory_with_metadata(self, memory_agent, memory_context):
        """Test storing memory with metadata"""
        await memory_agent.initialize()
        
        memory_context.additional_data["metadata"] = {
            "quest_id": "quest_123",
            "reward": 100
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert "memory_id" in response.result


@pytest.mark.unit
@pytest.mark.asyncio
class TestMemoryRetrieval:
    """Test memory retrieval operations"""
    
    async def test_retrieve_empty_memories(self, memory_agent, memory_context):
        """Test retrieving from empty memory"""
        await memory_agent.initialize()
        
        memory_context.additional_data = {
            "operation": "retrieve",
            "query": "test",
            "limit": 5
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["count"] == 0
        assert len(response.result["memories"]) == 0
    
    async def test_retrieve_memories(self, memory_agent, memory_context):
        """Test retrieving stored memories"""
        await memory_agent.initialize()
        
        # Store some memories
        for i in range(5):
            memory_context.additional_data = {
                "operation": "store",
                "content": f"Memory about quest {i}",
                "type": "interaction",
                "importance": 0.7
            }
            await memory_agent.execute(memory_context)
        
        # Retrieve memories
        memory_context.additional_data = {
            "operation": "retrieve",
            "query": "quest",
            "limit": 3
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["count"] <= 3
        assert len(response.result["memories"]) <= 3
    
    async def test_retrieve_by_type(self, memory_agent, memory_context):
        """Test retrieving memories by type"""
        await memory_agent.initialize()
        
        # Store different types
        for memory_type in ["interaction", "quest", "trade"]:
            memory_context.additional_data = {
                "operation": "store",
                "content": f"A {memory_type} memory",
                "type": memory_type,
                "importance": 0.7
            }
            await memory_agent.execute(memory_context)
        
        # Retrieve only quest memories
        memory_context.additional_data = {
            "operation": "retrieve",
            "type": "quest",
            "limit": 10
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        for memory in response.result["memories"]:
            assert memory["type"] == "quest"
    
    async def test_memory_access_tracking(self, memory_agent, memory_context):
        """Test that memory access is tracked"""
        await memory_agent.initialize()
        npc_id = memory_context.npc_state.npc_id
        
        # Store a memory
        memory_context.additional_data = {
            "operation": "store",
            "content": "Test memory",
            "importance": 0.8
        }
        await memory_agent.execute(memory_context)
        
        # Retrieve it multiple times
        memory_context.additional_data = {
            "operation": "retrieve",
            "query": "test",
            "limit": 5
        }
        await memory_agent.execute(memory_context)
        await memory_agent.execute(memory_context)
        
        # Check access count increased
        memories = list(memory_agent._short_term_memory.get(npc_id, []))
        if memories:
            assert memories[0].access_count >= 2


@pytest.mark.unit
@pytest.mark.asyncio
class TestRelationshipTracking:
    """Test relationship tracking"""
    
    async def test_update_relationship(self, memory_agent, memory_context, faker_instance):
        """Test updating relationship with entity"""
        await memory_agent.initialize()
        
        entity_id = faker_instance.uuid4()
        memory_context.additional_data = {
            "operation": "update_relationship",
            "entity_id": entity_id,
            "reputation_change": 10.0,
            "event": "Helped with quest"
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["entity_id"] == entity_id
        assert response.result["interaction_count"] == 1
    
    async def test_get_relationship(self, memory_agent, memory_context, faker_instance):
        """Test getting relationship information"""
        await memory_agent.initialize()
        
        entity_id = faker_instance.uuid4()
        
        # Create relationship
        memory_context.additional_data = {
            "operation": "update_relationship",
            "entity_id": entity_id,
            "reputation_change": 5.0
        }
        await memory_agent.execute(memory_context)
        
        # Get relationship
        memory_context.additional_data = {
            "operation": "get_relationship",
            "entity_id": entity_id
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["exists"] is True
        assert response.result["interaction_count"] == 1
    
    async def test_reputation_trend_improving(self, memory_agent, memory_context, faker_instance):
        """Test reputation trend calculation - improving"""
        await memory_agent.initialize()
        
        entity_id = faker_instance.uuid4()
        npc_id = memory_context.npc_state.npc_id
        memory_context.npc_state.reputation = {entity_id: 0.0}
        
        # Multiple positive interactions
        for i in range(5):
            memory_context.npc_state.reputation[entity_id] = i * 10
            memory_context.additional_data = {
                "operation": "update_relationship",
                "entity_id": entity_id,
                "reputation_change": 10.0
            }
            await memory_agent.execute(memory_context)
        
        # Get relationship
        memory_context.additional_data = {
            "operation": "get_relationship",
            "entity_id": entity_id
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.result["reputation_trend"] in ["improving", "stable"]
    
    async def test_get_nonexistent_relationship(self, memory_agent, memory_context, faker_instance):
        """Test getting non-existent relationship"""
        await memory_agent.initialize()
        
        entity_id = faker_instance.uuid4()
        memory_context.additional_data = {
            "operation": "get_relationship",
            "entity_id": entity_id
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.COMPLETED
        assert response.result["exists"] is False


@pytest.mark.unit
class TestMemoryRelevance:
    """Test memory relevance scoring"""
    
    def test_recency_weight_recent(self):
        """Test recency weight for recent memory"""
        agent = MemoryAgent()
        
        memory = MemoryEntry(
            memory_id="test_1",
            content="Recent memory",
            memory_type="interaction",
            importance=0.8,
            timestamp=datetime.utcnow()
        )
        
        weight = agent._calculate_recency_weight(memory)
        
        assert weight == 1.0
    
    def test_recency_weight_old(self):
        """Test recency weight for old memory"""
        agent = MemoryAgent()
        
        memory = MemoryEntry(
            memory_id="test_1",
            content="Old memory",
            memory_type="interaction",
            importance=0.8,
            timestamp=datetime.utcnow() - timedelta(days=60)
        )
        
        weight = agent._calculate_recency_weight(memory)
        
        assert weight < 1.0
    
    def test_relevance_with_query_match(self):
        """Test relevance calculation with query match"""
        agent = MemoryAgent()
        
        memory = MemoryEntry(
            memory_id="test_1",
            content="Quest about dragon slaying",
            memory_type="quest",
            importance=0.8,
            timestamp=datetime.utcnow()
        )
        
        context = MagicMock()
        relevance = agent._calculate_memory_relevance(memory, "dragon quest", context)
        
        # Should have high relevance due to keyword match
        assert relevance > 0.8


@pytest.mark.unit
@pytest.mark.asyncio
class TestMemoryConsolidation:
    """Test memory consolidation"""
    
    async def test_consolidate_memories(self, memory_agent, memory_context):
        """Test consolidating short-term to long-term"""
        await memory_agent.initialize()
        npc_id = memory_context.npc_state.npc_id
        
        # Store important short-term memories
        for i in range(5):
            memory_context.additional_data = {
                "operation": "store",
                "content": f"Important memory {i}",
                "importance": 0.7
            }
            await memory_agent.execute(memory_context)
        
        # Consolidate
        await memory_agent.consolidate_memories(npc_id)
        
        # Short-term should be cleared
        assert len(memory_agent._short_term_memory.get(npc_id, [])) == 0
        # Long-term should have memories
        assert len(memory_agent._long_term_memory.get(npc_id, [])) > 0
    
    async def test_consolidate_filters_by_importance(self, memory_agent, memory_context):
        """Test consolidation filters by importance"""
        await memory_agent.initialize()
        npc_id = memory_context.npc_state.npc_id
        
        # Store mix of important and unimportant
        for i in range(5):
            memory_context.additional_data = {
                "operation": "store",
                "content": f"Memory {i}",
                "importance": 0.3 if i % 2 else 0.8
            }
            await memory_agent.execute(memory_context)
        
        # Consolidate
        await memory_agent.consolidate_memories(npc_id)
        
        # Only important ones should be in long-term
        long_term = memory_agent._long_term_memory.get(npc_id, [])
        assert all(m.importance >= 0.6 or agent._calculate_recency_weight(m) >= 0.6 
                   for m in long_term)


@pytest.mark.unit
@pytest.mark.asyncio
class TestErrorHandling:
    """Test error handling"""
    
    async def test_invalid_operation(self, memory_agent, memory_context):
        """Test handling invalid operation"""
        await memory_agent.initialize()
        
        memory_context.additional_data = {"operation": "invalid_op"}
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.FAILED
        assert response.error is not None
    
    async def test_missing_entity_id_for_relationship(self, memory_agent, memory_context):
        """Test handling missing entity_id"""
        await memory_agent.initialize()
        
        memory_context.additional_data = {
            "operation": "update_relationship"
            # Missing entity_id
        }
        
        response = await memory_agent.execute(memory_context)
        
        assert response.status == AgentStatus.FAILED
        assert "entity_id" in response.error


@pytest.mark.unit
class TestMemoryEntry:
    """Test MemoryEntry class"""
    
    def test_memory_entry_creation(self):
        """Test creating memory entry"""
        memory = MemoryEntry(
            memory_id="test_1",
            content="Test content",
            memory_type="test",
            importance=0.8,
            timestamp=datetime.utcnow()
        )
        
        assert memory.memory_id == "test_1"
        assert memory.content == "Test content"
        assert memory.access_count == 0
    
    def test_memory_to_dict(self):
        """Test converting memory to dictionary"""
        agent = MemoryAgent()
        
        memory = MemoryEntry(
            memory_id="test_1",
            content="Test content",
            memory_type="test",
            importance=0.8,
            timestamp=datetime.utcnow()
        )
        
        memory_dict = agent._memory_to_dict(memory)
        
        assert "memory_id" in memory_dict
        assert "content" in memory_dict
        assert "importance" in memory_dict


@pytest.mark.unit
class TestRelationshipMemory:
    """Test RelationshipMemory class"""
    
    def test_relationship_memory_creation(self, faker_instance):
        """Test creating relationship memory"""
        entity_id = faker_instance.uuid4()
        
        rel = RelationshipMemory(entity_id)
        
        assert rel.entity_id == entity_id
        assert rel.interaction_count == 0
        assert len(rel.reputation_history) == 0
    
    def test_reputation_trend_calculation(self):
        """Test reputation trend calculation"""
        agent = MemoryAgent()
        rel = RelationshipMemory("test_entity")
        
        # Improving trend
        rel.reputation_history = [0, 10, 20, 30, 40]
        assert agent._calculate_reputation_trend(rel) == "improving"
        
        # Declining trend
        rel.reputation_history = [40, 30, 20, 10, 0]
        assert agent._calculate_reputation_trend(rel) == "declining"
        
        # Stable trend
        rel.reputation_history = [20, 21, 20, 19, 20]
        assert agent._calculate_reputation_trend(rel) == "stable"


@pytest.mark.unit
@pytest.mark.asyncio
class TestCleanup:
    """Test cleanup procedures"""
    
    async def test_cleanup_clears_memory(self):
        """Test cleanup clears all memory"""
        agent = MemoryAgent()
        await agent.initialize()
        
        # Add some data
        agent._short_term_memory["test"] = []
        agent._long_term_memory["test"] = []
        agent._relationships["test"] = {}
        
        # Cleanup
        await agent.cleanup()
        
        assert len(agent._short_term_memory) == 0
        assert len(agent._long_term_memory) == 0
        assert len(agent._relationships) == 0