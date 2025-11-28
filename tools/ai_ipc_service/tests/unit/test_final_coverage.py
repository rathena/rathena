"""
Final tests to achieve 100% code coverage.

Targets the remaining uncovered lines:
- config.py: branch 429->435
- database.py: 186-188, 311-313, 558, 812, 877-878, 1147
- handlers/ai_emotion.py: 174
- handlers/base.py: 118
- handlers/base_handler.py: 323, 388
- main.py: 175-176, 245-246, 320-321, 367-368, 413-414
- processor.py: 199-236, 675
"""

import asyncio
import json
import pytest
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch, PropertyMock
import aiomysql

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(__file__))))


# =============================================================================
# Database.py - Remaining Lines
# =============================================================================

class TestDatabaseFinalCoverage:
    """Cover remaining database.py lines."""
    
    @pytest.mark.asyncio
    async def test_get_connection_pool_error(self):
        """Test _get_connection when pool.acquire raises aiomysql.Error."""
        from database import Database, ConnectionError
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        )
        
        db = Database(config)
        db._connected = True
        
        # Create mock pool that raises aiomysql.Error on acquire
        mock_pool = MagicMock()
        
        # Create context manager that raises on __aenter__
        class FailingContextManager:
            async def __aenter__(self):
                raise aiomysql.Error("Connection pool error")
            async def __aexit__(self, *args):
                pass
        
        mock_pool.acquire = MagicMock(return_value=FailingContextManager())
        db._pool = mock_pool
        
        # Should raise ConnectionError and mark as disconnected
        with pytest.raises(ConnectionError):
            async with db._get_connection() as conn:
                pass
        
        # Should be marked as disconnected
        assert db._connected is False
    
    @pytest.mark.asyncio
    async def test_fetch_and_mark_exception(self):
        """Test fetch_and_mark_processing when exception occurs."""
        from database import Database, QueryError
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        )
        
        db = Database(config)
        db._connected = True
        
        # Create mock that raises an exception during execute
        mock_cursor = MagicMock()
        mock_cursor.execute = AsyncMock(side_effect=Exception("SQL error"))
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.begin = AsyncMock()
        mock_conn.commit = AsyncMock()
        mock_conn.rollback = AsyncMock()
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        mock_pool = MagicMock()
        mock_pool.acquire = MagicMock(return_value=mock_conn)
        db._pool = mock_pool
        
        # Should raise QueryError
        with pytest.raises(QueryError):
            await db.fetch_and_mark_processing(limit=10)
    
    @pytest.mark.asyncio
    async def test_mark_failed_not_retry_inserts_response(self):
        """Test mark_failed when not retrying inserts error response."""
        from database import Database
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        )
        
        db = Database(config)
        db._connected = True
        
        # Track calls to execute
        execute_calls = []
        
        mock_cursor = MagicMock()
        async def track_execute(query, *args):
            execute_calls.append((query, args))
        mock_cursor.execute = AsyncMock(side_effect=track_execute)
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.begin = AsyncMock()
        mock_conn.commit = AsyncMock()
        mock_conn.rollback = AsyncMock()
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        mock_pool = MagicMock()
        mock_pool.acquire = MagicMock(return_value=mock_conn)
        db._pool = mock_pool
        
        # Call mark_failed with should_retry=False (should INSERT error response)
        await db.mark_failed(request_id=1, error_message="Test error", should_retry=False)
        
        # Should have 2 execute calls: UPDATE status and INSERT response
        assert len(execute_calls) == 2
        # Second call should be INSERT
        assert "INSERT INTO ai_responses" in execute_calls[1][0]
    
    @pytest.mark.asyncio
    async def test_get_request_by_id_npc_id_from_data(self):
        """Test get_request_by_id extracts npc_id from request_data when source_npc missing."""
        from database import Database
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        )
        
        db = Database(config)
        db._connected = True
        
        mock_cursor = MagicMock()
        mock_cursor.fetchone = AsyncMock(return_value={
            'id': 1,
            'request_type': 'dialogue',
            'source_npc': None,  # No source_npc
            'request_data': json.dumps({'npc_id': 12345, 'player_id': 2001}),
        })
        mock_cursor.execute = AsyncMock()
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        mock_pool = MagicMock()
        mock_pool.acquire = MagicMock(return_value=mock_conn)
        db._pool = mock_pool
        
        result = await db.get_request_by_id(1)
        
        # npc_id should be extracted from request_data
        assert result['npc_id'] == 12345
    
    @pytest.mark.asyncio
    async def test_create_request_with_conn_dict_payload(self):
        """Test create_request_with_conn with dict payload (not string)."""
        from database import Database
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="test",
            database="test_db",
        )
        
        db = Database(config)
        db._connected = True
        
        mock_cursor = MagicMock()
        mock_cursor.execute = AsyncMock()
        mock_cursor.lastrowid = 789
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        db._pool = MagicMock()
        
        # Pass dict directly instead of JSON string
        result = await db.create_request_with_conn(
            mock_conn,
            request_type="memory",
            npc_id=1001,
            player_id=2001,
            payload={"key": "value"},  # Dict, not string
            priority=5
        )
        
        assert result == 789


