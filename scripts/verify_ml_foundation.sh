#!/bin/bash
# ML Monster AI Foundation Verification Script
# Version: 2.0
# Date: 2026-01-21
# Purpose: Verify all foundation components are properly installed and configured

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0
WARNINGS=0

# Helper functions
check_pass() {
    echo -e "${GREEN}✓ PASSED${NC}: $1"
    ((PASSED++))
}

check_fail() {
    echo -e "${RED}✗ FAILED${NC}: $1"
    ((FAILED++))
}

check_warn() {
    echo -e "${YELLOW}⚠ WARNING${NC}: $1"
    ((WARNINGS++))
}

echo "============================================================================="
echo "ML MONSTER AI FOUNDATION VERIFICATION"
echo "============================================================================="
echo ""

# =============================================================================
# 1. SYSTEM INFORMATION
# =============================================================================
echo "[1] System Information"
echo "---------------------"
echo "OS: $(lsb_release -d -s)"
echo "Kernel: $(uname -r)"
echo "Arch: $(uname -m)"
echo ""

# =============================================================================
# 2. POSTGRESQL VERIFICATION
# =============================================================================
echo "[2] PostgreSQL Verification"
echo "----------------------------"

# Check PostgreSQL version
if command -v psql &> /dev/null; then
    PG_VERSION=$(psql --version | grep -oP '\d+\.\d+' | head -1)
    if [[ "$PG_VERSION" == "17.7" ]] || [[ "$PG_VERSION" == "18.1" ]]; then
        check_pass "PostgreSQL version: $PG_VERSION"
    else
        check_warn "PostgreSQL version $PG_VERSION (expected 17.x)"
    fi
else
    check_fail "PostgreSQL not installed"
fi

# Check PostgreSQL service
if sudo systemctl is-active --quiet postgresql@17-main; then
    check_pass "PostgreSQL service running"
else
    check_fail "PostgreSQL service not running"
fi

# =============================================================================
# 3. POSTGRESQL EXTENSIONS
# =============================================================================
echo ""
echo "[3] PostgreSQL Extensions"
echo "-------------------------"

# Check TimescaleDB
TIMESCALEDB_VER=$(sudo -u postgres psql -d ragnarok_ml -tc "SELECT extversion FROM pg_extension WHERE extname='timescaledb';" 2>/dev/null | tr -d ' ')
if [[ ! -z "$TIMESCALEDB_VER" ]]; then
    check_pass "TimescaleDB installed: $TIMESCALEDB_VER"
else
    check_fail "TimescaleDB not installed in ragnarok_ml"
fi

# Check pgvector
PGVECTOR_VER=$(sudo -u postgres psql -d ragnarok_ml -tc "SELECT extversion FROM pg_extension WHERE extname='vector';" 2>/dev/null | tr -d ' ')
if [[ ! -z "$PGVECTOR_VER" ]]; then
    check_pass "pgvector installed: $PGVECTOR_VER"
else
    check_fail "pgvector not installed in ragnarok_ml"
fi

# Check Apache AGE
AGE_VER=$(sudo -u postgres psql -d ragnarok_ml -tc "SELECT extversion FROM pg_extension WHERE extname='age';" 2>/dev/null | tr -d ' ')
if [[ ! -z "$AGE_VER" ]]; then
    check_pass "Apache AGE installed: $AGE_VER"
else
    check_warn "Apache AGE not installed (optional for v2.0)"
fi

# =============================================================================
# 4. DATABASE SETUP
# =============================================================================
echo ""
echo "[4] Database Setup"
echo "------------------"

# Check ragnarok_ml database
if sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw ragnarok_ml; then
    check_pass "ragnarok_ml database exists"
else
    check_fail "ragnarok_ml database not found"
fi

# Check ml_user
ML_USER_EXISTS=$(sudo -u postgres psql -tc "SELECT 1 FROM pg_roles WHERE rolname='ml_user';" 2>/dev/null | tr -d ' ')
if [[ "$ML_USER_EXISTS" == "1" ]]; then
    check_pass "ml_user role exists"
else
    check_fail "ml_user role not found"
fi

# Check database tables
TABLE_COUNT=$(PGPASSWORD=ml_secure_password_2026 psql -h localhost -U ml_user -d ragnarok_ml -tc "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='public' AND (table_name LIKE 'ml_%' OR table_name LIKE 'ai_%');" 2>/dev/null | tr -d ' ')
if [[ "$TABLE_COUNT" -ge 9 ]]; then
    check_pass "ML tables created: $TABLE_COUNT tables"
else
    check_fail "ML tables incomplete (expected 9+, found $TABLE_COUNT)"
fi

