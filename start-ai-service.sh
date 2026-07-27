#!/bin/bash
# ========================================
# AI Service Startup Script
# ========================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
AI_DIR="${SCRIPT_DIR}/ai-autonomous-world/ai-service"
VENV_DIR="${SCRIPT_DIR}/.venv"

echo "Starting AI Service..."
echo "  Script dir: ${SCRIPT_DIR}"
echo "  AI dir: ${AI_DIR}"
echo "  Venv: ${VENV_DIR}"

cd "$AI_DIR"

# Activate virtual environment
source "${VENV_DIR}/bin/activate"

# Start the service
python main.py
