# AI IPC Database Tables - Installation and Maintenance Guide

## Overview

The AI IPC (Inter-Process Communication) database tables enable native C++ integration with external AI services without requiring external HTTP libraries (curl, zmq, hiredis). This system uses MySQL/MariaDB as a message queue for asynchronous AI request/response handling.

## Purpose

Replace HTTP-based NPC script commands (`httpget`, `httppost`) with a reliable, database-backed IPC system that:

- **Eliminates external dependencies**: No curl, zmq, or hiredis required
- **Provides transaction safety**: ACID guarantees via InnoDB
- **Enables async processing**: Non-blocking request/response pattern
- **Maintains audit trail**: Complete request history for debugging
- **Scales efficiently**: Database connection pooling and optimized indexes

## Architecture

```
┌─────────────┐    ┌──────────────┐    ┌──────────────┐    ┌─────────────┐
│  NPC Script │───▶│ C++ BUILDIN  │───▶│  MySQL DB    │◀──▶│  External   │
│  Commands   │    │  Functions   │    │ (IPC Tables) │    │  AI Worker  │
│             │◀───│              │◀───│              │    │  (Python)   │
└─────────────┘    └──────────────┘    └──────────────┘    └─────────────┘

Flow:
1. NPC calls ai_db_request() or httpget()
2. C++ inserts request into ai_requests table
3. External worker polls ai_requests for pending items
4. Worker processes request and inserts response into ai_responses
5. NPC calls ai_db_response() to retrieve result
```

## Schema Overview

### Table: `ai_requests`

Stores outgoing AI requests from NPC scripts.

**Key Columns:**
- `id`: Auto-increment primary key
- `request_type`: Type of request (e.g., "dialogue", "http_get", "decision")
- `endpoint`: Virtual API endpoint path (e.g., "/api/v1/health")
- `request_data`: JSON payload
- `priority`: 1-10 (1=highest priority)
- `timeout`: Seconds before request expires
- `status`: Current state (pending, processing, completed, failed, timeout, cancelled)
- `created_at`, `processed_at`, `expires_at`: Timestamps
- `source_npc`, `source_map`: Context for debugging

**Indexes:**
- `idx_status_priority`: Optimizes worker polling queries
- `idx_expires_at`: Optimizes timeout detection
- `idx_created_at`: Optimizes cleanup operations

### Table: `ai_responses`

Stores responses from external AI services.

**Key Columns:**
- `id`: Auto-increment primary key
- `request_id`: Foreign key to ai_requests.id (CASCADE delete)
- `response_data`: JSON response payload
- `error_message`: Error details if failed
- `processing_time_ms`: Processing duration
- `created_at`: Response timestamp

**Indexes:**
- `idx_request_id`: Optimizes 1:1 response lookup
- `idx_created_at`: Optimizes cleanup

### Table: `ai_request_log`

Audit trail for debugging and analytics.

**Key Columns:**
- `id`: Auto-increment primary key
- `request_id`: Reference to original request
- `event_type`: Event category (created, processing, completed, failed, archived, cleanup_started, etc.)
- `event_data`: JSON event details
- `created_at`: Log entry timestamp

**Indexes:**
- `idx_request_id`: Find all events for a request
- `idx_event_type`: Filter by event type
- `idx_created_at`: Time-based queries

## Installation

### Prerequisites

- MySQL 5.7+ or MariaDB 10.3+
- rAthena server with AI IPC custom implementation
- Database user with appropriate permissions

### Step 1: Source the SQL File

#### Option A: Via main.sql (Recommended)

The `main.sql` file already includes the AI IPC tables:

```bash
mysql -u root -p ragnarok < sql-files/main.sql
```

#### Option B: Standalone Installation

```bash
mysql -u root -p ragnarok < sql-files/ai_ipc_tables.sql
```

### Step 2: Enable Event Scheduler

The automatic cleanup feature requires the event scheduler to be enabled.

#### Temporary (Current Session Only):
```sql
SET GLOBAL event_scheduler = ON;
```

#### Permanent (Recommended):
Add to your MySQL configuration file (`my.cnf` or `my.ini`):

```ini
[mysqld]
event_scheduler = ON
```

