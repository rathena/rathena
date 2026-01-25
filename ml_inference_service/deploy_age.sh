#!/bin/bash
# ============================================================================
# Apache AGE Graph Database Deployment Script
# ============================================================================
# Purpose: Automated deployment and verification of Apache AGE graphs
#          for ML Monster AI system
#
# Architecture Reference: plans/enhanced_hybrid_ml_monster_ai_architecture_v2.md
# Version: 2.0
# Date: 2026-01-25
#
# Usage:
#   ./deploy_age.sh [--check-only] [--skip-install-check]
#
# Options:
#   --check-only: Only check if AGE is installed, don't deploy
#   --skip-install-check: Skip AGE installation verification
#
# Environment Variables:
#   POSTGRES_HOST: PostgreSQL host (default: localhost)
#   POSTGRES_PORT: PostgreSQL port (default: 5432)
#   POSTGRES_DB: Database name (default: ragnarok_ml)
#   POSTGRES_USER: Database user (default: ml_user)
#   POSTGRES_PASSWORD: Database password (required)
# ============================================================================

set -e  # Exit on error
set -u  # Exit on undefined variable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
POSTGRES_HOST="${POSTGRES_HOST:-localhost}"
POSTGRES_PORT="${POSTGRES_PORT:-5432}"
POSTGRES_DB="${POSTGRES_DB:-ragnarok_ml}"
POSTGRES_USER="${POSTGRES_USER:-ml_user}"
POSTGRES_PASSWORD="${POSTGRES_PASSWORD:-}"

# Script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SQL_FILE="${SCRIPT_DIR}/setup_age_graphs.sql"

# Flags
CHECK_ONLY=false
SKIP_INSTALL_CHECK=false

# Parse arguments
for arg in "$@"; do
    case $arg in
        --check-only)
            CHECK_ONLY=true
            shift
            ;;
        --skip-install-check)
            SKIP_INSTALL_CHECK=true
            shift
            ;;
        *)
            ;;
    esac
done

# ============================================================================
# HELPER FUNCTIONS
# ============================================================================

print_header() {
    echo -e "${BLUE}============================================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}============================================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${BLUE}→ $1${NC}"
}

# Execute SQL query
execute_sql() {
    local query="$1"
    local description="$2"
    
    print_info "$description"
    
    if [ -z "$POSTGRES_PASSWORD" ]; then
        psql -h "$POSTGRES_HOST" -p "$POSTGRES_PORT" -U "$POSTGRES_USER" -d "$POSTGRES_DB" -c "$query" 2>&1
    else
        PGPASSWORD="$POSTGRES_PASSWORD" psql -h "$POSTGRES_HOST" -p "$POSTGRES_PORT" -U "$POSTGRES_USER" -d "$POSTGRES_DB" -c "$query" 2>&1
    fi
}

# Execute SQL file
execute_sql_file() {
    local file="$1"
    local description="$2"
    
    print_info "$description"
    
    if [ -z "$POSTGRES_PASSWORD" ]; then
        psql -h "$POSTGRES_HOST" -p "$POSTGRES_PORT" -U "$POSTGRES_USER" -d "$POSTGRES_DB" -f "$file"
    else
        PGPASSWORD="$POSTGRES_PASSWORD" psql -h "$POSTGRES_HOST" -p "$POSTGRES_PORT" -U "$POSTGRES_USER" -d "$POSTGRES_DB" -f "$file"
    fi
}

# ============================================================================
# MAIN DEPLOYMENT
# ============================================================================

print_header "Apache AGE Graph Database Deployment"

echo ""
echo "Configuration:"
echo "  PostgreSQL Host: $POSTGRES_HOST"
echo "  PostgreSQL Port: $POSTGRES_PORT"
echo "  Database: $POSTGRES_DB"
echo "  User: $POSTGRES_USER"
echo "  SQL File: $SQL_FILE"
echo ""

# ============================================================================
# STEP 1: VERIFY POSTGRESQL CONNECTION
# ============================================================================

