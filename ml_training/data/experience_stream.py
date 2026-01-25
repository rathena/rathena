"""
Experience Stream - Memory-Efficient Database Streaming
Streams experiences from PostgreSQL without loading all into RAM
Uses server-side cursors for efficient data access
"""

import asyncpg
import numpy as np
from typing import AsyncGenerator, List, Dict, Any, Optional
from datetime import datetime, timedelta
import logging
from dataclasses import dataclass

logger = logging.getLogger(__name__)


@dataclass
class Experience:
    """Single experience tuple (s, a, r, s', done)"""
    experience_id: int
    monster_id: int
    mob_id: int
    archetype: str
    episode_id: int
    state_vector: np.ndarray
    action_type: str
    action_id: int
    reward: float
    next_state_vector: Optional[np.ndarray]
    done: bool
    td_error: Optional[float]
    priority: float
    timestamp: datetime
    
    # Additional context
    map_id: int
    position_x: int
    position_y: int
    pack_id: Optional[int]
    pack_size: int
    sample_count: int


class ExperienceStream:
    """
    Stream experiences from PostgreSQL without loading all into RAM
    
    Features:
    - Server-side cursor for memory efficiency
    - Filter by archetype, priority, timestamp
    - Batch yielding (256 experiences at a time)
    - Track processing progress
    - Priority-based sampling
    """
    
    def __init__(
        self,
        db_pool: asyncpg.Pool,
        table_name: str = "ml_experience_replay"
    ):
        """
        Initialize experience stream
        
        Args:
            db_pool: asyncpg connection pool
            table_name: Name of experience replay table
        """
        self.db_pool = db_pool
        self.table_name = table_name
        self.logger = logging.getLogger(__name__)
        
        # Tracking
        self.total_streamed = 0
        self.last_processed_timestamp = None
    
    async def stream_new_experiences(
        self,
        since_timestamp: datetime,
        batch_size: int = 256,
        archetype: Optional[str] = None,
        min_priority: Optional[float] = None,
        max_experiences: Optional[int] = None
    ) -> AsyncGenerator[List[Experience], None]:
        """
        Stream new experiences since timestamp
        
        Uses server-side cursor for memory efficiency - doesn't load all data at once
        
        Args:
            since_timestamp: Only get experiences after this time
            batch_size: Number of experiences per batch
            archetype: Filter by archetype (optional)
            min_priority: Minimum priority threshold (optional)
            max_experiences: Maximum total experiences to stream (optional)
        
        Yields:
            Batches of experiences
        """
        # Build query
        conditions = ["timestamp > $1"]
        params = [since_timestamp]
        param_idx = 2
        
        if archetype:
            conditions.append(f"archetype = ${param_idx}")
            params.append(archetype)
            param_idx += 1
        
        if min_priority is not None:
            conditions.append(f"priority >= ${param_idx}")
            params.append(min_priority)
            param_idx += 1
        
        where_clause = " AND ".join(conditions)
        
        query = f"""
        SELECT 
            experience_id,
            monster_id,
            mob_id,
            archetype,
            episode_id,
            state_vector,
            action_type,
            action_id,
            reward,
            next_state_vector,
            done,
            td_error,
            priority,
            timestamp,
            map_id,
            position_x,
            position_y,
            pack_id,
            pack_size,
            sample_count
        FROM {self.table_name}
        WHERE {where_clause}
        ORDER BY timestamp ASC
        """
        
        if max_experiences:
            query += f" LIMIT {max_experiences}"
        
        self.logger.info(
            f"Starting experience stream: since={since_timestamp}, "
            f"archetype={archetype}, min_priority={min_priority}, "
            f"batch_size={batch_size}"
        )
        
        # Use connection transaction with server-side cursor
        async with self.db_pool.acquire() as conn:
            async with conn.transaction():
                # Create server-side cursor (doesn't load all into memory)
                cursor = await conn.cursor(query, *params)
                
                experiences_streamed = 0
                batch = []
                
                async for row in cursor:
                    # Parse experience from row
                    experience = self._row_to_experience(row)
                    batch.append(experience)
                    experiences_streamed += 1
                    
                    # Yield when batch is full
                    if len(batch) >= batch_size:
                        self.logger.debug(f"Yielding batch of {len(batch)} experiences")
                        yield batch
                        batch = []
                        self.total_streamed += experiences_streamed
                        self.last_processed_timestamp = experience.timestamp
                
                # Yield remaining experiences
                if batch:
                    self.logger.debug(f"Yielding final batch of {len(batch)} experiences")
                    yield batch
                    self.total_streamed += len(batch)
                    if batch:
                        self.last_processed_timestamp = batch[-1].timestamp
        
        self.logger.info(
            f"Stream complete: {experiences_streamed} experiences streamed"
        )
    
    async def stream_priority_experiences(
        self,
        archetype: str,
        batch_size: int = 256,
        priority_threshold: float = 1.0,
        lookback_days: int = 7
    ) -> AsyncGenerator[List[Experience], None]:
        """
        Stream high-priority experiences for training
        
        Args:
            archetype: Monster archetype
            batch_size: Batch size
            priority_threshold: Minimum priority
            lookback_days: Only consider recent experiences
        
        Yields:
            Batches of high-priority experiences
        """
        since_timestamp = datetime.utcnow() - timedelta(days=lookback_days)
        
        async for batch in self.stream_new_experiences(
            since_timestamp=since_timestamp,
            batch_size=batch_size,
            archetype=archetype,
            min_priority=priority_threshold
        ):
            yield batch
    
    async def get_experience_count(
        self,
        since_timestamp: datetime,
        archetype: Optional[str] = None
    ) -> int:
        """
        Get count of new experiences since timestamp
        
        Args:
            since_timestamp: Start time
            archetype: Filter by archetype (optional)
        
        Returns:
            Count of experiences
        """
        conditions = ["timestamp > $1"]
        params = [since_timestamp]
        
        if archetype:
            conditions.append("archetype = $2")
            params.append(archetype)
        
        where_clause = " AND ".join(conditions)
        
        query = f"""
        SELECT COUNT(*) as count
        FROM {self.table_name}
        WHERE {where_clause}
        """
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(query, *params)
            return row['count']
    
    async def mark_experiences_processed(
        self,
        experience_ids: List[int]
    ) -> int:
        """
        Mark experiences as processed (increment sample_count)
        
        Args:
            experience_ids: List of experience IDs
        
        Returns:
            Number of experiences updated
        """
        if not experience_ids:
            return 0
        
        query = f"""
        UPDATE {self.table_name}
        SET sample_count = sample_count + 1
        WHERE experience_id = ANY($1::BIGINT[])
        """
        
        async with self.db_pool.acquire() as conn:
            result = await conn.execute(query, experience_ids)
            
            # Parse result (format: "UPDATE N")
            count = int(result.split()[-1]) if result else 0
            
            self.logger.debug(f"Marked {count} experiences as processed")
            return count
    
    async def get_archetype_distribution(
        self,
        lookback_days: int = 7
    ) -> Dict[str, int]:
        """
        Get distribution of experiences by archetype
        
        Args:
            lookback_days: Number of days to look back
        
        Returns:
            Dictionary of archetype counts
        """
        since = datetime.utcnow() - timedelta(days=lookback_days)
        
        query = f"""
        SELECT 
            archetype,
            COUNT(*) as count
        FROM {self.table_name}
        WHERE timestamp > $1
        GROUP BY archetype
        ORDER BY count DESC
        """
        
        async with self.db_pool.acquire() as conn:
            rows = await conn.fetch(query, since)
            
            distribution = {row['archetype']: row['count'] for row in rows}
            
            self.logger.info(f"Archetype distribution (last {lookback_days} days): {distribution}")
            return distribution
    
    async def get_priority_experiences(
        self,
        archetype: str,
        priority_threshold: float,
        limit: int = 1000
    ) -> List[Experience]:
        """
        Get high-priority experiences (for important events)
        
        Args:
            archetype: Monster archetype
            priority_threshold: Minimum priority
            limit: Maximum experiences to return
        
        Returns:
            List of high-priority experiences
        """
        query = f"""
        SELECT 
            experience_id,
            monster_id,
            mob_id,
            archetype,
            episode_id,
            state_vector,
            action_type,
            action_id,
            reward,
            next_state_vector,
            done,
            td_error,
            priority,
            timestamp,
            map_id,
            position_x,
            position_y,
            pack_id,
            pack_size,
            sample_count
        FROM {self.table_name}
        WHERE archetype = $1
            AND priority >= $2
            AND timestamp > NOW() - INTERVAL '7 days'
        ORDER BY priority DESC, timestamp DESC
        LIMIT $3
        """
        
        async with self.db_pool.acquire() as conn:
            rows = await conn.fetch(query, archetype, priority_threshold, limit)
            
            experiences = [self._row_to_experience(row) for row in rows]
            
            self.logger.info(
                f"Retrieved {len(experiences)} high-priority experiences "
                f"for {archetype} (threshold={priority_threshold})"
            )
            
            return experiences
    
    async def get_balanced_sample(
        self,
        archetypes: List[str],
        samples_per_archetype: int,
        lookback_days: int = 7
    ) -> Dict[str, List[Experience]]:
        """
        Get balanced sample across archetypes
        
        Ensures equal representation for each archetype
        
        Args:
            archetypes: List of archetypes to sample
            samples_per_archetype: Number of samples per archetype
            lookback_days: Look back window
        
        Returns:
            Dictionary mapping archetype to experiences
        """
        since = datetime.utcnow() - timedelta(days=lookback_days)
        
        samples = {}
        
        for archetype in archetypes:
            query = f"""
            SELECT 
                experience_id,
                monster_id,
                mob_id,
                archetype,
                episode_id,
                state_vector,
                action_type,
                action_id,
                reward,
                next_state_vector,
                done,
                td_error,
                priority,
                timestamp,
                map_id,
                position_x,
                position_y,
                pack_id,
                pack_size,
                sample_count
            FROM {self.table_name}
            WHERE archetype = $1
                AND timestamp > $2
            ORDER BY RANDOM() * priority DESC
            LIMIT $3
            """
            
            async with self.db_pool.acquire() as conn:
                rows = await conn.fetch(query, archetype, since, samples_per_archetype)
                samples[archetype] = [self._row_to_experience(row) for row in rows]
        
        total = sum(len(exps) for exps in samples.values())
        self.logger.info(
            f"Balanced sample: {total} experiences across {len(archetypes)} archetypes "
            f"({samples_per_archetype} per archetype)"
        )
        
        return samples
    
    async def update_priorities(
        self,
        experience_ids: List[int],
        td_errors: List[float]
    ) -> int:
        """
        Update priorities based on TD errors
        
        Args:
            experience_ids: List of experience IDs
            td_errors: Corresponding TD errors
        
        Returns:
            Number of updated experiences
        """
        if not experience_ids or not td_errors:
            return 0
        
        if len(experience_ids) != len(td_errors):
            raise ValueError(
                f"Mismatch: {len(experience_ids)} experience_ids but {len(td_errors)} td_errors"
            )
        
        # Use PostgreSQL function for batch update
        query = "SELECT update_experience_priorities($1::BIGINT[], $2::REAL[])"
        
        async with self.db_pool.acquire() as conn:
            await conn.execute(query, experience_ids, td_errors)
        
        self.logger.debug(f"Updated priorities for {len(experience_ids)} experiences")
        return len(experience_ids)
    
    async def get_latest_timestamp(self, archetype: Optional[str] = None) -> Optional[datetime]:
        """
        Get timestamp of most recent experience
        
        Args:
            archetype: Filter by archetype (optional)
        
        Returns:
            Latest timestamp or None
        """
        if archetype:
            query = f"""
            SELECT MAX(timestamp) as max_timestamp
            FROM {self.table_name}
            WHERE archetype = $1
            """
            params = [archetype]
        else:
            query = f"""
            SELECT MAX(timestamp) as max_timestamp
            FROM {self.table_name}
            """
            params = []
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(query, *params)
            return row['max_timestamp'] if row else None
    
    async def get_statistics(self) -> Dict[str, Any]:
        """
        Get streaming statistics
        
        Returns:
            Statistics dictionary
        """
        query = f"""
        SELECT 
            COUNT(*) as total_experiences,
            COUNT(DISTINCT archetype) as archetypes,
            COUNT(DISTINCT monster_id) as unique_monsters,
            COUNT(DISTINCT episode_id) as total_episodes,
            MIN(timestamp) as oldest_experience,
            MAX(timestamp) as newest_experience,
            AVG(priority) as avg_priority,
            AVG(reward) as avg_reward,
            SUM(CASE WHEN done THEN 1 ELSE 0 END) as terminal_states
        FROM {self.table_name}
        WHERE timestamp > NOW() - INTERVAL '7 days'
        """
        
        async with self.db_pool.acquire() as conn:
            row = await conn.fetchrow(query)
            
            stats = {
                'total_experiences': row['total_experiences'],
                'archetypes': row['archetypes'],
                'unique_monsters': row['unique_monsters'],
                'total_episodes': row['total_episodes'],
                'oldest_experience': row['oldest_experience'],
                'newest_experience': row['newest_experience'],
                'avg_priority': float(row['avg_priority']) if row['avg_priority'] else 0.0,
                'avg_reward': float(row['avg_reward']) if row['avg_reward'] else 0.0,
                'terminal_states': row['terminal_states'],
                'total_streamed': self.total_streamed,
                'last_processed_timestamp': self.last_processed_timestamp
            }
            
            return stats
    
    def _row_to_experience(self, row: asyncpg.Record) -> Experience:
        """
        Convert database row to Experience object
        
        Args:
            row: Database row
        
        Returns:
            Experience object
        """
        return Experience(
            experience_id=row['experience_id'],
            monster_id=row['monster_id'],
            mob_id=row['mob_id'],
            archetype=row['archetype'],
            episode_id=row['episode_id'],
            state_vector=np.array(row['state_vector'], dtype=np.float32),
            action_type=row['action_type'],
            action_id=row['action_id'],
            reward=float(row['reward']),
            next_state_vector=np.array(row['next_state_vector'], dtype=np.float32) if row['next_state_vector'] else None,
            done=row['done'],
            td_error=float(row['td_error']) if row['td_error'] else None,
            priority=float(row['priority']),
            timestamp=row['timestamp'],
            map_id=row['map_id'],
            position_x=row['position_x'],
            position_y=row['position_y'],
            pack_id=row['pack_id'],
            pack_size=row['pack_size'],
            sample_count=row['sample_count']
        )


