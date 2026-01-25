"""
CTDE Trainer - Centralized Training, Decentralized Execution
=============================================================
Trains multi-agent coordination using centralized critic and decentralized actors.

Training Flow:
1. Collect pack episodes (2-5 monsters coordinating)
2. Compute centralized value estimates (critic sees global state)
3. Calculate advantages using GAE
4. Update decentralized actors with policy gradient
5. Update centralized critic with TD error
6. Export individual actors for inference

Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md (Section 6)
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from typing import Dict, List, Any, Optional, Tuple
import numpy as np
import logging
from pathlib import Path
import time
from datetime import datetime
import asyncio

from ..models.ctde_architectures import (
    CentralizedCritic,
    DecentralizedActor,
    CommNet,
    QMIXMixer,
    create_agent_mask
)
from ..data.pack_episode_buffer import (
    PackEpisodeBuffer,
    PackEpisode,
    pad_episode_batch
)

logger = logging.getLogger(__name__)


class CTDETrainer:
    """
    CTDE training with centralized critic and decentralized actors
    
    Algorithm:
    - Centralized Training: Critic sees global state, trains on team performance
    - Decentralized Execution: Actors only see local state + signals
    
    Features:
    - PPO-style policy updates with clipped objective
    - GAE for advantage estimation
    - Entropy regularization for exploration
    - Gradient clipping for stability
    - Separate learning rates for actor and critic
    - Model checkpointing and ONNX export
    """
    
    def __init__(
        self,
        config: Dict[str, Any],
        n_agents: int = 5,
        device: str = 'cuda' if torch.cuda.is_available() else 'cpu'
    ):
        """
        Initialize CTDE trainer
        
        Args:
            config: Training configuration dict
            n_agents: Maximum number of agents in pack
            device: Device to train on ('cuda' or 'cpu')
        """
        self.config = config
        self.n_agents = n_agents
        self.device = torch.device(device)
        
        # Extract config params
        self.state_dim = config.get('state_dim', 64)
        self.signal_dim = config.get('signal_dim', 32)
        self.action_dim = config.get('action_dim', 7)
        
        # Training hyperparameters
        self.gamma = config.get('gamma', 0.99)
        self.gae_lambda = config.get('gae_lambda', 0.95)
        self.clip_epsilon = config.get('clip_epsilon', 0.2)
        self.value_loss_coef = config.get('value_loss_coef', 0.5)
        self.entropy_coef = config.get('entropy_coef', 0.01)
        self.max_grad_norm = config.get('max_grad_norm', 1.0)
        self.n_epochs = config.get('n_epochs', 4)
        self.batch_size = config.get('batch_size', 32)
        
        # Team reward weight
        self.team_reward_weight = config.get('team_reward_weight', 0.7)
        
        # Create models
        logger.info("Initializing CTDE models...")
        
        self.critic = CentralizedCritic(
            state_dim=self.state_dim,
            n_agents=n_agents,
            hidden_dims=config.get('critic_hidden_dims', [512, 512, 256]),
            use_attention=config.get('critic_use_attention', True),
            dropout=config.get('critic_dropout', 0.2)
        ).to(self.device)
        
        self.actor = DecentralizedActor(
            state_dim=self.state_dim,
            signal_dim=self.signal_dim,
            action_dim=self.action_dim,
            hidden_dim=config.get('actor_hidden_dim', 256),
            use_gru=config.get('actor_use_gru', True),
            dropout=config.get('actor_dropout', 0.1)
        ).to(self.device)
        
        # Communication network (optional, for training coordination)
        if config.get('use_communication', True):
            self.comm_net = CommNet(
                hidden_dim=config.get('comm_hidden_dim', 128),
                n_agents=n_agents,
                n_rounds=config.get('comm_rounds', 3),
                dropout=config.get('comm_dropout', 0.1)
            ).to(self.device)
        else:
            self.comm_net = None
        
        # QMIX mixer (optional)
        if config.get('use_qmix', False):
            self.qmix_mixer = QMIXMixer(
                n_agents=n_agents,
                state_dim=self.state_dim,
                mixing_embed_dim=config.get('qmix_mixing_embed', 32)
            ).to(self.device)
        else:
            self.qmix_mixer = None
        
        # Optimizers
        self.critic_optimizer = optim.Adam(
            self.critic.parameters(),
            lr=config.get('critic_lr', 0.0003),
            eps=1e-5
        )
        
        self.actor_optimizer = optim.Adam(
            self.actor.parameters(),
            lr=config.get('actor_lr', 0.0001),
            eps=1e-5
        )
        
        if self.comm_net is not None:
            self.comm_optimizer = optim.Adam(
                self.comm_net.parameters(),
                lr=config.get('comm_lr', 0.0002),
                eps=1e-5
            )
        
        # Training statistics
        self.training_step = 0
        self.episodes_trained = 0
        self.total_actor_loss = 0.0
        self.total_critic_loss = 0.0
        self.total_entropy = 0.0
        
        logger.info(f"CTDETrainer initialized on {device}")
        logger.info(f"  Critic params: {self.critic.count_parameters():,}")
        logger.info(f"  Actor params: {self.actor.count_parameters():,}")
    
    def train_episode(self, episode_data: Dict[str, torch.Tensor]) -> Dict[str, float]:
        """
        Train on a batch of pack episodes
        
        Args:
            episode_data: Padded episode batch from pad_episode_batch()
        
        Returns:
            Training metrics dict
        """
        # Move data to device
        global_states = episode_data['global_states'].to(self.device)
        local_obs = episode_data['local_obs'].to(self.device)
        actions = episode_data['actions'].to(self.device)
        team_rewards = episode_data['team_rewards'].to(self.device)
        individual_rewards = episode_data['individual_rewards'].to(self.device)
        signals = episode_data['signals'].to(self.device)
        length_mask = episode_data['length_mask'].to(self.device)
        agent_mask = episode_data['agent_mask'].to(self.device)
        
        batch_size = episode_data['batch_size']
        max_length = episode_data['max_length']
        
        # Compute returns and advantages
        returns, advantages = self.compute_returns_and_advantages(
            global_states,
            team_rewards,
            individual_rewards,
            length_mask,
            agent_mask
        )
        
        # Multiple epochs over data
        total_actor_loss = 0.0
        total_critic_loss = 0.0
        total_entropy = 0.0
        
        for epoch in range(self.n_epochs):
            # Update critic
            critic_loss = self.update_critic(
                global_states,
                returns,
                length_mask,
                agent_mask
            )
            
            # Update actors
            actor_loss, entropy = self.update_actors(
                local_obs,
                signals,
                actions,
                advantages,
                length_mask,
                agent_mask
            )
            
            total_actor_loss += actor_loss
            total_critic_loss += critic_loss
            total_entropy += entropy
        
        # Average over epochs
        avg_actor_loss = total_actor_loss / self.n_epochs
        avg_critic_loss = total_critic_loss / self.n_epochs
        avg_entropy = total_entropy / self.n_epochs
        
        # Update training stats
        self.training_step += 1
        self.episodes_trained += batch_size
        self.total_actor_loss += avg_actor_loss
        self.total_critic_loss += avg_critic_loss
        self.total_entropy += avg_entropy
        
        metrics = {
            'actor_loss': avg_actor_loss,
            'critic_loss': avg_critic_loss,
            'entropy': avg_entropy,
            'total_loss': avg_actor_loss + self.value_loss_coef * avg_critic_loss,
            'avg_return': float(returns.mean()),
            'avg_advantage': float(advantages.mean()),
            'training_step': self.training_step
        }
        
        logger.debug(
            f"Step {self.training_step}: "
            f"actor_loss={avg_actor_loss:.4f}, "
            f"critic_loss={avg_critic_loss:.4f}, "
            f"entropy={avg_entropy:.4f}"
        )
        
        return metrics
    
    def compute_returns(
        self,
        rewards: torch.Tensor,
        values: torch.Tensor,
        length_mask: torch.Tensor,
        gamma: float = 0.99
    ) -> torch.Tensor:
        """
        Compute discounted returns
        
        Args:
            rewards: (batch, max_length) rewards
            values: (batch, max_length, 1) value estimates
            length_mask: (batch, max_length) valid timestep mask
            gamma: Discount factor
        
        Returns:
            returns: (batch, max_length) discounted returns
        """
        batch_size, max_length = rewards.shape
        returns = torch.zeros_like(rewards)
        
        # Iterate backwards
        for b in range(batch_size):
            running_return = 0.0
            
            for t in reversed(range(max_length)):
                if length_mask[b, t]:
                    running_return = rewards[b, t] + gamma * running_return
                    returns[b, t] = running_return
                else:
                    running_return = 0.0
        
        return returns
    
    def compute_advantages(
        self,
        rewards: torch.Tensor,
        values: torch.Tensor,
        length_mask: torch.Tensor,
        gamma: float = 0.99,
        gae_lambda: float = 0.95
    ) -> torch.Tensor:
        """
        Compute Generalized Advantage Estimation (GAE)
        
        Args:
            rewards: (batch, max_length) rewards
            values: (batch, max_length, 1) value estimates
            length_mask: (batch, max_length) valid timestep mask
            gamma: Discount factor
            gae_lambda: GAE lambda parameter
        
        Returns:
            advantages: (batch, max_length) advantages
        """
        batch_size, max_length = rewards.shape
        advantages = torch.zeros_like(rewards)
        
        values_squeezed = values.squeeze(-1)  # (batch, max_length)
        
        # Iterate backwards to compute GAE
        for b in range(batch_size):
            running_advantage = 0.0
            
            for t in reversed(range(max_length)):
                if not length_mask[b, t]:
                    running_advantage = 0.0
                    continue
                
                # TD error: delta_t = r_t + gamma * V(s_{t+1}) - V(s_t)
                if t < max_length - 1 and length_mask[b, t + 1]:
                    next_value = values_squeezed[b, t + 1]
                else:
                    next_value = 0.0
                
                delta = rewards[b, t] + gamma * next_value - values_squeezed[b, t]
                
                # GAE: A_t = delta_t + gamma * lambda * A_{t+1}
                running_advantage = delta + gamma * gae_lambda * running_advantage
                advantages[b, t] = running_advantage
        
        return advantages
    
    def compute_returns_and_advantages(
        self,
        global_states: torch.Tensor,
        team_rewards: torch.Tensor,
        individual_rewards: torch.Tensor,
        length_mask: torch.Tensor,
        agent_mask: torch.Tensor
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Compute returns and advantages for CTDE
        
        Uses centralized critic to estimate global value,
        then computes advantages for each agent.
        
        Args:
            global_states: (batch, max_length, n_agents, state_dim)
            team_rewards: (batch, max_length) team rewards
            individual_rewards: (batch, max_length, n_agents)
            length_mask: (batch, max_length)
            agent_mask: (batch, n_agents)
        
        Returns:
            returns: (batch, max_length) returns
            advantages: (batch, max_length, n_agents) per-agent advantages
        """
        batch_size, max_length, n_agents, state_dim = global_states.shape
        
        # Compute centralized values for each timestep
        values_list = []
        
        with torch.no_grad():
            for t in range(max_length):
                # Get global state at timestep t
                global_state_t = global_states[:, t, :, :]  # (batch, n_agents, state_dim)
                
                # Compute centralized value
                value_t = self.critic(global_state_t, agent_mask)  # (batch, 1)
                values_list.append(value_t)
        
        values = torch.stack(values_list, dim=1)  # (batch, max_length, 1)
        
        # Combine team and individual rewards
        # Broadcast team reward to all agents
        team_rewards_expanded = team_rewards.unsqueeze(-1).expand(-1, -1, n_agents)
        
        # Combined reward: team_weight * team_reward + (1-team_weight) * individual_reward
        combined_rewards = (
            self.team_reward_weight * team_rewards_expanded +
            (1 - self.team_reward_weight) * individual_rewards
        )
        
        # Average over agents for global return
        # Only average over active agents (use agent_mask)
        agent_mask_expanded = agent_mask.unsqueeze(1).expand(-1, max_length, -1).float()
        masked_rewards = combined_rewards * agent_mask_expanded
        avg_rewards = masked_rewards.sum(dim=-1) / agent_mask_expanded.sum(dim=-1).clamp(min=1)
        
        # Compute returns using team rewards
        returns = self.compute_returns(avg_rewards, values, length_mask, self.gamma)
        
        # Compute per-agent advantages using GAE
        advantages = self.compute_advantages(avg_rewards, values, length_mask, self.gamma, self.gae_lambda)
        
        # Expand advantages to all agents
        advantages_expanded = advantages.unsqueeze(-1).expand(-1, -1, n_agents)
        
        return returns, advantages_expanded
    
    def update_critic(
        self,
        global_states: torch.Tensor,
        returns: torch.Tensor,
        length_mask: torch.Tensor,
        agent_mask: torch.Tensor
    ) -> float:
        """
        Update centralized critic
        
        Args:
            global_states: (batch, max_length, n_agents, state_dim)
            returns: (batch, max_length) target returns
            length_mask: (batch, max_length)
            agent_mask: (batch, n_agents)
        
        Returns:
            Critic loss value
        """
        batch_size, max_length, n_agents, state_dim = global_states.shape
        
        # Compute values for all timesteps
        values_list = []
        
        for t in range(max_length):
            global_state_t = global_states[:, t, :, :]
            value_t = self.critic(global_state_t, agent_mask)
            values_list.append(value_t)
        
        values = torch.stack(values_list, dim=1).squeeze(-1)  # (batch, max_length)
        
        # Critic loss (MSE between predicted values and returns)
        # Only compute loss for valid timesteps
        critic_loss = F.mse_loss(
            values[length_mask],
            returns[length_mask]
        )
        
        # Backprop
        self.critic_optimizer.zero_grad()
        critic_loss.backward()
        nn.utils.clip_grad_norm_(self.critic.parameters(), self.max_grad_norm)
        self.critic_optimizer.step()
        
        return critic_loss.item()
    
    def update_actors(
        self,
        local_obs: torch.Tensor,
        signals: torch.Tensor,
        actions: torch.Tensor,
        advantages: torch.Tensor,
        length_mask: torch.Tensor,
        agent_mask: torch.Tensor
    ) -> Tuple[float, float]:
        """
        Update decentralized actors using PPO
        
        Args:
            local_obs: (batch, max_length, n_agents, state_dim)
            signals: (batch, max_length, n_agents, signal_dim)
            actions: (batch, max_length, n_agents)
            advantages: (batch, max_length, n_agents)
            length_mask: (batch, max_length)
            agent_mask: (batch, n_agents)
        
        Returns:
            actor_loss: Actor loss value
            entropy: Average entropy
        """
        batch_size, max_length, n_agents, state_dim = local_obs.shape
        
        # Compute old action probabilities (for PPO ratio)
        with torch.no_grad():
            old_action_probs_list = []
            
            for t in range(max_length):
                local_obs_t = local_obs[:, t, :, :]  # (batch, n_agents, state_dim)
                signals_t = signals[:, t, :, :]  # (batch, n_agents, signal_dim)
                
                # Process each agent
                agent_probs_t = []
                for a in range(n_agents):
                    action_probs, _ = self.actor(
                        local_obs_t[:, a, :],
                        signals_t[:, a, :]
                    )
                    agent_probs_t.append(action_probs)
                
                # Stack: (batch, n_agents, action_dim)
                agent_probs_t = torch.stack(agent_probs_t, dim=1)
                old_action_probs_list.append(agent_probs_t)
            
            old_action_probs = torch.stack(old_action_probs_list, dim=1)  # (batch, max_length, n_agents, action_dim)
            
            # Get old log probs for actions taken
            old_log_probs = torch.log(
                old_action_probs.gather(-1, actions.unsqueeze(-1)).squeeze(-1) + 1e-8
            )
        
        # Compute new action probabilities
        new_action_probs_list = []
        
        for t in range(max_length):
            local_obs_t = local_obs[:, t, :, :]
            signals_t = signals[:, t, :, :]
            
            agent_probs_t = []
            for a in range(n_agents):
                action_probs, _ = self.actor(
                    local_obs_t[:, a, :],
                    signals_t[:, a, :]
                )
                agent_probs_t.append(action_probs)
            
            agent_probs_t = torch.stack(agent_probs_t, dim=1)
            new_action_probs_list.append(agent_probs_t)
        
        new_action_probs = torch.stack(new_action_probs_list, dim=1)
        
        # Get new log probs
        new_log_probs = torch.log(
            new_action_probs.gather(-1, actions.unsqueeze(-1)).squeeze(-1) + 1e-8
        )
        
        # PPO ratio
        ratio = torch.exp(new_log_probs - old_log_probs)
        
        # Normalize advantages
        active_advantages = advantages[length_mask.unsqueeze(-1).expand_as(advantages)]
        if active_advantages.numel() > 0:
            adv_mean = active_advantages.mean()
            adv_std = active_advantages.std() + 1e-8
            advantages_normalized = (advantages - adv_mean) / adv_std
        else:
            advantages_normalized = advantages
        
        # PPO clipped objective
        surr1 = ratio * advantages_normalized
        surr2 = torch.clamp(ratio, 1.0 - self.clip_epsilon, 1.0 + self.clip_epsilon) * advantages_normalized
        actor_loss = -torch.min(surr1, surr2)
        
        # Apply masks
        length_agent_mask = length_mask.unsqueeze(-1).expand_as(actor_loss)
        agent_mask_expanded = agent_mask.unsqueeze(1).expand_as(actor_loss)
        full_mask = length_agent_mask & agent_mask_expanded
        
        actor_loss_masked = actor_loss[full_mask].mean()
        
        # Entropy bonus (for exploration)
        entropy_dist = torch.distributions.Categorical(new_action_probs)
        entropy = entropy_dist.entropy()
        entropy_masked = entropy[full_mask].mean()
        
        # Total actor loss
        total_actor_loss = actor_loss_masked - self.entropy_coef * entropy_masked
        
        # Backprop
        self.actor_optimizer.zero_grad()
        total_actor_loss.backward()
        nn.utils.clip_grad_norm_(self.actor.parameters(), self.max_grad_norm)
        self.actor_optimizer.step()
        
        return total_actor_loss.item(), entropy_masked.item()
    
    async def train_on_buffer(
        self,
        buffer: PackEpisodeBuffer,
        n_iterations: int = 100,
        episodes_per_iteration: int = 32
    ) -> Dict[str, List[float]]:
        """
        Train on episodes from buffer
        
        Args:
            buffer: PackEpisodeBuffer with episodes
            n_iterations: Number of training iterations
            episodes_per_iteration: Episodes per iteration
        
        Returns:
            Training history with metrics
        """
        history = {
            'actor_loss': [],
            'critic_loss': [],
            'entropy': [],
            'avg_return': []
        }
        
        logger.info(f"Starting CTDE training: {n_iterations} iterations")
        
        for iteration in range(n_iterations):
            # Sample batch of episodes
            episodes = buffer.get_batch(
                batch_size=episodes_per_iteration,
                prioritized=self.config.get('prioritized_sampling', True)
            )
            
            if not episodes:
                logger.warning(f"No episodes available at iteration {iteration}")
                continue
            
            # Pad and batch episodes
            episode_data = pad_episode_batch(episodes, max_agents=self.n_agents)
            
            # Train on batch
            metrics = self.train_episode(episode_data)
            
            # Record history
            history['actor_loss'].append(metrics['actor_loss'])
            history['critic_loss'].append(metrics['critic_loss'])
            history['entropy'].append(metrics['entropy'])
            history['avg_return'].append(metrics['avg_return'])
            
            # Logging
            if (iteration + 1) % 10 == 0:
                logger.info(
                    f"Iteration {iteration + 1}/{n_iterations}: "
                    f"actor_loss={metrics['actor_loss']:.4f}, "
                    f"critic_loss={metrics['critic_loss']:.4f}, "
                    f"return={metrics['avg_return']:.2f}"
                )
        
        logger.info("CTDE training complete")
        return history
    
    def get_actor_policy(self, agent_id: int = 0) -> DecentralizedActor:
        """
        Get trained actor policy for deployment
        
        In CTDE, all agents share the same actor network.
        This returns the trained actor for decentralized execution.
        
        Args:
            agent_id: Agent ID (for logging, all share same policy)
        
        Returns:
            Trained DecentralizedActor model
        """
        logger.info(f"Retrieving actor policy for agent {agent_id}")
        return self.actor
    
    def save_checkpoint(
        self,
        path: str,
        metadata: Optional[Dict[str, Any]] = None
    ) -> bool:
        """
        Save training checkpoint
        
        Args:
            path: Path to save checkpoint
            metadata: Optional metadata to include
        
        Returns:
            True if saved successfully
        """
        try:
            checkpoint = {
                'training_step': self.training_step,
                'episodes_trained': self.episodes_trained,
                'critic_state_dict': self.critic.state_dict(),
                'actor_state_dict': self.actor.state_dict(),
                'critic_optimizer_state_dict': self.critic_optimizer.state_dict(),
                'actor_optimizer_state_dict': self.actor_optimizer.state_dict(),
                'config': self.config,
                'metadata': metadata or {}
            }
            
            if self.comm_net is not None:
                checkpoint['comm_net_state_dict'] = self.comm_net.state_dict()
                checkpoint['comm_optimizer_state_dict'] = self.comm_optimizer.state_dict()
            
            if self.qmix_mixer is not None:
                checkpoint['qmix_state_dict'] = self.qmix_mixer.state_dict()
            
            torch.save(checkpoint, path)
            
            logger.info(f"Saved checkpoint to {path}")
            return True
        
        except Exception as e:
            logger.error(f"Failed to save checkpoint: {e}", exc_info=True)
            return False
    
    def load_checkpoint(self, path: str) -> bool:
        """
        Load training checkpoint
        
        Args:
            path: Path to checkpoint file
        
        Returns:
            True if loaded successfully
        """
        try:
            checkpoint = torch.load(path, map_location=self.device)
            
            self.training_step = checkpoint.get('training_step', 0)
            self.episodes_trained = checkpoint.get('episodes_trained', 0)
            
            self.critic.load_state_dict(checkpoint['critic_state_dict'])
            self.actor.load_state_dict(checkpoint['actor_state_dict'])
            self.critic_optimizer.load_state_dict(checkpoint['critic_optimizer_state_dict'])
            self.actor_optimizer.load_state_dict(checkpoint['actor_optimizer_state_dict'])
            
            if 'comm_net_state_dict' in checkpoint and self.comm_net is not None:
                self.comm_net.load_state_dict(checkpoint['comm_net_state_dict'])
                self.comm_optimizer.load_state_dict(checkpoint['comm_optimizer_state_dict'])
            
            if 'qmix_state_dict' in checkpoint and self.qmix_mixer is not None:
                self.qmix_mixer.load_state_dict(checkpoint['qmix_state_dict'])
            
            logger.info(
                f"Loaded checkpoint from {path}: "
                f"step={self.training_step}, episodes={self.episodes_trained}"
            )
            return True
        
        except Exception as e:
            logger.error(f"Failed to load checkpoint: {e}", exc_info=True)
            return False
    
    def export_actor_to_onnx(
        self,
        output_path: str,
        archetype: str = 'aggressive'
    ) -> bool:
        """
        Export trained actor to ONNX for deployment
        
        The actor is used in decentralized execution (inference).
        Critic is NOT exported (only used during training).
        
        Args:
            output_path: Path to save ONNX model
            archetype: Monster archetype name
        
        Returns:
            True if exported successfully
        """
        try:
            self.actor.eval()
            
            # Create dummy inputs
            dummy_state = torch.randn(1, self.state_dim).to(self.device)
            dummy_signals = torch.randn(1, self.signal_dim).to(self.device)
            
            # Export to ONNX
            torch.onnx.export(
                self.actor,
                (dummy_state, dummy_signals, None, None),  # (state, signals, hidden, action_mask)
                output_path,
                export_params=True,
                opset_version=14,
                do_constant_folding=True,
                input_names=['state', 'signals'],
                output_names=['action_probs'],
                dynamic_axes={
                    'state': {0: 'batch'},
                    'signals': {0: 'batch'},
                    'action_probs': {0: 'batch'}
                }
            )
            
            logger.info(f"Exported actor to ONNX: {output_path}")
            
            # Verify ONNX model
            import onnx
            onnx_model = onnx.load(output_path)
            onnx.checker.check_model(onnx_model)
            
            logger.info("✓ ONNX model verification passed")
            
            self.actor.train()
            return True
        
        except Exception as e:
            logger.error(f"Failed to export actor to ONNX: {e}", exc_info=True)
            self.actor.train()
            return False
    
    def get_training_statistics(self) -> Dict[str, Any]:
        """
        Get training statistics
        
        Returns:
            Statistics dict
        """
        stats = {
            'training_step': self.training_step,
            'episodes_trained': self.episodes_trained,
            'avg_actor_loss': self.total_actor_loss / max(self.training_step, 1),
            'avg_critic_loss': self.total_critic_loss / max(self.training_step, 1),
            'avg_entropy': self.total_entropy / max(self.training_step, 1),
            'critic_params': self.critic.count_parameters(),
            'actor_params': self.actor.count_parameters()
        }
        
        return stats
    
    def set_train_mode(self, mode: bool = True) -> None:
        """Set models to train or eval mode"""
        self.critic.train(mode)
        self.actor.train(mode)
        if self.comm_net is not None:
            self.comm_net.train(mode)
        if self.qmix_mixer is not None:
            self.qmix_mixer.train(mode)
    
    def eval_mode(self) -> None:
        """Set all models to evaluation mode"""
        self.set_train_mode(False)