print_header "Step 1: Verify PostgreSQL Connection"

if ! command -v psql &> /dev/null; then
    print_error "psql command not found. Please install PostgreSQL client tools."
    exit 1
fi

print_info "Testing PostgreSQL connection..."

if execute_sql "SELECT version();" "Checking PostgreSQL version" > /dev/null 2>&1; then
    VERSION=$(execute_sql "SELECT version();" "PostgreSQL Version" | grep PostgreSQL | head -n1)
    print_success "PostgreSQL connection successful"
    echo "  $VERSION"
else
    print_error "Failed to connect to PostgreSQL"
    print_info "Check your connection parameters:"
    echo "    Host: $POSTGRES_HOST"
    echo "    Port: $POSTGRES_PORT"
    echo "    Database: $POSTGRES_DB"
    echo "    User: $POSTGRES_USER"
    exit 1
fi

# ============================================================================
# STEP 2: CHECK APACHE AGE EXTENSION
# ============================================================================

print_header "Step 2: Check Apache AGE Extension"

print_info "Checking if Apache AGE extension is installed..."

AGE_CHECK=$(execute_sql "SELECT COUNT(*) FROM pg_available_extensions WHERE name = 'age';" "Checking available extensions" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ "$AGE_CHECK" = "1" ]; then
    print_success "Apache AGE extension is available"
    
    # Check if already loaded
    AGE_LOADED=$(execute_sql "SELECT COUNT(*) FROM pg_extension WHERE extname = 'age';" "Checking loaded extensions" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')
    
    if [ "$AGE_LOADED" = "1" ]; then
        AGE_VERSION=$(execute_sql "SELECT extversion FROM pg_extension WHERE extname = 'age';" "AGE version" | grep -E "^[0-9]+\." | head -n1 | tr -d ' ')
        print_success "Apache AGE extension is already loaded (version $AGE_VERSION)"
    else
        print_info "Apache AGE is available but not loaded yet"
    fi
else
    print_error "Apache AGE extension is NOT installed"
    echo ""
    echo "================================ INSTALLATION INSTRUCTIONS ================================"
    echo ""
    echo "Apache AGE must be installed before proceeding. Follow these steps:"
    echo ""
    echo "1. Install build dependencies:"
    echo "   sudo apt-get update"
    echo "   sudo apt-get install build-essential postgresql-server-dev-17 flex bison"
    echo ""
    echo "2. Clone and build Apache AGE:"
    echo "   git clone https://github.com/apache/age.git"
    echo "   cd age"
    echo "   make PG_CONFIG=/usr/bin/pg_config"
    echo "   sudo make PG_CONFIG=/usr/bin/pg_config install"
    echo ""
    echo "3. Enable AGE in postgresql.conf:"
    echo "   Add to shared_preload_libraries: shared_preload_libraries = 'age'"
    echo "   Restart PostgreSQL: sudo systemctl restart postgresql"
    echo ""
    echo "4. Re-run this script:"
    echo "   ./deploy_age.sh"
    echo ""
    echo "==========================================================================================="
    echo ""
    
    if [ "$SKIP_INSTALL_CHECK" = false ]; then
        exit 1
    else
        print_warning "Skipping installation check as requested (--skip-install-check)"
    fi
fi

if [ "$CHECK_ONLY" = true ]; then
    print_success "Check complete. Apache AGE is ready."
    exit 0
fi

# ============================================================================
# STEP 3: VERIFY SQL FILE EXISTS
# ============================================================================

print_header "Step 3: Verify SQL File"

if [ ! -f "$SQL_FILE" ]; then
    print_error "SQL file not found: $SQL_FILE"
    exit 1
fi

print_success "SQL file found: $SQL_FILE"
FILE_SIZE=$(wc -l < "$SQL_FILE")
print_info "  Lines: $FILE_SIZE"

# ============================================================================
# STEP 4: EXECUTE SETUP SQL
# ============================================================================

print_header "Step 4: Execute Graph Setup SQL"

print_info "Executing setup_age_graphs.sql..."
echo ""

if execute_sql_file "$SQL_FILE" "Creating AGE graphs and schemas"; then
    print_success "SQL execution completed successfully"
else
    print_error "SQL execution failed"
    print_info "Check the error messages above for details"
    exit 1
fi

# ============================================================================
# STEP 5: VERIFY GRAPH CREATION
# ============================================================================

print_header "Step 5: Verify Graph Creation"

print_info "Verifying monster_ai graph exists..."

GRAPH_EXISTS=$(execute_sql "SELECT EXISTS(SELECT 1 FROM ag_graph WHERE name = 'monster_ai');" "Checking graph" | grep -E "^\s*[tf]\s*$" | tr -d ' ')

if [ "$GRAPH_EXISTS" = "t" ]; then
    print_success "Graph 'monster_ai' exists"
else
    print_error "Graph 'monster_ai' was not created"
    exit 1
fi

# ============================================================================
# STEP 6: VERIFY NODE AND EDGE CREATION
# ============================================================================

print_header "Step 6: Verify Node and Edge Creation"

print_info "Counting nodes by type..."

NODE_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (n) RETURN count(n) \$\$) AS (count agtype);" "Node count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$NODE_COUNT" ] && [ "$NODE_COUNT" -gt 0 ]; then
    print_success "Nodes created: $NODE_COUNT total"
