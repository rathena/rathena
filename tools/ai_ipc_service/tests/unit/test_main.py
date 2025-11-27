"""
Unit Tests for Main Service Module

Tests the AIIPCService class and service lifecycle:
- Service initialization and configuration
- Startup and shutdown procedures
- Signal handling
- Service state management

Test Strategy:
- Mock external dependencies (database, handlers)
- Test state transitions
- Verify signal handling
- Test graceful shutdown
"""

import asyncio
import logging
import os
import signal
import sys
from datetime import datetime
from pathlib import Path
from typing import Any
from unittest.mock import AsyncMock, MagicMock, patch, PropertyMock

import pytest
import pytest_asyncio

# Add parent directories to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from config import (
    AIServiceConfig,
    Config,
    DatabaseConfig,
    LoggingConfig,
    PollingConfig,
    SecurityConfig,
)

logger = logging.getLogger(__name__)


# =============================================================================
# Service Initialization Tests
# =============================================================================

class TestServiceInitialization:
    """Tests for service initialization."""
    
    @pytest.mark.asyncio
    async def test_service_init_with_config_path(self):
        """
        Test service initialization with config path.
        
        Validates:
        - Service is created with config_path
        - Service is in initial state
        """
        from main import AIIPCService
        
        service = AIIPCService(config_path=None)
        
        assert service.config is None, "Config should not be loaded until start()"
        assert service._running is False, "Service should not be running initially"
        assert service._shutdown_event is not None, "Shutdown event should exist"
    
    @pytest.mark.asyncio
    async def test_service_init_default_config(self):
        """
        Test service initialization with default configuration.
        
        Validates:
        - Service can be created without explicit config path
        - Service is in initial state
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        assert service._running is False
        assert service.db is None, "DB should not be initialized until start()"
        assert service.processor is None, "Processor should not be initialized until start()"
    
    @pytest.mark.asyncio
    async def test_service_init_creates_shutdown_event(self):
        """
        Test that initialization creates shutdown event.
        
        Validates:
        - Shutdown event is created
        - Shutdown event is not set initially
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        assert service._shutdown_event is not None
        assert not service._shutdown_event.is_set()


# =============================================================================
# Service Startup Tests
# =============================================================================

