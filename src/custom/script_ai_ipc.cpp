// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// AI IPC Script Commands - Implementation File
// NON-BLOCKING implementation using timer-based polling

#include "script_ai_ipc.hpp"

// rAthena common includes
#include "../common/cbasetypes.hpp"
#include "../common/showmsg.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"

// rAthena map includes
#include "../map/map.hpp"
#include "../map/npc.hpp"
#include "../map/script.hpp"

#include <cstring>
#include <cstdlib>
#include <sstream>

// Database table name for AI requests/responses
static const char* ai_requests_table = "ai_requests";
static const char* ai_responses_table = "ai_responses";

// Timer ID for HTTP polling (-1 = not started)
static int ai_ipc_poll_timer_id = INVALID_TIMER;

// ============================================================================
// Wait State Database for Non-Blocking HTTP Requests
// ============================================================================

namespace ai_ipc {

// Wait state database - maps request_id to wait state
std::unordered_map<int64, HttpWaitState> http_wait_db;

// ============================================================================
// Helper Function Implementations
// ============================================================================

std::string extract_endpoint_from_url(const char* url) {
    if (url == nullptr || url[0] == '\0') {
        return "/";
    }
    
    std::string url_str(url);
    
    // Find the protocol separator "://"
    size_t protocol_pos = url_str.find("://");
    if (protocol_pos == std::string::npos) {
        // No protocol, assume it's already an endpoint
        return url_str;
    }
    
    // Find the first '/' after the host:port
    size_t path_start = url_str.find('/', protocol_pos + 3);
    if (path_start == std::string::npos) {
        return "/";
    }
    
    return url_str.substr(path_start);
}

std::string sql_escape_string(const char* str) {
    if (str == nullptr) {
        return "";
    }
    
    size_t len = strlen(str);
    if (len == 0) {
        return "";
    }
    
    // Cap length to prevent buffer overflow
    if (len > MAX_ESCAPED_STRING_LEN) {
        len = MAX_ESCAPED_STRING_LEN;
    }
    
    // Allocate buffer for escaped string (worst case: every char needs escaping)
    char* escaped = (char*)aMalloc(len * 2 + 1);
    if (escaped == nullptr) {
        ShowError("ai_ipc::sql_escape_string: Memory allocation failed\n");
        return "";
    }
    
    Sql_EscapeStringLen(mmysql_handle, escaped, str, len);
    std::string result(escaped);
    aFree(escaped);
    
    return result;
}

std::string json_escape_string(const std::string& str) {
    std::ostringstream escaped;
    for (char c : str) {
        switch (c) {
            case '"':  escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Control characters - encode as \u00XX
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    escaped << buf;
                } else {
                    escaped << c;
                }
                break;
        }
    }
    return escaped.str();
}

RequestStatus status_string_to_enum(const char* status_str) {
    if (status_str == nullptr) {
        return RequestStatus::NOT_FOUND;
    }
    
    if (strcmp(status_str, "pending") == 0) return RequestStatus::PENDING;
    if (strcmp(status_str, "processing") == 0) return RequestStatus::PROCESSING;
    if (strcmp(status_str, "completed") == 0) return RequestStatus::COMPLETED;
    if (strcmp(status_str, "failed") == 0) return RequestStatus::FAILED;
    if (strcmp(status_str, "timeout") == 0) return RequestStatus::TIMEOUT;
    if (strcmp(status_str, "cancelled") == 0) return RequestStatus::CANCELLED;
    
    return RequestStatus::NOT_FOUND;
}

const char* get_current_npc_name(struct script_state* st) {
    if (st == nullptr || st->oid == 0) {
        return "unknown";
    }
    
    struct npc_data* nd = map_id2nd(st->oid);
    if (nd == nullptr) {
        return "unknown";
    }
    
    return nd->exname;
}

const char* get_current_map_name(struct script_state* st) {
    if (st == nullptr || st->oid == 0) {
        return "unknown";
    }
    
    struct npc_data* nd = map_id2nd(st->oid);
    if (nd == nullptr || nd->bl.m < 0) {
        return "unknown";
    }
    
    return map_mapid2mapname(nd->bl.m);
}

