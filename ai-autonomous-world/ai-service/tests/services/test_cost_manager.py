"""
Tests for Cost Management System

Tests budget enforcement, cost tracking, alerts, and reporting.
"""

import pytest
from datetime import datetime, date
from unittest.mock import Mock, patch

from services.cost_manager import (
    CostManager,
    CostEntry,
    BudgetExceededException,
    ProviderBudgetExceededException,
    init_cost_manager,
    get_cost_manager
)


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def cost_manager():
    """Create a fresh cost manager for each test"""
    return CostManager(
        daily_budget_usd=100.0,
        per_provider_budgets={
            "azure_openai": 50.0,
            "openai": 30.0,
            "anthropic": 20.0
        },
        alert_thresholds=[50, 75, 90, 100],
        enabled=True
    )


@pytest.fixture
def disabled_cost_manager():
    """Cost manager with tracking disabled"""
    return CostManager(enabled=False)


# ============================================================================
# BASIC FUNCTIONALITY TESTS
# ============================================================================

def test_cost_manager_initialization(cost_manager):
    """Test cost manager initializes correctly"""
    assert cost_manager.enabled is True
    assert cost_manager.daily_budget == 100.0
    assert cost_manager.per_provider_budgets["azure_openai"] == 50.0
    assert cost_manager.current_date == date.today()
    assert len(cost_manager.today_costs) == 0


def test_cost_recording(cost_manager):
    """Test recording a cost entry"""
    result = cost_manager.record_cost(
        provider="azure_openai",
        model="gpt-4",
        tokens_input=100,
        tokens_output=50,
        cost_usd=0.05,
        request_type="test"
    )
    
    assert result is True
    assert len(cost_manager.today_costs) == 1
    assert cost_manager.get_total_cost_today() == 0.05
    
    # Verify entry details
    entry = cost_manager.today_costs[0]
    assert entry.provider == "azure_openai"
    assert entry.model == "gpt-4"
    assert entry.tokens_input == 100
    assert entry.tokens_output == 50
    assert entry.cost_usd == 0.05


def test_multiple_cost_recordings(cost_manager):
    """Test recording multiple costs"""
    cost_manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.05, "test1")
    cost_manager.record_cost("openai", "gpt-3.5-turbo", 200, 100, 0.03, "test2")
    cost_manager.record_cost("anthropic", "claude-3", 150, 75, 0.04, "test3")
    
    assert len(cost_manager.today_costs) == 3
    assert cost_manager.get_total_cost_today() == 0.12


def test_disabled_cost_manager(disabled_cost_manager):
    """Test that disabled cost manager doesn't enforce limits"""
    result = disabled_cost_manager.record_cost(
        "azure_openai", "gpt-4", 1000000, 1000000, 10000.0, "test"
    )
    
    assert result is True  # Should allow even huge costs


# ============================================================================
# BUDGET ENFORCEMENT TESTS
# ============================================================================

def test_daily_budget_exceeded():
    """Test daily budget limit enforcement"""
    manager = CostManager(daily_budget_usd=1.0, enabled=True)
    
    # First request OK
    result = manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.5, "test1")
    assert result is True
    
    # Second request OK
    result = manager.record_cost("openai", "gpt-3.5", 100, 50, 0.3, "test2")
    assert result is True
    
    # Third request should exceed budget
    with pytest.raises(BudgetExceededException) as exc_info:
        manager.record_cost("anthropic", "claude-3", 100, 50, 0.5, "test3")
    
    assert exc_info.value.current > 1.0
    assert exc_info.value.limit == 1.0


def test_provider_budget_exceeded():
    """Test per-provider budget limit enforcement"""
    manager = CostManager(
        daily_budget_usd=100.0,
        per_provider_budgets={"azure_openai": 1.0},
        enabled=True
    )
    
    # First request OK
    result = manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.5, "test1")
    assert result is True
    
    # Second request should exceed provider budget
    with pytest.raises(ProviderBudgetExceededException) as exc_info:
        manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.8, "test2")
    
    assert exc_info.value.provider == "azure_openai"
    assert exc_info.value.current > 1.0
    assert exc_info.value.limit == 1.0


def test_budget_availability_check(cost_manager):
    """Test budget availability checking"""
    # Initially, budget should be available
    assert cost_manager.is_budget_available(0.05) is True
    
    # Add costs up to near limit
    cost_manager.record_cost("azure_openai", "gpt-4", 10000, 5000, 99.0, "test")
    
    # Small request should still fit
    assert cost_manager.is_budget_available(0.5) is True
    
    # Large request should not fit
    assert cost_manager.is_budget_available(5.0) is False


