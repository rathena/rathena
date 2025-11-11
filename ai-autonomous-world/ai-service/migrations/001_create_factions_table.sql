-- Create factions table for AI autonomous world
-- This table stores faction information for the faction system

CREATE TABLE IF NOT EXISTS factions (
    faction_id VARCHAR(255) PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    faction_type VARCHAR(50) NOT NULL DEFAULT 'neutral',
    description TEXT,
    
    -- Leadership
    leader_npc_id VARCHAR(255),
    leader_name VARCHAR(255),
    
    -- Territory (stored as JSONB for flexibility)
    capital_location JSONB DEFAULT '{}',
    controlled_areas JSONB DEFAULT '[]',
    
    -- Resources
    wealth INTEGER DEFAULT 0,
    military_power INTEGER DEFAULT 0,
    influence INTEGER DEFAULT 0,
    
    -- Relationships with other factions (stored as JSONB)
    relationships JSONB DEFAULT '{}',
    
    -- Member count
    npc_members JSONB DEFAULT '[]',
    player_members INTEGER DEFAULT 0,
    
    -- Faction-specific data
    ideology VARCHAR(255) DEFAULT 'neutral',
    goals JSONB DEFAULT '[]',
    
    -- Timestamps
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_updated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Create index on faction_type for faster queries
CREATE INDEX IF NOT EXISTS idx_factions_type ON factions(faction_type);

-- Create index on created_at for sorting
CREATE INDEX IF NOT EXISTS idx_factions_created ON factions(created_at);

-- Create player_reputation table
CREATE TABLE IF NOT EXISTS player_reputation (
    id SERIAL PRIMARY KEY,
    player_id VARCHAR(255) NOT NULL,
    faction_id VARCHAR(255) NOT NULL,
    reputation INTEGER DEFAULT 0,
    tier VARCHAR(50) DEFAULT 'neutral',
    
    -- History (stored as JSONB)
    reputation_history JSONB DEFAULT '[]',
    
    -- Rewards unlocked
    unlocked_rewards JSONB DEFAULT '[]',
    
    -- Timestamp
    last_updated TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    -- Unique constraint on player_id + faction_id
    UNIQUE(player_id, faction_id)
);

-- Create index on player_id for faster lookups
CREATE INDEX IF NOT EXISTS idx_player_reputation_player ON player_reputation(player_id);

-- Create index on faction_id for faster lookups
CREATE INDEX IF NOT EXISTS idx_player_reputation_faction ON player_reputation(faction_id);

-- Create faction_events table
CREATE TABLE IF NOT EXISTS faction_events (
    event_id VARCHAR(255) PRIMARY KEY,
    event_type VARCHAR(100) NOT NULL,
    description TEXT,
    
    -- Involved factions (stored as JSONB array)
    involved_factions JSONB DEFAULT '[]',
    
    -- Impact (stored as JSONB)
    reputation_changes JSONB DEFAULT '{}',
    relationship_changes JSONB DEFAULT '{}',
    
    -- Event properties
    duration INTEGER,
    severity REAL DEFAULT 0.5,
    
    -- Timestamps
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP
);

-- Create index on event_type
CREATE INDEX IF NOT EXISTS idx_faction_events_type ON faction_events(event_type);

-- Create index on created_at
CREATE INDEX IF NOT EXISTS idx_faction_events_created ON faction_events(created_at);

-- Create faction_conflicts table
CREATE TABLE IF NOT EXISTS faction_conflicts (
    conflict_id VARCHAR(255) PRIMARY KEY,
    faction_a VARCHAR(255) NOT NULL,
    faction_b VARCHAR(255) NOT NULL,
    conflict_type VARCHAR(100) NOT NULL,
    intensity REAL DEFAULT 0.5,
    
    -- Progress
    faction_a_score INTEGER DEFAULT 0,
    faction_b_score INTEGER DEFAULT 0,
    
    -- Objectives (stored as JSONB)
    objectives JSONB DEFAULT '[]',
    
    -- Timestamps
    started_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    ends_at TIMESTAMP
);

-- Create index on faction_a and faction_b
CREATE INDEX IF NOT EXISTS idx_faction_conflicts_factions ON faction_conflicts(faction_a, faction_b);

-- Create index on started_at
CREATE INDEX IF NOT EXISTS idx_faction_conflicts_started ON faction_conflicts(started_at);

