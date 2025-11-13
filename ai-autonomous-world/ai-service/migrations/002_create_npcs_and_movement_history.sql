-- Migration: Create npcs and npc_movement_history tables for AI NPC movement system

CREATE TABLE IF NOT EXISTS npcs (
    npc_id VARCHAR(64) PRIMARY KEY,
    name VARCHAR(128) NOT NULL,
    current_x INTEGER NOT NULL DEFAULT 0,
    current_y INTEGER NOT NULL DEFAULT 0,
    current_map VARCHAR(64) NOT NULL DEFAULT '',
    can_move BOOLEAN NOT NULL DEFAULT TRUE,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    last_activity_time TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    openness FLOAT DEFAULT 0.5,
    conscientiousness FLOAT DEFAULT 0.5,
    extraversion FLOAT DEFAULT 0.5,
    agreeableness FLOAT DEFAULT 0.5,
    neuroticism FLOAT DEFAULT 0.5
);

CREATE TABLE IF NOT EXISTS npc_movement_history (
    id SERIAL PRIMARY KEY,
    npc_id VARCHAR(64) NOT NULL REFERENCES npcs(npc_id) ON DELETE CASCADE,
    decision_data JSONB NOT NULL,
    context_data JSONB NOT NULL,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);