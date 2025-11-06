"""
DragonflyDB / Redis connection management
"""

import asyncio
import json
import redis.asyncio as aioredis
from redis.asyncio import ConnectionPool
from typing import Optional
from loguru import logger

try:
    from .config import settings
except ImportError:
    from config import settings


class Database:
    """DragonflyDB / Redis database connection manager"""
    
    def __init__(self):
        self.pool: Optional[ConnectionPool] = None
        self.client: Optional[aioredis.Redis] = None
        
    async def connect(self, max_retries: int = None, retry_delay: float = None):
        """
        Establish connection to DragonflyDB / Redis with retry logic

        Args:
            max_retries: Maximum number of connection attempts (defaults to settings value)
            retry_delay: Initial delay between retries in seconds (defaults to settings value)
        """
        # Use configuration defaults if not specified
        if max_retries is None:
            max_retries = settings.db_connection_max_retries
        if retry_delay is None:
            retry_delay = settings.db_connection_retry_delay

        last_error = None

        for attempt in range(1, max_retries + 1):
            try:
                logger.info(f"Connecting to Redis at {settings.redis_host}:{settings.redis_port} (attempt {attempt}/{max_retries})")

                # Create connection pool
                self.pool = ConnectionPool(
                    host=settings.redis_host,
                    port=settings.redis_port,
                    db=settings.redis_db,
                    password=settings.redis_password,
                    max_connections=settings.redis_max_connections,
                    decode_responses=False,  # Changed to False to support binary data
                    encoding="utf-8",
                )

                # Create Redis client
                self.client = aioredis.Redis(connection_pool=self.pool)

                # Test connection
                await self.client.ping()
                logger.info("✓ Successfully connected to Redis/DragonflyDB")

                # Log database info
                info = await self.client.info()
                logger.info(f"Redis version: {info.get('redis_version', 'unknown')}")
                logger.info(f"Connected clients: {info.get('connected_clients', 0)}")

                return  # Success - exit retry loop

            except Exception as e:
                last_error = e
                logger.warning(f"Connection attempt {attempt}/{max_retries} failed: {e}")

                if attempt < max_retries:
                    # Exponential backoff
                    wait_time = retry_delay * (2 ** (attempt - 1))
                    logger.info(f"Retrying in {wait_time:.1f} seconds...")
                    await asyncio.sleep(wait_time)
                else:
                    logger.error(f"Failed to connect to Redis after {max_retries} attempts")
                    raise ConnectionError(f"Could not connect to Redis: {last_error}") from last_error
    
    async def disconnect(self):
        """Close connection to DragonflyDB / Redis"""
        try:
            if self.client:
                await self.client.close()
                logger.info("Disconnected from Redis/DragonflyDB")
            
            if self.pool:
                await self.pool.disconnect()
                
        except Exception as e:
            logger.error(f"Error disconnecting from Redis: {e}")
    
    async def get_client(self) -> aioredis.Redis:
        """Get Redis client instance"""
        if not self.client:
            await self.connect()
        return self.client
    
    async def health_check(self) -> bool:
        """Check if database connection is healthy"""
        try:
            if not self.client:
                return False
            await self.client.ping()
            return True
        except Exception as e:
            logger.error(f"Database health check failed: {e}")
            return False
    
    # NPC State Operations
    async def set_npc_state(self, npc_id: str, state_data: dict):
        """Set NPC state in database"""
        try:
            key = f"npc:{npc_id}"
            await self.client.hset(key, mapping=state_data)
            logger.debug(f"Set state for NPC {npc_id}")
        except Exception as e:
            logger.error(f"Error setting NPC state for {npc_id}: {e}")
            raise
    
    async def get_npc_state(self, npc_id: str) -> Optional[dict]:
        """Get NPC state from database"""
        try:
            key = f"npc:{npc_id}"
            state = await self.client.hgetall(key)
            return state if state else None
        except Exception as e:
            logger.error(f"Error getting NPC state for {npc_id}: {e}")
            return None
    
    async def delete_npc_state(self, npc_id: str):
        """Delete NPC state from database"""
        try:
            key = f"npc:{npc_id}"
            await self.client.delete(key)
            logger.debug(f"Deleted state for NPC {npc_id}")
        except Exception as e:
            logger.error(f"Error deleting NPC state for {npc_id}: {e}")
            raise
    
    # World State Operations
    async def set_world_state(self, state_type: str, state_data: dict):
        """Set world state in database"""
        try:
            key = f"world:{state_type}"
            await self.client.hset(key, mapping=state_data)
            logger.debug(f"Set world state for {state_type}")
        except Exception as e:
            logger.error(f"Error setting world state for {state_type}: {e}")
            raise
    
    async def get_world_state(self, state_type: str) -> Optional[dict]:
        """Get world state from database"""
        try:
            key = f"world:{state_type}"
            state = await self.client.hgetall(key)
            return state if state else None
        except Exception as e:
            logger.error(f"Error getting world state for {state_type}: {e}")
            return None
    
    # Event Queue Operations
    async def push_event(self, event_id: str, event_data: dict, priority: int = 1):
        """Push event to event queue (sorted set by priority)"""
        try:
            key = "events:queue"
            event_json = json.dumps(event_data)
            await self.client.zadd(key, {event_json: priority})
            logger.debug(f"Pushed event {event_id} to queue with priority {priority}")
        except Exception as e:
            logger.error(f"Error pushing event {event_id}: {e}")
            raise

    async def pop_event(self) -> Optional[dict]:
        """Pop highest priority event from queue"""
        try:
            key = "events:queue"
            # Get highest priority event (lowest score)
            result = await self.client.zpopmin(key, count=1)
            if result:
                event_json, priority = result[0]
                # Decode if bytes
                if isinstance(event_json, bytes):
                    event_json = event_json.decode('utf-8')
                return json.loads(event_json)
            return None
        except Exception as e:
            logger.error(f"Error popping event: {e}")
            return None

    async def store_quest(self, quest_id: str, quest_data: dict):
        """Store quest data"""
        try:
            key = f"quest:{quest_id}"
            quest_json = json.dumps(quest_data)
            await self.client.set(key, quest_json)

            # Add to quest index
            await self.client.sadd("quests:all", quest_id)

            # Add to NPC's quest list
            npc_id = quest_data.get("giver_npc_id")
            if npc_id:
                await self.client.sadd(f"quests:npc:{npc_id}", quest_id)

            logger.debug(f"Stored quest {quest_id}")
        except Exception as e:
            logger.error(f"Error storing quest {quest_id}: {e}")
            raise

    async def get_quest(self, quest_id: str) -> Optional[dict]:
        """Get quest data by ID"""
        try:
            key = f"quest:{quest_id}"
            quest_json = await self.client.get(key)
            if quest_json:
                # Decode if bytes
                if isinstance(quest_json, bytes):
                    quest_json = quest_json.decode('utf-8')
                return json.loads(quest_json)
            return None
        except Exception as e:
            logger.error(f"Error getting quest {quest_id}: {e}")
            return None

    async def get_npc_quests(self, npc_id: str) -> list:
        """Get all quests for an NPC"""
        try:
            quest_ids = await self.client.smembers(f"quests:npc:{npc_id}")
            quests = []
            for quest_id in quest_ids:
                quest_data = await self.get_quest(quest_id)
                if quest_data:
                    quests.append(quest_data)
            return quests
        except Exception as e:
            logger.error(f"Error getting NPC quests for {npc_id}: {e}")
            return []

    async def delete_quest(self, quest_id: str):
        """Delete a quest"""
        try:
            # Get quest data first to remove from NPC index
            quest_data = await self.get_quest(quest_id)

            # Delete quest
            key = f"quest:{quest_id}"
            await self.client.delete(key)

            # Remove from indexes
            await self.client.srem("quests:all", quest_id)

            if quest_data:
                npc_id = quest_data.get("giver_npc_id")
                if npc_id:
                    await self.client.srem(f"quests:npc:{npc_id}", quest_id)

            logger.debug(f"Deleted quest {quest_id}")
        except Exception as e:
            logger.error(f"Error deleting quest {quest_id}: {e}")
            raise


# Global database instance
db = Database()

