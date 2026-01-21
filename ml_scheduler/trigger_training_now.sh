#!/bin/bash
# Manual Training Trigger Script
# Bypasses scheduling and starts training immediately

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ML Training Manual Trigger${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if user is sure
echo -e "${YELLOW}⚠️  This will start ML training immediately!${NC}"
echo -e "${YELLOW}Training may take several hours and consume significant resources.${NC}"
echo ""
read -p "Are you sure you want to proceed? (yes/no): " confirm

if [ "$confirm" != "yes" ]; then
    echo -e "${RED}Aborted.${NC}"
    exit 0
fi

echo ""
echo -e "${GREEN}Starting ML training scheduler service...${NC}"
echo ""

# Start the training service
sudo systemctl start ml-training-scheduler.service

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Training service started${NC}"
    echo ""
    echo -e "${BLUE}Monitor progress with:${NC}"
    echo "  sudo journalctl -u ml-training-scheduler -f"
    echo ""
    echo -e "${BLUE}Check training logs:${NC}"
    echo "  tail -f /opt/ml_monster_ai/logs/scheduler.log"
    echo "  tail -f /opt/ml_monster_ai/logs/training_*.log"
    echo ""
    echo -e "${BLUE}Check service status:${NC}"
    echo "  sudo systemctl status ml-training-scheduler"
    echo ""
else
    echo -e "${RED}✗ Failed to start training service${NC}"
    echo ""
    echo -e "${BLUE}Check service status:${NC}"
    echo "  sudo systemctl status ml-training-scheduler"
    echo ""
    echo -e "${BLUE}Check logs:${NC}"
    echo "  sudo journalctl -u ml-training-scheduler -n 50"
    exit 1
fi