def test_provider_budget_availability(cost_manager):
    """Test provider-specific budget availability"""
    # Add cost close to azure_openai limit
    cost_manager.record_cost("azure_openai", "gpt-4", 10000, 5000, 49.0, "test")
    
    # Check azure_openai provider
    assert cost_manager.is_budget_available(0.5, "azure_openai") is True
    assert cost_manager.is_budget_available(2.0, "azure_openai") is False
    
    # Other providers should still have budget
    assert cost_manager.is_budget_available(5.0, "openai") is True


# ============================================================================
# COST BREAKDOWN TESTS
# ============================================================================

def test_cost_breakdown(cost_manager):
    """Test cost breakdown generation"""
    cost_manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.05, "dialogue")
    cost_manager.record_cost("openai", "gpt-3.5-turbo", 200, 100, 0.03, "decision")
    cost_manager.record_cost("azure_openai", "gpt-4", 150, 75, 0.06, "dialogue")
    
    breakdown = cost_manager.get_cost_breakdown()
    
    # Check totals
    assert breakdown["total_cost_usd"] == 0.14
    assert breakdown["budget_usd"] == 100.0
    assert breakdown["total_requests"] == 3
    
    # Check by provider
    assert breakdown["by_provider"]["azure_openai"] == 0.11
    assert breakdown["by_provider"]["openai"] == 0.03
    
    # Check by request type
    assert breakdown["by_request_type"]["dialogue"] == 0.11
    assert breakdown["by_request_type"]["decision"] == 0.03


def test_get_provider_cost(cost_manager):
    """Test getting cost for specific provider"""
    cost_manager.record_cost("azure_openai", "gpt-4", 100, 50, 0.05, "test")
    cost_manager.record_cost("openai", "gpt-3.5", 200, 100, 0.03, "test")
    cost_manager.record_cost("azure_openai", "gpt-4", 150, 75, 0.06, "test")
    
    assert cost_manager.get_provider_cost_today("azure_openai") == 0.11
    assert cost_manager.get_provider_cost_today("openai") == 0.03
    assert cost_manager.get_provider_cost_today("anthropic") == 0.0


def test_remaining_budget(cost_manager):
    """Test remaining budget calculation"""
    cost_manager.record_cost("azure_openai", "gpt-4", 100, 50, 25.0, "test")
    
    # Check total remaining
    assert cost_manager.get_remaining_budget() == 75.0
    
    # Check provider remaining
    assert cost_manager.get_remaining_budget("azure_openai") == 25.0
    assert cost_manager.get_remaining_budget("openai") == 30.0


# ============================================================================
# ALERT TESTS
# ============================================================================

def test_budget_alerts(cost_manager, caplog):
    """Test budget threshold alerts"""
    # 50% threshold
    cost_manager.record_cost("azure_openai", "gpt-4", 10000, 5000, 50.0, "test")
    assert "50% budget used" in caplog.text
    
    # 75% threshold
    cost_manager.record_cost("openai", "gpt-3.5", 5000, 2500, 25.0, "test")
    assert "75% budget used" in caplog.text
    
    # 90% threshold
    cost_manager.record_cost("anthropic", "claude-3", 3000, 1500, 15.0, "test")
    assert "90% budget used" in caplog.text


def test_no_duplicate_alerts(cost_manager, caplog):
    """Test that alerts are not duplicated"""
    caplog.clear()
    
    # First time hitting 50% - should alert
    cost_manager.record_cost("azure_openai", "gpt-4", 10000, 5000, 50.0, "test1")
    alert_count_1 = caplog.text.count("50% budget used")
    
    # Second time near 50% - should NOT alert again
    cost_manager.record_cost("openai", "gpt-3.5", 100, 50, 1.0, "test2")
    alert_count_2 = caplog.text.count("50% budget used")
    
    assert alert_count_1 == 1
    assert alert_count_2 == 1  # Same count, no new alert


# ============================================================================
# DAILY RESET TESTS
# ============================================================================

def test_daily_reset():
    """Test daily cost reset"""
    manager = CostManager(daily_budget_usd=100.0, enabled=True)
    
    # Add some costs
    manager.record_cost("azure_openai", "gpt-4", 100, 50, 50.0, "test")
    assert manager.get_total_cost_today() == 50.0
    
    # Simulate new day
    manager.current_date = date(2020, 1, 1)
    
    # Record cost (should trigger reset)
    manager.record_cost("openai", "gpt-3.5", 100, 50, 10.0, "test")
    
    # Should only have new cost
    assert manager.get_total_cost_today() == 10.0
    assert len(manager.today_costs) == 1


