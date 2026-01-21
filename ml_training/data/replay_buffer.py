"""
ML Monster AI - Replay Buffer Implementation
Enhanced ML Monster AI System v2.0

Implements:
- Standard Replay Buffer
- Prioritized Experience Replay (PER)
- Multi-Agent Replay Buffer
- PostgreSQL integration for persistent storage
"""

import numpy as np
import torch
from typing import Tuple, List, Dict, Optional, Any
from collections import deque
import random
import logging
from dataclasses import dataclass
import pickle

logger = logging.getLogger(__name__)


@dataclass
class Experience:
    """Single experience tuple"""
    state: np.ndarray
    action: int
    reward: float
    next_state: np.ndarray
    done: bool
    info: Dict[str, Any] = None


class ReplayBuffer:
    """
    Standard experience replay buffer
    
    Stores (state, action, reward, next_state, done) tuples
    """
    
    def __init__(self, capacity: int = 100000, state_dim: int = 64):
        """
        Initialize replay buffer
        
        Args:
            capacity: Maximum number of experiences to store
            state_dim: Dimension of state vectors
        """
        self.capacity = capacity
        self.state_dim = state_dim
        
        # Pre-allocate arrays for efficiency
        self.states = np.zeros((capacity, state_dim), dtype=np.float32)
        self.actions = np.zeros(capacity, dtype=np.int64)
        self.rewards = np.zeros(capacity, dtype=np.float32)
        self.next_states = np.zeros((capacity, state_dim), dtype=np.float32)
        self.dones = np.zeros(capacity, dtype=np.float32)
        
        self.position = 0
        self.size = 0
        
        logger.info(f"ReplayBuffer initialized: capacity={capacity}, state_dim={state_dim}")
    
    def add(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool
    ):
        """
        Add experience to buffer
        
        Args:
            state: Current state
            action: Action taken
            reward: Reward received
            next_state: Resulting state
            done: Whether episode ended
        """
        idx = self.position
        
        self.states[idx] = state
        self.actions[idx] = action
        self.rewards[idx] = reward
        self.next_states[idx] = next_state
        self.dones[idx] = float(done)
        
        self.position = (self.position + 1) % self.capacity
        self.size = min(self.size + 1, self.capacity)
    
    def sample(self, batch_size: int) -> Tuple[np.ndarray, ...]:
        """
        Sample random batch
        
        Args:
            batch_size: Number of experiences to sample
        
        Returns:
            Tuple of (states, actions, rewards, next_states, dones)
        """
        if self.size < batch_size:
            logger.warning(f"Buffer size ({self.size}) < batch size ({batch_size}), sampling with replacement")
        
        indices = np.random.randint(0, self.size, size=batch_size)
        
        return (
            self.states[indices],
            self.actions[indices],
            self.rewards[indices],
            self.next_states[indices],
            self.dones[indices]
        )
    
    def __len__(self) -> int:
        return self.size
    
    def is_ready(self, min_size: int) -> bool:
        """Check if buffer has enough experiences"""
        return self.size >= min_size
    
    def clear(self):
        """Clear all experiences"""
        self.position = 0
        self.size = 0
        logger.info("ReplayBuffer cleared")
    
    def save(self, path: str):
        """Save buffer to disk"""
        data = {
            'states': self.states[:self.size],
            'actions': self.actions[:self.size],
            'rewards': self.rewards[:self.size],
            'next_states': self.next_states[:self.size],
            'dones': self.dones[:self.size],
            'position': self.position,
            'size': self.size
        }
        
        np.savez_compressed(path, **data)
        logger.info(f"ReplayBuffer saved to {path} ({self.size} experiences)")
    
    def load(self, path: str):
        """Load buffer from disk"""
        data = np.load(path)
        
        size = len(data['states'])
        if size > self.capacity:
            logger.warning(f"Loaded data size ({size}) > capacity ({self.capacity}), truncating")
            size = self.capacity
        
        self.states[:size] = data['states'][:size]
        self.actions[:size] = data['actions'][:size]
        self.rewards[:size] = data['rewards'][:size]
        self.next_states[:size] = data['next_states'][:size]
        self.dones[:size] = data['dones'][:size]
        self.position = size % self.capacity
        self.size = size
        
        logger.info(f"ReplayBuffer loaded from {path} ({size} experiences)")


