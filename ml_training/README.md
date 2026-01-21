# ML Monster AI - Training Pipeline
## Enhanced ML Monster AI System v2.0

Complete training infrastructure for the Enhanced ML Monster AI system.

---

## Overview

This directory contains the complete training pipeline for 54 ML models (9 types × 6 archetypes) used in the Enhanced ML Monster AI system.

### Features

✅ **9 Model Architectures**: DQN, PPO, SAC, LSTM, ViT, Transformers  
✅ **6 Archetypes**: Aggressive, Defensive, Support, Mage, Tank, Ranged  
✅ **54 Total Models**: Fully parameterized and ready to train  
✅ **FP16 Training**: Automatic mixed precision for 2x speedup  
✅ **VRAM Optimized**: Fits in 11GB (6GB models + 4GB training + 1GB buffer)  
✅ **Production Ready**: Enterprise-grade code with error handling  
✅ **PostgreSQL Integration**: Persistent experience storage  
✅ **ONNX Export**: Production inference format  

---

## Directory Structure

```
ml_training/
├── models/                  # Model architectures
│   ├── __init__.py
│   ├── model_architectures.py  # All 9 model types
│   └── model_factory.py        # Factory for creating/managing models
│
├── training/                # Training infrastructure
│   ├── __init__.py
│   ├── trainer.py          # DQN/PPO/SAC trainers
│   ├── rewards.py          # Archetype reward shaping
│   └── evaluator.py        # Model evaluation metrics
│
├── data/                    # Data management
│   ├── __init__.py
│   ├── replay_buffer.py    # Experience replay (prioritized, n-step)
│   └── preprocessor.py     # State/reward normalization
│
├── utils/                   # Utilities
│   ├── __init__.py
│   ├── model_registry.py   # PostgreSQL model tracking
│   ├── logger.py           # Logging setup
│   └── metrics.py          # Metrics tracking
│
├── config/                  # Configuration files
│   ├── training_config.yaml    # Training parameters
│   └── model_params.yaml       # Model hyperparameters
│
├── scripts/                 # Training scripts
│   ├── train_all_models.py     # Main training script
│   ├── export_to_onnx.py       # ONNX export
│   ├── collect_initial_data.py # Bootstrap data collection
│   └── deploy.sh               # Deployment script
│
└── docs/                    # Documentation
    ├── TRAINING_GUIDE.md       # Complete training guide
    ├── MODEL_ARCHITECTURE.md   # Architecture documentation
    └── README.md               # This file
```

---

## Quick Start

### 1. Deploy to Execution Directory

```bash
# Make deploy script executable
chmod +x scripts/deploy.sh

# Deploy to /opt/ml_monster_ai/
./scripts/deploy.sh
```

This creates symbolic links from `/opt/ml_monster_ai/` to this repository directory, so all changes are version controlled.

### 2. Collect Initial Data

```bash
# Activate Python environment
source /opt/ml_monster_ai/venv/bin/activate

# Collect 10K samples per archetype
python scripts/collect_initial_data.py --all --samples-per-archetype 10000
```

### 3. Train Models

```bash
# Train all 54 models
python scripts/train_all_models.py --all --epochs 100

# Or use the convenience script
/opt/ml_monster_ai/train.sh --all --epochs 100
```

### 4. Export to ONNX

```bash
# Export all models to ONNX FP16
python scripts/export_to_onnx.py --all --verify

# Or use the convenience script
/opt/ml_monster_ai/export.sh --all --verify
```

---

## Models

### Model Types (9 per archetype)

1. **Combat DQN** - Tactical combat decisions (40MB FP16)
2. **Movement PPO** - Spatial positioning (100MB FP16)
3. **Skill DQN** - Skill selection with embeddings (10MB FP16)
4. **Threat Assessment** - Ensemble threat evaluation (5MB FP16)
5. **Team Coordination LSTM** - Pack coordination (50MB FP16)
6. **Spatial ViT** - Vision transformer for spatial awareness (90MB FP16)
7. **Temporal Transformer-XL** - Long-term temporal modeling (100MB FP16)
8. **Pattern Recognition Transformer** - Combat pattern detection (250MB FP16)
9. **Soft Actor-Critic** - Continuous control (30MB FP16)

**Total per archetype**: ~675MB FP16  
**Total for 6 archetypes**: ~4GB FP16  
**With inference buffers**: ~6GB  

### Archetypes (6 total)

1. **Aggressive** - High damage, risk-taking, melee-focused
2. **Defensive** - Survival, damage mitigation, tank support
3. **Support** - Healing, buffing, keeping allies alive
4. **Mage** - Spell casting, burst damage, mana management, kiting
5. **Tank** - Aggro control, frontline, damage absorption
6. **Ranged** - Sustained DPS, kiting, distance maintenance

---

## Training

### Full Training Pipeline

```bash
# Step 1: Collect data (30 minutes)
python scripts/collect_initial_data.py --all

# Step 2: Train all models (150-200 hours)
python scripts/train_all_models.py --all --epochs 100

# Step 3: Export to ONNX (1 hour)
python scripts/export_to_onnx.py --all --verify --optimize

# Step 4: Deploy to production
# (See Phase 12 - Inference Service)
```

