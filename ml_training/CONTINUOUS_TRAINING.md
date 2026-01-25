# Continuous Training Pipeline Documentation

**Version:** 2.0  
**Last Updated:** 2026-01-25  
**Status:** Production Ready

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [How It Works](#how-it-works)
4. [Configuration](#configuration)
5. [Hot Reload Mechanism](#hot-reload-mechanism)
6. [Resource Management](#resource-management)
7. [Deployment Process](#deployment-process)
8. [Monitoring](#monitoring)
9. [Operations Guide](#operations-guide)
10. [Troubleshooting](#troubleshooting)
11. [Performance Considerations](#performance-considerations)

---

## Overview

The Continuous Training Pipeline enables **ongoing model improvement from live gameplay data without interrupting the game**. It runs in the background, continuously learning from player interactions and monster behaviors to improve AI quality over time.

### Key Features

✅ **Background Training** - Runs without blocking inference service  
✅ **Memory Efficient** - Streams data from database, doesn't load everything into RAM  
✅ **Hot Reload** - Updates models without service restart (zero downtime)  
✅ **Resource Aware** - Respects CPU/VRAM limits (70% for inference, 30% for training)  
✅ **Safe Deployment** - Validates before deploying, rolls back on degradation  
✅ **Automatic** - Runs continuously with minimal operator intervention

### Architecture Components

```
┌────────────────────────────────────────────────────────────────┐
│                   CONTINUOUS TRAINING PIPELINE                 │
├────────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  1. EXPERIENCE STREAMING (ExperienceStream)              │ │
│  │  • Streams from PostgreSQL ml_experience_replay table   │ │
│  │  • Server-side cursor (memory efficient)                │ │
│  │  • Filters by archetype, priority, timestamp            │ │
│  │  • Yields batches of 256 experiences                    │ │
│  └──────────────────────────────────────────────────────────┘ │
│                           │                                    │
│                           ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  2. INCREMENTAL TRAINING (ContinuousTrainer)             │ │
│  │  • Loads model checkpoint or creates new                │ │
│  │  • Trains for N steps (default 1000)                    │ │
│  │  • Uses gradient accumulation (memory efficient)        │ │
│  │  • Saves checkpoints every 500 steps                    │ │
│  └──────────────────────────────────────────────────────────┘ │
│                           │                                    │
│                           ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  3. VALIDATION (ContinuousValidator)                     │ │
│  │  • Validates on held-out test set                       │ │
│  │  • Compares with production model                       │ │
│  │  • Checks quality gates (accuracy, latency)             │ │
│  │  • Computes improvement delta                           │ │
│  └──────────────────────────────────────────────────────────┘ │
│                           │                                    │
│                           ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  4. ONNX EXPORT & DEPLOYMENT                             │ │
│  │  • Exports PyTorch → ONNX (FP16)                        │ │
│  │  • Verifies export correctness                          │ │
│  │  • Copies to production directory                       │ │
│  │  • Triggers hot reload via HTTP                         │ │
│  └──────────────────────────────────────────────────────────┘ │
│                           │                                    │
│                           ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  5. HOT RELOAD (ONNXInferenceEngine)                     │ │
│  │  • Loads new ONNX to temp session                       │ │
│  │  • Verifies model validity                              │ │
│  │  • Atomically swaps with lock                           │ │
│  │  • Cleans up old model                                  │ │
│  └──────────────────────────────────────────────────────────┘ │
│                           │                                    │
│                           ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐ │
│  │  6. MONITORING (RollbackDetector)                        │ │
│  │  • Monitors production performance                      │ │
│  │  • Detects degradation                                  │ │
│  │  • Triggers automatic rollback if needed                │ │
│  └──────────────────────────────────────────────────────────┘ │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

---

## Architecture

### Component Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    PostgreSQL (Data Source)                      │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ ml_experience_replay (TimescaleDB Hypertable)            │  │
│  │ • 30 days retention                                      │  │
│  │ • 7 days compression                                     │  │
│  │ • Indexed by archetype, priority, timestamp              │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ Server-side Cursor (streaming)
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│              Continuous Training Service (Python)                │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ Main Loop (continuous_training.py)                       │  │
│  │ • Checks every 5 minutes                                 │  │
│  │ • Monitors resources (CPU < 30%, VRAM < 70%)            │  │
│  │ • Builds training queue (priority-based)                │  │
│  │ • Trains models incrementally                           │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ Components:                                              │  │
│  │ • ExperienceStream - Database streaming                 │  │
│  │ • ContinuousValidator - Model validation                │  │
│  │ • ResourceMonitor - CPU/VRAM/RAM tracking              │  │
│  │ • DQNTrainer/PPOTrainer - Model training               │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                            │
                            │ HTTP POST /reload/{archetype}/{model_type}
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│              ML Inference Service (Python)                       │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ HTTP Server (port 8080)                                  │  │
│  │ • POST /reload/{archetype}/{model_type}                 │  │
│  │ • GET /health                                            │  │
│  │ • GET /models                                            │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ ONNXInferenceEngine.reload_single_model()                │  │
│  │ • Loads new ONNX model                                   │  │
│  │ • Verifies dimensions and test inference                │  │
│  │ • Atomically swaps with asyncio.Lock                    │  │
│  │ • Updates metadata                                       │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## How It Works

### Training Cycle Flow

```
1. CHECK TRIGGER
   ├─ Every 5 minutes (configurable)
   ├─ Check if ≥ 1000 new experiences
   └─ Check resources available (CPU < 30%, VRAM < 70%)

2. BUILD QUEUE
   ├─ High frequency: combat_dqn, movement_ppo, team_coordination, SAC
   ├─ Medium frequency: skill_dqn, threat_assessment (every 3rd cycle)
   └─ Low frequency: transformers (every 10th cycle)

3. FOR EACH MODEL IN QUEUE:
   │
   ├─ a. STREAM EXPERIENCES
   │   ├─ Query PostgreSQL with server-side cursor
   │   ├─ Filter by archetype and priority
   │   ├─ Yield batches of 256 experiences
   │   └─ Track last processed timestamp
   │
   ├─ b. LOAD OR CREATE MODEL
   │   ├─ Check for latest checkpoint
   │   ├─ Load if exists, else create fresh model
   │   └─ Move to GPU (if available)
   │
   ├─ c. TRAIN INCREMENTALLY
   │   ├─ Sample mini-batches (256 samples)
   │   ├─ Train for up to 1000 steps
   │   ├─ Use gradient accumulation (effective batch = 1024)
   │   ├─ Save checkpoint every 500 steps
   │   └─ Validate every 1000 steps
   │
   ├─ d. VALIDATE MODEL
   │   ├─ Load validation dataset (1000 samples)
   │   ├─ Calculate: accuracy, reward, win rate, latency
   │   ├─ Compare with production model
   │   └─ Check quality gates
   │
   ├─ e. DEPLOY (if improved)
   │   ├─ Export PyTorch → ONNX
   │   ├─ Verify ONNX matches PyTorch output
   │   ├─ Copy to production directory
   │   ├─ HTTP POST to inference service /reload
   │   └─ Log deployment to database
   │
   └─ f. MONITOR (after deployment)
       ├─ Track production performance
       ├─ Compare with baseline
       ├─ Rollback if degradation > 10%
       └─ Update baseline if stable

4. SLEEP & REPEAT
   └─ Wait 5 minutes before next cycle
```

### Data Flow

```
PostgreSQL ml_experience_replay
  ↓ (server-side cursor, memory efficient)
ExperienceStream.stream_new_experiences()
  ↓ (yields batches of 256)
ContinuousTrainer.train_model_incremental()
  ↓ (mini-batch gradient descent)
PyTorch Model (updated weights)
  ↓ (validation)
ContinuousValidator.validate_before_deployment()
  ↓ (if passes quality gates)
torch.onnx.export()
  ↓ (ONNX file)
HTTP POST /reload/{archetype}/{model_type}
  ↓ (hot reload request)
ONNXInferenceEngine.reload_single_model()
  ↓ (atomic swap with lock)
Production Model (serving inferences)
```

---

## Configuration

### Main Configuration File

Location: `/opt/ml_monster_ai/configs/continuous_training_config.yaml`

```yaml
continuous_training:
  enabled: true  # Master enable/disable switch
  
  # Scheduling
  check_interval_seconds: 300  # How often to check for new data
  min_new_experiences: 1000   # Minimum experiences to trigger training
  
  # Resources
  max_cpu_usage: 30          # Pause if CPU > 30%
  max_vram_usage: 0.7        # Use max 70% VRAM
  pause_on_high_load: true   # Auto-pause on high load
  
  # Training
  batch_size: 256
  max_steps_per_iteration: 1000
  validation_frequency: 1000
  
  # Deployment
  auto_deploy: true           # Auto-deploy improved models
  min_improvement: 0.01      # 1% improvement required
  rollback_on_degradation: true
```

### Key Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `enabled` | `true` | Master switch for continuous training |
| `check_interval_seconds` | `300` | How often to check for new data (5 min) |
| `min_new_experiences` | `1000` | Minimum experiences to start training |
| `max_cpu_usage` | `30` | Maximum CPU% for training (leave 70% for inference) |
| `max_vram_usage` | `0.7` | Maximum VRAM fraction (leave 30% for inference) |
| `batch_size` | `256` | Mini-batch size for training |
| `gradient_accumulation_steps` | `4` | Effective batch = 256 × 4 = 1024 |
| `max_steps_per_iteration` | `1000` | Max training steps per model |
| `checkpoint_frequency` | `500` | Save checkpoint every N steps |
| `validation_frequency` | `1000` | Validate every N steps |
| `auto_deploy` | `true` | Automatically deploy improved models |
| `min_improvement` | `0.01` | Minimum 1% improvement required |
| `rollback_on_degradation` | `true` | Auto-rollback on performance drop |

---

## Hot Reload Mechanism

### How Hot Reload Works

**Zero-Downtime Model Updates**

```python
# 1. Training service completes training
new_model = train_model(experiences)

# 2. Export to ONNX
onnx_path = export_to_onnx(new_model)

# 3. Trigger hot reload via HTTP
POST http://localhost:8080/reload/aggressive/combat_dqn

# 4. Inference service reloads model
async def reload_single_model(archetype, model_type):
    async with self.reload_lock:  # Thread safety
        # Load new model to temp
        new_session = ort.InferenceSession(new_model_path)
        
        # Verify model
        test_output = new_session.run(test_input)
        
        # Atomic swap
        old_session = self.sessions[(archetype, model_type)]
        self.sessions[(archetype, model_type)] = new_session
        
        # Clean up
        del old_session
    
    return True

# 5. Next inference uses new model automatically
```

### Thread Safety

The hot reload uses `asyncio.Lock` to ensure atomic model swapping:

```python
async with self.reload_lock:
    # No other inference can happen during swap
    self.sessions[key] = new_session
    # Swap is atomic - either old model or new model, never broken state
```

### Verification Steps

Before deploying, the system verifies:

1. **Model loads successfully** - ONNX file is valid
2. **Dimensions match** - Input/output shapes unchanged
3. **Test inference works** - Can run forward pass
4. **PyTorch ↔ ONNX agreement** - Outputs match within 1e-4
5. **Quality gates passed** - Accuracy ≥ 30%, win rate ≥ 20%
6. **Improvement validated** - Better than production by ≥ 1%
7. **Latency acceptable** - Inference < 50ms for batch of 128

---

## Resource Management

### Resource Allocation Strategy

**Target:** 70% for inference, 30% for training

```
┌────────────────────────────────────────────────────┐
│              12GB VRAM ALLOCATION                  │
├────────────────────────────────────────────────────┤
│                                                    │
│  INFERENCE (8.4GB = 70%)                          │
│  ├─ 54 models loaded                              │
│  ├─ Inference buffers                             │
│  └─ Reserved for serving                          │
│                                                    │
│  TRAINING (3.6GB = 30%)                           │
│  ├─ Model gradients (1.2GB)                       │
│  ├─ Optimizer state (1.2GB)                       │
│  ├─ Replay batches (0.6GB)                        │
│  └─ Training workspace (0.6GB)                    │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Resource Monitoring

The `ResourceMonitor` continuously checks:

```python
resources = monitor.check_resources()

# Returns:
{
    'can_train': True/False,       # Overall decision
    'cpu_percent': 25.0,            # Current CPU%
    'cpu_max': 30,                  # Threshold
    'ram_used_gb': 6.5,             # Current RAM
    'ram_max_gb': 8,                # Threshold
    'vram_percent': 65.0,           # Current VRAM%
    'vram_max_percent': 70,         # Threshold
    'gpu_available': True           # GPU status
}
```

### Pause/Resume Behavior

**Automatic Pausing:**

```
IF cpu_percent > max_cpu_usage OR
   vram_percent > max_vram_usage OR
   ram_used_gb > max_ram_gb:
    
    PAUSE training
    LOG "Resources exhausted, pausing training"
    WAIT for resources to free up (max 5 minutes)
    
    IF resources available:
        RESUME training
    ELSE:
        SKIP this training cycle
```

---

## Deployment Process

### Deployment Workflow

```
┌─────────────────────────────────────────────────────┐
│          DEPLOYMENT DECISION TREE                   │
└─────────────────────────────────────────────────────┘

Training Complete
    ↓
Validate Model
    ├─ Accuracy ≥ 30%? ──NO──> REJECT
    ├─ Win Rate ≥ 20%? ──NO──> REJECT
    ├─ Latency < 50ms? ──NO──> REJECT
    └─ YES
        ↓
Compare with Production
    ├─ Improvement ≥ 1%? ──NO──> REJECT (not enough improvement)
    └─ YES
        ↓
Export to ONNX
    ├─ PyTorch → ONNX
    ├─ Verify export (outputs match)
    └─ Copy to production dir
        ↓
Trigger Hot Reload
    ├─ HTTP POST /reload
    ├─ Inference service reloads
    └─ Verify reload succeeded
        ↓
Log Deployment
    ├─ Insert into ml_model_versions
    ├─ Mark as active
    └─ Deactivate old version
        ↓
Monitor Performance
    ├─ Track accuracy/reward for 30 min
    ├─ Compare with baseline
    └─ If degradation > 10%:
        ROLLBACK to previous version
```

### Manual Deployment

You can manually trigger deployment:

```bash
# Deploy specific model
curl -X POST http://localhost:8080/reload/aggressive/combat_dqn

# Check deployment status
curl http://localhost:8080/models | jq '.models[] | select(.archetype == "aggressive" and .model_type == "combat_dqn")'
```

### Rollback Procedure

**Automatic Rollback:**

```python
# Monitors production performance every 30 minutes
rollback_detector.monitor_deployed_model('aggressive', 'combat_dqn')

# If performance drops > 10%:
# 1. Fetch previous version from ml_model_versions
# 2. Copy previous ONNX to production location
# 3. Trigger hot reload
# 4. Mark rollback in ml_continuous_training_log
```

**Manual Rollback:**

```bash
# Query previous version
psql -d ragnarok_ml -c "
SELECT version, onnx_path
FROM ml_model_versions
WHERE archetype = 'aggressive'
    AND model_type = 'combat_dqn'
    AND active = FALSE
ORDER BY deployed_at DESC
LIMIT 1;
"

# Copy previous version
cp /path/to/previous/model.onnx /opt/ml_monster_ai/models/production/aggressive/combat_dqn.onnx

# Trigger reload
curl -X POST http://localhost:8080/reload/aggressive/combat_dqn
```

---

## Monitoring

### Logs

**Locations:**
- Service logs: `/opt/ml_monster_ai/logs/continuous_training/continuous_training.log`
- Systemd journal: `journalctl -u continuous-training -f`

**Log Levels:**
- `DEBUG` - Detailed execution traces
- `INFO` - Training progress, deployments
- `WARNING` - Resource issues, validation failures
- `ERROR` - Training errors, deployment failures
- `CRITICAL` - System failures

**Example Logs:**

```
2026-01-25 10:00:00 - INFO - Starting training cycle 15
2026-01-25 10:00:05 - INFO - Retrieved 1523 experiences for training
2026-01-25 10:00:10 - INFO - Training: aggressive/combat_dqn
2026-01-25 10:00:15 - INFO -   Step 100/1000: loss=0.3421, avg_loss=0.3856
2026-01-25 10:01:00 - INFO - Validating final model: aggressive/combat_dqn
2026-01-25 10:01:05 - INFO - ✓ Quality gate passed: accuracy=75.3%, win_rate=62.1%
2026-01-25 10:01:10 - INFO - ✓ Production comparison passed: improvement=3.2%
2026-01-25 10:01:15 - INFO - ✓ ONNX export complete: /opt/.../combat_dqn.onnx
2026-01-25 10:01:20 - INFO - ✓ Hot reload succeeded
2026-01-25 10:01:20 - INFO - ✓ Model deployed: aggressive/combat_dqn
```

### Database Monitoring

**Check training history:**

```sql
-- Recent training runs
SELECT 
    archetype,
    model_type,
    training_start,
    steps_taken,
    validation_metric,
    improvement_delta,
    deployed,
    rollback
FROM ml_continuous_training_log
ORDER BY created_at DESC
LIMIT 20;

-- Deployment history
SELECT 
    archetype,
    model_type,
    version,
    deployed_at,
    active,
    performance_score
FROM ml_model_versions
WHERE active = TRUE
ORDER BY deployed_at DESC;

-- Rollback events
SELECT * 
FROM ml_continuous_training_log
WHERE rollback = TRUE
ORDER BY created_at DESC;
```

### Metrics (Prometheus)

If Prometheus is enabled, metrics are available at `http://localhost:9091`:

- `ml_continuous_training_cycles_total` - Total training cycles
- `ml_continuous_training_models_trained_total` - Models trained
- `ml_continuous_training_models_deployed_total` - Models deployed
- `ml_continuous_training_rollbacks_total` - Rollbacks performed
- `ml_continuous_training_iteration_duration_seconds` - Training time
- `ml_model_improvement_delta` - Improvement vs production

---

## Operations Guide

### Starting the Service

```bash
# Install systemd service
sudo cp continuous-training.service /etc/systemd/system/
sudo systemctl daemon-reload

# Start service
sudo systemctl start continuous-training

# Enable auto-start on boot
sudo systemctl enable continuous-training

# Check status
sudo systemctl status continuous-training
```

### Stopping the Service

```bash
# Graceful stop (finishes current training iteration)
sudo systemctl stop continuous-training

# Check it stopped
sudo systemctl status continuous-training
```

### Viewing Logs

```bash
# Real-time logs
sudo journalctl -u continuous-training -f

# Last 100 lines
sudo journalctl -u continuous-training -n 100

# Logs since yesterday
sudo journalctl -u continuous-training --since yesterday

# Filter by priority
sudo journalctl -u continuous-training -p err  # Errors only
```

### Checking Service Health

```bash
# Is service running?
sudo systemctl is-active continuous-training

# Service status
sudo systemctl status continuous-training

# Resource usage
ps aux | grep continuous_training
nvidia-smi  # Check VRAM usage

# Database activity
psql -d ragnarok_ml -c "SELECT COUNT(*) FROM ml_experience_replay WHERE timestamp > NOW() - INTERVAL '1 hour';"
```

### Manual Training Trigger

```python
# Run single training iteration manually
python /opt/ml_monster_ai/training/continuous_training.py --config /opt/ml_monster_ai/configs/continuous_training_config.yaml --single-iteration
```

---

## Troubleshooting

### Issue: Service Won't Start

**Symptoms:**
```bash
$ sudo systemctl status continuous-training
● continuous-training.service - Continuous ML Training Service
   Loaded: loaded
   Active: failed (Result: exit-code)
```

**Diagnosis:**

```bash
# Check logs for error
sudo journalctl -u continuous-training -n 50

# Common errors:
# - Database connection failure
# - Missing configuration file
# - Python dependency missing
# - Permission issues
```

**Solutions:**

```bash
# Verify database connection
psql -h localhost -U ml_user -d ragnarok_ml -c "SELECT 1;"

# Check config file exists
ls -la /opt/ml_monster_ai/configs/continuous_training_config.yaml

# Verify Python environment
/opt/ml_monster_ai/venv/bin/python --version
/opt/ml_monster_ai/venv/bin/pip list | grep torch

# Check permissions
ls -la /opt/ml_monster_ai/training/continuous_training.py
sudo chown -R ml_user:ml_user /opt/ml_monster_ai
```

---

### Issue: Training Not Happening

**Symptoms:**
- Service running but no training logs
- No new deployments
- `models_trained=0` in logs

**Diagnosis:**

```bash
# Check if enough experiences
psql -d ragnarok_ml -c "
SELECT 
    archetype,
    COUNT(*) as count,
    MAX(timestamp) as latest
FROM ml_experience_replay
WHERE timestamp > NOW() - INTERVAL '1 hour'
GROUP BY archetype;
"

# Check resource usage
ps aux | grep continuous_training
nvidia-smi
```

**Possible Causes:**

1. **Not enough new experiences**
   - Solution: Lower `min_new_experiences` in config
   - Check if game is generating experiences

2. **Resources exhausted**
   - Check CPU% < 30%, VRAM% < 70%
   - Solution: Increase resource limits or reduce inference load

3. **Training disabled in config**
   - Check `enabled: true` in config

4. **Time interval not met**
   - Check `training_interval_seconds` - may need to wait

---

### Issue: Hot Reload Failing

**Symptoms:**
```
ERROR - Hot reload failed: Connection refused
ERROR - Deployment failed: hot reload unsuccessful
```

**Diagnosis:**

```bash
# Is inference service running?
sudo systemctl status ml-inference

# Is HTTP server listening?
netstat -tuln | grep 8080
curl http://localhost:8080/health

# Check inference service logs
sudo journalctl -u ml-inference -n 100 | grep reload
```

**Solutions:**

```bash
# Restart inference service
sudo systemctl restart ml-inference

# Verify HTTP server started
curl http://localhost:8080/health

# Check if HTTP server config enabled
# In inference_config.yaml:
http_server:
  enabled: true
  port: 8080
```

---

### Issue: Models Degrading After Deployment

**Symptoms:**
- Validation shows improvement but production performance drops
- Automatic rollbacks happening frequently

**Diagnosis:**

```sql
-- Check rollback frequency
SELECT COUNT(*) as rollback_count
FROM ml_continuous_training_log
WHERE rollback = TRUE
    AND created_at > NOW() - INTERVAL '24 hours';

-- Check validation vs production delta
SELECT 
    archetype,
    model_type,
    validation_metric,
    improvement_delta,
    deployed
FROM ml_continuous_training_log
WHERE deployed = TRUE
ORDER BY created_at DESC
LIMIT 10;
```

**Possible Causes:**

1. **Validation data not representative**
   - Solution: Increase `validation_samples`
   - Ensure validation data covers diverse scenarios

2. **Overfitting to recent experiences**
   - Solution: Increase `experience_lookback_days`
   - Enable `archetype_balancing`

3. **Distribution shift**
   - Player behavior changed
   - Solution: Retrain from scratch with recent data

---

### Issue: High Resource Usage

**Symptoms:**
- Service frequently paused due to resources
- Training very slow
- System unresponsive

**Diagnosis:**

```bash
# Check resource usage
top -u ml_user
nvidia-smi
free -h

# Check training config
grep -A 5 "max_cpu_usage\|max_vram_usage" /opt/ml_monster_ai/configs/continuous_training_config.yaml
```

**Solutions:**

```yaml
# Reduce resource limits in config
continuous_training:
  max_cpu_usage: 20  # Lower from 30
  max_vram_usage: 0.6  # Lower from 0.7
  batch_size: 128  # Reduce from 256
  max_steps_per_iteration: 500  # Reduce from 1000
```

---

## Performance Considerations

### Training Speed

**Expected Training Times:**

| Model Type | Steps | Batch Size | Time (GPU) | Time (CPU) |
|------------|-------|------------|------------|------------|
| combat_dqn | 1000 | 256 | ~2 minutes | ~15 minutes |
| movement_ppo | 1000 | 128 | ~3 minutes | ~20 minutes |
| team_coordination | 1000 | 64 | ~2 minutes | ~12 minutes |
| transformers | 1000 | 32 | ~8 minutes | ~60 minutes |

**Full Training Cycle (all high-frequency models, 6 archetypes):**
- GPU: ~20-30 minutes
- CPU: ~2-3 hours

### Memory Efficiency

**Experience Streaming:**

```python
# BAD: Load all experiences (exhausts RAM)
experiences = db.execute("SELECT * FROM ml_experience_replay")  # DON'T DO THIS

# GOOD: Stream in batches (memory efficient)
async for batch in stream.stream_new_experiences(batch_size=256):
    train_on_batch(batch)  # Only 256 in memory at once
```

**Gradient Accumulation:**

```python
# Instead of large batch (requires more VRAM):
batch_size = 1024  # 1024 × 64 × 4 bytes = 256KB per batch

# Use gradient accumulation (4× less VRAM):
batch_size = 256
accumulation_steps = 4
# Effective batch = 256 × 4 = 1024, but only 256 in memory at once
```

### Throughput Optimization

**Current Performance:**

- **Check interval:** 5 minutes
- **Training time:** 2-5 minutes per model
- **Models per cycle:** 4-6 (high frequency)
- **Cycle time:** 10-30 minutes

**Optimization Tips:**

1. **Adjust check interval** - Lower for faster updates, higher for less overhead
2. **Tune batch size** - Larger = faster but more memory
3. **Reduce max_steps** - Faster iterations, more frequent deployments
4. **Prioritize important models** - Train critical models more often

---

## Advanced Topics

### Custom Model Training

To add a new model to continuous training:

1. **Define model architecture** in `models/`
2. **Add to config:**
   ```yaml
   model_priority:
     high_frequency:
       - my_custom_model
   
   hyperparameters:
     my_custom_model:
       learning_rate: 0.0001
       batch_size: 256
   ```

3. **Add trainer creation** in `continuous_training.py`:
   ```python
   elif model_type == 'my_custom_model':
       trainer = CustomTrainer(model=model, **hyperparams)
   ```

4. **Restart service**

### Multi-GPU Support

```yaml
# Use specific GPU
Environment="CUDA_VISIBLE_DEVICES=1"  # Use GPU 1 instead of 0

# Use multiple GPUs (requires code changes)
# Currently single GPU only
```

### Distributed Training

For very large models or high-throughput requirements:

```python
# Use PyTorch DDP (Distributed Data Parallel)
# Requires multiple machines or GPUs
# Not currently implemented
```

---

## Best Practices

### Do's ✅

- **Monitor logs regularly** - Catch issues early
- **Validate deployments** - Always enable `validate_before_deploy`
- **Set realistic thresholds** - `min_improvement: 0.01` is reasonable
- **Use rollback protection** - Enable `rollback_on_degradation`
- **Track metrics** - Query `ml_continuous_training_log` regularly
- **Test in staging first** - Before enabling in production

### Don'ts ❌

- **Don't disable validation** - Could deploy bad models
- **Don't set resources too high** - Leave room for inference
- **Don't train on single day** - Use at least 7 days lookback
- **Don't ignore rollbacks** - Investigate why models degrade
- **Don't skip quality gates** - They prevent bad deployments

---

## System Requirements

### Minimum Requirements

- **CPU:** 8+ cores (4 for inference, 4 for training)
- **RAM:** 16GB (8GB for inference, 8GB for training)
- **VRAM:** 12GB GPU (8GB for inference, 4GB for training)
- **Storage:** 100GB SSD (models, checkpoints, logs)
- **PostgreSQL:** 17.7+ with TimescaleDB 2.24+
- **Python:** 3.11+
- **PyTorch:** 2.5+
- **ONNX Runtime:** 1.17+

### Recommended Requirements

- **CPU:** 16+ cores
- **RAM:** 32GB
- **VRAM:** 24GB GPU (RTX 4090 or better)
- **Storage:** 500GB NVMe SSD
- **Network:** 1Gbps (for distributed setup)

---

## FAQ

**Q: How often are models updated?**  
A: High-frequency models (combat_dqn, movement_ppo) every ~30 minutes. Low-frequency models (transformers) every ~5 hours.

**Q: Will continuous training affect game performance?**  
A: No. Training uses max 30% CPU and 30% VRAM, leaving 70% for inference. If resources are constrained, training pauses automatically.

**Q: What happens if continuous training crashes?**  
A: The inference service continues with existing models. Systemd restarts continuous training automatically.

**Q: Can I disable continuous training?**  
A: Yes. Set `enabled: false` in config or `sudo systemctl stop continuous-training`.

**Q: How do I know if a model improved?**  
A: Check `ml_continuous_training_log` table for `improvement_delta` and `deployed=TRUE`.

**Q: What if a deployed model performs worse?**  
A: Automatic rollback triggers if performance drops >10% within 30 minutes. Previous version is restored.

**Q: How much storage does this use?**  
A: ~10GB for checkpoints (rotated weekly), ~5GB for ONNX models, ~5GB for logs (rotated monthly).

---

## References

- **Architecture:** [`plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md`](../../plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md:1)
- **Training Infrastructure:** [`ml_training/training/trainer.py`](training/trainer.py:1)
- **Inference Service:** [`ml_inference_service/main.py`](../ml_inference_service/main.py:1)
- **Inference Engine:** [`ml_inference_service/inference_engine.py`](../ml_inference_service/inference_engine.py:1)
- **Database Schema:** [`sql-files/ml_monster_ai_schema.sql`](../sql-files/ml_monster_ai_schema.sql:1)

---

## Support

For issues or questions:
- **Logs:** `sudo journalctl -u continuous-training -f`
- **Database:** Check `ml_continuous_training_log` and `ml_model_versions`
- **Service:** `sudo systemctl status continuous-training`

---

**Document Status:** ✅ **PRODUCTION READY**  
**Last Validated:** 2026-01-25  
**Authors:** ML Monster AI Team
