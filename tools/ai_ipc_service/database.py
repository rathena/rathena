"""
Async Database Operations for AI IPC Service

This module provides an async database interface for the AI IPC service,
handling connection pooling, query execution, and all database operations
related to the ai_requests and ai_responses tables.

Features:
    - Async connection pooling with aiomysql
    - Automatic reconnection on connection loss
    - Transaction support for atomic operations
    - Row locking with SKIP LOCKED for concurrent processing
    - Parameterized queries to prevent SQL injection

Usage:
    db = Database(config.database)
    await db.connect()
    
    requests = await db.fetch_pending_requests(limit=50)
    await db.mark_processing([r['id'] for r in requests])
    await db.save_response(request_id, response_data)
    
    await db.disconnect()
"""

from __future__ import annotations

import asyncio
import json
import logging
from contextlib import asynccontextmanager
from datetime import datetime
from typing import Any, AsyncGenerator

import aiomysql
from aiomysql import Pool, Connection, DictCursor

from config import DatabaseConfig

logger = logging.getLogger(__name__)


class DatabaseError(Exception):
    """Base exception for database operations."""
    pass


class ConnectionError(DatabaseError):
    """Raised when database connection fails."""
    pass


class QueryError(DatabaseError):
    """Raised when a query fails."""
    pass