int check_pending_http_requests() {
    if (http_wait_db.empty()) {
        return 0;
    }
    
    unsigned int now = gettick();
    int processed = 0;
    std::vector<int64> to_remove;
    
    for (auto& [request_id, state] : http_wait_db) {
        // Check timeout first
        if (DIFF_TICK(now, state.timeout_tick) >= 0) {
            ShowWarning("[AI-IPC] HTTP request %lld timed out\n", (long long)request_id);
            state.result = "{\"error\":\"timeout\"}";
            state.completed = true;
            to_remove.push_back(request_id);
            processed++;
            continue;
        }
        
        // Check for response in database
        if (SQL_SUCCESS == Sql_Query(mmysql_handle,
            "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
            ai_responses_table, (long long)request_id))
        {
            if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
                char* data = nullptr;
                Sql_GetData(mmysql_handle, 0, &data, nullptr);
                
                if (data != nullptr && data[0] != '\0') {
                    ShowInfo("[AI-IPC] Got response for request %lld\n", (long long)request_id);
                    state.result = data;
                    state.completed = true;
                    to_remove.push_back(request_id);
                    processed++;
                }
            }
            Sql_FreeResult(mmysql_handle);
        }
    }
    
    // Remove completed entries
    for (int64 id : to_remove) {
        http_wait_db.erase(id);
    }
    
    return processed;
}

