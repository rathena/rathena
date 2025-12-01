-- Migration: Initial Schema (DOWN)
-- Rolls back the base schema by dropping all tables and partitions

DROP TABLE IF EXISTS combat_logs_2025_11 CASCADE;
DROP TABLE IF EXISTS ownership_migrations_2025_11 CASCADE;
DROP TABLE IF EXISTS combat_logs CASCADE;
DROP TABLE IF EXISTS ownership_migrations CASCADE;
DROP TABLE IF EXISTS player_inventory CASCADE;
DROP TABLE IF EXISTS entity_ownership CASCADE;
DROP TABLE IF EXISTS worker_load CASCADE;
DROP TABLE IF EXISTS player_state CASCADE;
DROP TABLE IF EXISTS peer_reputation CASCADE;
DROP TABLE IF EXISTS cheat_flags CASCADE;
DROP TABLE IF EXISTS player_accounts CASCADE;

-- Note: CASCADE ensures all dependent objects are removed.
-- For production, review dependencies before running DOWN migrations.