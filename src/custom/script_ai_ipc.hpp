// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// AI IPC Script Commands - Header File
// Implements database-based IPC for NPC scripts to communicate with AI services

#ifndef SCRIPT_AI_IPC_HPP
#define SCRIPT_AI_IPC_HPP

#include <string>
#include <cstdint>
#include <unordered_map>

// Forward declarations from rAthena
struct script_state;

/**
 * @file script_ai_ipc.hpp
 * @brief Native C++ script commands for AI IPC communication
 *
 * This module provides script commands that allow NPC scripts to communicate
 * with external AI services through a database-based IPC mechanism:
 * - NPC scripts write requests to the ai_requests table
 * - External AI worker processes read requests and write responses to ai_responses
 * - NPC scripts read responses from ai_responses table
 *
 * This replaces HTTP-based communication for better reliability and performance.
 *
 * IMPORTANT: httpget/httppost use NON-BLOCKING timer-based polling to avoid
 * freezing the server. They use script sleep mechanism to yield control.
 */

namespace ai_ipc {

// ============================================================================
// Status Codes (matching database enum)
// ============================================================================

/**
 * Request status codes matching the database enum values
 */
enum class RequestStatus : int {
    NOT_FOUND   = 0,  // Request does not exist
    PENDING     = 1,  // Request is waiting to be processed
    PROCESSING  = 2,  // Request is currently being processed
    COMPLETED   = 3,  // Request completed successfully
    FAILED      = 4,  // Request failed with an error
    TIMEOUT     = 5,  // Request timed out
    CANCELLED   = 6   // Request was cancelled
};

// ============================================================================
// Wait State for Non-Blocking HTTP Requests
// ============================================================================

/**
 * State tracking for non-blocking HTTP requests
 * Used to store context between timer checks
 */
struct HttpWaitState {
    int64 request_id;        // Database request ID
    unsigned int timeout_tick; // When to give up (gettick() + timeout)
    int st_oid;              // Script state object ID (NPC)
    int st_rid;              // Script state RID (player)
    bool is_post;            // true for POST, false for GET
    std::string result;      // Result to push when done
    bool completed;          // Whether request completed
};

// Wait state database - maps request_id to wait state
extern std::unordered_map<int64, HttpWaitState> http_wait_db;

// ============================================================================
// Configuration Constants
// ============================================================================

// Default expiration time for requests (in seconds)
constexpr int DEFAULT_EXPIRE_SECONDS = 30;

// Maximum timeout for blocking wait (in milliseconds) - DEPRECATED
constexpr int MAX_WAIT_TIMEOUT_MS = 30000;

// Default timeout for HTTP compatibility functions (in milliseconds)
constexpr int HTTP_COMPAT_TIMEOUT_MS = 5000;

// Poll interval for timer-based checking (in milliseconds)
// This is how often the timer checks for responses (non-blocking)
constexpr int POLL_INTERVAL_MS = 100;

// Maximum length for escaped SQL strings
constexpr size_t MAX_ESCAPED_STRING_LEN = 65535;

// ============================================================================
// Helper Function Declarations
// ============================================================================

/**
 * Extract endpoint path from a full URL
 * @param url Full URL (e.g., "http://host:8000/api/v1/health")
 * @return Endpoint path (e.g., "/api/v1/health") or original URL if parsing fails
 */
std::string extract_endpoint_from_url(const char* url);

/**
 * Escape a string for safe SQL insertion using rAthena's SQL library
 * @param str Raw string to escape
 * @return SQL-safe escaped string
 */
std::string sql_escape_string(const char* str);

/**
 * Convert database status string to RequestStatus enum
 * @param status_str Status string from database
 * @return Corresponding RequestStatus enum value
 */
RequestStatus status_string_to_enum(const char* status_str);

/**
 * Get current NPC name from script state if available
 * @param st Script state
 * @return NPC name or "unknown" if not available
 */
const char* get_current_npc_name(struct script_state* st);

/**
 * Get current map name from script state if available
 * @param st Script state
 * @return Map name or "unknown" if not available
 */
const char* get_current_map_name(struct script_state* st);

/**
 * Properly escape a string for JSON inclusion
 * Handles special characters like quotes, backslashes, newlines
 * @param str String to escape
 * @return JSON-safe escaped string
 */
std::string json_escape_string(const std::string& str);

/**
 * Check pending HTTP requests and resume completed scripts
 * Called by timer system
 * @return Number of requests processed
 */
int check_pending_http_requests();

/**
 * Clean up expired HTTP wait states
 */
void cleanup_expired_http_waits();

} // namespace ai_ipc

// Timer function declaration for non-blocking HTTP polling
TIMER_FUNC(ai_ipc_http_poll_timer);

// ============================================================================
// Script Command Declarations (BUILDIN_FUNC implementations)
// ============================================================================

/**
 * Initialize AI IPC script commands
 * Call this during script system initialization
 */
void script_ai_ipc_init(void);

/**
 * Cleanup AI IPC resources
 * Call this during script system finalization
 */
void script_ai_ipc_final(void);

// ============================================================================
// BUILDIN_FUNC declarations - these are implemented in script_ai_ipc.cpp
// and registered in script_def.inc
// ============================================================================

