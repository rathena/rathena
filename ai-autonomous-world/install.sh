#!/usr/bin/env bash
#
# AI Autonomous World System - Complete Installation Script
# For Ubuntu 24.04 LTS
#
# This script automates the complete installation of the AI autonomous world system
# including PostgreSQL 17, DragonflyDB, Python dependencies, and configuration.
#
# IMPORTANT - DATABASE ARCHITECTURE:
# ==================================
# This script installs PostgreSQL for AI SERVICES ONLY.
# - PostgreSQL: Used EXCLUSIVELY by AI services (ai-world, p2p-coordinator [C++], NPC memory)
# - MariaDB: Used EXCLUSIVELY by rAthena game servers (installed separately)
#
# DO NOT use PostgreSQL for rAthena components!
# DO NOT use MariaDB for AI services!
#
# Usage:
#   ./install.sh              # Full installation
#   ./install.sh --dry-run    # Show what would be installed without installing
#   ./install.sh --help       # Show help message
#

set -euo pipefail  # Exit on error, undefined variables, and pipe failures

# ============================================================================
# CONFIGURATION
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AI_SERVICE_DIR="${SCRIPT_DIR}/ai-service"
VENV_DIR="${SCRIPT_DIR}/venv"
LOG_FILE="${SCRIPT_DIR}/install.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Installation flags
DRY_RUN=false
SKIP_POSTGRES=false
SKIP_DRAGONFLY=false
SKIP_PYTHON=false
SKIP_CONFIG=false

# Database configuration (will be prompted if not set)
POSTGRES_DB="${POSTGRES_DB:-ai_world_memory}"
POSTGRES_USER="${POSTGRES_USER:-ai_world_user}"
POSTGRES_PASSWORD="${POSTGRES_PASSWORD:-}"

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

log() {
    echo -e "${GREEN}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $*" | tee -a "${LOG_FILE}"
}

log_error() {
    echo -e "${RED}[$(date +'%Y-%m-%d %H:%M:%S')] ERROR:${NC} $*" | tee -a "${LOG_FILE}" >&2
}

log_warn() {
    echo -e "${YELLOW}[$(date +'%Y-%m-%d %H:%M:%S')] WARNING:${NC} $*" | tee -a "${LOG_FILE}"
}

log_info() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')] INFO:${NC} $*" | tee -a "${LOG_FILE}"
}

print_header() {
    echo ""
    echo -e "${BLUE}============================================================================${NC}"
    echo -e "${BLUE}$*${NC}"
    echo -e "${BLUE}============================================================================${NC}"
    echo ""
}

check_root() {
    if [[ $EUID -eq 0 ]]; then
        log_error "This script should NOT be run as root"
        log_error "It will prompt for sudo when needed"
        exit 1
    fi
}

check_ubuntu_version() {
    if [[ ! -f /etc/os-release ]]; then
        log_error "Cannot determine OS version"
        exit 1
    fi
    
    source /etc/os-release
    
    if [[ "${ID}" != "ubuntu" ]]; then
        log_error "This script is designed for Ubuntu only"
        log_error "Detected OS: ${ID}"
        exit 1
    fi
    
    if [[ "${VERSION_ID}" != "24.04" ]]; then
        log_warn "This script is tested on Ubuntu 24.04"
        log_warn "Detected version: ${VERSION_ID}"
        read -p "Continue anyway? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    log "✓ Ubuntu ${VERSION_ID} detected"
}

check_internet() {
    if ! ping -c 1 8.8.8.8 &> /dev/null; then
        log_error "No internet connection detected"
        exit 1
    fi
    log "✓ Internet connection verified"
}

check_sudo() {
    if ! sudo -n true 2>/dev/null; then
        log_info "This script requires sudo access"
        sudo -v || {
            log_error "Failed to obtain sudo access"
            exit 1
        }
    fi
    log "✓ Sudo access verified"
}

check_prerequisites() {
    print_header "Checking Prerequisites"
    check_root
    check_ubuntu_version
    check_internet
    check_sudo
}

# ============================================================================
# INSTALLATION FUNCTIONS
# ============================================================================

