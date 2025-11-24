"""
Economy Agent - Manages economic simulation and market dynamics
"""

from typing import Dict, Any, List, Optional
from loguru import logger
import json
import random
from datetime import datetime, timedelta

from crewai import Agent
try:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.economy import (
        MarketItem, EconomicState, MarketTrend, ItemCategory,
        EconomicEvent, TradeRecommendation
    )
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.economy import (
        MarketItem, EconomicState, MarketTrend, ItemCategory,
        EconomicEvent, TradeRecommendation
    )

# NEW: Import economic simulation components
from agents.economic_simulation import EconomicSimulation
from agents.economic_agents import EconomicAgentType


class EconomyAgent(BaseAIAgent):
    """
    Specialized agent for economic simulation
    
    Manages:
    - Price fluctuations based on supply/demand
    - Market trends and analysis
    - Economic events generation
    - Trade recommendations
    - Inflation and deflation
    - Production chains
    - Trade routes
    - Economic agents
    - Resource depletion
    - Innovation cycles
    """
    
    def __init__(
        self,
        agent_id: str,
        llm_provider: Any,
        config: Dict[str, Any]
    ):
        super().__init__(
            agent_id=agent_id,
            agent_type="economy",
            llm_provider=llm_provider,
            config=config
        )
        from agents.moral_alignment import MoralAlignment
        self.moral_alignment = MoralAlignment()
        
        # NEW: Initialize Complete Economic Simulation System
        self.economic_simulation = EconomicSimulation()
        
        # NEW: Game statistics for innovation triggers
        self.game_statistics = {
            "total_iron_smelted": 0,
            "wood_depleted_count": 0,
            "potions_crafted": 0,
            "total_trades": 0
        }
        
        # NEW: Simulation update tracking
        self.last_simulation_update = datetime.now(__import__('datetime').timezone.utc)
        
        self.crew_agent = self._create_crew_agent()
        logger.info(
            f"Economy Agent {agent_id} initialized "
            f"(Production chains: {len(self.economic_simulation.production_chains)}, "
            f"Trade routes: {len(self.economic_simulation.trade_network.routes)}, "
            f"Resources: {len(self.economic_simulation.resources)})"
        )
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for economic simulation"""
        # Import CrewAI's LLM class
        from crewai import LLM
        import os

        # Create CrewAI-compatible LLM using litellm format for Azure OpenAI
        try:
            from config import settings

            # Set Azure OpenAI environment variables for litellm
            os.environ["AZURE_API_KEY"] = settings.azure_openai_api_key
            os.environ["AZURE_API_BASE"] = settings.azure_openai_endpoint
            os.environ["AZURE_API_VERSION"] = settings.azure_openai_api_version

            # Use litellm format: azure/<deployment_name>
            llm = LLM(
                model=f"azure/{settings.azure_openai_deployment}",
                temperature=0.7,
                max_tokens=2000
            )
            logger.info(f"Created Azure OpenAI LLM with deployment: {settings.azure_openai_deployment}")
        except Exception as e:
            logger.error(f"Failed to create Azure LLM: {e}")
            raise

        return Agent(
            role="Economic Analyst",
            goal="Simulate realistic market dynamics and provide economic insights",
            backstory="""You are an expert economist specializing in virtual economies.
            You understand supply and demand, market psychology, and economic cycles.
            You can predict market trends and generate realistic economic events.""",
            verbose=self.config.get("verbose", False),
            allow_delegation=False,
            llm=llm
        )
    
    async def _process(self, context: AgentContext) -> AgentResponse:
        """
        Process economic simulation task (internal implementation)
        
        Args:
            context: Agent context with operation type
            
        Returns:
            AgentResponse with economic data
        """
        try:
            operation = context.current_state.get("operation", "analyze")

            # --- Moral Alignment Integration ---
            # Update alignment based on economic operation and context
            alignment_action = {
                "type": f"economy_{operation}",
                "operation": operation
            }
            self.moral_alignment.update_from_action(alignment_action)

            logger.info(f"Economy Agent processing: {operation}")
            
            if operation == "update_prices":
                result = await self._update_market_prices(context)
            elif operation == "analyze":
                result = await self._analyze_market(context)
            elif operation == "generate_event":
                result = await self._generate_economic_event(context)
            elif operation == "recommend_trade":
                result = await self._recommend_trade(context)
            # NEW: Complete economic simulation operations
            elif operation == "simulate_complete":
                result = await self._simulate_complete_economy(context)
            elif operation == "add_agent":
                result = await self._add_economic_agent(context)
            elif operation == "check_innovations":
                result = await self._check_innovations(context)
            else:
                logger.warning(f"Unknown operation: {operation}")
                result = {"error": f"Unknown operation: {operation}"}
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data={**result, "alignment": self.moral_alignment.to_dict()},
                confidence=0.8,
                reasoning=f"Completed {operation} operation and updated alignment",
                metadata={"operation": operation, "alignment": self.moral_alignment.to_dict()},
                timestamp=datetime.now(__import__('datetime').timezone.utc)
            )
            
        except Exception as e:
            logger.error(f"Economy Agent error: {e}")
            return AgentResponse(
                agent_type=self.agent_type,
                success=False,
                data={},
                confidence=0.0,
                reasoning=f"Error: {str(e)}",
                metadata={"error": str(e)},
                timestamp=datetime.now(__import__('datetime').timezone.utc)
            )
    
    async def _update_market_prices(self, context: AgentContext) -> Dict[str, Any]:
        """Update market prices based on supply/demand"""
        
        items = context.current_state.get("items", [])
        economic_state = context.world_state.get("economy", {})
        
        updated_items = []
        
        for item_data in items:
            item = MarketItem(**item_data)
            
            # Calculate price change based on supply/demand
            supply_demand_ratio = item.supply / max(item.demand, 1)
            
            # Base price change
            if supply_demand_ratio > 1.5:
                # Oversupply - price decreases
                price_change = -0.1
                trend = MarketTrend.FALLING
            elif supply_demand_ratio < 0.5:
                # High demand - price increases
                price_change = 0.15
                trend = MarketTrend.RISING
            else:
                # Balanced - small random fluctuation
                price_change = random.uniform(-0.05, 0.05)
                trend = MarketTrend.STABLE
            
            # Apply inflation
            inflation = economic_state.get("inflation_rate", 0.0)
            price_change += inflation * 0.1
            
            # Apply volatility
            volatility = self.config.get("price_volatility", 0.1)
            price_change += random.uniform(-volatility, volatility)
            
            # Limit price change
            max_change = self.config.get("max_price_change", 0.2)
            price_change = max(-max_change, min(max_change, price_change))
            
            # Update price
            new_price = int(item.current_price * (1 + price_change))
            new_price = max(int(item.base_price * 0.5), new_price)  # Min 50% of base
            new_price = min(int(item.base_price * 3.0), new_price)  # Max 300% of base
            
            item.current_price = new_price
            item.trend = trend
            item.last_updated = datetime.now(__import__('datetime').timezone.utc)
            
            # Add to price history
            item.price_history.append({
                "timestamp": datetime.now(__import__('datetime').timezone.utc).isoformat(),
                "price": new_price
            })
            
            # Keep only last 100 entries
            if len(item.price_history) > 100:
                item.price_history = item.price_history[-100:]
            
            updated_items.append(item.dict())
        
        logger.info(f"Updated prices for {len(updated_items)} items")

        return {
            "updated_items": updated_items,
            "update_count": len(updated_items)
        }

    async def _analyze_market(self, context: AgentContext) -> Dict[str, Any]:
        """Analyze market conditions"""

        items = context.current_state.get("items", [])

        if not items:
            return {
                "economic_state": EconomicState().dict(),
                "analysis": "No market data available"
            }

        # Calculate overall metrics
        total_volume = sum(item.get("supply", 0) * item.get("current_price", 0) for item in items)
        avg_price_change = sum(
            (item.get("current_price", 0) - item.get("base_price", 1)) / item.get("base_price", 1)
            for item in items
        ) / len(items)

        # Determine dominant trend
        trend_counts = {}
        for item in items:
            trend = item.get("trend", "stable")
            trend_counts[trend] = trend_counts.get(trend, 0) + 1

        dominant_trend = max(trend_counts, key=trend_counts.get) if trend_counts else "stable"

        # Calculate market health (0.0 to 1.0)
        # Based on price stability and trade volume
        price_volatility = abs(avg_price_change)
        market_health = max(0.0, min(1.0, 1.0 - price_volatility))

        economic_state = EconomicState(
            inflation_rate=avg_price_change,
            trade_volume=total_volume,
            active_traders=context.current_state.get("active_traders", 0),
            market_health=market_health,
            dominant_trend=dominant_trend,
            last_updated=datetime.now(__import__('datetime').timezone.utc)
        )

        # Generate analysis using LLM
        analysis_text = await self._generate_market_analysis_with_llm(
            economic_state=economic_state,
            items=items,
            context=context
        )

        return {
            "economic_state": economic_state.dict(),
            "analysis": analysis_text,
            "metrics": {
                "total_volume": total_volume,
                "avg_price_change": avg_price_change,
                "market_health": market_health,
                "dominant_trend": dominant_trend
            }
        }

    async def _generate_economic_event(self, context: AgentContext) -> Dict[str, Any]:
        """Generate a random economic event"""

        event_types = [
            "shortage",
            "surplus",
            "crisis",
            "boom",
            "regulation",
            "discovery",
            "disaster"
        ]

        event_type = random.choice(event_types)

        # Generate event using LLM
        system_message = """You are an economic event generator for a fantasy MMORPG.
