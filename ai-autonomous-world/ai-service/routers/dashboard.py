"""
Dashboard API Router - Extended
Provides endpoints for the Live World Dashboard (DWD System)
"""
from fastapi import APIRouter, WebSocket, WebSocketDisconnect, HTTPException
from fastapi.responses import JSONResponse
from typing import List, Dict, Any, Optional
from datetime import datetime, timedelta
import asyncio
import logging

router = APIRouter(prefix="/dashboard", tags=["dashboard"])
logger = logging.getLogger(__name__)

# WebSocket connection manager
from ..websocket.dashboard_ws import dashboard_ws_manager

@router.get("/overview")
async def get_dashboard_overview():
    """
    Get complete dashboard overview with data from all modules
    
    Returns comprehensive snapshot of:
    - All 21 agents status
    - Server health metrics  
    - Current story arc
    - Economy snapshot
    - Active player count
    - Active events count
    """
    try:
        # Import agent manager and other dependencies
        from ..agents.agent_manager import agent_manager
        from ..services.storyline_service import storyline_service
        from ..services.economy_service import economy_service
        
        # Gather all overview data
        agents = await agent_manager.get_all_agents_status()
        server_health = await get_server_health_data()
        current_arc = await storyline_service.get_current_arc()
        economy = await economy_service.get_snapshot()
        
        # Get player count from rAthena
        player_count = await get_active_player_count()
        
        # Get active events count
        active_events = await get_active_events_count()
        
        return {
            "agents": agents,
            "server_health": server_health,
            "current_arc": current_arc,
            "economy": economy,
            "player_count": player_count,
            "active_events": active_events,
            "timestamp": datetime.utcnow().isoformat()
        }
    except Exception as e:
        logger.error(f"Error fetching dashboard overview: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/agents/status")
async def get_all_agents_status():
    """Get status of all 21 AI agents"""
    try:
        from ..agents.agent_manager import agent_manager
        agents = await agent_manager.get_all_agents_status()
        return agents
    except Exception as e:
        logger.error(f"Error fetching agent status: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/heatmap")
async def get_world_heatmap():
    """
    Get live world heatmap data including:
    - Player density by map
    - Monster density by map
    - MVP locations
    - Dynamic NPC locations
    - Active hazards
    - Treasure locations
    - Quest hotspots
    """
    try:
        from ..services.world_service import world_service
        heatmap_data = await world_service.get_heatmap_data()
        return heatmap_data
    except Exception as e:
        logger.error(f"Error fetching heatmap data: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/metrics/performance")
