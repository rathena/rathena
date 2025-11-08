"""
Economic Simulation Tasks - Agent-Driven Market Dynamics
Implements EconomyAgent-driven economic updates with learning
"""

import asyncio
from datetime import datetime, timedelta
from typing import List, Dict, Any, Optional
from loguru import logger

from ..config import settings
from ..database import db, postgres_db
from ..agents.economy_agent import EconomyAgent
from ..agents.base_agent import AgentContext
from ..models.economy import (
    EconomicState,
    MarketItem,
    ShopInventory,
    EconomicEvent,
    MarketTrend,
    EconomicSimulationConfig
)


class EconomicSimulationManager:
    """
    Manages economic simulation with EconomyAgent-driven decisions
    Learns from historical data and adapts to player behavior
    """

    def __init__(self):
        # Initialize EconomyAgent with required parameters
        from ..llm import get_llm_provider
        llm = get_llm_provider()
        config = {"verbose": False}
        self.economy_agent = EconomyAgent(
            agent_id="economy_001",
            llm_provider=llm,
            config=config
        )
        self.last_update_time: Optional[datetime] = None
        logger.info("EconomicSimulationManager initialized")
    
    async def update_economic_simulation(self) -> Dict[str, Any]:
        """
        Main economic update - called daily or on fixed interval
        EconomyAgent analyzes market and makes decisions
        """
        if not settings.economy_enabled:
            logger.warning("Economic simulation disabled")
            return {"success": False, "reason": "disabled"}
        
        logger.info("=" * 80)
        logger.info("ECONOMIC SIMULATION UPDATE")
        logger.info("=" * 80)
        
        try:
            # Get current economic state
            current_state = await self._get_current_state()
            
            # Get historical trading data
            trading_history = await self._get_trading_history(days=7)
            
            # Build context for EconomyAgent
            context = await self._build_economy_context(current_state, trading_history)
            
            # EconomyAgent analyzes and makes decisions
            logger.info("EconomyAgent analyzing market data...")
            decision = await self.economy_agent.process(context)
            
            if not decision.success:
                logger.error(f"EconomyAgent decision failed: {decision.error}")
                return {"success": False, "error": decision.error}
            
            # Apply economic decisions
            updates = await self._apply_economic_decisions(decision.data)
            
            # Store decision for learning
            if settings.agent_decision_logging_enabled:
                await self._store_economic_decision(decision.data, context)
            
            # Update last update time
            self.last_update_time = datetime.utcnow()
            
            logger.info(f"Economic update complete: {updates}")
            logger.info("=" * 80)
            
            return {
                "success": True,
                "timestamp": self.last_update_time.isoformat(),
                "updates": updates,
                "agent_decision": decision.data
            }
        
        except Exception as e:
            logger.error(f"Error in economic simulation update: {e}", exc_info=True)
            return {"success": False, "error": str(e)}
    
    async def update_item_price_with_learning(
        self,
        item_id: str,
        new_price: int,
        reason: Optional[str] = None
    ) -> Dict[str, Any]:
        """
        Update item price and record for learning
        EconomyAgent learns from price changes and their effects
        """
        try:
            # Get current price
            current_price = await self._get_item_price(item_id)
            
            # Update price in database
            await self._update_item_price_db(item_id, new_price, reason)
            
            # Record price change for learning
            if settings.economy_learning_enabled:
                await self._record_price_change(
                    item_id=item_id,
                    old_price=current_price,
                    new_price=new_price,
                    reason=reason,
                    timestamp=datetime.utcnow()
                )
            
            logger.info(f"Price updated: {item_id} {current_price} -> {new_price} ({reason})")
            
            return {
                "success": True,
                "item_id": item_id,
                "old_price": current_price,
                "new_price": new_price
            }
        
        except Exception as e:
            logger.error(f"Error updating price for {item_id}: {e}", exc_info=True)
            raise
    
    async def analyze_market_with_agent(
        self,
        item_ids: Optional[List[str]] = None,
        category: Optional[str] = None,
        time_range_days: int = 7
    ) -> Dict[str, Any]:
        """
        Use EconomyAgent to analyze market trends and provide recommendations
        """
        try:
            # Get market data
            market_data = await self._get_market_data(item_ids, category, time_range_days)
            
            # Build analysis context
            context = AgentContext(
                task_type="market_analysis",
                market_data=market_data,
                time_range_days=time_range_days,
                learning_enabled=settings.economy_learning_enabled
            )
            
            # EconomyAgent analyzes
            analysis = await self.economy_agent.analyze_market(context)
            
            return {
                "analysis_time": datetime.utcnow(),
                "items_analyzed": len(market_data.get("items", [])),
                "trends": analysis.get("trends", []),
                "recommendations": analysis.get("recommendations", []),
                "market_health": analysis.get("market_health", "unknown"),
                "summary": analysis.get("summary", "")
            }
        
        except Exception as e:
            logger.error(f"Error in market analysis: {e}", exc_info=True)
            raise

    # ========================================================================
    # Helper Methods
    # ========================================================================

    async def _get_current_state(self) -> EconomicState:
        """Get current economic state"""
        try:
            cache_key = "economy:state:current"
            cached = await db.get(cache_key)

            if cached:
                return EconomicState(**cached)

            # Default state
            state = EconomicState(
                inflation_rate=0.0,
                total_zeny_in_circulation=0,
                average_player_wealth=0,
                active_traders=0,
                market_volatility=0.0,
                timestamp=datetime.utcnow()
            )

            # Cache for 5 minutes
            await db.set(cache_key, state.dict(), expire=300)

            return state

        except Exception as e:
            logger.error(f"Error getting current economic state: {e}")
            return EconomicState(timestamp=datetime.utcnow())

    async def _get_trading_history(self, days: int = 7) -> List[Dict[str, Any]]:
        """Get recent trading history for analysis"""
        try:
            query = """
                SELECT * FROM trading_history
                WHERE timestamp >= $1
                ORDER BY timestamp DESC
                LIMIT 1000
            """
            cutoff = datetime.utcnow() - timedelta(days=days)
            results = await postgres_db.fetch_all(query, cutoff)

            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting trading history: {e}")
            return []

    async def _build_economy_context(
        self,
        current_state: EconomicState,
        trading_history: List[Dict[str, Any]]
    ) -> AgentContext:
        """Build context for EconomyAgent decision making"""

        # Get historical economic decisions for learning
        past_decisions = await self._get_past_economic_decisions(
            limit=settings.agent_context_window_size
        )

        # Get active economic events
        active_events = await self._get_active_economic_events()

        context = AgentContext(
            task_type="economic_update",
            current_state=current_state.dict(),
            trading_history=trading_history,
            past_decisions=past_decisions,
            active_events=active_events,
            learning_enabled=settings.economy_learning_enabled,
            adaptive_pricing=settings.economy_adaptive_pricing,
            config={
                "inflation_enabled": settings.economy_inflation_enabled,
                "supply_demand_enabled": settings.economy_supply_demand_enabled,
                "events_enabled": settings.economy_events_enabled
            }
        )

        return context

    async def _get_past_economic_decisions(self, limit: int = 10) -> List[Dict[str, Any]]:
        """Get past economic decisions for learning"""
        if not settings.economy_learning_enabled:
            return []

        try:
            query = """
                SELECT * FROM economic_decision_history
                ORDER BY timestamp DESC
                LIMIT $1
            """
            results = await postgres_db.fetch_all(query, limit)
            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting past economic decisions: {e}")
            return []

    async def _get_active_economic_events(self) -> List[Dict[str, Any]]:
        """Get currently active economic events"""
        try:
            query = """
                SELECT * FROM economic_events
                WHERE is_active = true
                AND end_time > $1
            """
            results = await postgres_db.fetch_all(query, datetime.utcnow())
            return [dict(row) for row in results]

        except Exception as e:
            logger.error(f"Error getting active economic events: {e}")
            return []

    async def _apply_economic_decisions(self, decision_data: Dict[str, Any]) -> Dict[str, Any]:
        """Apply EconomyAgent decisions to the economy"""
        updates = {
            "prices_updated": 0,
            "events_triggered": 0,
            "inflation_adjusted": False
        }

        try:
            # Update prices based on agent decisions
            if "price_adjustments" in decision_data:
                for adjustment in decision_data["price_adjustments"]:
                    await self._update_item_price_db(
                        item_id=adjustment["item_id"],
                        new_price=adjustment["new_price"],
                        reason=adjustment.get("reason", "agent_decision")
                    )
                    updates["prices_updated"] += 1

            # Trigger economic events if recommended
            if "recommended_events" in decision_data:
                for event in decision_data["recommended_events"]:
                    await self._create_economic_event(event)
                    updates["events_triggered"] += 1

            # Update inflation rate
            if "inflation_adjustment" in decision_data:
                await self._update_inflation_rate(decision_data["inflation_adjustment"])
                updates["inflation_adjusted"] = True

            return updates

        except Exception as e:
            logger.error(f"Error applying economic decisions: {e}")
            return updates

    async def _update_item_price_db(self, item_id: str, new_price: int, reason: str):
        """Update item price in database"""
        try:
            query = """
                UPDATE market_items
                SET current_price = $1, last_update = $2, update_reason = $3
                WHERE item_id = $4
            """
            await postgres_db.execute(query, new_price, datetime.utcnow(), reason, item_id)

            # Invalidate cache
            await db.delete(f"item:price:{item_id}")

        except Exception as e:
            logger.error(f"Error updating item price in DB: {e}")

    async def _get_item_price(self, item_id: str) -> int:
        """Get current item price"""
        try:
            cache_key = f"item:price:{item_id}"
            cached = await db.get(cache_key)

            if cached:
                return int(cached)

            query = "SELECT current_price FROM market_items WHERE item_id = $1"
            result = await postgres_db.fetch_one(query, item_id)

            if result:
                price = result["current_price"]
                await db.set(cache_key, price, expire=300)
                return price

            return 0

        except Exception as e:
            logger.error(f"Error getting item price: {e}")
            return 0

    async def _record_price_change(
        self,
        item_id: str,
        old_price: int,
        new_price: int,
        reason: Optional[str],
        timestamp: datetime
    ):
        """Record price change for learning"""
        try:
            query = """
                INSERT INTO price_change_history
                (item_id, old_price, new_price, reason, timestamp)
                VALUES ($1, $2, $3, $4, $5)
            """
            await postgres_db.execute(query, item_id, old_price, new_price, reason, timestamp)

        except Exception as e:
            logger.error(f"Error recording price change: {e}")

    async def _store_economic_decision(self, decision_data: Dict[str, Any], context: AgentContext):
        """Store economic decision for learning"""
        try:
            query = """
                INSERT INTO economic_decision_history
                (decision_data, context_data, timestamp)
                VALUES ($1, $2, $3)
            """
            await postgres_db.execute(
                query,
                decision_data,
                context.dict(),
                datetime.utcnow()
            )

        except Exception as e:
            logger.error(f"Error storing economic decision: {e}")

    async def _get_market_data(
        self,
        item_ids: Optional[List[str]],
        category: Optional[str],
        time_range_days: int
    ) -> Dict[str, Any]:
        """Get market data for analysis"""
        # Simplified implementation
        return {
            "items": [],
            "time_range_days": time_range_days,
            "category": category
        }

    async def _create_economic_event(self, event_data: Dict[str, Any]):
        """Create new economic event"""
        try:
            query = """
                INSERT INTO economic_events
                (event_type, severity, duration_hours, is_active, start_time, end_time)
                VALUES ($1, $2, $3, true, $4, $5)
            """
            start_time = datetime.utcnow()
            end_time = start_time + timedelta(hours=event_data.get("duration_hours", 24))

            await postgres_db.execute(
                query,
                event_data["event_type"],
                event_data.get("severity", 1.0),
                event_data.get("duration_hours", 24),
                start_time,
                end_time
            )

        except Exception as e:
            logger.error(f"Error creating economic event: {e}")

    async def _update_inflation_rate(self, adjustment: float):
        """Update inflation rate"""
        try:
            query = "UPDATE economic_state SET inflation_rate = inflation_rate + $1"
            await postgres_db.execute(query, adjustment)

        except Exception as e:
            logger.error(f"Error updating inflation rate: {e}")


