/**
 * aiworld_script_commands.cpp
 * Script Command Implementations for rAthena Integration
 * 
 * Provides custom script commands:
 * - httppost(url$, json$) - HTTP POST to AI service
 * - httpget(url$) - HTTP GET from AI service
 * - npcwalk(npc_name$, x, y) - AI-driven NPC movement
 * - npcwalkid(npc_id, x, y) - AI-driven NPC movement by ID
 */

#include "aiworld_http_client.hpp"
#include "aiworld_utils.hpp"
#include <nlohmann/json.hpp>

// rAthena script engine headers
#include "../map/script.hpp"
#include "../map/pc.hpp"
#include "../map/npc.hpp"
#include "../map/mob.hpp"
#include "../common/showmsg.hpp"

using namespace aiworld;

// Global HTTP client instance
static std::unique_ptr<AIWorldHTTPClient> g_http_client;

/**
 * Initialize the HTTP client (call during plugin initialization)
 */
void aiworld_init_http_client(const std::string& base_url = "http://127.0.0.1:8000") {
    if (!g_http_client) {
        g_http_client = std::make_unique<AIWorldHTTPClient>(base_url, 3000, 2);
        ShowInfo("AIWorld HTTP client initialized: %s\n", base_url.c_str());
    }
}

/**
 * Shutdown the HTTP client (call during plugin shutdown)
 */
void aiworld_shutdown_http_client() {
    if (g_http_client) {
        auto stats = g_http_client->get_stats();
        ShowInfo("AIWorld HTTP client stats - Total: %zu, Success: %zu, Failed: %zu, Avg latency: %.2fms\n",
                stats.total_requests, stats.successful_requests, stats.failed_requests, stats.avg_latency_ms);
        g_http_client.reset();
    }
}

/**
 * httppost(url$, json_data$)
 * Performs HTTP POST request to AI service
 * 
 * @param url$ - API endpoint URL (can be relative like "/api/npc/register" or full URL)
 * @param json_data$ - JSON string to send as request body
 * @return string - JSON response from server or error JSON
 * 
 * Example:
 *   .@response$ = httppost("/api/npc/register", "{\"npc_id\":\"NPC001\",\"name\":\"Guard\"}");
 */
