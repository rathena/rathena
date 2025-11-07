"""
P2P Coordinator Service - Main Application

FastAPI application for P2P coordinator service.
Handles host registration, zone management, and WebRTC signaling.
"""

import sys
from contextlib import asynccontextmanager
from typing import AsyncGenerator

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from loguru import logger

from config import settings
from database import db_manager
from api.hosts import router as hosts_router
from api.zones import router as zones_router
from api.signaling import router as signaling_router
from api.sessions import router as sessions_router
from api.monitoring import router as monitoring_router
from services.ai_integration import ai_service_client
from services.background_tasks import background_tasks


# Configure logging
logger.remove()  # Remove default handler
logger.add(
    sys.stderr,
    format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
    level=settings.monitoring.log_level,
    colorize=True,
)

# Add file logging
logger.add(
    "logs/p2p-coordinator.log",
    rotation="500 MB",
    retention="10 days",
    level=settings.monitoring.log_level,
    format="{time:YYYY-MM-DD HH:mm:ss} | {level: <8} | {name}:{function}:{line} - {message}",
)


@asynccontextmanager
async def lifespan(app: FastAPI) -> AsyncGenerator[None, None]:
    """
    Application lifespan manager
    
    Handles startup and shutdown events for the FastAPI application.
    """
    # Startup
    logger.info("=" * 80)
    logger.info(f"Starting P2P Coordinator Service v{settings.service.version}")
    logger.info(f"Environment: {settings.service.env}")
    logger.info(f"Host: {settings.service.host}:{settings.service.port}")
    logger.info("=" * 80)
    
    try:
        # Initialize database connections
        await db_manager.initialize()

        # Initialize AI service client if enabled
        if settings.ai_service.ai_service_enabled:
            await ai_service_client.initialize()
            logger.info("✅ AI Service client initialized")

        # Start background tasks
        await background_tasks.start()

        logger.info("✅ P2P Coordinator Service started successfully")
        logger.info(f"P2P Enabled: {settings.p2p_features.enable_p2p}")
        logger.info(f"Enabled Zones: {settings.p2p_features.enabled_zones_list}")

        yield

    finally:
        # Shutdown
        logger.info("Shutting down P2P Coordinator Service...")

        # Stop background tasks
        await background_tasks.stop()

        # Close AI service client
        if settings.ai_service.ai_service_enabled:
            await ai_service_client.close()
            logger.info("✅ AI Service client closed")

        await db_manager.close()
        logger.info("✅ P2P Coordinator Service shut down successfully")


# Create FastAPI application
app = FastAPI(
    title="P2P Coordinator Service",
    description="Server-side coordinator for hybrid P2P MMORPG architecture",
    version=settings.service.version,
    lifespan=lifespan,
)

# Add CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.security.cors_origins_list,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Include API routers
app.include_router(hosts_router)
app.include_router(zones_router)
app.include_router(signaling_router)
app.include_router(sessions_router)
app.include_router(monitoring_router)


@app.get("/")
async def root():
    """Root endpoint - service information"""
    return {
        "service": settings.service.name,
        "version": settings.service.version,
        "status": "running",
        "p2p_enabled": settings.p2p_features.enable_p2p,
        "enabled_zones": settings.p2p_features.enabled_zones_list,
    }


@app.get("/health")
async def health_check():
    """Health check endpoint"""
    try:
        # Check database connections
        redis = await db_manager.get_redis()
        await redis.ping()
        
        return {
            "status": "healthy",
            "service": settings.service.name,
            "version": settings.service.version,
            "database": {
                "postgres": "connected",
                "redis": "connected",
            },
        }
    except Exception as e:
        logger.error(f"Health check failed: {e}")
        return JSONResponse(
            status_code=503,
            content={
                "status": "unhealthy",
                "error": str(e),
            },
        )


@app.get("/metrics")
async def metrics():
    """Prometheus metrics endpoint"""
    if not settings.monitoring.prometheus_enabled:
        return JSONResponse(
            status_code=404,
            content={"error": "Metrics endpoint is disabled"},
        )
    
    # TODO: Implement Prometheus metrics
    return {
        "message": "Metrics endpoint - to be implemented in Phase 2",
    }


if __name__ == "__main__":
    import uvicorn
    
    uvicorn.run(
        "main:app",
        host=settings.service.host,
        port=settings.service.port,
        reload=settings.service.env == "development",
        log_level=settings.monitoring.log_level.lower(),
    )

