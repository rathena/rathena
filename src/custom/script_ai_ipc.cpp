// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
// AI IPC Script Commands - Implementation File

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
#include <thread>
#include <chrono>

// Database table name for AI requests/responses
static const char* ai_requests_table = "ai_requests";
static const char* ai_responses_table = "ai_responses";

// ============================================================================
// Helper Function Implementations
// ============================================================================

namespace ai_ipc {

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

} // namespace ai_ipc

// ============================================================================
// Module Initialization/Finalization
// ============================================================================

void script_ai_ipc_init(void) {
    ShowInfo("[AI-IPC] Initializing AI IPC script commands...\n");
    ShowStatus("[AI-IPC] AI IPC script commands loaded successfully.\n");
}

void script_ai_ipc_final(void) {
    ShowInfo("[AI-IPC] Finalizing AI IPC script commands...\n");
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
    
    ShowInfo("[AI-IPC] ai_db_request: type='%s', endpoint='%s', priority=%d, npc='%s', map='%s'\n",
             type, endpoint, priority, source_npc, source_map);
    
    // Escape strings for SQL injection prevention
    std::string esc_type = ai_ipc::sql_escape_string(type);
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint);
    std::string esc_data = ai_ipc::sql_escape_string(data);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    // Insert request into database
    // expires_at is set to NOW() + 30 seconds
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, `source_npc`, `source_map`, `expires_at`) "
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
        ShowError("[AI-IPC] ai_db_request: Failed to insert request into database\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, -1);
        return SCRIPT_CMD_SUCCESS;
    }
    
    // Get the auto-generated request ID
    int64 request_id = (int64)Sql_LastInsertId(mmysql_handle);
    
    ShowInfo("[AI-IPC] ai_db_request: Created request ID %lld\n", (long long)request_id);
    
    script_pushint(st, (int)request_id);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_response(request_id)
 * Retrieves the response for a completed request (non-blocking)
 */
BUILDIN_FUNC(ai_db_response) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    ShowInfo("[AI-IPC] ai_db_response: Checking response for request ID %lld\n", (long long)request_id);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
        ai_responses_table, (long long)request_id))
    {
        ShowError("[AI-IPC] ai_db_response: SQL query failed for request ID %lld\n", (long long)request_id);
        Sql_ShowDebug(mmysql_handle);
        script_pushstrcopy(st, "");
        return SCRIPT_CMD_SUCCESS;
    }
    
    if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
        char* data = nullptr;
        Sql_GetData(mmysql_handle, 0, &data, nullptr);
        
        if (data != nullptr && data[0] != '\0') {
            ShowInfo("[AI-IPC] ai_db_response: Found response for request ID %lld\n", (long long)request_id);
            script_pushstrcopy(st, data);
        } else {
            script_pushstrcopy(st, "");
        }
    } else {
        ShowInfo("[AI-IPC] ai_db_response: No response yet for request ID %lld\n", (long long)request_id);
        script_pushstrcopy(st, "");
    }
    
    Sql_FreeResult(mmysql_handle);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_wait(request_id, timeout_ms)
 * Waits for a response with blocking poll
 */
