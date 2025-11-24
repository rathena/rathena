"""
Trade Route Models
Defines trade connections between locations with autonomous behavior
"""

from pydantic import BaseModel, Field
from typing import List, Dict, Optional
from datetime import datetime
from enum import Enum


class RouteStatus(str, Enum):
    """Status of trade route"""
    ACTIVE = "active"
    INACTIVE = "inactive"
    BLOCKED = "blocked"
    SEASONAL = "seasonal"


class TradeRoute(BaseModel):
    """Trade connection between locations"""
    route_id: str
    from_location: str  # Map/town name
    to_location: str  # Map/town name
    established_by: List[str] = Field(default_factory=list)  # NPC IDs who created route
    goods_traded: Dict[str, int] = Field(default_factory=dict)  # {item_id: volume per cycle}
    transport_cost: float = 50.0  # Base cost in zeny
    travel_time: int = 300  # Seconds
    is_active: bool = True
    status: RouteStatus = RouteStatus.ACTIVE
    profitability: float = 0.0  # Calculated profit margin
    risk_level: int = 1  # 1-10 scale
    established_at: datetime = Field(default_factory=datetime.utcnow)
    last_trade: Optional[datetime] = None
    trade_count: int = 0
    
    def calculate_transport_cost(self, distance: float) -> float:
        """
        Calculate transport cost based on distance
        
        Args:
            distance: Distance in map units
            
        Returns:
            Total transport cost in zeny
        """
        # Base cost + distance factor + risk premium
        base = self.transport_cost
        distance_cost = distance * 0.5
        risk_premium = self.risk_level * 10.0
        
        return base + distance_cost + risk_premium
    
    def can_trade(self, current_time: datetime) -> bool:
        """Check if route can be used for trade"""
        if not self.is_active:
            return False
        if self.status == RouteStatus.BLOCKED:
            return False
        if self.status == RouteStatus.SEASONAL:
            # Check season (simplified - check month)
            month = current_time.month
            # Summer route (months 6-8)
            return 6 <= month <= 8
        return True
    
    def record_trade(self, items_traded: Dict[str, int], profit: float) -> None:
        """Record a completed trade"""
        self.last_trade = datetime.utcnow()
        self.trade_count += 1
        
        # Update goods volume (exponential moving average)
        for item_id, volume in items_traded.items():
            current = self.goods_traded.get(item_id, 0)
            # 0.7 weight to new data, 0.3 to old
            self.goods_traded[item_id] = int(current * 0.3 + volume * 0.7)
        
        # Update profitability (exponential moving average)
        self.profitability = self.profitability * 0.7 + profit * 0.3