class PrioritizedReplayBuffer(ReplayBuffer):
    """
    Prioritized Experience Replay (PER)
    
    Samples experiences based on their TD error (priority)
    Experiences with higher TD error are sampled more frequently
    
    Reference: Schaul et al. (2015) "Prioritized Experience Replay"
    """
    
    def __init__(
        self,
        capacity: int = 100000,
        state_dim: int = 64,
        alpha: float = 0.6,
        beta: float = 0.4,
        beta_increment: float = 0.001,
        epsilon: float = 1e-6
    ):
        """
        Initialize prioritized replay buffer
        
        Args:
            capacity: Maximum buffer size
            state_dim: State vector dimension
            alpha: Priority exponent (0 = uniform, 1 = full prioritization)
            beta: Importance sampling exponent (0 = no correction, 1 = full correction)
            beta_increment: Beta annealing increment per sample
            epsilon: Small constant to prevent zero priorities
        """
        super().__init__(capacity, state_dim)
        
        self.alpha = alpha
        self.beta = beta
        self.beta_increment = beta_increment
        self.epsilon = epsilon
        
        # Priority storage (using sum tree for efficient sampling)
        self.priorities = np.zeros(capacity, dtype=np.float32)
        self.max_priority = 1.0
        
        logger.info(f"PrioritizedReplayBuffer initialized: alpha={alpha}, beta={beta}")
    
    def add(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool,
        priority: Optional[float] = None
    ):
        """
        Add experience with priority
        
        New experiences get maximum priority for immediate learning
        """
        idx = self.position
        
        # Add to base buffer
        super().add(state, action, reward, next_state, done)
        
        # Set priority (new experiences get max priority)
        if priority is None:
            priority = self.max_priority
        
        self.priorities[idx] = priority
    
    def sample(
        self, 
        batch_size: int
    ) -> Tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
        """
        Sample batch with prioritized sampling
        
        Args:
            batch_size: Number of experiences to sample
        
        Returns:
            Tuple of (states, actions, rewards, next_states, dones, weights, indices)
            weights: Importance sampling weights
            indices: Buffer indices (for priority updates)
        """
        if self.size < batch_size:
            logger.warning(f"Buffer size ({self.size}) < batch size ({batch_size})")
        
        # Get valid priorities
        valid_priorities = self.priorities[:self.size]
        
        # Apply priority exponent
        priorities_alpha = np.power(valid_priorities + self.epsilon, self.alpha)
        
        # Compute sampling probabilities
        probs = priorities_alpha / priorities_alpha.sum()
        
        # Sample indices based on priorities
        indices = np.random.choice(self.size, size=batch_size, p=probs, replace=False)
        
        # Compute importance sampling weights
        total = self.size
        weights = np.power(total * probs[indices], -self.beta)
        weights /= weights.max()  # Normalize by max weight
        
        # Anneal beta
        self.beta = min(1.0, self.beta + self.beta_increment)
        
        return (
            self.states[indices],
            self.actions[indices],
            self.rewards[indices],
            self.next_states[indices],
            self.dones[indices],
            weights.astype(np.float32),
            indices
        )
    
    def update_priorities(self, indices: np.ndarray, priorities: np.ndarray):
        """
        Update priorities based on TD errors
        
        Args:
            indices: Buffer indices
            priorities: New priority values (typically abs(TD error))
        """
        for idx, priority in zip(indices, priorities):
            self.priorities[idx] = priority
            self.max_priority = max(self.max_priority, priority)
    
    def save(self, path: str):
        """Save buffer including priorities"""
        data = {
            'states': self.states[:self.size],
            'actions': self.actions[:self.size],
            'rewards': self.rewards[:self.size],
            'next_states': self.next_states[:self.size],
            'dones': self.dones[:self.size],
            'priorities': self.priorities[:self.size],
            'position': self.position,
            'size': self.size,
            'alpha': self.alpha,
            'beta': self.beta,
            'max_priority': self.max_priority
        }
        
        np.savez_compressed(path, **data)
        logger.info(f"PrioritizedReplayBuffer saved to {path}")
    
    def load(self, path: str):
        """Load buffer including priorities"""
        super().load(path)
        
        data = np.load(path)
        if 'priorities' in data:
            size = len(data['priorities'])
            self.priorities[:size] = data['priorities']
            self.max_priority = float(data.get('max_priority', 1.0))
            logger.info(f"Priorities loaded: max_priority={self.max_priority:.4f}")


