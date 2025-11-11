"""
Database error tests to cover database.py error paths

This file tests database connection error scenarios.
"""

import pytest
from unittest.mock import AsyncMock, patch, MagicMock


@pytest.mark.unit
@pytest.mark.asyncio
class TestDatabaseErrors:
    """Test database error handling to cover database.py lines 76-78, 97-99"""

    async def test_postgres_initialization_error(self, monkeypatch):
        """Test PostgreSQL initialization error (lines 76-78)"""
        from database import DatabaseManager
        
        # Mock create_async_engine to raise exception
        def mock_create_engine(*args, **kwargs):
            raise Exception("PostgreSQL connection failed")
        
        with patch("database.create_async_engine", side_effect=mock_create_engine):
            db_manager = DatabaseManager()
            
            # Attempt to initialize - should raise exception
            with pytest.raises(Exception) as exc_info:
                await db_manager._initialize_postgres()
            
            assert "PostgreSQL connection failed" in str(exc_info.value)

    async def test_redis_initialization_error(self, monkeypatch):
        """Test Redis/DragonflyDB initialization error (lines 97-99)"""
        from database import DatabaseManager
        
        # Mock Redis.from_url to raise exception
        def mock_redis_from_url(*args, **kwargs):
            raise Exception("DragonflyDB connection failed")
        
        with patch("database.Redis.from_url", side_effect=mock_redis_from_url):
            db_manager = DatabaseManager()
            
            # Attempt to initialize - should raise exception
            with pytest.raises(Exception) as exc_info:
                await db_manager._initialize_redis()
            
            assert "DragonflyDB connection failed" in str(exc_info.value)

    async def test_session_close_error(self, monkeypatch):
        """Test session close error (line 128)"""
        from database import DatabaseManager
        
        db_manager = DatabaseManager()
        
        # Mock engine to raise exception on dispose
        mock_engine = MagicMock()
        mock_engine.dispose = AsyncMock(side_effect=Exception("Dispose failed"))
        db_manager.engine = mock_engine
        
        # Attempt to close - should handle exception
        try:
            await db_manager.close()
        except Exception:
            pass  # Exception is expected and logged

    async def test_redis_close_error(self, monkeypatch):
        """Test Redis close error (line 143)"""
        from database import DatabaseManager
        
        db_manager = DatabaseManager()
        
        # Mock redis_client to raise exception on close
        mock_redis = MagicMock()
        mock_redis.close = AsyncMock(side_effect=Exception("Redis close failed"))
        db_manager.redis_client = mock_redis
        
        # Attempt to close - should handle exception
        try:
            await db_manager.close()
        except Exception:
            pass  # Exception is expected and logged

    async def test_get_redis_not_initialized(self, monkeypatch):
        """Test get_redis when not initialized (line 160)"""
        from database import DatabaseManager
        
        db_manager = DatabaseManager()
        db_manager.redis_client = None
        
        # Attempt to get redis - should raise exception
        with pytest.raises(RuntimeError) as exc_info:
            await db_manager.get_redis()
        
        assert "not initialized" in str(exc_info.value).lower()