# ============================================================================
# BUDGET UPDATE TESTS
# ============================================================================

def test_set_daily_budget(cost_manager):
    """Test updating daily budget"""
    assert cost_manager.daily_budget == 100.0
    
    cost_manager.set_daily_budget(200.0)
    assert cost_manager.daily_budget == 200.0


def test_set_provider_budget(cost_manager):
    """Test updating provider budget"""
    assert cost_manager.per_provider_budgets["azure_openai"] == 50.0
    
    cost_manager.set_provider_budget("azure_openai", 75.0)
    assert cost_manager.per_provider_budgets["azure_openai"] == 75.0
    
    # Test adding new provider
    cost_manager.set_provider_budget("new_provider", 25.0)
    assert cost_manager.per_provider_budgets["new_provider"] == 25.0


# ============================================================================
# COST HISTORY TESTS
# ============================================================================

def test_get_cost_history(cost_manager):
    """Test retrieving cost history"""
    # Add multiple costs
    for i in range(5):
        cost_manager.record_cost(
            "azure_openai",
            "gpt-4",
            100,
            50,
            0.05,
            f"test{i}"
        )
    
    history = cost_manager.get_cost_history(limit=3)
    assert len(history) == 3
    
    # Should get most recent entries
    assert history[0]["request_type"] == "test2"
    assert history[1]["request_type"] == "test3"
    assert history[2]["request_type"] == "test4"
    
    # Test getting all
    history_all = cost_manager.get_cost_history(limit=100)
    assert len(history_all) == 5


# ============================================================================
# GLOBAL INSTANCE TESTS
# ============================================================================

def test_global_instance_initialization():
    """Test global cost manager initialization"""
    manager = init_cost_manager(
        daily_budget_usd=50.0,
        per_provider_budgets={"test_provider": 10.0}
    )
    
    assert manager.daily_budget == 50.0
    assert manager.per_provider_budgets["test_provider"] == 10.0
    
    # Test retrieval
    retrieved = get_cost_manager()
    assert retrieved is manager
    assert retrieved.daily_budget == 50.0


def test_get_cost_manager_not_initialized():
    """Test getting cost manager before initialization"""
    # Reset global instance
    import services.cost_manager
    services.cost_manager._cost_manager = None
    
    with pytest.raises(RuntimeError, match="Cost manager not initialized"):
        get_cost_manager()


# ============================================================================
# COST ENTRY TESTS
# ============================================================================

def test_cost_entry_to_dict():
    """Test CostEntry conversion to dictionary"""
    entry = CostEntry(
        timestamp=datetime(2025, 1, 1, 12, 0, 0),
        provider="azure_openai",
        model="gpt-4",
        tokens_input=100,
        tokens_output=50,
        cost_usd=0.05,
        request_type="test"
    )
    
    entry_dict = entry.to_dict()
    
    assert entry_dict["provider"] == "azure_openai"
    assert entry_dict["model"] == "gpt-4"
    assert entry_dict["tokens_input"] == 100
    assert entry_dict["tokens_output"] == 50
    assert entry_dict["cost_usd"] == 0.05
    assert "timestamp" in entry_dict


# ============================================================================
# EDGE CASES AND ERROR HANDLING
# ============================================================================

def test_zero_cost_recording(cost_manager):
    """Test recording zero cost"""
    result = cost_manager.record_cost(
        "ollama", "llama2", 100, 50, 0.0, "test"
    )
    assert result is True
    assert cost_manager.get_total_cost_today() == 0.0


def test_very_small_costs(cost_manager):
    """Test handling very small costs"""
    result = cost_manager.record_cost(
        "azure_openai", "gpt-4", 1, 1, 0.0001, "test"
    )
    assert result is True
    assert cost_manager.get_total_cost_today() == 0.0001


def test_provider_without_budget_limit(cost_manager):
    """Test provider without explicit budget limit"""
    # Ollama not in per_provider_budgets
    result = cost_manager.record_cost(
        "ollama", "llama2", 1000, 500, 50.0, "test"
    )
    
    # Should only check total budget, not provider budget
    assert result is True
    assert cost_manager.get_provider_cost_today("ollama") == 50.0