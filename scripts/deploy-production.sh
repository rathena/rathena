#!/bin/bash
###############################################################################
# Production Deployment Script
# Version: 1.0.0
# Description: Automated deployment script for AI Autonomous World system
###############################################################################

set -euo pipefail  # Exit on error, undefined vars, pipe failures

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
INSTALL_DIR="/opt/ai-mmorpg/ai-mmorpg-world"
BACKUP_DIR="/opt/backups/$(date +%Y%m%d)"
LOG_FILE="/var/log/deployment_$(date +%Y%m%d_%H%M%S).log"
AI_SERVICE_DIR="$INSTALL_DIR/rathena-AI-world/ai-autonomous-world/ai-service"
RATHENA_DIR="$INSTALL_DIR/rathena-AI-world"
DASHBOARD_DIR="$INSTALL_DIR/rathena-AI-world/dashboard"
DB_HOST="192.168.0.100"
DB_NAME="ai_world_memory"
DB_USER="ai_world_user"

# Functions
log() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$LOG_FILE"
}

success() {
    echo -e "${GREEN}✓${NC} $1" | tee -a "$LOG_FILE"
}

error() {
    echo -e "${RED}✗${NC} $1" | tee -a "$LOG_FILE"
}

warning() {
    echo -e "${YELLOW}⚠${NC} $1" | tee -a "$LOG_FILE"
}

confirm() {
    read -p "$(echo -e ${YELLOW}$1${NC}) (yes/no): " response
    case "$response" in
        yes|YES|y|Y) return 0 ;;
        *) return 1 ;;
    esac
}

check_prerequisites() {
    log "Checking prerequisites..."
    
    # Check if running as correct user
    if [ "$EUID" -eq 0 ]; then 
        error "Do not run as root. Use a service account."
        exit 1
    fi
    
    # Check required commands
    local required_cmds=("psql" "redis-cli" "curl" "git" "systemctl" "pm2")
    for cmd in "${required_cmds[@]}"; do
        if ! command -v "$cmd" &> /dev/null; then
            error "Required command not found: $cmd"
            exit 1
        fi
    done
    
    # Check if directories exist
    if [ ! -d "$INSTALL_DIR" ]; then
        error "Installation directory not found: $INSTALL_DIR"
        exit 1
    fi
    
    success "Prerequisites check passed"
}

pre_deployment_backup() {
    log "Creating pre-deployment backup..."
    
    mkdir -p "$BACKUP_DIR"
    
    # Backup database
    log "Backing up PostgreSQL database..."
    PGPASSWORD="${DB_PASSWORD}" pg_dump -h "$DB_HOST" -U "$DB_USER" "$DB_NAME" | \
        gzip > "$BACKUP_DIR/pre_deployment.sql.gz"
    
    if [ $? -eq 0 ]; then
        success "Database backup created: $BACKUP_DIR/pre_deployment.sql.gz"
    else
        error "Database backup failed!"
        exit 1
    fi
    
    # Backup Redis
    log "Backing up DragonflyDB..."
    redis-cli -h "$DB_HOST" SAVE
    cp /var/lib/dragonfly/dump.rdb "$BACKUP_DIR/" 2>/dev/null || warning "Redis backup failed (non-critical)"
    
    # Backup configurations
    log "Backing up configurations..."
    tar czf "$BACKUP_DIR/configs.tar.gz" \
        "$RATHENA_DIR/conf" \
        "$AI_SERVICE_DIR/.env" \
        "$DASHBOARD_DIR/.env.local" 2>/dev/null || warning "Config backup incomplete"
    
    success "Pre-deployment backup completed"
}

stop_services() {
    log "Stopping services..."
    
    # Stop dashboard
    pm2 stop ai-dashboard 2>/dev/null || warning "Dashboard not running"
    
    # Stop rAthena
    cd "$RATHENA_DIR"
    ./athena-start stop || warning "rAthena stop failed"
    
    # Stop AI service
    sudo systemctl stop ai-service || warning "AI service stop failed"
    
    # Wait for graceful shutdown
    sleep 5
    
    # Verify all stopped
    if pgrep -f "uvicorn.*ai-service" > /dev/null; then
        warning "AI service still running, force stopping..."
        pkill -9 -f "uvicorn.*ai-service"
    fi
    
    success "Services stopped"
}

