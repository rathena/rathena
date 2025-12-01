/**
 * aiworld_native_commands.cpp
 * Native Script Commands Implementation for rAthena AI-World Integration
 * 
 * Implements 5 Phase 1 priority commands:
 * 1. ai_npc_dialogue - AI dialogue generation (uses ZeroMQ IPC)
 * 2. ai_npc_decide_action - AI decision making 
 * 3. ai_npc_get_emotion - Query NPC emotion
 * 4. ai_request_async - Queue async operations
 * 5. ai_check_result - Check async status
 */

#include "aiworld_native_commands.hpp"
#include "aiworld_native_api.hpp"
#include "aiworld_utils.hpp"
#include "aiworld_threadpool.hpp"
#include <sstream>
#include <iomanip>
#include <random>

// rAthena script engine headers (include dirs already configured in CMake)
#include "../map/script.hpp"
#include "../map/pc.hpp"
#include "../map/npc.hpp"
#include <common/showmsg.hpp>

namespace aiworld {

//=============================================================================
// Forward Declarations for Helper Functions
//=============================================================================

static std::string generate_request_id();
static std::string escape_json_string(const std::string& str);

//=============================================================================
// AsyncRequestManager Implementation
//=============================================================================

AsyncRequestManager& AsyncRequestManager::getInstance() {
    static AsyncRequestManager instance;
    return instance;
}

std::string AsyncRequestManager::createRequest(const std::string& operation_type,
                                               const std::string& npc_id,
                                               const std::string& callback_label,
                                               int timeout_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Generate unique request ID
    std::string request_id = generate_request_id();
    
    AsyncRequest req;
    req.request_id = request_id;
    req.operation_type = operation_type;
    req.npc_id = npc_id;
    req.callback_label = callback_label;
    req.timeout_ms = timeout_ms;
    req.start_time = std::chrono::steady_clock::now();
    req.completed = false;
    
    requests_[request_id] = req;
    return request_id;
}

bool AsyncRequestManager::updateRequest(const std::string& request_id,
                                       const nlohmann::json& result,
                                       int error_code,
                                       const std::string& error_message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = requests_.find(request_id);
    if (it == requests_.end()) {
        return false;
    }
    
    it->second.completed = true;
    it->second.result = result;
    it->second.error_code = error_code;
    it->second.error_message = error_message;
    
    return true;
}

AsyncRequest* AsyncRequestManager::getRequest(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = requests_.find(request_id);
    if (it == requests_.end()) {
        return nullptr;
    }
    
    return &(it->second);
}

void AsyncRequestManager::cleanup(int max_age_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto it = requests_.begin();
    
    while (it != requests_.end()) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.start_time).count();
        
        if (age > max_age_seconds) {
            it = requests_.erase(it);
        } else {
            ++it;
        }
    }
}

//=============================================================================
// Helper Functions (Static - Internal Use Only)
//=============================================================================

static std::string generate_request_id() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    
    std::stringstream ss;
    ss << "req_" << ms << "_" << std::hex << std::setw(8) << std::setfill('0') << dis(gen);
    return ss.str();
}

static std::string escape_json_string(const std::string& input) {
    std::string output;
    output.reserve(input.size() + 16);
    
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    output += buf;
                } else {
                    output += c;
                }
        }
    }
    
    return output;
}

} // namespace aiworld

//=============================================================================
// BUILDIN Command Implementations
//=============================================================================

/**
 * ai_npc_dialogue(npc_id$, player_id, message$, {options})
 * 
 * Generates AI-driven NPC dialogue response via ZeroMQ IPC
 * 
 * @param npc_id$ - NPC identifier string
 * @param player_id - Player character ID (integer)
 * @param message$ - Player's message string
 * @param options$ - Optional JSON string with context (default: "{}")
 * @return JSON string with response or error
 * 
 * Performance: SLOW (500-3000ms) - USE ASYNC PATTERN RECOMMENDED
 */
