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
from ai_service.routers.batch import router as batch_router
from ai_service.routers.economy import router as economy_router
from ai_service.routers.faction import router as faction_router
from ai_service.routers.gift import router as gift_router
from ai_service.routers.gift import router as gift_router
from ai_service.routers.npc_spawning import router as npc_spawning_router
from ai_service.routers.world_bootstrap import router as world_bootstrap_router
from ai_service.routers.npc_movement import router as npc_movement_router
from ai_service.routers.mvp import router as mvp_router
from ai_service.routers.relationship import router as relationship_router
from ai_service.routers.mvp import router as mvp_router
from ai_service.middleware import (
    APIKeyMiddleware,
    RateLimitMiddleware,
    RequestSizeLimitMiddleware,
)

# Global instances
_openmemory_manager = None

def get_openmemory_manager():
    """Get the global OpenMemory manager instance"""
    return _openmemory_manager

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
        rotation="10 MB",  # Rotate at 10MB to prevent disk space issues
        retention="7 days",  # Keep logs for 7 days
        compression="gz",  # Compress rotated logs
        enqueue=True,  # Async logging to prevent blocking
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

    # Initialize OpenMemory SDK with PostgreSQL backend
    global _openmemory_manager
    try:
        from ai_service.memory.openmemory_manager import initialize_openmemory
        logger.info("Initializing OpenMemory SDK...")

        # Get OpenMemory backend URL from environment or use default
        openmemory_url = settings.openmemory_url if hasattr(settings, 'openmemory_url') else "http://localhost:8080"
        openmemory_key = settings.openmemory_api_key if hasattr(settings, 'openmemory_api_key') else ""

        _openmemory_manager = initialize_openmemory(
            base_url=openmemory_url,
            api_key=openmemory_key
        )

        if _openmemory_manager and _openmemory_manager.is_available():
            logger.info("✓ OpenMemory SDK initialized with PostgreSQL backend")
        else:
            logger.warning("⚠ OpenMemory SDK not available - using DragonflyDB fallback for memory")
    except ImportError as e:
        logger.warning(f"OpenMemory SDK dependencies not available: {e}")
        logger.info("Using DragonflyDB for conversation history (10-minute TTL)")
    except Exception as e:
        logger.error(f"OpenMemory SDK initialization failed: {e}", exc_info=True)
        logger.info("Using DragonflyDB for conversation history (10-minute TTL)")

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

    # Initialize NPC Relationship Manager
    try:
        from ai_service.tasks.npc_relationships import npc_relationship_manager
        await npc_relationship_manager.initialize()
        await npc_relationship_manager.start_background_tasks()
        logger.info("✓ NPC Relationship Manager started")
    except ImportError as e:
        logger.warning(f"NPC Relationship Manager dependencies not available: {e}")
    except Exception as e:
        logger.error(f"NPC Relationship Manager initialization failed: {e}", exc_info=True)

    # Initialize Instant Response System
    try:
        from ai_service.tasks.instant_response import instant_response_manager
        await instant_response_manager.start()
        logger.info("✓ Instant Response System started")
    except ImportError as e:
        logger.warning(f"Instant Response System dependencies not available: {e}")
    except Exception as e:
        logger.error(f"Instant Response System initialization failed: {e}", exc_info=True)

    # Initialize Environment System
    try:
        from ai_service.tasks.environment import initialize_environment
        await initialize_environment()
        logger.info("✓ Environment System initialized")
    except ImportError as e:
        logger.warning(f"Environment System dependencies not available: {e}")
    except Exception as e:
        logger.error(f"Environment System initialization failed: {e}", exc_info=True)

    # Initialize Universal Consciousness Engine
    try:
        from ai_service.agents.universal_consciousness import universal_consciousness
        await universal_consciousness.initialize()
        logger.info("✓ Universal Consciousness Engine initialized")
    except ImportError as e:
        logger.warning(f"Universal Consciousness Engine dependencies not available: {e}")
    except Exception as e:
        logger.error(f"Universal Consciousness Engine initialization failed: {e}", exc_info=True)

    # Initialize Decision Optimizer
    try:
        from ai_service.agents.decision_optimizer import decision_optimizer
        await decision_optimizer.initialize()
        logger.info("✓ Decision Optimizer initialized")
    except ImportError as e:
        logger.warning(f"Decision Optimizer dependencies not available: {e}")
    except Exception as e:
        logger.error(f"Decision Optimizer initialization failed: {e}", exc_info=True)

    logger.info("AI Service startup complete")
    logger.info("=" * 80)
    logger.info("ADVANCED AUTONOMOUS FEATURES ENABLED:")
    logger.info(f"  • NPC-to-NPC Interactions: {settings.npc_to_npc_interactions_enabled}")
    logger.info(f"  • Instant Response System: {settings.instant_response_enabled}")
    logger.info(f"  • Universal Consciousness: {settings.universal_consciousness_enabled}")
    logger.info(f"  • Reasoning Depth: {settings.reasoning_depth}")
    logger.info(f"  • LLM Optimization Mode: {settings.llm_optimization_mode}")
    logger.info(f"  • Decision Caching: {settings.decision_cache_enabled}")
    logger.info(f"  • Decision Batching: {settings.decision_batch_enabled}")
    logger.info("=" * 80)

    yield
    
    # Shutdown
    logger.info("Shutting down AI Service")

    # Stop Instant Response System
    try:
        from ai_service.tasks.instant_response import instant_response_manager
        await instant_response_manager.stop()
        logger.info("✓ Instant Response System stopped")
    except Exception as e:
        logger.error(f"Instant Response System shutdown error: {e}", exc_info=True)

    # Stop NPC Relationship Manager
    try:
        from ai_service.tasks.npc_relationships import npc_relationship_manager
        await npc_relationship_manager.stop_background_tasks()
        logger.info("✓ NPC Relationship Manager stopped")
    except Exception as e:
        logger.error(f"NPC Relationship Manager shutdown error: {e}", exc_info=True)

    # Stop background task scheduler
    try:
        from ai_service.scheduler import autonomous_scheduler
        autonomous_scheduler.stop()
        logger.info("✓ Autonomous task scheduler stopped")
    except Exception as e:
        logger.error(f"Scheduler shutdown error: {e}", exc_info=True)

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