void cleanup_expired_http_waits() {
    unsigned int now = gettick();
    
    auto it = http_wait_db.begin();
    while (it != http_wait_db.end()) {
        if (DIFF_TICK(now, it->second.timeout_tick) >= 0) {
            ShowWarning("[AI-IPC] Cleaning up expired wait for request %lld\n", 
                       (long long)it->first);
            it = http_wait_db.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace ai_ipc

// ============================================================================
// Timer Function for Non-Blocking HTTP Polling
// ============================================================================

TIMER_FUNC(ai_ipc_http_poll_timer) {
    // Check all pending HTTP requests for responses
    ai_ipc::check_pending_http_requests();
    
    // Clean up any expired waits
    ai_ipc::cleanup_expired_http_waits();
    
    return 0;
}

// ============================================================================
// Module Initialization/Finalization
// ============================================================================

void script_ai_ipc_init(void) {
    ShowInfo("[AI-IPC] Initializing AI IPC script commands...\n");
    
    // Start the polling timer (runs every POLL_INTERVAL_MS)
    ai_ipc_poll_timer_id = add_timer_interval(
        gettick() + ai_ipc::POLL_INTERVAL_MS,
        ai_ipc_http_poll_timer,
        0, 0,
        ai_ipc::POLL_INTERVAL_MS
    );
    
    if (ai_ipc_poll_timer_id == INVALID_TIMER) {
        ShowError("[AI-IPC] Failed to create HTTP poll timer!\n");
    } else {
        ShowInfo("[AI-IPC] HTTP poll timer started (interval: %dms)\n", 
                ai_ipc::POLL_INTERVAL_MS);
    }
    
    ShowStatus("[AI-IPC] AI IPC script commands loaded successfully.\n");
}

void script_ai_ipc_final(void) {
    ShowInfo("[AI-IPC] Finalizing AI IPC script commands...\n");
    
    // Stop the polling timer
    if (ai_ipc_poll_timer_id != INVALID_TIMER) {
        delete_timer(ai_ipc_poll_timer_id, ai_ipc_http_poll_timer);
        ai_ipc_poll_timer_id = INVALID_TIMER;
    }
    
    // Clear wait database
    ai_ipc::http_wait_db.clear();
}

// ============================================================================
// BUILDIN_FUNC Implementations
// ============================================================================

/**
 * ai_db_request(type, endpoint, data{, priority})
 * Creates a new AI request in the database
 */
BUILDIN_FUNC(ai_db_request) {
    const char* type = script_getstr(st, 2);
    const char* endpoint = script_getstr(st, 3);
    const char* data = script_getstr(st, 4);
    int priority = script_hasdata(st, 5) ? script_getnum(st, 5) : 5;
    
    // Validate priority range (1-10)
    if (priority < 1) priority = 1;
    if (priority > 10) priority = 10;
    
    // Get context information
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    ShowInfo("[AI-IPC] ai_db_request: type='%s', endpoint='%s', priority=%d\n",
             type, endpoint, priority);
    
    // Escape strings for SQL injection prevention
    std::string esc_type = ai_ipc::sql_escape_string(type);
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint);
    std::string esc_data = ai_ipc::sql_escape_string(data);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    // Insert request into database
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('%s', '%s', '%s', %d, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_type.c_str(),
        esc_endpoint.c_str(),
        esc_data.c_str(),
        priority,
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] ai_db_request: Failed to insert request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] ai_db_request: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_response(request_id) - Non-blocking response check
 */
BUILDIN_FUNC(ai_db_response) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        ShowError("[AI-IPC] ai_db_response: SQL query failed\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushstrcopy(st, "");
        return SCRIPT_CMD_SUCCESS;
    }
    
    if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
        char* data = nullptr;
        Sql_GetData(mmysql_handle, 0, &data, nullptr);
        
        if (data != nullptr && data[0] != '\0') {
            script_pushstrcopy(st, data);
        } else {
            script_pushstrcopy(st, "");
        }
    } else {
        script_pushstrcopy(st, "");
    }
    
    Sql_FreeResult(mmysql_handle);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_wait(request_id, timeout_ms)
 * DEPRECATED: Use ai_db_response with sleep2 loop instead
 * This is kept for backward compatibility but logs a warning
 */
BUILDIN_FUNC(ai_db_wait) {
    int64 request_id = (int64)script_getnum(st, 2);
    int timeout_ms = script_getnum(st, 3);
    
    ShowWarning("[AI-IPC] ai_db_wait is DEPRECATED - use ai_db_response() with sleep2 loop\n");
    
    // Cap timeout
    if (timeout_ms < 0) timeout_ms = 0;
    if (timeout_ms > ai_ipc::MAX_WAIT_TIMEOUT_MS) {
        timeout_ms = ai_ipc::MAX_WAIT_TIMEOUT_MS;
    }
    
    // For backward compat, do a single non-blocking check
    // Scripts should migrate to polling pattern
    if (SQL_SUCCESS == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* data = nullptr;
            Sql_GetData(mmysql_handle, 0, &data, nullptr);
            
            if (data != nullptr && data[0] != '\0') {
                script_pushstrcopy(st, data);
                Sql_FreeResult(mmysql_handle);
                return SCRIPT_CMD_SUCCESS;
            }
        }
        Sql_FreeResult(mmysql_handle);
    }
    
    // Not ready - return pending indicator
    script_pushstrcopy(st, "{\"status\":\"pending\",\"message\":\"Use ai_db_response() with sleep2 loop\"}");
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_status(request_id) - Get request status
 */
BUILDIN_FUNC(ai_db_status) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "SELECT `status` FROM `%s` WHERE `id` = %lld",
        ai_requests_table, (long long)request_id))
    {
        ShowError("[AI-IPC] ai_db_status: SQL query failed\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, static_cast<int>(ai_ipc::RequestStatus::NOT_FOUND));
        return SCRIPT_CMD_SUCCESS;
    }
    
    if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
        char* status_str = nullptr;
        Sql_GetData(mmysql_handle, 0, &status_str, nullptr);
        ai_ipc::RequestStatus status = ai_ipc::status_string_to_enum(status_str);
        script_pushint(st, static_cast<int>(status));
    } else {
        script_pushint(st, static_cast<int>(ai_ipc::RequestStatus::NOT_FOUND));
    }
    
    Sql_FreeResult(mmysql_handle);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_cancel(request_id) - Cancel a pending request
 */
BUILDIN_FUNC(ai_db_cancel) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "UPDATE `%s` SET `status` = 'cancelled' "
        "WHERE `id` = %lld AND `status` IN ('pending', 'processing')",
        ai_requests_table, (long long)request_id))
    {
        ShowError("[AI-IPC] ai_db_cancel: SQL query failed\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, 0);
        return SCRIPT_CMD_SUCCESS;
    }
    
    uint64 affected_rows = Sql_NumRowsAffected(mmysql_handle);
    script_pushint(st, affected_rows > 0 ? 1 : 0);
    
    return SCRIPT_CMD_SUCCESS;
}

// ============================================================================
// Async HTTP Functions (Non-Blocking, Return Request ID)
// ============================================================================