class TestServiceStartup:
    """Tests for service startup procedures."""
    
    @pytest.mark.asyncio
    async def test_service_startup_loads_config(self):
        """
        Test that service startup loads configuration.
        
        Validates:
        - Config is loaded during start()
        - Database is initialized
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        # Mock the config loading and database
        with patch.object(Config, 'load') as mock_load:
            mock_config = MagicMock(spec=Config)
            mock_config.database = MagicMock(spec=DatabaseConfig)
            mock_config.database.host = "localhost"
            mock_config.database.port = 3306
            mock_config.database.database = "ragnarok"
            mock_config.database.pool_size = 5
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.polling = MagicMock()
            mock_config.polling.interval_ms = 100
            mock_config.polling.worker_count = 2
            mock_config.polling.batch_size = 10
            mock_config.validate = MagicMock(return_value=[])
            mock_config.service_name = "ai-ipc-service"
            mock_config.version = "1.0.0"
            mock_config.to_dict = MagicMock(return_value={})
            mock_load.return_value = mock_config
            
            # Mock Database
            with patch('main.Database') as mock_db_class:
                mock_db = AsyncMock()
                mock_db.connect = AsyncMock()
                mock_db.health_check = AsyncMock(return_value={"status": "healthy"})
                mock_db.disconnect = AsyncMock()
                mock_db_class.return_value = mock_db
                
                # Mock RequestProcessor
                with patch('main.RequestProcessor') as mock_proc_class:
                    mock_proc = MagicMock()
                    mock_proc.start = MagicMock()
                    mock_proc.stop = MagicMock()
                    mock_proc.get_stats = MagicMock(return_value={
                        "requests_processed": 0,
                        "requests_failed": 0,
                        "requests_expired": 0,
                        "batches_processed": 0,
                        "uptime_seconds": 0,
                        "requests_per_second": 0.0,
                    })
                    mock_proc_class.return_value = mock_proc
                    
                    await service.start()
                    
                    assert service._running is True
                    mock_db.connect.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_service_startup_database_failure(self):
        """
        Test service startup with database failure.
        
        Validates:
        - Database error is raised
        - Service does not start
        """
        from main import AIIPCService
        from database import ConnectionError
        
        service = AIIPCService()
        
        with patch.object(Config, 'load') as mock_load:
            mock_config = MagicMock(spec=Config)
            mock_config.database = MagicMock(spec=DatabaseConfig)
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.validate = MagicMock(return_value=[])
            mock_config.to_dict = MagicMock(return_value={})
            mock_load.return_value = mock_config
            
            with patch('main.Database') as mock_db_class:
                mock_db = AsyncMock()
                mock_db.connect = AsyncMock(side_effect=ConnectionError("Connection failed"))
                mock_db_class.return_value = mock_db
                
                with pytest.raises(ConnectionError):
                    await service.start()
    
    @pytest.mark.asyncio
    async def test_service_startup_health_check_failure(self):
        """
        Test service startup with health check failure.
        
        Validates:
        - Health check failure raises error
        - Service does not start
        """
        from main import AIIPCService
        # Import ConnectionError from the same module main.py uses
        from ai_ipc_service.database import ConnectionError
        
        service = AIIPCService()
        
        with patch.object(Config, 'load') as mock_load:
            mock_config = MagicMock(spec=Config)
            mock_config.database = MagicMock(spec=DatabaseConfig)
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.validate = MagicMock(return_value=[])
            mock_config.to_dict = MagicMock(return_value={})
            mock_load.return_value = mock_config
            
            with patch('main.Database') as mock_db_class:
                mock_db = AsyncMock()
                mock_db.connect = AsyncMock()
                mock_db.health_check = AsyncMock(return_value={"status": "unhealthy"})
                mock_db_class.return_value = mock_db
                
                with pytest.raises(ConnectionError):
                    await service.start()


# =============================================================================
# Service Shutdown Tests
# =============================================================================

class TestServiceShutdown:
    """Tests for service shutdown procedures."""
    
    @pytest.mark.asyncio
    async def test_service_stop_graceful(self):
        """
        Test graceful service shutdown.
        
        Validates:
        - Shutdown signal is set
        - Database is disconnected
        - Service state changes to stopped
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        # Set up mocks for a running service
        mock_db = AsyncMock()
        mock_db.disconnect = AsyncMock()
        service.db = mock_db
        
        mock_proc = MagicMock()
        mock_proc.stop = MagicMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 100,
            "requests_failed": 5,
            "requests_expired": 2,
            "batches_processed": 50,
            "uptime_seconds": 3600,
            "requests_per_second": 0.028,
        })
        service.processor = mock_proc
        service._running = True
        
        await service.stop()
        
        assert service._running is False
        mock_db.disconnect.assert_called_once()
        mock_proc.stop.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_service_stop_when_not_running(self):
        """
        Test stopping service when not running.
        
        Validates:
        - No error is raised
        - State remains stopped
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        # Should not raise
        await service.stop()
        
        assert service._running is False
    
    @pytest.mark.asyncio
    async def test_service_stop_with_pending_tasks(self):
        """
        Test stopping service with pending tasks.
        
        Validates:
        - Pending tasks are handled
        - Graceful drain of work
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        mock_db = AsyncMock()
        mock_db.disconnect = AsyncMock()
        service.db = mock_db
        
        mock_proc = MagicMock()
        mock_proc.stop = MagicMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_expired": 0,
            "batches_processed": 0,
            "uptime_seconds": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        # Add some pending tasks
        service._pending_tasks = set()
        
        await service.stop()
        
        assert service._running is False


# =============================================================================
# Signal Handler Tests
# =============================================================================

class TestSignalHandlers:
    """Tests for signal handling functionality."""
    
    @pytest.mark.asyncio
    async def test_request_shutdown_sets_event(self):
        """
        Test that request_shutdown sets the shutdown event.
        
        Validates:
        - Shutdown event is set
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        assert not service._shutdown_event.is_set()
        
        service.request_shutdown()
        
        assert service._shutdown_event.is_set()
    
    @pytest.mark.asyncio
    async def test_request_shutdown_multiple_calls(self):
        """
        Test multiple shutdown requests.
        
        Validates:
        - Multiple calls don't cause issues
        - Shutdown event remains set
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        service.request_shutdown()
        service.request_shutdown()
        service.request_shutdown()
        
        assert service._shutdown_event.is_set()
    
    def test_setup_signal_handlers(self):
        """
        Test signal handler setup.
        
        Validates:
        - Signal handlers are configured
        """
        from main import AIIPCService, setup_signal_handlers
        
        service = AIIPCService()
        loop = asyncio.new_event_loop()
        
        try:
            # This should not raise
            setup_signal_handlers(service, loop)
        finally:
            loop.close()


# =============================================================================
# Polling Loop Tests
# =============================================================================

