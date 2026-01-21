# Automated ML Training Scheduler

## Overview

The Automated ML Training Scheduler is a production-grade system that automatically trains and deploys ML models during off-peak server hours on a weekly cadence.

### Key Features

- **Usage Pattern Analysis**: Analyzes historical player activity and system metrics to identify optimal training windows
- **Intelligent Scheduling**: Only trains during low-usage periods to minimize impact on player experience
- **Resource Monitoring**: Monitors CPU, memory, and GPU usage; pauses training if resources spike
- **Automatic Deployment**: Exports trained models to ONNX and deploys them automatically
- **Rollback Support**: Automatically rolls back to previous models if deployment fails
- **Systemd Integration**: Runs as a systemd timer service for reliability and automatic startup

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                  systemd Timer (Weekly)                      │
│                    Every Sunday 3:00 AM                      │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              Scheduler Main (scheduler_main.py)              │
├─────────────────────────────────────────────────────────────┤
│  1. Load Configuration                                       │
│  2. Analyze Usage Patterns (UsagePatternAnalyzer)           │
│  3. Check if Current Time is Off-Peak                       │
│  4. Start Training if Conditions Met                        │
│  5. Monitor Training Progress                               │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│          Usage Pattern Analyzer (usage_analyzer.py)          │
├─────────────────────────────────────────────────────────────┤
│  - Query MariaDB for player login patterns                  │
│  - Query PostgreSQL for system metrics (optional)           │
│  - Calculate usage scores by hour/day                       │
│  - Identify off-peak windows                                │
└─────────────────────────────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│        Training Scheduler (training_scheduler.py)            │
├─────────────────────────────────────────────────────────────┤
│  - Check system resources (CPU, Memory, GPU)                │
│  - Start training process in background                     │
│  - Monitor resources during training                        │
│  - Pause/resume training on resource spikes                 │
│  - Trigger deployment on success                            │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│         Training Pipeline (train_all_models.py)              │
├─────────────────────────────────────────────────────────────┤
│  - Train models for specified archetypes                    │
│  - Save checkpoints periodically                            │
│  - Validate trained models                                  │
│  - Export to ONNX format                                    │
│  - Call auto-deploy script                                  │
└──────────────────────┬──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│         Auto-Deployment (auto_deploy.sh)                     │
├─────────────────────────────────────────────────────────────┤
│  1. Verify ONNX models exist                                │
│  2. Backup existing models                                  │
│  3. Copy new models to /opt/ml_monster_ai/models            │
│  4. Restart ml-inference service                            │
│  5. Verify service started successfully                     │
│  6. Rollback on failure                                     │
└─────────────────────────────────────────────────────────────┘
```

---

## Installation

### Prerequisites

- Python 3.8+
- MariaDB (game database)
- PostgreSQL (optional, for ML metrics)
- CUDA-capable GPU (for training)
- systemd (Linux)
- sudo access

### Step 1: Install Dependencies

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler

# Install Python dependencies
pip3 install -r requirements.txt
```

### Step 2: Configure Database Access

Edit [`scheduler_config.yaml`](../ml_scheduler/scheduler_config.yaml):

```yaml
database:
  mariadb_host: 192.168.0.100
  mariadb_port: 3306
  mariadb_database: ragnarok
  mariadb_user: ragnarok
  mariadb_password: YOUR_PASSWORD_HERE
```

### Step 3: Run Installation Script

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler

# Install scheduler (requires sudo)
sudo ./install_scheduler.sh
```

The installation script will:
- Install Python dependencies
- Create required directories
- Copy configuration to `/opt/ml_monster_ai/configs/`
- Install systemd timer and service
- Create `ml_user` if needed
- Enable and start the timer

### Step 4: Verify Installation

```bash
# Check timer status
sudo systemctl list-timers ml-training-scheduler