class Database:
    """
    Async database interface for AI IPC operations.
    
    Provides methods for all database operations required by the
    AI IPC service, including fetching pending requests, storing
    responses, and managing request status.
    
    Attributes:
        config: Database configuration object
        pool: aiomysql connection pool (initialized on connect)
    """
    
    def __init__(self, config: DatabaseConfig) -> None:
        """
        Initialize database interface.
        
        Args:
            config: DatabaseConfig instance with connection parameters
        """
        self.config = config
        self._pool: Pool | None = None
        self._connected = False
        self._reconnect_lock = asyncio.Lock()
    
    @property
    def is_connected(self) -> bool:
        """Check if database is connected."""
        return self._connected and self._pool is not None
    
    async def connect(self) -> None:
        """
        Establish connection pool to database.
        
        Creates an aiomysql connection pool with the configured parameters.
        
        Raises:
            ConnectionError: If connection cannot be established
        """
        if self._connected:
            logger.debug("Database already connected")
            return
        
        logger.info(
            f"Connecting to database {self.config.database}@"
            f"{self.config.host}:{self.config.port}"
        )
        
        try:
            self._pool = await aiomysql.create_pool(
                host=self.config.host,
                port=self.config.port,
                user=self.config.user,
                password=self.config.password,
                db=self.config.database,
                charset=self.config.charset,
                minsize=1,
                maxsize=self.config.pool_size,
                pool_recycle=self.config.pool_recycle,
                connect_timeout=self.config.connect_timeout,
                autocommit=True,  # We manage transactions explicitly
                cursorclass=DictCursor,
            )
            self._connected = True
            logger.info(
                f"Database connection established. Pool size: {self.config.pool_size}"
            )
        except Exception as e:
            logger.error(f"Failed to connect to database: {e}")
            raise ConnectionError(f"Database connection failed: {e}") from e
    
    async def disconnect(self) -> None:
        """
        Close all database connections.
        
        Properly closes the connection pool and releases all resources.
        """
        if self._pool is not None:
            logger.info("Closing database connection pool")
            self._pool.close()
            await self._pool.wait_closed()
            self._pool = None
            self._connected = False
            logger.info("Database connection pool closed")
    
    async def _ensure_connected(self) -> None:
        """
        Ensure database connection is available, reconnect if needed.
        
        Uses a lock to prevent multiple concurrent reconnection attempts.
        
        Raises:
            ConnectionError: If reconnection fails
        """
        if self.is_connected:
            return
        
        async with self._reconnect_lock:
            if self.is_connected:
                return
            
            logger.warning("Database connection lost. Attempting reconnect...")
            try:
                await self.connect()
            except Exception as e:
                logger.error(f"Reconnection failed: {e}")
                raise ConnectionError(f"Database reconnection failed: {e}") from e
    
    @asynccontextmanager
    async def _get_connection(self) -> AsyncGenerator[Connection, None]:
        """
        Get a connection from the pool with automatic error handling.
        
        Yields:
            Database connection
            
        Raises:
            ConnectionError: If no connection available
        """
        await self._ensure_connected()
        
        if self._pool is None:
            raise ConnectionError("Database pool not initialized")
        
        try:
            async with self._pool.acquire() as conn:
                yield conn
        except aiomysql.Error as e:
            logger.error(f"Database connection error: {e}")
            self._connected = False  # Mark as disconnected for reconnect
            raise ConnectionError(f"Database operation failed: {e}") from e
    
    @asynccontextmanager
    async def _transaction(self, conn: Connection) -> AsyncGenerator[None, None]:
        """
        Context manager for database transactions.
        
        Args:
            conn: Database connection
            
        Yields:
            None, manages transaction lifecycle
        """
        await conn.begin()
        try:
            yield
            await conn.commit()
        except Exception:
            await conn.rollback()
            raise
    
    async def fetch_and_mark_processing(
        self,
        limit: int = 50
    ) -> list[dict[str, Any]]:
        """
        Atomically fetch pending requests and mark them as processing.
        
        This method combines fetch and mark operations in a single transaction
        to prevent race conditions where requests could be stuck forever if
        the service crashes between SELECT and UPDATE.
        
        Uses SELECT ... FOR UPDATE SKIP LOCKED for safe concurrent access,
        then immediately marks selected requests as 'processing' within
        the same transaction.
        
        Args:
            limit: Maximum number of requests to fetch
            
        Returns:
            List of request dictionaries with all columns
        """
        logger.debug(f"Fetching and marking up to {limit} pending requests")
        
        # First, get IDs of pending requests we want to process
        select_ids_query = """
            SELECT id
            FROM ai_requests
            WHERE status = 'pending'
              AND (expires_at IS NULL OR expires_at > NOW())
            ORDER BY priority ASC, created_at ASC
            LIMIT %s
            FOR UPDATE SKIP LOCKED
        """
        
        # Update those requests to processing status
        update_query = """
            UPDATE ai_requests
            SET status = 'processing',
                updated_at = NOW()
            WHERE id IN ({placeholders})
              AND status = 'pending'
        """
        
        # Fetch full data for the updated requests
        fetch_query = """
            SELECT
                id,
                request_type,
                endpoint,
                request_data,
                status,
                priority,
                correlation_id,
                source_npc,
                source_map,
                created_at,
                expires_at,
                retry_count
            FROM ai_requests
            WHERE id IN ({placeholders})
        """
        
        try:
            async with self._get_connection() as conn:
                async with self._transaction(conn):
                    async with conn.cursor() as cur:
                        # Step 1: Select IDs with lock
                        await cur.execute(select_ids_query, (limit,))
                        id_rows = await cur.fetchall()
                        
                        if not id_rows:
                            logger.debug("No pending requests found")
                            return []
                        
                        request_ids = [row['id'] for row in id_rows]
                        
                        # Step 2: Mark as processing (within same transaction)
                        placeholders = ",".join(["%s"] * len(request_ids))
                        await cur.execute(
                            update_query.format(placeholders=placeholders),
                            request_ids
                        )
                        affected = cur.rowcount
                        
                        if affected == 0:
                            logger.warning(
                                "Selected requests were already taken by another worker"
                            )
                            return []
                        
                        # Step 3: Fetch full data for the marked requests
                        await cur.execute(
                            fetch_query.format(placeholders=placeholders),
                            request_ids
                        )
                        rows = await cur.fetchall()
            
            logger.debug(
                f"Atomically fetched and marked {len(rows)} requests as processing"
            )
            return list(rows) if rows else []
            
        except Exception as e:
            logger.error(f"Failed to fetch and mark pending requests: {e}")
            raise QueryError(f"Failed to fetch pending requests: {e}") from e
    
    async def fetch_pending_requests(
        self,
        limit: int = 50
    ) -> list[dict[str, Any]]:
        """
        Fetch pending requests from the queue (DEPRECATED - use fetch_and_mark_processing).
        
        This method is kept for backward compatibility but fetch_and_mark_processing
        should be used instead to avoid race conditions.
        
        Uses SELECT ... FOR UPDATE SKIP LOCKED for safe concurrent access.
        Requests are ordered by priority (ASC) and creation time (ASC).
        
        Args:
            limit: Maximum number of requests to fetch
            
        Returns:
            List of request dictionaries with all columns
        """
        logger.warning(
            "fetch_pending_requests is deprecated. "
            "Use fetch_and_mark_processing for atomic operation."
        )
        logger.debug(f"Fetching up to {limit} pending requests")
        
        query = """
            SELECT
                id,
                request_type,
                endpoint,
                request_data,
                status,
                priority,
                correlation_id,
                source_npc,
                source_map,
                created_at,
                expires_at,
                retry_count
            FROM ai_requests
            WHERE status = 'pending'
              AND (expires_at IS NULL OR expires_at > NOW())
            ORDER BY priority ASC, created_at ASC
            LIMIT %s
            FOR UPDATE SKIP LOCKED
        """
        
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(query, (limit,))
                    rows = await cur.fetchall()
                    
            logger.debug(f"Fetched {len(rows)} pending requests")
            return list(rows) if rows else []
            
        except Exception as e:
            logger.error(f"Failed to fetch pending requests: {e}")
            raise QueryError(f"Failed to fetch pending requests: {e}") from e
    
    async def mark_processing(self, request_ids: list[int]) -> int:
        """
        Mark requests as 'processing'.
        
        Updates the status of specified requests to 'processing' to prevent
        other workers from picking them up.
        
        Args:
            request_ids: List of request IDs to mark
            
        Returns:
            Number of rows updated
        """
        if not request_ids:
            return 0
        
        logger.debug(f"Marking {len(request_ids)} requests as processing")
        
        # Create placeholders for parameterized query
        placeholders = ",".join(["%s"] * len(request_ids))
        query = f"""
            UPDATE ai_requests
            SET status = 'processing',
                updated_at = NOW()
            WHERE id IN ({placeholders})
              AND status = 'pending'
        """
        
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(query, request_ids)
                    affected = cur.rowcount
                    
            logger.debug(f"Marked {affected} requests as processing")
            return affected
            
        except Exception as e:
            logger.error(f"Failed to mark requests as processing: {e}")
            raise QueryError(f"Failed to mark requests: {e}") from e
    
    async def save_response(
        self,
        request_id: int,
        response_data: str,
        http_status: int = 200,
        error_message: str | None = None,
        processing_time_ms: int = 0
    ) -> int:
        """
        Save response and mark request as completed.
        
        Performs two operations atomically:
        1. Insert response into ai_responses table
        2. Update request status to 'completed'
        
        Args:
            request_id: ID of the request being responded to
            response_data: JSON string of response data
            http_status: HTTP-like status code (200=OK, 500=Error)
            error_message: Optional error message if status != 200
            processing_time_ms: Time taken to process in milliseconds
            
        Returns:
            ID of the inserted response
        """
        logger.debug(
            f"Saving response for request {request_id} "
            f"(status={http_status}, time={processing_time_ms}ms)"
        )
        
        insert_query = """
            INSERT INTO ai_responses (
                request_id,
                response_data,
                http_status,
                error_message,
                processing_time_ms,
                created_at
            ) VALUES (%s, %s, %s, %s, %s, NOW())
        """
        
        update_query = """
            UPDATE ai_requests
            SET status = 'completed',
                updated_at = NOW()
            WHERE id = %s
        """
        
        try:
            async with self._get_connection() as conn:
                async with self._transaction(conn):
                    async with conn.cursor() as cur:
                        # Insert response
                        await cur.execute(
                            insert_query,
                            (request_id, response_data, http_status, 
                             error_message, processing_time_ms)
                        )
                        response_id = cur.lastrowid
                        
                        # Update request status
                        await cur.execute(update_query, (request_id,))
                        
            logger.info(
                f"Saved response {response_id} for request {request_id}"
            )
            return response_id
            
        except Exception as e:
            logger.error(f"Failed to save response for request {request_id}: {e}")
            raise QueryError(f"Failed to save response: {e}") from e
    
    async def mark_failed(
        self,
        request_id: int,
        error_message: str,
        should_retry: bool = False
    ) -> None:
        """
        Mark a request as failed with error details.
        
        Creates an error response and updates the request status.
        Optionally increments retry count if should_retry is True.
        
        Args:
            request_id: ID of the failed request
            error_message: Description of the failure
            should_retry: If True, mark as pending with incremented retry count
        """
        logger.warning(
            f"Marking request {request_id} as failed: {error_message}"
        )
        
        # Create error response
        error_response = json.dumps({
            "error": True,
            "message": error_message,
            "timestamp": datetime.utcnow().isoformat()
        })
        
        if should_retry:
            # Mark for retry
            update_query = """
                UPDATE ai_requests
                SET status = 'pending',
                    retry_count = retry_count + 1,
                    updated_at = NOW()
                WHERE id = %s
            """
        else:
            # Mark as permanently failed
            update_query = """
                UPDATE ai_requests
                SET status = 'failed',
                    updated_at = NOW()
                WHERE id = %s
            """
        
        insert_query = """
            INSERT INTO ai_responses (
                request_id,
                response_data,
                http_status,
                error_message,
                processing_time_ms,
                created_at
            ) VALUES (%s, %s, 500, %s, 0, NOW())
            ON DUPLICATE KEY UPDATE
                response_data = VALUES(response_data),
                http_status = VALUES(http_status),
                error_message = VALUES(error_message)
        """
        
        try:
            async with self._get_connection() as conn:
                async with self._transaction(conn):
                    async with conn.cursor() as cur:
                        # Update request status
                        await cur.execute(update_query, (request_id,))
                        
                        # Insert error response (only if not retrying)
                        if not should_retry:
                            await cur.execute(
                                insert_query,
                                (request_id, error_response, error_message)
                            )
                        
            action = "queued for retry" if should_retry else "marked as failed"
            logger.info(f"Request {request_id} {action}")
            
        except Exception as e:
            logger.error(f"Failed to mark request {request_id} as failed: {e}")
            raise QueryError(f"Failed to mark request as failed: {e}") from e
    
    async def cleanup_expired(self) -> int:
        """
        Mark expired pending requests as 'timeout'.
        
        Finds all pending requests where expires_at < NOW() and
        updates their status to 'timeout'.
        
        Returns:
            Number of requests marked as expired
        """
        logger.debug("Cleaning up expired requests")
        
        query = """
            UPDATE ai_requests
            SET status = 'timeout',
                updated_at = NOW()
            WHERE status = 'pending'
              AND expires_at IS NOT NULL
              AND expires_at < NOW()
        """
        
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(query)
                    affected = cur.rowcount
                    
            if affected > 0:
                logger.info(f"Marked {affected} expired requests as timeout")
            return affected
            
        except Exception as e:
            logger.error(f"Failed to cleanup expired requests: {e}")
            raise QueryError(f"Failed to cleanup expired requests: {e}") from e
    
    async def cleanup_stuck_processing(self, stuck_threshold_seconds: int = 300) -> int:
        """
        Clean up requests stuck in 'processing' state.
        
        Requests can get stuck in 'processing' if the worker crashes after
        marking them but before completing. This method marks such requests
        back to 'pending' for retry, or 'failed' if max retries exceeded.
        
        Args:
            stuck_threshold_seconds: How long a request can be in 'processing'
                                    before considered stuck (default: 5 minutes)
        
        Returns:
            Number of requests cleaned up
        """
        logger.debug(
            f"Cleaning up requests stuck in processing for >{stuck_threshold_seconds}s"
        )
        
        # Mark stuck requests back to pending (if retries available)
        reset_query = """
            UPDATE ai_requests
            SET status = 'pending',
                retry_count = retry_count + 1,
                updated_at = NOW()
            WHERE status = 'processing'
              AND updated_at < DATE_SUB(NOW(), INTERVAL %s SECOND)
              AND retry_count < %s
        """
        
        # Mark stuck requests as failed (if retries exhausted)
        fail_query = """
            UPDATE ai_requests
            SET status = 'failed',
                updated_at = NOW()
            WHERE status = 'processing'
              AND updated_at < DATE_SUB(NOW(), INTERVAL %s SECOND)
              AND retry_count >= %s
        """
        
        max_retries = 3  # Should come from config
        
        try:
            total_affected = 0
            async with self._get_connection() as conn:
                async with self._transaction(conn):
                    async with conn.cursor() as cur:
                        # Reset retryable requests
                        await cur.execute(
                            reset_query,
                            (stuck_threshold_seconds, max_retries)
                        )
                        reset_count = cur.rowcount
                        
                        # Fail non-retryable requests
                        await cur.execute(
                            fail_query,
                            (stuck_threshold_seconds, max_retries)
                        )
                        fail_count = cur.rowcount
                        
                        total_affected = reset_count + fail_count
                    
            if total_affected > 0:
                logger.info(
                    f"Cleaned up {total_affected} stuck requests "
                    f"({reset_count} reset, {fail_count} failed)"
                )
            return total_affected
            
        except Exception as e:
            logger.error(f"Failed to cleanup stuck requests: {e}")
            raise QueryError(f"Failed to cleanup stuck requests: {e}") from e
    
    async def get_request_status(self, request_id: int) -> str | None:
        """
        Get the current status of a request.
        
        Args:
            request_id: ID of the request
            
        Returns:
            Status string or None if request not found
        """
        query = "SELECT status FROM ai_requests WHERE id = %s"
        
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(query, (request_id,))
                    row = await cur.fetchone()
                    
            return row['status'] if row else None
            
        except Exception as e:
            logger.error(f"Failed to get status for request {request_id}: {e}")
            raise QueryError(f"Failed to get request status: {e}") from e
    
    async def get_pending_count(self) -> int:
        """
        Get the count of pending requests in the queue.
        
        Useful for monitoring and rate limiting.
        
        Returns:
            Number of pending requests
        """
        query = "SELECT COUNT(*) as count FROM ai_requests WHERE status = 'pending'"
        
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    await cur.execute(query)
                    row = await cur.fetchone()
                    
            return row['count'] if row else 0
            
        except Exception as e:
            logger.error(f"Failed to get pending count: {e}")
            return 0  # Return 0 on error to avoid blocking
    
    async def health_check(self) -> dict[str, Any]:
        """
        Perform a database health check.
        
        Verifies connectivity and returns status information.
        
        Returns:
            Dictionary with health status and metrics
        """
        try:
            async with self._get_connection() as conn:
                async with conn.cursor() as cur:
                    # Simple connectivity test
                    await cur.execute("SELECT 1 as alive")
                    await cur.fetchone()
                    
                    # Get pending request count
                    await cur.execute(
                        "SELECT COUNT(*) as count FROM ai_requests "
                        "WHERE status = 'pending'"
                    )
                    pending = (await cur.fetchone())['count']
                    
                    # Get processing request count
                    await cur.execute(
                        "SELECT COUNT(*) as count FROM ai_requests "
                        "WHERE status = 'processing'"
                    )
                    processing = (await cur.fetchone())['count']
                    
            return {
                "status": "healthy",
                "connected": True,
                "pending_requests": pending,
                "processing_requests": processing,
                "pool_size": self.config.pool_size,
            }
            
        except Exception as e:
            logger.error(f"Database health check failed: {e}")
            return {
                "status": "unhealthy",
                "connected": False,
                "error": str(e),
            }


