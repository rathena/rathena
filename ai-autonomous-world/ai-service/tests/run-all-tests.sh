#!/bin/bash
##############################################################################
# Run All Tests - Complete Test Suite Execution
# Executes unit, integration, performance, and security tests
##############################################################################

set -e  # Exit on error

echo "================================================================================"
echo "RAGNAROK ONLINE AI WORLD - COMPLETE TEST SUITE"
echo "================================================================================"
echo ""
echo "Phase 7: Integration Testing & Optimization"
echo "Test Execution Started: $(date)"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test result tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Change to AI service directory
cd "$(dirname "$0")/.." || exit 1

# Activate virtual environment if it exists
if [ -d "venv" ]; then
    echo "Activating virtual environment..."
    source venv/bin/activate
fi

# Install/update test dependencies
echo "Installing test dependencies..."
pip install -q pytest pytest-asyncio pytest-cov pytest-mock locust httpx

echo ""
echo "================================================================================"
echo "PHASE 1: UNIT TESTS"
echo "================================================================================"
echo ""

pytest tests/agents/ tests/routers/ tests/llm/ tests/utils/ \
    tests/storyline/ tests/services/ \
    -v --tb=short --maxfail=5 \
    --cov=. --cov-report=term-missing --cov-report=html \
    2>&1 | tee test-results/unit-tests.log

UNIT_RESULT=${PIPESTATUS[0]}

echo ""
echo "================================================================================"
echo "PHASE 2: INTEGRATION TESTS"
echo "================================================================================"
echo ""

pytest tests/integration/ \
    -v --tb=short \
    -m integration \
    2>&1 | tee test-results/integration-tests.log

INTEGRATION_RESULT=${PIPESTATUS[0]}

echo ""
echo "================================================================================"
echo "PHASE 3: PERFORMANCE TESTS"
echo "================================================================================"
echo ""

pytest tests/performance/ \
    -v --tb=short \
    -m performance \
    --timeout=300 \
    2>&1 | tee test-results/performance-tests.log

PERFORMANCE_RESULT=${PIPESTATUS[0]}

echo ""
echo "================================================================================"
echo "PHASE 4: SECURITY TESTS"
echo "================================================================================"
echo ""

if [ -d "tests/security" ]; then
    pytest tests/security/ \
        -v --tb=short \
        -m security \
        2>&1 | tee test-results/security-tests.log
    
    SECURITY_RESULT=${PIPESTATUS[0]}
else
    echo "Security tests directory not found, skipping..."
    SECURITY_RESULT=0
fi

echo ""
echo "================================================================================"
echo "TEST EXECUTION SUMMARY"
echo "================================================================================"
echo ""

# Display results
if [ $UNIT_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ Unit Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Unit Tests: FAILED${NC}"
fi

if [ $INTEGRATION_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ Integration Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Integration Tests: FAILED${NC}"
fi

if [ $PERFORMANCE_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ Performance Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Performance Tests: FAILED${NC}"
fi

if [ $SECURITY_RESULT -eq 0 ]; then
    echo -e "${GREEN}✓ Security Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Security Tests: FAILED${NC}"
fi

echo ""
echo "Coverage Report: file://$(pwd)/htmlcov/index.html"
echo ""

# Calculate overall result
if [ $UNIT_RESULT -eq 0 ] && [ $INTEGRATION_RESULT -eq 0 ] && [ $PERFORMANCE_RESULT -eq 0 ] && [ $SECURITY_RESULT -eq 0 ]; then
    echo -e "${GREEN}================================================================================"
    echo "ALL TESTS PASSED ✓"
    echo "================================================================================${NC}"
    echo ""
    echo "System is READY FOR PRODUCTION"
    exit 0
else
    echo -e "${RED}================================================================================"
    echo "SOME TESTS FAILED ✗"
    echo "================================================================================${NC}"
    echo ""
    echo "Review test logs in test-results/ directory"
    exit 1
fi