# Add security and performance middleware
logger.info("Configuring middleware...")

# Request size limit (first to reject large requests early)
app.add_middleware(RequestSizeLimitMiddleware)
logger.info(f"✓ Request size limit: {settings.max_request_size} bytes")

# Rate limiting (second to prevent abuse)
if settings.rate_limit_enabled:
    app.add_middleware(RateLimitMiddleware)
    logger.info(f"✓ Rate limiting: {settings.rate_limit_requests} requests per {settings.rate_limit_period}s")
else:
    logger.warning("⚠ Rate limiting disabled")

# API key authentication (third to validate access)
if settings.api_key_required and settings.api_key:
    app.add_middleware(APIKeyMiddleware)
    logger.info(f"✓ API key authentication enabled (header: {settings.api_key_header})")
else:
    logger.warning("⚠ API key authentication disabled")


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
    from datetime import datetime
    return {
        "status": "healthy",
        "service": settings.service_name,
        "version": "1.0.0",
        "timestamp": datetime.utcnow().isoformat()
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
app.include_router(batch_router)
app.include_router(economy_router)
app.include_router(faction_router)
app.include_router(gift_router)
app.include_router(gift_router)
app.include_router(npc_spawning_router)
app.include_router(world_bootstrap_router)
app.include_router(npc_movement_router)
app.include_router(mvp_router)
app.include_router(relationship_router)
app.include_router(mvp_router)

logger.info("✓ All routers registered (14 routers)")


# Run server
if __name__ == "__main__":
    uvicorn.run(
        app,  # Use app object directly instead of string import
        host=settings.service_host,
        port=settings.service_port,
        reload=False,  # Disable reload when using app object
        log_level=settings.log_level.lower(),
    )

