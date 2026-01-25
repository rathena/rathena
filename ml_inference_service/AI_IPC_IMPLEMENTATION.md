# AI IPC Implementation Guide

## Document Information
- **Version**: 1.0.0
- **Created**: 2026-01-25
- **Status**: Production Ready
- **Author**: AI Architect

## Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Components](#components)
4. [Database Schema](#database-schema)
5. [C++ Script Commands](#cpp-script-commands)
6. [Python Worker Service](#python-worker-service)
7. [Deployment Guide](#deployment-guide)
8. [Configuration](#configuration)
9. [Testing](#testing)
10. [Monitoring](#monitoring)
11. [Troubleshooting](#troubleshooting)
12. [Performance Tuning](#performance-tuning)

---

## Overview

The AI IPC (Inter-Process Communication) system enables NPC scripts to communicate with external AI services through a database-based queue mechanism. This replaces HTTP-based communication for better reliability, persistence, and observability.

### Key Benefits

| Feature | HTTP (Old) | Database IPC (New) |
|---------|-----------|-------------------|
| Dependencies | curl, zmq, hiredis | MySQL only (existing) |
| Reliability | Network failures | ACID transactions |
| Debugging | Network traces | SQL query logs |
| Persistence | None | Full audit trail |
| Concurrency | Thread-limited | Connection pool |

### Request Flow

```
┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
│   NPC    │───▶│  MariaDB │◀──▶│  Python  │───▶│    AI    │
│  Script  │    │  Tables  │    │  Worker  │    │  Service │
└──────────┘    └──────────┘    └──────────┘    └──────────┘
     │               │               │                │
     │  1. Request   │               │                │
     │──────────────▶│               │                │
     │               │  2. Poll      │                │
     │               │◀──────────────│                │
     │               │               │  3. Process    │
     │               │               │───────────────▶│
     │               │  4. Response  │                │
     │               │◀──────────────│                │
     │  5. Retrieve  │               │                │
     │◀──────────────│               │                │
```

---

## Architecture

### System Components

#### 1. MariaDB Tables
- **`ai_requests`**: Request queue from NPC scripts
- **`ai_responses`**: Response storage for NPC scripts
- **`ai_request_log`**: Audit trail for analytics

#### 2. C++ Script Commands
- **`ai_db_request`**: Create async request
- **`ai_db_response`**: Non-blocking response check
- **`ai_db_status`**: Get request status
- **`ai_db_cancel`**: Cancel pending request
- **`ai_db_cleanup`**: Admin cleanup command

#### 3. Python Worker Service
- Polls `ai_requests` table for pending requests
- Routes to appropriate AI service endpoints
- Writes responses back to `ai_responses` table
- Handles timeouts and errors

#### 4. AI Service
- External HTTP API (AI Autonomous World)
- Processes dialogue, decisions, emotions, etc.
- Returns JSON responses

---

## Components

### File Structure

```
rathena-AI-world/
├── sql-files/
│   └── ai_ipc_tables.sql              # Database schema
├── src/custom/
│   ├── script_ai_ipc.cpp              # C++ implementations
│   ├── script_ai_ipc.hpp              # C++ headers
│   └── script_def.inc                 # Command registrations
├── ml_inference_service/
│   ├── ai_ipc_worker.py               # Python worker service
│   ├── ai-ipc-worker.service          # Systemd unit file
│   ├── inference_config.yaml          # Configuration
│   ├── requirements.txt               # Python dependencies
│   ├── test_ipc_integration.py        # Integration tests
│   └── AI_IPC_IMPLEMENTATION.md       # This document
└── npc/custom/ai-world/test/
    └── test_ai_ipc_commands.txt       # NPC test scripts
```

---

## Database Schema

### ai_requests Table

Stores outgoing requests from NPC scripts.

```sql
CREATE TABLE IF NOT EXISTS `ai_requests` (
  `id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `request_type` VARCHAR(50) NOT NULL DEFAULT 'generic',
  `endpoint` VARCHAR(255) NOT NULL,
  `request_data` TEXT DEFAULT NULL,
  `priority` TINYINT(3) UNSIGNED NOT NULL DEFAULT 5,
  `timeout` INT(11) UNSIGNED NOT NULL DEFAULT 30,
  `status` ENUM('pending', 'processing', 'completed', 'failed', 'timeout', 'cancelled'),
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `processed_at` TIMESTAMP NULL DEFAULT NULL,
  `expires_at` DATETIME NOT NULL,
  `source_npc` VARCHAR(100) DEFAULT NULL,
  `source_map` VARCHAR(30) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `idx_status_priority` (`status`, `priority`, `created_at`),
  KEY `idx_expires_at` (`expires_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

**Key Columns:**
- `id`: Unique request identifier (returned to NPC script)
- `request_type`: Type of request (e.g., "dialogue", "decision")
- `endpoint`: Target API endpoint (e.g., "/api/v1/npc/dialogue")
- `request_data`: JSON payload
- `priority`: 1-10 (lower = higher priority)
- `status`: Current request state
- `expires_at`: When to mark as timeout

### ai_responses Table

Stores responses from AI services.

```sql
CREATE TABLE IF NOT EXISTS `ai_responses` (
  `id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `request_id` INT(11) UNSIGNED NOT NULL,
  `response_data` TEXT DEFAULT NULL,
  `error_message` TEXT DEFAULT NULL,
  `processing_time_ms` INT(11) UNSIGNED DEFAULT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_request_id` (`request_id`),
  CONSTRAINT `fk_ai_responses_request_id` 
    FOREIGN KEY (`request_id`) REFERENCES `ai_requests` (`id`) 
    ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### ai_request_log Table

Audit trail for debugging and analytics.

```sql
CREATE TABLE IF NOT EXISTS `ai_request_log` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `request_id` INT(11) UNSIGNED NOT NULL,
  `event_type` VARCHAR(50) NOT NULL,
  `event_data` TEXT DEFAULT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_request_id` (`request_id`),
  KEY `idx_event_type` (`event_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

## C++ Script Commands

### 1. ai_db_request

Creates a new AI request in the database.

**Signature:**
```c
int ai_db_request(string type, string endpoint, string data, int priority)
```

**Parameters:**
- `type`: Request type identifier (e.g., "dialogue", "decision")
- `endpoint`: API endpoint path (e.g., "/api/v1/npc/dialogue")
- `data`: JSON request data
- `priority`: Optional priority 1-10 (default: 5, lower = higher)

**Returns:** Request ID (positive integer) or -1 on error

**Example:**
```c
.@data$ = "{\"message\":\"Hello AI\",\"npc_id\":123}";
.@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@data$, 5);
```

### 2. ai_db_response

Non-blocking check for response availability.

**Signature:**
```c
string ai_db_response(int request_id)
```

**Returns:** JSON response string or empty string if not ready

**Example:**
```c
.@response$ = ai_db_response(.@req_id);
if (.@response$ != "") {
    // Response is ready
    mes "AI says: " + .@response$;
}
```

### 3. ai_db_status

Gets the current status of a request.

**Signature:**
```c
int ai_db_status(int request_id)
```

**Returns:** Status code:
- `0` = NOT_FOUND
- `1` = PENDING
- `2` = PROCESSING
- `3` = COMPLETED
- `4` = FAILED
- `5` = TIMEOUT
- `6` = CANCELLED

**Example:**
```c
.@status = ai_db_status(.@req_id);
if (.@status == 3) {
    .@response$ = ai_db_response(.@req_id);
}
```

### 4. ai_db_cancel

Cancels a pending or processing request.

**Signature:**
```c
int ai_db_cancel(int request_id)
```

**Returns:** 1 on success, 0 if already completed/not found

**Example:**
```c
if (ai_db_cancel(.@req_id)) {
    mes "Request cancelled successfully";
}
```

### 5. ai_db_cleanup

Admin command to cleanup old completed/failed requests.

**Signature:**
```c
int ai_db_cleanup(int age_seconds)
```

**Parameters:**
- `age_seconds`: Delete requests older than this (default: 3600, min: 60, max: 2592000)

**Returns:** Number of records deleted

**Example:**
```c
// Cleanup requests older than 1 hour (GM only)
if (getgmlevel() >= 99) {
    .@deleted = ai_db_cleanup(3600);
    mes "Cleaned up " + .@deleted + " old requests";
}
```

---

## Python Worker Service

### Overview

The Python worker service polls the `ai_requests` table, processes requests, and writes responses back.

### Key Features

- **Async I/O**: Uses `aiomysql` and `aiohttp` for non-blocking operations
- **Batch Processing**: Processes up to 10 requests per poll cycle
- **Connection Pooling**: Maintains 5-20 database connections
- **FOR UPDATE SKIP LOCKED**: Ensures safe concurrent processing
- **Graceful Shutdown**: Handles SIGINT/SIGTERM signals
- **Auto-Retry**: Retries failed requests with exponential backoff
- **Timeout Handling**: Marks expired requests as timeout

### Main Components

```python
class AIIPCWorker:
    async def initialize()              # Setup pools and sessions
    async def poll_and_process()        # Main polling loop
    async def process_request()         # Process single request
    async def route_to_service()        # Route to AI service
    async def write_response()          # Write response to DB
    async def handle_timeout()          # Handle timeout
    async def cleanup_expired_requests()# Cleanup expired
    async def run_forever()             # Main event loop
```

### Processing Flow

1. **Poll**: SELECT pending requests with `FOR UPDATE SKIP LOCKED`
2. **Mark**: UPDATE status to 'processing'
3. **Route**: Send HTTP request to AI service
4. **Write**: INSERT response into `ai_responses`
5. **Update**: UPDATE request status to 'completed' or 'failed'
6. **Log**: INSERT audit log entry

---

## Deployment Guide

### Prerequisites

- MariaDB 10.3+ or MySQL 5.7+
- Python 3.11+
- rAthena compiled with custom scripts
- AI Autonomous World service running

### Step 1: Install Database Schema

```bash
# Connect to MariaDB
mysql -u root -p

# Source the schema file
USE ragnarok;
SOURCE /path/to/rathena-AI-world/sql-files/ai_ipc_tables.sql;

# Verify tables created
SHOW TABLES LIKE 'ai_%';

# Enable event scheduler for auto-cleanup
SET GLOBAL event_scheduler = ON;
```

### Step 2: Compile rAthena with IPC Commands

The C++ files are already in place:
- `src/custom/script_ai_ipc.cpp`
- `src/custom/script_ai_ipc.hpp`
- `src/custom/script_def.inc`

Recompile the map server:

```bash
cd /path/to/rathena-AI-world
./configure
make clean
make server
```

### Step 3: Install Python Dependencies

```bash
cd /path/to/rathena-AI-world/ml_inference_service

# Install in virtual environment
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

### Step 4: Configure Environment

Create `.env` file:

```bash
# Database configuration
DB_HOST=192.168.0.100
DB_PORT=3306
DB_USER=ragnarok
DB_PASSWORD=your_password_here
DB_NAME=ragnarok

# AI Service configuration
AI_SERVICE_BASE_URL=http://127.0.0.1:8000
AI_SERVICE_TIMEOUT=10

# Worker configuration
POLL_INTERVAL_MS=100
BATCH_SIZE=10
CLEANUP_INTERVAL=60
TIMEOUT_SECONDS=30
```

### Step 5: Install Systemd Service

```bash
# Copy service file
sudo cp ai-ipc-worker.service /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload

# Enable and start service
sudo systemctl enable ai-ipc-worker
sudo systemctl start ai-ipc-worker

# Check status
sudo systemctl status ai-ipc-worker
```

### Step 6: Load NPC Test Scripts

Add to `npc/scripts_custom.conf`:

```
npc: npc/custom/ai-world/test/test_ai_ipc_commands.txt
```

Reload scripts in-game:
```
@reloadscript
```

### Step 7: Verify Installation

```bash
# Check worker logs
sudo journalctl -u ai-ipc-worker -f

# Check database
mysql -u ragnarok -p ragnarok -e "SELECT COUNT(*) FROM ai_requests WHERE status='pending';"

# Test in-game
# Talk to "AI IPC Test" NPC in Prontera (150, 150)
```

---

## Configuration

### inference_config.yaml

```yaml
ipc_worker:
  enabled: true
  poll_interval_ms: 100
  batch_size: 10
  max_processing_time: 5000
  cleanup_interval: 60
  timeout_seconds: 30
  ai_service_base_url: "http://127.0.0.1:8000"
  ai_service_timeout: 10

database:
  mariadb:
    host: "192.168.0.100"
    port: 3306
    database: "ragnarok"
    user: "${DB_USER}"
    password: "${DB_PASSWORD}"
    min_pool_size: 5
    max_pool_size: 20
```

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `DB_HOST` | MariaDB host | 192.168.0.100 |
| `DB_PORT` | MariaDB port | 3306 |
| `DB_USER` | Database user | ragnarok |
| `DB_PASSWORD` | Database password | (required) |
| `DB_NAME` | Database name | ragnarok |
| `AI_SERVICE_BASE_URL` | AI service URL | http://127.0.0.1:8000 |
| `POLL_INTERVAL_MS` | Poll interval | 100 |
| `BATCH_SIZE` | Requests per batch | 10 |
| `TIMEOUT_SECONDS` | Request timeout | 30 |

---

## Testing

### Unit Tests (Python)

```bash
cd ml_inference_service
python3 test_ipc_integration.py
```

Tests include:
- Database connectivity
- Table existence
- Request insertion
- Request processing
- Response retrieval
- Timeout handling
- Cleanup operations
- Performance metrics

### Integration Tests (NPC)

In-game, talk to "AI IPC Test" NPC:
- Test 1: Basic Request/Response
- Test 2: Status Checking
- Test 3: Cancel Request
- Test 4: Timeout Handling
- Test 5: Cleanup (Admin)
- Test 6: Batch Processing
- Test 7: Error Handling

### Performance Benchmark

Talk to "AI IPC Benchmark" NPC for:
- Throughput test (100 requests)
- Latency test (10 requests with timing)

---

## Monitoring

### Key Metrics

1. **Request Throughput**: Requests processed per second
2. **Average Latency**: Time from request to response
3. **Success Rate**: Completed / Total requests
4. **Queue Depth**: Pending requests count
5. **Processing Time**: Time spent in AI service

### Monitoring Queries

```sql
-- Pending requests count
SELECT COUNT(*) as pending FROM ai_requests WHERE status='pending';

-- Average processing time
SELECT AVG(processing_time_ms) as avg_ms FROM ai_responses 
WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 HOUR);

-- Request rate (last hour)
SELECT COUNT(*) as total FROM ai_requests 
WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 HOUR);

-- Success rate (last hour)
SELECT 
  SUM(CASE WHEN status='completed' THEN 1 ELSE 0 END) as completed,
  SUM(CASE WHEN status='failed' THEN 1 ELSE 0 END) as failed,
  COUNT(*) as total
FROM ai_requests 
WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 HOUR);
```

### Log Monitoring

```bash
# Worker logs
sudo journalctl -u ai-ipc-worker -f

# Filter for errors
sudo journalctl -u ai-ipc-worker | grep ERROR

# Check statistics
sudo journalctl -u ai-ipc-worker | grep "Statistics"
```

---

## Troubleshooting

### Problem: No requests being processed

**Symptoms:**
- Requests stay in 'pending' status
- No worker activity in logs

**Diagnosis:**
```bash
# Check if worker is running
sudo systemctl status ai-ipc-worker

# Check worker logs
sudo journalctl -u ai-ipc-worker -n 50

# Check database connectivity
mysql -u ragnarok -p ragnarok -e "SELECT 1;"
```

**Solutions:**
1. Restart worker: `sudo systemctl restart ai-ipc-worker`
2. Check database credentials in `.env`
3. Verify AI service is running: `curl http://127.0.0.1:8000/api/v1/health`

### Problem: Requests timing out

**Symptoms:**
- All requests marked as 'timeout'
- Long processing times

**Diagnosis:**
```sql
SELECT * FROM ai_requests WHERE status='timeout' ORDER BY id DESC LIMIT 10;
```

**Solutions:**
1. Increase `TIMEOUT_SECONDS` in configuration
2. Check AI service response time
3. Increase worker `BATCH_SIZE` for better throughput
4. Check network connectivity to AI service

### Problem: High database load

**Symptoms:**
- Slow query performance
- High CPU usage on database server

**Diagnosis:**
```sql
SHOW PROCESSLIST;
SHOW ENGINE INNODB STATUS;
```

**Solutions:**
1. Increase `POLL_INTERVAL_MS` to reduce polling frequency
2. Add database indexes if missing
3. Run cleanup more frequently
4. Archive old requests to separate table

### Problem: Memory leaks in worker

**Symptoms:**
- Worker memory usage grows over time
- Worker becomes unresponsive

**Diagnosis:**
```bash
# Check memory usage
ps aux | grep ai_ipc_worker.py

# Check for zombie connections
mysql -e "SHOW PROCESSLIST;" | grep Sleep
```

**Solutions:**
1. Restart worker regularly with systemd timer
2. Reduce `max_pool_size` in configuration
3. Update to latest `aiomysql` version

---

## Performance Tuning

### Database Optimization

```sql
-- Analyze table statistics
ANALYZE TABLE ai_requests, ai_responses, ai_request_log;

-- Optimize tables
OPTIMIZE TABLE ai_requests, ai_responses, ai_request_log;

-- Check index usage
EXPLAIN SELECT * FROM ai_requests 
WHERE status='pending' 
ORDER BY priority, created_at 
LIMIT 10;
```

### Worker Tuning

| Parameter | Low Load | Medium Load | High Load |
|-----------|----------|-------------|-----------|
| `POLL_INTERVAL_MS` | 200 | 100 | 50 |
| `BATCH_SIZE` | 5 | 10 | 20 |
| `min_pool_size` | 5 | 10 | 15 |
| `max_pool_size` | 10 | 20 | 30 |

### Cleanup Strategy

```sql
-- Run manual cleanup
CALL sp_ai_cleanup_old_requests(7);  -- 7 days retention

-- Check cleanup event
SHOW EVENTS WHERE Name = 'evt_ai_cleanup_old_requests';

-- Adjust cleanup frequency
ALTER EVENT evt_ai_cleanup_old_requests ON SCHEDULE EVERY 6 HOUR;
```

---

## Best Practices

1. **Use Priority Wisely**: Reserve priority 1-3 for critical requests
2. **Keep Data Small**: Limit JSON payloads to <10KB
3. **Handle Timeouts**: Always check status before retrieving response
4. **Cleanup Regularly**: Run cleanup at least daily
5. **Monitor Queue**: Alert if pending count > 1000
6. **Log Errors**: Check logs regularly for patterns
7. **Test Before Production**: Use test NPCs to validate
8. **Backup Database**: Regular backups of IPC tables
9. **Use Transactions**: Batch operations in transactions
10. **Graceful Degradation**: Handle worker failures gracefully in scripts

---

## Appendix

### Status Code Reference

```c
// Request status codes
#define AI_REQ_NOT_FOUND   0
#define AI_REQ_PENDING     1
#define AI_REQ_PROCESSING  2
#define AI_REQ_COMPLETED   3
#define AI_REQ_FAILED      4
#define AI_REQ_TIMEOUT     5
#define AI_REQ_CANCELLED   6
```

### Example NPC Script Pattern

```c
// Async request pattern with polling
prontera,150,150,4	script	AI NPC Example	4_F_KAFRA1,{
    mes "[AI NPC]";
    mes "Let me think about that...";
    
    // Create request
    .@data$ = "{\"message\":\"" + escape(.@input$) + "\"}";
    .@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@data$, 5);
    
    if (.@req_id <= 0) {
        mes "Sorry, I'm having trouble connecting to my AI.";
        close;
    }
    
    // Poll for response (max 5 seconds)
    .@attempts = 0;
    while (.@attempts < 50) {
        .@response$ = ai_db_response(.@req_id);
        if (.@response$ != "") break;
        sleep2 100;
        .@attempts++;
    }
    
    // Handle response
    if (.@response$ != "") {
        mes "AI Response: " + .@response$;
    } else {
        mes "Sorry, that took too long. Please try again.";
    }
    close;
}
```

---

## Changelog

### Version 1.0.0 (2026-01-25)
- Initial production release
- Complete C++ implementation (5 commands)
- Python worker service with async I/O
- Comprehensive testing suite
- Full documentation

---

## Support

For issues or questions:
1. Check logs: `sudo journalctl -u ai-ipc-worker -f`
2. Run tests: `python3 test_ipc_integration.py`
3. Review architecture: `doc/NATIVE_IPC_ARCHITECTURE.md`
4. Check NPC guide: `npc/custom/AI_NPC_GUIDE.md`

**End of Documentation**