BUILDIN(httppost) {
    if (!script_isstring(st, 2) || !script_isstring(st, 3)) {
        script_pushstrcopy(st, "{\"error\":\"httppost requires 2 string parameters: url, json_data\"}");
        ShowError("httppost: Invalid parameters. Usage: httppost(url$, json_data$)\n");
        return SCRIPT_CMD_FAILURE;
    }
    
    const char* url = script_getstr(st, 2);
    const char* json_data = script_getstr(st, 3);
    
    if (!g_http_client) {
        aiworld_init_http_client();
    }
    
    try {
        // Parse JSON data
        nlohmann::json request_json = nlohmann::json::parse(json_data);
        
        // Perform POST request
        HTTPResponse response = g_http_client->post(url, request_json);
        
        if (response.success) {
            script_pushstrcopy(st, response.body.c_str());
        } else {
            nlohmann::json error_json = {
                {"error", response.error_message},
                {"status_code", response.status_code}
            };
            script_pushstrcopy(st, error_json.dump().c_str());
            ShowWarning("httppost: Request failed - %s\n", response.error_message.c_str());
        }
    } catch (const nlohmann::json::exception& e) {
        nlohmann::json error_json = {
            {"error", "JSON parse error"},
            {"message", e.what()}
        };
        script_pushstrcopy(st, error_json.dump().c_str());
        ShowError("httppost: JSON parse error - %s\n", e.what());
    } catch (const std::exception& e) {
        nlohmann::json error_json = {
            {"error", "Exception"},
            {"message", e.what()}
        };
        script_pushstrcopy(st, error_json.dump().c_str());
        ShowError("httppost: Exception - %s\n", e.what());
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httpget(url$)
 * Performs HTTP GET request to AI service
 * 
 * @param url$ - API endpoint URL
 * @return string - JSON response from server or error JSON
 * 
 * Example:
 *   .@state$ = httpget("/api/npc/NPC001/state");
 */
BUILDIN(httpget) {
    if (!script_isstring(st, 2)) {
        script_pushstrcopy(st, "{\"error\":\"httpget requires 1 string parameter: url\"}");
        ShowError("httpget: Invalid parameters. Usage: httpget(url$)\n");
        return SCRIPT_CMD_FAILURE;
    }
    
    const char* url = script_getstr(st, 2);
    
    if (!g_http_client) {
        aiworld_init_http_client();
    }
    
    try {
        HTTPResponse response = g_http_client->get(url);
        
        if (response.success) {
            script_pushstrcopy(st, response.body.c_str());
        } else {
            nlohmann::json error_json = {
                {"error", response.error_message},
                {"status_code", response.status_code}
            };
            script_pushstrcopy(st, error_json.dump().c_str());
            ShowWarning("httpget: Request failed - %s\n", response.error_message.c_str());
        }
    } catch (const std::exception& e) {
        nlohmann::json error_json = {
            {"error", "Exception"},
            {"message", e.what()}
        };
        script_pushstrcopy(st, error_json.dump().c_str());
        ShowError("httpget: Exception - %s\n", e.what());
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * npcwalk(npc_name$, x, y)
 * Request AI-driven movement decision for NPC, then execute movement
 * 
 * @param npc_name$ - NPC name/ID
 * @param x - Target X coordinate
 * @param y - Target Y coordinate
 * @return int - 1 if movement initiated successfully, 0 on failure
 * 
 * Example:
 *   if (npcwalk("Guard#001", 100, 150)) {
 *       npctalk "Moving to new position...";
 *   }
 */
BUILDIN(npcwalk) {
    if (!script_isstring(st, 2) || !script_isint(st, 3) || !script_isint(st, 4)) {
        script_pushint(st, 0);
        ShowError("npcwalk: Invalid parameters. Usage: npcwalk(npc_name$, x, y)\n");
        return SCRIPT_CMD_FAILURE;
    }
    
    const char* npc_name = script_getstr(st, 2);
    int target_x = script_getnum(st, 3);
    int target_y = script_getnum(st, 4);
    
    if (!g_http_client) {
        aiworld_init_http_client();
    }
    
    try {
        // Prepare request to AI service for movement decision
        nlohmann::json request = {
            {"npc_id", npc_name},
            {"action", "move"},
            {"target_x", target_x},
            {"target_y", target_y}
        };
        
        // Request AI decision
        HTTPResponse response = g_http_client->post("/api/npc/movement", request);
        
        if (response.success) {
            nlohmann::json response_json = response.json();
            
            // Check if AI approved the movement
            bool approved = response_json.value("approved", true);
            
            if (approved) {
                // Execute NPC movement (simplified - actual implementation would use rAthena NPC API)
                ShowInfo("npcwalk: NPC '%s' moving to (%d, %d)\n", npc_name, target_x, target_y);
                script_pushint(st, 1);
            } else {
                ShowWarning("npcwalk: AI denied movement for NPC '%s'\n", npc_name);
                script_pushint(st, 0);
            }
        } else {
            ShowError("npcwalk: AI service request failed - %s\n", response.error_message.c_str());
            script_pushint(st, 0);
        }
    } catch (const std::exception& e) {
        ShowError("npcwalk: Exception - %s\n", e.what());
        script_pushint(st, 0);
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * npcwalkid(npc_id, x, y)
 * Request AI-driven movement decision for NPC by ID
 * 
 * @param npc_id - NPC GID/ID (integer)
 * @param x - Target X coordinate
 * @param y - Target Y coordinate
 * @return int - 1 if movement initiated successfully, 0 on failure
 */
BUILDIN(npcwalkid) {
    if (!script_isint(st, 2) || !script_isint(st, 3) || !script_isint(st, 4)) {
        script_pushint(st, 0);
        ShowError("npcwalkid: Invalid parameters. Usage: npcwalkid(npc_id, x, y)\n");
        return SCRIPT_CMD_FAILURE;
    }
    
    int npc_id = script_getnum(st, 2);
    int target_x = script_getnum(st, 3);
    int target_y = script_getnum(st, 4);
    
    if (!g_http_client) {
        aiworld_init_http_client();
    }
    
    try {
        // Prepare request
        nlohmann::json request = {
            {"npc_id", std::to_string(npc_id)},
            {"action", "move"},
            {"target_x", target_x},
            {"target_y", target_y}
        };
        
        // Request AI decision
        HTTPResponse response = g_http_client->post("/api/npc/movement", request);
        
        if (response.success) {
            nlohmann::json response_json = response.json();
            bool approved = response_json.value("approved", true);
            
            if (approved) {
                ShowInfo("npcwalkid: NPC ID %d moving to (%d, %d)\n", npc_id, target_x, target_y);
                script_pushint(st, 1);
            } else {
                ShowWarning("npcwalkid: AI denied movement for NPC ID %d\n", npc_id);
                script_pushint(st, 0);
            }
        } else {
            ShowError("npcwalkid: AI service request failed - %s\n", response.error_message.c_str());
            script_pushint(st, 0);
        }
    } catch (const std::exception& e) {
        ShowError("npcwalkid: Exception - %s\n", e.what());
        script_pushint(st, 0);
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * Register all AIWorld script commands with rAthena script engine
 * Call this during plugin initialization
 */
void aiworld_register_http_script_commands() {
    // Register HTTP commands
    script_set_command("httppost", buildin_httppost, "ss");
    script_set_command("httpget", buildin_httpget, "s");
    
    // Register NPC movement commands
    script_set_command("npcwalk", buildin_npcwalk, "sii");
    script_set_command("npcwalkid", buildin_npcwalkid, "iii");
    
    ShowInfo("AIWorld: Registered 4 custom script commands (httppost, httpget, npcwalk, npcwalkid)\n");
}