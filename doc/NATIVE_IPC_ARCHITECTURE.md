# Native IPC Architecture for AI Service Integration

## Document Information
- **Version**: 1.0.0
- **Author**: AI Architect
- **Created**: 2025-11-27
- **Status**: Design Complete

## Executive Summary

This document defines the architecture for replacing HTTP-based NPC script commands (`httpget`, `httppost`) with native C++ database-based IPC (Inter-Process Communication). The solution eliminates external library dependencies (curl, zmq, hiredis) by leveraging the existing MySQL infrastructure.

---

## 1. Solution Architecture Overview

### 1.1 Problem Statement

The current HTTP implementation has multiple critical issues:
- **Multiple definition errors**: Duplicate implementations in `script.cpp` and `script_http.cpp`
- **Missing library linkages**: `-lcurl`, `-lzmq`, `-lhiredis` not linked
- **Naming conflicts**: `curl_write_callback` conflicts with curl.h typedef
- **External dependencies**: Requires curl library installation and maintenance

### 1.2 Proposed Solution: Database-Based IPC

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        DATABASE-BASED IPC ARCHITECTURE                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌────────────┐ │
│  │  NPC Script  │───▶│ Native C++   │───▶│   MySQL DB   │◀──▶│  External  │ │
│  │  Commands    │    │ BUILDIN_FUNC │    │ (ai_requests │    │  Service   │ │
│  │              │◀───│              │◀───│ ai_responses)│    │  (Python)  │ │
│  └──────────────┘    └──────────────┘    └──────────────┘    └────────────┘ │
│                                                                              │
│  Flow:                                                                       │
│  1. NPC script calls ai_db_request()                                        │
│  2. C++ inserts request into ai_requests table                              │
│  3. External service polls and processes requests                           │
│  4. External service writes response to ai_responses table                  │
│  5. NPC script calls ai_db_response() to retrieve result                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 1.3 Architecture Benefits

| Aspect | HTTP (Current) | Database IPC (Proposed) |
|--------|----------------|-------------------------|
| Dependencies | curl, zmq, hiredis | MySQL only (existing) |
| Build Complexity | High (multiple libs) | None (uses existing) |
| Reliability | Network dependent | Database transaction |
| Debugging | Network traces | Query logs |
| Scalability | Limited by threads | Database connection pool |
| Persistence | None | Full request/response audit trail |

---

## 2. Database Schema Design

### 2.1 ai_requests Table

```sql
--
-- Table structure for `ai_requests`
-- Stores outgoing requests from NPC scripts to external AI service
--

CREATE TABLE IF NOT EXISTS `ai_requests` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `request_type` VARCHAR(50) NOT NULL DEFAULT 'generic',
  `npc_id` VARCHAR(50) DEFAULT NULL,
  `player_id` INT(11) UNSIGNED DEFAULT NULL,
  `endpoint` VARCHAR(255) NOT NULL COMMENT 'Virtual endpoint path (e.g., /api/v1/health)',
  `method` ENUM('GET', 'POST') NOT NULL DEFAULT 'GET',
  `request_data` TEXT DEFAULT NULL COMMENT 'JSON payload for POST requests',
  `status` ENUM('pending', 'processing', 'completed', 'failed', 'expired') NOT NULL DEFAULT 'pending',
  `priority` TINYINT(3) UNSIGNED NOT NULL DEFAULT '5' COMMENT '1=highest, 10=lowest',
  `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `processed_at` DATETIME DEFAULT NULL,
  `expires_at` DATETIME NOT NULL DEFAULT (CURRENT_TIMESTAMP + INTERVAL 60 SECOND),
  `map_name` VARCHAR(30) DEFAULT NULL,
  `source_script` VARCHAR(100) DEFAULT NULL COMMENT 'NPC script name for debugging',
  PRIMARY KEY (`id`),
  KEY `idx_status_priority` (`status`, `priority`, `created_at`),
  KEY `idx_expires_at` (`expires_at`),
  KEY `idx_npc_id` (`npc_id`),
  KEY `idx_player_id` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI service request queue';
```

### 2.2 ai_responses Table

```sql
--
-- Table structure for `ai_responses`
-- Stores responses from external AI service back to NPC scripts
--

CREATE TABLE IF NOT EXISTS `ai_responses` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `request_id` BIGINT(20) UNSIGNED NOT NULL,
  `response_data` TEXT DEFAULT NULL COMMENT 'JSON response payload',
  `http_status` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '200',
  `error_message` VARCHAR(500) DEFAULT NULL,
  `processing_time_ms` INT(10) UNSIGNED DEFAULT NULL,
  `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `idx_request_id` (`request_id`),
  KEY `idx_created_at` (`created_at`),
  CONSTRAINT `fk_response_request` FOREIGN KEY (`request_id`) 
    REFERENCES `ai_requests` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI service response storage';
```

### 2.3 ai_request_log Table (Audit Trail)

```sql
--
-- Table structure for `ai_request_log`
-- Archival table for completed requests (for analytics and debugging)
--

CREATE TABLE IF NOT EXISTS `ai_request_log` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `original_request_id` BIGINT(20) UNSIGNED NOT NULL,
  `request_type` VARCHAR(50) NOT NULL,
  `endpoint` VARCHAR(255) NOT NULL,
  `method` ENUM('GET', 'POST') NOT NULL,
  `request_data` TEXT DEFAULT NULL,
  `response_data` TEXT DEFAULT NULL,
  `http_status` SMALLINT(5) UNSIGNED DEFAULT NULL,
  `processing_time_ms` INT(10) UNSIGNED DEFAULT NULL,
  `final_status` ENUM('completed', 'failed', 'expired') NOT NULL,
  `npc_id` VARCHAR(50) DEFAULT NULL,
  `player_id` INT(11) UNSIGNED DEFAULT NULL,
  `map_name` VARCHAR(30) DEFAULT NULL,
  `created_at` DATETIME NOT NULL,
  `completed_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_completed_at` (`completed_at`),
  KEY `idx_request_type` (`request_type`),
  KEY `idx_final_status` (`final_status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='AI request audit log';