# Check Apache AGE graph
if [[ ! -z "$AGE_VER" ]]; then
    GRAPH_EXISTS=$(sudo -u postgres psql -d ragnarok_ml -tc "SELECT COUNT(*) FROM ag_graph WHERE name='monster_ai';" 2>/dev/null | tr -d ' ')
    if [[ "$GRAPH_EXISTS" == "1" ]]; then
        check_pass "Apache AGE monster_ai graph created"
    else
        check_warn "Apache AGE graph not created"
    fi
fi

# =============================================================================
# 5. REDIS/CACHE
# =============================================================================
echo ""
echo "[5] Redis Cache"
echo "---------------"

# Check Redis service
if sudo systemctl is-active --quiet redis-server; then
    check_pass "Redis service running"
else
    check_fail "Redis service not running"
fi

# Test Redis connection
if redis-cli -a ml_monster_ai_2026 PING 2>/dev/null | grep -q "PONG"; then
    check_pass "Redis connection successful"
else
    check_fail "Redis connection failed (check password)"
fi

# Check Redis memory configuration
REDIS_MAXMEM=$(redis-cli -a ml_monster_ai_2026 CONFIG GET maxmemory 2>/dev/null | tail -1)
if [[ "$REDIS_MAXMEM" != "0" ]]; then
    REDIS_MAXMEM_GB=$((REDIS_MAXMEM / 1024 / 1024 / 1024))
    check_pass "Redis maxmemory configured: ${REDIS_MAXMEM_GB}GB"
else
    check_warn "Redis maxmemory not configured"
fi

# =============================================================================
# 6. PYTHON ENVIRONMENT
# =============================================================================
echo ""
echo "[6] Python Environment"
echo "----------------------"

# Check Python version
if [[ -f /opt/ml_monster_ai/venv/bin/python ]]; then
    PYTHON_VER=$(/opt/ml_monster_ai/venv/bin/python --version 2>&1 | grep -oP '\d+\.\d+\.\d+')
    if [[ "$PYTHON_VER" =~ ^3\.1[1-9] ]]; then
        check_pass "Python version: $PYTHON_VER"
    else
        check_warn "Python version $PYTHON_VER (expected 3.11+)"
    fi
else
    check_fail "Python virtual environment not found at /opt/ml_monster_ai/venv"
fi

# Check PyTorch installation
if /opt/ml_monster_ai/venv/bin/python -c "import torch" 2>/dev/null; then
    TORCH_VER=$(/opt/ml_monster_ai/venv/bin/python -c "import torch; print(torch.__version__)" 2>/dev/null)
    check_pass "PyTorch installed: $TORCH_VER"
else
    check_fail "PyTorch not installed"
fi

# Check CUDA availability
if /opt/ml_monster_ai/venv/bin/python -c "import torch; assert torch.cuda.is_available()" 2>/dev/null; then
    GPU_NAME=$(/opt/ml_monster_ai/venv/bin/python -c "import torch; print(torch.cuda.get_device_name(0))" 2>/dev/null)
    GPU_MEM=$(/opt/ml_monster_ai/venv/bin/python -c "import torch; print(f'{torch.cuda.get_device_properties(0).total_memory / 1024**3:.2f}')" 2>/dev/null)
    check_pass "CUDA available: $GPU_NAME ($GPU_MEM GB)"
else
    check_fail "CUDA not available to PyTorch"
fi

# Check ONNX Runtime GPU
if /opt/ml_monster_ai/venv/bin/python -c "import onnxruntime as ort; assert 'CUDAExecutionProvider' in ort.get_available_providers()" 2>/dev/null; then
    ONNX_VER=$(/opt/ml_monster_ai/venv/bin/python -c "import onnxruntime as ort; print(ort.__version__)" 2>/dev/null)
    check_pass "ONNX Runtime GPU: $ONNX_VER"
else
    check_fail "ONNX Runtime GPU not available"
fi

# Check database connectors
if /opt/ml_monster_ai/venv/bin/python -c "import asyncpg, aiomysql, redis" 2>/dev/null; then
    check_pass "Database connectors installed (asyncpg, aiomysql, redis)"
else
    check_fail "Database connectors missing"
fi

# =============================================================================
# 7. NVIDIA GPU
# =============================================================================
echo ""
echo "[7] NVIDIA GPU"
echo "--------------"

# Check nvidia-smi
if command -v nvidia-smi &> /dev/null; then
    GPU_MODEL=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -1)
    GPU_MEM=$(nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits 2>/dev/null | head -1)
    
    if [[ "$GPU_MODEL" =~ "3060" ]]; then
        check_pass "GPU detected: $GPU_MODEL ($GPU_MEM MB)"
    else
        check_warn "GPU detected: $GPU_MODEL (expected RTX 3060)"
    fi
else
    check_fail "nvidia-smi not found (NVIDIA drivers not installed)"
fi

# Check CUDA version
if command -v nvcc &> /dev/null; then
    CUDA_VER=$(nvcc --version | grep -oP 'release \K[0-9.]+')
    check_pass "CUDA Toolkit: $CUDA_VER"
