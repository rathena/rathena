#!/bin/bash
# Deploy ML Inference Service from workspace to /opt/ml_monster_ai
# This script copies source files and configurations to production location

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR"
TARGET_DIR="/opt/ml_monster_ai/inference_service"
CONFIG_DIR="/opt/ml_monster_ai/configs"

echo "=========================================="
echo "ML Inference Service Deployment"
echo "=========================================="
echo "Source: $SOURCE_DIR"
echo "Target: $TARGET_DIR"
echo ""

# Check if running as correct user
if [ "$USER" != "lot399" ] && [ "$USER" != "root" ]; then
    echo "Warning: Should run as lot399 or root"
fi

# Create target directories
echo "[1/5] Creating target directories..."
sudo mkdir -p "$TARGET_DIR"
sudo mkdir -p "$CONFIG_DIR"
sudo mkdir -p "/opt/ml_monster_ai/logs"
sudo mkdir -p "/opt/ml_monster_ai/scripts"

# Copy Python source files
echo "[2/5] Copying Python source files..."
sudo cp "$SOURCE_DIR"/*.py "$TARGET_DIR/"
sudo chmod 644 "$TARGET_DIR"/*.py
sudo chmod 755 "$TARGET_DIR"/main.py  # Main entry point executable

# Copy configuration
echo "[3/5] Copying configuration..."
sudo cp "$SOURCE_DIR"/inference_config.yaml "$CONFIG_DIR/"
sudo chmod 644 "$CONFIG_DIR"/inference_config.yaml

# Set ownership
echo "[4/5] Setting ownership..."
sudo chown -R lot399:lot399 /opt/ml_monster_ai/inference_service
sudo chown -R lot399:lot399 /opt/ml_monster_ai/configs
sudo chown -R lot399:lot399 /opt/ml_monster_ai/logs

# Verify deployment
echo "[5/5] Verifying deployment..."
if [ -f "$TARGET_DIR/main.py" ]; then
    echo "✓ main.py deployed"
else
    echo "✗ main.py missing"
    exit 1
fi

if [ -f "$CONFIG_DIR/inference_config.yaml" ]; then
    echo "✓ inference_config.yaml deployed"
else
    echo "✗ inference_config.yaml missing"
    exit 1
fi

# List deployed files
echo ""
echo "Deployed files:"
ls -lh "$TARGET_DIR"/*.py
echo ""
ls -lh "$CONFIG_DIR"/*.yaml

echo ""
echo "=========================================="
echo "✓ Deployment complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "  1. Set environment variables (ML_DB_PASSWORD)"
echo "  2. Install systemd service: sudo cp ml-inference.service /etc/systemd/system/"
echo "  3. Enable service: sudo systemctl enable ml-inference"
echo "  4. Start service: sudo systemctl start ml-inference"
echo "  5. Check status: sudo systemctl status ml-inference"
echo "  6. View logs: sudo journalctl -u ml-inference -f"
echo ""
