#!/bin/bash
# Automated Model Deployment Script
# Deploys trained models after successful training

set -e  # Exit on error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
MODELS_SRC="/opt/ml_monster_ai/ml_training/models"
MODELS_DST="/opt/ml_monster_ai/models"
BACKUP_BASE="/opt/ml_monster_ai/models_backup"
LOG_FILE="/opt/ml_monster_ai/logs/deployment.log"

# Logging function
log() {
    echo -e "${GREEN}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}[$(date +'%Y-%m-%d %H:%M:%S')] ERROR:${NC} $1" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "${YELLOW}[$(date +'%Y-%m-%d %H:%M:%S')] WARNING:${NC} $1" | tee -a "$LOG_FILE"
}

log_info() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')] INFO:${NC} $1" | tee -a "$LOG_FILE"
}

# Create log directory
mkdir -p "$(dirname "$LOG_FILE")"

log "========================================"
log "ML Model Automated Deployment"
log "========================================"

# Step 1: Verify source models exist
log_info "Step 1/6: Verifying source models..."

if [ ! -d "$MODELS_SRC" ]; then
    log_error "Source models directory not found: $MODELS_SRC"
    exit 1
fi

# Count ONNX models
ONNX_COUNT=$(find "$MODELS_SRC" -name "*.onnx" 2>/dev/null | wc -l)

if [ "$ONNX_COUNT" -eq 0 ]; then
    log_error "No ONNX models found in $MODELS_SRC"
    log_info "Did training complete successfully? Check training logs."
    exit 1
fi

log "✓ Found $ONNX_COUNT ONNX models in source directory"

# Step 2: Backup existing models
log_info "Step 2/6: Backing up existing models..."

if [ -d "$MODELS_DST" ]; then
    BACKUP_DIR="${BACKUP_BASE}_$(date +%Y%m%d_%H%M%S)"
    
    log_info "Creating backup: $BACKUP_DIR"
    cp -r "$MODELS_DST" "$BACKUP_DIR"
    
    if [ $? -eq 0 ]; then
        BACKUP_SIZE=$(du -sh "$BACKUP_DIR" | cut -f1)
        log "✓ Backup created successfully: $BACKUP_SIZE"
        echo "$BACKUP_DIR" > /tmp/ml_last_backup.txt
    else
        log_warning "Backup failed, but continuing with deployment..."
    fi
else
    log_info "No existing models to backup (first deployment)"
fi

# Step 3: Deploy new models
log_info "Step 3/6: Deploying new models..."

# Create destination directory structure
mkdir -p "$MODELS_DST"

# Copy models by archetype
ARCHETYPES=("aggressive" "defensive" "support" "mage" "tank" "ranged")
DEPLOYED_COUNT=0

