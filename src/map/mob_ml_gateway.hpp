// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MOB_ML_GATEWAY_HPP_
#define _MOB_ML_GATEWAY_HPP_

#include "mob.hpp"
#include <string>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>

// Forward declarations for external libraries
typedef struct redisContext redisContext;
// PGconn is defined in libpq-fe.h, no forward declaration needed
struct pg_conn;  // Opaque forward declaration only

/**
 * ML Database Configuration
 */
struct MLDatabaseConfig {
    // PostgreSQL
    std::string pg_host;
    uint16 pg_port;
    std::string pg_database;
    std::string pg_user;
    std::string pg_password;
    int32 pg_connect_timeout;
    
    // Redis
    std::string redis_host;
    uint16 redis_port;
    int32 redis_timeout;
    int32 redis_db;
    
    // Behavior
    int32 cache_ttl;
    int32 request_priority;
    
    MLDatabaseConfig() :
        pg_host("127.0.0.1"),
        pg_port(5432),
        pg_database("ragnarok_ml"),
        pg_user("ml_user"),
        pg_password("ml_password"),
        pg_connect_timeout(10),
        redis_host("127.0.0.1"),
        redis_port(6379),
        redis_timeout(2),
        redis_db(0),
        cache_ttl(10),
        request_priority(5)
    {}
};

/**
 * ML Decision Gateway
 * Handles ML inference requests with fallback to traditional AI
 * 
 * Architecture:
 * 1. Check Redis cache (L2: action cache)
 * 2. If cache miss, insert request to PostgreSQL (ai_requests table)
 * 3. Poll for response from PostgreSQL (ai_responses table) with timeout
 * 4. Update Redis cache with result
 * 5. If timeout or error, fallback to traditional AI
 */
class MobMLGateway {
public:
    /**
     * ML Action Types (matching Python ML service)
     */
    enum class MLAction : uint8 {
        IDLE = 0,
        ATTACK = 1,
        MOVE_CLOSER = 2,
        MOVE_AWAY = 3,
        MOVE_RANDOM = 4,
        SKILL_1 = 5,
        SKILL_2 = 6,
        SKILL_3 = 7,
        CHANGE_TARGET = 8,
        FLEE = 9,
        TRADITIONAL_AI = 255  // Fallback to traditional AI
    };
    
    /**
     * ML Decision Result
     */
    struct MLDecision {
        MLAction action;
        float confidence;
        uint32 request_id;
        std::chrono::milliseconds latency;
        bool from_cache;
        bool success;
        std::string error_message;
        
        MLDecision() : 
            action(MLAction::TRADITIONAL_AI),
            confidence(0.0f),
            request_id(0),
            latency(0),
            from_cache(false),
            success(false),
            error_message("")
        {}
    };
    
    /**
     * Initialize ML gateway
     * Establishes connections to Redis and PostgreSQL
     * @return true if successful
     */
    static bool initialize();
    
    /**
     * Shutdown ML gateway
     * Closes all connections
     */
    static void shutdown();
    
    /**
     * Get ML decision for monster
     * @param md Monster data
     * @return ML decision with action and metadata
     */
    static MLDecision get_decision(struct mob_data* md);
    
    /**
     * Check if ML system is healthy
     * @return true if ML available
     */
    static bool is_healthy();
    
    /**
     * Get system statistics
     */
    static void get_stats(uint64_t& total_requests, uint64_t& cache_hits, 
                         uint64_t& ml_successes, uint64_t& fallbacks,
                         float& avg_latency_ms);
    
    /**
     * Perform health check
     * Tests Redis and PostgreSQL connectivity
     */
    static void health_check();
    
private:
    // Configuration
    static MLDatabaseConfig config_;
    
    // Initialization state
    static bool initialized_;
    static bool healthy_;
    static std::mutex mutex_;
    
    // Connections
    static redisContext* redis_ctx_;
    static void* pg_conn_;  // PGconn* (void* to avoid header conflicts)
    
    // Configuration reading
    static bool read_config(const char* config_file);
    
    // Statistics
    static uint64_t total_requests_;
    static uint64_t cache_hits_;
    static uint64_t ml_successes_;
    static uint64_t fallback_count_;
    static float avg_latency_ms_;
    
    // Last health check
    static t_tick last_health_check_;
    
    // Try to get decision from Redis cache
    static MLDecision try_cache(const std::vector<float>& state, uint32 mob_id);
    
    // Submit request to ML service via PostgreSQL
    static MLDecision request_ml_inference(const std::vector<float>& state, uint32 mob_id, const char* archetype);
    
    // Update Redis cache with result
    static void update_cache(const std::vector<float>& state, MLAction action, float confidence, uint32 mob_id);
    
    // Generate cache key from state vector
    static std::string generate_cache_key(const std::vector<float>& state, uint32 mob_id);
    
    // Determine archetype from mob_data
    static const char* determine_archetype(const struct mob_data* md);
    
    // Connect to Redis
    static bool connect_redis();
    
    // Connect to PostgreSQL
    static bool connect_postgresql();
    
    // Reconnect if connection lost
    static bool reconnect_if_needed();
};

#endif // _MOB_ML_GATEWAY_HPP_
