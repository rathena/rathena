"""
Tests for BaseAgent Class

Tests:
- Agent initialization
- Async execution
- Timeout handling
- Error handling
- Metrics tracking
- Thread pool management
- Cleanup procedures
"""

import asyncio
import pytest
from datetime import datetime
from unittest.mock import AsyncMock, MagicMock, patch

from agents.base_agent import (
    BaseAgent,
    AgentContext,
    AgentResponse,
    AgentStatus,
)


class ConcreteTestAgent(BaseAgent):
    """Concrete implementation of BaseAgent for testing"""
    
    def __init__(self, *args, should_fail=False, should_timeout=False, **kwargs):
        super().__init__(*args, **kwargs)
        self.should_fail = should_fail
        self.should_timeout = should_timeout
        self.process_called = False
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """Test implementation of process method"""
        self.process_called = True
        
        if self.should_timeout:
            await asyncio.sleep(10)  # Simulate timeout
        
        if self.should_fail:
            raise ValueError("Test error")
        
        return AgentResponse(
            agent_name=self.name,
            status=AgentStatus.COMPLETED,
            result={"processed": True, "context_id": context.request_id},
            confidence=0.95,
            reasoning="Successful test processing",
            metadata={"test": True},
        )


@pytest.mark.unit
class TestBaseAgentInitialization:
    """Test agent initialization"""
    
    def test_basic_initialization(self):
        """Test basic agent initialization"""
        agent = ConcreteTestAgent(
            name="test_agent",
            description="Test agent description"
        )
        
        assert agent.name == "test_agent"
        assert agent.description == "Test agent description"
        assert agent.timeout_seconds == 30.0
        assert not agent.is_initialized
        assert agent._executor is not None
    
    def test_custom_timeout(self):
        """Test agent with custom timeout"""
        agent = ConcreteTestAgent(
            name="test_agent",
            timeout_seconds=60.0
        )
        
        assert agent.timeout_seconds == 60.0
    
    def test_custom_workers(self):
        """Test agent with custom worker count"""
        agent = ConcreteTestAgent(
            name="test_agent",
            max_workers=8
        )
        
        assert agent._executor._max_workers == 8


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseAgentExecution:
    """Test agent execution"""
    
    async def test_successful_execution(self, sample_agent_context):
        """Test successful agent execution"""
        agent = ConcreteTestAgent(name="test_agent")
        
        response = await agent.execute(sample_agent_context)
        
        assert response.agent_name == "test_agent"
        assert response.status == AgentStatus.COMPLETED
        assert response.is_successful()
        assert response.result["processed"] is True
        assert response.confidence == 0.95
        assert response.error is None
        assert response.execution_time_ms > 0
        assert agent.process_called
    
    async def test_auto_initialization(self, sample_agent_context):
        """Test agent auto-initializes on first execution"""
        agent = ConcreteTestAgent(name="test_agent")
        
        assert not agent.is_initialized
        
        await agent.execute(sample_agent_context)
        
        assert agent.is_initialized
    
    async def test_execution_with_error(self, sample_agent_context):
        """Test agent execution with error"""
        agent = ConcreteTestAgent(
            name="test_agent",
            should_fail=True
        )
        
        response = await agent.execute(sample_agent_context)
        
        assert response.status == AgentStatus.FAILED
        assert not response.is_successful()
        assert response.error == "Test error"
        assert response.execution_time_ms > 0
    
    async def test_execution_timeout(self, sample_agent_context):
        """Test agent execution timeout"""
        agent = ConcreteTestAgent(
            name="test_agent",
            timeout_seconds=0.1,
            should_timeout=True
        )
        
        response = await agent.execute(sample_agent_context)
        
        assert response.status == AgentStatus.TIMEOUT
        assert not response.is_successful()
        assert "timeout" in response.error.lower()
        assert response.execution_time_ms > 0


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseAgentMetrics:
    """Test agent metrics tracking"""
    
    async def test_metrics_on_success(self, sample_agent_context):
        """Test metrics are recorded on successful execution"""
        agent = ConcreteTestAgent(name="test_agent")
        
        with patch.object(agent, 'requests_total') as mock_counter:
            response = await agent.execute(sample_agent_context)
            
            # Verify counter was incremented
            mock_counter.labels.assert_called_once()
            mock_counter.labels.return_value.inc.assert_called_once()
    
    async def test_metrics_on_failure(self, sample_agent_context):
        """Test metrics are recorded on failed execution"""
        agent = ConcreteTestAgent(name="test_agent", should_fail=True)
        
        with patch.object(agent, 'errors_total') as mock_counter:
            response = await agent.execute(sample_agent_context)
            
            # Verify error counter was incremented
            mock_counter.labels.assert_called_once()
    
    async def test_active_requests_tracking(self, sample_agent_context):
        """Test active requests gauge is updated"""
        agent = ConcreteTestAgent(name="test_agent")
        
        with patch.object(agent, 'active_requests') as mock_gauge:
            await agent.execute(sample_agent_context)
            
            # Verify gauge was incremented and decremented
            assert mock_gauge.labels.return_value.inc.called
            assert mock_gauge.labels.return_value.dec.called


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseAgentLifecycle:
    """Test agent lifecycle methods"""
    
    async def test_initialization(self):
        """Test agent initialization"""
        agent = ConcreteTestAgent(name="test_agent")
        
        await agent.initialize()
        
        assert agent.is_initialized
    
    async def test_double_initialization(self):
        """Test double initialization is safe"""
        agent = ConcreteTestAgent(name="test_agent")
        
        await agent.initialize()
        await agent.initialize()  # Should not raise
        
        assert agent.is_initialized
    
    async def test_cleanup(self):
        """Test agent cleanup"""
        agent = ConcreteTestAgent(name="test_agent")
        await agent.initialize()
        
        await agent.cleanup()
        
        assert not agent.is_initialized
    
    async def test_executor_shutdown_on_cleanup(self):
        """Test thread pool is shut down on cleanup"""
        agent = ConcreteTestAgent(name="test_agent")
        
        executor = agent._executor
        await agent.cleanup()
        
        # Executor should be shut down (this will raise if we try to submit)
        with pytest.raises(RuntimeError):
            executor.submit(lambda: None)