int32 buildin_ai_npc_dialogue(struct script_state* st) {
    using namespace aiworld;
    
    // Parameter validation
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3) || !script_hasdata(st, 4)) {
        ShowError("ai_npc_dialogue: Missing required parameters\n");
        script_pushstrcopy(st, "{\"error_code\":-101,\"error_message\":\"Missing parameters\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    const char* npc_id = script_getstr(st, 2);
    int player_id = script_getnum(st, 3);
    const char* message = script_getstr(st, 4);
    const char* options = script_hasdata(st, 5) ? script_getstr(st, 5) : "{}";
    
    // Build request payload
    try {
        nlohmann::json request_data;
        request_data["npc_id"] = npc_id;
        request_data["player_id"] = player_id;
        request_data["message"] = message;
        
        // Parse options JSON
        try {
            nlohmann::json opts = nlohmann::json::parse(options);
            request_data["context"] = opts;
        } catch (const nlohmann::json::exception&) {
            request_data["context"] = nlohmann::json::object();
        }
        
        // Send IPC request via Native API (1000ms timeout for dialogue)
        APIResult result = AIWorldNativeAPI::getInstance().handleNPCInteraction(
            npc_id, "dialogue", request_data, std::to_string(player_id), 1000);
        
        if (!result.success) {
            // Return error JSON
            nlohmann::json error_resp;
            error_resp["error_code"] = result.error_code;
            error_resp["error_message"] = result.error_message;
            script_pushstrcopy(st, error_resp.dump().c_str());
            ShowWarning("ai_npc_dialogue: %s (code: %d)\n", 
                       result.error_message.c_str(), result.error_code);
        } else {
            // Return success response
            script_pushstrcopy(st, result.data.dump().c_str());
        }
        
    } catch (const std::exception& e) {
        ShowError("ai_npc_dialogue: Exception: %s\n", e.what());
        script_pushstrcopy(st, "{\"error_code\":-500,\"error_message\":\"Internal error\"}");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_npc_decide_action(npc_id$, context$)
 * 
 * AI decides NPC's next action based on situation context
 * 
 * @param npc_id$ - NPC identifier string
 * @param context$ - JSON string with situation context
 * @return JSON string with action decision
 * 
 * Performance: SLOW (300-2000ms) - USE ASYNC PATTERN RECOMMENDED
 */
int32 buildin_ai_npc_decide_action(struct script_state* st) {
    using namespace aiworld;
    
    // Parameter validation
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3)) {
        ShowError("ai_npc_decide_action: Missing required parameters\n");
        script_pushstrcopy(st, "{\"error_code\":-101,\"error_message\":\"Missing parameters\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    const char* npc_id = script_getstr(st, 2);
    const char* context_str = script_getstr(st, 3);
    
    try {
        // Parse context JSON
        nlohmann::json context;
        try {
            context = nlohmann::json::parse(context_str);
        } catch (const nlohmann::json::exception&) {
            context = nlohmann::json::object();
            context["raw_context"] = context_str;
        }
        
        nlohmann::json request_data;
        request_data["npc_id"] = npc_id;
        request_data["situation"] = context;
        
        // Send IPC request (fast decision < 100ms timeout)
        APIResult result = AIWorldNativeAPI::getInstance().handleNPCInteraction(
            npc_id, "decide_action", request_data, "", 100);
        
        if (!result.success) {
            nlohmann::json error_resp;
            error_resp["error_code"] = result.error_code;
            error_resp["error_message"] = result.error_message;
            script_pushstrcopy(st, error_resp.dump().c_str());
            ShowWarning("ai_npc_decide_action: %s (code: %d)\n", 
                       result.error_message.c_str(), result.error_code);
        } else {
            script_pushstrcopy(st, result.data.dump().c_str());
        }
        
    } catch (const std::exception& e) {
        ShowError("ai_npc_decide_action: Exception: %s\n", e.what());
        script_pushstrcopy(st, "{\"error_code\":-500,\"error_message\":\"Internal error\"}");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_npc_get_emotion(npc_id$)
 * 
 * Retrieves current emotional state of NPC
 * 
 * @param npc_id$ - NPC identifier string
 * @return JSON string with emotion state
 * 
 * Performance: FAST (< 50ms)
 */
int32 buildin_ai_npc_get_emotion(struct script_state* st) {
    using namespace aiworld;
    
    // Parameter validation
    if (!script_hasdata(st, 2)) {
        ShowError("ai_npc_get_emotion: Missing npc_id parameter\n");
        script_pushstrcopy(st, "{\"error_code\":-101,\"error_message\":\"Missing npc_id\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    const char* npc_id = script_getstr(st, 2);
    
    try {
        // Query NPC state with emotion field filter
        std::vector<std::string> fields = {"emotion", "emotional_state"};
        APIResult result = AIWorldNativeAPI::getInstance().getNPCState(
            npc_id, fields, 50); // Very fast query (50ms)
        
        if (!result.success) {
            nlohmann::json error_resp;
            error_resp["error_code"] = result.error_code;
            error_resp["error_message"] = result.error_message;
            script_pushstrcopy(st, error_resp.dump().c_str());
            ShowWarning("ai_npc_get_emotion: %s (code: %d)\n", 
                       result.error_message.c_str(), result.error_code);
        } else {
            script_pushstrcopy(st, result.data.dump().c_str());
        }
        
    } catch (const std::exception& e) {
        ShowError("ai_npc_get_emotion: Exception: %s\n", e.what());
        script_pushstrcopy(st, "{\"error_code\":-500,\"error_message\":\"Internal error\"}");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_request_async(request_type$, data$, callback_label$, {timeout})
 * 
 * Queues an async AI operation with callback
 * 
 * @param request_type$ - Operation type: "dialogue", "decide_action", etc.
 * @param data$ - JSON string with request data (must include npc_id)
 * @param callback_label$ - NPC event label to trigger when ready
 * @param timeout - Timeout in milliseconds (default: 5000)
 * @return request_id string for tracking
 * 
 * Performance: Immediate return (non-blocking)
 */
int32 buildin_ai_request_async(struct script_state* st) {
    using namespace aiworld;
    
    // Parameter validation
    if (!script_hasdata(st, 2) || !script_hasdata(st, 3) || !script_hasdata(st, 4)) {
        ShowError("ai_request_async: Missing required parameters\n");
        script_pushstrcopy(st, "");
        return SCRIPT_CMD_SUCCESS;
    }
    
    const char* request_type = script_getstr(st, 2);
    const char* data_str = script_getstr(st, 3);
    const char* callback_label = script_getstr(st, 4);
    int timeout_ms = script_hasdata(st, 5) ? script_getnum(st, 5) : 5000;
    
    try {
        // Parse data JSON to extract npc_id
        nlohmann::json data = nlohmann::json::parse(data_str);
        std::string npc_id = data.value("npc_id", "");
        
        if (npc_id.empty()) {
            ShowError("ai_request_async: data JSON must contain 'npc_id' field\n");
            script_pushstrcopy(st, "");
            return SCRIPT_CMD_SUCCESS;
        }
        
        // Create async request
        std::string request_id = AsyncRequestManager::getInstance().createRequest(
            request_type, npc_id, callback_label, timeout_ms);
        
        ShowInfo("ai_request_async: Created async request %s (type=%s, npc=%s)\n",
                request_id.c_str(), request_type, npc_id.c_str());
        
        // Convert to std::string for lambda capture
        std::string req_type_str(request_type);
        std::string callback_str(callback_label);
        
        // Launch async IPC call in background thread pool
        ThreadPool::getInstance().submit([request_id, req_type_str, npc_id, data, callback_str, timeout_ms]() {
            try {
                APIResult result;
                
                // Route to appropriate API call based on request type
                if (req_type_str == "dialogue") {
                    int player_id = data.value("player_id", 0);
                    std::string message = data.value("message", "");
                    result = AIWorldNativeAPI::getInstance().handleNPCInteraction(
                        npc_id, "dialogue", data, std::to_string(player_id), timeout_ms);
                    
                } else if (req_type_str == "decide_action") {
                    result = AIWorldNativeAPI::getInstance().handleNPCInteraction(
                        npc_id, "decide_action", data, "", timeout_ms);
                    
                } else if (req_type_str == "get_emotion") {
                    std::vector<std::string> fields = {"emotion", "emotional_state"};
                    result = AIWorldNativeAPI::getInstance().getNPCState(npc_id, fields, timeout_ms);
                    
                } else {
                    // Generic action request
                    result = AIWorldNativeAPI::getInstance().handleNPCInteraction(
                        npc_id, req_type_str, data, "", timeout_ms);
                }
                
                // Update request with result
                AsyncRequestManager::getInstance().updateRequest(
                    request_id, result.data, result.error_code, result.error_message);
                
                // Trigger callback event if specified
                if (!callback_str.empty() && !npc_id.empty()) {
                    // Build event name: npc_name::callback_label
                    std::string event_name = npc_id + "::" + callback_str;
                    
                    // Trigger NPC event with request_id as parameter
                    // Note: This schedules the event to run on main thread
                    npc_event_do(event_name.c_str());
                    
                    ShowInfo("ai_request_async: Triggered callback %s for request %s\n",
                            event_name.c_str(), request_id.c_str());
                }
                
            } catch (const std::exception& e) {
                ShowError("ai_request_async: Exception in worker thread: %s\n", e.what());
                AsyncRequestManager::getInstance().updateRequest(
                    request_id, nlohmann::json::object(), -500,
                    std::string("Worker thread exception: ") + e.what());
            }
        });
        
        // Return request ID immediately (non-blocking)
        script_pushstrcopy(st, request_id.c_str());
        
    } catch (const nlohmann::json::exception& e) {
        ShowError("ai_request_async: JSON parse error: %s\n", e.what());
        script_pushstrcopy(st, "");
    } catch (const std::exception& e) {
        ShowError("ai_request_async: Exception: %s\n", e.what());
        script_pushstrcopy(st, "");
    }
    
    return SCRIPT_CMD_SUCCESS;
}

/**
 * ai_check_result(request_id)
 * 
 * Checks if async operation is complete and retrieves result
 * 
 * @param request_id - Request identifier from ai_request_async
 * @return JSON string with status and result
 * 
 * Status values:
 * - "pending" - Still processing
 * - "complete" - Finished successfully
 * - "error" - Failed with error
 * - "timeout" - Operation timed out
 * - "not_found" - Invalid request_id
 */
int32 buildin_ai_check_result(struct script_state* st) {
    using namespace aiworld;
    
    // Parameter validation
    if (!script_hasdata(st, 2)) {
        ShowError("ai_check_result: Missing request_id parameter\n");
        script_pushstrcopy(st, "{\"status\":\"error\",\"error_message\":\"Missing request_id\"}");
        return SCRIPT_CMD_SUCCESS;
    }
    
    const char* request_id = script_getstr(st, 2);
    
    try {
        AsyncRequest* req = AsyncRequestManager::getInstance().getRequest(request_id);
        
        if (!req) {
            // Request not found
            nlohmann::json resp;
            resp["status"] = "not_found";
            resp["error_message"] = "Invalid request_id";
            script_pushstrcopy(st, resp.dump().c_str());
            return SCRIPT_CMD_SUCCESS;
        }
        
        // Calculate elapsed time
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - req->start_time).count();
        
        nlohmann::json resp;
        resp["elapsed_ms"] = elapsed;
        
        // Check timeout
        if (!req->completed && elapsed > req->timeout_ms) {
            resp["status"] = "timeout";
            resp["error_message"] = "Operation timed out";
        }
        // Check completion
        else if (req->completed) {
            if (req->error_code != 0) {
                resp["status"] = "error";
                resp["error_code"] = req->error_code;
                resp["error_message"] = req->error_message;
            } else {
                resp["status"] = "complete";
                resp["result"] = req->result;
            }
        }
        // Still pending
        else {
            resp["status"] = "pending";
        }
        
        script_pushstrcopy(st, resp.dump().c_str());
        
    } catch (const std::exception& e) {
        ShowError("ai_check_result: Exception: %s\n", e.what());
        script_pushstrcopy(st, "{\"status\":\"error\",\"error_message\":\"Internal error\"}");
    }
    
    return SCRIPT_CMD_SUCCESS;
}