### Incremental Training

Train models in stages to manage time:

```bash
# Stage 1: Core combat models (30 hours)
python scripts/train_all_models.py \
    --all \
    --model-types combat_dqn movement_ppo threat_assessment \
    --epochs 100

# Stage 2: Coordination models (30 hours)
python scripts/train_all_models.py \
    --all \
    --model-types team_coordination skill_dqn soft_actor_critic \
    --epochs 100

# Stage 3: Transformer models (90 hours)
python scripts/train_all_models.py \
    --all \
    --model-types spatial_vit temporal_transformer pattern_recognition \
    --epochs 100
```

---

## Hardware Requirements

### Minimum

- **GPU**: NVIDIA RTX 3060 (12GB VRAM)
- **RAM**: 32GB
- **Storage**: 100GB SSD
- **CPU**: 8 cores

### Recommended

- **GPU**: NVIDIA RTX 4090 (24GB VRAM)
- **RAM**: 64GB
- **Storage**: 500GB NVMe SSD
- **CPU**: 16+ cores

---

## Resource Usage

### Training VRAM

```
Models (FP16):              6 GB
Training (gradients, etc):  4 GB
Safety margin:              1-2 GB
────────────────────────────────
Total:                      11-12 GB
```

### Disk Usage

```
Training data:             30 GB
Checkpoints:               60 GB
Final models:              5 GB
Logs:                      5 GB
────────────────────────────────
Total:                     ~100 GB
```

### Training Time

**Sequential** (recommended):
- Per model: 1-7 hours (depends on model type)
- Per archetype: 25-30 hours (9 models)
- All 54 models: 150-200 hours (6-8 days)

**Parallel** (experimental):
- Requires VRAM management
- Can reduce total time by 50-70%
- Higher risk of OOM errors

---

## Configuration

### training_config.yaml

Main training configuration:
- Batch sizes, learning rates
- Replay buffer settings
- Hardware settings (VRAM, GPU)
- Checkpoint frequency
- Logging options

### model_params.yaml

Model-specific hyperparameters:
- Network architectures
- Layer dimensions
- Dropout rates
- Algorithm parameters (DQN/PPO/SAC)
- Size estimates

---

## Integration

### PostgreSQL

Models integrate with PostgreSQL for:
- **Experience storage**: `ml_experience_replay` (TimescaleDB)
- **Model registry**: `ml_models` table
- **Episode outcomes**: `ml_episode_outcomes` table

### C++ Game Server

Trained models are used by:
- **Inference Service** (Phase 12): ONNX Runtime serving
- **ML Integration Layer** (Phase 10): C++ state encoding/action execution
- **Database IPC**: Request/response via PostgreSQL

---

## Monitoring

### TensorBoard

```bash
tensorboard --logdir=/opt/ml_monster_ai/logs/tensorboard --port=6006
```

View at: http://localhost:6006

### Logs

Training logs: `/opt/ml_monster_ai/logs/training.log`

```bash
tail -f /opt/ml_monster_ai/logs/training.log
```

### VRAM Monitoring

```bash
watch -n 1 nvidia-smi
```

---

## Documentation

- [`TRAINING_GUIDE.md`](TRAINING_GUIDE.md) - Complete training guide
- [`MODEL_ARCHITECTURE.md`](MODEL_ARCHITECTURE.md) - Architecture details
- [`../../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md) - Full system architecture

---

## Troubleshooting

### VRAM OOM

Reduce batch size:
```bash
python scripts/train_all_models.py --all --batch-size 128
```

### Slow Training

Enable AMP (should be default):
```yaml
# training_config.yaml
hardware:
  use_amp: true
```

### Not Converging

Check rewards:
```bash
python scripts/analyze_rewards.py --archetype aggressive
```

Adjust learning rate:
```bash
python scripts/train_all_models.py --all --lr 0.0001
```

---

## Next Steps

After training:

1. **Validate Models** - Ensure >70% win rate vs traditional AI
2. **Export to ONNX** - FP16 format for production
3. **Deploy to Inference** - Phase 12 implementation
4. **Enable Continuous Learning** - Background training from live data
5. **Monitor Performance** - Track metrics in production

---

## Development

### Adding New Model Type

1. Implement model in [`models/model_architectures.py`](models/model_architectures.py)
2. Add to `ModelFactory.MODEL_TYPES`
3. Add config to [`config/model_params.yaml`](config/model_params.yaml)
4. Add trainer logic if needed
5. Update documentation

### Adding New Archetype

1. Add to `ModelFactory.ARCHETYPES`
2. Implement reward function in [`training/rewards.py`](training/rewards.py)
3. Add config to [`config/training_config.yaml`](config/training_config.yaml)
4. Train models for new archetype

---

## License

Part of the rAthena Enhanced ML Monster AI System v2.0

---

## Contributors

Enhanced ML Monster AI Team - January 2026

---

## Status

✅ **Phase 11 - Complete**  
🚧 Phase 12 - Inference Service (Next)  

---

For detailed training instructions, see [`docs/TRAINING_GUIDE.md`](docs/TRAINING_GUIDE.md)
