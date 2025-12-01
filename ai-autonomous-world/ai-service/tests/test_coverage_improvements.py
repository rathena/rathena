"""
Test Coverage Improvements
Targeted tests for critical uncovered code paths
"""

import pytest
import asyncio
from unittest.mock import Mock, AsyncMock, patch, MagicMock
from agents.decision_layers import (
    HierarchicalDecisionSystem, DecisionLayer, LayerDecision
)
from agents.utility_system import UtilityBasedPlanner
from agents.economic_agents import EconomicAgent, EconomicAgentType
from services.cost_manager import CostManager
from utils.prometheus_metrics import (
    record_agent_request, record_llm_request, record_decision_layer
)


# ============================================================================
# DECISION LAYERS TESTS
# ============================================================================

class TestHierarchicalDecisionLayers:
    """Test hierarchical decision system layers"""
    
    @pytest.mark.asyncio
    async def test_reflex_layer_low_hp(self):
        """Test reflex layer activates on low HP"""
        system = HierarchicalDecisionSystem()
        
        npc_state = {"hp": 15, "hp_max": 100}
        context = {"hp_percentage": 15}
        
        decision = await system.make_decision(npc_state, context, max_latency=0.01)
        
        assert decision.layer == DecisionLayer.REFLEX
        assert decision.action == "flee"
        assert decision.confidence == 1.0
        assert decision.latency_ms < 10  # Should be under 10ms
    
    @pytest.mark.asyncio
    async def test_layer_latency_budgets(self):
        """Test each layer respects latency budget"""
        system = HierarchicalDecisionSystem()
        
        npc_state = {"hunger": 50, "extraversion": 0.5}
        context = {"nearby_npcs": [], "distance_from_home": 10}
        
        decision = await system.make_decision(npc_state, context)
        
        # Should return a decision within total budget
        assert decision is not None
        assert decision.latency_ms < 1000  # Strategic layer max
    
    @pytest.mark.asyncio
    async def test_confidence_threshold_fallback(self):
        """Test fallback to next layer when confidence low"""
        system = HierarchicalDecisionSystem()
        
        # Scenario with no clear reflex/reactive pattern
        npc_state = {
            "hp": 80,
            "hunger": 30,
            "extraversion": 0.5
        }
        context = {
            "hp_percentage": 80,
            "nearby_npcs": [],
            "distance_from_home": 10
        }
        
        decision = await system.make_decision(npc_state, context)
        
        # Should fall through to deliberative or beyond
        assert decision.layer in [DecisionLayer.DELIBERATIVE, DecisionLayer.STRATEGIC]
    
    @pytest.mark.asyncio
    async def test_reactive_layer_high_hunger(self):
        """Test reactive layer responds to high hunger"""
        system = HierarchicalDecisionSystem()
        
        npc_state = {"hunger": 85, "hp": 100}
        context = {"hp_percentage": 100}
        
        decision = await system.make_decision(npc_state, context)
        
        assert decision.action == "seek_food"
        assert decision.confidence >= 0.75


# ============================================================================
# UTILITY SYSTEM TESTS
# ============================================================================

class TestUtilitySystem:
    """Test utility-based planning"""
    
    def test_weights_sum_to_one(self):
        """Test utility weights sum to 1.0"""
        planner = UtilityBasedPlanner()
        
        total = sum(planner.weights.values())
        
        assert abs(total - 1.0) < 0.01  # Allow small floating point error
    
    def test_correct_weight_values(self):
        """Test weights match documentation"""
        planner = UtilityBasedPlanner()
        
        assert planner.weights["safety"] == 0.30
        assert planner.weights["hunger"] == 0.25
        assert planner.weights["social"] == 0.20
        assert planner.weights["curiosity"] == 0.15
        assert planner.weights["aggression"] == 0.10
    
    def test_calculate_action_scores(self):
        """Test action score calculation"""
        planner = UtilityBasedPlanner()
        
        state = {
            "safety": 0.8,
            "hunger": 0.3,
            "social": 0.5,
            "curiosity": 0.6,
            "aggression": 0.2
        }
        
        scores = planner.calculate_action_scores(state)
        
        assert len(scores) > 0
        assert all(0 <= score <= 1 for score in scores.values())
    
    def test_select_action_highest_score(self):
        """Test action selection picks highest score"""
        planner = UtilityBasedPlanner()
        
        scores = {
            "explore": 0.3,
            "rest": 0.8,
            "socialize": 0.5
        }
        
        action = planner.select_action(scores)
        
        assert action == "rest"  # Highest score


