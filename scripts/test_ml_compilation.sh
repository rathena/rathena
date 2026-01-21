#!/bin/bash
# Test ML integration compilation script
# Enhanced ML Monster AI v2.0 - Phase 10

echo "========================================"
echo "Testing ML Integration Compilation"
echo "Enhanced ML Monster AI v2.0"
echo "========================================"
echo ""

cd "$(dirname "$0")/.." || exit 1

# Check if required libraries are available
echo "[1/6] Checking dependencies..."

# Check for libpq (PostgreSQL)
if ! pkg-config --exists libpq; then
    echo "⚠️  WARNING: libpq (PostgreSQL client library) not found"
    echo "    Install: sudo apt-get install libpq-dev"
else
    echo "✓ libpq found: $(pkg-config --modversion libpq)"
fi

# Check for hiredis (Redis)
if ! pkg-config --exists hiredis; then
    echo "⚠️  WARNING: hiredis (Redis client library) not found"
    echo "    Install: sudo apt-get install libhiredis-dev"
else
    echo "✓ hiredis found: $(pkg-config --modversion hiredis)"
fi

echo ""
echo "[2/6] Verifying ML source files..."

# Check ML source files exist
ML_FILES=(
    "src/map/mob_ml_encoder.hpp"
    "src/map/mob_ml_encoder.cpp"
    "src/map/mob_ml_gateway.hpp"
    "src/map/mob_ml_gateway.cpp"
    "src/map/mob_ml_executor.hpp"
    "src/map/mob_ml_executor.cpp"
)

for file in "${ML_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file (missing)"
        exit 1
    fi
done

echo ""
echo "[3/6] Configuring build system..."

# Configure with CMake (if not already configured)
if [ ! -d "build" ]; then
    mkdir -p build
    cd build || exit 1
    cmake .. -DCMAKE_BUILD_TYPE=Debug || exit 1
    cd ..
fi

echo ""
echo "[4/6] Building map-server..."

# Build only map-server
cd build || exit 1
make map-server -j$(nproc) 2>&1 | tee ../build_ml.log

BUILD_STATUS=$?

cd ..

if [ $BUILD_STATUS -eq 0 ]; then
    echo ""
    echo "✓ Compilation successful"
    
    echo ""
    echo "[5/6] Checking for ML symbols..."
    
    # Check for ML symbols in binary
    if command -v nm &> /dev/null; then
        if nm build/map-server 2>/dev/null | grep -i "MobML" > /dev/null; then
            echo "✓ ML symbols found in binary"
        else
            echo "⚠️  ML symbols not found (might be optimized out or static)"
        fi
    fi
    
    echo ""
    echo "[6/6] Verifying binary executable..."
    
    # Check if binary is executable
    if [ -x "build/map-server" ]; then
        echo "✓ Binary is executable"
        
        # Show binary size
        BINARY_SIZE=$(du -h build/map-server | cut -f1)
        echo "  Binary size: $BINARY_SIZE"
    else
        echo "✗ Binary is not executable"
        exit 1
    fi
    
    echo ""
    echo "========================================"
    echo "✅ ML Integration Compilation SUCCESS"
    echo "========================================"
    echo ""
    echo "Next steps:"
    echo "1. Start PostgreSQL with ML schema:"
    echo "   psql -U postgres -d ragnarok_ml -f sql-files/ml_monster_ai_schema.sql"
    echo ""
    echo "2. Start Redis/DragonFlyDB:"
    echo "   redis-server (or dragonfly --port 6379)"
    echo ""
    echo "3. Start Python ML inference service:"
    echo "   cd /opt/ml_monster_ai && python3 ml_inference_service.py"
    echo ""
    echo "4. Start map-server:"
    echo "   ./map-server"
    echo ""
    echo "Configuration:"
    echo "  - ML settings: conf/battle/monster.conf"
    echo "  - Enable/disable: ml_monster_ai_enabled (1/0)"
    echo "  - Debug logging: ml_debug_logging (1/0)"
    echo ""
    
    exit 0
else
    echo ""
    echo "========================================"
    echo "✗ Compilation FAILED"
    echo "========================================"
    echo ""
    echo "Check build_ml.log for errors"
    echo ""
    echo "Common issues:"
    echo "1. Missing libpq-dev: sudo apt-get install libpq-dev"
    echo "2. Missing libhiredis-dev: sudo apt-get install libhiredis-dev"
    echo "3. Missing pthread: Usually included with build-essential"
    echo ""
    echo "Full log: $(pwd)/build_ml.log"
    echo ""
    
    exit 1
fi
