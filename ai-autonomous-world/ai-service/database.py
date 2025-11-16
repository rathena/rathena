"""
Database connection management for AI Service
- DragonFlyDB: High-speed caching and real-time state (Redis protocol)
- PostgreSQL: Persistent memory storage for OpenMemory SDK

Note: DragonFlyDB is used as a drop-in replacement for Redis, but all configuration and documentation should reference DragonFlyDB as the backend.
"""

import asyncio
import json
import redis.asyncio as aioredis
from redis.asyncio import ConnectionPool
from typing import Optional
from loguru import logger
from prometheus_client import Counter, Gauge

# Prometheus metrics for DragonFlyDB
dragonfly_connection_attempts = Counter(
    "dragonfly_connection_attempts_total",
    "Total DragonFlyDB connection attempts"
)
dragonfly_connection_failures = Counter(
    "dragonfly_connection_failures_total",
    "Total DragonFlyDB connection failures"
)
dragonfly_connection_success = Counter(
    "dragonfly_connection_success_total",
    "Total successful DragonFlyDB connections"
)
dragonfly_health_gauge = Gauge(
    "dragonfly_health_status",
    "DragonFlyDB health status (1=healthy, 0=unhealthy)"
)

try:
    from config import settings
except ImportError:
    from config import settings


class PostgreSQLManager:
    """PostgreSQL database connection manager for persistent memory storage"""

    def __init__(self):
        self.engine = None
        self.session_factory = None
        self._initialized = False

    async def connect(self, max_retries: int = None, retry_delay: float = None):
        """
        Establish connection to PostgreSQL with retry logic

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
                logger.info(f"Connecting to PostgreSQL at {settings.postgres_host}:{settings.postgres_port}/{settings.postgres_db} (attempt {attempt}/{max_retries})")

                # Import SQLAlchemy here to avoid import errors if not installed
                from sqlalchemy import create_engine, text
                from sqlalchemy.orm import sessionmaker
                from sqlalchemy.pool import QueuePool

                # Create engine with connection pooling
                self.engine = create_engine(
                    settings.postgres_connection_string,
                    poolclass=QueuePool,
                    pool_size=settings.postgres_pool_size,
                    max_overflow=settings.postgres_max_overflow,
                    pool_pre_ping=True,  # Verify connections before using
                    echo=settings.postgres_echo_sql,
                )

                # Test connection
                with self.engine.connect() as conn:
                    result = conn.execute(text("SELECT version()"))
                    version = result.scalar()
                    logger.info(f"✓ Successfully connected to PostgreSQL")
                    logger.info(f"PostgreSQL version: {version}")

                # Create session factory
                self.session_factory = sessionmaker(bind=self.engine)
                self._initialized = True

                return  # Success - exit retry loop

            except Exception as e:
                last_error = e
                logger.warning(f"PostgreSQL connection attempt {attempt}/{max_retries} failed: {e}")

                if attempt < max_retries:
                    # Exponential backoff
                    wait_time = retry_delay * (2 ** (attempt - 1))
                    logger.info(f"Retrying in {wait_time:.1f} seconds...")
                    await asyncio.sleep(wait_time)
                else:
                    logger.error(f"Failed to connect to PostgreSQL after {max_retries} attempts")
                    raise ConnectionError(f"Could not connect to PostgreSQL: {last_error}") from last_error

    async def disconnect(self):
        """Close connection to PostgreSQL"""
        try:
            if self.engine:
                self.engine.dispose()
                logger.info("Disconnected from PostgreSQL")
                self._initialized = False
        except Exception as e:
            logger.error(f"Error disconnecting from PostgreSQL: {e}")

    def get_session(self):
        """Get a new database session"""
        if not self._initialized or not self.session_factory:
            raise RuntimeError("PostgreSQL not initialized. Call connect() first.")
        return self.session_factory()

    async def health_check(self) -> bool:
        """Check if PostgreSQL connection is healthy"""
        try:
            if not self.engine:
                return False

            from sqlalchemy import text
            with self.engine.connect() as conn:
                conn.execute(text("SELECT 1"))
            return True
        except Exception as e:
            logger.error(f"PostgreSQL health check failed: {e}")
            return False

    async def fetch_one(self, query: str, *args):
        """
        Execute a query and fetch one result

        Args:
            query: SQL query with $1, $2, etc. placeholders
            *args: Query parameters

        Returns:
            Row object or None
        """
        if not self.engine:
            raise RuntimeError("PostgreSQL not initialized. Call connect() first.")

        try:
            from sqlalchemy import text

            # Convert PostgreSQL-style placeholders ($1, $2) to SQLAlchemy-style (:param1, :param2)
            converted_query = query
            params = {}
            for i, arg in enumerate(args, 1):
                placeholder = f"${i}"
                param_name = f"param{i}"
                converted_query = converted_query.replace(placeholder, f":{param_name}")
                params[param_name] = arg

            with self.engine.connect() as conn:
                result = conn.execute(text(converted_query), params)
                row = result.fetchone()
                if row:
                    # Convert Row to dict
                    return dict(row._mapping)
                return None
        except Exception as e:
            logger.error(f"Error executing fetch_one query: {e}")
            raise

    async def fetch_all(self, query: str, *args):
        """
        Execute a query and fetch all results

        Args:
            query: SQL query with $1, $2, etc. placeholders
            *args: Query parameters

        Returns:
            List of row objects
        """
        if not self.engine:
            raise RuntimeError("PostgreSQL not initialized. Call connect() first.")

        try:
            from sqlalchemy import text

            # Convert PostgreSQL-style placeholders ($1, $2) to SQLAlchemy-style (:param1, :param2)
            converted_query = query
            params = {}
            for i, arg in enumerate(args, 1):
                placeholder = f"${i}"
                param_name = f"param{i}"
                converted_query = converted_query.replace(placeholder, f":{param_name}")
                params[param_name] = arg

            with self.engine.connect() as conn:
                result = conn.execute(text(converted_query), params)
                rows = result.fetchall()
                # Convert Rows to dicts
                return [dict(row._mapping) for row in rows]
        except Exception as e:
            logger.error(f"Error executing fetch_all query: {e}")
            raise

    async def execute(self, query: str, *args):
        """
        Execute a query without returning results (INSERT, UPDATE, DELETE)

        Args:
            query: SQL query with $1, $2, etc. placeholders
            *args: Query parameters

        Returns:
            Number of affected rows
        """
        if not self.engine:
            raise RuntimeError("PostgreSQL not initialized. Call connect() first.")

        try:
            from sqlalchemy import text

            # Convert PostgreSQL-style placeholders ($1, $2) to SQLAlchemy-style (:param1, :param2)
            converted_query = query
            params = {}
            for i, arg in enumerate(args, 1):
                placeholder = f"${i}"
                param_name = f"param{i}"
                converted_query = converted_query.replace(placeholder, f":{param_name}")
                params[param_name] = arg

            with self.engine.connect() as conn:
                result = conn.execute(text(converted_query), params)
                conn.commit()
                return result.rowcount
        except Exception as e:
            logger.error(f"Error executing query: {e}")
            raise

    def get_connection_string(self) -> str:
        """Get the PostgreSQL connection string (for OpenMemory SDK)"""
        return settings.postgres_connection_string


class Database:
    """DragonFlyDB database connection manager (Redis protocol)"""

    def __init__(self):
        self.pool: Optional[ConnectionPool] = None
        self.client: Optional[aioredis.Redis] = None

    @property
    def redis(self):
        """Alias for client to maintain compatibility with code that uses db.redis"""
        return self.client
        
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
                dragonfly_connection_attempts.inc()
                logger.info(f"Connecting to DragonflyDB at {settings.redis_host}:{settings.redis_port} (attempt {attempt}/{max_retries})")

                # TLS/SSL support (optional)
                ssl_params = {}
                if hasattr(settings, "dragonfly_ssl") and settings.dragonfly_ssl:
                    import ssl as sslmod
                    ssl_params["ssl"] = True
                    ssl_params["ssl_cert_reqs"] = sslmod.CERT_REQUIRED if getattr(settings, "dragonfly_ssl_verify", True) else sslmod.CERT_NONE
                    if getattr(settings, "dragonfly_ssl_ca_certs", None):
                        ssl_params["ssl_ca_certs"] = settings.dragonfly_ssl_ca_certs

                # Create connection pool for DragonflyDB (Redis-compatible)
                self.pool = ConnectionPool(
                    host=getattr(settings, "dragonfly_host", settings.redis_host),
                    port=getattr(settings, "dragonfly_port", settings.redis_port),
                    db=getattr(settings, "dragonfly_db", settings.redis_db),
                    password=getattr(settings, "dragonfly_password", settings.redis_password),
                    max_connections=getattr(settings, "dragonfly_max_connections", settings.redis_max_connections),
                    decode_responses=False,  # False to support binary data
                    encoding="utf-8",
                    socket_timeout=getattr(settings, "dragonfly_socket_timeout", 5),
                    socket_connect_timeout=getattr(settings, "dragonfly_socket_connect_timeout", 5),
                    **ssl_params
                )

                # Create DragonflyDB client (using Redis protocol)
                self.client = aioredis.Redis(connection_pool=self.pool)

                # Test connection
                await self.client.ping()
                logger.info("✓ Successfully connected to DragonflyDB")
                dragonfly_connection_success.inc()
                dragonfly_health_gauge.set(1)

                # Log database info
                info = await self.client.info()
                logger.info(f"DragonflyDB version: {info.get('redis_version', 'unknown')}")
                logger.info(f"Connected clients: {info.get('connected_clients', 0)}")

                return  # Success - exit retry loop

            except Exception as e:
                last_error = e
                dragonfly_connection_failures.inc()
                dragonfly_health_gauge.set(0)
                logger.warning(f"Connection attempt {attempt}/{max_retries} failed: {e}")

                if attempt < max_retries:
                    # Exponential backoff
                    wait_time = retry_delay * (2 ** (attempt - 1))
                    logger.info(f"Retrying in {wait_time:.1f} seconds...")
                    await asyncio.sleep(wait_time)
                else:
                    logger.error(f"Failed to connect to DragonflyDB after {max_retries} attempts")
                    raise ConnectionError(f"Could not connect to DragonflyDB: {last_error}") from last_error
    
    async def disconnect(self):
        """Close connection to DragonflyDB"""
        try:
            if self.client:
                await self.client.close()
                logger.info("Disconnected from DragonflyDB")

            if self.pool:
                await self.pool.disconnect()

        except Exception as e:
            logger.error(f"Error disconnecting from DragonflyDB: {e}")
    
    async def get_client(self) -> aioredis.Redis:
        """Get Redis client instance"""
        if not self.client:
            await self.connect()
        return self.client
    
    async def health_check(self) -> bool:
        """Check if database connection is healthy"""
        try:
            if not self.client:
                dragonfly_health_gauge.set(0)
                return False
            await self.client.ping()
            dragonfly_health_gauge.set(1)
            return True
        except Exception as e:
            dragonfly_health_gauge.set(0)
            logger.error(f"Database health check failed: {e}")
            return False

    # Generic Cache Operations
    async def get(self, key: str):
        """Get value from cache"""
        try:
            value = await self.client.get(key)
            if value:
                # Try to decode as JSON
                if isinstance(value, bytes):
                    value = value.decode('utf-8')
                try:
                    return json.loads(value)
                except (json.JSONDecodeError, TypeError):
                    return value
            return None
        except Exception as e:
            logger.error(f"Error getting key {key}: {e}")
            return None

    async def set(self, key: str, value, expire: int = None):
        """Set value in cache with optional expiration"""
        try:
            # Serialize value as JSON if it's a dict or list
            if isinstance(value, (dict, list)):
                from datetime import datetime
                # Handle datetime objects in serialization
                def json_serial(obj):
                    if isinstance(obj, datetime):
                        return obj.isoformat()
                    raise TypeError(f"Type {type(obj)} not serializable")

                value = json.dumps(value, default=json_serial)

            if expire:
                await self.client.setex(key, expire, value)
            else:
                await self.client.set(key, value)

            logger.debug(f"Set key {key} with expire={expire}")
        except Exception as e:
            logger.error(f"Error setting key {key}: {e}")
            raise

    async def delete(self, key: str):
        """Delete key from cache"""
        try:
            await self.client.delete(key)
            logger.debug(f"Deleted key {key}")
        except Exception as e:
            logger.error(f"Error deleting key {key}: {e}")
            raise

    async def setex(self, key: str, expire: int, value):
        """Set value with expiration time (Redis-compatible method)"""
        try:
            # Serialize value as JSON if it's a dict or list
            if isinstance(value, (dict, list)):
                from datetime import datetime
                # Handle datetime objects in serialization
                def json_serial(obj):
                    if isinstance(obj, datetime):
                        return obj.isoformat()
                    raise TypeError(f"Type {type(obj)} not serializable")

                value = json.dumps(value, default=json_serial)

            await self.client.setex(key, expire, value)
            logger.debug(f"Set key {key} with expire={expire}s")
        except Exception as e:
            logger.error(f"Error setting key {key} with expiration: {e}")
            raise

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
            # Ensure client is connected
            if not self.client:
                logger.warning(f"Database client not connected, attempting to connect...")
                await self.connect()

            key = f"npc:{npc_id}"
            state = await self.client.hgetall(key)

            if not state:
                return None

            # Decode bytes to strings and parse JSON fields
            decoded_state = {}
            json_fields = ['location', 'spawn_position', 'information_items']  # Fields that contain JSON

            for k, v in state.items():
                key_str = k.decode('utf-8') if isinstance(k, bytes) else k
                val_str = v.decode('utf-8') if isinstance(v, bytes) else v

                # Try to parse JSON fields
                if key_str in json_fields:
                    try:
                        decoded_state[key_str] = json.loads(val_str)
                    except (json.JSONDecodeError, TypeError):
                        decoded_state[key_str] = val_str
                else:
                    # Try to convert numeric fields
                    if key_str in ['level', 'movement_radius', 'x', 'y']:
                        try:
                            decoded_state[key_str] = int(val_str)
                        except (ValueError, TypeError):
                            decoded_state[key_str] = val_str
                    elif key_str in ['openness', 'conscientiousness', 'extraversion', 'agreeableness', 'neuroticism']:
                        try:
                            decoded_state[key_str] = float(val_str)
                        except (ValueError, TypeError):
                            decoded_state[key_str] = val_str
                    else:
                        decoded_state[key_str] = val_str

            return decoded_state
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

    async def create_quest(self, quest_data: dict):
        """Create a new quest (alias for store_quest for test compatibility)"""
        quest_id = quest_data.get("quest_id")
        if not quest_id:
            raise ValueError("quest_data must contain 'quest_id'")
        await self.store_quest(quest_id, quest_data)

    # Player Memory Operations
    async def get_player_memory(self, player_id: str, npc_id: str) -> list:
        """Get player-NPC interaction memory"""
        try:
            key = f"memory:player:{player_id}:npc:{npc_id}"
            memories = await self.client.lrange(key, 0, -1)

            # Decode and parse JSON memories
            result = []
            for memory in memories:
                if isinstance(memory, bytes):
                    memory = memory.decode('utf-8')
                result.append(json.loads(memory))

            logger.debug(f"Retrieved {len(result)} memories for player {player_id} and NPC {npc_id}")
            return result
        except Exception as e:
            logger.error(f"Error getting player memory for {player_id} and {npc_id}: {e}")
            return []

    async def add_player_memory(self, player_id: str, npc_id: str, memory: dict):
        """Add a player-NPC interaction memory"""
        try:
            key = f"memory:player:{player_id}:npc:{npc_id}"
            memory_json = json.dumps(memory)
            await self.client.rpush(key, memory_json)

            # Set expiration (30 days)
            await self.client.expire(key, 30 * 24 * 60 * 60)

            logger.debug(f"Added memory for player {player_id} and NPC {npc_id}")
        except Exception as e:
            logger.error(f"Error adding player memory for {player_id} and {npc_id}: {e}")
            raise

    # Rate Limiting Operations
    async def check_rate_limit(self, player_id: str, action_type: str, window_seconds: int) -> bool:
        """Check if player is rate limited for an action"""
        try:
            key = f"ratelimit:{player_id}:{action_type}"
            exists = await self.client.exists(key)

            if exists:
                ttl = await self.client.ttl(key)
                logger.debug(f"Player {player_id} is rate limited for {action_type} (TTL: {ttl}s)")
                return True

            return False
        except Exception as e:
            logger.error(f"Error checking rate limit for {player_id} and {action_type}: {e}")
            return False

    async def set_rate_limit(self, player_id: str, action_type: str, window_seconds: int):
        """Set rate limit for a player action"""
        try:
            key = f"ratelimit:{player_id}:{action_type}"
            await self.client.setex(key, window_seconds, "1")
            logger.debug(f"Set rate limit for player {player_id} and {action_type} ({window_seconds}s)")
        except Exception as e:
            logger.error(f"Error setting rate limit for {player_id} and {action_type}: {e}")
            raise

    # Conversation History Management
    async def get_conversation_history(self, conversation_key: str, max_messages: int = 20):
        """
        Get conversation history for a player-NPC interaction

        Args:
            conversation_key: Key in format "conversation:{npc_id}:{player_id}"
            max_messages: Maximum number of messages to retrieve (default: 20)

        Returns:
            List of conversation messages, or empty list if no history
        """
        try:
            history_json = await self.client.get(conversation_key)
            if history_json:
                if isinstance(history_json, bytes):
                    history_json = history_json.decode('utf-8')
                history = json.loads(history_json)
                # Return last N messages
                return history[-max_messages:] if len(history) > max_messages else history
            return []
        except Exception as e:
            logger.error(f"Error getting conversation history for {conversation_key}: {e}")
            return []

    async def save_conversation_history(self, conversation_key: str, history: list, ttl: int = 600):
        """
        Save conversation history for a player-NPC interaction

        Args:
            conversation_key: Key in format "conversation:{npc_id}:{player_id}"
            history: List of conversation messages
            ttl: Time to live in seconds (default: 600 = 10 minutes)
        """
        try:
            # Keep only last 50 messages to prevent unbounded growth
            if len(history) > 50:
                history = history[-50:]

            history_json = json.dumps(history)
            await self.client.setex(conversation_key, ttl, history_json)
            logger.debug(f"Saved conversation history for {conversation_key} ({len(history)} messages, TTL: {ttl}s)")
        except Exception as e:
            logger.error(f"Error saving conversation history for {conversation_key}: {e}")
            raise

    async def clear_conversation_history(self, conversation_key: str):
        """
        Clear conversation history for a player-NPC interaction

        Args:
            conversation_key: Key in format "conversation:{npc_id}:{player_id}"
        """
        try:
            await self.client.delete(conversation_key)
            logger.debug(f"Cleared conversation history for {conversation_key}")
        except Exception as e:
            logger.error(f"Error clearing conversation history for {conversation_key}: {e}")
            raise

# Global database instances
db = Database()  # DragonflyDB/Redis for caching and real-time state
postgres_db = PostgreSQLManager()  # PostgreSQL for persistent memory storage


# Utility functions for accessing database instances
def get_dragonfly_client():
    """Get the global DragonflyDB instance"""
    return db


def get_postgres_pool():
    """Get the global PostgreSQL manager instance"""
    return postgres_db