# ============================================================================
# TRAINING UTILITIES
# ============================================================================

def create_default_ctde_config() -> Dict[str, Any]:
    """Create default CTDE training configuration"""
    return {
        'state_dim': 64,
        'signal_dim': 32,
        'action_dim': 7,
        'n_agents': 5,
        
        # Critic config
        'critic_hidden_dims': [512, 512, 256],
        'critic_use_attention': True,
        'critic_dropout': 0.2,
        'critic_lr': 0.0003,
        
        # Actor config
        'actor_hidden_dim': 256,
        'actor_use_gru': True,
        'actor_dropout': 0.1,
        'actor_lr': 0.0001,
        
        # Communication config
        'use_communication': True,
        'comm_hidden_dim': 128,
        'comm_rounds': 3,
        'comm_dropout': 0.1,
        'comm_lr': 0.0002,
        
        # Training hyperparameters
        'gamma': 0.99,
        'gae_lambda': 0.95,
        'clip_epsilon': 0.2,
        'value_loss_coef': 0.5,
        'entropy_coef': 0.01,
        'max_grad_norm': 1.0,
        'n_epochs': 4,
        'batch_size': 32,
        
        # Reward composition
        'team_reward_weight': 0.7,  # 70% team, 30% individual
        
        # Sampling
        'prioritized_sampling': True,
        
        # QMIX (optional)
        'use_qmix': False,
        'qmix_mixing_embed': 32
    }


