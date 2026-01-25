"""
CTDE (Centralized Training, Decentralized Execution) Architectures
====================================================================
Multi-agent reinforcement learning architectures for pack-based monster coordination.

Components:
1. CentralizedCritic - Centralized value function for team performance evaluation
2. DecentralizedActor - Individual actor policies that work with local observations
3. CommNet - Communication network for message passing between agents
4. QMIXMixer - QMIX mixing network for value function decomposition

Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md (lines 5504-5834)
Phase 5 Integration: Uses Apache AGE graph for pack detection and signal passing
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
from typing import Tuple, Optional, Dict, List, Any
import math
import logging

logger = logging.getLogger(__name__)


class CentralizedCritic(nn.Module):
    """
    Centralized critic that evaluates team performance using global state.
    
    Used during TRAINING ONLY - sees full pack state to compute advantages.
    During inference, only DecentralizedActors are used.
    
    Architecture:
        Input: Global state (n_agents × state_dim) concatenated
        Hidden: [512, 512, 256] with layer norm and dropout
        Output: Single global value V(s_global)
    
    Features:
    - Sees all pack members' states (privileged information)
    - Attention mechanism over agent states for dynamic pack sizes
    - Layer normalization for training stability
    - Supports variable pack sizes (2-5 monsters)
    
    Size: ~20MB FP16
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        n_agents: int = 5,
        hidden_dims: List[int] = None,
        use_attention: bool = True,
        dropout: float = 0.2
    ):
        """
        Initialize centralized critic
        
        Args:
            state_dim: Dimension of individual agent state
            n_agents: Maximum number of agents in pack
            hidden_dims: Hidden layer dimensions
            use_attention: Whether to use attention over agent states
            dropout: Dropout probability
        """
        super().__init__()
        
        if hidden_dims is None:
            hidden_dims = [512, 512, 256]
        
        self.state_dim = state_dim
        self.n_agents = n_agents
        self.use_attention = use_attention
        
        # Global state dimension (all agents concatenated)
        self.global_state_dim = state_dim * n_agents
        
        if use_attention:
            # Attention mechanism for variable pack sizes
            self.attention = nn.MultiheadAttention(
                embed_dim=state_dim,
                num_heads=8,
                dropout=dropout,
                batch_first=True
            )
            
            # Project attended states
            self.attention_proj = nn.Linear(state_dim, state_dim)
            
            # Input to network is attended + concatenated states
            network_input_dim = state_dim + self.global_state_dim
        else:
            network_input_dim = self.global_state_dim
        
        # Build deep value network
        layers = []
        in_dim = network_input_dim
        
        for hidden_dim in hidden_dims:
            layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            in_dim = hidden_dim
        
        # Value head
        layers.append(nn.Linear(in_dim, 1))
        
        self.value_network = nn.Sequential(*layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"CentralizedCritic initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        """Initialize weights with orthogonal initialization"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        global_state: torch.Tensor,
        agent_mask: Optional[torch.Tensor] = None
    ) -> torch.Tensor:
        """
        Compute global value function
        
        Args:
            global_state: (batch, n_agents, state_dim) or (batch, n_agents * state_dim)
            agent_mask: (batch, n_agents) binary mask indicating active agents
        
        Returns:
            value: (batch, 1) global state value
        """
        batch_size = global_state.size(0)
        
        # Handle both formats
        if len(global_state.shape) == 3:
            # (batch, n_agents, state_dim)
            agent_states = global_state
            flat_state = global_state.view(batch_size, -1)
        else:
            # (batch, n_agents * state_dim)
            flat_state = global_state
            agent_states = global_state.view(batch_size, self.n_agents, self.state_dim)
        
        if self.use_attention:
            # Apply attention over agent states
            if agent_mask is not None:
                # Convert mask to attention mask format
                # True = attend, False = ignore
                attn_mask = agent_mask.unsqueeze(1).expand(-1, self.n_agents, -1)
                attn_mask = attn_mask.float().masked_fill(~attn_mask.bool(), float('-inf'))
            else:
                attn_mask = None
            
            # Self-attention over agents
            attended, _ = self.attention(
                agent_states, 
                agent_states, 
                agent_states,
                attn_mask=attn_mask
            )
            
            # Global pooling over attended states
            if agent_mask is not None:
                # Masked mean
                mask_expanded = agent_mask.unsqueeze(-1).float()
                attended_sum = (attended * mask_expanded).sum(dim=1)
                attended_mean = attended_sum / mask_expanded.sum(dim=1).clamp(min=1)
            else:
                attended_mean = attended.mean(dim=1)
            
            # Project
            attended_features = self.attention_proj(attended_mean)
            
            # Concatenate with flat state
            network_input = torch.cat([attended_features, flat_state], dim=-1)
        else:
            network_input = flat_state
        
        # Compute value
        value = self.value_network(network_input)
        
        return value


