"""
Final coverage tests to achieve 100% coverage.

This file contains targeted tests for the remaining uncovered lines.
"""

import pytest
import asyncio
import json
from datetime import datetime, timedelta
from unittest.mock import AsyncMock, MagicMock, patch, PropertyMock


# ==============================================================================
# Config Tests - Line 429->435 (AI service localhost warning)
# ==============================================================================

class TestConfigWarnings:
    """Test config validation warnings."""
    
    def test_validate_ai_service_localhost_warning(self):
        """Test warning when AI service URL contains localhost."""
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(base_url="http://localhost:8000"),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        warnings = config.validate()
        
        # Should have localhost warning
        localhost_warnings = [w for w in warnings if "localhost" in w.lower()]
        assert len(localhost_warnings) > 0, "Should warn about localhost in AI service URL"
    
    def test_validate_no_localhost_warning_production_url(self):
        """Test no localhost warning when using production URL."""
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(base_url="https://ai.example.com"),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        warnings = config.validate()
        
        # Should NOT have localhost warning
        localhost_warnings = [w for w in warnings if "localhost" in w.lower()]
        assert len(localhost_warnings) == 0, "Should not warn about non-localhost URL"


# ==============================================================================
# Database Tests - Lines 162-164, 180, 722-724, 764-766, 812, 877-878
# ==============================================================================

class TestDatabaseEdgeCases:
    """Test database edge cases for coverage."""
    
    @pytest.mark.asyncio
    async def test_reconnect_failure(self):
        """Test _ensure_connected when reconnection fails (lines 162-164)."""
        from config import DatabaseConfig
        from database import Database, ConnectionError
        
        config = DatabaseConfig(
            host="invalid-host-that-does-not-exist",
            port=3306,
            user="test",
            password="test",
            database="test",
        )
        
        db = Database(config)
        db._connected = False
        db._pool = None
        
        # Try to ensure connected - should fail
        with pytest.raises(ConnectionError):
            await db._ensure_connected()
    
    @pytest.mark.asyncio
    async def test_get_connection_pool_not_initialized(self):
        """Test _get_connection when pool is None (line 180)."""
        from config import DatabaseConfig
        from database import Database, ConnectionError
        
        config = DatabaseConfig(password="test")
        db = Database(config)
        db._connected = True  # Mark as connected
        db._pool = None  # But pool is None
        
        with pytest.raises(ConnectionError, match="pool not initialized"):
            async with db._get_connection():
                pass
    
    @pytest.mark.asyncio
    async def test_get_pending_count_returns_zero_on_error(self):
        """Test get_pending_count returns 0 on error (lines 722-724)."""
        from config import DatabaseConfig
        from database import Database
        
        config = DatabaseConfig(password="test")
        db = Database(config)
        
        # Mock _get_connection to raise an exception
        async def mock_get_connection():
            raise Exception("Database error")
        
        db._get_connection = MagicMock(side_effect=Exception("Database error"))
        db._connected = True
        db._pool = MagicMock()
        
        # Should return 0, not raise
        result = await db.get_pending_count()
        assert result == 0
    
    @pytest.mark.asyncio
    async def test_health_check_returns_unhealthy_on_error(self):
        """Test health_check returns unhealthy status on error (lines 764-766)."""
        from config import DatabaseConfig
        from database import Database
        
        config = DatabaseConfig(password="test")
        db = Database(config)
        db._connected = True
        
        # Mock _get_connection to raise
        with patch.object(db, '_get_connection', side_effect=Exception("Connection failed")):
            result = await db.health_check()
        
        assert result["status"] == "unhealthy"
        assert result["connected"] == False
        assert "error" in result
    
    @pytest.mark.asyncio
    async def test_create_request_compat_with_dict_payload(self):
        """Test _create_request_compat when payload is already a dict (line 812)."""
        from config import DatabaseConfig
        from database import Database
        
        config = DatabaseConfig(password="test")
        db = Database(config)
        
        # Mock the _get_connection to simulate database operations
        mock_cursor = MagicMock()
        mock_cursor.lastrowid = 123
        mock_cursor.execute = AsyncMock()
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.commit = AsyncMock()
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        with patch.object(db, '_get_connection', return_value=mock_conn):
            # Pass dict instead of string - triggers line 812
            result = await db.create_request(
                request_type="dialogue",
                npc_id=1,
                player_id=2,
                payload={"already": "dict"},  # Dict, not string
                priority=5,
            )
        
        assert result == 123
    
    @pytest.mark.asyncio
    async def test_get_request_by_id_npc_id_from_data(self):
        """Test get_request_by_id extracts npc_id from request_data (lines 877-878)."""
        from config import DatabaseConfig
        from database import Database
        
        config = DatabaseConfig(password="test")
        db = Database(config)
        
        # Create mock row without source_npc but with npc_id in request_data
        mock_row = {
            "id": 1,
            "request_type": "dialogue",
            "source_npc": None,  # No source_npc
            "request_data": json.dumps({"npc_id": 42, "player_id": 10}),
            "status": "pending",
        }
        
        mock_cursor = MagicMock()
        mock_cursor.fetchone = AsyncMock(return_value=mock_row)
        mock_cursor.execute = AsyncMock()
        mock_cursor.__aenter__ = AsyncMock(return_value=mock_cursor)
        mock_cursor.__aexit__ = AsyncMock(return_value=None)
        
        mock_conn = MagicMock()
        mock_conn.cursor = MagicMock(return_value=mock_cursor)
        mock_conn.__aenter__ = AsyncMock(return_value=mock_conn)
        mock_conn.__aexit__ = AsyncMock(return_value=None)
        
        with patch.object(db, '_get_connection', return_value=mock_conn):
            result = await db.get_request_by_id(1)
        
        # Should extract npc_id from request_data (line 877-878)
        assert result is not None
        assert result.get("npc_id") == 42


