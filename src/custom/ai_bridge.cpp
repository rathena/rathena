// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_bridge.hpp"
#include <common/showmsg.hpp>
#include <sstream>
#include <chrono>

// httplib for HTTP client
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../../3rdparty/httplib/httplib.h"

// JSON library
#include "../../3rdparty/json/include/nlohmann/json.hpp"
using json = nlohmann::json;

// AI Service Configuration (from environment or config)
static const char* AI_SERVICE_HOST = "127.0.0.1";
static const int AI_SERVICE_PORT = 8000;
static const int HTTP_TIMEOUT_SEC = 5;

// === Singleton Instance ===

AIBridge& AIBridge::instance() noexcept {
    static AIBridge bridge;
    return bridge;
}

// === Private Helper: HTTP POST Request ===

/**
 * @brief Make HTTP POST request to AI service with timeout and error handling
 * @param endpoint API endpoint path (e.g., "/api/npc/dialogue")
 * @param json_body JSON request body
 * @return JSON response string or empty string on error
 */
static std::string http_post_json(const std::string& endpoint, const std::string& json_body) noexcept {
    try {
        httplib::Client client(AI_SERVICE_HOST, AI_SERVICE_PORT);
        client.set_connection_timeout(HTTP_TIMEOUT_SEC, 0); // 5 seconds
        client.set_read_timeout(HTTP_TIMEOUT_SEC, 0);
        client.set_write_timeout(HTTP_TIMEOUT_SEC, 0);
        
        httplib::Headers headers = {
            {"Content-Type", "application/json"},
            {"Accept", "application/json"}
        };
        
        ShowInfo("[AIBridge] POST %s:%d%s - Body: %s\n",
                 AI_SERVICE_HOST, AI_SERVICE_PORT, endpoint.c_str(), json_body.c_str());
        
        auto res = client.Post(endpoint.c_str(), headers, json_body, "application/json");
        
        if (!res) {
            ShowError("[AIBridge] HTTP request failed: Connection error to %s:%d%s\n",
                     AI_SERVICE_HOST, AI_SERVICE_PORT, endpoint.c_str());
            return "";
        }
        
        if (res->status != 200) {
            ShowError("[AIBridge] HTTP request failed: Status %d for %s\n",
                     res->status, endpoint.c_str());
            ShowError("[AIBridge] Response: %s\n", res->body.c_str());
            return "";
        }
        
        ShowInfo("[AIBridge] Response: %s\n", res->body.c_str());
        return res->body;
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] HTTP exception: %s\n", e.what());
        return "";
    }
}

// === Initialization & Shutdown ===

bool AIBridge::initialize() noexcept {
    std::lock_guard<std::mutex> lock(m_init_mutex);
    
    if (m_initialized.load(std::memory_order_acquire)) {
        ShowWarning("[AIBridge] Already initialized\n");
        return true;
    }
    
    ShowInfo("[AIBridge] Initializing AI Bridge (HTTP mode)...\n");
    ShowInfo("[AIBridge] AI Service: http://%s:%d\n", AI_SERVICE_HOST, AI_SERVICE_PORT);
    
    // Test connection to AI service
    try {
        httplib::Client client(AI_SERVICE_HOST, AI_SERVICE_PORT);
        client.set_connection_timeout(2, 0); // 2 second quick test
        
        auto res = client.Get("/health");
        if (!res || res->status != 200) {
            ShowWarning("[AIBridge] AI service health check failed - service may be offline\n");
            ShowWarning("[AIBridge] Will continue initialization but AI calls may fail\n");
        } else {
            ShowInfo("[AIBridge] AI service health check: OK\n");
        }
    } catch (const std::exception& e) {
        ShowWarning("[AIBridge] AI service test failed: %s\n", e.what());
        ShowWarning("[AIBridge] Continuing initialization - AI calls will use fallback responses\n");
    }
    
    m_initialized.store(true, std::memory_order_release);
    ShowInfo("[AIBridge] AI Bridge initialized successfully\n");
    return true;
}