# =============================================================================
# Backward Compatibility Alias
# =============================================================================

# Alias for backward compatibility with tests expecting DatabaseManager name
DatabaseManager = Database


# =============================================================================
# Test Compatibility Methods (Added to Database class)
# =============================================================================

# Extend Database class with test compatibility methods
async def _create_request_compat(
    self,
    request_type: str,
    npc_id: int,
    player_id: int,
    payload: str,
    priority: int = 5,
) -> int:
    """
    Create a new AI request (test compatibility method).
    
    Args:
        request_type: Type of request
        npc_id: NPC identifier
        player_id: Player identifier (stored in request_data)
        payload: JSON payload string
        priority: Request priority
        
    Returns:
        ID of created request
    """
    # Include player_id in the request_data JSON since database doesn't have that column
    try:
        if isinstance(payload, str):
            payload_data = json.loads(payload)
        else:
            payload_data = payload
    except (json.JSONDecodeError, TypeError):
        payload_data = {"raw_payload": payload}
    
    # Add player_id to payload for test compatibility
    payload_data["player_id"] = player_id
    payload_data["npc_id"] = npc_id
    
    # Determine endpoint from request_type
    endpoint_map = {
        'dialogue': '/dialogue',
        'decision': '/decision',
        'emotion': '/emotion',
        'memory': '/memory',
    }
    endpoint = endpoint_map.get(request_type, f'/{request_type}')
    
    query = """
        INSERT INTO ai_requests
        (request_type, endpoint, source_npc, request_data, priority, status, created_at, expires_at)
        VALUES (%s, %s, %s, %s, %s, 'pending', NOW(), DATE_ADD(NOW(), INTERVAL 5 MINUTE))
    """
    async with self._get_connection() as conn:
        async with conn.cursor() as cursor:
            await cursor.execute(
                query,
                (request_type, endpoint, str(npc_id), json.dumps(payload_data), priority)
            )
            await conn.commit()
            return cursor.lastrowid


