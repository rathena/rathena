#!/bin/bash
# Initialize PostgreSQL database for AI World Memory
# This script creates the database, user, and required extensions

set -e

echo "=== PostgreSQL Database Initialization for AI World Memory ==="
echo ""

# Database configuration
DB_NAME="ai_world_memory"
DB_USER="ai_world_user"
DB_PASS="ai_world_pass_2025"

# Check if running as postgres user or with sudo
if [ "$EUID" -eq 0 ] || [ "$(whoami)" = "postgres" ]; then
    PSQL_CMD="psql"
else
    echo "This script needs to be run with sudo or as postgres user"
    echo "Usage: sudo ./init_postgres.sh"
    echo "   or: sudo -u postgres ./init_postgres.sh"
    exit 1
fi

echo "Step 1: Creating database user..."
$PSQL_CMD -U postgres <<EOF
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = '$DB_USER') THEN
        CREATE USER $DB_USER WITH PASSWORD '$DB_PASS';
        RAISE NOTICE 'User $DB_USER created';
    ELSE
        RAISE NOTICE 'User $DB_USER already exists';
    END IF;
END
\$\$;
EOF

echo "Step 2: Creating database..."
$PSQL_CMD -U postgres <<EOF
SELECT 'CREATE DATABASE $DB_NAME OWNER $DB_USER'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '$DB_NAME')\gexec
EOF

echo "Step 3: Granting privileges..."
$PSQL_CMD -U postgres <<EOF
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
EOF

echo "Step 4: Creating extensions..."
$PSQL_CMD -U postgres -d $DB_NAME <<EOF
CREATE EXTENSION IF NOT EXISTS vector;
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS age;
EOF

echo "Step 5: Setting up schema permissions..."
$PSQL_CMD -U postgres -d $DB_NAME <<EOF
GRANT ALL ON SCHEMA public TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO $DB_USER;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO $DB_USER;
EOF

echo "Step 6: Creating OpenMemory tables..."
$PSQL_CMD -U postgres -d $DB_NAME <<'EOF'
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

-- Create hypertable for time-series data
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

-- Create hypertable
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
EOF

echo "Step 7: Granting final permissions..."
$PSQL_CMD -U postgres -d $DB_NAME <<EOF
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO $DB_USER;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO $DB_USER;
EOF

echo ""
echo "✓ PostgreSQL database initialization complete!"
echo "  Database: $DB_NAME"
echo "  User: $DB_USER"
echo "  Extensions: vector, timescaledb, age"
echo "  Tables: npc_memories, npc_relationships, player_interactions, world_events, npc_states"
echo ""
echo "You can now start the AI service with: cd ai-service && source ../venv/bin/activate && python main.py"

