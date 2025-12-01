// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Unit tests for WorkerPool (multi-threaded entity assignment, migration, scaling)

#include "worker_pool.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

void test_basic_assignment() {
    WorkerPoolConfig cfg;
    cfg.num_threads = 4;
    cfg.min_threads = 2;
    cfg.max_threads = 8;
    cfg.assignment_strategy = AssignmentStrategy::LEAST_LOADED;
    cfg.enabled = true;
    WorkerPool pool(cfg);
    pool.start();

    // Assign 100 entities
    for (uint64_t i = 0; i < 100; ++i) {
        int worker = pool.assign_entity(i);
        assert(worker >= 0 && worker < cfg.num_threads);
    }
    assert(pool.get_num_entities() == 100);

    // Remove half
    for (uint64_t i = 0; i < 50; ++i) {
        pool.remove_entity(i);
    }
    assert(pool.get_num_entities() == 50);

    pool.stop();
    std::cout << "test_basic_assignment passed\n";
}

void test_migration() {
    WorkerPoolConfig cfg;
    cfg.num_threads = 4;
    cfg.assignment_strategy = AssignmentStrategy::ROUND_ROBIN;
    cfg.enabled = true;
    WorkerPool pool(cfg);
    pool.start();

    uint64_t eid = 12345;
    int w1 = pool.assign_entity(eid);
    int w2 = (w1 + 1) % cfg.num_threads;
    bool migrated = pool.migrate_entity(eid, w2);
    assert(migrated);
    auto w_check = pool.get_worker_for_entity(eid);
    assert(w_check && *w_check == w2);

    pool.remove_entity(eid);
    assert(!pool.get_worker_for_entity(eid).has_value());

    pool.stop();
    std::cout << "test_migration passed\n";
}

void test_dynamic_scaling_stub() {
    WorkerPoolConfig cfg;
    cfg.num_threads = 2;
    cfg.min_threads = 2;
    cfg.max_threads = 4;
    cfg.dynamic_scaling = true;
    cfg.enabled = true;
    WorkerPool pool(cfg);
    pool.start();

    pool.scale_workers(4); // Should log scaling (stub)
    pool.stop();
    std::cout << "test_dynamic_scaling_stub passed\n";
}

/* int main() {
    test_basic_assignment();
    test_migration();
    test_dynamic_scaling_stub();
    std::cout << "All WorkerPool tests passed.\n";
    return 0;
} */
// Main removed: now integrated into map.cpp for production.