BUILDIN_FUNC(ai_db_wait) {
    int64 request_id = (int64)script_getnum(st, 2);
    int timeout_ms = script_getnum(st, 3);
    
    // Cap timeout to prevent indefinite blocking
    if (timeout_ms < 0) timeout_ms = 0;
    if (timeout_ms > ai_ipc::MAX_WAIT_TIMEOUT_MS) {
        ShowWarning("[AI-IPC] ai_db_wait: Timeout %d exceeds maximum %d, capping\n",
                    timeout_ms, ai_ipc::MAX_WAIT_TIMEOUT_MS);
        timeout_ms = ai_ipc::MAX_WAIT_TIMEOUT_MS;
    }
    
    ShowInfo("[AI-IPC] ai_db_wait: Waiting for request ID %lld with timeout %dms\n",
             (long long)request_id, timeout_ms);
    
    int elapsed_ms = 0;
    
    while (elapsed_ms < timeout_ms) {
        // Check for response
        if (SQL_ERROR == Sql_Query(mmysql_handle,
            "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
            ai_responses_table, (long long)request_id))
        {
            ShowError("[AI-IPC] ai_db_wait: SQL query failed\n");
            Sql_ShowDebug(mmysql_handle);
            script_pushstrcopy(st, "{\"error\":\"database_error\"}");
            return SCRIPT_CMD_SUCCESS;
        }
        
        if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
            char* data = nullptr;
            Sql_GetData(mmysql_handle, 0, &data, nullptr);
            
            if (data != nullptr && data[0] != '\0') {
                ShowInfo("[AI-IPC] ai_db_wait: Got response for request ID %lld after %dms\n",
                         (long long)request_id, elapsed_ms);
                script_pushstrcopy(st, data);
                Sql_FreeResult(mmysql_handle);
                return SCRIPT_CMD_SUCCESS;
            }
        }
        Sql_FreeResult(mmysql_handle);
        
        // Also check if the request status indicates failure/timeout/cancelled
        if (SQL_SUCCESS == Sql_Query(mmysql_handle,
            "SELECT `status` FROM `%s` WHERE `id` = %lld",
            ai_requests_table, (long long)request_id))
        {
            if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
                char* status = nullptr;
                Sql_GetData(mmysql_handle, 0, &status, nullptr);
                
                if (status != nullptr) {
                    ai_ipc::RequestStatus req_status = ai_ipc::status_string_to_enum(status);
                    if (req_status == ai_ipc::RequestStatus::FAILED) {
                        Sql_FreeResult(mmysql_handle);
                        script_pushstrcopy(st, "{\"error\":\"request_failed\"}");
                        return SCRIPT_CMD_SUCCESS;
                    }
                    if (req_status == ai_ipc::RequestStatus::TIMEOUT) {
                        Sql_FreeResult(mmysql_handle);
                        script_pushstrcopy(st, "{\"error\":\"request_timeout\"}");
                        return SCRIPT_CMD_SUCCESS;
                    }
                    if (req_status == ai_ipc::RequestStatus::CANCELLED) {
                        Sql_FreeResult(mmysql_handle);
                        script_pushstrcopy(st, "{\"error\":\"request_cancelled\"}");
                        return SCRIPT_CMD_SUCCESS;
                    }
                }
            }
            Sql_FreeResult(mmysql_handle);
        }
        
        // Sleep before next poll
        std::this_thread::sleep_for(std::chrono::milliseconds(ai_ipc::POLL_INTERVAL_MS));
        elapsed_ms += ai_ipc::POLL_INTERVAL_MS;
    }
    
    ShowWarning("[AI-IPC] ai_db_wait: Timeout waiting for request ID %lld\n", (long long)request_id);
    script_pushstrcopy(st, "{\"error\":\"timeout\"}");
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_status(request_id)
 * Gets the current status of a request
 */
BUILDIN_FUNC(ai_db_status) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    ShowInfo("[AI-IPC] ai_db_status: Checking status for request ID %lld\n", (long long)request_id);
    
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
        ShowInfo("[AI-IPC] ai_db_status: Request %lld status = %d (%s)\n",
                 (long long)request_id, static_cast<int>(status), status_str ? status_str : "null");
        
        script_pushint(st, static_cast<int>(status));
    } else {
        ShowInfo("[AI-IPC] ai_db_status: Request %lld not found\n", (long long)request_id);
        script_pushint(st, static_cast<int>(ai_ipc::RequestStatus::NOT_FOUND));
    }
    
    Sql_FreeResult(mmysql_handle);
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_db_cancel(request_id)
 * Cancels a pending or processing request
 */
