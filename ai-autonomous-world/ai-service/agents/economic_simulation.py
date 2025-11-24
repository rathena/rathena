"""
Economic Simulation Extensions
Resource depletion, innovation cycles, and complete economic behaviors
"""

from typing import Dict, List, Optional, Any
from dataclasses import dataclass
from datetime import datetime, timedelta
from enum import Enum
import random
from loguru import logger

from models.production_chain import ProductionChain, PRODUCTION_CHAINS
from models.trade_route import TradeRoute, TradeNetwork, MAJOR_ROUTES
from agents.economic_agents import EconomicAgent, EconomicAgentType


class ResourceState(str, Enum):
    """State of a resource"""
    ABUNDANT = "abundant"
    NORMAL = "normal"
    SCARCE = "scarce"
    DEPLETED = "depleted"


@dataclass
class Resource:
    """Depletable resource"""
    resource_id: str
    name: str
    total_reserves: float
    current_reserves: float
    extraction_rate: float = 0.0
    regeneration_rate: float = 0.0
    depletion_rate: float = 0.0
    state: ResourceState = ResourceState.NORMAL
    last_updated: datetime = None
    
    def __post_init__(self):
        if self.last_updated is None:
            self.last_updated = datetime.utcnow()
    
    def extract(self, amount: float) -> float:
        """
        Extract resource
        
        Returns:
            Actual amount extracted
        """
        actual = min(amount, self.current_reserves)
        self.current_reserves -= actual
        self.extraction_rate = actual
        self._update_state()
        return actual
    
    def regenerate(self, delta_seconds: float) -> None:
        """Natural resource regeneration"""
        regen_amount = self.regeneration_rate * (delta_seconds / 3600.0)
        self.current_reserves = min(
            self.total_reserves,
            self.current_reserves + regen_amount
        )
        self._update_state()
    
    def _update_state(self) -> None:
        """Update resource state based on current reserves"""
        ratio = self.current_reserves / max(self.total_reserves, 1)
        
        if ratio >= 0.8:
            self.state = ResourceState.ABUNDANT
        elif ratio >= 0.4:
            self.state = ResourceState.NORMAL
        elif ratio >= 0.1:
            self.state = ResourceState.SCARCE
        else:
            self.state = ResourceState.DEPLETED
        
        self.depletion_rate = 1.0 - ratio


@dataclass
class Innovation:
    """Technological innovation"""
    innovation_id: str
    name: str
    description: str
    unlock_conditions: Dict[str, Any]
    effects: Dict[str, float]  # {effect_type: multiplier}
    cost: float
    discovered: bool = False
    discovered_at: Optional[datetime] = None
    discovered_by: Optional[str] = None  # NPC ID


