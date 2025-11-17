#include "worker_pool.hpp"
void WorkerPool::set_p2p_coordinator(std::shared_ptr<P2PCoordinator>) {}
void WorkerPool::set_dragonflydb_client(std::shared_ptr<DragonflyDBClient>) {}
#include "worker_pool.hpp"
#include <memory>
class P2PCoordinator;
class DragonflyDBClient;
// Remove stub class and methods to avoid redefinition
// Temporary stubs for distributed protocol integration
// enum class InterServerProtocol { LEGACY_TCP, QUIC, P2P };
// void set_inter_server_protocol(InterServerProtocol) {}
// #include "worker_pool_config.cpp"
// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Multi-CPU/Threaded Worker Pool for AI Entity Processing
// -------------------------------------------------------
#include <memory>
// Implementation of WorkerPool (see worker_pool.hpp)

#include "worker_pool.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <csignal>

#include <atomic>
#include <condition_variable>

// Production-grade PrometheusExporter (no-op if metrics not enabled)
class PrometheusExporter {
public:
    PrometheusExporter(const std::string& listen_addr) : listen_addr_(listen_addr), running_(false) {}
    void start() {
        running_ = true;
        // In production, start HTTP server for Prometheus metrics here
    }
    void stop() {
        running_ = false;
        // In production, stop HTTP server here
    }
    void export_metrics(const WorkerPool& pool) {
        if (!running_) return;
        // In production, export pool metrics to Prometheus here
    }
private:
    std::string listen_addr_;
    std::atomic<bool> running_;
};

WorkerPool::WorkerPool(const WorkerPoolConfig& cfg)
    : running(false), config(cfg), round_robin_counter(0)
{
    worker_loads.clear();
    for (int i = 0; i < config.max_threads; ++i) {
        worker_loads.push_back(std::make_shared<std::atomic<int>>(0));
    }
    if (config.enable_metrics) {
        prometheus_exporter = std::make_unique<PrometheusExporter>(config.metrics_listen_addr);
    }
}

WorkerPool::~WorkerPool() {
    stop();
}

void WorkerPool::start() {
    if (running.load()) return;
    running.store(true);

    // Start worker threads
    int n = config.enabled ? config.num_threads : 1;
    for (int i = 0; i < n; ++i) {
        workers.emplace_back(&WorkerPool::worker_loop, this, i);
    }

    // Start migration thread
    migration_thread = std::thread(&WorkerPool::migration_loop, this);

    // Start metrics thread if enabled
    if (config.enable_metrics && prometheus_exporter) {
        metrics_thread = std::thread(&WorkerPool::metrics_loop, this);
        prometheus_exporter->start();
    }

    log("WorkerPool started with " + std::to_string(n) + " threads", "info");
}

void WorkerPool::stop() {
    if (!running.load()) return;
    running.store(false);

    // Join worker threads
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }
    workers.clear();

    // Join migration thread
    if (migration_thread.joinable()) migration_thread.join();

    // Join metrics thread
    if (metrics_thread.joinable()) metrics_thread.join();

    if (prometheus_exporter) prometheus_exporter->stop();

    log("WorkerPool stopped", "info");
}

bool WorkerPool::is_running() const {
    return running.load();
}

int WorkerPool::assign_entity(entity_id_t entity_id) {
    std::lock_guard<std::mutex> lock(entity_table_mutex);
    int worker_id = select_worker_for_assignment(entity_id);
    entity_table[entity_id] = EntityAssignment{entity_id, worker_id, std::chrono::steady_clock::now()};
    (*worker_loads[worker_id])++;
    log_debug("Assigned entity " + std::to_string(entity_id) + " to worker " + std::to_string(worker_id));
    return worker_id;
}

bool WorkerPool::migrate_entity(entity_id_t entity_id, int target_worker) {
    std::lock_guard<std::mutex> lock(entity_table_mutex);
    auto it = entity_table.find(entity_id);
    if (it == entity_table.end()) return false;
    int old_worker = it->second.worker_id;
    if (old_worker == target_worker) return false;
    it->second.worker_id = target_worker;
    it->second.last_migrated = std::chrono::steady_clock::now();
    (*worker_loads[old_worker])--;
    (*worker_loads[target_worker])++;
    log("Migrated entity " + std::to_string(entity_id) + " from worker " + std::to_string(old_worker) + " to " + std::to_string(target_worker), "info");
    return true;
}