# View full status
./view_scheduler_status.sh
```

---

## Configuration

Configuration file: [`/opt/ml_monster_ai/configs/scheduler_config.yaml`](../ml_scheduler/scheduler_config.yaml)

### Key Settings

#### Training Parameters

```yaml
training:
  archetypes: all  # or specific: 'aggressive defensive support'
  epochs: 50       # Number of training epochs
  batch_size: 256
  learning_rate: 0.0003
  auto_deploy: true  # Auto-deploy after successful training
```

#### Resource Limits

```yaml
training:
  # Defer training if exceeded BEFORE starting
  max_cpu_before_training: 30      # percent
  max_memory_before_training: 70   # percent
  
  # Pause training if exceeded DURING training
  pause_threshold_cpu: 80          # percent
  pause_threshold_memory: 90       # percent
```

#### Scheduling

```yaml
scheduling:
  window_hours: 8          # Duration of training window
  force_training: false    # Skip time window check (DANGEROUS)
```

#### Analysis

```yaml
analysis:
  history_days: 30         # Days of history to analyze
  min_data_points: 100     # Minimum samples for reliable analysis
```

---

## Usage

### Automatic Training (Default)

The scheduler runs automatically every Sunday at 3:00 AM (configurable in [`ml-training-scheduler.timer`](../ml_scheduler/ml-training-scheduler.timer)).

When triggered:
1. Analyzes last 30 days of player activity
2. Identifies off-peak time windows
3. Checks if current time is off-peak
4. If yes, starts training; if no, exits (waits for next week)

### Manual Training

To trigger training immediately:

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler

./trigger_training_now.sh
```

### Dry-Run (Testing)

Test the scheduler without actually starting training:

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler

python3 scheduler_main.py --dry-run
```

This will:
- Analyze usage patterns
- Identify off-peak windows
- Show if current time is suitable for training
- Exit without training

---

## Monitoring

### View Status

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler

./view_scheduler_status.sh
```

Shows:
- Timer status and next scheduled run
- Service status
- Configuration summary
- Recent logs
- Deployed models count

### Monitor Live Logs

```bash
# Scheduler logs
tail -f /opt/ml_monster_ai/logs/scheduler.log

# Training logs
tail -f /opt/ml_monster_ai/logs/training_*.log

# Systemd journal
sudo journalctl -u ml-training-scheduler -f
```

### Check Next Scheduled Run

```bash
systemctl list-timers ml-training-scheduler
```

---

## Workflow

### Typical Weekly Cycle

1. **Sunday 3:00 AM**: Systemd timer triggers service
2. **Scheduler starts**: Loads configuration
3. **Usage analysis**: Queries last 30 days of player data
4. **Window identification**: Finds top 3 off-peak windows
5. **Time check**: Verifies current time is in a window
6. **Resource check**: Ensures CPU/Memory/GPU available
7. **Training starts**: Launches training in background
8. **Monitoring**: Checks resources every 30 seconds
9. **Pause/Resume**: Pauses if resources spike, resumes when recovered
10. **Training completes**: After all epochs finished
11. **ONNX export**: Exports models to ONNX FP16 format
12. **Backup**: Backs up existing models
13. **Deployment**: Copies new models to `/opt/ml_monster_ai/models`
14. **Service restart**: Restarts `ml-inference` service
15. **Verification**: Checks service started successfully
16. **Completion**: Logs results

### If Training Deferred

If current time is NOT in an off-peak window:
- Scheduler logs the decision and exits
- No training occurs
- Timer will trigger again next week
- Check logs to see next scheduled window

---

## Troubleshooting

### Timer Not Running

```bash
# Check if timer is enabled
systemctl is-enabled ml-training-scheduler.timer

# Enable if disabled
sudo systemctl enable ml-training-scheduler.timer

# Start timer
sudo systemctl start ml-training-scheduler.timer

# Check status
systemctl status ml-training-scheduler.timer
```

### Training Not Starting

**Check if current time is off-peak:**

```bash
python3 scheduler_main.py --dry-run
```

