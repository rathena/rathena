# AI NPC Implementation Guide

**Version:** 1.0  
**Last Updated:** 2026-01-20  
**Author:** AI-MMORPG-World Team

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Available Script Commands](#available-script-commands)
4. [Implementation Patterns](#implementation-patterns)
5. [Common Pitfalls](#common-pitfalls)
6. [Best Practices](#best-practices)
7. [Testing & Debugging](#testing--debugging)
8. [Examples](#examples)
9. [Troubleshooting](#troubleshooting)

---

## Overview

This guide explains how to create AI-powered NPCs in rAthena using the **Database IPC** integration approach. The system enables NPCs to communicate with external AI services through a MySQL-based message queue, providing:

- **Non-blocking AI requests** - Server never freezes waiting for AI
- **Persistent requests** - Survive server restarts
- **Priority queue** - Urgent requests processed first
- **Full audit trail** - All requests logged in database
- **Graceful degradation** - Works even if AI service is offline

### What Makes This Different

**Traditional AI NPCs (Don't Use):**
- ❌ Blocking HTTP calls that freeze the server
- ❌ No persistence across restarts
- ❌ No error handling
- ❌ Outdated command patterns

**Database IPC AI NPCs (Use This):**
- ✅ Non-blocking with sleep2 polling
- ✅ Request/response persistence
- ✅ Comprehensive error handling
- ✅ Production-ready implementation

---

## Architecture

### System Flow

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│  NPC Script │ ───▶ │   C++ Map    │ ───▶ │   MySQL     │
│  (Player    │      │   Server     │      │  Database   │
│  Interacts) │      │              │      │  (IPC Queue)│
└─────────────┘      └──────────────┘      └─────────────┘
                                                   │
                                                   ▼
                                            ┌─────────────┐
                                            │   Python    │
                                            │   Worker    │
                                            │   Service   │
                                            └─────────────┘
                                                   │
                                                   ▼
                                            ┌─────────────┐
                                            │   FastAPI   │
                                            │   AI        │
                                            │   Service   │
                                            └─────────────┘
                                                   │
                                                   ▼
                                            ┌─────────────┐
                                            │   21 AI     │
                                            │   Agents    │
                                            │   (CrewAI)  │
                                            └─────────────┘
```

### Database Tables

**ai_requests** - Request queue
```sql
CREATE TABLE `ai_requests` (
  `id` BIGINT AUTO_INCREMENT PRIMARY KEY,
  `request_type` VARCHAR(50),
  `endpoint` VARCHAR(255),
  `request_data` TEXT,
  `priority` INT DEFAULT 5,
  `status` ENUM('pending','processing','completed','failed','timeout','cancelled'),
  `source_npc` VARCHAR(50),
  `source_map` VARCHAR(20),
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  `expires_at` TIMESTAMP
);
```

**ai_responses** - Response storage
```sql
CREATE TABLE `ai_responses` (
  `id` BIGINT AUTO_INCREMENT PRIMARY KEY,
  `request_id` BIGINT,
  `response_data` TEXT,
  `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (request_id) REFERENCES ai_requests(id)
);
```

---

## Available Script Commands

### Core Commands (src/custom/script_ai_ipc.cpp)

#### `ai_db_request(type, endpoint, data, priority)`

Creates a new AI request in the database queue.

**Parameters:**
- `type` (string) - Request type identifier
- `endpoint` (string) - API endpoint path
- `data` (string) - JSON request data
- `priority` (int, optional) - Priority 1-10 (default: 5, lower=higher priority)

**Returns:** Request ID (int) or -1 on error

**Example:**
```c
.@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@json_data$, 5);
if (.@req_id <= 0) {
    mes "Error: Could not create request";
    close;
}
```

---

#### `ai_db_response(request_id)`

Retrieves the response for a completed request (non-blocking).

**Parameters:**
- `request_id` (int) - Request ID from ai_db_request

**Returns:** Response JSON string or empty string if not ready

**Example:**
```c
.@response$ = ai_db_response(.@req_id);
if (.@response$ != "") {
    // Got response
    mes .@response$;
} else {
    // Still pending
    mes "Waiting for AI...";
}
```

---

#### `ai_db_status(request_id)`

Gets the current status of a request.

**Parameters:**
- `request_id` (int) - Request ID to check

**Returns:** Status code (int)
- 0 = NOT_FOUND
- 1 = PENDING
- 2 = PROCESSING
- 3 = COMPLETED
- 4 = FAILED
- 5 = TIMEOUT
- 6 = CANCELLED

**Example:**
```c
.@status = ai_db_status(.@req_id);
if (.@status == 3) {
    .@response$ = ai_db_response(.@req_id);
    // Process response
} else if (.@status == 4) {
    mes "AI request failed";
}
```

---

#### `ai_db_cancel(request_id)`

Cancels a pending or processing request.

**Parameters:**
- `request_id` (int) - Request ID to cancel

**Returns:** 1 on success, 0 if already completed/not found

**Example:**
```c
if (ai_db_cancel(.@req_id)) {
    mes "Request cancelled";
}
```

---

### Movement Commands (src/custom/script_http.cpp)

#### `npcwalk(npc_name, x, y)`

Makes an NPC walk to specified coordinates.

**Parameters:**
- `npc_name` (string) - NPC name
- `x` (int) - Target X coordinate
- `y` (int) - Target Y coordinate

**Returns:** 1 on success, 0 on failure

**Example:**
```c
if (npcwalk("MyNPC", 150, 200)) {
    npctalk "Moving to destination!";
} else {
    npctalk "Cannot reach that location";
}
```

---

## Implementation Patterns

### Pattern 1: Basic AI Request with Polling

**Correct Pattern:**
```c
// 1. Create request
.@data$ = "{\"message\":\"" + .@player_msg$ + "\"}";
.@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@data$, 5);

if (.@req_id <= 0) {
    // Fallback behavior
    mes "AI unavailable - using simple response";
    close;
}

// 2. Poll for response (non-blocking)
.@attempts = 0;
.@max_attempts = 50;  // 5 seconds max
.@response$ = "";

while (.@attempts < .@max_attempts && .@response$ == "") {
    sleep2 100;  // Wait 100ms
    .@response$ = ai_db_response(.@req_id);
    .@attempts++;
}

// 3. Handle result
if (.@response$ != "") {
    mes "[AI Response]";
    mes .@response$;
} else {
    // Timeout fallback
    mes "AI is thinking... please try again later";
}
```

**Why This Works:**
- ✅ Non-blocking (uses sleep2)
- ✅ Has timeout
- ✅ Provides fallback
- ✅ Good user experience

---

### Pattern 2: Status-Based Decision Making

```c
.@req_id = ai_db_request("decision", "/api/v1/npc/decide", .@context$, 5);

if (.@req_id <= 0) {
    // Immediate fallback
    callsub S_SimpleFallback;
    end;
}

// Wait with status checks
.@attempts = 0;

while (.@attempts < 30) {
    sleep2 100;
    
    .@status = ai_db_status(.@req_id);
    
    if (.@status == 3) {
        // Completed
        .@response$ = ai_db_response(.@req_id);
        break;
    } else if (.@status == 4) {
        // Failed
        callsub S_ErrorFallback;
        end;
    }
    
    .@attempts++;
}

// Process result
if (.@status == 3) {
    callsub S_ProcessDecision, .@response$;
} else {
    callsub S_TimeoutFallback;
}
```

---

### Pattern 3: Multiple Concurrent Requests

```c
// Create multiple requests
setarray .@req_ids[0], 0, 0, 0;

.@req_ids[0] = ai_db_request("quest", "/api/v1/quest/generate", .@data1$, 5);
.@req_ids[1] = ai_db_request("lore", "/api/v1/lore/generate", .@data2$, 3);
.@req_ids[2] = ai_db_request("price", "/api/v1/economy/price", .@data3$, 1);

// Wait for all to complete
.@max_wait = 50;
.@attempts = 0;
.@completed = 0;

while (.@attempts < .@max_wait && .@completed < 3) {
    sleep2 100;
    .@completed = 0;
    
    for (.@i = 0; .@i < 3; .@i++) {
        if (.@req_ids[.@i] > 0) {
            .@status = ai_db_status(.@req_ids[.@i]);
            if (.@status >= 3) {  // Completed, failed, or timeout
                .@completed++;
            }
        }
    }
    
    .@attempts++;
}

// Collect results
for (.@i = 0; .@i < 3; .@i++) {
    if (.@req_ids[.@i] > 0) {
        .@responses$[.@i] = ai_db_response(.@req_ids[.@i]);
    }
}
```

---

## Common Pitfalls

### ❌ Pitfall 1: Blocking Wait (DEPRECATED)

**Don't Do This:**
```c
// ai_db_wait is DEPRECATED and doesn't actually block!
.@response$ = ai_db_wait(.@req_id, 5000);
```

**Why It's Wrong:**
- ai_db_wait is deprecated
- Returns immediately with pending JSON
- Was never truly blocking

**Do This Instead:**
```c
// Use polling pattern
.@attempts = 0;
.@response$ = "";

while (.@attempts < 50 && .@response$ == "") {
    sleep2 100;
    .@response$ = ai_db_response(.@req_id);
    .@attempts++;
}
```

---

### ❌ Pitfall 2: No Fallback Behavior

**Don't Do This:**
```c
.@req_id = ai_db_request("dialogue", "/api/dialogue", .@data$);
// Assume it will work...
.@response$ = ai_db_response(.@req_id);
mes .@response$;  // May show empty string!
```

**Do This Instead:**
```c
.@req_id = ai_db_request("dialogue", "/api/dialogue", .@data$);

if (.@req_id <= 0) {
    mes "Sorry, I cannot process that right now.";
    close;
}

// Poll with timeout...
if (.@response$ != "") {
    mes .@response$;
} else {
    mes "I'm still thinking... please try again.";
}
```

---

### ❌ Pitfall 3: Using Non-Existent Commands

**Don't Do This:**
```c
// These commands DON'T EXIST!
.@resp$ = ai_dialogue(.@npc_id, .@message$);
.@resp$ = ai_decision(.@npc_id, .@context$);
.@resp$ = ai_remember(.@npc_id, .@memory$);
```

**Use These Instead:**
```c
// Correct: Use ai_db_request
.@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@data$);
.@response$ = ai_db_response(.@req_id);
```

---

### ❌ Pitfall 4: Incorrect JSON Escaping

**Don't Do This:**
```c
// Player input might contain quotes!
.@data$ = "{\"message\":\"" + .@player_msg$ + "\"}";
// If player_msg$ = 'I said "hello"', JSON breaks!
```

**Do This Instead:**
```c
// Escape special characters manually
// Or use simple content that doesn't need escaping
.@safe_msg$ = .@player_msg$;  // For now, rAthena doesn't have JSON escape
// Keep player input simple or validate it

.@data$ = "{";
.@data$ += "\"npc_id\":\"MyNPC\",";
.@data$ += "\"player_name\":\"" + strcharinfo(0) + "\",";
.@data$ += "\"simple_field\":true";
.@data$ += "}";
```

**Note:** C++ side (script_ai_ipc.cpp) handles SQL escaping automatically.

---

## Best Practices

### 1. Always Provide Fallback Behavior

```c
S_Dialogue:
    .@req_id = ai_db_request("dialogue", "/api/dialogue", .@data$, 5);
    
    if (.@req_id <= 0) {
        // FALLBACK: Simple canned response
        mes "[NPC]";
        mes "I understand. Let me think about that.";
        return;
    }
    
    // Poll for response...
    
    if (.@response$ != "") {
        // SUCCESS: AI response
        mes "[AI NPC]";
        mes .@response$;
    } else {
        // FALLBACK: Timeout
        mes "[NPC]";
        mes "I'm processing your request. Please check back soon.";
    }
    return;
```

---

### 2. Use Appropriate Timeouts

```c
// Quick responses (health checks, status): 1-2 seconds
.@max_attempts = 10;  // 1 second

// Normal responses (dialogue, decisions): 3-5 seconds
.@max_attempts = 30;  // 3 seconds
.@max_attempts = 50;  // 5 seconds

// Complex operations (quest generation, world state): 10+ seconds
.@max_attempts = 100;  // 10 seconds
```

---

### 3. Use Priority Correctly

```c
// System critical (10): Server status, emergency shutdown
.@req_id = ai_db_request("system", "/admin/shutdown", .@data$, 10);

// High priority (8): Combat decisions, urgent NPC actions
.@req_id = ai_db_request("combat", "/npc/combat_decision", .@data$, 8);

// Normal priority (5): Dialogue, standard decisions
.@req_id = ai_db_request("dialogue", "/npc/dialogue", .@data$, 5);

// Low priority (3): Background tasks, lore generation
.@req_id = ai_db_request("lore", "/world/lore", .@data$, 3);

// Very low priority (1): Analytics, non-urgent logging
.@req_id = ai_db_request("analytics", "/log/player_action", .@data$, 1);
```

---

### 4. Implement User-Friendly Error Messages

```c
.@status = ai_db_status(.@req_id);

switch (.@status) {
    case 0:  // NOT_FOUND
        mes "^FF0000Error: Request not found.^000000";
        break;
    case 1:  // PENDING
        mes "^FFAA00Your request is queued...^000000";
        break;
    case 2:  // PROCESSING
        mes "^00AA00Processing your request...^000000";
        break;
    case 3:  // COMPLETED
        mes "^00FF00Success!^000000";
        break;
    case 4:  // FAILED
        mes "^FF0000Request failed. Please try again.^000000";
        break;
    case 5:  // TIMEOUT
        mes "^FF8800Request timed out.^000000";
        break;
    case 6:  // CANCELLED
        mes "^777777Request was cancelled.^000000";
        break;
}
```

---

### 5. Log Debug Information

```c
// Use debugmes for server-side logging
debugmes "[MyNPC] Created AI request ID: " + .@req_id;
debugmes "[MyNPC] Request type: dialogue, priority: 5";

// Use npctalk for in-game visibility (testing)
npctalk "AI request created: " + .@req_id;

// Use ShowInfo in C++ for detailed logging (already implemented)
```

---

## Testing & Debugging

### Testing Checklist

✅ **Script Syntax**
```bash
# Check for syntax errors before loading
cd /path/to/rathena
./map-server --script-check
```

✅ **NPC Visibility**
```c
// Verify NPC appears in-game
// Check coordinates are valid
// Ensure sprite ID exists
```

✅ **Command Availability**
```c
// Test each command returns expected type
.@test = ai_db_request("test", "/test", "{}", 1);
debugmes "Request ID type test: " + .@test;
```

✅ **Error Handling**
```c
// Test with AI service offline
// Test with invalid data
// Test with timeout scenarios
```

✅ **Performance**
```c
// Measure response times
.@start = gettimetick(0);
// ... do AI request ...
.@elapsed = gettimetick(0) - .@start;
debugmes "AI request took: " + .@elapsed + "μs";
```

---

### Debugging Commands

**Check Database Requests:**
```sql
-- View recent requests
SELECT * FROM ai_requests ORDER BY created_at DESC LIMIT 10;

-- Check pending requests
SELECT * FROM ai_requests WHERE status = 'pending';

-- View responses
SELECT r.id, r.request_type, r.status, res.response_data
FROM ai_requests r
LEFT JOIN ai_responses res ON r.id = res.request_id
ORDER BY r.created_at DESC LIMIT 10;
```

**Check Services:**
```bash
# Check worker service
systemctl status ai-worker
journalctl -u ai-worker -f

# Check AI service
systemctl status ai-service
journalctl -u ai-service -f

# Check map-server
tail -f log/map-server.log | grep "AI-IPC"
```

---

## Examples

### Example 1: Simple AI Dialogue NPC

```c
prontera,150,150,5  script  Simple AI NPC  4_M_SAGE_C,{
    
    mes "[Simple AI]";
    mes "Hello! I use AI to respond.";
    next;
    
    mes "[Simple AI]";
    mes "What would you like to say?";
    input .@msg$;
    
    if (.@msg$ == "") {
        mes "You didn't say anything!";
        close;
    }
    
    // Build request
    .@data$ = "{\"message\":\"" + .@msg$ + "\"}";
    .@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@data$, 5);
    
    if (.@req_id <= 0) {
        mes "AI is offline. I heard: " + .@msg$;
        close;
    }
    
    mes "Thinking...";
    
    // Poll
    .@attempts = 0;
    .@response$ = "";
    
    while (.@attempts < 30 && .@response$ == "") {
        sleep2 100;
        .@response$ = ai_db_response(.@req_id);
        .@attempts++;
    }
    
    next;
    if (.@response$ != "") {
        mes "[AI]";
        mes .@response$;
    } else {
        mes "Still thinking... try again later!";
    }
    
    close;
}
```

---

### Example 2: AI-Driven Movement NPC

```c
prontera,155,155,5  script  Patrol Guard  4_M_JOB_KNIGHT1,{
    
    mes "[Patrol Guard]";
    mes "I patrol this area using AI!";
    next;
    
    if (select("Watch me patrol", "Talk instead") == 2) {
        mes "Protecting the city!";
        close;
    }
    
    mes "Starting patrol...";
    close2;
    
    // Patrol pattern
    npcwalk "Patrol Guard", 160, 155;
    sleep2 2000;
    npcwalk "Patrol Guard", 160, 160;
    sleep2 2000;
    npcwalk "Patrol Guard", 155, 160;
    sleep2 2000;
    npcwalk "Patrol Guard", 155, 155;
    
    npctalk "Patrol complete!";
    end;
}
```

---

## Troubleshooting

### Problem: "ai_db_request: Failed to insert request"

**Cause:** Database connection issue or table doesn't exist

**Solution:**
```sql
-- Check table exists
SHOW TABLES LIKE 'ai_requests';

-- If not, create it
SOURCE sql-files/ai_ipc_schema.sql;

-- Check permissions
SHOW GRANTS FOR 'ragnarok'@'localhost';
```

---

### Problem: "AI service timeout"

**Cause:** Worker service not running or AI service offline

**Solution:**
```bash
# Check worker
systemctl status ai-worker

# Start if needed
systemctl start ai-worker

# Check AI service
curl http://localhost:8000/api/v1/health

# Review logs
journalctl -u ai-worker -n 50
```

---

### Problem: Response always empty

**Cause:** Worker not processing or slow AI service

**Solution:**
1. Check worker is polling database
2. Verify AI service is reachable
3. Check request priority (higher = processed first)
4. Increase timeout in NPC script

---

### Problem: NPC movement doesn't work

**Cause:** NPC not properly initialized or path blocked

**Solution:**
```c
// Check NPC exists
.@nd = npc_name2id("MyNPC");
if (.@nd == 0) {
    debugmes "NPC not found!";
}

// Verify coordinates are walkable
// Use @go in-game to test coordinates
```

---

## Additional Resources

- **Source Code:** `/src/custom/script_ai_ipc.cpp`
- **Header File:** `/src/custom/script_ai_ipc.hpp`
- **Example NPC:** `/npc/custom/KicapMasin.txt`
- **Test NPC:** `/npc/custom/ai_integration_test.txt`
- **Database Schema:** `/sql-files/ai_ipc_schema.sql`

---

## Conclusion

The Database IPC approach provides a robust, production-ready method for integrating AI into rAthena NPCs. By following the patterns and best practices in this guide, you can create sophisticated AI-driven NPCs that enhance player experience without compromising server stability.

**Key Takeaways:**
1. Always use polling pattern (not deprecated ai_db_wait)
2. Provide fallback behavior for offline scenarios
3. Use appropriate timeouts and priorities
4. Test thoroughly with diagnostics NPC
5. Monitor logs and database for issues

Happy NPC scripting! 🎮🤖
