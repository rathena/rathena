# ML Monster AI - Training Guide
## Enhanced ML Monster AI System v2.0

This guide covers training all 54 ML models for the Enhanced ML Monster AI system.

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Quick Start](#quick-start)
4. [Training Pipeline](#training-pipeline)
5. [Model Types](#model-types)
6. [Archetype Configuration](#archetype-configuration)
7. [Monitoring Training](#monitoring-training)
8. [Troubleshooting](#troubleshooting)

---

## Overview

The ML Monster AI system consists of **54 models**:
- **6 archetypes**: aggressive, defensive, support, mage, tank, ranged
- **9 model types per archetype**:
  1. Combat DQN (10-12 layers, 512-1024 units)
  2. Movement PPO (12 layers, actor-critic)
  3. Skill DQN (embeddings + MLP)
  4. Threat Assessment (ensemble)
  5. Team Coordination LSTM (3 layers, 256 units)
  6. Spatial Vision Transformer (ViT-tiny adapted)
  7. Temporal Transformer-XL (4 layers)
  8. Pattern Recognition Transformer (6 layers)
  9. Soft Actor-Critic (continuous control)

**Total**: 54 models, ~5.4 GB FP16 (6 GB with buffers)

---

## Prerequisites

### Hardware Requirements

- **GPU**: NVIDIA RTX 3060 (12GB VRAM) or better
- **RAM**: 32GB minimum
- **Storage**: 100GB+ SSD
- **CPU**: 8+ cores

### Software Requirements

- **OS**: Ubuntu 20.04+ or Rocky Linux 8+
- **Python**: 3.11+
- **CUDA**: 12.1+
- **PostgreSQL**: 17.7 with TimescaleDB, Apache AGE, pgvector

### Python Environment

Already set up at `/opt/ml_monster_ai/venv` with:
- PyTorch 2.5.1
- CUDA 12.1
- ONNX Runtime
- asyncpg

---

## Quick Start

### 1. Collect Initial Data

```bash
# Activate environment
source /opt/ml_monster_ai/venv/bin/activate

# Collect data for all archetypes
python scripts/collect_initial_data.py --all --samples-per-archetype 10000
```

### 2. Train All Models

```bash
# Train all 54 models (sequential, recommended)
python scripts/train_all_models.py --all --epochs 100

# Train specific archetype
python scripts/train_all_models.py --archetypes aggressive --epochs 50

# Train specific models
python scripts/train_all_models.py \
    --archetypes aggressive defensive \
    --model-types combat_dqn movement_ppo \
    --epochs 100
```

### 3. Export to ONNX

```bash
# Export all models to ONNX FP16
python scripts/export_to_onnx.py --all --verify

# Export with optimization
python scripts/export_to_onnx.py --all --optimize --verify
```

---

## Training Pipeline

### Phase 1: Data Collection

Initial data provides bootstrap samples for supervised pre-training:

```bash
# Collect 10K samples per archetype
python scripts/collect_initial_data.py --all --samples-per-archetype 10000
```

Data sources:
1. **PostgreSQL**: Existing `ml_experience_replay` table
2. **Synthetic**: Generated based on archetype characteristics
3. **Traditional AI**: Recordings from existing monster behavior

### Phase 2: Model Training

Train models using collected data:

```bash
# Full training (all 54 models)
python scripts/train_all_models.py --all --epochs 100 --batch-size 256 --lr 0.0003

# Custom training
python scripts/train_all_models.py \
    --archetypes aggressive mage ranged \
    --model-types combat_dqn movement_ppo spatial_vit \
    --epochs 50 \
    --batch-size 128 \
    --lr 0.0001 \
    --device cuda:0
```

Training features:
- **Automatic Mixed Precision (AMP)**: FP16 training for 2x speedup
- **Gradient Checkpointing**: Saves VRAM for large models
- **Prioritized Experience Replay**: Learns from important experiences first
- **Early Stopping**: Stops when validation plateaus
- **Checkpointing**: Saves progress every N steps

### Phase 3: Validation

Validation runs automatically during training. Manual validation:

```bash
python scripts/evaluate_models.py \
    --archetype aggressive \
    --model-type combat_dqn \
    --num-episodes 100
```

### Phase 4: ONNX Export

Export for production inference:

```bash
# Export all models (FP16, optimized)
python scripts/export_to_onnx.py --all --verify --optimize

# Export specific models (FP32)
python scripts/export_to_onnx.py \
    --archetypes aggressive \
    --model-types combat_dqn \
    --fp32
```

---

## Model Types

### 1. Combat DQN

**Purpose**: Tactical combat decisions

**Training Algorithm**: Double DQN with Prioritized Experience Replay

**Input**: 64-dim state vector  
**Output**: 10 Q-values (one per action)

**Hyperparameters**:
- Hidden dims: [1024, 1024, 512, 512, 256, 256, 128, 128, 64, 64]
- Dropout: 0.2
- Learning rate: 0.0001
- Gamma: 0.99
- Batch size: 256

**Training Time**: ~2-3 hours per archetype

### 2. Movement PPO

**Purpose**: Spatial positioning and movement

**Training Algorithm**: Proximal Policy Optimization

**Input**: 64-dim state vector  
**Output**: Action probabilities (9 directions) + state value

**Hyperparameters**:
- Shared layers: 6 (1024 → 256 units)
- Actor/Critic layers: 3 each
- Learning rate: 0.00003
- Clip epsilon: 0.2
- GAE lambda: 0.97

**Training Time**: ~3-4 hours per archetype

### 3. Soft Actor-Critic

**Purpose**: Continuous control

**Training Algorithm**: SAC with entropy maximization

**Components**: Actor + Twin Critics

**Hyperparameters**:
- Hidden dim: 256
- Num layers: 3
- Actor LR: 0.0003
- Critic LR: 0.0003
- Auto-tune temperature: Yes

**Training Time**: ~2-3 hours per archetype

### 4. Vision Transformer

**Purpose**: Spatial understanding

**Training Algorithm**: Supervised learning on spatial tasks

**Architecture**: ViT-tiny adapted

**Hyperparameters**:
- Patch size: 8
- Embed dim: 192
- Depth: 12 layers
- Num heads: 3
- Learning rate: 0.0003

**Training Time**: ~4-5 hours per archetype

### 5. Pattern Recognition Transformer

**Purpose**: Detect numerical patterns in combat

**Training Algorithm**: Supervised with contrastive learning

**Architecture**: Large transformer (6 layers, 512-dim)

**Hyperparameters**:
- d_model: 512
- Num heads: 16
- Num layers: 6
- Learning rate: 0.0001
- Dropout: 0.1

**Training Time**: ~5-7 hours per archetype

---

## Archetype Configuration

### Aggressive

**Characteristics**: High damage, risk-taking, melee-focused

**Reward Weights**:
- Damage dealt: ×2.0
- Kills: +100
- Damage taken: -0.5
- Deaths: -50

**Training Parameters**:
- Learning rate: 0.0004 (higher for faster adaptation)
- Exploration noise: 0.1
- Reward scale: 1.5

**Training Time**: ~15-20 hours for all 9 models

### Defensive

**Characteristics**: High DEF, survival-focused, damage mitigation

**Reward Weights**:
- Survival time: ×1.0
- Damage blocked: ×1.5
- Allies protected: ×20
- Survival bonus: +100

**Training Parameters**:
- Learning rate: 0.0002 (more conservative)
- Exploration noise: 0.05
- Reward scale: 1.2

### Support

**Characteristics**: Healing/buffs, team-oriented

**Reward Weights**:
- Healing done: ×2.0
- Effective healing: ×4.0
- Buffs applied: ×15
- Allies died: -50

**Training Parameters**:
- Team reward weight: 0.7 (high team focus)
- Individual weight: 0.3

### Mage

**Characteristics**: High MATK, ranged magic, burst damage

**Reward Weights**:
- Spell damage: ×3.0
- Spell kills: +80 + 40 bonus
- Optimal distance (5-10): +10
- Mana efficiency: ×0.5

**Training Parameters**:
- Learning rate: 0.00035
- Exploration noise: 0.12

### Tank

**Characteristics**: Very high HP/DEF, crowd control

**Reward Weights**:
- Aggro count: ×15
- Damage absorbed: ×2.0
- Allies died: -80
- Survival: +150

**Training Parameters**:
- Team reward weight: 0.7
- Individual weight: 0.3

### Ranged

**Characteristics**: Sustained DPS from safety, kiting

**Reward Weights**:
- Sustained damage: ×1.5
- Optimal distance (7-12): +15
- Kiting active: +8
- Damage taken: -1.2

**Training Parameters**:
- Learning rate: 0.0003
- Exploration noise: 0.1

---

## Monitoring Training

### TensorBoard

```bash
# Start TensorBoard
tensorboard --logdir=/opt/ml_monster_ai/logs/tensorboard --port=6006

# View in browser
http://localhost:6006
```

Metrics tracked:
- Loss (train/validation)
- Q-values (mean/max)
- TD errors
- Rewards
- Episode length
- Win rate

### Training Logs

Real-time logs:
```bash
tail -f /opt/ml_monster_ai/logs/training.log
```

### VRAM Monitoring

```bash
# Watch VRAM usage
watch -n 1 nvidia-smi
```

Target VRAM during training:
- **Models**: 6 GB (all 54 loaded)
- **Training**: 4 GB (gradients, optimizer)
- **Total**: 10-11 GB / 12 GB (83-92%)

### Performance Metrics

Expected training metrics:
- **Convergence**: 60-80 epochs
- **Final loss**: <0.5 for DQN, <0.2 for PPO
- **Validation win rate**: >70% vs traditional AI
- **Inference latency**: <15ms per decision

---

## Troubleshooting

### VRAM Out of Memory

**Symptoms**: CUDA OOM errors during training

**Solutions**:
1. Reduce batch size: `--batch-size 128` (from 256)
2. Pause continuous training (edit `training_config.yaml`)
3. Train models sequentially (don't use `--parallel`)
4. Use gradient accumulation

```python
# In training code
for i in range(accumulation_steps):
    loss = train_step()
    loss = loss / accumulation_steps
    loss.backward()

optimizer.step()
optimizer.zero_grad()
```

### Training Not Converging

**Symptoms**: Loss not decreasing after many epochs

**Diagnosis**:
1. Check reward distribution: `python scripts/analyze_rewards.py`
2. Verify data quality: Check for NaN/Inf in states
3. Check learning rate: May be too high or too low

**Solutions**:
- Adjust learning rate: Try 10x lower or higher
- Check reward shaping: Ensure rewards are not too sparse
- Increase buffer size: More diverse experiences
- Add curriculum learning: Start with easier tasks

### Models Overfitting

**Symptoms**: Training loss decreases but validation loss increases

**Solutions**:
- Increase dropout: `dropout: 0.3` (from 0.2)
- Add data augmentation
- Reduce model size
- Early stopping (already enabled)
- L2 regularization: `weight_decay: 0.0001`

### Slow Training

**Symptoms**: Training taking too long

**Solutions**:
- Enable AMP: `use_amp: true` (should be default)
- Increase batch size: More GPU utilization
- Use multiple GPUs (if available)
- Reduce validation frequency
- Profile training: `python scripts/profile_training.py`

### Database Connection Errors

**Symptoms**: Can't connect to PostgreSQL

**Solutions**:
1. Verify PostgreSQL is running: `systemctl status postgresql`
2. Check credentials in config
3. Verify network access: `psql -h localhost -U ml_user -d ragnarok_ml`
4. Check firewall rules

---

## Advanced Topics

### Curriculum Learning

Enable progressive difficulty:

```yaml
# training_config.yaml
curriculum:
  enabled: true
  num_levels: 5
  level_thresholds: [50.0, 100.0, 200.0, 400.0, 800.0]
```

Training starts easy (level 1) and increases difficulty as model improves.

### Multi-Agent Training

For pack coordination:

```bash
python scripts/train_coordination.py \
    --archetype aggressive \
    --pack-size 5 \
    --episodes 1000
```

Uses CTDE (Centralized Training, Decentralized Execution).

### Continuous Learning

Enable background training in production:

```yaml
# training_config.yaml
strategies:
  continuous:
    enabled: true
    update_frequency: "hourly"
    min_new_experiences: 1000
```

Models automatically improve from live gameplay data.

### Transfer Learning

Transfer knowledge between archetypes:

```bash
python scripts/transfer_learning.py \
    --source-archetype aggressive \
    --target-archetype mage \
    --model-type combat_dqn
```

Useful for faster training of similar archetypes.

---

## Training Schedule

### Full Training (All 54 Models)

**Sequential Training** (recommended):
- **Time**: ~150-200 hours (6-8 days continuous)
- **VRAM**: 10-11 GB peak
- **Command**: `python scripts/train_all_models.py --all --epochs 100`

**Per-Archetype Estimate**:
- Aggressive: ~25 hours
- Defensive: ~25 hours
- Support: ~25 hours
- Mage: ~30 hours
- Tank: ~25 hours
- Ranged: ~25 hours

**Per-Model-Type Estimate**:
- Combat DQN: 2-3 hours
- Movement PPO: 3-4 hours
- Skill DQN: 2 hours
- Threat Assessment: 1 hour
- Team Coordination: 3-4 hours
- Spatial ViT: 4-5 hours
- Temporal Transformer: 4-5 hours
- Pattern Recognition: 5-7 hours
- Soft Actor-Critic: 2-3 hours

### Incremental Training

Train in stages:

```bash
# Stage 1: Core models (24 hours)
python scripts/train_all_models.py \
    --all \
    --model-types combat_dqn movement_ppo threat_assessment \
    --epochs 100

# Stage 2: Advanced models (48 hours)
python scripts/train_all_models.py \
    --all \
    --model-types spatial_vit temporal_transformer pattern_recognition \
    --epochs 100

# Stage 3: Coordination models (24 hours)
python scripts/train_all_models.py \
    --all \
    --model-types team_coordination soft_actor_critic skill_dqn \
    --epochs 100
```

---

## Validation Criteria

### Minimum Performance Targets

Before deploying to production, models must achieve:

| Metric | Target | Measurement |
|--------|--------|-------------|
| **Win Rate** | >70% | vs traditional AI |
| **Avg Reward** | >100 | Per episode |
| **Inference Latency** | <15ms | P95 latency |
| **Action Diversity** | >2.0 | Entropy |
| **Value Accuracy** | >80% | Explained variance |

### Validation Command

```bash
python scripts/evaluate_models.py \
    --archetype aggressive \
    --num-episodes 100 \
    --save-report
```

---

## Production Deployment

### 1. Export to ONNX

```bash
python scripts/export_to_onnx.py --all --verify --optimize
```

### 2. Register Models

Models are automatically registered in PostgreSQL `ml_models` table during training.

Verify registration:

```sql
SELECT archetype, model_type, version, is_deployed, 
       metrics->>'val_loss' as val_loss
FROM ml_models
ORDER BY archetype, model_type;
```

### 3. Deploy to Inference Service

```bash
# Mark models as deployed
python scripts/deploy_models.py --all

# Restart inference service
systemctl restart ml_monster_ai_inference
```

---

## Configuration Reference

### training_config.yaml

Main training configuration. Key sections:

```yaml
training:
  batch_size: 256
  learning_rate: 0.0003
  epochs: 100
  gamma: 0.99

hardware:
  device: "cuda:0"
  max_vram_gb: 11.0
  use_amp: true

checkpoints:
  save_frequency: 1000
  keep_last_n: 5
```

### model_params.yaml

Model-specific hyperparameters. Example:

```yaml
models:
  combat_dqn:
    hidden_dims: [1024, 1024, 512, 512, 256, 256, 128, 128, 64, 64]
    dropout: 0.2
    learning_rate: 0.0001
```

---

## Best Practices

### DO:
✅ Train sequentially to avoid VRAM issues  
✅ Use automatic mixed precision (FP16)  
✅ Monitor VRAM usage with `nvidia-smi`  
✅ Save checkpoints frequently  
✅ Validate before deployment  
✅ Use TensorBoard for monitoring  
✅ Keep training logs  

### DON'T:
❌ Train in parallel without VRAM management  
❌ Skip validation  
❌ Deploy models with <60% win rate  
❌ Ignore convergence metrics  
❌ Train without data augmentation  
❌ Use FP32 (wastes VRAM and slower)  

---

## Resources

### Disk Space Requirements

- **Training data**: ~5 GB per archetype = 30 GB total
- **Checkpoints**: ~10 GB per archetype = 60 GB total
- **Final models**: ~5 GB (54 models)
- **Logs**: ~5 GB
- **Total**: ~100 GB recommended

### Network Requirements

- PostgreSQL connection: Low bandwidth (<1 Mbps)
- Data transfer: Periodic (batch inserts)
- No real-time streaming required

---

## Support

For issues:
1. Check logs: `/opt/ml_monster_ai/logs/training.log`
2. Review TensorBoard: `http://localhost:6006`
3. Verify VRAM: `nvidia-smi`
4. Check database: `psql -U ml_user -d ragnarok_ml`

---

## Next Steps

After training completes:
1. **Validate all models**: Ensure >70% win rate
2. **Export to ONNX**: FP16 for production
3. **Deploy to inference service**: Phase 12
4. **Enable continuous learning**: Background training
5. **Monitor production performance**: Track metrics

---

**Last Updated**: January 21, 2026  
**Version**: 2.0  
**Status**: Production Ready