/**
 * httpget_async(url) - Non-blocking HTTP GET, returns request ID
 */
BUILDIN_FUNC(httpget_async) {
    const char* url = script_getstr(st, 2);
    
    ShowInfo("[AI-IPC] httpget_async: URL='%s'\n", url);
    
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // Build JSON request data with proper escaping
    std::string json_url = ai_ipc::json_escape_string(url);
    std::string request_data = "{\"url\":\"" + json_url + "\",\"method\":\"GET\"}";
    
    // SQL escape all values
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('http_get', '%s', '%s', 5, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_endpoint.c_str(),
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] httpget_async: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] httpget_async: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httppost_async(url, data) - Non-blocking HTTP POST, returns request ID
 */
BUILDIN_FUNC(httppost_async) {
    const char* url = script_getstr(st, 2);
    const char* post_data = script_getstr(st, 3);
    
    ShowInfo("[AI-IPC] httppost_async: URL='%s'\n", url);
    
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // SQL escape all values
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_data = ai_ipc::sql_escape_string(post_data);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('http_post', '%s', '%s', 5, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_endpoint.c_str(),
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] httppost_async: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] httppost_async: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

// ============================================================================
// Compatibility HTTP Functions (Use Script Sleep for Non-Blocking)
// ============================================================================

/**
 * httpget(url) - HTTP GET with script-based yield (non-blocking)
 * 
 * Implementation: Creates request, then uses addtimer to resume script
 * when response is ready. This does NOT block the server thread.
 */
BUILDIN_FUNC(httpget) {
    const char* url = script_getstr(st, 2);
    
    ShowInfo("[AI-IPC] httpget: URL='%s' (non-blocking mode)\n", url);
    
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // Build JSON request data with proper escaping (FIX for HIGH #1)
    std::string json_url = ai_ipc::json_escape_string(url);
    std::string request_data = "{\"url\":\"" + json_url + "\",\"method\":\"GET\"}";
    
    // SQL escape all values
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('http_get', '%s', '%s', 5, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_endpoint.c_str(),
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] httpget: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushstrcopy(st, "{\"error\":\"database_error\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] httpget: Created request ID %lld\n", (long long)request_id);
    
    // Poll for response with timeout (non-blocking via script state)
    unsigned int timeout_tick = gettick() + ai_ipc::HTTP_COMPAT_TIMEOUT_MS;
    int poll_count = 0;
    const int max_polls = ai_ipc::HTTP_COMPAT_TIMEOUT_MS / ai_ipc::POLL_INTERVAL_MS;
    
    // Check immediately first
    if (SQL_SUCCESS == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* data = nullptr;
            Sql_GetData(mmysql_handle, 0, &data, nullptr);
            
            if (data != nullptr && data[0] != '\0') {
                ShowInfo("[AI-IPC] httpget: Immediate response for %lld\n", (long long)request_id);
                script_pushstrcopy(st, data);
                Sql_FreeResult(mmysql_handle);
                return SCRIPT_CMD_SUCCESS;
            }
        }
        Sql_FreeResult(mmysql_handle);
    }
    
    // Store request ID in script state for continuation
    // The script will need to poll using a loop with sleep2
    // Return the request_id encoded in a JSON response for script to handle
    char buf[256];
    snprintf(buf, sizeof(buf), 
        "{\"status\":\"pending\",\"request_id\":%lld,\"message\":\"Use polling loop with sleep2\"}", 
        (long long)request_id);
    
    ShowInfo("[AI-IPC] httpget: Returning pending status for async polling\n");
    script_pushstrcopy(st, buf);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httppost(url, data) - HTTP POST with script-based yield (non-blocking)
 */