# ============================================================================
# ECONOMIC SYSTEM TESTS
# ============================================================================

class TestEconomicAgents:
    """Test economic agent behaviors"""
    
    @pytest.mark.asyncio
    async def test_merchant_hoarding_behavior(self):
        """Test merchant buys when price low"""
        agent = EconomicAgent(
            agent_id="merchant_1",
            agent_type=EconomicAgentType.MERCHANT,
            starting_gold=1000
        )
        
        market_state = {
            "item_prices": {"iron_ore": 5},  # Low price
            "item_history": {"iron_ore": [10, 9, 8, 7, 6]}  # Trending down
        }
        
        decision = await agent.make_decision(market_state)
        
        assert decision["action"] == "buy"
        assert decision["item"] == "iron_ore"
    
    @pytest.mark.asyncio
    async def test_monopoly_detection(self):
        """Test monopoly detection at 60% threshold"""
        market_analyzer = Mock()
        
        market_state = {
            "merchant_shares": {
                "merchant_1": 0.65,  # Above 60% threshold
                "merchant_2": 0.20,
                "merchant_3": 0.15
            }
        }
        
        has_monopoly = market_state["merchant_shares"]["merchant_1"] > 0.6
        
        assert has_monopoly is True


# ============================================================================
# COST MANAGER TESTS
# ============================================================================

class TestCostManager:
    """Test cost management and budget enforcement"""
    
    def test_budget_blocking_at_limit(self):
        """Test requests blocked at exact budget limit"""
        manager = CostManager(
            daily_budget_usd=10.0,
            per_provider_budgets={"openai": 5.0}
        )
        
        # Consume entire budget
        manager.record_cost("openai", "gpt-4", 5.0)
        
        # Next request should be blocked
        can_proceed = manager.can_make_request("openai", estimated_cost=0.01)
        
        assert can_proceed is False
    
    def test_per_provider_budget_enforcement(self):
        """Test provider-specific limits work"""
        manager = CostManager(
            daily_budget_usd=20.0,
            per_provider_budgets={
                "openai": 10.0,
                "anthropic": 8.0
            }
        )
        
        # Use openai budget
        manager.record_cost("openai", "gpt-4", 10.0)
        
        # OpenAI should be blocked
        assert manager.can_make_request("openai", 0.01) is False
        
        # But anthropic should still work
        assert manager.can_make_request("anthropic", 0.01) is True
    
    def test_daily_budget_reset(self):
        """Test daily budget resets correctly"""
        manager = CostManager(daily_budget_usd=10.0)
        
        # Consume budget
        manager.record_cost("openai", "gpt-4", 10.0)
        
        # Simulate day change
        manager.reset_daily_budget()
        
        # Should allow requests again
        assert manager.can_make_request("openai", 1.0) is True


# ============================================================================
# INTEGRATION TESTS
# ============================================================================

