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
    from ai_service.agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from ai_service.models.economy import (
        MarketItem, EconomicState, MarketTrend, ItemCategory,
        EconomicEvent, TradeRecommendation
    )
except ModuleNotFoundError:
    from agents.base_agent import BaseAIAgent, AgentContext, AgentResponse
    from models.economy import (
        MarketItem, EconomicState, MarketTrend, ItemCategory,
        EconomicEvent, TradeRecommendation
    )


class EconomyAgent(BaseAIAgent):
    """
    Specialized agent for economic simulation
    
    Manages:
    - Price fluctuations based on supply/demand
    - Market trends and analysis
    - Economic events generation
    - Trade recommendations
    - Inflation and deflation
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
        
        self.crew_agent = self._create_crew_agent()
        logger.info(f"Economy Agent {agent_id} initialized")
    
    def _create_crew_agent(self) -> Agent:
        """Create CrewAI agent for economic simulation"""
        # Import CrewAI's LLM class
        from crewai import LLM
        import os

        # Create CrewAI-compatible LLM using litellm format for Azure OpenAI
        try:
            from ai_service.config import settings

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
    
    async def process(self, context: AgentContext) -> AgentResponse:
        """
        Process economic simulation task
        
        Args:
            context: Agent context with operation type
            
        Returns:
            AgentResponse with economic data
        """
        try:
            operation = context.current_state.get("operation", "analyze")
            
            logger.info(f"Economy Agent processing: {operation}")
            
            if operation == "update_prices":
                result = await self._update_market_prices(context)
            elif operation == "analyze":
                result = await self._analyze_market(context)
            elif operation == "generate_event":
                result = await self._generate_economic_event(context)
            elif operation == "recommend_trade":
                result = await self._recommend_trade(context)
            else:
                logger.warning(f"Unknown operation: {operation}")
                result = {"error": f"Unknown operation: {operation}"}
            
            return AgentResponse(
                agent_type=self.agent_type,
                success=True,
                data=result,
                confidence=0.8,
                reasoning=f"Completed {operation} operation",
                metadata={"operation": operation},
                timestamp=datetime.utcnow()
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
                timestamp=datetime.utcnow()
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
            item.last_updated = datetime.utcnow()
            
            # Add to price history
            item.price_history.append({
                "timestamp": datetime.utcnow().isoformat(),
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
            last_updated=datetime.utcnow()
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

            event_id = f"event_{int(datetime.utcnow().timestamp())}"

            event = EconomicEvent(
                event_id=event_id,
                event_type=event_type,
                description=event_data.get("description", "An economic event occurred"),
                affected_categories=event_data.get("affected_categories", []),
                price_multiplier=event_data.get("price_multiplier", 1.0),
                duration=event_data.get("duration", 3600),
                severity=event_data.get("severity", 0.5),
                expires_at=datetime.utcnow() + timedelta(seconds=event_data.get("duration", 3600))
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

