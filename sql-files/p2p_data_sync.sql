-- P2P Data Synchronization SQL Schema
-- This file defines the database schema for the P2P data synchronization system.
-- It includes tables for vector embeddings, sync operations, and security validation.

-- Enable pgvector extension if not already enabled
CREATE EXTENSION IF NOT EXISTS vector;

-- Create schema for P2P data sync
CREATE SCHEMA IF NOT EXISTS p2p_sync;

-- Table for storing vector embeddings
CREATE TABLE IF NOT EXISTS p2p_sync.embeddings (
    id TEXT PRIMARY KEY,
    embedding VECTOR(1536),
    metadata JSONB,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Create index on embeddings for similarity search
CREATE INDEX IF NOT EXISTS p2p_embeddings_embedding_idx 
ON p2p_sync.embeddings USING ivfflat (embedding vector_cosine_ops);

-- Table for tracking sync operations
CREATE TABLE IF NOT EXISTS p2p_sync.operations (
    operation_id TEXT PRIMARY KEY,
    account_id INTEGER NOT NULL,
    map_name TEXT NOT NULL,
    data_type INTEGER NOT NULL,
    status TEXT NOT NULL,
    error_message TEXT,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    completed_at TIMESTAMP WITH TIME ZONE,
    is_priority BOOLEAN DEFAULT FALSE,
    retry_count INTEGER DEFAULT 0,
    data TEXT,
    encrypted_data TEXT
);

-- Create index on account_id and map_name
CREATE INDEX IF NOT EXISTS p2p_operations_account_map_idx 
ON p2p_sync.operations (account_id, map_name);

-- Create index on created_at for cleanup
CREATE INDEX IF NOT EXISTS p2p_operations_created_at_idx 
ON p2p_sync.operations (created_at);

-- Table for storing map state data
CREATE TABLE IF NOT EXISTS p2p_sync.map_states (
    map_name TEXT PRIMARY KEY,
    state JSONB NOT NULL,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    host_account_id INTEGER,
    is_vps_hosted BOOLEAN DEFAULT TRUE,
    backup_host_ids INTEGER[],
    assistance_host_ids INTEGER[]
);

-- Table for security validation results
CREATE TABLE IF NOT EXISTS p2p_sync.security_validations (
    validation_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL,
    map_name TEXT NOT NULL,
    validation_type TEXT NOT NULL,
    is_valid BOOLEAN NOT NULL,
    details JSONB,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Create index on account_id and map_name for security validations
CREATE INDEX IF NOT EXISTS p2p_security_validations_account_map_idx 
ON p2p_sync.security_validations (account_id, map_name);

-- Table for caching frequently accessed data
CREATE TABLE IF NOT EXISTS p2p_sync.cache (
    cache_key TEXT PRIMARY KEY,
    cache_value TEXT NOT NULL,
    expires_at TIMESTAMP WITH TIME ZONE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Create index on expires_at for cleanup
CREATE INDEX IF NOT EXISTS p2p_cache_expires_at_idx 
ON p2p_sync.cache (expires_at);

-- Table for tracking P2P host metrics
CREATE TABLE IF NOT EXISTS p2p_sync.host_metrics (
    account_id INTEGER PRIMARY KEY,
    cpu_ghz DOUBLE PRECISION NOT NULL,
    cpu_cores INTEGER NOT NULL,
    free_ram_mb BIGINT NOT NULL,
    network_speed_mbps INTEGER NOT NULL,
    latency_ms INTEGER NOT NULL,
    disconnects_per_hour INTEGER NOT NULL,
    last_health_check TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    consecutive_failed_checks INTEGER DEFAULT 0,
    regional_latencies JSONB,
    vps_connection_active BOOLEAN DEFAULT TRUE,
    host_score DOUBLE PRECISION,
    last_updated TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);

-- Table for tracking P2P host assignments
CREATE TABLE IF NOT EXISTS p2p_sync.host_assignments (
    assignment_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL,
    map_name TEXT NOT NULL,
    assigned_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
    unassigned_at TIMESTAMP WITH TIME ZONE,
    is_active BOOLEAN DEFAULT TRUE,
    is_backup BOOLEAN DEFAULT FALSE,
    is_assistance BOOLEAN DEFAULT FALSE,
    UNIQUE (account_id, map_name, is_active)
);

-- Create index on account_id and is_active
CREATE INDEX IF NOT EXISTS p2p_host_assignments_account_active_idx 
ON p2p_sync.host_assignments (account_id, is_active);

-- Create index on map_name and is_active
CREATE INDEX IF NOT EXISTS p2p_host_assignments_map_active_idx 
ON p2p_sync.host_assignments (map_name, is_active);

-- Function to update the updated_at timestamp
CREATE OR REPLACE FUNCTION p2p_sync.update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Trigger to update the updated_at timestamp for embeddings
CREATE TRIGGER update_embeddings_updated_at
BEFORE UPDATE ON p2p_sync.embeddings
FOR EACH ROW
EXECUTE FUNCTION p2p_sync.update_updated_at_column();

-- Function to clean up old operations
CREATE OR REPLACE FUNCTION p2p_sync.cleanup_old_operations()
RETURNS INTEGER AS $$
DECLARE
    deleted_count INTEGER;
BEGIN
    DELETE FROM p2p_sync.operations
    WHERE created_at < NOW() - INTERVAL '7 days'
    AND (status = 'completed' OR status = 'failed');
    
    GET DIAGNOSTICS deleted_count = ROW_COUNT;
    RETURN deleted_count;
END;
$$ LANGUAGE plpgsql;

-- Function to clean up expired cache entries
CREATE OR REPLACE FUNCTION p2p_sync.cleanup_expired_cache()
RETURNS INTEGER AS $$
DECLARE
    deleted_count INTEGER;
BEGIN
    DELETE FROM p2p_sync.cache
    WHERE expires_at IS NOT NULL AND expires_at < NOW();
    
    GET DIAGNOSTICS deleted_count = ROW_COUNT;
    RETURN deleted_count;
END;
$$ LANGUAGE plpgsql;

-- Function to calculate host score
CREATE OR REPLACE FUNCTION p2p_sync.calculate_host_score(
    p_cpu_ghz DOUBLE PRECISION,
    p_cpu_cores INTEGER,
    p_free_ram_mb BIGINT,
    p_network_speed_mbps INTEGER,
    p_latency_ms INTEGER,
    p_disconnects_per_hour INTEGER,
    p_min_cpu_ghz DOUBLE PRECISION,
    p_min_cpu_cores INTEGER,
    p_min_ram_mb BIGINT,
    p_min_network_mbps INTEGER,
    p_max_latency_ms INTEGER,
    p_max_disconnects_per_hour INTEGER
)
RETURNS DOUBLE PRECISION AS $$
DECLARE
    cpu_score DOUBLE PRECISION;
    ram_score DOUBLE PRECISION;
    network_score DOUBLE PRECISION;
    latency_score DOUBLE PRECISION;
    stability_score DOUBLE PRECISION;
    total_score DOUBLE PRECISION;
BEGIN
    -- Calculate individual scores
    cpu_score := (p_cpu_ghz * p_cpu_cores) / p_min_cpu_ghz;
    ram_score := p_free_ram_mb::DOUBLE PRECISION / p_min_ram_mb;
    network_score := p_network_speed_mbps::DOUBLE PRECISION / p_min_network_mbps;
    
    -- Latency score (lower is better)
    IF p_max_latency_ms > 0 THEN
        latency_score := p_max_latency_ms::DOUBLE PRECISION / (p_latency_ms + 1);
    ELSE
        latency_score := 1.0;
    END IF;
    
    -- Stability score (lower disconnects is better)
    IF p_max_disconnects_per_hour > 0 THEN
        stability_score := 1.0 - (p_disconnects_per_hour::DOUBLE PRECISION / p_max_disconnects_per_hour);
    ELSE
        stability_score := 1.0;
    END IF;
    
    -- Combine scores with weights
    total_score := (cpu_score * 0.2) + (ram_score * 0.2) + (network_score * 0.2) + 
                  (latency_score * 0.3) + (stability_score * 0.1);
    
    RETURN total_score;
END;
$$ LANGUAGE plpgsql;

-- Function to update host metrics and score
CREATE OR REPLACE FUNCTION p2p_sync.update_host_score(p_account_id INTEGER)
RETURNS DOUBLE PRECISION AS $$
DECLARE
    v_host_score DOUBLE PRECISION;
    v_min_cpu_ghz DOUBLE PRECISION := 3.0;
    v_min_cpu_cores INTEGER := 4;
    v_min_ram_mb BIGINT := 8192;
    v_min_network_mbps INTEGER := 100;
    v_max_latency_ms INTEGER := 30;
    v_max_disconnects_per_hour INTEGER := 1;
BEGIN
    -- Get configuration values from a config table if available
    -- For now, using hardcoded values
    
    -- Calculate the host score
    SELECT p2p_sync.calculate_host_score(
        cpu_ghz, cpu_cores, free_ram_mb, network_speed_mbps, latency_ms, disconnects_per_hour,
        v_min_cpu_ghz, v_min_cpu_cores, v_min_ram_mb, v_min_network_mbps, v_max_latency_ms, v_max_disconnects_per_hour
    ) INTO v_host_score
    FROM p2p_sync.host_metrics
    WHERE account_id = p_account_id;
    
    -- Update the host score
    UPDATE p2p_sync.host_metrics
    SET host_score = v_host_score,
        last_updated = NOW()
    WHERE account_id = p_account_id;
    
    RETURN v_host_score;
END;
$$ LANGUAGE plpgsql;

-- Function to get the best host for a map
CREATE OR REPLACE FUNCTION p2p_sync.get_best_host_for_map(p_map_name TEXT)
RETURNS INTEGER AS $$
DECLARE
    v_best_host_id INTEGER;
BEGIN
    -- Get the host with the highest score
    SELECT account_id INTO v_best_host_id
    FROM p2p_sync.host_metrics
    WHERE vps_connection_active = TRUE
    AND consecutive_failed_checks < 3
    ORDER BY host_score DESC
    LIMIT 1;
    
    -- If no host found, return 0 (VPS)
    IF v_best_host_id IS NULL THEN
        RETURN 0;
    END IF;
    
    -- Assign the map to the host
    INSERT INTO p2p_sync.host_assignments (account_id, map_name, is_active, is_backup, is_assistance)
    VALUES (v_best_host_id, p_map_name, TRUE, FALSE, FALSE)
    ON CONFLICT (account_id, map_name, is_active)
    DO UPDATE SET assigned_at = NOW(), is_backup = FALSE, is_assistance = FALSE;
    
    RETURN v_best_host_id;
END;
$$ LANGUAGE plpgsql;

-- Function to get backup hosts for a map
CREATE OR REPLACE FUNCTION p2p_sync.get_backup_hosts_for_map(p_map_name TEXT, p_count INTEGER)
RETURNS SETOF INTEGER AS $$
BEGIN
    -- Get hosts with the highest scores, excluding the current host
    RETURN QUERY
    SELECT hm.account_id
    FROM p2p_sync.host_metrics hm
    WHERE hm.vps_connection_active = TRUE
    AND hm.consecutive_failed_checks < 3
    AND hm.account_id NOT IN (
        SELECT ha.account_id
        FROM p2p_sync.host_assignments ha
        WHERE ha.map_name = p_map_name
        AND ha.is_active = TRUE
        AND ha.is_backup = FALSE
        AND ha.is_assistance = FALSE
    )
    ORDER BY hm.host_score DESC
    LIMIT p_count;
END;
$$ LANGUAGE plpgsql;

-- Function to migrate a map from one host to another
CREATE OR REPLACE FUNCTION p2p_sync.migrate_map(p_map_name TEXT, p_from_host_id INTEGER, p_to_host_id INTEGER)
RETURNS BOOLEAN AS $$
BEGIN
    -- Update the current host assignment
    UPDATE p2p_sync.host_assignments
    SET is_active = FALSE,
        unassigned_at = NOW()
    WHERE map_name = p_map_name
    AND account_id = p_from_host_id
    AND is_active = TRUE;
    
    -- If migrating to VPS (p_to_host_id = 0), just update the map state
    IF p_to_host_id = 0 THEN
        UPDATE p2p_sync.map_states
        SET host_account_id = NULL,
            is_vps_hosted = TRUE,
            last_updated = NOW()
        WHERE map_name = p_map_name;
        
        RETURN TRUE;
    END IF;
    
    -- Assign the map to the new host
    INSERT INTO p2p_sync.host_assignments (account_id, map_name, is_active, is_backup, is_assistance)
    VALUES (p_to_host_id, p_map_name, TRUE, FALSE, FALSE)
    ON CONFLICT (account_id, map_name, is_active)
    DO UPDATE SET assigned_at = NOW(), is_backup = FALSE, is_assistance = FALSE;
    
    -- Update the map state
    UPDATE p2p_sync.map_states
    SET host_account_id = p_to_host_id,
        is_vps_hosted = FALSE,
        last_updated = NOW()
    WHERE map_name = p_map_name;
    
    RETURN TRUE;
END;
$$ LANGUAGE plpgsql;