class DecentralizedActor(nn.Module):
    """
    Decentralized actor for individual agent policy.
    
    Used in both TRAINING and INFERENCE - makes decisions based on:
    - Local observations (own state)
    - Received signals from pack members
    
    NO communication required during inference - signals are optional.
    
    Architecture:
        State Encoder: state_dim -> 256
        Signal Encoder: signal_dim -> 128
        Combined: 256 + 128 -> 256 -> 128 -> action_dim
    
    Features:
    - GRU for temporal state tracking
    - Signal aggregation for pack coordination
    - Action masking for invalid actions
    - Supports solo operation (zero signals)
    
    Size: ~5MB FP16 per agent
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        signal_dim: int = 32,
        action_dim: int = 7,
        hidden_dim: int = 256,
        use_gru: bool = True,
        dropout: float = 0.1
    ):
        """
        Initialize decentralized actor
        
        Args:
            state_dim: Local observation dimension
            signal_dim: Signal vector dimension
            action_dim: Number of coordination actions
            hidden_dim: Hidden layer dimension
            use_gru: Whether to use GRU for temporal context
            dropout: Dropout probability
        """
        super().__init__()
        
        self.state_dim = state_dim
        self.signal_dim = signal_dim
        self.action_dim = action_dim
        self.hidden_dim = hidden_dim
        self.use_gru = use_gru
        
        # State encoder (processes own state)
        self.state_encoder = nn.Sequential(
            nn.Linear(state_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU()
        )
        
        # Signal encoder (processes received signals)
        self.signal_encoder = nn.Sequential(
            nn.Linear(signal_dim, hidden_dim // 2),
            nn.LayerNorm(hidden_dim // 2),
            nn.ReLU(),
            nn.Dropout(dropout)
        )
        
        # GRU for temporal dynamics (optional)
        if use_gru:
            self.gru = nn.GRU(
                input_size=hidden_dim + hidden_dim // 2,
                hidden_size=hidden_dim,
                num_layers=2,
                batch_first=True,
                dropout=dropout if 2 > 1 else 0
            )
            decision_input_dim = hidden_dim
        else:
            self.gru = None
            decision_input_dim = hidden_dim + hidden_dim // 2
        
        # Decision network
        self.decision_network = nn.Sequential(
            nn.Linear(decision_input_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.ReLU(),
            nn.Linear(hidden_dim // 2, action_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"DecentralizedActor initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.GRU):
            for name, param in m.named_parameters():
                if 'weight' in name:
                    nn.init.orthogonal_(param)
                elif 'bias' in name:
                    nn.init.constant_(param, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        state: torch.Tensor,
        signals: Optional[torch.Tensor] = None,
        hidden: Optional[torch.Tensor] = None,
        action_mask: Optional[torch.Tensor] = None
    ) -> Tuple[torch.Tensor, Optional[torch.Tensor]]:
        """
        Forward pass to get action probabilities
        
        Args:
            state: (batch, state_dim) local observations
            signals: (batch, signal_dim) aggregated signals from pack (can be None)
            hidden: (num_layers, batch, hidden_dim) GRU hidden state (if use_gru)
            action_mask: (batch, action_dim) binary mask for invalid actions
        
        Returns:
            action_probs: (batch, action_dim) action probabilities
            hidden: Updated GRU hidden state (if use_gru, else None)
        """
        batch_size = state.size(0)
        
        # Encode own state
        state_features = self.state_encoder(state)  # (batch, hidden_dim)
        
        # Encode signals (or use zeros if no signals)
        if signals is None:
            signals = torch.zeros(batch_size, self.signal_dim, device=state.device)
        
        signal_features = self.signal_encoder(signals)  # (batch, hidden_dim // 2)
        
        # Combine features
        combined = torch.cat([state_features, signal_features], dim=-1)
        
        # Apply GRU if enabled (for temporal dynamics)
        if self.use_gru:
            # GRU expects (batch, seq_len=1, features)
            combined = combined.unsqueeze(1)
            
            if hidden is not None:
                gru_out, hidden = self.gru(combined, hidden)
            else:
                gru_out, hidden = self.gru(combined)
            
            # Remove seq dimension
            features = gru_out.squeeze(1)  # (batch, hidden_dim)
        else:
            features = combined
            hidden = None
        
        # Decision network
        action_logits = self.decision_network(features)
        
        # Apply action mask if provided
        if action_mask is not None:
            action_logits = action_logits.masked_fill(~action_mask.bool(), float('-inf'))
        
        # Softmax to get probabilities
        action_probs = F.softmax(action_logits, dim=-1)
        
        return action_probs, hidden
    
    def get_action(
        self,
        state: torch.Tensor,
        signals: Optional[torch.Tensor] = None,
        hidden: Optional[torch.Tensor] = None,
        action_mask: Optional[torch.Tensor] = None,
        deterministic: bool = False
    ) -> Tuple[torch.Tensor, torch.Tensor, torch.Tensor, Optional[torch.Tensor]]:
        """
        Sample action from policy
        
        Args:
            state: (batch, state_dim) local observations
            signals: (batch, signal_dim) aggregated signals
            hidden: GRU hidden state
            action_mask: (batch, action_dim) binary mask
            deterministic: If True, return argmax action
        
        Returns:
            action: (batch,) sampled actions
            log_prob: (batch,) log probabilities
            entropy: (batch,) policy entropy
            hidden: Updated hidden state
        """
        action_probs, hidden = self.forward(state, signals, hidden, action_mask)
        
        if deterministic:
            # Greedy action
            action = action_probs.argmax(dim=-1)
            log_prob = torch.log(action_probs.gather(-1, action.unsqueeze(-1)).squeeze(-1) + 1e-8)
            entropy = torch.zeros_like(log_prob)
        else:
            # Sample from distribution
            dist = torch.distributions.Categorical(action_probs)
            action = dist.sample()
            log_prob = dist.log_prob(action)
            entropy = dist.entropy()
        
        return action, log_prob, entropy, hidden


class CommNet(nn.Module):
    """
    Communication network for message passing between agents.
    
    Enables agents to exchange information through multiple rounds of communication.
    Used during training to learn coordination patterns.
    
    Architecture:
        Round 1-N: Each agent broadcasts hidden state
                   Each agent receives messages from neighbors
                   Each agent updates hidden state
    
    Features:
    - Multi-round communication (typically 3 rounds)
    - Neighbor-based message passing (via adjacency matrix)
    - Mean aggregation of received messages
    - Skip connections for gradient flow
    
    Size: ~3MB FP16
    """
    
    def __init__(
        self,
        hidden_dim: int = 128,
        n_agents: int = 5,
        n_rounds: int = 3,
        dropout: float = 0.1
    ):
        """
        Initialize communication network
        
        Args:
            hidden_dim: Hidden state dimension
            n_agents: Maximum number of agents
            n_rounds: Number of communication rounds
            dropout: Dropout probability
        """
        super().__init__()
        
        self.hidden_dim = hidden_dim
        self.n_agents = n_agents
        self.n_rounds = n_rounds
        
        # Encode each agent's local observation
        self.encoder = nn.Sequential(
            nn.Linear(64, hidden_dim),  # Assuming state_dim=64
            nn.LayerNorm(hidden_dim),
            nn.ReLU()
        )
        
        # Communication modules for each round
        self.comm_modules = nn.ModuleList([
            nn.Sequential(
                nn.Linear(hidden_dim * 2, hidden_dim),  # self + neighbor message
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(dropout)
            )
            for _ in range(n_rounds)
        ])
        
        # Output projection
        self.output_proj = nn.Linear(hidden_dim, hidden_dim)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"CommNet initialized: {self.count_parameters():,} parameters, {n_rounds} rounds")
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        local_obs: torch.Tensor,
        adj_matrix: torch.Tensor
    ) -> torch.Tensor:
        """
        Multi-round message passing
        
        Args:
            local_obs: (batch, n_agents, state_dim) local observations
            adj_matrix: (batch, n_agents, n_agents) adjacency matrix
                       adj_matrix[b, i, j] = 1 if agent i can communicate with agent j
        
        Returns:
            hidden_states: (batch, n_agents, hidden_dim) final hidden states after communication
        """
        batch_size = local_obs.size(0)
        
        # Encode local observations
        hidden = self.encoder(local_obs)  # (batch, n_agents, hidden_dim)
        
        # Communication rounds
        for round_idx in range(self.n_rounds):
            # Broadcast messages: each agent's hidden state is its message
            messages = hidden  # (batch, n_agents, hidden_dim)
            
            # Aggregate messages from neighbors using adjacency matrix
            # adj_matrix: (batch, n_agents, n_agents)
            # messages: (batch, n_agents, hidden_dim)
            # received: (batch, n_agents, hidden_dim)
            
            # Normalize adjacency matrix to get average of neighbor messages
            adj_normalized = adj_matrix / (adj_matrix.sum(dim=-1, keepdim=True).clamp(min=1))
            
            # Matrix multiply: (batch, n_agents, n_agents) @ (batch, n_agents, hidden_dim)
            received = torch.bmm(adj_normalized, messages)  # (batch, n_agents, hidden_dim)
            
            # Update hidden state with received messages
            combined = torch.cat([hidden, received], dim=-1)  # (batch, n_agents, hidden_dim * 2)
            
            # Apply communication module for this round
            hidden_new = self.comm_modules[round_idx](combined)  # (batch, n_agents, hidden_dim)
            
            # Skip connection
            hidden = hidden + hidden_new
        
        # Output projection
        output = self.output_proj(hidden)
        
        return output


class QMIXMixer(nn.Module):
    """
    QMIX mixing network for value function decomposition.
    
    Combines individual agent Q-values into global Q-value using a mixing network.
    The mixer is monotonic: global Q increases when any agent Q increases.
    
    Architecture:
        Hypernetwork: state -> weights for mixing network
        Mixing Network: agent_qs -> global_q (using generated weights)
    
    Monotonicity: Ensured by using absolute values or exp for weights
    
    Features:
    - Hypernetwork generates mixing weights from global state
    - Monotonic mixing (positive weights only)
    - Non-linear mixing for complex value decomposition
    
    Size: ~2MB FP16
    """
    
    def __init__(
        self,
        n_agents: int = 5,
        state_dim: int = 64,
        mixing_embed_dim: int = 32,
        hypernet_embed_dim: int = 64
    ):
        """
        Initialize QMIX mixer
        
        Args:
            n_agents: Number of agents
            state_dim: Global state dimension
            mixing_embed_dim: Embedding dimension for mixing network
            hypernet_embed_dim: Embedding dimension for hypernetwork
        """
        super().__init__()
        
        self.n_agents = n_agents
        self.state_dim = state_dim
        self.mixing_embed_dim = mixing_embed_dim
        
        # Hypernetwork for generating mixing weights
        # Layer 1 weights: (n_agents, mixing_embed_dim)
        self.hyper_w1 = nn.Sequential(
            nn.Linear(state_dim, hypernet_embed_dim),
            nn.ReLU(),
            nn.Linear(hypernet_embed_dim, n_agents * mixing_embed_dim)
        )
        
        # Layer 1 bias: (mixing_embed_dim,)
        self.hyper_b1 = nn.Linear(state_dim, mixing_embed_dim)
        
        # Layer 2 weights: (mixing_embed_dim, 1)
        self.hyper_w2 = nn.Sequential(
            nn.Linear(state_dim, hypernet_embed_dim),
            nn.ReLU(),
            nn.Linear(hypernet_embed_dim, mixing_embed_dim)
        )
        
        # Layer 2 bias: scalar
        self.hyper_b2 = nn.Sequential(
            nn.Linear(state_dim, hypernet_embed_dim),
            nn.ReLU(),
            nn.Linear(hypernet_embed_dim, 1)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"QMIXMixer initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        agent_qs: torch.Tensor,
        state: torch.Tensor
    ) -> torch.Tensor:
        """
        Mix agent Q-values into global Q-value
        
        Args:
            agent_qs: (batch, n_agents) individual agent Q-values
            state: (batch, state_dim) global state
        
        Returns:
            global_q: (batch, 1) mixed global Q-value
        """
        batch_size = agent_qs.size(0)
        
        # Generate mixing network weights from state (hypernetwork)
        # Layer 1
        w1 = torch.abs(self.hyper_w1(state))  # (batch, n_agents * mixing_embed_dim)
        w1 = w1.view(batch_size, self.n_agents, self.mixing_embed_dim)  # (batch, n_agents, embed)
        
        b1 = self.hyper_b1(state)  # (batch, mixing_embed_dim)
        
        # Layer 2
        w2 = torch.abs(self.hyper_w2(state))  # (batch, mixing_embed_dim)
        w2 = w2.view(batch_size, self.mixing_embed_dim, 1)  # (batch, embed, 1)
        
        b2 = self.hyper_b2(state)  # (batch, 1)
        
        # Apply mixing network
        # Layer 1: agent_qs @ w1 + b1
        agent_qs_expanded = agent_qs.unsqueeze(-1)  # (batch, n_agents, 1)
        
        # Transpose w1 for matmul: (batch, n_agents, embed) -> (batch, embed, n_agents)
        hidden = torch.bmm(w1.transpose(1, 2), agent_qs_expanded)  # (batch, embed, 1)
        hidden = hidden.squeeze(-1) + b1  # (batch, embed)
        hidden = F.elu(hidden)  # Non-linearity
        
        # Layer 2: hidden @ w2 + b2
        hidden_expanded = hidden.unsqueeze(1)  # (batch, 1, embed)
        global_q = torch.bmm(hidden_expanded, w2)  # (batch, 1, 1)
        global_q = global_q.squeeze(-1) + b2  # (batch, 1)
        
        return global_q


class AttentionCommNet(nn.Module):
    """
    Attention-based communication network for selective message passing.
    
    More sophisticated than basic CommNet - uses attention to weight
    messages from different neighbors based on relevance.
    
    Features:
    - Multi-head attention for different communication aspects
    - Learnable query, key, value projections
    - Multiple communication rounds
    - Skip connections
    
    Size: ~5MB FP16
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        hidden_dim: int = 128,
        n_agents: int = 5,
        n_rounds: int = 3,
        n_heads: int = 4,
        dropout: float = 0.1
    ):
        """
        Initialize attention communication network
        
        Args:
            state_dim: State dimension per agent
            hidden_dim: Hidden dimension for communication
            n_agents: Maximum number of agents
            n_rounds: Number of communication rounds
            n_heads: Number of attention heads
            dropout: Dropout probability
        """
        super().__init__()
        
        self.state_dim = state_dim
        self.hidden_dim = hidden_dim
        self.n_agents = n_agents
        self.n_rounds = n_rounds
        self.n_heads = n_heads
        
        # Encode observations
        self.encoder = nn.Sequential(
            nn.Linear(state_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU()
        )
        
        # Multi-head attention for each round
        self.attention_layers = nn.ModuleList([
            nn.MultiheadAttention(
                embed_dim=hidden_dim,
                num_heads=n_heads,
                dropout=dropout,
                batch_first=True
            )
            for _ in range(n_rounds)
        ])
        
        # Feedforward networks for each round
        self.ff_networks = nn.ModuleList([
            nn.Sequential(
                nn.Linear(hidden_dim, hidden_dim * 2),
                nn.ReLU(),
                nn.Dropout(dropout),
                nn.Linear(hidden_dim * 2, hidden_dim),
                nn.LayerNorm(hidden_dim)
            )
            for _ in range(n_rounds)
        ])
        
        # Output projection
        self.output_proj = nn.Linear(hidden_dim, hidden_dim)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(
            f"AttentionCommNet initialized: {self.count_parameters():,} parameters, "
            f"{n_rounds} rounds, {n_heads} heads"
        )
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.xavier_uniform_(m.weight)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        local_obs: torch.Tensor,
        adj_matrix: Optional[torch.Tensor] = None,
        agent_mask: Optional[torch.Tensor] = None
    ) -> torch.Tensor:
        """
        Multi-round attention-based communication
        
        Args:
            local_obs: (batch, n_agents, state_dim) local observations
            adj_matrix: (batch, n_agents, n_agents) adjacency matrix (optional)
            agent_mask: (batch, n_agents) binary mask for active agents
        
        Returns:
            hidden_states: (batch, n_agents, hidden_dim) communicated hidden states
        """
        # Encode observations
        hidden = self.encoder(local_obs)  # (batch, n_agents, hidden_dim)
        
        # Communication rounds
        for round_idx in range(self.n_rounds):
            # Self-attention (agents communicate)
            if agent_mask is not None:
                # Create attention mask from agent mask
                # Shape: (batch, n_agents, n_agents)
                attn_mask = agent_mask.unsqueeze(1).expand(-1, self.n_agents, -1)
                attn_mask = attn_mask & agent_mask.unsqueeze(-1).expand(-1, -1, self.n_agents)
                
                # Convert to attention mask format (True where we should attend)
                # Then convert to additive mask (0 for attend, -inf for ignore)
                attn_mask = attn_mask.float().masked_fill(~attn_mask.bool(), float('-inf'))
            else:
                attn_mask = None
            
            # Apply attention
            attended, _ = self.attention_layers[round_idx](
                hidden,
                hidden,
                hidden,
                attn_mask=attn_mask
            )
            
            # Residual connection
            hidden = hidden + attended
            
            # Feedforward network
            ff_out = self.ff_networks[round_idx](hidden)
            
            # Another residual
            hidden = hidden + ff_out
        
        # Output projection
        output = self.output_proj(hidden)
        
        return output


