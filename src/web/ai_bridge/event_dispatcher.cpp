// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "event_dispatcher.hpp"

#include <sstream>

#include <common/showmsg.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace AIBridge {

// ============================================================================
// GameEvent Implementation
// ============================================================================

std::string GameEvent::to_json() const {
    json j;
    
    // Convert event type to string
    const char* type_str = "unknown";
    switch (type) {
        case GameEventType::PLAYER_LOGIN: type_str = "player_login"; break;
        case GameEventType::PLAYER_LOGOUT: type_str = "player_logout"; break;
        case GameEventType::NPC_INTERACTION: type_str = "npc_interaction"; break;
        case GameEventType::MONSTER_KILL: type_str = "monster_kill"; break;
        case GameEventType::MVP_KILL: type_str = "mvp_kill"; break;
        case GameEventType::DAILY_RESET: type_str = "daily_reset"; break;
        case GameEventType::MAP_CHANGE: type_str = "map_change"; break;
        case GameEventType::ITEM_DROP: type_str = "item_drop"; break;
        case GameEventType::QUEST_COMPLETE: type_str = "quest_complete"; break;
        case GameEventType::TRADE_COMPLETE: type_str = "trade_complete"; break;
        case GameEventType::CUSTOM: type_str = "custom"; break;
    }
    
    j["event_type"] = type_str;
    j["timestamp"] = timestamp_ms;
    
    if (player_id > 0) {
        j["player_id"] = player_id;
    }
    
    if (!map_name.empty()) {
        j["map"] = map_name;
    }
    
    // Convert priority to string
    const char* priority_str = "normal";
    switch (priority) {
        case EventPriority::LOW: priority_str = "low"; break;
        case EventPriority::NORMAL: priority_str = "normal"; break;
        case EventPriority::HIGH: priority_str = "high"; break;
        case EventPriority::URGENT: priority_str = "urgent"; break;
    }
    j["priority"] = priority_str;
    
    // Parse and merge payload if it's JSON
    if (!payload.empty()) {
        try {
            json payload_json = json::parse(payload);
            j["payload"] = payload_json;
        } catch (...) {
            // If not valid JSON, store as string
            j["payload"] = payload;
        }
    }
    
    return j.dump();
}

// ============================================================================
// EventDispatcher Implementation
// ============================================================================

EventDispatcher::EventDispatcher(
    std::shared_ptr<HTTPClient> http_client,
    size_t batch_size,
    int64_t batch_interval_ms,
    size_t queue_size
)
    : http_client_(http_client),
      batch_size_(batch_size),
      batch_interval_ms_(batch_interval_ms),
      queue_size_(queue_size),
      dlq_path_("log/ai_bridge_dlq.log") {
}

EventDispatcher::~EventDispatcher() {
    stop();
}

bool EventDispatcher::start() {
    if (running_.load()) {
        ShowWarning("[EventDispatcher] Already running\n");
        return false;
    }
    
    should_stop_ = false;
    running_ = true;
    
    dispatch_thread_ = std::make_unique<std::thread>(&EventDispatcher::dispatch_thread_func, this);
    
    ShowInfo("[EventDispatcher] Started (batch_size=%zu, interval=%lldms, queue_size=%zu)\n",
             batch_size_, batch_interval_ms_, queue_size_);
    
    return true;
}

void EventDispatcher::stop() {
    if (!running_.load()) {
        return;
    }
    
    ShowInfo("[EventDispatcher] Stopping...\n");
    
    should_stop_ = true;
    queue_cv_.notify_all();
    
    if (dispatch_thread_ && dispatch_thread_->joinable()) {
        dispatch_thread_->join();
    }
    
    running_ = false;
    
    // Flush any remaining events
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!queue_.empty()) {
        ShowWarning("[EventDispatcher] %zu events still in queue, saving to DLQ\n", queue_.size());
        std::vector<GameEvent> remaining(queue_.begin(), queue_.end());
        save_to_dlq(remaining);
    }
    
    ShowInfo("[EventDispatcher] Stopped\n");
}

