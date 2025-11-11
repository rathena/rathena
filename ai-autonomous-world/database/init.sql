-- ============================================
-- AI-rAthena Integration - Database Initialization
-- ============================================
-- This script initializes the PostgreSQL database
-- for NPC memories, relationships, and AI state

-- Create extension for UUID generation
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ============================================
-- NPC Memories Table
-- ============================================
CREATE TABLE IF NOT EXISTS npc_memories (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    npc_id INTEGER NOT NULL,
    npc_name VARCHAR(255) NOT NULL,
    memory_type VARCHAR(50) NOT NULL,  -- 'short_term', 'long_term', 'episodic'
    content TEXT NOT NULL,
    importance_score FLOAT DEFAULT 0.5,
    emotional_valence FLOAT DEFAULT 0.0,  -- -1.0 (negative) to 1.0 (positive)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_accessed TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    access_count INTEGER DEFAULT 0,
    metadata JSONB DEFAULT '{}'::jsonb,
    CONSTRAINT valid_importance CHECK (importance_score >= 0 AND importance_score <= 1),
    CONSTRAINT valid_valence CHECK (emotional_valence >= -1 AND emotional_valence <= 1)
);

CREATE INDEX idx_npc_memories_npc_id ON npc_memories(npc_id);
CREATE INDEX idx_npc_memories_type ON npc_memories(memory_type);
CREATE INDEX idx_npc_memories_created ON npc_memories(created_at DESC);
CREATE INDEX idx_npc_memories_importance ON npc_memories(importance_score DESC);

-- ============================================
-- NPC Relationships Table
-- ============================================
CREATE TABLE IF NOT EXISTS npc_relationships (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    npc_id INTEGER NOT NULL,
    target_id INTEGER NOT NULL,  -- Player or NPC ID
    target_type VARCHAR(20) NOT NULL,  -- 'player' or 'npc'
    relationship_type VARCHAR(50) NOT NULL,  -- 'friend', 'enemy', 'neutral', 'romantic', etc.
    affinity_score FLOAT DEFAULT 0.0,  -- -1.0 (hostile) to 1.0 (friendly)
    trust_level FLOAT DEFAULT 0.5,
    interaction_count INTEGER DEFAULT 0,
    last_interaction TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    metadata JSONB DEFAULT '{}'::jsonb,
    CONSTRAINT valid_affinity CHECK (affinity_score >= -1 AND affinity_score <= 1),
    CONSTRAINT valid_trust CHECK (trust_level >= 0 AND trust_level <= 1),
    UNIQUE(npc_id, target_id, target_type)
);

CREATE INDEX idx_npc_relationships_npc ON npc_relationships(npc_id);
CREATE INDEX idx_npc_relationships_target ON npc_relationships(target_id, target_type);
CREATE INDEX idx_npc_relationships_affinity ON npc_relationships(affinity_score DESC);

-- ============================================
-- NPC Personalities Table
-- ============================================
CREATE TABLE IF NOT EXISTS npc_personalities (
    npc_id INTEGER PRIMARY KEY,
    npc_name VARCHAR(255) NOT NULL,
    openness FLOAT DEFAULT 0.5,
    conscientiousness FLOAT DEFAULT 0.5,
    extraversion FLOAT DEFAULT 0.5,
    agreeableness FLOAT DEFAULT 0.5,
    neuroticism FLOAT DEFAULT 0.5,
    background_story TEXT,
    goals TEXT,
    fears TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT valid_openness CHECK (openness >= 0 AND openness <= 1),
    CONSTRAINT valid_conscientiousness CHECK (conscientiousness >= 0 AND conscientiousness <= 1),
    CONSTRAINT valid_extraversion CHECK (extraversion >= 0 AND extraversion <= 1),
    CONSTRAINT valid_agreeableness CHECK (agreeableness >= 0 AND agreeableness <= 1),
    CONSTRAINT valid_neuroticism CHECK (neuroticism >= 0 AND neuroticism <= 1)
);

CREATE INDEX idx_npc_personalities_name ON npc_personalities(npc_name);

-- ============================================
-- Faction System Tables
-- ============================================
CREATE TABLE IF NOT EXISTS factions (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) UNIQUE NOT NULL,
    description TEXT,
    alignment VARCHAR(50),  -- 'good', 'evil', 'neutral'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    metadata JSONB DEFAULT '{}'::jsonb
);

CREATE TABLE IF NOT EXISTS faction_reputations (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    player_id INTEGER NOT NULL,
    faction_id INTEGER NOT NULL REFERENCES factions(id),
    reputation_score INTEGER DEFAULT 0,
    rank VARCHAR(100),
    last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(player_id, faction_id)
);

CREATE INDEX idx_faction_reputations_player ON faction_reputations(player_id);
CREATE INDEX idx_faction_reputations_faction ON faction_reputations(faction_id);

-- ============================================
-- Quest Generation History
-- ============================================
CREATE TABLE IF NOT EXISTS generated_quests (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    quest_id INTEGER,
    npc_id INTEGER NOT NULL,
    player_id INTEGER,
    quest_type VARCHAR(50),
    difficulty VARCHAR(20),
    generated_content JSONB NOT NULL,
    status VARCHAR(20) DEFAULT 'active',  -- 'active', 'completed', 'failed', 'abandoned'
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP
);

CREATE INDEX idx_generated_quests_npc ON generated_quests(npc_id);
CREATE INDEX idx_generated_quests_player ON generated_quests(player_id);
CREATE INDEX idx_generated_quests_status ON generated_quests(status);

-- ============================================
-- Performance Metrics
-- ============================================
CREATE TABLE IF NOT EXISTS ai_performance_metrics (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    endpoint VARCHAR(255) NOT NULL,
    response_time_ms INTEGER NOT NULL,
    success BOOLEAN DEFAULT true,
    error_message TEXT,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    metadata JSONB DEFAULT '{}'::jsonb
);

CREATE INDEX idx_ai_metrics_endpoint ON ai_performance_metrics(endpoint);
CREATE INDEX idx_ai_metrics_timestamp ON ai_performance_metrics(timestamp DESC);

-- Grant permissions
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO rathena;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO rathena;