else
    print_error "No nodes found in graph"
    exit 1
fi

print_info "Counting edges..."

EDGE_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH ()-[r]->() RETURN count(r) \$\$) AS (count agtype);" "Edge count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$EDGE_COUNT" ] && [ "$EDGE_COUNT" -gt 0 ]; then
    print_success "Edges created: $EDGE_COUNT total"
else
    print_warning "No edges found (this may be expected if only nodes were created)"
fi

# ============================================================================
# STEP 7: RUN VALIDATION QUERIES
# ============================================================================

print_header "Step 7: Run Validation Queries"

# Test 1: Query Monster nodes
print_info "Test 1: Query Monster nodes..."
MONSTER_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (m:Monster) RETURN count(m) \$\$) AS (count agtype);" "Monster count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$MONSTER_COUNT" ] && [ "$MONSTER_COUNT" -gt 0 ]; then
    print_success "Found $MONSTER_COUNT Monster nodes"
else
    print_warning "No Monster nodes found (may need to be created by application)"
fi

# Test 2: Query pack leadership
print_info "Test 2: Query pack leadership..."
LEADS_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH ()-[r:LEADS]->() RETURN count(r) \$\$) AS (count agtype);" "LEADS edges" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$LEADS_COUNT" ] && [ "$LEADS_COUNT" -gt 0 ]; then
    print_success "Found $LEADS_COUNT LEADS relationships"
else
    print_info "No LEADS relationships yet (will be created during runtime)"
fi

# Test 3: Query Skills
print_info "Test 3: Query Skill nodes..."
SKILL_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (s:Skill) RETURN count(s) \$\$) AS (count agtype);" "Skill count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$SKILL_COUNT" ] && [ "$SKILL_COUNT" -gt 0 ]; then
    print_success "Found $SKILL_COUNT Skill nodes"
fi

# Test 4: Query skill combos
print_info "Test 4: Query skill combos..."
COMBO_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH ()-[r:COMBOS_WITH]->() RETURN count(r) \$\$) AS (count agtype);" "Combo count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$COMBO_COUNT" ] && [ "$COMBO_COUNT" -gt 0 ]; then
    print_success "Found $COMBO_COUNT COMBOS_WITH relationships"
fi

# Test 5: Query Territories
print_info "Test 5: Query Territory nodes..."
TERRITORY_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (t:Territory) RETURN count(t) \$\$) AS (count agtype);" "Territory count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$TERRITORY_COUNT" ] && [ "$TERRITORY_COUNT" -gt 0 ]; then
    print_success "Found $TERRITORY_COUNT Territory nodes"
fi