```

### 2.4 Index Design Rationale

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           INDEX OPTIMIZATION STRATEGY                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ai_requests.idx_status_priority:                                            │
│    Purpose: External service polling query                                   │
│    Query: SELECT * FROM ai_requests                                          │
│           WHERE status = 'pending'                                           │
│           ORDER BY priority ASC, created_at ASC                              │
│           LIMIT 100                                                          │
│                                                                              │
│  ai_requests.idx_expires_at:                                                 │
│    Purpose: Cleanup expired requests                                         │
│    Query: UPDATE ai_requests SET status = 'expired'                          │
│           WHERE status = 'pending' AND expires_at < NOW()                    │
│                                                                              │
│  ai_responses.idx_request_id:                                                │
│    Purpose: Script response retrieval (UNIQUE for 1:1 mapping)               │
│    Query: SELECT response_data FROM ai_responses                             │
│           WHERE request_id = ?                                               │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 2.5 Data Cleanup Strategy

```sql
-- Scheduled cleanup procedure (run every hour)
-- Move completed requests to log table and purge old data

DELIMITER //
CREATE PROCEDURE IF NOT EXISTS `ai_cleanup_requests`()
BEGIN
  DECLARE v_cutoff DATETIME;
  SET v_cutoff = DATE_SUB(NOW(), INTERVAL 24 HOUR);
  
  -- Archive completed/failed requests older than 24 hours
  INSERT INTO ai_request_log 
    (original_request_id, request_type, endpoint, method, request_data,
     response_data, http_status, processing_time_ms, final_status,
     npc_id, player_id, map_name, created_at)
  SELECT 
    r.id, r.request_type, r.endpoint, r.method, r.request_data,
    resp.response_data, resp.http_status, resp.processing_time_ms, r.status,
    r.npc_id, r.player_id, r.map_name, r.created_at
  FROM ai_requests r
  LEFT JOIN ai_responses resp ON r.id = resp.request_id
  WHERE r.status IN ('completed', 'failed', 'expired')
    AND r.created_at < v_cutoff;
  
  -- Delete archived responses
  DELETE resp FROM ai_responses resp
  INNER JOIN ai_requests r ON resp.request_id = r.id
  WHERE r.status IN ('completed', 'failed', 'expired')
    AND r.created_at < v_cutoff;
  
  -- Delete archived requests
  DELETE FROM ai_requests 
  WHERE status IN ('completed', 'failed', 'expired')
    AND created_at < v_cutoff;
  
  -- Mark expired pending requests
  UPDATE ai_requests 
  SET status = 'expired'
  WHERE status = 'pending' AND expires_at < NOW();
  
END //
DELIMITER ;

-- Schedule via MySQL Event Scheduler or external cron
CREATE EVENT IF NOT EXISTS `ai_cleanup_event`
ON SCHEDULE EVERY 1 HOUR
DO CALL ai_cleanup_requests();
```

---

## 3. New Script Command Definitions

### 3.1 Command Overview

| Command | Signature | Description |
|---------|-----------|-------------|
| `ai_db_request` | `ai_db_request(type, endpoint, data{, priority})` | Submit async request, returns request_id |
| `ai_db_response` | `ai_db_response(request_id)` | Non-blocking poll for response |
| `ai_db_wait` | `ai_db_wait(request_id, timeout_ms)` | Blocking wait for response |
| `ai_db_status` | `ai_db_status(request_id)` | Check request status |
| `ai_db_cancel` | `ai_db_cancel(request_id)` | Cancel pending request |

### 3.2 BUILDIN_DEF Registration

```c
// Add to script_def.inc or script.cpp BUILDIN_DEF section
BUILDIN_DEF(ai_db_request, "ss?i"),   // type, endpoint, data (opt), priority (opt)
BUILDIN_DEF(ai_db_response, "i"),      // request_id -> returns string
BUILDIN_DEF(ai_db_wait, "ii"),         // request_id, timeout_ms -> returns string
BUILDIN_DEF(ai_db_status, "i"),        // request_id -> returns status code
BUILDIN_DEF(ai_db_cancel, "i"),        // request_id -> returns 1/0

// Backward-compatible wrappers
BUILDIN_DEF(httpget, "s"),             // url -> string (maps to ai_db_request + ai_db_wait)
BUILDIN_DEF(httppost, "ss"),           // url, data -> string (maps to ai_db_request + ai_db_wait)
```

### 3.3 Parameter Signatures Explained

```
Signature Key:
  s  = string (required)
  i  = integer (required)
  ?  = following params are optional
  *  = variable arguments

ai_db_request("ss?i"):
  - arg 2: type (string) - request type identifier
  - arg 3: endpoint (string) - virtual API endpoint  
  - arg 4: data (string, optional) - JSON payload
  - arg 5: priority (int, optional) - 1-10, default 5

ai_db_response("i"):
  - arg 2: request_id (int)
  - returns: response JSON string, or "" if not ready

ai_db_wait("ii"):
  - arg 2: request_id (int)
  - arg 3: timeout_ms (int) - max wait time
  - returns: response JSON string, or "" on timeout

ai_db_status("i"):
  - arg 2: request_id (int)
  - returns: 0=pending, 1=processing, 2=completed, 3=failed, 4=expired

