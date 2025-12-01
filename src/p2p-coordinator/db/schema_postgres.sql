-- PostgreSQL Schema for AI-MMORPG World
-- Generated: 2025-11-16

-- 1. player_accounts
CREATE TABLE player_accounts (
    id SERIAL PRIMARY KEY,
    username VARCHAR(64) NOT NULL UNIQUE,
    email VARCHAR(128) NOT NULL UNIQUE,
    password_hash VARCHAR(128) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT NOW(),
    last_login TIMESTAMP,
    is_banned BOOLEAN NOT NULL DEFAULT FALSE
);

-- 2. player_inventory
CREATE TABLE player_inventory (
    id SERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    item_id INTEGER NOT NULL,
    quantity INTEGER NOT NULL CHECK (quantity >= 0),
    acquired_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE(player_id, item_id)
);

-- 3. entity_ownership
CREATE TABLE entity_ownership (
    id SERIAL PRIMARY KEY,
    entity_id UUID NOT NULL,
    owner_player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    acquired_at TIMESTAMP NOT NULL DEFAULT NOW(),
    released_at TIMESTAMP,
    is_active BOOLEAN NOT NULL DEFAULT TRUE,
    UNIQUE(entity_id, is_active) -- Only one active owner per entity
);

-- 4. combat_logs (partitioned by month)
CREATE TABLE combat_logs (
    id BIGSERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    target_entity_id UUID NOT NULL,
    action_type VARCHAR(32) NOT NULL,
    damage INTEGER,
    result VARCHAR(32),
    created_at TIMESTAMP NOT NULL DEFAULT NOW()
) PARTITION BY RANGE (created_at);

-- Example partition for November 2025
CREATE TABLE combat_logs_2025_11 PARTITION OF combat_logs
    FOR VALUES FROM ('2025-11-01') TO ('2025-12-01');

-- 5. worker_load
CREATE TABLE worker_load (
    id SERIAL PRIMARY KEY,
    worker_id UUID NOT NULL,
    load_metric INTEGER NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT NOW()
);

-- 6. player_state
CREATE TABLE player_state (
    id SERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    state_json JSONB NOT NULL,
    updated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE(player_id)
);

-- 7. ownership_migrations (partitioned by month)
CREATE TABLE ownership_migrations (
    id BIGSERIAL PRIMARY KEY,
    entity_id UUID NOT NULL,
    from_player_id INTEGER REFERENCES player_accounts(id),
    to_player_id INTEGER REFERENCES player_accounts(id),
    migrated_at TIMESTAMP NOT NULL DEFAULT NOW(),
    reason VARCHAR(128)
) PARTITION BY RANGE (migrated_at);

-- Example partition for November 2025
CREATE TABLE ownership_migrations_2025_11 PARTITION OF ownership_migrations
    FOR VALUES FROM ('2025-11-01') TO ('2025-12-01');

-- 8. peer_reputation
CREATE TABLE peer_reputation (
    id SERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    peer_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    reputation_score INTEGER NOT NULL DEFAULT 0,
    last_updated TIMESTAMP NOT NULL DEFAULT NOW(),
    UNIQUE(player_id, peer_id)
);

-- 9. cheat_flags
CREATE TABLE cheat_flags (
    id SERIAL PRIMARY KEY,
    player_id INTEGER NOT NULL REFERENCES player_accounts(id) ON DELETE CASCADE,
    flag_type VARCHAR(64) NOT NULL,
    details TEXT,
    flagged_at TIMESTAMP NOT NULL DEFAULT NOW(),
    resolved BOOLEAN NOT NULL DEFAULT FALSE
);

-- Indexes for performance
CREATE INDEX idx_combat_logs_player_id ON combat_logs(player_id);
CREATE INDEX idx_combat_logs_created_at ON combat_logs(created_at);
CREATE INDEX idx_ownership_migrations_entity_id ON ownership_migrations(entity_id);
CREATE INDEX idx_player_inventory_player_id ON player_inventory(player_id);
CREATE INDEX idx_entity_ownership_entity_id ON entity_ownership(entity_id);
CREATE INDEX idx_cheat_flags_player_id ON cheat_flags(player_id);

-- Foreign key constraints and data integrity are enforced throughout.