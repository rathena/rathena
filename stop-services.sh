#!/bin/bash

# ============================================
# AI-rAthena Stop Services Script
# ============================================
# Stops all services started by start-services.sh

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

print_step() {
    echo -e "${BLUE}[STOP]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_step "Stopping AI-rAthena services..."

# Stop AI Service
print_step "Stopping AI Service..."
screen -S ai-service -X quit 2>/dev/null || true
print_success "AI Service stopped"

# Stop rAthena Map Server
print_step "Stopping rAthena Map Server..."
screen -S rathena-map -X quit 2>/dev/null || true
print_success "rAthena Map Server stopped"

echo ""
print_success "All services stopped"
echo ""