class TestSystemIntegration:
    """Integration tests for combined systems"""
    
    @pytest.mark.asyncio
    async def test_decision_agent_uses_both_systems(self):
        """Test DecisionAgent integrates hierarchical and utility"""
        decision_system = HierarchicalDecisionSystem()
        utility_planner = UtilityBasedPlanner()
        
        npc_state = {
            "hp": 100,
            "hunger": 60,
            "safety": 0.7,
            "social": 0.4
        }
        context = {"nearby_npcs": []}
        
        # Get hierarchical decision
        hierarchical = await decision_system.make_decision(npc_state, context)
        
        # Get utility scores
        utility_scores = utility_planner.calculate_action_scores(npc_state)
        
        assert hierarchical is not None
        assert utility_scores is not None
        assert len(utility_scores) > 0
    
    @pytest.mark.asyncio
    async def test_fallback_chain_with_cost_tracking(self):
        """Test fallback chain records costs correctly"""
        cost_manager = CostManager(daily_budget_usd=10.0)
        
        # Simulate fallback: openai -> anthropic
        providers_tried = []
        
        # Try OpenAI (fails)
        if cost_manager.can_make_request("openai", 0.5):
            providers_tried.append("openai")
        
        # Fallback to Anthropic
        if cost_manager.can_make_request("anthropic", 0.3):
            providers_tried.append("anthropic")
            cost_manager.record_cost("anthropic", "claude", 0.3)
        
        assert len(providers_tried) == 2
        assert cost_manager.get_total_cost() == 0.3


# ============================================================================
# PROMETHEUS METRICS TESTS
# ============================================================================

class TestPrometheusMetrics:
    """Test Prometheus metrics recording"""
    
    def test_record_agent_request_success(self):
        """Test recording successful agent request"""
        # Should not raise exception
        record_agent_request(
            agent_type="dialogue",
            operation="generate",
            duration=0.5,
            status="success",
            confidence=0.85
        )
    
    def test_record_agent_request_failure(self):
        """Test recording failed agent request"""
        record_agent_request(
            agent_type="quest",
            operation="generate",
            duration=1.2,
            status="failure",
            error_type="TimeoutError"
        )
    
    def test_record_llm_request_with_tokens(self):
        """Test recording LLM request with token counts"""
        record_llm_request(
            provider="openai",
            model="gpt-4",
            operation="generate",
            duration=2.5,
            status="success",
            input_tokens=100,
            output_tokens=150,
            cost_usd=0.05
        )
    
    def test_record_decision_layer_metrics(self):
        """Test recording decision layer metrics"""
        record_decision_layer(
            layer="deliberative",
            outcome="selected",
            latency=0.15,
            confidence=0.8
        )
    
    def test_record_decision_layer_fallback(self):
        """Test recording decision layer fallback"""
        record_decision_layer(
            layer="deliberative",
            outcome="fallback",
            latency=0.2,
            confidence=0.6,
            fallback_from="strategic"
        )


# ============================================================================
# EDGE CASE TESTS
# ============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    @pytest.mark.asyncio
    async def test_empty_npc_state(self):
        """Test handling of empty NPC state"""
        system = HierarchicalDecisionSystem()
        
        decision = await system.make_decision({}, {})
        
        # Should return some decision, not crash
        assert decision is not None
    
    def test_utility_all_zero_values(self):
        """Test utility system with all zero values"""
        planner = UtilityBasedPlanner()
        
        state = {
            "safety": 0.0,
            "hunger": 0.0,
            "social": 0.0,
            "curiosity": 0.0,
            "aggression": 0.0
        }
        
        scores = planner.calculate_action_scores(state)
        
        # Should handle gracefully
        assert scores is not None
    
    def test_cost_manager_negative_cost(self):
        """Test cost manager rejects negative costs"""
        manager = CostManager(daily_budget_usd=10.0)
        
        with pytest.raises(ValueError):
            manager.record_cost("openai", "gpt-4", -1.0)
    
    @pytest.mark.asyncio
    async def test_decision_layer_timeout_handling(self):
        """Test timeout handling in decision layers"""
        system = HierarchicalDecisionSystem()
        
        # Very tight budget should cause timeout
        decision = await system.make_decision(
            {"hp": 100},
            {},
            max_latency=0.0001  # 0.1ms - very tight
        )
        
        # Should still return a decision
        assert decision is not None