class MultiAgentReplayBuffer:
    """
    Replay buffer for multi-agent scenarios (pack coordination)
    
    Stores experiences for multiple agents with pack-level information
    """
    
    def __init__(
        self,
        capacity: int = 50000,
        state_dim: int = 64,
        max_agents: int = 5
    ):
        """
        Initialize multi-agent replay buffer
        
        Args:
            capacity: Maximum number of pack experiences
            state_dim: State dimension per agent
            max_agents: Maximum pack size
        """
        self.capacity = capacity
        self.state_dim = state_dim
        self.max_agents = max_agents
        
        # Pack-level storage
        # Pad with zeros for variable pack sizes
        self.pack_states = np.zeros((capacity, max_agents, state_dim), dtype=np.float32)
        self.pack_actions = np.zeros((capacity, max_agents), dtype=np.int64)
        self.pack_rewards = np.zeros((capacity, max_agents), dtype=np.float32)
        self.pack_next_states = np.zeros((capacity, max_agents, state_dim), dtype=np.float32)
        self.pack_dones = np.zeros((capacity, max_agents), dtype=np.float32)
        self.pack_sizes = np.zeros(capacity, dtype=np.int32)  # Actual pack size
        
        # Team rewards (shared by pack)
        self.team_rewards = np.zeros(capacity, dtype=np.float32)
        
        self.position = 0
        self.size = 0
        
        logger.info(f"MultiAgentReplayBuffer initialized: capacity={capacity}, "
                   f"state_dim={state_dim}, max_agents={max_agents}")
    
    def add(
        self,
        pack_states: np.ndarray,
        pack_actions: np.ndarray,
        pack_rewards: np.ndarray,
        pack_next_states: np.ndarray,
        pack_dones: np.ndarray,
        team_reward: float
    ):
        """
        Add pack experience
        
        Args:
            pack_states: (pack_size, state_dim)
            pack_actions: (pack_size,)
            pack_rewards: (pack_size,)
            pack_next_states: (pack_size, state_dim)
            pack_dones: (pack_size,)
            team_reward: Shared team reward
        """
        idx = self.position
        pack_size = len(pack_states)
        
        if pack_size > self.max_agents:
            logger.warning(f"Pack size ({pack_size}) > max_agents ({self.max_agents}), truncating")
            pack_size = self.max_agents
        
        # Store with zero padding
        self.pack_states[idx, :pack_size] = pack_states[:pack_size]
        self.pack_actions[idx, :pack_size] = pack_actions[:pack_size]
        self.pack_rewards[idx, :pack_size] = pack_rewards[:pack_size]
        self.pack_next_states[idx, :pack_size] = pack_next_states[:pack_size]
        self.pack_dones[idx, :pack_size] = pack_dones[:pack_size]
        self.pack_sizes[idx] = pack_size
        self.team_rewards[idx] = team_reward
        
        self.position = (self.position + 1) % self.capacity
        self.size = min(self.size + 1, self.capacity)
    
    def sample(self, batch_size: int) -> Dict[str, np.ndarray]:
        """
        Sample random batch of pack experiences
        
        Args:
            batch_size: Number of pack experiences to sample
        
        Returns:
            Dictionary with pack experience data
        """
        indices = np.random.randint(0, self.size, size=batch_size)
        
        return {
            'pack_states': self.pack_states[indices],
            'pack_actions': self.pack_actions[indices],
            'pack_rewards': self.pack_rewards[indices],
            'pack_next_states': self.pack_next_states[indices],
            'pack_dones': self.pack_dones[indices],
            'pack_sizes': self.pack_sizes[indices],
            'team_rewards': self.team_rewards[indices]
        }
    
    def __len__(self) -> int:
        return self.size
    
    def clear(self):
        """Clear all experiences"""
        self.position = 0
        self.size = 0
        logger.info("MultiAgentReplayBuffer cleared")