Generate realistic economic events that affect the game's market.

Return ONLY a JSON object with this structure:
{
    "description": "Event description",
    "affected_categories": ["consumable", "equipment"],
    "price_multiplier": 1.2,
    "duration": 3600,
    "severity": 0.7
}"""

        user_prompt = f"""Generate a {event_type} economic event.
Current economic state:
- Inflation: {context.world_state.get('economy', {}).get('inflation_rate', 0.0)}
- Market health: {context.world_state.get('economy', {}).get('market_health', 1.0)}

Make it interesting and impactful!"""

        try:
            response_text = await self._generate_with_llm(
                prompt=user_prompt,
                system_message=system_message,
                temperature=0.9
            )

            event_data = self._parse_json(response_text)

            if not event_data:
                # Fallback event
                event_data = {
                    "description": f"A {event_type} has occurred in the market",
                    "affected_categories": [random.choice(list(ItemCategory))],
                    "price_multiplier": random.uniform(0.8, 1.3),
                    "duration": random.randint(1800, 7200),
                    "severity": random.uniform(0.3, 0.8)
                }

            event_id = f"event_{int(datetime.now(__import__('datetime').timezone.utc).timestamp())}"

            event = EconomicEvent(
                event_id=event_id,
                event_type=event_type,
                description=event_data.get("description", "An economic event occurred"),
                affected_categories=event_data.get("affected_categories", []),
                price_multiplier=event_data.get("price_multiplier", 1.0),
                duration=event_data.get("duration", 3600),
                severity=event_data.get("severity", 0.5),
                expires_at=datetime.now(__import__('datetime').timezone.utc) + timedelta(seconds=event_data.get("duration", 3600))
            )

            logger.info(f"Generated economic event: {event_id} - {event_type}")

            return {
                "event": event.dict(),
                "event_id": event_id,
                "event_type": event_type
            }

        except Exception as e:
            logger.error(f"Event generation failed: {e}")
            return {"error": str(e)}

    async def _recommend_trade(self, context: AgentContext) -> Dict[str, Any]:
        """Generate trade recommendations"""

        items = context.current_state.get("items", [])
        player_level = context.current_state.get("player_level", 1)

        recommendations = []

        for item_data in items[:5]:  # Top 5 items
            item = MarketItem(**item_data)

            # Log player level for context (used for future recommendation filtering)
            logger.debug(f"Generating trade recommendations for player level {player_level}")

            # Simple recommendation logic
            if item.trend == MarketTrend.FALLING and item.current_price < item.base_price * 0.8:
                action = "buy"
                confidence = 0.7
                reasoning = "Price is falling and below base price - good buying opportunity"
            elif item.trend == MarketTrend.RISING and item.current_price > item.base_price * 1.2:
                action = "sell"
                confidence = 0.8
                reasoning = "Price is rising and above base price - good selling opportunity"
            else:
                action = "hold"
                confidence = 0.5
                reasoning = "Market is stable - wait for better opportunity"

            recommendation = TradeRecommendation(
                item_id=item.item_id,
                item_name=item.item_name,
                action=action,
                confidence=confidence,
                reasoning=reasoning,
                suggested_price=item.current_price,
                risk_level="medium"
            )

            recommendations.append(recommendation.dict())

        return {
            "recommendations": recommendations,
            "count": len(recommendations)
        }

    async def _generate_market_analysis_with_llm(
        self,
        economic_state: EconomicState,
        items: List[Dict],
        context: AgentContext
    ) -> str:
        """Generate market analysis using LLM"""

        try:
            system_message = """You are an economic analyst for a fantasy MMORPG market.
