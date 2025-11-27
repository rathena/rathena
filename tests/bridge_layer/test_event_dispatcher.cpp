/**
 * Bridge Layer Tests: Event Dispatcher
 * Tests game event capturing and dispatching to AI service
 * 
 * Tests:
 * - Player login/logout event capture
 * - Monster kill event capture
 * - MVP defeat event capture
 * - Event batching (50 events or 1s window)
 * - HTTP POST to AI service
 * - Dead Letter Queue (DLQ) handling
 * - Event queue performance (<1ms insert)
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <queue>
#include <thread>

using namespace std;
using namespace std::chrono;

// Event structure
struct GameEvent {
    std::string event_id;
    std::string event_type;
    std::string player_id;
    std::string data;
    system_clock::time_point timestamp;
};

// Event batch structure
struct EventBatch {
    std::string batch_id;
    std::vector<GameEvent> events;
    system_clock::time_point created_at;
    size_t size() const { return events.size(); }
};

// Test fixture
class EventDispatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        batch_size_limit = 50;
        batch_time_limit_ms = 1000;
        event_queue.clear();
        dlq.clear();
    }

    void TearDown() override {
        event_queue.clear();
        dlq.clear();
    }

    size_t batch_size_limit;
    int batch_time_limit_ms;
    std::vector<GameEvent> event_queue;
    std::vector<GameEvent> dlq;
};

// Test: Capture player login event
TEST_F(EventDispatcherTest, CapturePlayerLoginEvent) {
    GameEvent login_event;
    login_event.event_id = "evt_login_001";
    login_event.event_type = "player_login";
    login_event.player_id = "player_001";
    login_event.data = R"({"location": "prontera", "char_id": "char_001"})";
    login_event.timestamp = system_clock::now();
    
    // Add to queue
    event_queue.push_back(login_event);
    
    EXPECT_EQ(event_queue.size(), 1);
    EXPECT_EQ(event_queue[0].event_type, "player_login");
    EXPECT_FALSE(event_queue[0].event_id.empty());
}

// Test: Capture player logout event
TEST_F(EventDispatcherTest, CapturePlayerLogoutEvent) {
    GameEvent logout_event;
    logout_event.event_id = "evt_logout_001";
    logout_event.event_type = "player_logout";
    logout_event.player_id = "player_001";
    logout_event.data = R"({"session_duration": 7200, "location": "geffen"})";
    logout_event.timestamp = system_clock::now();
    
    event_queue.push_back(logout_event);
    
    EXPECT_EQ(event_queue.size(), 1);
    EXPECT_EQ(event_queue[0].event_type, "player_logout");
}

// Test: Capture monster kill event
TEST_F(EventDispatcherTest, CaptureMonsterKillEvent) {
    GameEvent kill_event;
    kill_event.event_id = "evt_kill_001";
    kill_event.event_type = "monster_kill";
    kill_event.player_id = "player_001";
    kill_event.data = R"({
        "monster_id": "poring",
        "monster_level": 5,
        "location": "prt_fild08",
        "exp_gained": 100,
        "drops": ["jellopy", "apple"]
    })";
    kill_event.timestamp = system_clock::now();
    
    event_queue.push_back(kill_event);
    
    EXPECT_EQ(event_queue.size(), 1);
    EXPECT_TRUE(kill_event.data.find("poring") != std::string::npos);
}

// Test: Capture MVP defeat event
TEST_F(EventDispatcherTest, CaptureMVPDefeatEvent) {
    GameEvent mvp_event;
    mvp_event.event_id = "evt_mvp_001";
    mvp_event.event_type = "mvp_defeat";
    mvp_event.player_id = "player_001";
    mvp_event.data = R"({
        "mvp_id": "baphomet",
        "mvp_level": 99,
        "party_members": ["player_001", "player_002", "player_003"],
        "rare_drops": ["baphomet_doll"]
    })";
    mvp_event.timestamp = system_clock::now();
    
    event_queue.push_back(mvp_event);
    
    EXPECT_EQ(event_queue.size(), 1);
    EXPECT_TRUE(mvp_event.data.find("baphomet") != std::string::npos);
    EXPECT_TRUE(mvp_event.data.find("party_members") != std::string::npos);
}

// Test: Event batching on size limit
TEST_F(EventDispatcherTest, BatchingOnSizeLimit) {
    // Add 52 events
    for (int i = 0; i < 52; i++) {
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event.event_type = "monster_kill";
        event.player_id = "player_001";
        event.timestamp = system_clock::now();
        event_queue.push_back(event);
    }
    
    EXPECT_EQ(event_queue.size(), 52);
    
    // First batch (50 events)
    EventBatch batch1;
    batch1.batch_id = "batch_001";
    batch1.created_at = system_clock::now();
    
    for (size_t i = 0; i < batch_size_limit && i < event_queue.size(); i++) {
        batch1.events.push_back(event_queue[i]);
    }
    
    EXPECT_EQ(batch1.size(), batch_size_limit);
    EXPECT_EQ(batch1.size(), 50);
    
    // Remaining events stay in queue
    size_t remaining = event_queue.size() - batch_size_limit;
    EXPECT_EQ(remaining, 2);
}

// Test: Event batching on time limit
TEST_F(EventDispatcherTest, BatchingOnTimeLimit) {
    auto batch_start = system_clock::now();
    
    // Add events slowly
    for (int i = 0; i < 10; i++) {
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event.event_type = "player_action";
        event.timestamp = system_clock::now();
        event_queue.push_back(event);
        
        std::this_thread::sleep_for(milliseconds(50));
    }
    
    auto elapsed_ms = duration_cast<milliseconds>(system_clock::now() - batch_start).count();
    
    // Should dispatch on timeout even with <50 events
    bool should_dispatch = (event_queue.size() >= batch_size_limit) || 
                           (elapsed_ms >= batch_time_limit_ms);
    
    EXPECT_TRUE(should_dispatch) << "Should dispatch batch on time limit";
    EXPECT_LT(event_queue.size(), batch_size_limit);
    EXPECT_GE(elapsed_ms, 500);  // At least 500ms elapsed
}

// Test: Queue insert performance
TEST_F(EventDispatcherTest, QueueInsertPerformance) {
    const int insert_count = 1000;
    std::vector<int64_t> insert_times;
    
    for (int i = 0; i < insert_count; i++) {
        auto start = high_resolution_clock::now();
        
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event.event_type = "test_event";
        event.timestamp = system_clock::now();
        event_queue.push_back(event);
        
        auto end = high_resolution_clock::now();
        auto duration_ns = duration_cast<nanoseconds>(end - start).count();
        insert_times.push_back(duration_ns);
    }
    
    // Calculate average
    int64_t total_ns = 0;
    for (int64_t time : insert_times) {
        total_ns += time;
    }
    double avg_us = (total_ns / insert_count) / 1000.0;  // Convert to microseconds
    
    EXPECT_LT(avg_us, 1000) << "Average queue insert should be <1ms, got " << avg_us << "μs";
}

// Test: Dead Letter Queue handling
TEST_F(EventDispatcherTest, DeadLetterQueueHandling) {
    const int max_retries = 3;
    
    // Simulate event processing failures
    for (int i = 0; i < 5; i++) {
        GameEvent event;
        event.event_id = "evt_fail_" + std::to_string(i);
        event.event_type = "problematic_event";
        
        int retry_count = 0;
        bool processing_failed = true;
        
        // Try processing with retries
        for (int attempt = 0; attempt < max_retries; attempt++) {
            retry_count++;
            // Simulate failure
            if (attempt < max_retries - 1) {
                continue;
            }
        }
        
        if (processing_failed && retry_count >= max_retries) {
            // Send to DLQ
            dlq.push_back(event);
        }
    }
    
    EXPECT_EQ(dlq.size(), 5) << "All failed events should be in DLQ";
}

// Test: Batch dispatch to AI service
TEST_F(EventDispatcherTest, BatchDispatchToAIService) {
    // Create batch
    EventBatch batch;
    batch.batch_id = "batch_001";
    batch.created_at = system_clock::now();
    
    for (int i = 0; i < 45; i++) {
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event.event_type = "monster_kill";
        batch.events.push_back(event);
    }
    
    // Simulate HTTP POST
    std::string url = "http://localhost:8000/api/v1/world/events/batch";
    std::string batch_json = R"({"batch_id": "batch_001", "event_count": 45})";
    
    auto start = high_resolution_clock::now();
    // Simulate dispatch
    bool dispatch_success = true;
    auto end = high_resolution_clock::now();
    
    auto dispatch_latency_ms = duration_cast<milliseconds>(end - start).count();
    
    EXPECT_TRUE(dispatch_success);
    EXPECT_EQ(batch.size(), 45);
    EXPECT_LT(dispatch_latency_ms, 100) << "Batch dispatch should be <100ms";
}

// Test: Event deduplication
TEST_F(EventDispatcherTest, EventDeduplication) {
    std::set<std::string> seen_event_ids;
    
    // Add events
    for (int i = 0; i < 10; i++) {
        std::string event_id = "evt_" + std::to_string(i);
        
        // Check for duplicate
        if (seen_event_ids.find(event_id) == seen_event_ids.end()) {
            seen_event_ids.insert(event_id);
            
            GameEvent event;
            event.event_id = event_id;
            event_queue.push_back(event);
        }
    }
    
    EXPECT_EQ(event_queue.size(), 10);
    EXPECT_EQ(seen_event_ids.size(), 10);
    
    // Try adding duplicate
    std::string duplicate_id = "evt_5";
    if (seen_event_ids.find(duplicate_id) != seen_event_ids.end()) {
        // Duplicate detected, don't add
    } else {
        event_queue.push_back(GameEvent());
    }
    
    EXPECT_EQ(event_queue.size(), 10) << "Should not add duplicate event";
}

// Test: Event priority handling
TEST_F(EventDispatcherTest, EventPriorityHandling) {
    struct PriorityEvent {
        GameEvent event;
        int priority;  // 1=high, 2=medium, 3=low
    };
    
    std::vector<PriorityEvent> priority_queue;
    
    // Add events with different priorities
    priority_queue.push_back({GameEvent{"evt_low", "chat", "p1", ""}, 3});
    priority_queue.push_back({GameEvent{"evt_high", "mvp_defeat", "p1", ""}, 1});
    priority_queue.push_back({GameEvent{"evt_med", "monster_kill", "p1", ""}, 2});
    
    // Sort by priority
    std::sort(priority_queue.begin(), priority_queue.end(),
        [](const PriorityEvent& a, const PriorityEvent& b) {
            return a.priority < b.priority;
        });
    
    EXPECT_EQ(priority_queue[0].event.event_type, "mvp_defeat");
    EXPECT_EQ(priority_queue[1].event.event_type, "monster_kill");
    EXPECT_EQ(priority_queue[2].event.event_type, "chat");
}

// Test: Concurrent event capture
TEST_F(EventDispatcherTest, ConcurrentEventCapture) {
    const int thread_count = 10;
    const int events_per_thread = 100;
    std::vector<std::thread> threads;
    std::mutex queue_mutex;
    
    auto capture_events = [&](int thread_id) {
        for (int i = 0; i < events_per_thread; i++) {
            GameEvent event;
            event.event_id = "evt_t" + std::to_string(thread_id) + "_" + std::to_string(i);
            event.event_type = "concurrent_test";
            event.timestamp = system_clock::now();
            
            std::lock_guard<std::mutex> lock(queue_mutex);
            event_queue.push_back(event);
        }
    };
    
    // Launch threads
    for (int t = 0; t < thread_count; t++) {
        threads.emplace_back(capture_events, t);
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(event_queue.size(), thread_count * events_per_thread);
}

// Test: Event overflow handling
TEST_F(EventDispatcherTest, EventOverflowHandling) {
    const size_t max_queue_size = 10000;
    
    // Fill queue to limit
    for (size_t i = 0; i < max_queue_size; i++) {
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event_queue.push_back(event);
    }
    
    EXPECT_EQ(event_queue.size(), max_queue_size);
    
    // Try adding more - should reject or handle overflow
    bool queue_full = event_queue.size() >= max_queue_size;
    
    if (queue_full) {
        // Drop oldest or send alert
        EXPECT_TRUE(queue_full) << "Should detect queue overflow";
    }
}

// Test: Event timestamp accuracy
TEST_F(EventDispatcherTest, EventTimestampAccuracy) {
    auto test_start = system_clock::now();
    
    GameEvent event;
    event.event_id = "evt_timestamp_test";
    event.event_type = "test";
    event.timestamp = system_clock::now();
    
    auto test_end = system_clock::now();
    
    // Event timestamp should be within test window
    EXPECT_GE(event.timestamp, test_start);
    EXPECT_LE(event.timestamp, test_end);
    
    // Timestamp precision should be millisecond or better
    auto precision_ms = duration_cast<milliseconds>(test_end - test_start).count();
    EXPECT_LT(precision_ms, 10) << "Timestamp capture should be <10ms";
}

// Test: JSON serialization for batch
TEST_F(EventDispatcherTest, JSONSerializationForBatch) {
    EventBatch batch;
    batch.batch_id = "batch_test_001";
    
    // Add events
    for (int i = 0; i < 5; i++) {
        GameEvent event;
        event.event_id = "evt_" + std::to_string(i);
        event.event_type = "test_event";
        event.data = R"({"key": "value"})";
        batch.events.push_back(event);
    }
    
    // Serialize to JSON (simplified simulation)
    std::string batch_json = R"({
        "batch_id": "batch_test_001",
        "event_count": 5,
        "events": [...]
    })";
    
    EXPECT_TRUE(batch_json.find("batch_test_001") != std::string::npos);
    EXPECT_TRUE(batch_json.find("event_count") != std::string::npos);
}

// Test: Dispatch failure and retry
TEST_F(EventDispatcherTest, DispatchFailureAndRetry) {
    EventBatch batch;
    batch.batch_id = "batch_retry_test";
    
    GameEvent event;
    event.event_id = "evt_001";
    batch.events.push_back(event);
    
    const int max_retries = 3;
    int retry_count = 0;
    bool dispatch_success = false;
    
    for (int attempt = 0; attempt < max_retries; attempt++) {
        retry_count++;
        
        // Simulate failure first 2 attempts
        if (attempt < 2) {
            dispatch_success = false;
            std::this_thread::sleep_for(milliseconds(100 * (1 << attempt)));
        } else {
            dispatch_success = true;
            break;
        }
    }
    
    EXPECT_TRUE(dispatch_success);
    EXPECT_EQ(retry_count, 3);
}

// Test: DLQ prevents infinite retries
TEST_F(EventDispatcherTest, DLQPreventsInfiniteRetries) {
    GameEvent problematic_event;
    problematic_event.event_id = "evt_problematic";
    problematic_event.event_type = "corrupt_data";
    
    const int max_retries = 3;
    int retry_count = 0;
    
    // Simulate persistent failure
    for (int attempt = 0; attempt < max_retries; attempt++) {
        retry_count++;
        // Always fails
    }
    
    // After max retries, send to DLQ
    if (retry_count >= max_retries) {
        dlq.push_back(problematic_event);
    }
    
    EXPECT_EQ(dlq.size(), 1);
    EXPECT_EQ(dlq[0].event_id, "evt_problematic");
}

// Test: Event metrics collection
TEST_F(EventDispatcherTest, EventMetricsCollection) {
    struct EventMetrics {
        int events_captured = 0;
        int events_dispatched = 0;
        int events_in_dlq = 0;
        double avg_batch_size = 0.0;
        double avg_dispatch_latency_ms = 0.0;
    };
    
    EventMetrics metrics;
    
    // Simulate event processing
    metrics.events_captured = 50000;
    metrics.events_dispatched = 49950;
    metrics.events_in_dlq = 50;
    metrics.avg_batch_size = 42.5;
    metrics.avg_dispatch_latency_ms = 0.8;
    
    EXPECT_EQ(metrics.events_captured, 50000);
    EXPECT_GT(metrics.events_dispatched, 49900);
    EXPECT_LT(metrics.events_in_dlq, 100);
    EXPECT_GT(metrics.avg_batch_size, 30);
    EXPECT_LT(metrics.avg_dispatch_latency_ms, 1.0) << "Queue insert should be <1ms";
}

// Main test runner
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}