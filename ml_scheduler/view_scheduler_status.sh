#!/bin/bash
# View ML Training Scheduler Status
# Shows timer status, service status, and next scheduled run

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ML Training Scheduler Status${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check Timer Status
echo -e "${CYAN}[1] Timer Status${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"

if systemctl list-timers ml-training-scheduler.timer &> /dev/null; then
    systemctl list-timers ml-training-scheduler.timer --all
    
    if systemctl is-active --quiet ml-training-scheduler.timer; then
        echo -e "${GREEN}✓ Timer is ACTIVE${NC}"
    else
        echo -e "${YELLOW}⚠ Timer is INACTIVE${NC}"
    fi
else
    echo -e "${RED}✗ Timer not found (not installed)${NC}"
fi

echo ""

# Check Service Status
echo -e "${CYAN}[2] Service Status${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"

if systemctl list-unit-files | grep -q "ml-training-scheduler.service"; then
    systemctl status ml-training-scheduler.service --no-pager -l || true
else
    echo -e "${RED}✗ Service not found (not installed)${NC}"
fi

echo ""

# Check Configuration
echo -e "${CYAN}[3] Configuration${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"

CONFIG_FILE="/opt/ml_monster_ai/configs/scheduler_config.yaml"

if [ -f "$CONFIG_FILE" ]; then
    echo -e "${GREEN}✓ Configuration file exists: $CONFIG_FILE${NC}"
    
    # Extract key settings
    if command -v yq &> /dev/null || command -v python3 &> /dev/null; then
        echo ""
        echo "Key Settings:"
        
        # Try with python since yq might not be installed
        python3 -c "
import yaml
try:
    with open('$CONFIG_FILE') as f:
        config = yaml.safe_load(f)
    
    print(f\"  Training Epochs: {config.get('training', {}).get('epochs', 'N/A')}\")
    print(f\"  Archetypes: {config.get('training', {}).get('archetypes', 'N/A')}\")
    print(f\"  Auto-Deploy: {config.get('training', {}).get('auto_deploy', 'N/A')}\")
    print(f\"  History Days: {config.get('analysis', {}).get('history_days', 'N/A')}\")
    print(f\"  Force Training: {config.get('scheduling', {}).get('force_training', 'N/A')}\")
except Exception as e:
    print(f\"  Error reading config: {e}\")
" 2>/dev/null || echo "  (Could not parse config)"
    fi
else
    echo -e "${RED}✗ Configuration file not found: $CONFIG_FILE${NC}"
fi

echo ""

# Check Logs
echo -e "${CYAN}[4] Recent Logs${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"

LOG_FILE="/opt/ml_monster_ai/logs/scheduler.log"

if [ -f "$LOG_FILE" ]; then
    echo -e "${GREEN}✓ Log file: $LOG_FILE${NC}"
    echo ""
    echo "Last 10 lines:"
    tail -n 10 "$LOG_FILE" | sed 's/^/  /'
else
    echo -e "${YELLOW}⚠ No log file found yet${NC}"
fi

echo ""

# Check Models Directory
echo -e "${CYAN}[5] Deployed Models${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"

MODELS_DIR="/opt/ml_monster_ai/models"

if [ -d "$MODELS_DIR" ]; then
    MODEL_COUNT=$(find "$MODELS_DIR" -name "*.onnx" 2>/dev/null | wc -l)
    
    if [ "$MODEL_COUNT" -gt 0 ]; then
        echo -e "${GREEN}✓ $MODEL_COUNT ONNX models deployed${NC}"
        
        # Show models by archetype
        for archetype in aggressive defensive support mage tank ranged; do
            arch_count=$(find "$MODELS_DIR/$archetype" -name "*.onnx" 2>/dev/null | wc -l)
            if [ "$arch_count" -gt 0 ]; then
                echo "  - $archetype: $arch_count models"
            fi
        done
    else
        echo -e "${YELLOW}⚠ No models deployed yet${NC}"
    fi
else
    echo -e "${YELLOW}⚠ Models directory not found${NC}"
fi

echo ""

# Commands
echo -e "${CYAN}[6] Useful Commands${NC}"
echo -e "${CYAN}───────────────────────────────────${NC}"
echo "Monitor scheduler logs:"
echo "  tail -f /opt/ml_monster_ai/logs/scheduler.log"
echo ""
echo "Monitor training logs:"
echo "  tail -f /opt/ml_monster_ai/logs/training_*.log"
echo ""
echo "View service journal:"
echo "  sudo journalctl -u ml-training-scheduler -f"
echo ""
echo "Manually trigger training:"
echo "  ./trigger_training_now.sh"
echo ""
echo "Test scheduler (dry-run):"
echo "  python3 scheduler_main.py --dry-run"
echo ""

echo -e "${BLUE}========================================${NC}"