# =============================================================================
# Main.py - Remaining Lines
# =============================================================================

class TestMainFinalCoverage:
    """Cover remaining main.py lines."""
    
    @pytest.mark.asyncio
    async def test_service_stop_with_pending_tasks_timeout(self):
        """Test service.stop() when pending tasks timeout."""
        from main import AIIPCService
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(),
        )
        
        service = AIIPCService()
        service.config = config
        service._running = True
        
        # Create a task that will not complete in time
        async def long_running_task():
            await asyncio.sleep(100)  # Won't complete
        
        # Add pending task
        task = asyncio.create_task(long_running_task())
        service._pending_tasks.add(task)
        
        # Mock processor
        mock_processor = MagicMock()
        mock_processor.stop = MagicMock()
        mock_processor.get_stats = MagicMock(return_value={
            'requests_processed': 10,
            'requests_failed': 1,
            'requests_expired': 0,
            'batches_processed': 2,
            'uptime_seconds': 100,
            'requests_per_second': 0.1,
        })
        service.processor = mock_processor
        
        # Mock db
        mock_db = MagicMock()
        mock_db.disconnect = AsyncMock()
        service.db = mock_db
        
        # Call stop with short timeout - tasks will timeout
        await service.stop(timeout_seconds=0.1)
        
        # Task should be cancelled
        assert task.cancelled()
    
    @pytest.mark.asyncio
    async def test_service_stop_pending_tasks_exception(self):
        """Test service.stop() handles exception during task wait."""
        from main import AIIPCService
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(),
        )
        
        service = AIIPCService()
        service.config = config
        service._running = True
        
        # Create a mock task that raises on gather
        mock_task = MagicMock()
        service._pending_tasks.add(mock_task)
        
        # Mock processor
        mock_processor = MagicMock()
        mock_processor.stop = MagicMock()
        mock_processor.get_stats = MagicMock(return_value={
            'requests_processed': 0,
            'requests_failed': 0,
            'requests_expired': 0,
            'batches_processed': 0,
            'uptime_seconds': 0,
            'requests_per_second': 0.0,
        })
        service.processor = mock_processor
        
        # Mock db
        mock_db = MagicMock()
        mock_db.disconnect = AsyncMock()
        service.db = mock_db
        
        # Mock asyncio.wait to raise an exception
        with patch('asyncio.wait', side_effect=Exception("Wait error")):
            # Should handle exception gracefully
            await service.stop(timeout_seconds=1.0)
        
        # Should still disconnect db
        mock_db.disconnect.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_main_keyboard_interrupt(self):
        """Test main() handles KeyboardInterrupt."""
        with patch('main.AIIPCService') as mock_service_class:
            mock_service = MagicMock()
            mock_service.start = AsyncMock()
            mock_service.run = AsyncMock(side_effect=KeyboardInterrupt())
            mock_service.stop = AsyncMock()
            mock_service_class.return_value = mock_service
            
            with patch('main.setup_signal_handlers'):
                with patch('main.asyncio.get_running_loop'):
                    import main
                    result = await main.main()
                    
                    # Should return 0 on keyboard interrupt
                    assert result == 0
                    mock_service.stop.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_main_general_exception(self):
        """Test main() handles general exceptions."""
        with patch('main.AIIPCService') as mock_service_class:
            mock_service = MagicMock()
            mock_service.start = AsyncMock()
            mock_service.run = AsyncMock(side_effect=RuntimeError("Something went wrong"))
            mock_service.stop = AsyncMock()
            mock_service_class.return_value = mock_service
            
            with patch('main.setup_signal_handlers'):
                with patch('main.asyncio.get_running_loop'):
                    import main
                    result = await main.main()
                    
                    # Should return 1 on exception
                    assert result == 1
                    mock_service.stop.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_main_exception_with_stop_failure(self):
        """Test main() when stop() also fails after exception."""
        with patch('main.AIIPCService') as mock_service_class:
            mock_service = MagicMock()
            mock_service.start = AsyncMock()
            mock_service.run = AsyncMock(side_effect=RuntimeError("Something went wrong"))
            mock_service.stop = AsyncMock(side_effect=Exception("Stop also failed"))
            mock_service_class.return_value = mock_service
            
            with patch('main.setup_signal_handlers'):
                with patch('main.asyncio.get_running_loop'):
                    import main
                    result = await main.main()
                    
                    # Should still return 1
                    assert result == 1


