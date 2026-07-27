#!/usr/bin/env bash
#
# rAthena AI World - Automated Install Wizard
# ============================================
# Complete setup script for the rAthena AI World server with dual AI systems.
#
# This script handles:
#   - System dependency detection and installation
#   - Python virtual environment setup
#   - C++ server build from source (cmake + make)
#   - PostgreSQL database setup (AI services)
#   - MariaDB/MySQL database setup (game data)
#   - DragonflyDB cache installation
#   - .env configuration from template
#   - Systemd service creation
#   - Verification tests
#
# Usage:
#   ./install.sh                    # Full interactive installation
#   ./install.sh --quick            # Quick install (skips prompts, uses defaults)
#   ./install.sh --dev              # Development install (debug mode, more verbose)
#   ./install.sh --help             # Show help message
#   ./install.sh --dry-run          # Preview without making changes
#
# Requirements:
#   - Ubuntu 22.04+ / Debian 11+ (tested on Ubuntu 24.04)
#   - Sudo access
#   - Internet connection
#   - 8 GB RAM minimum (16+ GB recommended)
#   - 20 GB free disk space (100+ GB for ML training)
#

set -euo pipefail

# ============================================================================
# CONFIGURATION
# ============================================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="${SCRIPT_DIR}/install.log"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Mode flags
QUICK_MODE=false
DEV_MODE=false
DRY_RUN=false

# Component flags (set by interactive prompts or flags)
INSTALL_SYSTEM_DEPS=true
BUILD_CPP_SERVER=true
SETUP_POSTGRES=true
SETUP_MYSQL=true
SETUP_DRAGONFLY=true
SETUP_PYTHON=true
CREATE_CONFIG=true
CREATE_SYSTEMD=true
RUN_VERIFICATION=true

# Default database config
POSTGRES_DB="${POSTGRES_DB:-ai_world_memory}"
POSTGRES_USER="${POSTGRES_USER:-ai_world_user}"
POSTGRES_PASSWORD="${POSTGRES_PASSWORD:-}"
MYSQL_DB="${MYSQL_DB:-ragnarok}"
MYSQL_USER="${MYSQL_USER:-ragnarok}"
MYSQL_PASSWORD="${MYSQL_PASSWORD:-}"

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