class SegmentTree:
    """
    Segment tree data structure for efficient sum/min queries
    
    Used by PrioritizedReplayBuffer for O(log n) priority sampling
    """
    
    def __init__(self, capacity: int, operation: str = 'sum'):
        """
        Initialize segment tree
        
        Args:
            capacity: Number of leaf nodes (must be power of 2)
            operation: 'sum' or 'min'
        """
        # Round up to nearest power of 2
        self.capacity = capacity
        self.tree_capacity = 1
        while self.tree_capacity < capacity:
            self.tree_capacity *= 2
        
        self.tree = np.zeros(2 * self.tree_capacity, dtype=np.float32)
        self.operation = operation
        
        # Set neutral element
        if operation == 'sum':
            self.neutral_element = 0.0
            self.aggregate_fn = np.add
        elif operation == 'min':
            self.neutral_element = float('inf')
            self.aggregate_fn = np.minimum
        else:
            raise ValueError(f"Unknown operation: {operation}")
    
    def _propagate(self, idx: int):
        """Propagate change up the tree"""
        parent = (idx - 1) // 2
        
        if parent >= 0:
            left = 2 * parent + 1
            right = 2 * parent + 2
            
            self.tree[parent] = self.aggregate_fn(self.tree[left], self.tree[right])
            
            if parent > 0:
                self._propagate(parent)
    
    def update(self, data_idx: int, value: float):
        """
        Update leaf value
        
        Args:
            data_idx: Index in data array [0, capacity)
            value: New value
        """
        tree_idx = data_idx + self.tree_capacity - 1
        self.tree[tree_idx] = value
        self._propagate(tree_idx)
    
    def get(self, data_idx: int) -> float:
        """Get leaf value"""
        tree_idx = data_idx + self.tree_capacity - 1
        return self.tree[tree_idx]
    
    def reduce(self, start: int = 0, end: Optional[int] = None) -> float:
        """
        Reduce operation over range [start, end)
        
        Args:
            start: Start index
            end: End index (exclusive), None = capacity
        
        Returns:
            Aggregated value
        """
        if end is None:
            end = self.capacity
        
        if end > self.capacity:
            end = self.capacity
        
        # Convert to tree indices
        start += self.tree_capacity - 1
        end += self.tree_capacity - 1
        
        result = self.neutral_element
        
        while start < end:
            if start % 2 == 0:  # Start is a right child
                result = self.aggregate_fn(result, self.tree[start])
                start += 1
            if end % 2 == 0:  # End is a right child
                end -= 1
                result = self.aggregate_fn(result, self.tree[end])
            
            start = (start - 1) // 2
            end = (end - 1) // 2
        
        return result
    
    def find_prefixsum_idx(self, target: float) -> int:
        """
        Find index where cumulative sum >= target
        
        Used for proportional sampling
        
        Args:
            target: Target cumulative sum
        
        Returns:
            Data index
        """
        idx = 0  # Start at root
        
        while idx < self.tree_capacity - 1:  # While not leaf
            left = 2 * idx + 1
            right = 2 * idx + 2
            
            if target <= self.tree[left]:
                idx = left
            else:
                target -= self.tree[left]
                idx = right
        
        data_idx = idx - (self.tree_capacity - 1)
        return data_idx


class SumTree:
    """
    Sum tree for efficient proportional priority sampling
    
    Wrapper around SegmentTree with sum operation
    """
    
    def __init__(self, capacity: int):
        self.tree = SegmentTree(capacity, operation='sum')
        self.capacity = capacity
    
    def update(self, data_idx: int, priority: float):
        """Update priority"""
        self.tree.update(data_idx, priority)
    
    def get(self, data_idx: int) -> float:
        """Get priority"""
        return self.tree.get(data_idx)
    
    def total(self) -> float:
        """Get total sum of all priorities"""
        return self.tree.tree[0]
    
    def sample(self, target: float) -> int:
        """Sample index proportional to priority"""
        return self.tree.find_prefixsum_idx(target)