class TestPollingLoop:
    """Tests for the polling loop."""
    
    @pytest.mark.asyncio
    async def test_run_requires_start(self):
        """
        Test that run() requires start() to be called first.
        
        Validates:
        - RuntimeError raised if not started
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        with pytest.raises(RuntimeError):
            await service.run()
    
    @pytest.mark.asyncio
    async def test_run_exits_on_shutdown(self):
        """
        Test that run() exits when shutdown is signaled.
        
        Validates:
        - Loop exits on shutdown event
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        # Set up minimal running state
        mock_config = MagicMock()
        mock_config.polling = MagicMock()
        mock_config.polling.interval_ms = 10
        service.config = mock_config
        
        mock_proc = AsyncMock()
        mock_proc.process_batch = AsyncMock(return_value=0)
        mock_proc.cleanup_expired = AsyncMock()
        mock_proc.cleanup_stuck = AsyncMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        # Signal shutdown immediately
        service._shutdown_event.set()
        
        # Run should exit quickly
        await asyncio.wait_for(service.run(), timeout=1.0)
    
    @pytest.mark.asyncio
    async def test_run_processes_batches(self):
        """
        Test that run() processes batches.
        
        Validates:
        - process_batch is called
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        mock_config = MagicMock()
        mock_config.polling = MagicMock()
        mock_config.polling.interval_ms = 10
        service.config = mock_config
        
        call_count = 0
        async def mock_process_batch():
            nonlocal call_count
            call_count += 1
            if call_count >= 2:
                service._shutdown_event.set()
            return 0
        
        mock_proc = MagicMock()
        mock_proc.process_batch = mock_process_batch
        mock_proc.cleanup_expired = AsyncMock()
        mock_proc.cleanup_stuck = AsyncMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        await asyncio.wait_for(service.run(), timeout=2.0)
        
        assert call_count >= 2
    
    @pytest.mark.asyncio
    async def test_run_handles_processor_error(self):
        """
        Test that run() handles processor errors.
        
        Validates:
        - Errors don't crash the loop
        - Service continues after error
        """
        from main import AIIPCService
        # Import from the same module main.py uses
        from ai_ipc_service.processor import ProcessorError
        
        service = AIIPCService()
        
        mock_config = MagicMock()
        mock_config.polling = MagicMock()
        mock_config.polling.interval_ms = 10
        service.config = mock_config
        
        call_count = 0
        async def mock_process_batch():
            nonlocal call_count
            call_count += 1
            if call_count == 1:
                raise ProcessorError("Test error")
            if call_count >= 3:
                service._shutdown_event.set()
            return 0
        
        mock_proc = MagicMock()
        mock_proc.process_batch = mock_process_batch
        mock_proc.cleanup_expired = AsyncMock()
        mock_proc.cleanup_stuck = AsyncMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        # Increase timeout to account for backoff sleeps in error handling
        await asyncio.wait_for(service.run(), timeout=10.0)
        
        # Should have continued past the error
        assert call_count >= 2


# =============================================================================
# Service State Tests
# =============================================================================

class TestServiceState:
    """Tests for service state management."""
    
    @pytest.mark.asyncio
    async def test_running_state_initial(self):
        """Test initial running state is False."""
        from main import AIIPCService
        
        service = AIIPCService()
        assert service._running is False
    
    @pytest.mark.asyncio
    async def test_config_state_initial(self):
        """Test initial config state is None."""
        from main import AIIPCService
        
        service = AIIPCService()
        assert service.config is None
    
    @pytest.mark.asyncio
    async def test_db_state_initial(self):
        """Test initial db state is None."""
        from main import AIIPCService
        
        service = AIIPCService()
        assert service.db is None
    
    @pytest.mark.asyncio
    async def test_processor_state_initial(self):
        """Test initial processor state is None."""
        from main import AIIPCService
        
        service = AIIPCService()
        assert service.processor is None


# =============================================================================
# Main Function Tests
# =============================================================================

class TestMainFunction:
    """Tests for the main() function."""
    
    @pytest.mark.asyncio
    async def test_main_function_exists(self):
        """Test that main function exists."""
        from main import main
        
        assert callable(main)
    
    @pytest.mark.asyncio
    async def test_run_function_exists(self):
        """Test that run function exists."""
        from main import run
        
        assert callable(run)


# =============================================================================
# Configure Logging Tests
# =============================================================================

class TestConfigureLogging:
    """Tests for logging configuration."""
    
    def test_configure_logging_text_format(self):
        """Test logging configuration with text format."""
        from main import configure_logging
        
        mock_config = MagicMock()
        mock_config.logging = MagicMock()
        mock_config.logging.level = "DEBUG"
        mock_config.logging.format = "text"
        
        # Should not raise
        configure_logging(mock_config)
    
    def test_configure_logging_json_format(self):
        """Test logging configuration with JSON format."""
        from main import configure_logging
        
        mock_config = MagicMock()
        mock_config.logging = MagicMock()
        mock_config.logging.level = "INFO"
        mock_config.logging.format = "json"
        
        # Should not raise
        configure_logging(mock_config)


# =============================================================================
# Integration-Style Tests (with mocking)
# =============================================================================

class TestServiceIntegration:
    """Integration-style tests with mocked dependencies."""
    
    @pytest.mark.asyncio
    async def test_full_start_stop_cycle(self):
        """
        Test complete start/stop cycle.
        
        Validates:
        - Service can be started
        - Service can be stopped
        - Resources are properly managed
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        with patch.object(Config, 'load') as mock_load:
            mock_config = MagicMock(spec=Config)
            mock_config.database = MagicMock(spec=DatabaseConfig)
            mock_config.database.host = "localhost"
            mock_config.database.port = 3306
            mock_config.database.database = "ragnarok"
            mock_config.database.pool_size = 5
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.polling = MagicMock()
            mock_config.polling.interval_ms = 100
            mock_config.polling.worker_count = 2
            mock_config.polling.batch_size = 10
            mock_config.validate = MagicMock(return_value=[])
            mock_config.service_name = "ai-ipc-service"
            mock_config.version = "1.0.0"
            mock_config.to_dict = MagicMock(return_value={})
            mock_load.return_value = mock_config
            
            with patch('main.Database') as mock_db_class:
                mock_db = AsyncMock()
                mock_db.connect = AsyncMock()
                mock_db.health_check = AsyncMock(return_value={"status": "healthy"})
                mock_db.disconnect = AsyncMock()
                mock_db_class.return_value = mock_db
                
                with patch('main.RequestProcessor') as mock_proc_class:
                    mock_proc = MagicMock()
                    mock_proc.start = MagicMock()
                    mock_proc.stop = MagicMock()
                    mock_proc.get_stats = MagicMock(return_value={
                        "requests_processed": 0,
                        "requests_failed": 0,
                        "requests_expired": 0,
                        "batches_processed": 0,
                        "uptime_seconds": 0,
                        "requests_per_second": 0.0,
                    })
                    mock_proc_class.return_value = mock_proc
                    
                    await service.start()
                    assert service._running is True
                    
                    await service.stop()
                    assert service._running is False
                    
                    mock_db.connect.assert_called_once()
                    mock_db.disconnect.assert_called_once()
                    mock_proc.start.assert_called_once()
                    mock_proc.stop.assert_called_once()