# Test 6: Query patrol routes
print_info "Test 6: Query patrol routes..."
PATROL_COUNT=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (p:PatrolPoint) RETURN count(p) \$\$) AS (count agtype);" "PatrolPoint count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ')

if [ -n "$PATROL_COUNT" ] && [ "$PATROL_COUNT" -gt 0 ]; then
    print_success "Found $PATROL_COUNT PatrolPoint nodes"
fi

# ============================================================================
# STEP 8: PERFORMANCE VALIDATION
# ============================================================================

print_header "Step 8: Performance Validation"

print_info "Testing query performance (target: <5ms for simple queries)..."

# Test query performance
START_TIME=$(date +%s%N)
execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (m:Monster) RETURN m LIMIT 10 \$\$) AS (m agtype);" "Performance test" > /dev/null 2>&1
END_TIME=$(date +%s%N)

ELAPSED_NS=$((END_TIME - START_TIME))
ELAPSED_MS=$((ELAPSED_NS / 1000000))

if [ $ELAPSED_MS -lt 10 ]; then
    print_success "Query performance: ${ELAPSED_MS}ms (excellent, <10ms)"
elif [ $ELAPSED_MS -lt 50 ]; then
    print_success "Query performance: ${ELAPSED_MS}ms (good, <50ms)"
else
    print_warning "Query performance: ${ELAPSED_MS}ms (may need optimization)"
    print_info "Consider creating additional indexes or analyzing tables"
fi

# ============================================================================
# STEP 9: INDEX VERIFICATION
# ============================================================================

print_header "Step 9: Verify Indexes"

print_info "Checking indexes on Monster nodes..."

# Check if indexes exist (AGE stores indexes in ag_label)
INDEX_CHECK=$(execute_sql "SELECT COUNT(*) FROM pg_indexes WHERE schemaname = 'monster_ai';" "Index count" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ' || echo "0")

if [ -n "$INDEX_CHECK" ] && [ "$INDEX_CHECK" -gt 0 ]; then
    print_success "Found $INDEX_CHECK PostgreSQL indexes on graph tables"
else
    print_info "No additional indexes found (Cypher indexes may not show in pg_indexes)"
fi

# ============================================================================
# STEP 10: CREATE SAMPLE TEST DATA (if graph is empty)
# ============================================================================

print_header "Step 10: Sample Test Data"

if [ -n "$MONSTER_COUNT" ] && [ "$MONSTER_COUNT" -gt 0 ]; then
    print_info "Sample data already exists ($MONSTER_COUNT monsters)"
    print_info "Skipping sample data creation"
else
    print_info "Creating sample test data..."
    
    # Create test monster
    execute_sql "SELECT * FROM cypher('monster_ai', \$\$ CREATE (m:Monster {id: 9999, mob_id: 1002, name: 'Test Monster', level: 5, archetype: 'aggressive', leadership_score: 0.5, experience: 100, spawn_time: timestamp(), current_hp_ratio: 1.0, position_x: 100, position_y: 100, map_id: 1}) RETURN m.id \$\$) AS (id agtype);" "Creating test monster" > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        print_success "Test monster created (id: 9999)"
    else
        print_warning "Failed to create test monster (may already exist)"
    fi
fi

# ============================================================================
# STEP 11: RUN COMPREHENSIVE VALIDATION
# ============================================================================

print_header "Step 11: Comprehensive Validation"

VALIDATION_PASSED=true

# Validate each schema
print_info "Validating Schema 1: Core Entity Schema..."
SCHEMA1_NODES=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (n) WHERE 'Monster' IN labels(n) OR 'Team' IN labels(n) RETURN count(n) \$\$) AS (count agtype);" "Schema 1 nodes" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ' || echo "0")

if [ "$SCHEMA1_NODES" -gt 0 ]; then
    print_success "Schema 1 validated: $SCHEMA1_NODES nodes"
else
    print_warning "Schema 1: No nodes found"
    VALIDATION_PASSED=false
fi