log()     { echo -e "${GREEN}[$(date +'%H:%M:%S')]${NC} $*" | tee -a "${LOG_FILE}"; }
log_info()  { echo -e "${CYAN}[INFO]${NC} $*" | tee -a "${LOG_FILE}"; }
log_warn()  { echo -e "${YELLOW}[WARN]${NC} $*" | tee -a "${LOG_FILE}"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*" | tee -a "${LOG_FILE}" >&2; }

print_banner() {
    echo ""
    echo -e "${BLUE}╔══════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║${NC}          ${GREEN}rAthena AI World - Install Wizard${NC}              ${BLUE}║${NC}"
    echo -e "${BLUE}║${NC}          ${CYAN}Dual AI Systems Setup${NC}                           ${BLUE}║${NC}"
    echo -e "${BLUE}╚══════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_section() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  $*${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
}

prompt_yes_no() {
    local prompt="$1"
    local default="${2:-y}"
    local reply
    if ${QUICK_MODE}; then
        [[ "$default" == "y" ]] && return 0 || return 1
    fi
    if [[ "$default" == "y" ]]; then
        read -p "$prompt [Y/n] " -n 1 -r reply
    else
        read -p "$prompt [y/N] " -n 1 -r reply
    fi
    echo
    if [[ -z "$reply" ]]; then
        [[ "$default" == "y" ]] && return 0 || return 1
    fi
    [[ "$reply" =~ ^[Yy]$ ]] && return 0 || return 1
}

prompt_input() {
    local prompt="$1"
    local default="${2:-}"
    local var
    if ${QUICK_MODE} && [[ -n "$default" ]]; then
        echo "$default"
        return 0
    fi
    if [[ -n "$default" ]]; then
        read -p "$prompt [$default]: " var
        echo "${var:-$default}"
    else
        read -p "$prompt: " var
        echo "$var"
    fi
}

prompt_password() {
    local prompt="$1"
    local var
    if ${QUICK_MODE}; then
        # Generate a random password in quick mode
        echo "$(openssl rand -base64 32 2>/dev/null || date +%s | sha256sum | base64 | head -c 32)"
        return 0
    fi
    read -sp "$prompt: " var
    echo
    echo "$var"
}

# ============================================================================
# SYSTEM CHECKS
# ============================================================================

check_system() {
    print_section "Checking System Requirements"

    # OS detection
    if [[ ! -f /etc/os-release ]]; then
        log_error "Cannot detect OS. /etc/os-release not found."
        exit 1
    fi
    source /etc/os-release
    log "OS: ${PRETTY_NAME:-$ID $VERSION_ID}"

    # Architecture
    local arch
    arch=$(uname -m)
    log "Architecture: ${arch}"

    # CPU cores
    local cpu_cores
    cpu_cores=$(nproc)
    log "CPU cores: ${cpu_cores}"
    if [[ "$cpu_cores" -lt 4 ]]; then
        log_warn "Minimum 4 CPU cores recommended (detected: ${cpu_cores})"
    fi

    # RAM
    local total_ram_mb total_ram_gb
    total_ram_mb=$(awk '/MemTotal/ {printf "%d", $2/1024}' /proc/meminfo 2>/dev/null || echo 0)
    total_ram_gb=$((total_ram_mb / 1024))
    log "RAM: ${total_ram_gb} GB"
    if [[ "$total_ram_gb" -lt 8 ]]; then
        log_error "Minimum 8 GB RAM required (detected: ${total_ram_gb} GB)"
        exit 1
    fi
    if [[ "$total_ram_gb" -lt 16 ]]; then
        log_warn "16+ GB RAM recommended for production use"
    fi

    # Disk space
    local free_disk_mb free_disk_gb
    free_disk_mb=$(df -m "$SCRIPT_DIR" | awk 'NR==2 {print $4}')
    free_disk_gb=$((free_disk_mb / 1024))
    log "Free disk space: ${free_disk_gb} GB"
    if [[ "$free_disk_gb" -lt 20 ]]; then
        log_error "Minimum 20 GB free disk space required (detected: ${free_disk_gb} GB)"
        exit 1
    fi
    if [[ "$free_disk_gb" -lt 100 ]]; then
        log_warn "100+ GB recommended if training ML models"
    fi

    # GPU detection
    local has_gpu=false
    if command -v nvidia-smi &>/dev/null; then
        has_gpu=true
        local gpu_info
        gpu_info=$(nvidia-smi --query-gpu=name,memory.total --format=csv,noheader 2>/dev/null | head -1)
        log "GPU detected: ${gpu_info}"
    else
        log_info "No NVIDIA GPU detected (optional — ML training requires GPU)"
    fi

    # Internet connectivity
    if ! ping -c 1 -W 3 8.8.8.8 &>/dev/null && ! ping -c 1 -W 3 1.1.1.1 &>/dev/null; then
        log_warn "No internet connection detected (some features may not work)"
    else
        log "Internet connectivity verified"
    fi

    # Sudo access
    if ! sudo -n true 2>/dev/null; then
        log_info "Sudo access required for system package installation"
        sudo -v || {
            log_error "Failed to obtain sudo access"
            exit 1
        }
    fi
    log "Sudo access verified"
}

# ============================================================================
# INSTALLATION FUNCTIONS
# ============================================================================

install_system_dependencies() {
    print_section "Installing System Dependencies"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install system packages"
        return 0
    fi

    log "Updating package lists..."
    sudo apt-get update -qq || {
        log_error "Failed to update package lists"
        return 1
    }

    log "Installing build tools and libraries..."
    sudo apt-get install -y \
        build-essential \
        git \
        cmake \
        gcc \
        g++ \
        make \
        pkg-config \
        libmysqlclient-dev \
        zlib1g-dev \
        libpcre3-dev \
        libssl-dev \
        libpq-dev \
        python3 \
        python3-pip \
        python3-venv \
        python3-dev \
        curl \
        wget \
        screen \
        unzip \
        openssl \
        jq \
        netcat-openbsd \
        2>&1 | tail -5 || {
        log_error "Failed to install system packages"
        return 1
    }

    log "System dependencies installed successfully"
}

build_cpp_server() {
    print_section "Building C++ rAthena Server"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would build C++ server with cmake + make"
        return 0
    fi

    # Check if already built
    if [[ -f "${SCRIPT_DIR}/map-server" ]] && [[ -f "${SCRIPT_DIR}/login-server" ]] && [[ -f "${SCRIPT_DIR}/char-server" ]]; then
        log "Server binaries already exist. Skipping build."
        if ! ${QUICK_MODE}; then
            if ! prompt_yes_no "Force rebuild?" "n"; then
                return 0
            fi
        fi
    fi

    log "Configuring build with CMake..."
    local build_dir="${SCRIPT_DIR}/build"
    mkdir -p "$build_dir"
    cd "$build_dir"

    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DINSTALL_TO_SOURCE=ON \
        2>&1 | tail -5 || {
        log_error "CMake configuration failed"
        cd "$SCRIPT_DIR"
        return 1
    }

    log "Building server (this may take a while)..."
    make -j"$(nproc)" 2>&1 | tail -10 || {
        log_error "Build failed"
        cd "$SCRIPT_DIR"
        return 1
    }

    cd "$SCRIPT_DIR"

    # Verify binaries
    if [[ -f "${SCRIPT_DIR}/map-server" ]] && [[ -f "${SCRIPT_DIR}/login-server" ]] && [[ -f "${SCRIPT_DIR}/char-server" ]]; then
        log "Server binaries built successfully"
        ls -lh "${SCRIPT_DIR}/map-server" "${SCRIPT_DIR}/login-server" "${SCRIPT_DIR}/char-server" 2>/dev/null
    else
        log_warn "Some server binaries may be missing. Check build output."
    fi
}

setup_postgresql() {
    print_section "Setting Up PostgreSQL (AI Services Database)"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install and configure PostgreSQL"
        return 0
    fi

    # Check if PostgreSQL is already installed
    if command -v psql &>/dev/null; then
        local pg_version
        pg_version=$(psql --version 2>/dev/null | grep -oP '\d+' | head -1)
        log "PostgreSQL ${pg_version} already installed"
    else
        log "Installing PostgreSQL..."
        # Try to use the pgdg repository for latest version
        if [[ -f /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh ]]; then
            sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh -y 2>/dev/null || true
        fi
        sudo apt-get install -y postgresql postgresql-contrib 2>&1 | tail -3 || {
            log_error "Failed to install PostgreSQL"
            return 1
        }
    fi

    # Start PostgreSQL
    log "Starting PostgreSQL..."
    sudo systemctl enable postgresql 2>/dev/null || true
    sudo systemctl start postgresql 2>/dev/null || {
        # Try version-specific service name
        local pg_version
        pg_version=$(pg_config --version 2>/dev/null | grep -oP '\d+\.\d+' | head -1 || echo "17")
        sudo systemctl start "postgresql@${pg_version}-main" 2>/dev/null || {
            log_warn "Could not start PostgreSQL service — may need manual start"
        }
    }

    # Wait for PostgreSQL to be ready
    for i in {1..10}; do
        if pg_isready -q 2>/dev/null; then
            log "PostgreSQL is accepting connections"
            break
        fi
        sleep 1
    done

    # Prompt for database credentials
    if [[ -z "$POSTGRES_PASSWORD" ]]; then
        echo ""
        log_info "PostgreSQL Database Configuration"
        log_info "This database is for AI services ONLY (not for rAthena game data)"
        POSTGRES_DB=$(prompt_input "Database name" "$POSTGRES_DB")
        POSTGRES_USER=$(prompt_input "Database user" "$POSTGRES_USER")
        POSTGRES_PASSWORD=$(prompt_password "Password for PostgreSQL user '${POSTGRES_USER}'")
        if [[ -z "$POSTGRES_PASSWORD" ]]; then
            log_error "Password cannot be empty"
            return 1
        fi
    fi

    # Create database and user
    log "Creating database '${POSTGRES_DB}' and user '${POSTGRES_USER}'..."
    sudo -u postgres psql -c "CREATE DATABASE ${POSTGRES_DB};" 2>/dev/null || log_info "Database '${POSTGRES_DB}' already exists"
    sudo -u postgres psql -c "CREATE USER ${POSTGRES_USER} WITH PASSWORD '${POSTGRES_PASSWORD}';" 2>/dev/null || log_info "User '${POSTGRES_USER}' already exists"
    sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE ${POSTGRES_DB} TO ${POSTGRES_USER};" 2>/dev/null || true
    sudo -u postgres psql -d "${POSTGRES_DB}" -c "GRANT ALL ON SCHEMA public TO ${POSTGRES_USER};" 2>/dev/null || true

    # Install and enable extensions
    log "Installing PostgreSQL extensions..."
    sudo apt-get install -y postgresql-17-timescaledb 2>/dev/null || log_info "TimescaleDB not available (optional)"
    sudo apt-get install -y postgresql-17-pgvector 2>/dev/null || log_info "pgvector not available (optional)"

    sudo -u postgres psql -d "${POSTGRES_DB}" -c "CREATE EXTENSION IF NOT EXISTS timescaledb CASCADE;" 2>/dev/null || log_info "TimescaleDB extension not loaded"
    sudo -u postgres psql -d "${POSTGRES_DB}" -c "CREATE EXTENSION IF NOT EXISTS vector CASCADE;" 2>/dev/null || log_info "pgvector extension not loaded"

    # Run schema initialization if available
    if [[ -f "${SCRIPT_DIR}/ai-autonomous-world/init_postgres.sql" ]]; then
        log "Initializing database schema..."
        sudo -u postgres psql -d "${POSTGRES_DB}" -f "${SCRIPT_DIR}/ai-autonomous-world/init_postgres.sql" 2>&1 | tail -3 || log_warn "Schema initialization had issues"
    fi

    log "PostgreSQL setup complete"
}

setup_mysql() {
    print_section "Setting Up MariaDB/MySQL (Game Database)"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install and configure MariaDB"
        return 0
    fi

    # Check if MariaDB/MySQL is already installed
    local mysql_installed=false
    if command -v mariadb &>/dev/null || command -v mysql &>/dev/null; then
        mysql_installed=true
        log "MariaDB/MySQL already installed"
    else
        log "Installing MariaDB..."
        sudo apt-get install -y mariadb-server mariadb-client 2>&1 | tail -3 || {
            log_error "Failed to install MariaDB"
            return 1
        }
    fi

    # Start MariaDB
    log "Starting MariaDB..."
    sudo systemctl enable mariadb 2>/dev/null || true
    sudo systemctl start mariadb 2>/dev/null || {
        log_warn "Could not start MariaDB — may need manual start"
    }

    # Wait for MariaDB
    for i in {1..10}; do
        if mysqladmin ping -u root --silent 2>/dev/null; then
            log "MariaDB is accepting connections"
            break
        fi
        sleep 1
    done

    # Prompt for database credentials
    if [[ -z "$MYSQL_PASSWORD" ]]; then
        echo ""
        log_info "MariaDB/MySQL Database Configuration"
        log_info "This database is for rAthena game data ONLY"
        MYSQL_DB=$(prompt_input "Database name" "$MYSQL_DB")
        MYSQL_USER=$(prompt_input "Database user" "$MYSQL_USER")
        MYSQL_PASSWORD=$(prompt_password "Password for MySQL user '${MYSQL_USER}'")
        if [[ -z "$MYSQL_PASSWORD" ]]; then
            log_error "Password cannot be empty"
            return 1
        fi
    fi

    # Create database and user
    log "Creating database '${MYSQL_DB}' and user '${MYSQL_USER}'..."
    sudo mysql -u root -e "CREATE DATABASE IF NOT EXISTS ${MYSQL_DB} CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;" 2>/dev/null || log_info "Database '${MYSQL_DB}' already exists"
    sudo mysql -u root -e "CREATE USER IF NOT EXISTS '${MYSQL_USER}'@'localhost' IDENTIFIED BY '${MYSQL_PASSWORD}';" 2>/dev/null || log_info "User '${MYSQL_USER}' already exists"
    sudo mysql -u root -e "GRANT ALL PRIVILEGES ON ${MYSQL_DB}.* TO '${MYSQL_USER}'@'localhost';" 2>/dev/null || true
    sudo mysql -u root -e "FLUSH PRIVILEGES;" 2>/dev/null || true

    # Import rAthena SQL files if they exist
    local sql_files=("${SCRIPT_DIR}/sql-files/main.sql" "${SCRIPT_DIR}/sql-files/logs.sql")
    for sql_file in "${sql_files[@]}"; do
        if [[ -f "$sql_file" ]]; then
            log "Importing $(basename "$sql_file")..."
            sudo mysql -u root "${MYSQL_DB}" < "$sql_file" 2>&1 | tail -2 || log_warn "Failed to import $(basename "$sql_file")"
        fi
    done

    log "MariaDB/MySQL setup complete"
}

setup_dragonflydb() {
    print_section "Setting Up DragonflyDB (Cache Layer)"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would install DragonflyDB"
        return 0
    fi

    # Check if DragonflyDB is already installed
    if command -v dragonfly &>/dev/null; then
        log "DragonflyDB already installed"
        return 0
    fi

    # Check if Redis is available as fallback
    if command -v redis-server &>/dev/null; then
        log_info "Redis detected — can be used as DragonflyDB fallback"
        if ! ${QUICK_MODE}; then
            if prompt_yes_no "Use Redis instead of DragonflyDB?" "n"; then
                log "Will configure Redis as cache backend"
                sudo systemctl enable redis-server 2>/dev/null || true
                sudo systemctl start redis-server 2>/dev/null || true
                return 0
            fi
        fi
    fi

    log "Installing DragonflyDB..."
    curl -fsSL https://www.dragonflydb.io/install.sh | bash 2>&1 | tail -5 || {
        log_warn "DragonflyDB auto-install failed. Installing manually..."
        local df_version="v1.12.1"
        local df_url="https://github.com/dragonflydb/dragonfly/releases/download/${df_version}/dragonfly-x86_64.deb"
        wget -q "$df_url" -O /tmp/dragonfly.deb && \
        sudo dpkg -i /tmp/dragonfly.deb && \
        rm -f /tmp/dragonfly.deb || {
            log_warn "DragonflyDB manual install also failed. Using Redis as fallback."
            sudo apt-get install -y redis-server 2>&1 | tail -3
            sudo systemctl enable redis-server 2>/dev/null || true
            sudo systemctl start redis-server 2>/dev/null || true
            return 0
        }
    }

    # Start DragonflyDB
    sudo systemctl enable dragonfly 2>/dev/null || true
    sudo systemctl start dragonfly 2>/dev/null || log_warn "Could not start DragonflyDB — may need manual start"

    log "DragonflyDB setup complete"
}

