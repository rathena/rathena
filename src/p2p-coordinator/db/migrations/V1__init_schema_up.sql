-- Migration: Initial Schema (UP)
-- Applies the base schema for all tables

-- (Repeat of db/schema_postgres.sql, but without partition examples for brevity)
-- See db/schema_postgres.sql for full DDL details

-- Create tables
-- ... (see schema_postgres.sql for full content)
\i ../schema_postgres.sql

-- Add any additional migration-specific logic here if needed