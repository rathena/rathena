#!/bin/bash
# AI NPC Integration Verification Script
# Verifies that KicapMasin and AI Integration Test NPCs are properly set up

echo "=========================================="
echo "AI NPC Integration Verification"
echo "=========================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PASS=0
FAIL=0

# Check 1: NPC files exist
echo "1. Checking NPC files..."
if [ -f "npc/custom/KicapMasin.txt" ]; then
    echo -e "   ${GREEN}✓${NC} KicapMasin.txt exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} KicapMasin.txt missing"
    ((FAIL++))
fi

if [ -f "npc/custom/ai_integration_test.txt" ]; then
    echo -e "   ${GREEN}✓${NC} ai_integration_test.txt exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} ai_integration_test.txt missing"
    ((FAIL++))
fi

# Check 2: scripts_custom.conf contains the NPCs
echo ""
echo "2. Checking scripts_custom.conf..."
if grep -q "npc/custom/KicapMasin.txt" npc/scripts_custom.conf; then
    echo -e "   ${GREEN}✓${NC} KicapMasin.txt registered"
    ((PASS++))
else
    echo -e "   ${YELLOW}⚠${NC} KicapMasin.txt not registered (commented out?)"
fi

if grep -q "npc/custom/ai_integration_test.txt" npc/scripts_custom.conf; then
    echo -e "   ${GREEN}✓${NC} ai_integration_test.txt registered"
    ((PASS++))
else
    echo -e "   ${YELLOW}⚠${NC} ai_integration_test.txt not registered"
fi

# Check 3: Database schema
echo ""
echo "3. Checking database schema..."
if [ -f "sql-files/ai_ipc_tables.sql" ]; then
    echo -e "   ${GREEN}✓${NC} ai_ipc_tables.sql exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} ai_ipc_tables.sql missing"
    ((FAIL++))
fi

# Check 4: C++ source files
echo ""
echo "4. Checking C++ implementation..."
if [ -f "src/custom/script_ai_ipc.cpp" ]; then
    echo -e "   ${GREEN}✓${NC} script_ai_ipc.cpp exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} script_ai_ipc.cpp missing"
    ((FAIL++))
fi

if [ -f "src/custom/script_def.inc" ]; then
    echo -e "   ${GREEN}✓${NC} script_def.inc exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} script_def.inc missing"
    ((FAIL++))
fi

# Check 5: Script command registrations
echo ""
echo "5. Checking script command registrations..."
if grep -q "BUILDIN_DEF(ai_db_request" src/custom/script_def.inc; then
    echo -e "   ${GREEN}✓${NC} ai_db_request registered"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} ai_db_request not registered"
    ((FAIL++))
fi

if grep -q "BUILDIN_DEF(npcwalk" src/custom/script_def.inc; then
    echo -e "   ${GREEN}✓${NC} npcwalk registered"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} npcwalk not registered"
    ((FAIL++))
fi

# Check 6: Python worker service
echo ""
echo "6. Checking Python worker service..."
if [ -f "tools/ai_ipc_service/main.py" ]; then
    echo -e "   ${GREEN}✓${NC} Worker service main.py exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} Worker service missing"
    ((FAIL++))
fi

if [ -f "tools/ai_ipc_service/processor.py" ]; then
    echo -e "   ${GREEN}✓${NC} Worker processor.py exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} Worker processor.py missing"
    ((FAIL++))
fi

# Check 7: Documentation
echo ""
echo "7. Checking documentation..."
if [ -f "npc/custom/AI_NPC_GUIDE.md" ]; then
    echo -e "   ${GREEN}✓${NC} AI_NPC_GUIDE.md exists"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} AI_NPC_GUIDE.md missing"
    ((FAIL++))
fi

# Check 8: Basic NPC syntax validation
echo ""
echo "8. Validating NPC syntax..."

# Check for common errors
SYNTAX_ERRORS=0

# Check KicapMasin.txt
if grep -q "script.*KicapMasin.*{$" npc/custom/KicapMasin.txt; then
    echo -e "   ${GREEN}✓${NC} KicapMasin.txt syntax looks valid"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} KicapMasin.txt syntax issues"
    ((FAIL++))
    SYNTAX_ERRORS=1
fi

# Check ai_integration_test.txt
if grep -q "script.*AI Integration Test.*{$" npc/custom/ai_integration_test.txt; then
    echo -e "   ${GREEN}✓${NC} ai_integration_test.txt syntax looks valid"
    ((PASS++))
else
    echo -e "   ${RED}✗${NC} ai_integration_test.txt syntax issues"
    ((FAIL++))
    SYNTAX_ERRORS=1
fi

# Summary
echo ""
echo "=========================================="
echo "Verification Summary"
echo "=========================================="
echo -e "Tests Passed: ${GREEN}$PASS${NC}"
echo -e "Tests Failed: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}SUCCESS: All checks passed!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Ensure database tables are created:"
    echo "   mysql -u ragnarok -p ragnarok < sql-files/ai_ipc_tables.sql"
    echo ""
    echo "2. Compile map-server with AI IPC support:"
    echo "   ./configure && make clean && make server"
    echo ""
    echo "3. Start services:"
    echo "   cd tools/ai_ipc_service && python3 main.py &"
    echo "   ./map-server"
    echo ""
    echo "4. Test in-game:"
    echo "   Go to Prontera (155,185) and talk to KicapMasin"
    echo "   Run integration test at (150,185)"
    exit 0
else
    echo -e "${RED}FAILURE: $FAIL check(s) failed${NC}"
    echo ""
    echo "Please fix the issues above before continuing."
    exit 1
fi
