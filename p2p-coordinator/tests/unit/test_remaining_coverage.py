"""
Unit tests to cover remaining uncovered code

This file contains tests specifically designed to cover all remaining
uncovered lines to achieve 100% coverage.
"""

import pytest
from unittest.mock import AsyncMock, MagicMock, patch
from datetime import datetime


@pytest.mark.unit
class TestModelsCoverage:
    """Test uncovered model methods"""

    def test_host_model_properties(self):
        """Test Host model property methods"""
        from models.host import Host, HostStatus

        host = Host(
            host_id="test_host",
            player_id="player1",
            player_name="Test Player",
            ip_address="192.168.1.1",
            port=8080,
            status=HostStatus.ONLINE,
            quality_score=85.0,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=100,
            current_players=5,
            max_players=10,
            current_zones=2,
            max_zones=5
        )

        # Test __repr__
        repr_str = repr(host)
        assert "test_host" in repr_str

        # Test __str__
        str_str = str(host)
        assert "test_host" in str_str

        # Test is_available property
        assert host.is_available == True

        # Test capacity_percent property
        assert host.capacity_percent == 50.0

        # Test capacity_percent edge case (max_players = 0)
        host.max_players = 0
        assert host.capacity_percent == 100.0

    def test_zone_model_properties(self):
        """Test Zone model property methods"""
        from models.zone import Zone, ZoneStatus

        zone = Zone(
            zone_id="test_zone",
            zone_name="Test Zone",
            zone_display_name="Test Zone Display",
            map_name="test_map",
            max_players=100,
            recommended_players=50,
            p2p_enabled=True,
            p2p_priority=1,
            fallback_enabled=True,
            min_host_quality_score=7.0,
            min_bandwidth_mbps=100,
            max_latency_ms=100,
            status=ZoneStatus.ENABLED
        )

        # Test __repr__
        repr_str = repr(zone)
        assert "test_zone" in repr_str

        # Test __str__
        str_str = str(zone)
        assert "test_zone" in str_str

        # Test is_available_for_p2p property
        assert zone.is_available_for_p2p == True

        # Test capacity_percent property
        capacity = zone.capacity_percent
        assert capacity >= 0.0

        # Test capacity_percent edge case (max_players = 0)
        zone.max_players = 0
        assert zone.capacity_percent == 100.0

    def test_session_model_properties(self):
        """Test P2PSession model property methods"""
        from models.session import P2PSession, SessionStatus
        from datetime import datetime, timedelta

        session = P2PSession(
            session_id="test_session",
            host_id=1,
            zone_id=1,
            status=SessionStatus.ACTIVE,
            connected_players=["player1", "player2"],
            current_players=2,
            max_players=10
        )

        # Test __repr__
        repr_str = repr(session)
        assert "test_session" in repr_str

        # Test is_active property
        assert session.is_active == True

        # Test capacity_percent property
        assert session.capacity_percent == 20.0

        # Test capacity_percent edge case (max_players = 0)
        session.max_players = 0
        assert session.capacity_percent == 100.0
        session.max_players = 10  # Reset

        # Test duration_seconds property with no started_at
        assert session.duration_seconds is None

        # Test duration_seconds with started_at
        session.started_at = datetime.utcnow() - timedelta(seconds=100)
        duration = session.duration_seconds
        assert duration is not None
        assert duration >= 99  # Allow for small timing differences

        # Test add_player method
        result = session.add_player("player3")
        assert result == True
        assert "player3" in session.connected_players
        assert session.current_players == 3

        # Test add_player when full
        session.current_players = 10
        result = session.add_player("player11")
        assert result == False

        # Test remove_player method
        session.current_players = 3
        result = session.remove_player("player3")
        assert result == True
        assert "player3" not in session.connected_players
        assert session.current_players == 2

        # Test remove_player when player not found
        result = session.remove_player("nonexistent")
        assert result == False

        # Test add_player when connected_players is None
        session2 = P2PSession(
            session_id="test_session2",
            host_id=1,
            zone_id=1,
            status=SessionStatus.ACTIVE,
            connected_players=None,  # None instead of empty list
            current_players=0,
            max_players=10
        )
        result = session2.add_player("player1")
        assert result == True
        assert session2.connected_players == ["player1"]


@pytest.mark.unit
class TestSessionManagerCoverage:
    """Test uncovered session manager code"""

    @pytest.mark.asyncio
    async def test_create_session_host_offline(self):
        """Test create session with offline host"""
        from services.session_manager import SessionManagerService
        from models.host import Host, HostStatus
        from models.zone import Zone, ZoneStatus
        from sqlalchemy.ext.asyncio import AsyncSession
        from unittest.mock import AsyncMock, MagicMock
        
        service = SessionManagerService()
        db = AsyncMock(spec=AsyncSession)
        
        # Mock host query to return offline host
        host = Host(
            id=1,
            host_id="test_host",
            player_id="player1",
            player_name="Test Player",
            ip_address="192.168.1.1",
            port=8080,
            status=HostStatus.OFFLINE,
            quality_score=85.0,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=100
        )
        
        host_result = MagicMock()
        host_result.scalar_one_or_none.return_value = host
        
        zone = Zone(
            id=1,
            zone_id="test_zone",
            zone_name="Test Zone",
            zone_display_name="Test Zone Display",
            map_name="test_map",
            max_players=100,
            recommended_players=50,
            p2p_enabled=True,
            p2p_priority=1,
            fallback_enabled=True,
            min_host_quality_score=7.0,
            min_bandwidth_mbps=100,
            max_latency_ms=100,
            status=ZoneStatus.ENABLED
        )
        
        zone_result = MagicMock()
        zone_result.scalar_one_or_none.return_value = zone
        
        db.execute = AsyncMock(side_effect=[host_result, zone_result])
        
        # Create session should return None because host is offline
        result = await service.create_session(db, "test_host", "test_zone", ["player1"])
        assert result is None

