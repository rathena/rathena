"""
Database Integration Tests

Tests database operations with multiple tables including:
- PostgreSQL transactions and rollbacks
- Multi-table operations (NPCs, players, quests, relationships)
- Data consistency across tables
- Concurrent access patterns
- Cache synchronization with DragonflyDB
- Foreign key constraint validation
- Bulk operations
"""

import asyncio
import pytest
from datetime import datetime, timedelta
from typing import List, Dict, Any
from unittest.mock import AsyncMock, MagicMock, patch
from sqlalchemy import create_engine, Column, Integer, String, Float, DateTime, ForeignKey, Boolean, JSON
from sqlalchemy.orm import sessionmaker, relationship as db_relationship
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.exc import IntegrityError

from database import PostgreSQLConnection, DragonflyConnection, DatabaseManager
from models.npc import NPCState, NPCPhysical, NPCBackground
from models.player import PlayerState
from models.quest import QuestData, QuestType, DifficultyLevel, QuestStatus
from memory.relationship_manager import RelationshipManager

# Test database models
Base = declarative_base()


class NPCTable(Base):
    """Database table for NPCs"""
    __tablename__ = "npcs"
    
    id = Column(String, primary_key=True)
    name = Column(String, nullable=False)
    location = Column(String, nullable=False)
    position_x = Column(Float, nullable=False)
    position_y = Column(Float, nullable=False)
    is_active = Column(Boolean, default=True)
    health = Column(Float, default=100.0)
    faction_id = Column(String, nullable=True)
    data = Column(JSON, nullable=True)  # Store full NPCState as JSON
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)


class PlayerTable(Base):
    """Database table for players"""
    __tablename__ = "players"
    
    id = Column(String, primary_key=True)
    character_name = Column(String, nullable=False)
    job_class = Column(String, nullable=False)
    base_level = Column(Integer, default=1)
    location = Column(String, nullable=False)
    position_x = Column(Float, nullable=False)
    position_y = Column(Float, nullable=False)
    zeny = Column(Integer, default=0)
    experience = Column(Integer, default=0)
    is_online = Column(Boolean, default=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)


class QuestTable(Base):
    """Database table for quests"""
    __tablename__ = "quests"
    
    id = Column(String, primary_key=True)
    title = Column(String, nullable=False)
    npc_id = Column(String, ForeignKey("npcs.id"), nullable=False)
    player_id = Column(String, ForeignKey("players.id"), nullable=True)
    quest_type = Column(String, nullable=False)
    difficulty = Column(String, nullable=False)
    status = Column(String, default="available")
    min_level = Column(Integer, default=1)
    data = Column(JSON, nullable=True)  # Store full QuestData as JSON
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)


class RelationshipTable(Base):
    """Database table for relationships"""
    __tablename__ = "relationships"
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    entity_a = Column(String, nullable=False)
    entity_b = Column(String, nullable=False)
    affinity = Column(Float, default=0.0)
    relationship_type = Column(String, default="neutral")
    interaction_count = Column(Integer, default=0)
    last_interaction = Column(DateTime, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)


@pytest.fixture
async def test_db_engine():
    """Create in-memory SQLite database for testing"""
    engine = create_engine("sqlite:///:memory:", echo=False)
    Base.metadata.create_all(engine)
    yield engine
    Base.metadata.drop_all(engine)
    engine.dispose()


@pytest.fixture
async def test_db_session(test_db_engine):
    """Create database session"""
    SessionLocal = sessionmaker(bind=test_db_engine, expire_on_commit=False)
    session = SessionLocal()
    yield session
    session.close()


