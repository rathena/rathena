#!/bin/bash
# Database Setup Script for AI-rAthena Integration
# This script initializes PostgreSQL database and schema

set -e  # Exit on error

echo "=========================================="
echo "AI-rAthena Database Setup"
echo "=========================================="

# Load environment variables
if [ -f "ai-autonomous-world/ai-service/.env" ]; then
    export $(grep -v '^#' ai-autonomous-world/ai-service/.env | xargs)
    echo "✓ Loaded environment variables from .env"
else
    echo "✗ Error: .env file not found at ai-autonomous-world/ai-service/.env"
    exit 1
fi

# Database credentials from .env
DB_NAME="${POSTGRES_DB:-ai_world_memory}"
DB_USER="${POSTGRES_USER:-ai_world_user}"
DB_PASS="${POSTGRES_PASSWORD:-ai_world_pass_2025}"
DB_HOST="${POSTGRES_HOST:-localhost}"
DB_PORT="${POSTGRES_PORT:-5432}"

echo ""
echo "Database Configuration:"
echo "  Host: $DB_HOST:$DB_PORT"
echo "  Database: $DB_NAME"
echo "  User: $DB_USER"
echo ""

# Check if PostgreSQL is running
if ! pg_isready -h "$DB_HOST" -p "$DB_PORT" > /dev/null 2>&1; then
    echo "✗ Error: PostgreSQL is not running on $DB_HOST:$DB_PORT"
    echo "  Please start PostgreSQL first"
    exit 1
fi
echo "✓ PostgreSQL is running"

# Function to run SQL as postgres user
run_sql_as_postgres() {
    local sql="$1"
    echo "$sql" | sudo -u postgres psql 2>&1
}

# Function to run SQL as app user
run_sql_as_user() {
    local sql="$1"
    PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "$sql" 2>&1
}

echo ""
echo "Step 1: Creating database user and database..."
echo "  (This requires sudo access to run as postgres user)"
echo ""

# Create user if not exists
run_sql_as_postgres "
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_user WHERE usename = '$DB_USER') THEN
        CREATE USER $DB_USER WITH PASSWORD '$DB_PASS';
        RAISE NOTICE 'User $DB_USER created';
    ELSE
        RAISE NOTICE 'User $DB_USER already exists';
    END IF;
END
\$\$;
" || echo "  Note: User creation may have failed (might already exist)"

# Create database if not exists
run_sql_as_postgres "
SELECT 'CREATE DATABASE $DB_NAME OWNER $DB_USER' 
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '$DB_NAME')\gexec
" || echo "  Note: Database creation may have failed (might already exist)"

# Grant privileges
run_sql_as_postgres "
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
" || echo "  Note: Grant privileges may have failed"

echo "✓ Database and user setup complete"

echo ""
echo "Step 2: Initializing database schema..."
echo ""

# Test connection
if PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1;" > /dev/null 2>&1; then
    echo "✓ Successfully connected to database"
else
    echo "✗ Error: Cannot connect to database"
    echo "  Please check credentials in .env file"
    exit 1
fi

# Run schema initialization
if [ -f "ai-autonomous-world/database/init.sql" ]; then
    echo "  Applying schema from init.sql..."
    PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f "ai-autonomous-world/database/init.sql" > /dev/null 2>&1
    echo "✓ Database schema initialized"
else
    echo "✗ Warning: init.sql not found, skipping schema initialization"
fi

echo ""
echo "Step 3: Verifying database setup..."
echo ""

# Check tables
TABLE_COUNT=$(PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -t -c "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'public';" 2>/dev/null | tr -d ' ')

if [ "$TABLE_COUNT" -gt 0 ]; then
    echo "✓ Found $TABLE_COUNT tables in database"
    echo ""
    echo "Tables:"
    PGPASSWORD="$DB_PASS" psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "\dt" 2>/dev/null | grep "public" || echo "  (Unable to list tables)"
else
    echo "✗ Warning: No tables found in database"
fi

echo ""
echo "=========================================="
echo "Database Setup Complete!"
echo "=========================================="
echo ""
echo "Connection string:"
echo "  postgresql://$DB_USER:****@$DB_HOST:$DB_PORT/$DB_NAME"
echo ""
echo "Next steps:"
echo "  1. Run: cd ai-autonomous-world/ai-service"
echo "  2. Run: python3 -m pytest tests/ -v"
echo "  3. Run: uvicorn main:app --host 0.0.0.0 --port 8000"
echo ""

