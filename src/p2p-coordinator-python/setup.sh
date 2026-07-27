#!/usr/bin/env bash
# Setup P2P Coordinator on any fresh machine
# Run from the rathena-AI-world root directory

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COORD_DIR="$SCRIPT_DIR/src/p2p-coordinator-python"

echo "=== P2P Coordinator Setup ==="
echo ""

# 1. Install Python dependencies
echo "[1/3] Installing Python dependencies..."
cd "$COORD_DIR"
pip install -r requirements.txt 2>&1 | tail -1
echo "  ✓ Dependencies installed"

# 2. Install systemd service (user-level, no sudo needed)
echo "[2/3] Installing systemd service..."
mkdir -p "$HOME/.config/systemd/user"
sed "s|%h|$HOME|g" "$COORD_DIR/p2p-coordinator.service" > "$HOME/.config/systemd/user/p2p-coordinator.service"
systemctl --user daemon-reload
echo "  ✓ Service installed"

# 3. Enable and start
echo "[3/3] Starting service..."
systemctl --user enable p2p-coordinator.service
systemctl --user restart p2p-coordinator.service
sleep 2

# Verify
if systemctl --user is-active --quiet p2p-coordinator.service; then
    echo "  ✓ P2P Coordinator is running on port 8001"
    echo ""
    echo "=== Setup Complete ==="
    echo "Test: curl -X POST http://localhost:8001/api/v1/auth/token \\"
    echo "  -H 'Content-Type: application/json' \\"
    echo "  -d '{\"peer_id\":\"test\",\"client_version\":\"1.0.0\"}'"
else
    echo "  ✗ Service failed to start. Check: systemctl --user status p2p-coordinator.service"
    exit 1
fi