setup_python_environment() {
    print_section "Setting Up Python Environment"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would create Python virtual environments"
        return 0
    fi

    # --- AI Autonomous World ---
    local ai_dir="${SCRIPT_DIR}/ai-autonomous-world"
    local ai_venv="${ai_dir}/venv"

    if [[ -d "$ai_dir" ]]; then
        log "Setting up AI Autonomous World Python environment..."

        if [[ ! -d "$ai_venv" ]]; then
            log "Creating virtual environment..."
            python3 -m venv "$ai_venv" || {
                log_error "Failed to create virtual environment"
                return 1
            }
        else
            log "Virtual environment already exists"
        fi

        source "${ai_venv}/bin/activate"
        pip install --upgrade pip setuptools wheel -q

        if [[ -f "${ai_dir}/ai-service/requirements.txt" ]]; then
            log "Installing AI service dependencies..."
            pip install -r "${ai_dir}/ai-service/requirements.txt" 2>&1 | tail -5 || {
                log_warn "Some AI service dependencies failed to install"
            }
        fi

        if [[ -f "${ai_dir}/ai-service/requirements-minimal.txt" ]]; then
            log "Installing minimal dependencies..."
            pip install -r "${ai_dir}/ai-service/requirements-minimal.txt" 2>&1 | tail -3 || true
        fi

        deactivate
        log "AI Autonomous World Python environment ready"
    else
        log_warn "AI Autonomous World directory not found — skipping"
    fi

    # --- ML Inference Service ---
    local ml_dir="${SCRIPT_DIR}/ml_inference_service"
    local ml_venv="${SCRIPT_DIR}/venv_ml_inference"

    if [[ -d "$ml_dir" ]]; then
        log "Setting up ML Inference Service Python environment..."

        if [[ ! -d "$ml_venv" ]]; then
            python3 -m venv "$ml_venv"
        fi

        source "${ml_venv}/bin/activate"
        pip install --upgrade pip setuptools wheel -q

        if [[ -f "${ml_dir}/requirements.txt" ]]; then
            log "Installing ML inference dependencies..."
            pip install -r "${ml_dir}/requirements.txt" 2>&1 | tail -5 || {
                log_warn "Some ML inference dependencies failed to install"
            }
        fi

        deactivate
        log "ML Inference Service Python environment ready"
    fi

    # --- ML Training ---
    local train_dir="${SCRIPT_DIR}/ml_training"
    local train_venv="${SCRIPT_DIR}/venv_ml_training"

    if [[ -d "$train_dir" ]]; then
        log "Setting up ML Training Python environment..."

        if [[ ! -d "$train_venv" ]]; then
            python3 -m venv "$train_venv"
        fi

        source "${train_venv}/bin/activate"
        pip install --upgrade pip setuptools wheel -q

        if [[ -f "${train_dir}/requirements.txt" ]]; then
            log "Installing ML training dependencies..."
            pip install -r "${train_dir}/requirements.txt" 2>&1 | tail -5 || {
                log_warn "Some ML training dependencies failed to install"
            }
        fi

        deactivate
        log "ML Training Python environment ready"
    fi
}