Provide concise, actionable market analysis."""

            user_prompt = f"""Analyze the current market:
- Inflation rate: {economic_state.inflation_rate:.2%}
- Market health: {economic_state.market_health:.2f}/1.0
- Dominant trend: {economic_state.dominant_trend}
- Trade volume: {economic_state.trade_volume:,} zeny
- Items tracked: {len(items)}

Provide a brief analysis (2-3 sentences) and one key recommendation."""

            analysis = await self._generate_with_llm(
                prompt=user_prompt,
                system_message=system_message,
                temperature=0.7
            )

            return analysis

        except Exception as e:
            logger.error(f"LLM analysis failed: {e}")
            return "Market analysis unavailable"

    def _parse_json(self, response_text: str) -> Optional[Dict[str, Any]]:
        """Parse JSON from LLM response"""
        try:
            return json.loads(response_text)
        except json.JSONDecodeError:
            if "```json" in response_text:
                start = response_text.find("```json") + 7
                end = response_text.find("```", start)
                json_str = response_text[start:end].strip()
                return json.loads(json_str)
            elif "```" in response_text:
                start = response_text.find("```") + 3
                end = response_text.find("```", start)
                json_str = response_text[start:end].strip()
                return json.loads(json_str)
            return None
    
    async def _simulate_complete_economy(self, context: AgentContext) -> Dict[str, Any]:
        """
        Run complete economic simulation cycle
        
        Includes:
        - Production chains
        - Trade routes
        - Economic agents
        - Resource depletion
        - Innovation checks
        """
        current_time = datetime.now(__import__('datetime').timezone.utc)
        delta_seconds = (current_time - self.last_simulation_update).total_seconds()
        self.last_simulation_update = current_time
        
        market_state = context.current_state.get("market_state", {})
        
        results = {
            "simulation_time": current_time.isoformat(),
            "delta_seconds": delta_seconds
        }
        
        try:
            # 1. Simulate production chains
            production_results = await self.economic_simulation.simulate_production_chains(market_state)
            results["production"] = production_results
            
            # Update statistics
            for item_id, qty in production_results.get("items_produced", {}).items():
                if "iron" in item_id:
                    self.game_statistics["total_iron_smelted"] += qty
                elif "potion" in item_id:
                    self.game_statistics["potions_crafted"] += qty
            
            # 2. Simulate trade routes
            trade_results = await self.economic_simulation.simulate_trade_routes(market_state)
            results["trade"] = trade_results
            self.game_statistics["total_trades"] += trade_results.get("routes_active", 0)
            
            # 3. Simulate economic agents
            agent_results = await self.economic_simulation.simulate_economic_agents(market_state)
            results["agents"] = agent_results
            
            # 4. Simulate resource depletion
            depletion_results = self.economic_simulation.simulate_resource_depletion(delta_seconds)
            results["resources"] = depletion_results
            
            # Track wood depletion
            if "wood" in depletion_results.get("resources_depleted", []):
                self.game_statistics["wood_depleted_count"] += 1
            
            # 5. Check for monopolies
            monopolies = self.economic_simulation.detect_monopoly(market_state)
            results["monopolies"] = monopolies
            
            # 6. Check innovations
            innovations = self.economic_simulation.check_innovation_triggers(self.game_statistics)
            results["innovations_discovered"] = [
                {
                    "id": inn.innovation_id,
                    "name": inn.name,
                    "discovered_at": inn.discovered_at.isoformat() if inn.discovered_at else None
                }
                for inn in innovations
            ]
            
            # 7. Get simulation state
            results["simulation_state"] = self.economic_simulation.get_simulation_state()
            
            logger.info(
                f"Economic simulation completed: "
                f"{production_results['chains_executed']} productions, "
                f"{trade_results['routes_active']} trades, "
                f"{agent_results['agents_active']} agents active"
            )
            
            return results
            
        except Exception as e:
            logger.error(f"Economic simulation failed: {e}")
            return {"error": str(e)}
    
    async def _add_economic_agent(self, context: AgentContext) -> Dict[str, Any]:
        """Add new economic agent to simulation"""
        npc_id = context.current_state.get("npc_id")
        agent_type_str = context.current_state.get("agent_type", "merchant")
        
        try:
            agent_type = EconomicAgentType(agent_type_str)
            agent = self.economic_simulation.add_economic_agent(npc_id, agent_type)
            
            return {
                "success": True,
                "agent_id": npc_id,
                "agent_type": agent_type.value,
                "starting_wealth": agent.wealth
            }
        except Exception as e:
            logger.error(f"Failed to add economic agent: {e}")
            return {"success": False, "error": str(e)}
    
    async def _check_innovations(self, context: AgentContext) -> Dict[str, Any]:
        """Check and trigger innovations"""
        innovations = self.economic_simulation.check_innovation_triggers(self.game_statistics)
        
        return {
            "innovations_discovered": len(innovations),
            "innovations": [
                {
                    "id": inn.innovation_id,
                    "name": inn.name,
                    "description": inn.description,
                    "effects": inn.effects,
                    "discovered_at": inn.discovered_at.isoformat() if inn.discovered_at else None
                }
                for inn in innovations
            ],
            "game_statistics": self.game_statistics
        }

