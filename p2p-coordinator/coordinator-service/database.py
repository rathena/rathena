"""
Database Connection Management

Handles PostgreSQL and DragonflyDB (Redis-compatible) connections for the P2P coordinator service.
Uses SQLAlchemy 2.0 async engine and Redis async client (compatible with DragonflyDB).
"""

from typing import AsyncGenerator, Optional
from contextlib import asynccontextmanager

from sqlalchemy.ext.asyncio import (
    create_async_engine,
    AsyncSession,
    AsyncEngine,
    async_sessionmaker
)
from sqlalchemy.pool import NullPool
from redis.asyncio import Redis
from loguru import logger

from config import settings
from models.base import Base


class DatabaseManager:
    """
    Database Manager

    Manages PostgreSQL and DragonflyDB (Redis-compatible) connections with proper lifecycle management.
    """
    
    def __init__(self):
        self.postgres_engine: Optional[AsyncEngine] = None
        self.session_factory: Optional[async_sessionmaker[AsyncSession]] = None
        self.redis_client: Optional[Redis] = None
        
    async def initialize(self) -> None:
        """Initialize database connections"""
        logger.info("Initializing database connections...")
        
        # Initialize PostgreSQL
        await self._initialize_postgres()

        # Initialize DragonflyDB (Redis-compatible)
        await self._initialize_redis()
        
        logger.info("Database connections initialized successfully")
    
    async def _initialize_postgres(self) -> None:
        """Initialize PostgreSQL connection"""
        try:
            logger.info(f"Connecting to PostgreSQL at {settings.database.postgres_host}:{settings.database.postgres_port}")
            
            # Create async engine
            self.postgres_engine = create_async_engine(
                settings.database.postgres_url,
                echo=settings.service.env == "development",
                pool_pre_ping=True,
                pool_size=10,
                max_overflow=20,
            )
            
            # Create session factory
            self.session_factory = async_sessionmaker(
                self.postgres_engine,
                class_=AsyncSession,
                expire_on_commit=False,
            )
            
            # Create tables
            async with self.postgres_engine.begin() as conn:
                await conn.run_sync(Base.metadata.create_all)
            
            logger.info("PostgreSQL connection established and tables created")
            
        except Exception as e:
            logger.error(f"Failed to initialize PostgreSQL: {e}")
            raise
    
    async def _initialize_redis(self) -> None:
        """Initialize DragonflyDB connection (using Redis protocol)"""
        try:
            logger.info(f"Connecting to DragonflyDB at {settings.database.redis_host}:{settings.database.redis_port}")

            # Create DragonflyDB client (using Redis protocol)
            self.redis_client = Redis.from_url(
                settings.database.redis_url,
                encoding="utf-8",
                decode_responses=True,
            )

            # Test connection
            await self.redis_client.ping()

            logger.info("DragonflyDB connection established")

        except Exception as e:
            logger.error(f"Failed to initialize DragonflyDB: {e}")
            raise
    
    async def close(self) -> None:
        """Close all database connections"""
        logger.info("Closing database connections...")
        
        # Close PostgreSQL
        if self.postgres_engine:
            await self.postgres_engine.dispose()
            logger.info("PostgreSQL connection closed")

        # Close DragonflyDB
        if self.redis_client:
            await self.redis_client.close()
            logger.info("DragonflyDB connection closed")
        
        logger.info("All database connections closed")
    
    @asynccontextmanager
    async def get_session(self) -> AsyncGenerator[AsyncSession, None]:
        """
        Get a database session
        
        Usage:
            async with db_manager.get_session() as session:
                # Use session
                pass
        """
        if not self.session_factory:
            raise RuntimeError("Database not initialized. Call initialize() first.")
        
        async with self.session_factory() as session:
            try:
                yield session
                await session.commit()
            except Exception:
                await session.rollback()
                raise
            finally:
                await session.close()
    
    async def get_redis(self) -> Redis:
        """Get DragonflyDB client (using Redis protocol)"""
        if not self.redis_client:
            raise RuntimeError("DragonflyDB not initialized. Call initialize() first.")
        return self.redis_client


# Global database manager instance
db_manager = DatabaseManager()


# Dependency for FastAPI
async def get_db_session() -> AsyncGenerator[AsyncSession, None]:
    """FastAPI dependency for database sessions"""
    async with db_manager.get_session() as session:
        yield session


async def get_redis_client() -> Redis:
    """FastAPI dependency for DragonflyDB client (using Redis protocol)"""
    return await db_manager.get_redis()

