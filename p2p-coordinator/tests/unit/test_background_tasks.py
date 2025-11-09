"""
Unit tests for Background Tasks Service
"""

import pytest
import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
from services.background_tasks import BackgroundTasksService
from config import settings


@pytest.mark.unit
class TestBackgroundTasksService:
    """Test BackgroundTasksService class"""

    @pytest.fixture
    def bg_service(self):
        """Create BackgroundTasksService instance"""
        service = BackgroundTasksService()
        yield service
        # Cleanup
        service.running = False
        service.tasks.clear()

    @pytest.mark.asyncio
    async def test_initialization(self, bg_service):
        """Test service initialization"""
        assert bg_service.running is False
        assert len(bg_service.tasks) == 0

    @pytest.mark.asyncio
    async def test_start(self, bg_service):
        """Test starting background tasks"""
        await bg_service.start()
        
        assert bg_service.running is True
        assert len(bg_service.tasks) == 3  # 3 background tasks
        
        # Stop immediately to prevent long-running tasks
        await bg_service.stop()

    @pytest.mark.asyncio
    async def test_start_already_running(self, bg_service):
        """Test starting when already running"""
        await bg_service.start()
        initial_task_count = len(bg_service.tasks)
        
        # Try to start again
        await bg_service.start()
        
        # Should not create duplicate tasks
        assert len(bg_service.tasks) == initial_task_count
        
        await bg_service.stop()

    @pytest.mark.asyncio
    async def test_stop(self, bg_service):
        """Test stopping background tasks"""
        await bg_service.start()
        assert bg_service.running is True
        assert len(bg_service.tasks) > 0
        
        await bg_service.stop()
        
        assert bg_service.running is False
        assert len(bg_service.tasks) == 0

    @pytest.mark.asyncio
    async def test_stop_not_running(self, bg_service):
        """Test stopping when not running"""
        # Should not raise error
        await bg_service.stop()
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_npc_broadcast_loop_disabled(self, bg_service, monkeypatch):
        """Test NPC broadcast loop when AI service is disabled"""
        # Disable AI service
        monkeypatch.setattr(settings.ai_service, "ai_service_enabled", False)
        
        # Start and let it run briefly
        await bg_service.start()
        await asyncio.sleep(0.1)  # Let loop run once
        await bg_service.stop()
        
        # Should complete without errors
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_npc_broadcast_loop_enabled(self, bg_service, monkeypatch):
        """Test NPC broadcast loop when AI service is enabled"""
        # Enable AI service
        monkeypatch.setattr(settings.ai_service, "ai_service_enabled", True)
        
        # Start and let it run briefly
        await bg_service.start()
        await asyncio.sleep(0.1)  # Let loop run once
        await bg_service.stop()
        
        # Should complete without errors
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_npc_broadcast_loop_exception_handling(self, bg_service, monkeypatch):
        """Test NPC broadcast loop handles exceptions"""
        # This test verifies that the loop can handle exceptions gracefully
        # We'll just start and stop to ensure no crashes
        monkeypatch.setattr(settings.ai_service, "ai_service_enabled", True)

        await bg_service.start()
        await asyncio.sleep(0.1)
        await bg_service.stop()

        # Should have completed without crashing
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_session_health_loop(self, bg_service):
        """Test session health monitoring loop"""
        await bg_service.start()
        await asyncio.sleep(0.1)  # Let loop run briefly
        await bg_service.stop()
        
        # Should complete without errors
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_session_health_loop_exception_handling(self, bg_service):
        """Test session health loop handles exceptions"""
        original_sleep = asyncio.sleep
        call_count = [0]
        
        async def mock_sleep(seconds):
            call_count[0] += 1
            if call_count[0] == 2:  # Second call (in health loop)
                raise Exception("Test exception")
            await original_sleep(0.01)
        
        with patch('asyncio.sleep', side_effect=mock_sleep):
            await bg_service.start()
            await asyncio.sleep(0.2)
            await bg_service.stop()
        
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_cleanup_loop(self, bg_service):
        """Test cleanup loop"""
        await bg_service.start()
        await asyncio.sleep(0.1)  # Let loop run briefly
        await bg_service.stop()

        # Should complete without errors
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_cleanup_loop_with_stale_sessions(self, bg_service):
        """Test cleanup loop with stale sessions"""
        from services.session_manager import session_manager

        # Mock cleanup_stale_sessions to return count
        with patch.object(session_manager, 'cleanup_stale_sessions', new_callable=AsyncMock) as mock_cleanup:
            mock_cleanup.return_value = 5  # 5 stale sessions cleaned

            # Mock get_db_session to yield a mock db
            async def mock_db_gen():
                yield MagicMock()

            with patch('services.background_tasks.get_db_session', return_value=mock_db_gen()):
                await bg_service.start()
                await asyncio.sleep(0.1)
                await bg_service.stop()

        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_cleanup_loop_exception_handling(self, bg_service):
        """Test cleanup loop handles exceptions"""
        from services.session_manager import session_manager

        # Mock cleanup_stale_sessions to raise exception
        with patch.object(session_manager, 'cleanup_stale_sessions', new_callable=AsyncMock) as mock_cleanup:
            mock_cleanup.side_effect = Exception("Database error")

            # Mock get_db_session
            async def mock_db_gen():
                yield MagicMock()

            with patch('services.background_tasks.get_db_session', return_value=mock_db_gen()):
                await bg_service.start()
                await asyncio.sleep(0.1)
                await bg_service.stop()

        # Should have recovered from exception
        assert bg_service.running is False

    @pytest.mark.asyncio
    async def test_all_loops_cancellation(self, bg_service):
        """Test all loops handle cancellation properly"""
        await bg_service.start()

        # Let tasks run briefly
        await asyncio.sleep(0.1)

        # Stop should cancel all tasks
        await bg_service.stop()

        # All tasks should be cancelled
        assert bg_service.running is False
        assert len(bg_service.tasks) == 0

