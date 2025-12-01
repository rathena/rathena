"""
Dashboard WebSocket Manager
Handles real-time updates for the dashboard
"""
from fastapi import WebSocket
from typing import Set, Dict, Any
import asyncio
import json
import logging
from datetime import datetime

logger = logging.getLogger(__name__)


class DashboardWebSocketManager:
    """Manages WebSocket connections and broadcasts for the dashboard"""
    
    def __init__(self):
        self.active_connections: Set[WebSocket] = set()
        self._update_interval = 5  # seconds
        self._is_running = False
        
    async def connect(self, websocket: WebSocket):
        """Accept and register a new WebSocket connection"""
        await websocket.accept()
        self.active_connections.add(websocket)
        logger.info(f"Dashboard WebSocket connected. Total connections: {len(self.active_connections)}")
        
    async def disconnect(self, websocket: WebSocket):
        """Remove a WebSocket connection"""
        self.active_connections.discard(websocket)
        logger.info(f"Dashboard WebSocket disconnected. Total connections: {len(self.active_connections)}")
        
    async def broadcast(self, message: Dict[str, Any]):
        """Broadcast message to all connected clients"""
        if not self.active_connections:
            return
            
        disconnected = set()
        
        for connection in self.active_connections:
            try:
                await connection.send_json(message)
            except Exception as e:
                logger.error(f"Error sending to client: {e}")
                disconnected.add(connection)
                
        # Clean up disconnected clients
        for connection in disconnected:
            await self.disconnect(connection)
    
    async def send_periodic_updates_to_client(self, websocket: WebSocket):
        """Send periodic updates to a specific client"""
        try:
            while websocket in self.active_connections:
                # Collect dashboard data
                data = await self.collect_dashboard_data()
                
                # Send to client
                await websocket.send_json(data)
                
                # Wait for next update
                await asyncio.sleep(self._update_interval)
                
        except asyncio.CancelledError:
            logger.info("Periodic update task cancelled")
        except Exception as e:
            logger.error(f"Error in periodic updates: {e}")
            
    async def collect_dashboard_data(self) -> Dict[str, Any]:
        """Collect aggregated dashboard data for broadcast"""
        try:
            # Import services
            from ..agents.agent_manager import agent_manager
            
            # Collect data from various sources
            agents_status = await agent_manager.get_all_agents_status()
            
            # Get quick metrics
            active_agents = sum(1 for agent in agents_status if agent.get('status') == 'active')
            error_agents = sum(1 for agent in agents_status if agent.get('status') == 'error')
            
            return {
                "type": "dashboard_update",
                "timestamp": datetime.utcnow().isoformat(),
                "data": {
                    "agents": {
                        "total": len(agents_status),
                        "active": active_agents,
                        "errors": error_agents
                    },
                    "server": {
                        "status": "healthy",
                        "uptime": 0
                    },
                    "players": {
                        "online": 0
                    }
                }
            }
        except Exception as e:
            logger.error(f"Error collecting dashboard data: {e}")
            return {
                "type": "error",
                "timestamp": datetime.utcnow().isoformat(),
                "message": str(e)
            }
    
    async def send_agent_update(self, agent_data: Dict[str, Any]):
        """Send agent status update to all clients"""
        message = {
            "type": "agent_update",
            "timestamp": datetime.utcnow().isoformat(),
            "data": agent_data
        }
        await self.broadcast(message)
        
    async def send_event_created(self, event_data: Dict[str, Any]):
        """Send new event notification to all clients"""
        message = {
            "type": "event_created",
            "timestamp": datetime.utcnow().isoformat(),
            "data": event_data
        }
        await self.broadcast(message)
        
    async def send_economy_snapshot(self, economy_data: Dict[str, Any]):
        """Send economy snapshot to all clients"""
        message = {
            "type": "economy_snapshot",
            "timestamp": datetime.utcnow().isoformat(),
            "data": economy_data
        }
        await self.broadcast(message)
        
    async def send_health_alert(self, alert_data: Dict[str, Any]):
        """Send health alert to all clients"""
        message = {
            "type": "health_alert",
            "timestamp": datetime.utcnow().isoformat(),
            "data": alert_data
        }
        await self.broadcast(message)
        
    async def send_story_progress(self, story_data: Dict[str, Any]):
        """Send story progression update to all clients"""
        message = {
            "type": "story_progress",
            "timestamp": datetime.utcnow().isoformat(),
            "data": story_data
        }
        await self.broadcast(message)


# Global singleton instance
dashboard_ws_manager = DashboardWebSocketManager()