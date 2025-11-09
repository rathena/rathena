"""
Unit tests for database module
"""

import pytest
import asyncio
from unittest.mock import AsyncMock, MagicMock, patch
from database import Database


class TestDatabase:
    """Test Database class"""
    
    @pytest.mark.asyncio
    async def test_database_initialization(self):
        """Test database initializes correctly"""
        db = Database()
        assert db.pool is None
        assert db.client is None
    
    @pytest.mark.asyncio
    async def test_connect_success(self, mock_settings):
        """Test successful database connection"""
        db = Database()

        with patch('database.ConnectionPool') as mock_pool_class:
            with patch('database.aioredis.Redis') as mock_redis_class:
                mock_pool = MagicMock()
                mock_pool_class.return_value = mock_pool

                mock_client = AsyncMock()
                mock_client.ping = AsyncMock()
                mock_client.info = AsyncMock(return_value={'redis_version': '7.0.0', 'connected_clients': 1})
                mock_redis_class.return_value = mock_client

                await db.connect(max_retries=1, retry_delay=0.1)

                assert db.client is not None
                mock_pool_class.assert_called_once()
                mock_redis_class.assert_called_once()

    @pytest.mark.asyncio
    async def test_connect_retry_logic(self):
        """Test connection retry with exponential backoff"""
        db = Database()

        with patch('database.ConnectionPool') as mock_pool_class:
            with patch('database.aioredis.Redis') as mock_redis_class:
                mock_pool = MagicMock()
                mock_pool_class.return_value = mock_pool

                # Fail first 2 attempts, succeed on 3rd
                mock_client_fail = AsyncMock()
                mock_client_fail.ping = AsyncMock(side_effect=ConnectionError("Connection failed"))

                mock_client_success = AsyncMock()
                mock_client_success.ping = AsyncMock()
                mock_client_success.info = AsyncMock(return_value={'redis_version': '7.0.0', 'connected_clients': 1})

                mock_redis_class.side_effect = [mock_client_fail, mock_client_fail, mock_client_success]

                await db.connect(max_retries=3, retry_delay=0.1)

                assert db.client is not None
                assert mock_redis_class.call_count == 3

    @pytest.mark.asyncio
    async def test_connect_max_retries_exceeded(self):
        """Test connection fails after max retries"""
        db = Database()

        with patch('database.ConnectionPool') as mock_pool_class:
            with patch('database.aioredis.Redis') as mock_redis_class:
                mock_pool = MagicMock()
                mock_pool_class.return_value = mock_pool

                mock_client = AsyncMock()
                mock_client.ping = AsyncMock(side_effect=ConnectionError("Connection failed"))
                mock_redis_class.return_value = mock_client

                with pytest.raises(ConnectionError):
                    await db.connect(max_retries=2, retry_delay=0.1)

    @pytest.mark.asyncio
    async def test_disconnect(self):
        """Test database disconnect"""
        db = Database()
        db.client = AsyncMock()
        db.pool = AsyncMock()

        await db.disconnect()

        db.client.close.assert_called_once()
        db.pool.disconnect.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_get_npc_state(self, mock_database):
        """Test getting NPC state"""
        db = Database()
        db.client = mock_database.client
        
        mock_database.client.hgetall.return_value = {
            b"npc_id": b"test_001",
            b"name": b"Test NPC",
            b"level": b"50"
        }
        
        state = await db.get_npc_state("test_001")
        
        assert state is not None
        mock_database.client.hgetall.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_set_npc_state(self, mock_database):
        """Test setting NPC state"""
        db = Database()
        db.client = mock_database.client
        
        npc_data = {
            "npc_id": "test_001",
            "name": "Test NPC",
            "level": 50
        }
        
        await db.set_npc_state("test_001", npc_data)
        
        mock_database.client.hset.assert_called()
    
    @pytest.mark.asyncio
    async def test_get_player_memory(self, mock_database):
        """Test getting player-NPC memory"""
        db = Database()
        db.client = mock_database.client
        
        mock_database.client.lrange.return_value = [
            b'{"event": "greeting", "timestamp": "2024-01-01"}',
            b'{"event": "trade", "timestamp": "2024-01-02"}'
        ]
        
        memories = await db.get_player_memory("player_001", "npc_001")
        
        assert len(memories) == 2
        mock_database.client.lrange.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_add_player_memory(self, mock_database):
        """Test adding player-NPC memory"""
        db = Database()
        db.client = mock_database.client
        
        memory = {
            "event": "conversation",
            "content": "Discussed quest",
            "timestamp": "2024-01-03"
        }
        
        await db.add_player_memory("player_001", "npc_001", memory)
        
        mock_database.client.rpush.assert_called()
    
    @pytest.mark.asyncio
    async def test_rate_limit_check(self, mock_database):
        """Test rate limit checking"""
        db = Database()
        db.client = mock_database.client
        
        # Not rate limited
        mock_database.client.exists.return_value = 0
        is_limited = await db.check_rate_limit("player_001", "action_type", 60)
        assert is_limited is False
        
        # Rate limited
        mock_database.client.exists.return_value = 1
        mock_database.client.ttl.return_value = 30
        is_limited = await db.check_rate_limit("player_001", "action_type", 60)
        assert is_limited is True
    
    @pytest.mark.asyncio
    async def test_set_rate_limit(self, mock_database):
        """Test setting rate limit"""
        db = Database()
        db.client = mock_database.client
        
        await db.set_rate_limit("player_001", "action_type", 60)
        
        mock_database.client.setex.assert_called_once()
    
    @pytest.mark.asyncio
    async def test_quest_operations(self, mock_database):
        """Test quest CRUD operations"""
        db = Database()
        db.client = mock_database.client
        
        quest_data = {
            "quest_id": "quest_001",
            "title": "Test Quest",
            "giver_npc_id": "npc_001"
        }
        
        # Create quest
        await db.create_quest(quest_data)
        # store_quest uses set, not hset
        mock_database.client.set.assert_called()

        # Get quest
        mock_database.client.get.return_value = b'{"quest_id": "quest_001", "title": "Test Quest"}'
        quest = await db.get_quest("quest_001")
        assert quest is not None
        assert quest["quest_id"] == "quest_001"

        # Delete quest
        mock_database.client.get.return_value = b'{"quest_id": "quest_001", "giver_npc_id": "npc_001"}'
        await db.delete_quest("quest_001")
        mock_database.client.delete.assert_called()