BUILDIN_FUNC(ai_db_cancel) {
    int64 request_id = (int64)script_getnum(st, 2);
    
    ShowInfo("[AI-IPC] ai_db_cancel: Cancelling request ID %lld\n", (long long)request_id);
    
    // Only cancel if status is 'pending' or 'processing'
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "UPDATE `%s` SET `status` = 'cancelled' WHERE `id` = %lld AND `status` IN ('pending', 'processing')",
        ai_requests_table, (long long)request_id))
    {
        ShowError("[AI-IPC] ai_db_cancel: SQL query failed\n");
        Sql_ShowDebug(mmysql_handle);
        script_pushint(st, 0);
        return SCRIPT_CMD_SUCCESS;
    }
    
    // Check if any rows were affected
    uint64 affected_rows = Sql_NumRowsAffected(mmysql_handle);
    
    if (affected_rows > 0) {
        ShowInfo("[AI-IPC] ai_db_cancel: Successfully cancelled request ID %lld\n", (long long)request_id);
        script_pushint(st, 1);
    } else {
        ShowInfo("[AI-IPC] ai_db_cancel: Request %lld not found or already completed/cancelled\n", (long long)request_id);
        script_pushint(st, 0);
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httpget(url)
 * Backward compatibility wrapper - performs HTTP GET via database IPC
 */
BUILDIN_FUNC(httpget) {
    const char* url = script_getstr(st, 2);
    
    ShowInfo("[AI-IPC] httpget: URL='%s' (compatibility mode)\n", url);
    
    // Extract endpoint from URL
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    
    // Get context information
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // Escape strings
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_url = ai_ipc::sql_escape_string(url);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    // Create request with original URL as data for context
    std::string request_data = "{\"url\":\"" + esc_url + "\",\"method\":\"GET\"}";
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, `source_npc`, `source_map`, `expires_at`) "
        "VALUES ('http_get', '%s', '%s', 5, '%s', '%s', DATE_ADD(NOW(), INTERVAL %d SECOND))",
        ai_requests_table,
        esc_endpoint.c_str(),
        request_data.c_str(),
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
    ShowInfo("[AI-IPC] httpget: Created request ID %lld, waiting...\n", (long long)request_id);
    
    // Wait for response with HTTP compatibility timeout
    int elapsed_ms = 0;
    
    while (elapsed_ms < ai_ipc::HTTP_COMPAT_TIMEOUT_MS) {
        if (SQL_SUCCESS == Sql_Query(mmysql_handle,
            "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
            ai_responses_table, (long long)request_id))
        {
            if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
                char* data = nullptr;
                Sql_GetData(mmysql_handle, 0, &data, nullptr);
                
                if (data != nullptr && data[0] != '\0') {
                    ShowInfo("[AI-IPC] httpget: Got response after %dms\n", elapsed_ms);
                    script_pushstrcopy(st, data);
                    Sql_FreeResult(mmysql_handle);
                    return SCRIPT_CMD_SUCCESS;
                }
            }
            Sql_FreeResult(mmysql_handle);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(ai_ipc::POLL_INTERVAL_MS));
        elapsed_ms += ai_ipc::POLL_INTERVAL_MS;
    }
    
    ShowWarning("[AI-IPC] httpget: Timeout after %dms\n", elapsed_ms);
    script_pushstrcopy(st, "{\"error\":\"timeout\"}");
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httppost(url, data)
 * Backward compatibility wrapper - performs HTTP POST via database IPC
 */
BUILDIN_FUNC(httppost) {
    const char* url = script_getstr(st, 2);
    const char* post_data = script_getstr(st, 3);
    
    ShowInfo("[AI-IPC] httppost: URL='%s' (compatibility mode)\n", url);
    
    // Extract endpoint from URL
    std::string endpoint = ai_ipc::extract_endpoint_from_url(url);
    
    // Get context information
    const char* source_npc = ai_ipc::get_current_npc_name(st);
    const char* source_map = ai_ipc::get_current_map_name(st);
    
    // Escape strings
    std::string esc_endpoint = ai_ipc::sql_escape_string(endpoint.c_str());
    std::string esc_data = ai_ipc::sql_escape_string(post_data);
    std::string esc_npc = ai_ipc::sql_escape_string(source_npc);
    std::string esc_map = ai_ipc::sql_escape_string(source_map);
    
    if (SQL_ERROR == Sql_Query(mmysql_handle,
        "INSERT INTO `%s` (`request_type`, `endpoint`, `request_data`, `priority`, `source_npc`, `source_map`, `expires_at`) "
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
    ShowInfo("[AI-IPC] httppost: Created request ID %lld, waiting...\n", (long long)request_id);
    
    // Wait for response with HTTP compatibility timeout
    int elapsed_ms = 0;
    
    while (elapsed_ms < ai_ipc::HTTP_COMPAT_TIMEOUT_MS) {
        if (SQL_SUCCESS == Sql_Query(mmysql_handle,
            "SELECT `response_data` FROM `%s` WHERE `request_id` = %lld",
            ai_responses_table, (long long)request_id))
        {
            if (SQL_SUCCESS == Sql_NextRow(mmysql_handle)) {
                char* data = nullptr;
                Sql_GetData(mmysql_handle, 0, &data, nullptr);
                
                if (data != nullptr && data[0] != '\0') {
                    ShowInfo("[AI-IPC] httppost: Got response after %dms\n", elapsed_ms);
                    script_pushstrcopy(st, data);
                    Sql_FreeResult(mmysql_handle);
                    return SCRIPT_CMD_SUCCESS;
                }
            }
            Sql_FreeResult(mmysql_handle);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(ai_ipc::POLL_INTERVAL_MS));
        elapsed_ms += ai_ipc::POLL_INTERVAL_MS;
    }
    
    ShowWarning("[AI-IPC] httppost: Timeout after %dms\n", elapsed_ms);
    script_pushstrcopy(st, "{\"error\":\"timeout\"}");
    return SCRIPT_CMD_SUCCESS;
}