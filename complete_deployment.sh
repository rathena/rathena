#!/bin/bash
# Complete rAthena AI World Deployment Script
# This script completes the deployment by:
# 1. Initializing PostgreSQL databases
# 2. Installing systemd services
# 3. Starting all services
# 4. Verifying deployment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}rAthena AI World - Deployment Completion${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if running with sudo
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}✗ This script must be run with sudo${NC}"
    echo "Usage: sudo ./complete_deployment.sh"
    exit 1
fi

# Get the actual user (not root)
ACTUAL_USER="${SUDO_USER:-$USER}"
echo -e "${GREEN}Running as: $ACTUAL_USER (via sudo)${NC}"
echo ""

# ============================================================================
# STEP 1: Initialize PostgreSQL Databases
# ============================================================================
echo -e "${YELLOW}Step 1: Initializing PostgreSQL databases...${NC}"

# AI World Memory Database
echo "  → Creating ai_world_memory database..."
sudo -u postgres psql <<EOF
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'ai_world_user') THEN
        CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';
    END IF;
END
\$\$;

SELECT 'CREATE DATABASE ai_world_memory OWNER ai_world_user'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'ai_world_memory')\gexec

GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;
EOF

# Create extensions and tables for AI World
sudo -u postgres psql -d ai_world_memory <<'EOF'
CREATE EXTENSION IF NOT EXISTS vector;
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS age;

GRANT ALL ON SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_world_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO ai_world_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO ai_world_user;

-- NPC memories table
CREATE TABLE IF NOT EXISTS npc_memories (
    id BIGSERIAL PRIMARY KEY,
    npc_id INTEGER NOT NULL,
    memory_type VARCHAR(50) NOT NULL,
    content TEXT NOT NULL,
    embedding vector(1536),
    metadata JSONB DEFAULT '{}',
    importance_score FLOAT DEFAULT 0.5,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    accessed_count INTEGER DEFAULT 0,
    last_accessed_at TIMESTAMPTZ
);

