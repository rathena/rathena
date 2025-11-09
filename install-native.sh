#!/bin/bash

# ============================================
# AI-rAthena Native Installation Script
# ============================================
# Installs all dependencies and services natively (no Docker)
# Supports: Ubuntu 20.04+, Debian 11+, CentOS 8+

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Detect OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    VER=$VERSION_ID
else
    print_error "Cannot detect OS"
    exit 1
fi

print_step "Detected OS: $OS $VER"

# ============================================
# 1. Install System Dependencies
# ============================================
print_step "Installing system dependencies..."

if [[ "$OS" == "ubuntu" ]] || [[ "$OS" == "debian" ]]; then
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        git \
        cmake \
        gcc \
        g++ \
        make \
        libmysqlclient-dev \
        zlib1g-dev \
        libpcre3-dev \
        libssl-dev \
        python3 \
        python3-pip \
        python3-venv \
        postgresql \
        postgresql-contrib \
        redis-server \
        curl \
        wget \
        screen
elif [[ "$OS" == "centos" ]] || [[ "$OS" == "rhel" ]]; then
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y \
        git \
        cmake \
        gcc \
        gcc-c++ \
        make \
        mysql-devel \
        zlib-devel \
        pcre-devel \
        openssl-devel \
        python3 \
        python3-pip \
        postgresql \
        postgresql-server \
        redis \
        curl \
        wget \
        screen
else
    print_error "Unsupported OS: $OS"
    exit 1
fi

print_success "System dependencies installed"

# ============================================
# 2. Setup PostgreSQL
# ============================================
print_step "Setting up PostgreSQL..."

# Initialize PostgreSQL (CentOS/RHEL only)
if [[ "$OS" == "centos" ]] || [[ "$OS" == "rhel" ]]; then
    sudo postgresql-setup --initdb
fi

# Start PostgreSQL
sudo systemctl start postgresql
sudo systemctl enable postgresql

# Create database and user
sudo -u postgres psql -c "CREATE DATABASE rathena_ai;" 2>/dev/null || true
sudo -u postgres psql -c "CREATE USER rathena WITH PASSWORD 'changeme';" 2>/dev/null || true
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE rathena_ai TO rathena;" 2>/dev/null || true

# Initialize database schema
sudo -u postgres psql -d rathena_ai -f ai-autonomous-world/database/init.sql

print_success "PostgreSQL configured"

# ============================================
# 3. Setup Redis
# ============================================
print_step "Setting up Redis..."

# Start Redis
sudo systemctl start redis
sudo systemctl enable redis

print_success "Redis configured"

# ============================================
# 4. Setup Python Environment
# ============================================
print_step "Setting up Python environment..."

cd ai-autonomous-world/ai-service

# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Upgrade pip
pip install --upgrade pip

# Install dependencies
pip install -r requirements.txt

print_success "Python environment configured"

# ============================================
# 5. Build rAthena
# ============================================
print_step "Building rAthena with AI Bridge..."

cd ../..

# Configure
./configure

# Build
make clean
make server -j$(nproc)

print_success "rAthena built successfully"

# ============================================
# 6. Create Environment Configuration
# ============================================
print_step "Creating environment configuration..."

if [ ! -f .env ]; then
    cp .env.example .env
    print_warning "Created .env file - PLEASE EDIT IT AND ADD YOUR API KEYS!"
fi

print_success "Environment configuration created"

# ============================================
# 7. Create Systemd Services
# ============================================
print_step "Creating systemd service files..."

# AI Service
sudo tee /etc/systemd/system/rathena-ai-service.service > /dev/null <<EOF
[Unit]
Description=rAthena AI Service
After=network.target postgresql.service redis.service

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)/ai-autonomous-world/ai-service
Environment="PATH=$(pwd)/ai-autonomous-world/ai-service/venv/bin"
EnvironmentFile=$(pwd)/.env
ExecStart=$(pwd)/ai-autonomous-world/ai-service/venv/bin/uvicorn main:app --host 0.0.0.0 --port 8000 --workers 4
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# rAthena Map Server
sudo tee /etc/systemd/system/rathena-map.service > /dev/null <<EOF
[Unit]
Description=rAthena Map Server with AI Bridge
After=network.target rathena-ai-service.service

[Service]
Type=simple
User=$USER
WorkingDirectory=$(pwd)
EnvironmentFile=$(pwd)/.env
ExecStart=$(pwd)/map-server
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# Reload systemd
sudo systemctl daemon-reload

print_success "Systemd services created"

# ============================================
# Installation Complete
# ============================================
echo ""
echo "============================================"
print_success "Installation Complete!"
echo "============================================"
echo ""
echo "Next steps:"
echo "1. Edit .env file and add your LLM API keys:"
echo "   nano .env"
echo ""
echo "2. Start services:"
echo "   sudo systemctl start rathena-ai-service"
echo "   sudo systemctl start rathena-map"
echo ""
echo "3. Enable services to start on boot:"
echo "   sudo systemctl enable rathena-ai-service"
echo "   sudo systemctl enable rathena-map"
echo ""
echo "4. Check service status:"
echo "   sudo systemctl status rathena-ai-service"
echo "   sudo systemctl status rathena-map"
echo ""
echo "5. View logs:"
echo "   sudo journalctl -u rathena-ai-service -f"
echo "   sudo journalctl -u rathena-map -f"
echo ""
echo "6. Test the installation:"
echo "   curl http://localhost:8000/health"
echo "   python3 tests/e2e_test.py"
echo ""
echo "============================================"

