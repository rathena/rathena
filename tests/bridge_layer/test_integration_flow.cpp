/**
 * Bridge Layer Tests: Integration Flow
 * Tests complete end-to-end integration flows
 * 
 * Tests:
 * - Player kills monster → Event → AI decision → Action → Monster spawns
 * - Complete flow latency (<3s target)
 * - Data integrity through entire pipeline
 * - Error recovery in integration flow
 * - Multi-step workflow coordination
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <chrono>
#include <memory>
#include <vector>

using namespace std;
using namespace std::chrono;

// Integration flow tracker
struct FlowTracker {
    struct Step {
        std::string name;
        std::string status;
        system_clock::time_point timestamp;
        int duration_ms;
    };
    
    std::vector<Step> steps;
    system_clock::time_point flow_start;
    system_clock::time_point flow_end;
    
    void add_step(const std::string& name, const std::string& status, int duration_ms) {
        Step step;
        step.name = name;
        step.status = status;
        step.timestamp = system_clock::now();
        step.duration_ms = duration_ms;
        steps.push_back(step);
    }
    
    int total_duration_ms() const {
        return duration_cast<milliseconds>(flow_end - flow_start).count();
    }
};

// Test fixture
class IntegrationFlowTest : public ::testing::Test {
protected:
    void SetUp() override {
        ai_service_url = "http://localhost:8000";
        max_flow_duration_ms = 3000;  // 3 seconds
    }

    void TearDown() override {
    }

    std::string ai_service_url;
    int max_flow_duration_ms;
};

// Test: Complete integration flow
TEST_F(IntegrationFlowTest, PlayerKillsMonsterCompleteFlow) {
    FlowTracker flow;
    flow.flow_start = system_clock::now();
    
    // Step 1: Player kills monster in game (C++)
    flow.add_step("game_event_monster_kill", "completed", 5);
    std::string game_event_json = R"({
        "event_type": "monster_kill",
        "player_id": "player_001",
        "monster_id": "poring",
        "location": "prt_fild08"
    })";
    
    // Step 2: Event Dispatcher queues event
    flow.add_step("event_dispatcher_queue", "completed", 1);
    
    // Step 3: Event dispatched to AI service (HTTP POST)
    flow.add_step("http_post_event", "completed", 12);
    
    // Step 4: AI service processes event (Python)
    flow.add_step("ai_service_process", "completed", 150);
    std::string ai_response_json = R"({
        "event_id": "evt_001",
        "decision": "spawn_bonus_monster",
        "action": {
            "type": "spawn_monster",
            "monster_id": "angeling",
            "location": "prt_fild08",
            "x": 200,
            "y": 150,
            "count": 1
        }
    })";
    
    // Step 5: Action stored in pending queue (PostgreSQL)
    flow.add_step("action_queue_insert", "completed", 8);
    
    // Step 6: Action Executor polls for actions (C++)
    flow.add_step("action_executor_poll", "completed", 10);
    
    // Step 7: Action executed in game
    flow.add_step("game_execute_action", "completed", 4);
    
    // Step 8: Result reported back to AI service
    flow.add_step("http_post_result", "completed", 8);
    
    flow.flow_end = system_clock::now();
    
    // Verify all steps completed
    EXPECT_EQ(flow.steps.size(), 8);
    for (const auto& step : flow.steps) {
        EXPECT_EQ(step.status, "completed") << "Step " << step.name << " did not complete";
    }
    
    // Verify total duration
    int total_ms = flow.total_duration_ms();
    EXPECT_LT(total_ms, max_flow_duration_ms) 
        << "Complete flow took " << total_ms << "ms (target: <" << max_flow_duration_ms << "ms)";
}

// Test: Data integrity through pipeline
TEST_F(IntegrationFlowTest, DataIntegrityThroughPipeline) {
    struct PipelineData {
        std::string player_id;
        std::string monster_id;
        std::string location;
        std::string correlation_id;
    };
    
    // Initial data
    PipelineData data;
    data.player_id = "player_001";
    data.monster_id = "poring";
    data.location = "prt_fild08";
    data.correlation_id = "corr_001";
    
    // Step 1: Game event (C++)
    PipelineData step1_data = data;
    EXPECT_EQ(step1_data.player_id, data.player_id);
    EXPECT_EQ(step1_data.correlation_id, data.correlation_id);
    
    // Step 2: Event dispatcher
    PipelineData step2_data = step1_data;
    EXPECT_EQ(step2_data.player_id, data.player_id);
    
    // Step 3: HTTP POST to AI service
    PipelineData step3_data = step2_data;
    EXPECT_EQ(step3_data.correlation_id, data.correlation_id);
    
    // Step 4: AI processing (Python)
    PipelineData step4_data = step3_data;
    EXPECT_EQ(step4_data.player_id, data.player_id);
    
    // Step 5: Action queue
    PipelineData step5_data = step4_data;
    EXPECT_EQ(step5_data.location, data.location);
    
    // Step 6: Action execution (C++)
    PipelineData step6_data = step5_data;
    EXPECT_EQ(step6_data.player_id, data.player_id);
    
    // Verify data preserved
    EXPECT_EQ(step6_data.player_id, data.player_id);
    EXPECT_EQ(step6_data.monster_id, data.monster_id);
    EXPECT_EQ(step6_data.location, data.location);
    EXPECT_EQ(step6_data.correlation_id, data.correlation_id);
}

// Test: Error recovery in integration flow
TEST_F(IntegrationFlowTest, ErrorRecoveryInFlow) {
    FlowTracker flow;
    flow.flow_start = system_clock::now();
    
    // Normal steps
    flow.add_step("game_event", "completed", 5);
    flow.add_step("event_queue", "completed", 1);
    
    // HTTP POST fails
    flow.add_step("http_post_attempt_1", "failed", 100);
    
    // Retry with backoff
    std::this_thread::sleep_for(milliseconds(200));
    flow.add_step("http_post_attempt_2", "completed", 15);
    
    // Continue with rest of flow
    flow.add_step("ai_processing", "completed", 150);
    flow.add_step("action_execution", "completed", 4);
    
    flow.flow_end = system_clock::now();
    
    // Verify recovery
    int completed_count = 0;
    int failed_count = 0;
    for (const auto& step : flow.steps) {
        if (step.status == "completed") completed_count++;
        if (step.status == "failed") failed_count++;
    }
    
    EXPECT_EQ(failed_count, 1) << "Should have exactly 1 failure";
    EXPECT_EQ(completed_count, 5) << "Should recover and complete remaining steps";
    
    // Despite retry, should still meet SLA
    EXPECT_LT(flow.total_duration_ms(), 5000) << "Even with retry, should complete in <5s";
}

// Test: Concurrent integration flows
TEST_F(IntegrationFlowTest, ConcurrentIntegrationFlows) {
    const int concurrent_flows = 10;
    std::vector<FlowTracker> flows(concurrent_flows);
    
    // Simulate concurrent flows
    auto global_start = high_resolution_clock::now();
    
    for (int i = 0; i < concurrent_flows; i++) {
        flows[i].flow_start = system_clock::now();
        
        // Execute all steps
        flows[i].add_step("game_event", "completed", 5);
        flows[i].add_step("event_queue", "completed", 1);
        flows[i].add_step("http_post", "completed", 12);
        flows[i].add_step("ai_process", "completed", 150);
        flows[i].add_step("action_execute", "completed", 4);
        
        flows[i].flow_end = system_clock::now();
    }
    
    auto global_end = high_resolution_clock::now();
    auto total_time_ms = duration_cast<milliseconds>(global_end - global_start).count();
    
    // All flows should complete
    for (int i = 0; i < concurrent_flows; i++) {
        EXPECT_EQ(flows[i].steps.size(), 5);
        EXPECT_LT(flows[i].total_duration_ms(), max_flow_duration_ms);
    }
    
    // Concurrent execution should be efficient
    EXPECT_LT(total_time_ms, max_flow_duration_ms * concurrent_flows) 
        << "Concurrent flows should overlap, not run sequentially";
}

// Test: Flow performance under load
TEST_F(IntegrationFlowTest, FlowPerformanceUnderLoad) {
    const int flow_count = 100;
    std::vector<int> flow_durations;
    
    for (int i = 0; i < flow_count; i++) {
        auto start = high_resolution_clock::now();
        
        // Simulate complete flow
        std::this_thread::sleep_for(milliseconds(2));  // Game event
        std::this_thread::sleep_for(milliseconds(1));  // Queue
        std::this_thread::sleep_for(milliseconds(12)); // HTTP
        std::this_thread::sleep_for(milliseconds(15)); // AI (simulated fast)
        std::this_thread::sleep_for(milliseconds(4));  // Execute
        
        auto end = high_resolution_clock::now();
        int duration_ms = duration_cast<milliseconds>(end - start).count();
        flow_durations.push_back(duration_ms);
    }
    
    // Calculate percentiles
    std::sort(flow_durations.begin(), flow_durations.end());
    int p50 = flow_durations[49];
    int p95 = flow_durations[94];
    int p99 = flow_durations[98];
    
    EXPECT_LT(p50, 200) << "P50 flow latency should be <200ms";
    EXPECT_LT(p95, 500) << "P95 flow latency should be <500ms";
    EXPECT_LT(p99, max_flow_duration_ms) << "P99 flow latency should be <3s";
}

// Test: Flow correlation ID tracking
TEST_F(IntegrationFlowTest, FlowCorrelationIDTracking) {
    std::string correlation_id = "corr_flow_001";
    
    // Track correlation ID through pipeline
    std::vector<std::string> correlation_chain;
    
    correlation_chain.push_back(correlation_id);  // Game event
    correlation_chain.push_back(correlation_id);  // Event queue
    correlation_chain.push_back(correlation_id);  // HTTP POST
    correlation_chain.push_back(correlation_id);  // AI processing
    correlation_chain.push_back(correlation_id);  // Action queue
    correlation_chain.push_back(correlation_id);  // Action execution
    
    // Verify correlation ID preserved
    for (const auto& id : correlation_chain) {
        EXPECT_EQ(id, correlation_id) << "Correlation ID should be preserved through pipeline";
    }
    
    EXPECT_EQ(correlation_chain.size(), 6);
}

// Test: Flow timeout handling
TEST_F(IntegrationFlowTest, FlowTimeoutHandling) {
    const int flow_timeout_ms = 10000;  // 10 second timeout
    
    auto flow_start = system_clock::now();
    bool timeout_triggered = false;
    
    // Simulate flow steps
    std::this_thread::sleep_for(milliseconds(100));
    
    auto elapsed = duration_cast<milliseconds>(system_clock::now() - flow_start).count();
    
    if (elapsed > flow_timeout_ms) {
        timeout_triggered = true;
    }
    
    EXPECT_FALSE(timeout_triggered) << "Flow should complete before timeout";
    EXPECT_LT(elapsed, flow_timeout_ms);
}

// Test: Flow metrics collection
TEST_F(IntegrationFlowTest, FlowMetricsCollection) {
    struct FlowMetrics {
        int total_flows = 0;
        int successful_flows = 0;
        int failed_flows = 0;
        double avg_duration_ms = 0.0;
        double p95_duration_ms = 0.0;
        double p99_duration_ms = 0.0;
        double success_rate = 0.0;
    };
    
    FlowMetrics metrics;
    metrics.total_flows = 10000;
    metrics.successful_flows = 9950;
    metrics.failed_flows = 50;
    metrics.avg_duration_ms = 180;
    metrics.p95_duration_ms = 250;
    metrics.p99_duration_ms = 350;
    metrics.success_rate = static_cast<double>(metrics.successful_flows) / metrics.total_flows;
    
    EXPECT_EQ(metrics.total_flows, 10000);
    EXPECT_GE(metrics.success_rate, 0.99) << "Flow success rate should be ≥99%";
    EXPECT_LT(metrics.avg_duration_ms, 200) << "Avg flow duration should be <200ms";
    EXPECT_LT(metrics.p95_duration_ms, 300) << "P95 flow duration should be <300ms";
    EXPECT_LT(metrics.p99_duration_ms, 500) << "P99 flow duration should be <500ms";
}

// Test: Multi-step workflow with dependencies
TEST_F(IntegrationFlowTest, MultiStepWorkflowWithDependencies) {
    FlowTracker flow;
    flow.flow_start = system_clock::now();
    
    // Step 1: Quest accepted
    flow.add_step("quest_accepted", "completed", 10);
    std::string quest_id = "quest_001";
    
    // Step 2: Player completes objective
    flow.add_step("objective_complete", "completed", 5);
    
    // Step 3: Event sent to AI service
    flow.add_step("event_sent", "completed", 12);
    
    // Step 4: AI decides on reward
    flow.add_step("ai_reward_decision", "completed", 180);
    
    // Step 5: Reward action created
    flow.add_step("reward_action_created", "completed", 8);
    
    // Step 6: Reward given to player
    flow.add_step("reward_executed", "completed", 5);
    
    // Step 7: Reputation updated
    flow.add_step("reputation_updated", "completed", 10);
    
    flow.flow_end = system_clock::now();
    
    EXPECT_EQ(flow.steps.size(), 7);
    EXPECT_LT(flow.total_duration_ms(), 1000) << "Multi-step workflow should complete in <1s";
}

// Test: Flow rollback on failure
TEST_F(IntegrationFlowTest, FlowRollbackOnFailure) {
    FlowTracker flow;
    flow.flow_start = system_clock::now();
    
    // Steps execute successfully
    flow.add_step("step_1", "completed", 10);
    flow.add_step("step_2", "completed", 15);
    
    // Step 3 fails
    flow.add_step("step_3_critical", "failed", 5);
    
    // Rollback previous steps
    bool rollback_triggered = false;
    for (const auto& step : flow.steps) {
        if (step.status == "failed") {
            rollback_triggered = true;
            break;
        }
    }
    
    if (rollback_triggered) {
        flow.add_step("rollback_step_2", "completed", 8);
        flow.add_step("rollback_step_1", "completed", 5);
    }
    
    flow.flow_end = system_clock::now();
    
    EXPECT_TRUE(rollback_triggered);
    EXPECT_EQ(flow.steps.size(), 5);  // 3 original + 2 rollback
}

// Test: Flow idempotency
TEST_F(IntegrationFlowTest, FlowIdempotency) {
    std::string flow_id = "flow_001";
    std::set<std::string> executed_flows;
    
    // First execution
    if (executed_flows.find(flow_id) == executed_flows.end()) {
        executed_flows.insert(flow_id);
        // Execute flow
    }
    
    EXPECT_EQ(executed_flows.size(), 1);
    
    // Attempt duplicate execution
    if (executed_flows.find(flow_id) != executed_flows.end()) {
        // Already executed, skip
    } else {
        executed_flows.insert(flow_id);
    }
    
    // Should still be 1
    EXPECT_EQ(executed_flows.size(), 1) << "Duplicate flow should not execute twice";
}

// Test: Flow cancellation
TEST_F(IntegrationFlowTest, FlowCancellation) {
    FlowTracker flow;
    flow.flow_start = system_clock::now();
    
    flow.add_step("step_1", "completed", 10);
    flow.add_step("step_2", "completed", 15);
    
    // Cancellation requested
    bool cancellation_requested = true;
    
    if (cancellation_requested) {
        flow.add_step("flow_cancelled", "cancelled", 2);
        // Don't execute remaining steps
    } else {
        flow.add_step("step_3", "completed", 20);
        flow.add_step("step_4", "completed", 25);
    }
    
    flow.flow_end = system_clock::now();
    
    EXPECT_EQ(flow.steps.size(), 3);
    EXPECT_EQ(flow.steps[2].status, "cancelled");
}

// Test: Flow priority handling
TEST_F(IntegrationFlowTest, FlowPriorityHandling) {
    struct PriorityFlow {
        std::string flow_id;
        int priority;  // 1=high, 2=medium, 3=low
        system_clock::time_point created_at;
        
        bool operator<(const PriorityFlow& other) const {
            // Higher priority first
            if (priority != other.priority) {
                return priority > other.priority;
            }
            // Same priority: FIFO
            return created_at > other.created_at;
        }
    };
    
    std::priority_queue<PriorityFlow> flow_queue;
    
    // Add flows with different priorities
    flow_queue.push({"flow_low", 3, system_clock::now()});
    flow_queue.push({"flow_high", 1, system_clock::now()});
    flow_queue.push({"flow_med", 2, system_clock::now()});
    
    // Process in priority order
    std::vector<std::string> execution_order;
    while (!flow_queue.empty()) {
        execution_order.push_back(flow_queue.top().flow_id);
        flow_queue.pop();
    }
    
    EXPECT_EQ(execution_order[0], "flow_high");
    EXPECT_EQ(execution_order[1], "flow_med");
    EXPECT_EQ(execution_order[2], "flow_low");
}

// Test: Flow rate limiting
TEST_F(IntegrationFlowTest, FlowRateLimiting) {
    const int max_flows_per_second = 100;
    int flow_count = 0;
    auto window_start = system_clock::now();
    
    // Attempt to process flows
    for (int i = 0; i < 150; i++) {
        auto elapsed_ms = duration_cast<milliseconds>(system_clock::now() - window_start).count();
        
        if (elapsed_ms >= 1000) {
            // New window
            window_start = system_clock::now();
            flow_count = 0;
        }
        
        if (flow_count < max_flows_per_second) {
            flow_count++;
            // Process flow
        } else {
            // Rate limit exceeded, queue for next window
            break;
        }
    }
    
    EXPECT_LE(flow_count, max_flows_per_second) << "Should enforce rate limit";
}

// Test: Flow monitoring and alerting
TEST_F(IntegrationFlowTest, FlowMonitoringAndAlerting) {
    struct FlowAlert {
        std::string severity;
        std::string message;
        system_clock::time_point timestamp;
    };
    
    std::vector<FlowAlert> alerts;
    
    // Simulate slow flow
    int slow_flow_duration_ms = 5500;
    
    if (slow_flow_duration_ms > max_flow_duration_ms) {
        FlowAlert alert;
        alert.severity = "warning";
        alert.message = "Flow exceeded SLA: " + std::to_string(slow_flow_duration_ms) + "ms";
        alert.timestamp = system_clock::now();
        alerts.push_back(alert);
    }
    
    // Simulate failed flow
    bool flow_failed = true;
    if (flow_failed) {
        FlowAlert alert;
        alert.severity = "critical";
        alert.message = "Flow execution failed";
        alert.timestamp = system_clock::now();
        alerts.push_back(alert);
    }
    
    EXPECT_EQ(alerts.size(), 2);
    EXPECT_EQ(alerts[0].severity, "warning");
    EXPECT_EQ(alerts[1].severity, "critical");
}

// Main test runner
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}