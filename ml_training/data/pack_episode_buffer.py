"""
Pack Episode Buffer for CTDE Training
=====================================
Stores and manages pack episodes for multi-agent reinforcement learning.

Each episode contains data for a team of 2-5 monsters working together:
- Global states (full battlefield view)
- Local observations (per-agent view)
- Actions taken by each agent
- Team rewards and individual rewards
- Communication signals exchanged

Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md
Integration: Loads episodes from PostgreSQL ml_pack_episodes table
"""

import numpy as np
import torch
from typing import Dict, List, Any, Optional, Tuple
from collections import deque, defaultdict
import logging
import asyncio
import asyncpg
from dataclasses import dataclass, asdict
import json
from datetime import datetime

logger = logging.getLogger(__name__)


@dataclass
class PackEpisode:
    """
    Container for a single pack episode
    
    Episode structure matches CTDE requirements:
    - Global states: Full battlefield state visible to centralized critic
    - Local observations: Per-agent local view for decentralized actors
    - Actions: Per-agent action indices
    - Rewards: Team reward (shared) + individual rewards (per agent)
    - Signals: Communication signals exchanged between agents
    """
    
    pack_id: int
    monster_ids: List[int]
    pack_size: int
    
    # Temporal data (T = episode length)
    global_states: np.ndarray  # (T, n_agents, state_dim)
    local_obs: np.ndarray  # (T, n_agents, state_dim)
    actions: np.ndarray  # (T, n_agents)
    team_rewards: np.ndarray  # (T,) - shared team reward
    individual_rewards: np.ndarray  # (T, n_agents) - per-agent rewards
    signals: Optional[np.ndarray] = None  # (T, n_agents, signal_dim)
    
    # Episode metadata
    episode_length: int = 0
    outcome: str = 'unknown'  # 'victory', 'defeat', 'retreat', 'timeout'
    total_team_reward: float = 0.0
    avg_individual_reward: float = 0.0
    
    # Auxiliary data
    positions: Optional[np.ndarray] = None  # (T, n_agents, 2) - x,y positions
    done_mask: Optional[np.ndarray] = None  # (T, n_agents) - which agents died
    
    # Timestamps
    created_at: datetime = None
    
    def __post_init__(self):
        """Validate and compute derived fields"""
        if self.created_at is None:
            self.created_at = datetime.now()
        
        if self.episode_length == 0:
            self.episode_length = len(self.team_rewards)
        
        if self.total_team_reward == 0.0:
            self.total_team_reward = float(np.sum(self.team_rewards))
        
        if self.avg_individual_reward == 0.0 and len(self.individual_rewards) > 0:
            self.avg_individual_reward = float(np.mean(self.individual_rewards))
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for serialization"""
        data = asdict(self)
        
        # Convert numpy arrays to lists for JSON serialization
        for key, value in data.items():
            if isinstance(value, np.ndarray):
                data[key] = value.tolist()
            elif isinstance(value, datetime):
                data[key] = value.isoformat()
        
        return data
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'PackEpisode':
        """Create from dictionary"""
        # Convert lists back to numpy arrays
        for key in ['global_states', 'local_obs', 'actions', 'team_rewards', 
                    'individual_rewards', 'signals', 'positions', 'done_mask']:
            if key in data and data[key] is not None:
                data[key] = np.array(data[key])
        
        # Convert timestamp
        if 'created_at' in data and isinstance(data['created_at'], str):
            data['created_at'] = datetime.fromisoformat(data['created_at'])
        
        return cls(**data)


class PackEpisodeBuffer:
    """
    Buffer for storing pack episodes for CTDE training
    
    Features:
    - In-memory storage with configurable capacity
    - Priority sampling based on team reward
    - Pack size balancing (equal representation of 2,3,4,5 agent packs)
    - Integration with PostgreSQL for persistence
    - Efficient batch sampling for training
    - Statistics tracking
    
    Storage:
    - Memory: Recent episodes (last 1000)
    - Database: All episodes for long-term analysis
    """
    
    def __init__(
        self,
        max_episodes: int = 1000,
        db_pool: Optional[asyncpg.Pool] = None,
        prioritize_by_reward: bool = True
    ):
        """
        Initialize pack episode buffer
        
        Args:
            max_episodes: Maximum episodes to keep in memory
            db_pool: PostgreSQL connection pool for persistence
            prioritize_by_reward: Whether to prioritize high-reward episodes
        """
        self.max_episodes = max_episodes
        self.db_pool = db_pool
        self.prioritize_by_reward = prioritize_by_reward
        
        # Episode storage by pack size (for balanced sampling)
        self.episodes_by_size: Dict[int, deque] = {
            2: deque(maxlen=max_episodes // 4),
            3: deque(maxlen=max_episodes // 4),
            4: deque(maxlen=max_episodes // 4),
            5: deque(maxlen=max_episodes // 4)
        }
        
        # All episodes (for sequential access)
        self.all_episodes: deque = deque(maxlen=max_episodes)
        
        # Statistics
        self.total_added = 0
        self.episodes_saved_to_db = 0
        
        # Priority weights (if using prioritized sampling)
        self.priorities: Dict[int, float] = {}  # pack_id -> priority
        
        logger.info(f"PackEpisodeBuffer initialized: max_episodes={max_episodes}")
    
    def add_episode(self, episode: PackEpisode) -> None:
        """
        Add episode to buffer
        
        Args:
            episode: PackEpisode instance
        """
        pack_size = episode.pack_size
        
        # Validate pack size
        if pack_size < 2 or pack_size > 5:
            logger.warning(f"Invalid pack size {pack_size}, skipping episode")
            return
        
        # Add to size-specific buffer
        if pack_size in self.episodes_by_size:
            self.episodes_by_size[pack_size].append(episode)
        
        # Add to all episodes
        self.all_episodes.append(episode)
        
        # Update priority
        if self.prioritize_by_reward:
            # Priority based on absolute team reward
            priority = abs(episode.total_team_reward) + 1.0  # Add 1 to avoid zero
            self.priorities[episode.pack_id] = priority
        
        # Update stats
        self.total_added += 1
        
        logger.debug(
            f"Added pack episode: id={episode.pack_id}, size={pack_size}, "
            f"length={episode.episode_length}, reward={episode.total_team_reward:.2f}"
        )
    
    def get_batch(
        self,
        batch_size: int,
        pack_size: Optional[int] = None,
        prioritized: bool = False
    ) -> List[PackEpisode]:
        """
        Sample batch of episodes for training
        
        Args:
            batch_size: Number of episodes to sample
            pack_size: If specified, only sample from this pack size
            prioritized: Whether to use priority sampling
        
        Returns:
            List of PackEpisode instances
        """
        if pack_size is not None:
            # Sample from specific pack size
            if pack_size not in self.episodes_by_size:
                logger.warning(f"No episodes for pack size {pack_size}")
                return []
            
            episodes = list(self.episodes_by_size[pack_size])
        else:
            # Sample from all episodes with balanced pack sizes
            episodes = []
            samples_per_size = batch_size // 4
            
            for size in [2, 3, 4, 5]:
                size_episodes = list(self.episodes_by_size[size])
                if size_episodes:
                    n_sample = min(samples_per_size, len(size_episodes))
                    sampled = np.random.choice(size_episodes, size=n_sample, replace=False).tolist()
                    episodes.extend(sampled)
            
            # Fill remaining if needed
            remaining = batch_size - len(episodes)
            if remaining > 0 and self.all_episodes:
                additional = np.random.choice(
                    list(self.all_episodes),
                    size=min(remaining, len(self.all_episodes)),
                    replace=False
                ).tolist()
                episodes.extend(additional)
        
        if not episodes:
            return []
        
        # Apply priority sampling if enabled
        if prioritized and self.prioritize_by_reward:
            # Get priorities for available episodes
            priorities = np.array([
                self.priorities.get(ep.pack_id, 1.0) for ep in episodes
            ])
            
            # Normalize to probabilities
            probs = priorities / priorities.sum()
            
            # Sample with replacement according to priorities
            indices = np.random.choice(
                len(episodes),
                size=min(batch_size, len(episodes)),
                replace=True,
                p=probs
            )
            
            batch = [episodes[i] for i in indices]
        else:
            # Uniform random sampling
            batch = np.random.choice(
                episodes,
                size=min(batch_size, len(episodes)),
                replace=False
            ).tolist()
        
        return batch
    
    def get_pack_statistics(self) -> Dict[str, Any]:
        """
        Get buffer statistics
        
        Returns:
            Dictionary with buffer stats
        """
        stats = {
            'total_episodes': len(self.all_episodes),
            'total_added_lifetime': self.total_added,
            'episodes_saved_to_db': self.episodes_saved_to_db,
            'by_pack_size': {}
        }
        
        # Stats by pack size
        for size, episodes in self.episodes_by_size.items():
            if episodes:
                rewards = [ep.total_team_reward for ep in episodes]
                lengths = [ep.episode_length for ep in episodes]
                
                stats['by_pack_size'][size] = {
                    'count': len(episodes),
                    'avg_reward': float(np.mean(rewards)),
                    'std_reward': float(np.std(rewards)),
                    'min_reward': float(np.min(rewards)),
                    'max_reward': float(np.max(rewards)),
                    'avg_length': float(np.mean(lengths)),
                    'outcome_distribution': self._get_outcome_distribution(episodes)
                }
        
        return stats
    
    def _get_outcome_distribution(self, episodes: List[PackEpisode]) -> Dict[str, int]:
        """Get distribution of episode outcomes"""
        outcomes = defaultdict(int)
        for ep in episodes:
            outcomes[ep.outcome] += 1
        return dict(outcomes)
    
    async def save_to_database(self, episode: PackEpisode) -> bool:
        """
        Save episode to PostgreSQL database
        
        Args:
            episode: PackEpisode to save
        
        Returns:
            True if saved successfully
        """
        if self.db_pool is None:
            logger.warning("No database pool configured, skipping save")
            return False
        
        try:
            async with self.db_pool.acquire() as conn:
                # Insert into ml_pack_episodes table
                query = """
                INSERT INTO ml_pack_episodes (
                    pack_id, monster_ids, pack_size, episode_length,
                    team_reward, outcome, global_states, local_obs,
                    actions, individual_rewards, signals, positions,
                    created_at
                )
                VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
                ON CONFLICT (pack_id) DO UPDATE SET
                    episode_length = EXCLUDED.episode_length,
                    team_reward = EXCLUDED.team_reward,
                    outcome = EXCLUDED.outcome,
                    created_at = EXCLUDED.created_at
                """
                
                await conn.execute(
                    query,
                    episode.pack_id,
                    episode.monster_ids,
                    episode.pack_size,
                    episode.episode_length,
                    episode.total_team_reward,
                    episode.outcome,
                    episode.global_states.tobytes(),
                    episode.local_obs.tobytes(),
                    episode.actions.tobytes(),
                    episode.individual_rewards.tobytes(),
                    episode.signals.tobytes() if episode.signals is not None else None,
                    episode.positions.tobytes() if episode.positions is not None else None,
                    episode.created_at
                )
                
                self.episodes_saved_to_db += 1
                logger.debug(f"Saved pack episode {episode.pack_id} to database")
                
                return True
        
        except Exception as e:
            logger.error(f"Failed to save episode to database: {e}", exc_info=True)
            return False
    
    async def load_from_database(
        self,
        pack_ids: Optional[List[int]] = None,
        limit: int = 100,
        min_reward: Optional[float] = None
    ) -> int:
        """
        Load episodes from database into buffer
        
        Args:
            pack_ids: Specific pack IDs to load (if None, load recent)
            limit: Maximum episodes to load
            min_reward: Minimum team reward filter
        
        Returns:
            Number of episodes loaded
        """
        if self.db_pool is None:
            logger.warning("No database pool configured, cannot load")
            return 0
        
        try:
            async with self.db_pool.acquire() as conn:
                # Build query
                if pack_ids is not None:
                    query = """
                    SELECT * FROM ml_pack_episodes
                    WHERE pack_id = ANY($1::BIGINT[])
                    ORDER BY created_at DESC
                    LIMIT $2
                    """
                    rows = await conn.fetch(query, pack_ids, limit)
                elif min_reward is not None:
                    query = """
                    SELECT * FROM ml_pack_episodes
                    WHERE team_reward >= $1
                    ORDER BY created_at DESC
                    LIMIT $2
                    """
                    rows = await conn.fetch(query, min_reward, limit)
                else:
                    query = """
                    SELECT * FROM ml_pack_episodes
                    ORDER BY created_at DESC
                    LIMIT $1
                    """
                    rows = await conn.fetch(query, limit)
                
                # Parse episodes
                loaded_count = 0
                for row in rows:
                    try:
                        episode = self._parse_episode_from_row(row)
                        self.add_episode(episode)
                        loaded_count += 1
                    except Exception as e:
                        logger.error(f"Failed to parse episode {row['pack_id']}: {e}")
                
                logger.info(f"Loaded {loaded_count} pack episodes from database")
                return loaded_count
        
        except Exception as e:
            logger.error(f"Failed to load from database: {e}", exc_info=True)
            return 0
    
    def _parse_episode_from_row(self, row: asyncpg.Record) -> PackEpisode:
        """Parse database row into PackEpisode"""
        # Reconstruct numpy arrays from bytes
        # We need to know the shapes - stored in metadata or inferred
        pack_size = row['pack_size']
        episode_length = row['episode_length']
        state_dim = 64  # Fixed from architecture
        signal_dim = 32  # Fixed from architecture
        
        # Parse arrays
        global_states = np.frombuffer(
            row['global_states'],
            dtype=np.float32
        ).reshape(episode_length, pack_size, state_dim)
        
        local_obs = np.frombuffer(
            row['local_obs'],
            dtype=np.float32
        ).reshape(episode_length, pack_size, state_dim)
        
        actions = np.frombuffer(
            row['actions'],
            dtype=np.int32
        ).reshape(episode_length, pack_size)
        
        individual_rewards = np.frombuffer(
            row['individual_rewards'],
            dtype=np.float32
        ).reshape(episode_length, pack_size)
        
        # Team rewards (1D)
        # Infer size from total bytes
        team_rewards_bytes = row['team_reward']
        if isinstance(team_rewards_bytes, bytes):
            team_rewards = np.frombuffer(team_rewards_bytes, dtype=np.float32)
        else:
            # Might be stored as single float in some schemas
            team_rewards = np.array([team_rewards_bytes] * episode_length, dtype=np.float32)
        
        # Optional fields
        signals = None
        if row.get('signals'):
            signals = np.frombuffer(
                row['signals'],
                dtype=np.float32
            ).reshape(episode_length, pack_size, signal_dim)
        
        positions = None
        if row.get('positions'):
            positions = np.frombuffer(
                row['positions'],
                dtype=np.float32
            ).reshape(episode_length, pack_size, 2)
        
        # Create episode
        episode = PackEpisode(
            pack_id=row['pack_id'],
            monster_ids=row['monster_ids'],
            pack_size=pack_size,
            global_states=global_states,
            local_obs=local_obs,
            actions=actions,
            team_rewards=team_rewards,
            individual_rewards=individual_rewards,
            signals=signals,
            episode_length=episode_length,
            outcome=row['outcome'],
            positions=positions,
            created_at=row['created_at']
        )
        
        return episode
    
    def clear(self) -> None:
        """Clear all episodes from buffer"""
        for deque_buffer in self.episodes_by_size.values():
            deque_buffer.clear()
        self.all_episodes.clear()
        self.priorities.clear()
        
        logger.info("Pack episode buffer cleared")
    
    def __len__(self) -> int:
        """Get total number of episodes in buffer"""
        return len(self.all_episodes)
    
    def get_episode_by_id(self, pack_id: int) -> Optional[PackEpisode]:
        """Get specific episode by pack ID"""
        for episode in self.all_episodes:
            if episode.pack_id == pack_id:
                return episode
        return None


class PackEpisodeCollector:
    """
    Collects pack episodes during gameplay
    
    Manages ongoing episodes and finalizes them when complete.
    Works with both live gameplay and replay data.
    
    Features:
    - Tracks active pack episodes
    - Accumulates transitions
    - Computes team rewards
    - Handles pack member death/removal
    - Finalizes episodes on completion
    """
    
    def __init__(self, buffer: PackEpisodeBuffer):
        """
        Initialize episode collector
        
        Args:
            buffer: PackEpisodeBuffer to add completed episodes
        """
        self.buffer = buffer
        
        # Active episodes: pack_id -> episode data
        self.active_episodes: Dict[int, Dict[str, Any]] = {}
        
        # Episode counters
        self.next_pack_id = 1
        self.completed_count = 0
        
        logger.info("PackEpisodeCollector initialized")
    
    def start_episode(
        self,
        monster_ids: List[int],
        initial_states: np.ndarray,
        pack_id: Optional[int] = None
    ) -> int:
        """
        Start tracking a new pack episode
        
        Args:
            monster_ids: List of monster IDs in pack
            initial_states: (n_agents, state_dim) initial states
            pack_id: Optional pack ID (auto-generated if None)
        
        Returns:
            pack_id for this episode
        """
        if pack_id is None:
            pack_id = self.next_pack_id
            self.next_pack_id += 1
        
        pack_size = len(monster_ids)
        
        self.active_episodes[pack_id] = {
            'pack_id': pack_id,
            'monster_ids': monster_ids,
            'pack_size': pack_size,
            'global_states': [initial_states],
            'local_obs': [initial_states],  # Same as global for now
            'actions': [],
            'team_rewards': [],
            'individual_rewards': [],
            'signals': [],
            'positions': [],
            'alive_mask': [True] * pack_size,
            'start_time': datetime.now()
        }
        
        logger.info(f"Started pack episode {pack_id} with {pack_size} monsters: {monster_ids}")
        
        return pack_id
    
    def add_transition(
        self,
        pack_id: int,
        next_states: np.ndarray,
        actions: np.ndarray,
        team_reward: float,
        individual_rewards: np.ndarray,
        signals: Optional[np.ndarray] = None,
        positions: Optional[np.ndarray] = None
    ) -> bool:
        """
        Add transition to active episode
        
        Args:
            pack_id: Pack episode ID
            next_states: (n_agents, state_dim) next states
            actions: (n_agents,) actions taken
            team_reward: Team reward for this step
            individual_rewards: (n_agents,) individual rewards
            signals: (n_agents, signal_dim) signals exchanged
            positions: (n_agents, 2) agent positions
        
        Returns:
            True if added successfully
        """
        if pack_id not in self.active_episodes:
            logger.warning(f"Pack {pack_id} not in active episodes")
            return False
        
        episode = self.active_episodes[pack_id]
        
        # Add data
        episode['global_states'].append(next_states)
        episode['local_obs'].append(next_states)  # Same as global for simplicity
        episode['actions'].append(actions)
        episode['team_rewards'].append(team_reward)
        episode['individual_rewards'].append(individual_rewards)
        
        if signals is not None:
            episode['signals'].append(signals)
        
        if positions is not None:
            episode['positions'].append(positions)
        
        return True
    
    def update_alive_mask(self, pack_id: int, monster_id: int, alive: bool) -> bool:
        """
        Update agent alive status
        
        Args:
            pack_id: Pack episode ID
            monster_id: Monster that died/revived
            alive: Whether monster is alive
        
        Returns:
            True if updated
        """
        if pack_id not in self.active_episodes:
            return False
        
        episode = self.active_episodes[pack_id]
        
        # Find monster index
        try:
            idx = episode['monster_ids'].index(monster_id)
            episode['alive_mask'][idx] = alive
            
            logger.debug(f"Pack {pack_id}: Monster {monster_id} alive={alive}")
            return True
        except ValueError:
            logger.warning(f"Monster {monster_id} not in pack {pack_id}")
            return False
    
    def finalize_episode(
        self,
        pack_id: int,
        outcome: str = 'unknown'
    ) -> Optional[PackEpisode]:
        """
        Finalize and store completed episode
        
        Args:
            pack_id: Pack episode ID
            outcome: Episode outcome ('victory', 'defeat', 'retreat', 'timeout')
        
        Returns:
            Completed PackEpisode or None if error
        """
        if pack_id not in self.active_episodes:
            logger.warning(f"Pack {pack_id} not in active episodes")
            return None
        
        data = self.active_episodes.pop(pack_id)
        
        try:
            # Convert lists to numpy arrays
            episode = PackEpisode(
                pack_id=data['pack_id'],
                monster_ids=data['monster_ids'],
                pack_size=data['pack_size'],
                global_states=np.array(data['global_states'], dtype=np.float32),
                local_obs=np.array(data['local_obs'], dtype=np.float32),
                actions=np.array(data['actions'], dtype=np.int32),
                team_rewards=np.array(data['team_rewards'], dtype=np.float32),
                individual_rewards=np.array(data['individual_rewards'], dtype=np.float32),
                signals=np.array(data['signals'], dtype=np.float32) if data['signals'] else None,
                outcome=outcome,
                positions=np.array(data['positions'], dtype=np.float32) if data['positions'] else None
            )
            
            # Add to buffer
            self.buffer.add_episode(episode)
            
            # Save to database asynchronously
            if self.buffer.db_pool is not None:
                asyncio.create_task(self.buffer.save_to_database(episode))
            
            self.completed_count += 1
            
            logger.info(
                f"Finalized pack episode {pack_id}: length={episode.episode_length}, "
                f"reward={episode.total_team_reward:.2f}, outcome={outcome}"
            )
            
            return episode
        
        except Exception as e:
            logger.error(f"Failed to finalize episode {pack_id}: {e}", exc_info=True)
            return None
    
    def get_active_episode_count(self) -> int:
        """Get number of active (ongoing) episodes"""
        return len(self.active_episodes)
    
    def abort_episode(self, pack_id: int) -> bool:
        """
        Abort an active episode (e.g., if data is corrupted)
        
        Args:
            pack_id: Pack episode ID
        
        Returns:
            True if aborted
        """
        if pack_id in self.active_episodes:
            del self.active_episodes[pack_id]
            logger.warning(f"Aborted pack episode {pack_id}")
            return True
        
        return False


# ============================================================================
# UTILITIES
# ============================================================================

def pad_episode_batch(
    episodes: List[PackEpisode],
    max_length: Optional[int] = None,
    max_agents: int = 5
) -> Dict[str, torch.Tensor]:
    """
    Pad and batch episodes for training
    
    Handles variable episode lengths and pack sizes by padding.
    
    Args:
        episodes: List of PackEpisode instances
        max_length: Maximum sequence length (if None, use longest in batch)
        max_agents: Maximum number of agents
    
    Returns:
        Batched tensors dict
    """
    if not episodes:
        return {}
    
    # Determine max length
    if max_length is None:
        max_length = max(ep.episode_length for ep in episodes)
    
    batch_size = len(episodes)
    state_dim = episodes[0].global_states.shape[-1]
    signal_dim = 32  # Fixed
    
    # Initialize padded tensors
    global_states_batch = torch.zeros(batch_size, max_length, max_agents, state_dim)
    local_obs_batch = torch.zeros(batch_size, max_length, max_agents, state_dim)
    actions_batch = torch.zeros(batch_size, max_length, max_agents, dtype=torch.long)
    team_rewards_batch = torch.zeros(batch_size, max_length)
    individual_rewards_batch = torch.zeros(batch_size, max_length, max_agents)
    signals_batch = torch.zeros(batch_size, max_length, max_agents, signal_dim)
    
    # Masks
    length_mask = torch.zeros(batch_size, max_length, dtype=torch.bool)
    agent_mask = torch.zeros(batch_size, max_agents, dtype=torch.bool)
    
    # Fill tensors
    for i, episode in enumerate(episodes):
        ep_len = min(episode.episode_length, max_length)
        n_agents = episode.pack_size
        
        # Copy data
        global_states_batch[i, :ep_len, :n_agents] = torch.from_numpy(
            episode.global_states[:ep_len, :n_agents]
        )
        local_obs_batch[i, :ep_len, :n_agents] = torch.from_numpy(
            episode.local_obs[:ep_len, :n_agents]
        )
        actions_batch[i, :ep_len, :n_agents] = torch.from_numpy(
            episode.actions[:ep_len, :n_agents]
        )
        team_rewards_batch[i, :ep_len] = torch.from_numpy(
            episode.team_rewards[:ep_len]
        )
        individual_rewards_batch[i, :ep_len, :n_agents] = torch.from_numpy(
            episode.individual_rewards[:ep_len, :n_agents]
        )
        
        if episode.signals is not None:
            signals_batch[i, :ep_len, :n_agents] = torch.from_numpy(
                episode.signals[:ep_len, :n_agents]
            )
        
        # Set masks
        length_mask[i, :ep_len] = True
        agent_mask[i, :n_agents] = True
    
    return {
        'global_states': global_states_batch,
        'local_obs': local_obs_batch,
        'actions': actions_batch,
        'team_rewards': team_rewards_batch,
        'individual_rewards': individual_rewards_batch,
        'signals': signals_batch,
        'length_mask': length_mask,
        'agent_mask': agent_mask,
        'batch_size': batch_size,
        'max_length': max_length,
        'max_agents': max_agents
    }


def compute_pack_team_reward(
    individual_metrics: Dict[str, np.ndarray],
    config: Dict[str, float]
) -> float:
    """
    Compute team reward from individual metrics
    
    Team reward composition:
    - Pack survival rate (how many survived)
    - Coordinated kills (kills with 2+ attackers)
    - Formation quality (tight formation bonus)
    - Support effectiveness (helping low-HP allies)
    
    Args:
        individual_metrics: Dict of metric arrays per agent
        config: Reward weights configuration
    
    Returns:
        Team reward (float)
    """
    # Extract metrics
    survivors = individual_metrics.get('survivors', 0)
    total_agents = individual_metrics.get('total_agents', 1)
    coordinated_kills = individual_metrics.get('coordinated_kills', 0)
    position_variance = individual_metrics.get('position_variance', 100.0)
    support_actions = individual_metrics.get('support_actions', 0)
    support_needed = individual_metrics.get('support_needed', 1)
    
    # Get weights from config
    survival_weight = config.get('pack_survival_weight', 10.0)
    coordination_weight = config.get('coordinated_kill_weight', 5.0)
    formation_weight = config.get('formation_quality_weight', 3.0)
    support_weight = config.get('support_effectiveness_weight', 2.0)
    
    # Compute components
    survival_rate = survivors / max(total_agents, 1)
    survival_reward = survival_rate * survival_weight
    
    coordination_reward = coordinated_kills * coordination_weight
    
    formation_quality = 1.0 / (1.0 + position_variance / 100.0)
    formation_reward = formation_quality * formation_weight
    
    support_effectiveness = support_actions / max(support_needed, 1)
    support_reward = support_effectiveness * support_weight
    
    # Total team reward
    team_reward = (
        survival_reward +
        coordination_reward +
        formation_reward +
        support_reward
    )
    
    return team_reward


# ============================================================================
# TESTING
# ============================================================================

def test_pack_episode_buffer():
    """Test pack episode buffer functionality"""
    logger.info("Testing PackEpisodeBuffer...")
    
    # Create buffer
    buffer = PackEpisodeBuffer(max_episodes=100)
    
    # Create sample episodes
    for i in range(50):
        pack_size = np.random.randint(2, 6)
        episode_length = np.random.randint(10, 100)
        
        episode = PackEpisode(
            pack_id=i,
            monster_ids=list(range(i*10, i*10 + pack_size)),
            pack_size=pack_size,
            global_states=np.random.randn(episode_length, pack_size, 64).astype(np.float32),
            local_obs=np.random.randn(episode_length, pack_size, 64).astype(np.float32),
            actions=np.random.randint(0, 7, size=(episode_length, pack_size), dtype=np.int32),
            team_rewards=np.random.randn(episode_length).astype(np.float32),
            individual_rewards=np.random.randn(episode_length, pack_size).astype(np.float32),
            signals=np.random.randn(episode_length, pack_size, 32).astype(np.float32),
            outcome=np.random.choice(['victory', 'defeat', 'retreat'])
        )
        
        buffer.add_episode(episode)
    
    # Test statistics
    stats = buffer.get_pack_statistics()
    logger.info(f"\n✓ Buffer stats: {json.dumps(stats, indent=2)}")
    
    # Test batch sampling
    batch = buffer.get_batch(batch_size=8)
    logger.info(f"\n✓ Sampled batch of {len(batch)} episodes")
    
    # Test pack size specific sampling
    batch_size_3 = buffer.get_batch(batch_size=5, pack_size=3)
    logger.info(f"✓ Sampled {len(batch_size_3)} episodes with pack_size=3")
    
    # Test padding
    padded = pad_episode_batch(batch)
    logger.info(f"\n✓ Padded batch tensors:")
    for key, tensor in padded.items():
        if isinstance(tensor, torch.Tensor):
            logger.info(f"  {key}: {tensor.shape}")
    
    logger.info("\n✓ All PackEpisodeBuffer tests passed!")


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    test_pack_episode_buffer()
