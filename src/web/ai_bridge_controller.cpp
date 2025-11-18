// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_bridge_controller.hpp"

#include <sstream>
#include <string>

#include <common/cbasetypes.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>

#include "auth.hpp"
#include "http.hpp"
#include "web.hpp"

#include <nlohmann/json.hpp>
#include "aiworld/aiworld_plugin.hpp"
namespace AIBridge {
	// AI Service configuration with defaults
	std::string ai_service_url = "127.0.0.1";
	uint16 ai_service_port = 8000;
	std::string ai_service_api_key = "";
	bool ai_service_enabled = true;

	// --- AIWorld IPC Integration ---
	// If enabled, use ZeroMQ IPC instead of HTTP for internal AI/NPC/quest/event calls
	bool aiworld_ipc_enabled = true;

	void initialize() {
		ShowInfo("[AI Bridge] Initializing AI Bridge Layer...\n");
		ShowInfo("[AI Bridge] AI Service URL: %s:%d\n", ai_service_url.c_str(), ai_service_port);
		ShowInfo("[AI Bridge] AI Service Enabled: %s\n", ai_service_enabled ? "Yes" : "No");
	}

	void read_config(const char* w1, const char* w2) {
		if (!strcmpi(w1, "ai_service_enabled")) {
			ai_service_enabled = config_switch(w2) == 1;
		} else if (!strcmpi(w1, "ai_service_url")) {
			ai_service_url = w2;
		} else if (!strcmpi(w1, "ai_service_port")) {
			ai_service_port = (uint16)atoi(w2);
		} else if (!strcmpi(w1, "ai_service_api_key")) {
			ai_service_api_key = w2;
		}
	}

	// Helper function to extract path parameter (e.g., NPC ID from /ai/npc/{id}/action)
	std::string extract_path_param(const std::string& path, const std::string& prefix) {
		if (path.find(prefix) != 0) {
			return "";
		}

		std::string param = path.substr(prefix.length());

		// Remove leading/trailing slashes
		while (!param.empty() && param[0] == '/') {
			param = param.substr(1);
		}
		while (!param.empty() && param[param.length() - 1] == '/') {
			param = param.substr(0, param.length() - 1);
		}

		return param;
	}

	// Helper function to build query string from request parameters
	std::string build_query_string(const httplib::Request& req) {
		std::string query_string;
		bool first = true;

		for (const auto& param : req.params) {
			if (!first) {
				query_string += "&";
			}
			query_string += param.first + "=" + param.second;
			first = false;
		}

		return query_string;
	}