class MinTree:
    """
    Min tree for efficient minimum priority tracking
    
    Wrapper around SegmentTree with min operation
    """
    
    def __init__(self, capacity: int):
        self.tree = SegmentTree(capacity, operation='min')
        self.capacity = capacity
    
    def update(self, data_idx: int, priority: float):
        """Update priority"""
        self.tree.update(data_idx, priority)
    
    def min(self) -> float:
        """Get minimum priority"""
        return self.tree.tree[0]


class NStepReplayBuffer(PrioritizedReplayBuffer):
    """
    N-step replay buffer for multi-step returns
    
    Stores n-step transitions: (s_t, a_t, R_t, s_{t+n}, done_{t+n})
    where R_t = r_t + γ*r_{t+1} + γ²*r_{t+2} + ... + γ^{n-1}*r_{t+n-1}
    
    Improves credit assignment by looking ahead n steps
    """
    
    def __init__(
        self,
        capacity: int = 100000,
        state_dim: int = 64,
        n_step: int = 5,
        gamma: float = 0.99,
        **kwargs
    ):
        """
        Initialize n-step replay buffer
        
        Args:
            capacity: Maximum buffer size
            state_dim: State dimension
            n_step: Number of steps to look ahead
            gamma: Discount factor
            **kwargs: Additional arguments for PrioritizedReplayBuffer
        """
        super().__init__(capacity, state_dim, **kwargs)
        
        self.n_step = n_step
        self.gamma = gamma
        
        # N-step buffer (stores last n transitions)
        self.n_step_buffer = deque(maxlen=n_step)
        
        logger.info(f"NStepReplayBuffer initialized: n_step={n_step}, gamma={gamma}")
    
    def add(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool
    ):
        """
        Add transition to n-step buffer, then to main buffer when n-step complete
        """
        # Add to n-step buffer
        self.n_step_buffer.append((state, action, reward, next_state, done))
        
        # If buffer full or episode done, compute n-step return
        if len(self.n_step_buffer) == self.n_step or done:
            # Compute n-step return
            n_step_state, n_step_action, n_step_reward, n_step_next_state, n_step_done = self._compute_n_step()
            
            # Add to main buffer
            super().add(n_step_state, n_step_action, n_step_reward, n_step_next_state, n_step_done)
            
            # If episode done, clear n-step buffer
            if done:
                self.n_step_buffer.clear()
    
    def _compute_n_step(self) -> Tuple[np.ndarray, int, float, np.ndarray, bool]:
        """
        Compute n-step transition
        
        Returns:
            (state_t, action_t, n_step_return, state_{t+n}, done_{t+n})
        """
        # Get first transition
        state, action, _, _, _ = self.n_step_buffer[0]
        
        # Compute n-step return
        n_step_return = 0.0
        for i, (_, _, reward, _, _) in enumerate(self.n_step_buffer):
            n_step_return += (self.gamma ** i) * reward
        
        # Get final state and done
        _, _, _, next_state, done = self.n_step_buffer[-1]
        
        return state, action, n_step_return, next_state, done


