// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "ai_dialogue_state.hpp"

#include <algorithm>

#include <common/showmsg.hpp>
#include <common/timer.hpp>

/**
 * Constructor
 */
AIDialogueStateManager::AIDialogueStateManager() 
    : cooldown_ms(5000)                 // 5 seconds default
    , rate_limit_window_ms(60000)       // 1 minute default
    , max_requests_per_window(10)       // 10 requests per minute default
    , state_cleanup_interval(300000)    // 5 minutes default
    , last_cleanup_time(0)
{
    ShowStatus("AI Dialogue State Manager: Initialized (cooldown=%dms, rate_limit=%d/%dms)\n", 
               cooldown_ms, max_requests_per_window, rate_limit_window_ms);
}

/**
 * Destructor
 */
AIDialogueStateManager::~AIDialogueStateManager() {
    clear_all_states();
    ShowStatus("AI Dialogue State Manager: Shutdown complete\n");
}

/**
 * Set cooldown duration
 */
void AIDialogueStateManager::set_cooldown(int32 ms) {
    cooldown_ms = ms;
    ShowInfo("AI Dialogue State Manager: Cooldown set to %dms\n", ms);
}

/**
 * Set rate limit parameters
 */
void AIDialogueStateManager::set_rate_limit(int32 max_requests, int32 window_ms) {
    max_requests_per_window = max_requests;
    rate_limit_window_ms = window_ms;
    ShowInfo("AI Dialogue State Manager: Rate limit set to %d requests per %dms\n", 
             max_requests, window_ms);
}

/**
 * Cleanup old request history entries
 */
void AIDialogueStateManager::cleanup_request_history(PlayerAIState& state, t_tick current_time) {
    t_tick cutoff_time = current_time - rate_limit_window_ms;
    
    while (!state.request_history.empty() && state.request_history.front() < cutoff_time) {
        state.request_history.pop_front();
    }
}

/**
 * Check if player is rate limited
 */
bool AIDialogueStateManager::is_rate_limited(const PlayerAIState& state, t_tick current_time) {
    // Count requests within the rate limit window
    t_tick cutoff_time = current_time - rate_limit_window_ms;
    int32 recent_requests = 0;
    
    for (t_tick request_time : state.request_history) {
        if (request_time >= cutoff_time) {
            recent_requests++;
        }
    }
    
    return recent_requests >= max_requests_per_window;
}

/**
 * Check if player can make a new AI dialogue request
 * @param char_id Player's character ID
 * @return true if player can make request, false if blocked by cooldown/rate limit
 */
bool AIDialogueStateManager::can_make_request(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    
    t_tick current_time = gettick();
    
    // Get or create player state
    PlayerAIState& state = player_states[char_id];
    
    // Check if player has pending request
    if (state.has_pending_request) {
        ShowDebug("AI Dialogue State: char_id=%u blocked - pending request\n", char_id);
        return false;
    }
    
    // Check cooldown
    if (current_time < state.cooldown_until) {
        int32 remaining = (int32)(state.cooldown_until - current_time);
        ShowDebug("AI Dialogue State: char_id=%u blocked - cooldown (%dms remaining)\n", 
                  char_id, remaining);
        return false;
    }
    
    // Cleanup old request history
    cleanup_request_history(state, current_time);
    
    // Check rate limit
    if (is_rate_limited(state, current_time)) {
        ShowDebug("AI Dialogue State: char_id=%u blocked - rate limited (%d requests in last %dms)\n", 
                  char_id, max_requests_per_window, rate_limit_window_ms);
        return false;
    }
    
    return true;
}

/**
 * Mark that a request has been sent for this player
 */
void AIDialogueStateManager::mark_request_sent(uint32 char_id, uint32 npc_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    
    t_tick current_time = gettick();
    PlayerAIState& state = player_states[char_id];
    
    state.has_pending_request = true;
    state.last_request_time = current_time;
    state.current_npc_id = npc_id;
    state.request_history.push_back(current_time);
    state.total_requests++;
    
    ShowDebug("AI Dialogue State: char_id=%u request sent (total: %d)\n", 
              char_id, state.total_requests);
}

/**
 * Mark that a request has been completed (response received)
 */