apply_migrations() {
    log "Applying database migrations..."
    
    cd "$AI_SERVICE_DIR/migrations"
    
    # Check if migrations exist
    if [ ! -d "$AI_SERVICE_DIR/migrations" ]; then
        warning "No migrations directory found, skipping..."
        return 0
    fi
    
    # Apply each migration
    local migration_count=0
    for migration in $(ls -1 *.sql 2>/dev/null | sort); do
        log "Applying migration: $migration"
        PGPASSWORD="${DB_PASSWORD}" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" \
            -f "$migration" >> "$LOG_FILE" 2>&1
        
        if [ $? -eq 0 ]; then
            success "Migration $migration applied"
            ((migration_count++))
        else
            error "Migration $migration failed!"
            return 1
        fi
    done
    
    if [ $migration_count -eq 0 ]; then
        log "No migrations to apply"
    else
        success "Applied $migration_count migrations"
    fi
}

deploy_ai_service() {
    log "Deploying AI service..."
    
    cd "$AI_SERVICE_DIR"
    
    # Pull latest code
    git pull origin main >> "$LOG_FILE" 2>&1
    
    # Activate virtual environment
    source venv/bin/activate
    
    # Update dependencies
    pip install --upgrade pip >> "$LOG_FILE" 2>&1
    pip install -r requirements.txt >> "$LOG_FILE" 2>&1
    
    success "AI service code deployed"
}

compile_bridge_layer() {
    log "Compiling Bridge Layer..."
    
    cd "$RATHENA_DIR"
    
    # Clean build
    make clean >> "$LOG_FILE" 2>&1
    
    # Compile (detect CPU cores)
    local cores=$(nproc)
    log "Compiling with $cores parallel jobs..."
    make server -j"$cores" >> "$LOG_FILE" 2>&1
    
    if [ $? -eq 0 ]; then
        success "Bridge Layer compiled"
        ls -lh ./login-server ./char-server ./map-server >> "$LOG_FILE"
    else
        error "Compilation failed! Check $LOG_FILE"
        return 1
    fi
}

deploy_dashboard() {
    log "Deploying dashboard..."
    
    cd "$DASHBOARD_DIR"
    
    # Pull latest code
    git pull origin main >> "$LOG_FILE" 2>&1
    
    # Install dependencies
    if command -v pnpm &> /dev/null; then
        pnpm install >> "$LOG_FILE" 2>&1
        pnpm build >> "$LOG_FILE" 2>&1
    else
        npm install >> "$LOG_FILE" 2>&1
        npm run build >> "$LOG_FILE" 2>&1
    fi
    
    if [ $? -eq 0 ]; then
        success "Dashboard deployed"
    else
        error "Dashboard build failed!"
        return 1
    fi
}

start_services() {
    log "Starting services..."
    
    # Start PostgreSQL (if not running)
    sudo systemctl start postgresql || warning "PostgreSQL already running"
    sleep 2
    
    # Start DragonflyDB (if not running)
    sudo systemctl start dragonfly || warning "DragonflyDB already running"
    sleep 2
    
    # Start AI service
    log "Starting AI service..."
    sudo systemctl start ai-service
    sleep 10  # Wait for initialization
    
    # Verify AI service started
    if ! curl -s http://192.168.0.100:8000/api/v1/health > /dev/null; then
        error "AI service failed to start!"
        return 1
    fi
    success "AI service started"
    
    # Start rAthena
    log "Starting rAthena servers..."
    cd "$RATHENA_DIR"
    ./athena-start start
    sleep 5
    
    # Start dashboard
    log "Starting dashboard..."
    cd "$DASHBOARD_DIR"
    pm2 start npm --name "ai-dashboard" -- start || pm2 restart ai-dashboard
    
    success "All services started"
}