ai_db_cancel("i"):
  - arg 2: request_id (int)
  - returns: 1=cancelled, 0=could not cancel
```

---

## 4. C++ Implementation Design

### 4.1 File Structure

```
src/
├── custom/
│   ├── script_ai_ipc.cpp      # New: AI IPC implementation
│   ├── script_ai_ipc.hpp      # New: AI IPC headers
│   └── script_def.inc         # Modify: Add BUILDIN_DEF entries
├── map/
│   └── script.cpp             # Modify: Remove stub definitions
```

### 4.2 Header File Design (script_ai_ipc.hpp)

```cpp
// script_ai_ipc.hpp
// AI Database IPC Native Commands for rAthena Script Engine

#ifndef SCRIPT_AI_IPC_HPP
#define SCRIPT_AI_IPC_HPP

#include <common/cbasetypes.hpp>
#include <common/sql.hpp>
#include <map/script.hpp>

// Request status codes
enum e_ai_request_status {
    AI_REQ_PENDING = 0,
    AI_REQ_PROCESSING = 1,
    AI_REQ_COMPLETED = 2,
    AI_REQ_FAILED = 3,
    AI_REQ_EXPIRED = 4
};

// Configuration constants
#define AI_IPC_DEFAULT_TIMEOUT_MS 5000
#define AI_IPC_MAX_POLL_INTERVAL_MS 100
#define AI_IPC_MAX_REQUEST_DATA_SIZE 10240
#define AI_IPC_MAX_RESPONSE_SIZE 65535

// Function prototypes
void script_ai_ipc_init();
void script_ai_ipc_final();

// Internal helper functions (not exposed to scripts)
int64 ai_ipc_insert_request(const char* type, const char* endpoint, 
                            const char* data, int priority,
                            const char* npc_id, int player_id, 
                            const char* map_name, const char* source_script);
                            
const char* ai_ipc_get_response(int64 request_id);
int ai_ipc_get_status(int64 request_id);
bool ai_ipc_cancel_request(int64 request_id);
const char* ai_ipc_wait_response(int64 request_id, int timeout_ms);

#endif // SCRIPT_AI_IPC_HPP
```

### 4.3 Implementation Design (script_ai_ipc.cpp)

```cpp
// Pseudo-code implementation structure following rathena patterns

/**
 * BUILDIN_FUNC(ai_db_request)
 * 
 * Insert a new AI request into the database queue.
 * Returns: request_id (positive int) on success, -1 on failure
 * 
 * Script usage:
 *   .@req_id = ai_db_request("health_check", "/api/v1/health");
 *   .@req_id = ai_db_request("event", "/api/v1/events/dispatch", .@json$, 1);
 */