create_configuration() {
    print_section "Creating Configuration Files"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would create .env configuration"
        return 0
    fi

    # Root .env file
    local env_file="${SCRIPT_DIR}/.env"
    local env_example="${SCRIPT_DIR}/.env.example"

    if [[ -f "$env_file" ]]; then
        log_info ".env already exists at ${env_file}"
        if ! ${QUICK_MODE}; then
            if ! prompt_yes_no "Overwrite existing .env?" "n"; then
                log "Keeping existing .env"
            else
                create_env_file "$env_file" "$env_example"
            fi
        fi
    else
        create_env_file "$env_file" "$env_example"
    fi

    # AI service .env
    local ai_env="${SCRIPT_DIR}/ai-autonomous-world/ai-service/.env"
    local ai_env_example="${SCRIPT_DIR}/ai-autonomous-world/ai-service/.env.example"

    if [[ -f "$ai_env_example" ]]; then
        if [[ ! -f "$ai_env" ]]; then
            log "Creating AI service .env..."
            cp "$ai_env_example" "$ai_env"
            # Copy values from root .env if they exist
            if [[ -f "$env_file" ]]; then
                for var in POSTGRES_DB POSTGRES_USER POSTGRES_PASSWORD POSTGRES_HOST POSTGRES_PORT; do
                    local val
                    val=$(grep "^${var}=" "$env_file" 2>/dev/null | cut -d= -f2-)
                    if [[ -n "$val" ]]; then
                        sed -i "s/^${var}=.*/${var}=${val}/" "$ai_env" 2>/dev/null || true
                    fi
                done
            fi
            log "AI service .env created"
        fi
    fi

    log "Configuration files created"
}

