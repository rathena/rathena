#!/bin/bash

# ============================================
# AI-rAthena Complete Rebuild and Restart Script
# ============================================
# Rebuilds rAthena core, P2P coordinator, and AI service
# Then restarts all services in the correct dependency order

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_step() {
    echo -e "${YELLOW}[STEP]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

# ============================================
# PHASE 1: Stop All Running Services
# ============================================
print_header "PHASE 1: Stopping All Running Services"

print_step "Stopping AI Service..."
screen -S ai-service -X quit 2>/dev/null || true
print_success "AI Service stopped"

print_step "Stopping P2P Coordinator..."
screen -S p2p-coordinator -X quit 2>/dev/null || true
print_success "P2P Coordinator stopped"

print_step "Stopping rAthena servers..."
screen -S rathena-map -X quit 2>/dev/null || true
screen -S rathena-char -X quit 2>/dev/null || true
screen -S rathena-login -X quit 2>/dev/null || true
print_success "rAthena servers stopped"

# Give processes time to terminate
sleep 2

# ============================================
# PHASE 2: Clean and Rebuild rAthena Core
# ============================================
print_header "PHASE 2: Rebuilding rAthena Core Server"

print_step "Cleaning previous build..."
make clean 2>/dev/null || true
print_success "Clean complete"

print_step "Configuring rAthena..."
if [ ! -f "configure" ]; then
    print_error "configure script not found!"
    exit 1
fi
./configure
print_success "Configure complete"

print_step "Building rAthena server (this may take several minutes)..."
make server -j$(nproc)
if [ $? -eq 0 ]; then
    print_success "rAthena server built successfully"
else
    print_error "rAthena build failed!"
    exit 1
fi

# Verify executables exist
if [ ! -f "login-server" ] || [ ! -f "char-server" ] || [ ! -f "map-server" ]; then
    print_error "Server executables not found after build!"
    exit 1
fi
print_success "All server executables verified"

# ============================================
# PHASE 3: Verify P2P Coordinator Dependencies
# ============================================
print_header "PHASE 3: Verifying P2P Coordinator"

cd src/p2p-coordinator

print_step "Checking P2P coordinator virtual environment..."
if [ ! -d "venv" ]; then
    print_step "Creating virtual environment..."
    python3 -m venv venv
fi

source venv/bin/activate

print_step "Installing/updating P2P coordinator dependencies..."
pip install --upgrade pip -q
pip install -r requirements.txt -q
print_success "P2P coordinator dependencies verified"

deactivate
cd ..

# ============================================
# PHASE 4: Verify AI Service Dependencies
# ============================================
print_header "PHASE 4: Verifying AI Service"

cd ai-autonomous-world/ai-service

print_step "Checking AI service virtual environment..."
if [ ! -d "venv" ]; then
    print_step "Creating virtual environment..."
    python3 -m venv venv
fi

source venv/bin/activate

print_step "Installing/updating AI service dependencies..."
pip install --upgrade pip -q
pip install -r requirements.txt -q
print_success "AI service dependencies verified"

deactivate
cd ../..

# ============================================
# PHASE 5: Verify Database Services
# ============================================
print_header "PHASE 5: Verifying Database Services"

print_step "Checking DragonflyDB..."
if nc -z localhost 6379 2>/dev/null; then
    print_success "DragonflyDB is running on port 6379"
else
    print_error "DragonflyDB is not running on port 6379!"
    print_info "Please start DragonflyDB: sudo systemctl start dragonfly"
    exit 1
fi

print_step "Checking PostgreSQL..."
if nc -z localhost 5432 2>/dev/null; then
    print_success "PostgreSQL is running on port 5432"
else
    print_error "PostgreSQL is not running on port 5432!"
    print_info "Please start PostgreSQL: sudo systemctl start postgresql"
    exit 1
fi

# ============================================
# PHASE 6: Start rAthena Core Servers
# ============================================
print_header "PHASE 6: Starting rAthena Core Servers"

print_step "Starting login-server..."
screen -dmS rathena-login bash -c "./login-server"
sleep 2
if screen -list | grep -q "rathena-login"; then
    print_success "Login server started"
else
    print_error "Failed to start login server"
    exit 1
fi

print_step "Starting char-server..."
screen -dmS rathena-char bash -c "./char-server"
sleep 2
if screen -list | grep -q "rathena-char"; then
    print_success "Char server started"
else
    print_error "Failed to start char server"
    exit 1
fi

print_step "Starting map-server..."
screen -dmS rathena-map bash -c "./map-server"
sleep 3
if screen -list | grep -q "rathena-map"; then
    print_success "Map server started"
else
    print_error "Failed to start map server"
    exit 1
fi

# ============================================
# PHASE 7: Start P2P Coordinator Service
# ============================================
print_header "PHASE 7: Starting P2P Coordinator Service"

cd src/p2p-coordinator
# Start the C++ p2p-coordinator here if needed (update this section for C++ build/run)
cd ../..

sleep 3

print_step "Checking P2P coordinator health..."
if curl -f http://localhost:8001/health > /dev/null 2>&1; then
    print_success "P2P Coordinator started (http://localhost:8001)"
else
    print_error "P2P Coordinator failed to start or health check failed"
    print_info "Check logs: screen -r p2p-coordinator"
    exit 1
fi

# ============================================
# PHASE 8: Start AI Autonomous World Service
# ============================================
print_header "PHASE 8: Starting AI Autonomous World Service"

cd ai-autonomous-world/ai-service
screen -dmS ai-service bash -c "source venv/bin/activate && uvicorn main:app --host 0.0.0.0 --port 8000 --log-level info"
cd ../..

sleep 3

print_step "Checking AI service health..."
if curl -f http://localhost:8000/health > /dev/null 2>&1; then
    print_success "AI Service started (http://localhost:8000)"
else
    print_error "AI Service failed to start or health check failed"
    print_info "Check logs: screen -r ai-service"
    exit 1
fi

# ============================================
# PHASE 9: Verify All Services
# ============================================
print_header "PHASE 9: Verifying All Services"

print_step "Checking all screen sessions..."
screen -list

echo ""
print_step "Service Status Summary:"
echo ""

# Check rAthena servers
if screen -list | grep -q "rathena-login"; then
    print_success "✓ Login Server: Running"
else
    print_error "✗ Login Server: Not Running"
fi

if screen -list | grep -q "rathena-char"; then
    print_success "✓ Char Server: Running"
else
    print_error "✗ Char Server: Not Running"
fi

if screen -list | grep -q "rathena-map"; then
    print_success "✓ Map Server: Running"
else
    print_error "✗ Map Server: Not Running"
fi

# Check P2P coordinator
if curl -f http://localhost:8001/health > /dev/null 2>&1; then
    print_success "✓ P2P Coordinator: Running (http://localhost:8001)"
else
    print_error "✗ P2P Coordinator: Not Running"
fi

# Check AI service
if curl -f http://localhost:8000/health > /dev/null 2>&1; then
    print_success "✓ AI Service: Running (http://localhost:8000)"
else
    print_error "✗ AI Service: Not Running"
fi

# Check databases
if nc -z localhost 6379 2>/dev/null; then
    print_success "✓ DragonflyDB: Running (port 6379)"
else
    print_error "✗ DragonflyDB: Not Running"
fi

if nc -z localhost 5432 2>/dev/null; then
    print_success "✓ PostgreSQL: Running (port 5432)"
else
    print_error "✗ PostgreSQL: Not Running"
fi

echo ""
print_header "Rebuild and Restart Complete!"
echo ""
print_info "To view service logs, use:"
print_info "  screen -r rathena-login   # Login server"
print_info "  screen -r rathena-char    # Char server"
print_info "  screen -r rathena-map     # Map server"
# print_info "  screen -r p2p-coordinator # P2P coordinator"
print_info "  screen -r ai-service      # AI service"
echo ""
print_info "To detach from a screen session: Ctrl+A then D"
echo ""