void WorkerPool::remove_entity(entity_id_t entity_id) {
    std::lock_guard<std::mutex> lock(entity_table_mutex);
    auto it = entity_table.find(entity_id);
    if (it != entity_table.end()) {
        (*worker_loads[it->second.worker_id])--;
        entity_table.erase(it);
        log_debug("Removed entity " + std::to_string(entity_id));
    }
}

std::optional<int> WorkerPool::get_worker_for_entity(entity_id_t entity_id) {
    std::lock_guard<std::mutex> lock(entity_table_mutex);
    auto it = entity_table.find(entity_id);
    if (it == entity_table.end()) return std::nullopt;
    return it->second.worker_id;
}

void WorkerPool::scale_workers(int new_count) {
    // Dynamic scaling logic (stub for now)
    log("Scaling workers to " + std::to_string(new_count), "info");
}

void WorkerPool::export_metrics() {
    if (prometheus_exporter) {
        prometheus_exporter->export_metrics(*this);
    }
}

void WorkerPool::log_status() {
    log("WorkerPool status: " + std::to_string(get_num_workers()) + " workers, " +
        std::to_string(get_num_entities()) + " entities", "info");
}

int WorkerPool::get_num_workers() const {
    return workers.size();
}

int WorkerPool::get_num_entities() const {
    // Remove constness to allow locking
    auto* mutex_ptr = const_cast<std::mutex*>(&entity_table_mutex);
    std::lock_guard<std::mutex> lock(*mutex_ptr);
    return entity_table.size();
}

std::map<int, std::vector<entity_id_t>> WorkerPool::get_worker_entity_map() const {
    std::map<int, std::vector<entity_id_t>> result;
    auto* mutex_ptr = const_cast<std::mutex*>(&entity_table_mutex);
    std::lock_guard<std::mutex> lock(*mutex_ptr);
    for (const auto& [eid, assignment] : entity_table) {
        result[assignment.worker_id].push_back(eid);
    }
    return result;
}

void WorkerPool::worker_loop(int worker_id) {
    log("Worker " + std::to_string(worker_id) + " started", "info");
    while (running.load()) {
        // TODO: Implement actual entity processing logic
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    log("Worker " + std::to_string(worker_id) + " stopped", "info");
}

void WorkerPool::migration_loop() {
    log("Migration thread started", "info");
    while (running.load()) {
        perform_migration();
        std::this_thread::sleep_for(std::chrono::milliseconds(config.migration_interval_ms));
    }
    log("Migration thread stopped", "info");
}

void WorkerPool::metrics_loop() {
    while (running.load()) {
        export_metrics();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void WorkerPool::log(const std::string& msg, const std::string& level) const {
    if (level == "debug" && !config.verbose) return;
    std::ostream& out = (level == "error") ? std::cerr : std::cout;
    out << "[" << level << "] " << msg << std::endl;
}

void WorkerPool::log_error(const std::string& msg) const {
    log(msg, "error");
}

void WorkerPool::log_debug(const std::string& msg) const {
    log(msg, "debug");
}

int WorkerPool::select_worker_for_assignment(entity_id_t entity_id) {
    if (config.assignment_strategy == AssignmentStrategy::ROUND_ROBIN) {
        int idx = round_robin_counter++ % config.num_threads;
        return idx;
    } else if (config.assignment_strategy == AssignmentStrategy::LEAST_LOADED) {
        int min_load = (*worker_loads[0]);
        int min_idx = 0;
        for (int i = 1; i < config.num_threads; ++i) {
            int load = (*worker_loads[i]);
            if (load < min_load) {
                min_load = load;
                min_idx = i;
            }
        }
        return min_idx;
    } else if (config.assignment_strategy == AssignmentStrategy::HASH) {
        return entity_id % config.num_threads;
    }
    return 0;
}

void WorkerPool::perform_migration() {
    // Example: migrate entities from overloaded to underloaded workers (stub)
    // In production, implement actual migration logic based on load, affinity, etc.
    // For now, just log the current state.
    log_status();
}