class TradeNetwork(BaseModel):
    """Network of interconnected trade routes"""
    network_id: str
    routes: List[TradeRoute] = Field(default_factory=list)
    hubs: List[str] = Field(default_factory=list)  # Major trading hubs
    total_volume: int = 0
    created_at: datetime = Field(default_factory=datetime.utcnow)
    
    def add_route(self, route: TradeRoute) -> None:
        """Add route to network"""
        self.routes.append(route)
        
        # Update hubs
        for location in [route.from_location, route.to_location]:
            if location not in self.hubs:
                # Check if location has enough connections to be a hub (3+)
                connections = sum(
                    1 for r in self.routes
                    if r.from_location == location or r.to_location == location
                )
                if connections >= 3:
                    self.hubs.append(location)
    
    def find_route(self, from_loc: str, to_loc: str) -> Optional[TradeRoute]:
        """Find direct route between locations"""
        for route in self.routes:
            if route.from_location == from_loc and route.to_location == to_loc:
                return route
            # Check reverse direction
            if route.from_location == to_loc and route.to_location == from_loc:
                return route
        return None
    
    def find_best_path(
        self, from_loc: str, to_loc: str, optimize_for: str = "cost"
    ) -> Optional[List[TradeRoute]]:
        """
        Find best multi-hop path between locations
        
        Args:
            from_loc: Starting location
            to_loc: Destination location
            optimize_for: "cost", "time", or "profit"
            
        Returns:
            List of routes forming the path, or None if no path exists
        """
        # Simple BFS for now (can be enhanced with Dijkstra for optimization)
        from collections import deque
        
        queue = deque([(from_loc, [])])
        visited = {from_loc}
        
        while queue:
            current_loc, path = queue.popleft()
            
            if current_loc == to_loc:
                return path
            
            # Find connected routes
            for route in self.routes:
                if not route.can_trade(datetime.utcnow()):
                    continue
                
                next_loc = None
                if route.from_location == current_loc:
                    next_loc = route.to_location
                elif route.to_location == current_loc:
                    next_loc = route.from_location
                
                if next_loc and next_loc not in visited:
                    visited.add(next_loc)
                    queue.append((next_loc, path + [route]))
        
        return None
    
    def get_hub_statistics(self) -> Dict[str, Dict]:
        """Get statistics for each trading hub"""
        stats = {}
        
        for hub in self.hubs:
            connections = [
                r for r in self.routes
                if r.from_location == hub or r.to_location == hub
            ]
            
            total_volume = sum(
                sum(r.goods_traded.values()) for r in connections
            )
            
            avg_profitability = (
                sum(r.profitability for r in connections) / len(connections)
                if connections else 0.0
            )
            
            stats[hub] = {
                "connections": len(connections),
                "total_volume": total_volume,
                "avg_profitability": avg_profitability,
                "active_routes": sum(1 for r in connections if r.is_active)
            }
        
        return stats


class TradeCaravan(BaseModel):
    """Moving trade caravan on a route"""
    caravan_id: str
    route: TradeRoute
    merchant_npc_id: str
    cargo: Dict[str, int] = Field(default_factory=dict)  # {item_id: quantity}
    current_location: str
    destination: str
    progress: float = 0.0  # 0.0 to 1.0
    departure_time: datetime = Field(default_factory=datetime.utcnow)
    estimated_arrival: Optional[datetime] = None
    
    def update_progress(self, elapsed_seconds: int) -> bool:
        """
        Update caravan progress
        
        Args:
            elapsed_seconds: Time elapsed since last update
            
        Returns:
            True if destination reached
        """
        if self.progress >= 1.0:
            return True
        
        # Calculate progress increment
        travel_time = self.route.travel_time
        increment = elapsed_seconds / travel_time
        
        self.progress = min(1.0, self.progress + increment)
        
        return self.progress >= 1.0
    
    def calculate_cargo_value(self, market_prices: Dict[str, float]) -> float:
        """Calculate total cargo value"""
        total = 0.0
        for item_id, qty in self.cargo.items():
            price = market_prices.get(item_id, 0)
            total += price * qty
        return total


# Pre-defined major trade routes
MAJOR_ROUTES = [
    TradeRoute(
        route_id="prontera_geffen",
        from_location="prontera",
        to_location="geffen",
        goods_traded={"potion": 100, "equipment": 50},
        transport_cost=100.0,
        travel_time=600,
        risk_level=2,
        profitability=0.15
    ),
    TradeRoute(
        route_id="prontera_morroc",
        from_location="prontera",
        to_location="morocc",
        goods_traded={"food": 150, "cloth": 80},
        transport_cost=150.0,
        travel_time=900,
        risk_level=4,
        profitability=0.20
    ),
    TradeRoute(
        route_id="geffen_aldebaran",
        from_location="geffen",
        to_location="aldebaran",
        goods_traded={"magic_item": 30, "reagent": 100},
        transport_cost=120.0,
        travel_time=720,
        risk_level=3,
        profitability=0.25
    ),
    TradeRoute(
        route_id="aldebaran_juno",
        from_location="aldebaran",
        to_location="juno",
        goods_traded={"book": 40, "technology": 20},
        transport_cost=200.0,
        travel_time=1200,
        risk_level=5,
        profitability=0.30
    )
]