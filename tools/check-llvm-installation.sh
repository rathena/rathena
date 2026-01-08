#!/usr/bin/env bash
# Quick script to check LLVM installation and provide setup instructions

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}==================================${NC}"
echo -e "${CYAN}LLVM Installation Checker${NC}"
echo -e "${CYAN}==================================${NC}"
echo ""

# Check for clang-format
echo -e "${YELLOW}Checking clang-format...${NC}"
if command -v clang-format &> /dev/null; then
    CLANG_FORMAT_PATH=$(which clang-format)
    CLANG_FORMAT_VERSION=$(clang-format --version | head -1)
    echo -e "${GREEN}✓ Found: $CLANG_FORMAT_PATH${NC}"
    echo -e "${GREEN}  Version: $CLANG_FORMAT_VERSION${NC}"
else
    echo -e "${RED}✗ clang-format not found in PATH${NC}"
fi
echo ""

# Check for clang-tidy
echo -e "${YELLOW}Checking clang-tidy...${NC}"
if command -v clang-tidy &> /dev/null; then
    CLANG_TIDY_PATH=$(which clang-tidy)
    CLANG_TIDY_VERSION=$(clang-tidy --version | head -1)
    echo -e "${GREEN}✓ Found: $CLANG_TIDY_PATH${NC}"
    echo -e "${GREEN}  Version: $CLANG_TIDY_VERSION${NC}"
else
    echo -e "${RED}✗ clang-tidy not found in PATH${NC}"
fi
echo ""

# Check for CMake
echo -e "${YELLOW}Checking CMake...${NC}"
if command -v cmake &> /dev/null; then
    CMAKE_PATH=$(which cmake)
    CMAKE_VERSION=$(cmake --version | head -1)
    echo -e "${GREEN}✓ Found: $CMAKE_PATH${NC}"
    echo -e "${GREEN}  Version: $CMAKE_VERSION${NC}"
else
    echo -e "${RED}✗ CMake not found in PATH${NC}"
fi
echo ""

# Check for Homebrew LLVM (macOS)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo -e "${YELLOW}Checking Homebrew LLVM installation...${NC}"
    if command -v brew &> /dev/null; then
        LLVM_PREFIX=$(brew --prefix llvm 2>/dev/null)
        if [ -n "$LLVM_PREFIX" ]; then
            echo -e "${GREEN}✓ Homebrew LLVM installed at: $LLVM_PREFIX${NC}"
            
            if [ -f "$LLVM_PREFIX/bin/clang-format" ]; then
                echo -e "${GREEN}  - clang-format binary exists${NC}"
            fi
            
            if [ -f "$LLVM_PREFIX/bin/clang-tidy" ]; then
                echo -e "${GREEN}  - clang-tidy binary exists${NC}"
            fi
            
            # Check if it's in PATH
            if [[ ":$PATH:" != *":$LLVM_PREFIX/bin:"* ]]; then
                echo ""
                echo -e "${YELLOW}⚠ LLVM is installed but not in PATH${NC}"
                echo -e "${CYAN}To add it to your PATH, run:${NC}"
                echo ""
                echo -e "  ${GREEN}echo 'export PATH=\"$LLVM_PREFIX/bin:\$PATH\"' >> ~/.zshrc${NC}"
                echo -e "  ${GREEN}source ~/.zshrc${NC}"
                echo ""
                echo -e "${CYAN}Or for a one-time use:${NC}"
                echo -e "  ${GREEN}export PATH=\"$LLVM_PREFIX/bin:\$PATH\"${NC}"
            else
                echo -e "${GREEN}✓ LLVM is in your PATH${NC}"
            fi
        else
            echo -e "${RED}✗ Homebrew LLVM not installed${NC}"
            echo -e "${CYAN}To install, run:${NC}"
            echo -e "  ${GREEN}brew install llvm${NC}"
        fi
    else
        echo -e "${YELLOW}Homebrew not found${NC}"
    fi
fi

echo ""
echo -e "${CYAN}==================================${NC}"
echo -e "${CYAN}Summary${NC}"
echo -e "${CYAN}==================================${NC}"

ALL_OK=true
if ! command -v clang-format &> /dev/null; then
    ALL_OK=false
fi
if ! command -v clang-tidy &> /dev/null; then
    ALL_OK=false
fi
if ! command -v cmake &> /dev/null; then
    ALL_OK=false
fi

if [ "$ALL_OK" = true ]; then
    echo -e "${GREEN}✓ All required tools are installed and accessible!${NC}"
    echo ""
    echo -e "${CYAN}You can now run the code quality checks:${NC}"
    echo -e "  ${GREEN}./tools/code-quality-check.sh${NC}"
else
    echo -e "${RED}✗ Some tools are missing or not in PATH${NC}"
    echo ""
    echo -e "${CYAN}Please refer to tools/README_CODE_QUALITY.md for installation instructions${NC}"
fi