class PrioritizedExperienceStream(ExperienceStream):
    """
    Experience stream with prioritized sampling
    
    Samples experiences based on priority (TD error)
    Higher priority experiences are sampled more frequently
    """
    
    async def sample_weighted_batch(
        self,
        archetype: str,
        batch_size: int = 256,
        beta: float = 0.4,
        lookback_days: int = 7
    ) -> Tuple[List[Experience], np.ndarray]:
        """
        Sample batch using priority-weighted sampling
        
        Args:
            archetype: Monster archetype
            batch_size: Number of experiences to sample
            beta: Importance sampling exponent
            lookback_days: Look back window
        
        Returns:
            Tuple of (experiences, importance_weights)
        """
        since = datetime.utcnow() - timedelta(days=lookback_days)
        
        # Use PostgreSQL prioritized sampling function
        query = """
        SELECT * FROM sample_prioritized_experience($1, $2)
        WHERE timestamp > $3
        """
        
        async with self.db_pool.acquire() as conn:
            rows = await conn.fetch(query, archetype, batch_size, since)
            
            if not rows:
                self.logger.warning(f"No experiences found for {archetype}")
                return [], np.array([])
            
            experiences = []
            weights = []
            
            for row in rows:
                # Note: sample_prioritized_experience returns different schema
                exp = Experience(
                    experience_id=row['experience_id'],
                    monster_id=0,  # Not returned by function
                    mob_id=0,
                    archetype=archetype,
                    episode_id=0,
                    state_vector=np.array(row['state_vector'], dtype=np.float32),
                    action_type=row['action_type'],
                    action_id=row['action_id'],
                    reward=float(row['reward']),
                    next_state_vector=np.array(row['next_state_vector'], dtype=np.float32) if row['next_state_vector'] else None,
                    done=row['done'],
                    td_error=None,
                    priority=float(row['priority']),
                    timestamp=datetime.utcnow(),
                    map_id=0,
                    position_x=0,
                    position_y=0,
                    pack_id=None,
                    pack_size=1,
                    sample_count=0
                )
                experiences.append(exp)
                weights.append(float(row['weight']))
            
            importance_weights = np.array(weights, dtype=np.float32)
            
            self.logger.debug(
                f"Sampled {len(experiences)} prioritized experiences for {archetype}"
            )
            
            return experiences, importance_weights
    
    async def sample_uniform_batch(
        self,
        archetype: str,
        batch_size: int = 256,
        lookback_days: int = 7
    ) -> List[Experience]:
        """
        Sample batch uniformly (no priority weighting)
        
        Args:
            archetype: Monster archetype
            batch_size: Number of experiences
            lookback_days: Look back window
        
        Returns:
            List of experiences
        """
        since = datetime.utcnow() - timedelta(days=lookback_days)
        
        query = f"""
        SELECT 
            experience_id,
            monster_id,
            mob_id,
            archetype,
            episode_id,
            state_vector,
            action_type,
            action_id,
            reward,
            next_state_vector,
            done,
            td_error,
            priority,
            timestamp,
            map_id,
            position_x,
            position_y,
            pack_id,
            pack_size,
            sample_count
        FROM {self.table_name}
        WHERE archetype = $1
            AND timestamp > $2
        ORDER BY RANDOM()
        LIMIT $3
        """
        
        async with self.db_pool.acquire() as conn:
            rows = await conn.fetch(query, archetype, since, batch_size)
            experiences = [self._row_to_experience(row) for row in rows]
            
            self.logger.debug(
                f"Sampled {len(experiences)} uniform experiences for {archetype}"
            )
            
            return experiences