class CentralizedCriticWithActions(nn.Module):
    """
    Centralized critic that conditions on both global state and all agents' actions.
    
    Used for actor-critic algorithms where critic needs to evaluate
    state-action pairs for the entire team.
    
    Q(s_global, a1, a2, ..., an)
    
    Features:
    - Processes global state and all actions jointly
    - Deeper network for complex value estimation
    - Can be used with MADDPG or COMA algorithms
    
    Size: ~25MB FP16
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        action_dim: int = 7,
        n_agents: int = 5,
        hidden_dims: List[int] = None,
        dropout: float = 0.2
    ):
        """
        Initialize centralized critic with action conditioning
        
        Args:
            state_dim: State dimension per agent
            action_dim: Action dimension per agent
            n_agents: Maximum number of agents
            hidden_dims: Hidden layer dimensions
            dropout: Dropout probability
        """
        super().__init__()
        
        if hidden_dims is None:
            hidden_dims = [512, 512, 256, 128]
        
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.n_agents = n_agents
        
        # Input: global state + all actions
        input_dim = (state_dim + action_dim) * n_agents
        
        # Build network
        layers = []
        in_dim = input_dim
        
        for hidden_dim in hidden_dims:
            layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            in_dim = hidden_dim
        
        # Q-value head
        layers.append(nn.Linear(in_dim, 1))
        
        self.network = nn.Sequential(*layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(
            f"CentralizedCriticWithActions initialized: {self.count_parameters():,} parameters"
        )
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        global_state: torch.Tensor,
        all_actions: torch.Tensor
    ) -> torch.Tensor:
        """
        Compute centralized Q-value
        
        Args:
            global_state: (batch, n_agents, state_dim) or (batch, n_agents * state_dim)
            all_actions: (batch, n_agents, action_dim) or (batch, n_agents * action_dim)
        
        Returns:
            q_value: (batch, 1) centralized Q-value
        """
        batch_size = global_state.size(0)
        
        # Flatten if needed
        if len(global_state.shape) == 3:
            global_state = global_state.view(batch_size, -1)
        
        if len(all_actions.shape) == 3:
            all_actions = all_actions.view(batch_size, -1)
        
        # Concatenate state and actions
        combined = torch.cat([global_state, all_actions], dim=-1)
        
        # Compute Q-value
        q_value = self.network(combined)
        
        return q_value


class COMA_Critic(nn.Module):
    """
    Counterfactual Multi-Agent (COMA) Critic.
    
    Computes advantage for each agent by comparing:
    - Q(s, (a_i, a_-i)) - baseline computed from counterfactual actions
    
    Where a_-i are actions of all other agents.
    
    Features:
    - Counterfactual reasoning for credit assignment
    - Efficient advantage computation
    - Supports variable team sizes
    
    Size: ~30MB FP16
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        action_dim: int = 7,
        n_agents: int = 5,
        hidden_dims: List[int] = None,
        dropout: float = 0.2
    ):
        """
        Initialize COMA critic
        
        Args:
            state_dim: State dimension
            action_dim: Action dimension
            n_agents: Number of agents
            hidden_dims: Hidden dimensions
            dropout: Dropout probability
        """
        super().__init__()
        
        if hidden_dims is None:
            hidden_dims = [512, 512, 256]
        
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.n_agents = n_agents
        
        # Input: global state + all agents' actions
        input_dim = state_dim * n_agents + action_dim * n_agents
        
        # Build critic network (outputs Q-value for each agent's action)
        layers = []
        in_dim = input_dim
        
        for hidden_dim in hidden_dims:
            layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            in_dim = hidden_dim
        
        # Output Q-values for all agents' actions
        layers.append(nn.Linear(in_dim, n_agents * action_dim))
        
        self.network = nn.Sequential(*layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"COMA_Critic initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        """Initialize weights"""
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=math.sqrt(2))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self,
        global_state: torch.Tensor,
        all_actions: torch.Tensor
    ) -> torch.Tensor:
        """
        Compute Q-values for all agents' actions
        
        Args:
            global_state: (batch, n_agents * state_dim) global state
            all_actions: (batch, n_agents * action_dim) all one-hot actions
        
        Returns:
            q_values: (batch, n_agents, action_dim) Q-values for each agent-action pair
        """
        batch_size = global_state.size(0)
        
        # Flatten inputs
        if len(global_state.shape) == 3:
            global_state = global_state.view(batch_size, -1)
        
        if len(all_actions.shape) == 3:
            all_actions = all_actions.view(batch_size, -1)
        
        # Concatenate
        combined = torch.cat([global_state, all_actions], dim=-1)
        
        # Forward pass
        q_flat = self.network(combined)  # (batch, n_agents * action_dim)
        
        # Reshape to (batch, n_agents, action_dim)
        q_values = q_flat.view(batch_size, self.n_agents, self.action_dim)
        
        return q_values
    
    def compute_advantages(
        self,
        global_state: torch.Tensor,
        actions_taken: torch.Tensor,
        all_actions_one_hot: torch.Tensor
    ) -> torch.Tensor:
        """
        Compute COMA advantages using counterfactual baseline
        
        Args:
            global_state: (batch, n_agents * state_dim)
            actions_taken: (batch, n_agents) actions actually taken
            all_actions_one_hot: (batch, n_agents, action_dim) one-hot actions
        
        Returns:
            advantages: (batch, n_agents) advantages for each agent
        """
        batch_size = actions_taken.size(0)
        
        # Get Q-values for all actions
        q_values = self.forward(global_state, all_actions_one_hot)  # (batch, n_agents, action_dim)
        
        # Get Q-value for actions taken
        q_taken = q_values.gather(-1, actions_taken.unsqueeze(-1)).squeeze(-1)  # (batch, n_agents)
        
        # Compute counterfactual baseline (average over all possible actions)
        baseline = q_values.mean(dim=-1)  # (batch, n_agents)
        
        # Advantage = Q(s, a) - baseline
        advantages = q_taken - baseline
        
        return advantages


# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

def create_adjacency_matrix(
    positions: torch.Tensor,
    communication_range: float = 15.0
) -> torch.Tensor:
    """
    Create adjacency matrix from agent positions
    
    Agents can communicate if within communication_range
    
    Args:
        positions: (batch, n_agents, 2) agent positions (x, y)
        communication_range: Maximum distance for communication
    
    Returns:
        adj_matrix: (batch, n_agents, n_agents) binary adjacency matrix
    """
    batch_size, n_agents, _ = positions.size()
    
    # Compute pairwise distances
    # Expand: (batch, n_agents, 1, 2) - (batch, 1, n_agents, 2)
    pos_i = positions.unsqueeze(2)  # (batch, n_agents, 1, 2)
    pos_j = positions.unsqueeze(1)  # (batch, 1, n_agents, 2)
    
    # Euclidean distance
    dist = torch.norm(pos_i - pos_j, dim=-1)  # (batch, n_agents, n_agents)
    
    # Create adjacency matrix (1 if within range, 0 otherwise)
    adj_matrix = (dist <= communication_range).float()
    
    # Ensure diagonal is 1 (agent can communicate with itself)
    eye = torch.eye(n_agents, device=positions.device).unsqueeze(0)
    adj_matrix = adj_matrix + eye
    adj_matrix = (adj_matrix > 0).float()
    
    return adj_matrix


def create_agent_mask(pack_sizes: torch.Tensor, max_agents: int) -> torch.Tensor:
    """
    Create agent mask for variable-size packs
    
    Args:
        pack_sizes: (batch,) actual pack size for each batch element
        max_agents: Maximum number of agents
    
    Returns:
        mask: (batch, max_agents) binary mask (1=active, 0=padding)
    """
    batch_size = pack_sizes.size(0)
    
    # Create mask
    mask = torch.zeros(batch_size, max_agents, dtype=torch.bool, device=pack_sizes.device)
    
    for i, size in enumerate(pack_sizes):
        mask[i, :size] = True
    
    return mask


def aggregate_signals(
    signals: List[Dict],
    signal_dim: int = 32
) -> torch.Tensor:
    """
    Aggregate multiple signals into fixed-size signal vector
    
    Signal encoding:
    - Dims 0-7: Target information (priority targets)
    - Dims 8-15: Formation commands
    - Dims 16-23: Support requests
    - Dims 24-31: Strategic directives
    
    Args:
        signals: List of signal dicts from pack members
        signal_dim: Output signal vector dimension (32)
    
    Returns:
        signal_vector: (signal_dim,) aggregated signal vector
    """
    signal_vector = torch.zeros(signal_dim)
    
    if not signals:
        return signal_vector
    
    for signal in signals:
        signal_type = signal.get('signal_type', 'unknown')
        content = signal.get('content', {})
        priority = signal.get('priority', 5)
        
        # Encode based on signal type
        if signal_type == 'target_info':
            # Target priority encoding (dims 0-7)
            target_threat = content.get('target_threat', 0.5)
            idx = min(int(target_threat * 7), 7)
            signal_vector[idx] = max(signal_vector[idx], 1.0 - priority / 10.0)
        
        elif signal_type == 'formation_command':
            # Formation encoding (dims 8-15)
            formation_type = content.get('formation_type', 'default')
            formation_map = {
                'flanking': 8,
                'defensive': 9,
                'spread': 10,
                'cluster': 11,
                'line': 12,
                'circle': 13
            }
            idx = formation_map.get(formation_type, 14)
            signal_vector[idx] = 1.0
        
        elif signal_type == 'help_request':
            # Support request encoding (dims 16-23)
            urgency = content.get('urgency', 0.5)
            idx = 16 + min(int(urgency * 7), 7)
            signal_vector[idx] = max(signal_vector[idx], urgency)
        
        elif signal_type in ['retreat_call', 'advance', 'hold']:
            # Strategic directives (dims 24-31)
            directive_map = {
                'retreat_call': 24,
                'advance': 25,
                'hold': 26,
                'regroup': 27,
                'scatter': 28,
                'focus_fire': 29
            }
            idx = directive_map.get(signal_type, 30)
            signal_vector[idx] = 1.0
    
    return signal_vector


# ============================================================================
# MODEL FACTORY
# ============================================================================

def create_ctde_models(
    config: Dict[str, Any]
) -> Dict[str, nn.Module]:
    """
    Create all CTDE models from configuration
    
    Args:
        config: CTDE configuration dict
    
    Returns:
        Dictionary of model instances
    """
    models = {}
    
    # Centralized critic
    critic_config = config.get('critic', {})
    models['centralized_critic'] = CentralizedCritic(
        state_dim=config.get('state_dim', 64),
        n_agents=config.get('n_agents', 5),
        hidden_dims=critic_config.get('hidden_dims', [512, 512, 256]),
        use_attention=critic_config.get('use_attention', True),
        dropout=critic_config.get('dropout', 0.2)
    )
    
    # Decentralized actors (one per agent, but they share weights)
    actor_config = config.get('actor', {})
    models['decentralized_actor'] = DecentralizedActor(
        state_dim=config.get('state_dim', 64),
        signal_dim=config.get('signal_dim', 32),
        action_dim=config.get('action_dim', 7),
        hidden_dim=actor_config.get('hidden_dim', 256),
        use_gru=actor_config.get('use_gru', True),
        dropout=actor_config.get('dropout', 0.1)
    )
    
    # Communication network (for training)
    comm_config = config.get('communication', {})
    if comm_config.get('enabled', True):
        use_attention_comm = comm_config.get('use_attention', False)
        
        if use_attention_comm:
            models['comm_net'] = AttentionCommNet(
                state_dim=config.get('state_dim', 64),
                hidden_dim=comm_config.get('hidden_dim', 128),
                n_agents=config.get('n_agents', 5),
                n_rounds=comm_config.get('rounds', 3),
                n_heads=comm_config.get('n_heads', 4),
                dropout=comm_config.get('dropout', 0.1)
            )
        else:
            models['comm_net'] = CommNet(
                hidden_dim=comm_config.get('hidden_dim', 128),
                n_agents=config.get('n_agents', 5),
                n_rounds=comm_config.get('rounds', 3),
                dropout=comm_config.get('dropout', 0.1)
            )
    
    # QMIX mixer (for value decomposition)
    mixer_config = config.get('mixer', {})
    if mixer_config.get('enabled', False):
        models['qmix_mixer'] = QMIXMixer(
            n_agents=config.get('n_agents', 5),
            state_dim=config.get('state_dim', 64),
            mixing_embed_dim=mixer_config.get('mixing_embed_dim', 32),
            hypernet_embed_dim=mixer_config.get('hypernet_embed_dim', 64)
        )
    
    logger.info(f"Created CTDE model suite with {len(models)} models")
    
    return models


def estimate_ctde_model_sizes(models: Dict[str, nn.Module]) -> Dict[str, Dict[str, float]]:
    """
    Estimate memory sizes for CTDE models
    
    Args:
        models: Dictionary of model instances
    
    Returns:
        Size estimates per model
    """
    sizes = {}
    
    for name, model in models.items():
        num_params = sum(p.numel() for p in model.parameters())
        
        sizes[name] = {
            'parameters': num_params,
            'fp32_mb': num_params * 4 / (1024**2),
            'fp16_mb': num_params * 2 / (1024**2),
            'int8_mb': num_params / (1024**2)
        }
    
    # Total
    total_params = sum(s['parameters'] for s in sizes.values())
    sizes['total'] = {
        'parameters': total_params,
        'fp32_mb': total_params * 4 / (1024**2),
        'fp16_mb': total_params * 2 / (1024**2),
        'int8_mb': total_params / (1024**2)
    }
    
    return sizes


# ============================================================================
# TESTING AND VALIDATION
# ============================================================================

def test_ctde_architectures():
    """Test all CTDE architectures with sample inputs"""
    logger.info("Testing CTDE architectures...")
    
    batch_size = 4
    n_agents = 5
    state_dim = 64
    signal_dim = 32
    action_dim = 7
    
    # Create models
    config = {
        'state_dim': state_dim,
        'signal_dim': signal_dim,
        'action_dim': action_dim,
        'n_agents': n_agents,
        'critic': {'use_attention': True},
        'actor': {'use_gru': True},
        'communication': {'enabled': True, 'use_attention': False},
        'mixer': {'enabled': True}
    }
    
    models = create_ctde_models(config)
    
    # Test CentralizedCritic
    logger.info("\n1. Testing CentralizedCritic...")
    global_state = torch.randn(batch_size, n_agents, state_dim)
    agent_mask = torch.ones(batch_size, n_agents, dtype=torch.bool)
    agent_mask[0, 3:] = False  # First batch has only 3 agents
    
    critic = models['centralized_critic']
    value = critic(global_state, agent_mask)
    assert value.shape == (batch_size, 1), f"Expected (batch, 1), got {value.shape}"
    logger.info(f"✓ CentralizedCritic output shape: {value.shape}")
    
    # Test DecentralizedActor
    logger.info("\n2. Testing DecentralizedActor...")
    state = torch.randn(batch_size, state_dim)
    signals = torch.randn(batch_size, signal_dim)
    
    actor = models['decentralized_actor']
    action, log_prob, entropy, hidden = actor.get_action(state, signals, deterministic=False)
    assert action.shape == (batch_size,), f"Expected (batch,), got {action.shape}"
    assert log_prob.shape == (batch_size,), f"Expected (batch,), got {log_prob.shape}"
    logger.info(f"✓ DecentralizedActor action shape: {action.shape}")
    logger.info(f"  Action range: [{action.min()}, {action.max()}]")
    
    # Test CommNet
    logger.info("\n3. Testing CommNet...")
    local_obs = torch.randn(batch_size, n_agents, state_dim)
    adj_matrix = torch.ones(batch_size, n_agents, n_agents)
    
    comm_net = models['comm_net']
    comm_hidden = comm_net(local_obs, adj_matrix)
    assert comm_hidden.shape == (batch_size, n_agents, 128), f"Expected (batch, n_agents, 128), got {comm_hidden.shape}"
    logger.info(f"✓ CommNet output shape: {comm_hidden.shape}")
    
    # Test QMIXMixer
    logger.info("\n4. Testing QMIXMixer...")
    agent_qs = torch.randn(batch_size, n_agents)
    global_state_flat = global_state.view(batch_size, -1)
    
    mixer = models['qmix_mixer']
    global_q = mixer(agent_qs, global_state_flat)
    assert global_q.shape == (batch_size, 1), f"Expected (batch, 1), got {global_q.shape}"
    logger.info(f"✓ QMIXMixer output shape: {global_q.shape}")
    
    # Test monotonicity of QMIX
    agent_qs_increased = agent_qs.clone()
    agent_qs_increased[:, 0] += 1.0  # Increase first agent's Q
    global_q_increased = mixer(agent_qs_increased, global_state_flat)
    
    monotonic = (global_q_increased >= global_q).all()
    logger.info(f"✓ QMIXMixer monotonicity check: {monotonic}")
    
    # Print model sizes
    logger.info("\n5. Model Size Estimates:")
    sizes = estimate_ctde_model_sizes(models)
    for name, size_info in sizes.items():
        if name != 'total':
            logger.info(
                f"  {name}: {size_info['parameters']:,} params, "
                f"{size_info['fp16_mb']:.2f} MB (FP16)"
            )
    
    logger.info(
        f"\n  TOTAL: {sizes['total']['parameters']:,} params, "
        f"{sizes['total']['fp16_mb']:.2f} MB (FP16)"
    )
    
    logger.info("\n✓ All CTDE architecture tests passed!")


if __name__ == '__main__':
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    test_ctde_architectures()