bool EventDispatcher::queue_event(const GameEvent& event) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    
    // Check if queue is full
    if (queue_.size() >= queue_size_) {
        total_dropped_++;
        ShowWarning("[EventDispatcher] Queue full (%zu events), dropping event\n", queue_.size());
        return false;
    }
    
    // Add to queue based on priority (simple: urgent events go to front)
    if (event.priority == EventPriority::URGENT) {
        queue_.push_front(event);
    } else {
        queue_.push_back(event);
    }
    
    total_queued_++;
    
    // Notify dispatcher thread
    queue_cv_.notify_one();
    
    return true;
}

size_t EventDispatcher::queue_events(const std::vector<GameEvent>& events) {
    size_t queued = 0;
    for (const auto& event : events) {
        if (queue_event(event)) {
            queued++;
        }
    }
    return queued;
}

void EventDispatcher::dispatch_thread_func() {
    auto last_dispatch = std::chrono::steady_clock::now();
    
    while (!should_stop_.load()) {
        std::vector<GameEvent> batch;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Wait for events or timeout
            queue_cv_.wait_for(lock, std::chrono::milliseconds(batch_interval_ms_), [this] {
                return should_stop_.load() || queue_.size() >= batch_size_;
            });
            
            if (should_stop_.load() && queue_.empty()) {
                break;
            }
            
            // Check if we should dispatch
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_dispatch).count();
            
            bool should_dispatch = queue_.size() >= batch_size_ || 
                                  elapsed_ms >= batch_interval_ms_ ||
                                  should_stop_.load();
            
            if (!should_dispatch || queue_.empty()) {
                continue;
            }
            
            // Build batch (take up to batch_size events)
            size_t batch_count = std::min(queue_.size(), batch_size_);
            batch.reserve(batch_count);
            
            for (size_t i = 0; i < batch_count; ++i) {
                batch.push_back(std::move(queue_.front()));
                queue_.pop_front();
            }
            
            last_dispatch = now;
        }
        
        // Dispatch batch (outside lock)
        if (!batch.empty()) {
            if (!dispatch_batch(batch)) {
                // Save failed batch to DLQ
                save_to_dlq(batch);
            }
        }
    }
}

bool EventDispatcher::dispatch_batch(const std::vector<GameEvent>& batch) {
    if (batch.empty()) {
        return true;
    }
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Build batch JSON
    std::string json_body = build_batch_json(batch);
    
    ShowDebug("[EventDispatcher] Dispatching batch of %zu events\n", batch.size());
    
    // Send to AI service
    HTTPResponse response = http_client_->post("/api/v1/events/dispatch", json_body);
    
    auto end_time = std::chrono::steady_clock::now();
    int64_t latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    ).count();
    
    if (response.success) {
        total_dispatched_ += batch.size();
        total_batches_++;
        total_dispatch_latency_ms_ += latency_ms;
        
        ShowInfo("[EventDispatcher] Dispatched %zu events in %lldms (status=%d)\n",
                 batch.size(), latency_ms, response.status_code);
        return true;
    } else {
        total_failed_ += batch.size();
        ShowError("[EventDispatcher] Batch dispatch failed: %s\n", response.error_message.c_str());
        return false;
    }
}

std::string EventDispatcher::build_batch_json(const std::vector<GameEvent>& batch) const {
    json j;
    j["events"] = json::array();
    
    for (const auto& event : batch) {
        try {
            j["events"].push_back(json::parse(event.to_json()));
        } catch (const std::exception& e) {
            ShowError("[EventDispatcher] Failed to parse event JSON: %s\n", e.what());
        }
    }
    
    return j.dump();
}

void EventDispatcher::save_to_dlq(const std::vector<GameEvent>& events) {
    std::lock_guard<std::mutex> lock(dlq_mutex_);
    
    try {
        std::ofstream dlq_file(dlq_path_, std::ios::app);
        if (!dlq_file.is_open()) {
            ShowError("[EventDispatcher] Failed to open DLQ file: %s\n", dlq_path_.c_str());
            return;
        }
        
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);
        
        dlq_file << "# Failed batch at " << std::ctime(&timestamp);
        dlq_file << build_batch_json(events) << "\n\n";
        
        dlq_file.close();
        
        ShowWarning("[EventDispatcher] Saved %zu events to DLQ: %s\n", events.size(), dlq_path_.c_str());
    } catch (const std::exception& e) {
        ShowError("[EventDispatcher] Exception writing to DLQ: %s\n", e.what());
    }
}

