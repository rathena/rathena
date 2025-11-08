"""
Economy Router - Economic Simulation Endpoints
Provides access to AI-driven economic system
"""

from fastapi import APIRouter, HTTPException, status
from pydantic import BaseModel, Field
from typing import List, Dict, Any, Optional
from datetime import datetime
from loguru import logger

from config import settings
from models.economy import (
    EconomicState,
    MarketItem,
    ShopInventory,
    EconomicEvent,
    MarketTrend
)


router = APIRouter(prefix="/ai/economy", tags=["economy"])


# ============================================================================
# Request/Response Models
# ============================================================================

class PriceUpdateRequest(BaseModel):
    """Request to update item price"""
    item_id: str = Field(..., description="Item ID")
    new_price: int = Field(..., gt=0, description="New price in zeny")
    reason: Optional[str] = Field(None, description="Reason for price change")


class MarketAnalysisRequest(BaseModel):
    """Request for market analysis"""
    item_ids: Optional[List[str]] = Field(None, description="Specific items to analyze")
    category: Optional[str] = Field(None, description="Item category to analyze")
    time_range_days: int = Field(default=7, ge=1, le=90, description="Days of history to analyze")


class MarketAnalysisResponse(BaseModel):
    """Market analysis response"""
    analysis_time: datetime
    items_analyzed: int
    trends: List[MarketTrend]
    recommendations: List[Dict[str, Any]]
    market_health: str  # "healthy", "volatile", "stagnant"
    summary: str


class ShopRestockRequest(BaseModel):
    """Request to restock shop"""
    shop_id: str = Field(..., description="Shop ID")
    force: bool = Field(default=False, description="Force restock even if not needed")


# ============================================================================
# Economy State Endpoints
# ============================================================================

@router.get("/state", response_model=EconomicState)
async def get_economic_state():
    """
    Get current economic state
    
    Returns overall economic indicators, inflation rate, market trends
    """
    try:
        from tasks.economy import get_current_economic_state
        
        state = await get_current_economic_state()
        
        logger.info("Economic state retrieved")
        return state
    
    except Exception as e:
        logger.error(f"Error getting economic state: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get economic state: {str(e)}"
        )


@router.post("/update")
async def trigger_economic_update():
    """
    Manually trigger economic simulation update
    
    Useful for testing or admin commands
    Normally runs automatically based on economy_update_mode
    """
    try:
        if not settings.economy_enabled:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Economic simulation is disabled"
            )
        
        from tasks.economy import update_economic_simulation
        
        result = await update_economic_simulation()
        
        logger.info("Manual economic update triggered")
        return {
            "success": True,
            "message": "Economic update completed",
            "timestamp": datetime.utcnow().isoformat(),
            "updates": result
        }
    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error triggering economic update: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update economy: {str(e)}"
        )


# ============================================================================
# Price Management Endpoints
# ============================================================================

@router.post("/price/update")
async def update_item_price(request: PriceUpdateRequest):
    """
    Update item price (admin/event-driven)
    
    EconomyAgent will learn from this price change and its effects
    """
    try:
        from tasks.economy import update_item_price_with_learning
        
        result = await update_item_price_with_learning(
            item_id=request.item_id,
            new_price=request.new_price,
            reason=request.reason
        )
        
        logger.info(f"Price updated for item {request.item_id}: {request.new_price} zeny")
        return {
            "success": True,
            "item_id": request.item_id,
            "old_price": result.get("old_price"),
            "new_price": request.new_price,
            "timestamp": datetime.utcnow().isoformat()
        }
    
    except Exception as e:
        logger.error(f"Error updating price for {request.item_id}: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update price: {str(e)}"
        )


@router.get("/price/{item_id}")
async def get_item_price(item_id: str):
    """Get current price and price history for an item"""
    try:
        from tasks.economy import get_item_price_info
        
        price_info = await get_item_price_info(item_id)
        
        if not price_info:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Item {item_id} not found"
            )
        
        return price_info
    
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error getting price for {item_id}: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get price: {str(e)}"
        )


# ============================================================================
# Market Analysis Endpoints
# ============================================================================

