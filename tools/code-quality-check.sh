#!/usr/bin/env bash
# Code Quality Check Script for rAthena
# This script runs the same checks as the GitHub Actions code quality pipeline

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
GRAY='\033[0;90m'
NC='\033[0m' # No Color

# Default options
SKIP_CMAKE=false
SKIP_FORMAT=false
SKIP_TIDY=false
FIX_FORMAT=false
BASE_BRANCH="master"
BASE_COMMIT=""
HEAD_COMMIT=""
GENERATOR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-cmake)
            SKIP_CMAKE=true
            shift
            ;;
        --skip-format)
            SKIP_FORMAT=true
            shift
            ;;
        --skip-tidy)
            SKIP_TIDY=true
            shift
            ;;
        --fix-format)
            FIX_FORMAT=true
            shift
            ;;
        --base-branch)
            BASE_BRANCH="$2"
            shift 2
            ;;
        --base-commit)
            BASE_COMMIT="$2"
            shift 2
            ;;
        --head-commit)
            HEAD_COMMIT="$2"
            shift 2
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --skip-cmake       Skip CMake generation check"
            echo "  --skip-format      Skip clang-format check"
            echo "  --skip-tidy        Skip clang-tidy check"
            echo "  --fix-format       Fix formatting issues automatically"
            echo "  --base-branch      Base branch for comparison (default: master)"
            echo "  --base-commit      Base commit SHA (for CI use)"
            echo "  --head-commit      Head commit SHA (for CI use)"
            echo "  --generator        CMake generator to use"
            echo "  --help             Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Get script directory and root directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

echo -e "${CYAN}==================================${NC}"
if [ "$FIX_FORMAT" = true ]; then
    echo -e "${CYAN}rAthena Code Formatter${NC}"
else
    echo -e "${CYAN}rAthena Code Quality Check${NC}"
fi
echo -e "${CYAN}==================================${NC}"
echo ""

cd "$ROOT_DIR"

# Detect LLVM installation and add to PATH if needed
if ! command -v clang-format &> /dev/null || ! command -v clang-tidy &> /dev/null; then
    echo -e "${YELLOW}LLVM tools not found in PATH. Attempting to locate...${NC}"
    
    # Check Homebrew installation
    if command -v brew &> /dev/null; then
        LLVM_PREFIX=$(brew --prefix llvm 2>/dev/null)
        if [ -n "$LLVM_PREFIX" ] && [ -d "$LLVM_PREFIX/bin" ]; then
            echo -e "${GREEN}Found LLVM at: $LLVM_PREFIX${NC}"
            export PATH="$LLVM_PREFIX/bin:$PATH"
            echo -e "${GREEN}Added to PATH for this session${NC}"
        fi
    fi
    
    # Check common installation paths
    for LLVM_PATH in "/usr/local/opt/llvm/bin" "/opt/homebrew/opt/llvm/bin" "/usr/local/llvm/bin"; do
        if [ -d "$LLVM_PATH" ]; then
            export PATH="$LLVM_PATH:$PATH"
            echo -e "${GREEN}Found LLVM tools at: $LLVM_PATH${NC}"
            break
        fi
    done
fi

FAILED_CHECKS=()