class PostgreSQLReplayBuffer:
    """
    Replay buffer with PostgreSQL backend for persistent storage
    
    Integrates with ml_experience_replay table (TimescaleDB hypertable)
    """
    
    def __init__(
        self,
        archetype: str,
        capacity: int = 100000,
        state_dim: int = 64,
        db_config: Optional[Dict[str, str]] = None
    ):
        """
        Initialize PostgreSQL-backed replay buffer
        
        Args:
            archetype: Monster archetype
            capacity: In-memory cache size
            state_dim: State dimension
            db_config: Database connection configuration
        """
        self.archetype = archetype
        self.capacity = capacity
        self.state_dim = state_dim
        self.db_config = db_config or {}
        
        # In-memory cache (for fast sampling)
        self.memory_buffer = PrioritizedReplayBuffer(capacity, state_dim)
        
        # Database connection (lazy initialization)
        self.db_conn = None
        
        logger.info(f"PostgreSQLReplayBuffer initialized for archetype={archetype}")
    
    async def connect_db(self):
        """Establish database connection"""
        try:
            import asyncpg
            
            self.db_conn = await asyncpg.connect(
                host=self.db_config.get('host', 'localhost'),
                port=self.db_config.get('port', 5432),
                user=self.db_config.get('user', 'ml_user'),
                password=self.db_config.get('password', ''),
                database=self.db_config.get('database', 'ragnarok_ml')
            )
            
            logger.info("PostgreSQL connection established")
            return True
        
        except Exception as e:
            logger.error(f"Failed to connect to PostgreSQL: {e}")
            return False
    
    async def store_to_db(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool,
        monster_id: int,
        episode_id: int
    ):
        """
        Store experience to PostgreSQL
        
        Inserts into ml_experience_replay table
        """
        if self.db_conn is None:
            await self.connect_db()
        
        try:
            query = """
            INSERT INTO ml_experience_replay 
            (monster_id, episode_id, archetype, state_vector, action_type, 
             reward, next_state_vector, done, created_at)
            VALUES ($1, $2, $3, $4, $5, $6, $7, $8, NOW())
            """
            
            await self.db_conn.execute(
                query,
                monster_id,
                episode_id,
                self.archetype,
                state.tobytes(),  # Store as bytea
                action,
                reward,
                next_state.tobytes(),
                done
            )
        
        except Exception as e:
            logger.error(f"Failed to store experience to DB: {e}")
    
    async def load_from_db(self, limit: int = None) -> int:
        """
        Load experiences from PostgreSQL to memory buffer
        
        Args:
            limit: Maximum number of experiences to load (None = all)
        
        Returns:
            Number of experiences loaded
        """
        if self.db_conn is None:
            await self.connect_db()
        
        try:
            query = """
            SELECT state_vector, action_type, reward, next_state_vector, done
            FROM ml_experience_replay
            WHERE archetype = $1
            ORDER BY created_at DESC
            """
            
            if limit:
                query += f" LIMIT {limit}"
            
            rows = await self.db_conn.fetch(query, self.archetype)
            
            count = 0
            for row in rows:
                # Deserialize state vectors
                state = np.frombuffer(row['state_vector'], dtype=np.float32)
                next_state = np.frombuffer(row['next_state_vector'], dtype=np.float32)
                
                # Add to memory buffer
                self.memory_buffer.add(
                    state,
                    row['action_type'],
                    row['reward'],
                    next_state,
                    row['done']
                )
                
                count += 1
            
            logger.info(f"Loaded {count} experiences from PostgreSQL for archetype={self.archetype}")
            return count
        
        except Exception as e:
            logger.error(f"Failed to load experiences from DB: {e}")
            return 0
    
    def add(self, *args, **kwargs):
        """Add to memory buffer"""
        self.memory_buffer.add(*args, **kwargs)
    
    def sample(self, batch_size: int):
        """Sample from memory buffer"""
        return self.memory_buffer.sample(batch_size)
    
    def update_priorities(self, indices: np.ndarray, priorities: np.ndarray):
        """Update priorities"""
        self.memory_buffer.update_priorities(indices, priorities)
    
    def __len__(self) -> int:
        return len(self.memory_buffer)
    
    async def close(self):
        """Close database connection"""
        if self.db_conn:
            await self.db_conn.close()
            logger.info("PostgreSQL connection closed")


