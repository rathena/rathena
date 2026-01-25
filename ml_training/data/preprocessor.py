"""
ML Monster AI - Data Preprocessing
Enhanced ML Monster AI System v2.0

Implements:
- State normalization (running mean/std)
- Reward clipping and normalization
- State augmentation
- Data validation
"""

import numpy as np
import torch
from typing import Tuple, Optional, Dict, List, Any
import logging
import pickle

logger = logging.getLogger(__name__)


class StatePreprocessor:
    """
    State vector preprocessor
    
    Features:
    - Running mean/std normalization
    - Outlier clipping
    - NaN/Inf handling
    - State validation
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        clip_range: Tuple[float, float] = (-10.0, 10.0),
        epsilon: float = 1e-8
    ):
        """
        Initialize preprocessor
        
        Args:
            state_dim: State vector dimension
            clip_range: Range to clip normalized states
            epsilon: Small constant for numerical stability
        """
        self.state_dim = state_dim
        self.clip_range = clip_range
        self.epsilon = epsilon
        
        # Running statistics
        self.mean = np.zeros(state_dim, dtype=np.float32)
        self.var = np.ones(state_dim, dtype=np.float32)
        self.count = 0
        
        logger.info(f"StatePreprocessor initialized: state_dim={state_dim}")
    
    def update_stats(self, state: np.ndarray):
        """
        Update running mean and variance
        
        Uses Welford's online algorithm for numerical stability
        
        Args:
            state: State vector (state_dim,) or batch (batch_size, state_dim)
        """
        if state.ndim == 1:
            state = state.reshape(1, -1)
        
        batch_mean = np.mean(state, axis=0)
        batch_var = np.var(state, axis=0)
        batch_count = state.shape[0]
        
        # Update running statistics
        delta = batch_mean - self.mean
        total_count = self.count + batch_count
        
        self.mean = self.mean + delta * batch_count / total_count
        
        m_a = self.var * self.count
        m_b = batch_var * batch_count
        M2 = m_a + m_b + np.square(delta) * self.count * batch_count / total_count
        
        self.var = M2 / total_count
        self.count = total_count
    
    def normalize(
        self,
        state: np.ndarray,
        update_stats: bool = True
    ) -> np.ndarray:
        """
        Normalize state using running statistics
        
        Args:
            state: State vector
            update_stats: Whether to update running statistics
        
        Returns:
            Normalized state
        """
        # Validate state
        if not self._validate_state(state):
            logger.warning("Invalid state detected, returning zeros")
            return np.zeros_like(state)
        
        # Update statistics if requested
        if update_stats:
            self.update_stats(state)
        
        # Normalize: (x - mean) / sqrt(var + epsilon)
        normalized = (state - self.mean) / np.sqrt(self.var + self.epsilon)
        
        # Clip to range
        normalized = np.clip(normalized, self.clip_range[0], self.clip_range[1])
        
        return normalized.astype(np.float32)
    
    def denormalize(self, normalized_state: np.ndarray) -> np.ndarray:
        """
        Denormalize state back to original scale
        
        Args:
            normalized_state: Normalized state vector
        
        Returns:
            Original scale state
        """
        return normalized_state * np.sqrt(self.var + self.epsilon) + self.mean
    
    def _validate_state(self, state: np.ndarray) -> bool:
        """
        Validate state vector
        
        Checks for:
        - Correct shape
        - NaN values
        - Inf values
        - Extreme outliers
        
        Returns:
            True if valid, False otherwise
        """
        # Check shape
        if state.shape[-1] != self.state_dim:
            logger.error(f"Invalid state shape: {state.shape}, expected (..., {self.state_dim})")
            return False
        
        # Check for NaN
        if np.isnan(state).any():
            logger.error("State contains NaN values")
            return False
        
        # Check for Inf
        if np.isinf(state).any():
            logger.error("State contains Inf values")
            return False
        
        # Check for extreme outliers (> 1000 std from mean)
        if self.count > 100:  # Only check after seeing enough data
            z_scores = np.abs(state - self.mean) / (np.sqrt(self.var) + self.epsilon)
            if (z_scores > 1000).any():
                logger.warning(f"State contains extreme outliers: max z-score = {z_scores.max():.2f}")
        
        return True
    
    def reset(self):
        """Reset statistics"""
        self.mean = np.zeros(self.state_dim, dtype=np.float32)
        self.var = np.ones(self.state_dim, dtype=np.float32)
        self.count = 0
        logger.info("StatePreprocessor statistics reset")
    
    def save(self, path: str):
        """Save preprocessor statistics"""
        data = {
            'mean': self.mean,
            'var': self.var,
            'count': self.count,
            'state_dim': self.state_dim,
            'clip_range': self.clip_range,
            'epsilon': self.epsilon
        }
        
        with open(path, 'wb') as f:
            pickle.dump(data, f)
        
        logger.info(f"StatePreprocessor saved to {path}")
    
    def load(self, path: str):
        """Load preprocessor statistics"""
        with open(path, 'rb') as f:
            data = pickle.load(f)
        
        self.mean = data['mean']
        self.var = data['var']
        self.count = data['count']
        self.state_dim = data['state_dim']
        self.clip_range = data['clip_range']
        self.epsilon = data['epsilon']
        
        logger.info(f"StatePreprocessor loaded from {path} (count={self.count})")


class RewardNormalizer:
    """
    Reward normalization and clipping
    
    Features:
    - Running mean/std normalization
    - Reward clipping
    - Reward shaping
    """
    
    def __init__(
        self,
        clip_range: Tuple[float, float] = (-10.0, 10.0),
        normalize: bool = True,
        epsilon: float = 1e-8
    ):
        """
        Initialize reward normalizer
        
        Args:
            clip_range: Range to clip rewards
            normalize: Whether to normalize rewards
            epsilon: Small constant for stability
        """
        self.clip_range = clip_range
        self.normalize = normalize
        self.epsilon = epsilon
        
        # Running statistics
        self.mean = 0.0
        self.var = 1.0
        self.count = 0
        
        logger.info(f"RewardNormalizer initialized: clip_range={clip_range}, normalize={normalize}")
    
    def update_stats(self, reward: float):
        """Update running statistics"""
        self.count += 1
        
        delta = reward - self.mean
        self.mean += delta / self.count
        
        delta2 = reward - self.mean
        self.var += (delta * delta2 - self.var) / self.count
    
    def normalize_reward(
        self,
        reward: float,
        update_stats: bool = True
    ) -> float:
        """
        Normalize and clip reward
        
        Args:
            reward: Raw reward
            update_stats: Whether to update running statistics
        
        Returns:
            Processed reward
        """
        # Update statistics
        if update_stats:
            self.update_stats(reward)
        
        # Normalize
        if self.normalize and self.count > 10:  # Need some data first
            normalized = (reward - self.mean) / (np.sqrt(self.var) + self.epsilon)
        else:
            normalized = reward
        
        # Clip
        clipped = np.clip(normalized, self.clip_range[0], self.clip_range[1])
        
        return float(clipped)
    
    def reset(self):
        """Reset statistics"""
        self.mean = 0.0
        self.var = 1.0
        self.count = 0
        logger.info("RewardNormalizer reset")
    
    def save(self, path: str):
        """Save normalizer statistics"""
        data = {
            'mean': self.mean,
            'var': self.var,
            'count': self.count,
            'clip_range': self.clip_range,
            'normalize': self.normalize,
            'epsilon': self.epsilon
        }
        
        with open(path, 'wb') as f:
            pickle.dump(data, f)
        
        logger.info(f"RewardNormalizer saved to {path}")
    
    def load(self, path: str):
        """Load normalizer statistics"""
        with open(path, 'rb') as f:
            data = pickle.load(f)
        
        self.mean = data['mean']
        self.var = data['var']
        self.count = data['count']
        self.clip_range = data['clip_range']
        self.normalize = data['normalize']
        self.epsilon = data['epsilon']
        
        logger.info(f"RewardNormalizer loaded from {path} (count={self.count})")


class StateAugmentor:
    """
    State augmentation for data efficiency
    
    Techniques:
    - Gaussian noise injection
    - Small perturbations
    - Symmetry transformations (for applicable states)
    """
    
    def __init__(
        self,
        noise_std: float = 0.01,
        perturbation_prob: float = 0.1
    ):
        """
        Initialize augmentor
        
        Args:
            noise_std: Standard deviation of Gaussian noise
            perturbation_prob: Probability of applying perturbation
        """
        self.noise_std = noise_std
        self.perturbation_prob = perturbation_prob
        
        logger.info(f"StateAugmentor initialized: noise_std={noise_std}")
    
    def augment(self, state: np.ndarray) -> np.ndarray:
        """
        Apply augmentation to state
        
        Args:
            state: State vector
        
        Returns:
            Augmented state
        """
        augmented = state.copy()
        
        # Apply with probability
        if np.random.random() < self.perturbation_prob:
            # Add Gaussian noise
            noise = np.random.normal(0, self.noise_std, size=state.shape)
            augmented = state + noise
        
        return augmented.astype(np.float32)
    
    def augment_batch(self, states: np.ndarray) -> np.ndarray:
        """
        Augment batch of states
        
        Args:
            states: (batch_size, state_dim)
        
        Returns:
            Augmented states
        """
        return np.array([self.augment(s) for s in states])


class TrajectorySegmenter:
    """
    Segment trajectories into episodes and sub-episodes
    
    Useful for hierarchical RL and temporal abstraction
    """
    
    def __init__(self, max_episode_length: int = 1000):
        """
        Initialize segmenter
        
        Args:
            max_episode_length: Maximum episode length before forced segmentation
        """
        self.max_episode_length = max_episode_length
        self.current_trajectory = []
        
        logger.info(f"TrajectorySegmenter initialized: max_length={max_episode_length}")
    
    def add_transition(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool
    ):
        """Add transition to current trajectory"""
        self.current_trajectory.append({
            'state': state,
            'action': action,
            'reward': reward,
            'next_state': next_state,
            'done': done
        })
        
        # Check if trajectory complete
        if done or len(self.current_trajectory) >= self.max_episode_length:
            return self.finalize_trajectory()
        
        return None
    
    def finalize_trajectory(self) -> Optional[Dict]:
        """
        Finalize current trajectory and return it
        
        Returns:
            Dictionary with trajectory data
        """
        if not self.current_trajectory:
            return None
        
        trajectory = {
            'states': np.array([t['state'] for t in self.current_trajectory]),
            'actions': np.array([t['action'] for t in self.current_trajectory]),
            'rewards': np.array([t['reward'] for t in self.current_trajectory]),
            'next_states': np.array([t['next_state'] for t in self.current_trajectory]),
            'dones': np.array([t['done'] for t in self.current_trajectory]),
            'length': len(self.current_trajectory),
            'total_reward': sum(t['reward'] for t in self.current_trajectory)
        }
        
        # Clear current trajectory
        self.current_trajectory = []
        
        return trajectory
    
    def reset(self):
        """Reset current trajectory"""
        self.current_trajectory = []


class DataValidator:
    """
    Validate training data quality
    
    Checks:
    - State vector validity
    - Reward range
    - Action validity
    - Data distribution
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        action_dim: int = 10,
        reward_range: Tuple[float, float] = (-1000.0, 1000.0)
    ):
        """
        Initialize validator
        
        Args:
            state_dim: Expected state dimension
            action_dim: Expected action dimension
            reward_range: Valid reward range
        """
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.reward_range = reward_range
        
        # Statistics
        self.total_samples = 0
        self.invalid_samples = 0
        self.warnings = []
        
        logger.info(f"DataValidator initialized")
    
    def validate_experience(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool
    ) -> Tuple[bool, List[str]]:
        """
        Validate single experience
        
        Args:
            state: State vector
            action: Action
            reward: Reward
            next_state: Next state
            done: Done flag
        
        Returns:
            (is_valid, error_messages)
        """
        errors = []
        self.total_samples += 1
        
        # Validate state
        if state.shape != (self.state_dim,):
            errors.append(f"Invalid state shape: {state.shape}, expected ({self.state_dim},)")
        
        if np.isnan(state).any():
            errors.append("State contains NaN")
        
        if np.isinf(state).any():
            errors.append("State contains Inf")
        
        # Validate next_state
        if next_state.shape != (self.state_dim,):
            errors.append(f"Invalid next_state shape: {next_state.shape}")
        
        if np.isnan(next_state).any():
            errors.append("Next state contains NaN")
        
        if np.isinf(next_state).any():
            errors.append("Next state contains Inf")
        
        # Validate action
        if not (0 <= action < self.action_dim):
            errors.append(f"Invalid action: {action}, must be in [0, {self.action_dim})")
        
        # Validate reward
        if not (self.reward_range[0] <= reward <= self.reward_range[1]):
            errors.append(f"Reward {reward} outside valid range {self.reward_range}")
        
        if np.isnan(reward) or np.isinf(reward):
            errors.append(f"Invalid reward: {reward}")
        
        # Validate done
        if not isinstance(done, (bool, np.bool_, int, np.integer)):
            errors.append(f"Invalid done type: {type(done)}")
        
        is_valid = len(errors) == 0
        
        if not is_valid:
            self.invalid_samples += 1
            for error in errors:
                logger.debug(f"Validation error: {error}")
        
        return is_valid, errors
    
    def validate_batch(
        self,
        states: np.ndarray,
        actions: np.ndarray,
        rewards: np.ndarray,
        next_states: np.ndarray,
        dones: np.ndarray
    ) -> Tuple[bool, int, List[str]]:
        """
        Validate batch of experiences
        
        Returns:
            (all_valid, num_invalid, error_summary)
        """
        batch_size = len(states)
        num_invalid = 0
        all_errors = set()
        
        for i in range(batch_size):
            is_valid, errors = self.validate_experience(
                states[i],
                actions[i],
                rewards[i],
                next_states[i],
                dones[i]
            )
            
            if not is_valid:
                num_invalid += 1
                all_errors.update(errors)
        
        all_valid = num_invalid == 0
        
        if not all_valid:
            logger.warning(f"Batch validation: {num_invalid}/{batch_size} invalid samples")
        
        return all_valid, num_invalid, list(all_errors)
    
    def get_stats(self) -> Dict[str, Any]:
        """Get validation statistics"""
        return {
            'total_samples': self.total_samples,
            'invalid_samples': self.invalid_samples,
            'error_rate': self.invalid_samples / max(self.total_samples, 1),
            'warnings': self.warnings[-10:]  # Last 10 warnings
        }
    
    def reset_stats(self):
        """Reset validation statistics"""
        self.total_samples = 0
        self.invalid_samples = 0
        self.warnings = []
        logger.info("DataValidator statistics reset")