# Step 1: CMake Generation Check (only for Visual Studio generators)
if [ "$SKIP_CMAKE" = false ]; then
    echo -e "${YELLOW}[1/3] Checking CMake generation...${NC}"
    
    # Determine which generator to use and if we should run the check
    RUN_CMAKE_CHECK=true
    if [ -z "$GENERATOR" ]; then
        # Default: Unix Makefiles generates files not tracked in git, skip the check
        RUN_CMAKE_CHECK=false
        echo -e "${GRAY}Skipping CMake check (not using Visual Studio generator on this platform).${NC}"
        echo -e "${GREEN}✓ CMake check skipped${NC}"
    else
        # Check if generator is Visual Studio
        if [[ "$GENERATOR" == *"Visual Studio"* ]]; then
            echo -e "${GRAY}Generating $GENERATOR project files...${NC}"
        else
            # Non-VS generator: skip CMake check
            RUN_CMAKE_CHECK=false
            echo -e "${GRAY}Skipping CMake check (generator '$GENERATOR' is not Visual Studio).${NC}"
            echo -e "${GREEN}✓ CMake check skipped${NC}"
        fi
    fi
    
    if [ "$RUN_CMAKE_CHECK" = true ]; then
        if cmake -G "$GENERATOR" -DALLOW_SAME_DIRECTORY=ON . > /dev/null 2>&1; then
            # Check for changes using git status (doesn't modify staging area)
            STATUS=$(git status --porcelain)
            
            if [ -n "$STATUS" ]; then
                echo -e "${RED}✗ CMake generation created or modified files:${NC}"
                echo "$STATUS" | while read -r line; do
                    # Remove status prefix (first 3 characters)
                    file="${line:3}"
                    echo -e "${RED}  - $file${NC}"
                done
                FAILED_CHECKS+=("CMake generation")
            else
                echo -e "${GREEN}✓ CMake generation check passed${NC}"
            fi
        else
            echo -e "${RED}✗ CMake generation failed${NC}"
            FAILED_CHECKS+=("CMake generation")
        fi
    fi
    echo ""
fi

