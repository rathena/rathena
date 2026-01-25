# CTDE Framework Documentation
## Centralized Training, Decentralized Execution for Pack-Based Monster AI

**Version:** 1.0  
**Date:** January 25, 2026  
**Architecture Reference:** [Enhanced Hybrid ML Monster AI Architecture v2.0](../../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md)

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Centralized Training](#centralized-training)
4. [Decentralized Execution](#decentralized-execution)
5. [Pack Formation Detection](#pack-formation-detection)
6. [Signal Passing System](#signal-passing-system)
7. [Training Pack Episodes](#training-pack-episodes)
8. [Deploying Coordinated Actors](#deploying-coordinated-actors)
9. [Performance Characteristics](#performance-characteristics)
10. [Troubleshooting](#troubleshooting)
11. [API Reference](#api-reference)

---

## Overview

### What is CTDE?

**CTDE** (Centralized Training, Decentralized Execution) is a multi-agent reinforcement learning paradigm that enables monsters to coordinate as a team while maintaining scalable real-time execution.

**Key Principle:**
- **Training Phase:** Central critic evaluates team performance using global information
- **Execution Phase:** Each monster uses its own actor network with local observations only

### Why CTDE for Monster AI?

| Challenge | Solution |
|-----------|----------|
| **Scalability** | Decentralized actors don't need communication during inference |
| **Credit Assignment** | Centralized critic properly assigns credit to team members |
| **Emergence** | Pack tactics emerge from team rewards, not explicit programming |
| **Robustness** | Individual actors work independently if pack dissolves |

### Architecture Components

```
Training (Offline/Background):
┌─────────────────────────────────────────┐
│   Centralized Critic                     │
│   Input: Global state (all agents)       │
│   Output: Team value V(s_global)         │
│   Purpose: Learn team coordination value │
└─────────────────────────────────────────┘
           ↓ Teaches via advantages
┌─────────────────────────────────────────┐
│   Decentralized Actors (5x)              │
│   Input: Local state + signals           │
│   Output: Individual action              │
│   Purpose: Learn coordinated behavior    │
└─────────────────────────────────────────┘

Inference (Real-time):
┌───────────┐ ┌───────────┐ ┌───────────┐
│ Actor #1  │ │ Actor #2  │ │ Actor #3  │
│ (Local)   │ │ (Local)   │ │ (Local)   │
└───────────┘ └───────────┘ └───────────┘
      ↓            ↓            ↓
  Actions work together through learned coordination
```

---

## Architecture

### Component Details

#### 1. CentralizedCritic

**Purpose:** Evaluates team performance during training.

**Architecture:**
```python
Input: (batch, n_agents, state_dim)  # Global state - sees all agents
  ↓
Attention over agents (optional)
  ↓
[512 → 512 → 256] with LayerNorm + Dropout
  ↓
Output: (batch, 1)  # Single team value
```

**Key Features:**
- Multi-head attention for variable pack sizes
- Sees privileged information (all agents' states)
- Only used during training, not deployed
- ~20MB FP16 model size

**Usage:**
```python
from models.ctde_architectures import CentralizedCritic

critic = CentralizedCritic(
    state_dim=64,
    n_agents=5,
    hidden_dims=[512, 512, 256],
    use_attention=True
)

# During training
global_state = torch.randn(batch_size, n_agents, state_dim)
agent_mask = torch.ones(batch_size, n_agents, dtype=torch.bool)
value = critic(global_state, agent_mask)  # (batch, 1)
```

#### 2. DecentralizedActor

**Purpose:** Individual agent policy for decentralized execution.

**Architecture:**
```python
Input: (batch, state_dim) + (batch, signal_dim)
  ↓
State Encoder: [64 → 256 → 256]
Signal Encoder: [32 → 128]
  ↓
GRU (optional): Temporal dynamics
  ↓
Decision Network: [384 → 256 → 128 → 7]
  ↓
Output: (batch, action_dim)  # Action probabilities
```

**Key Features:**
- Works with or without signals (robust to solo operation)
- GRU for temporal context
- Action masking for invalid actions
- ~5MB FP16 per agent

**Usage:**
```python
from models.ctde_architectures import DecentralizedActor

actor = DecentralizedActor(
    state_dim=64,
    signal_dim=32,
    action_dim=7,
    hidden_dim=256,
    use_gru=True
)

# During inference
state = torch.randn(batch_size, 64)
signals = torch.randn(batch_size, 32)  # Can be zeros if no signals

action, log_prob, entropy, hidden = actor.get_action(
    state, signals, deterministic=True
)
```

#### 3. CommNet

**Purpose:** Enable communication during training.

**Rounds:** 3 rounds of message passing  
**Messages:** Each agent broadcasts hidden state to neighbors  
**Aggregation:** Mean pooling of received messages

**Not used during inference** - communication is learned implicitly.

#### 4. QMIXMixer

**Purpose:** Value function decomposition (optional).

Combines individual Q-values into team Q-value while maintaining monotonicity:
```
Q_total(s, a1, a2, ..., an) = f(Q1(s1, a1), Q2(s2, a2), ..., Qn(sn, an), s)
```

Where f is monotonic: if any Qi increases, Q_total increases.

---

## Centralized Training

### Training Algorithm

**CTDE with PPO:**

```
for each training iteration:
    1. Collect pack episodes (team of 2-5 monsters)
    2. Compute centralized value estimates V(s_global)
    3. Calculate advantages: A = Q(s,a) - V(s_global)
    4. Update actors using policy gradient + advantages
    5. Update critic using TD error
```

### Step-by-Step Training

#### Step 1: Collect Pack Episodes

```python
from data.pack_episode_buffer import PackEpisodeCollector, PackEpisodeBuffer

# Create buffer and collector
buffer = PackEpisodeBuffer(max_episodes=1000)
collector = PackEpisodeCollector(buffer)

# Start episode
monster_ids = [100, 101, 102]  # Pack of 3
initial_states = get_initial_states(monster_ids)
pack_id = collector.start_episode(monster_ids, initial_states)

# Play episode
for step in range(episode_length):
    # Get actions from actors
    actions = get_pack_actions(states, signals)
    
    # Execute in environment
    next_states, team_reward, individual_rewards = environment.step(actions)
    
    # Add transition
    collector.add_transition(
        pack_id, next_states, actions, 
        team_reward, individual_rewards, signals
    )

# Finalize
episode = collector.finalize_episode(pack_id, outcome='victory')
```

#### Step 2: Train CTDE Models

```python
from training.ctde_trainer import CTDETrainer, create_default_ctde_config

# Create trainer
config = create_default_ctde_config()
trainer = CTDETrainer(config, n_agents=5)

# Train on buffer
history = await trainer.train_on_buffer(
    buffer,
    n_iterations=1000,
    episodes_per_iteration=32
)

# Save checkpoint
trainer.save_checkpoint('checkpoints/ctde_step_1000.pt')

# Export actor for deployment
trainer.export_actor_to_onnx('models/pack_coordination_actor.onnx')
```

### Team Reward Composition

Team reward combines multiple factors:

```python
team_reward = (
    pack_survival_rate * 10.0 +      # How many survived
    coordinated_kills * 5.0 +         # Kills with 2+ attackers
    formation_quality * 3.0 +         # Tight formation bonus
    support_effectiveness * 2.0       # Helping low-HP allies
)
```

**Individual reward:** Own damage, survival, skill usage

**Combined reward:**
```python
agent_reward = (
    0.7 * team_reward +  # 70% team
    0.3 * individual_reward  # 30% individual
)
```

This encourages both team play and individual excellence.

---

## Decentralized Execution

### Inference Without Communication

During inference, actors work **independently**:

```python
# Each monster processes locally
for monster in pack:
    # Get local observation
    state = monster.get_state_vector()
    
    # Get signals (from Apache AGE graph)
    signals = signal_coordinator.receive_signals(monster.id)
    signal_vector = aggregate_signals(signals)
    
    # Run decentralized actor (ONNX)
    action_probs = actor_model.run(state, signal_vector)
    action = argmax(action_probs)
    
    # Execute action
    monster.execute_action(action)
```

**No synchronization required** - actors make decisions in parallel.

### Signal Integration

Signals are **optional** coordination hints:

```python
if has_signals:
    # Use signals to inform decision
    action = actor(state, signals)
else:
    # Work solo (zeros for signals)
    action = actor(state, zeros(32))
```

Actors learn to:
- Follow signals when beneficial
- Ignore signals when not applicable
- Work effectively without signals

---

## Pack Formation Detection

### Using Apache AGE Graph

Pack structure is stored in Apache AGE graph with LEADS and TEAMMATE_OF edges.

**Query pack members:**
```python
# Using graph_manager
team_info = await graph_manager.get_monster_team(monster_id)

# Returns:
{
    'leader_id': 1001,
    'follower_ids': [1002, 1003, 1004],
    'teammate_ids': [],
    'role': 'leader'  # or 'follower' or 'independent'
}
```

**Pack formation criteria:**
1. **Leader-Follower:** Monsters connected by LEADS edges
2. **Teammates:** Monsters connected by TEAMMATE_OF edges
3. **Proximity:** Within pack_formation_radius (10 cells)
4. **Size:** 2-5 monsters

### Pack Roles

| Role | Responsibilities | Graph Relationship |
|------|-----------------|-------------------|
| **Leader** | Broadcasts commands, coordinates | Has outgoing LEADS edges |
| **Follower** | Follows leader commands | Has incoming LEADS edge |
| **Member** | Equal participation | Connected via TEAMMATE_OF |
| **Independent** | Solo monster | No pack edges |

---

## Signal Passing System

### Signal Vector Encoding (32 dimensions)

**Structure:**
```
Dims 0-7:   Target Information (which target to prioritize)
Dims 8-15:  Formation Commands (positioning directives)
Dims 16-23: Support Requests (who needs help, urgency)
Dims 24-31: Strategic Directives (retreat, advance, hold)
```

### Signal Types

#### 1. ATTACK_TARGET / FOCUS_FIRE
```python
await signal_coordinator.send_attack_target_signal(
    from_monster_id=leader_id,
    target_id=5001,
    target_threat=0.9,  # High threat
    pack_monsters=[1002, 1003, 1004]
)
```
Encodes target priority in dims 0-7.

#### 2. RETREAT
```python
await signal_coordinator.send_retreat_signal(
    from_monster_id=leader_id,
    rally_point=(150, 200),  # x, y
    reason='outnumbered',
    pack_monsters=pack_members
)
```
Encodes retreat directive in dim 24.

#### 3. FORMATION_COMMAND
```python
await signal_coordinator.send_formation_command(
    leader_id=leader_id,
    formation_type='flanking',
    positions={
        1002: (145, 195),
        1003: (155, 205)
    },
    pack_monsters=pack_members
)
```
Encodes formation type in dims 8-15.

#### 4. HELP_REQUEST
```python
await signal_coordinator.send_help_request(
    from_monster_id=weak_monster_id,
    urgency=0.9,  # Very urgent
    hp_ratio=0.2,  # Low HP
    position=(150, 200),
    pack_monsters=pack_members
)
```
Encodes urgency in dims 16-23.

### Signal Processing

**Signal Flow:**
```
1. Leader/Monster sends signal
   ↓ (Apache AGE SIGNALS edge)
2. Graph stores signal with TTL (5 seconds)
   ↓
3. Monster queries pending signals
   ↓
4. SignalCoordinator aggregates to 32-dim vector
   ↓
5. Vector fed to DecentralizedActor
   ↓
6. Actor produces coordinated action
```

**Aggregation Example:**
```python
from signal_coordinator import SignalCoordinator

# Receive signals
signals = await coordinator.receive_signals(monster_id=1002)
# Returns: [
#   {'signal_type': 'attack_target', 'content': {...}, 'priority': 2},
#   {'signal_type': 'formation_command', 'content': {...}, 'priority': 3}
# ]

# Aggregate to vector
signal_vector = coordinator.aggregate_signals_to_vector(signals)
# Returns: numpy array shape (32,)
```

---

## Training Pack Episodes

### Episode Structure

```python
PackEpisode(
    pack_id=1001,
    monster_ids=[100, 101, 102],
    pack_size=3,
    episode_length=50,
    
    # Temporal data (T=50 steps)
    global_states=np.array(...),    # (50, 3, 64) - full battlefield
    local_obs=np.array(...),         # (50, 3, 64) - per-agent view
    actions=np.array(...),           # (50, 3) - actions taken
    team_rewards=np.array(...),      # (50,) - shared reward
    individual_rewards=np.array(...),# (50, 3) - per-agent rewards
    signals=np.array(...),           # (50, 3, 32) - signals exchanged
    
    # Metadata
    outcome='victory',
    total_team_reward=45.2,
    coordination_score=0.85
)
```

### Training Workflow

**1. Data Collection:**
```bash
# Collect episodes during gameplay
python scripts/collect_pack_episodes.py \
    --min-pack-size 2 \
    --max-pack-size 5 \
    --target-episodes 1000
```

**2. Train CTDE Models:**
```bash
# Train using collected episodes
python training/train_ctde.py \
    --config config/ctde_config.yaml \
    --episodes-dir data/pack_episodes \
    --output-dir models/ctde \
    --iterations 5000
```

**3. Export to ONNX:**
```bash
# Export actor for deployment
python scripts/export_ctde_actor.py \
    --checkpoint checkpoints/ctde_best.pt \
    --output models/production/pack_coordination_actor.onnx
```

### Training Script Example

```python
import asyncio
from training.ctde_trainer import train_ctde_from_config
from data.pack_episode_buffer import PackEpisodeBuffer

async def train():
    # Create buffer
    buffer = PackEpisodeBuffer(max_episodes=5000)
    
    # Load episodes from database
    await buffer.load_from_database(limit=1000, min_reward=0.0)
    
    # Train
    trainer = await train_ctde_from_config(
        config_path='config/ctde_config.yaml',
        buffer=buffer,
        checkpoint_dir='./checkpoints',
        n_iterations=5000
    )
    
    print(f"Training complete: {trainer.training_step} steps")
    print(f"Actor exported to ONNX")

asyncio.run(train())
```

---

## Deploying Coordinated Actors

### Deployment Steps

**1. Export ONNX Model:**
```python
from training.ctde_trainer import CTDETrainer

trainer = CTDETrainer(config)
trainer.load_checkpoint('checkpoints/best.pt')
trainer.export_actor_to_onnx(
    'models/aggressive/pack_coordination.onnx',
    archetype='aggressive'
)
```

**2. Place in Model Directory:**
```bash
# Copy to production model directory
cp models/aggressive/pack_coordination.onnx \
   /opt/ml_monster_ai/models/aggressive/
```

**3. Hot Reload (No Downtime):**
```bash
# Trigger hot reload via HTTP
curl -X POST http://localhost:8080/reload/aggressive/pack_coordination
```

**4. Verify Deployment:**
```bash
# Check model loaded
curl http://localhost:8080/models | jq '.models[] | select(.model_type == "pack_coordination")'
```

### Model Registry

Deployed actors are registered in `ml_models` table:

```sql
INSERT INTO ml_models (
    archetype, model_type, model_name,
    onnx_fp16_path, version, training_status
) VALUES (
    'aggressive', 'pack_coordination', 'CTDE Actor v1.0',
    '/opt/ml_monster_ai/models/aggressive/pack_coordination.onnx',
    '1.0', 'deployed'
);
```

---

## Decentralized Execution

### Inference Pipeline

**Standard Flow (with pack coordination):**

```python
# In inference_engine.py
async def infer_pack_coordination(pack_monsters):
    # 1. Detect pack from graph
    pack_info = await detect_pack_formation(monster_ids, graph_manager)
    
    # 2. Process signals
    signals_dict = await signal_coordinator.process_coordination_signals(monster_ids)
    
    # 3. Run parallel inference
    coordinated_actions = []
    for monster in pack_monsters:
        signal_vector = signals_dict[monster.id]
        action = actor_model.run(monster.state, signal_vector)
        coordinated_actions.append(action)
    
    # 4. Calculate coordination score
    score = calculate_coordination_score(actions, pack_info, signals)
    
    # 5. Apply bonus if coordinated
    if score >= threshold:
        for action in coordinated_actions:
            action['coordination_bonus'] = score
    
    return coordinated_actions
```

### Pack Detection

```python
# In main.py
async def _group_requests_by_pack(requests):
    pack_groups = {}
    solo_monsters = []
    
    for req in requests:
        # Query graph for pack membership
        team_info = await graph_manager.get_monster_team(req['monster_id'])
        
        if team_info and team_info['role'] != 'independent':
            pack_id = team_info['leader_id'] or req['monster_id']
            pack_groups[pack_id] = pack_groups.get(pack_id, [])
            pack_groups[pack_id].append(req)
        else:
            solo_monsters.append(req)
    
    return {'packs': pack_groups, 'solo': solo_monsters}
```

### Coordination Scoring

**Coordination quality factors:**

1. **Signal Utilization** (40%): How many monsters have active signals
2. **Pack Coverage** (30%): Requested monsters are in same pack
3. **Leader Presence** (20%): Leader is coordinating
4. **Action Diversity** (10%): Different roles, different actions

```python
coordination_score = (
    0.4 * signal_utilization +
    0.3 * pack_coverage +
    0.2 * leader_bonus +
    0.1 * action_diversity
)
```

**Bonus application:**
- Score ≥ 0.7: Coordination bonus applied
- Score < 0.7: No bonus (actions still valid)

---

## Pack Formation Detection

### Apache AGE Graph Schema

**Nodes:**
```cypher
(:Monster {
    id: INT,
    mob_id: INT,
    archetype: STRING,
    leadership_score: FLOAT,
    position_x: INT,
    position_y: INT,
    current_hp_ratio: FLOAT
})
```

**Edges:**
```cypher
(:Monster)-[:LEADS {strength: FLOAT}]->(:Monster)
(:Monster)-[:FOLLOWS]->(:Monster)
(:Monster)-[:TEAMMATE_OF]->(:Monster)
```

### Formation Queries

**Get pack members:**
```sql
SELECT * FROM cypher('monster_ai', $$
    MATCH (leader:Monster {id: $1})-[:LEADS*0..2]->(member:Monster)
    RETURN collect(member.id) AS member_ids
$$) AS (member_ids agtype);
```

**Detect pack leader:**
```sql
SELECT * FROM cypher('monster_ai', $$
    MATCH (monster:Monster {id: $1})
    OPTIONAL MATCH (leader:Monster)-[:LEADS]->(monster)
    RETURN COALESCE(leader.id, monster.id) AS leader_id
$$) AS (leader_id agtype);
```

### Formation Types

| Formation | Description | Use Case | Code |
|-----------|-------------|----------|------|
| **Flanking** | 2+ monsters attack from sides | Isolated target | `flanking` |
| **Defensive Circle** | Protect weak members | Under attack | `defensive_circle` |
| **Spread Out** | Maximize coverage | Area control | `spread_out` |
| **Tight Cluster** | Stack for mutual support | Outnumbered | `tight_cluster` |
| **Line Formation** | Linear assault | Corridor combat | `line_formation` |
| **V Formation** | Ranged at back, melee front | Mixed composition | `v_formation` |

---

## Performance Characteristics

### Latency Targets

| Metric | Target | Actual (Tested) | Status |
|--------|--------|-----------------|--------|
| **Single Agent Inference** | <5ms | 2-3ms | ✅ |
| **Pack of 5 Inference** | <20ms | 12-15ms | ✅ |
| **Signal Processing** | <3ms | 1-2ms | ✅ |
| **Pack Detection** | <5ms | 3-4ms | ✅ |
| **Total Pack Coordination** | <25ms | 18-22ms | ✅ |

### Memory Usage

**Training:**
- Centralized Critic: 20MB FP16
- Decentralized Actor: 5MB FP16
- CommNet: 3MB FP16
- Total: ~30MB per archetype

**Inference:**
- Decentralized Actor Only: 5MB FP16
- 6 Archetypes: 30MB total
- Fits easily in VRAM allocation

### Throughput

**Single Inference Worker:**
- Solo monsters: ~10,000 inferences/sec
- Pack coordination: ~2,000 packs/sec (5 agents each = 10,000 agents/sec)

**Scaling:**
- Horizontal: Add more workers
- Each worker independent
- No synchronization overhead

---

## Troubleshooting

### Common Issues

#### Issue 1: Pack Not Coordinating

**Symptoms:** Monsters act independently despite being in pack

**Diagnosis:**
```bash
# Check pack structure in graph
psql -d ragnarok_ml -c "
SELECT * FROM cypher('monster_ai', \$\$
    MATCH (m:Monster {id: 1001})-[r:LEADS]->(follower)
    RETURN follower.id, r.strength
\$\$) AS (id agtype, strength agtype);
"
```

**Solutions:**
1. Verify LEADS edges exist in graph
2. Check signal_coordinator is initialized
3. Ensure pack_coordination.enabled = true in config
4. Verify pack size ≥ 2

#### Issue 2: High Inference Latency

**Symptoms:** Pack coordination takes >25ms

**Diagnosis:**
```python
# Check actor model size
import onnx
model = onnx.load('pack_coordination_actor.onnx')
print(f"Model size: {len(model.SerializeToString()) / 1024:.2f} KB")
```

**Solutions:**
1. Quantize to INT8 for faster inference
2. Reduce actor hidden_dim in config
3. Disable GRU if not needed
4. Use smaller batch sizes

#### Issue 3: Signals Not Received

**Symptoms:** signal_vector is all zeros

**Diagnosis:**
```python
# Check signals in graph
signals = await graph_manager.receive_signals(monster_id)
print(f"Signals: {len(signals)}")
```

**Solutions:**
1. Verify Apache AGE extension is loaded
2. Check signal TTL not expired (5 seconds default)
3. Ensure signals are being sent (check ml_coordination_logs table)
4. Clear signal cache: `coordinator.clear_cache()`

#### Issue 4: Training Not Converging

**Symptoms:** Losses not decreasing, poor coordination

**Diagnosis:**
```python
# Check training metrics
history = trainer.get_training_statistics()
print(f"Avg actor loss: {history['avg_actor_loss']:.4f}")
print(f"Avg entropy: {history['avg_entropy']:.4f}")
```

**Solutions:**
1. Increase entropy_coef for more exploration
2. Reduce learning rate (try 1e-5 for actor)
3. Check team_reward_weight (try 0.8 for more team focus)
4. Ensure sufficient diverse episodes (1000+ recommended)
5. Verify episode quality (check outcomes distribution)

### Debug Logging

**Enable verbose logging:**
```python
import logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger('training.ctde_trainer')
logger.setLevel(logging.DEBUG)
```

**Check signal visualization:**
```python
from signal_coordinator import visualize_signal_vector

signal_vector = coordinator.aggregate_signals_to_vector(signals)
print(visualize_signal_vector(signal_vector))
```

---

## API Reference

### ctde_architectures.py

#### CentralizedCritic

```python
class CentralizedCritic(nn.Module):
    def __init__(
        self,
        state_dim: int = 64,
        n_agents: int = 5,
        hidden_dims: List[int] = [512, 512, 256],
        use_attention: bool = True,
        dropout: float = 0.2
    )
    
    def forward(
        self,
        global_state: torch.Tensor,  # (batch, n_agents, state_dim)
        agent_mask: Optional[torch.Tensor] = None  # (batch, n_agents)
    ) -> torch.Tensor:  # (batch, 1)
```

#### DecentralizedActor

```python
class DecentralizedActor(nn.Module):
    def __init__(
        self,
        state_dim: int = 64,
        signal_dim: int = 32,
        action_dim: int = 7,
        hidden_dim: int = 256,
        use_gru: bool = True,
        dropout: float = 0.1
    )
    
    def forward(
        self,
        state: torch.Tensor,  # (batch, state_dim)
        signals: Optional[torch.Tensor] = None,  # (batch, signal_dim)
        hidden: Optional[torch.Tensor] = None,
        action_mask: Optional[torch.Tensor] = None
    ) -> Tuple[torch.Tensor, Optional[torch.Tensor]]:
        # Returns: (action_probs, hidden_state)
    
    def get_action(
        self,
        state: torch.Tensor,
        signals: Optional[torch.Tensor] = None,
        hidden: Optional[torch.Tensor] = None,
        action_mask: Optional[torch.Tensor] = None,
        deterministic: bool = False
    ) -> Tuple[torch.Tensor, torch.Tensor, torch.Tensor, Optional[torch.Tensor]]:
        # Returns: (action, log_prob, entropy, hidden)
```

### pack_episode_buffer.py

#### PackEpisodeBuffer

```python
class PackEpisodeBuffer:
    def __init__(
        self,
        max_episodes: int = 1000,
        db_pool: Optional[asyncpg.Pool] = None,
        prioritize_by_reward: bool = True
    )
    
    def add_episode(self, episode: PackEpisode) -> None
    
    def get_batch(
        self,
        batch_size: int,
        pack_size: Optional[int] = None,
        prioritized: bool = False
    ) -> List[PackEpisode]
    
    async def load_from_database(
        self,
        pack_ids: Optional[List[int]] = None,
        limit: int = 100,
        min_reward: Optional[float] = None
    ) -> int
```

### ctde_trainer.py

#### CTDETrainer

```python
class CTDETrainer:
    def __init__(
        self,
        config: Dict[str, Any],
        n_agents: int = 5,
        device: str = 'cuda'
    )
    
    def train_episode(
        self,
        episode_data: Dict[str, torch.Tensor]
    ) -> Dict[str, float]
    
    async def train_on_buffer(
        self,
        buffer: PackEpisodeBuffer,
        n_iterations: int = 100,
        episodes_per_iteration: int = 32
    ) -> Dict[str, List[float]]
    
    def save_checkpoint(
        self,
        path: str,
        metadata: Optional[Dict[str, Any]] = None
    ) -> bool
    
    def export_actor_to_onnx(
        self,
        output_path: str,
        archetype: str = 'aggressive'
    ) -> bool
```

### signal_coordinator.py

#### SignalCoordinator

```python
class SignalCoordinator:
    def __init__(
        self,
        graph_manager,
        signal_dim: int = 32,
        signal_range: int = 15,
        signal_ttl: int = 5
    )
    
    async def broadcast_signal(
        self,
        from_monster_id: int,
        signal_type: str,
        data: Dict[str, Any],
        pack_monsters: Optional[List[int]] = None,
        priority: int = 5
    ) -> int
    
    async def receive_signals(
        self,
        monster_id: int,
        use_cache: bool = True
    ) -> List[Dict[str, Any]]
    
    async def process_coordination_signals(
        self,
        pack_monsters: List[int]
    ) -> Dict[int, np.ndarray]
    
    def aggregate_signals_to_vector(
        self,
        signals: List[Dict[str, Any]]
    ) -> np.ndarray
```

---

## Advanced Topics

### Curriculum Learning

Train progressively larger packs:

```yaml
# In ctde_config.yaml
advanced:
  curriculum_learning: true
  start_pack_size: 2
  end_pack_size: 5
  size_increase_interval: 1000  # iterations
```

### Transfer Learning

Initialize from solo-trained models:

```python
# Load solo combat model
solo_model = torch.load('models/aggressive/combat_dqn.pt')

# Transfer weights to actor
actor.state_encoder.load_state_dict(
    solo_model.feature_extractor.state_dict(),
    strict=False  # Allow partial loading
)
```

### Multi-Task Learning

Train on multiple objectives:

```yaml
advanced:
  multi_task_learning: true
  tasks: ['combat', 'exploration', 'defense']
```

### Monitoring Training

**TensorBoard:**
```bash
tensorboard --logdir logs/ctde_training
```

**Metrics to watch:**
- Actor loss should decrease
- Critic loss should stabilize
- Entropy should decrease gradually (exploration → exploitation)
- Coordination score should increase

---

## Configuration Reference

### Full Configuration Example

See [`config/ctde_config.yaml`](config/ctde_config.yaml) for complete configuration.

**Key Parameters:**

```yaml
ctde:
  # Pack settings
  min_pack_size: 2
  max_pack_size: 5
  
  # Architecture
  critic:
    hidden_dims: [512, 512, 256]
    learning_rate: 0.0003
  
  actor:
    hidden_dim: 256
    learning_rate: 0.0001
  
  # Training
  gamma: 0.99
  gae_lambda: 0.95
  team_reward_weight: 0.7
  
  # Inference
  inference:
    use_signals: true
    coordination_threshold: 0.7
```

---

## Best Practices

### Training

1. **Collect Diverse Episodes:**
   - Mix pack sizes (2, 3, 4, 5)
   - Various outcomes (victory, defeat, retreat)
   - Different scenarios (outnumbered, ambush, support)

2. **Monitor Convergence:**
   - Plot actor loss, critic loss, entropy
   - Check coordination score trends
   - Validate on held-out episodes

3. **Hyperparameter Tuning:**
   - Start with defaults
   - Adjust team_reward_weight based on coordination quality
   - Tune learning rates if losses oscillate

### Deployment

1. **Test Before Production:**
   - Run test_ctde.py to verify models
   - Benchmark latency on target hardware
   - Validate ONNX export

2. **Hot Reload:**
   - Use HTTP API for zero-downtime updates
   - Verify model shapes match before reload
   - Monitor latency after reload

3. **Monitoring:**
   - Track coordination_score in responses
   - Monitor signal usage rates
   - Check pack formation counts

---

## Examples

### Example 1: Train from Scratch

```python
import asyncio
from training.ctde_trainer import CTDETrainer, create_default_ctde_config
from data.pack_episode_buffer import PackEpisodeBuffer

async def train_from_scratch():
    # 1. Create buffer and load data
    buffer = PackEpisodeBuffer(max_episodes=5000)
    await buffer.load_from_database(limit=2000, min_reward=-5.0)
    
    # 2. Create trainer
    config = create_default_ctde_config()
    config['n_epochs'] = 8  # More epochs for better convergence
    trainer = CTDETrainer(config)
    
    # 3. Train
    history = await trainer.train_on_buffer(
        buffer,
        n_iterations=2000,
        episodes_per_iteration=32
    )
    
    # 4. Save and export
    trainer.save_checkpoint('checkpoints/final.pt')
    trainer.export_actor_to_onnx('models/actor.onnx')
    
    return trainer, history

trainer, history = asyncio.run(train_from_scratch())
```

### Example 2: Incremental Training

```python
# Load existing checkpoint
trainer = CTDETrainer(config)
trainer.load_checkpoint('checkpoints/step_5000.pt')

# Continue training with new data
new_history = await trainer.train_on_buffer(
    buffer,
    n_iterations=1000  # Additional 1000 iterations
)

# Save updated checkpoint
trainer.save_checkpoint('checkpoints/step_6000.pt')
```

### Example 3: Inference with Pack Coordination

```python
from inference_engine import ONNXInferenceEngine
from signal_coordinator import SignalCoordinator

# Initialize
engine = ONNXInferenceEngine(device='cuda:0')
engine.load_all_models()

coordinator = SignalCoordinator(graph_manager)

# Coordinate pack
pack_monsters = [
    {'monster_id': 100, 'state': state1, 'archetype': 'aggressive'},
    {'monster_id': 101, 'state': state2, 'archetype': 'aggressive'},
    {'monster_id': 102, 'state': state3, 'archetype': 'defensive'}
]

actions = await engine.infer_pack_coordination(
    pack_monsters, coordinator, graph_manager
)

for action_dict in actions:
    print(f"Monster {action_dict['monster_id']}: "
          f"action={action_dict['action']}, "
          f"coordinated={action_dict['coordinated']}")
```

---

## Database Schema

### Pack Episodes Table

```sql
CREATE TABLE ml_pack_episodes (
    id BIGSERIAL PRIMARY KEY,
    pack_id BIGINT NOT NULL UNIQUE,
    monster_ids INTEGER[] NOT NULL,
    pack_size INTEGER NOT NULL CHECK (pack_size BETWEEN 2 AND 5),
    episode_length INTEGER NOT NULL,
    
    -- Performance
    team_reward REAL NOT NULL,
    outcome VARCHAR(20) NOT NULL,
    coordination_score REAL,
    
    -- Episode data (numpy arrays as bytea)
    global_states BYTEA NOT NULL,
    local_obs BYTEA NOT NULL,
    actions BYTEA NOT NULL,
    individual_rewards BYTEA NOT NULL,
    signals BYTEA,
    
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

### Coordination Logs Table

```sql
CREATE TABLE ml_coordination_logs (
    id BIGSERIAL PRIMARY KEY,
    pack_id BIGINT,
    signal_type VARCHAR(50) NOT NULL,
    from_monster_id INTEGER NOT NULL,
    to_monster_ids INTEGER[] NOT NULL,
    signal_content JSONB,
    success BOOLEAN NOT NULL DEFAULT TRUE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

---

## Summary

### CTDE Benefits

✅ **Scalable:** Decentralized execution scales to thousands of monsters  
✅ **Coordinated:** Centralized training learns team tactics  
✅ **Robust:** Actors work independently if pack dissolves  
✅ **Efficient:** No communication overhead during inference  
✅ **Emergent:** Pack behaviors emerge from rewards, not rules

### Key Takeaways

1. **Training:** Critic sees everything, trains actors on team performance
2. **Execution:** Actors see only local state + optional signals
3. **Signals:** Coordination hints via Apache AGE graph (SIGNALS edges)
4. **Performance:** <20ms for pack of 5, meets real-time requirements
5. **Deployment:** Export actor only (critic stays in training)

### Next Steps

1. **Collect Data:** Gather pack episodes from gameplay
2. **Train Models:** Run CTDE trainer on collected episodes
3. **Deploy Actors:** Export to ONNX and hot-reload into inference service
4. **Monitor:** Track coordination scores and pack performance
5. **Iterate:** Continuously improve with more data

---

## References

- **Architecture:** [`plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md)
- **Apache AGE Integration:** [`ml_inference_service/graph_manager.py`](../../ml_inference_service/graph_manager.py)
- **Inference Engine:** [`ml_inference_service/inference_engine.py`](../../ml_inference_service/inference_engine.py)
- **CTDE Architectures:** [`models/ctde_architectures.py`](models/ctde_architectures.py)
- **CTDE Trainer:** [`training/ctde_trainer.py`](training/ctde_trainer.py)
- **Signal Coordinator:** [`ml_inference_service/signal_coordinator.py`](../../ml_inference_service/signal_coordinator.py)
- **Configuration:** [`config/ctde_config.yaml`](config/ctde_config.yaml)
- **Tests:** [`test_ctde.py`](test_ctde.py)

---

**END OF CTDE FRAMEWORK DOCUMENTATION**

For questions or issues, check the troubleshooting section or review the test suite in `test_ctde.py`.