async def _get_request_by_id_compat(self, request_id: int) -> dict[str, Any] | None:
    """
    Get request by ID (test compatibility method).
    
    Args:
        request_id: Request ID
        
    Returns:
        Request data or None (with compatibility aliases)
    """
    query = "SELECT * FROM ai_requests WHERE id = %s"
    async with self._get_connection() as conn:
        async with conn.cursor(aiomysql.DictCursor) as cursor:
            await cursor.execute(query, (request_id,))
            row = await cursor.fetchone()
            if row:
                # Add compatibility aliases for test expectations
                row["payload"] = row.get("request_data", "{}")
                # Convert source_npc to int for npc_id
                source_npc = row.get("source_npc")
                if source_npc is not None:
                    try:
                        row["npc_id"] = int(source_npc)
                    except (ValueError, TypeError):
                        row["npc_id"] = source_npc
                # Parse request_data to extract player_id if present
                try:
                    data = json.loads(row.get("request_data", "{}"))
                    if "player_id" in data:
                        row["player_id"] = data["player_id"]
                    if "npc_id" in data and not row.get("npc_id"):
                        try:
                            row["npc_id"] = int(data["npc_id"])
                        except (ValueError, TypeError):
                            row["npc_id"] = data["npc_id"]
                except (json.JSONDecodeError, TypeError):
                    pass
            return row


