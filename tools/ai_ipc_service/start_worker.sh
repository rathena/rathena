#!/bin/bash
# =============================================================================
# AI IPC Service - Start Script
# =============================================================================
# This script starts the AI IPC polling worker service.
#
# Usage:
#   ./start_worker.sh [options]
#
# Options:
#   --config PATH    Specify custom config file (default: config.yaml)
#   --debug          Run with DEBUG log level
#   --help           Show this help message
#
# Environment Variables:
#   DB_HOST          Database host (overrides config)
#   DB_PORT          Database port (overrides config)
#   DB_USER          Database user (overrides config)
#   DB_PASSWORD      Database password (overrides config)
#   DB_NAME          Database name (overrides config)
#   LOG_LEVEL        Log level (DEBUG, INFO, WARNING, ERROR)
#   LOG_FORMAT       Log format (json, text)
#
# =============================================================================

set -e  # Exit on error

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Default values
CONFIG_FILE="config.yaml"
PYTHON_CMD="python3"
DEBUG_MODE=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --config)
            CONFIG_FILE="$2"
            shift 2
            ;;
        --debug)
            DEBUG_MODE=1
            export LOG_LEVEL=DEBUG
            shift
            ;;
        --help)
            echo "AI IPC Service - Start Script"
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --config PATH    Specify custom config file (default: config.yaml)"
            echo "  --debug          Run with DEBUG log level"
            echo "  --help           Show this help message"
            echo ""
            echo "Environment Variables:"
            echo "  DB_HOST          Database host"
            echo "  DB_PORT          Database port"
            echo "  DB_USER          Database user"
            echo "  DB_PASSWORD      Database password"
            echo "  DB_NAME          Database name"
            echo "  LOG_LEVEL        Log level (DEBUG, INFO, WARNING, ERROR)"
            echo "  LOG_FORMAT       Log format (json, text)"
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
echo "  AI IPC Service - Python Database Polling Worker"
echo "============================================================================="
echo ""

# Check if Python is installed
if ! command -v $PYTHON_CMD &> /dev/null; then
    print_error "Python 3 not found. Please install Python 3.9 or later."
    exit 1
fi

# Check Python version
PYTHON_VERSION=$($PYTHON_CMD --version 2>&1 | awk '{print $2}')
print_info "Python version: $PYTHON_VERSION"

# Check if config file exists
if [[ ! -f "$CONFIG_FILE" ]]; then
    print_warning "Config file not found: $CONFIG_FILE"
    if [[ -f "config.example.yaml" ]]; then
        print_info "Copying config.example.yaml to $CONFIG_FILE"
        cp config.example.yaml "$CONFIG_FILE"
        print_warning "Please edit $CONFIG_FILE with your database credentials"
        print_warning "Especially set DB_PASSWORD for production use"
    else
        print_error "No configuration file available"
        exit 1
    fi
fi

# Check if virtual environment exists
if [[ -d "venv" ]]; then
    print_info "Virtual environment found. Activating..."
    source venv/bin/activate
    print_success "Virtual environment activated"
elif [[ -d "../../../venv" ]]; then
    print_info "Project-level virtual environment found. Activating..."
    source ../../../venv/bin/activate
    print_success "Virtual environment activated"
else
    print_warning "No virtual environment found. Using system Python."
    print_warning "Consider creating one with: python3 -m venv venv"
fi

# Check if dependencies are installed
print_info "Checking dependencies..."
if ! $PYTHON_CMD -c "import aiomysql" &> /dev/null; then
    print_warning "Dependencies not installed. Run ./install.sh first."
    print_info "Attempting to install now..."
    if [[ -f "requirements.txt" ]]; then
        pip install -r requirements.txt
    else
        print_error "requirements.txt not found"
        exit 1
    fi
fi

# Check database connectivity (basic check)
if [[ -n "$DB_HOST" ]] && [[ -n "$DB_USER" ]] && command -v mysql &> /dev/null; then
    print_info "Testing database connectivity..."
    if mysql -h"${DB_HOST:-localhost}" -u"${DB_USER:-ragnarok}" -p"${DB_PASSWORD:-ragnarok}" -e "SELECT 1" "${DB_NAME:-ragnarok}" &> /dev/null; then
        print_success "Database connection OK"
    else
        print_warning "Database connection test failed. Service may not start correctly."
    fi
fi

# Create logs directory if it doesn't exist
if [[ ! -d "logs" ]]; then
    print_info "Creating logs directory..."
    mkdir -p logs
fi

# Print configuration
print_info "Configuration:"
print_info "  Config file: $CONFIG_FILE"
print_info "  Working directory: $SCRIPT_DIR"
if [[ $DEBUG_MODE -eq 1 ]]; then
    print_info "  Log level: DEBUG"
fi

# Export config file path
export AI_IPC_CONFIG="$CONFIG_FILE"

echo ""
echo "============================================================================="
print_info "Starting AI IPC Service..."
echo "============================================================================="
echo ""
print_info "Press Ctrl+C to stop the service gracefully"
echo ""

# Start the service
# Use exec to replace the shell process with Python process
# This ensures signals (SIGTERM, SIGINT) are properly forwarded
exec $PYTHON_CMD -m ai_ipc_service.main --config "$CONFIG_FILE"
