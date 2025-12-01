"""
Test suite for EconomyAgent - Market dynamics and economy management.

Tests cover:
- Dynamic price calculations
- Supply and demand mechanics
- Market condition assessment
- Price trend analysis
- Market updates
"""

import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
import pytest

from agents.base_agent import AgentContext, AgentStatus
from agents.economy_agent import (
    EconomyAgent,
    PriceTrend,
    MarketCondition
)
from models.world import WorldState, EconomicState


# ============================================================================
# FIXTURES
# ============================================================================

@pytest.fixture
def economy_agent():
    """Create economy agent instance."""
    return EconomyAgent(
        base_inflation_rate=0.01,
        price_volatility=0.1
    )


@pytest.fixture
def sample_economy_state():
    """Create sample economy state."""
    economy = EconomicState(
        item_prices={
            "health_potion": 50.0,
            "iron_sword": 150.0,
            "rare_gem": 1000.0
        },
        item_supply={
            "health_potion": 100,
            "iron_sword": 50,
            "rare_gem": 10
        },
        item_demand={
            "health_potion": 0.8,
            "iron_sword": 0.5,
            "rare_gem": 0.9
        },
        price_history={
            "health_potion": [45.0, 48.0, 50.0],
            "iron_sword": [150.0, 148.0, 150.0]
        },
        inflation_rate=0.02,
        average_player_wealth=5000.0
    )
    return economy


@pytest.fixture
def sample_world_state(sample_economy_state):
    """Create sample world state with economy."""
    return WorldState(
        world_id="test_world",
        world_name="Test World",
        economy=sample_economy_state,
        active_players=50,
        active_npcs=100
    )


@pytest.fixture
def economy_context(sample_world_state, faker_instance):
    """Create agent context for economy operations."""
    return AgentContext(
        request_id=faker_instance.uuid4(),
        npc_id=faker_instance.uuid4(),
        npc_state=MagicMock(),
        world_state=sample_world_state,
        location="market",
        additional_data={}
    )


# ============================================================================
# INITIALIZATION TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_economy_agent_initialization():
    """Test economy agent initializes correctly."""
    agent = EconomyAgent(
        base_inflation_rate=0.01,
        price_volatility=0.15
    )
    
    await agent.initialize()
    
    assert agent.name == "EconomyAgent"
    assert agent.base_inflation_rate == 0.01
    assert agent.price_volatility == 0.15
    assert len(agent._base_prices) > 0
    
    await agent.cleanup()


@pytest.mark.unit
def test_base_prices_initialized(economy_agent):
    """Test base prices are properly initialized."""
    base_prices = economy_agent._base_prices
    
    assert "health_potion" in base_prices
    assert "iron_sword" in base_prices
    assert "rare_gem" in base_prices
    assert all(price > 0 for price in base_prices.values())


