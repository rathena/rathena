#!/bin/bash
###############################################################################
# System Health Check Script
# Version: 1.0.0
# Description: Comprehensive health check for AI Autonomous World system
# Usage: ./health-check.sh [--verbose] [--json]
###############################################################################

set -euo pipefail

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
DB_HOST="192.168.0.100"
DB_NAME="ai_world_memory"
DB_USER="ai_world_user"
AI_SERVICE_URL="http://192.168.0.100:8000"
DASHBOARD_URL="http://localhost:3000"
REDIS_HOST="192.168.0.100"
REDIS_PORT="6379"

# Flags
VERBOSE=false
JSON_OUTPUT=false
CHECKS_PASSED=0
CHECKS_FAILED=0
CHECKS_WARNING=0

# Parse arguments
for arg in "$@"; do
    case $arg in
        --verbose|-v) VERBOSE=true ;;
        --json|-j) JSON_OUTPUT=true ;;
        --help|-h) 
            echo "Usage: $0 [--verbose] [--json]"
            echo "  --verbose, -v  Show detailed output"
            echo "  --json, -j     Output results in JSON format"
            exit 0
            ;;
    esac
done

log() {
    if [ "$JSON_OUTPUT" = false ]; then
        echo -e "${BLUE}[$(date +'%H:%M:%S')]${NC} $1"
    fi
}

success() {
    ((CHECKS_PASSED++))
    if [ "$JSON_OUTPUT" = false ]; then
        echo -e "${GREEN}✓${NC} $1"
    fi
}

error() {
    ((CHECKS_FAILED++))
    if [ "$JSON_OUTPUT" = false ]; then
        echo -e "${RED}✗${NC} $1"
    fi
}

warning() {
    ((CHECKS_WARNING++))
    if [ "$JSON_OUTPUT" = false ]; then
        echo -e "${YELLOW}⚠${NC} $1"
    fi
}

verbose() {
    if [ "$VERBOSE" = true ] && [ "$JSON_OUTPUT" = false ]; then
        echo "    $1"
    fi
}

check_service_status() {
    local service=$1
    local status_output=$(systemctl is-active "$service" 2>/dev/null || echo "inactive")
    
    if [ "$status_output" = "active" ]; then
        success "$service is running"
        verbose "$(systemctl status "$service" --no-pager -l | head -3)"
        return 0
    else
        error "$service is not running (status: $status_output)"
        return 1
    fi
}

check_port() {
    local port=$1
    local description=$2
    
    if nc -z localhost "$port" 2>/dev/null || netstat -tuln 2>/dev/null | grep -q ":$port "; then
        success "Port $port ($description) is open"
        return 0
    else
        error "Port $port ($description) is not accessible"
        return 1
    fi
}

check_http_endpoint() {
    local url=$1
    local description=$2
    local timeout=${3:-5}
    
    local response=$(curl -s -w "\n%{http_code}" --max-time "$timeout" "$url" 2>/dev/null || echo "000")
    local http_code=$(echo "$response" | tail -1)
    local body=$(echo "$response" | sed '$d')
    
    if [ "$http_code" = "200" ]; then
        success "$description is accessible (HTTP $http_code)"
        verbose "Response: ${body:0:100}"
        return 0
    else
        error "$description failed (HTTP $http_code)"
        verbose "URL: $url"
        return 1
    fi
}

