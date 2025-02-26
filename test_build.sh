#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${YELLOW}Starting build and test verification...${NC}"

# Create build directory
mkdir -p build
cd build || exit 1

# Configure with tests enabled
echo -e "\n${YELLOW}Configuring CMake...${NC}"
cmake -DBUILD_TESTING=ON \
      -DMYSQL_INCLUDE_DIR=/usr/include/mysql \
      -DMYSQL_LIBRARY=/usr/lib/mysql/libmysqlclient.so \
      .. || exit 1

# Build the project
echo -e "\n${YELLOW}Building project...${NC}"
cmake --build . || exit 1

# Setup test database
echo -e "\n${YELLOW}Setting up test database...${NC}"
if mysql -u test_user -ptest_pass -e "source src/test/setup_test_db.sql"; then
    echo -e "${GREEN}Test database setup complete${NC}"
else
    echo -e "${RED}Failed to setup test database${NC}"
    exit 1
fi

# Run tests
echo -e "\n${YELLOW}Running tests...${NC}"

# Run sync tests
echo -e "\n${YELLOW}Running synchronization tests...${NC}"
./src/test/network_tests --gtest_filter=SyncTest.* || exit 1

# Run network monitor tests
echo -e "\n${YELLOW}Running network monitor tests...${NC}"
./src/test/network_tests --gtest_filter=NetworkMonitorTest.* || exit 1

# Run config parser tests
echo -e "\n${YELLOW}Running config parser tests...${NC}"
./src/test/network_tests --gtest_filter=P2PConfigParserTest.* || exit 1

# Memory leak check with Valgrind if available
if command -v valgrind &> /dev/null; then
    echo -e "\n${YELLOW}Running memory leak checks...${NC}"
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --verbose \
             --log-file=valgrind-out.txt \
             ./src/test/network_tests
    
    if grep -q "definitely lost: 0 bytes" valgrind-out.txt; then
        echo -e "${GREEN}No memory leaks detected${NC}"
    else
        echo -e "${RED}Memory leaks detected! Check valgrind-out.txt for details${NC}"
        exit 1
    fi
fi

# Thread race condition check with Helgrind if available
if command -v valgrind &> /dev/null; then
    echo -e "\n${YELLOW}Running thread race condition checks...${NC}"
    valgrind --tool=helgrind \
             --log-file=helgrind-out.txt \
             ./src/test/network_tests
    
    if grep -q "ERROR SUMMARY: 0 errors" helgrind-out.txt; then
        echo -e "${GREEN}No thread race conditions detected${NC}"
    else
        echo -e "${RED}Thread race conditions detected! Check helgrind-out.txt for details${NC}"
        exit 1
    fi
fi

# Run performance tests if enabled
if [ "$1" == "--performance" ]; then
    echo -e "\n${YELLOW}Running performance tests...${NC}"
    ./src/test/network_tests --gtest_filter=*Performance*
fi

# Final cleanup
echo -e "\n${YELLOW}Cleaning up test database...${NC}"
mysql -u test_user -ptest_pass -e "DROP DATABASE IF EXISTS test_rathena"

echo -e "\n${GREEN}All tests completed successfully!${NC}"
exit 0