async def train_ctde_from_config(
    config_path: str,
    buffer: PackEpisodeBuffer,
    checkpoint_dir: str = './checkpoints',
    n_iterations: int = 1000
) -> CTDETrainer:
    """
    Train CTDE model from configuration file
    
    Args:
        config_path: Path to YAML config file
        buffer: PackEpisodeBuffer with training data
        checkpoint_dir: Directory for checkpoints
        n_iterations: Number of training iterations
    
    Returns:
        Trained CTDETrainer instance
    """
    import yaml
    
    # Load config
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    
    # Get CTDE config section
    ctde_config = config.get('ctde', create_default_ctde_config())
    
    # Create trainer
    trainer = CTDETrainer(ctde_config)
    
    # Create checkpoint directory
    Path(checkpoint_dir).mkdir(parents=True, exist_ok=True)
    
    # Training loop
    logger.info(f"Starting CTDE training for {n_iterations} iterations")
    
    history = await trainer.train_on_buffer(
        buffer,
        n_iterations=n_iterations,
        episodes_per_iteration=ctde_config.get('batch_size', 32)
    )
    
    # Save final checkpoint
    final_checkpoint_path = f"{checkpoint_dir}/ctde_final_step_{trainer.training_step}.pt"
    trainer.save_checkpoint(
        final_checkpoint_path,
        metadata={
            'history': history,
            'config_path': config_path,
            'training_completed': datetime.now().isoformat()
        }
    )
    
    # Export actor to ONNX
    onnx_path = f"{checkpoint_dir}/decentralized_actor.onnx"
    trainer.export_actor_to_onnx(onnx_path)
    
    logger.info(f"Training complete. Checkpoint saved to {final_checkpoint_path}")
    
    return trainer