async def _update_request_status_compat(self, request_id: int, status: str) -> None:
    """
    Update request status (test compatibility method).
    
    Args:
        request_id: Request ID
        status: New status
    """
    query = "UPDATE ai_requests SET status = %s WHERE id = %s"
    async with self._get_connection() as conn:
        async with conn.cursor() as cursor:
            await cursor.execute(query, (status, request_id))
            await conn.commit()


async def _get_pending_requests_compat(self, limit: int = 10) -> list[dict[str, Any]]:
    """
    Get pending requests (test compatibility method).
    
    Args:
        limit: Maximum requests to return
        
    Returns:
        List of pending requests
    """
    query = """
        SELECT * FROM ai_requests
        WHERE status = 'pending'
        ORDER BY priority DESC, created_at ASC
        LIMIT %s
    """
    async with self._get_connection() as conn:
        async with conn.cursor(aiomysql.DictCursor) as cursor:
            await cursor.execute(query, (limit,))
            return await cursor.fetchall()


async def _store_response_compat(
    self,
    request_id: int,
    status_code: int,
    response_data: str,
) -> int:
    """
    Store response (test compatibility method).
    
    Args:
        request_id: Request ID
        status_code: HTTP status code
        response_data: JSON response data
        
    Returns:
        Response ID
    """
    # First update request status
    await _update_request_status_compat(self, request_id, "completed")
    
    # Use INSERT ... ON DUPLICATE KEY UPDATE to handle re-inserts
    query = """
        INSERT INTO ai_responses
        (request_id, response_data, http_status, processing_time_ms, created_at)
        VALUES (%s, %s, %s, 0, NOW())
        ON DUPLICATE KEY UPDATE
            response_data = VALUES(response_data),
            http_status = VALUES(http_status),
            processing_time_ms = VALUES(processing_time_ms)
    """
    async with self._get_connection() as conn:
        async with conn.cursor() as cursor:
            await cursor.execute(query, (request_id, response_data, status_code))
            await conn.commit()
            return cursor.lastrowid if cursor.lastrowid else request_id