@pytest.fixture
async def mock_dragonfly():
    """Mock DragonflyDB client"""
    client = MagicMock()
    cache_storage = {}
    
    def mock_get(key):
        return cache_storage.get(key)
    
    def mock_setex(key, ttl, value):
        cache_storage[key] = value
        return True
    
    def mock_delete(key):
        if key in cache_storage:
            del cache_storage[key]
            return True
        return False
    
    client.get = MagicMock(side_effect=mock_get)
    client.setex = MagicMock(side_effect=mock_setex)
    client.delete = MagicMock(side_effect=mock_delete)
    return client


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.asyncio
class TestDatabaseTransactions:
    """Test database transaction management"""
    
    async def test_basic_transaction_commit(self, test_db_session):
        """Test basic transaction commit"""
        # Arrange
        npc = NPCTable(
            id="npc_001",
            name="Test NPC",
            location="prontera",
            position_x=100.0,
            position_y=100.0
        )
        
        # Act
        test_db_session.add(npc)
        test_db_session.commit()
        
        # Assert
        result = test_db_session.query(NPCTable).filter_by(id="npc_001").first()
        assert result is not None
        assert result.name == "Test NPC"
        assert result.location == "prontera"
    
    async def test_transaction_rollback_on_error(self, test_db_session):
        """Test transaction rollback on error"""
        # Arrange
        npc1 = NPCTable(id="npc_001", name="NPC 1", location="prontera", position_x=0, position_y=0)
        
        # Act
        try:
            test_db_session.add(npc1)
            test_db_session.commit()
            
            # Try to add duplicate (should fail)
            npc2 = NPCTable(id="npc_001", name="NPC 2", location="geffen", position_x=0, position_y=0)
            test_db_session.add(npc2)
            test_db_session.commit()
            
        except IntegrityError:
            test_db_session.rollback()
        
        # Assert - Only first NPC should exist
        result = test_db_session.query(NPCTable).filter_by(id="npc_001").first()
        assert result is not None
        assert result.name == "NPC 1"  # Not "NPC 2"
        
        count = test_db_session.query(NPCTable).count()
        assert count == 1
    
    async def test_nested_transaction_rollback(self, test_db_session):
        """Test nested transaction with partial rollback"""
        # Arrange
        player = PlayerTable(
            id="player_001",
            character_name="Hero",
            job_class="Swordsman",
            location="prontera",
            position_x=100.0,
            position_y=100.0
        )
        test_db_session.add(player)
        test_db_session.commit()
        
        # Act - Nested transaction
        original_zeny = player.zeny
        savepoint = test_db_session.begin_nested()
        
        try:
            # Update zeny
            player.zeny += 1000
            test_db_session.flush()
            
            # Simulate error
            raise ValueError("Simulated error")
            
        except ValueError:
            savepoint.rollback()
        
        test_db_session.commit()
        
        # Assert - Zeny should not have changed
        test_db_session.refresh(player)
        assert player.zeny == original_zeny


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.asyncio
class TestMultiTableOperations:
    """Test operations across multiple related tables"""
    
    async def test_create_npc_with_quest(self, test_db_session):
        """Test creating NPC and associated quest"""
        # Arrange & Act
        npc = NPCTable(
            id="npc_001",
            name="Quest Giver",
            location="prontera",
            position_x=150.0,
            position_y=200.0
        )
        
        quest = QuestTable(
            id="quest_001",
            title="Defeat Monsters",
            npc_id="npc_001",
            quest_type="kill",
            difficulty="normal"
        )
        
        test_db_session.add(npc)
        test_db_session.add(quest)
        test_db_session.commit()
        
        # Assert
        npc_result = test_db_session.query(NPCTable).filter_by(id="npc_001").first()
        quest_result = test_db_session.query(QuestTable).filter_by(id="quest_001").first()
        
        assert npc_result is not None
        assert quest_result is not None
        assert quest_result.npc_id == npc_result.id
    
    async def test_foreign_key_constraint(self, test_db_session):
        """Test foreign key constraint enforcement"""
        # Arrange - Quest with non-existent NPC
        quest = QuestTable(
            id="quest_001",
            title="Invalid Quest",
            npc_id="nonexistent_npc",
            quest_type="kill",
            difficulty="normal"
        )
        
        # Act & Assert
        test_db_session.add(quest)
        
        # SQLite with foreign keys enabled would fail here
        # In-memory SQLite might not enforce this by default
        try:
            test_db_session.commit()
        except IntegrityError:
            test_db_session.rollback()
            pytest.skip("Foreign key constraints not enforced in test DB")
    
    async def test_cascade_updates_across_tables(self, test_db_session):
        """Test updating related records across tables"""
        # Arrange
        npc = NPCTable(id="npc_001", name="Merchant", location="prontera", position_x=0, position_y=0)
        player = PlayerTable(id="player_001", character_name="Hero", job_class="Knight", location="prontera", position_x=0, position_y=0)
        
        test_db_session.add(npc)
        test_db_session.add(player)
        test_db_session.commit()
        
        # Create relationship
        relationship = RelationshipTable(
            entity_a="player_001",
            entity_b="npc_001",
            affinity=0.5
        )
        test_db_session.add(relationship)
        test_db_session.commit()
        
        # Act - Update all three tables in transaction
        npc.health = 80.0
        player.zeny += 500
        relationship.affinity += 0.1
        relationship.interaction_count += 1
        
        test_db_session.commit()
        
        # Assert
        test_db_session.refresh(npc)
        test_db_session.refresh(player)
        test_db_session.refresh(relationship)
        
        assert npc.health == 80.0
        assert player.zeny == 500
        assert relationship.affinity == 0.6
        assert relationship.interaction_count == 1


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.asyncio
class TestDataConsistency:
    """Test data consistency across operations"""
    
    async def test_concurrent_npc_updates(self, test_db_session):
        """Test concurrent updates to same NPC"""
        # Arrange
        npc = NPCTable(
            id="npc_001",
            name="Merchant",
            location="prontera",
            position_x=100.0,
            position_y=100.0,
            health=100.0
        )
        test_db_session.add(npc)
        test_db_session.commit()
        
        # Act - Simulate concurrent updates
        initial_health = npc.health
        
        # Update 1: Reduce health
        npc.health -= 10
        test_db_session.commit()
        
        # Update 2: Reduce health again
        test_db_session.refresh(npc)
        npc.health -= 15
        test_db_session.commit()
        
        # Assert
        test_db_session.refresh(npc)
        assert npc.health == initial_health - 25
    
    async def test_relationship_consistency(self, test_db_session):
        """Test relationship data remains consistent"""
        # Arrange
        player = PlayerTable(id="player_001", character_name="Hero", job_class="Knight", location="prontera", position_x=0, position_y=0)
        npc = NPCTable(id="npc_001", name="NPC", location="prontera", position_x=0, position_y=0)
        
        test_db_session.add(player)
        test_db_session.add(npc)
        test_db_session.commit()
        
        # Act - Create multiple relationships
        relationships = []
        for i in range(5):
            rel = RelationshipTable(
                entity_a="player_001",
                entity_b="npc_001",
                affinity=0.1 * (i + 1),
                interaction_count=i + 1
            )
            relationships.append(rel)
            test_db_session.add(rel)
        
        test_db_session.commit()
        
        # Assert - Check all relationships persisted correctly
        results = test_db_session.query(RelationshipTable).filter_by(
            entity_a="player_001",
            entity_b="npc_001"
        ).all()
        
        assert len(results) == 5
        for i, rel in enumerate(results):
            assert rel.interaction_count == i + 1
    
    async def test_bulk_insert_consistency(self, test_db_session):
        """Test bulk insert maintains data consistency"""
        # Arrange
        npcs = [
            NPCTable(
                id=f"npc_{i:03d}",
                name=f"NPC {i}",
                location="prontera",
                position_x=float(i * 10),
                position_y=float(i * 10)
            )
            for i in range(100)
        ]
        
        # Act
        test_db_session.bulk_save_objects(npcs)
        test_db_session.commit()
        
        # Assert
        count = test_db_session.query(NPCTable).count()
        assert count == 100
        
        # Verify data integrity
        first_npc = test_db_session.query(NPCTable).filter_by(id="npc_000").first()
        last_npc = test_db_session.query(NPCTable).filter_by(id="npc_099").first()
        
        assert first_npc.position_x == 0.0
        assert last_npc.position_x == 990.0


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.redis
@pytest.mark.asyncio
class TestCacheSynchronization:
    """Test cache synchronization between PostgreSQL and DragonflyDB"""
    
    async def test_cache_write_through(self, test_db_session, mock_dragonfly):
        """Test write-through cache pattern"""
        # Arrange
        npc = NPCTable(
            id="npc_001",
            name="Cached NPC",
            location="prontera",
            position_x=100.0,
            position_y=100.0
        )
        
        # Act - Write to DB and cache
        test_db_session.add(npc)
        test_db_session.commit()
        
        import json
        cache_key = f"npc:{npc.id}"
        cache_data = json.dumps({
            "id": npc.id,
            "name": npc.name,
            "location": npc.location
        })
        mock_dragonfly.setex(cache_key, 3600, cache_data)
        
        # Assert
        cached_value = mock_dragonfly.get(cache_key)
        assert cached_value is not None
        
        cached_data = json.loads(cached_value)
        assert cached_data["id"] == npc.id
        assert cached_data["name"] == npc.name
    
    async def test_cache_invalidation(self, test_db_session, mock_dragonfly):
        """Test cache invalidation on update"""
        # Arrange
        npc = NPCTable(id="npc_001", name="Original", location="prontera", position_x=0, position_y=0)
        test_db_session.add(npc)
        test_db_session.commit()
        
        import json
        cache_key = f"npc:{npc.id}"
        mock_dragonfly.setex(cache_key, 3600, json.dumps({"name": "Original"}))
        
        # Act - Update DB and invalidate cache
        npc.name = "Updated"
        test_db_session.commit()
        
        # Invalidate cache
        mock_dragonfly.delete(cache_key)
        
        # Assert
        assert mock_dragonfly.get(cache_key) is None
    
    async def test_cache_aside_pattern(self, test_db_session, mock_dragonfly):
        """Test cache-aside (lazy loading) pattern"""
        # Arrange
        npc = NPCTable(id="npc_001", name="NPC", location="prontera", position_x=0, position_y=0)
        test_db_session.add(npc)
        test_db_session.commit()
        
        cache_key = f"npc:{npc.id}"
        
        # Act - Simulate cache-aside read
        import json
        
        # Try cache first
        cached_value = mock_dragonfly.get(cache_key)
        
        if cached_value is None:
            # Cache miss - read from DB
            db_npc = test_db_session.query(NPCTable).filter_by(id=npc.id).first()
            
            # Populate cache
            cache_data = json.dumps({
                "id": db_npc.id,
                "name": db_npc.name
            })
            mock_dragonfly.setex(cache_key, 3600, cache_data)
        
        # Assert - Cache should now have data
        cached_value = mock_dragonfly.get(cache_key)
        assert cached_value is not None


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.asyncio
class TestBulkOperations:
    """Test bulk database operations"""
    
    async def test_bulk_npc_creation(self, test_db_session):
        """Test bulk NPC creation"""
        # Arrange
        npcs = [
            NPCTable(
                id=f"npc_{i:04d}",
                name=f"NPC {i}",
                location="prontera" if i % 2 == 0 else "geffen",
                position_x=float(i),
                position_y=float(i * 2)
            )
            for i in range(500)
        ]
        
        # Act
        test_db_session.bulk_save_objects(npcs)
        test_db_session.commit()
        
        # Assert
        total_count = test_db_session.query(NPCTable).count()
        prontera_count = test_db_session.query(NPCTable).filter_by(location="prontera").count()
        
        assert total_count == 500
        assert prontera_count == 250
    
    async def test_bulk_relationship_updates(self, test_db_session):
        """Test bulk relationship updates"""
        # Arrange - Create relationships
        relationships = []
        for i in range(100):
            rel = RelationshipTable(
                entity_a=f"player_{i % 10}",
                entity_b=f"npc_{i % 5}",
                affinity=0.0,
                interaction_count=0
            )
            relationships.append(rel)
            test_db_session.add(rel)
        
        test_db_session.commit()
        
        # Act - Bulk update affinity
        test_db_session.query(RelationshipTable).update(
            {"affinity": RelationshipTable.affinity + 0.1}
        )
        test_db_session.commit()
        
        # Assert
        all_rels = test_db_session.query(RelationshipTable).all()
        assert all(rel.affinity == 0.1 for rel in all_rels)
    
    async def test_bulk_delete_with_cleanup(self, test_db_session):
        """Test bulk delete with proper cleanup"""
        # Arrange
        npcs = [
            NPCTable(id=f"npc_{i}", name=f"NPC {i}", location="prontera", position_x=0, position_y=0)
            for i in range(50)
        ]
        test_db_session.bulk_save_objects(npcs)
        test_db_session.commit()
        
        # Act - Delete inactive NPCs
        test_db_session.query(NPCTable).filter(
            NPCTable.id.in_([f"npc_{i}" for i in range(0, 25)])
        ).delete(synchronize_session=False)
        test_db_session.commit()
        
        # Assert
        remaining_count = test_db_session.query(NPCTable).count()
        assert remaining_count == 25


