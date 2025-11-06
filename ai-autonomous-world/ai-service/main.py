"""
AI Service - Main FastAPI Application
"""

import sys
from pathlib import Path
from contextlib import asynccontextmanager

from fastapi import FastAPI, Request, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from loguru import logger
import uvicorn

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from ai_service.config import settings
from ai_service.database import db
from ai_service.routers import npc_router, world_router, player_router
from ai_service.routers.quest import router as quest_router


# Configure logging
def setup_logging():
    """Configure loguru logging"""
    # Remove default handler
    logger.remove()
    
    # Add console handler
    logger.add(
        sys.stderr,
        format="<green>{time:YYYY-MM-DD HH:mm:ss}</green> | <level>{level: <8}</level> | <cyan>{name}</cyan>:<cyan>{function}</cyan>:<cyan>{line}</cyan> - <level>{message}</level>",
        level=settings.log_level,
        colorize=True,
    )
    
    # Add file handler
    log_file = Path(settings.log_file)
    log_file.parent.mkdir(parents=True, exist_ok=True)
    
    logger.add(
        settings.log_file,
        format="{time:YYYY-MM-DD HH:mm:ss} | {level: <8} | {name}:{function}:{line} - {message}",
        level=settings.log_level,
        rotation="100 MB",
        retention="30 days",
        compression="zip",
    )
    
    logger.info("Logging configured")


# Lifespan context manager for startup/shutdown
@asynccontextmanager
async def lifespan(app: FastAPI):
    """Handle startup and shutdown events"""
    # Startup
    logger.info("=" * 80)
    logger.info(f"Starting {settings.service_name}")
    logger.info(f"Environment: {settings.environment}")
    logger.info(f"Debug mode: {settings.debug}")
    logger.info("=" * 80)
    
    # Connect to database
    try:
        await db.connect()
        logger.info("Database connection established")
    except Exception as e:
        logger.error(f"Failed to connect to database: {e}")
        raise
    
    # Initialize LLM provider (test connection)
    try:
        from ai_service.llm import get_llm_provider
        llm = get_llm_provider()
        logger.info(f"LLM provider initialized: {settings.default_llm_provider}")
    except Exception as e:
        logger.warning(f"LLM provider initialization failed: {e}")
    
    logger.info("AI Service startup complete")
    
    yield
    
    # Shutdown
    logger.info("Shutting down AI Service")
    
    # Disconnect from database
    try:
        await db.disconnect()
        logger.info("Database connection closed")
    except Exception as e:
        logger.error(f"Error closing database connection: {e}")
    
    logger.info("AI Service shutdown complete")


# Initialize logging
setup_logging()

# Create FastAPI app
app = FastAPI(
    title="AI Autonomous World Service",
    description="AI service for autonomous NPC behavior and world simulation",
    version="1.0.0",
    lifespan=lifespan,
    debug=settings.debug,
)

# Configure CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# Exception handlers
@app.exception_handler(Exception)
async def global_exception_handler(request: Request, exc: Exception):
    """Global exception handler"""
    logger.error(f"Unhandled exception: {exc}", exc_info=True)
    return JSONResponse(
        status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
        content={"detail": "Internal server error"}
    )


# Health check endpoint
@app.get("/health", tags=["health"])
async def health_check():
    """Health check endpoint"""
    db_healthy = await db.health_check()
    
    return {
        "status": "healthy" if db_healthy else "degraded",
        "service": settings.service_name,
        "version": "1.0.0",
        "environment": settings.environment,
        "database": "connected" if db_healthy else "disconnected",
    }


# Root endpoint
@app.get("/", tags=["root"])
async def root():
    """Root endpoint"""
    return {
        "service": settings.service_name,
        "version": "1.0.0",
        "status": "running",
        "docs": "/docs",
    }


# Include routers
app.include_router(npc_router)
app.include_router(world_router)
app.include_router(player_router)
app.include_router(quest_router)


# Run server
if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host=settings.service_host,
        port=settings.service_port,
        reload=settings.debug,
        log_level=settings.log_level.lower(),
    )

