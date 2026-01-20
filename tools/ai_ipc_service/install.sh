#!/bin/bash
# =============================================================================
# AI IPC Service - Installation Script
# =============================================================================
# This script installs Python dependencies and sets up the AI IPC worker service.
#
# Usage:
#   ./install.sh [options]
#
# Options:
#   --venv           Create and use a virtual environment (recommended)
#   --system         Install to system Python (not recommended)
#   --dev            Install development dependencies
#   --help           Show this help message
#
# =============================================================================

set -e  # Exit on error

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Default values
USE_VENV=1
INSTALL_DEV=0
PYTHON_CMD="python3"
PIP_CMD="pip3"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --venv)
            USE_VENV=1
            shift
            ;;
        --system)
            USE_VENV=0
            shift
            ;;
        --dev)
            INSTALL_DEV=1
            shift
            ;;
        --help)
            echo "AI IPC Service - Installation Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --venv           Create and use a virtual environment (recommended)"
            echo "  --system         Install to system Python (not recommended)"
            echo "  --dev            Install development dependencies"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Error: Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Function to print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Print banner
echo "============================================================================="
echo "  AI IPC Service - Installation"
echo "============================================================================="
echo ""

# Check if Python is installed
if ! command -v $PYTHON_CMD &> /dev/null; then
    print_error "Python 3 not found. Please install Python 3.9 or later."
    exit 1
fi

# Check Python version
PYTHON_VERSION=$($PYTHON_CMD --version 2>&1 | awk '{print $2}')
PYTHON_MAJOR=$(echo $PYTHON_VERSION | cut -d. -f1)
PYTHON_MINOR=$(echo $PYTHON_VERSION | cut -d. -f2)

print_info "Python version: $PYTHON_VERSION"

if [[ $PYTHON_MAJOR -lt 3 ]] || [[ $PYTHON_MAJOR -eq 3 && $PYTHON_MINOR -lt 9 ]]; then
    print_error "Python 3.9 or later is required. Current version: $PYTHON_VERSION"
    exit 1
fi

print_success "Python version check passed"

# Create virtual environment if requested
if [[ $USE_VENV -eq 1 ]]; then
    print_info "Creating virtual environment..."
    
    if [[ -d "venv" ]]; then
        print_warning "Virtual environment already exists at ./venv"
        read -p "Do you want to remove it and create a new one? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            print_info "Removing old virtual environment..."
            rm -rf venv
        else
            print_info "Using existing virtual environment"
        fi
    fi
    
    if [[ ! -d "venv" ]]; then
        $PYTHON_CMD -m venv venv
        print_success "Virtual environment created at ./venv"
    fi
    
    # Activate virtual environment
    print_info "Activating virtual environment..."
    source venv/bin/activate
    
    # Update pip in venv
    print_info "Upgrading pip..."
    pip install --upgrade pip
    
    PYTHON_CMD="python"
    PIP_CMD="pip"
    
    print_success "Virtual environment activated"
else
    print_warning "Installing to system Python. This may require sudo privileges."
    
    # Check if we need sudo
    if ! pip install --help &> /dev/null; then
        print_warning "pip not found. Trying pip3..."
        PIP_CMD="pip3"
    fi
    
    # Test if we can install without sudo
    if ! $PIP_CMD install --dry-run --user pip &> /dev/null 2>&1; then
        print_warning "May need sudo for system-wide installation"
        PIP_CMD="sudo $PIP_CMD"
    else
        PIP_CMD="$PIP_CMD install --user"
    fi
fi

# Check if requirements.txt exists
if [[ ! -f "requirements.txt" ]]; then
    print_error "requirements.txt not found"
    exit 1
fi

print_info "Installing Python dependencies from requirements.txt..."
$PIP_CMD install -r requirements.txt

print_success "Dependencies installed successfully"

# Install development dependencies if requested
if [[ $INSTALL_DEV -eq 1 ]]; then
    if [[ -f "requirements-dev.txt" ]]; then
        print_info "Installing development dependencies..."
        $PIP_CMD install -r requirements-dev.txt
        print_success "Development dependencies installed"
    else
        print_warning "requirements-dev.txt not found. Skipping development dependencies."
    fi
fi

# Create config file if it doesn't exist
if [[ ! -f "config.yaml" ]]; then
    if [[ -f "config.example.yaml" ]]; then
        print_info "Creating config.yaml from config.example.yaml..."
        cp config.example.yaml config.yaml
        print_success "Config file created at ./config.yaml"
        print_warning "Please edit config.yaml with your database credentials"
        print_warning "Set DB_PASSWORD environment variable for production use"
    else
        print_warning "config.example.yaml not found. No config file created."
    fi
else
    print_info "Config file already exists at ./config.yaml"
fi

# Create logs directory
if [[ ! -d "logs" ]]; then
    print_info "Creating logs directory..."
    mkdir -p logs
    print_success "Logs directory created at ./logs"
fi

# Make scripts executable
print_info "Making scripts executable..."
chmod +x start_worker.sh 2>/dev/null || print_warning "Could not make start_worker.sh executable"
chmod +x install.sh 2>/dev/null || print_warning "Could not make install.sh executable"

# Check database connectivity
print_info "Checking database requirements..."

if command -v mysql &> /dev/null; then
    print_success "MySQL client found"
    print_info "You can test database connectivity with:"
    print_info "  mysql -h localhost -u ragnarok -p ragnarok"
else
    print_warning "MySQL client not found. Cannot test database connectivity."
    print_info "Make sure MySQL/MariaDB is installed and the database is accessible."
fi

# Print installation summary
echo ""
echo "============================================================================="
print_success "Installation Complete!"
echo "============================================================================="
echo ""
print_info "Next steps:"
echo ""
echo "1. Configure the service:"
if [[ $USE_VENV -eq 1 ]]; then
    echo "   - Edit config.yaml with your database credentials"
else
    echo "   - Edit $SCRIPT_DIR/config.yaml with your database credentials"
fi
echo "   - Or set environment variables (DB_HOST, DB_USER, DB_PASSWORD, etc.)"
echo ""
echo "2. Ensure database schema is created:"
echo "   - Run the SQL script: rathena-AI-world/sql-files/ai_ipc_tables.sql"
echo "   - mysql -u ragnarok -p ragnarok < ../../sql-files/ai_ipc_tables.sql"
echo ""
echo "3. Start the service:"
if [[ $USE_VENV -eq 1 ]]; then
    echo "   ./start_worker.sh"
else
    echo "   cd $SCRIPT_DIR && ./start_worker.sh"
fi
echo ""
echo "4. Monitor the logs:"
echo "   tail -f logs/ai_ipc_worker.log  (if file logging is enabled)"
echo "   or watch the console output"
echo ""
echo "For more information, see README.md"
echo ""

if [[ $USE_VENV -eq 1 ]]; then
    print_info "Virtual environment is at: $SCRIPT_DIR/venv"
    print_info "To activate it manually: source venv/bin/activate"
fi

echo ""
print_success "Ready to start! Run ./start_worker.sh"
echo ""
