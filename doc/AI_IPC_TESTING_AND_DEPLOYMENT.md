# AI IPC System - Testing & Deployment Guide

## Document Information
- **Version**: 1.0.0
- **Created**: 2025-11-27
- **Status**: Production Ready
- **Related Documents**: [NATIVE_IPC_ARCHITECTURE.md](./NATIVE_IPC_ARCHITECTURE.md)

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Installation Steps](#2-installation-steps)
3. [Pre-Deployment Checklist](#3-pre-deployment-checklist)
4. [Testing Procedures](#4-testing-procedures)
5. [Troubleshooting](#5-troubleshooting)
6. [Rollback Procedure](#6-rollback-procedure)
7. [Performance Monitoring](#7-performance-monitoring)
8. [Appendix](#8-appendix)

---

## 1. Prerequisites

### 1.1 Software Requirements

| Component | Minimum Version | Recommended | Notes |
|-----------|-----------------|-------------|-------|
| **MySQL/MariaDB** | 5.7+ / 10.2+ | 8.0+ / 10.5+ | Required for DEFAULT expressions |
| **Python** | 3.9+ | 3.11+ | For AI IPC service |
| **GCC/Clang** | 9+ / 10+ | 12+ / 14+ | C++17 support required |
| **CMake** | 3.16+ | 3.26+ | For rAthena compilation |
| **Make** | 4.0+ | 4.3+ | Build automation |

### 1.2 System Requirements

| Resource | Minimum | Recommended |
|----------|---------|-------------|
| **RAM** | 2 GB | 4 GB+ |
| **CPU** | 2 cores | 4+ cores |
| **Disk** | 1 GB free | 5 GB+ free |
| **Network** | Local connectivity | Low-latency connection |

### 1.3 Required Packages (Ubuntu/Debian)

```bash
# System dependencies
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    mysql-server \
    mysql-client \
    libmysqlclient-dev \
    python3 \
    python3-pip \
    python3-venv

# Verify installations
mysql --version
python3 --version
cmake --version
g++ --version
```

### 1.4 Required Packages (CentOS/RHEL)

```bash
# System dependencies
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake \
    git \
    mysql-server \
    mysql-devel \
    python3 \
    python3-pip

# Start MySQL
sudo systemctl start mysqld
sudo systemctl enable mysqld
```

---

## 2. Installation Steps

### 2.1 Database Setup

#### Step 1: Apply Database Schema

```bash
# Navigate to rAthena directory
cd rathena-AI-world

# Apply the AI IPC tables schema
mysql -u root -p ragnarok < sql-files/ai_ipc_tables.sql
```

#### Step 2: Verify Table Creation

```sql
-- Connect to database
mysql -u ragnarok -p ragnarok

-- Check tables were created
SHOW TABLES LIKE 'ai_%';
```

**Expected Output:**
```
+---------------------------+
| Tables_in_ragnarok (ai_%) |
+---------------------------+
| ai_request_log            |
| ai_requests               |
| ai_responses              |
+---------------------------+
3 rows in set (0.00 sec)
```

#### Step 3: Verify Table Structure

```sql
-- Check ai_requests structure
DESC ai_requests;

-- Check ai_responses structure
DESC ai_responses;

-- Verify indexes
SHOW INDEX FROM ai_requests;
SHOW INDEX FROM ai_responses;
```

#### Step 4: Enable Event Scheduler (Optional)

```sql
-- Enable for automatic cleanup
SET GLOBAL event_scheduler = ON;

-- Verify event is created
SHOW EVENTS LIKE 'ai_%';
```

### 2.2 Python Service Setup

#### Step 1: Create Virtual Environment

```bash
# Navigate to service directory
cd rathena-AI-world/tools/ai_ipc_service

# Create Python virtual environment
python3 -m venv venv

# Activate virtual environment
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

#### Step 2: Install Dependencies

```bash
# Install required packages
pip install -r requirements.txt

# Verify installation
pip list
```

**Expected packages:**
- aiomysql >= 0.2.0
- PyMySQL >= 1.1.0
- aiohttp >= 3.9.0
- pyyaml >= 6.0.1
- python-dotenv >= 1.0.0
- structlog >= 23.2.0

#### Step 3: Configure Service

```bash
# Copy example configuration
cp config.example.yaml config.yaml

# Edit configuration with your database credentials
nano config.yaml
```

**Minimal config.yaml:**
```yaml
database:
  host: localhost
  port: 3306
  user: ragnarok
  password: YOUR_PASSWORD_HERE  # Replace this!
  database: ragnarok
  pool_size: 5

polling:
  interval_ms: 100
  batch_size: 50
  worker_count: 4

logging:
  level: INFO
  format: text  # Use 'json' for production
```

#### Step 4: Test Service Connection

```bash
# Test the service starts successfully
python main.py

# Expected output:
# ============================================================
# AI IPC Service Starting
# ============================================================
# Loading configuration...
# Connecting to database...
# Database health: {'status': 'healthy', ...}
# ...
# AI IPC Service Started Successfully

# Stop with Ctrl+C
```

### 2.3 C++ Compilation

#### Step 1: Clean Previous Build

```bash
# Navigate to rAthena directory
cd rathena-AI-world

# Clean previous compilation
make clean

# Or with CMake
rm -rf build/
```

#### Step 2: Compile with New IPC Commands

```bash
# Using Make
./configure
make server

# Or using CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Step 3: Verify Build Success

```bash
# Check executable exists
ls -la map-server

# Verify no missing symbols related to AI IPC
ldd map-server | grep -i "not found"  # Should return empty
```

### 2.4 NPC Script Installation

The NPC scripts using the new IPC commands are already updated. Verify they are present:

```bash
# Check NPC script files exist
ls -la npc/custom/ai_integration_demo.txt
ls -la npc/custom/ai-world/examples/test_httpget.txt
ls -la npc/custom/ai-world/examples/test_httppost.txt
ls -la npc/custom/ai-world/example_ai_npc.txt
```

---

## 3. Pre-Deployment Checklist

Use this checklist before deploying to production.

### 3.1 Files Verification

| Status | File | Purpose | Lines |
|--------|------|---------|-------|
| ☐ | [`src/custom/script_ai_ipc.hpp`](../src/custom/script_ai_ipc.hpp:1) | Header declarations | ~243 |
| ☐ | [`src/custom/script_ai_ipc.cpp`](../src/custom/script_ai_ipc.cpp:1) | BUILDIN_FUNC implementations | ~546 |
| ☐ | [`sql-files/ai_ipc_tables.sql`](../sql-files/ai_ipc_tables.sql:1) | Database schema | ~491 |
| ☐ | [`src/custom/script_def.inc`](../src/custom/script_def.inc:1) | BUILDIN_DEF registrations | ~39 |
| ☐ | [`tools/ai_ipc_service/main.py`](../tools/ai_ipc_service/main.py:1) | Python service entry | ~413 |
| ☐ | [`tools/ai_ipc_service/config.yaml`](../tools/ai_ipc_service/config.example.yaml:1) | Service configuration | Present |

### 3.2 Code Verification

| Status | Check | Command/Query |
|--------|-------|---------------|
| ☐ | No duplicate BUILDIN_FUNC | `grep -rn "BUILDIN_FUNC(httpget)" src/` (should show only script_ai_ipc.cpp) |
| ☐ | script_def.inc has correct entries | Check lines 19-28 for ai_db_* and http* commands |
| ☐ | Old HTTP stubs disabled | Verify `script_http.cpp` has `#if 0` wrapper |
| ☐ | script_ai_bridge disabled | Verify `script_ai_bridge.cpp` has `#if 0` wrapper |

### 3.3 Database Verification

| Status | Check | Query |
|--------|-------|-------|
| ☐ | Tables exist | `SHOW TABLES LIKE 'ai_%';` (3 tables) |
| ☐ | Indexes created | `SHOW INDEX FROM ai_requests;` (5+ indexes) |
| ☐ | Foreign key works | Check `fk_response_request` constraint |
| ☐ | Stored procedures exist | `SHOW PROCEDURE STATUS LIKE 'ai_%';` |
| ☐ | Event scheduler ready | `SHOW EVENTS LIKE 'ai_%';` |

### 3.4 Python Service Verification

| Status | Check | Command |
|--------|-------|---------|
| ☐ | Virtual environment created | `ls tools/ai_ipc_service/venv/` |
| ☐ | Dependencies installed | `pip list \| grep -E 'aiomysql\|aiohttp\|pyyaml'` |
| ☐ | Config file present | `ls tools/ai_ipc_service/config.yaml` |
| ☐ | Database password set | Check config.yaml or DB_PASSWORD env var |
| ☐ | Service starts | `python main.py` (no errors) |

### 3.5 Build Verification

| Status | Check | Command |
|--------|-------|---------|
| ☐ | Clean build completed | `make server` (no errors) |
| ☐ | Executables present | `ls map-server char-server login-server` |
| ☐ | No linker errors | Check build output for undefined symbols |
| ☐ | No compiler warnings | `make 2>&1 \| grep -i warning` |

---

## 4. Testing Procedures

### 4.1 Database Verification Tests

#### Test 1: Table Structure

```sql
-- Connect to database
mysql -u ragnarok -p ragnarok

-- Verify ai_requests structure
DESC ai_requests;
-- Should show: id, request_type, endpoint, request_data, status, priority, etc.

-- Verify ai_responses structure  
DESC ai_responses;
-- Should show: id, request_id, response_data, http_status, error_message, etc.
```

#### Test 2: Insert Test Request

```sql
-- Insert a test request
INSERT INTO ai_requests (
    request_type, 
    endpoint, 
    request_data, 
    status, 
    priority,
    source_npc,
    source_map,
    expires_at
) VALUES (
    'health_check',
    '/api/v1/health',
    '{"test": true}',
    'pending',
    5,
    'TEST_NPC',
    'prontera',
    DATE_ADD(NOW(), INTERVAL 60 SECOND)
);

-- Check it was inserted
SELECT * FROM ai_requests WHERE request_type = 'health_check';

-- Get the request ID
SELECT LAST_INSERT_ID() AS request_id;
```

#### Test 3: Insert Test Response

```sql
-- Get the test request ID (replace with actual ID)
SET @test_id = (SELECT MAX(id) FROM ai_requests WHERE request_type = 'health_check');

-- Insert a test response
INSERT INTO ai_responses (
    request_id,
    response_data,
    http_status,
    processing_time_ms
) VALUES (
    @test_id,
    '{"status": "ok", "test": true}',
    200,
    50
);

-- Verify response
SELECT r.*, resp.response_data 
FROM ai_requests r 
LEFT JOIN ai_responses resp ON r.id = resp.request_id 
WHERE r.id = @test_id;
```

#### Test 4: Cleanup Test Data

```sql
-- Clean up test data
DELETE FROM ai_requests WHERE request_type = 'health_check' AND source_npc = 'TEST_NPC';
```

### 4.2 Python Service Verification Tests

#### Test 1: Service Startup

```bash
cd rathena-AI-world/tools/ai_ipc_service
source venv/bin/activate

# Start service with verbose output
LOG_LEVEL=DEBUG python main.py
```

**Expected Output:**
```
============================================================
AI IPC Service Starting
============================================================
Loading configuration...
Configuration: {...}
Connecting to database...
Database health: {'status': 'healthy', ...}
Initializing request processor...
============================================================
AI IPC Service Started Successfully
  Service: ai_ipc_service v1.0.0
  Database: ragnarok@localhost
  Polling: 100ms interval
  Workers: 4
  Batch size: 50
============================================================
Starting polling loop...
```

#### Test 2: Database Connectivity Test

```bash
# Quick connectivity test
python -c "
import asyncio
from database import Database
from config import Config

async def test():
    config = Config.load()
    db = Database(config.database)
    await db.connect()
    health = await db.health_check()
    print('Database Health:', health)
    await db.disconnect()

asyncio.run(test())
"
```

#### Test 3: Request Processing Test

```bash
# In terminal 1: Start the service
python main.py

# In terminal 2: Insert a test request via MySQL
mysql -u ragnarok -p ragnarok -e "
INSERT INTO ai_requests (request_type, endpoint, request_data, expires_at)
VALUES ('health_check', '/api/v1/health', '{}', DATE_ADD(NOW(), INTERVAL 60 SECOND));
"

# Watch terminal 1 for processing logs
# Then check for response
mysql -u ragnarok -p ragnarok -e "
SELECT r.id, r.status, resp.response_data, resp.http_status
FROM ai_requests r
LEFT JOIN ai_responses resp ON r.id = resp.request_id
WHERE r.request_type = 'health_check'
ORDER BY r.id DESC LIMIT 1;
"
```

**Expected: Status should change from 'pending' to 'completed' with response data**

### 4.3 C++ Build Verification Tests

#### Test 1: Compilation Check

```bash
cd rathena-AI-world

# Full clean build
make clean
make server 2>&1 | tee build.log

# Check for errors
grep -i "error:" build.log
# Should return empty

# Check for AI IPC related warnings
grep -i "script_ai_ipc" build.log
```

#### Test 2: Symbol Verification

```bash
# Check that BUILDIN_FUNC symbols are exported
nm map-server | grep -i "buildin_ai_db"
nm map-server | grep -i "buildin_httpget"
nm map-server | grep -i "buildin_httppost"
```

**Expected: Each command should show symbol entries**

#### Test 3: Server Startup Test

```bash
# Start map-server (will need login/char servers running)
./map-server

# Check for AI IPC initialization message
# Should see: [AI-IPC] Initializing AI IPC script commands...
# Should see: [AI-IPC] AI IPC script commands loaded successfully.
```

### 4.4 NPC Script Testing

#### Test 1: Create Test NPC Script

Create a test NPC file `npc/custom/test_ai_ipc.txt`:

```c
// AI IPC Test NPC - For testing database-based IPC commands
prontera,155,180,5	script	AI IPC Test	4_F_KAFRA1,{
    mes "[AI IPC Tester]";
    mes "This NPC tests the AI IPC system.";
    next;
    
    switch(select("Test ai_db_request:Test httpget:Test httppost:Check status:Exit")) {
        case 1:
            mes "[AI IPC Tester]";
            mes "Testing ai_db_request...";
            
            .@json$ = "{\"test\": \"hello\"}";
            .@req_id = ai_db_request("test", "/api/v1/test", .@json$);
            
            if (.@req_id > 0) {
                mes "^00FF00Success!^000000";
                mes "Request ID: " + .@req_id;
            } else {
                mes "^FF0000Failed!^000000";
                mes "Error code: " + .@req_id;
            }
            close;
            
        case 2:
            mes "[AI IPC Tester]";
            mes "Testing httpget...";
            
            .@result$ = httpget("http://localhost:8000/api/v1/health");
            
            if (.@result$ != "" && .@result$ != "{\"error\":\"timeout\"}") {
                mes "^00FF00Success!^000000";
                mes "Response: " + .@result$;
            } else {
                mes "^FFFF00Timeout or error^000000";
                mes "Response: " + .@result$;
            }
            close;
            
        case 3:
            mes "[AI IPC Tester]";
            mes "Testing httppost...";
            
            .@data$ = "{\"message\": \"test\"}";
            .@result$ = httppost("http://localhost:8000/api/v1/echo", .@data$);
            
            if (.@result$ != "" && .@result$ != "{\"error\":\"timeout\"}") {
                mes "^00FF00Success!^000000";
                mes "Response: " + .@result$;
            } else {
                mes "^FFFF00Timeout or error^000000";
                mes "Response: " + .@result$;
            }
            close;
            
        case 4:
            mes "[AI IPC Tester]";
            input(.@check_id);
            
            .@status = ai_db_status(.@check_id);
            
            mes "Request ID: " + .@check_id;
            mes "Status code: " + .@status;
            
            switch(.@status) {
                case 0: mes "Status: NOT_FOUND"; break;
                case 1: mes "Status: PENDING"; break;
                case 2: mes "Status: PROCESSING"; break;
                case 3: mes "Status: COMPLETED"; break;
                case 4: mes "Status: FAILED"; break;
                case 5: mes "Status: TIMEOUT"; break;
                case 6: mes "Status: CANCELLED"; break;
            }
            
            if (.@status == 3) {
                .@response$ = ai_db_response(.@check_id);
                mes "Response: " + .@response$;
            }
            close;
            
        case 5:
            mes "[AI IPC Tester]";
            mes "Goodbye!";
            close;
    }
}
```

#### Test 2: Load and Test NPC

```bash
# Reload NPC scripts in-game
@reloadnpc

# Or restart map-server
# Then interact with NPC in Prontera at coordinates 155,180
```

### 4.5 Integration Testing

#### End-to-End Test Scenario

**Setup:**
1. Start MySQL database
2. Start Python AI IPC service
3. Start rAthena servers (login, char, map)
4. Login to game client

**Test Steps:**

1. **Verify Python service is running:**
   ```bash
   ps aux | grep "ai_ipc_service"
   # Should show python process
   ```

2. **Check database queue:**
   ```sql
   SELECT COUNT(*) as pending FROM ai_requests WHERE status = 'pending';
   -- Should be 0 or low number
   ```

3. **In-game: Talk to test NPC and select "Test ai_db_request"**

4. **Verify request was created:**
   ```sql
   SELECT * FROM ai_requests ORDER BY id DESC LIMIT 1;
   -- Should show the test request
   ```

5. **Verify response was created (within seconds):**
   ```sql
   SELECT r.id, r.status, resp.response_data
   FROM ai_requests r
   LEFT JOIN ai_responses resp ON r.id = resp.request_id
   ORDER BY r.id DESC LIMIT 1;
   -- Should show status = 'completed' with response
   ```

6. **In-game: Use "Check status" option with the request ID**

7. **Verify cleanup works:**
   ```sql
   -- Wait 1 hour or manually call cleanup
   CALL ai_archive_completed_requests(0);
   
   -- Check log table
   SELECT * FROM ai_request_log ORDER BY id DESC LIMIT 5;
   ```

---

## 5. Troubleshooting

### 5.1 Common Build Errors

#### Error: Multiple definition of `BUILDIN_FUNC(httpget)`

**Symptom:**
```
error: multiple definition of `buildin_httpget(script_state*)'
```

**Solution:**
1. Check that old HTTP stubs are disabled:
   ```bash
   grep -n "#if 0" src/custom/script_http.cpp
   grep -n "#if 0" src/map/script_ai_bridge.cpp
   ```
2. If not disabled, wrap the file contents with `#if 0 ... #endif`

#### Error: Undefined reference to `mmysql_handle`

**Symptom:**
```
error: undefined reference to `mmysql_handle'
```

**Solution:**
Ensure includes are correct in [`script_ai_ipc.cpp`](../src/custom/script_ai_ipc.cpp:14):
```cpp
#include "../common/sql.hpp"
#include "../map/script.hpp"
```

#### Error: Missing `script_getstr` or `script_pushint`

**Symptom:**
```
error: 'script_getstr' was not declared in this scope
```

**Solution:**
Add include for script header in [`script_ai_ipc.cpp`](../src/custom/script_ai_ipc.cpp:17):
```cpp
#include "../map/script.hpp"
```

### 5.2 Database Connection Issues

#### Error: Access denied for user

**Symptom:**
```
Access denied for user 'ragnarok'@'localhost'
```

**Solution:**
1. Verify credentials:
   ```bash
   mysql -u ragnarok -p ragnarok -e "SELECT 1"
   ```
2. Check permissions:
   ```sql
   SHOW GRANTS FOR 'ragnarok'@'localhost';
   ```
3. Grant if needed:
   ```sql
   GRANT SELECT, INSERT, UPDATE ON ragnarok.ai_* TO 'ragnarok'@'localhost';
   FLUSH PRIVILEGES;
   ```

#### Error: Table 'ragnarok.ai_requests' doesn't exist

**Symptom:**
```
ERROR 1146 (42S02): Table 'ragnarok.ai_requests' doesn't exist
```

**Solution:**
Apply the schema:
```bash
mysql -u root -p ragnarok < sql-files/ai_ipc_tables.sql
```

#### Error: Foreign key constraint fails

**Symptom:**
```
ERROR 1452 (23000): Cannot add or update a child row: a foreign key constraint fails
```

**Solution:**
Insert request first, then response:
```sql
-- This order is required due to FK constraint
INSERT INTO ai_requests (...) VALUES (...);
INSERT INTO ai_responses (request_id, ...) VALUES (LAST_INSERT_ID(), ...);
```

### 5.3 Python Service Issues

#### Error: ModuleNotFoundError

**Symptom:**
```
ModuleNotFoundError: No module named 'aiomysql'
```

**Solution:**
1. Activate virtual environment:
   ```bash
   source venv/bin/activate
   ```
2. Reinstall dependencies:
   ```bash
   pip install -r requirements.txt
   ```

#### Error: Connection refused (database)

**Symptom:**
```
ConnectionRefusedError: [Errno 111] Connection refused
```

**Solution:**
1. Check MySQL is running:
   ```bash
   systemctl status mysql
   # or
   systemctl status mariadb
   ```
2. Check host/port in config.yaml matches MySQL settings

#### Error: Too many connections

**Symptom:**
```
OperationalError: Too many connections
```

**Solution:**
1. Reduce pool_size in config.yaml:
   ```yaml
   database:
     pool_size: 3  # Reduce from 5
   ```
2. Or increase MySQL max_connections:
   ```sql
   SET GLOBAL max_connections = 200;
   ```

### 5.4 NPC Script Issues

#### Error: Unknown command 'ai_db_request'

**Symptom:**
```
script error in file 'npc/...' line XXX: unknown command: ai_db_request
```

**Solution:**
1. Verify BUILDIN_DEF entries exist in [`script_def.inc`](../src/custom/script_def.inc:19):
   ```cpp
   BUILDIN_DEF(ai_db_request,"sss?"),
   ```
2. Rebuild the server:
   ```bash
   make clean && make server
   ```

#### Error: Request always times out

**Symptom:**
NPC shows timeout error even though service is running

**Solution:**
1. Check Python service is processing:
   ```bash
   # Look for processing logs
   tail -f /var/log/ai-ipc-service.log
   ```
2. Check for pending requests:
   ```sql
   SELECT COUNT(*) FROM ai_requests WHERE status = 'pending';
   ```
3. Increase timeout in [`script_ai_ipc.hpp`](../src/custom/script_ai_ipc.hpp:58):
   ```cpp
   constexpr int HTTP_COMPAT_TIMEOUT_MS = 10000;  // Increase from 5000
   ```

#### Error: Empty response

**Symptom:**
NPC receives empty string from ai_db_response

**Solution:**
1. Check response exists:
   ```sql
   SELECT * FROM ai_responses WHERE request_id = <your_id>;
   ```
2. Check request status:
   ```sql
   SELECT status FROM ai_requests WHERE id = <your_id>;
   -- Should be 'completed', not 'pending' or 'processing'
   ```

### 5.5 Debug Logging

#### Enable Verbose Logging in Python Service

```yaml
# config.yaml
logging:
  level: DEBUG
  format: text
```

#### Enable Verbose Logging in rAthena

Add to map_athena.conf:
```
// Enable debug messages
debug_level: 2
```

Or check server console for `[AI-IPC]` prefix messages.

---

## 6. Rollback Procedure

### 6.1 When to Rollback

- Build fails and cannot be fixed quickly
- Production database errors
- Severe performance degradation
- Critical functionality broken

### 6.2 Quick Rollback Steps

#### Step 1: Stop Services

```bash
# Stop Python service
pkill -f "ai_ipc_service"

# Stop rAthena servers gracefully
./athena-shutdown.sh
# Or kill processes
pkill -f map-server
pkill -f char-server
pkill -f login-server
```

#### Step 2: Re-enable Old HTTP Implementation

Edit [`src/custom/script_http.cpp`](../src/custom/script_http.cpp:1):
```cpp
// Change #if 0 to #if 1 at the top
#if 1  // Was: #if 0
// ... existing HTTP implementation
#endif
```

Edit [`src/map/script_ai_bridge.cpp`](../src/map/script_ai_bridge.cpp:1):
```cpp
// Change #if 0 to #if 1 at the top
#if 1  // Was: #if 0
// ... existing implementation
#endif
```

#### Step 3: Disable New IPC Implementation

Edit [`src/custom/script_ai_ipc.cpp`](../src/custom/script_ai_ipc.cpp:1):
```cpp
// Add at the very top of the file
#if 0  // DISABLED FOR ROLLBACK
// ... entire file contents
#endif
```

#### Step 4: Rebuild

```bash
make clean
make server
```

#### Step 5: Restart Servers

```bash
./athena-start.sh
```

### 6.3 Git-Based Rollback

If using Git version control:

```bash
# View commit history
git log --oneline -20

# Find the commit before AI IPC changes
# Rollback specific files
git checkout <previous_commit> -- src/custom/script_http.cpp
git checkout <previous_commit> -- src/map/script_ai_bridge.cpp

# Or create a revert commit
git revert <ai_ipc_commit_hash>

# Rebuild
make clean && make server
```

### 6.4 Database Rollback

**Warning: Only if absolutely necessary - will lose AI IPC data**

```sql
-- Backup first!
CREATE TABLE ai_requests_backup AS SELECT * FROM ai_requests;
CREATE TABLE ai_responses_backup AS SELECT * FROM ai_responses;
CREATE TABLE ai_request_log_backup AS SELECT * FROM ai_request_log;

-- Drop tables
DROP TABLE ai_responses;
DROP TABLE ai_request_log;
DROP TABLE ai_requests;

-- Drop procedures and events
DROP PROCEDURE IF EXISTS ai_archive_completed_requests;
DROP PROCEDURE IF EXISTS ai_cleanup_expired_requests;
DROP EVENT IF EXISTS ai_cleanup_event;
```

### 6.5 Post-Rollback Verification

```bash
# Verify old commands work
@reloadnpc

# Test old HTTP functionality in-game
# (using previous test NPCs)
```

---

## 7. Performance Monitoring

### 7.1 Database Monitoring Queries

#### Queue Status

```sql
-- Current queue depth by status
SELECT status, COUNT(*) as count
FROM ai_requests
GROUP BY status;
```

**Healthy Output:**
```
+-----------+-------+
| status    | count |
+-----------+-------+
| pending   |     0 |
| completed |    50 |
+-----------+-------+
```

**Warning Signs:**
- pending > 100: Service may be falling behind
- processing > 20: Possible stuck requests

#### Processing Latency

```sql
-- Average processing time (last hour)
SELECT 
    AVG(processing_time_ms) as avg_ms,
    MAX(processing_time_ms) as max_ms,
    MIN(processing_time_ms) as min_ms,
    COUNT(*) as total
FROM ai_responses
WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 HOUR);
```

#### Request Rate

```sql
-- Requests per minute (last 10 minutes)
SELECT 
    DATE_FORMAT(created_at, '%Y-%m-%d %H:%i') as minute,
    COUNT(*) as requests
FROM ai_requests
WHERE created_at > DATE_SUB(NOW(), INTERVAL 10 MINUTE)
GROUP BY DATE_FORMAT(created_at, '%Y-%m-%d %H:%i')
ORDER BY minute DESC;
```

#### Stuck Requests Detection

```sql
-- Find requests stuck in 'processing' for > 5 minutes
SELECT id, request_type, endpoint, created_at, updated_at
FROM ai_requests
WHERE status = 'processing'
  AND updated_at < DATE_SUB(NOW(), INTERVAL 5 MINUTE);
```

**Recovery Action:**
```sql
-- Reset stuck requests
UPDATE ai_requests 
SET status = 'pending'
WHERE status = 'processing'
  AND updated_at < DATE_SUB(NOW(), INTERVAL 5 MINUTE);
```

### 7.2 Python Service Metrics

The service logs statistics every 100 polls:
```
Status: processed=1234, failed=5, rate=12.34/s
```

Monitor these in real-time:
```bash
# Follow service logs
tail -f /var/log/ai-ipc-service.log | grep "Status:"
```

Key metrics to watch:
- **processed**: Total successful requests
- **failed**: Error count (should be < 1% of processed)
- **rate**: Requests per second (depends on load)

### 7.3 System Resource Monitoring

```bash
# CPU and memory usage
top -p $(pgrep -f ai_ipc_service)

# MySQL connection count
mysql -e "SHOW STATUS LIKE 'Threads_connected';"

# Disk I/O
iostat -x 1 3
```

### 7.4 Health Check Endpoints

Create a monitoring script `tools/ai_ipc_service/health_check.sh`:

```bash
#!/bin/bash
# AI IPC Health Check Script

DB_USER="ragnarok"
DB_PASS="YOUR_PASSWORD"
DB_NAME="ragnarok"

# Check database connectivity
mysql -u$DB_USER -p$DB_PASS $DB_NAME -e "SELECT 1" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "CRITICAL: Database connection failed"
    exit 2
fi

# Check for stuck requests
STUCK=$(mysql -u$DB_USER -p$DB_PASS $DB_NAME -N -e "
    SELECT COUNT(*) FROM ai_requests 
    WHERE status = 'processing' 
    AND updated_at < DATE_SUB(NOW(), INTERVAL 5 MINUTE)
")
if [ "$STUCK" -gt 0 ]; then
    echo "WARNING: $STUCK stuck requests found"
    exit 1
fi

# Check queue depth
PENDING=$(mysql -u$DB_USER -p$DB_PASS $DB_NAME -N -e "
    SELECT COUNT(*) FROM ai_requests WHERE status = 'pending'
")
if [ "$PENDING" -gt 100 ]; then
    echo "WARNING: High queue depth: $PENDING pending"
    exit 1
fi

# Check Python service is running
pgrep -f "ai_ipc_service" > /dev/null
if [ $? -ne 0 ]; then
    echo "CRITICAL: AI IPC service not running"
    exit 2
fi

echo "OK: AI IPC system healthy (pending: $PENDING)"
exit 0
```

### 7.5 Alerting Thresholds

| Metric | Warning | Critical | Action |
|--------|---------|----------|--------|
| Pending requests | > 100 | > 500 | Scale workers or investigate |
| Stuck requests | > 0 | > 10 | Reset stuck, check logs |
| Error rate | > 1% | > 5% | Check handler logs |
| Avg latency | > 500ms | > 2000ms | Optimize handlers |
| Service memory | > 500MB | > 1GB | Check for leaks |

---

## 8. Appendix

### A. Complete File Listing

#### C++ Source Files

| File | Purpose | Lines |
|------|---------|-------|
| [`src/custom/script_ai_ipc.hpp`](../src/custom/script_ai_ipc.hpp:1) | AI IPC header with declarations | 243 |
| [`src/custom/script_ai_ipc.cpp`](../src/custom/script_ai_ipc.cpp:1) | BUILDIN_FUNC implementations | 546 |
| [`src/custom/script_def.inc`](../src/custom/script_def.inc:1) | Script command registrations | 39 |

#### Database Files

| File | Purpose | Lines |
|------|---------|-------|
| [`sql-files/ai_ipc_tables.sql`](../sql-files/ai_ipc_tables.sql:1) | Complete database schema | 491 |

#### Python Service Files

| File | Purpose | Lines |
|------|---------|-------|
| [`tools/ai_ipc_service/main.py`](../tools/ai_ipc_service/main.py:1) | Service entry point | 413 |
| [`tools/ai_ipc_service/config.py`](../tools/ai_ipc_service/config.py:1) | Configuration management | - |
| [`tools/ai_ipc_service/database.py`](../tools/ai_ipc_service/database.py:1) | Database operations | - |
| [`tools/ai_ipc_service/processor.py`](../tools/ai_ipc_service/processor.py:1) | Request processing | - |
| [`tools/ai_ipc_service/handlers/__init__.py`](../tools/ai_ipc_service/handlers/__init__.py:1) | Handler registry | - |
| [`tools/ai_ipc_service/handlers/base.py`](../tools/ai_ipc_service/handlers/base.py:1) | Base handler class | - |
| [`tools/ai_ipc_service/handlers/health_check.py`](../tools/ai_ipc_service/handlers/health_check.py:1) | Health check handler | - |
| [`tools/ai_ipc_service/handlers/http_proxy.py`](../tools/ai_ipc_service/handlers/http_proxy.py:1) | HTTP proxy handler | - |
| [`tools/ai_ipc_service/handlers/ai_dialogue.py`](../tools/ai_ipc_service/handlers/ai_dialogue.py:1) | AI dialogue handler | - |

#### Documentation

| File | Purpose |
|------|---------|
| [`doc/NATIVE_IPC_ARCHITECTURE.md`](./NATIVE_IPC_ARCHITECTURE.md) | System architecture |
| [`doc/AI_IPC_TESTING_AND_DEPLOYMENT.md`](./AI_IPC_TESTING_AND_DEPLOYMENT.md) | This document |
| [`tools/ai_ipc_service/README.md`](../tools/ai_ipc_service/README.md) | Python service documentation |

### B. Command Reference

#### NPC Script Commands

| Command | Signature | Description | Returns |
|---------|-----------|-------------|---------|
| [`ai_db_request`](../src/custom/script_ai_ipc.cpp:150) | `(type$, endpoint$, data${, priority})` | Create async request | Request ID (int) |
| [`ai_db_response`](../src/custom/script_ai_ipc.cpp:207) | `(request_id)` | Get response (non-blocking) | JSON string |
| [`ai_db_wait`](../src/custom/script_ai_ipc.cpp:245) | `(request_id, timeout_ms)` | Wait for response (blocking) | JSON string |
| [`ai_db_status`](../src/custom/script_ai_ipc.cpp:333) | `(request_id)` | Get request status | Status code (int) |
| [`ai_db_cancel`](../src/custom/script_ai_ipc.cpp:370) | `(request_id)` | Cancel request | 1=success, 0=fail |
| [`httpget`](../src/custom/script_ai_ipc.cpp:404) | `(url$)` | HTTP GET (compat) | JSON string |
| [`httppost`](../src/custom/script_ai_ipc.cpp:479) | `(url$, data$)` | HTTP POST (compat) | JSON string |

#### Usage Examples

```c
// Async pattern (recommended)
.@req_id = ai_db_request("dialogue", "/api/v1/npc/talk", .@json$);
sleep2 100;  // Let other processing happen
.@response$ = ai_db_wait(.@req_id, 5000);

// Sync pattern (backward compatible)
.@response$ = httpget("http://ai-service:8000/api/v1/health");
.@response$ = httppost("http://ai-service:8000/api/v1/chat", .@json$);

// Status check pattern
.@status = ai_db_status(.@req_id);
switch(.@status) {
    case 1: debugmes "Pending..."; break;
    case 2: debugmes "Processing..."; break;
    case 3: 
        .@result$ = ai_db_response(.@req_id);
        break;
    case 4: debugmes "Failed!"; break;
}

// Cancel pattern
if (ai_db_cancel(.@req_id)) {
    debugmes "Request cancelled";
}
```

### C. Status Codes

| Code | Enum Constant | Database Value | Description |
|------|---------------|----------------|-------------|
| 0 | `NOT_FOUND` | - | Request does not exist |
| 1 | `PENDING` | `'pending'` | Waiting to be processed |
| 2 | `PROCESSING` | `'processing'` | Currently being handled |
| 3 | `COMPLETED` | `'completed'` | Successfully completed |
| 4 | `FAILED` | `'failed'` | Processing error |
| 5 | `TIMEOUT` | `'timeout'` | Request expired |
| 6 | `CANCELLED` | `'cancelled'` | Manually cancelled |

### D. Configuration Reference

#### Python Service Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `AI_IPC_CONFIG` | `config.yaml` | Path to config file |
| `DB_HOST` | `localhost` | Database host |
| `DB_PORT` | `3306` | Database port |
| `DB_USER` | `ragnarok` | Database username |
| `DB_PASSWORD` | - | Database password (required) |
| `DB_NAME` | `ragnarok` | Database name |
| `DB_POOL_SIZE` | `5` | Connection pool size |
| `POLL_INTERVAL_MS` | `100` | Poll interval in ms |
| `BATCH_SIZE` | `50` | Requests per batch |
| `WORKER_COUNT` | `4` | Concurrent workers |
| `AI_SERVICE_BASE_URL` | `http://localhost:8000` | AI service URL |
| `AI_SERVICE_TIMEOUT` | `10` | AI service timeout (sec) |
| `LOG_LEVEL` | `INFO` | Logging level |
| `LOG_FORMAT` | `json` | Log format (json/text) |

### E. Quick Reference Cards

#### Deployment Quick Reference

```
1. Apply database schema:   mysql -u root -p ragnarok < sql-files/ai_ipc_tables.sql
2. Setup Python service:    cd tools/ai_ipc_service && python -m venv venv && source venv/bin/activate && pip install -r requirements.txt
3. Configure service:       cp config.example.yaml config.yaml && nano config.yaml
4. Build rAthena:           make clean && make server
5. Start Python service:    python main.py &
6. Start rAthena:           ./athena-start.sh
```

#### Troubleshooting Quick Reference

```
Check pending queue:     SELECT COUNT(*) FROM ai_requests WHERE status = 'pending';
Check stuck requests:    SELECT * FROM ai_requests WHERE status = 'processing' AND updated_at < NOW() - INTERVAL 5 MINUTE;
Reset stuck requests:    UPDATE ai_requests SET status = 'pending' WHERE status = 'processing' AND updated_at < NOW() - INTERVAL 5 MINUTE;
Check service running:   pgrep -f ai_ipc_service
View service logs:       tail -f /var/log/ai-ipc-service.log
Restart service:         pkill -f ai_ipc_service && python main.py &
```

---

## Document History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0.0 | 2025-11-27 | AI Team | Initial release |

---

**End of Document**