smoke_tests() {
    log "Running smoke tests..."
    
    local tests_passed=0
    local tests_failed=0
    
    # Test 1: AI Service health
    log "Test 1: AI Service health check..."
    if curl -s http://192.168.0.100:8000/api/v1/health | grep -q "healthy"; then
        success "AI Service is healthy"
        ((tests_passed++))
    else
        error "AI Service health check failed"
        ((tests_failed++))
    fi
    
    # Test 2: Agent status
    log "Test 2: Checking agent status..."
    local active_agents=$(curl -s http://192.168.0.100:8000/api/v1/world/agents/status | \
        jq '[.agents[] | select(.status == "active")] | length')
    
    if [ "$active_agents" -eq 21 ]; then
        success "All 21 agents are active"
        ((tests_passed++))
    else
        error "Only $active_agents/21 agents active"
        ((tests_failed++))
    fi
    
    # Test 3: Database connectivity
    log "Test 3: Database connection..."
    if PGPASSWORD="${DB_PASSWORD}" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" \
        -c "SELECT 1;" > /dev/null 2>&1; then
        success "Database connection successful"
        ((tests_passed++))
    else
        error "Database connection failed"
        ((tests_failed++))
    fi
    
    # Test 4: DragonflyDB
    log "Test 4: Cache connection..."
    if redis-cli -h "$DB_HOST" PING | grep -q "PONG"; then
        success "Cache connection successful"
        ((tests_passed++))
    else
        error "Cache connection failed"
        ((tests_failed++))
    fi
    
    # Test 5: Dashboard
    log "Test 5: Dashboard accessibility..."
    if curl -s http://localhost:3000/api/health > /dev/null; then
        success "Dashboard is accessible"
        ((tests_passed++))
    else
        error "Dashboard is not accessible"
        ((tests_failed++))
    fi
    
    # Test 6: WebSocket
    log "Test 6: WebSocket connection..."
    if timeout 5 wscat -c ws://192.168.0.100:8000/ws < /dev/null 2>&1 | grep -q "connected"; then
        success "WebSocket connection successful"
        ((tests_passed++))
    else
        warning "WebSocket test inconclusive (wscat may not be installed)"
    fi
    
    # Summary
    log "Smoke tests completed: $tests_passed passed, $tests_failed failed"
    
    if [ $tests_failed -gt 0 ]; then
        error "Smoke tests failed! Review logs: $LOG_FILE"
        return 1
    else
        success "All smoke tests passed!"
        return 0
    fi
}

rollback() {
    error "Deployment failed! Initiating rollback..."
    
    log "Stopping services..."
    sudo systemctl stop ai-service
    pm2 stop ai-dashboard
    cd "$RATHENA_DIR" && ./athena-start stop
    
    log "Restoring database from backup..."
    gunzip -c "$BACKUP_DIR/pre_deployment.sql.gz" | \
        PGPASSWORD="${DB_PASSWORD}" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME"
    
    log "Restoring configurations..."
    tar xzf "$BACKUP_DIR/configs.tar.gz" -C /
    
    log "Starting services..."
    sudo systemctl start ai-service
    cd "$RATHENA_DIR" && ./athena-start start
    pm2 start ai-dashboard
    
    error "Rollback completed. Check logs: $LOG_FILE"
    exit 1
}

main() {
    log "=========================================="
    log "AI Autonomous World - Production Deployment"
    log "=========================================="
    
    # Confirm deployment
    if ! confirm "Ready to deploy to production?"; then
        log "Deployment cancelled by user"
        exit 0
    fi
    
    # Pre-deployment checks
    check_prerequisites || exit 1
    
    # Require database password
    if [ -z "${DB_PASSWORD:-}" ]; then
        read -sp "Enter PostgreSQL password for $DB_USER: " DB_PASSWORD
        echo
        export DB_PASSWORD
    fi
    
    # Backup
    pre_deployment_backup || exit 1
    
    # Stop services
    stop_services || { error "Failed to stop services"; exit 1; }
    
    # Apply migrations
    apply_migrations || { rollback; exit 1; }
    
    # Deploy code
    deploy_ai_service || { rollback; exit 1; }
    compile_bridge_layer || { rollback; exit 1; }
    deploy_dashboard || { rollback; exit 1; }
    
    # Start services
    start_services || { rollback; exit 1; }
    
    # Wait for services to stabilize
    log "Waiting 30 seconds for services to stabilize..."
    sleep 30
    
    # Smoke tests
    if smoke_tests; then
        success "=========================================="
        success "Deployment completed successfully!"
        success "=========================================="
        log "Deployment log: $LOG_FILE"
        log "Backup location: $BACKUP_DIR"
        log ""
        log "Next steps:"
        log "1. Monitor logs for 1 hour: sudo journalctl -u ai-service -f"
        log "2. Check dashboard: http://localhost:3000"
        log "3. Verify agent status: curl http://192.168.0.100:8000/api/v1/world/agents/status"
        log "4. Review deployment checklist: docs/DEPLOYMENT_CHECKLIST.md"
        exit 0
    else
        rollback
    fi
}

# Handle interrupts
trap 'error "Deployment interrupted!"; rollback' INT TERM

# Run deployment
main "$@"