async def _cancel_request_compat(self, request_id: int) -> bool:
    """
    Cancel a request (test compatibility method).
    
    Args:
        request_id: Request ID to cancel
        
    Returns:
        True if cancelled, False otherwise
    """
    query = """
        UPDATE ai_requests
        SET status = 'cancelled', updated_at = NOW()
        WHERE id = %s AND status IN ('pending', 'processing')
    """
    async with self._get_connection() as conn:
        async with conn.cursor() as cursor:
            await cursor.execute(query, (request_id,))
            await conn.commit()
            return cursor.rowcount > 0


async def _get_response_compat(self, request_id: int) -> dict[str, Any] | None:
    """
    Get response by request ID (test compatibility method).
    
    Args:
        request_id: Request ID
        
    Returns:
        Response data or None (with status_code alias for http_status)
    """
    query = "SELECT * FROM ai_responses WHERE request_id = %s"
    async with self._get_connection() as conn:
        async with conn.cursor(aiomysql.DictCursor) as cursor:
            await cursor.execute(query, (request_id,))
            row = await cursor.fetchone()
            if row:
                # Add status_code alias for test compatibility
                row["status_code"] = row.get("http_status", 200)
            return row


async def _cleanup_expired_requests_compat(
    self,
    timeout_seconds: int = 300,
) -> int:
    """
    Cleanup expired requests (test compatibility method).
    
    Args:
        timeout_seconds: Timeout threshold in seconds
        
    Returns:
        Number of requests cleaned up
    """
    # Use the existing cleanup methods
    expired = await self.cleanup_expired()
    stuck = await self.cleanup_stuck_processing(timeout_seconds)
    return expired + stuck