install_postgresql() {
    print_header "Installing PostgreSQL 17 (for AI Services ONLY)"

    log_info "PostgreSQL will be used EXCLUSIVELY for AI services"
    log_info "rAthena game servers use MariaDB (installed separately)"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install PostgreSQL 17"
        return 0
    fi

    # Check if PostgreSQL 17 is already installed
    if command -v psql &> /dev/null; then
        PG_VERSION=$(psql --version | grep -oP '\d+' | head -1)
        if [[ "${PG_VERSION}" == "17" ]]; then
            log "✓ PostgreSQL 17 already installed"
            return 0
        fi
    fi

    log "Installing PostgreSQL 17 for AI services..."

    # Install PostgreSQL APT repository
    sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh -y || {
        log_error "Failed to add PostgreSQL repository"
        return 1
    }

    # Update package list
    sudo apt-get update -qq

    # Install PostgreSQL 17
    sudo apt-get install -y postgresql-17 postgresql-contrib-17 || {
        log_error "Failed to install PostgreSQL 17"
        return 1
    }

    # Start and enable PostgreSQL
    sudo systemctl start postgresql@17-main
    sudo systemctl enable postgresql@17-main

    log "✓ PostgreSQL 17 installed successfully"
}

install_postgresql_extensions() {
    print_header "Installing PostgreSQL Extensions"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install PostgreSQL extensions"
        return 0
    fi

    log "Installing TimescaleDB..."
    sudo apt-get install -y postgresql-17-timescaledb || log_warn "TimescaleDB installation failed (optional)"

    log "Installing pgvector..."
    sudo apt-get install -y postgresql-17-pgvector || log_warn "pgvector installation failed (optional)"

    log "Installing Apache AGE..."
    sudo apt-get install -y postgresql-17-age || log_warn "Apache AGE installation failed (optional)"

    log "✓ PostgreSQL extensions installed"
}

setup_postgresql_database() {
    print_header "Setting Up PostgreSQL Database (for AI Services)"

    log_info "Creating database: ${POSTGRES_DB}"
    log_info "This database is for AI services ONLY (not for rAthena)"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would create database and user"
        return 0
    fi

    # Prompt for password if not set
    if [[ -z "${POSTGRES_PASSWORD}" ]]; then
        read -sp "Enter password for PostgreSQL user '${POSTGRES_USER}': " POSTGRES_PASSWORD
        echo
        if [[ -z "${POSTGRES_PASSWORD}" ]]; then
            log_error "Password cannot be empty"
            return 1
        fi
    fi

    log "Creating AI services database '${POSTGRES_DB}'..."
    sudo -u postgres psql -c "CREATE DATABASE ${POSTGRES_DB};" 2>/dev/null || log_info "Database already exists"

    log "Creating AI services user '${POSTGRES_USER}'..."
    sudo -u postgres psql -c "CREATE USER ${POSTGRES_USER} WITH PASSWORD '${POSTGRES_PASSWORD}';" 2>/dev/null || log_info "User already exists"

    log "Granting privileges..."
    sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE ${POSTGRES_DB} TO ${POSTGRES_USER};"

    # Enable extensions
    log "Enabling PostgreSQL extensions for AI services..."
    sudo -u postgres psql -d "${POSTGRES_DB}" -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;" 2>/dev/null || log_info "TimescaleDB not available"
    sudo -u postgres psql -d "${POSTGRES_DB}" -c "CREATE EXTENSION IF NOT EXISTS vector CASCADE;" 2>/dev/null || log_info "pgvector not available"
    sudo -u postgres psql -d "${POSTGRES_DB}" -c "CREATE EXTENSION IF NOT EXISTS age CASCADE;" 2>/dev/null || log_info "Apache AGE not available"

    log "✓ PostgreSQL database configured for AI services"
}

install_dragonfly() {
    print_header "Installing DragonflyDB"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install DragonflyDB"
        return 0
    fi

    # Check if DragonflyDB is already installed
    if command -v dragonfly &> /dev/null; then
        log "✓ DragonflyDB already installed"
        return 0
    fi

    log "Installing DragonflyDB..."

    # Download and run installation script
    curl -fsSL https://www.dragonflydb.io/install.sh | bash || {
        log_error "Failed to install DragonflyDB"
        return 1
    }

    # Start and enable DragonflyDB
    sudo systemctl start dragonfly 2>/dev/null || log_warn "Failed to start DragonflyDB service"
    sudo systemctl enable dragonfly 2>/dev/null || log_warn "Failed to enable DragonflyDB service"

    log "✓ DragonflyDB installed successfully"
}