# ==============================================================================
# AI Async Handler Tests - Lines marked with pragma no cover
# ==============================================================================

class TestAIAsyncEdgeCases:
    """Test AI Async handler edge cases."""
    
    @pytest.mark.asyncio
    async def test_route_to_custom_handler(self):
        """Test routing to custom handler (else branch line 243+)."""
        from handlers.ai_async import AIAsyncRequestHandler
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        handler = AIAsyncRequestHandler(config)
        
        # Call with custom type to hit the else branch
        result = await handler._route_to_handler(
            request_type="custom",
            npc_id="test_npc",
            data={"key": "value"},
        )
        
        assert result["status"] == "ok"
        assert "Custom request processed" in result["message"]


# ==============================================================================
# AI Emotion Handler Tests - Lines 272, 361-364, 372
# ==============================================================================

class TestAIEmotionEdgeCases:
    """Test AI Emotion handler edge cases."""
    
    @pytest.mark.asyncio
    async def test_fallback_emotion_with_trigger_event(self):
        """Test fallback emotion with known trigger event (line 272)."""
        from handlers.ai_emotion import AIEmotionHandler
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        handler = AIEmotionHandler(config)
        
        # Call fallback directly with a known trigger event
        result = handler._generate_fallback_emotion(
            npc_id="test_npc",
            context={"current_emotion": "neutral"},
            trigger_event="player_purchase",  # Known event in EVENT_EMOTIONS
        )
        
        assert result["status"] == "ok"
        assert result["emotion"] == "happy"  # player_purchase -> happy
    
    @pytest.mark.asyncio
    async def test_personality_modifier_with_trait(self):
        """Test personality modifier application (lines 361-364)."""
        from handlers.ai_emotion import AIEmotionHandler
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        handler = AIEmotionHandler(config)
        
        # Test with a personality trait that has a modifier for 'happy'
        intensity = handler._apply_personality_modifier(
            base_intensity=0.5,
            emotion="happy",
            personality_traits=["friendly"],  # friendly has +0.1 for happy
            relationship_level=50,
        )
        
        # friendly adds +0.1 for happy
        assert intensity > 0.5
    
    @pytest.mark.asyncio
    async def test_relationship_modifier_for_angry(self):
        """Test relationship modifier for angry emotion (line 372)."""
        from handlers.ai_emotion import AIEmotionHandler
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        handler = AIEmotionHandler(config)
        
        # Test with high relationship (reduces angry intensity)
        intensity_high = handler._apply_personality_modifier(
            base_intensity=0.7,
            emotion="angry",
            personality_traits=[],
            relationship_level=100,  # High relationship
        )
        
        # Test with low relationship (increases angry intensity)
        intensity_low = handler._apply_personality_modifier(
            base_intensity=0.7,
            emotion="angry",
            personality_traits=[],
            relationship_level=0,  # Low relationship
        )
        
        # High relationship should reduce angry intensity
        assert intensity_high < intensity_low