class ExperienceBuffer:
    """
    In-memory buffer for batching experiences before training
    
    Used to accumulate experiences from stream before feeding to trainer
    """
    
    def __init__(self, max_size: int = 10000):
        """
        Initialize buffer
        
        Args:
            max_size: Maximum buffer size
        """
        self.max_size = max_size
        self.buffer: List[Experience] = []
        self.logger = logging.getLogger(__name__)
    
    def add(self, experience: Experience):
        """Add experience to buffer"""
        self.buffer.append(experience)
        
        # Remove oldest if over capacity
        if len(self.buffer) > self.max_size:
            self.buffer.pop(0)
    
    def add_batch(self, experiences: List[Experience]):
        """Add batch of experiences"""
        for exp in experiences:
            self.add(exp)
    
    def sample(self, batch_size: int) -> List[Experience]:
        """
        Sample random batch from buffer
        
        Args:
            batch_size: Number of experiences to sample
        
        Returns:
            List of experiences
        """
        if len(self.buffer) < batch_size:
            return self.buffer.copy()
        
        indices = np.random.choice(len(self.buffer), batch_size, replace=False)
        return [self.buffer[i] for i in indices]
    
    def get_all(self) -> List[Experience]:
        """Get all experiences in buffer"""
        return self.buffer.copy()
    
    def clear(self):
        """Clear buffer"""
        self.buffer.clear()
    
    def size(self) -> int:
        """Get current buffer size"""
        return len(self.buffer)
    
    def is_full(self) -> bool:
        """Check if buffer is at capacity"""
        return len(self.buffer) >= self.max_size
    
    def to_numpy_arrays(self) -> Dict[str, np.ndarray]:
        """
        Convert buffer to numpy arrays for training
        
        Returns:
            Dictionary of numpy arrays
        """
        if not self.buffer:
            return {
                'states': np.array([]),
                'actions': np.array([]),
                'rewards': np.array([]),
                'next_states': np.array([]),
                'dones': np.array([])
            }
        
        states = np.stack([exp.state_vector for exp in self.buffer])
        actions = np.array([exp.action_id for exp in self.buffer], dtype=np.int32)
        rewards = np.array([exp.reward for exp in self.buffer], dtype=np.float32)
        next_states = np.stack([
            exp.next_state_vector if exp.next_state_vector is not None else np.zeros_like(exp.state_vector)
            for exp in self.buffer
        ])
        dones = np.array([exp.done for exp in self.buffer], dtype=np.float32)
        
        return {
            'states': states,
            'actions': actions,
            'rewards': rewards,
            'next_states': next_states,
            'dones': dones
        }
