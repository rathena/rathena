#!/usr/bin/env python3
"""
Integration Verification Script

Verifies that all new systems are properly integrated and functional.
Tests imports, instantiation, and basic functionality of all components.
"""

import sys
import os
import asyncio
from typing import List, Tuple
from pathlib import Path

# Add parent directory to Python path
script_dir = Path(__file__).parent
ai_service_dir = script_dir.parent
sys.path.insert(0, str(ai_service_dir))

from loguru import logger

# Configure logger for script
logger.remove()
logger.add(sys.stdout, format="<level>{message}</level>", level="INFO")


class IntegrationVerifier:
    """Verifies system integration completeness"""
    
    def __init__(self):
        self.results: List[Tuple[str, bool, str]] = []
    
    def check(self, name: str, func) -> bool:
        """Run a check and record result"""
        try:
            func()
            self.results.append((name, True, "✅ PASS"))
            logger.info(f"✅ {name}")
            return True
        except Exception as e:
            self.results.append((name, False, f"❌ FAIL: {e}"))
            logger.error(f"❌ {name}: {e}")
            return False
    
    async def async_check(self, name: str, func) -> bool:
        """Run async check and record result"""
        try:
            await func()
            self.results.append((name, True, "✅ PASS"))
            logger.info(f"✅ {name}")
            return True
        except Exception as e:
            self.results.append((name, False, f"❌ FAIL: {e}"))
            logger.error(f"❌ {name}: {e}")
            return False
    
    def print_summary(self):
        """Print final summary"""
        print("\n" + "="*80)
        print("INTEGRATION VERIFICATION SUMMARY")
        print("="*80 + "\n")
        
        passed = sum(1 for _, success, _ in self.results if success)
        failed = len(self.results) - passed
        
        for name, success, message in self.results:
            if not success:
                print(f"❌ {name}: {message}")
        
        print(f"\n{'='*80}")
        print(f"Total: {passed}/{len(self.results)} checks passed")
        
        if failed == 0:
            print("🎉 ALL INTEGRATION CHECKS PASSED - 100% COMPLETE!")
            print("="*80 + "\n")
            return 0
        else:
            print(f"⚠️  {failed} checks failed - review issues above")
            print("="*80 + "\n")
            return 1