	bool make_ai_request(
		const std::string& endpoint,
		const std::string& method,
		const std::string& body,
		std::string& response_body,
		int& status_code
	) {
		// If aiworld_ipc_enabled, use ZeroMQ IPC for internal AI/NPC/quest/event calls
		if (aiworld_ipc_enabled) {
			try {
				// Use the aiworld_plugin to send/receive via IPC
				extern aiworld::AIWorldPlugin* g_aiworld_plugin;
				if (!g_aiworld_plugin || !g_aiworld_plugin->is_initialized) {
					ShowError("[AI Bridge] AIWorld IPC not initialized.\n");
					status_code = 503;
					response_body = "{\"error\": \"AIWorld IPC not initialized\"}";
					return false;
				}
				// Map endpoint/method to plugin call (example for /ai/npc/{id}/state)
				if (endpoint.find("/ai/npc/") == 0 && endpoint.find("/state") != std::string::npos) {
					std::string npc_id = extract_path_param(endpoint, "/ai/npc/");
					size_t pos = npc_id.find("/state");
					if (pos != std::string::npos) npc_id = npc_id.substr(0, pos);
					if (method == "GET") {
						auto state = g_aiworld_plugin->get_npc_consciousness(npc_id);
						response_body = state.dump();
						status_code = 200;
						return true;
					} else if (method == "PUT") {
						nlohmann::json j = nlohmann::json::parse(body);
						bool ok = g_aiworld_plugin->update_npc_consciousness(npc_id, j);
						status_code = ok ? 200 : 500;
						response_body = ok ? "{\"status\": \"ok\"}" : "{\"error\": \"update failed\"}";
						return ok;
					}
				}
				// Fallback: not implemented
				status_code = 501;
				response_body = "{\"error\": \"Not implemented in IPC bridge\"}";
				return false;
			} catch (const std::exception& e) {
				ShowError("[AI Bridge] Exception during IPC request: %s\n", e.what());
				status_code = 500;
				response_body = "{\"error\": \"Internal server error\"}";
				return false;
			}
		}
		// Fallback to HTTP if not using IPC
		if (!ai_service_enabled) {
			ShowWarning("[AI Bridge] AI Service is disabled. Skipping request to %s\n", endpoint.c_str());
			status_code = 503; // Service Unavailable
			response_body = "{\"error\": \"AI Service is disabled\"}";
			return false;
		}
		try {
			ShowDebug("[AI Bridge] Making %s request to AI Service: %s\n", method.c_str(), endpoint.c_str());
			ShowDebug("[AI Bridge] Request body: %s\n", body.c_str());
			httplib::Client client(ai_service_url.c_str(), ai_service_port);
			client.set_connection_timeout(5, 0); // 5 seconds
			client.set_read_timeout(30, 0); // 30 seconds
			client.set_write_timeout(5, 0); // 5 seconds
			httplib::Headers headers = {
				{"Content-Type", "application/json"},
				{"User-Agent", "rAthena-AI-Bridge/1.0"}
			};
			if (!ai_service_api_key.empty()) {
				headers.insert({"X-API-Key", ai_service_api_key});
			}
			if (method == "GET") {
				auto res = client.Get(endpoint.c_str(), headers);
				if (!res) {
					auto err = res.error();
					ShowError("[AI Bridge] HTTP GET request failed: %s (error code: %d)\n",
						httplib::to_string(err).c_str(), static_cast<int>(err));
					status_code = 503; // Service Unavailable
					response_body = "{\"error\": \"Failed to connect to AI Service\"}";
					return false;
				}
				status_code = res->status;
				response_body = res->body;
			} else if (method == "POST") {
				auto res = client.Post(endpoint.c_str(), headers, body, "application/json");
				if (!res) {
					auto err = res.error();
					ShowError("[AI Bridge] HTTP POST request failed: %s (error code: %d)\n",
						httplib::to_string(err).c_str(), static_cast<int>(err));
					status_code = 503; // Service Unavailable
					response_body = "{\"error\": \"Failed to connect to AI Service\"}";
					return false;
				}
				status_code = res->status;
				response_body = res->body;
			} else if (method == "PUT") {
				auto res = client.Put(endpoint.c_str(), headers, body, "application/json");
				if (!res) {
					auto err = res.error();
					ShowError("[AI Bridge] HTTP PUT request failed: %s (error code: %d)\n",
						httplib::to_string(err).c_str(), static_cast<int>(err));
					status_code = 503; // Service Unavailable
					response_body = "{\"error\": \"Failed to connect to AI Service\"}";
					return false;
				}
				status_code = res->status;
				response_body = res->body;
			} else {
				ShowError("[AI Bridge] Unsupported HTTP method: %s\n", method.c_str());
				status_code = 400;
				response_body = "{\"error\": \"Unsupported HTTP method\"}";
				return false;
			}
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

// POST /ai/quest/generate
HANDLER_FUNC(ai_quest_generate) {
	ShowInfo("[AI Bridge] Received quest generation request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/quest/generate", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Quest generated successfully\n");
	} else {
		ShowWarning("[AI Bridge] Quest generation failed with status %d\n", status_code);
	}
}

// POST /ai/quest/progress (quest_id in request body)
HANDLER_FUNC(ai_quest_progress) {
	ShowInfo("[AI Bridge] Received quest progress update\n");

	// Get request body (contains quest_id, objective_id, progress_amount)
	std::string request_body = req.body;

	// Forward to AI service - quest_id is in the request body, not the path
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/quest/progress", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Quest progress updated successfully\n");
	} else {
		ShowWarning("[AI Bridge] Quest progress update failed with status %d\n", status_code);
	}
}

// GET /ai/quest/{quest_id}
HANDLER_FUNC(ai_quest_get) {
	ShowInfo("[AI Bridge] Received quest details request\n");

	// Extract quest ID from path
	std::string quest_id = extract_path_param(req.path, "/ai/quest/");

	if (quest_id.empty()) {
		ShowError("[AI Bridge] Missing quest ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing quest ID\"}", "application/json");
		return;
	}

	ShowDebug("[AI Bridge] Getting details for quest ID: %s\n", quest_id.c_str());

	// Build endpoint with quest ID
	std::string endpoint = "/ai/quest/" + quest_id;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Quest details retrieved successfully for quest %s\n", quest_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Failed to get quest details for quest %s (status %d)\n", quest_id.c_str(), status_code);
	}
}