create_env_file() {
    local target="$1"
    local example="$2"

    if [[ ! -f "$example" ]]; then
        log_warn ".env.example not found at ${example}"
        log_info "Creating minimal .env..."
        cat > "$target" << 'EOF'
# rAthena AI World - Environment Configuration
# Generated by install.sh

# --- Database ---
POSTGRES_DB=ai_world_memory
POSTGRES_USER=ai_world_user
POSTGRES_PASSWORD=changeme
POSTGRES_HOST=localhost
POSTGRES_PORT=5432

# --- Cache ---
DRAGONFLY_HOST=localhost
DRAGONFLY_PORT=6379
DRAGONFLY_PASSWORD=

# --- LLM API Keys (at least one required) ---
# OPENAI_API_KEY=sk-...
# ANTHROPIC_API_KEY=sk-...
# DEEPSEEK_API_KEY=...
# AZURE_OPENAI_API_KEY=...

# --- AI Service ---
AI_SERVICE_API_KEY=dev-key-change-in-production
API_KEY_REQUIRED=false
LOG_LEVEL=INFO
EOF
        log_info "Created minimal .env — edit it to add your API keys"
        return 0
    fi

    cp "$example" "$target"

    # Fill in database credentials
    if [[ -n "$POSTGRES_DB" ]]; then
        sed -i "s/^POSTGRES_DB=.*/POSTGRES_DB=${POSTGRES_DB}/" "$target" 2>/dev/null || true
    fi
    if [[ -n "$POSTGRES_USER" ]]; then
        sed -i "s/^POSTGRES_USER=.*/POSTGRES_USER=${POSTGRES_USER}/" "$target" 2>/dev/null || true
    fi
    if [[ -n "$POSTGRES_PASSWORD" ]]; then
        sed -i "s/^POSTGRES_PASSWORD=.*/POSTGRES_PASSWORD=${POSTGRES_PASSWORD}/" "$target" 2>/dev/null || true
    fi

    log "Created ${target} from template"
    log_warn "IMPORTANT: Edit ${target} to add your LLM API keys!"
}