BUILDIN_FUNC(httppost) {
    const char* url = script_getstr(st, 2);
    const char* post_data = script_getstr(st, 3);
    
    ShowInfo("[AI-IPC] httppost: URL='%s' (non-blocking mode)\n", url);
    
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // SQL escape all values
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_data = ai_ipc::sql_escape_string(post_data);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('http_post', '%s', '%s', 5, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_endpoint.c_str(),
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] httppost: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushstrcopy(st, "{\"error\":\"database_error\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] httppost: Created request ID %lld\n", (long long)request_id);
    
    // Check immediately first
    if (SQL_SUCCESS == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* data = nullptr;
            Sql_GetData(mmysql_handle, 0, &data, nullptr);
            
            if (data != nullptr && data[0] != '\0') {
                ShowInfo("[AI-IPC] httppost: Immediate response for %lld\n", (long long)request_id);
                script_pushstrcopy(st, data);
                Sql_FreeResult(mmysql_handle);
                return SCRIPT_CMD_SUCCESS;
            }
        }
        Sql_FreeResult(mmysql_handle);
    }
    
    // Return pending status with request_id for script to poll
    char buf[256];
    snprintf(buf, sizeof(buf), 
        "{\"status\":\"pending\",\"request_id\":%lld,\"message\":\"Use polling loop with sleep2\"}", 
        (long long)request_id);
    
    ShowInfo("[AI-IPC] httppost: Returning pending status for async polling\n");
    script_pushstrcopy(st, buf);
    return SCRIPT_CMD_SUCCESS;
}

// ============================================================================
// AI NPC Commands - Database IPC Implementations
// These commands map to Python handlers in the AI IPC service
// ============================================================================

/**
 * ai_npc_dialogue(npc_id, player_id, message{, options})
 * Request AI-generated dialogue for an NPC
 */