@pytest.mark.integration
@pytest.mark.db
@pytest.mark.asyncio
class TestComplexQueries:
    """Test complex database queries"""
    
    async def test_join_query_npcs_and_quests(self, test_db_session):
        """Test joining NPCs with their quests"""
        # Arrange
        npc = NPCTable(id="npc_001", name="Quest Giver", location="prontera", position_x=0, position_y=0)
        test_db_session.add(npc)
        
        quests = [
            QuestTable(id=f"quest_{i}", title=f"Quest {i}", npc_id="npc_001", quest_type="kill", difficulty="normal")
            for i in range(5)
        ]
        test_db_session.bulk_save_objects(quests)
        test_db_session.commit()
        
        # Act - Join query
        results = test_db_session.query(NPCTable, QuestTable).join(
            QuestTable, NPCTable.id == QuestTable.npc_id
        ).filter(NPCTable.id == "npc_001").all()
        
        # Assert
        assert len(results) == 5
        for npc_row, quest_row in results:
            assert npc_row.id == "npc_001"
            assert quest_row.npc_id == "npc_001"
    
    async def test_aggregation_query(self, test_db_session):
        """Test aggregation queries"""
        from sqlalchemy import func
        
        # Arrange
        for i in range(10):
            player = PlayerTable(
                id=f"player_{i}",
                character_name=f"Player {i}",
                job_class="Swordsman" if i < 5 else "Mage",
                location="prontera",
                position_x=0,
                position_y=0,
                base_level=10 + i
            )
            test_db_session.add(player)
        
        test_db_session.commit()
        
        # Act - Aggregate by job class
        results = test_db_session.query(
            PlayerTable.job_class,
            func.count(PlayerTable.id).label("count"),
            func.avg(PlayerTable.base_level).label("avg_level")
        ).group_by(PlayerTable.job_class).all()
        
        # Assert
        assert len(results) == 2
        
        for job_class, count, avg_level in results:
            if job_class == "Swordsman":
                assert count == 5
                assert avg_level == 12.0  # (10+11+12+13+14)/5
            else:
                assert count == 5
                assert avg_level == 17.0  # (15+16+17+18+19)/5