**Force training (TESTING ONLY):**

Edit `/opt/ml_monster_ai/configs/scheduler_config.yaml`:

```yaml
scheduling:
  force_training: true  # CAUTION: Skips time window check
```

Then manually trigger:

```bash
./trigger_training_now.sh
```

**Check resource availability:**

```bash
# CPU usage
top

# Memory usage
free -h

# GPU memory
nvidia-smi
```

### Training Fails

**Check training logs:**

```bash
tail -n 100 /opt/ml_monster_ai/logs/training_*.log
```

**Check scheduler logs:**

```bash
tail -n 100 /opt/ml_monster_ai/logs/scheduler.log
```

**Check systemd journal:**

```bash
sudo journalctl -u ml-training-scheduler -n 100
```

### Deployment Fails

**Check deployment logs:**

```bash
tail -n 100 /opt/ml_monster_ai/logs/deployment.log
```

**Manually verify models:**

```bash
ls -lah /opt/ml_monster_ai/models/*/
```

**Check if backup exists:**

```bash
ls -lah /opt/ml_monster_ai/models_backup_*/
```

**Manual rollback:**

```bash
# Find latest backup
LATEST_BACKUP=$(ls -td /opt/ml_monster_ai/models_backup_* | head -1)

# Restore
sudo rm -rf /opt/ml_monster_ai/models
sudo cp -r "$LATEST_BACKUP" /opt/ml_monster_ai/models

# Restart service
sudo systemctl restart ml-inference
```

### Database Connection Issues

**Test MariaDB connection:**

```bash
mysql -h 192.168.0.100 -u ragnarok -p ragnarok -e "SELECT COUNT(*) FROM char;"
```

**Test PostgreSQL connection:**

```bash
psql -h localhost -U ml_user -d ragnarok_ml -c "SELECT version();"
```

**Check configuration:**

```bash
cat /opt/ml_monster_ai/configs/scheduler_config.yaml | grep -A 10 database
```

---

## Advanced Configuration

### Customize Schedule

Edit [`/etc/systemd/system/ml-training-scheduler.timer`](../ml_scheduler/ml-training-scheduler.timer):

```ini
[Timer]
# Run every Sunday at 3:00 AM
OnCalendar=Sun *-*-* 03:00:00

# Run daily at 2:00 AM
# OnCalendar=*-*-* 02:00:00

# Run every Monday and Friday at 1:00 AM
# OnCalendar=Mon,Fri *-*-* 01:00:00
```

After editing:

```bash
sudo systemctl daemon-reload
sudo systemctl restart ml-training-scheduler.timer
```

### Adjust Training Parameters

Edit `/opt/ml_monster_ai/configs/scheduler_config.yaml`:

```yaml
training:
  epochs: 100  # More epochs = better models, longer training
  batch_size: 512  # Larger batch = faster but more VRAM
  archetypes: aggressive defensive  # Train only specific archetypes
```

### Change Off-Peak Window Detection

```yaml
analysis:
  history_days: 60  # Analyze more history for better patterns
  
  # Adjust usage scoring weights
  weight_players: 2.0  # Player count impact (higher = more important)
  weight_cpu: 0.5
  weight_memory: 0.3
  weight_monsters: 0.2
```

---

## Security Considerations

1. **Database Credentials**: Stored in plaintext in config file
   - Restrict file permissions: `chmod 600 /opt/ml_monster_ai/configs/scheduler_config.yaml`
   - Consider using environment variables or secrets management

2. **User Isolation**: Runs as `ml_user` (not root)
   - Limited permissions for security
   - Cannot modify system files

3. **Resource Limits**: Enforced by systemd
   - CPU quota: 80%
   - Memory limit: 12GB
   - Prevents runaway processes

4. **Sudo Requirements**: Deployment script needs sudo for service restart
   - Consider adding specific sudo rules for `ml_user`

---

## Performance Tuning

### Training Speed