class EpisodeBuffer:
    """
    Buffer for storing complete episodes
    
    Useful for on-policy algorithms (PPO) that need full trajectories
    """
    
    def __init__(self, capacity: int = 1000):
        """
        Initialize episode buffer
        
        Args:
            capacity: Maximum number of episodes to store
        """
        self.capacity = capacity
        self.episodes = deque(maxlen=capacity)
        
        logger.info(f"EpisodeBuffer initialized: capacity={capacity}")
    
    def add_episode(
        self,
        states: List[np.ndarray],
        actions: List[int],
        rewards: List[float],
        dones: List[bool],
        values: Optional[List[float]] = None,
        log_probs: Optional[List[float]] = None
    ):
        """
        Add complete episode
        
        Args:
            states: List of states
            actions: List of actions
            rewards: List of rewards
            dones: List of done flags
            values: Optional list of value estimates
            log_probs: Optional list of log probabilities
        """
        episode = {
            'states': np.array(states),
            'actions': np.array(actions),
            'rewards': np.array(rewards),
            'dones': np.array(dones),
            'length': len(states)
        }
        
        if values is not None:
            episode['values'] = np.array(values)
        
        if log_probs is not None:
            episode['log_probs'] = np.array(log_probs)
        
        self.episodes.append(episode)
    
    def get_episodes(self, num_episodes: Optional[int] = None) -> List[Dict]:
        """
        Get episodes
        
        Args:
            num_episodes: Number of recent episodes to get (None = all)
        
        Returns:
            List of episode dictionaries
        """
        if num_episodes is None:
            return list(self.episodes)
        
        return list(self.episodes)[-num_episodes:]
    
    def compute_returns(
        self,
        gamma: float = 0.99,
        use_gae: bool = False,
        gae_lambda: float = 0.97
    ) -> List[np.ndarray]:
        """
        Compute returns for all episodes
        
        Args:
            gamma: Discount factor
            use_gae: Whether to use Generalized Advantage Estimation
            gae_lambda: GAE lambda parameter
        
        Returns:
            List of return arrays (one per episode)
        """
        all_returns = []
        
        for episode in self.episodes:
            rewards = episode['rewards']
            dones = episode['dones']
            
            if use_gae and 'values' in episode:
                # GAE computation
                values = episode['values']
                returns = self._compute_gae(rewards, values, dones, gamma, gae_lambda)
            else:
                # Simple discounted returns
                returns = self._compute_returns(rewards, gamma)
            
            all_returns.append(returns)
        
        return all_returns
    
    @staticmethod
    def _compute_returns(rewards: np.ndarray, gamma: float) -> np.ndarray:
        """Compute discounted returns"""
        returns = np.zeros_like(rewards)
        running_return = 0.0
        
        for t in reversed(range(len(rewards))):
            running_return = rewards[t] + gamma * running_return
            returns[t] = running_return
        
        return returns
    
    @staticmethod
    def _compute_gae(
        rewards: np.ndarray,
        values: np.ndarray,
        dones: np.ndarray,
        gamma: float,
        gae_lambda: float
    ) -> np.ndarray:
        """
        Compute Generalized Advantage Estimation
        
        GAE(γ, λ) balances bias-variance trade-off in advantage estimation
        """
        advantages = np.zeros_like(rewards)
        last_gae = 0.0
        
        for t in reversed(range(len(rewards))):
            if t == len(rewards) - 1:
                next_value = 0.0
            else:
                next_value = values[t + 1]
            
            # TD error
            delta = rewards[t] + gamma * next_value * (1 - dones[t]) - values[t]
            
            # GAE
            last_gae = delta + gamma * gae_lambda * (1 - dones[t]) * last_gae
            advantages[t] = last_gae
        
        # Returns = advantages + values
        returns = advantages + values
        
        return returns
    
    def clear(self):
        """Clear all episodes"""
        self.episodes.clear()
        logger.info("EpisodeBuffer cleared")
    
    def __len__(self) -> int:
        return len(self.episodes)
    
    def get_total_steps(self) -> int:
        """Get total number of steps across all episodes"""
        return sum(ep['length'] for ep in self.episodes)


# Utility functions

def create_replay_buffer(
    buffer_type: str = 'prioritized',
    archetype: str = 'aggressive',
    **kwargs
) -> ReplayBuffer:
    """
    Factory function to create appropriate replay buffer
    
    Args:
        buffer_type: 'standard', 'prioritized', 'n_step', 'multi_agent', 'postgresql'
        archetype: Monster archetype
        **kwargs: Additional arguments for buffer
    
    Returns:
        Initialized replay buffer
    """
    if buffer_type == 'standard':
        return ReplayBuffer(**kwargs)
    
    elif buffer_type == 'prioritized':
        return PrioritizedReplayBuffer(**kwargs)
    
    elif buffer_type == 'n_step':
        return NStepReplayBuffer(**kwargs)
    
    elif buffer_type == 'multi_agent':
        return MultiAgentReplayBuffer(**kwargs)
    
    elif buffer_type == 'postgresql':
        return PostgreSQLReplayBuffer(archetype=archetype, **kwargs)
    
    else:
        raise ValueError(f"Unknown buffer type: {buffer_type}")