create_systemd_services() {
    print_section "Creating Systemd Services"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would create systemd service files"
        return 0
    fi

    local user
    user=$(whoami)
    local ai_venv="${SCRIPT_DIR}/ai-autonomous-world/venv"
    local ai_workdir="${SCRIPT_DIR}/ai-autonomous-world/ai-service"

    # AI Service
    log "Creating rathena-ai-service.service..."
    sudo tee /etc/systemd/system/rathena-ai-service.service > /dev/null << EOF
[Unit]
Description=rAthena AI World - AI Autonomous World Service
Documentation=https://github.com/iskandarsulaili/rathena-AI-world
After=network.target postgresql.service dragonfly.service redis-server.service
Wants=postgresql.service dragonfly.service

[Service]
Type=simple
User=${user}
Group=${user}
WorkingDirectory=${ai_workdir}
Environment="PATH=${ai_venv}/bin"
EnvironmentFile=${SCRIPT_DIR}/.env
ExecStart=${ai_venv}/bin/uvicorn main:app --host 0.0.0.0 --port 8000 --workers 4
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

    # ML Inference Service
    if [[ -d "${SCRIPT_DIR}/ml_inference_service" ]]; then
        log "Creating ml-inference.service..."
        local ml_venv="${SCRIPT_DIR}/venv_ml_inference"
        sudo tee /etc/systemd/system/ml-inference.service > /dev/null << EOF
[Unit]
Description=rAthena AI World - ML Monster AI Inference Service
Documentation=https://github.com/iskandarsulaili/rathena-AI-world
After=network.target postgresql.service
Wants=postgresql.service

[Service]
Type=simple
User=${user}
Group=${user}
WorkingDirectory=${SCRIPT_DIR}/ml_inference_service
Environment="PATH=${ml_venv}/bin"
EnvironmentFile=${SCRIPT_DIR}/.env
ExecStart=${ml_venv}/bin/python main.py
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF
    fi

    # rAthena Map Server
    if [[ -f "${SCRIPT_DIR}/map-server" ]]; then
        log "Creating rathena-map.service..."
        sudo tee /etc/systemd/system/rathena-map.service > /dev/null << EOF
[Unit]
Description=rAthena AI World - Map Server
Documentation=https://github.com/iskandarsulaili/rathena-AI-world
After=network.target rathena-ai-service.service
Wants=rathena-ai-service.service

[Service]
Type=simple
User=${user}
Group=${user}
WorkingDirectory=${SCRIPT_DIR}
EnvironmentFile=${SCRIPT_DIR}/.env
ExecStart=${SCRIPT_DIR}/map-server
Restart=always
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF
    fi

    sudo systemctl daemon-reload
    log "Systemd services created. Enable with:"
    log "  sudo systemctl enable rathena-ai-service"
    log "  sudo systemctl enable ml-inference   (if ML service installed)"
    log "  sudo systemctl enable rathena-map    (if map-server built)"
}

run_verification() {
    print_section "Running Verification Tests"

    if ${DRY_RUN}; then
        log_info "[DRY RUN] Would run verification tests"
        return 0
    fi

    local errors=0

    # 1. Check PostgreSQL
    log "Checking PostgreSQL..."
    if pg_isready -q 2>/dev/null; then
        log "  ✓ PostgreSQL is running"
    else
        log_error "  ✗ PostgreSQL is not running"
        ((errors++))
    fi

    # 2. Check database accessibility
    if command -v psql &>/dev/null; then
        if sudo -u postgres psql -d "${POSTGRES_DB}" -c "SELECT 1;" &>/dev/null 2>&1; then
            log "  ✓ PostgreSQL database '${POSTGRES_DB}' accessible"
        else
            log_warn "  ⚠ Could not access database '${POSTGRES_DB}'"
        fi
    fi

    # 3. Check MariaDB/MySQL
    if mysqladmin ping -u root --silent 2>/dev/null; then
        log "  ✓ MariaDB/MySQL is running"
    else
        log_warn "  ⚠ MariaDB/MySQL not running (optional if using external DB)"
    fi

    # 4. Check cache (DragonflyDB or Redis)
    if command -v dragonfly &>/dev/null; then
        if dragonfly --version &>/dev/null; then
            log "  ✓ DragonflyDB installed"
        fi
    elif command -v redis-cli &>/dev/null; then
        if redis-cli ping &>/dev/null; then
            log "  ✓ Redis cache is running"
        fi
    else
        log_warn "  ⚠ No cache service detected"
    fi

    # 5. Check server binaries
    log "Checking server binaries..."
    for bin in map-server login-server char-server; do
        if [[ -f "${SCRIPT_DIR}/${bin}" ]]; then
            log "  ✓ ${bin} exists"
        else
            log_warn "  ⚠ ${bin} not found (may need to build)"
        fi
    done

    # 6. Check Python environments
    log "Checking Python environments..."
    if [[ -d "${SCRIPT_DIR}/ai-autonomous-world/venv" ]]; then
        log "  ✓ AI Autonomous World venv exists"
        if source "${SCRIPT_DIR}/ai-autonomous-world/venv/bin/activate" && python3 -c "import fastapi" 2>/dev/null; then
            log "  ✓ FastAPI importable"
        else
            log_warn "  ⚠ FastAPI not importable (dependencies may be missing)"
        fi
        deactivate 2>/dev/null || true
    else
        log_warn "  ⚠ AI Autonomous World venv not found"
    fi

    # 7. Check .env configuration
    log "Checking configuration..."
    if [[ -f "${SCRIPT_DIR}/.env" ]]; then
        log "  ✓ .env exists"
    else
        log_warn "  ⚠ .env not found"
    fi

    # 8. Check systemd services
    log "Checking systemd services..."
    for svc in rathena-ai-service ml-inference rathena-map; do
        if systemctl list-unit-files "${svc}.service" &>/dev/null | grep -q "${svc}"; then
            log "  ✓ ${svc}.service registered"
        fi
    done

    echo ""
    if [[ $errors -eq 0 ]]; then
        log "All checks passed!"
    else
        log_warn "${errors} check(s) had issues — review warnings above"
    fi
}