class TestDatabaseErrorHandling:
    """Test database error handling"""

    @pytest.mark.asyncio
    async def test_connection_error_handling(self):
        """Test connection error is properly handled"""
        db = Database()

        with patch('database.ConnectionPool') as mock_pool_class:
            with patch('database.aioredis.Redis') as mock_redis_class:
                mock_pool = MagicMock()
                mock_pool_class.return_value = mock_pool

                mock_client = AsyncMock()
                mock_client.ping = AsyncMock(side_effect=ConnectionError("Network error"))
                mock_redis_class.return_value = mock_client

                with pytest.raises(ConnectionError):
                    await db.connect(max_retries=1, retry_delay=0.1)

    @pytest.mark.asyncio
    async def test_timeout_error_handling(self):
        """Test timeout error is properly handled"""
        db = Database()

        with patch('database.ConnectionPool') as mock_pool_class:
            with patch('database.aioredis.Redis') as mock_redis_class:
                mock_pool = MagicMock()
                mock_pool_class.return_value = mock_pool

                mock_client = AsyncMock()
                mock_client.ping = AsyncMock(side_effect=TimeoutError("Connection timeout"))
                mock_redis_class.return_value = mock_client

                with pytest.raises(ConnectionError):  # Our code wraps all errors in ConnectionError
                    await db.connect(max_retries=1, retry_delay=0.1)

