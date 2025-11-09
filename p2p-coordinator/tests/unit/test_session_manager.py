"""
Unit tests for SessionManagerService

Tests the SessionManagerService with real database operations.
"""

import pytest
import uuid
from sqlalchemy.ext.asyncio import AsyncSession

from services.session_manager import SessionManagerService
from services.host_registry import HostRegistryService
from models.session import P2PSession, SessionStatus
from models.zone import Zone


@pytest.mark.unit
@pytest.mark.asyncio
class TestSessionManagerService:
    """Test suite for SessionManagerService"""
    
    @pytest.fixture
    def service(self):
        """Create SessionManagerService instance"""
        return SessionManagerService()
    
    @pytest.fixture
    def host_service(self):
        """Create HostRegistryService instance"""
        return HostRegistryService()
    
    async def test_create_session_success(self, service, host_service, db_session: AsyncSession):
        """Test creating a new session"""
        # Arrange - Create a host first
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        host = await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_001",
            player_name="TestPlayer",
            ip_address="192.168.1.100",
            port=8080,
            cpu_cores=8,
            cpu_frequency_mhz=3600,
            memory_mb=16384,
            network_speed_mbps=1000,
        )
        
        # Create a zone
        zone = Zone(
            zone_id=zone_id,
            zone_name="Prontera",
            zone_display_name="Prontera City",
            map_name="prontera",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()
        await db_session.refresh(zone)
        
        # Act
        result = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001", "player_002", "player_003"],
        )
        
        # Assert
        assert result is not None
        assert result.session_id is not None
        assert result.host_id == host.id
        assert result.zone_id == zone.id
        assert result.current_players == 3
        assert result.status == SessionStatus.PENDING
    
    async def test_create_session_host_not_found(self, service, db_session: AsyncSession):
        """Test creating session with non-existent host"""
        # Act
        result = await service.create_session(
            db=db_session,
            host_id="nonexistent_host",
            zone_id="prontera",
            player_ids=["player_001"],
        )
        
        # Assert
        assert result is None
    
    async def test_activate_session(self, service, host_service, db_session: AsyncSession):
        """Test activating a pending session"""
        # Arrange - Create host, zone, and session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_activate",
            player_name="PlayerActivate",
            ip_address="192.168.1.101",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        zone = Zone(
            zone_id=zone_id,
            zone_name="Geffen",
            zone_display_name="Geffen City",
            map_name="geffen",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()
        
        session_obj = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        
        # Act
        result = await service.activate_session(db_session, session_obj.id)
        
        # Assert
        assert result is True
        
        # Verify session is active
        updated_session = await service.get_session(db_session, session_obj.id)
        assert updated_session.status == SessionStatus.ACTIVE
        assert updated_session.started_at is not None
    
    async def test_end_session(self, service, host_service, db_session: AsyncSession):
        """Test ending an active session"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_end",
            player_name="PlayerEnd",
            ip_address="192.168.1.102",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        zone = Zone(
            zone_id=zone_id,
            zone_name="Payon",
            zone_display_name="Payon Village",
            map_name="payon",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()
        
        session_obj = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        
        await service.activate_session(db_session, session_obj.id)
        
        # Act
        result = await service.end_session(db_session, session_obj.id)
        
        # Assert
        assert result is True
        
        # Verify session is ended
        updated_session = await service.get_session(db_session, session_obj.id)
        assert updated_session.status == SessionStatus.ENDED
        assert updated_session.ended_at is not None
    
    async def test_get_session(self, service, host_service, db_session: AsyncSession):
        """Test getting a session by ID"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_get",
            player_name="PlayerGet",
            ip_address="192.168.1.103",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        zone = Zone(
            zone_id=zone_id,
            zone_name="Alberta",
            zone_display_name="Alberta Port",
            map_name="alberta",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()
        
        session_obj = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        
        # Act
        result = await service.get_session(db_session, session_obj.id)
        
        # Assert
        assert result is not None
        assert result.id == session_obj.id
    
    async def test_get_session_nonexistent(self, service, db_session: AsyncSession):
        """Test getting non-existent session returns None"""
        # Act
        result = await service.get_session(db_session, 999999)
        
        # Assert
        assert result is None
    
    async def test_get_active_sessions_for_host(self, service, host_service, db_session: AsyncSession):
        """Test getting active sessions for a specific host"""
        # Arrange - Create host and zones
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone1_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone2_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        host = await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_multi",
            player_name="PlayerMulti",
            ip_address="192.168.1.104",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        zone1 = Zone(
            zone_id=zone1_id,
            zone_name="Zone1",
            zone_display_name="Zone 1",
            map_name="zone1",
            p2p_enabled=True,
            max_players=100,
        )
        zone2 = Zone(
            zone_id=zone2_id,
            zone_name="Zone2",
            zone_display_name="Zone 2",
            map_name="zone2",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add_all([zone1, zone2])
        await db_session.commit()
        
        # Create multiple sessions
        session1 = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone1_id,
            player_ids=["player_001"],
        )
        session2 = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone2_id,
            player_ids=["player_002"],
        )
        
        # Activate both sessions
        await service.activate_session(db_session, session1.id)
        await service.activate_session(db_session, session2.id)

        # Act - Use host_id (string) as the method now expects
        result = await service.get_active_sessions_for_host(db_session, host_id)

        # Assert
        assert len(result) == 2
        assert all(s.status == SessionStatus.ACTIVE for s in result)
    
    async def test_get_active_sessions_for_zone(self, service, host_service, db_session: AsyncSession):
        """Test getting active sessions for a specific zone"""
        # Arrange - Create hosts and zone
        host1_id = f"test_host_{uuid.uuid4().hex[:8]}"
        host2_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host1_id,
            player_id="player_zone1",
            player_name="PlayerZone1",
            ip_address="192.168.1.105",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        await host_service.register_host(
            session=db_session,
            host_id=host2_id,
            player_id="player_zone2",
            player_name="PlayerZone2",
            ip_address="192.168.1.106",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="SharedZone",
            zone_display_name="Shared Zone",
            map_name="shared",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()
        await db_session.refresh(zone)  # Get the database ID

        # Create sessions on different hosts but same zone
        session1 = await service.create_session(
            db=db_session,
            host_id=host1_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        session2 = await service.create_session(
            db=db_session,
            host_id=host2_id,
            zone_id=zone_id,
            player_ids=["player_002"],
        )

        # Activate both sessions
        await service.activate_session(db_session, session1.id)
        await service.activate_session(db_session, session2.id)

        # Act - Use zone_id (string) as the method now expects
        result = await service.get_active_sessions_for_zone(db_session, zone_id)

        # Assert
        assert len(result) == 2
        assert all(s.status == SessionStatus.ACTIVE for s in result)

    async def test_create_session_host_offline(self, service, host_service, db_session: AsyncSession):
        """Test creating session with offline host returns None"""
        # Arrange - Create offline host
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        host = await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_offline",
            player_name="PlayerOffline",
            ip_address="192.168.1.200",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        # Mark host as offline
        await host_service.unregister_host(db_session, host_id)

        # Create zone
        zone = Zone(
            zone_id=zone_id,
            zone_name="TestZone",
            zone_display_name="Test Zone",
            map_name="test",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        # Act
        result = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Assert
        assert result is None

    async def test_create_session_zone_not_found(self, service, host_service, db_session: AsyncSession):
        """Test creating session with non-existent zone returns None"""
        # Arrange - Create host
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_nozone",
            player_name="PlayerNoZone",
            ip_address="192.168.1.201",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        # Act
        result = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id="nonexistent_zone",
            player_ids=["player_001"],
        )

        # Assert
        assert result is None

    async def test_create_session_p2p_disabled(self, service, host_service, db_session: AsyncSession):
        """Test creating session with P2P disabled zone returns None"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_nop2p",
            player_name="PlayerNoP2P",
            ip_address="192.168.1.202",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        # Create zone with P2P disabled
        zone = Zone(
            zone_id=zone_id,
            zone_name="NoP2PZone",
            zone_display_name="No P2P Zone",
            map_name="nop2p",
            p2p_enabled=False,  # P2P disabled
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        # Act
        result = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Assert
        assert result is None

    async def test_update_session_metrics_success(self, service, host_service, db_session: AsyncSession):
        """Test updating session metrics"""
        # Arrange - Create session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_metrics",
            player_name="PlayerMetrics",
            ip_address="192.168.1.203",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="MetricsZone",
            zone_display_name="Metrics Zone",
            map_name="metrics",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Act
        result = await service.update_session_metrics(
            db=db_session,
            session_id=session.id,
            avg_latency_ms=25.5,
            packet_loss_percent=0.5,
            bandwidth_usage_mbps=150.0,
        )

        # Assert
        assert result is True

        # Verify metrics were updated
        updated_session = await service.get_session(db_session, session.id)
        assert updated_session.average_latency_ms == 25.5
        assert updated_session.average_packet_loss_percent == 0.5
        assert updated_session.bandwidth_usage_mbps == 150.0

    async def test_update_session_metrics_not_found(self, service, db_session: AsyncSession):
        """Test updating metrics for non-existent session"""
        # Act
        result = await service.update_session_metrics(
            db=db_session,
            session_id=999999,
            avg_latency_ms=25.5,
        )

        # Assert
        assert result is False

    async def test_add_player_to_session_success(self, service, host_service, db_session: AsyncSession):
        """Test adding a player to a session"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_addplayer",
            player_name="PlayerAddPlayer",
            ip_address="192.168.1.204",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="AddPlayerZone",
            zone_display_name="Add Player Zone",
            map_name="addplayer",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Act
        result = await service.add_player_to_session(
            db=db_session,
            session_id=session.id,
            player_id="player_002",
        )

        # Assert
        assert result is True

        # Verify player was added
        updated_session = await service.get_session(db_session, session.id)
        assert "player_002" in updated_session.connected_players
        assert updated_session.current_players == 2

    async def test_remove_player_from_session_success(self, service, host_service, db_session: AsyncSession):
        """Test removing a player from a session"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_removeplayer",
            player_name="PlayerRemovePlayer",
            ip_address="192.168.1.205",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="RemovePlayerZone",
            zone_display_name="Remove Player Zone",
            map_name="removeplayer",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001", "player_002", "player_003"],
        )

        # Act
        result = await service.remove_player_from_session(
            db=db_session,
            session_id=session.id,
            player_id="player_002",
        )

        # Assert
        assert result is True

        # Verify player was removed
        updated_session = await service.get_session(db_session, session.id)
        assert "player_002" not in updated_session.connected_players
        assert updated_session.current_players == 2

    async def test_create_session_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test create_session handles database errors gracefully"""
        # Arrange
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_error",
            player_name="PlayerError",
            ip_address="192.168.1.110",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="ErrorZone",
            zone_display_name="Error Zone",
            map_name="error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Assert
        assert result is None

    async def test_activate_session_not_found(self, service, db_session: AsyncSession):
        """Test activating a non-existent session returns False"""
        # Act
        result = await service.activate_session(db_session, 999999)

        # Assert
        assert result is False

    async def test_activate_session_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test activate_session handles database errors gracefully"""
        # Arrange - Create a session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_activate_error",
            player_name="PlayerActivateError",
            ip_address="192.168.1.111",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="ActivateErrorZone",
            zone_display_name="Activate Error Zone",
            map_name="activate_error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.activate_session(db_session, session.id)

        # Assert
        assert result is False

    async def test_end_session_not_found(self, service, db_session: AsyncSession):
        """Test ending a non-existent session returns False"""
        # Act
        result = await service.end_session(db_session, 999999)

        # Assert
        assert result is False

    async def test_end_session_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test end_session handles database errors gracefully"""
        # Arrange - Create and activate a session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_end_error",
            player_name="PlayerEndError",
            ip_address="192.168.1.112",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="EndErrorZone",
            zone_display_name="End Error Zone",
            map_name="end_error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        await service.activate_session(db_session, session.id)

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.end_session(db_session, session.id)

        # Assert
        assert result is False

    async def test_update_session_metrics_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test update_session_metrics handles database errors gracefully"""
        # Arrange - Create a session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_metrics_error",
            player_name="PlayerMetricsError",
            ip_address="192.168.1.113",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="MetricsErrorZone",
            zone_display_name="Metrics Error Zone",
            map_name="metrics_error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.update_session_metrics(
            db=db_session,
            session_id=session.id,
            avg_latency_ms=50.0,
            packet_loss_percent=0.1,
            bandwidth_usage_mbps=10.0,
        )

        # Assert
        assert result is False

    async def test_add_player_to_session_not_found(self, service, db_session: AsyncSession):
        """Test adding player to non-existent session returns False"""
        # Act
        result = await service.add_player_to_session(
            db=db_session,
            session_id=999999,
            player_id="player_999",
        )

        # Assert
        assert result is False

    async def test_add_player_to_session_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test add_player_to_session handles database errors gracefully"""
        # Arrange - Create a session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_add_error",
            player_name="PlayerAddError",
            ip_address="192.168.1.114",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="AddErrorZone",
            zone_display_name="Add Error Zone",
            map_name="add_error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.add_player_to_session(
            db=db_session,
            session_id=session.id,
            player_id="player_002",
        )

        # Assert
        assert result is False

    async def test_remove_player_from_session_not_found(self, service, db_session: AsyncSession):
        """Test removing player from non-existent session returns False"""
        # Act
        result = await service.remove_player_from_session(
            db=db_session,
            session_id=999999,
            player_id="player_999",
        )

        # Assert
        assert result is False

    async def test_remove_player_from_session_database_error(self, service, host_service, db_session: AsyncSession, monkeypatch):
        """Test remove_player_from_session handles database errors gracefully"""
        # Arrange - Create a session with multiple players
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_remove_error",
            player_name="PlayerRemoveError",
            ip_address="192.168.1.115",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="RemoveErrorZone",
            zone_display_name="Remove Error Zone",
            map_name="remove_error",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001", "player_002"],
        )

        # Mock db.commit to raise an exception
        async def mock_commit_error():
            raise Exception("Database commit failed")

        monkeypatch.setattr(db_session, "commit", mock_commit_error)

        # Act
        result = await service.remove_player_from_session(
            db=db_session,
            session_id=session.id,
            player_id="player_002",
        )

        # Assert
        assert result is False

    async def test_get_active_sessions_for_host_not_found(self, service, db_session: AsyncSession):
        """Test getting active sessions for non-existent host returns empty list"""
        # Act
        result = await service.get_active_sessions_for_host(db_session, "nonexistent_host")

        # Assert
        assert result == []

    async def test_get_active_sessions_for_zone_not_found(self, service, db_session: AsyncSession):
        """Test getting active sessions for non-existent zone returns empty list"""
        # Act
        result = await service.get_active_sessions_for_zone(db_session, "nonexistent_zone")

        # Assert
        assert result == []

    async def test_cleanup_stale_sessions_success(self, service, host_service, db_session: AsyncSession):
        """Test cleaning up stale sessions"""
        from datetime import datetime, timedelta

        # Arrange - Create old sessions
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_cleanup",
            player_name="PlayerCleanup",
            ip_address="192.168.1.116",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="CleanupZone",
            zone_display_name="Cleanup Zone",
            map_name="cleanup",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        # Create sessions and manually set old created_at
        session1 = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )
        session2 = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_002"],
        )

        # Manually set old created_at (2 hours ago)
        old_time = datetime.utcnow() - timedelta(hours=2)
        session1.created_at = old_time
        session2.created_at = old_time
        await db_session.commit()

        # Act - Cleanup sessions older than 30 minutes
        count = await service.cleanup_stale_sessions(db_session, timeout_minutes=30)

        # Assert
        assert count == 2

        # Verify sessions are marked as FAILED
        await db_session.refresh(session1)
        await db_session.refresh(session2)
        assert session1.status == SessionStatus.FAILED
        assert session2.status == SessionStatus.FAILED
        assert session1.ended_at is not None
        assert session2.ended_at is not None

    async def test_cleanup_stale_sessions_no_stale(self, service, host_service, db_session: AsyncSession):
        """Test cleanup when there are no stale sessions"""
        # Arrange - Create recent session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_no_cleanup",
            player_name="PlayerNoCleanup",
            ip_address="192.168.1.117",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="NoCleanupZone",
            zone_display_name="No Cleanup Zone",
            map_name="no_cleanup",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Act - Cleanup sessions older than 30 minutes (none should be found)
        count = await service.cleanup_stale_sessions(db_session, timeout_minutes=30)

        # Assert
        assert count == 0

    async def test_cleanup_stale_sessions_database_error(self, service, db_session: AsyncSession, monkeypatch):
        """Test cleanup_stale_sessions handles database errors gracefully"""
        # Mock db.execute to raise an exception
        async def mock_execute_error(*args, **kwargs):
            raise Exception("Database query failed")

        monkeypatch.setattr(db_session, "execute", mock_execute_error)

        # Act
        count = await service.cleanup_stale_sessions(db_session, timeout_minutes=30)

        # Assert
        assert count == 0

    async def test_add_player_to_session_at_max_capacity(self, service, host_service, db_session: AsyncSession):
        """Test adding player to session at max capacity returns False"""
        # Arrange - Create session at max capacity
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_max_capacity",
            player_name="PlayerMaxCapacity",
            ip_address="192.168.1.118",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="MaxCapacityZone",
            zone_display_name="Max Capacity Zone",
            map_name="max_capacity",
            p2p_enabled=True,
            max_players=2,  # Set low max
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001", "player_002"],  # Already at max
        )

        # Act - Try to add another player
        result = await service.add_player_to_session(
            db=db_session,
            session_id=session.id,
            player_id="player_003",
        )

        # Assert
        assert result is False

    async def test_remove_player_not_in_session(self, service, host_service, db_session: AsyncSession):
        """Test removing player not in session returns False"""
        # Arrange - Create session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_not_in",
            player_name="PlayerNotIn",
            ip_address="192.168.1.119",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="NotInZone",
            zone_display_name="Not In Zone",
            map_name="not_in",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Act - Try to remove player not in session
        result = await service.remove_player_from_session(
            db=db_session,
            session_id=session.id,
            player_id="player_999",  # Not in session
        )

        # Assert
        assert result is False

    async def test_activate_session_not_pending(self, service, host_service, db_session: AsyncSession):
        """Test activating a session that is not pending returns False"""
        # Arrange - Create and activate a session
        host_id = f"test_host_{uuid.uuid4().hex[:8]}"
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await host_service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_not_pending",
            player_name="PlayerNotPending",
            ip_address="192.168.1.120",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )

        zone = Zone(
            zone_id=zone_id,
            zone_name="NotPendingZone",
            zone_display_name="Not Pending Zone",
            map_name="not_pending",
            p2p_enabled=True,
            max_players=100,
        )
        db_session.add(zone)
        await db_session.commit()

        session = await service.create_session(
            db=db_session,
            host_id=host_id,
            zone_id=zone_id,
            player_ids=["player_001"],
        )

        # Activate the session
        await service.activate_session(db_session, session.id)

        # Act - Try to activate again (should fail because it's already ACTIVE)
        result = await service.activate_session(db_session, session.id)

        # Assert
        assert result is False

    async def test_get_session_database_error(self, service, db_session: AsyncSession, monkeypatch):
        """Test get_session handles database errors gracefully"""
        # Mock db.execute to raise an exception
        async def mock_execute_error(*args, **kwargs):
            raise Exception("Database query failed")

        monkeypatch.setattr(db_session, "execute", mock_execute_error)

        # Act
        result = await service.get_session(db_session, 123)

        # Assert
        assert result is None
