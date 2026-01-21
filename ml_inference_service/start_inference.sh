#!/bin/bash
# Start ML Inference Service
# Can run standalone or via systemd

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICE_DIR="/opt/ml_monster_ai/inference_service"
VENV_DIR="/opt/ml_monster_ai/venv"
CONFIG_FILE="/opt/ml_monster_ai/configs/inference_config.yaml"

echo "=========================================="
echo "Starting ML Inference Service"
echo "=========================================="

# Check if service directory exists
if [ ! -d "$SERVICE_DIR" ]; then
    echo "Error: Service directory not found at $SERVICE_DIR"
    echo "Please run deploy.sh first"
    exit 1
fi

# Check if virtual environment exists
if [ ! -d "$VENV_DIR" ]; then
    echo "Error: Python virtual environment not found at $VENV_DIR"
    echo "Please create venv and install dependencies first"
    exit 1
fi

# Check if config exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Warning: Config file not found at $CONFIG_FILE"
    echo "Service will create default configuration"
fi

# Check if already running via systemd
if systemctl is-active --quiet ml-inference 2>/dev/null; then
    echo "Service is already running via systemd"
    echo "Status:"
    sudo systemctl status ml-inference --no-pager
    exit 0
fi

# Check for required environment variables
if [ -z "$ML_DB_PASSWORD" ]; then
    echo "Warning: ML_DB_PASSWORD not set, will use empty password"
fi

# Activate virtual environment and run
echo "Activating Python environment: $VENV_DIR"
source "$VENV_DIR/bin/activate"

echo "Starting inference service..."
echo "Service directory: $SERVICE_DIR"
echo "Config file: $CONFIG_FILE"
echo "Press Ctrl+C to stop"
echo ""

# Run service
cd "$SERVICE_DIR"
python main.py

# Deactivate on exit
deactivate