print_info "Validating Schema 2: Spatial Relationship Schema..."
SCHEMA2_NODES=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (n) WHERE 'Zone' IN labels(n) OR 'Territory' IN labels(n) OR 'PatrolPoint' IN labels(n) RETURN count(n) \$\$) AS (count agtype);" "Schema 2 nodes" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ' || echo "0")

if [ "$SCHEMA2_NODES" -gt 0 ]; then
    print_success "Schema 2 validated: $SCHEMA2_NODES nodes"
else
    print_warning "Schema 2: No spatial nodes found"
fi

print_info "Validating Schema 5: Learned Behavior Schema..."
SCHEMA5_NODES=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (n) WHERE 'Skill' IN labels(n) OR 'Behavior' IN labels(n) OR 'Pattern' IN labels(n) RETURN count(n) \$\$) AS (count agtype);" "Schema 5 nodes" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ' || echo "0")

if [ "$SCHEMA5_NODES" -gt 0 ]; then
    print_success "Schema 5 validated: $SCHEMA5_NODES nodes"
else
    print_warning "Schema 5: No behavior nodes found"
fi

print_info "Validating Schema 8: Strategy Schema..."
SCHEMA8_NODES=$(execute_sql "SELECT * FROM cypher('monster_ai', \$\$ MATCH (n) WHERE 'Strategy' IN labels(n) OR 'Tactic' IN labels(n) RETURN count(n) \$\$) AS (count agtype);" "Schema 8 nodes" | grep -E "^\s*[0-9]+\s*$" | tr -d ' ' || echo "0")

if [ "$SCHEMA8_NODES" -gt 0 ]; then
    print_success "Schema 8 validated: $SCHEMA8_NODES nodes"
else
    print_warning "Schema 8: No strategy nodes found"
fi

# ============================================================================
# STEP 12: DEPLOYMENT SUMMARY
# ============================================================================

print_header "Deployment Summary"

echo ""
echo "Database: $POSTGRES_DB @ $POSTGRES_HOST:$POSTGRES_PORT"
echo "Graph Name: monster_ai"
echo ""
echo "Deployment Results:"
echo "  ✓ PostgreSQL connection: OK"
echo "  ✓ Apache AGE extension: Available"
echo "  ✓ Graph created: OK"
echo "  ✓ Total Nodes: $NODE_COUNT"
echo "  ✓ Total Edges: $EDGE_COUNT"
echo "  ✓ Query Performance: ${ELAPSED_MS}ms"
echo ""
echo "Schema Status:"
echo "  Schema 1 (Core Entity): $SCHEMA1_NODES nodes"
echo "  Schema 2 (Spatial): $SCHEMA2_NODES nodes"
echo "  Schema 5 (Behavior): $SCHEMA5_NODES nodes"
echo "  Schema 8 (Strategy): $SCHEMA8_NODES nodes"
echo ""

if [ "$VALIDATION_PASSED" = true ]; then
    print_success "DEPLOYMENT SUCCESSFUL"
    echo ""
    echo "Next Steps:"
    echo "  1. Test Python integration:"
    echo "     python -m graph_manager"
    echo ""
    echo "  2. Run comprehensive tests:"
    echo "     python test_graph_integration.py"
    echo ""
    echo "  3. Start ML inference service:"
    echo "     python main.py"
    echo ""
    echo "  4. Monitor graph operations:"
    echo "     tail -f /opt/ml_monster_ai/logs/inference_service.log | grep -i graph"
    echo ""
    exit 0
else
    print_warning "DEPLOYMENT COMPLETED WITH WARNINGS"
    echo ""
    echo "Some schemas have no data yet. This is normal if:"
    echo "  - This is initial setup"
    echo "  - Nodes will be created during runtime"
    echo "  - You're running in development mode"
    echo ""
    echo "To populate graph with live data:"
    echo "  1. Start rAthena game server"
    echo "  2. Start ML inference service"
    echo "  3. Spawn monsters in-game"
    echo "  4. Monitor graph population"
    echo ""
    exit 0
fi
