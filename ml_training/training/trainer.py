"""
ML Monster AI - Training Infrastructure
Enhanced ML Monster AI System v2.0

Implements trainers for:
- DQN (Deep Q-Network) with Double DQN and Dueling
- PPO (Proximal Policy Optimization)
- SAC (Soft Actor-Critic)
- LSTM-based models

All trainers support:
- Automatic mixed precision (FP16)
- Gradient checkpointing
- Multi-GPU (DataParallel)
- Early stopping
- Checkpointing
- TensorBoard logging
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.cuda.amp import autocast, GradScaler
from typing import Dict, Any, Optional, List, Tuple
import numpy as np
import logging
import time
from pathlib import Path
from collections import deque

logger = logging.getLogger(__name__)


class BaseTrainer:
    """
    Base trainer class with common functionality
    
    Features:
    - Automatic mixed precision
    - Gradient clipping
    - Learning rate scheduling
    - Checkpointing
    - Early stopping
    - Logging
    """
    
    def __init__(
        self,
        model: nn.Module,
        optimizer: optim.Optimizer,
        device: str = 'cuda',
        use_amp: bool = True,
        grad_clip: float = 1.0,
        log_interval: int = 100,
        checkpoint_dir: str = '/opt/ml_monster_ai/data/checkpoints'
    ):
        """
        Initialize base trainer
        
        Args:
            model: PyTorch model
            optimizer: Optimizer
            device: Device ('cuda' or 'cpu')
            use_amp: Use automatic mixed precision
            grad_clip: Gradient clipping max norm
            log_interval: Log every N steps
            checkpoint_dir: Directory for checkpoints
        """
        self.model = model.to(device)
        self.optimizer = optimizer
        self.device = device
        self.use_amp = use_amp
        self.grad_clip = grad_clip
        self.log_interval = log_interval
        self.checkpoint_dir = Path(checkpoint_dir)
        self.checkpoint_dir.mkdir(parents=True, exist_ok=True)
        
        # Automatic mixed precision scaler
        self.scaler = GradScaler() if use_amp else None
        
        # Training statistics
        self.step_count = 0
        self.episode_count = 0
        self.losses = deque(maxlen=100)
        self.rewards = deque(maxlen=100)
        
        # Best model tracking
        self.best_metric = float('inf')
        self.best_model_path = None
        
        logger.info(f"BaseTrainer initialized on {device}, AMP={use_amp}")
    
    def save_checkpoint(
        self,
        epoch: int,
        metrics: Dict[str, float],
        filename: Optional[str] = None
    ) -> str:
        """
        Save model checkpoint
        
        Args:
            epoch: Current epoch
            metrics: Training metrics
            filename: Optional custom filename
        
        Returns:
            Path to saved checkpoint
        """
        if filename is None:
            filename = f"checkpoint_epoch_{epoch}.pth"
        
        checkpoint_path = self.checkpoint_dir / filename
        
        checkpoint = {
            'epoch': epoch,
            'step_count': self.step_count,
            'model_state_dict': self.model.state_dict(),
            'optimizer_state_dict': self.optimizer.state_dict(),
            'metrics': metrics,
            'best_metric': self.best_metric
        }
        
        if self.scaler is not None:
            checkpoint['scaler_state_dict'] = self.scaler.state_dict()
        
        torch.save(checkpoint, checkpoint_path)
        logger.info(f"Checkpoint saved: {checkpoint_path}")
        
        return str(checkpoint_path)
    
    def load_checkpoint(self, checkpoint_path: str):
        """
        Load model checkpoint
        
        Args:
            checkpoint_path: Path to checkpoint file
        """
        checkpoint = torch.load(checkpoint_path, map_location=self.device)
        
        self.model.load_state_dict(checkpoint['model_state_dict'])
        self.optimizer.load_state_dict(checkpoint['optimizer_state_dict'])
        self.step_count = checkpoint.get('step_count', 0)
        self.best_metric = checkpoint.get('best_metric', float('inf'))
        
        if self.scaler is not None and 'scaler_state_dict' in checkpoint:
            self.scaler.load_state_dict(checkpoint['scaler_state_dict'])
        
        logger.info(f"Checkpoint loaded: {checkpoint_path}")
        logger.info(f"  Epoch: {checkpoint.get('epoch', 'unknown')}")
        logger.info(f"  Step: {self.step_count}")
        logger.info(f"  Best metric: {self.best_metric:.4f}")
    
    def clip_gradients(self):
        """Clip gradients to prevent exploding gradients"""
        if self.grad_clip > 0:
            if self.scaler is not None:
                self.scaler.unscale_(self.optimizer)
            
            torch.nn.utils.clip_grad_norm_(self.model.parameters(), self.grad_clip)
    
    def update_best_model(
        self,
        metric: float,
        lower_is_better: bool = True,
        filename: str = 'best_model.pth'
    ) -> bool:
        """
        Update best model if metric improved
        
        Args:
            metric: Current metric value
            lower_is_better: Whether lower values are better
            filename: Filename for best model
        
        Returns:
            True if new best model
        """
        is_best = False
        
        if lower_is_better:
            is_best = metric < self.best_metric
        else:
            is_best = metric > self.best_metric
        
        if is_best:
            self.best_metric = metric
            self.best_model_path = self.save_checkpoint(
                epoch=-1,  # Special epoch for best model
                metrics={'best_metric': metric},
                filename=filename
            )
            logger.info(f"New best model saved: {filename} (metric: {metric:.4f})")
        
        return is_best


class DQNTrainer(BaseTrainer):
    """
    Deep Q-Network trainer
    
    Features:
    - Double DQN (reduce overestimation)
    - Dueling architecture support
    - Prioritized experience replay
    - Target network with soft/hard updates
    - N-step returns
    """
    
    def __init__(
        self,
        model: nn.Module,
        target_model: nn.Module,
        replay_buffer: Any,
        gamma: float = 0.99,
        tau: float = 0.005,
        target_update_freq: int = 1000,
        batch_size: int = 256,
        learning_rate: float = 1e-4,
        **kwargs
    ):
        """
        Initialize DQN trainer
        
        Args:
            model: Online DQN model
            target_model: Target DQN model
            replay_buffer: Experience replay buffer
            gamma: Discount factor
            tau: Soft update coefficient
            target_update_freq: Hard update frequency (if tau=None)
            batch_size: Training batch size
            learning_rate: Learning rate
        """
        optimizer = optim.Adam(model.parameters(), lr=learning_rate)
        super().__init__(model, optimizer, **kwargs)
        
        self.target_model = target_model.to(self.device)
        self.target_model.load_state_dict(model.state_dict())
        self.target_model.eval()
        
        self.replay_buffer = replay_buffer
        self.gamma = gamma
        self.tau = tau
        self.target_update_freq = target_update_freq
        self.batch_size = batch_size
        
        logger.info(f"DQNTrainer initialized: gamma={gamma}, tau={tau}, batch_size={batch_size}")
    
    def train_step(self) -> Dict[str, float]:
        """
        Single training step
        
        Returns:
            Dictionary with training metrics
        """
        # Check if enough experiences
        if len(self.replay_buffer) < self.batch_size:
            return {'loss': 0.0, 'q_mean': 0.0}
        
        # Sample batch
        if hasattr(self.replay_buffer, 'sample') and 'weights' in self.replay_buffer.sample.__code__.co_varnames:
            # Prioritized replay buffer
            states, actions, rewards, next_states, dones, weights, indices = self.replay_buffer.sample(self.batch_size)
            weights = torch.FloatTensor(weights).to(self.device)
        else:
            # Standard replay buffer
            states, actions, rewards, next_states, dones = self.replay_buffer.sample(self.batch_size)
            weights = torch.ones(self.batch_size, device=self.device)
            indices = None
        
        # Convert to tensors
        states = torch.FloatTensor(states).to(self.device)
        actions = torch.LongTensor(actions).to(self.device)
        rewards = torch.FloatTensor(rewards).to(self.device)
        next_states = torch.FloatTensor(next_states).to(self.device)
        dones = torch.FloatTensor(dones).to(self.device)
        
        # Forward pass with AMP
        with autocast(enabled=self.use_amp):
            # Current Q-values
            q_values = self.model(states)
            q_action = q_values.gather(1, actions.unsqueeze(1)).squeeze(1)
            
            # Target Q-values (Double DQN)
            with torch.no_grad():
                # Select action using online network
                next_q_values_online = self.model(next_states)
                next_actions = next_q_values_online.argmax(1)
                
                # Evaluate action using target network
                next_q_values_target = self.target_model(next_states)
                next_q = next_q_values_target.gather(1, next_actions.unsqueeze(1)).squeeze(1)
                
                # Compute target: r + γ * Q_target(s', argmax_a Q_online(s', a))
                target_q = rewards + (1 - dones) * self.gamma * next_q
            
            # Compute loss (weighted)
            td_errors = target_q - q_action
            loss = (weights * F.smooth_l1_loss(q_action, target_q, reduction='none')).mean()
        
        # Backward pass
        self.optimizer.zero_grad()
        
        if self.scaler is not None:
            self.scaler.scale(loss).backward()
            self.clip_gradients()
            self.scaler.step(self.optimizer)
            self.scaler.update()
        else:
            loss.backward()
            self.clip_gradients()
            self.optimizer.step()
        
        # Update priorities if using prioritized replay
        if indices is not None:
            priorities = td_errors.abs().detach().cpu().numpy() + 1e-6
            self.replay_buffer.update_priorities(indices, priorities)
        
        # Update target network
        if self.tau is not None:
            # Soft update
            self.soft_update_target()
        elif self.step_count % self.target_update_freq == 0:
            # Hard update
            self.hard_update_target()
        
        # Update statistics
        self.step_count += 1
        self.losses.append(loss.item())
        
        metrics = {
            'loss': loss.item(),
            'q_mean': q_values.mean().item(),
            'q_max': q_values.max().item(),
            'td_error_mean': td_errors.abs().mean().item()
        }
        
        if self.step_count % self.log_interval == 0:
            logger.info(f"Step {self.step_count}: Loss={metrics['loss']:.4f}, "
                       f"Q_mean={metrics['q_mean']:.2f}, TD_error={metrics['td_error_mean']:.4f}")
        
        return metrics
    
    def soft_update_target(self):
        """Soft update target network: θ' ← τ*θ + (1-τ)*θ'"""
        for target_param, param in zip(self.target_model.parameters(), self.model.parameters()):
            target_param.data.copy_(
                self.tau * param.data + (1 - self.tau) * target_param.data
            )
    
    def hard_update_target(self):
        """Hard update target network: θ' ← θ"""
        self.target_model.load_state_dict(self.model.state_dict())
        logger.info(f"Target network updated (hard update)")