EventQueueStats EventDispatcher::get_stats() const {
    EventQueueStats stats;
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stats.current_size = queue_.size();
    }
    
    stats.total_queued = total_queued_.load();
    stats.total_dispatched = total_dispatched_.load();
    stats.total_failed = total_failed_.load();
    stats.total_batches = total_batches_.load();
    
    if (stats.total_batches > 0) {
        stats.avg_batch_size = static_cast<double>(stats.total_dispatched) / stats.total_batches;
        stats.avg_dispatch_latency_ms = static_cast<double>(total_dispatch_latency_ms_) / stats.total_batches;
    }
    
    return stats;
}

size_t EventDispatcher::get_queue_size() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_.size();
}

bool EventDispatcher::is_running() const {
    return running_.load();
}

void EventDispatcher::flush() {
    std::vector<GameEvent> batch;
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (queue_.empty()) {
            return;
        }
        
        batch.assign(queue_.begin(), queue_.end());
        queue_.clear();
    }
    
    if (!batch.empty()) {
        dispatch_batch(batch);
    }
}

void EventDispatcher::set_dlq_path(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(dlq_mutex_);
    dlq_path_ = filepath;
}

// ============================================================================
// EventFactory Implementation
// ============================================================================

namespace EventFactory {

GameEvent create_player_login(uint32 player_id, const std::string& map_name) {
    GameEvent event;
    event.type = GameEventType::PLAYER_LOGIN;
    event.priority = EventPriority::NORMAL;
    event.player_id = player_id;
    event.map_name = map_name;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["action"] = "login";
    event.payload = j.dump();
    
    return event;
}

GameEvent create_player_logout(uint32 player_id) {
    GameEvent event;
    event.type = GameEventType::PLAYER_LOGOUT;
    event.priority = EventPriority::NORMAL;
    event.player_id = player_id;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["action"] = "logout";
    event.payload = j.dump();
    
    return event;
}

GameEvent create_monster_kill(uint32 player_id, uint32 monster_id, const std::string& map_name) {
    GameEvent event;
    event.type = GameEventType::MONSTER_KILL;
    event.priority = EventPriority::NORMAL;
    event.player_id = player_id;
    event.map_name = map_name;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["monster_id"] = monster_id;
    event.payload = j.dump();
    
    return event;
}

GameEvent create_mvp_kill(uint32 player_id, uint32 mvp_id, const std::string& map_name) {
    GameEvent event;
    event.type = GameEventType::MVP_KILL;
    event.priority = EventPriority::HIGH;
    event.player_id = player_id;
    event.map_name = map_name;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["mvp_id"] = mvp_id;
    event.payload = j.dump();
    
    return event;
}

GameEvent create_daily_reset() {
    GameEvent event;
    event.type = GameEventType::DAILY_RESET;
    event.priority = EventPriority::URGENT;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["action"] = "daily_reset";
    event.payload = j.dump();
    
    return event;
}

GameEvent create_map_change(uint32 player_id, const std::string& from_map, const std::string& to_map) {
    GameEvent event;
    event.type = GameEventType::MAP_CHANGE;
    event.priority = EventPriority::LOW;
    event.player_id = player_id;
    event.map_name = to_map;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["from_map"] = from_map;
    j["to_map"] = to_map;
    event.payload = j.dump();
    
    return event;
}

GameEvent create_npc_interaction(uint32 player_id, uint32 npc_id, const std::string& map_name) {
    GameEvent event;
    event.type = GameEventType::NPC_INTERACTION;
    event.priority = EventPriority::NORMAL;
    event.player_id = player_id;
    event.map_name = map_name;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["npc_id"] = npc_id;
    event.payload = j.dump();
    
    return event;
}

GameEvent create_quest_complete(uint32 player_id, uint32 quest_id) {
    GameEvent event;
    event.type = GameEventType::QUEST_COMPLETE;
    event.priority = EventPriority::HIGH;
    event.player_id = player_id;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    json j;
    j["quest_id"] = quest_id;
    event.payload = j.dump();
    
    return event;
}

GameEvent create_custom(const std::string& event_type, const std::string& payload, EventPriority priority) {
    GameEvent event;
    event.type = GameEventType::CUSTOM;
    event.priority = priority;
    event.payload = payload;
    event.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return event;
}

} // namespace EventFactory

} // namespace AIBridge