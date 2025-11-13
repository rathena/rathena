"""
Economic system data models
"""

from pydantic import BaseModel, Field
from typing import Dict, List, Optional, Any
from datetime import datetime
from enum import Enum


class MarketTrend(str, Enum):
    """Market trend direction"""
    RISING = "rising"
    FALLING = "falling"
    STABLE = "stable"
    VOLATILE = "volatile"


class ItemCategory(str, Enum):
    """Item categories for economic simulation"""
    CONSUMABLE = "consumable"
    EQUIPMENT = "equipment"
    MATERIAL = "material"
    RARE = "rare"
    QUEST = "quest"
    MISC = "misc"


class MarketItem(BaseModel):
    """Item in the market"""
    item_id: int
    item_name: str
    category: ItemCategory
    base_price: int  # Base price in zeny
    current_price: int  # Current market price
    supply: int = 0  # Available quantity
    demand: int = 0  # Demand level
    trend: MarketTrend = MarketTrend.STABLE
    price_history: List[Dict[str, Any]] = Field(default_factory=list)  # [{timestamp, price}]
    last_updated: datetime = Field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))


class TradeTransaction(BaseModel):
    """Record of a trade transaction"""
    transaction_id: str
    seller_id: str
    buyer_id: str
    item_id: int
    quantity: int
    price_per_unit: int
    total_price: int
    timestamp: datetime = Field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))
    location: str = "unknown"


class EconomicState(BaseModel):
    """Overall economic state of the world"""
    inflation_rate: float = 0.0  # -1.0 to 1.0
    trade_volume: int = 0  # Total zeny traded
    active_traders: int = 0
    market_health: float = 1.0  # 0.0 to 1.0
    dominant_trend: MarketTrend = MarketTrend.STABLE
    
    # Category-specific metrics
    category_trends: Dict[str, MarketTrend] = Field(default_factory=dict)
    category_volumes: Dict[str, int] = Field(default_factory=dict)
    
    # Time-based data
    daily_volume: int = 0
    weekly_volume: int = 0
    monthly_volume: int = 0
    
    last_updated: datetime = Field(default_factory=lambda: datetime.now(__import__('datetime').timezone.utc))


class PriceUpdateRequest(BaseModel):
    """Request to update item price"""
    item_id: int
    supply_change: int = 0
    demand_change: int = 0
    external_factor: Optional[float] = None  # -1.0 to 1.0


class MarketAnalysisRequest(BaseModel):
    """Request for market analysis"""
    item_ids: Optional[List[int]] = None
    categories: Optional[List[ItemCategory]] = None
    time_range: int = 24  # Hours


class MarketAnalysisResponse(BaseModel):
    """Market analysis response"""
    success: bool
    economic_state: Optional[EconomicState] = None
    item_analyses: List[Dict[str, Any]] = Field(default_factory=list)
    recommendations: List[str] = Field(default_factory=list)
    reasoning: Optional[str] = None


class TradeRecommendation(BaseModel):
    """AI-generated trade recommendation"""
    item_id: int
    item_name: str
    action: str  # "buy", "sell", "hold"
    confidence: float  # 0.0 to 1.0
    reasoning: str
    suggested_price: int
    expected_profit: Optional[int] = None
    risk_level: str = "medium"  # "low", "medium", "high"


class EconomicEvent(BaseModel):
    """Economic event that affects the market"""
    event_id: str
    event_type: str  # "shortage", "surplus", "crisis", "boom", "regulation"
    description: str
    affected_items: List[int] = Field(default_factory=list)
    affected_categories: List[ItemCategory] = Field(default_factory=list)
    price_multiplier: float = 1.0  # Multiplier for affected items
    duration: int = 3600  # Seconds
    severity: float = 0.5  # 0.0 to 1.0
    created_at: datetime = Field(default_factory=datetime.utcnow)
    expires_at: Optional[datetime] = None


class ShopInventory(BaseModel):
    """NPC shop inventory"""
    shop_id: str
    npc_id: str
    items: List[Dict[str, Any]] = Field(default_factory=list)  # [{item_id, quantity, price}]
    last_restock: datetime = Field(default_factory=datetime.utcnow)
    restock_interval: int = 3600  # Seconds
    pricing_strategy: str = "market_based"  # "fixed", "market_based", "dynamic"


class EconomicSimulationConfig(BaseModel):
    """Configuration for economic simulation"""
    enable_inflation: bool = True
    enable_supply_demand: bool = True
    enable_events: bool = True
    price_volatility: float = 0.1  # 0.0 to 1.0
    update_interval: int = 300  # Seconds
    max_price_change: float = 0.2  # Max 20% change per update
    event_frequency: float = 0.1  # Events per hour