void AIBridge::shutdown() noexcept {
    std::lock_guard<std::mutex> lock(m_init_mutex);
    
    if (!m_initialized.load(std::memory_order_acquire)) {
        return;
    }
    
    ShowInfo("[AIBridge] Shutting down AI Bridge...\n");
    
    m_initialized.store(false, std::memory_order_release);
    ShowInfo("[AIBridge] AI Bridge shutdown complete\n");
}

// === AI Agent Functions ===

std::string AIBridge::generate_dialogue(int npc_id, const std::string& player_message) noexcept {
    if (!is_initialized()) {
        ShowError("[AIBridge] generate_dialogue() called but bridge not initialized\n");
        return "{\"response\":\"AI bridge not initialized\"}";
    }
    
    try {
        json request = {
            {"npc_id", npc_id},
            {"player_message", player_message},
            {"context", {}}
        };
        
        std::string response = http_post_json("/api/npc/dialogue", request.dump());
        
        if (response.empty()) {
            ShowWarning("[AIBridge] generate_dialogue() failed, using fallback\n");
            return "{\"response\":\"I'm having trouble connecting to my thoughts right now...\"}";
        }
        
        // Validate JSON
        try {
            json::parse(response);
            return response;
        } catch (const json::exception& e) {
            ShowError("[AIBridge] Invalid JSON response: %s\n", e.what());
            return "{\"response\":\"Invalid response format\"}";
        }
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] generate_dialogue() exception: %s\n", e.what());
        return "{\"response\":\"An error occurred\"}";
    }
}

std::string AIBridge::get_npc_decision(int npc_id, const std::string& context_json) noexcept {
    if (!is_initialized()) {
        ShowError("[AIBridge] get_npc_decision() called but bridge not initialized\n");
        return "{\"action\":\"idle\"}";
    }
    
    try {
        json request = {
            {"npc_id", npc_id},
            {"context", json::parse(context_json)}
        };
        
        std::string response = http_post_json("/api/npc/decision", request.dump());
        
        if (response.empty()) {
            ShowWarning("[AIBridge] get_npc_decision() failed, using fallback\n");
            return "{\"action\":\"idle\"}";
        }
        
        // Validate JSON
        try {
            json::parse(response);
            return response;
        } catch (const json::exception& e) {
            ShowError("[AIBridge] Invalid JSON response: %s\n", e.what());
            return "{\"action\":\"idle\"}";
        }
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] get_npc_decision() exception: %s\n", e.what());
        return "{\"action\":\"idle\"}";
    }
}

bool AIBridge::store_memory(int npc_id, const std::string& memory_data) noexcept {
    if (!is_initialized()) {
        ShowError("[AIBridge] store_memory() called but bridge not initialized\n");
        return false;
    }
    
    try {
        json request = {
            {"npc_id", npc_id},
            {"memory_data", memory_data}
        };
        
        std::string response = http_post_json("/api/npc/memory", request.dump());
        
        if (response.empty()) {
            ShowWarning("[AIBridge] store_memory() failed\n");
            return false;
        }
        
        // Parse response to check success
        try {
            json response_json = json::parse(response);
            if (response_json.contains("success")) {
                return response_json["success"].get<bool>();
            }
            return true; // Assume success if no explicit field
        } catch (const json::exception& e) {
            ShowError("[AIBridge] Invalid JSON response: %s\n", e.what());
            return false;
        }
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] store_memory() exception: %s\n", e.what());
        return false;
    }
}

std::string AIBridge::generate_quest(int npc_id, int player_level) noexcept {
    if (!is_initialized()) {
        ShowError("[AIBridge] generate_quest() called but bridge not initialized\n");
        return "{\"quest_id\":0}";
    }
    
    try {
        json request = {
            {"npc_id", npc_id},
            {"player_level", player_level}
        };
        
        std::string response = http_post_json("/api/npc/quest", request.dump());
        
        if (response.empty()) {
            ShowWarning("[AIBridge] generate_quest() failed, using fallback\n");
            return "{\"quest_id\":0,\"title\":\"No quest available\"}";
        }
        
        // Validate JSON
        try {
            json::parse(response);
            return response;
        } catch (const json::exception& e) {
            ShowError("[AIBridge] Invalid JSON response: %s\n", e.what());
            return "{\"quest_id\":0}";
        }
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] generate_quest() exception: %s\n", e.what());
        return "{\"quest_id\":0}";
    }
}