async def _get_request_log_compat(self, request_id: int) -> dict[str, Any] | None:
    """
    Get request log (test compatibility method).
    
    Args:
        request_id: Request ID
        
    Returns:
        Log data or None (returns request data as log since no separate log table)
    """
    # Since there's no separate log table, return request data as log
    request = await _get_request_by_id_compat(self, request_id)
    if request:
        return {
            "request_id": request.get("id"),
            "request_type": request.get("request_type"),
            "status": request.get("status"),
            "created_at": request.get("created_at"),
            "updated_at": request.get("updated_at"),
        }
    return None


async def _run_cleanup_procedure_compat(
    self,
    retention_days: int = 30,
) -> dict[str, Any]:
    """
    Run cleanup procedure (test compatibility method).
    
    Args:
        retention_days: Data retention period in days
        
    Returns:
        Cleanup results
    """
    # Run cleanup operations
    expired = await self.cleanup_expired()
    stuck = await self.cleanup_stuck_processing()
    
    return {
        "expired_cleaned": expired,
        "stuck_cleaned": stuck,
        "retention_days": retention_days,
    }


async def _get_stats_compat(self) -> dict[str, Any]:
    """
    Get database statistics (test compatibility method).
    
    Returns:
        Statistics dictionary
    """
    query_total = "SELECT COUNT(*) as count FROM ai_requests"
    query_by_status = """
        SELECT status, COUNT(*) as count
        FROM ai_requests
        GROUP BY status
    """
    
    async with self._get_connection() as conn:
        async with conn.cursor(aiomysql.DictCursor) as cursor:
            await cursor.execute(query_total)
            total_row = await cursor.fetchone()
            total = total_row["count"] if total_row else 0
            
            await cursor.execute(query_by_status)
            status_rows = await cursor.fetchall()
            
    by_status = {row["status"]: row["count"] for row in status_rows}
    
    return {
        "total_requests": total,
        "processed": by_status.get("completed", 0),
        "pending": by_status.get("pending", 0),
        "failed": by_status.get("failed", 0),
        "by_status": by_status,
    }


