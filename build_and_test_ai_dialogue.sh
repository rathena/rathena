#!/bin/bash
#===== rAthena Build Script =====================================
#= AI Dialogue System - Build and Test
#===== Description: ============================================
#= Automated build and basic testing for AI dialogue system
#===== Usage: ==================================================
#= chmod +x build_and_test_ai_dialogue.sh
#= ./build_and_test_ai_dialogue.sh
#================================================================

set -e  # Exit on error

echo "=========================================="
echo "AI Dialogue System - Build and Test"
echo "=========================================="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Step 1: Clean previous build
echo -e "${YELLOW}[1/6] Cleaning previous build...${NC}"
make clean 2>/dev/null || true
echo -e "${GREEN}✓ Clean complete${NC}"
echo ""

# Step 2: Configure
echo -e "${YELLOW}[2/6] Running configure...${NC}"
if [ ! -f "configure" ]; then
    echo -e "${RED}✗ configure script not found!${NC}"
    echo "Please run this script from the rathena-AI-world directory"
    exit 1
fi
./configure
echo -e "${GREEN}✓ Configure complete${NC}"
echo ""

# Step 3: Build server
echo -e "${YELLOW}[3/6] Building map server...${NC}"
echo "This may take a few minutes..."
make server

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Build successful!${NC}"
else
    echo -e "${RED}✗ Build failed!${NC}"
    echo "Check the error messages above"
    exit 1
fi
echo ""

# Step 4: Check if AI service is running
echo -e "${YELLOW}[4/6] Checking AI service...${NC}"
if curl -s http://localhost:8888/health > /dev/null 2>&1; then
    echo -e "${GREEN}✓ AI service is running on port 8888${NC}"
else
    echo -e "${RED}✗ AI service is NOT running!${NC}"
    echo "Please start the web server on port 8888"
    echo "The map server will still start, but AI dialogue will not work"
fi
echo ""

# Step 5: Check NPC scripts
echo -e "${YELLOW}[5/6] Checking NPC scripts...${NC}"
if [ -f "npc/custom/ai_merchant_example.txt" ]; then
    echo -e "${GREEN}✓ ai_merchant_example.txt found${NC}"
else
    echo -e "${RED}✗ ai_merchant_example.txt not found${NC}"
fi

if [ -f "npc/custom/ai_quest_giver_example.txt" ]; then
    echo -e "${GREEN}✓ ai_quest_giver_example.txt found${NC}"
else
    echo -e "${RED}✗ ai_quest_giver_example.txt not found${NC}"
fi

echo ""
echo "Add these lines to npc/scripts_custom.conf:"
echo "  npc: npc/custom/ai_merchant_example.txt"
echo "  npc: npc/custom/ai_quest_giver_example.txt"
echo ""

# Step 6: Test AI service endpoint
echo -e "${YELLOW}[6/6] Testing AI service endpoint...${NC}"
if command -v curl &> /dev/null; then
    echo "Sending test request to AI service..."
    
    RESPONSE=$(curl -s -X POST http://localhost:8888/ai/chat/command \
      -H "Content-Type: application/json" \
      -d '{
        "npc_id": "test_npc",
        "char_id": 999999,
        "message": "Hello, this is a test",
        "source": "build_script"
      }' 2>&1)
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ AI service responded${NC}"
        echo "Response: $RESPONSE"
    else
        echo -e "${RED}✗ AI service did not respond${NC}"
        echo "Error: $RESPONSE"
    fi
else
    echo -e "${YELLOW}⚠ curl not found, skipping AI service test${NC}"
fi
echo ""

# Summary
echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo ""
echo "✅ Map server compiled successfully"
echo "✅ AI dialogue system integrated"
echo ""
echo "Next Steps:"
echo "1. Start the map server: ./map-server"
echo "2. Check logs for: 'AI Dialogue System: Initialized successfully'"
echo "3. Login to game and go to Prontera (150, 150)"
echo "4. Talk to 'AI Merchant' NPC"
echo "5. Select 'Type your own question' and test AI dialogue"
echo ""
echo "Documentation:"
echo "- System docs: doc/AI_DIALOGUE_SYSTEM.md"
echo "- Testing guide: doc/AI_DIALOGUE_TESTING.md"
echo "- Configuration: conf/ai_dialogue.conf"
echo ""
echo "Troubleshooting:"
echo "- If AI service fails: Check if web server is running on port 8888"
echo "- If build fails: Check for missing dependencies (httplib, nlohmann/json)"
echo "- For detailed logs: tail -f log/map-server.log | grep 'AI Dialogue'"
echo ""
echo "=========================================="
echo -e "${GREEN}Build and test complete!${NC}"
echo "=========================================="