else
    check_warn "nvcc not found (CUDA toolkit not in PATH)"
fi

# =============================================================================
# 8. CONFIGURATION FILES
# =============================================================================
echo ""
echo "[8] Configuration Files"
echo "-----------------------"

if [[ -f rathena-AI-world/conf/ml_monster_ai.conf ]]; then
    check_pass "ML configuration file exists"
else
    check_fail "ML configuration file not found"
fi

if [[ -f rathena-AI-world/.env ]]; then
    check_pass ".env file exists"
else
    check_warn ".env file not found (use .env.template)"
fi

if [[ -f /opt/ml_monster_ai/requirements.txt ]]; then
    check_pass "requirements.txt exists"
else
    check_warn "requirements.txt not found"
fi

# =============================================================================
# 9. SQL SCHEMA FILES
# =============================================================================
echo ""
echo "[9] SQL Schema Files"
echo "--------------------"

if [[ -f rathena-AI-world/sql-files/ml_monster_ai_schema.sql ]]; then
    check_pass "ML schema SQL file exists"
else
    check_fail "ML schema SQL file not found"
fi

if [[ -f rathena-AI-world/sql-files/age_graph_schema.sql ]]; then
    check_pass "AGE graph schema SQL file exists"
else
    check_warn "AGE graph schema SQL file not found"
fi

# =============================================================================
# 10. DIRECTORY STRUCTURE
# =============================================================================
echo ""
echo "[10] Directory Structure"
echo "------------------------"

REQUIRED_DIRS=(
    "/opt/ml_monster_ai"
    "/opt/ml_monster_ai/venv"
    "/opt/ml_monster_ai/models"
    "/opt/ml_monster_ai/logs"
    "/opt/ml_monster_ai/config"
)

for dir in "${REQUIRED_DIRS[@]}"; do
    if [[ -d "$dir" ]]; then
        check_pass "Directory exists: $dir"
    else
        check_fail "Directory missing: $dir"
    fi
done

# =============================================================================
# 11. DATABASE CONNECTIVITY TEST
# =============================================================================
echo ""
echo "[11] Database Connectivity"
echo "--------------------------"

# Test PostgreSQL connection
if PGPASSWORD=ml_secure_password_2026 psql -h localhost -U ml_user -d ragnarok_ml -c "SELECT 1;" &>/dev/null; then
    check_pass "PostgreSQL connection successful"
else
    check_fail "PostgreSQL connection failed"
fi

# Test Redis connection
if redis-cli -a ml_monster_ai_2026 SET test_key "test_value" &>/dev/null && \
   redis-cli -a ml_monster_ai_2026 GET test_key &>/dev/null; then
    redis-cli -a ml_monster_ai_2026 DEL test_key &>/dev/null
    check_pass "Redis read/write successful"
else
    check_fail "Redis read/write failed"
fi

# =============================================================================
# 12. PYTHON PACKAGE VERIFICATION
# =============================================================================
echo ""
echo "[12] Python Packages"
echo "--------------------"

REQUIRED_PACKAGES=(
    "torch"
    "onnxruntime-gpu"
    "stable-baselines3"
    "transformers"
    "asyncpg"
    "aiomysql"
    "redis"
    "numpy"
    "scipy"
    "scikit-learn"
    "pandas"
    "pydantic"
    "fastapi"
    "uvicorn"
    "prometheus-client"
)

for package in "${REQUIRED_PACKAGES[@]}"; do
    if /opt/ml_monster_ai/venv/bin/pip show "$package" &>/dev/null; then
        VER=$(/opt/ml_monster_ai/venv/bin/pip show "$package" | grep Version | awk '{print $2}')
        check_pass "$package: $VER"
    else
        check_fail "$package not installed"
    fi
done

# =============================================================================
# SUMMARY
# =============================================================================
echo ""
echo "============================================================================="
echo "VERIFICATION SUMMARY"
echo "============================================================================="
echo -e "${GREEN}Passed:${NC}   $PASSED"
echo -e "${RED}Failed:${NC}   $FAILED"
echo -e "${YELLOW}Warnings:${NC} $WARNINGS"
echo ""

if [[ $FAILED -eq 0 ]]; then
    echo -e "${GREEN}✓ ALL CRITICAL CHECKS PASSED${NC}"
    echo ""
    echo "Foundation setup complete! Ready for Phase 10 (Model Training)."
    echo ""
    echo "Next steps:"
    echo "1. Review configuration in rathena-AI-world/conf/ml_monster_ai.conf"
    echo "2. Update credentials in rathena-AI-world/.env"
    echo "3. Begin model training (Phase 10)"
    echo "4. Integrate with rAthena C++ code (Phase 11)"
    exit 0
else
    echo -e "${RED}✗ FOUNDATION SETUP INCOMPLETE${NC}"
    echo ""
    echo "Please fix the failed checks before proceeding."
    echo "Check installation logs for details."
    exit 1
fi