async def main():
    """Run all integration verifications"""
    verifier = IntegrationVerifier()
    
    print("\n🚀 rAthena AI World - Integration Verification\n")
    print("="*80)
    print("Verifying all systems are properly integrated...")
    print("="*80 + "\n")
    
    # ========================================================================
    # 1. IMPORT CHECKS
    # ========================================================================
    
    print("📦 Phase 1: Import Verification\n")
    
    verifier.check(
        "Import HierarchicalDecisionSystem",
        lambda: __import__('agents.decision_layers', fromlist=['HierarchicalDecisionSystem'])
    )
    
    verifier.check(
        "Import UtilityBasedPlanner",
        lambda: __import__('agents.utility_system', fromlist=['UtilityBasedPlanner'])
    )
    
    verifier.check(
        "Import EconomicSimulation",
        lambda: __import__('agents.economic_simulation', fromlist=['EconomicSimulation'])
    )
    
    verifier.check(
        "Import EconomicAgent",
        lambda: __import__('agents.economic_agents', fromlist=['EconomicAgent'])
    )
    
    verifier.check(
        "Import ProductionChain",
        lambda: __import__('models.production_chain', fromlist=['ProductionChain'])
    )
    
    verifier.check(
        "Import TradeRoute",
        lambda: __import__('models.trade_route', fromlist=['TradeRoute'])
    )
    
    verifier.check(
        "Import FallbackLLMProvider",
        lambda: __import__('llm.fallback_provider', fromlist=['FallbackLLMProvider'])
    )
    
    verifier.check(
        "Import CostManager",
        lambda: __import__('services.cost_manager', fromlist=['CostManager'])
    )
    
    # ========================================================================
    # 2. INSTANTIATION CHECKS
    # ========================================================================
    
    print("\n🔧 Phase 2: Component Instantiation\n")
    
    def test_hierarchical_system():
        from agents.decision_layers import HierarchicalDecisionSystem
        system = HierarchicalDecisionSystem()
        assert system is not None
        assert len(system.max_latencies) == 5
    
    verifier.check("Instantiate HierarchicalDecisionSystem", test_hierarchical_system)
    
    def test_utility_planner():
        from agents.utility_system import UtilityBasedPlanner
        planner = UtilityBasedPlanner()
        assert planner is not None
        assert abs(sum(planner.weights.values()) - 1.0) < 0.01
    
    verifier.check("Instantiate UtilityBasedPlanner", test_utility_planner)
    
    def test_economic_simulation():
        from agents.economic_simulation import EconomicSimulation
        sim = EconomicSimulation()
        assert sim is not None
        assert len(sim.production_chains) > 0
        assert len(sim.trade_network.routes) > 0
        assert len(sim.resources) > 0
        assert len(sim.innovations) > 0
    
    verifier.check("Instantiate EconomicSimulation", test_economic_simulation)
    
    def test_cost_manager():
        from services.cost_manager import CostManager
        cm = CostManager(daily_budget_usd=100.0)
        assert cm is not None
        assert cm.daily_budget == 100.0
    
    verifier.check("Instantiate CostManager", test_cost_manager)
    
    # ========================================================================
    # 3. INTEGRATION CHECKS
    # ========================================================================
    
    print("\n🔗 Phase 3: Integration Verification\n")
    
    def test_decision_agent_integration():
        from agents.decision_agent import DecisionAgent
        from llm.factory import get_llm_provider
        
        # Create with minimal config
        agent = DecisionAgent(
            agent_id="test_decision",
            llm_provider=None,  # Will use fallback chain
            config={}
        )
        
        # Verify new systems initialized
        assert hasattr(agent, 'hierarchical_system')
        assert hasattr(agent, 'utility_planner')
        assert agent.hierarchical_system is not None
        assert agent.utility_planner is not None
    
    verifier.check("DecisionAgent Integration", test_decision_agent_integration)
    
    def test_economy_agent_integration():
        from agents.economy_agent import EconomyAgent
        
        agent = EconomyAgent(
            agent_id="test_economy",
            llm_provider=None,
            config={}
        )
        
        # Verify economic simulation initialized
        assert hasattr(agent, 'economic_simulation')
        assert hasattr(agent, 'game_statistics')
        assert agent.economic_simulation is not None
        assert len(agent.economic_simulation.production_chains) > 0
    
    verifier.check("EconomyAgent Integration", test_economy_agent_integration)
    
    def test_base_agent_fallback():
        from agents.base_agent import BaseAIAgent
        from agents.dialogue_agent import DialogueAgent
        
        # Create agent without providing llm_provider
        agent = DialogueAgent(
            agent_id="test_dialogue",
            llm_provider=None,  # Should use fallback chain
            config={}
        )
        
        # Verify fallback provider initialized
        assert agent.llm_provider is not None
        # Check if it's a fallback provider
        assert hasattr(agent.llm_provider, 'providers') or hasattr(agent.llm_provider, 'generate')
    
    verifier.check("BaseAgent Fallback Chain", test_base_agent_fallback)
    
    # ========================================================================
    # 4. FUNCTIONALITY CHECKS
    # ========================================================================
    
    print("\n⚡ Phase 4: Functionality Tests\n")
    
    async def test_hierarchical_decision():
        from agents.decision_layers import HierarchicalDecisionSystem, DecisionLayer
        
        system = HierarchicalDecisionSystem()
        
        npc_state = {
            "npc_id": "test_npc",
            "personality": {"curiosity": 0.8},
            "hunger": 50,
            "social_need": 30
        }
        
        context = {
            "hp_percentage": 90,
            "nearby_npcs": [],
            "time_of_day": "day"
        }
        
        decision = await system.make_decision(npc_state, context, max_latency=0.5)
        assert decision is not None
        assert decision.layer in DecisionLayer
        assert decision.action is not None
    
    await verifier.async_check("Hierarchical Decision Making", test_hierarchical_decision)
    
    def test_utility_evaluation():
        from agents.utility_system import UtilityBasedPlanner
        
        planner = UtilityBasedPlanner()
        
        npc_state = {"personality": {"extraversion": 0.8}, "hunger": 70}
        context = {"nearby_npcs": [1, 2], "hp_percentage": 100}
        
        action, utility = planner.select_best_action(
            ["idle", "social_interaction", "seek_food"],
            npc_state,
            context
        )
        
        assert action is not None
        assert 0.0 <= utility <= 1.0
    
    verifier.check("Utility-Based Selection", test_utility_evaluation)
    
    async def test_economic_simulation():
        from agents.economic_simulation import EconomicSimulation
        
        sim = EconomicSimulation()
        
        market_state = {
            "iron_ore": {"price": 100, "supply": 1000, "demand": 800},
            "coal": {"price": 50, "supply": 800, "demand": 600}
        }
        
        # Test production
        result = await sim.simulate_production_chains(market_state)
        assert result is not None
        assert "chains_executed" in result
    
    await verifier.async_check("Economic Simulation", test_economic_simulation)
    
    def test_cost_tracking():
        from services.cost_manager import CostManager
        
        cm = CostManager(daily_budget_usd=10.0)
        
        # Record cost
        cm.record_cost(
            provider="azure_openai",
            model="gpt-4",
            tokens_input=100,
            tokens_output=50,
            cost_usd=0.002,
            request_type="test"
        )
        
        # Verify tracking
        total = cm.get_total_cost_today()
        assert total == 0.002
        
        breakdown = cm.get_cost_breakdown()
        assert breakdown["total_cost_usd"] == 0.002
    
    verifier.check("Cost Tracking", test_cost_tracking)
    
    # ========================================================================
    # 5. EXPORTS VERIFICATION
    # ========================================================================
    
    print("\n📤 Phase 5: Module Exports Verification\n")
    
    def test_agents_exports():
        import agents
        assert hasattr(agents, 'HierarchicalDecisionSystem')
        assert hasattr(agents, 'UtilityBasedPlanner')
        assert hasattr(agents, 'EconomicSimulation')
        assert hasattr(agents, 'EconomicAgent')
    
    verifier.check("agents/__init__.py exports", test_agents_exports)
    
    def test_services_exports():
        import services
        assert hasattr(services, 'CostManager')
        assert hasattr(services, 'get_cost_manager')
        assert hasattr(services, 'init_cost_manager')
    
    verifier.check("services/__init__.py exports", test_services_exports)
    
    def test_llm_exports():
        import llm
        assert hasattr(llm, 'FallbackLLMProvider')
        assert hasattr(llm, 'get_llm_provider_with_fallback')
    
    verifier.check("llm/__init__.py exports", test_llm_exports)
    
    def test_models_exports():
        import models
        assert hasattr(models, 'ProductionChain')
        assert hasattr(models, 'TradeRoute')
    
    verifier.check("models/__init__.py exports", test_models_exports)
    
    # ========================================================================
    # FINAL SUMMARY
    # ========================================================================
    
    return verifier.print_summary()


if __name__ == "__main__":
    try:
        exit_code = asyncio.run(main())
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print("\n\n⚠️  Verification interrupted by user")
        sys.exit(1)
    except Exception as e:
        logger.error(f"\n❌ Verification script failed: {e}")
        sys.exit(1)