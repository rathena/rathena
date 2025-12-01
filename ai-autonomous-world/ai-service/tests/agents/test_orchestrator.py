"""
Test suite for AgentOrchestrator - Multi-agent coordination.

Tests cover:
- Agent initialization and management
- Workflow execution (simple, sequential, parallel, hierarchical)
- NPC interaction workflows
- Quest generation workflows
- Agent coordination and result aggregation
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch, call
import pytest

from agents.base_agent import AgentContext, AgentResponse, AgentStatus
from agents.orchestrator import AgentOrchestrator, WorkflowType


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def orchestrator():
    """Create orchestrator instance."""
    return AgentOrchestrator(
        enable_crewai=False,  # Phase 3 feature
        max_parallel_agents=5
    )


@pytest.fixture
def orchestrator_context(sample_agent_context):
    """Create agent context for orchestrator."""
    return sample_agent_context


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_orchestrator_initialization():
    """Test orchestrator initializes with sub-agents."""
    orchestrator = AgentOrchestrator(
        enable_crewai=False,
        max_parallel_agents=3
    )
    
    await orchestrator.initialize()
    
    assert orchestrator.name == "AgentOrchestrator"
    assert orchestrator.max_parallel_agents == 3
    assert len(orchestrator._agents) > 0
    assert "dialogue" in orchestrator._agents
    assert "decision" in orchestrator._agents
    assert "memory" in orchestrator._agents
    assert "world" in orchestrator._agents
    assert "quest" in orchestrator._agents
    assert "economy" in orchestrator._agents
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_get_agent_by_name(orchestrator):
    """Test retrieving agents by name."""
    await orchestrator.initialize()
    
    dialogue_agent = orchestrator.get_agent("dialogue")
    memory_agent = orchestrator.get_agent("memory")
    
    assert dialogue_agent is not None
    assert memory_agent is not None
    assert orchestrator.get_agent("nonexistent") is None
    
    await orchestrator.cleanup()


# ============================================================================
# SIMPLE WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_simple_workflow_single_agent(orchestrator, orchestrator_context):
    """Test simple workflow with single agent."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.SIMPLE,
        "agents": ["dialogue"]
    }
    
    # Mock the dialogue agent
    mock_response = AgentResponse(
        agent_name="dialogue",
        status=AgentStatus.COMPLETED,
        result={"message": "Test response"},
        confidence=0.9
    )
    
    with patch.object(
        orchestrator._agents["dialogue"],
        'execute',
        return_value=mock_response
    ):
        response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "agent" in response.result
    assert response.result["agent"] == "dialogue"
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_simple_workflow_no_agents(orchestrator, orchestrator_context):
    """Test simple workflow with no agents specified."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.SIMPLE,
        "agents": []
    }
    
    response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.FAILED
    
    await orchestrator.cleanup()


# ============================================================================
# SEQUENTIAL WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_sequential_workflow(orchestrator, orchestrator_context):
    """Test sequential workflow execution."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.SEQUENTIAL,
        "agents": ["memory", "decision", "dialogue"]
    }
    
    # Mock all agent responses
    mock_responses = {
        "memory": AgentResponse(
            agent_name="memory",
            status=AgentStatus.COMPLETED,
            result={"memories": ["memory1", "memory2"]},
            confidence=0.8
        ),
        "decision": AgentResponse(
            agent_name="decision",
            status=AgentStatus.COMPLETED,
            result={"action": "greet"},
            confidence=0.85
        ),
        "dialogue": AgentResponse(
            agent_name="dialogue",
            status=AgentStatus.COMPLETED,
            result={"message": "Hello!"},
            confidence=0.9
        )
    }
    
    for agent_name, mock_resp in mock_responses.items():
        with patch.object(
            orchestrator._agents[agent_name],
            'execute',
            return_value=mock_resp
        ):
            pass
    
    response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "results" in response.result
    assert response.result["workflow"] == "sequential"
    assert response.result["agents_executed"] > 0
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_sequential_workflow_context_passing(orchestrator, orchestrator_context):
    """Test that sequential workflow passes results between agents."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.SEQUENTIAL,
        "agents": ["memory", "dialogue"]
    }
    
    memory_result = {"memories": ["important"]}
    
    # Track calls to verify context passing
    calls_made = []
    
    async def mock_memory_execute(ctx):
        calls_made.append(("memory", ctx.additional_data.copy()))
        return AgentResponse(
            agent_name="memory",
            status=AgentStatus.COMPLETED,
            result=memory_result,
            confidence=0.8
        )
    
    async def mock_dialogue_execute(ctx):
        calls_made.append(("dialogue", ctx.additional_data.copy()))
        return AgentResponse(
            agent_name="dialogue",
            status=AgentStatus.COMPLETED,
            result={"message": "Test"},
            confidence=0.9
        )
    
    orchestrator._agents["memory"].execute = mock_memory_execute
    orchestrator._agents["dialogue"].execute = mock_dialogue_execute
    
    response = await orchestrator.process(orchestrator_context)
    
    assert len(calls_made) == 2
    # Second agent should have previous results
    assert "previous_results" in calls_made[1][1]
    
    await orchestrator.cleanup()


# ============================================================================
# PARALLEL WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_parallel_workflow(orchestrator, orchestrator_context):
    """Test parallel workflow execution."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.PARALLEL,
        "agents": ["memory", "world", "economy"]
    }
    
    # Mock responses for all agents
    for agent_name in ["memory", "world", "economy"]:
        mock_resp = AgentResponse(
            agent_name=agent_name,
            status=AgentStatus.COMPLETED,
            result={agent_name: "data"},
            confidence=0.8
        )
        orchestrator._agents[agent_name].execute = AsyncMock(return_value=mock_resp)
    
    response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "results" in response.result
    assert response.result["workflow"] == "parallel"
    assert len(response.result["results"]) == 3
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_parallel_workflow_max_agents_limit(orchestrator, orchestrator_context):
    """Test parallel workflow respects max agent limit."""
    await orchestrator.initialize()
    
    # Set low limit
    orchestrator.max_parallel_agents = 2
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.PARALLEL,
        "agents": ["memory", "world", "economy", "dialogue", "decision"]  # 5 agents
    }
    
    # Mock all agents
    for agent_name in orchestrator._agents.keys():
        mock_resp = AgentResponse(
            agent_name=agent_name,
            status=AgentStatus.COMPLETED,
            result={},
            confidence=0.8
        )
        orchestrator._agents[agent_name].execute = AsyncMock(return_value=mock_resp)
    
    response = await orchestrator.process(orchestrator_context)
    
    # Should only execute first 2 agents
    assert len(response.result["results"]) == 2
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_parallel_workflow_handles_failures(orchestrator, orchestrator_context):
    """Test parallel workflow handles individual agent failures."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.PARALLEL,
        "agents": ["memory", "world"]
    }
    
    # Mock one success, one failure
    orchestrator._agents["memory"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="memory",
            status=AgentStatus.COMPLETED,
            result={"data": "ok"},
            confidence=0.8
        )
    )
    orchestrator._agents["world"].execute = AsyncMock(
        side_effect=Exception("Test error")
    )
    
    response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert len(response.result["results"]) == 2
    # Check that error is captured
    assert any("error" in r for r in response.result["results"])
    
    await orchestrator.cleanup()


# ============================================================================
# HIERARCHICAL WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_hierarchical_workflow_fallback(orchestrator, orchestrator_context):
    """Test hierarchical workflow (currently falls back to sequential)."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.HIERARCHICAL,
        "agents": ["memory", "dialogue"]
    }
    
    # Mock agents
    for agent_name in ["memory", "dialogue"]:
        mock_resp = AgentResponse(
            agent_name=agent_name,
            status=AgentStatus.COMPLETED,
            result={},
            confidence=0.8
        )
        orchestrator._agents[agent_name].execute = AsyncMock(return_value=mock_resp)
    
    response = await orchestrator.process(orchestrator_context)
    
    # Should fallback to sequential for now
    assert response.status == AgentStatus.COMPLETED
    assert "sequential" in response.result["workflow"]
    
    await orchestrator.cleanup()