@asynccontextmanager
async def _transaction_compat(self):
    """
    Transaction context manager (test compatibility method).
    
    Yields:
        Database connection with active transaction
    """
    async with self._get_connection() as conn:
        await conn.begin()
        try:
            yield conn
            await conn.commit()
        except Exception:
            await conn.rollback()
            raise


async def _create_request_with_conn_compat(
    self,
    conn,
    request_type: str,
    npc_id: int,
    player_id: int,
    payload: str,
    priority: int = 5,
) -> int:
    """
    Create request using existing connection (test compatibility method).
    
    Args:
        conn: Database connection
        request_type: Type of request
        npc_id: NPC identifier
        player_id: Player identifier
        payload: JSON payload
        priority: Request priority
        
    Returns:
        Created request ID
    """
    # Parse and enrich payload
    try:
        if isinstance(payload, str):
            payload_data = json.loads(payload)
        else:
            payload_data = payload
    except (json.JSONDecodeError, TypeError):
        payload_data = {"raw_payload": payload}
    
    payload_data["player_id"] = player_id
    payload_data["npc_id"] = npc_id
    
    endpoint_map = {
        'dialogue': '/dialogue',
        'decision': '/decision',
        'emotion': '/emotion',
        'memory': '/memory',
    }
    endpoint = endpoint_map.get(request_type, f'/{request_type}')
    
    query = """
        INSERT INTO ai_requests
        (request_type, endpoint, source_npc, request_data, priority, status, created_at, expires_at)
        VALUES (%s, %s, %s, %s, %s, 'pending', NOW(), DATE_ADD(NOW(), INTERVAL 5 MINUTE))
    """
    
    async with conn.cursor() as cursor:
        await cursor.execute(
            query,
            (request_type, endpoint, str(npc_id), json.dumps(payload_data), priority)
        )
        return cursor.lastrowid


@property
def _pool_size_compat(self) -> int:
    """Pool size property (test compatibility)."""
    return self.config.pool_size


# Bind compatibility methods to Database class
Database.create_request = _create_request_compat
Database.get_request_by_id = _get_request_by_id_compat
Database.update_request_status = _update_request_status_compat
Database.get_pending_requests = _get_pending_requests_compat
Database.store_response = _store_response_compat
Database.get_response = _get_response_compat
Database.cancel_request = _cancel_request_compat
Database.cleanup_expired_requests = _cleanup_expired_requests_compat
Database.get_request_log = _get_request_log_compat
Database.run_cleanup_procedure = _run_cleanup_procedure_compat
Database.get_stats = _get_stats_compat
Database.transaction = _transaction_compat
Database.create_request_with_conn = _create_request_with_conn_compat
Database.pool_size = _pool_size_compat