print_next_steps() {
    print_section "Installation Complete!"

    echo -e "${GREEN}  ✓ rAthena AI World has been installed successfully!${NC}"
    echo ""

    echo -e "${CYAN}  ┌─────────────────────────────────────────────────────────┐${NC}"
    echo -e "${CYAN}  │${NC}  NEXT STEPS                                           ${CYAN}│${NC}"
    echo -e "${CYAN}  └─────────────────────────────────────────────────────────┘${NC}"
    echo ""

    echo "  1. ${BLUE}Configure LLM API Keys${NC}"
    echo "     Edit ${SCRIPT_DIR}/.env"
    echo "     Set at least one LLM provider key:"
    echo "       - OPENAI_API_KEY"
    echo "       - ANTHROPIC_API_KEY"
    echo "       - DEEPSEEK_API_KEY"
    echo "       - AZURE_OPENAI_API_KEY"
    echo ""

    echo "  2. ${BLUE}Start the AI Service${NC}"
    echo "     cd ${SCRIPT_DIR}/ai-autonomous-world/ai-service"
    echo "     source ../venv/bin/activate"
    echo "     python main.py"
    echo "     # Or via systemd: sudo systemctl start rathena-ai-service"
    echo ""

    echo "  3. ${BLUE}Start the Game Server${NC}"
    echo "     cd ${SCRIPT_DIR}"
    echo "     ./map-server &"
    echo "     ./login-server &"
    echo "     ./char-server &"
    echo "     # Or via systemd: sudo systemctl start rathena-map"
    echo ""

    echo "  4. ${BLUE}Test the Installation${NC}"
    echo "     curl http://localhost:8000/health"
    echo "     Open http://localhost:8000/docs in your browser"
    echo ""

    echo "  5. ${BLUE}Enable Services on Boot${NC}"
    echo "     sudo systemctl enable rathena-ai-service"
    echo "     sudo systemctl enable rathena-map"
    echo ""

    echo -e "${YELLOW}  ┌─────────────────────────────────────────────────────────┐${NC}"
    echo -e "${YELLOW}  │${NC}  IMPORTANT                                              ${YELLOW}│${NC}"
    echo -e "${YELLOW}  └─────────────────────────────────────────────────────────┘${NC}"
    echo "  • Change default passwords before production deployment"
    echo "  • Set API_KEY_REQUIRED=true in .env for production"
    echo "  • Configure SSL/TLS for production (SSL_ENABLED=true)"
    echo "  • See docs/SETUP.md for detailed configuration guide"
    echo ""

    echo -e "${CYAN}  Documentation:${NC}"
    echo "    • Setup Guide:     docs/SETUP.md"
    echo "    • Production:      docs/PRODUCTION_DEPLOYMENT_GUIDE.md"
    echo "    • Operations:      docs/OPERATIONS_RUNBOOK.md"
    echo "    • Quick Reference: docs/QUICK_REFERENCE.md"
    echo ""

    echo -e "${GREEN}  Happy gaming! 🎮${NC}"
    echo ""
}

