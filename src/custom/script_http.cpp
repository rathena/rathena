// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
//
// HTTP Bridge Layer for AI Service Integration
// This module provides HTTP communication capabilities to rAthena's script engine
// enabling AI-driven NPC behavior through REST API calls.

#include "script_http.hpp"

#include <curl/curl.h>
#include <cstring>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/malloc.hpp>
#include "../map/npc.hpp"
#include "../map/unit.hpp"
#include "../map/map.hpp"

// Maximum size for HTTP request/response bodies (10KB security limit)
#define MAX_HTTP_BODY_SIZE 10240

// Default HTTP timeout in seconds
#define HTTP_TIMEOUT 5L

// Global CURL handle for connection reuse
static CURL* global_curl = nullptr;

/**
 * Callback function for CURL to write response data
 * @param ptr Pointer to delivered data
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userdata Pointer to user-defined data (our response string)
 * @return Number of bytes processed
 */
static size_t curl_write_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
	std::string* response = static_cast<std::string*>(userdata);
	size_t total_size = size * nmemb;
	
	// Security: Prevent excessive response sizes
	if (response->length() + total_size > MAX_HTTP_BODY_SIZE) {
		ShowWarning("script_http: Response size exceeds maximum (%d bytes). Truncating...\n", MAX_HTTP_BODY_SIZE);
		total_size = MAX_HTTP_BODY_SIZE - response->length();
	}
	
	if (total_size > 0) {
		response->append(static_cast<char*>(ptr), total_size);
	}
	
	return size * nmemb; // Return original size to CURL
}

/**
 * Initialize CURL if not already initialized
 * @return true on success, false on failure
 */
static bool curl_init_check() {
	if (global_curl == nullptr) {
		global_curl = curl_easy_init();
		if (global_curl == nullptr) {
			ShowError("script_http: Failed to initialize CURL\n");
			return false;
		}
	}
	return true;
}

/**
 * Validate URL format for security
 * @param url URL string to validate
 * @return true if valid, false otherwise
 */
static bool validate_url(const std::string& url) {
	// Basic URL validation
	if (url.empty() || url.length() > 2048) {
		return false;
	}
	
	// Must start with http:// or https://
	if (url.compare(0, 7, "http://") != 0 && url.compare(0, 8, "https://") != 0) {
		return false;
	}
	
	// Security: Block potential injection attempts
	if (url.find('\n') != std::string::npos || url.find('\r') != std::string::npos) {
		return false;
	}
	
	return true;
}

/**
 * Perform HTTP POST request
 * @param url Target URL
 * @param data JSON data to send
 * @return Response string or empty string on failure
 */