install_python_dependencies() {
    print_header "Installing Python Dependencies"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install Python dependencies"
        return 0
    fi

    # Check Python version
    if ! command -v python3 &> /dev/null; then
        log_error "Python 3 not found"
        return 1
    fi

    PYTHON_VERSION=$(python3 --version | grep -oP '\d+\.\d+' | head -1)
    log "Python version: ${PYTHON_VERSION}"

    # Install system dependencies
    log "Installing system dependencies..."
    sudo apt-get install -y python3-pip python3-venv python3-dev build-essential libpq-dev || {
        log_error "Failed to install system dependencies"
        return 1
    }

    # Create virtual environment
    if [[ ! -d "${VENV_DIR}" ]]; then
        log "Creating Python virtual environment..."
        python3 -m venv "${VENV_DIR}" || {
            log_error "Failed to create virtual environment"
            return 1
        }
    else
        log "✓ Virtual environment already exists"
    fi

    # Activate virtual environment and install packages
    log "Installing Python packages..."
    source "${VENV_DIR}/bin/activate"

    pip install --upgrade pip setuptools wheel
    pip install -r "${AI_SERVICE_DIR}/requirements.txt" || {
        log_error "Failed to install Python packages"
        return 1
    }

    log "✓ Python dependencies installed"
}

setup_configuration() {
    print_header "Setting Up Configuration"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would create .env configuration file"
        return 0
    fi

    ENV_FILE="${AI_SERVICE_DIR}/.env"
    ENV_EXAMPLE="${AI_SERVICE_DIR}/.env.example"

    if [[ -f "${ENV_FILE}" ]]; then
        log_warn ".env file already exists"
        read -p "Overwrite existing .env file? (y/N) " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log "Keeping existing .env file"
            return 0
        fi
    fi

    if [[ ! -f "${ENV_EXAMPLE}" ]]; then
        log_error ".env.example not found"
        return 1
    fi

    log "Creating .env file from .env.example..."
    cp "${ENV_EXAMPLE}" "${ENV_FILE}"

    # Update PostgreSQL configuration
    sed -i "s/^POSTGRES_DB=.*/POSTGRES_DB=${POSTGRES_DB}/" "${ENV_FILE}"
    sed -i "s/^POSTGRES_USER=.*/POSTGRES_USER=${POSTGRES_USER}/" "${ENV_FILE}"
    sed -i "s/^POSTGRES_PASSWORD=.*/POSTGRES_PASSWORD=${POSTGRES_PASSWORD}/" "${ENV_FILE}"

    log "✓ Configuration file created: ${ENV_FILE}"
    log_warn "IMPORTANT: Edit ${ENV_FILE} to add your LLM API keys"
    log_warn "Required: At least one of AZURE_OPENAI_API_KEY, OPENAI_API_KEY, DEEPSEEK_API_KEY, etc."
}

verify_installation() {
    print_header "Verifying Installation"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would verify installation"
        return 0
    fi

    local errors=0

    # Check PostgreSQL
    if sudo systemctl is-active --quiet postgresql@17-main; then
        log "✓ PostgreSQL service is running"
    else
        log_error "PostgreSQL service is not running"
        ((errors++))
    fi

    # Check DragonflyDB
    if sudo systemctl is-active --quiet dragonfly 2>/dev/null || pgrep -x dragonfly > /dev/null; then
        log "✓ DragonflyDB is running"
    else
        log_warn "DragonflyDB service is not running (may need manual start)"
    fi

    # Check Python virtual environment
    if [[ -d "${VENV_DIR}" ]]; then
        log "✓ Python virtual environment exists"
    else
        log_error "Python virtual environment not found"
        ((errors++))
    fi

    # Check .env file
    if [[ -f "${AI_SERVICE_DIR}/.env" ]]; then
        log "✓ Configuration file exists"
    else
        log_warn "Configuration file not found"
    fi

    # Test PostgreSQL connection
    if sudo -u postgres psql -d "${POSTGRES_DB}" -c "SELECT 1;" &> /dev/null; then
        log "✓ PostgreSQL database accessible"
    else
        log_error "Cannot access PostgreSQL database"
        ((errors++))
    fi

    # Test Python imports
    source "${VENV_DIR}/bin/activate"
    if python3 -c "import fastapi, redis, sqlalchemy, psycopg2" 2>/dev/null; then
        log "✓ Python dependencies importable"
    else
        log_error "Python dependencies not properly installed"
        ((errors++))
    fi

    if [[ ${errors} -eq 0 ]]; then
        log "✓ All verification checks passed"
        return 0
    else
        log_error "${errors} verification check(s) failed"
        return 1
    fi
}