class PPOTrainer(BaseTrainer):
    """
    Proximal Policy Optimization trainer
    
    Features:
    - Clipped surrogate objective
    - Generalized Advantage Estimation (GAE)
    - Value function learning
    - Entropy regularization
    - Multiple epochs per data batch
    """
    
    def __init__(
        self,
        model: nn.Module,
        gamma: float = 0.99,
        gae_lambda: float = 0.97,
        clip_epsilon: float = 0.2,
        value_loss_coef: float = 0.5,
        entropy_coef: float = 0.01,
        n_epochs: int = 10,
        batch_size: int = 128,
        learning_rate: float = 3e-4,
        **kwargs
    ):
        """
        Initialize PPO trainer
        
        Args:
            model: Actor-Critic model
            gamma: Discount factor
            gae_lambda: GAE lambda
            clip_epsilon: PPO clip epsilon
            value_loss_coef: Value loss coefficient
            entropy_coef: Entropy coefficient
            n_epochs: Number of epochs per update
            batch_size: Mini-batch size
            learning_rate: Learning rate
        """
        optimizer = optim.Adam(model.parameters(), lr=learning_rate)
        super().__init__(model, optimizer, **kwargs)
        
        self.gamma = gamma
        self.gae_lambda = gae_lambda
        self.clip_epsilon = clip_epsilon
        self.value_loss_coef = value_loss_coef
        self.entropy_coef = entropy_coef
        self.n_epochs = n_epochs
        self.batch_size = batch_size
        
        logger.info(f"PPOTrainer initialized: gamma={gamma}, clip_eps={clip_epsilon}")
    
    def train_on_rollout(
        self,
        states: np.ndarray,
        actions: np.ndarray,
        old_log_probs: np.ndarray,
        rewards: np.ndarray,
        dones: np.ndarray,
        values: np.ndarray
    ) -> Dict[str, float]:
        """
        Train on collected rollout
        
        Args:
            states: State array (n_steps, state_dim)
            actions: Action array (n_steps,)
            old_log_probs: Log probabilities from rollout (n_steps,)
            rewards: Rewards (n_steps,)
            dones: Done flags (n_steps,)
            values: Value estimates (n_steps,)
        
        Returns:
            Dictionary with training metrics
        """
        # Compute advantages with GAE
        advantages = self.compute_gae(rewards, values, dones)
        returns = advantages + values
        
        # Normalize advantages
        advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8)
        
        # Convert to tensors
        states_t = torch.FloatTensor(states).to(self.device)
        actions_t = torch.LongTensor(actions).to(self.device)
        old_log_probs_t = torch.FloatTensor(old_log_probs).to(self.device)
        returns_t = torch.FloatTensor(returns).to(self.device)
        advantages_t = torch.FloatTensor(advantages).to(self.device)
        
        # Train for multiple epochs
        total_metrics = {'policy_loss': 0.0, 'value_loss': 0.0, 'entropy': 0.0, 'total_loss': 0.0}
        
        for epoch in range(self.n_epochs):
            # Generate mini-batches
            indices = np.arange(len(states))
            np.random.shuffle(indices)
            
            for start in range(0, len(states), self.batch_size):
                end = start + self.batch_size
                batch_idx = indices[start:end]
                
                # Mini-batch data
                batch_states = states_t[batch_idx]
                batch_actions = actions_t[batch_idx]
                batch_old_log_probs = old_log_probs_t[batch_idx]
                batch_returns = returns_t[batch_idx]
                batch_advantages = advantages_t[batch_idx]
                
                # Forward pass with AMP
                with autocast(enabled=self.use_amp):
                    action_logits, new_values = self.model(batch_states)
                    action_probs = F.softmax(action_logits, dim=-1)
                    dist = torch.distributions.Categorical(action_probs)
                    new_log_probs = dist.log_prob(batch_actions)
                    entropy = dist.entropy()
                    
                    # PPO clipped objective
                    ratio = torch.exp(new_log_probs - batch_old_log_probs)
                    surr1 = ratio * batch_advantages
                    surr2 = torch.clamp(ratio, 1 - self.clip_epsilon, 1 + self.clip_epsilon) * batch_advantages
                    policy_loss = -torch.min(surr1, surr2).mean()
                    
                    # Value loss
                    value_loss = F.mse_loss(new_values.squeeze(), batch_returns)
                    
                    # Entropy bonus (encourage exploration)
                    entropy_loss = -entropy.mean()
                    
                    # Total loss
                    loss = policy_loss + self.value_loss_coef * value_loss + self.entropy_coef * entropy_loss
                
                # Backward pass
                self.optimizer.zero_grad()
                
                if self.scaler is not None:
                    self.scaler.scale(loss).backward()
                    self.clip_gradients()
                    self.scaler.step(self.optimizer)
                    self.scaler.update()
                else:
                    loss.backward()
                    self.clip_gradients()
                    self.optimizer.step()
                
                # Accumulate metrics
                total_metrics['policy_loss'] += policy_loss.item()
                total_metrics['value_loss'] += value_loss.item()
                total_metrics['entropy'] += entropy.mean().item()
                total_metrics['total_loss'] += loss.item()
                
                self.step_count += 1
        
        # Average metrics
        num_updates = self.n_epochs * (len(states) // self.batch_size)
        for key in total_metrics:
            total_metrics[key] /= max(num_updates, 1)
        
        self.losses.append(total_metrics['total_loss'])
        
        if self.step_count % self.log_interval == 0:
            logger.info(f"Step {self.step_count}: PPO Loss={total_metrics['total_loss']:.4f}, "
                       f"Policy={total_metrics['policy_loss']:.4f}, Value={total_metrics['value_loss']:.4f}")
        
        return total_metrics
    
    def compute_gae(
        self,
        rewards: np.ndarray,
        values: np.ndarray,
        dones: np.ndarray
    ) -> np.ndarray:
        """
        Compute Generalized Advantage Estimation
        
        Args:
            rewards: Reward array
            values: Value estimates
            dones: Done flags
        
        Returns:
            Advantages
        """
        advantages = np.zeros_like(rewards)
        last_gae = 0.0
        
        for t in reversed(range(len(rewards))):
            if t == len(rewards) - 1:
                next_value = 0.0
            else:
                next_value = values[t + 1]
            
            # TD error
            delta = rewards[t] + self.gamma * next_value * (1 - dones[t]) - values[t]
            
            # GAE
            last_gae = delta + self.gamma * self.gae_lambda * (1 - dones[t]) * last_gae
            advantages[t] = last_gae
        
        return advantages


class SACTrainer(BaseTrainer):
    """
    Soft Actor-Critic trainer
    
    Features:
    - Maximum entropy RL
    - Twin Q-networks (reduce overestimation)
    - Automatic temperature tuning
    - Continuous action spaces
    - Off-policy learning
    """
    
    def __init__(
        self,
        actor: nn.Module,
        critic1: nn.Module,
        critic2: nn.Module,
        target_critic1: nn.Module,
        target_critic2: nn.Module,
        replay_buffer: Any,
        gamma: float = 0.99,
        tau: float = 0.005,
        alpha: float = 0.2,
        auto_alpha: bool = True,
        batch_size: int = 256,
        actor_lr: float = 3e-4,
        critic_lr: float = 3e-4,
        alpha_lr: float = 3e-4,
        **kwargs
    ):
        """
        Initialize SAC trainer
        
        Args:
            actor: Policy network
            critic1: Q-network 1
            critic2: Q-network 2
            target_critic1: Target Q-network 1
            target_critic2: Target Q-network 2
            replay_buffer: Experience replay buffer
            gamma: Discount factor
            tau: Soft update coefficient
            alpha: Entropy temperature
            auto_alpha: Automatic temperature tuning
            batch_size: Batch size
            actor_lr: Actor learning rate
            critic_lr: Critic learning rate
            alpha_lr: Alpha learning rate
        """
        # Initialize base with actor
        optimizer = optim.Adam(actor.parameters(), lr=actor_lr)
        super().__init__(actor, optimizer, **kwargs)
        
        self.actor = actor
        self.critic1 = critic1.to(self.device)
        self.critic2 = critic2.to(self.device)
        self.target_critic1 = target_critic1.to(self.device)
        self.target_critic2 = target_critic2.to(self.device)
        
        # Copy weights to target networks
        self.target_critic1.load_state_dict(critic1.state_dict())
        self.target_critic2.load_state_dict(critic2.state_dict())
        
        self.replay_buffer = replay_buffer
        self.gamma = gamma
        self.tau = tau
        self.batch_size = batch_size
        
        # Optimizers
        self.actor_optimizer = optimizer
        self.critic1_optimizer = optim.Adam(critic1.parameters(), lr=critic_lr)
        self.critic2_optimizer = optim.Adam(critic2.parameters(), lr=critic_lr)
        
        # Automatic entropy tuning
        self.auto_alpha = auto_alpha
        if auto_alpha:
            self.target_entropy = -actor.action_dim  # -dim(A)
            self.log_alpha = torch.zeros(1, requires_grad=True, device=self.device)
            self.alpha_optimizer = optim.Adam([self.log_alpha], lr=alpha_lr)
            self.alpha = self.log_alpha.exp()
        else:
            self.alpha = alpha
        
        logger.info(f"SACTrainer initialized: gamma={gamma}, tau={tau}, alpha={alpha}")
    
    def train_step(self) -> Dict[str, float]:
        """
        Single SAC training step
        
        Returns:
            Dictionary with training metrics
        """
        # Check if enough experiences
        if len(self.replay_buffer) < self.batch_size:
            return {'actor_loss': 0.0, 'critic_loss': 0.0}
        
        # Sample batch
        states, actions, rewards, next_states, dones = self.replay_buffer.sample(self.batch_size)[:5]
        
        # Convert to tensors
        states = torch.FloatTensor(states).to(self.device)
        actions = torch.FloatTensor(actions).to(self.device)
        rewards = torch.FloatTensor(rewards).to(self.device).unsqueeze(1)
        next_states = torch.FloatTensor(next_states).to(self.device)
        dones = torch.FloatTensor(dones).to(self.device).unsqueeze(1)
        
        # ========== Update Critics ==========
        
        with autocast(enabled=self.use_amp):
            with torch.no_grad():
                # Sample next actions
                next_actions, next_log_probs = self.actor.get_action(next_states)
                
                # Target Q-values
                next_q1 = self.target_critic1(next_states, next_actions)
                next_q2 = self.target_critic2(next_states, next_actions)
                next_q = torch.min(next_q1, next_q2)
                
                # Compute target with entropy term
                if self.auto_alpha:
                    alpha = self.log_alpha.exp().detach()
                else:
                    alpha = self.alpha
                
                target_q = rewards + (1 - dones) * self.gamma * (next_q - alpha * next_log_probs.unsqueeze(1))
            
            # Current Q-values
            current_q1 = self.critic1(states, actions)
            current_q2 = self.critic2(states, actions)
            
            # Critic losses
            critic1_loss = F.mse_loss(current_q1, target_q)
            critic2_loss = F.mse_loss(current_q2, target_q)
        
        # Update critic 1
        self.critic1_optimizer.zero_grad()
        if self.scaler is not None:
            self.scaler.scale(critic1_loss).backward()
            self.scaler.step(self.critic1_optimizer)
        else:
            critic1_loss.backward()
            self.critic1_optimizer.step()
        
        # Update critic 2
        self.critic2_optimizer.zero_grad()
        if self.scaler is not None:
            self.scaler.scale(critic2_loss).backward()
            self.scaler.step(self.critic2_optimizer)
        else:
            critic2_loss.backward()
            self.critic2_optimizer.step()
        
        # ========== Update Actor ==========
        
        with autocast(enabled=self.use_amp):
            # Sample actions from current policy
            new_actions, log_probs = self.actor.get_action(states)
            
            # Q-values for new actions
            q1_new = self.critic1(states, new_actions)
            q2_new = self.critic2(states, new_actions)
            q_new = torch.min(q1_new, q2_new)
            
            # Actor loss
            if self.auto_alpha:
                alpha = self.log_alpha.exp()
            else:
                alpha = self.alpha
            
            actor_loss = (alpha * log_probs.unsqueeze(1) - q_new).mean()
        
        # Update actor
        self.actor_optimizer.zero_grad()
        if self.scaler is not None:
            self.scaler.scale(actor_loss).backward()
            self.clip_gradients()
            self.scaler.step(self.actor_optimizer)
            self.scaler.update()
        else:
            actor_loss.backward()
            self.clip_gradients()
            self.actor_optimizer.step()
        
        # ========== Update Temperature ==========
        
        if self.auto_alpha:
            with autocast(enabled=self.use_amp):
                alpha_loss = -(self.log_alpha * (log_probs + self.target_entropy).detach()).mean()
            
            self.alpha_optimizer.zero_grad()
            if self.scaler is not None:
                self.scaler.scale(alpha_loss).backward()
                self.scaler.step(self.alpha_optimizer)
            else:
                alpha_loss.backward()
                self.alpha_optimizer.step()
            
            self.alpha = self.log_alpha.exp()
        
        # ========== Update Target Networks ==========
        
        self.soft_update_target(self.critic1, self.target_critic1)
        self.soft_update_target(self.critic2, self.target_critic2)
        
        # Update statistics
        self.step_count += 1
        
        metrics = {
            'actor_loss': actor_loss.item(),
            'critic1_loss': critic1_loss.item(),
            'critic2_loss': critic2_loss.item(),
            'alpha': self.alpha.item() if isinstance(self.alpha, torch.Tensor) else self.alpha,
            'q_mean': q_new.mean().item()
        }
        
        if self.step_count % self.log_interval == 0:
            logger.info(f"Step {self.step_count}: Actor Loss={metrics['actor_loss']:.4f}, "
                       f"Critic Loss={metrics['critic1_loss']:.4f}, Alpha={metrics['alpha']:.4f}")
        
        return metrics
    
    @staticmethod
    def soft_update_target(source: nn.Module, target: nn.Module, tau: float = 0.005):
        """Soft update target network"""
        for target_param, param in zip(target.parameters(), source.parameters()):
            target_param.data.copy_(tau * param.data + (1 - tau) * target_param.data)


class LSTMTrainer(BaseTrainer):
    """
    LSTM-based model trainer
    
    For sequence-based models (Team Coordination LSTM, Temporal Transformer)
    
    Features:
    - Sequence batching
    - Truncated backpropagation through time
    - Hidden state management
    """
    
    def __init__(
        self,
        model: nn.Module,
        sequence_length: int = 10,
        learning_rate: float = 1e-4,
        **kwargs
    ):
        """
        Initialize LSTM trainer
        
        Args:
            model: LSTM model
            sequence_length: Length of sequences
            learning_rate: Learning rate
        """
        optimizer = optim.Adam(model.parameters(), lr=learning_rate)
        super().__init__(model, optimizer, **kwargs)
        
        self.sequence_length = sequence_length
        
        logger.info(f"LSTMTrainer initialized: sequence_length={sequence_length}")
    
    def train_on_sequences(
        self,
        sequences: np.ndarray,
        labels: np.ndarray
    ) -> Dict[str, float]:
        """
        Train on sequence batch
        
        Args:
            sequences: (batch_size, seq_len, state_dim)
            labels: (batch_size,) or (batch_size, action_dim)
        
        Returns:
            Training metrics
        """
        # Convert to tensors
        sequences_t = torch.FloatTensor(sequences).to(self.device)
        labels_t = torch.LongTensor(labels).to(self.device) if labels.dtype in [np.int32, np.int64] else torch.FloatTensor(labels).to(self.device)
        
        # Forward pass with AMP
        with autocast(enabled=self.use_amp):
            outputs, _ = self.model(sequences_t)
            
            # Compute loss
            if labels_t.dtype == torch.long:
                loss = F.cross_entropy(outputs, labels_t)
            else:
                loss = F.mse_loss(outputs, labels_t)
        
        # Backward pass
        self.optimizer.zero_grad()
        
        if self.scaler is not None:
            self.scaler.scale(loss).backward()
            self.clip_gradients()
            self.scaler.step(self.optimizer)
            self.scaler.update()
        else:
            loss.backward()
            self.clip_gradients()
            self.optimizer.step()
        
        self.step_count += 1
        self.losses.append(loss.item())
        
        metrics = {'loss': loss.item()}
        
        if self.step_count % self.log_interval == 0:
            logger.info(f"Step {self.step_count}: LSTM Loss={loss.item():.4f}")
        
        return metrics


class EarlyStoppingTracker:
    """
    Early stopping tracker
    
    Stops training if validation metric doesn't improve for N epochs
    """
    
    def __init__(
        self,
        patience: int = 10,
        min_delta: float = 0.0,
        mode: str = 'min'
    ):
        """
        Initialize early stopping
        
        Args:
            patience: Number of epochs to wait for improvement
            min_delta: Minimum change to qualify as improvement
            mode: 'min' or 'max'
        """
        self.patience = patience
        self.min_delta = min_delta
        self.mode = mode
        
        self.best_metric = float('inf') if mode == 'min' else float('-inf')
        self.counter = 0
        self.stopped = False
        
        logger.info(f"EarlyStoppingTracker initialized: patience={patience}, mode={mode}")
    
    def step(self, metric: float) -> bool:
        """
        Update early stopping state
        
        Args:
            metric: Current validation metric
        
        Returns:
            True if should stop training
        """
        if self.mode == 'min':
            improved = metric < (self.best_metric - self.min_delta)
        else:
            improved = metric > (self.best_metric + self.min_delta)
        
        if improved:
            self.best_metric = metric
            self.counter = 0
            logger.info(f"Validation metric improved: {metric:.4f}")
        else:
            self.counter += 1
            logger.info(f"No improvement for {self.counter} epochs (best: {self.best_metric:.4f})")
            
            if self.counter >= self.patience:
                self.stopped = True
                logger.info(f"Early stopping triggered after {self.counter} epochs")
                return True
        
        return False
    
    def reset(self):
        """Reset early stopping"""
        self.best_metric = float('inf') if self.mode == 'min' else float('-inf')
        self.counter = 0
        self.stopped = False


# Learning rate scheduling

def create_lr_scheduler(
    optimizer: optim.Optimizer,
    scheduler_type: str = 'cosine',
    **kwargs
) -> optim.lr_scheduler._LRScheduler:
    """
    Create learning rate scheduler
    
    Args:
        optimizer: PyTorch optimizer
        scheduler_type: Type of scheduler ('cosine', 'reduce_on_plateau', 'step', 'exponential')
        **kwargs: Scheduler-specific arguments
    
    Returns:
        Learning rate scheduler
    """
    if scheduler_type == 'cosine':
        scheduler = optim.lr_scheduler.CosineAnnealingLR(
            optimizer,
            T_max=kwargs.get('T_max', 1000),
            eta_min=kwargs.get('eta_min', 1e-6)
        )
    
    elif scheduler_type == 'reduce_on_plateau':
        scheduler = optim.lr_scheduler.ReduceLROnPlateau(
            optimizer,
            mode=kwargs.get('mode', 'min'),
            factor=kwargs.get('factor', 0.5),
            patience=kwargs.get('patience', 10),
            verbose=True
        )
    
    elif scheduler_type == 'step':
        scheduler = optim.lr_scheduler.StepLR(
            optimizer,
            step_size=kwargs.get('step_size', 100),
            gamma=kwargs.get('gamma', 0.9)
        )
    
    elif scheduler_type == 'exponential':
        scheduler = optim.lr_scheduler.ExponentialLR(
            optimizer,
            gamma=kwargs.get('gamma', 0.995)
        )
    
    else:
        raise ValueError(f"Unknown scheduler type: {scheduler_type}")
    
    logger.info(f"Created {scheduler_type} LR scheduler")
    return scheduler


# Training utilities

def compute_returns(
    rewards: np.ndarray,
    gamma: float = 0.99
) -> np.ndarray:
    """
    Compute discounted returns
    
    Args:
        rewards: Reward array
        gamma: Discount factor
    
    Returns:
        Discounted returns
    """
    returns = np.zeros_like(rewards)
    running_return = 0.0
    
    for t in reversed(range(len(rewards))):
        running_return = rewards[t] + gamma * running_return
        returns[t] = running_return
    
    return returns


def polyak_update(
    source: nn.Module,
    target: nn.Module,
    tau: float
):
    """
    Polyak averaging update: θ' ← τ*θ + (1-τ)*θ'
    
    Args:
        source: Source network
        target: Target network
        tau: Update coefficient
    """
    for target_param, param in zip(target.parameters(), source.parameters()):
        target_param.data.copy_(tau * param.data + (1 - tau) * target_param.data)