@pytest.mark.unit
def test_agent_response_methods():
    """Test AgentResponse helper methods"""
    # Successful response
    success_response = AgentResponse(
        agent_name="test",
        status=AgentStatus.COMPLETED,
        result={"data": "test"},
        error=None,
    )
    assert success_response.is_successful()
    
    # Failed response
    failed_response = AgentResponse(
        agent_name="test",
        status=AgentStatus.FAILED,
        result=None,
        error="Test error",
    )
    assert not failed_response.is_successful()
    
    # Response to dict
    response_dict = success_response.to_dict()
    assert response_dict["agent_name"] == "test"
    assert response_dict["status"] == "completed"
    assert response_dict["result"] == {"data": "test"}


@pytest.mark.unit
def test_agent_context_creation(faker_instance):
    """Test AgentContext creation"""
    context = AgentContext(
        request_id=faker_instance.uuid4(),
        npc_id=faker_instance.uuid4(),
        npc_state=MagicMock(),
        world_state=MagicMock(),
        location="test_location",
    )
    
    assert context.request_id is not None
    assert context.npc_id is not None
    assert context.location == "test_location"
    assert context.temperature == 0.7
    assert context.max_response_length == 500


@pytest.mark.unit
def test_agent_status_enum():
    """Test AgentStatus enum values"""
    assert AgentStatus.IDLE.value == "idle"
    assert AgentStatus.PROCESSING.value == "processing"
    assert AgentStatus.COMPLETED.value == "completed"
    assert AgentStatus.FAILED.value == "failed"
    assert AgentStatus.TIMEOUT.value == "timeout"


@pytest.mark.unit
@pytest.mark.asyncio
class TestBaseAgentThreadPool:
    """Test thread pool functionality"""
    
    async def test_run_in_executor(self):
        """Test running synchronous function in executor"""
        agent = ConcreteTestAgent(name="test_agent")
        
        def sync_function(x, y):
            return x + y
        
        result = await agent.run_in_executor(sync_function, 5, 3)
        
        assert result == 8
    
    async def test_concurrent_executor_tasks(self):
        """Test running multiple tasks concurrently"""
        agent = ConcreteTestAgent(name="test_agent")
        
        def slow_function(duration):
            import time
            time.sleep(duration)
            return duration
        
        # Run 3 tasks concurrently
        tasks = [
            agent.run_in_executor(slow_function, 0.1),
            agent.run_in_executor(slow_function, 0.1),
            agent.run_in_executor(slow_function, 0.1),
        ]
        
        start_time = datetime.utcnow()
        results = await asyncio.gather(*tasks)
        end_time = datetime.utcnow()
        
        # Should complete in ~0.1s (concurrent) not 0.3s (sequential)
        duration = (end_time - start_time).total_seconds()
        assert duration < 0.25  # Allow some overhead
        assert results == [0.1, 0.1, 0.1]


@pytest.mark.unit
def test_agent_repr():
    """Test agent string representation"""
    agent = ConcreteTestAgent(name="test_agent")
    
    repr_str = repr(agent)
    
    assert "ConcreteTestAgent" in repr_str
    assert "test_agent" in repr_str