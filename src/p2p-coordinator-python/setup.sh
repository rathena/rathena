#!/usr/bin/env bash
# Setup P2P Coordinator on any fresh machine
# Run from the rathena-AI-world root directory

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COORD_DIR="$SCRIPT_DIR/src/p2p-coordinator-python"

echo "=== P2P Coordinator Setup ==="
echo ""

# 1. Install Python dependencies
echo "[1/4] Installing Python dependencies..."
cd "$COORD_DIR"
pip install -r requirements.txt 2>&1 | tail -1
echo "  ✓ Dependencies installed"

# 2. Generate JWT secret if not already set
echo "[2/4] Configuring JWT secret..."
SECRET_FILE="$HOME/.p2p_coordinator_secret"
if [ ! -f "$SECRET_FILE" ]; then
    python3 -c "import secrets; print(secrets.token_hex(32))" > "$SECRET_FILE"
    chmod 600 "$SECRET_FILE"
    echo "  ✓ Generated JWT secret: $SECRET_FILE"
else
    echo "  ✓ JWT secret already exists: $SECRET_FILE"
fi

# 3. Install systemd service (user-level, no sudo needed)
echo "[3/4] Installing systemd service..."
mkdir -p "$HOME/.config/systemd/user"
JWT_SECRET=$(cat "$SECRET_FILE")
sed -e "s|%h|$HOME|g" \
    -e "/Environment=P2P_JWT_SECRET/d" \
    "$COORD_DIR/p2p-coordinator.service" > "$HOME/.config/systemd/user/p2p-coordinator.service"
# Inject JWT secret
echo "Environment=P2P_JWT_SECRET=$JWT_SECRET" >> "$HOME/.config/systemd/user/p2p-coordinator.service"
systemctl --user daemon-reload
echo "  ✓ Service installed"

# 4. Enable and start
echo "[4/4] Starting service..."
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
    journalctl --user -u p2p-coordinator.service --no-pager -n 20
    exit 1
fi
