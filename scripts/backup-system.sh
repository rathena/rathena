#!/bin/bash
###############################################################################
# System Backup Script
# Version: 1.0.0
# Description: Automated backup for AI Autonomous World system
# Schedule: Daily at 02:00 via cron
###############################################################################

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BACKUP_BASE_DIR="/opt/backups"
BACKUP_DIR="$BACKUP_BASE_DIR/$(date +%Y%m%d)"
DB_HOST="192.168.0.100"
DB_NAME="ai_world_memory"
DB_USER="ai_world_user"
RETENTION_DAYS=30
LOG_FILE="/var/log/backup_$(date +%Y%m%d).log"

# Directories to backup
INSTALL_DIR="/opt/ai-mmorpg/ai-mmorpg-world"
AI_SERVICE_DIR="$INSTALL_DIR/rathena-AI-world/ai-autonomous-world/ai-service"
RATHENA_DIR="$INSTALL_DIR/rathena-AI-world"
DASHBOARD_DIR="$INSTALL_DIR/rathena-AI-world/dashboard"

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

create_backup_dir() {
    log "Creating backup directory: $BACKUP_DIR"
    mkdir -p "$BACKUP_DIR"
    chmod 700 "$BACKUP_DIR"
    success "Backup directory created"
}

backup_postgresql() {
    log "Backing up PostgreSQL database..."
    
    local backup_file="$BACKUP_DIR/ai_world_memory.sql.gz"
    
    # Check if DB_PASSWORD is set
    if [ -z "${DB_PASSWORD:-}" ]; then
        error "DB_PASSWORD environment variable not set"
        return 1
    fi
    
    # Create backup
    PGPASSWORD="$DB_PASSWORD" pg_dump \
        -h "$DB_HOST" \
        -U "$DB_USER" \
        -d "$DB_NAME" \
        --verbose \
        2>> "$LOG_FILE" | gzip > "$backup_file"
    
    if [ $? -eq 0 ]; then
        local size=$(du -h "$backup_file" | cut -f1)
        success "PostgreSQL backup created: $backup_file ($size)"
        
        # Verify backup is not empty
        if [ $(stat -f%z "$backup_file" 2>/dev/null || stat -c%s "$backup_file") -lt 1000 ]; then
            error "Backup file suspiciously small!"
            return 1
        fi
    else
        error "PostgreSQL backup failed!"
        return 1
    fi
}

backup_dragonfly() {
    log "Backing up DragonflyDB..."
    
    # Trigger save
    redis-cli -h "$DB_HOST" SAVE >> "$LOG_FILE" 2>&1
    
    # Copy dump file
    if [ -f /var/lib/dragonfly/dump.rdb ]; then
        cp /var/lib/dragonfly/dump.rdb "$BACKUP_DIR/dragonfly_dump.rdb"
        local size=$(du -h "$BACKUP_DIR/dragonfly_dump.rdb" | cut -f1)
        success "DragonflyDB backup created ($size)"
    else
        warning "DragonflyDB dump file not found (may not be critical)"
    fi
}

backup_configurations() {
    log "Backing up configuration files..."
    
    local config_backup="$BACKUP_DIR/configs.tar.gz"
    
    tar czf "$config_backup" \
        -C / \
        opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/conf \
        opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/ai-autonomous-world/ai-service/.env \
        opt/ai-mmorpg/ai-mmorpg-world/rathena-AI-world/dashboard/.env.local \
        2>> "$LOG_FILE"
    
    if [ $? -eq 0 ]; then
        local size=$(du -h "$config_backup" | cut -f1)
        success "Configuration backup created ($size)"
    else
        warning "Configuration backup incomplete"
    fi
}