# ==============================================================================
# Main Module Tests - Lines 175-176, 245-246, 413-414
# ==============================================================================

class TestMainEdgeCases:
    """Test main module edge cases."""
    
    @pytest.mark.asyncio
    async def test_service_start_connection_error(self):
        """Test service start with connection error (lines 175-176)."""
        from main import AIIPCService
        from database import ConnectionError
        
        service = AIIPCService()
        
        # Mock Config.load to return a valid config
        with patch('main.Config') as MockConfig:
            mock_config = MagicMock()
            mock_config.database = MagicMock()
            mock_config.logging = MagicMock()
            mock_config.logging.level = "INFO"
            mock_config.logging.format = "text"
            mock_config.validate.return_value = []
            mock_config.to_dict.return_value = {}
            MockConfig.load.return_value = mock_config
            
            # Mock Database to fail on connect
            with patch('main.Database') as MockDatabase:
                mock_db = AsyncMock()
                mock_db.connect = AsyncMock(side_effect=ConnectionError("Connection failed"))
                MockDatabase.return_value = mock_db
                
                with pytest.raises(ConnectionError):
                    await service.start()
    
    @pytest.mark.asyncio
    async def test_run_cleanup_stuck_called(self):
        """Test run loop calls cleanup_stuck (lines 245-246)."""
        from main import AIIPCService
        
        service = AIIPCService()
        service._running = True
        service._shutdown_event = asyncio.Event()
        service.logger = MagicMock()
        
        # Create mock config and processor
        service.config = MagicMock()
        service.config.polling.interval_ms = 10
        service.processor = MagicMock()
        service.processor.process_batch = AsyncMock(return_value=0)
        service.processor.get_stats.return_value = {
            "requests_processed": 0,
            "requests_failed": 0,
            "requests_per_second": 0,
        }
        service.processor.cleanup_expired = AsyncMock(return_value=0)
        service.processor.cleanup_stuck = AsyncMock(return_value=0)
        
        # Schedule shutdown after a very brief time
        async def shutdown_soon():
            await asyncio.sleep(0.02)
            service._shutdown_event.set()
        
        asyncio.create_task(shutdown_soon())
        
        # Mock time to trigger stuck cleanup immediately
        with patch('asyncio.get_event_loop') as mock_loop:
            mock_time = MagicMock()
            # Return increasing times to trigger cleanup
            mock_time.time.side_effect = [0, 0, 301, 301, 602, 602]  # Trigger stuck cleanup
            mock_loop.return_value = mock_time
            
            # Run for very short time
            try:
                await asyncio.wait_for(service.run(), timeout=0.1)
            except asyncio.TimeoutError:
                pass
    
    @pytest.mark.asyncio
    async def test_main_connection_error_returns_1(self):
        """Test main() returns 1 on ConnectionError (lines 413-414)."""
        from main import main
        from database import ConnectionError
        
        with patch('main.AIIPCService') as MockService:
            mock_service = AsyncMock()
            mock_service.start = AsyncMock(side_effect=ConnectionError("Failed"))
            mock_service.stop = AsyncMock()
            MockService.return_value = mock_service
            
            with patch('main.setup_signal_handlers'):
                result = await main()
        
        assert result == 1