@router.post("/market/analyze", response_model=MarketAnalysisResponse)
async def analyze_market(request: MarketAnalysisRequest):
    """
    Get AI-powered market analysis

    EconomyAgent analyzes market trends, supply/demand, and provides recommendations
    """
    try:
        from tasks.economy import analyze_market_with_agent

        analysis = await analyze_market_with_agent(
            item_ids=request.item_ids,
            category=request.category,
            time_range_days=request.time_range_days
        )

        logger.info(f"Market analysis completed: {analysis.items_analyzed} items")
        return analysis

    except Exception as e:
        logger.error(f"Error analyzing market: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to analyze market: {str(e)}"
        )


@router.get("/trends")
async def get_market_trends(
    category: Optional[str] = None,
    limit: int = 20
):
    """Get current market trends"""
    try:
        from tasks.economy import get_market_trends

        trends = await get_market_trends(category=category, limit=limit)

        return {
            "trends": trends,
            "timestamp": datetime.utcnow().isoformat(),
            "category": category or "all"
        }

    except Exception as e:
        logger.error(f"Error getting market trends: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get trends: {str(e)}"
        )


# ============================================================================
# Shop Inventory Endpoints
# ============================================================================

@router.get("/shop/{shop_id}/inventory", response_model=ShopInventory)
async def get_shop_inventory(shop_id: str):
    """Get shop inventory"""
    try:
        from tasks.economy import get_shop_inventory

        inventory = await get_shop_inventory(shop_id)

        if not inventory:
            raise HTTPException(
                status_code=status.HTTP_404_NOT_FOUND,
                detail=f"Shop {shop_id} not found"
            )

        return inventory

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error getting shop inventory for {shop_id}: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get inventory: {str(e)}"
        )


@router.post("/shop/{shop_id}/restock")
async def restock_shop(shop_id: str, request: ShopRestockRequest):
    """
    Trigger shop restocking

    In npc_driven mode, shop owner NPC decides what/how much to restock
    In fixed_interval mode, automatic restocking
    """
    try:
        if not settings.shop_restock_enabled:
            raise HTTPException(
                status_code=status.HTTP_400_BAD_REQUEST,
                detail="Shop restocking is disabled"
            )

        from tasks.shop_restock import restock_shop_with_npc_decision

        result = await restock_shop_with_npc_decision(
            shop_id=shop_id,
            force=request.force
        )

        logger.info(f"Shop {shop_id} restocked: {result.get('items_restocked', 0)} items")
        return {
            "success": True,
            "shop_id": shop_id,
            "items_restocked": result.get("items_restocked", 0),
            "total_cost": result.get("total_cost", 0),
            "npc_decision": result.get("npc_decision", {}),
            "timestamp": datetime.utcnow().isoformat()
        }

    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error restocking shop {shop_id}: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to restock shop: {str(e)}"
        )


# ============================================================================
# Economic Events Endpoints
# ============================================================================

@router.get("/events")
async def get_economic_events(
    active_only: bool = True,
    limit: int = 10
):
    """Get economic events (market crashes, booms, shortages, etc.)"""
    try:
        from tasks.economy import get_economic_events

        events = await get_economic_events(active_only=active_only, limit=limit)

        return {
            "events": events,
            "count": len(events),
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error getting economic events: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to get events: {str(e)}"
        )


@router.post("/events/trigger")
async def trigger_economic_event(
    event_type: str,
    severity: float = 1.0,
    duration_hours: int = 24
):
    """
    Trigger an economic event (admin command)

    Event types: market_crash, market_boom, shortage, surplus, inflation_spike
    """
    try:
        from tasks.economy import trigger_economic_event

        event = await trigger_economic_event(
            event_type=event_type,
            severity=severity,
            duration_hours=duration_hours
        )

        logger.info(f"Economic event triggered: {event_type} (severity: {severity})")
        return {
            "success": True,
            "event": event,
            "timestamp": datetime.utcnow().isoformat()
        }

    except Exception as e:
        logger.error(f"Error triggering economic event: {e}", exc_info=True)
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to trigger event: {str(e)}"
        )

