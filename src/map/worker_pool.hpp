// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Multi-CPU/Threaded Worker Pool for AI Entity Processing
// -------------------------------------------------------
// This module provides a production-grade, multi-threaded worker pool for AI entity management.
// It supports atomic entity assignment, migration, dynamic scaling, Prometheus metrics, and structured logging.

#ifndef WORKER_POOL_HPP
#define WORKER_POOL_HPP

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <functional>
#include <string>
#include <chrono>
#include <set>
#include <map>
#include <optional>

// Forward declarations
struct EntityAssignment;
struct WorkerPoolConfig;
class WorkerPool;
class PrometheusExporter;

// Entity identifier type
using entity_id_t = uint64_t;

// Assignment strategy
enum class AssignmentStrategy {
    ROUND_ROBIN,
    LEAST_LOADED,
    HASH
};

// WorkerPoolConfig: loaded from conf/worker_pool.conf and .env
struct WorkerPoolConfig {
    int num_threads = 4;
    int min_threads = 2;
    int max_threads = 8;
    AssignmentStrategy assignment_strategy = AssignmentStrategy::LEAST_LOADED;
    int migration_interval_ms = 5000;
    bool enable_metrics = true;
    std::string metrics_listen_addr = "0.0.0.0:9100";
    std::string log_level = "info";
    std::string log_file = "";
    bool verbose = false;
    bool dynamic_scaling = true;
    std::set<int> cpu_affinity;
    bool enabled = true;
};

// EntityAssignment: tracks which worker owns which entity
struct EntityAssignment {
    entity_id_t entity_id;
    int worker_id;
    std::chrono::steady_clock::time_point last_migrated;
};

// WorkerPool: manages worker threads and entity assignment
class WorkerPool {
public:
    WorkerPool(const WorkerPoolConfig& config);
    ~WorkerPool();

    void start();
    void stop();
    bool is_running() const;

    // Assign an entity to a worker (atomic)
    int assign_entity(entity_id_t entity_id);

    // Migrate entity to another worker (atomic)
    bool migrate_entity(entity_id_t entity_id, int target_worker);

    // Remove entity from pool (atomic)
    void remove_entity(entity_id_t entity_id);

    // Get worker for entity
    std::optional<int> get_worker_for_entity(entity_id_t entity_id);

    // Dynamic scaling
    void scale_workers(int new_count);

    // Metrics
    void export_metrics();

    // Logging
    void log_status();

    // For testing/monitoring
    int get_num_workers() const;
    int get_num_entities() const;
    std::map<int, std::vector<entity_id_t>> get_worker_entity_map() const;

private:
    void worker_loop(int worker_id);
    void migration_loop();
    void metrics_loop();

    std::vector<std::thread> workers;
    std::atomic<bool> running;
    WorkerPoolConfig config;

    // Entity assignment table (entity_id -> assignment)
    std::unordered_map<entity_id_t, EntityAssignment> entity_table;
    std::mutex entity_table_mutex;

    // Worker load tracking
    std::vector<std::atomic<int>> worker_loads;

    // Assignment state
    std::atomic<int> round_robin_counter;

    // Migration thread
    std::thread migration_thread;

    // Metrics exporter
    std::unique_ptr<PrometheusExporter> prometheus_exporter;
    std::thread metrics_thread;

    // Logging
    void log(const std::string& msg, const std::string& level = "info") const;
    void log_error(const std::string& msg) const;
    void log_debug(const std::string& msg) const;

    // Internal helpers
    int select_worker_for_assignment(entity_id_t entity_id);
    void perform_migration();

    // Prevent copy
    WorkerPool(const WorkerPool&) = delete;
    WorkerPool& operator=(const WorkerPool&) = delete;
};

#endif // WORKER_POOL_HPP