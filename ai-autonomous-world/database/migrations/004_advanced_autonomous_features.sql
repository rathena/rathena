-- Migration 004: Advanced Autonomous Features
-- NPC-to-NPC Interactions, Universal Consciousness, Decision Optimization

-- NPC Relationships Table
CREATE TABLE IF NOT EXISTS npc_relationships (
    npc_id_1 VARCHAR(255) NOT NULL,
    npc_id_2 VARCHAR(255) NOT NULL,
    relationship_type VARCHAR(50) NOT NULL DEFAULT 'neutral',
    relationship_value FLOAT NOT NULL DEFAULT 0.0,
    trust_level FLOAT NOT NULL DEFAULT 50.0,
    interaction_count INTEGER NOT NULL DEFAULT 0,
    last_interaction TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    metadata JSONB DEFAULT '{}',
    PRIMARY KEY (npc_id_1, npc_id_2),
    CHECK (npc_id_1 < npc_id_2),  -- Ensure consistent ordering
    CHECK (relationship_value >= -100.0 AND relationship_value <= 100.0),
    CHECK (trust_level >= 0.0 AND trust_level <= 100.0)
);

CREATE INDEX idx_npc_relationships_type ON npc_relationships(relationship_type);
CREATE INDEX idx_npc_relationships_value ON npc_relationships(relationship_value);
CREATE INDEX idx_npc_relationships_last_interaction ON npc_relationships(last_interaction);

-- NPC Interactions Table
CREATE TABLE IF NOT EXISTS npc_interactions (
    interaction_id VARCHAR(255) PRIMARY KEY,
    npc_id_1 VARCHAR(255) NOT NULL,
    npc_id_2 VARCHAR(255) NOT NULL,
    interaction_type VARCHAR(50) NOT NULL,
    location JSONB NOT NULL,
    duration FLOAT,
    outcome VARCHAR(50) NOT NULL,
    relationship_change FLOAT NOT NULL DEFAULT 0.0,
    information_shared TEXT[],
    timestamp TIMESTAMP NOT NULL DEFAULT NOW(),
    metadata JSONB DEFAULT '{}'
);

CREATE INDEX idx_npc_interactions_npc1 ON npc_interactions(npc_id_1);
CREATE INDEX idx_npc_interactions_npc2 ON npc_interactions(npc_id_2);
CREATE INDEX idx_npc_interactions_type ON npc_interactions(interaction_type);
CREATE INDEX idx_npc_interactions_timestamp ON npc_interactions(timestamp);

-- Shared Information Table
CREATE TABLE IF NOT EXISTS shared_information (
    info_id VARCHAR(255) PRIMARY KEY,
    info_type VARCHAR(50) NOT NULL,
    content TEXT NOT NULL,
    source_npc_id VARCHAR(255) NOT NULL,
    current_holder_npc_ids TEXT[] NOT NULL DEFAULT '{}',
    reliability FLOAT NOT NULL DEFAULT 0.5,
    importance FLOAT NOT NULL DEFAULT 0.5,
    expiry TIMESTAMP,
    spread_count INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    CHECK (reliability >= 0.0 AND reliability <= 1.0),
    CHECK (importance >= 0.0 AND importance <= 1.0)
);

CREATE INDEX idx_shared_info_type ON shared_information(info_type);
CREATE INDEX idx_shared_info_source ON shared_information(source_npc_id);
CREATE INDEX idx_shared_info_holders ON shared_information USING GIN(current_holder_npc_ids);
CREATE INDEX idx_shared_info_expiry ON shared_information(expiry);

-- Entity Decisions Table (for Universal Consciousness)
CREATE TABLE IF NOT EXISTS entity_decisions (
    decision_id VARCHAR(255) PRIMARY KEY,
    entity_id VARCHAR(255) NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    decision_type VARCHAR(100) NOT NULL,
    decision_data JSONB NOT NULL,
    outcome VARCHAR(50),
    timestamp TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE INDEX idx_entity_decisions_entity ON entity_decisions(entity_id, entity_type);
CREATE INDEX idx_entity_decisions_type ON entity_decisions(decision_type);
CREATE INDEX idx_entity_decisions_timestamp ON entity_decisions(timestamp);

-- Decision Outcomes Table
CREATE TABLE IF NOT EXISTS decision_outcomes (
    outcome_id SERIAL PRIMARY KEY,
    decision_id VARCHAR(255) NOT NULL,
    entity_id VARCHAR(255) NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    outcome_type VARCHAR(100) NOT NULL,
    outcome_data JSONB NOT NULL,
    success BOOLEAN NOT NULL,
    timestamp TIMESTAMP NOT NULL DEFAULT NOW(),
    FOREIGN KEY (decision_id) REFERENCES entity_decisions(decision_id) ON DELETE CASCADE
);

CREATE INDEX idx_decision_outcomes_decision ON decision_outcomes(decision_id);
CREATE INDEX idx_decision_outcomes_entity ON decision_outcomes(entity_id, entity_type);
CREATE INDEX idx_decision_outcomes_success ON decision_outcomes(success);
CREATE INDEX idx_decision_outcomes_timestamp ON decision_outcomes(timestamp);

-- Reasoning Chains Table
CREATE TABLE IF NOT EXISTS reasoning_chains (
    chain_id SERIAL PRIMARY KEY,
    decision_id VARCHAR(255) NOT NULL,
    entity_id VARCHAR(255) NOT NULL,
    entity_type VARCHAR(50) NOT NULL,
    reasoning_depth VARCHAR(20) NOT NULL,
    past_context JSONB NOT NULL DEFAULT '{}',
    present_context JSONB NOT NULL DEFAULT '{}',
    future_context JSONB NOT NULL DEFAULT '{}',
    reasoning_steps TEXT[] DEFAULT '{}',
    decision_rationale TEXT,
    confidence_score FLOAT NOT NULL DEFAULT 0.5,
    llm_calls_used INTEGER NOT NULL DEFAULT 0,
    processing_time_ms FLOAT NOT NULL DEFAULT 0.0,
    timestamp TIMESTAMP NOT NULL DEFAULT NOW(),
    CHECK (confidence_score >= 0.0 AND confidence_score <= 1.0),
    CHECK (reasoning_depth IN ('shallow', 'medium', 'deep'))
);

CREATE INDEX idx_reasoning_chains_decision ON reasoning_chains(decision_id);
CREATE INDEX idx_reasoning_chains_entity ON reasoning_chains(entity_id, entity_type);
CREATE INDEX idx_reasoning_chains_depth ON reasoning_chains(reasoning_depth);
CREATE INDEX idx_reasoning_chains_timestamp ON reasoning_chains(timestamp);

-- Decision Cache Table (for optimization)
CREATE TABLE IF NOT EXISTS decision_cache (
    cache_id SERIAL PRIMARY KEY,
    context_hash VARCHAR(64) NOT NULL UNIQUE,
    decision_data JSONB NOT NULL,
    use_count INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    last_used_at TIMESTAMP NOT NULL DEFAULT NOW(),
    expires_at TIMESTAMP NOT NULL
);

CREATE INDEX idx_decision_cache_hash ON decision_cache(context_hash);
CREATE INDEX idx_decision_cache_expires ON decision_cache(expires_at);

-- Create function to auto-update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Create triggers for auto-updating updated_at
CREATE TRIGGER update_npc_relationships_updated_at BEFORE UPDATE ON npc_relationships
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_shared_information_updated_at BEFORE UPDATE ON shared_information
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Grant permissions (adjust as needed)
-- GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO ai_service_user;
-- GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO ai_service_user;

