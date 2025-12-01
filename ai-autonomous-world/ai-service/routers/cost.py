"""
Cost Management API Endpoints

Provides endpoints for cost tracking, budget management, and reporting.
"""

from typing import Dict, Any, Optional
from fastapi import APIRouter, HTTPException, status, Query
from pydantic import BaseModel, Field
from loguru import logger

from services.cost_manager import (
    get_cost_manager,
    BudgetExceededException,
    ProviderBudgetExceededException
)


# ============================================================================
# REQUEST/RESPONSE MODELS
# ============================================================================

class BudgetUpdateRequest(BaseModel):
    """Request model for updating budget limits"""
    daily_budget_usd: Optional[float] = Field(
        None,
        description="New daily budget limit in USD",
        gt=0
    )
    provider_budgets: Optional[Dict[str, float]] = Field(
        None,
        description="Per-provider budget limits"
    )


class BudgetStatusResponse(BaseModel):
    """Response model for budget status"""
    total_spent_usd: float
    budget_usd: float
    remaining_usd: float
    percentage_used: float
    budget_available: bool
    enabled: bool


class CostBreakdownResponse(BaseModel):
    """Response model for detailed cost breakdown"""
    date: str
    total_cost_usd: float
    budget_usd: float
    budget_used_percentage: float
    budget_remaining_usd: float
    by_provider: Dict[str, float]
    by_model: Dict[str, float]
    by_request_type: Dict[str, float]
    total_requests: int
    enabled: bool


class ProviderBudgetResponse(BaseModel):
    """Response model for provider-specific budget status"""
    provider: str
    spent_today_usd: float
    budget_usd: Optional[float]
    remaining_usd: Optional[float]
    percentage_used: Optional[float]


# ============================================================================
# ROUTER
# ============================================================================

router = APIRouter(
    prefix="/api/v1/cost",
    tags=["cost-management"]
)


@router.get("/today", response_model=CostBreakdownResponse)
async def get_today_costs():
    """
    Get today's cost breakdown.
    
    Returns detailed breakdown of costs by provider, model, and request type.
    """
    try:
        cost_manager = get_cost_manager()
        breakdown = cost_manager.get_cost_breakdown()
        return breakdown
    except RuntimeError as e:
        logger.error(f"Cost manager not initialized: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Cost management not initialized"
        )
    except Exception as e:
        logger.error(f"Error retrieving cost breakdown: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to retrieve cost breakdown: {str(e)}"
        )


@router.get("/budget/status", response_model=BudgetStatusResponse)
async def get_budget_status():
    """
    Check current budget status.
    
    Returns overall budget utilization and availability.
    """
    try:
        cost_manager = get_cost_manager()
        total = cost_manager.get_total_cost_today()
        budget = cost_manager.daily_budget
        
        return BudgetStatusResponse(
            total_spent_usd=round(total, 6),
            budget_usd=budget,
            remaining_usd=round(max(0, budget - total), 6),
            percentage_used=round((total / budget) * 100, 2) if budget > 0 else 0,
            budget_available=total < budget,
            enabled=cost_manager.enabled
        )
    except RuntimeError as e:
        logger.error(f"Cost manager not initialized: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Cost management not initialized"
        )
    except Exception as e:
        logger.error(f"Error retrieving budget status: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to retrieve budget status: {str(e)}"
        )


@router.get("/budget/provider/{provider}", response_model=ProviderBudgetResponse)
async def get_provider_budget_status(provider: str):
    """
    Get budget status for specific provider.
    
    Args:
        provider: Provider name (e.g., "azure_openai", "openai")
    """
    try:
        cost_manager = get_cost_manager()
        spent_today = cost_manager.get_provider_cost_today(provider)
        provider_budget = cost_manager.per_provider_budgets.get(provider)
        
        response = ProviderBudgetResponse(
            provider=provider,
            spent_today_usd=round(spent_today, 6),
            budget_usd=provider_budget,
            remaining_usd=None,
            percentage_used=None
        )
        
        if provider_budget is not None:
            response.remaining_usd = round(max(0, provider_budget - spent_today), 6)
            response.percentage_used = round((spent_today / provider_budget) * 100, 2) if provider_budget > 0 else 0
        
        return response
    except RuntimeError as e:
        logger.error(f"Cost manager not initialized: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Cost management not initialized"
        )
    except Exception as e:
        logger.error(f"Error retrieving provider budget: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to retrieve provider budget: {str(e)}"
        )


@router.post("/budget/update")
async def update_budget(request: BudgetUpdateRequest):
    """
    Update budget limits.
    
    Allows updating daily budget and/or per-provider budgets.
    """
    try:
        cost_manager = get_cost_manager()
        
        if request.daily_budget_usd is not None:
            cost_manager.set_daily_budget(request.daily_budget_usd)
            logger.info(f"Daily budget updated to ${request.daily_budget_usd:.2f}")
        
        if request.provider_budgets:
            for provider, budget in request.provider_budgets.items():
                if budget <= 0:
                    raise HTTPException(
                        status_code=status.HTTP_400_BAD_REQUEST,
                        detail=f"Provider budget must be positive for {provider}"
                    )
                cost_manager.set_provider_budget(provider, budget)
            logger.info(f"Provider budgets updated: {request.provider_budgets}")
        
        return {
            "message": "Budget updated successfully",
            "daily_budget_usd": cost_manager.daily_budget,
            "per_provider_budgets": cost_manager.per_provider_budgets
        }
    except RuntimeError as e:
        logger.error(f"Cost manager not initialized: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Cost management not initialized"
        )
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Error updating budget: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to update budget: {str(e)}"
        )


@router.get("/history")
async def get_cost_history(
    limit: int = Query(
        default=100,
        ge=1,
        le=1000,
        description="Maximum number of entries to return"
    )
):
    """
    Get recent cost entries.
    
    Returns list of recent cost entries with details.
    
    Args:
        limit: Maximum number of entries (1-1000, default 100)
    """
    try:
        cost_manager = get_cost_manager()
        history = cost_manager.get_cost_history(limit=limit)
        
        return {
            "entries": history,
            "count": len(history),
            "limit": limit
        }
    except RuntimeError as e:
        logger.error(f"Cost manager not initialized: {e}")
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail="Cost management not initialized"
        )
    except Exception as e:
        logger.error(f"Error retrieving cost history: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail=f"Failed to retrieve cost history: {str(e)}"
        )


@router.get("/health")
async def cost_health_check():
    """
    Health check for cost management system.
    
    Returns status and basic information about cost tracking.
    """
    try:
        cost_manager = get_cost_manager()
        
        return {
            "status": "healthy",
            "enabled": cost_manager.enabled,
            "current_date": cost_manager.current_date.isoformat(),
            "daily_budget_usd": cost_manager.daily_budget,
            "total_cost_today_usd": round(cost_manager.get_total_cost_today(), 6),
            "requests_today": len(cost_manager.today_costs)
        }
    except RuntimeError:
        return {
            "status": "not_initialized",
            "enabled": False,
            "message": "Cost manager not initialized"
        }
    except Exception as e:
        logger.error(f"Cost health check failed: {e}")
        return {
            "status": "error",
            "enabled": False,
            "error": str(e)
        }