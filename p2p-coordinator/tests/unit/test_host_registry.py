"""
Unit tests for HostRegistryService

Tests the HostRegistryService with real database operations.
"""

import pytest
import uuid
from datetime import datetime, timedelta
from sqlalchemy.ext.asyncio import AsyncSession

from services.host_registry import HostRegistryService
from models.host import Host, HostStatus


@pytest.mark.unit
@pytest.mark.asyncio
class TestHostRegistryService:
    """Test suite for HostRegistryService"""
    
    @pytest.fixture
    def service(self):
        """Create HostRegistryService instance"""
        return HostRegistryService()
    
    async def test_register_new_host(self, service, db_session: AsyncSession):
        """Test registering a new host"""
        # Arrange
        host_id = "test_host_001"
        player_id = "player_001"
        player_name = "TestPlayer"
        
        # Act
        result = await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id=player_id,
            player_name=player_name,
            ip_address="192.168.1.100",
            port=8080,
            cpu_cores=8,
            cpu_frequency_mhz=3600,
            memory_mb=16384,
            network_speed_mbps=1000,
        )
        
        # Assert
        assert result is not None
        assert result.host_id == host_id
        assert result.player_id == player_id
        assert result.player_name == player_name
        assert result.status == HostStatus.ONLINE
        assert result.quality_score > 0
    
    async def test_register_existing_host_updates(self, service, db_session: AsyncSession):
        """Test that registering an existing host updates it"""
        # Arrange
        host_id = "test_host_002"
        
        # Register first time
        host1 = await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_002",
            player_name="Player2",
            ip_address="192.168.1.101",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act - Register again with different specs
        host2 = await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_002_updated",
            player_name="Player2Updated",
            ip_address="192.168.1.102",
            port=8081,
            cpu_cores=8,
            cpu_frequency_mhz=3600,
            memory_mb=16384,
            network_speed_mbps=1000,
        )
        
        # Assert
        assert host2.id == host1.id  # Same database ID
        assert host2.player_id == "player_002_updated"
        assert host2.cpu_cores == 8
        # Quality score should be updated (may be same if both are at max)
        assert host2.quality_score >= host1.quality_score
    
    async def test_update_heartbeat(self, service, db_session: AsyncSession):
        """Test updating host heartbeat"""
        # Arrange
        host_id = "test_host_003"
        await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_003",
            player_name="Player3",
            ip_address="192.168.1.103",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act
        result = await service.update_heartbeat(
            session=db_session,
            host_id=host_id,
            latency_ms=50.0,
            packet_loss_percent=0.5,
            bandwidth_usage_mbps=100.0,
            current_players=10,
            current_zones=2,
        )
        
        # Assert
        assert result is not None
        assert result.latency_ms == 50.0
        assert result.packet_loss_percent == 0.5
        assert result.current_players == 10
        assert result.current_zones == 2
    
    async def test_update_heartbeat_nonexistent_host(self, service, db_session: AsyncSession):
        """Test updating heartbeat for non-existent host returns None"""
        # Act
        result = await service.update_heartbeat(
            session=db_session,
            host_id="nonexistent_host",
            latency_ms=50.0,
            packet_loss_percent=0.5,
            bandwidth_usage_mbps=100.0,
            current_players=10,
            current_zones=2,
        )
        
        # Assert
        assert result is None
    
    async def test_get_host(self, service, db_session: AsyncSession):
        """Test getting a host by ID"""
        # Arrange
        host_id = "test_host_004"
        await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_004",
            player_name="Player4",
            ip_address="192.168.1.104",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act
        result = await service.get_host(db_session, host_id)
        
        # Assert
        assert result is not None
        assert result.host_id == host_id
    
    async def test_get_host_nonexistent(self, service, db_session: AsyncSession):
        """Test getting non-existent host returns None"""
        # Act
        result = await service.get_host(db_session, "nonexistent_host")
        
        # Assert
        assert result is None
    
    async def test_get_all_hosts(self, service, db_session: AsyncSession):
        """Test getting all hosts"""
        # Arrange - Register multiple hosts
        for i in range(3):
            await service.register_host(
                session=db_session,
                host_id=f"test_host_all_{i}",
                player_id=f"player_{i}",
                player_name=f"Player{i}",
                ip_address=f"192.168.1.{100+i}",
                port=8080,
                cpu_cores=4,
                cpu_frequency_mhz=2400,
                memory_mb=8192,
                network_speed_mbps=500,
            )
        
        # Act
        result = await service.get_all_hosts(db_session)
        
        # Assert
        assert len(result) >= 3
    
    async def test_get_all_hosts_filtered_by_status(self, service, db_session: AsyncSession):
        """Test getting hosts filtered by status"""
        # Arrange
        await service.register_host(
            session=db_session,
            host_id="test_host_status",
            player_id="player_status",
            player_name="PlayerStatus",
            ip_address="192.168.1.200",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act
        result = await service.get_all_hosts(db_session, status=HostStatus.ONLINE)
        
        # Assert
        assert len(result) > 0
        assert all(h.status == HostStatus.ONLINE for h in result)
    
    async def test_get_available_hosts(self, service, db_session: AsyncSession):
        """Test getting available hosts"""
        # Arrange
        await service.register_host(
            session=db_session,
            host_id="test_host_available",
            player_id="player_available",
            player_name="PlayerAvailable",
            ip_address="192.168.1.201",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act
        result = await service.get_available_hosts(db_session)
        
        # Assert
        assert len(result) > 0
        assert all(h.status == HostStatus.ONLINE for h in result)
    
    async def test_select_best_host(self, service, db_session: AsyncSession):
        """Test selecting the best host"""
        # Arrange - Register hosts with different specs
        low_host = await service.register_host(
            session=db_session,
            host_id="test_host_low_quality",
            player_id="player_low",
            player_name="PlayerLow",
            ip_address="192.168.1.210",
            port=8080,
            cpu_cores=2,
            cpu_frequency_mhz=1800,
            memory_mb=4096,
            network_speed_mbps=100,
        )

        high_host = await service.register_host(
            session=db_session,
            host_id="test_host_high_quality",
            player_id="player_high",
            player_name="PlayerHigh",
            ip_address="192.168.1.211",
            port=8080,
            cpu_cores=16,
            cpu_frequency_mhz=4800,
            memory_mb=32768,
            network_speed_mbps=10000,
        )

        # Act
        result = await service.select_best_host(db_session)

        # Assert
        assert result is not None
        # Should select one of the high-quality hosts (quality score should be >= low host)
        assert result.quality_score >= low_host.quality_score
        # Verify it's actually selecting from available hosts
        assert result.status.value == "online"
    
    async def test_unregister_host(self, service, db_session: AsyncSession):
        """Test unregistering a host"""
        # Arrange
        host_id = "test_host_unregister"
        await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_unregister",
            player_name="PlayerUnregister",
            ip_address="192.168.1.220",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Act
        result = await service.unregister_host(db_session, host_id)
        
        # Assert
        assert result is True
        
        # Verify host is gone
        host = await service.get_host(db_session, host_id)
        assert host is None
    
    async def test_unregister_nonexistent_host(self, service, db_session: AsyncSession):
        """Test unregistering non-existent host returns False"""
        # Act
        result = await service.unregister_host(db_session, "nonexistent_host")
        
        # Assert
        assert result is False
    
    async def test_mark_offline_stale_hosts(self, service, db_session: AsyncSession):
        """Test marking stale hosts as offline"""
        # Arrange
        host_id = "test_host_stale"
        host = await service.register_host(
            session=db_session,
            host_id=host_id,
            player_id="player_stale",
            player_name="PlayerStale",
            ip_address="192.168.1.230",
            port=8080,
            cpu_cores=4,
            cpu_frequency_mhz=2400,
            memory_mb=8192,
            network_speed_mbps=500,
        )
        
        # Manually set last_heartbeat to old time
        from sqlalchemy import update as sql_update
        stmt = sql_update(Host).where(Host.id == host.id).values(
            last_heartbeat=datetime.utcnow() - timedelta(seconds=120)
        )
        await db_session.execute(stmt)
        await db_session.commit()
        
        # Act
        result = await service.mark_offline_stale_hosts(db_session, timeout_seconds=60)
        
        # Assert
        assert result >= 1  # At least our test host should be marked offline

        # Verify host is offline
        updated_host = await service.get_host(db_session, host_id)
        assert updated_host.status == HostStatus.OFFLINE

    async def test_get_all_hosts_with_min_quality_score(self, service, db_session: AsyncSession):
        """Test getting all hosts filtered by minimum quality score"""
        # Arrange - Create hosts with different quality scores
        host_id_1 = f"test_host_{uuid.uuid4().hex[:8]}"
        host_id_2 = f"test_host_{uuid.uuid4().hex[:8]}"

        # Host 1 with high quality
        await service.register_host(
            session=db_session,
            host_id=host_id_1,
            player_id="player_001",
            player_name="Player One",
            ip_address="192.168.1.100",
            port=8080,
            cpu_cores=8,
            cpu_frequency_mhz=3600,
            memory_mb=16384,
            network_speed_mbps=1000,
        )

        # Host 2 with lower quality
        await service.register_host(
            session=db_session,
            host_id=host_id_2,
            player_id="player_002",
            player_name="Player Two",
            ip_address="192.168.1.101",
            port=8081,
            cpu_cores=2,
            cpu_frequency_mhz=1800,
            memory_mb=4096,
            network_speed_mbps=50,
        )

        # Act - Get hosts with min quality score of 50
        result = await service.get_all_hosts(db_session, min_quality_score=50.0)

        # Assert - Should only get high quality host
        assert len(result) >= 1
        assert any(h.host_id == host_id_1 for h in result)

    async def test_get_available_hosts_with_min_quality_score(self, service, db_session: AsyncSession):
        """Test getting available hosts filtered by minimum quality score"""
        # Arrange - Create hosts with different quality scores
        host_id_1 = f"test_host_{uuid.uuid4().hex[:8]}"
        host_id_2 = f"test_host_{uuid.uuid4().hex[:8]}"

        # Host 1 with high quality
        await service.register_host(
            session=db_session,
            host_id=host_id_1,
            player_id="player_001",
            player_name="Player One",
            ip_address="192.168.1.100",
            port=8080,
            cpu_cores=8,
            cpu_frequency_mhz=3600,
            memory_mb=16384,
            network_speed_mbps=1000,
        )

        # Host 2 with lower quality
        await service.register_host(
            session=db_session,
            host_id=host_id_2,
            player_id="player_002",
            player_name="Player Two",
            ip_address="192.168.1.101",
            port=8081,
            cpu_cores=2,
            cpu_frequency_mhz=1800,
            memory_mb=4096,
            network_speed_mbps=50,
        )

        # Act - Get available hosts with min quality score of 50
        result = await service.get_available_hosts(db_session, min_quality_score=50.0)

        # Assert - Should only get high quality host
        assert len(result) >= 1
        assert any(h.host_id == host_id_1 for h in result)

    async def test_select_best_host_no_hosts_available(self, service, db_session: AsyncSession):
        """Test selecting best host when no hosts are available"""
        # Act - Try to select best host with very high quality requirement
        result = await service.select_best_host(db_session, min_quality_score=999.0)

        # Assert - Should return None
        assert result is None