# Step 2: clang-format Check/Fix
if [ "$SKIP_FORMAT" = false ]; then
    if [ "$FIX_FORMAT" = true ]; then
        echo -e "${YELLOW}[2/3] Fixing code formatting...${NC}"
        echo -e "${GRAY}Running clang-format on source files in src/ directory only...${NC}"
    else
        echo -e "${YELLOW}[2/3] Checking code formatting...${NC}"
        echo -e "${GRAY}Running clang-format on source files in src/ directory only...${NC}"
    fi
    
    if command -v clang-format &> /dev/null; then
        # Find and format all source files - ONLY in src/ directory
        FILE_COUNT=$(find src -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" \) | wc -l | tr -d ' ')
        echo -e "${GRAY}Found $FILE_COUNT source files in src/ to check...${NC}"
        
        find src -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" \) -exec clang-format -i {} \;
        
        # Check for changes using git status (doesn't modify staging area) - filter to only src/ directory
        STATUS=$(git status --porcelain | grep ' src/' || true)
        
        if [ -n "$STATUS" ]; then
            # Extract file names from status output (remove first 3 characters)
            DIFF=$(echo "$STATUS" | while read -r line; do echo "${line:3}"; done)
            if [ "$FIX_FORMAT" = true ]; then
                FILE_COUNT=$(echo "$DIFF" | wc -l | tr -d ' ')
                echo -e "${GREEN}✓ Fixed formatting in $FILE_COUNT file(s):${NC}"
                echo "$DIFF" | while read -r file; do
                    echo -e "${GREEN}  - $file${NC}"
                done
                echo ""
                echo -e "${CYAN}Files have been formatted. Review and commit the changes.${NC}"
            else
                echo -e "${RED}✗ Code formatting issues detected in:${NC}"
                echo "$DIFF" | while read -r file; do
                    echo -e "${RED}  - $file${NC}"
                done
                echo -e "${YELLOW}Run './tools/code-quality-check.sh --fix-format' to fix formatting.${NC}"
                FAILED_CHECKS+=("Code formatting")
            fi
        else
            echo -e "${GREEN}✓ Code formatting check passed${NC}"
        fi
    else
        echo -e "${RED}✗ clang-format not found. Please install LLVM/Clang or add it to PATH.${NC}"
        echo -e "${YELLOW}See tools/README_CODE_QUALITY.md for installation instructions.${NC}"
        FAILED_CHECKS+=("Code formatting (tool not found)")
    fi
    echo ""
fi

# Step 3: clang-tidy Check (only for src/ directory)
if [ "$SKIP_TIDY" = false ] && [ "$FIX_FORMAT" = false ]; then
    echo -e "${YELLOW}[3/3] Running clang-tidy analysis on src/ directory only...${NC}"
    
    if command -v clang-tidy &> /dev/null; then
        # Get changed files - ONLY from src/ directory
        CHANGED_FILES=""
        GIT_FAILED=false
        
        if [ -n "$BASE_COMMIT" ] && [ -n "$HEAD_COMMIT" ]; then
            # CI mode: use explicit commits
            echo -e "${GRAY}Finding changed files in src/ between $BASE_COMMIT and $HEAD_COMMIT...${NC}"
            if CHANGED_FILES=$(git diff --name-only --diff-filter=d "$BASE_COMMIT...$HEAD_COMMIT" 2>/dev/null | grep -E '\.(cpp|hpp|c|h)$' | grep '^src/' || true); then
                true
            else
                if [ $? -ne 0 ] && [ $? -ne 1 ]; then
                    GIT_FAILED=true
                fi
            fi
        else
            # Local mode: compare with base branch
            echo -e "${GRAY}Finding changed files in src/ compared to origin/$BASE_BRANCH...${NC}"
            if CHANGED_FILES=$(git diff --name-only --diff-filter=d "origin/$BASE_BRANCH...HEAD" 2>/dev/null | grep -E '\.(cpp|hpp|c|h)$' | grep '^src/' || true); then
                true
            else
                if [ $? -ne 0 ] && [ $? -ne 1 ]; then
                    GIT_FAILED=true
                fi
            fi
        fi
        
        if [ "$GIT_FAILED" = true ]; then
            echo -e "${YELLOW}⚠ Unable to determine changed files (base branch may not exist). Skipping clang-tidy.${NC}"
            echo -e "${GREEN}✓ clang-tidy check skipped (nothing to compare against)${NC}"
        elif [ -z "$CHANGED_FILES" ]; then
            echo -e "${GRAY}No C/C++ source files changed in src/ directory. Skipping clang-tidy.${NC}"
            echo -e "${GREEN}✓ clang-tidy check passed (no files to check)${NC}"
        else
            FILE_COUNT=$(echo "$CHANGED_FILES" | wc -l | tr -d ' ')
            echo -e "${GRAY}Analyzing $FILE_COUNT changed file(s)...${NC}"
            
            # Generate compile_commands.json if it doesn't exist
            if [ ! -f "compile_commands.json" ]; then
                echo -e "${GRAY}Generating compile_commands.json...${NC}"
                cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . > /dev/null 2>&1 || true
            fi
            
            ERRORS=()
            while IFS= read -r file; do
                if [ -f "$file" ]; then
                    echo -e "${GRAY}  Checking: $file${NC}"
                    if ! clang-tidy -p . "$file" 2>&1; then
                        echo -e "${RED}    Issues found!${NC}"
                        ERRORS+=("$file")
                    fi
                fi
            done <<< "$CHANGED_FILES"
            
            if [ ${#ERRORS[@]} -gt 0 ]; then
                echo -e "${RED}✗ clang-tidy found issues in:${NC}"
                for file in "${ERRORS[@]}"; do
                    echo -e "${RED}  - $file${NC}"
                done
                FAILED_CHECKS+=("clang-tidy analysis")
            else
                echo -e "${GREEN}✓ clang-tidy analysis passed${NC}"
            fi
        fi
    else
        echo -e "${RED}✗ clang-tidy not found. Please install LLVM/Clang or add it to PATH.${NC}"
        echo -e "${YELLOW}See tools/README_CODE_QUALITY.md for installation instructions.${NC}"
        FAILED_CHECKS+=("clang-tidy (tool not found)")
    fi
    echo ""
fi

# Summary
echo -e "${CYAN}==================================${NC}"
echo -e "${CYAN}Summary${NC}"
echo -e "${CYAN}==================================${NC}"

if [ ${#FAILED_CHECKS[@]} -eq 0 ]; then
    echo -e "${GREEN}✓ All code quality checks passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Failed checks:${NC}"
    for check in "${FAILED_CHECKS[@]}"; do
        echo -e "${RED}  - $check${NC}"
    done
    exit 1
fi