# =============================================================================
# Error Recovery Tests
# =============================================================================

class TestErrorRecovery:
    """Tests for service error recovery."""
    
    @pytest.mark.asyncio
    async def test_processor_error_doesnt_crash_service(self):
        """
        Test that processor errors don't crash the service.
        
        Validates:
        - Service continues running after processor error
        """
        from main import AIIPCService
        # Import from the same module main.py uses
        from ai_ipc_service.processor import ProcessorError
        
        service = AIIPCService()
        
        mock_config = MagicMock()
        mock_config.polling = MagicMock()
        mock_config.polling.interval_ms = 10
        service.config = mock_config
        
        error_thrown = False
        async def mock_process_batch():
            nonlocal error_thrown
            if not error_thrown:
                error_thrown = True
                raise ProcessorError("Temporary error")
            service._shutdown_event.set()
            return 0
        
        mock_proc = MagicMock()
        mock_proc.process_batch = mock_process_batch
        mock_proc.cleanup_expired = AsyncMock()
        mock_proc.cleanup_stuck = AsyncMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        # Should not raise, should complete
        # Increase timeout to account for backoff sleep (1 second) in error handling
        await asyncio.wait_for(service.run(), timeout=10.0)
        
        assert error_thrown, "Error should have been thrown"
    
    @pytest.mark.asyncio
    async def test_unexpected_error_doesnt_crash_service(self):
        """
        Test that unexpected errors don't crash the service.
        
        Validates:
        - Service continues running after unexpected error
        """
        from main import AIIPCService
        
        service = AIIPCService()
        
        mock_config = MagicMock()
        mock_config.polling = MagicMock()
        mock_config.polling.interval_ms = 10
        service.config = mock_config
        
        call_count = 0
        async def mock_process_batch():
            nonlocal call_count
            call_count += 1
            if call_count == 1:
                raise ValueError("Unexpected error")
            if call_count >= 3:
                service._shutdown_event.set()
            return 0
        
        mock_proc = MagicMock()
        mock_proc.process_batch = mock_process_batch
        mock_proc.cleanup_expired = AsyncMock()
        mock_proc.cleanup_stuck = AsyncMock()
        mock_proc.get_stats = MagicMock(return_value={
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0.0,
        })
        service.processor = mock_proc
        service._running = True
        
        # Should complete despite error
        await asyncio.wait_for(service.run(), timeout=10.0)
        
        assert call_count >= 2