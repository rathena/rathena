// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AI_DIALOGUE_STATE_HPP
#define AI_DIALOGUE_STATE_HPP

#include <unordered_map>
#include <mutex>
#include <deque>

#include <common/cbasetypes.hpp>
#include <common/timer.hpp>

/**
 * Per-Player AI Dialogue State
 * Tracks cooldowns, rate limits, and pending requests
 */
struct PlayerAIState {
    bool has_pending_request;           // Currently waiting for AI response
    t_tick last_request_time;           // Last request timestamp
    t_tick cooldown_until;              // Cooldown expiration time
    uint32 current_npc_id;              // NPC ID of pending request
    std::deque<t_tick> request_history; // Request timestamps for rate limiting
    int32 total_requests;               // Total requests made (lifetime)
    int32 failed_requests;              // Failed request count
};

/**
 * AI Dialogue State Manager
 * Manages per-player state for cooldowns, rate limiting, and spam prevention
 */
class AIDialogueStateManager {
private:
    std::unordered_map<uint32, PlayerAIState> player_states;
    std::mutex state_mutex;
    
    // Configuration (loaded from config file)
    int32 cooldown_ms;              // Cooldown between requests (default: 5000ms)
    int32 rate_limit_window_ms;     // Rate limit window (default: 60000ms = 1 minute)
    int32 max_requests_per_window;  // Max requests per window (default: 10)
    int32 state_cleanup_interval;   // How often to cleanup old states (default: 300000ms = 5 min)
    
    t_tick last_cleanup_time;       // Last time cleanup was performed
    
    // Helper methods
    void cleanup_request_history(PlayerAIState& state, t_tick current_time);
    bool is_rate_limited(const PlayerAIState& state, t_tick current_time);
    
public:
    AIDialogueStateManager();
    ~AIDialogueStateManager();
    
    // Configuration
    void set_cooldown(int32 ms);
    void set_rate_limit(int32 max_requests, int32 window_ms);
    
    // State management
    bool can_make_request(uint32 char_id);
    void mark_request_sent(uint32 char_id, uint32 npc_id);
    void mark_request_completed(uint32 char_id);
    void mark_request_failed(uint32 char_id);
    void apply_cooldown(uint32 char_id, int32 cooldown_ms);
    
    // Cleanup
    void cleanup_old_states();
    void clear_player_state(uint32 char_id);
    void clear_all_states();
    
    // Statistics
    int32 get_active_states_count();
    bool get_player_state(uint32 char_id, PlayerAIState& out_state);
    
    // Utility
    bool is_player_on_cooldown(uint32 char_id);
    bool has_pending_request(uint32 char_id);
    int32 get_remaining_cooldown(uint32 char_id);
};

#endif /* AI_DIALOGUE_STATE_HPP */