def compute_n_step_returns(
    rewards: np.ndarray,
    dones: np.ndarray,
    n_step: int = 5,
    gamma: float = 0.99
) -> np.ndarray:
    """
    Compute n-step returns
    
    R_t^n = r_t + γ*r_{t+1} + γ²*r_{t+2} + ... + γ^{n-1}*r_{t+n-1}
    
    Args:
        rewards: Array of rewards
        dones: Array of done flags
        n_step: Number of steps
        gamma: Discount factor
    
    Returns:
        Array of n-step returns
    """
    n_step_returns = np.zeros_like(rewards)
    
    for t in range(len(rewards)):
        n_step_return = 0.0
        discount = 1.0
        
        for i in range(n_step):
            if t + i >= len(rewards):
                break
            
            n_step_return += discount * rewards[t + i]
            discount *= gamma
            
            if dones[t + i]:
                break
        
        n_step_returns[t] = n_step_return
    
    return n_step_returns


def compute_gae(
    rewards: np.ndarray,
    values: np.ndarray,
    dones: np.ndarray,
    gamma: float = 0.99,
    gae_lambda: float = 0.97
) -> np.ndarray:
    """
    Compute Generalized Advantage Estimation (GAE)
    
    GAE(γ, λ) provides a balance between bias and variance
    
    Args:
        rewards: Array of rewards
        values: Array of value estimates
        dones: Array of done flags
        gamma: Discount factor
        gae_lambda: GAE lambda parameter
    
    Returns:
        Array of advantages
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
    
    return advantages


def normalize_advantages(advantages: np.ndarray, epsilon: float = 1e-8) -> np.ndarray:
    """
    Normalize advantages to have zero mean and unit variance
    
    Args:
        advantages: Array of advantages
        epsilon: Small constant for stability
    
    Returns:
        Normalized advantages
    """
    mean = np.mean(advantages)
    std = np.std(advantages)
    
    normalized = (advantages - mean) / (std + epsilon)
    
    return normalized


class ExperienceCollector:
    """
    Collect experiences during episodes
    
    Helper class for organizing data collection
    """
    
    def __init__(self, state_dim: int = 64):
        """
        Initialize collector
        
        Args:
            state_dim: State dimension
        """
        self.state_dim = state_dim
        self.reset()
        
        logger.info(f"ExperienceCollector initialized")
    
    def reset(self):
        """Reset for new episode"""
        self.states = []
        self.actions = []
        self.rewards = []
        self.next_states = []
        self.dones = []
        self.values = []
        self.log_probs = []
        self.infos = []
    
    def add(
        self,
        state: np.ndarray,
        action: int,
        reward: float,
        next_state: np.ndarray,
        done: bool,
        value: Optional[float] = None,
        log_prob: Optional[float] = None,
        info: Optional[Dict] = None
    ):
        """Add transition"""
        self.states.append(state)
        self.actions.append(action)
        self.rewards.append(reward)
        self.next_states.append(next_state)
        self.dones.append(done)
        
        if value is not None:
            self.values.append(value)
        
        if log_prob is not None:
            self.log_probs.append(log_prob)
        
        if info is not None:
            self.infos.append(info)
    
    def get_episode(self) -> Dict[str, np.ndarray]:
        """
        Get collected episode data
        
        Returns:
            Dictionary with episode data
        """
        episode = {
            'states': np.array(self.states, dtype=np.float32),
            'actions': np.array(self.actions, dtype=np.int64),
            'rewards': np.array(self.rewards, dtype=np.float32),
            'next_states': np.array(self.next_states, dtype=np.float32),
            'dones': np.array(self.dones, dtype=np.float32),
            'length': len(self.states),
            'total_reward': sum(self.rewards)
        }
        
        if self.values:
            episode['values'] = np.array(self.values, dtype=np.float32)
        
        if self.log_probs:
            episode['log_probs'] = np.array(self.log_probs, dtype=np.float32)
        
        if self.infos:
            episode['infos'] = self.infos
        
        return episode
    
    def __len__(self) -> int:
        return len(self.states)


# Batch processing utilities

def collate_experiences(experiences: List[Experience]) -> Dict[str, np.ndarray]:
    """
    Collate list of experiences into batch arrays
    
    Args:
        experiences: List of Experience objects
    
    Returns:
        Dictionary with batched arrays
    """
    states = np.array([exp.state for exp in experiences], dtype=np.float32)
    actions = np.array([exp.action for exp in experiences], dtype=np.int64)
    rewards = np.array([exp.reward for exp in experiences], dtype=np.float32)
    next_states = np.array([exp.next_state for exp in experiences], dtype=np.float32)
    dones = np.array([exp.done for exp in experiences], dtype=np.float32)
    
    return {
        'states': states,
        'actions': actions,
        'rewards': rewards,
        'next_states': next_states,
        'dones': dones
    }


def to_tensor(
    array: np.ndarray,
    device: str = 'cuda',
    dtype: torch.dtype = torch.float32
) -> torch.Tensor:
    """
    Convert numpy array to PyTorch tensor
    
    Args:
        array: Numpy array
        device: Device to place tensor on
        dtype: Tensor dtype
    
    Returns:
        PyTorch tensor
    """
    return torch.from_numpy(array).to(device=device, dtype=dtype)


def to_tensors(
    batch: Dict[str, np.ndarray],
    device: str = 'cuda'
) -> Dict[str, torch.Tensor]:
    """
    Convert batch of numpy arrays to tensors
    
    Args:
        batch: Dictionary of numpy arrays
        device: Device to place tensors on
    
    Returns:
        Dictionary of tensors
    """
    tensor_batch = {}
    
    for key, value in batch.items():
        if isinstance(value, np.ndarray):
            # Determine dtype
            if value.dtype in [np.int32, np.int64]:
                dtype = torch.long
            else:
                dtype = torch.float32
            
            tensor_batch[key] = to_tensor(value, device, dtype)
        else:
            tensor_batch[key] = value
    
    return tensor_batch