# ============================================================================
# PRICE CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_calculate_price_basic(economy_agent, economy_context):
    """Test basic price calculation."""
    await economy_agent.initialize()
    
    economy_context.additional_data = {
        "operation": "calculate_price",
        "item_id": "health_potion"
    }
    
    response = await economy_agent.process(economy_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "new_price" in response.result
    assert "modifiers" in response.result
    assert response.result["new_price"] > 0
    
    await economy_agent.cleanup()


@pytest.mark.unit
def test_supply_demand_modifier_high_demand_low_supply(economy_agent):
    """Test price modifier with high demand and low supply."""
    # High demand, low supply = higher prices
    modifier = economy_agent._calculate_supply_demand_modifier(
        supply=20,  # Low supply
        demand=0.9  # High demand
    )
    
    assert modifier > 1.0  # Prices should increase


@pytest.mark.unit
def test_supply_demand_modifier_low_demand_high_supply(economy_agent):
    """Test price modifier with low demand and high supply."""
    # Low demand, high supply = lower prices
    modifier = economy_agent._calculate_supply_demand_modifier(
        supply=200,  # High supply
        demand=0.2   # Low demand
    )
    
    assert modifier < 1.0  # Prices should decrease


@pytest.mark.unit
def test_supply_demand_modifier_balanced(economy_agent):
    """Test price modifier with balanced supply/demand."""
    modifier = economy_agent._calculate_supply_demand_modifier(
        supply=100,  # Normal supply
        demand=0.5   # Normal demand
    )
    
    assert 0.8 < modifier < 1.2  # Should be close to neutral


@pytest.mark.unit
def test_supply_demand_modifier_no_supply(economy_agent):
    """Test price modifier with zero supply."""
    modifier = economy_agent._calculate_supply_demand_modifier(
        supply=0,     # No supply
        demand=0.8
    )
    
    assert modifier == 2.0  # Maximum modifier


# ============================================================================
# MARKET CONDITION TESTS
# ============================================================================

@pytest.mark.unit
def test_assess_market_condition_boom(economy_agent, sample_world_state):
    """Test market condition assessment - boom."""
    sample_world_state.economy.inflation_rate = 0.12
    
    condition = economy_agent._assess_market_condition(sample_world_state)
    
    assert condition == MarketCondition.BOOM


@pytest.mark.unit
def test_assess_market_condition_stable(economy_agent, sample_world_state):
    """Test market condition assessment - stable."""
    sample_world_state.economy.inflation_rate = 0.02
    
    condition = economy_agent._assess_market_condition(sample_world_state)
    
    assert condition == MarketCondition.STABLE


@pytest.mark.unit
def test_assess_market_condition_crash(economy_agent, sample_world_state):
    """Test market condition assessment - crash."""
    sample_world_state.economy.inflation_rate = -0.15
    
    condition = economy_agent._assess_market_condition(sample_world_state)
    
    assert condition == MarketCondition.CRASH


@pytest.mark.unit
def test_get_market_modifier(economy_agent):
    """Test market condition price modifiers."""
    assert economy_agent._get_market_modifier(MarketCondition.BOOM) > 1.0
    assert economy_agent._get_market_modifier(MarketCondition.STABLE) == 1.0
    assert economy_agent._get_market_modifier(MarketCondition.CRASH) < 1.0


# ============================================================================
# MARKET UPDATE TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_update_market(economy_agent, economy_context):
    """Test market update operation."""
    await economy_agent.initialize()
    
    economy_context.additional_data["operation"] = "update_market"
    
    response = await economy_agent.process(economy_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "updated_items" in response.result
    assert "inflation_rate" in response.result
    assert "market_condition" in response.result
    assert response.result["updated_items"] > 0
    
    await economy_agent.cleanup()


# ============================================================================
# MARKET ANALYSIS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_analyze_market(economy_agent, economy_context):
    """Test market analysis operation."""
    await economy_agent.initialize()
    
    economy_context.additional_data["operation"] = "analyze_market"
    
    response = await economy_agent.process(economy_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "market_condition" in response.result
    assert "inflation_rate" in response.result
    assert "trend_summary" in response.result
    assert "opportunities" in response.result
    
    await economy_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_market_opportunities_identification(economy_agent, economy_context):
    """Test identification of market opportunities."""
    await economy_agent.initialize()
    
    # Set up trends
    economy_context.world_state.economy.price_history = {
        "health_potion": [60.0, 55.0, 50.0],  # Falling - buy opportunity
        "iron_sword": [140.0, 150.0, 160.0],  # Rising - sell opportunity
    }
    
    economy_context.additional_data["operation"] = "analyze_market"
    response = await economy_agent.process(economy_context)
    
    opportunities = response.result["opportunities"]
    assert len(opportunities) > 0
    
    # Check for buy/sell recommendations
    types = [opp["type"] for opp in opportunities]
    assert "buy" in types or "sell" in types
    
    await economy_agent.cleanup()


# ============================================================================
# PRICE TREND TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_get_price_trend(economy_agent, economy_context):
    """Test price trend retrieval."""
    await economy_agent.initialize()
    
    economy_context.additional_data = {
        "operation": "get_trend",
        "item_id": "health_potion"
    }
    
    response = await economy_agent.process(economy_context)
    
    assert response.status == AgentStatus.COMPLETED
    assert "trend" in response.result
    assert "statistics" in response.result
    assert "price_history" in response.result
    
    stats = response.result["statistics"]
    assert "average" in stats
    assert "min" in stats
    assert "max" in stats
    assert "volatility" in stats
    
    await economy_agent.cleanup()


# ============================================================================
# COST CALCULATION TESTS
# ============================================================================

@pytest.mark.unit
def test_calculate_cost(economy_agent):
    """Test cost calculation."""
    cost = economy_agent._calculate_cost(1000)
    
    # Should be based on token pricing
    assert cost >= 0
    assert isinstance(cost, float)


# ============================================================================
# PRICE BOUNDS TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_price_bounds_enforcement(economy_agent, economy_context):
    """Test that prices stay within reasonable bounds."""
    await economy_agent.initialize()
    
    # Set extreme conditions
    economy_context.world_state.economy.item_supply["health_potion"] = 1
    economy_context.world_state.economy.item_demand["health_potion"] = 1.0
    economy_context.world_state.economy.inflation_rate = 0.5
    
    economy_context.additional_data = {
        "operation": "calculate_price",
        "item_id": "health_potion"
    }
    
    response = await economy_agent.process(economy_context)
    
    base_price = economy_agent._base_prices["health_potion"]
    new_price = response.result["new_price"]
    
    # Price should not deviate more than 5x from base
    assert new_price >= base_price * 0.2
    assert new_price <= base_price * 5.0
    
    await economy_agent.cleanup()


# ============================================================================
# ERROR HANDLING TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.asyncio
async def test_invalid_operation(economy_agent, economy_context):
    """Test handling of invalid operations."""
    await economy_agent.initialize()
    
    economy_context.additional_data["operation"] = "invalid_operation"
    response = await economy_agent.process(economy_context)
    
    assert response.status == AgentStatus.FAILED
    assert response.error is not None
    
    await economy_agent.cleanup()


@pytest.mark.unit
@pytest.mark.asyncio
async def test_missing_item_id(economy_agent, economy_context):
    """Test handling of missing item ID."""
    await economy_agent.initialize()
    
    economy_context.additional_data = {
        "operation": "calculate_price"
        # Missing item_id
    }
    
    response = await economy_agent.process(economy_context)
    
    # Should use default empty string and default price
    assert response.status == AgentStatus.COMPLETED
    
    await economy_agent.cleanup()


# ============================================================================
# PARAMETRIZED TESTS
# ============================================================================

@pytest.mark.unit
@pytest.mark.parametrize("inflation_rate,expected_condition", [
    (0.12, MarketCondition.BOOM),
    (0.07, MarketCondition.GROWTH),
    (0.02, MarketCondition.STABLE),
    (-0.07, MarketCondition.RECESSION),
    (-0.15, MarketCondition.CRASH),
])
def test_market_conditions(economy_agent, sample_world_state, inflation_rate, expected_condition):
    """Test market condition mapping."""
    sample_world_state.economy.inflation_rate = inflation_rate
    
    condition = economy_agent._assess_market_condition(sample_world_state)
    
    assert condition == expected_condition


@pytest.mark.unit
@pytest.mark.parametrize("supply,demand,expected_trend", [
    (20, 0.9, "higher"),   # Low supply, high demand
    (200, 0.2, "lower"),   # High supply, low demand
    (100, 0.5, "neutral"), # Balanced
])
def test_supply_demand_trends(economy_agent, supply, demand, expected_trend):
    """Test supply/demand impact on prices."""
    modifier = economy_agent._calculate_supply_demand_modifier(supply, demand)
    
    if expected_trend == "higher":
        assert modifier > 1.1
    elif expected_trend == "lower":
        assert modifier < 0.9
    else:  # neutral
        assert 0.9 <= modifier <= 1.1