// POST /ai/quest/{quest_id}/complete
HANDLER_FUNC(ai_quest_complete) {
	ShowInfo("[AI Bridge] Received quest completion request\n");

	// Extract quest ID from path
	std::string quest_id = extract_path_param(req.path, "/ai/quest/");

	if (quest_id.empty()) {
		ShowError("[AI Bridge] Missing quest ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing quest ID\"}", "application/json");
		return;
	}

	// Remove "/complete" suffix if present
	size_t pos = quest_id.find("/complete");
	if (pos != std::string::npos) {
		quest_id = quest_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Completing quest ID: %s\n", quest_id.c_str());

	// Build endpoint with quest ID
	std::string endpoint = "/ai/quest/" + quest_id + "/complete";

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Quest completed successfully for quest %s\n", quest_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Quest completion failed for quest %s (status %d)\n", quest_id.c_str(), status_code);
	}
}

// POST /ai/chat/command
HANDLER_FUNC(ai_chat_command) {
	ShowInfo("[AI Bridge] Received chat command request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/chat/command", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Chat command processed successfully\n");
	} else {
		ShowWarning("[AI Bridge] Chat command processing failed with status %d\n", status_code);
	}
}

// GET /ai/chat/history
HANDLER_FUNC(ai_chat_history) {
	ShowInfo("[AI Bridge] Received chat history request\n");

	// Build query string from parameters
	std::string query_string = build_query_string(req);
	std::string endpoint = "/ai/chat/history";

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
		ShowInfo("[AI Bridge] Chat history retrieved successfully\n");
	} else {
		ShowWarning("[AI Bridge] Chat history retrieval failed with status %d\n", status_code);
	}
}

