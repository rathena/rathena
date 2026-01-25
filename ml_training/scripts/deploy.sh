#!/bin/bash
#
# ML Monster AI - Deployment Script
# Enhanced ML Monster AI System v2.0
#
# This script deploys the ML training code from the repository to /opt/ml_monster_ai/
# It creates symbolic links so updates to the repo are reflected in the deployment.

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Directories
REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEPLOY_DIR="/opt/ml_monster_ai"

echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}ML Monster AI - Deployment${NC}"
echo -e "${GREEN}================================${NC}"
echo ""
echo "Repository: $REPO_DIR"
echo "Deployment: $DEPLOY_DIR"
echo ""

# Check if deployment directory exists
if [ ! -d "$DEPLOY_DIR" ]; then
    echo -e "${RED}Error: Deployment directory does not exist: $DEPLOY_DIR${NC}"
    echo "Please create it first: sudo mkdir -p $DEPLOY_DIR && sudo chown $(whoami):$(whoami) $DEPLOY_DIR"
    exit 1
fi

# Check if we have write permission
if [ ! -w "$DEPLOY_DIR" ]; then
    echo -e "${RED}Error: No write permission to $DEPLOY_DIR${NC}"
    echo "Run: sudo chown $(whoami):$(whoami) $DEPLOY_DIR"
    exit 1
fi

# Create directory structure in deployment location
echo -e "${YELLOW}Creating directory structure...${NC}"
mkdir -p "$DEPLOY_DIR"/{models,data,logs,configs}
mkdir -p "$DEPLOY_DIR"/models/{aggressive,defensive,support,mage,tank,ranged}
mkdir -p "$DEPLOY_DIR"/data/{replay_buffer,checkpoints}
mkdir -p "$DEPLOY_DIR"/logs/{tensorboard,wandb}

echo -e "${GREEN}✓${NC} Directory structure created"

# Create symbolic links for source code
echo -e "${YELLOW}Creating symbolic links...${NC}"

# Remove old links if they exist
rm -f "$DEPLOY_DIR"/src 2>/dev/null || true

# Link source directories
ln -sfn "$REPO_DIR/models" "$DEPLOY_DIR/src_models"
ln -sfn "$REPO_DIR/training" "$DEPLOY_DIR/src_training"
ln -sfn "$REPO_DIR/data" "$DEPLOY_DIR/src_data"
ln -sfn "$REPO_DIR/utils" "$DEPLOY_DIR/src_utils"
ln -sfn "$REPO_DIR/scripts" "$DEPLOY_DIR/src_scripts"

echo -e "${GREEN}✓${NC} Source code linked"

# Link configuration files
echo -e "${YELLOW}Linking configuration files...${NC}"

if [ -f "$REPO_DIR/config/training_config.yaml" ]; then
    ln -sfn "$REPO_DIR/config/training_config.yaml" "$DEPLOY_DIR/configs/training_config.yaml"
    echo -e "${GREEN}✓${NC} training_config.yaml linked"
fi

if [ -f "$REPO_DIR/config/model_params.yaml" ]; then
    ln -sfn "$REPO_DIR/config/model_params.yaml" "$DEPLOY_DIR/configs/model_params.yaml"
    echo -e "${GREEN}✓${NC} model_params.yaml linked"
fi

# Copy virtual environment if it doesn't exist
echo -e "${YELLOW}Checking Python environment...${NC}"

if [ ! -d "$DEPLOY_DIR/venv" ]; then
    echo -e "${YELLOW}Creating virtual environment...${NC}"
    python3.11 -m venv "$DEPLOY_DIR/venv"
    source "$DEPLOY_DIR/venv/bin/activate"
    
    # Install requirements
    if [ -f "$REPO_DIR/../requirements.txt" ]; then
        pip install -r "$REPO_DIR/../requirements.txt"
    else
        echo -e "${YELLOW}Warning: requirements.txt not found, install manually${NC}"
    fi
    
    deactivate
    echo -e "${GREEN}✓${NC} Virtual environment created"
else
    echo -e "${GREEN}✓${NC} Virtual environment exists"
fi

# Create convenience scripts
echo -e "${YELLOW}Creating convenience scripts...${NC}"

# Training script
cat > "$DEPLOY_DIR/train.sh" << 'EOF'
#!/bin/bash
cd /opt/ml_monster_ai
source venv/bin/activate
python src_scripts/train_all_models.py "$@"
EOF
chmod +x "$DEPLOY_DIR/train.sh"

# Export script
cat > "$DEPLOY_DIR/export.sh" << 'EOF'
#!/bin/bash
cd /opt/ml_monster_ai
source venv/bin/activate
python src_scripts/export_to_onnx.py "$@"
EOF
chmod +x "$DEPLOY_DIR/export.sh"

# Data collection script
cat > "$DEPLOY_DIR/collect_data.sh" << 'EOF'
#!/bin/bash
cd /opt/ml_monster_ai
source venv/bin/activate
python src_scripts/collect_initial_data.py "$@"
EOF
chmod +x "$DEPLOY_DIR/collect_data.sh"

echo -e "${GREEN}✓${NC} Convenience scripts created"

# Create Python path file
echo -e "${YELLOW}Configuring Python paths...${NC}"

cat > "$DEPLOY_DIR/setup_paths.py" << EOF
import sys
from pathlib import Path

# Add source directories to Python path
base_dir = Path('$DEPLOY_DIR')
sys.path.insert(0, str(base_dir / 'src_models'))
sys.path.insert(0, str(base_dir / 'src_training'))
sys.path.insert(0, str(base_dir / 'src_data'))
sys.path.insert(0, str(base_dir / 'src_utils'))
EOF

echo -e "${GREEN}✓${NC} Python paths configured"

# Set permissions
echo -e "${YELLOW}Setting permissions...${NC}"
chmod -R u+rw "$DEPLOY_DIR"
chmod -R g+r "$DEPLOY_DIR"

echo -e "${GREEN}✓${NC} Permissions set"

# Verify deployment
echo ""
echo -e "${YELLOW}Verifying deployment...${NC}"

# Check symbolic links
if [ -L "$DEPLOY_DIR/src_models" ]; then
    echo -e "${GREEN}✓${NC} src_models linked"
else
    echo -e "${RED}✗${NC} src_models not linked"
fi

if [ -L "$DEPLOY_DIR/src_training" ]; then
    echo -e "${GREEN}✓${NC} src_training linked"
else
    echo -e "${RED}✗${NC} src_training not linked"
fi

# Check Python environment
if [ -f "$DEPLOY_DIR/venv/bin/python" ]; then
    echo -e "${GREEN}✓${NC} Python environment exists"
    PYTHON_VERSION=$("$DEPLOY_DIR/venv/bin/python" --version 2>&1)
    echo "  $PYTHON_VERSION"
else
    echo -e "${RED}✗${NC} Python environment not found"
fi

# Display summary
echo ""
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Deployment Complete!${NC}"
echo -e "${GREEN}================================${NC}"
echo ""
echo "Deployment directory: $DEPLOY_DIR"
echo "Repository directory: $REPO_DIR"
echo ""
echo "Quick commands:"
echo "  Train all models:    $DEPLOY_DIR/train.sh --all --epochs 100"
echo "  Export to ONNX:      $DEPLOY_DIR/export.sh --all --verify"
echo "  Collect data:        $DEPLOY_DIR/collect_data.sh --all"
echo ""
echo "Activate environment: source $DEPLOY_DIR/venv/bin/activate"
echo ""
echo -e "${YELLOW}Note: All source code changes in the repository will be reflected in deployment${NC}"
echo ""