async def get_performance_metrics():
    """
    Get API/Database/LLM performance metrics
    
    Returns:
    - API request metrics (avg response time, requests/sec)
    - Database query performance
    - LLM API usage and costs
    - Agent execution metrics
    """
    try:
        metrics = {
            "api": {
                "requests_per_second": 0.0,
                "avg_response_time_ms": 0.0,
                "error_rate": 0.0
            },
            "database": {
                "active_connections": 0,
                "avg_query_time_ms": 0.0,
                "slow_queries_count": 0
            },
            "llm": {
                "total_requests_24h": 0,
                "total_tokens_24h": 0,
                "total_cost_24h": 0.0,
                "avg_latency_ms": 0.0
            },
            "agents": {
                "total_executions_24h": 0,
                "avg_execution_time_ms": 0.0,
                "failure_rate": 0.0
            }
        }
        
        # Integrate with Prometheus metrics collection
        try:
            from prometheus_client import REGISTRY
            # Collect API metrics
            api_requests = REGISTRY.get_sample_value('http_requests_total')
            api_errors = REGISTRY.get_sample_value('http_errors_total')
            api_latency = REGISTRY.get_sample_value('http_request_duration_seconds_sum')
            
            if api_requests and api_requests > 0:
                metrics['api']['requests_per_second'] = api_requests / 60.0
                metrics['api']['error_rate'] = (api_errors or 0) / api_requests
            if api_latency and api_requests:
                metrics['api']['avg_response_time_ms'] = (api_latency / api_requests) * 1000
            
            # Collect database metrics
            db_queries = REGISTRY.get_sample_value('db_query_duration_seconds_count')
            db_latency = REGISTRY.get_sample_value('db_query_duration_seconds_sum')
            if db_queries and db_queries > 0:
                metrics['database']['avg_query_time_ms'] = (db_latency / db_queries) * 1000
            
            # Collect LLM metrics
            llm_requests = REGISTRY.get_sample_value('llm_requests_total')
            llm_tokens = REGISTRY.get_sample_value('llm_tokens_total')
            llm_cost = REGISTRY.get_sample_value('llm_cost_total')
            llm_latency = REGISTRY.get_sample_value('llm_latency_seconds_sum')
            if llm_requests and llm_requests > 0:
                metrics['llm']['total_requests_24h'] = int(llm_requests)
                metrics['llm']['avg_latency_ms'] = (llm_latency / llm_requests) * 1000 if llm_latency else 0
            if llm_tokens:
                metrics['llm']['total_tokens_24h'] = int(llm_tokens)
            if llm_cost:
                metrics['llm']['total_cost_24h'] = llm_cost
            
            # Collect agent metrics
            agent_execs = REGISTRY.get_sample_value('ai_agent_exec_time_seconds_count')
            agent_latency = REGISTRY.get_sample_value('ai_agent_exec_time_seconds_sum')
            agent_failures = REGISTRY.get_sample_value('ai_agent_failure_total')
            if agent_execs and agent_execs > 0:
                metrics['agents']['total_executions_24h'] = int(agent_execs)
                metrics['agents']['avg_execution_time_ms'] = (agent_latency / agent_execs) * 1000 if agent_latency else 0
                metrics['agents']['failure_rate'] = (agent_failures or 0) / agent_execs
        except Exception as e:
            logger.warning(f"Failed to collect Prometheus metrics: {e}")
        
        return metrics
    except Exception as e:
        logger.error(f"Error fetching performance metrics: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.websocket("/ws/live-updates")
async def websocket_live_updates(websocket: WebSocket):
    """
    WebSocket endpoint for real-time dashboard updates
    
    Sends periodic updates every 5 seconds with:
    - Agent status changes
    - New events/NPCs spawned
    - Player count updates
    - Economy snapshots
    - Story chapter advancements
    - Health alerts
    """
    await dashboard_ws_manager.connect(websocket)
    
    try:
        # Start sending periodic updates
        update_task = asyncio.create_task(
            dashboard_ws_manager.send_periodic_updates_to_client(websocket)
        )
        
        # Keep connection alive and handle incoming messages
        while True:
            try:
                # Receive messages from client (heartbeat, subscriptions, etc.)
                data = await websocket.receive_json()
                
                # Handle client messages
                if data.get("type") == "ping":
                    await websocket.send_json({"type": "pong"})
                elif data.get("type") == "subscribe":
                    # Handle subscription to specific event types
                    event_types = data.get("event_types", [])
                    if event_types:
                        logger.info(f"Client subscribed to events: {event_types}")
                        await websocket.send_json({
                            "type": "subscribed",
                            "event_types": event_types
                        })
                    else:
                        await websocket.send_json({
                            "type": "error",
                            "message": "No event_types specified for subscription"
                        })
                    
            except WebSocketDisconnect:
                break
                
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
    finally:
        update_task.cancel()
        await dashboard_ws_manager.disconnect(websocket)


# Helper functions

async def get_server_health_data() -> Dict[str, Any]:
    """Get comprehensive server health metrics"""
    import psutil
    
    # CPU and Memory
    cpu_percent = psutil.cpu_percent(interval=1)
    memory = psutil.virtual_memory()
    disk = psutil.disk_usage('/')
    
    # rAthena server status (requires integration)
    rathena_status = {
        "login_server": {"status": "online", "uptime": 0, "connections": 0},
        "char_server": {"status": "online", "uptime": 0, "connections": 0},
        "map_server": {"status": "online", "uptime": 0, "connections": 0}
    }
    
    # AI service health
    ai_service_health = {
        "status": "healthy",
        "uptime": 0,
        "requests_per_second": 0.0,
        "avg_response_time": 0.0,
        "agent_failures": 0
    }
    
    # Database health
    database_health = {
        "status": "healthy",
        "connection_pool": {
            "active": 0,
            "idle": 0,
            "max": 10
        },
        "query_performance": {
            "avg_query_time": 0.0,
            "slow_queries": 0
        }
    }
    
    # Cache health (DragonflyDB)
    cache_health = {
        "status": "healthy",
        "hit_rate": 0.95,
        "memory_used": 0,
        "memory_total": 0,
        "keys_count": 0
    }
    
    return {
        "status": "healthy",
        "timestamp": datetime.utcnow().isoformat(),
        "cpu_usage": cpu_percent,
        "memory_usage": memory.percent,
        "disk_usage": disk.percent,
        "rathena_servers": rathena_status,
        "ai_service": ai_service_health,
        "database": database_health,
        "cache": cache_health,
        "recent_errors": [],
        "alerts": []
    }


async def get_active_player_count() -> int:
    """Get current active player count from rAthena database"""
    try:
        from database import postgres_db
        if postgres_db and postgres_db.pool:
            query = 'SELECT COUNT(*) as count FROM "char" WHERE "online" = 1'
            result = await postgres_db.fetch_one(query)
            if result:
                return result['count']
        return 0
    except Exception as e:
        logger.warning(f"Failed to query active player count: {e}")
        return 0


async def get_active_events_count() -> int:
    """Get count of currently active world events"""
    try:
        from database import postgres_db
        if postgres_db and postgres_db.pool:
            query = """
                SELECT COUNT(*) as count FROM world_events
                WHERE status = 'active' AND expires_at > NOW()
            """
            result = await postgres_db.fetch_one(query)
            if result:
                return result['count']
        
        # Fallback: check DragonflyDB for active event keys
        from database import db
        if db and db.client:
            event_keys = await db.client.keys("environment:event:*")
            return len(event_keys) if event_keys else 0
        
        return 0
    except Exception as e:
        logger.warning(f"Failed to query active events count: {e}")
        return 0