# Installation Guide

This document provides detailed information about the automated installation script for the AI Autonomous World System.

## Quick Start

```bash
# Navigate to the project directory
cd rathena-AI-world/ai-autonomous-world

# Run the installation script
./install.sh
```

## Installation Script Features

### ✅ What Gets Installed

1. **PostgreSQL 17**
   - Official PostgreSQL APT repository
   - PostgreSQL 17 server and contrib packages
   - Extensions: TimescaleDB, Apache AGE, pgvector
   - Database: `ai_world_memory`
   - User: `ai_world_user` (with configurable password)

2. **DragonflyDB**
   - Latest stable version via official installation script
   - Configured as systemd service
   - Redis-compatible interface on port 6379

3. **Python Environment**
   - Python 3.11+ virtual environment
   - All dependencies from `requirements.txt`
   - System packages: python3-dev, build-essential, libpq-dev

4. **Configuration**
   - `.env` file created from `.env.example`
   - PostgreSQL credentials configured
   - Ready for LLM API key configuration

### 🛡️ Safety Features

- **Idempotent**: Safe to run multiple times
- **Dry-run mode**: Preview changes without installing
- **Error handling**: Automatic rollback on failures
- **Verbose logging**: All actions logged to `install.log`
- **Prerequisite checks**: Validates OS, internet, sudo access
- **Non-root execution**: Runs as regular user, prompts for sudo when needed

## Usage

### Basic Installation

```bash
./install.sh
```

This will install all components and prompt for PostgreSQL password.

### Dry Run (Preview Only)

```bash
./install.sh --dry-run
```

Shows what would be installed without making any changes.

### Skip Components

```bash
# Skip PostgreSQL (already installed)
./install.sh --skip-postgres

# Skip DragonflyDB (already installed)
./install.sh --skip-dragonfly

# Skip Python dependencies (already installed)
./install.sh --skip-python

# Skip configuration file creation
./install.sh --skip-config

# Combine multiple skip options
./install.sh --skip-postgres --skip-dragonfly
```

### Set Password via Environment Variable

```bash
POSTGRES_PASSWORD=your_secure_password ./install.sh
```

### Custom Database Configuration

```bash
POSTGRES_DB=my_database \
POSTGRES_USER=my_user \
POSTGRES_PASSWORD=my_password \
./install.sh
```

## Command Line Options

| Option | Description |
|--------|-------------|
| `--help` | Show help message and exit |
| `--dry-run` | Preview installation without making changes |
| `--skip-postgres` | Skip PostgreSQL installation |
| `--skip-dragonfly` | Skip DragonflyDB installation |
| `--skip-python` | Skip Python dependencies installation |
| `--skip-config` | Skip configuration file creation |

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `POSTGRES_DB` | `ai_world_memory` | PostgreSQL database name |
| `POSTGRES_USER` | `ai_world_user` | PostgreSQL user name |
| `POSTGRES_PASSWORD` | *(prompted)* | PostgreSQL password |

## Installation Steps

The script performs the following steps in order:

1. **Prerequisites Check**
   - Verify not running as root
   - Check Ubuntu version (24.04 recommended)
   - Verify internet connection
   - Verify sudo access

2. **PostgreSQL Installation**
   - Add PostgreSQL APT repository
   - Install PostgreSQL 17
   - Install extensions (TimescaleDB, pgvector, Apache AGE)
   - Create database and user
   - Grant privileges
   - Enable extensions

3. **DragonflyDB Installation**
   - Download and run official installation script
   - Start and enable systemd service

4. **Python Dependencies**
   - Install system packages (python3-dev, build-essential, libpq-dev)
   - Create Python virtual environment
   - Install all packages from requirements.txt

5. **Configuration Setup**
   - Create `.env` file from `.env.example`
   - Configure PostgreSQL credentials
   - Prompt for LLM API keys (manual step)

6. **Verification**
   - Check PostgreSQL service status
   - Check DragonflyDB service status
   - Verify Python virtual environment
   - Test PostgreSQL connection
   - Test Python imports

## Post-Installation

After the installation completes successfully:

1. **Configure LLM API Keys**
   ```bash
   cd ai-service
   nano .env  # or use your preferred editor
   ```
   
   Add at least one LLM provider API key:
   - `AZURE_OPENAI_API_KEY` (recommended)
   - `OPENAI_API_KEY`
   - `DEEPSEEK_API_KEY` (cost-effective alternative)
   - `ANTHROPIC_API_KEY`
   - `GOOGLE_API_KEY`

2. **Start the AI Service**
   ```bash
   cd ai-service
   source ../venv/bin/activate
   python main.py
   ```

3. **Verify the Service**
   ```bash
   # Check health endpoint
   curl http://localhost:8000/health
   
   # Open API documentation
   xdg-open http://localhost:8000/docs
   ```

## Troubleshooting

### Installation Fails

Check the log file for detailed error messages:
```bash
cat install.log
```

### PostgreSQL Connection Issues

Verify PostgreSQL is running:
```bash
sudo systemctl status postgresql@17-main
```

Test connection manually:
```bash
psql -h localhost -U ai_world_user -d ai_world_memory
```

### DragonflyDB Not Running

Start DragonflyDB manually:
```bash
sudo systemctl start dragonfly
```

Or run in foreground for debugging:
```bash
dragonfly --logtostderr
```

### Python Import Errors

Ensure virtual environment is activated:
```bash
source venv/bin/activate
```

Reinstall dependencies:
```bash
pip install -r ai-service/requirements.txt
```

## Uninstallation

To remove installed components:

```bash
# Remove PostgreSQL
sudo apt-get remove --purge postgresql-17 postgresql-contrib-17
sudo rm -rf /var/lib/postgresql/17

# Remove DragonflyDB
sudo systemctl stop dragonfly
sudo systemctl disable dragonfly
sudo rm /usr/local/bin/dragonfly

# Remove Python virtual environment
rm -rf venv

# Remove configuration
rm ai-service/.env
```

## Requirements

- **OS**: Ubuntu 24.04 LTS (tested), Ubuntu 22.04+ (should work)
- **RAM**: Minimum 8GB, recommended 16GB
- **Disk**: Minimum 10GB free space
- **Network**: Internet connection required for package downloads
- **Permissions**: Sudo access required

## Support

For issues or questions:
- Check the log file: `install.log`
- Review documentation: `docs/QUICK_START.md`
- Check configuration: `docs/CONFIGURATION.md`