Then restart MySQL:
```bash
# Linux
sudo systemctl restart mysql

# Windows
net stop MySQL
net start MySQL
```

### Step 3: Verify Installation

```sql
-- Check tables exist
SHOW TABLES LIKE 'ai_%';
-- Expected: ai_requests, ai_responses, ai_request_log

-- Check event scheduler status
SHOW PROCESSLIST;
-- Should see "event_scheduler" user

-- Check scheduled events
SHOW EVENTS WHERE Db = 'ragnarok';
-- Should see "evt_ai_cleanup_old_requests"

-- Verify stored procedure
SHOW PROCEDURE STATUS WHERE Db = 'ragnarok' AND Name LIKE 'sp_ai%';
-- Should see "sp_ai_cleanup_old_requests"
```

### Step 4: Test Basic Functionality

```sql
-- Insert test request
INSERT INTO ai_requests (request_type, endpoint, request_data, expires_at)
VALUES ('test', '/api/v1/health', '{}', DATE_ADD(NOW(), INTERVAL 60 SECOND));

-- Check it was created
SELECT * FROM ai_requests WHERE request_type = 'test';

-- Simulate response
INSERT INTO ai_responses (request_id, response_data, processing_time_ms)
SELECT id, '{"status":"ok"}', 50 FROM ai_requests WHERE request_type = 'test';

-- Verify response
SELECT * FROM ai_responses;

-- Clean up test data
DELETE FROM ai_requests WHERE request_type = 'test';
```

## Database Permissions

Grant appropriate permissions to your rAthena database user:

```sql
-- Replace 'ragnarok'@'localhost' with your actual database user

GRANT SELECT, INSERT, UPDATE ON ragnarok.ai_requests TO 'ragnarok'@'localhost';
GRANT SELECT, INSERT ON ragnarok.ai_responses TO 'ragnarok'@'localhost';
GRANT SELECT, INSERT ON ragnarok.ai_request_log TO 'ragnarok'@'localhost';
GRANT EXECUTE ON PROCEDURE ragnarok.sp_ai_cleanup_old_requests TO 'ragnarok'@'localhost';

FLUSH PRIVILEGES;
```

## Maintenance Procedures

### Manual Cleanup

Clean up requests older than N days:

```sql
-- Clean requests older than 7 days (default)
CALL sp_ai_cleanup_old_requests(7);

-- Clean requests older than 1 day
CALL sp_ai_cleanup_old_requests(1);

-- Clean requests older than 30 days
CALL sp_ai_cleanup_old_requests(30);
```

The procedure will:
1. Mark expired pending requests as timeout
2. Archive metadata to ai_request_log
3. Delete old responses
4. Delete old requests
5. Return cleanup statistics

### Automatic Cleanup

The event `evt_ai_cleanup_old_requests` runs daily at the same time it was created + 1 hour.

**View Event Schedule:**
```sql
SHOW EVENTS WHERE Name = 'evt_ai_cleanup_old_requests';
```

**Disable Automatic Cleanup:**
```sql
ALTER EVENT evt_ai_cleanup_old_requests DISABLE;
```

**Enable Automatic Cleanup:**
```sql
ALTER EVENT evt_ai_cleanup_old_requests ENABLE;
```

**Change Cleanup Interval:**
```sql
-- Run every 6 hours instead of daily
ALTER EVENT evt_ai_cleanup_old_requests
ON SCHEDULE EVERY 6 HOUR;

-- Run every week instead of daily
ALTER EVENT evt_ai_cleanup_old_requests
ON SCHEDULE EVERY 1 WEEK;
```

**Change Retention Period:**

Edit the event definition to call the procedure with different days:

```sql
DROP EVENT evt_ai_cleanup_old_requests;

CREATE EVENT evt_ai_cleanup_old_requests
ON SCHEDULE EVERY 1 DAY
DO
  CALL sp_ai_cleanup_old_requests(14);  -- Keep 14 days instead of 7
```

### Monitoring

#### Check Request Queue Size

```sql
SELECT status, COUNT(*) as count 
FROM ai_requests 
GROUP BY status;
```

Expected output for healthy system:
```
+-----------+-------+
| status    | count |
+-----------+-------+
| pending   |    25 |
| completed |   150 |
+-----------+-------+
```

