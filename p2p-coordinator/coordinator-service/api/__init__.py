"""
P2P Coordinator API Endpoints

FastAPI routers for P2P coordinator service.
"""

from api.hosts import router as hosts_router
from api.zones import router as zones_router
from api.signaling import router as signaling_router
from api.sessions import router as sessions_router

__all__ = [
    "hosts_router",
    "zones_router",
    "signaling_router",
    "sessions_router",
]