print_next_steps() {
    print_header "Installation Complete!"

    echo ""
    echo -e "${GREEN}✓ AI Autonomous World System installed successfully!${NC}"
    echo ""
    echo -e "${BLUE}Next Steps:${NC}"
    echo ""
    echo "1. Configure LLM API Keys:"
    echo "   Edit ${AI_SERVICE_DIR}/.env"
    echo "   Add at least one LLM provider API key (Azure OpenAI, OpenAI, DeepSeek, etc.)"
    echo ""
    echo "2. Start the AI Service:"
    echo "   cd ${AI_SERVICE_DIR}"
    echo "   source ../venv/bin/activate"
    echo "   python main.py"
    echo ""
    echo "3. Test the API:"
    echo "   curl http://localhost:8000/health"
    echo "   Open http://localhost:8000/docs in your browser"
    echo ""
    echo "4. Monitor Services:"
    echo "   PostgreSQL: sudo systemctl status postgresql@17-main"
    echo "   DragonflyDB: sudo systemctl status dragonfly"
    echo ""
    echo -e "${YELLOW}Documentation:${NC}"
    echo "   Quick Start: ${SCRIPT_DIR}/docs/QUICK_START.md"
    echo "   Configuration: ${SCRIPT_DIR}/docs/CONFIGURATION.md"
    echo ""
}

show_help() {
    cat << EOF
AI Autonomous World System - Installation Script

Usage: $0 [OPTIONS]

Options:
    --help              Show this help message
    --dry-run           Show what would be installed without installing
    --skip-postgres     Skip PostgreSQL installation
    --skip-dragonfly    Skip DragonflyDB installation
    --skip-python       Skip Python dependencies installation
    --skip-config       Skip configuration file creation

Environment Variables:
    POSTGRES_DB         PostgreSQL database name (default: ai_world_memory)
    POSTGRES_USER       PostgreSQL user name (default: ai_world_user)
    POSTGRES_PASSWORD   PostgreSQL password (will prompt if not set)

Examples:
    $0                          # Full installation
    $0 --dry-run                # Preview installation
    $0 --skip-postgres          # Skip PostgreSQL (already installed)
    POSTGRES_PASSWORD=secret $0 # Set password via environment

Requirements:
    - Ubuntu 24.04 LTS
    - Sudo access
    - Internet connection

EOF
}

# ============================================================================
# MAIN FUNCTION
# ============================================================================

main() {
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --help)
                show_help
                exit 0
                ;;
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            --skip-postgres)
                SKIP_POSTGRES=true
                shift
                ;;
            --skip-dragonfly)
                SKIP_DRAGONFLY=true
                shift
                ;;
            --skip-python)
                SKIP_PYTHON=true
                shift
                ;;
            --skip-config)
                SKIP_CONFIG=true
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # Initialize log file
    : > "${LOG_FILE}"

    print_header "AI Autonomous World System - Installation"

    if ${DRY_RUN}; then
        log_warn "DRY RUN MODE - No changes will be made"
    fi

    log "Installation started at $(date)"
    log "Script directory: ${SCRIPT_DIR}"
    log "Log file: ${LOG_FILE}"

    # Run installation steps
    check_prerequisites

    if ! ${SKIP_POSTGRES}; then
        install_postgresql
        install_postgresql_extensions
        setup_postgresql_database
    else
        log_info "Skipping PostgreSQL installation"
    fi

    if ! ${SKIP_DRAGONFLY}; then
        install_dragonfly
    else
        log_info "Skipping DragonflyDB installation"
    fi

    if ! ${SKIP_PYTHON}; then
        install_python_dependencies
    else
        log_info "Skipping Python dependencies installation"
    fi

    if ! ${SKIP_CONFIG}; then
        setup_configuration
    else
        log_info "Skipping configuration setup"
    fi

    if ! ${DRY_RUN}; then
        verify_installation
        print_next_steps
    fi

    log "Installation completed at $(date)"
    log "Full log available at: ${LOG_FILE}"
}

# Run main function
main "$@"