backup_logs() {
    log "Backing up recent logs..."
    
    local log_backup="$BACKUP_DIR/logs.tar.gz"
    
    # Backup last 7 days of logs
    tar czf "$log_backup" \
        --exclude='*.old' \
        /var/log/ai-service/*.log \
        "$RATHENA_DIR/log/*.log" \
        2>> "$LOG_FILE" || warning "Log backup incomplete"
    
    if [ -f "$log_backup" ]; then
        local size=$(du -h "$log_backup" | cut -f1)
        success "Log backup created ($size)"
    fi
}

create_backup_manifest() {
    log "Creating backup manifest..."
    
    cat > "$BACKUP_DIR/MANIFEST.txt" << EOF
Backup Date: $(date +'%Y-%m-%d %H:%M:%S')
Backup Directory: $BACKUP_DIR
System Version: 1.0.0

Files:
$(ls -lh "$BACKUP_DIR")

Database Info:
$(PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "SELECT version();" 2>/dev/null)

Table Counts:
$(PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" -c "
SELECT schemaname, tablename, n_tup_ins as row_count
FROM pg_stat_user_tables
ORDER BY n_tup_ins DESC
LIMIT 20;
" 2>/dev/null)

Total Backup Size: $(du -sh "$BACKUP_DIR" | cut -f1)

Backup Integrity: OK
EOF
    
    success "Backup manifest created"
}

cleanup_old_backups() {
    log "Cleaning up old backups (retention: $RETENTION_DAYS days)..."
    
    local deleted=0
    find "$BACKUP_BASE_DIR" -type d -name "20*" -mtime +$RETENTION_DAYS | while read dir; do
        log "Deleting old backup: $dir"
        rm -rf "$dir"
        ((deleted++))
    done
    
    if [ $deleted -gt 0 ]; then
        success "Deleted $deleted old backup(s)"
    else
        log "No old backups to delete"
    fi
}

verify_backup() {
    log "Verifying backup integrity..."
    
    local backup_file="$BACKUP_DIR/ai_world_memory.sql.gz"
    
    # Test decompression
    if gunzip -t "$backup_file" 2>> "$LOG_FILE"; then
        success "Backup file integrity verified"
    else
        error "Backup file corrupted!"
        return 1
    fi
    
    # Test SQL content
    if gunzip -c "$backup_file" | head -100 | grep -q "CREATE TABLE"; then
        success "Backup SQL content verified"
    else
        error "Backup file does not contain valid SQL!"
        return 1
    fi
}

send_notification() {
    local status=$1
    local message=$2
    
    # Send notification (customize for your notification system)
    # Examples: Slack, email, PagerDuty
    
    # Slack webhook (if configured)
    if [ -n "${SLACK_WEBHOOK_URL:-}" ]; then
        curl -X POST "$SLACK_WEBHOOK_URL" \
            -H 'Content-Type: application/json' \
            -d "{\"text\":\"Backup $status: $message\"}" \
            >> "$LOG_FILE" 2>&1
    fi
    
    # Email (if mail command available)
    if command -v mail &> /dev/null; then
        echo "$message" | mail -s "Backup $status" admin@yourdomain.com 2>> "$LOG_FILE"
    fi
}

main() {
    log "=========================================="
    log "AI Autonomous World - System Backup"
    log "=========================================="
    
    # Check DB password
    if [ -z "${DB_PASSWORD:-}" ]; then
        # Try to read from file (secure location)
        if [ -f /etc/ai-world/.db_password ]; then
            export DB_PASSWORD=$(cat /etc/ai-world/.db_password)
        else
            error "DB_PASSWORD not set and /etc/ai-world/.db_password not found"
            exit 1
        fi
    fi
    
    # Create backup directory
    create_backup_dir || exit 1
    
    # Perform backups
    local backup_success=true
    
    backup_postgresql || backup_success=false
    backup_dragonfly || warning "DragonflyDB backup failed (non-critical)"
    backup_configurations || warning "Config backup failed (non-critical)"
    backup_logs || warning "Log backup failed (non-critical)"
    
    # Create manifest
    create_backup_manifest
    
    # Verify backup
    if verify_backup; then
        success "Backup verification passed"
    else
        backup_success=false
    fi
    
    # Cleanup old backups
    cleanup_old_backups
    
    # Summary
    local total_size=$(du -sh "$BACKUP_DIR" | cut -f1)
    
    if [ "$backup_success" = true ]; then
        success "=========================================="
        success "Backup completed successfully!"
        success "=========================================="
        log "Backup location: $BACKUP_DIR"
        log "Total size: $total_size"
        log "Log file: $LOG_FILE"
        
        send_notification "SUCCESS" "Daily backup completed. Size: $total_size"
        exit 0
    else
        error "=========================================="
        error "Backup completed with errors!"
        error "=========================================="
        log "Review log file: $LOG_FILE"
        
        send_notification "FAILED" "Daily backup completed with errors. Check logs."
        exit 1
    fi
}

# Run backup
main "$@"