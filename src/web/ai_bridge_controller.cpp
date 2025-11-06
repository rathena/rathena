// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_bridge_controller.hpp"

#include <sstream>
#include <string>

#include <common/showmsg.hpp>
#include <common/socket.hpp>

#include "auth.hpp"
#include "http.hpp"
#include "web.hpp"

namespace AIBridge {
	// AI Service configuration with defaults
	std::string ai_service_url = "127.0.0.1";
	uint16 ai_service_port = 8000;
	std::string ai_service_api_key = "";
	bool ai_service_enabled = true;
	
	void initialize() {
		ShowInfo("[AI Bridge] Initializing AI Bridge Layer...\n");
		ShowInfo("[AI Bridge] AI Service URL: %s:%d\n", ai_service_url.c_str(), ai_service_port);
		ShowInfo("[AI Bridge] AI Service Enabled: %s\n", ai_service_enabled ? "Yes" : "No");
	}
	
	bool make_ai_request(
		const std::string& endpoint,
		const std::string& method,
		const std::string& body,
		std::string& response_body,
		int& status_code
	) {
		if (!ai_service_enabled) {
			ShowWarning("[AI Bridge] AI Service is disabled. Skipping request to %s\n", endpoint.c_str());
			status_code = 503; // Service Unavailable
			response_body = "{\"error\": \"AI Service is disabled\"}";
			return false;
		}
		
		try {
			ShowDebug("[AI Bridge] Making %s request to AI Service: %s\n", method.c_str(), endpoint.c_str());
			ShowDebug("[AI Bridge] Request body: %s\n", body.c_str());
			
			// Create HTTP client
			httplib::Client client(ai_service_url.c_str(), ai_service_port);
			client.set_connection_timeout(5, 0); // 5 seconds
			client.set_read_timeout(30, 0); // 30 seconds
			client.set_write_timeout(5, 0); // 5 seconds
			
			// Set headers
			httplib::Headers headers = {
				{"Content-Type", "application/json"},
				{"User-Agent", "rAthena-AI-Bridge/1.0"}
			};
			
			// Add API key if configured
			if (!ai_service_api_key.empty()) {
				headers.insert({"X-API-Key", ai_service_api_key});
			}
			
			// Make request based on method
			httplib::Result res;
			if (method == "GET") {
				res = client.Get(endpoint.c_str(), headers);
			} else if (method == "POST") {
				res = client.Post(endpoint.c_str(), headers, body, "application/json");
			} else if (method == "PUT") {
				res = client.Put(endpoint.c_str(), headers, body, "application/json");
			} else {
				ShowError("[AI Bridge] Unsupported HTTP method: %s\n", method.c_str());
				status_code = 400;
				response_body = "{\"error\": \"Unsupported HTTP method\"}";
				return false;
			}
			
			// Check if request was successful
			if (!res) {
				auto err = res.error();
				ShowError("[AI Bridge] HTTP request failed: %s (error code: %d)\n", 
					httplib::to_string(err).c_str(), static_cast<int>(err));
				status_code = 503; // Service Unavailable
				response_body = "{\"error\": \"Failed to connect to AI Service\"}";
				return false;
			}
			
			// Get response
			status_code = res->status;
			response_body = res->body;
			
			ShowDebug("[AI Bridge] Response status: %d\n", status_code);
			ShowDebug("[AI Bridge] Response body: %s\n", response_body.c_str());
			
			return (status_code >= 200 && status_code < 300);
			
		} catch (const std::exception& e) {
			ShowError("[AI Bridge] Exception during HTTP request: %s\n", e.what());
			status_code = 500;
			response_body = "{\"error\": \"Internal server error\"}";
			return false;
		}
	}
}

// Helper function to extract path parameter
std::string extract_path_param(const std::string& path, const std::string& prefix) {
	size_t pos = path.find(prefix);
	if (pos == std::string::npos) {
		return "";
	}
	
	size_t start = pos + prefix.length();
	size_t end = path.find('/', start);
	
	if (end == std::string::npos) {
		return path.substr(start);
	}
	return path.substr(start, end - start);
}

// Helper function to build query string from request parameters
std::string build_query_string(const Request& req) {
	std::ostringstream oss;
	bool first = true;
	
	for (const auto& param : req.params) {
		if (!first) {
			oss << "&";
		}
		oss << param.first << "=" << param.second;
		first = false;
	}
	
	return oss.str();
}

// POST /ai/npc/register
HANDLER_FUNC(ai_npc_register) {
	ShowInfo("[AI Bridge] Received NPC registration request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/npc/register", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC registration successful\n");
	} else {
		ShowWarning("[AI Bridge] NPC registration failed with status %d\n", status_code);
	}
}

// POST /ai/npc/event
HANDLER_FUNC(ai_npc_event) {
	ShowInfo("[AI Bridge] Received NPC event\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/npc/event", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC event processed successfully\n");
	} else {
		ShowWarning("[AI Bridge] NPC event processing failed with status %d\n", status_code);
	}
}

// GET /ai/npc/{id}/action
HANDLER_FUNC(ai_npc_action) {
	ShowInfo("[AI Bridge] Received NPC action request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	ShowDebug("[AI Bridge] Getting action for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/action";

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC action retrieved successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Failed to get NPC action for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// POST /ai/world/state
HANDLER_FUNC(ai_world_state_update) {
	ShowInfo("[AI Bridge] Received world state update\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/world/state", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] World state updated successfully\n");
	} else {
		ShowWarning("[AI Bridge] World state update failed with status %d\n", status_code);
	}
}

// GET /ai/world/state
HANDLER_FUNC(ai_world_state_get) {
	ShowInfo("[AI Bridge] Received world state query\n");

	// Build query string from parameters
	std::string query_string = build_query_string(req);
	std::string endpoint = "/ai/world/state";

	if (!query_string.empty()) {
		endpoint += "?" + query_string;
		ShowDebug("[AI Bridge] Query parameters: %s\n", query_string.c_str());
	}

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] World state retrieved successfully\n");
	} else {
		ShowWarning("[AI Bridge] World state query failed with status %d\n", status_code);
	}
}

// POST /ai/player/interaction
HANDLER_FUNC(ai_player_interaction) {
	ShowInfo("[AI Bridge] Received player-NPC interaction\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/player/interaction", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Player interaction processed successfully\n");
	} else {
		ShowWarning("[AI Bridge] Player interaction processing failed with status %d\n", status_code);
	}
}

