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
from ai_service.database import db, postgres_db
from ai_service.routers import npc_router, world_router, player_router
from ai_service.routers.quest import router as quest_router
from ai_service.routers.chat_command import router as chat_command_router


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
    
    # Connect to DragonflyDB (cache and real-time state)
    try:
        await db.connect()
        logger.info("✓ DragonflyDB connection established")
    except ConnectionError as e:
        logger.error(f"DragonflyDB connection failed: {e}")
        raise
    except TimeoutError as e:
        logger.error(f"DragonflyDB connection timeout: {e}")
        raise
    except Exception as e:
        logger.error(f"Unexpected DragonflyDB error: {e}", exc_info=True)
        raise

    # Connect to PostgreSQL (persistent memory storage)
    try:
        await postgres_db.connect()
        logger.info("✓ PostgreSQL connection established")
    except ConnectionError as e:
        logger.error(f"PostgreSQL connection failed: {e}")
        raise
    except TimeoutError as e:
        logger.error(f"PostgreSQL connection timeout: {e}")
        raise
    except Exception as e:
        logger.error(f"Unexpected PostgreSQL error: {e}", exc_info=True)
        raise

    # Initialize GPU manager
    try:
        from ai_service.utils import initialize_gpu
        gpu_manager = initialize_gpu(settings)
        if gpu_manager.is_available():
            logger.info(f"GPU acceleration enabled: {gpu_manager.get_info().device_name}")
        else:
            logger.info("GPU acceleration disabled or unavailable")
    except ImportError as e:
        logger.warning(f"GPU dependencies not available: {e}")
    except RuntimeError as e:
        logger.warning(f"GPU runtime error: {e}")
    except Exception as e:
        logger.warning(f"GPU manager initialization failed: {e}", exc_info=True)

    # Initialize Memori SDK with PostgreSQL backend
    try:
        from ai_service.memory.memori_manager import initialize_memori
        memori_mgr = initialize_memori(
            connection_string=settings.postgres_connection_string,
            namespace="ai_world"
        )
        if memori_mgr.is_available():
            logger.info("✓ Memori SDK initialized with PostgreSQL backend")
        else:
            logger.warning("⚠ Memori SDK not available - using DragonflyDB fallback for memory")
    except ImportError as e:
        logger.warning(f"Memori SDK dependencies not available: {e}")
    except Exception as e:
        logger.warning(f"Memori SDK initialization failed: {e}", exc_info=True)

    # Initialize LLM provider (test connection)
    try:
        from ai_service.llm import get_llm_provider
        llm = get_llm_provider()
        logger.info(f"LLM provider initialized: {settings.default_llm_provider}")
        logger.debug(f"LLM provider type: {type(llm).__name__}")
    except ValueError as e:
        logger.error(f"LLM provider configuration error: {e}")
        raise
    except ImportError as e:
        logger.error(f"LLM provider dependencies missing: {e}")
        raise
    except Exception as e:
        logger.error(f"LLM provider initialization failed: {e}", exc_info=True)
        raise

    logger.info("AI Service startup complete")
    
    yield
    
    # Shutdown
    logger.info("Shutting down AI Service")

    # Shutdown Memori SDK
    try:
        from ai_service.memory.memori_manager import get_memori_manager
        memori_mgr = get_memori_manager()
        if memori_mgr:
            memori_mgr.shutdown()
            logger.info("✓ Memori SDK shutdown complete")
    except Exception as e:
        logger.error(f"Memori SDK shutdown error: {e}", exc_info=True)

    # Disconnect from PostgreSQL
    try:
        await postgres_db.disconnect()
        logger.info("✓ PostgreSQL connection closed")
    except Exception as e:
        logger.error(f"PostgreSQL disconnection error: {e}", exc_info=True)

    # Disconnect from DragonflyDB
    try:
        await db.disconnect()
        logger.info("✓ DragonflyDB connection closed")
    except Exception as e:
        logger.error(f"DragonflyDB disconnection error: {e}", exc_info=True)

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
    """
    Lightweight health check endpoint

    Returns basic service status without expensive operations.
    For detailed health check, use /health/detailed
    """
    return {
        "status": "healthy",
        "service": settings.service_name,
        "version": "1.0.0",
        "timestamp": logger._core.handlers[0]._sink._stream.name if hasattr(logger, '_core') else "N/A"
    }


@app.get("/health/detailed", tags=["health"])
async def detailed_health_check():
    """
    Detailed health check with database and service checks

    This endpoint performs actual connectivity tests and should be used
    for monitoring/alerting, not for load balancer health checks.
    """
    db_healthy = False
    db_error = None

    try:
        db_healthy = await db.health_check()
    except Exception as e:
        db_error = str(e)
        logger.warning(f"Database health check failed: {e}")

    return {
        "status": "healthy" if db_healthy else "degraded",
        "service": settings.service_name,
        "version": "1.0.0",
        "environment": settings.environment,
        "database": {
            "status": "connected" if db_healthy else "disconnected",
            "error": db_error
        },
        "gpu_enabled": settings.gpu_enabled,
        "llm_provider": settings.default_llm_provider
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
app.include_router(chat_command_router)


# Run server
if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host=settings.service_host,
        port=settings.service_port,
        reload=settings.debug,
        log_level=settings.log_level.lower(),
    )