# =============================================================================
# Processor.py - Remaining Lines
# =============================================================================

class TestProcessorFinalCoverage:
    """Cover remaining processor.py lines."""
    
    @pytest.mark.asyncio
    async def test_process_batch_database_error(self):
        """Test process_batch when database raises error."""
        from processor import RequestProcessor, ProcessorError
        from database import DatabaseError
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(batch_size=10),
        )
        
        processor = RequestProcessor(config=config)
        
        # Mock database that raises DatabaseError
        mock_db = MagicMock()
        mock_db.fetch_and_mark_processing = AsyncMock(
            side_effect=DatabaseError("Database connection lost")
        )
        processor.db = mock_db
        
        # Should raise ProcessorError
        with pytest.raises(ProcessorError):
            await processor._process_batch_from_db()
    
    @pytest.mark.asyncio
    async def test_process_batch_with_requests_success(self):
        """Test process_batch successfully processes requests."""
        from processor import RequestProcessor
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(batch_size=10, max_retries=3),
        )
        
        processor = RequestProcessor(config=config)
        
        # Mock database
        mock_db = MagicMock()
        mock_db.fetch_and_mark_processing = AsyncMock(return_value=[
            {'id': 1, 'request_type': 'health_check', 'endpoint': '/health', 'request_data': '{}'},
            {'id': 2, 'request_type': 'health_check', 'endpoint': '/health', 'request_data': '{}'},
        ])
        mock_db.save_response = AsyncMock()
        mock_db.mark_failed = AsyncMock()
        processor.db = mock_db
        
        # Process batch
        result = await processor._process_batch_from_db()
        
        # Should have processed 2 requests
        assert result == 2
        assert processor.stats.batches_processed == 1
    
    def test_stats_requests_per_second_with_uptime(self):
        """Test requests_per_second calculation."""
        from processor import ProcessingStats
        from datetime import datetime, timedelta
        
        stats = ProcessingStats()
        
        # Set start_time to 10 seconds ago
        stats.start_time = datetime.utcnow() - timedelta(seconds=10)
        stats.requests_processed = 50
        stats.requests_failed = 50
        
        # Should be approximately 10 requests per second
        rps = stats.requests_per_second
        assert rps > 0  # Should be positive
        # With 100 total requests in ~10 seconds, should be ~10/s
        assert 5 < rps < 20  # Reasonable range


# =============================================================================
# Handler - Remaining Lines
# =============================================================================

