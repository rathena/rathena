#!/bin/bash

# Test runner script for AI Autonomous World Service
# This script runs all tests: unit, performance, and load tests

set -e  # Exit on error

echo "================================================================================"
echo "AI AUTONOMOUS WORLD SERVICE - COMPREHENSIVE TEST SUITE"
echo "================================================================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if virtual environment is activated
if [ -z "$VIRTUAL_ENV" ]; then
    echo -e "${YELLOW}Warning: Virtual environment not activated${NC}"
    echo "Attempting to activate venv..."
    if [ -d "venv" ]; then
        source venv/bin/activate
    elif [ -d "../venv" ]; then
        source ../venv/bin/activate
    else
        echo -e "${RED}Error: Virtual environment not found${NC}"
        exit 1
    fi
fi

# Install test dependencies
echo -e "${YELLOW}Installing test dependencies...${NC}"
pip install -q -r requirements-test.txt

echo ""
echo "================================================================================"
echo "PHASE 1: UNIT TESTS"
echo "================================================================================"
echo ""

# Run unit tests with coverage
pytest tests/ \
    --verbose \
    --cov=. \
    --cov-report=term-missing \
    --cov-report=html:htmlcov \
    --cov-report=xml:coverage.xml \
    --ignore=tests/locustfile.py \
    --ignore=tests/test_performance.py \
    -v

UNIT_TEST_EXIT_CODE=$?

echo ""
echo "================================================================================"
echo "PHASE 2: PERFORMANCE TESTS"
echo "================================================================================"
echo ""

# Run performance tests
pytest tests/test_performance.py \
    --verbose \
    -v

PERF_TEST_EXIT_CODE=$?

echo ""
echo "================================================================================"
echo "PHASE 3: LOAD TEST PREPARATION"
echo "================================================================================"
echo ""

echo -e "${YELLOW}Load tests require the service to be running.${NC}"
echo "To run load tests manually:"
echo ""
echo "  1. Start the service:"
echo "     python main.py"
echo ""
echo "  2. In another terminal, run:"
echo "     locust -f tests/locustfile.py --host=http://localhost:8000"
echo ""
echo "  3. Open browser to http://localhost:8089"
echo "  4. Configure users and spawn rate, then start test"
echo ""

echo "================================================================================"
echo "TEST SUMMARY"
echo "================================================================================"
echo ""

if [ $UNIT_TEST_EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Unit Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Unit Tests: FAILED${NC}"
fi

if [ $PERF_TEST_EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Performance Tests: PASSED${NC}"
else
    echo -e "${RED}✗ Performance Tests: FAILED${NC}"
fi

echo ""
echo "Coverage report generated in: htmlcov/index.html"
echo ""

# Exit with error if any tests failed
if [ $UNIT_TEST_EXIT_CODE -ne 0 ] || [ $PERF_TEST_EXIT_CODE -ne 0 ]; then
    exit 1
fi

echo -e "${GREEN}All automated tests passed!${NC}"
exit 0

