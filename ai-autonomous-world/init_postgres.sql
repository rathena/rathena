-- Initialize PostgreSQL database for AI World Memory (OpenMemory)
-- This script creates the database, user, and required extensions

-- Create user if not exists
DO
$$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'ai_world_user') THEN
        CREATE USER ai_world_user WITH PASSWORD 'ai_world_pass_2025';
        RAISE NOTICE 'User ai_world_user created';
    ELSE
        RAISE NOTICE 'User ai_world_user already exists';
    END IF;
END
$$;

-- Create database if not exists
SELECT 'CREATE DATABASE ai_world_memory OWNER ai_world_user'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'ai_world_memory')\gexec

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE ai_world_memory TO ai_world_user;

-- Connect to the new database and create extensions
\c ai_world_memory

-- Enable required extensions
CREATE EXTENSION IF NOT EXISTS vector;
CREATE EXTENSION IF NOT EXISTS timescaledb;
CREATE EXTENSION IF NOT EXISTS age;

-- Grant schema permissions
GRANT ALL ON SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_world_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO ai_world_user;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO ai_world_user;

-- Create OpenMemory tables
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
    target_type VARCHAR(20) NOT NULL, -- 'npc' or 'player'
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

-- Create hypertable for player interactions
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

-- Create hypertable for world events
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

-- Create hypertable for NPC states
SELECT create_hypertable('npc_states', 'created_at', if_not_exists => TRUE);

-- Grant permissions on all tables
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_world_user;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_world_user;

\echo 'PostgreSQL database initialization complete!'
\echo 'Database: ai_world_memory'
\echo 'User: ai_world_user'
\echo 'Extensions: vector, timescaledb, age'
\echo 'Tables created: npc_memories, npc_relationships, player_interactions, world_events, npc_states'