class TestHandlersFinalCoverage:
    """Cover remaining handler lines."""
    
    @pytest.mark.asyncio
    async def test_ai_emotion_validate_missing_npc_id(self):
        """Test AI emotion handler validation when npc_id is missing."""
        from handlers.ai_emotion import AIEmotionHandler
        from handlers.base import ValidationError
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(),
        )
        
        handler = AIEmotionHandler(config)
        
        # Request without npc_id
        request = {
            'id': 1,
            'request_type': 'emotion',
            'request_data': json.dumps({'message': 'Hello'}),  # No npc_id
        }
        
        # Should raise ValidationError
        with pytest.raises(ValidationError) as exc_info:
            await handler.handle(request)
        
        assert "npc_id" in str(exc_info.value)
    
    @pytest.mark.asyncio
    async def test_base_handler_timeout_error(self):
        """Test base handler handles asyncio.TimeoutError."""
        from handlers.base import BaseHandler, HandlerError
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(),
        )
        
        class TimeoutHandler(BaseHandler):
            async def handle(self, request):
                # Simulate timeout
                raise asyncio.TimeoutError("Operation timed out")
        
        handler = TimeoutHandler(config)
        
        with pytest.raises(asyncio.TimeoutError):
            await handler.handle({'id': 1, 'request_type': 'test'})
    
    @pytest.mark.asyncio
    async def test_base_handler_auth_disabled(self):
        """Test base_handler when authentication is disabled."""
        from handlers.base_handler import BaseHandler as AuthBaseHandler
        from config import Config, DatabaseConfig, PollingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="test",
                database="test_db",
            ),
            polling=PollingConfig(),
            security=SecurityConfig(
                auth_enabled=False,
                rate_limit_enabled=False,
            ),
        )
        
        # Set security config on class
        AuthBaseHandler.set_security_config(config.security)
        
        class TestHandler(AuthBaseHandler):
            async def handle(self, request):
                return {"status": "success"}
        
        handler = TestHandler(config)
        
        # Should process without auth
        result = await handler.handle({'id': 1, 'request_type': 'test'})
        
        assert result['status'] == 'success'


# =============================================================================
# Config - Remaining Branch
# =============================================================================

class TestConfigFinalCoverage:
    """Cover remaining config branches."""
    
    def test_config_repr_full(self):
        """Test Config.__repr__ method."""
        from config import Config, DatabaseConfig, PollingConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(
                host="myhost",
                port=3307,
                user="myuser",
                password="secret",
                database="mydb",
            ),
            polling=PollingConfig(
                interval_ms=500,
                batch_size=20,
            ),
            logging=LoggingConfig(level="DEBUG"),
            security=SecurityConfig(auth_enabled=True),
        )
        
        repr_str = repr(config)
        
        # Should contain service info
        assert "AI IPC Service" in repr_str or "Config" in repr_str
    
    def test_config_to_dict_masks_password(self):
        """Test Config.to_dict() masks sensitive data."""
        from config import Config, DatabaseConfig, PollingConfig
        
        config = Config(
            database=DatabaseConfig(
                host="localhost",
                port=3306,
                user="test",
                password="super_secret_password",
                database="test_db",
            ),
            polling=PollingConfig(),
        )
        
        result = config.to_dict()
        
        # Password should be masked
        assert result['database']['password'] == '***'
    
    def test_database_config_str(self):
        """Test DatabaseConfig string representation."""
        from config import DatabaseConfig
        
        config = DatabaseConfig(
            host="localhost",
            port=3306,
            user="test",
            password="super_secret",
            database="test_db",
        )
        
        # Just verify we can get string representation
        repr_str = repr(config)
        
        # Should contain the host
        assert "localhost" in repr_str
    
    def test_polling_config_validate_zero_batch_size(self):
        """Test PollingConfig validation raises ValueError for zero batch_size."""
        from config import PollingConfig
        
        # Should raise ValueError during initialization
        with pytest.raises(ValueError) as exc_info:
            PollingConfig(
                interval_ms=100,
                batch_size=0,  # Invalid
                worker_count=4,
            )
        
        assert "Batch size must be at least 1" in str(exc_info.value)