- **Increase batch size**: Faster but more VRAM
- **Reduce epochs**: Faster but potentially lower quality
- **Train specific archetypes**: Skip unnecessary models
- **Use FP16**: Already enabled for ONNX export

### Resource Management

- **Lower pause thresholds**: More aggressive pausing
- **Increase check interval**: Less frequent monitoring (default: 30s)
- **Adjust resource limits**: In systemd service file

---

## Files Reference

### Core Components

| File | Purpose |
|------|---------|
| [`scheduler_main.py`](../ml_scheduler/scheduler_main.py) | Main orchestrator |
| [`usage_analyzer.py`](../ml_scheduler/usage_analyzer.py) | Usage pattern analysis |
| [`training_scheduler.py`](../ml_scheduler/training_scheduler.py) | Training management |
| [`scheduler_config.yaml`](../ml_scheduler/scheduler_config.yaml) | Configuration |

### Systemd Files

| File | Purpose |
|------|---------|
| [`ml-training-scheduler.timer`](../ml_scheduler/ml-training-scheduler.timer) | Weekly schedule |
| [`ml-training-scheduler.service`](../ml_scheduler/ml-training-scheduler.service) | Service definition |

### Scripts

| File | Purpose |
|------|---------|
| [`install_scheduler.sh`](../ml_scheduler/install_scheduler.sh) | Install scheduler |
| [`auto_deploy.sh`](../ml_scheduler/auto_deploy.sh) | Deploy models |
| [`trigger_training_now.sh`](../ml_scheduler/trigger_training_now.sh) | Manual trigger |
| [`view_scheduler_status.sh`](../ml_scheduler/view_scheduler_status.sh) | View status |

### Logs

| File | Content |
|------|---------|
| `/opt/ml_monster_ai/logs/scheduler.log` | Scheduler execution |
| `/opt/ml_monster_ai/logs/training_*.log` | Training progress |
| `/opt/ml_monster_ai/logs/deployment.log` | Deployment results |

---

## FAQ

### Q: How often does training run?

**A:** By default, every Sunday at 3:00 AM. Customizable in the timer file.

### Q: What if the server is off at the scheduled time?

**A:** The timer has `Persistent=true`, so it will run on next boot.

### Q: Can I train manually?

**A:** Yes, run [`./trigger_training_now.sh`](../ml_scheduler/trigger_training_now.sh)

### Q: How long does training take?

**A:** Depends on epochs and archetypes. For 50 epochs × 54 models, expect 4-8 hours.

### Q: Will training impact players?

**A:** Minimal. Training only runs during off-peak hours and pauses if server load increases.

### Q: What happens if training fails?

**A:** Models are not deployed. Existing models continue to work. Check logs for errors.

### Q: Can I change the schedule?

**A:** Yes, edit [`/etc/systemd/system/ml-training-scheduler.timer`](../ml_scheduler/ml-training-scheduler.timer) and reload systemd.

### Q: How do I disable automated training?

**A:** `sudo systemctl stop ml-training-scheduler.timer && sudo systemctl disable ml-training-scheduler.timer`

### Q: Where are models deployed?

**A:** `/opt/ml_monster_ai/models/<archetype>/*.onnx`

### Q: How do I verify models are being used?

**A:** Check ml-inference service: `sudo systemctl status ml-inference`

---

## Support

For issues or questions:

1. Check logs first: `./view_scheduler_status.sh`
2. Review this documentation
3. Test with dry-run: `python3 scheduler_main.py --dry-run`
4. Check GitHub issues or create a new one

---

## License

Same as parent project (rAthena/Ragnarok Online Server).

---

## Changelog

### Version 1.0.0 (2026-01-21)

- Initial release
- Usage pattern analysis from game database
- Intelligent off-peak window detection
- Resource monitoring and auto-pause
- Automatic ONNX export and deployment
- Rollback on failure
- Systemd timer integration
- Comprehensive logging and monitoring
