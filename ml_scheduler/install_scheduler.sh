#!/bin/bash
# ML Training Scheduler Installation Script
# Installs systemd timer, service, and configuration

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[$(date +'%H:%M:%S')]${NC} $1"
}

log_error() {
    echo -e "${RED}[$(date +'%H:%M:%S')] ERROR:${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[$(date +'%H:%M:%S')] WARNING:${NC} $1"
}

log_info() {
    echo -e "${BLUE}[$(date +'%H:%M:%S')] INFO:${NC} $1"
}

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ML Training Scheduler Installation${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if running as root or with sudo
if [ "$EUID" -ne 0 ]; then
    log_error "This script must be run as root or with sudo"
    log_info "Usage: sudo ./install_scheduler.sh"
    exit 1
fi

# Detect script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
log_info "Script directory: $SCRIPT_DIR"

# Step 1: Install Python dependencies
log "Step 1/8: Installing Python dependencies..."

if [ -f "$SCRIPT_DIR/requirements.txt" ]; then
    pip3 install -r "$SCRIPT_DIR/requirements.txt" || {
        log_warning "Could not install via pip3, trying with venv..."
        
        if [ -d "/opt/ml_monster_ai/venv" ]; then
            /opt/ml_monster_ai/venv/bin/pip install -r "$SCRIPT_DIR/requirements.txt"
        fi
    }
    log "✓ Dependencies installed"
else
    log_warning "requirements.txt not found, skipping"
fi

# Step 2: Create directories
log "Step 2/8: Creating directory structure..."

mkdir -p /opt/ml_monster_ai/configs
mkdir -p /opt/ml_monster_ai/logs
mkdir -p /opt/ml_monster_ai/models
mkdir -p /opt/ml_monster_ai/models_backup

log "✓ Directories created"

# Step 3: Copy configuration
log "Step 3/8: Installing configuration..."

if [ -f "$SCRIPT_DIR/scheduler_config.yaml" ]; then
    cp "$SCRIPT_DIR/scheduler_config.yaml" /opt/ml_monster_ai/configs/
    log "✓ Configuration installed to /opt/ml_monster_ai/configs/"
else
    log_error "scheduler_config.yaml not found in $SCRIPT_DIR"
    exit 1
fi

# Step 4: Make scripts executable
log "Step 4/8: Setting script permissions..."

chmod +x "$SCRIPT_DIR"/*.sh 2>/dev/null || true
chmod +x "$SCRIPT_DIR"/*.py 2>/dev/null || true

log "✓ Scripts made executable"

# Step 5: Install systemd files
log "Step 5/8: Installing systemd files..."

if [ -f "$SCRIPT_DIR/ml-training-scheduler.timer" ]; then
    cp "$SCRIPT_DIR/ml-training-scheduler.timer" /etc/systemd/system/
    log "✓ Timer installed to /etc/systemd/system/"
else
    log_error "ml-training-scheduler.timer not found"
    exit 1
fi

if [ -f "$SCRIPT_DIR/ml-training-scheduler.service" ]; then
    cp "$SCRIPT_DIR/ml-training-scheduler.service" /etc/systemd/system/
    log "✓ Service installed to /etc/systemd/system/"
else
    log_error "ml-training-scheduler.service not found"
    exit 1
fi

# Step 6: Create ml_user if needed
log "Step 6/8: Checking ml_user..."

if ! id "ml_user" &>/dev/null; then
    log_info "Creating ml_user..."
    useradd -r -s /bin/bash -d /opt/ml_monster_ai ml_user
    log "✓ ml_user created"
else
    log "✓ ml_user already exists"
fi

# Set ownership
chown -R ml_user:ml_user /opt/ml_monster_ai
log "✓ Ownership set"

# Step 7: Reload systemd
log "Step 7/8: Reloading systemd..."

systemctl daemon-reload
log "✓ systemd reloaded"

# Step 8: Enable timer
log "Step 8/8: Enabling timer..."

systemctl enable ml-training-scheduler.timer
systemctl start ml-training-scheduler.timer

if systemctl is-active --quiet ml-training-scheduler.timer; then
    log "✓ Timer enabled and started"
else
    log_error "Timer failed to start"
    systemctl status ml-training-scheduler.timer
    exit 1
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ Installation Complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Show next scheduled run
log_info "Next scheduled training run:"
systemctl list-timers ml-training-scheduler.timer

echo ""
log_info "Useful commands:"
echo "  View status:      ./view_scheduler_status.sh"
echo "  Manual trigger:   ./trigger_training_now.sh"
echo "  Test (dry-run):   python3 scheduler_main.py --dry-run"
echo "  View logs:        tail -f /opt/ml_monster_ai/logs/scheduler.log"
echo "  Service journal:  sudo journalctl -u ml-training-scheduler -f"
echo ""
log_info "Configuration file: /opt/ml_monster_ai/configs/scheduler_config.yaml"
echo ""

exit 0
