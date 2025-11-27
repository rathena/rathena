// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "script_ai_bridge.hpp"

#include <cstring>

#include <common/showmsg.hpp>
#include <common/strlib.hpp>

#include "script.hpp"
#include "../web/ai_bridge/ai_bridge.hpp"

using namespace AIBridge;

// Maximum response size (10KB)
#define MAX_HTTP_RESPONSE_SIZE 10240

/**
 * httpget(url$)
 * 
 * Performs HTTP GET request to AI service
 * Returns response body as string or empty string on error
 * 
 * Example:
 *   .@response$ = httpget("http://192.168.0.100:8000/api/v1/health");
 *   mes .@response$;
 */
BUILDIN_FUNC(httpget) {
    const char* url = script_getstr(st, 2);
    
    if (!url) {
        ShowError("script:httpget: null URL provided\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Validate URL
    if (strlen(url) == 0 || strlen(url) > 2048) {
        ShowError("script:httpget: invalid URL length\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Check if Bridge Layer is initialized
    if (!BridgeLayer::instance().is_initialized()) {
        ShowWarning("script:httpget: Bridge Layer not initialized\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Execute HTTP GET
    std::string response = BridgeLayer::instance().http_get(url);
    
    if (response.empty()) {
        ShowDebug("script:httpget: Empty response from %s\n", url);
        script_pushconststr(st, "");
        return SCRIPT_CMD_SUCCESS;
    }
    
    // Limit response size
    if (response.length() > MAX_HTTP_RESPONSE_SIZE) {
        ShowWarning("script:httpget: Response too large (%zu bytes), truncating to %d\n",
                    response.length(), MAX_HTTP_RESPONSE_SIZE);
        response = response.substr(0, MAX_HTTP_RESPONSE_SIZE);
    }
    
    // Return response
    script_pushstrcopy(st, response.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * httppost(url$, json_body$)
 * 
 * Performs HTTP POST request to AI service
 * Returns response body as string or empty string on error
 * 
 * Example:
 *   .@data$ = "{\"player_id\":150001,\"action\":\"login\"}";
 *   .@response$ = httppost("http://192.168.0.100:8000/api/v1/events/dispatch", .@data$);
 *   mes .@response$;
 */
BUILDIN_FUNC(httppost) {
    const char* url = script_getstr(st, 2);
    const char* json_body = script_getstr(st, 3);
    
    if (!url || !json_body) {
        ShowError("script:httppost: null parameters provided\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Validate URL
    if (strlen(url) == 0 || strlen(url) > 2048) {
        ShowError("script:httppost: invalid URL length\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Validate JSON body
    if (strlen(json_body) > MAX_HTTP_RESPONSE_SIZE) {
        ShowError("script:httppost: JSON body too large (%zu bytes)\n", strlen(json_body));
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Check if Bridge Layer is initialized
    if (!BridgeLayer::instance().is_initialized()) {
        ShowWarning("script:httppost: Bridge Layer not initialized\n");
        script_pushconststr(st, "");
        return SCRIPT_CMD_FAILURE;
    }
    
    // Execute HTTP POST
    std::string response = BridgeLayer::instance().http_post(url, json_body);
    
    if (response.empty()) {
        ShowDebug("script:httppost: Empty response from %s\n", url);
        script_pushconststr(st, "");
        return SCRIPT_CMD_SUCCESS;
    }
    
    // Limit response size
    if (response.length() > MAX_HTTP_RESPONSE_SIZE) {
        ShowWarning("script:httppost: Response too large (%zu bytes), truncating to %d\n",
                    response.length(), MAX_HTTP_RESPONSE_SIZE);
        response = response.substr(0, MAX_HTTP_RESPONSE_SIZE);
    }
    
    // Return response
    script_pushstrcopy(st, response.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_bridge_status()
 * 
 * Returns JSON with Bridge Layer status
 * 
 * Example:
 *   .@status$ = ai_bridge_status();
 *   mes .@status$;
 */
BUILDIN_FUNC(ai_bridge_status) {
    if (!BridgeLayer::instance().is_initialized()) {
        script_pushconststr(st, "{\"initialized\":false}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    std::string status_json = BridgeLayer::instance().get_status_json();
    script_pushstrcopy(st, status_json.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_bridge_reload()
 * 
 * Reloads Bridge Layer configuration
 * Returns 1 on success, 0 on failure
 * 
 * Example:
 *   if (ai_bridge_reload()) {
 *       mes "Configuration reloaded";
 *   }
 */
BUILDIN_FUNC(ai_bridge_reload) {
    if (!BridgeLayer::instance().is_initialized()) {
        ShowWarning("script:ai_bridge_reload: Bridge Layer not initialized\n");
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    bool success = BridgeLayer::instance().reload_config();
    script_pushint(st, success ? 1 : 0);
    
    return success ? SCRIPT_CMD_SUCCESS : SCRIPT_CMD_FAILURE;
}

// ============================================================================
// Initialization
// ============================================================================

void script_ai_bridge_init() {
    ShowInfo("[ScriptAIBridge] Registering AI Bridge script commands...\n");
    
    // Note: These commands must also be added to the buildin array in script.cpp
    // with BUILDIN_DEF(httpget, "s") and BUILDIN_DEF(httppost, "ss")
    
    ShowInfo("[ScriptAIBridge] Commands available:\n");
    ShowInfo("[ScriptAIBridge]   - httpget(url$)\n");
    ShowInfo("[ScriptAIBridge]   - httppost(url$, json_body$)\n");
    ShowInfo("[ScriptAIBridge]   - ai_bridge_status()\n");
    ShowInfo("[ScriptAIBridge]   - ai_bridge_reload()\n");
}

void script_ai_bridge_final() {
    ShowInfo("[ScriptAIBridge] Cleaning up AI Bridge script commands...\n");
    // Nothing to clean up for now
}