CREATE INDEX IF NOT EXISTS idx_npc_memories_npc_id ON npc_memories(npc_id);
CREATE INDEX IF NOT EXISTS idx_npc_memories_type ON npc_memories(memory_type);
CREATE INDEX IF NOT EXISTS idx_npc_memories_created ON npc_memories(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_npc_memories_embedding ON npc_memories USING ivfflat (embedding vector_cosine_ops) WITH (lists = 100);

SELECT create_hypertable('npc_memories', 'created_at', if_not_exists => TRUE);

-- NPC relationships table
CREATE TABLE IF NOT EXISTS npc_relationships (
    id BIGSERIAL PRIMARY KEY,
    npc_id INTEGER NOT NULL,
    target_id INTEGER NOT NULL,
    target_type VARCHAR(20) NOT NULL,
    relationship_type VARCHAR(50) NOT NULL,
    strength FLOAT DEFAULT 0.5,
    sentiment FLOAT DEFAULT 0.0,
    metadata JSONB DEFAULT '{}',
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(npc_id, target_id, target_type, relationship_type)
);

CREATE INDEX IF NOT EXISTS idx_npc_relationships_npc ON npc_relationships(npc_id);
CREATE INDEX IF NOT EXISTS idx_npc_relationships_target ON npc_relationships(target_id, target_type);

-- Player interaction history
CREATE TABLE IF NOT EXISTS player_interactions (
    id BIGSERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL,
    npc_id INTEGER NOT NULL,
    interaction_type VARCHAR(50) NOT NULL,
    content TEXT,
    metadata JSONB DEFAULT '{}',
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_player_interactions_player ON player_interactions(player_id);
CREATE INDEX IF NOT EXISTS idx_player_interactions_npc ON player_interactions(npc_id);
CREATE INDEX IF NOT EXISTS idx_player_interactions_created ON player_interactions(created_at DESC);

SELECT create_hypertable('player_interactions', 'created_at', if_not_exists => TRUE);

-- World events table
CREATE TABLE IF NOT EXISTS world_events (
    id BIGSERIAL PRIMARY KEY,
    event_type VARCHAR(50) NOT NULL,
    location VARCHAR(100),
    description TEXT,
    participants JSONB DEFAULT '[]',
    metadata JSONB DEFAULT '{}',
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_world_events_type ON world_events(event_type);
CREATE INDEX IF NOT EXISTS idx_world_events_location ON world_events(location);
CREATE INDEX IF NOT EXISTS idx_world_events_created ON world_events(created_at DESC);

SELECT create_hypertable('world_events', 'created_at', if_not_exists => TRUE);

-- NPC state snapshots
CREATE TABLE IF NOT EXISTS npc_states (
    id BIGSERIAL PRIMARY KEY,
    npc_id INTEGER NOT NULL,
    state_data JSONB NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_npc_states_npc ON npc_states(npc_id);
CREATE INDEX IF NOT EXISTS idx_npc_states_created ON npc_states(created_at DESC);

SELECT create_hypertable('npc_states', 'created_at', if_not_exists => TRUE);

GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_world_user;
EOF

echo -e "${GREEN}  ✓ ai_world_memory database initialized${NC}"

# P2P Coordinator Database
echo "  → Creating p2p_coordinator database..."
sudo -u postgres psql <<EOF
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'p2p_user') THEN
        CREATE USER p2p_user WITH PASSWORD 'p2p_pass_2025';
    END IF;
END
\$\$;

SELECT 'CREATE DATABASE p2p_coordinator OWNER p2p_user'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'p2p_coordinator')\gexec

GRANT ALL PRIVILEGES ON DATABASE p2p_coordinator TO p2p_user;
EOF

echo -e "${GREEN}  ✓ p2p_coordinator database initialized${NC}"
echo ""

# ============================================================================
# STEP 2: Install Systemd Services
# ============================================================================
echo -e "${YELLOW}Step 2: Installing systemd services...${NC}"

# Copy service files to systemd directory
cp "$SCRIPT_DIR/systemd/ai-world.service" /etc/systemd/system/
cp "$SCRIPT_DIR/systemd/p2p-coordinator.service" /etc/systemd/system/
cp "$SCRIPT_DIR/systemd/rathena-login.service" /etc/systemd/system/
cp "$SCRIPT_DIR/systemd/rathena-char.service" /etc/systemd/system/
cp "$SCRIPT_DIR/systemd/rathena-map.service" /etc/systemd/system/

# Reload systemd daemon
systemctl daemon-reload

echo -e "${GREEN}  ✓ Systemd services installed${NC}"
echo ""

# ============================================================================
# STEP 3: Enable Services
# ============================================================================
echo -e "${YELLOW}Step 3: Enabling services to start on boot...${NC}"

systemctl enable postgresql
systemctl enable mariadb
systemctl enable dragonfly
systemctl enable ai-world
systemctl enable p2p-coordinator
systemctl enable rathena-login
systemctl enable rathena-char
systemctl enable rathena-map

echo -e "${GREEN}  ✓ All services enabled${NC}"
echo ""

# ============================================================================
# STEP 4: Start Database Services
# ============================================================================
echo -e "${YELLOW}Step 4: Starting database services...${NC}"

# Ensure PostgreSQL is running
systemctl start postgresql
systemctl status postgresql --no-pager | head -3
echo -e "${GREEN}  ✓ PostgreSQL started${NC}"

# Ensure MariaDB is running
systemctl start mariadb
systemctl status mariadb --no-pager | head -3
echo -e "${GREEN}  ✓ MariaDB started${NC}"

# Ensure DragonflyDB is running
systemctl start dragonfly
systemctl status dragonfly --no-pager | head -3
echo -e "${GREEN}  ✓ DragonflyDB started${NC}"

echo ""

# ============================================================================
# STEP 5: Start rAthena Servers
# ============================================================================
echo -e "${YELLOW}Step 5: Starting rAthena servers...${NC}"

# Start login server
systemctl start rathena-login
sleep 2
systemctl status rathena-login --no-pager | head -3
echo -e "${GREEN}  ✓ Login server started${NC}"

# Start char server
systemctl start rathena-char
sleep 2
systemctl status rathena-char --no-pager | head -3
echo -e "${GREEN}  ✓ Char server started${NC}"

# Start map server
systemctl start rathena-map
sleep 2
systemctl status rathena-map --no-pager | head -3
echo -e "${GREEN}  ✓ Map server started${NC}"

echo ""

# ============================================================================
# STEP 6: Start AI Services
# ============================================================================
echo -e "${YELLOW}Step 6: Starting AI services...${NC}"

# Start AI World service
systemctl start ai-world
sleep 3
systemctl status ai-world --no-pager | head -3
echo -e "${GREEN}  ✓ AI World service started${NC}"

# Start P2P Coordinator
systemctl start p2p-coordinator
sleep 2
systemctl status p2p-coordinator --no-pager | head -3
echo -e "${GREEN}  ✓ P2P Coordinator started${NC}"

echo ""

# ============================================================================
# STEP 7: Verification
# ============================================================================
echo -e "${YELLOW}Step 7: Verifying deployment...${NC}"

# Check listening ports
echo "  → Checking listening ports..."
netstat -tlnp 2>/dev/null | grep -E ":(6900|6121|5121|8000|8001|5432|3306|6379)" || ss -tlnp | grep -E ":(6900|6121|5121|8000|8001|5432|3306|6379)"

echo ""
echo -e "${GREEN}  ✓ Port verification complete${NC}"

# Check service status
echo ""
echo "  → Service Status Summary:"
systemctl is-active postgresql && echo -e "    ${GREEN}✓ PostgreSQL: active${NC}" || echo -e "    ${RED}✗ PostgreSQL: inactive${NC}"
systemctl is-active mariadb && echo -e "    ${GREEN}✓ MariaDB: active${NC}" || echo -e "    ${RED}✗ MariaDB: inactive${NC}"
systemctl is-active dragonfly && echo -e "    ${GREEN}✓ DragonflyDB: active${NC}" || echo -e "    ${RED}✗ DragonflyDB: inactive${NC}"
systemctl is-active rathena-login && echo -e "    ${GREEN}✓ rAthena Login: active${NC}" || echo -e "    ${RED}✗ rAthena Login: inactive${NC}"
systemctl is-active rathena-char && echo -e "    ${GREEN}✓ rAthena Char: active${NC}" || echo -e "    ${RED}✗ rAthena Char: inactive${NC}"
systemctl is-active rathena-map && echo -e "    ${GREEN}✓ rAthena Map: active${NC}" || echo -e "    ${RED}✗ rAthena Map: inactive${NC}"
systemctl is-active ai-world && echo -e "    ${GREEN}✓ AI World: active${NC}" || echo -e "    ${RED}✗ AI World: inactive${NC}"
systemctl is-active p2p-coordinator && echo -e "    ${GREEN}✓ P2P Coordinator: active${NC}" || echo -e "    ${RED}✗ P2P Coordinator: inactive${NC}"

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}✓ Deployment Complete!${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Server Information:"
echo "  → Server IP: $(hostname -I | awk '{print $1}')"
echo "  → Login Server: Port 6900"
echo "  → Char Server: Port 6121"
echo "  → Map Server: Port 5121"
echo "  → AI Service: Port 8000"
echo "  → P2P Coordinator: Port 8001"
echo ""
echo "Database Credentials:"
echo "  → MariaDB (rAthena):"
echo "    - Database: ragnarok"
echo "    - User: ragnarok / ragnarok"
echo "  → PostgreSQL (AI Memory):"
echo "    - Database: ai_world_memory"
echo "    - User: ai_world_user / ai_world_pass_2025"
echo "  → PostgreSQL (P2P):"
echo "    - Database: p2p_coordinator"
echo "    - User: p2p_user / p2p_pass_2025"
echo ""
echo "Service Management Commands:"
echo "  → View all services: sudo systemctl status 'rathena-*' ai-world p2p-coordinator"
echo "  → View logs: sudo journalctl -u <service-name> -f"
echo "  → Restart service: sudo systemctl restart <service-name>"
echo "  → Stop all: sudo systemctl stop rathena-map rathena-char rathena-login ai-world p2p-coordinator"
echo ""
echo "Next Steps:"
echo "  1. Configure Windows clients to connect to: $(hostname -I | awk '{print $1}')"
echo "  2. Monitor logs: sudo journalctl -f"
echo "  3. Test AI NPCs in-game"
echo ""