# Global instance
economic_simulation_manager = EconomicSimulationManager()


# Public functions for scheduler and router
async def update_economic_simulation() -> Dict[str, Any]:
    """Update economic simulation"""
    return await economic_simulation_manager.update_economic_simulation()


async def get_current_economic_state() -> EconomicState:
    """Get current economic state"""
    return await economic_simulation_manager._get_current_state()


async def update_item_price_with_learning(item_id: str, new_price: int, reason: Optional[str] = None) -> Dict[str, Any]:
    """Update item price with learning"""
    return await economic_simulation_manager.update_item_price_with_learning(item_id, new_price, reason)


async def analyze_market_with_agent(
    item_ids: Optional[List[str]] = None,
    category: Optional[str] = None,
    time_range_days: int = 7
) -> Dict[str, Any]:
    """Analyze market with EconomyAgent"""
    return await economic_simulation_manager.analyze_market_with_agent(item_ids, category, time_range_days)


async def get_item_price_info(item_id: str) -> Optional[Dict[str, Any]]:
    """Get item price info"""
    price = await economic_simulation_manager._get_item_price(item_id)
    if price > 0:
        return {"item_id": item_id, "current_price": price}
    return None


async def get_market_trends(category: Optional[str] = None, limit: int = 20) -> List[Dict[str, Any]]:
    """Get market trends from cached economic data"""
    try:
        from ..database import db

        # Get current economic state
        state_key = "economy:state:current"
        state_data = await db.get(state_key)

        if not state_data:
            # Return default trends if no data available
            return [
                {
                    "item_category": category or "general",
                    "trend": "stable",
                    "price_change_percent": 0.0,
                    "demand_level": "medium",
                    "supply_level": "medium",
                    "timestamp": datetime.utcnow().isoformat()
                }
            ]

        # Extract trends from economic state
        trends = []

        # If state_data is a string, try to parse it as JSON
        if isinstance(state_data, str):
            import json
            try:
                state_data = json.loads(state_data)
            except json.JSONDecodeError:
                pass

        # Generate trends based on available data
        if isinstance(state_data, dict):
            inflation_rate = state_data.get("inflation_rate", 0.0)

            # Create a trend entry
            trend_entry = {
                "item_category": category or "all",
                "trend": "increasing" if inflation_rate > 0.05 else "decreasing" if inflation_rate < -0.05 else "stable",
                "price_change_percent": inflation_rate * 100,
                "demand_level": "high" if inflation_rate > 0.1 else "low" if inflation_rate < -0.1 else "medium",
                "supply_level": "low" if inflation_rate > 0.1 else "high" if inflation_rate < -0.1 else "medium",
                "timestamp": datetime.utcnow().isoformat()
            }
            trends.append(trend_entry)

        return trends[:limit]

    except Exception as e:
        logger.error(f"Error getting market trends: {e}")
        # Return default trend on error
        return [
            {
                "item_category": category or "general",
                "trend": "stable",
                "price_change_percent": 0.0,
                "demand_level": "medium",
                "supply_level": "medium",
                "timestamp": datetime.utcnow().isoformat()
            }
        ]


async def get_shop_inventory(shop_id: str) -> Optional[ShopInventory]:
    """Get shop inventory"""
    return None  # Simplified


async def get_economic_events(active_only: bool = True, limit: int = 10) -> List[Dict[str, Any]]:
    """Get economic events"""
    return []  # Simplified


async def trigger_economic_event(event_type: str, severity: float, duration_hours: int) -> Dict[str, Any]:
    """Trigger economic event"""
    event_data = {
        "event_type": event_type,
        "severity": severity,
        "duration_hours": duration_hours
    }
    await economic_simulation_manager._create_economic_event(event_data)
    return event_data