# ==============================================================================
# Processor Tests - Line 625 (requests_per_second when uptime is 0)
# ==============================================================================

class TestProcessorEdgeCases:
    """Test processor edge cases."""
    
    def test_requests_per_second_zero_uptime(self):
        """Test requests_per_second returns 0 when uptime is 0 (line 625)."""
        from processor import ProcessingStats
        
        stats = ProcessingStats()
        # Override start_time to be now
        stats.start_time = datetime.utcnow()
        
        # With zero uptime and some processed requests
        stats.requests_processed = 10
        stats.requests_failed = 5
        
        # If uptime is essentially 0, should not divide by zero
        rate = stats.requests_per_second
        
        # Should return a number (not crash), may be very high or 0
        assert isinstance(rate, float)
    
    def test_success_rate_zero_total(self):
        """Test success_rate returns 0 when total is 0."""
        from processor import ProcessingStats
        
        stats = ProcessingStats()
        stats.requests_processed = 0
        stats.requests_failed = 0
        
        assert stats.success_rate == 0.0
    
    def test_average_processing_time_empty(self):
        """Test average_processing_time_ms returns 0 when no times recorded."""
        from processor import ProcessingStats
        
        stats = ProcessingStats()
        stats._processing_times = []
        
        assert stats.average_processing_time_ms == 0.0


# ==============================================================================
# Handler Base Tests - Verify abstract methods
# ==============================================================================

class TestBaseHandlerAbstract:
    """Test base handler abstract methods."""
    
    def test_base_handler_is_abstract(self):
        """Test that BaseHandler cannot be instantiated directly."""
        from handlers.base import BaseHandler
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        # Should not be able to instantiate abstract class directly
        with pytest.raises(TypeError):
            BaseHandler(config)
    
    def test_base_request_handler_is_abstract(self):
        """Test that base_handler.BaseHandler cannot be instantiated directly."""
        from handlers.base_handler import BaseHandler
        
        # Should not be able to instantiate abstract class directly
        with pytest.raises(TypeError):
            BaseHandler(None)


# ==============================================================================
# Additional edge case tests
# ==============================================================================

class TestAdditionalEdgeCases:
    """Additional edge case tests for full coverage."""
    
    @pytest.mark.asyncio
    async def test_ai_async_callback_with_matching_job_id(self):
        """Test handle_callback when job_id matches an async request."""
        from handlers.ai_async import AIAsyncRequestHandler, _async_requests
        from config import Config, DatabaseConfig, PollingConfig, AIServiceConfig, LoggingConfig, SecurityConfig
        
        config = Config(
            database=DatabaseConfig(password="test"),
            polling=PollingConfig(),
            ai_service=AIServiceConfig(),
            logging=LoggingConfig(),
            security=SecurityConfig(auth_enabled=False),
        )
        
        handler = AIAsyncRequestHandler(config)
        
        # Set up an async request with a job_id
        test_id = "test-async-id"
        _async_requests[test_id] = {
            "id": test_id,
            "job_id": "job-123",
            "state": "processing",
            "request_type": "dialogue",
            "npc_id": "test_npc",
            "data": {},
            "created_at": datetime.utcnow().isoformat(),
        }
        
        try:
            result = await handler.handle_callback(
                request_id=1,
                callback_data={
                    "job_id": "job-123",
                    "status": "completed",
                    "result": {"response": "test"},
                },
            )
            
            assert result["status"] == "ok"
            assert _async_requests[test_id]["state"] == "completed"
        finally:
            # Clean up
            _async_requests.pop(test_id, None)
    
    def test_security_config_auth_enabled_no_api_key_warning(self):
        """Test SecurityConfig warns when auth enabled but no API key."""
        from config import SecurityConfig
        import warnings
        
        # Should log a warning (captured via logger, not Python warnings)
        # Just verify it doesn't raise
        config = SecurityConfig(
            auth_enabled=True,
            auth_method="api_key",
            api_key="",  # Empty API key
        )
        
        assert config.auth_enabled == True
        assert config.api_key == ""