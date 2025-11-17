#include "worker_pool.hpp"
// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// WorkerPoolConfig loader: loads from conf/worker_pool.conf and .env

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <map>

// Helper: trim whitespace
static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Helper: parse assignment strategy
static AssignmentStrategy parse_strategy(const std::string& s) {
    if (s == "round_robin") return AssignmentStrategy::ROUND_ROBIN;
    if (s == "least_loaded") return AssignmentStrategy::LEAST_LOADED;
    if (s == "hash") return AssignmentStrategy::HASH;
    return AssignmentStrategy::LEAST_LOADED;
}

// Load config from conf/worker_pool.conf and .env
WorkerPoolConfig load_worker_pool_config();
WorkerPoolConfig load_worker_pool_config() {
    WorkerPoolConfig cfg;
    std::map<std::string, std::string> kv;

    // 1. Load conf/worker_pool.conf
    std::ifstream conf("conf/worker_pool.conf");
    std::string line;
    while (std::getline(conf, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));
        kv[key] = val;
    }

    // 2. Load .env (override)
    std::ifstream env(".env");
    while (std::getline(env, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));
        kv[key] = val;
    }

    // 3. Parse values
    if (kv.count("num_threads")) cfg.num_threads = std::stoi(kv["num_threads"]);
    if (kv.count("WORKER_POOL_NUM_THREADS")) cfg.num_threads = std::stoi(kv["WORKER_POOL_NUM_THREADS"]);
    if (kv.count("min_threads")) cfg.min_threads = std::stoi(kv["min_threads"]);
    if (kv.count("WORKER_POOL_MIN_THREADS")) cfg.min_threads = std::stoi(kv["WORKER_POOL_MIN_THREADS"]);
    if (kv.count("max_threads")) cfg.max_threads = std::stoi(kv["max_threads"]);
    if (kv.count("WORKER_POOL_MAX_THREADS")) cfg.max_threads = std::stoi(kv["WORKER_POOL_MAX_THREADS"]);
    if (kv.count("assignment_strategy")) cfg.assignment_strategy = parse_strategy(kv["assignment_strategy"]);
    if (kv.count("WORKER_POOL_ASSIGNMENT_STRATEGY")) cfg.assignment_strategy = parse_strategy(kv["WORKER_POOL_ASSIGNMENT_STRATEGY"]);
    if (kv.count("migration_interval_ms")) cfg.migration_interval_ms = std::stoi(kv["migration_interval_ms"]);
    if (kv.count("enable_metrics")) cfg.enable_metrics = (kv["enable_metrics"] == "true");
    if (kv.count("WORKER_POOL_METRICS_ENABLED")) cfg.enable_metrics = (kv["WORKER_POOL_METRICS_ENABLED"] == "true");
    if (kv.count("metrics_listen_addr")) cfg.metrics_listen_addr = kv["metrics_listen_addr"];
    if (kv.count("WORKER_POOL_METRICS_ADDR")) cfg.metrics_listen_addr = kv["WORKER_POOL_METRICS_ADDR"];
    if (kv.count("log_level")) cfg.log_level = kv["log_level"];
    if (kv.count("WORKER_POOL_LOG_LEVEL")) cfg.log_level = kv["WORKER_POOL_LOG_LEVEL"];
    if (kv.count("log_file")) cfg.log_file = kv["log_file"];
    if (kv.count("WORKER_POOL_LOG_FILE")) cfg.log_file = kv["WORKER_POOL_LOG_FILE"];
    if (kv.count("verbose")) cfg.verbose = (kv["verbose"] == "true");
    if (kv.count("WORKER_POOL_VERBOSE")) cfg.verbose = (kv["WORKER_POOL_VERBOSE"] == "true");
    if (kv.count("dynamic_scaling")) cfg.dynamic_scaling = (kv["dynamic_scaling"] == "true");
    if (kv.count("WORKER_POOL_DYNAMIC_SCALING")) cfg.dynamic_scaling = (kv["WORKER_POOL_DYNAMIC_SCALING"] == "true");
    if (kv.count("cpu_affinity")) {
        cfg.cpu_affinity.clear();
        std::istringstream iss(kv["cpu_affinity"]);
        std::string token;
        while (std::getline(iss, token, ',')) {
            token = trim(token);
            if (!token.empty()) cfg.cpu_affinity.insert(std::stoi(token));
        }
    }
    if (kv.count("WORKER_POOL_CPU_AFFINITY")) {
        cfg.cpu_affinity.clear();
        std::istringstream iss(kv["WORKER_POOL_CPU_AFFINITY"]);
        std::string token;
        while (std::getline(iss, token, ',')) {
            token = trim(token);
            if (!token.empty()) cfg.cpu_affinity.insert(std::stoi(token));
        }
    }
    if (kv.count("enabled")) cfg.enabled = (kv["enabled"] == "true");
    if (kv.count("WORKER_POOL_ENABLED")) cfg.enabled = (kv["WORKER_POOL_ENABLED"] == "true");

    return cfg;
}