# ============================================================================
# NPC INTERACTION WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_handle_npc_interaction(orchestrator, orchestrator_context):
    """Test complete NPC interaction workflow."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data["player_message"] = "Hello NPC!"
    orchestrator_context.additional_data["conversation_history"] = []
    
    # Mock all required agents
    orchestrator._agents["memory"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="memory",
            status=AgentStatus.COMPLETED,
            result={"memories": ["past_interaction"]},
            confidence=0.8
        )
    )
    orchestrator._agents["decision"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="decision",
            status=AgentStatus.COMPLETED,
            result={"action": "greet"},
            confidence=0.85
        )
    )
    orchestrator._agents["dialogue"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="dialogue",
            status=AgentStatus.COMPLETED,
            result={"message": "Greetings, traveler!"},
            confidence=0.9
        )
    )
    
    result = await orchestrator.handle_npc_interaction(orchestrator_context)
    
    assert result["interaction_complete"] is True
    assert "results" in result
    assert "memories" in result["results"]
    assert "decision" in result["results"]
    assert "dialogue" in result["results"]
    
    await orchestrator.cleanup()


# ============================================================================
# QUEST GENERATION WORKFLOW TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_generate_quest_with_context(orchestrator, orchestrator_context):
    """Test quest generation with world context."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data["player_level"] = 10
    
    # Mock agents
    orchestrator._agents["world"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="world",
            status=AgentStatus.COMPLETED,
            result={"time_of_day": "night", "weather": "stormy"},
            confidence=0.8
        )
    )
    orchestrator._agents["quest"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="quest",
            status=AgentStatus.COMPLETED,
            result={"quest_id": "test_quest", "title": "Test Quest"},
            confidence=0.9
        )
    )
    
    result = await orchestrator.generate_quest_with_context(orchestrator_context)
    
    assert "quest" in result
    assert "world_context" in result
    
    await orchestrator.cleanup()


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_invalid_workflow_type(orchestrator, orchestrator_context):
    """Test handling of invalid workflow type."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": "invalid_workflow",
        "agents": ["dialogue"]
    }
    
    response = await orchestrator.process(orchestrator_context)
    
    assert response.status == AgentStatus.FAILED
    assert response.error is not None
    
    await orchestrator.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_unknown_agent_skip(orchestrator, orchestrator_context):
    """Test that unknown agents are skipped gracefully."""
    await orchestrator.initialize()
    
    orchestrator_context.additional_data = {
        "workflow": WorkflowType.SEQUENTIAL,
        "agents": ["memory", "nonexistent_agent", "dialogue"]
    }
    
    # Mock valid agents
    orchestrator._agents["memory"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="memory",
            status=AgentStatus.COMPLETED,
            result={},
            confidence=0.8
        )
    )
    orchestrator._agents["dialogue"].execute = AsyncMock(
        return_value=AgentResponse(
            agent_name="dialogue",
            status=AgentStatus.COMPLETED,
            result={},
            confidence=0.9
        )
    )
    
    response = await orchestrator.process(orchestrator_context)
    
    # Should complete with only valid agents
    assert response.status == AgentStatus.COMPLETED
    assert response.result["agents_executed"] == 2  # Only valid agents
    
    await orchestrator.cleanup()


# ============================================================================
# CLEANUP TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_cleanup_all_agents(orchestrator):
    """Test that cleanup properly cleans up all sub-agents."""
    await orchestrator.initialize()
    
    # Track cleanup calls
    cleanup_calls = []
    for agent_name, agent in orchestrator._agents.items():
        original_cleanup = agent.cleanup
        async def mock_cleanup():
            cleanup_calls.append(agent_name)
            await original_cleanup()
        agent.cleanup = mock_cleanup
    
    await orchestrator.cleanup()
    
    # All agents should be cleaned up
    assert len(cleanup_calls) == len(orchestrator._agents)