⚠️ **Warning**: If `pending` count > 1000, your worker may be overwhelmed.

#### Check Processing Performance

```sql
-- Average processing time by request type
SELECT 
  req.request_type,
  COUNT(*) as total_requests,
  AVG(resp.processing_time_ms) as avg_ms,
  MAX(resp.processing_time_ms) as max_ms,
  MIN(resp.processing_time_ms) as min_ms
FROM ai_requests req
JOIN ai_responses resp ON req.id = resp.request_id
WHERE req.status = 'completed'
  AND req.created_at > DATE_SUB(NOW(), INTERVAL 1 DAY)
GROUP BY req.request_type;
```

#### Check Error Rate

```sql
-- Error rate in last 24 hours
SELECT 
  status,
  COUNT(*) as count,
  ROUND(COUNT(*) * 100.0 / (SELECT COUNT(*) FROM ai_requests WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 DAY)), 2) as percentage
FROM ai_requests
WHERE created_at > DATE_SUB(NOW(), INTERVAL 1 DAY)
GROUP BY status;
```

#### View Recent Errors

```sql
SELECT 
  req.id,
  req.request_type,
  req.endpoint,
  resp.error_message,
  req.created_at
FROM ai_requests req
LEFT JOIN ai_responses resp ON req.id = resp.request_id
WHERE req.status = 'failed'
  AND req.created_at > DATE_SUB(NOW(), INTERVAL 1 DAY)
ORDER BY req.created_at DESC
LIMIT 20;
```

### Disk Space Management

#### Check Table Sizes

```sql
SELECT 
  table_name,
  ROUND(((data_length + index_length) / 1024 / 1024), 2) AS size_mb,
  table_rows
FROM information_schema.TABLES
WHERE table_schema = 'ragnarok'
  AND table_name LIKE 'ai_%'
ORDER BY (data_length + index_length) DESC;
```

#### Optimize Tables

If tables become fragmented after many deletions:

```sql
OPTIMIZE TABLE ai_requests;
OPTIMIZE TABLE ai_responses;
OPTIMIZE TABLE ai_request_log;
```

**Note**: OPTIMIZE locks the table during operation. Run during low-traffic periods.

## Troubleshooting

### Issue: Event Scheduler Not Running

**Symptoms:**
- Automatic cleanup not occurring
- Old requests accumulating

**Check:**
```sql
SHOW VARIABLES LIKE 'event_scheduler';
```

**Fix:**
```sql
SET GLOBAL event_scheduler = ON;
```

And add to my.cnf for persistence.

### Issue: Foreign Key Constraint Errors

**Symptoms:**
- Cannot insert response: "Cannot add or update a child row"
- Cannot delete request: "Cannot delete or update a parent row"

**Cause:**
- Attempting to insert response for non-existent request_id
- Manual deletion interfering with FK constraints

**Fix:**
Always delete via CASCADE (automatic) or delete responses first:
```sql
-- Delete in correct order
DELETE FROM ai_responses WHERE request_id = 123;
DELETE FROM ai_requests WHERE id = 123;
```

### Issue: High Pending Request Count

**Symptoms:**
- Many requests stuck in "pending" status
- Slow response times

**Check:**
```sql
SELECT COUNT(*) FROM ai_requests WHERE status = 'pending';
```

**Possible Causes:**
1. External worker not running
2. Worker polling interval too slow
3. High request volume exceeding worker capacity

**Fix:**
- Verify external AI worker process is running
- Increase worker thread count
- Reduce worker polling interval
- Add more worker instances

### Issue: Requests Timing Out

**Symptoms:**
- Many requests in "timeout" status
- NPC scripts receiving empty responses

**Check:**
```sql
SELECT COUNT(*) FROM ai_requests WHERE status = 'timeout';
```

**Fix:**
- Increase timeout value in C++ code or requests
- Optimize AI service processing time
- Check network connectivity between worker and AI service

### Issue: Database Growing Too Large

**Symptoms:**
- ai_request_log table > 1GB
- Slow query performance

**Fix:**
1. Reduce retention period:
   ```sql
   CALL sp_ai_cleanup_old_requests(1);  -- Keep only 1 day
   ```

2. Truncate log table (loses history):
   ```sql
   TRUNCATE TABLE ai_request_log;
   ```

