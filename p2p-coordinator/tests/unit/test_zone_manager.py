"""
Unit tests for ZoneManagerService

Tests the ZoneManagerService with real database operations.
"""

import pytest
import uuid
from sqlalchemy.ext.asyncio import AsyncSession

from services.zone_manager import ZoneManagerService
from models.zone import Zone, ZoneStatus


@pytest.mark.unit
@pytest.mark.asyncio
class TestZoneManagerService:
    """Test suite for ZoneManagerService"""
    
    @pytest.fixture
    def service(self):
        """Create ZoneManagerService instance"""
        return ZoneManagerService()
    
    async def test_create_zone_new(self, service, db_session: AsyncSession):
        """Test creating a new zone"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        # Act
        result = await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="TestZone",
            zone_display_name="Test Zone",
            map_name="test_map",
            max_players=100,
            recommended_players=50,
            p2p_enabled=True,
            p2p_priority=5,
            fallback_enabled=True,
            min_host_quality_score=7.0,
            min_bandwidth_mbps=100,
            max_latency_ms=100,
            description="Test zone description",
        )
        
        # Assert
        assert result is not None
        assert result.zone_id == zone_id
        assert result.zone_name == "TestZone"
        assert result.zone_display_name == "Test Zone"
        assert result.map_name == "test_map"
        assert result.max_players == 100
        assert result.recommended_players == 50
        assert result.p2p_enabled is True
        assert result.p2p_priority == 5
        assert result.fallback_enabled is True
        assert result.min_host_quality_score == 7.0
        assert result.min_bandwidth_mbps == 100
        assert result.max_latency_ms == 100
        assert result.description == "Test zone description"
        assert result.status == ZoneStatus.ENABLED
    
    async def test_create_zone_update_existing(self, service, db_session: AsyncSession):
        """Test updating an existing zone"""
        # Arrange - Create initial zone
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        initial_zone = await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="InitialZone",
            zone_display_name="Initial Zone",
            map_name="initial_map",
            max_players=50,
        )
        
        # Act - Update the zone
        updated_zone = await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="UpdatedZone",
            zone_display_name="Updated Zone",
            map_name="updated_map",
            max_players=200,
            recommended_players=100,
            p2p_priority=10,
        )
        
        # Assert
        assert updated_zone.id == initial_zone.id  # Same database ID
        assert updated_zone.zone_id == zone_id
        assert updated_zone.zone_name == "UpdatedZone"
        assert updated_zone.zone_display_name == "Updated Zone"
        assert updated_zone.map_name == "updated_map"
        assert updated_zone.max_players == 200
        assert updated_zone.recommended_players == 100
        assert updated_zone.p2p_priority == 10
    
    async def test_get_zone_success(self, service, db_session: AsyncSession):
        """Test getting a zone by ID"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        created_zone = await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="GetZone",
            zone_display_name="Get Zone",
            map_name="get_map",
        )
        
        # Act
        result = await service.get_zone(db_session, zone_id)
        
        # Assert
        assert result is not None
        assert result.id == created_zone.id
        assert result.zone_id == zone_id
    
    async def test_get_zone_not_found(self, service, db_session: AsyncSession):
        """Test getting non-existent zone returns None"""
        # Act
        result = await service.get_zone(db_session, "nonexistent_zone")
        
        # Assert
        assert result is None
    
    async def test_get_all_zones_no_filter(self, service, db_session: AsyncSession):
        """Test getting all zones without filters"""
        # Arrange - Create multiple zones
        zone1_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone2_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone3_id = f"zone_{uuid.uuid4().hex[:8]}"
        
        await service.create_zone(
            session=db_session,
            zone_id=zone1_id,
            zone_name="Zone1",
            zone_display_name="Zone 1",
            map_name="map1",
            p2p_enabled=True,
            p2p_priority=5,
        )
        await service.create_zone(
            session=db_session,
            zone_id=zone2_id,
            zone_name="Zone2",
            zone_display_name="Zone 2",
            map_name="map2",
            p2p_enabled=False,
            p2p_priority=3,
        )
        await service.create_zone(
            session=db_session,
            zone_id=zone3_id,
            zone_name="Zone3",
            zone_display_name="Zone 3",
            map_name="map3",
            p2p_enabled=True,
            p2p_priority=10,
        )

        # Act
        result = await service.get_all_zones(db_session)

        # Assert
        assert len(result) >= 3  # At least our 3 zones
        zone_ids = [z.zone_id for z in result]
        assert zone1_id in zone_ids
        assert zone2_id in zone_ids
        assert zone3_id in zone_ids

    async def test_get_all_zones_filter_p2p_enabled(self, service, db_session: AsyncSession):
        """Test getting zones filtered by P2P enabled"""
        # Arrange
        zone1_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone2_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone1_id,
            zone_name="P2PZone",
            zone_display_name="P2P Zone",
            map_name="p2p_map",
            p2p_enabled=True,
        )
        await service.create_zone(
            session=db_session,
            zone_id=zone2_id,
            zone_name="NoP2PZone",
            zone_display_name="No P2P Zone",
            map_name="nop2p_map",
            p2p_enabled=False,
        )

        # Act
        result = await service.get_all_zones(db_session, p2p_enabled=True)

        # Assert
        zone_ids = [z.zone_id for z in result]
        assert zone1_id in zone_ids
        assert zone2_id not in zone_ids
        assert all(z.p2p_enabled for z in result)

    async def test_get_all_zones_filter_status(self, service, db_session: AsyncSession):
        """Test getting zones filtered by status"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="EnabledZone",
            zone_display_name="Enabled Zone",
            map_name="enabled_map",
        )

        # Act
        result = await service.get_all_zones(db_session, status=ZoneStatus.ENABLED)

        # Assert
        zone_ids = [z.zone_id for z in result]
        assert zone_id in zone_ids
        assert all(z.status == ZoneStatus.ENABLED for z in result)

    async def test_get_p2p_enabled_zones(self, service, db_session: AsyncSession):
        """Test getting only P2P enabled zones"""
        # Arrange
        zone1_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone2_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone1_id,
            zone_name="P2PZone1",
            zone_display_name="P2P Zone 1",
            map_name="p2p1",
            p2p_enabled=True,
        )
        await service.create_zone(
            session=db_session,
            zone_id=zone2_id,
            zone_name="NoP2PZone1",
            zone_display_name="No P2P Zone 1",
            map_name="nop2p1",
            p2p_enabled=False,
        )

        # Act
        result = await service.get_p2p_enabled_zones(db_session)

        # Assert
        zone_ids = [z.zone_id for z in result]
        assert zone1_id in zone_ids
        assert zone2_id not in zone_ids
        assert all(z.p2p_enabled for z in result)

    async def test_enable_p2p_for_zone(self, service, db_session: AsyncSession):
        """Test enabling P2P for a zone"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="DisabledP2PZone",
            zone_display_name="Disabled P2P Zone",
            map_name="disabled",
            p2p_enabled=False,
        )

        # Act
        result = await service.enable_p2p_for_zone(db_session, zone_id)

        # Assert
        assert result is not None
        assert result.p2p_enabled is True

    async def test_enable_p2p_for_zone_not_found(self, service, db_session: AsyncSession):
        """Test enabling P2P for non-existent zone"""
        # Act
        result = await service.enable_p2p_for_zone(db_session, "nonexistent_zone")

        # Assert
        assert result is None

    async def test_disable_p2p_for_zone(self, service, db_session: AsyncSession):
        """Test disabling P2P for a zone"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="EnabledP2PZone",
            zone_display_name="Enabled P2P Zone",
            map_name="enabled",
            p2p_enabled=True,
        )

        # Act
        result = await service.disable_p2p_for_zone(db_session, zone_id)

        # Assert
        assert result is not None
        assert result.p2p_enabled is False

    async def test_disable_p2p_for_zone_not_found(self, service, db_session: AsyncSession):
        """Test disabling P2P for non-existent zone"""
        # Act
        result = await service.disable_p2p_for_zone(db_session, "nonexistent_zone")

        # Assert
        assert result is None

    async def test_update_zone_status(self, service, db_session: AsyncSession):
        """Test updating zone status"""
        # Arrange
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"

        await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="StatusZone",
            zone_display_name="Status Zone",
            map_name="status",
        )

        # Act
        result = await service.update_zone_status(db_session, zone_id, ZoneStatus.MAINTENANCE)

        # Assert
        assert result is not None
        assert result.status == ZoneStatus.MAINTENANCE

    async def test_update_zone_status_not_found(self, service, db_session: AsyncSession):
        """Test updating status for non-existent zone"""
        # Act
        result = await service.update_zone_status(db_session, "nonexistent_zone", ZoneStatus.DISABLED)

        # Assert
        assert result is None

    async def test_update_zone_statistics_success(self, service, db_session: AsyncSession):
        """Test updating zone statistics"""
        # Arrange - Create zone
        zone_id = f"zone_{uuid.uuid4().hex[:8]}"
        zone = await service.create_zone(
            session=db_session,
            zone_id=zone_id,
            zone_name="TestZone",
            zone_display_name="Test Zone",
            map_name="test_map",
            max_players=100,
            p2p_enabled=True,
        )

        # Act - Update statistics
        result = await service.update_zone_statistics(
            session=db_session,
            zone_id=zone_id,
            active_sessions_delta=2,
            total_sessions_delta=5,
            total_players_served_delta=10,
        )

        # Assert
        assert result is not None
        assert result.active_sessions == 2
        assert result.total_sessions == 5
        assert result.total_players_served == 10

    async def test_update_zone_statistics_not_found(self, service, db_session: AsyncSession):
        """Test updating statistics for non-existent zone returns None"""
        # Act
        result = await service.update_zone_statistics(
            session=db_session,
            zone_id="nonexistent_zone",
            active_sessions_delta=1,
            total_sessions_delta=1,
            total_players_served_delta=1,
        )

        # Assert
        assert result is None