/**
 * ai_db_request(type, endpoint, data{, priority})
 * 
 * Creates a new AI request in the database.
 * 
 * @param type     Request type string (e.g., "dialogue", "decision", "http_get")
 * @param endpoint Target endpoint path (e.g., "/api/v1/npc/dialogue")
 * @param data     Request data as JSON string
 * @param priority Optional priority (1-10, default 5, lower = higher priority)
 * @return Request ID (BIGINT) on success, -1 on error
 * 
 * Script usage:
 *   .@req_id = ai_db_request("dialogue", "/api/v1/npc/dialogue", .@json_data$);
 *   .@req_id = ai_db_request("urgent", "/api/v1/alert", .@json$, 1);
 */
BUILDIN_FUNC(ai_db_request);

/**
 * ai_db_response(request_id)
 * 
 * Retrieves the response for a completed request (non-blocking).
 * 
 * @param request_id The request ID returned by ai_db_request
 * @return Response JSON string if completed, empty string if not ready
 * 
 * Script usage:
 *   .@response$ = ai_db_response(.@req_id);
 *   if (.@response$ != "") {
 *       // Process response
 *   }
 */
BUILDIN_FUNC(ai_db_response);

/**
 * ai_db_wait(request_id, timeout_ms)
 * 
 * Waits for a response with blocking poll (use sparingly!).
 * 
 * @param request_id The request ID to wait for
 * @param timeout_ms Maximum time to wait in milliseconds (capped at 30000)
 * @return Response JSON string if completed, error JSON on timeout
 * 
 * WARNING: This BLOCKS the script execution. Use with caution!
 * Prefer non-blocking ai_db_response with event-based design.
 * 
 * Script usage:
 *   .@response$ = ai_db_wait(.@req_id, 5000);  // Wait up to 5 seconds
 */
BUILDIN_FUNC(ai_db_wait);

/**
 * ai_db_status(request_id)
 * 
 * Gets the current status of a request.
 * 
 * @param request_id The request ID to check
 * @return Status code: 0=not_found, 1=pending, 2=processing, 
 *                      3=completed, 4=failed, 5=timeout, 6=cancelled
 * 
 * Script usage:
 *   .@status = ai_db_status(.@req_id);
 *   if (.@status == 3) {
 *       .@response$ = ai_db_response(.@req_id);
 *   }
 */
BUILDIN_FUNC(ai_db_status);

/**
 * ai_db_cancel(request_id)
 * 
 * Cancels a pending or processing request.
 * 
 * @param request_id The request ID to cancel
 * @return 1 on success, 0 if request already completed/cancelled/not found
 * 
 * Script usage:
 *   if (ai_db_cancel(.@req_id)) {
 *       mes "Request cancelled.";
 *   }
 */
BUILDIN_FUNC(ai_db_cancel);

/**
 * httpget(url)
 *
 * HTTP GET via database IPC with NON-BLOCKING implementation.
 *
 * @param url Full URL to request
 * @return Response data or error JSON
 *
 * Implementation: Uses timer-based polling to avoid blocking the server.
 * The script yields control using sleep mechanism and resumes when
 * response is ready or timeout occurs.
 *
 * Recommended: Use ai_db_request/ai_db_response for more control.
 *
 * Script usage:
 *   .@result$ = httpget("http://ai-server:8000/api/v1/health");
 */
BUILDIN_FUNC(httpget);

/**
 * httppost(url, data)
 *
 * HTTP POST via database IPC with NON-BLOCKING implementation.
 *
 * @param url  Full URL to request
 * @param data POST body data (usually JSON)
 * @return Response data or error JSON
 *
 * Implementation: Uses timer-based polling to avoid blocking the server.
 * The script yields control using sleep mechanism and resumes when
 * response is ready or timeout occurs.
 *
 * Recommended: Use ai_db_request/ai_db_response for more control.
 *
 * Script usage:
 *   .@result$ = httppost("http://ai-server:8000/api/v1/dialogue", .@json$);
 */
BUILDIN_FUNC(httppost);

/**
 * httpget_async(url)
 *
 * Async HTTP GET - returns immediately with request ID.
 * Use ai_db_response() or ai_db_status() to check completion.
 *
 * @param url Full URL to request
 * @return Request ID for tracking
 *
 * Script usage:
 *   .@req_id = httpget_async("http://ai-server:8000/api/v1/health");
 *   sleep2 1000;
 *   .@result$ = ai_db_response(.@req_id);
 */
BUILDIN_FUNC(httpget_async);

/**
 * httppost_async(url, data)
 *
 * Async HTTP POST - returns immediately with request ID.
 * Use ai_db_response() or ai_db_status() to check completion.
 *
 * @param url  Full URL to request
 * @param data POST body data (usually JSON)
 * @return Request ID for tracking
 *
 * Script usage:
 *   .@req_id = httppost_async("http://ai-server:8000/api/v1/dialogue", .@json$);
 *   sleep2 1000;
 *   .@result$ = ai_db_response(.@req_id);
 */
BUILDIN_FUNC(httppost_async);

#endif // SCRIPT_AI_IPC_HPP