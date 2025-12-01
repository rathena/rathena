"""
Economic Agents - Autonomous economic actors
Merchants, Craftsmen, Consumers, and Investors with independent behaviors
"""

from typing import Dict, List, Optional, Any
from enum import Enum
from dataclasses import dataclass, field
from datetime import datetime
import random
from loguru import logger


class EconomicAgentType(str, Enum):
    """Types of economic actors"""
    MERCHANT = "merchant"
    CRAFTSMAN = "craftsman"
    CONSUMER = "consumer"
    INVESTOR = "investor"


class EconomicBehavior(str, Enum):
    """Economic behavior patterns"""
    HOARDING = "hoarding"
    SPECULATION = "speculation"
    MONOPOLY = "monopoly"
    PRICE_FIXING = "price_fixing"
    BLACK_MARKET = "black_market"
    NORMAL = "normal"


@dataclass
class TradeHistory:
    """Record of trade transaction"""
    timestamp: datetime
    action: str  # "buy" or "sell"
    item_id: str
    quantity: int
    price: float
    profit: float = 0.0


class EconomicAgent:
    """
    Individual economic actor with autonomous behavior.
    
    Acts as merchant, craftsman, consumer, or investor
    making independent economic decisions.
    """
    
    def __init__(
        self, 
        agent_type: EconomicAgentType, 
        npc_id: str,
        starting_wealth: float = 1000.0
    ):
        self.agent_type = agent_type
        self.npc_id = npc_id
        self.inventory: Dict[str, int] = {}
        self.wealth: float = starting_wealth
        self.trade_history: List[TradeHistory] = []
        self.behavior: EconomicBehavior = EconomicBehavior.NORMAL
        self.risk_tolerance: float = 0.5  # 0.0 to 1.0
        self.greed_factor: float = 0.5  # 0.0 to 1.0
        
        # Agent-specific attributes
        self.specialization: Optional[str] = None  # For craftsmen
        self.target_profit_margin: float = 0.2  # 20% default
        
        logger.info(f"Created {agent_type.value} agent: {npc_id}")
    
    async def make_decision(
        self, market_state: Dict[str, Any]
    ) -> Optional[Dict[str, Any]]:
        """
        Make economic decision based on agent type
        
        Args:
            market_state: Current market conditions
            
        Returns:
            Decision dictionary or None
        """
        if self.agent_type == EconomicAgentType.MERCHANT:
            return self._merchant_decision(market_state)
        elif self.agent_type == EconomicAgentType.CRAFTSMAN:
            return self._craftsman_decision(market_state)
        elif self.agent_type == EconomicAgentType.CONSUMER:
            return self._consumer_decision(market_state)
        elif self.agent_type == EconomicAgentType.INVESTOR:
            return self._investor_decision(market_state)
        
        return None
    
    def _merchant_decision(self, market_state: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """
        Merchant: Buy low, sell high
        
        Behaviors:
        - Hoarding: Buy when cheap and hold for profit
        - Speculation: Bet on future price increases
        - Monopoly: Control market supply
        """
        actions = []
        
        for item_id, data in market_state.items():
            current_price = data.get('price', 0)
            base_price = data.get('base_price', current_price)
            supply = data.get('supply', 0)
            trend = data.get('trend', 'stable')
            
            # Hoarding behavior: Buy when cheap
            if current_price < base_price * 0.8 and self.wealth > current_price * 10:
                buy_quantity = min(10, int(self.wealth // current_price))
                if buy_quantity > 0:
                    actions.append({
                        "action": "buy",
                        "item_id": item_id,
                        "quantity": buy_quantity,
                        "max_price": current_price * 1.1,
                        "reason": "hoarding_opportunity",
                        "behavior": EconomicBehavior.HOARDING
                    })
                    self.behavior = EconomicBehavior.HOARDING
            
            # Speculation: Buy rising items
            if trend == "rising" and self.risk_tolerance > 0.6:
                speculate_qty = min(5, int(self.wealth // current_price * 0.2))
                if speculate_qty > 0:
                    actions.append({
                        "action": "buy",
                        "item_id": item_id,
                        "quantity": speculate_qty,
                        "max_price": current_price * 1.2,
                        "reason": "speculation",
                        "behavior": EconomicBehavior.SPECULATION
                    })
                    self.behavior = EconomicBehavior.SPECULATION
            
            # Monopoly attempt: Buy large quantity to control supply
            if item_id in self.inventory:
                my_supply = self.inventory[item_id]
                market_share = my_supply / max(supply, 1)
                
                if market_share > 0.6 and self.greed_factor > 0.7:
                    # Attempt price fixing
                    actions.append({
                        "action": "set_price",
                        "item_id": item_id,
                        "new_price": current_price * 1.5,
                        "reason": "monopoly_pricing",
                        "behavior": EconomicBehavior.MONOPOLY
                    })
                    self.behavior = EconomicBehavior.MONOPOLY
            
            # Sell when expensive
            if item_id in self.inventory and current_price > base_price * (1 + self.target_profit_margin):
                sell_quantity = self.inventory[item_id]
                if sell_quantity > 0:
                    actions.append({
                        "action": "sell",
                        "item_id": item_id,
                        "quantity": sell_quantity,
                        "min_price": current_price * 0.95,
                        "reason": "profit_taking",
                        "behavior": EconomicBehavior.NORMAL
                    })
        
        return actions[0] if actions else None
    
    def _craftsman_decision(self, market_state: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """
        Craftsman: Produce goods based on market demand
        
        Behaviors:
        - Check material prices
        - Produce if profitable
        - Innovate new products
        """
        # Check if we have a specialization
        if not self.specialization:
            self.specialization = random.choice(["weapons", "armor", "potions", "tools"])
        
        # Find profitable production opportunities
        for item_id, data in market_state.items():
            if not self._is_craftable(item_id):
                continue
            
            current_price = data.get('price', 0)
            demand = data.get('demand', 0)
            
            # Check material costs
            materials = self._get_required_materials(item_id)
            material_cost = sum(
                market_state.get(mat, {}).get('price', 0) * qty
                for mat, qty in materials.items()
            )
            
            # Produce if profitable
            profit_margin = (current_price - material_cost) / max(material_cost, 1)
            
            if profit_margin > 0.3 and demand > 5:
                return {
                    "action": "produce",
                    "item_id": item_id,
                    "quantity": min(5, demand),
                    "reason": "profitable_production",
                    "expected_profit": profit_margin,
                    "behavior": EconomicBehavior.NORMAL
                }
        
        return None
    
    def _consumer_decision(self, market_state: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """
        Consumer: Buy goods for consumption
        
        Behaviors:
        - Buy necessities first
        - Buy luxury when wealthy
        - Avoid overpriced items
        """
        necessities = ["food", "potion", "arrow"]
        luxuries = ["equipment", "accessory", "pet"]
        
        # Buy necessities if missing
        for item_id in necessities:
            if item_id not in self.inventory or self.inventory[item_id] < 5:
                if item_id in market_state:
                    price = market_state[item_id].get('price', 0)
                    if self.wealth > price * 5:
                        return {
                            "action": "buy",
                            "item_id": item_id,
                            "quantity": 5,
                            "max_price": price * 1.1,
                            "reason": "necessity",
                            "behavior": EconomicBehavior.NORMAL
                        }
        
        # Buy luxuries if wealthy
        if self.wealth > 5000:
            for item_id in luxuries:
                if item_id in market_state:
                    price = market_state[item_id].get('price', 0)
                    base_price = market_state[item_id].get('base_price', price)
                    
                    # Only buy if not overpriced
                    if price < base_price * 1.5 and self.wealth > price:
                        return {
                            "action": "buy",
                            "item_id": item_id,
                            "quantity": 1,
                            "max_price": price,
                            "reason": "luxury_purchase",
                            "behavior": EconomicBehavior.NORMAL
                        }
        
        return None
    
    def _investor_decision(self, market_state: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """
        Investor: Trade for long-term profit
        
        Behaviors:
        - Analyze trends
        - Diversify portfolio
        - Take calculated risks
        """
        portfolio_value = sum(
            self.inventory.get(item_id, 0) * market_state.get(item_id, {}).get('price', 0)
            for item_id in market_state
        )
        
        # Diversification check
        if len(self.inventory) < 3 and self.wealth > 1000:
            # Find undervalued items
            for item_id, data in market_state.items():
                price = data.get('price', 0)
                base_price = data.get('base_price', price)
                trend = data.get('trend', 'stable')
                
                # Undervalued with positive trend
                if price < base_price * 0.9 and trend in ["rising", "stable"]:
                    invest_amount = self.wealth * 0.2  # Invest 20%
                    quantity = int(invest_amount // price)
                    
                    if quantity > 0:
                        return {
                            "action": "buy",
                            "item_id": item_id,
                            "quantity": quantity,
                            "max_price": price * 1.05,
                            "reason": "diversification",
                            "behavior": EconomicBehavior.NORMAL
                        }
        
        # Rebalance portfolio
        if portfolio_value > self.wealth * 2:
            # Sell some holdings
            for item_id, quantity in self.inventory.items():
                if quantity > 0 and item_id in market_state:
                    price = market_state[item_id].get('price', 0)
                    base_price = market_state[item_id].get('base_price', price)
                    
                    # Sell if profitable
                    if price > base_price * 1.2:
                        return {
                            "action": "sell",
                            "item_id": item_id,
                            "quantity": quantity // 2,
                            "min_price": price * 0.95,
                            "reason": "rebalancing",
                            "behavior": EconomicBehavior.NORMAL
                        }
        
        return None
    
    def execute_action(
        self, action: Dict[str, Any], market_state: Dict[str, Any]
    ) -> bool:
        """
        Execute economic action
        
        Args:
            action: Action dictionary
            market_state: Current market state
            
        Returns:
            True if action succeeded
        """
        action_type = action.get("action")
        item_id = action.get("item_id")
        quantity = action.get("quantity", 0)
        
        if action_type == "buy":
            price = market_state.get(item_id, {}).get('price', 0)
            total_cost = price * quantity
            
            if self.wealth >= total_cost:
                self.wealth -= total_cost
                self.inventory[item_id] = self.inventory.get(item_id, 0) + quantity
                
                # Record trade
                self.trade_history.append(TradeHistory(
                    timestamp=datetime.utcnow(),
                    action="buy",
                    item_id=item_id,
                    quantity=quantity,
                    price=price
                ))
                
                logger.info(f"{self.npc_id} bought {quantity}x {item_id} for {total_cost}")
                return True
        
        elif action_type == "sell":
            if self.inventory.get(item_id, 0) >= quantity:
                price = market_state.get(item_id, {}).get('price', 0)
                total_revenue = price * quantity
                
                self.inventory[item_id] -= quantity
                self.wealth += total_revenue
                
                # Calculate profit
                buy_price = self._get_average_buy_price(item_id)
                profit = (price - buy_price) * quantity
                
                # Record trade
                self.trade_history.append(TradeHistory(
                    timestamp=datetime.utcnow(),
                    action="sell",
                    item_id=item_id,
                    quantity=quantity,
                    price=price,
                    profit=profit
                ))
                
                logger.info(f"{self.npc_id} sold {quantity}x {item_id} for {total_revenue} (profit: {profit})")
                return True
        
        return False
    
    def _is_craftable(self, item_id: str) -> bool:
        """Check if item can be crafted by this craftsman"""
        craftable_items = {
            "weapons": ["sword", "bow", "staff"],
            "armor": ["helmet", "armor", "shield"],
            "potions": ["health_potion", "mana_potion"],
            "tools": ["pickaxe", "fishing_rod"]
        }
        
        if not self.specialization:
            return False
        
        return item_id in craftable_items.get(self.specialization, [])
    
    def _get_required_materials(self, item_id: str) -> Dict[str, int]:
        """Get materials needed to craft item"""
        recipes = {
            "sword": {"iron_ore": 3, "wood": 1},
            "armor": {"iron_ore": 5, "leather": 2},
            "health_potion": {"herb": 2, "water": 1}
        }
        return recipes.get(item_id, {})
    
    def _get_average_buy_price(self, item_id: str) -> float:
        """Calculate average buy price for item"""
        buys = [
            t.price for t in self.trade_history
            if t.item_id == item_id and t.action == "buy"
        ]
        return sum(buys) / len(buys) if buys else 0.0
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get agent statistics"""
        total_trades = len(self.trade_history)
        total_profit = sum(t.profit for t in self.trade_history if t.profit > 0)
        
        return {
            "agent_type": self.agent_type.value,
            "npc_id": self.npc_id,
            "wealth": self.wealth,
            "inventory_items": len(self.inventory),
            "total_trades": total_trades,
            "total_profit": total_profit,
            "current_behavior": self.behavior.value,
            "risk_tolerance": self.risk_tolerance,
            "greed_factor": self.greed_factor
        }