BUILDIN_FUNC(ai_db_request) {
    // 1. Extract parameters
    const char* type = script_getstr(st, 2);
    const char* endpoint = script_getstr(st, 3);
    const char* data = script_hasdata(st, 4) ? script_getstr(st, 4) : "";
    int priority = script_hasdata(st, 5) ? script_getnum(st, 5) : 5;
    
    // 2. Validate inputs
    //    - Check endpoint length <= 255
    //    - Check data length <= 10240
    //    - Check priority 1-10
    
    // 3. Get context info (player_id, map_name, npc_name)
    map_session_data* sd = nullptr;
    script_rid2sd(sd);  // May be null for non-player contexts
    
    npc_data* nd = map_id2nd(st->oid);
    const char* npc_name = nd ? nd->exname : "unknown";
    const char* map_name = nd ? map_getmapdata(nd->m)->name : "";
    int player_id = sd ? sd->status.char_id : 0;
    
    // 4. Build and execute INSERT query
    //    INSERT INTO ai_requests (request_type, npc_id, player_id, 
    //                            endpoint, method, request_data, ...)
    //    VALUES (...)
    
    // 5. Get last insert ID
    //    Sql_LastInsertId() or SELECT LAST_INSERT_ID()
    
    // 6. Log and return
    ShowInfo("script:ai_db_request: Created request #%lld type='%s' endpoint='%s'\n",
             request_id, type, endpoint);
    script_pushint(st, request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(ai_db_response)
 * 
 * Non-blocking check for response availability.
 * Returns: response JSON string, or "" if not ready
 */
BUILDIN_FUNC(ai_db_response) {
    int64 request_id = script_getnum(st, 2);
    
    // Query: SELECT response_data FROM ai_responses WHERE request_id = ?
    // Also update ai_requests.status = 'completed' if not already
    
    const char* response = ai_ipc_get_response(request_id);
    if (response && strlen(response) > 0) {
        script_pushstrcopy(st, response);
    } else {
        script_pushconststr(st, "");
    }
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(ai_db_wait)
 * 
 * Blocking wait for response with timeout.
 * WARNING: This blocks the script! Use sparingly.
 */
BUILDIN_FUNC(ai_db_wait) {
    int64 request_id = script_getnum(st, 2);
    int timeout_ms = script_getnum(st, 3);
    
    // Clamp timeout to reasonable range (100ms - 30000ms)
    timeout_ms = cap_value(timeout_ms, 100, 30000);
    
    // Polling loop with sleep
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        const char* response = ai_ipc_get_response(request_id);
        if (response && strlen(response) > 0) {
            script_pushstrcopy(st, response);
            return SCRIPT_CMD_SUCCESS;
        }
        
        // Sleep for poll interval
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed += 100;
    }
    
    // Timeout - return empty string
    ShowWarning("script:ai_db_wait: Request #%lld timed out after %dms\n",
                request_id, timeout_ms);
    script_pushconststr(st, "");
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(ai_db_status)
 * 
 * Check current status of a request.
 * Returns: 0=pending, 1=processing, 2=completed, 3=failed, 4=expired
 */
BUILDIN_FUNC(ai_db_status) {
    int64 request_id = script_getnum(st, 2);
    int status = ai_ipc_get_status(request_id);
    script_pushint(st, status);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(ai_db_cancel)
 * 
 * Cancel a pending request (only works if status is still 'pending').
 * Returns: 1 if cancelled, 0 if could not cancel
 */
BUILDIN_FUNC(ai_db_cancel) {
    int64 request_id = script_getnum(st, 2);
    bool cancelled = ai_ipc_cancel_request(request_id);
    script_pushint(st, cancelled ? 1 : 0);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(httpget) - Backward compatible wrapper
 * 
 * Extracts endpoint from URL and performs synchronous request.
 */
BUILDIN_FUNC(httpget) {
    const char* url = script_getstr(st, 2);
    
    // Parse URL to extract endpoint (strip protocol and host)
    // e.g., "http://192.168.0.100:8000/api/v1/health" -> "/api/v1/health"
    const char* endpoint = extract_endpoint_from_url(url);
    
    // Create request
    int64 request_id = ai_ipc_insert_request("httpget", endpoint, "", 5,
                                             /*context info*/);
    
    if (request_id <= 0) {
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Wait for response (default 5 second timeout)
    const char* response = ai_ipc_wait_response(request_id, 5000);
    
    if (response && strlen(response) > 0) {
        script_pushstrcopy(st, response);
    } else {
        script_pushconststr(st, "");
    }
    return SCRIPT_CMD_SUCCESS;
}

/**
 * BUILDIN_FUNC(httppost) - Backward compatible wrapper
 */
BUILDIN_FUNC(httppost) {
    const char* url = script_getstr(st, 2);
    const char* data = script_getstr(st, 3);
    
    const char* endpoint = extract_endpoint_from_url(url);
    
    int64 request_id = ai_ipc_insert_request("httppost", endpoint, data, 5,
                                             /*context info*/);
    
    if (request_id <= 0) {
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    const char* response = ai_ipc_wait_response(request_id, 5000);
    
    if (response && strlen(response) > 0) {
        script_pushstrcopy(st, response);
    } else {
        script_pushconststr(st, "");
    }
    return SCRIPT_CMD_SUCCESS;
}
```

### 4.4 SQL Query Templates

```cpp
// Query templates for ai_ipc functions

// INSERT request
const char* SQL_INSERT_REQUEST = 
    "INSERT INTO `ai_requests` "
    "(`request_type`, `npc_id`, `player_id`, `endpoint`, `method`, "
    "`request_data`, `priority`, `map_name`, `source_script`, `expires_at`) "
    "VALUES ('%s', '%s', %d, '%s', '%s', '%s', %d, '%s', '%s', "
    "DATE_ADD(NOW(), INTERVAL %d SECOND))";

// GET response
const char* SQL_GET_RESPONSE = 
    "SELECT r.`response_data`, r.`http_status`, r.`error_message` "
    "FROM `ai_responses` r "
    "WHERE r.`request_id` = %lld";

// GET status
const char* SQL_GET_STATUS = 
    "SELECT `status` FROM `ai_requests` WHERE `id` = %lld";

// CANCEL request
const char* SQL_CANCEL_REQUEST = 
    "UPDATE `ai_requests` SET `status` = 'expired' "
    "WHERE `id` = %lld AND `status` = 'pending'";

// UPDATE status to completed (when response retrieved)
const char* SQL_MARK_COMPLETED =
    "UPDATE `ai_requests` SET `status` = 'completed', `processed_at` = NOW() "
    "WHERE `id` = %lld AND `status` IN ('pending', 'processing')";
```

---

## 5. External Service Interface

### 5.1 Polling Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    EXTERNAL SERVICE POLLING DESIGN                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                     Python/Node.js AI Service                        │    │
│  │                                                                       │    │
│  │  ┌───────────────┐    ┌───────────────┐    ┌───────────────┐        │    │
│  │  │ Poll Thread   │───▶│ Request       │───▶│ AI Processing │        │    │
│  │  │ (100ms loop)  │    │ Dispatcher    │    │ (LLM, Logic)  │        │    │
│  │  └───────────────┘    └───────────────┘    └───────────────┘        │    │
│  │          │                    │                    │                 │    │
│  │          ▼                    │                    ▼                 │    │
│  │  ┌───────────────┐            │           ┌───────────────┐         │    │
│  │  │ SELECT FROM   │            │           │ INSERT INTO   │         │    │
│  │  │ ai_requests   │            │           │ ai_responses  │         │    │
│  │  │ WHERE pending │            │           │               │         │    │
│  │  └───────────────┘            │           └───────────────┘         │    │
│  │          │                    │                    │                 │    │
│  │          └────────────────────┴────────────────────┘                 │    │
│  │                               │                                      │    │
│  │                               ▼                                      │    │
│  │                      ┌───────────────┐                               │    │
│  │                      │   MySQL DB    │                               │    │
│  │                      │ ai_requests   │                               │    │
│  │                      │ ai_responses  │                               │    │
│  │                      └───────────────┘                               │    │
│  │                                                                       │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 5.2 Polling Service Pseudocode (Python)

```python
# ai_request_processor.py
# External service that polls database for AI requests

import mysql.connector
import json
import time
import logging
from typing import Optional, Dict, Any
from concurrent.futures import ThreadPoolExecutor

# Configuration (from environment variables)
DB_CONFIG = {
    'host': os.getenv('DB_HOST', 'localhost'),
    'port': int(os.getenv('DB_PORT', 3306)),
    'user': os.getenv('DB_USER', 'ragnarok'),
    'password': os.getenv('DB_PASSWORD'),  # No defaults for secrets
    'database': os.getenv('DB_NAME', 'ragnarok'),
}

POLL_INTERVAL_MS = int(os.getenv('POLL_INTERVAL_MS', 100))
BATCH_SIZE = int(os.getenv('BATCH_SIZE', 50))
WORKER_THREADS = int(os.getenv('WORKER_THREADS', 4))

class AIRequestProcessor:
    def __init__(self):
        self.db_pool = mysql.connector.pooling.MySQLConnectionPool(
            pool_name="ai_pool",
            pool_size=WORKER_THREADS + 2,
            **DB_CONFIG
        )
        self.executor = ThreadPoolExecutor(max_workers=WORKER_THREADS)
        self.running = True
        self.logger = logging.getLogger(__name__)
    
    def poll_requests(self) -> list:
        """Fetch pending requests from database"""
        conn = self.db_pool.get_connection()
        cursor = conn.cursor(dictionary=True)
        
        try:
            # Lock and fetch pending requests
            cursor.execute("""
                SELECT id, request_type, endpoint, method, request_data,
                       npc_id, player_id, map_name, source_script
                FROM ai_requests
                WHERE status = 'pending'
                  AND expires_at > NOW()
                ORDER BY priority ASC, created_at ASC
                LIMIT %s
                FOR UPDATE SKIP LOCKED
            """, (BATCH_SIZE,))
            
            requests = cursor.fetchall()
            
            if requests:
                # Mark as processing
                ids = [r['id'] for r in requests]
                cursor.execute("""
                    UPDATE ai_requests 
                    SET status = 'processing'
                    WHERE id IN (%s)
                """ % ','.join(['%s'] * len(ids)), ids)
                conn.commit()
            
            return requests
            
        finally:
            cursor.close()
            conn.close()
    
    def process_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        """Route and process a single request"""
        endpoint = request['endpoint']
        method = request['method']
        data = json.loads(request['request_data']) if request['request_data'] else {}
        
        start_time = time.time()
        
        try:
            # Route to appropriate handler based on endpoint
            if endpoint == '/api/v1/health':
                response = self.handle_health_check()
            elif endpoint.startswith('/api/v1/events/'):
                response = self.handle_event(endpoint, data)
            elif endpoint.startswith('/api/v1/npc/'):
                response = self.handle_npc_request(endpoint, method, data)
            elif endpoint.startswith('/api/v1/procedural/'):
                response = self.handle_procedural(endpoint, data)
            else:
                response = {'error': f'Unknown endpoint: {endpoint}'}
            
            processing_time = int((time.time() - start_time) * 1000)
            
            return {
                'request_id': request['id'],
                'response_data': json.dumps(response),
                'http_status': 200,
                'error_message': None,
                'processing_time_ms': processing_time
            }
            
        except Exception as e:
            self.logger.error(f"Error processing request {request['id']}: {e}")
            return {
                'request_id': request['id'],
                'response_data': json.dumps({'error': str(e)}),
                'http_status': 500,
                'error_message': str(e),
                'processing_time_ms': int((time.time() - start_time) * 1000)
            }
    
    def store_response(self, response: Dict[str, Any]):
        """Store response in database"""
        conn = self.db_pool.get_connection()
        cursor = conn.cursor()
        
        try:
            cursor.execute("""
                INSERT INTO ai_responses 
                (request_id, response_data, http_status, error_message, processing_time_ms)
                VALUES (%s, %s, %s, %s, %s)
            """, (
                response['request_id'],
                response['response_data'],
                response['http_status'],
                response['error_message'],
                response['processing_time_ms']
            ))
            
            # Update request status
            status = 'completed' if response['http_status'] == 200 else 'failed'
            cursor.execute("""
                UPDATE ai_requests 
                SET status = %s, processed_at = NOW()
                WHERE id = %s
            """, (status, response['request_id']))
            
            conn.commit()
            
        finally:
            cursor.close()
            conn.close()
    
    def run(self):
        """Main polling loop"""
        self.logger.info("AI Request Processor started")
        
        while self.running:
            try:
                requests = self.poll_requests()
                
                if requests:
                    # Process in parallel
                    futures = [
                        self.executor.submit(self.process_request, req)
                        for req in requests
                    ]
                    
                    for future in futures:
                        response = future.result()
                        self.store_response(response)
                
                # Sleep between polls
                time.sleep(POLL_INTERVAL_MS / 1000.0)
                
            except Exception as e:
                self.logger.error(f"Poll loop error: {e}")
                time.sleep(1)  # Back off on error
    
    # Handler methods (implement actual AI logic)
    def handle_health_check(self) -> Dict:
        return {'status': 'healthy', 'timestamp': time.time()}
    
    def handle_event(self, endpoint: str, data: Dict) -> Dict:
        # Implement event handling logic
        return {'accepted': True, 'event_id': 'evt_' + str(time.time())}
    
    def handle_npc_request(self, endpoint: str, method: str, data: Dict) -> Dict:
        # Implement NPC AI logic
        return {'result': 'processed', 'data': data}
    
    def handle_procedural(self, endpoint: str, data: Dict) -> Dict:
        # Implement procedural content generation
        return {'generated': True}

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    processor = AIRequestProcessor()
    processor.run()
```

### 5.3 Service Configuration

```yaml
# config/ai_service.yaml
# Configuration for external AI service

database:
  host: ${DB_HOST:localhost}
  port: ${DB_PORT:3306}
  user: ${DB_USER}
  password: ${DB_PASSWORD}  # From environment only
  database: ${DB_NAME:ragnarok}
  pool_size: 10

polling:
  interval_ms: 100
  batch_size: 50
  timeout_seconds: 60

processing:
  worker_threads: 4
  max_retries: 3
  retry_delay_ms: 1000

logging:
  level: INFO
  format: "%(asctime)s [%(levelname)s] %(name)s: %(message)s"
```

---

## 6. Migration Strategy

### 6.1 Migration Mapping Table

| Old HTTP Pattern | New Native Pattern |
|------------------|-------------------|
| `httpget(url)` | `ai_db_request("get", endpoint)` + `ai_db_wait(id, 5000)` |
| `httppost(url, data)` | `ai_db_request("post", endpoint, data)` + `ai_db_wait(id, 5000)` |
| URL parsing | Endpoint extraction (strip host/protocol) |
| Response parsing | Same JSON response format |

### 6.2 NPC Script Migration Examples

#### Example 1: Health Check (test_httpget.txt)

**Before:**
```c
// OLD - Using httpget
.@response$ = httpget("http://192.168.0.100:8000/api/v1/health");
if (.@response$ == "") {
    mes "^FF0000Error: AI service is not responding!^000000";
    close;
}
```

**After (Simple - Backward Compatible):**
```c
// NEW - Using backward-compatible httpget wrapper
// No changes needed! httpget internally uses database IPC
.@response$ = httpget("http://192.168.0.100:8000/api/v1/health");
if (.@response$ == "") {
    mes "^FF0000Error: AI service is not responding!^000000";
    close;
}
```

**After (Advanced - Async Pattern):**
```c
// NEW - Using async pattern for better performance
.@req_id = ai_db_request("health", "/api/v1/health");
if (.@req_id <= 0) {
    mes "^FF0000Error: Failed to submit request!^000000";
    close;
}

// Non-blocking: do other things while waiting
mes "Checking AI service...";
sleep2 500; // Let player see the message

// Now wait for response
.@response$ = ai_db_wait(.@req_id, 5000);
if (.@response$ == "") {
    mes "^FF0000Error: AI service timed out!^000000";
    close;
}
```

#### Example 2: Event Dispatch (test_httppost.txt)

**Before:**
```c
// OLD - Using httppost
.@payload$ = "{\"event_type\":\"player_login\",\"player_id\":" + .@player_id + "}";
.@response$ = httppost("http://192.168.0.100:8000/api/v1/events/dispatch", .@payload$);
```

**After (Simple):**
```c
// NEW - Same API, different backend
.@payload$ = "{\"event_type\":\"player_login\",\"player_id\":" + .@player_id + "}";
.@response$ = httppost("http://192.168.0.100:8000/api/v1/events/dispatch", .@payload$);
```

**After (Advanced - Fire and Forget):**
```c
// NEW - Fire and forget pattern for events
.@payload$ = "{\"event_type\":\"player_login\",\"player_id\":" + .@player_id + "}";
.@req_id = ai_db_request("event", "/api/v1/events/dispatch", .@payload$, 8); // Low priority

// Don't wait for response - events can be async
if (.@req_id > 0) {
    debugmes "Event queued: request #" + .@req_id;
}
```

#### Example 3: NPC State Query with Retry

**Before:**
```c
// OLD - Simple query, no retry logic
.@url$ = "http://192.168.0.100:8000/api/v1/npc/" + .@npc_id + "/state";
.@response$ = httpget(.@url$);
```

**After (With Retry Logic):**
```c
// NEW - With retry logic and status checking
.@endpoint$ = "/api/v1/npc/" + .@npc_id + "/state";
.@req_id = ai_db_request("npc_state", .@endpoint$);

.@retries = 3;
.@response$ = "";

while (.@retries > 0 && .@response$ == "") {
    sleep2 1000;
    .@status = ai_db_status(.@req_id);
    
    if (.@status == 2) { // Completed
        .@response$ = ai_db_response(.@req_id);
    } else if (.@status >= 3) { // Failed or Expired
        .@retries -= 1;
        if (.@retries > 0) {
            // Retry with new request
            .@req_id = ai_db_request("npc_state", .@endpoint$);
        }
    }
}
```

### 6.3 Migration Checklist by File

#### test_httpget.txt (4 HTTP calls)

| Line | Original Call | Migration Status |
|------|--------------|------------------|
| 16 | `httpget("/api/v1/health")` | ✅ Backward compatible |
| 44 | `httpget(".@url$")` - dynamic NPC state | ✅ Backward compatible |
| 63 | `httpget("/api/v1/procedural/problem/active")` | ✅ Backward compatible |

#### test_httppost.txt (6 HTTP calls)

| Line | Original Call | Migration Status |
|------|--------------|------------------|
| 38 | `httppost("/api/v1/events/dispatch", ...)` | ✅ Backward compatible |
| 70 | `httppost("/api/v1/events/dispatch", ...)` | ✅ Backward compatible |
| 93 | `httppost("/api/v1/events/dispatch", ...)` | ✅ Backward compatible |
| 123 | `httppost("/api/v1/events/dispatch", ...)` | ✅ Backward compatible |

#### ai_integration_demo.txt (10+ HTTP calls)

| Line | Original Call | Migration Status |
|------|--------------|------------------|
| 22 | `httpget("/health")` | ✅ Backward compatible |
| 29 | `httppost("/api/npc/register", ...)` | ✅ Backward compatible |
| 36 | `httppost("/api/npc/movement", ...)` | ✅ Backward compatible |
| 42 | `httpget("/api/npc/.../state")` | ✅ Backward compatible |
| 122 | `httppost("/api/npc/.../interact", ...)` | ✅ Backward compatible |
| 148 | `httpget("/health")` | ✅ Backward compatible |
| 160 | `httppost("/api/npc/movement", ...)` | ✅ Backward compatible |
| 169 | `httpget("/health")` | ✅ Backward compatible |

---

## 7. Error Handling & Edge Cases

### 7.1 Error Categories

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           ERROR HANDLING MATRIX                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Category           │ Detection              │ Response                      │
│  ─────────────────────────────────────────────────────────────────────────  │
│  DB Connection      │ SQL_ERROR on query     │ Return -1/empty, log error   │
│  Request Timeout    │ expires_at < NOW()     │ Status = 'expired'           │
│  Invalid JSON       │ JSON parse error       │ Status = 'failed', log error │
│  Service Unavail    │ No response in timeout │ Return empty string          │
│  Max Queue Size     │ Pending count > limit  │ Reject new requests          │
│  Invalid Request    │ Validation failure     │ Return -1, log warning       │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### 7.2 Error Response Codes

| Status Code | Meaning | Script Return |
|-------------|---------|---------------|
| -1 | Database error / validation failed | `script_pushint(st, -1)` |
| 0 | Pending (not ready) | `script_pushconststr(st, "")` |
| -2 | Request expired | `script_pushconststr(st, "")` |
| -3 | Request cancelled | `script_pushconststr(st, "")` |

### 7.3 Timeout Handling

```cpp
// Timeout configuration
#define AI_IPC_MIN_TIMEOUT_MS 100     // Minimum wait
#define AI_IPC_MAX_TIMEOUT_MS 30000   // Maximum wait (30 seconds)
#define AI_IPC_DEFAULT_TIMEOUT_MS 5000 // Default (5 seconds)

// Request expiration calculation
// expires_at = created_at + INTERVAL ? SECOND
// Default: 60 seconds from creation
```

### 7.4 Database Connection Failure Recovery

```cpp
// Connection failure handling pattern
int ai_ipc_insert_request(...) {
    if (SQL_ERROR == Sql_QueryStr(qsmysql_handle, query)) {
        Sql_ShowDebug(qsmysql_handle);
        ShowError("script:ai_db_request: Database error\n");
        return -1;  // Caller should handle gracefully
    }
    
    // Success path
    return last_insert_id;
}
```

### 7.5 Queue Overflow Protection

```sql
-- Add constraint to prevent queue overflow
-- Check before INSERT in C++ code

SELECT COUNT(*) as pending_count 
FROM ai_requests 
WHERE status = 'pending';

-- If pending_count > MAX_QUEUE_SIZE (e.g., 10000)
-- Reject new requests with warning
```

---

## 8. Files to Create/Modify

### 8.1 New Files to Create

| File Path | Purpose | Size Estimate |
|-----------|---------|---------------|
| `src/custom/script_ai_ipc.hpp` | Header with structs and prototypes | ~80 lines |
| `src/custom/script_ai_ipc.cpp` | Implementation of all BUILDIN_FUNC | ~350 lines |
| `sql-files/ai_ipc_tables.sql` | Database schema DDL | ~100 lines |
| `doc/NATIVE_IPC_ARCHITECTURE.md` | This document | ~500 lines |

### 8.2 Files to Modify

| File Path | Modification | Lines Affected |
|-----------|--------------|----------------|
| `src/custom/script_def.inc` | Add BUILDIN_DEF entries | +6 lines |
| `src/map/script.cpp` | Remove httpget/httppost stubs (lines 27879-27928) | -50 lines |
| `src/map/script.cpp` | Include script_ai_ipc.hpp | +1 line |
| `Makefile` or `CMakeLists.txt` | Add script_ai_ipc.cpp to build | +1 line |

### 8.3 Files to Delete (Optional Cleanup)

| File Path | Reason |
|-----------|--------|
| `src/custom/script_http.cpp` | Replaced by script_ai_ipc.cpp |
| `src/custom/script_http.hpp` | Replaced by script_ai_ipc.hpp |

### 8.4 Modification Details

#### script_def.inc Additions
```cpp
// Add after existing BUILDIN_DEF entries
BUILDIN_DEF(ai_db_request, "ss?i"),
BUILDIN_DEF(ai_db_response, "i"),
BUILDIN_DEF(ai_db_wait, "ii"),
BUILDIN_DEF(ai_db_status, "i"),
BUILDIN_DEF(ai_db_cancel, "i"),
// Backward-compatible wrappers (replace broken implementations)
BUILDIN_DEF(httpget, "s"),
BUILDIN_DEF(httppost, "ss"),
```

#### script.cpp Modifications
```cpp
// At top of file, add include:
#include "../custom/script_ai_ipc.hpp"

// Remove stub implementations at lines 27879-27928
// These will be replaced by script_ai_ipc.cpp implementations
```

---

## 9. Sequence Diagrams

### 9.1 Synchronous Request Flow

```
┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│NPC Script│     │C++ Native│     │  MySQL   │     │  Python  │     │AI Service│
└────┬─────┘     └────┬─────┘     └────┬─────┘     └────┬─────┘     └────┬─────┘
     │                │                │                │                │
     │ httpget(url)   │                │                │                │
     │───────────────▶│                │                │                │
     │                │ INSERT request │                │                │
     │                │───────────────▶│                │                │
     │                │   request_id   │                │                │
     │                │◀───────────────│                │                │
     │                │                │                │                │
     │                │    poll loop   │                │                │
     │                │                │  SELECT pending│                │
     │                │                │◀───────────────│                │
     │                │                │   requests     │                │
     │                │                │───────────────▶│                │
     │                │                │                │  process(req)  │
     │                │                │                │───────────────▶│
     │                │                │                │    response    │
     │                │                │                │◀───────────────│
     │                │                │ INSERT response│                │
     │                │                │◀───────────────│                │
     │                │ SELECT response│                │                │
     │                │───────────────▶│                │                │
     │                │  response_data │                │                │
     │                │◀───────────────│                │                │
     │   response$    │                │                │                │
     │◀───────────────│                │                │                │
     │                │                │                │                │
```

### 9.2 Asynchronous Request Flow

```
┌──────────┐     ┌──────────┐     ┌──────────┐
│NPC Script│     │C++ Native│     │  MySQL   │
└────┬─────┘     └────┬─────┘     └────┬─────┘
     │                │                │
     │ai_db_request() │                │
     │───────────────▶│ INSERT request │
     │                │───────────────▶│
     │   request_id   │   request_id   │
     │◀───────────────│◀───────────────│
     │                │                │
     │ (do other work)│                │
     │                │                │
     │ai_db_status(id)│                │
     │───────────────▶│ SELECT status  │
     │                │───────────────▶│
     │    status=0    │    pending     │
     │◀───────────────│◀───────────────│
     │                │                │
     │ (wait/retry)   │                │
     │                │                │
     │ai_db_response()│                │
     │───────────────▶│ SELECT response│
     │                │───────────────▶│
     │   response$    │  response_data │
     │◀───────────────│◀───────────────│
```

---

## 10. Performance Considerations

### 10.1 Latency Analysis

| Component | Expected Latency | Notes |
|-----------|------------------|-------|
| INSERT request | 1-5ms | Single row insert |
| SELECT response | 1-3ms | Indexed lookup |
| Poll interval | 100ms | Configurable |
| AI processing | 10-1000ms | Depends on endpoint |
| **Total round-trip** | **112-1108ms** | Typical case |

### 10.2 Throughput Estimates

- **Requests/second**: ~100-500 (limited by poll interval and batch size)
- **Concurrent scripts**: Unlimited (async pattern)
- **Database connections**: 10-20 (pooled)

### 10.3 Optimization Strategies

1. **Batch polling**: Process multiple requests per poll cycle
2. **Connection pooling**: Reuse database connections
3. **Index optimization**: Composite indexes on status+priority
4. **Response caching**: Cache frequent health checks
5. **Priority queuing**: Process high-priority requests first

---

## 11. Security Considerations

### 11.1 Input Validation

- Endpoint length: Max 255 characters
- Request data: Max 10KB
- No SQL injection: Use prepared statements
- JSON validation: Validate structure before processing

### 11.2 Access Control

- Database user permissions: INSERT, SELECT, UPDATE only
- No DELETE permission for application user
- Cleanup via scheduled procedure with elevated privileges

### 11.3 Data Protection

- No secrets in request/response data
- Sensitive data in environment variables only
- Request expiration prevents data accumulation

---

## 12. Testing Strategy

### 12.1 Unit Tests

```cpp
// Test cases for script_ai_ipc.cpp

// Test 1: Request insertion
test_ai_db_request_basic() {
    // Insert request, verify ID returned > 0
    // Verify database record created
}

// Test 2: Response retrieval
test_ai_db_response_basic() {
    // Insert request, insert response manually
    // Verify ai_db_response returns correct data
}

// Test 3: Timeout handling
test_ai_db_wait_timeout() {
    // Insert request without response
    // Verify ai_db_wait returns empty after timeout
}

// Test 4: Status transitions
test_request_status_flow() {
    // Verify: pending -> processing -> completed
}

// Test 5: Backward compatibility
test_httpget_wrapper() {
    // Verify httpget("url") works same as before
}
```

### 12.2 Integration Tests

```bash
# Test script: tests/test_ai_ipc.txt
# Load in-game and verify all functions work
```

---

## Appendix A: Quick Reference

### Command Signatures

```c
// Core commands
int    ai_db_request(type$, endpoint${, data${, priority}}) // Returns request_id
string ai_db_response(request_id)                           // Returns JSON or ""
string ai_db_wait(request_id, timeout_ms)                   // Blocking wait
int    ai_db_status(request_id)                             // 0-4 status code
int    ai_db_cancel(request_id)                             // 1=success, 0=fail

// Backward-compatible
string httpget(url$)                                        // Same as before
string httppost(url$, data$)                                // Same as before
```

### Status Codes

| Code | Enum | Meaning |
|------|------|---------|
| 0 | AI_REQ_PENDING | Request queued |
| 1 | AI_REQ_PROCESSING | Being processed |
| 2 | AI_REQ_COMPLETED | Response ready |
| 3 | AI_REQ_FAILED | Processing error |
| 4 | AI_REQ_EXPIRED | Timed out |

---

## Appendix B: SQL Installation Script

```sql
-- ai_ipc_tables.sql
-- Run this to create required tables

-- Drop existing tables if upgrading
-- DROP TABLE IF EXISTS ai_responses;
-- DROP TABLE IF EXISTS ai_requests;
-- DROP TABLE IF EXISTS ai_request_log;

-- Create tables (paste from Section 2)
-- ... (full CREATE TABLE statements)

-- Create cleanup procedure (paste from Section 2.5)
-- ... (full procedure definition)

-- Create event scheduler (optional)
-- ... (full event definition)

-- Grant permissions
GRANT SELECT, INSERT, UPDATE ON ragnarok.ai_requests TO 'ragnarok'@'localhost';
GRANT SELECT, INSERT ON ragnarok.ai_responses TO 'ragnarok'@'localhost';
GRANT SELECT, INSERT ON ragnarok.ai_request_log TO 'ragnarok'@'localhost';
```

---

**Document End**

*This architecture document provides a complete design for replacing HTTP-based NPC script commands with native C++ database-based IPC. Implementation should follow the patterns and specifications outlined above.*