// === Script Command Implementations ===

BUILDIN_FUNC(ai_dialogue) {
    // Get parameters: ai_dialogue(<npc_id>, "<player_message>")
    int npc_id = script_getnum(st, 2);
    const char* player_message = script_getstr(st, 3);
    
    if (!player_message) {
        ShowError("[AIBridge] ai_dialogue: Invalid player_message parameter\n");
        script_pushstrcopy(st, "{\"response\":\"Invalid parameter\"}");
        return SCRIPT_CMD_FAILURE;
    }
    
    std::string response = AIBridge::instance().generate_dialogue(npc_id, player_message);
    script_pushstrcopy(st, response.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_decision) {
    // Get parameters: ai_decision(<npc_id>, "<context_json>")
    int npc_id = script_getnum(st, 2);
    const char* context_json = script_getstr(st, 3);
    
    if (!context_json) {
        ShowError("[AIBridge] ai_decision: Invalid context_json parameter\n");
        script_pushstrcopy(st, "{\"action\":\"idle\"}");
        return SCRIPT_CMD_FAILURE;
    }
    
    std::string decision = AIBridge::instance().get_npc_decision(npc_id, context_json);
    script_pushstrcopy(st, decision.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_remember) {
    // Get parameters: ai_remember(<npc_id>, "<memory_data>")
    int npc_id = script_getnum(st, 2);
    const char* memory_data = script_getstr(st, 3);
    
    if (!memory_data) {
        ShowError("[AIBridge] ai_remember: Invalid memory_data parameter\n");
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    bool success = AIBridge::instance().store_memory(npc_id, memory_data);
    script_pushint(st, success ? 1 : 0);
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_quest) {
    // Get parameters: ai_quest(<npc_id>, <player_level>)
    int npc_id = script_getnum(st, 2);
    int player_level = script_getnum(st, 3);
    
    if (player_level < 1 || player_level > 999) {
        ShowError("[AIBridge] ai_quest: Invalid player_level %d\n", player_level);
        script_pushstrcopy(st, "{\"quest_id\":0}");
        return SCRIPT_CMD_FAILURE;
    }
    
    std::string quest_json = AIBridge::instance().generate_quest(npc_id, player_level);
    script_pushstrcopy(st, quest_json.c_str());
    
    return SCRIPT_CMD_SUCCESS;
}

BUILDIN_FUNC(ai_walk) {
    // Get parameters: ai_walk(<npc_id>, <x>, <y>, "<map>")
    int npc_id = script_getnum(st, 2);
    int x = script_getnum(st, 3);
    int y = script_getnum(st, 4);
    const char* map_name = script_getstr(st, 5);
    
    if (!map_name) {
        ShowError("[AIBridge] ai_walk: Invalid map_name parameter\n");
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    if (x < 0 || y < 0 || x > 999 || y > 999) {
        ShowError("[AIBridge] ai_walk: Invalid coordinates (%d, %d)\n", x, y);
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
    
    try {
        // Request AI-aware pathfinding decision
        json request = {
            {"npc_id", npc_id},
            {"x", x},
            {"y", y},
            {"map", map_name}
        };
        
        std::string response = http_post_json("/api/npc/walk", request.dump());
        
        if (response.empty()) {
            ShowWarning("[AIBridge] ai_walk() failed\n");
            script_pushint(st, 0);
            return SCRIPT_CMD_SUCCESS;
        }
        
        // Parse response
        json response_json = json::parse(response);
        bool success = response_json.value("success", false);
        
        script_pushint(st, success ? 1 : 0);
        return SCRIPT_CMD_SUCCESS;
        
    } catch (const std::exception& e) {
        ShowError("[AIBridge] ai_walk() exception: %s\n", e.what());
        script_pushint(st, 0);
        return SCRIPT_CMD_FAILURE;
    }
}