// GET /ai/npc/{npc_id}/state
HANDLER_FUNC(ai_npc_state_get) {
	ShowInfo("[AI Bridge] Received NPC state get request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	// Remove "/state" suffix if present
	size_t pos = npc_id.find("/state");
	if (pos != std::string::npos) {
		npc_id = npc_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Getting state for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/state";

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC state retrieved successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Failed to get NPC state for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// PUT /ai/npc/{npc_id}/state
HANDLER_FUNC(ai_npc_state_update) {
	ShowInfo("[AI Bridge] Received NPC state update request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	// Remove "/state" suffix if present
	size_t pos = npc_id.find("/state");
	if (pos != std::string::npos) {
		npc_id = npc_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Updating state for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/state";

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "PUT", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC state updated successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] NPC state update failed for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// DELETE /ai/npc/{npc_id}
HANDLER_FUNC(ai_npc_delete) {
	ShowInfo("[AI Bridge] Received NPC deletion request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	ShowDebug("[AI Bridge] Deleting NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "DELETE", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC deleted successfully: %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] NPC deletion failed for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// GET /ai/npc/{npc_id}/memory
HANDLER_FUNC(ai_npc_memory_get) {
	ShowInfo("[AI Bridge] Received NPC memory get request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	// Remove "/memory" suffix if present
	size_t pos = npc_id.find("/memory");
	if (pos != std::string::npos) {
		npc_id = npc_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Getting memory for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/memory";

	// Build query string from parameters
	std::string query_string = build_query_string(req);
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
		ShowInfo("[AI Bridge] NPC memory retrieved successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Failed to get NPC memory for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// POST /ai/npc/{npc_id}/memory
HANDLER_FUNC(ai_npc_memory_add) {
	ShowInfo("[AI Bridge] Received NPC memory add request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	// Remove "/memory" suffix if present
	size_t pos = npc_id.find("/memory");
	if (pos != std::string::npos) {
		npc_id = npc_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Adding memory for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/memory";

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC memory added successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] NPC memory addition failed for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// POST /api/batch/npcs/update
HANDLER_FUNC(ai_batch_npcs_update) {
	ShowInfo("[AI Bridge] Received batch NPC update request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/api/batch/npcs/update", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Batch NPC update completed successfully\n");
	} else {
		ShowWarning("[AI Bridge] Batch NPC update failed with status %d\n", status_code);
	}
}

// POST /api/batch/players/interact
HANDLER_FUNC(ai_batch_interactions) {
	ShowInfo("[AI Bridge] Received batch player interactions request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service - correct path is /api/batch/players/interact
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/api/batch/players/interact", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Batch player interactions completed successfully\n");
	} else {
		ShowWarning("[AI Bridge] Batch player interactions failed with status %d\n", status_code);
	}
}

// POST /ai/npc/{npc_id}/execute-action
HANDLER_FUNC(ai_npc_execute_action) {
	ShowInfo("[AI Bridge] Received NPC execute action request\n");

	// Extract NPC ID from path
	std::string npc_id = extract_path_param(req.path, "/ai/npc/");

	if (npc_id.empty()) {
		ShowError("[AI Bridge] Missing NPC ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing NPC ID\"}", "application/json");
		return;
	}

	// Remove "/execute-action" suffix if present
	size_t pos = npc_id.find("/execute-action");
	if (pos != std::string::npos) {
		npc_id = npc_id.substr(0, pos);
	}

	ShowDebug("[AI Bridge] Executing action for NPC ID: %s\n", npc_id.c_str());

	// Build endpoint with NPC ID
	std::string endpoint = "/ai/npc/" + npc_id + "/execute-action";

	// Get request body (contains action_type, action_data, etc.)
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] NPC action executed successfully for NPC %s\n", npc_id.c_str());
	} else {
		ShowWarning("[AI Bridge] NPC action execution failed for NPC %s (status %d)\n", npc_id.c_str(), status_code);
	}
}

// GET /ai/chat/status
HANDLER_FUNC(ai_chat_status) {
	ShowInfo("[AI Bridge] Received chat status request\n");

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/chat/status", "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Chat status retrieved successfully\n");
	} else {
		ShowWarning("[AI Bridge] Chat status retrieval failed with status %d\n", status_code);
	}
}

// ============================================================================
// Economy Endpoints
// ============================================================================

// GET /ai/economy/state
HANDLER_FUNC(ai_economy_state) {
	ShowInfo("[AI Bridge] Received economy state request\n");

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/economy/state", "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Economy state retrieved successfully\n");
	} else {
		ShowWarning("[AI Bridge] Economy state retrieval failed with status %d\n", status_code);
	}
}

// POST /ai/economy/price/update
HANDLER_FUNC(ai_economy_price_update) {
	ShowInfo("[AI Bridge] Received economy price update request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/economy/price/update", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Economy price updated successfully\n");
	} else {
		ShowWarning("[AI Bridge] Economy price update failed with status %d\n", status_code);
	}
}

// POST /ai/economy/market/analyze
HANDLER_FUNC(ai_economy_market_analyze) {
	ShowInfo("[AI Bridge] Received market analysis request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/economy/market/analyze", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Market analysis completed successfully\n");
	} else {
		ShowWarning("[AI Bridge] Market analysis failed with status %d\n", status_code);
	}
}

// ============================================================================
// Faction Endpoints
// ============================================================================

// GET /ai/faction/list
HANDLER_FUNC(ai_faction_list) {
	ShowInfo("[AI Bridge] Received faction list request\n");

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/faction/list", "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Faction list retrieved successfully\n");
	} else {
		ShowWarning("[AI Bridge] Faction list retrieval failed with status %d\n", status_code);
	}
}

// POST /ai/faction/create
HANDLER_FUNC(ai_faction_create) {
	ShowInfo("[AI Bridge] Received faction creation request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/faction/create", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Faction created successfully\n");
	} else {
		ShowWarning("[AI Bridge] Faction creation failed with status %d\n", status_code);
	}
}

// POST /ai/faction/reputation/update
HANDLER_FUNC(ai_faction_reputation_update) {
	ShowInfo("[AI Bridge] Received faction reputation update request\n");

	// Get request body
	std::string request_body = req.body;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request("/ai/faction/reputation/update", "POST", request_body, response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Faction reputation updated successfully\n");
	} else {
		ShowWarning("[AI Bridge] Faction reputation update failed with status %d\n", status_code);
	}
}

// GET /ai/faction/reputation/:player_id
HANDLER_FUNC(ai_faction_reputation_get) {
	ShowInfo("[AI Bridge] Received faction reputation get request\n");

	// Extract player ID from path
	std::string player_id = AIBridge::extract_path_param(req.path, "/ai/faction/reputation/");

	if (player_id.empty()) {
		ShowError("[AI Bridge] Missing player ID in path\n");
		res.status = 400;
		res.set_content("{\"error\": \"Missing player ID\"}", "application/json");
		return;
	}

	ShowDebug("[AI Bridge] Getting faction reputation for player ID: %s\n", player_id.c_str());

	// Build endpoint with player ID
	std::string endpoint = "/ai/faction/reputation/" + player_id;

	// Forward to AI service
	std::string response_body;
	int status_code;
	bool success = AIBridge::make_ai_request(endpoint, "GET", "", response_body, status_code);

	// Set response
	res.status = status_code;
	res.set_content(response_body, "application/json");

	if (success) {
		ShowInfo("[AI Bridge] Faction reputation retrieved successfully for player %s\n", player_id.c_str());
	} else {
		ShowWarning("[AI Bridge] Faction reputation retrieval failed with status %d\n", status_code);
	}
}