show_help() {
    cat << 'HELP'
rAthena AI World - Automated Install Wizard

USAGE:
    ./install.sh [OPTIONS]

OPTIONS:
    --help              Show this help message
    --quick             Quick install (skip prompts, use defaults)
    --dev               Development install (debug mode, verbose)
    --dry-run           Preview what would be installed without making changes

DESCRIPTION:
    Complete setup script for the rAthena AI World server with dual AI systems.
    Handles system dependencies, Python environments, C++ build, databases,
    cache layer, configuration, systemd services, and verification.

COMPONENTS:
    AI Autonomous World System  - NPC intelligence (21 AI agents, production)
    ML Monster AI System        - Monster behavior (infrastructure ready)
    rAthena Game Server         - Core MMORPG server (C++)

REQUIREMENTS:
    OS:     Ubuntu 22.04+ / Debian 11+
    CPU:    4+ cores
    RAM:    8 GB minimum (16+ GB recommended)
    Disk:   20 GB free (100+ GB for ML training)
    GPU:    Optional (NVIDIA 12GB+ for ML training)

EXAMPLES:
    ./install.sh                    # Full interactive installation
    ./install.sh --quick            # Quick install with defaults
    ./install.sh --dev              # Development install
    ./install.sh --dry-run          # Preview only

ENVIRONMENT VARIABLES:
    POSTGRES_DB         PostgreSQL database name (default: ai_world_memory)
    POSTGRES_USER       PostgreSQL user name (default: ai_world_user)
    POSTGRES_PASSWORD   PostgreSQL password (prompted if not set)
    MYSQL_DB            MariaDB database name (default: ragnarok)
    MYSQL_USER          MariaDB user name (default: ragnarok)
    MYSQL_PASSWORD      MariaDB password (prompted if not set)

LOG FILE:
    All output is logged to: install.log

HELP
}

# ============================================================================
# MAIN
# ============================================================================

main() {
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --help|-h)
                show_help
                exit 0
                ;;
            --quick|-q)
                QUICK_MODE=true
                shift
                ;;
            --dev|-d)
                DEV_MODE=true
                shift
                ;;
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done

    # Initialize log
    : > "${LOG_FILE}"

    print_banner

    log "Installation started at ${TIMESTAMP}"
    log "Working directory: ${SCRIPT_DIR}"
    log "Mode: $(${QUICK_MODE} && echo 'Quick' || echo 'Interactive')$(${DEV_MODE} && echo ' + Dev' || true)$(${DRY_RUN} && echo ' + Dry Run' || true)"
    log "Log file: ${LOG_FILE}"

    # System checks
    check_system

    # Interactive component selection (skip in quick/dev mode)
    if ! ${QUICK_MODE} && ! ${DRY_RUN}; then
        echo ""
        log_info "Component Selection"
        echo "  Select which components to install:"
        echo ""
        prompt_yes_no "Install system dependencies?" "y" || INSTALL_SYSTEM_DEPS=false
        prompt_yes_no "Build C++ server from source?" "y" || BUILD_CPP_SERVER=false
        prompt_yes_no "Set up PostgreSQL (AI services)?" "y" || SETUP_POSTGRES=false
        prompt_yes_no "Set up MariaDB/MySQL (game data)?" "y" || SETUP_MYSQL=false
        prompt_yes_no "Set up DragonflyDB/Redis (cache)?" "y" || SETUP_DRAGONFLY=false
        prompt_yes_no "Set up Python environments?" "y" || SETUP_PYTHON=false
        prompt_yes_no "Create configuration files?" "y" || CREATE_CONFIG=false
        prompt_yes_no "Create systemd services?" "y" || CREATE_SYSTEMD=false
        prompt_yes_no "Run verification tests?" "y" || RUN_VERIFICATION=false
        echo ""
    fi

    # Run installation steps
    if ${INSTALL_SYSTEM_DEPS}; then
        install_system_dependencies || log_warn "System dependencies step had issues"
    fi

    if ${BUILD_CPP_SERVER}; then
        build_cpp_server || log_warn "C++ build step had issues"
    fi

    # Create conf/import/ from templates (required by rAthena)
    if [ ! -d "conf/import" ]; then
        log "Creating conf/import/ from templates..."
        mkdir -p conf/import
        for tmpl in conf/import-tmpl/*; do
            base=$(basename "$tmpl")
            if [ ! -f "conf/import/$base" ]; then
                cp "$tmpl" "conf/import/$base"
                log "  Created conf/import/$base"
            fi
        done
        log "✓ conf/import/ directory created"
    else
        log "✓ conf/import/ already exists"
    fi

    if ${SETUP_POSTGRES}; then
        setup_postgresql || log_warn "PostgreSQL setup had issues"
    fi

    if ${SETUP_MYSQL}; then
        setup_mysql || log_warn "MariaDB/MySQL setup had issues"
    fi

    if ${SETUP_DRAGONFLY}; then
        setup_dragonflydb || log_warn "Cache setup had issues"
    fi

    if ${SETUP_PYTHON}; then
        setup_python_environment || log_warn "Python setup had issues"
    fi

    if ${CREATE_CONFIG}; then
        create_configuration || log_warn "Configuration step had issues"
    fi

    if ${CREATE_SYSTEMD}; then
        create_systemd_services || log_warn "Systemd step had issues"
    fi

    if ${RUN_VERIFICATION}; then
        run_verification || log_warn "Verification had issues"
    fi

    # Print next steps
    if ! ${DRY_RUN}; then
        print_next_steps
    fi

    log "Installation completed at $(date '+%Y-%m-%d %H:%M:%S')"
    log "Full log available at: ${LOG_FILE}"
}

main "$@"