check_database() {
    log "Checking PostgreSQL..."
    
    # Check service
    check_service_status postgresql
    
    # Check connectivity
    if [ -n "${DB_PASSWORD:-}" ]; then
        if PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" -d "$DB_NAME" \
            -c "SELECT 1;" > /dev/null 2>&1; then
            success "PostgreSQL connection successful"
            
            # Get connection count
            local conn_count=$(PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" \
                -d "$DB_NAME" -t -c "SELECT count(*) FROM pg_stat_activity;" | tr -d ' ')
            local max_conn=$(PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" \
                -d "$DB_NAME" -t -c "SELECT setting FROM pg_settings WHERE name='max_connections';" | tr -d ' ')
            
            verbose "Active connections: $conn_count / $max_conn"
            
            # Warning if >80%
            local percent=$((conn_count * 100 / max_conn))
            if [ $percent -gt 80 ]; then
                warning "Connection pool usage high: $percent%"
            else
                success "Connection pool healthy: $percent%"
            fi
            
            # Check database size
            local db_size=$(PGPASSWORD="$DB_PASSWORD" psql -h "$DB_HOST" -U "$DB_USER" \
                -d "$DB_NAME" -t -c "SELECT pg_size_pretty(pg_database_size('$DB_NAME'));" | tr -d ' ')
            verbose "Database size: $db_size"
            
        else
            error "PostgreSQL connection failed"
            verbose "Host: $DB_HOST, Database: $DB_NAME, User: $DB_USER"
        fi
    else
        warning "DB_PASSWORD not set, skipping connection test"
    fi
}

check_redis() {
    log "Checking DragonflyDB/Redis..."
    
    # Check service
    check_service_status dragonfly || warning "DragonflyDB service check failed"
    
    # Check connectivity
    if redis-cli -h "$REDIS_HOST" -p "$REDIS_PORT" PING 2>/dev/null | grep -q "PONG"; then
        success "DragonflyDB connection successful"
        
        # Get stats
        local used_memory=$(redis-cli -h "$REDIS_HOST" INFO memory 2>/dev/null | \
            grep "used_memory_human" | cut -d: -f2 | tr -d '\r')
        local total_keys=$(redis-cli -h "$REDIS_HOST" DBSIZE 2>/dev/null | cut -d' ' -f2)
        
        verbose "Memory used: $used_memory"
        verbose "Total keys: $total_keys"
    else
        error "DragonflyDB connection failed"
    fi
}

check_ai_service() {
    log "Checking AI Service..."
    
    # Check service
    check_service_status ai-service
    
    # Check HTTP endpoint
    check_http_endpoint "$AI_SERVICE_URL/api/v1/health" "AI Service health endpoint"
    
    # Check detailed health
    local health_response=$(curl -s "$AI_SERVICE_URL/api/v1/health/detailed" 2>/dev/null)
    
    if [ -n "$health_response" ]; then
        # Check agent count
        local agent_count=$(echo "$health_response" | jq -r '.agents | length' 2>/dev/null || echo "0")
        if [ "$agent_count" -eq 21 ]; then
            success "All 21 agents detected"
        else
            error "Only $agent_count agents detected (expected 21)"
        fi
        
        # Check active agents
        local active_count=$(echo "$health_response" | jq -r '[.agents[] | select(.status == "active")] | length' 2>/dev/null || echo "0")
        if [ "$active_count" -eq 21 ]; then
            success "All 21 agents are active"
        else
            warning "Only $active_count/21 agents are active"
        fi
        
        # Check API latency
        local latency=$(curl -s -w "%{time_total}" -o /dev/null "$AI_SERVICE_URL/api/v1/health" 2>/dev/null)
        verbose "API latency: ${latency}s"
        
    else
        error "Could not retrieve detailed health status"
    fi
}

check_rathena() {
    log "Checking rAthena servers..."
    
    # Check login server
    if pgrep -f "login-server" > /dev/null; then
        success "Login server is running"
        check_port 6900 "Login server"
    else
        error "Login server is not running"
    fi
    
    # Check char server
    if pgrep -f "char-server" > /dev/null; then
        success "Char server is running"
        check_port 6121 "Char server"
    else
        error "Char server is not running"
    fi
    
    # Check map server
    if pgrep -f "map-server" > /dev/null; then
        success "Map server is running"
        check_port 5121 "Map server"
    else
        error "Map server is not running"
    fi
}

check_dashboard() {
    log "Checking Dashboard..."
    
    # Check PM2 process
    if pm2 jlist 2>/dev/null | jq -r '.[].name' | grep -q "ai-dashboard"; then
        local status=$(pm2 jlist 2>/dev/null | jq -r '.[] | select(.name=="ai-dashboard") | .pm2_env.status')
        if [ "$status" = "online" ]; then
            success "Dashboard PM2 process is online"
        else
            error "Dashboard PM2 process is $status"
        fi
    else
        error "Dashboard PM2 process not found"
    fi
    
    # Check HTTP endpoint
    check_http_endpoint "$DASHBOARD_URL/api/health" "Dashboard health endpoint" || \
        warning "Dashboard may still be starting up"
}

check_system_resources() {
    log "Checking system resources..."
    
    # CPU usage
    local cpu_usage=$(top -bn1 | grep "Cpu(s)" | awk '{print $2}' | cut -d'%' -f1)
    verbose "CPU usage: ${cpu_usage}%"
    
    if (( $(echo "$cpu_usage > 80" | bc -l 2>/dev/null || echo 0) )); then
        warning "High CPU usage: ${cpu_usage}%"
    else
        success "CPU usage normal: ${cpu_usage}%"
    fi
    
    # Memory usage
    local mem_percent=$(free | grep Mem | awk '{printf "%.0f", $3/$2 * 100.0}')
    verbose "Memory usage: ${mem_percent}%"
    
    if [ "$mem_percent" -gt 90 ]; then
        warning "High memory usage: ${mem_percent}%"
    else
        success "Memory usage normal: ${mem_percent}%"
    fi
    
    # Disk usage
    local disk_percent=$(df -h / | tail -1 | awk '{print $5}' | tr -d '%')
    verbose "Disk usage: ${disk_percent}%"
    
    if [ "$disk_percent" -gt 85 ]; then
        warning "High disk usage: ${disk_percent}%"
    else
        success "Disk usage normal: ${disk_percent}%"
    fi
}

check_network() {
    log "Checking network connectivity..."
    
    # Check AI service reachability
    if ping -c 1 -W 2 "$DB_HOST" > /dev/null 2>&1; then
        success "Network connectivity to $DB_HOST OK"
    else
        error "Cannot reach $DB_HOST"
    fi
    
    # Check external connectivity (LLM APIs)
    if ping -c 1 -W 2 api.openai.com > /dev/null 2>&1; then
        success "External network (OpenAI) reachable"
    else
        warning "Cannot reach OpenAI API (may affect LLM calls)"
    fi
}

check_logs_for_errors() {
    log "Checking recent logs for errors..."
    
    # AI Service errors (last hour)
    local error_count=$(sudo journalctl -u ai-service --since "1 hour ago" 2>/dev/null | \
        grep -c -i "error\|critical\|exception" || echo "0")
    
    if [ "$error_count" -eq 0 ]; then
        success "No errors in AI service logs (last hour)"
    elif [ "$error_count" -lt 10 ]; then
        warning "Found $error_count errors in AI service logs (last hour)"
    else
        error "High error count in AI service logs: $error_count (last hour)"
    fi
    
    # Check for critical errors
    local critical_count=$(sudo journalctl -u ai-service --since "1 hour ago" 2>/dev/null | \
        grep -c -i "critical" || echo "0")
    
    if [ "$critical_count" -gt 0 ]; then
        error "Found $critical_count CRITICAL errors in logs!"
    fi
}

check_llm_costs() {
    log "Checking LLM costs..."
    
    local cost_data=$(curl -s "$AI_SERVICE_URL/api/v1/admin/llm-costs" 2>/dev/null)
    
    if [ -n "$cost_data" ]; then
        local daily_cost=$(echo "$cost_data" | jq -r '.daily_cost_usd // 0' 2>/dev/null)
        local monthly_proj=$(echo "$cost_data" | jq -r '.monthly_projection_usd // 0' 2>/dev/null)
        local budget_pct=$(echo "$cost_data" | jq -r '.budget_usage_percent // 0' 2>/dev/null)
        
        verbose "Daily cost: \$${daily_cost}"
        verbose "Monthly projection: \$${monthly_proj}"
        verbose "Budget usage: ${budget_pct}%"
        
        if (( $(echo "$budget_pct > 85" | bc -l 2>/dev/null || echo 0) )); then
            error "LLM budget usage critical: ${budget_pct}%"
        elif (( $(echo "$budget_pct > 70" | bc -l 2>/dev/null || echo 0) )); then
            warning "LLM budget usage high: ${budget_pct}%"
        else
            success "LLM budget usage healthy: ${budget_pct}%"
        fi
    else
        warning "Could not retrieve LLM cost data"
    fi
}

check_performance() {
    log "Checking performance metrics..."
    
    # Check API response time
    local response_time=$(curl -s -w "%{time_total}" -o /dev/null "$AI_SERVICE_URL/api/v1/health" 2>/dev/null)
    
    if [ -n "$response_time" ]; then
        local response_ms=$(echo "$response_time * 1000" | bc -l | cut -d. -f1)
        verbose "API response time: ${response_ms}ms"
        
        if [ "$response_ms" -gt 500 ]; then
            warning "High API response time: ${response_ms}ms"
        elif [ "$response_ms" -gt 250 ]; then
            warning "Elevated API response time: ${response_ms}ms"
        else
            success "API response time healthy: ${response_ms}ms"
        fi
    fi
}

check_disk_space() {
    log "Checking disk space..."
    
    # Check main partition
    local disk_pct=$(df -h / | tail -1 | awk '{print $5}' | tr -d '%')
    verbose "Root partition: ${disk_pct}% used"
    
    if [ "$disk_pct" -gt 90 ]; then
        error "Critical disk usage: ${disk_pct}%"
    elif [ "$disk_pct" -gt 80 ]; then
        warning "High disk usage: ${disk_pct}%"
    else
        success "Disk space healthy: ${disk_pct}% used"
    fi
    
    # Check backup directory
    if [ -d "/opt/backups" ]; then
        local backup_size=$(du -sh /opt/backups 2>/dev/null | cut -f1)
        verbose "Backup directory size: $backup_size"
    fi
    
    # Check log directory
    if [ -d "/var/log/ai-service" ]; then
        local log_size=$(du -sh /var/log/ai-service 2>/dev/null | cut -f1)
        verbose "Log directory size: $log_size"
    fi
}

generate_json_report() {
    local status="healthy"
    if [ $CHECKS_FAILED -gt 0 ]; then
        status="critical"
    elif [ $CHECKS_WARNING -gt 0 ]; then
        status="warning"
    fi
    
    cat << EOF
{
  "timestamp": "$(date -Iseconds)",
  "status": "$status",
  "checks": {
    "passed": $CHECKS_PASSED,
    "failed": $CHECKS_FAILED,
    "warnings": $CHECKS_WARNING,
    "total": $((CHECKS_PASSED + CHECKS_FAILED + CHECKS_WARNING))
  },
  "components": {
    "postgresql": "$(systemctl is-active postgresql 2>/dev/null || echo unknown)",
    "dragonfly": "$(systemctl is-active dragonfly 2>/dev/null || echo unknown)",
    "ai_service": "$(systemctl is-active ai-service 2>/dev/null || echo unknown)",
    "dashboard": "$(pm2 jlist 2>/dev/null | jq -r '.[] | select(.name=="ai-dashboard") | .pm2_env.status' || echo unknown)",
    "login_server": "$(pgrep -f login-server > /dev/null && echo active || echo inactive)",
    "char_server": "$(pgrep -f char-server > /dev/null && echo active || echo inactive)",
    "map_server": "$(pgrep -f map-server > /dev/null && echo active || echo inactive)"
  }
}
EOF
}

print_summary() {
    if [ "$JSON_OUTPUT" = true ]; then
        generate_json_report
        return
    fi
    
    echo ""
    echo "=========================================="
    echo "Health Check Summary"
    echo "=========================================="
    echo -e "${GREEN}Passed:${NC}   $CHECKS_PASSED"
    echo -e "${YELLOW}Warnings:${NC} $CHECKS_WARNING"
    echo -e "${RED}Failed:${NC}   $CHECKS_FAILED"
    echo "=========================================="
    
    if [ $CHECKS_FAILED -eq 0 ] && [ $CHECKS_WARNING -eq 0 ]; then
        echo -e "${GREEN}✓ System Status: HEALTHY${NC}"
        return 0
    elif [ $CHECKS_FAILED -eq 0 ]; then
        echo -e "${YELLOW}⚠ System Status: WARNING${NC}"
        return 0
    else
        echo -e "${RED}✗ System Status: CRITICAL${NC}"
        return 1
    fi
}

main() {
    if [ "$JSON_OUTPUT" = false ]; then
        echo "=========================================="
        echo "AI Autonomous World - Health Check"
        echo "=========================================="
        echo ""
    fi
    
    # Run all checks
    check_database
    check_redis
    check_ai_service
    check_rathena
    check_dashboard
    check_system_resources
    check_network
    check_logs_for_errors
    check_llm_costs
    check_performance
    check_disk_space
    
    # Print summary and exit with appropriate code
    print_summary
    local exit_code=$?
    
    exit $exit_code
}

# Run health check
main "$@"