std::string http_post(const std::string& url, const std::string& data) {
	if (!curl_init_check()) {
		return "";
	}
	
	// Validate inputs
	if (!validate_url(url)) {
		ShowError("script_http: Invalid URL format\n");
		return "";
	}
	
	if (data.length() > MAX_HTTP_BODY_SIZE) {
		ShowError("script_http: POST data exceeds maximum size (%d bytes)\n", MAX_HTTP_BODY_SIZE);
		return "";
	}
	
	std::string response;
	struct curl_slist* headers = nullptr;
	
	// Set headers
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept: application/json");
	
	// Configure CURL options
	curl_easy_reset(global_curl);
	curl_easy_setopt(global_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(global_curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(global_curl, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(global_curl, CURLOPT_POSTFIELDSIZE, data.size());
	curl_easy_setopt(global_curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
	curl_easy_setopt(global_curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
	curl_easy_setopt(global_curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(global_curl, CURLOPT_NOSIGNAL, 1L); // Thread-safe
	
	// Perform request
	CURLcode res = curl_easy_perform(global_curl);
	long http_code = 0;
	curl_easy_getinfo(global_curl, CURLINFO_RESPONSE_CODE, &http_code);
	
	// Cleanup
	curl_slist_free_all(headers);
	
	// Check results
	if (res != CURLE_OK) {
		ShowDebug("script_http: POST request failed: %s\n", curl_easy_strerror(res));
		return "";
	}
	
	if (http_code < 200 || http_code >= 300) {
		ShowDebug("script_http: POST request returned HTTP %ld\n", http_code);
		return "";
	}
	
	return response;
}

/**
 * Perform HTTP GET request
 * @param url Target URL
 * @return Response string or empty string on failure
 */
std::string http_get(const std::string& url) {
	if (!curl_init_check()) {
		return "";
	}
	
	// Validate input
	if (!validate_url(url)) {
		ShowError("script_http: Invalid URL format\n");
		return "";
	}
	
	std::string response;
	struct curl_slist* headers = nullptr;
	
	// Set headers
	headers = curl_slist_append(headers, "Accept: application/json");
	
	// Configure CURL options
	curl_easy_reset(global_curl);
	curl_easy_setopt(global_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(global_curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(global_curl, CURLOPT_HTTPGET, 1L);
	curl_easy_setopt(global_curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
	curl_easy_setopt(global_curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
	curl_easy_setopt(global_curl, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(global_curl, CURLOPT_NOSIGNAL, 1L); // Thread-safe
	
	// Perform request
	CURLcode res = curl_easy_perform(global_curl);
	long http_code = 0;
	curl_easy_getinfo(global_curl, CURLINFO_RESPONSE_CODE, &http_code);
	
	// Cleanup
	curl_slist_free_all(headers);
	
	// Check results
	if (res != CURLE_OK) {
		ShowDebug("script_http: GET request failed: %s\n", curl_easy_strerror(res));
		return "";
	}
	
	if (http_code < 200 || http_code >= 300) {
		ShowDebug("script_http: GET request returned HTTP %ld\n", http_code);
		return "";
	}
	
	return response;
}

/**
 * Script command: httppost
 * Performs an HTTP POST request to the AI service
 * 
 * Usage: httppost("<url>", "<json_data>")
 * Returns: Response string or empty string on failure
 * 
 * Example:
 *   .@response$ = httppost("http://localhost:8000/api/v1/npc/1001/action", "{\"action\":\"walk\",\"x\":150,\"y\":180}");
 */
BUILDIN_FUNC(httppost) {
	const char* url = script_getstr(st, 2);
	const char* data = script_getstr(st, 3);
	
	if (url == nullptr || data == nullptr) {
		ShowError("script:httppost: null arguments provided\n");
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}
	
	// Perform HTTP POST
	std::string response = http_post(url, data);
	
	// Return response to script
	if (!response.empty()) {
		script_pushstrcopy(st, response.c_str());
	} else {
		script_pushconststr(st, "");
	}
	
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Script command: httpget
 * Performs an HTTP GET request to the AI service
 * 
 * Usage: httpget("<url>")
 * Returns: Response string or empty string on failure
 * 
 * Example:
 *   .@response$ = httpget("http://localhost:8000/api/v1/npc/1001/state");
 */
BUILDIN_FUNC(httpget) {
	const char* url = script_getstr(st, 2);
	
	if (url == nullptr) {
		ShowError("script:httpget: null argument provided\n");
		script_pushconststr(st, "");
		return SCRIPT_CMD_FAILURE;
	}
	
	// Perform HTTP GET
	std::string response = http_get(url);
	
	// Return response to script
	if (!response.empty()) {
		script_pushstrcopy(st, response.c_str());
	} else {
		script_pushconststr(st, "");
	}
	
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Script command: npcwalk
 * Moves an NPC by name to target coordinates and notifies AI service
 * 
 * Usage: npcwalk("<npc_name>", <x>, <y>)
 * Returns: 1 on success, 0 on failure
 * 
 * Example:
 *   npcwalk("Guard#1", 150, 180);
 */
BUILDIN_FUNC(npcwalk) {
	const char* npc_name = script_getstr(st, 2);
	int32 x = script_getnum(st, 3);
	int32 y = script_getnum(st, 4);
	
	// Find NPC by name
	npc_data* nd = npc_name2id(npc_name);
	if (nd == nullptr) {
		ShowWarning("script:npcwalk: NPC '%s' not found\n", npc_name);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	// Validate map bounds
	if (nd->m < 0) {
		ShowError("script:npcwalk: NPC '%s' is not on a valid map\n", npc_name);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	struct map_data* mapdata = map_getmapdata(nd->m);
	if (x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys) {
		ShowWarning("script:npcwalk: coordinates (%d,%d) are out of bounds for map %s\n", x, y, mapdata->name);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	// Prepare AI service notification (non-blocking)
	char ai_url[512];
	char ai_data[512];
	snprintf(ai_url, sizeof(ai_url), "http://localhost:8000/api/v1/npc/%d/action", nd->id);
	snprintf(ai_data, sizeof(ai_data), 
		"{\"command_type\":\"npc_move\",\"npc_id\":\"%s\",\"target_x\":%d,\"target_y\":%d,\"map\":\"%s\"}", 
		npc_name, x, y, mapdata->name);
	
	// Send to AI service (best effort, don't block on failure)
	std::string ai_response = http_post(ai_url, ai_data);
	if (ai_response.empty()) {
		ShowDebug("script:npcwalk: AI service notification failed for NPC '%s', continuing with movement\n", npc_name);
	}
	
	// Initialize NPC status if needed
	if (!nd->status.hp) {
		status_calc_npc(nd, SCO_FIRST);
	} else {
		status_calc_npc(nd, SCO_NONE);
	}
	
	// Execute movement
	bool success = unit_walktoxy(nd, x, y, 0);
	
	script_pushint(st, success ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Script command: npcwalkid
 * Moves an NPC by game ID to target coordinates and notifies AI service
 * 
 * Usage: npcwalkid(<npc_id>, <x>, <y>)
 * Returns: 1 on success, 0 on failure
 * 
 * Example:
 *   npcwalkid(1001, 150, 180);
 */
BUILDIN_FUNC(npcwalkid) {
	int32 npc_id = script_getnum(st, 2);
	int32 x = script_getnum(st, 3);
	int32 y = script_getnum(st, 4);
	
	// Find NPC by GID
	npc_data* nd = map_id2nd(npc_id);
	if (nd == nullptr) {
		ShowWarning("script:npcwalkid: NPC with ID %d not found\n", npc_id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	// Validate map bounds
	if (nd->m < 0) {
		ShowError("script:npcwalkid: NPC ID %d is not on a valid map\n", npc_id);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	struct map_data* mapdata = map_getmapdata(nd->m);
	if (x < 0 || x >= mapdata->xs || y < 0 || y >= mapdata->ys) {
		ShowWarning("script:npcwalkid: coordinates (%d,%d) are out of bounds for map %s\n", x, y, mapdata->name);
		script_pushint(st, 0);
		return SCRIPT_CMD_FAILURE;
	}
	
	// Prepare AI service notification (non-blocking)
	char ai_url[512];
	char ai_data[512];
	snprintf(ai_url, sizeof(ai_url), "http://localhost:8000/api/v1/npc/%d/action", npc_id);
	snprintf(ai_data, sizeof(ai_data), 
		"{\"command_type\":\"npc_move\",\"npc_id\":%d,\"target_x\":%d,\"target_y\":%d,\"map\":\"%s\"}", 
		npc_id, x, y, mapdata->name);
	
	// Send to AI service (best effort, don't block on failure)
	std::string ai_response = http_post(ai_url, ai_data);
	if (ai_response.empty()) {
		ShowDebug("script:npcwalkid: AI service notification failed for NPC ID %d, continuing with movement\n", npc_id);
	}
	
	// Initialize NPC status if needed
	if (!nd->status.hp) {
		status_calc_npc(nd, SCO_FIRST);
	} else {
		status_calc_npc(nd, SCO_NONE);
	}
	
	// Execute movement
	bool success = unit_walktoxy(nd, x, y, 0);
	
	script_pushint(st, success ? 1 : 0);
	return SCRIPT_CMD_SUCCESS;
}

/**
 * Cleanup function to be called on shutdown
 */
void script_http_final() {
	if (global_curl != nullptr) {
		curl_easy_cleanup(global_curl);
		global_curl = nullptr;
	}
}

/**
 * Initialization function
 */
void script_http_init() {
	// CURL will be initialized on first use
	global_curl = nullptr;
}