class EconomicSimulation:
    """
    Complete economic simulation system
    
    Manages:
    - Production chains
    - Trade routes
    - Economic agents
    - Resource depletion
    - Innovation cycles
    - Market behaviors
    """
    
    def __init__(self):
        self.production_chains: Dict[str, ProductionChain] = PRODUCTION_CHAINS.copy()
        self.trade_network = TradeNetwork(
            network_id="main_network",
            routes=MAJOR_ROUTES.copy()
        )
        self.economic_agents: Dict[str, EconomicAgent] = {}
        self.resources: Dict[str, Resource] = {}
        self.innovations: Dict[str, Innovation] = {}
        self.market_state: Dict[str, Any] = {}
        
        self._initialize_resources()
        self._initialize_innovations()
        
        logger.info("Economic Simulation initialized")
    
    def _initialize_resources(self) -> None:
        """Initialize depletable resources"""
        self.resources = {
            "iron_ore": Resource(
                resource_id="iron_ore",
                name="Iron Ore",
                total_reserves=10000.0,
                current_reserves=10000.0,
                regeneration_rate=10.0  # per hour
            ),
            "coal": Resource(
                resource_id="coal",
                name="Coal",
                total_reserves=8000.0,
                current_reserves=8000.0,
                regeneration_rate=5.0
            ),
            "wood": Resource(
                resource_id="wood",
                name="Wood",
                total_reserves=15000.0,
                current_reserves=15000.0,
                regeneration_rate=50.0  # Fast growing
            ),
            "herbs": Resource(
                resource_id="herbs",
                name="Medicinal Herbs",
                total_reserves=5000.0,
                current_reserves=5000.0,
                regeneration_rate=20.0
            )
        }
    
    def _initialize_innovations(self) -> None:
        """Initialize innovation tree"""
        self.innovations = {
            "advanced_smelting": Innovation(
                innovation_id="advanced_smelting",
                name="Advanced Smelting Techniques",
                description="Reduces iron ore consumption by 20%",
                unlock_conditions={"total_iron_smelted": 1000},
                effects={"iron_ore_efficiency": 1.2},
                cost=5000.0
            ),
            "sustainable_forestry": Innovation(
                innovation_id="sustainable_forestry",
                name="Sustainable Forestry",
                description="Increases wood regeneration by 50%",
                unlock_conditions={"wood_depleted_count": 3},
                effects={"wood_regeneration": 1.5},
                cost=3000.0
            ),
            "alchemy_mastery": Innovation(
                innovation_id="alchemy_mastery",
                name="Alchemy Mastery",
                description="Increases potion production success rate",
                unlock_conditions={"potions_crafted": 500},
                effects={"potion_success_rate": 1.1},
                cost=4000.0
            ),
            "trade_network_expansion": Innovation(
                innovation_id="trade_network_expansion",
                name="Trade Network Expansion",
                description="Reduces transport costs by 30%",
                unlock_conditions={"total_trades": 1000},
                effects={"transport_cost": 0.7},
                cost=10000.0
            )
        }
    
    def add_economic_agent(
        self, npc_id: str, agent_type: EconomicAgentType
    ) -> EconomicAgent:
        """Add new economic agent"""
        agent = EconomicAgent(agent_type, npc_id)
        self.economic_agents[npc_id] = agent
        logger.info(f"Added {agent_type.value} agent: {npc_id}")
        return agent
    
    async def simulate_production_chains(
        self, market_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Simulate production from raw materials to finished goods
        
        Returns:
            Production results
        """
        results = {
            "chains_executed": 0,
            "items_produced": {},
            "resources_consumed": {}
        }
        
        for chain_id, chain in self.production_chains.items():
            # Check if all inputs available
            can_produce = True
            required_inputs = chain.get_all_required_inputs()
            
            for item_id, qty in required_inputs.items():
                # Check resource availability
                if item_id in self.resources:
                    resource = self.resources[item_id]
                    if resource.current_reserves < qty:
                        can_produce = False
                        logger.debug(f"Chain {chain_id}: Insufficient {item_id}")
                        break
                # Check market supply
                elif market_state.get(item_id, {}).get('supply', 0) < qty:
                    can_produce = False
                    break
            
            if can_produce:
                # Execute production
                for step in chain.steps:
                    # Consume inputs
                    for item_id, qty in step.inputs.items():
                        if item_id in self.resources:
                            actual = self.resources[item_id].extract(qty)
                            results["resources_consumed"][item_id] = \
                                results["resources_consumed"].get(item_id, 0) + actual
                        else:
                            market_state[item_id]['supply'] -= qty
                    
                    # Produce outputs (with success rate)
                    if random.random() <= step.success_rate:
                        for item_id, qty in step.outputs.items():
                            if item_id not in market_state:
                                market_state[item_id] = {'supply': 0, 'demand': 0}
                            market_state[item_id]['supply'] += qty
                            results["items_produced"][item_id] = \
                                results["items_produced"].get(item_id, 0) + qty
                
                results["chains_executed"] += 1
                logger.debug(f"Production chain {chain_id} executed")
        
        return results
    
    async def simulate_trade_routes(
        self, market_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Simulate trade between locations
        
        Returns:
            Trade results
        """
        results = {
            "routes_active": 0,
            "goods_transferred": {},
            "profit_generated": 0.0
        }
        
        for route in self.trade_network.routes:
            if not route.can_trade(datetime.utcnow()):
                continue
            
            # Transfer goods along route
            for item_id, volume in route.goods_traded.items():
                if item_id not in market_state:
                    continue
                
                # Price arbitrage: move from low to high price locations
                from_data = market_state.get(f"{route.from_location}_{item_id}", {})
                to_data = market_state.get(f"{route.to_location}_{item_id}", {})
                
                from_price = from_data.get('price', 0)
                to_price = to_data.get('price', 0)
                
                transport_cost = route.calculate_transport_cost(100)  # Assume 100 units distance
                
                if to_price > from_price + transport_cost:
                    # Profitable trade
                    actual_volume = min(
                        volume,
                        from_data.get('supply', 0)
                    )
                    
                    if actual_volume > 0:
                        # Transfer
                        from_data['supply'] -= actual_volume
                        to_data['supply'] = to_data.get('supply', 0) + actual_volume
                        
                        # Calculate profit
                        profit = (to_price - from_price - transport_cost) * actual_volume
                        results["profit_generated"] += profit
                        results["goods_transferred"][item_id] = \
                            results["goods_transferred"].get(item_id, 0) + actual_volume
                        
                        # Record trade
                        route.record_trade({item_id: actual_volume}, profit)
            
            results["routes_active"] += 1
        
        return results
    
    async def simulate_economic_agents(
        self, market_state: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Run autonomous economic agent decisions
        
        Returns:
            Agent activity results
        """
        results = {
            "agents_active": 0,
            "actions_taken": 0,
            "monopolies_detected": [],
            "black_market_activity": 0
        }
        
        for agent_id, agent in self.economic_agents.items():
            decision = await agent.make_decision(market_state)
            
            if decision:
                success = agent.execute_action(decision, market_state)
                if success:
                    results["actions_taken"] += 1
                    
                    # Detect monopoly behavior
                    behavior = decision.get("behavior")
                    if behavior == "monopoly":
                        results["monopolies_detected"].append({
                            "agent_id": agent_id,
                            "item_id": decision.get("item_id"),
                            "action": decision.get("action")
                        })
                    elif behavior == "black_market":
                        results["black_market_activity"] += 1
                
                results["agents_active"] += 1
        
        return results
    
    def simulate_resource_depletion(self, delta_seconds: float) -> Dict[str, Any]:
        """
        Reduce resource availability over time
        
        Args:
            delta_seconds: Time elapsed since last update
            
        Returns:
            Depletion results
        """
        results = {
            "resources_depleted": [],
            "resources_scarce": [],
            "regeneration_total": 0.0
        }
        
        for resource_id, resource in self.resources.items():
            # Regenerate
            resource.regenerate(delta_seconds)
            results["regeneration_total"] += resource.regeneration_rate * (delta_seconds / 3600.0)
            
            # Check state
            if resource.state == ResourceState.DEPLETED:
                results["resources_depleted"].append(resource_id)
            elif resource.state == ResourceState.SCARCE:
                results["resources_scarce"].append(resource_id)
        
        return results
    
    def check_innovation_triggers(
        self, game_statistics: Dict[str, Any]
    ) -> List[Innovation]:
        """
        Check if innovations should be triggered
        
        Args:
            game_statistics: Current game stats
            
        Returns:
            List of newly discovered innovations
        """
        newly_discovered = []
        
        for innovation_id, innovation in self.innovations.items():
            if innovation.discovered:
                continue
            
            # Check unlock conditions
            conditions_met = all(
                game_statistics.get(condition, 0) >= threshold
                for condition, threshold in innovation.unlock_conditions.items()
            )
            
            if conditions_met:
                # High price or scarcity can trigger innovation
                trigger_chance = 0.1  # 10% base chance
                
                # Increase chance if related resource is scarce
                for resource_id, resource in self.resources.items():
                    if resource.state in [ResourceState.SCARCE, ResourceState.DEPLETED]:
                        trigger_chance += 0.2
                
                if random.random() < trigger_chance:
                    innovation.discovered = True
                    innovation.discovered_at = datetime.utcnow()
                    newly_discovered.append(innovation)
                    
                    logger.info(f"Innovation discovered: {innovation.name}")
                    
                    # Apply effects
                    self._apply_innovation_effects(innovation)
        
        return newly_discovered
    
    def _apply_innovation_effects(self, innovation: Innovation) -> None:
        """Apply innovation effects to simulation"""
        for effect_type, multiplier in innovation.effects.items():
            if "regeneration" in effect_type:
                # Increase regeneration rate
                resource_id = effect_type.replace("_regeneration", "")
                if resource_id in self.resources:
                    self.resources[resource_id].regeneration_rate *= multiplier
                    logger.info(f"Applied {innovation.name}: {resource_id} regeneration x{multiplier}")
            
            elif "efficiency" in effect_type:
                # Reduce input requirements in production chains
                # This would require modifying production chain inputs
                logger.info(f"Applied {innovation.name}: efficiency boost")
            
            elif "transport_cost" in effect_type:
                # Reduce trade route costs
                for route in self.trade_network.routes:
                    route.transport_cost *= multiplier
                logger.info(f"Applied {innovation.name}: transport cost x{multiplier}")
    
    def detect_monopoly(self, market_state: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Detect if single agent controls market"""
        monopolies = []
        
        for item_id, data in market_state.items():
            total_supply = data.get('supply', 0)
            if total_supply == 0:
                continue
            
            # Check agent holdings
            for agent_id, agent in self.economic_agents.items():
                agent_supply = agent.inventory.get(item_id, 0)
                market_share = agent_supply / max(total_supply, 1)
                
                if market_share > 0.6:  # 60% threshold
                    monopolies.append({
                        "item_id": item_id,
                        "agent_id": agent_id,
                        "market_share": market_share,
                        "detected_at": datetime.utcnow().isoformat()
                    })
                    logger.warning(f"Monopoly detected: {agent_id} controls {market_share:.1%} of {item_id}")
        
        return monopolies
    
    def get_simulation_state(self) -> Dict[str, Any]:
        """Get current simulation state"""
        return {
            "production_chains": len(self.production_chains),
            "trade_routes": len(self.trade_network.routes),
            "economic_agents": {
                agent_type.value: sum(
                    1 for a in self.economic_agents.values()
                    if a.agent_type == agent_type
                )
                for agent_type in EconomicAgentType
            },
            "resources": {
                resource_id: {
                    "state": resource.state.value,
                    "reserves": resource.current_reserves,
                    "depletion_rate": resource.depletion_rate
                }
                for resource_id, resource in self.resources.items()
            },
            "innovations": {
                innovation_id: innovation.discovered
                for innovation_id, innovation in self.innovations.items()
            }
        }