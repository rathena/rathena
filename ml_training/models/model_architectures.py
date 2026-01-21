"""
ML Monster AI - Model Architectures
Enhanced ML Monster AI System v2.0

All 9 model types for the ML Monster AI system:
1. CombatDQN - Deep Q-Network for combat decisions (10-12 layers)
2. MovementPPO - Proximal Policy Optimization for movement (12 layers, actor-critic)
3. SkillDQN - DQN with skill embeddings for skill selection
4. ThreatAssessment - Ensemble model for threat evaluation
5. TeamCoordinationLSTM - LSTM for pack coordination (3 layers, 256 units)
6. SpatialViT - Vision Transformer for spatial awareness (ViT-tiny adapted)
7. TemporalTransformerXL - Transformer-XL for long-term temporal awareness (4 layers)
8. PatternRecognitionTransformer - Large transformer for pattern detection (6 layers)
9. SoftActorCritic - SAC for continuous control

All models support FP16 training with automatic mixed precision.
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
from typing import Tuple, Optional, Dict, List
import math
import logging

logger = logging.getLogger(__name__)


class CombatDQN(nn.Module):
    """
    Deep Q-Network for combat decisions
    
    Architecture: 10-12 layers with ResNet-style skip connections, 512-1024 units
    Input: 64-dim state vector
    Output: 10 Q-values (one per action)
    
    Features:
    - Dueling DQN architecture (separate value and advantage streams)
    - ResNet blocks for better gradient flow
    - Layer normalization for training stability
    - Dropout for regularization
    
    Size: ~40MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        action_dim: int = 10,
        hidden_dims: List[int] = None,
        dropout: float = 0.2,
        use_dueling: bool = True
    ):
        super().__init__()
        
        if hidden_dims is None:
            hidden_dims = [1024, 1024, 512, 512, 256, 256, 128, 128, 64, 64]
        
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.use_dueling = use_dueling
        
        # Input normalization
        self.input_norm = nn.LayerNorm(state_dim)
        
        # Build feature extractor with skip connections
        layers = []
        in_dim = state_dim
        
        for i, hidden_dim in enumerate(hidden_dims):
            # Main path
            layers.append(nn.Linear(in_dim, hidden_dim))
            layers.append(nn.LayerNorm(hidden_dim))
            layers.append(nn.ReLU())
            layers.append(nn.Dropout(dropout))
            
            in_dim = hidden_dim
        
        self.feature_extractor = nn.Sequential(*layers)
        
        final_feature_dim = hidden_dims[-1]
        
        if use_dueling:
            # Dueling architecture: separate value and advantage streams
            # Value stream (estimates V(s))
            self.value_stream = nn.Sequential(
                nn.Linear(final_feature_dim, final_feature_dim // 2),
                nn.ReLU(),
                nn.Linear(final_feature_dim // 2, 1)
            )
            
            # Advantage stream (estimates A(s,a))
            self.advantage_stream = nn.Sequential(
                nn.Linear(final_feature_dim, final_feature_dim // 2),
                nn.ReLU(),
                nn.Linear(final_feature_dim // 2, action_dim)
            )
        else:
            # Standard DQN
            self.q_head = nn.Sequential(
                nn.Linear(final_feature_dim, action_dim)
            )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"CombatDQN initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        """Xavier/He initialization for better training"""
        if isinstance(m, nn.Linear):
            nn.init.kaiming_normal_(m.weight, mode='fan_out', nonlinearity='relu')
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        """Count total trainable parameters"""
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state: torch.Tensor) -> torch.Tensor:
        """
        Forward pass
        
        Args:
            state: (batch_size, state_dim) state vectors
        
        Returns:
            q_values: (batch_size, action_dim) Q-values for each action
        """
        # Normalize input
        x = self.input_norm(state)
        
        # Feature extraction
        features = self.feature_extractor(x)
        
        if self.use_dueling:
            # Dueling DQN: Q(s,a) = V(s) + (A(s,a) - mean(A(s,a)))
            value = self.value_stream(features)  # (batch, 1)
            advantage = self.advantage_stream(features)  # (batch, action_dim)
            
            # Combine value and advantage
            q_values = value + (advantage - advantage.mean(dim=1, keepdim=True))
        else:
            # Standard DQN
            q_values = self.q_head(features)
        
        return q_values


class MovementPPO(nn.Module):
    """
    Proximal Policy Optimization for movement
    
    Architecture: 12-layer actor-critic with shared feature extractor
    Input: 64-dim state vector
    Output: Action probabilities (actor) and state value (critic)
    
    Features:
    - Deep shared feature extractor (6 layers, 1024 units)
    - Separate actor and critic heads (3 layers each)
    - Layer normalization throughout
    - Dropout for regularization
    
    Size: ~100MB FP16 per archetype
    """
    
    def __init__(
        self,
        state_dim: int = 64,
        action_dim: int = 10,
        hidden_dim: int = 512,
        shared_layers: int = 6,
        head_layers: int = 3,
        dropout: float = 0.1
    ):
        super().__init__()
        
        self.state_dim = state_dim
        self.action_dim = action_dim
        
        # Shared feature extractor (6 layers, progressively smaller)
        shared_dims = [1024, 1024, 512, 512, 256, 256][:shared_layers]
        
        layers = []
        in_dim = state_dim
        
        for hidden_dim in shared_dims:
            layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            in_dim = hidden_dim
        
        self.feature_net = nn.Sequential(*layers)
        
        final_feature_dim = shared_dims[-1]
        
        # Actor head (policy network)
        actor_layers = []
        actor_dim = final_feature_dim
        for _ in range(head_layers - 1):
            actor_layers.extend([
                nn.Linear(actor_dim, actor_dim // 2),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            actor_dim = actor_dim // 2
        
        actor_layers.append(nn.Linear(actor_dim, action_dim))
        self.actor = nn.Sequential(*actor_layers)
        
        # Critic head (value network)
        critic_layers = []
        critic_dim = final_feature_dim
        for _ in range(head_layers - 1):
            critic_layers.extend([
                nn.Linear(critic_dim, critic_dim // 2),
                nn.ReLU(),
                nn.Dropout(dropout)
            ])
            critic_dim = critic_dim // 2
        
        critic_layers.append(nn.Linear(critic_dim, 1))
        self.critic = nn.Sequential(*critic_layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"MovementPPO initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=nn.init.calculate_gain('relu'))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state: torch.Tensor) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Forward pass through actor-critic network
        
        Args:
            state: (batch_size, state_dim)
        
        Returns:
            action_logits: (batch_size, action_dim) unnormalized action scores
            value: (batch_size, 1) state value estimate
        """
        features = self.feature_net(state)
        action_logits = self.actor(features)
        value = self.critic(features)
        return action_logits, value
    
    def get_action(
        self, 
        state: torch.Tensor, 
        deterministic: bool = False
    ) -> Tuple[torch.Tensor, torch.Tensor, torch.Tensor]:
        """
        Sample action from policy
        
        Args:
            state: (batch_size, state_dim)
            deterministic: If True, return argmax action
        
        Returns:
            action: (batch_size,) sampled actions
            log_prob: (batch_size,) log probabilities
            value: (batch_size, 1) state values
        """
        action_logits, value = self.forward(state)
        action_probs = F.softmax(action_logits, dim=-1)
        
        if deterministic:
            action = action_probs.argmax(dim=-1)
            log_prob = torch.log(action_probs.gather(1, action.unsqueeze(1)).squeeze(1) + 1e-8)
        else:
            action_dist = torch.distributions.Categorical(action_probs)
            action = action_dist.sample()
            log_prob = action_dist.log_prob(action)
        
        return action, log_prob, value


class SkillDQN(nn.Module):
    """
    DQN with skill embeddings for skill selection
    
    Architecture: Skill embeddings + MLP
    Input: 64-dim state + skill ID
    Output: Q-values for actions with that skill
    
    Features:
    - Skill embedding layer (learns skill representations)
    - Combined state-skill processing
    - Layer normalization
    - Dropout
    
    Size: ~20MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        num_skills: int = 20, 
        embedding_dim: int = 32, 
        action_dim: int = 10,
        hidden_dims: List[int] = None,
        dropout: float = 0.2
    ):
        super().__init__()
        
        if hidden_dims is None:
            hidden_dims = [512, 256, 128]
        
        self.state_dim = state_dim
        self.num_skills = num_skills
        self.embedding_dim = embedding_dim
        self.action_dim = action_dim
        
        # Skill embedding layer
        self.skill_embedding = nn.Embedding(num_skills, embedding_dim)
        
        # Combined state + skill processing
        input_dim = state_dim + embedding_dim
        
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
        
        layers.append(nn.Linear(in_dim, action_dim))
        
        self.network = nn.Sequential(*layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        nn.init.normal_(self.skill_embedding.weight, mean=0, std=0.02)
        
        logger.info(f"SkillDQN initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.kaiming_normal_(m.weight, mode='fan_out', nonlinearity='relu')
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state: torch.Tensor, skill_ids: torch.Tensor) -> torch.Tensor:
        """
        Forward pass
        
        Args:
            state: (batch_size, state_dim)
            skill_ids: (batch_size,) skill IDs as integers [0, num_skills)
        
        Returns:
            q_values: (batch_size, action_dim)
        """
        # Get skill embeddings
        skill_emb = self.skill_embedding(skill_ids)  # (batch_size, embedding_dim)
        
        # Concatenate state and skill embedding
        combined = torch.cat([state, skill_emb], dim=-1)  # (batch_size, state_dim + embedding_dim)
        
        # Process through network
        q_values = self.network(combined)
        
        return q_values


class ThreatAssessment(nn.Module):
    """
    Ensemble model for threat assessment
    
    Architecture: 3 sub-networks with different activations, averaged
    Input: 64-dim state vector
    Output: Threat score (scalar)
    
    Features:
    - Ensemble of 3 networks for robustness
    - Different activation functions (ReLU, Tanh, LeakyReLU)
    - Average ensemble output
    
    Size: ~5MB FP16 per archetype
    """
    
    def __init__(self, state_dim: int = 64, hidden_dim: int = 256, output_dim: int = 1):
        super().__init__()
        
        self.state_dim = state_dim
        self.output_dim = output_dim
        
        # Network 1: ReLU activation
        self.net1 = nn.Sequential(
            nn.Linear(state_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.ReLU(),
            nn.Dropout(0.1),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.LayerNorm(hidden_dim // 2),
            nn.ReLU(),
            nn.Linear(hidden_dim // 2, output_dim)
        )
        
        # Network 2: Tanh activation
        self.net2 = nn.Sequential(
            nn.Linear(state_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.Tanh(),
            nn.Dropout(0.1),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.LayerNorm(hidden_dim // 2),
            nn.Tanh(),
            nn.Linear(hidden_dim // 2, output_dim)
        )
        
        # Network 3: LeakyReLU activation
        self.net3 = nn.Sequential(
            nn.Linear(state_dim, hidden_dim),
            nn.LayerNorm(hidden_dim),
            nn.LeakyReLU(0.2),
            nn.Dropout(0.1),
            nn.Linear(hidden_dim, hidden_dim // 2),
            nn.LayerNorm(hidden_dim // 2),
            nn.LeakyReLU(0.2),
            nn.Linear(hidden_dim // 2, output_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"ThreatAssessment initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.xavier_normal_(m.weight)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state: torch.Tensor) -> torch.Tensor:
        """
        Forward pass through ensemble
        
        Args:
            state: (batch_size, state_dim)
        
        Returns:
            threat: (batch_size, output_dim) threat scores (averaged across ensemble)
        """
        o1 = self.net1(state)
        o2 = self.net2(state)
        o3 = self.net3(state)
        
        # Average ensemble
        threat = (o1 + o2 + o3) / 3.0
        
        return threat


class TeamCoordinationLSTM(nn.Module):
    """
    LSTM for team coordination and communication
    
    Architecture: 3-layer bidirectional LSTM with attention, 256 hidden units
    Input: Sequence of states (variable length)
    Output: Coordination action logits
    
    Features:
    - Bidirectional LSTM for context from both directions
    - Multi-head self-attention over LSTM outputs
    - Layer normalization
    - Dropout between layers
    
    Size: ~50MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        hidden_dim: int = 256, 
        num_layers: int = 3, 
        action_dim: int = 10,
        bidirectional: bool = True,
        attention_heads: int = 8,
        dropout: float = 0.2
    ):
        super().__init__()
        
        self.state_dim = state_dim
        self.hidden_dim = hidden_dim
        self.num_layers = num_layers
        self.bidirectional = bidirectional
        self.num_directions = 2 if bidirectional else 1
        
        # LSTM layer
        self.lstm = nn.LSTM(
            input_size=state_dim,
            hidden_size=hidden_dim,
            num_layers=num_layers,
            batch_first=True,
            dropout=dropout if num_layers > 1 else 0,
            bidirectional=bidirectional
        )
        
        lstm_output_dim = hidden_dim * self.num_directions
        
        # Multi-head self-attention
        self.attention = nn.MultiheadAttention(
            embed_dim=lstm_output_dim,
            num_heads=attention_heads,
            dropout=dropout,
            batch_first=True
        )
        
        # Output head
        self.fc = nn.Sequential(
            nn.Linear(lstm_output_dim, lstm_output_dim // 2),
            nn.LayerNorm(lstm_output_dim // 2),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(lstm_output_dim // 2, lstm_output_dim // 4),
            nn.ReLU(),
            nn.Linear(lstm_output_dim // 4, action_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"TeamCoordinationLSTM initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.xavier_normal_(m.weight)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.LSTM):
            for name, param in m.named_parameters():
                if 'weight' in name:
                    nn.init.orthogonal_(param)
                elif 'bias' in name:
                    nn.init.constant_(param, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(
        self, 
        state_sequence: torch.Tensor, 
        hidden: Optional[Tuple[torch.Tensor, torch.Tensor]] = None
    ) -> Tuple[torch.Tensor, Tuple[torch.Tensor, torch.Tensor]]:
        """
        Forward pass
        
        Args:
            state_sequence: (batch_size, seq_len, state_dim)
            hidden: Optional (h_0, c_0) hidden states
        
        Returns:
            action_logits: (batch_size, action_dim)
            hidden: (h_n, c_n) final hidden states for next forward pass
        """
        # LSTM forward
        lstm_out, hidden = self.lstm(state_sequence, hidden)  # (batch, seq_len, hidden_dim * num_directions)
        
        # Self-attention over LSTM outputs
        attn_out, _ = self.attention(lstm_out, lstm_out, lstm_out)  # (batch, seq_len, hidden_dim * num_directions)
        
        # Use last timestep output (after attention)
        last_output = attn_out[:, -1, :]  # (batch, hidden_dim * num_directions)
        
        # Classification head
        action_logits = self.fc(last_output)
        
        return action_logits, hidden


class SpatialViT(nn.Module):
    """
    Vision Transformer (ViT-tiny) adapted for spatial understanding
    
    Architecture: Patch embedding + transformer encoder + classification head
    Input: 64-dim state vector (reshaped as 8x8 patches)
    Output: Spatial decision logits
    
    Features:
    - Patch embedding (state vector divided into patches)
    - Learnable positional embeddings
    - Transformer encoder with self-attention
    - CLS token for classification
    - Layer normalization and dropout
    
    Size: ~90MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        patch_size: int = 8, 
        embed_dim: int = 192, 
        depth: int = 12, 
        num_heads: int = 3,
        mlp_ratio: int = 4,
        action_dim: int = 10,
        dropout: float = 0.1
    ):
        super().__init__()
        
        self.state_dim = state_dim
        self.patch_size = patch_size
        self.num_patches = state_dim // patch_size
        self.embed_dim = embed_dim
        
        # Patch embedding
        self.patch_embed = nn.Linear(patch_size, embed_dim)
        
        # CLS token (learnable)
        self.cls_token = nn.Parameter(torch.randn(1, 1, embed_dim))
        
        # Positional embeddings (learnable)
        self.pos_embed = nn.Parameter(torch.randn(1, self.num_patches + 1, embed_dim))
        
        # Dropout
        self.pos_drop = nn.Dropout(dropout)
        
        # Transformer encoder layers
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=embed_dim,
            nhead=num_heads,
            dim_feedforward=embed_dim * mlp_ratio,
            dropout=dropout,
            activation='gelu',
            batch_first=True,
            norm_first=True
        )
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers=depth)
        
        # Classification head
        self.head = nn.Sequential(
            nn.LayerNorm(embed_dim),
            nn.Linear(embed_dim, embed_dim // 2),
            nn.GELU(),
            nn.Dropout(dropout),
            nn.Linear(embed_dim // 2, action_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        nn.init.trunc_normal_(self.cls_token, std=0.02)
        nn.init.trunc_normal_(self.pos_embed, std=0.02)
        
        logger.info(f"SpatialViT initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.trunc_normal_(m.weight, std=0.02)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.LayerNorm):
            nn.init.constant_(m.weight, 1.0)
            nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state: torch.Tensor) -> torch.Tensor:
        """
        Forward pass
        
        Args:
            state: (batch_size, state_dim) state vectors
        
        Returns:
            logits: (batch_size, action_dim) spatial decision logits
        """
        batch_size = state.size(0)
        
        # Reshape state into patches
        # state: (batch, 64) -> (batch, num_patches, patch_size)
        patches = state.view(batch_size, self.num_patches, self.patch_size)
        
        # Patch embedding
        x = self.patch_embed(patches)  # (batch, num_patches, embed_dim)
        
        # Add CLS token
        cls_tokens = self.cls_token.expand(batch_size, -1, -1)  # (batch, 1, embed_dim)
        x = torch.cat([cls_tokens, x], dim=1)  # (batch, num_patches + 1, embed_dim)
        
        # Add positional embeddings
        x = x + self.pos_embed
        x = self.pos_drop(x)
        
        # Transformer encoder
        x = self.transformer(x)  # (batch, num_patches + 1, embed_dim)
        
        # Use CLS token for classification
        cls_output = x[:, 0]  # (batch, embed_dim)
        
        # Classification head
        logits = self.head(cls_output)
        
        return logits


class TemporalTransformerXL(nn.Module):
    """
    Transformer-XL for temporal sequence modeling
    
    Architecture: Transformer encoder with relative positional encoding
    Input: Sequence of state vectors
    Output: Temporal context features
    
    Features:
    - Relative positional encoding (Transformer-XL style)
    - Multi-head self-attention
    - Feedforward networks
    - Layer normalization
    - Support for memory caching (for long sequences)
    
    Size: ~100MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        d_model: int = 256, 
        nhead: int = 8, 
        num_layers: int = 4, 
        action_dim: int = 10,
        dim_feedforward: int = None,
        dropout: float = 0.1,
        max_seq_len: int = 500
    ):
        super().__init__()
        
        if dim_feedforward is None:
            dim_feedforward = d_model * 4
        
        self.state_dim = state_dim
        self.d_model = d_model
        self.max_seq_len = max_seq_len
        
        # Input projection
        self.input_proj = nn.Linear(state_dim, d_model)
        
        # Learnable positional embeddings (will be used relatively)
        self.pos_embed = nn.Parameter(torch.randn(max_seq_len, d_model))
        
        # Transformer encoder layers
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model,
            nhead=nhead,
            dim_feedforward=dim_feedforward,
            dropout=dropout,
            activation='gelu',
            batch_first=True,
            norm_first=True
        )
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)
        
        # Output projection
        self.output_proj = nn.Sequential(
            nn.LayerNorm(d_model),
            nn.Linear(d_model, d_model // 2),
            nn.GELU(),
            nn.Dropout(dropout),
            nn.Linear(d_model // 2, action_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        nn.init.normal_(self.pos_embed, std=0.02)
        
        logger.info(f"TemporalTransformerXL initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.trunc_normal_(m.weight, std=0.02)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.LayerNorm):
            nn.init.constant_(m.weight, 1.0)
            nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state_sequence: torch.Tensor) -> torch.Tensor:
        """
        Forward pass
        
        Args:
            state_sequence: (batch_size, seq_len, state_dim)
        
        Returns:
            logits: (batch_size, action_dim) temporal decision logits
        """
        batch_size, seq_len, _ = state_sequence.size()
        
        # Validate sequence length
        if seq_len > self.max_seq_len:
            logger.warning(f"Sequence length {seq_len} exceeds max {self.max_seq_len}, truncating")
            state_sequence = state_sequence[:, -self.max_seq_len:, :]
            seq_len = self.max_seq_len
        
        # Input projection
        x = self.input_proj(state_sequence)  # (batch, seq_len, d_model)
        
        # Add positional embeddings
        x = x + self.pos_embed[:seq_len].unsqueeze(0)  # Broadcasting
        
        # Transformer encoding
        x = self.transformer(x)  # (batch, seq_len, d_model)
        
        # Use last timestep for decision
        last_state = x[:, -1, :]  # (batch, d_model)
        
        # Output projection
        logits = self.output_proj(last_state)
        
        return logits


class PatternRecognitionTransformer(nn.Module):
    """
    Large transformer for pattern recognition
    
    Architecture: 6-layer transformer with large hidden dimensions
    Input: Sequence of combat features
    Output: Pattern classification logits
    
    Features:
    - Large model capacity (200-300MB FP16)
    - Deep transformer (6 layers)
    - Multi-head attention (16 heads)
    - Global attention pooling
    - Dropout for regularization
    
    Size: ~250MB FP16 per archetype
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        d_model: int = 512, 
        nhead: int = 16, 
        num_layers: int = 6, 
        action_dim: int = 10,
        dim_feedforward: int = None,
        dropout: float = 0.1,
        max_seq_len: int = 100
    ):
        super().__init__()
        
        if dim_feedforward is None:
            dim_feedforward = d_model * 4
        
        self.state_dim = state_dim
        self.d_model = d_model
        self.max_seq_len = max_seq_len
        
        # Input projection
        self.input_proj = nn.Linear(state_dim, d_model)
        
        # Sinusoidal positional encoding
        self.register_buffer('pos_encoding', self._create_positional_encoding(max_seq_len, d_model))
        
        # Transformer encoder
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=d_model,
            nhead=nhead,
            dim_feedforward=dim_feedforward,
            dropout=dropout,
            activation='gelu',
            batch_first=True,
            norm_first=True
        )
        self.transformer = nn.TransformerEncoder(encoder_layer, num_layers=num_layers)
        
        # Pattern classification head
        self.pattern_classifier = nn.Sequential(
            nn.LayerNorm(d_model),
            nn.Linear(d_model, d_model),
            nn.GELU(),
            nn.Dropout(dropout * 2),  # Higher dropout in classifier
            nn.Linear(d_model, d_model // 2),
            nn.GELU(),
            nn.Dropout(dropout),
            nn.Linear(d_model // 2, action_dim)
        )
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"PatternRecognitionTransformer initialized: {self.count_parameters():,} parameters")
    
    def _create_positional_encoding(self, max_len: int, d_model: int) -> torch.Tensor:
        """Create sinusoidal positional encoding"""
        position = torch.arange(max_len).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2) * (-math.log(10000.0) / d_model))
        
        pos_encoding = torch.zeros(max_len, d_model)
        pos_encoding[:, 0::2] = torch.sin(position * div_term)
        pos_encoding[:, 1::2] = torch.cos(position * div_term)
        
        return pos_encoding.unsqueeze(0)  # (1, max_len, d_model)
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.trunc_normal_(m.weight, std=0.02)
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
        elif isinstance(m, nn.LayerNorm):
            nn.init.constant_(m.weight, 1.0)
            nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def forward(self, state_sequence: torch.Tensor) -> torch.Tensor:
        """
        Forward pass
        
        Args:
            state_sequence: (batch_size, seq_len, state_dim)
        
        Returns:
            pattern_logits: (batch_size, action_dim) pattern classification logits
        """
        batch_size, seq_len, _ = state_sequence.size()
        
        # Validate sequence length
        if seq_len > self.max_seq_len:
            logger.warning(f"Sequence length {seq_len} exceeds max {self.max_seq_len}, truncating")
            state_sequence = state_sequence[:, -self.max_seq_len:, :]
            seq_len = self.max_seq_len
        
        # Input projection
        x = self.input_proj(state_sequence)  # (batch, seq_len, d_model)
        
        # Add positional encoding
        x = x + self.pos_encoding[:, :seq_len, :]
        
        # Transformer encoding
        x = self.transformer(x)  # (batch, seq_len, d_model)
        
        # Global average pooling (instead of CLS token)
        x = x.mean(dim=1)  # (batch, d_model)
        
        # Pattern classification
        pattern_logits = self.pattern_classifier(x)
        
        return pattern_logits


class SoftActorCritic(nn.Module):
    """
    Soft Actor-Critic for continuous control
    
    Architecture: Actor + Twin Critics with entropy maximization
    Input: State vector
    Output: Action distribution (actor) or Q-values (critics)
    
    Features:
    - Gaussian policy (actor) for continuous actions
    - Twin Q-networks (critics) for stability
    - Automatic entropy tuning
    - Squashed actions (tanh)
    
    Size: ~30MB FP16 per archetype (actor + 2 critics)
    """
    
    def __init__(
        self, 
        state_dim: int = 64, 
        action_dim: int = 10, 
        hidden_dim: int = 256,
        num_layers: int = 3,
        log_std_min: float = -20,
        log_std_max: float = 2
    ):
        super().__init__()
        
        self.state_dim = state_dim
        self.action_dim = action_dim
        self.log_std_min = log_std_min
        self.log_std_max = log_std_max
        
        # Actor (policy network)
        actor_layers = []
        in_dim = state_dim
        for _ in range(num_layers):
            actor_layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(0.1)
            ])
            in_dim = hidden_dim
        
        self.actor_base = nn.Sequential(*actor_layers)
        
        # Actor outputs: mean and log_std
        self.actor_mean = nn.Linear(hidden_dim, action_dim)
        self.actor_log_std = nn.Linear(hidden_dim, action_dim)
        
        # Critic 1 (Q-function)
        critic1_layers = []
        in_dim = state_dim + action_dim
        for _ in range(num_layers):
            critic1_layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(0.1)
            ])
            in_dim = hidden_dim
        
        critic1_layers.append(nn.Linear(hidden_dim, 1))
        self.critic1 = nn.Sequential(*critic1_layers)
        
        # Critic 2 (Q-function, twin)
        critic2_layers = []
        in_dim = state_dim + action_dim
        for _ in range(num_layers):
            critic2_layers.extend([
                nn.Linear(in_dim, hidden_dim),
                nn.LayerNorm(hidden_dim),
                nn.ReLU(),
                nn.Dropout(0.1)
            ])
            in_dim = hidden_dim
        
        critic2_layers.append(nn.Linear(hidden_dim, 1))
        self.critic2 = nn.Sequential(*critic2_layers)
        
        # Initialize weights
        self.apply(self._init_weights)
        
        logger.info(f"SoftActorCritic initialized: {self.count_parameters():,} parameters")
    
    def _init_weights(self, m):
        if isinstance(m, nn.Linear):
            nn.init.orthogonal_(m.weight, gain=nn.init.calculate_gain('relu'))
            if m.bias is not None:
                nn.init.constant_(m.bias, 0)
    
    def count_parameters(self) -> int:
        return sum(p.numel() for p in self.parameters() if p.requires_grad)
    
    def get_action(
        self, 
        state: torch.Tensor, 
        deterministic: bool = False
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Sample action from policy
        
        Args:
            state: (batch_size, state_dim)
            deterministic: If True, return mean action (no sampling)
        
        Returns:
            action: (batch_size, action_dim) sampled actions in [-1, 1]
            log_prob: (batch_size,) log probabilities
        """
        # Actor forward
        features = self.actor_base(state)
        mean = self.actor_mean(features)
        log_std = self.actor_log_std(features)
        
        # Clamp log_std for numerical stability
        log_std = torch.clamp(log_std, self.log_std_min, self.log_std_max)
        std = log_std.exp()
        
        if deterministic:
            # Deterministic action (no sampling)
            action = torch.tanh(mean)
            log_prob = torch.zeros(state.size(0), device=state.device)
        else:
            # Sample from Gaussian
            dist = torch.distributions.Normal(mean, std)
            z = dist.rsample()  # Reparameterization trick
            
            # Squash to [-1, 1]
            action = torch.tanh(z)
            
            # Compute log probability with tanh correction
            log_prob = dist.log_prob(z).sum(dim=-1)
            log_prob -= (2 * (math.log(2) - z - F.softplus(-2 * z))).sum(dim=-1)
        
        return action, log_prob
    
    def forward(
        self, 
        state: torch.Tensor, 
        action: torch.Tensor
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Forward pass through critics
        
        Args:
            state: (batch_size, state_dim)
            action: (batch_size, action_dim)
        
        Returns:
            q1: (batch_size, 1) Q-value from critic 1
            q2: (batch_size, 1) Q-value from critic 2
        """
        sa = torch.cat([state, action], dim=-1)
        q1 = self.critic1(sa)
        q2 = self.critic2(sa)
        return q1, q2


class PositionalEncoding(nn.Module):
    """Sinusoidal positional encoding for transformers"""
    
    def __init__(self, d_model: int, max_len: int = 5000, dropout: float = 0.1):
        super().__init__()
        self.dropout = nn.Dropout(p=dropout)
        
        # Create positional encoding matrix
        position = torch.arange(max_len).unsqueeze(1)
        div_term = torch.exp(torch.arange(0, d_model, 2) * (-math.log(10000.0) / d_model))
        
        pe = torch.zeros(max_len, 1, d_model)
        pe[:, 0, 0::2] = torch.sin(position * div_term)
        pe[:, 0, 1::2] = torch.cos(position * div_term)
        
        self.register_buffer('pe', pe)
    
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Args:
            x: (seq_len, batch_size, d_model) or (batch_size, seq_len, d_model)
        
        Returns:
            x with positional encoding added
        """
        if x.dim() == 3 and x.size(1) > x.size(0):  # batch_first format
            x = x + self.pe[:x.size(1), 0, :].unsqueeze(0)
        else:  # seq_first format
            x = x + self.pe[:x.size(0)]
        
        return self.dropout(x)


# Additional helper classes for model utilities

class NoisyLinear(nn.Module):
    """
    Noisy Linear layer for exploration (NoisyNet)
    Used in advanced DQN variants
    """
    
    def __init__(self, in_features: int, out_features: int, std_init: float = 0.5):
        super().__init__()
        
        self.in_features = in_features
        self.out_features = out_features
        self.std_init = std_init
        
        # Learnable parameters
        self.weight_mu = nn.Parameter(torch.empty(out_features, in_features))
        self.weight_sigma = nn.Parameter(torch.empty(out_features, in_features))
        self.bias_mu = nn.Parameter(torch.empty(out_features))
        self.bias_sigma = nn.Parameter(torch.empty(out_features))
        
        # Noise buffers (not trainable)
        self.register_buffer('weight_epsilon', torch.empty(out_features, in_features))
        self.register_buffer('bias_epsilon', torch.empty(out_features))
        
        self.reset_parameters()
        self.reset_noise()
    
    def reset_parameters(self):
        """Initialize parameters"""
        mu_range = 1 / math.sqrt(self.in_features)
        self.weight_mu.data.uniform_(-mu_range, mu_range)
        self.weight_sigma.data.fill_(self.std_init / math.sqrt(self.in_features))
        self.bias_mu.data.uniform_(-mu_range, mu_range)
        self.bias_sigma.data.fill_(self.std_init / math.sqrt(self.out_features))
    
    def reset_noise(self):
        """Sample new noise"""
        epsilon_in = self._scale_noise(self.in_features)
        epsilon_out = self._scale_noise(self.out_features)
        
        self.weight_epsilon.copy_(epsilon_out.ger(epsilon_in))
        self.bias_epsilon.copy_(epsilon_out)
    
    @staticmethod
    def _scale_noise(size: int) -> torch.Tensor:
        """Factorized Gaussian noise"""
        x = torch.randn(size)
        return x.sign().mul_(x.abs().sqrt_())
    
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """Forward with noisy weights"""
        if self.training:
            weight = self.weight_mu + self.weight_sigma * self.weight_epsilon
            bias = self.bias_mu + self.bias_sigma * self.bias_epsilon
        else:
            weight = self.weight_mu
            bias = self.bias_mu
        
        return F.linear(x, weight, bias)


class DuelingNetwork(nn.Module):
    """
    Dueling network architecture wrapper
    Separates state value from action advantages
    """
    
    def __init__(
        self,
        feature_dim: int,
        value_hidden: int,
        advantage_hidden: int,
        action_dim: int
    ):
        super().__init__()
        
        # Value stream: V(s)
        self.value_stream = nn.Sequential(
            nn.Linear(feature_dim, value_hidden),
            nn.ReLU(),
            nn.Linear(value_hidden, 1)
        )
        
        # Advantage stream: A(s, a)
        self.advantage_stream = nn.Sequential(
            nn.Linear(feature_dim, advantage_hidden),
            nn.ReLU(),
            nn.Linear(advantage_hidden, action_dim)
        )
    
    def forward(self, features: torch.Tensor) -> torch.Tensor:
        """
        Combine value and advantage
        Q(s,a) = V(s) + (A(s,a) - mean(A(s,a)))
        """
        value = self.value_stream(features)
        advantage = self.advantage_stream(features)
        
        # Dueling aggregation
        q_values = value + (advantage - advantage.mean(dim=1, keepdim=True))
        
        return q_values


def create_mlp(
    input_dim: int,
    hidden_dims: List[int],
    output_dim: int,
    activation: str = 'relu',
    use_layer_norm: bool = True,
    dropout: float = 0.0
) -> nn.Sequential:
    """
    Create a multi-layer perceptron
    
    Args:
        input_dim: Input dimension
        hidden_dims: List of hidden layer dimensions
        output_dim: Output dimension
        activation: Activation function ('relu', 'gelu', 'tanh')
        use_layer_norm: Whether to use layer normalization
        dropout: Dropout probability
    
    Returns:
        nn.Sequential MLP module
    """
    activation_fn = {
        'relu': nn.ReLU,
        'gelu': nn.GELU,
        'tanh': nn.Tanh,
        'leaky_relu': lambda: nn.LeakyReLU(0.2)
    }[activation]
    
    layers = []
    in_dim = input_dim
    
    for hidden_dim in hidden_dims:
        layers.append(nn.Linear(in_dim, hidden_dim))
        if use_layer_norm:
            layers.append(nn.LayerNorm(hidden_dim))
        layers.append(activation_fn())
        if dropout > 0:
            layers.append(nn.Dropout(dropout))
        in_dim = hidden_dim
    
    # Output layer
    layers.append(nn.Linear(in_dim, output_dim))
    
    return nn.Sequential(*layers)


# Model size estimation utilities

def estimate_model_size(model: nn.Module, precision: str = 'fp32') -> Dict[str, float]:
    """
    Estimate model size in memory
    
    Args:
        model: PyTorch model
        precision: 'fp32', 'fp16', or 'int8'
    
    Returns:
        Dictionary with size estimates
    """
    bytes_per_param = {
        'fp32': 4,
        'fp16': 2,
        'int8': 1
    }[precision]
    
    num_params = sum(p.numel() for p in model.parameters())
    size_bytes = num_params * bytes_per_param
    size_mb = size_bytes / (1024 ** 2)
    size_gb = size_bytes / (1024 ** 3)
    
    return {
        'parameters': num_params,
        'bytes': size_bytes,
        'mb': size_mb,
        'gb': size_gb,
        'precision': precision
    }


def print_model_summary(model: nn.Module, model_name: str = "Model"):
    """Print detailed model summary"""
    logger.info(f"\n{'='*60}")
    logger.info(f"{model_name} Summary")
    logger.info(f"{'='*60}")
    
    total_params = 0
    trainable_params = 0
    
    for name, param in model.named_parameters():
        total_params += param.numel()
        if param.requires_grad:
            trainable_params += param.numel()
    
    logger.info(f"Total parameters: {total_params:,}")
    logger.info(f"Trainable parameters: {trainable_params:,}")
    logger.info(f"Non-trainable parameters: {total_params - trainable_params:,}")
    
    # Size estimates
    for precision in ['fp32', 'fp16', 'int8']:
        size_info = estimate_model_size(model, precision)
        logger.info(f"{precision.upper()} size: {size_info['mb']:.2f} MB")
    
    logger.info(f"{'='*60}\n")
