#!/bin/bash
# System Status Check Script for AI-rAthena Integration
# Verifies all services and components are operational

set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}AI-rAthena System Status Check${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Function to check service
check_service() {
    local service_name=$1
    local check_command=$2
    
    if eval "$check_command" > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} $service_name: ${GREEN}RUNNING${NC}"
        return 0
    else
        echo -e "${RED}✗${NC} $service_name: ${RED}NOT RUNNING${NC}"
        return 1
    fi
}

# Function to check port
check_port() {
    local service_name=$1
    local port=$2
    
    if lsof -i :$port > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} $service_name (port $port): ${GREEN}LISTENING${NC}"
        return 0
    else
        echo -e "${RED}✗${NC} $service_name (port $port): ${RED}NOT LISTENING${NC}"
        return 1
    fi
}

# Function to check HTTP endpoint
check_http() {
    local service_name=$1
    local url=$2
    
    if curl -s -f "$url" > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} $service_name: ${GREEN}HEALTHY${NC}"
        return 0
    else
        echo -e "${RED}✗${NC} $service_name: ${RED}UNHEALTHY${NC}"
        return 1
    fi
}

# Track overall status
TOTAL_CHECKS=0
PASSED_CHECKS=0

echo -e "${BLUE}1. Checking rAthena Servers${NC}"
echo "-----------------------------------"
if check_service "Login Server" "pgrep -f login-server"; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
if check_service "Char Server" "pgrep -f char-server"; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
if check_service "Map Server" "pgrep -f map-server"; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
echo ""

echo -e "${BLUE}2. Checking AI Service${NC}"
echo "-----------------------------------"
if check_port "AI Service" 8000; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
if check_http "AI Service Health" "http://localhost:8000/health"; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
echo ""

echo -e "${BLUE}3. Checking Databases${NC}"
echo "-----------------------------------"
if check_service "PostgreSQL" "systemctl is-active postgresql"; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))
if check_port "DragonflyDB/Redis" 6379; then ((PASSED_CHECKS++)); fi; ((TOTAL_CHECKS++))

# Check PostgreSQL connection
if PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory -c "SELECT 1" > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} PostgreSQL Connection: ${GREEN}OK${NC}"
    ((PASSED_CHECKS++))
else
    echo -e "${RED}✗${NC} PostgreSQL Connection: ${RED}FAILED${NC}"
fi
((TOTAL_CHECKS++))

# Check Redis connection
if redis-cli -p 6379 ping > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} DragonflyDB Connection: ${GREEN}OK${NC}"
    ((PASSED_CHECKS++))
else
    echo -e "${RED}✗${NC} DragonflyDB Connection: ${RED}FAILED${NC}"
fi
((TOTAL_CHECKS++))
echo ""

echo -e "${BLUE}4. Checking Database Tables${NC}"
echo "-----------------------------------"
TABLE_COUNT=$(PGPASSWORD='ai_world_pass_2025' psql -h localhost -U ai_world_user -d ai_world_memory -t -c "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'public';" 2>/dev/null | tr -d ' ')
if [ "$TABLE_COUNT" -ge 18 ]; then
    echo -e "${GREEN}✓${NC} Database Tables: ${GREEN}$TABLE_COUNT tables found${NC}"
    ((PASSED_CHECKS++))
else
    echo -e "${YELLOW}⚠${NC} Database Tables: ${YELLOW}$TABLE_COUNT tables (expected 18+)${NC}"
fi
((TOTAL_CHECKS++))
echo ""

echo -e "${BLUE}5. Checking API Endpoints${NC}"
echo "-----------------------------------"
if curl -s http://localhost:8000/health/detailed > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Detailed Health Endpoint: ${GREEN}OK${NC}"
    ((PASSED_CHECKS++))
else
    echo -e "${YELLOW}⚠${NC} Detailed Health Endpoint: ${YELLOW}NOT AVAILABLE${NC}"
fi
((TOTAL_CHECKS++))

if curl -s http://localhost:8000/docs > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} API Documentation: ${GREEN}OK${NC}"
    ((PASSED_CHECKS++))
else
    echo -e "${YELLOW}⚠${NC} API Documentation: ${YELLOW}NOT AVAILABLE${NC}"
fi
((TOTAL_CHECKS++))
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Summary${NC}"
echo -e "${BLUE}========================================${NC}"
SUCCESS_RATE=$(echo "scale=1; $PASSED_CHECKS * 100 / $TOTAL_CHECKS" | bc)
echo -e "Total Checks: $TOTAL_CHECKS"
echo -e "Passed: ${GREEN}$PASSED_CHECKS${NC}"
echo -e "Failed: ${RED}$((TOTAL_CHECKS - PASSED_CHECKS))${NC}"
echo -e "Success Rate: ${GREEN}${SUCCESS_RATE}%${NC}"
echo ""

if [ "$PASSED_CHECKS" -eq "$TOTAL_CHECKS" ]; then
    echo -e "${GREEN}✓ All systems operational!${NC}"
    echo -e "${GREEN}The AI-rAthena integration is 100% functional.${NC}"
    exit 0
elif [ "$SUCCESS_RATE" -ge 80 ]; then
    echo -e "${YELLOW}⚠ System mostly operational (${SUCCESS_RATE}%)${NC}"
    echo -e "${YELLOW}Some non-critical components may need attention.${NC}"
    exit 0
else
    echo -e "${RED}✗ System has issues (${SUCCESS_RATE}%)${NC}"
    echo -e "${RED}Please check the failed components above.${NC}"
    exit 1
fi

