#pragma once
/**
 * aiworld_native_commands.hpp
 * Native Script Commands for rAthena AI-World Integration
 *
 * Implements Phase 1 priority commands from NATIVE_COMMAND_ARCHITECTURE.md:
 * - ai_npc_dialogue: AI-driven dialogue generation (async recommended)
 * - ai_npc_decide_action: AI decision-making for NPC actions
 * - ai_npc_get_emotion: Query NPC emotional state
 * - ai_request_async: Queue async operations with callbacks
 * - ai_check_result: Check async operation status/result
 *
 * Architecture: ZeroMQ IPC → Python AI Service
 * Performance: < 50ms (queries), 500-3000ms (LLM operations)
 */

#include "../common/cbasetypes.hpp"  // For int32, int64, etc.
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>
#include "aiworld_native_api.hpp"
#include "aiworld_errors.hpp"

// Forward declarations for rAthena script engine
struct script_state;

namespace aiworld {

/**
 * Async Request Tracker
 * Manages pending async operations and their callbacks
 */
struct AsyncRequest {
    std::string request_id;
    std::string operation_type;
    std::string npc_id;
    std::string callback_label;
    std::chrono::steady_clock::time_point start_time;
    int timeout_ms;
    bool completed;
    nlohmann::json result;
    int error_code;
    std::string error_message;
    
    AsyncRequest()
        : start_time(std::chrono::steady_clock::now()),
          timeout_ms(5000),
          completed(false),
          error_code(0) {}
};

/**
 * Async Request Manager (Singleton)
 * Thread-safe storage and retrieval of async operation status
 */
class AsyncRequestManager {
public:
    static AsyncRequestManager& getInstance();
    
    std::string createRequest(const std::string& operation_type,
                             const std::string& npc_id,
                             const std::string& callback_label,
                             int timeout_ms);
    
    bool updateRequest(const std::string& request_id,
                      const nlohmann::json& result,
                      int error_code = 0,
                      const std::string& error_message = "");
    
    AsyncRequest* getRequest(const std::string& request_id);
    
    void cleanup(int max_age_seconds = 300);
    
private:
    AsyncRequestManager() = default;
    std::unordered_map<std::string, AsyncRequest> requests_;
    std::mutex mutex_;
    std::atomic<uint64_t> request_counter_{0};
};

} // namespace aiworld

/**
 * BUILDIN Command Declarations
 * These are the actual script command implementations
 * Using explicit function signatures since BUILDIN_FUNC macro is not yet defined
 */
extern "C" {
    // ai_npc_dialogue(npc_id$, player_id, message$, {options})
    // Returns: JSON string with dialogue response or error
    int32 buildin_ai_npc_dialogue(struct script_state* st);

    // ai_npc_decide_action(npc_id$, context$)
    // Returns: JSON string with action decision
    int32 buildin_ai_npc_decide_action(struct script_state* st);

    // ai_npc_get_emotion(npc_id$)
    // Returns: JSON string with emotion state
    int32 buildin_ai_npc_get_emotion(struct script_state* st);

    // ai_request_async(request_type$, data$, callback_label$, {timeout})
    // Returns: request_id string for tracking
    int32 buildin_ai_request_async(struct script_state* st);

    // ai_check_result(request_id)
    // Returns: JSON string with status and result
    int32 buildin_ai_check_result(struct script_state* st);
}