for archetype in "${ARCHETYPES[@]}"; do
    SRC_DIR="$MODELS_SRC/$archetype"
    DST_DIR="$MODELS_DST/$archetype"
    
    if [ -d "$SRC_DIR" ]; then
        mkdir -p "$DST_DIR"
        
        # Copy ONNX files
        ARCH_COUNT=$(find "$SRC_DIR" -name "*.onnx" 2>/dev/null | wc -l)
        
        if [ "$ARCH_COUNT" -gt 0 ]; then
            cp "$SRC_DIR"/*.onnx "$DST_DIR/" 2>/dev/null || true
            log_info "  $archetype: $ARCH_COUNT models"
            DEPLOYED_COUNT=$((DEPLOYED_COUNT + ARCH_COUNT))
        fi
    fi
done

if [ "$DEPLOYED_COUNT" -eq 0 ]; then
    log_error "No models were deployed!"
    exit 1
fi

log "✓ Deployed $DEPLOYED_COUNT models"

# Step 4: Set permissions
log_info "Step 4/6: Setting permissions..."

chown -R ml_user:ml_user "$MODELS_DST" 2>/dev/null || {
    log_warning "Could not set ownership (may need sudo)"
}

chmod -R 755 "$MODELS_DST"
log "✓ Permissions set"

# Step 5: Verify deployment
log_info "Step 5/6: Verifying deployment..."

VERIFY_COUNT=$(find "$MODELS_DST" -name "*.onnx" | wc -l)

if [ "$VERIFY_COUNT" -eq 0 ]; then
    log_error "Verification failed: No models found in destination"
    
    # Attempt rollback
    if [ -f /tmp/ml_last_backup.txt ]; then
        LAST_BACKUP=$(cat /tmp/ml_last_backup.txt)
        log_warning "Attempting rollback to: $LAST_BACKUP"
        
        rm -rf "$MODELS_DST"
        cp -r "$LAST_BACKUP" "$MODELS_DST"
        
        log_warning "Rollback complete"
    fi
    
    exit 1
fi

log "✓ Verification passed: $VERIFY_COUNT models in destination"

# Step 6: Restart inference service
log_info "Step 6/6: Restarting ML inference service..."

# Check if service exists
if systemctl list-unit-files | grep -q "ml-inference.service"; then
    log_info "Restarting ml-inference service..."
    
    # Try with sudo first, then without
    if sudo systemctl restart ml-inference 2>/dev/null; then
        log "✓ Service restarted with sudo"
    elif systemctl restart ml-inference 2>/dev/null; then
        log "✓ Service restarted"
    else
        log_warning "Could not restart service automatically"
        log_info "Run manually: sudo systemctl restart ml-inference"
    fi
    
    # Wait for service to start
    sleep 5
    
    # Verify service is running
    if systemctl is-active --quiet ml-inference; then
        log "✓ ML inference service is active"
        
        # Check metrics if available
        if command -v curl &> /dev/null; then
            LOADED_MODELS=$(curl -s http://localhost:9090/metrics 2>/dev/null | grep -oP 'ml_models_loaded_total \K\d+' || echo "unknown")
            
            if [ "$LOADED_MODELS" != "unknown" ]; then
                log_info "Loaded models count: $LOADED_MODELS"
            fi
        fi
    else
        log_error "ML inference service failed to start!"
        log_info "Check logs: sudo journalctl -u ml-inference -n 50"
        
        # Attempt rollback
        if [ -f /tmp/ml_last_backup.txt ]; then
            LAST_BACKUP=$(cat /tmp/ml_last_backup.txt)
            log_warning "Attempting rollback due to service failure..."
            
            rm -rf "$MODELS_DST"
            cp -r "$LAST_BACKUP" "$MODELS_DST"
            
            sudo systemctl restart ml-inference 2>/dev/null || systemctl restart ml-inference 2>/dev/null
            
            log_warning "Rollback complete, service restarted with old models"
        fi
        
        exit 1
    fi
else
    log_warning "ml-inference service not found, skipping restart"
    log_info "Service may not be installed yet"
fi

# Success summary
log "========================================"
log "✓ DEPLOYMENT COMPLETED SUCCESSFULLY"
log "========================================"
log "Models deployed: $DEPLOYED_COUNT"
log "Destination: $MODELS_DST"

if [ -f /tmp/ml_last_backup.txt ]; then
    log "Backup location: $(cat /tmp/ml_last_backup.txt)"
fi

log "========================================"

# Cleanup old backups (keep last 5)
log_info "Cleaning up old backups..."

BACKUP_COUNT=$(find "$BACKUP_BASE"_* -maxdepth 0 -type d 2>/dev/null | wc -l)

if [ "$BACKUP_COUNT" -gt 5 ]; then
    REMOVE_COUNT=$((BACKUP_COUNT - 5))
    log_info "Removing $REMOVE_COUNT old backup(s)..."
    
    find "$BACKUP_BASE"_* -maxdepth 0 -type d 2>/dev/null | sort | head -n "$REMOVE_COUNT" | xargs rm -rf
    
    log "✓ Old backups cleaned up"
fi

exit 0