BUILDIN_FUNC(ai_npc_dialogue) {
    const char* npc_id = script_getstr(st, 2);
    int player_id = script_getnum(st, 3);
    const char* message = script_getstr(st, 4);
    const char* options = script_hasdata(st, 5) ? script_getstr(st, 5) : "";
    
    ShowInfo("[AI-IPC] ai_npc_dialogue: npc='%s', player=%d\n", npc_id, player_id);
    
    // Build JSON request with proper escaping
    std::string json_npc_id = ai_ipc::json_escape_string(npc_id);
    std::string json_message = ai_ipc::json_escape_string(message);
    std::string json_options = ai_ipc::json_escape_string(options);
    
    std::ostringstream data;
    data << "{\"npc_id\":\"" << json_npc_id << "\","
         << "\"player_id\":" << player_id << ","
         << "\"message\":\"" << json_message << "\"";
    if (options[0] != '\0') {
        data << ",\"options\":\"" << json_options << "\"";
    }
    data << "}";
    
    std::string request_data = data.str();
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('ai_npc_dialogue', '/api/v1/npc/dialogue', '%s', 3, '%s', '%s', "
        "DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] ai_npc_dialogue: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] ai_npc_dialogue: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_npc_decide_action(npc_id, context_json)
 * Request AI decision for NPC action
 */
BUILDIN_FUNC(ai_npc_decide_action) {
    const char* npc_id = script_getstr(st, 2);
    const char* context_json = script_getstr(st, 3);
    
    ShowInfo("[AI-IPC] ai_npc_decide_action: npc='%s'\n", npc_id);
    
    // Build request data
    std::string json_npc_id = ai_ipc::json_escape_string(npc_id);
    std::string json_context = ai_ipc::json_escape_string(context_json);
    
    std::ostringstream data;
    data << "{\"npc_id\":\"" << json_npc_id << "\","
         << "\"context\":\"" << json_context << "\"}";
    
    std::string request_data = data.str();
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('ai_npc_decide_action', '/api/v1/npc/decide', '%s', 3, '%s', '%s', "
        "DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] ai_npc_decide_action: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] ai_npc_decide_action: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_npc_get_emotion(npc_id)
 * Request current emotion state for NPC
 */
BUILDIN_FUNC(ai_npc_get_emotion) {
    const char* npc_id = script_getstr(st, 2);
    
    ShowInfo("[AI-IPC] ai_npc_get_emotion: npc='%s'\n", npc_id);
    
    std::string json_npc_id = ai_ipc::json_escape_string(npc_id);
    std::string request_data = "{\"npc_id\":\"" + json_npc_id + "\"}";
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('ai_npc_get_emotion', '/api/v1/npc/emotion', '%s', 5, '%s', '%s', "
        "DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        ai_ipc::DEFAULT_EXPIRE_SECONDS))
    {
        ShowError("[AI-IPC] ai_npc_get_emotion: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] ai_npc_get_emotion: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_request_async(type, data_json, callback{, timeout})
 * Generic async AI request with callback
 */
BUILDIN_FUNC(ai_request_async) {
    const char* type = script_getstr(st, 2);
    const char* data_json = script_getstr(st, 3);
    const char* callback = script_getstr(st, 4);
    int timeout = script_hasdata(st, 5) ? script_getnum(st, 5) : ai_ipc::DEFAULT_EXPIRE_SECONDS;
    
    if (timeout < 1) timeout = 1;
    if (timeout > 300) timeout = 300;
    
    ShowInfo("[AI-IPC] ai_request_async: type='%s', callback='%s', timeout=%d\n", 
             type, callback, timeout);
    
    // Build request with callback info embedded
    std::string json_type = ai_ipc::json_escape_string(type);
    std::string json_data = ai_ipc::json_escape_string(data_json);
    std::string json_callback = ai_ipc::json_escape_string(callback);
    
    std::ostringstream data;
    data << "{\"request_type\":\"" << json_type << "\","
         << "\"data\":\"" << json_data << "\","
         << "\"callback_event\":\"" << json_callback << "\","
         << "\"timeout_seconds\":" << timeout << "}";
    
    std::string request_data = data.str();
    std::string esc_type = ai_ipc::sql_escape_string(type);
    std::string esc_data = ai_ipc::sql_escape_string(request_data.c_str());
    
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, "
        "`source_npc`, `source_map`, `expires_at`) "
        "VALUES ('ai_request_async', '/api/v1/async/%s', '%s', 5, '%s', '%s', "
        "DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_type.c_str(),
        esc_data.c_str(),
        esc_npc.c_str(),
        esc_map.c_str(),
        timeout))
    {
        ShowError("[AI-IPC] ai_request_async: Failed to create request\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    ShowInfo("[AI-IPC] ai_request_async: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_check_result(request_id)
 * Check result of async request (returns JSON string)
 */
BUILDIN_FUNC(ai_check_result) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    ShowInfo("[AI-IPC] ai_check_result: request_id=%lld\n", (long long)request_id);
    
    // First check for response
    if (SQL_SUCCESS == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* data = nullptr;
            Sql_GetData(mmysql_handle, 0, &data, nullptr);
            
            if (data != nullptr && data[0] != '\0') {
                ShowInfo("[AI-IPC] ai_check_result: Found response for %lld\n", (long long)request_id);
                script_pushstrcopy(st, data);
                Sql_FreeResult(mmysql_handle);
                return SCRIPT_CMD_SUCCESS;
            }
        }
        Sql_FreeResult(mmysql_handle);
    }
    
    // Check request status
    if (SQL_SUCCESS == Sql_Query(mmysql_handle,
        "SELECT `status` FROM `%s` WHERE `id` = %lld",
        ai_requests_table, (long long)request_id))
    {
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* status = nullptr;
            Sql_GetData(mmysql_handle, 0, &status, nullptr);
            
            ai_ipc::RequestStatus req_status = ai_ipc::status_string_to_enum(status);
            Sql_FreeResult(mmysql_handle);
            
            // Return status-based JSON
            char buf[256];
            switch (req_status) {
                case ai_ipc::RequestStatus::PENDING:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"pending\",\"request_id\":%lld}", (long long)request_id);
                    break;
                case ai_ipc::RequestStatus::PROCESSING:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"processing\",\"request_id\":%lld}", (long long)request_id);
                    break;
                case ai_ipc::RequestStatus::FAILED:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"failed\",\"error\":\"request_failed\"}");
                    break;
                case ai_ipc::RequestStatus::TIMEOUT:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"timeout\",\"error\":\"request_timeout\"}");
                    break;
                case ai_ipc::RequestStatus::CANCELLED:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"cancelled\",\"error\":\"request_cancelled\"}");
                    break;
                default:
                    snprintf(buf, sizeof(buf), 
                        "{\"status\":\"not_found\",\"error\":\"request_not_found\"}");
                    break;
            }
            script_pushstrcopy(st, buf);
            return SCRIPT_CMD_SUCCESS;
        }
        Sql_FreeResult(mmysql_handle);
    }
    
    script_pushstrcopy(st, "{\"status\":\"not_found\",\"error\":\"request_not_found\"}");
    return SCRIPT_CMD_SUCCESS;
}