# ============================================================================
# TESTING
# ============================================================================

def test_ctde_trainer():
    """Test CTDE trainer with synthetic data"""
    logger.info("Testing CTDETrainer...")
    
    # Create config
    config = create_default_ctde_config()
    config['n_epochs'] = 2  # Fewer epochs for testing
    
    # Create trainer
    trainer = CTDETrainer(config, n_agents=5)
    
    # Create synthetic episode data
    batch_size = 4
    max_length = 20
    n_agents = 5
    state_dim = 64
    signal_dim = 32
    action_dim = 7
    
    episode_data = {
        'global_states': torch.randn(batch_size, max_length, n_agents, state_dim),
        'local_obs': torch.randn(batch_size, max_length, n_agents, state_dim),
        'actions': torch.randint(0, action_dim, (batch_size, max_length, n_agents)),
        'team_rewards': torch.randn(batch_size, max_length),
        'individual_rewards': torch.randn(batch_size, max_length, n_agents),
        'signals': torch.randn(batch_size, max_length, n_agents, signal_dim),
        'length_mask': torch.ones(batch_size, max_length, dtype=torch.bool),
        'agent_mask': torch.ones(batch_size, n_agents, dtype=torch.bool),
        'batch_size': batch_size,
        'max_length': max_length,
        'max_agents': n_agents
    }
    
    # Train
    logger.info("\n1. Testing train_episode()...")
    metrics = trainer.train_episode(episode_data)
    logger.info(f"✓ Training metrics: {metrics}")
    
    # Test checkpoint save/load
    logger.info("\n2. Testing checkpoint save/load...")
    checkpoint_path = '/tmp/test_ctde_checkpoint.pt'
    trainer.save_checkpoint(checkpoint_path, metadata={'test': True})
    
    trainer2 = CTDETrainer(config, n_agents=5)
    trainer2.load_checkpoint(checkpoint_path)
    
    assert trainer2.training_step == trainer.training_step
    logger.info(f"✓ Checkpoint loaded successfully: step={trainer2.training_step}")
    
    # Test ONNX export
    logger.info("\n3. Testing ONNX export...")
    onnx_path = '/tmp/test_actor.onnx'
    success = trainer.export_actor_to_onnx(onnx_path)
    assert success, "ONNX export failed"
    logger.info(f"✓ Actor exported to {onnx_path}")
    
    # Test get actor policy
    logger.info("\n4. Testing get_actor_policy()...")
    actor = trainer.get_actor_policy(agent_id=0)
    assert actor is trainer.actor
    logger.info("✓ Actor policy retrieved")
    
    # Test statistics
    logger.info("\n5. Testing statistics...")
    stats = trainer.get_training_statistics()
    logger.info(f"✓ Training stats: {stats}")
    
    logger.info("\n✓ All CTDETrainer tests passed!")


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    test_ctde_trainer()