void AIDialogueStateManager::mark_request_completed(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    
    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return;
    }
    
    PlayerAIState& state = it->second;
    state.has_pending_request = false;
    state.current_npc_id = 0;
    state.cooldown_until = gettick() + cooldown_ms;
    
    ShowDebug("AI Dialogue State: char_id=%u request completed (cooldown until +%dms)\n", 
              char_id, cooldown_ms);
}

/**
 * Mark that a request has failed
 */
void AIDialogueStateManager::mark_request_failed(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    
    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return;
    }
    
    PlayerAIState& state = it->second;
    state.has_pending_request = false;
    state.current_npc_id = 0;
    state.failed_requests++;
    state.cooldown_until = gettick() + cooldown_ms;
    
    ShowDebug("AI Dialogue State: char_id=%u request failed (total failures: %d)\n",
              char_id, state.failed_requests);
}

/**
 * Apply custom cooldown to a player
 */
void AIDialogueStateManager::apply_cooldown(uint32 char_id, int32 custom_cooldown_ms) {
    std::lock_guard<std::mutex> lock(state_mutex);

    PlayerAIState& state = player_states[char_id];
    state.cooldown_until = gettick() + custom_cooldown_ms;

    ShowDebug("AI Dialogue State: char_id=%u custom cooldown applied (%dms)\n",
              char_id, custom_cooldown_ms);
}

/**
 * Cleanup old player states (called periodically)
 */
void AIDialogueStateManager::cleanup_old_states() {
    std::lock_guard<std::mutex> lock(state_mutex);

    t_tick current_time = gettick();

    // Only cleanup if enough time has passed
    if (current_time - last_cleanup_time < state_cleanup_interval) {
        return;
    }

    last_cleanup_time = current_time;

    // Remove states for players who haven't made requests in a long time
    t_tick cutoff_time = current_time - (rate_limit_window_ms * 5); // 5x rate limit window

    auto it = player_states.begin();
    int32 removed_count = 0;

    while (it != player_states.end()) {
        const PlayerAIState& state = it->second;

        // Keep state if player has pending request or recent activity
        if (state.has_pending_request || state.last_request_time > cutoff_time) {
            ++it;
        } else {
            it = player_states.erase(it);
            removed_count++;
        }
    }

    if (removed_count > 0) {
        ShowInfo("AI Dialogue State: Cleaned up %d old player states\n", removed_count);
    }
}

/**
 * Clear state for a specific player
 */
void AIDialogueStateManager::clear_player_state(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);
    player_states.erase(char_id);
    ShowDebug("AI Dialogue State: Cleared state for char_id=%u\n", char_id);
}

/**
 * Clear all player states
 */
void AIDialogueStateManager::clear_all_states() {
    std::lock_guard<std::mutex> lock(state_mutex);
    int32 count = (int32)player_states.size();
    player_states.clear();
    ShowStatus("AI Dialogue State: Cleared all states (%d players)\n", count);
}

/**
 * Get count of active player states
 */
int32 AIDialogueStateManager::get_active_states_count() {
    std::lock_guard<std::mutex> lock(state_mutex);
    return (int32)player_states.size();
}

/**
 * Get player state (for debugging/monitoring)
 */
bool AIDialogueStateManager::get_player_state(uint32 char_id, PlayerAIState& out_state) {
    std::lock_guard<std::mutex> lock(state_mutex);

    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return false;
    }

    out_state = it->second;
    return true;
}

/**
 * Check if player is on cooldown
 */
bool AIDialogueStateManager::is_player_on_cooldown(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);

    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return false;
    }

    return gettick() < it->second.cooldown_until;
}

/**
 * Check if player has pending request
 */
bool AIDialogueStateManager::has_pending_request(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);

    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return false;
    }

    return it->second.has_pending_request;
}

/**
 * Get remaining cooldown time in milliseconds
 */
int32 AIDialogueStateManager::get_remaining_cooldown(uint32 char_id) {
    std::lock_guard<std::mutex> lock(state_mutex);

    auto it = player_states.find(char_id);
    if (it == player_states.end()) {
        return 0;
    }

    t_tick current_time = gettick();
    if (current_time >= it->second.cooldown_until) {
        return 0;
    }

    return (int32)(it->second.cooldown_until - current_time);
}