3. Archive to external storage before truncating:
   ```bash
   mysqldump ragnarok ai_request_log > ai_request_log_archive_$(date +%Y%m%d).sql
   ```

### Issue: Slow Queries

**Symptoms:**
- Worker polling queries taking > 100ms
- NPC script commands lagging

**Check:**
```sql
SHOW PROCESSLIST;
```

**Fix:**
1. Verify indexes exist:
   ```sql
   SHOW INDEX FROM ai_requests;
   SHOW INDEX FROM ai_responses;
   ```

2. Analyze query performance:
   ```sql
   EXPLAIN SELECT * FROM ai_requests 
   WHERE status = 'pending' 
   ORDER BY priority ASC, created_at ASC 
   LIMIT 100;
   ```
   
   Should show "Using index" in Extra column.

3. Rebuild indexes if needed:
   ```sql
   ALTER TABLE ai_requests DROP INDEX idx_status_priority;
   ALTER TABLE ai_requests ADD INDEX idx_status_priority (status, priority, created_at);
   ```

## Performance Benchmarks

### Expected Performance

| Metric | Expected Value | Notes |
|--------|---------------|-------|
| Request INSERT | < 5ms | Single row insert |
| Response SELECT | < 3ms | Indexed lookup by request_id |
| Worker poll query | < 10ms | Indexed scan of pending requests |
| Cleanup procedure | < 30s | For 100K rows deletion |

### Capacity Estimates

| Scenario | Requests/Hour | 7-Day Storage | Disk Space |
|----------|--------------|---------------|------------|
| Light usage | 100 | 16,800 rows | ~10 MB |
| Medium usage | 1,000 | 168,000 rows | ~100 MB |
| Heavy usage | 10,000 | 1,680,000 rows | ~1 GB |

## Best Practices

1. **Set appropriate retention period**: Balance disk space vs. audit trail needs
2. **Monitor queue depth**: Alert if pending > 1000
3. **Review error logs**: Investigate failed requests regularly
4. **Backup before upgrades**: Export schema and data before modifications
5. **Test in development**: Validate schema changes in test environment first
6. **Use transactions**: Wrap related inserts in transactions for consistency
7. **Index monitoring**: Check slow query log for missing indexes

## Integration with NPC Scripts

### Basic Usage

```c
// NPC script example
prontera,150,150,4	script	AI Test NPC	1_M_01,{
    mes "[AI Test]";
    mes "Testing AI IPC...";
    
    // Create request
    .@req_id = ai_db_request("test", "/api/v1/health", "{}");
    if (.@req_id <= 0) {
        mes "Failed to create request!";
        close;
    }
    
    // Wait for response with polling
    .@timeout = 10;  // 10 attempts
    while (.@timeout > 0) {
        sleep2 500;  // Wait 500ms
        .@response$ = ai_db_response(.@req_id);
        if (.@response$ != "") {
            mes "Response: " + .@response$;
            close;
        }
        .@timeout--;
    }
    
    mes "Request timed out!";
    close;
}
```

### Backward Compatibility

Existing scripts using `httpget()` and `httppost()` work without modification:

```c
// Old code - still works
.@response$ = httpget("http://localhost:8000/api/v1/health");
mes .@response$;
```

## Migration from HTTP-based System

If migrating from the old HTTP-based implementation:

1. **No script changes required**: `httpget()` and `httppost()` are wrapper functions
2. **Update external workers**: Point workers to database instead of HTTP server
3. **Test thoroughly**: Verify all endpoints work correctly
4. **Monitor performance**: Database IPC may have different latency characteristics
5. **Remove old dependencies**: Uninstall curl, zmq, hiredis if no longer needed

## Support and Resources

- **Architecture Documentation**: See `/doc/NATIVE_IPC_ARCHITECTURE.md`
- **Implementation Details**: See `/src/custom/script_ai_ipc.cpp`
- **Example Scripts**: See `/npc/custom/ai_*.txt`

## Version History

- **v1.0.0** (2026-01-20): Initial release
  - Three-table schema (requests, responses, log)
  - Automatic cleanup procedure
  - Event scheduler integration
  - Full audit trail support

---

**End of README_AI_IPC.md**
