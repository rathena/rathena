#!/bin/bash

# ============================================
# AI-rAthena Quick Start Script
# ============================================
# Starts all services in screen sessions for development/testing
# No systemd required - useful for development

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_step() {
    echo -e "${BLUE}[START]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if .env exists
if [ ! -f .env ]; then
    print_warning ".env file not found! Creating from template..."
    cp .env.example .env
    print_warning "Please edit .env and add your API keys before continuing!"
    echo "Press Enter to continue after editing .env, or Ctrl+C to exit..."
    read
fi

# Load environment variables
source .env

# Check for API keys
if [ -z "$OPENAI_API_KEY" ] && [ -z "$ANTHROPIC_API_KEY" ] && [ -z "$GOOGLE_API_KEY" ]; then
    print_warning "No LLM API key found in .env!"
    print_warning "Please set at least one: OPENAI_API_KEY, ANTHROPIC_API_KEY, or GOOGLE_API_KEY"
    exit 1
fi

print_step "Starting AI-rAthena services..."

# ============================================
# 1. Check PostgreSQL
# ============================================
print_step "Checking PostgreSQL..."
if ! sudo systemctl is-active --quiet postgresql; then
    print_step "Starting PostgreSQL..."
    sudo systemctl start postgresql
fi
print_success "PostgreSQL is running"

# ============================================
# 2. Check Redis
# ============================================
print_step "Checking Redis..."
if ! sudo systemctl is-active --quiet redis; then
    print_step "Starting Redis..."
    sudo systemctl start redis
fi
print_success "Redis is running"

# ============================================
# 3. Start AI Service
# ============================================
print_step "Starting AI Service..."

# Kill existing screen session if it exists
screen -S ai-service -X quit 2>/dev/null || true

# Start AI service in screen
cd ai-autonomous-world/ai-service
screen -dmS ai-service bash -c "source venv/bin/activate && uvicorn main:app --host 0.0.0.0 --port 8000 --workers 4 --log-level info"
cd ../..

# Wait for AI service to start
sleep 3

# Check if AI service is running
if curl -f http://localhost:8000/health > /dev/null 2>&1; then
    print_success "AI Service started (http://localhost:8000)"
else
    print_warning "AI Service may still be starting..."
fi

# ============================================
# 4. Start rAthena Map Server
# ============================================
print_step "Starting rAthena Map Server..."

# Kill existing screen session if it exists
screen -S rathena-map -X quit 2>/dev/null || true

# Start map server in screen
screen -dmS rathena-map bash -c "./map-server"

print_success "rAthena Map Server started"

# ============================================
# Display Status
# ============================================
echo ""
echo "============================================"
print_success "All services started!"
echo "============================================"
echo ""
echo "Service URLs:"
echo "  AI Service API:  http://localhost:8000"
echo "  AI Service Docs: http://localhost:8000/docs"
echo "  rAthena Map:     localhost:5121"
echo "  rAthena Web:     http://localhost:8888"
echo ""
echo "Screen Sessions:"
echo "  AI Service:      screen -r ai-service"
echo "  rAthena Map:     screen -r rathena-map"
echo ""
echo "View Logs:"
echo "  screen -r ai-service    (Ctrl+A, D to detach)"
echo "  screen -r rathena-map   (Ctrl+A, D to detach)"
echo ""
echo "Stop Services:"
echo "  ./stop-services.sh"
echo ""
echo "Test Installation:"
echo "  curl http://localhost:8000/health"
echo "  python3 tests/e2e_test.py"
echo ""
echo "============================================"

