# ML Training Scheduler

Automated ML training scheduler with usage pattern analysis for Ragnarok Online ML Monster AI.

## Quick Start

```bash
# Install the scheduler
sudo ./install_scheduler.sh

# View status
./view_scheduler_status.sh

# Test without training (dry-run)
python3 scheduler_main.py --dry-run

# Manual trigger
./trigger_training_now.sh
```

## What It Does

The scheduler automatically:
1. **Analyzes** 30 days of player activity to find off-peak hours
2. **Schedules** training during low-usage windows (default: Sunday 3 AM)
3. **Monitors** system resources during training (CPU, Memory, GPU)
4. **Pauses** training if server load spikes
5. **Exports** trained models to ONNX format
6. **Deploys** models automatically
7. **Rolls back** if deployment fails

## Files

| File | Description |
|------|-------------|
| `scheduler_main.py` | Main orchestrator |
| `usage_analyzer.py` | Analyzes player activity patterns |
| `training_scheduler.py` | Manages training lifecycle |
| `scheduler_config.yaml` | Configuration file |
| `ml-training-scheduler.timer` | Systemd timer (weekly) |
| `ml-training-scheduler.service` | Systemd service |
| `auto_deploy.sh` | Automated deployment script |
| `install_scheduler.sh` | Installation script |
| `trigger_training_now.sh` | Manual trigger script |
| `view_scheduler_status.sh` | Status checker |
| `requirements.txt` | Python dependencies |

## Installation

```bash
cd /home/lot399/RagnarokOnlineServer/rathena-AI-world/ml_scheduler
sudo ./install_scheduler.sh
```

The installer will:
- Install Python dependencies
- Create directories in `/opt/ml_monster_ai/`
- Copy configuration
- Install systemd timer and service
- Enable automatic weekly training

## Configuration

Edit `/opt/ml_monster_ai/configs/scheduler_config.yaml`:

```yaml
training:
  epochs: 50                      # Training epochs
  auto_deploy: true               # Auto-deploy after training
  max_cpu_before_training: 30     # Don't start if CPU > 30%
  pause_threshold_cpu: 80         # Pause if CPU > 80%

scheduling:
  window_hours: 8                 # 8-hour training window
  force_training: false           # Skip time checks (testing only)

analysis:
  history_days: 30                # Days of history to analyze
```

## Usage

### Automatic (Default)

The scheduler runs every Sunday at 3:00 AM via systemd timer.

Check next run:
```bash
systemctl list-timers ml-training-scheduler
```

### Manual Trigger

```bash
./trigger_training_now.sh
```

### Dry-Run (Testing)

```bash
python3 scheduler_main.py --dry-run
```

Shows usage patterns and off-peak windows without training.

## Monitoring

### View Status

```bash
./view_scheduler_status.sh
```

### Live Logs

```bash
# Scheduler log
tail -f /opt/ml_monster_ai/logs/scheduler.log

# Training log
tail -f /opt/ml_monster_ai/logs/training_*.log

# Systemd journal
sudo journalctl -u ml-training-scheduler -f
```

## How It Works

1. **Timer triggers** service (weekly)
2. **Analyzer queries** MariaDB for player activity
3. **Off-peak windows** identified (e.g., Sunday 2-10 AM)
4. **Resource check**: CPU, Memory, GPU availability
5. **Training starts** if conditions met
6. **Monitor** resources every 30 seconds
7. **Pause/Resume** based on thresholds
8. **Export** to ONNX on success
9. **Backup** existing models
10. **Deploy** new models
11. **Restart** ml-inference service
12. **Verify** deployment success

## Resource Management

The scheduler:
- **Defers training** if CPU > 30% or Memory > 70%
- **Pauses training** if CPU > 80% or Memory > 90%
- **Resumes automatically** when resources recover
- **Never blocks** player activity

## Troubleshooting

### Training Not Running

```bash
# Check if timer is active
systemctl status ml-training-scheduler.timer

# Check if current time is off-peak
python3 scheduler_main.py --dry-run

# Force training (testing only)
# Edit config: force_training: true
./trigger_training_now.sh
```

### Deployment Failed

```bash
# Check deployment log
tail -n 100 /opt/ml_monster_ai/logs/deployment.log

# Manual rollback
LATEST_BACKUP=$(ls -td /opt/ml_monster_ai/models_backup_* | head -1)
sudo rm -rf /opt/ml_monster_ai/models
sudo cp -r "$LATEST_BACKUP" /opt/ml_monster_ai/models
sudo systemctl restart ml-inference
```

### Database Connection Error

```bash
# Test MariaDB
mysql -h 192.168.0.100 -u ragnarok -p ragnarok -e "SELECT COUNT(*) FROM char;"

# Update config if needed
sudo nano /opt/ml_monster_ai/configs/scheduler_config.yaml
```

## Customization

### Change Schedule

Edit `/etc/systemd/system/ml-training-scheduler.timer`:

```ini
[Timer]
# Daily at 2 AM
OnCalendar=*-*-* 02:00:00

# Monday and Friday at 1 AM
# OnCalendar=Mon,Fri *-*-* 01:00:00
```

Then:
```bash
sudo systemctl daemon-reload
sudo systemctl restart ml-training-scheduler.timer
```

### Train Specific Archetypes

Edit config:
```yaml
training:
  archetypes: aggressive defensive  # Instead of 'all'
```

### Adjust Resource Limits

Edit config:
```yaml
training:
  max_cpu_before_training: 20   # More conservative
  pause_threshold_cpu: 70        # Pause earlier
```

## Directory Structure

```
/opt/ml_monster_ai/
├── configs/
│   └── scheduler_config.yaml          # Main configuration
├── logs/
│   ├── scheduler.log                  # Scheduler logs
│   ├── training_YYYYMMDD_HHMMSS.log  # Training logs
│   └── deployment.log                 # Deployment logs
├── models/                            # Deployed models
│   ├── aggressive/*.onnx
│   ├── defensive/*.onnx
│   └── ...
└── models_backup_*/                   # Backup models
```

## Dependencies

- Python 3.8+
- MariaDB (game database)
- PostgreSQL (optional, for ML metrics)
- CUDA GPU (for training)
- systemd (Linux)

Python packages (installed via `requirements.txt`):
- aiomysql
- asyncpg
- pandas
- numpy
- PyYAML
- psutil

## Documentation

See [`docs/AUTOMATED_TRAINING.md`](../docs/AUTOMATED_TRAINING.md) for comprehensive documentation.

## Support

1. Check logs: `./view_scheduler_status.sh`
2. Test with dry-run: `python3 scheduler_main.py --dry-run`
3. Review documentation
4. Check systemd journal: `sudo journalctl -u ml-training-scheduler -n 50`

## License

Same as parent project (rAthena).
