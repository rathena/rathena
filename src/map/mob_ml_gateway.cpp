// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "mob_ml_gateway.hpp"
#include "mob_ml_encoder.hpp"

#include <common/nullpo.hpp>
#include <common/showmsg.hpp>
#include <common/timer.hpp>
#include <common/md5calc.hpp>

#include "battle.hpp"

#include <hiredis/hiredis.h>
#include <libpq-fe.h>

#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>

using namespace rathena;

// Helper macro to cast void* pg_conn_ to PGconn* (avoids header conflicts)
#define PGCONN() (static_cast<PGconn*>(pg_conn_))

// Static member initialization
bool MobMLGateway::initialized_ = false;
bool MobMLGateway::healthy_ = false;
std::mutex MobMLGateway::mutex_;
redisContext* MobMLGateway::redis_ctx_ = nullptr;
void* MobMLGateway::pg_conn_ = nullptr;  // Actually PGconn*, cast in functions
uint64_t MobMLGateway::total_requests_ = 0;
uint64_t MobMLGateway::cache_hits_ = 0;
uint64_t MobMLGateway::ml_successes_ = 0;
uint64_t MobMLGateway::fallback_count_ = 0;
float MobMLGateway::avg_latency_ms_ = 0.0f;
t_tick MobMLGateway::last_health_check_ = 0;

/**
 * Initialize ML gateway
 */
bool MobMLGateway::initialize() {
	std::lock_guard<std::mutex> lock(mutex_);
	
	if (initialized_) {
		ShowWarning("[ML-GATEWAY] Already initialized\n");
		return true;
	}
	
	ShowStatus("[ML-GATEWAY] Initializing ML Monster AI Gateway...\n");
	
	// Connect to Redis
	if (!connect_redis()) {
		ShowError("[ML-GATEWAY] Failed to connect to Redis, cache disabled\n");
		redis_ctx_ = nullptr;  // Continue without cache
	} else {
		ShowStatus("[ML-GATEWAY] Redis connection established\n");
	}
	
	// Connect to PostgreSQL (required)
	if (!connect_postgresql()) {
		ShowError("[ML-GATEWAY] Failed to connect to PostgreSQL, ML AI disabled\n");
		healthy_ = false;
		initialized_ = false;
		return false;
	}
	
	ShowStatus("[ML-GATEWAY] PostgreSQL connection established\n");
	
	// Test connections
	health_check();
	
	initialized_ = true;
	healthy_ = true;
	last_health_check_ = gettick();
	
	ShowStatus("[ML-GATEWAY] ML Monster AI Gateway initialized successfully\n");
	ShowInfo("[ML-GATEWAY] Redis: %s, PostgreSQL: %s\n", 
	         redis_ctx_ ? "Connected" : "Disabled",
	         pg_conn_ ? "Connected" : "Failed");
	
	return true;
}

/**
 * Shutdown ML gateway
 */
void MobMLGateway::shutdown() {
	std::lock_guard<std::mutex> lock(mutex_);
	
	ShowStatus("[ML-GATEWAY] Shutting down ML Monster AI Gateway...\n");
	
	// Print final statistics
	ShowInfo("[ML-GATEWAY] Final Statistics:\n");
	ShowInfo("[ML-GATEWAY]   Total Requests: %llu\n", (unsigned long long)total_requests_);
	ShowInfo("[ML-GATEWAY]   Cache Hits: %llu (%.1f%%)\n", 
	         (unsigned long long)cache_hits_,
	         total_requests_ > 0 ? (float)cache_hits_ / total_requests_ * 100.0f : 0.0f);
	ShowInfo("[ML-GATEWAY]   ML Successes: %llu\n", (unsigned long long)ml_successes_);
	ShowInfo("[ML-GATEWAY]   Fallbacks: %llu\n", (unsigned long long)fallback_count_);
	ShowInfo("[ML-GATEWAY]   Avg Latency: %.2fms\n", avg_latency_ms_);
	
	// Close Redis
	if (redis_ctx_) {
		redisFree(redis_ctx_);
		redis_ctx_ = nullptr;
		ShowStatus("[ML-GATEWAY] Redis connection closed\n");
	}
	
	// Close PostgreSQL
	if (pg_conn_) {
		PQfinish(PGCONN());
		pg_conn_ = nullptr;
		ShowStatus("[ML-GATEWAY] PostgreSQL connection closed\n");
	}
	
	initialized_ = false;
	healthy_ = false;
	
	ShowStatus("[ML-GATEWAY] ML Monster AI Gateway shutdown complete\n");
}

/**
 * Get ML decision for monster
 */
MobMLGateway::MLDecision MobMLGateway::get_decision(struct mob_data* md) {
	nullpo_retr(MLDecision(), md);
	
	std::lock_guard<std::mutex> lock(mutex_);
	
	auto start_time = std::chrono::high_resolution_clock::now();
	MLDecision decision;
	
	total_requests_++;
	
	// Check if ML system is healthy
	if (!healthy_ || !initialized_) {
		decision.success = false;
		decision.error_message = "ML system not healthy";
		decision.action = MLAction::TRADITIONAL_AI;
		fallback_count_++;
		return decision;
	}
	
	// Periodic health check (every 100 ticks = ~10 seconds)
	t_tick tick = gettick();
	if (DIFF_TICK(tick, last_health_check_) >= battle_config.ml_health_check_interval * MIN_MOBTHINKTIME) {
		health_check();
		last_health_check_ = tick;
	}
	
	// Encode state
	std::vector<float> state = MobMLEncoder::encode_state(md);
	if (state.size() != MobMLEncoder::STATE_DIM) {
		ShowError("[ML-GATEWAY] State encoding failed for mob %d, expected %d dims, got %d\n",
		          md->id, (int)MobMLEncoder::STATE_DIM, (int)state.size());
		decision.success = false;
		decision.action = MLAction::TRADITIONAL_AI;
		fallback_count_++;
		return decision;
	}
	
	// Try cache first (if Redis available)
	if (battle_config.ml_cache_enabled && redis_ctx_) {
		decision = try_cache(state, md->mob_id);
		if (decision.success) {
			cache_hits_++;
			
			// Calculate latency
			auto end_time = std::chrono::high_resolution_clock::now();
			decision.latency = std::chrono::duration_cast<std::chrono::milliseconds>(
				end_time - start_time);
			
			return decision;
		}
	}
	
	// Cache miss - request ML inference via PostgreSQL
	const char* archetype = determine_archetype(md);
	decision = request_ml_inference(state, md->mob_id, archetype);
	
	// Update cache if successful
	if (decision.success && redis_ctx_ && decision.action != MLAction::TRADITIONAL_AI) {
		update_cache(state, decision.action, decision.confidence, md->mob_id);
	}
	
	// Calculate latency
	auto end_time = std::chrono::high_resolution_clock::now();
	decision.latency = std::chrono::duration_cast<std::chrono::milliseconds>(
		end_time - start_time);
	
	// Update statistics
	if (decision.success && decision.action != MLAction::TRADITIONAL_AI) {
		ml_successes_++;
	} else {
		fallback_count_++;
	}
	
	// Update average latency (exponential moving average)
	float alpha = 0.1f;  // Smoothing factor
	avg_latency_ms_ = avg_latency_ms_ * (1.0f - alpha) + decision.latency.count() * alpha;
	
	return decision;
}

/**
 * Check if ML system is healthy
 */
bool MobMLGateway::is_healthy() {
	return healthy_ && initialized_;
}

/**
 * Get system statistics
 */
void MobMLGateway::get_stats(uint64_t& total_requests, uint64_t& cache_hits, 
                             uint64_t& ml_successes, uint64_t& fallbacks,
                             float& avg_latency_ms) {
	std::lock_guard<std::mutex> lock(mutex_);
	
	total_requests = total_requests_;
	cache_hits = cache_hits_;
	ml_successes = ml_successes_;
	fallbacks = fallback_count_;
	avg_latency_ms = avg_latency_ms_;
}

/**
 * Try to get decision from Redis cache
 */
MobMLGateway::MLDecision MobMLGateway::try_cache(const std::vector<float>& state, uint32 mob_id) {
	MLDecision decision;
	
	if (!redis_ctx_) {
		decision.success = false;
		return decision;
	}
	
	// Generate cache key
	std::string cache_key = generate_cache_key(state, mob_id);
	
	// Query Redis
	redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "GET ml:action:%s", cache_key.c_str());
	
	if (!reply) {
		ShowWarning("[ML-GATEWAY] Redis command failed\n");
		decision.success = false;
		return decision;
	}
	
	if (reply->type == REDIS_REPLY_STRING) {
		// Parse cached action: format "action_id:confidence"
		std::string cached_value(reply->str);
		size_t colon_pos = cached_value.find(':');
		
		if (colon_pos != std::string::npos) {
			int action_id = std::stoi(cached_value.substr(0, colon_pos));
			float confidence = std::stof(cached_value.substr(colon_pos + 1));
			
			decision.action = static_cast<MLAction>(action_id);
			decision.confidence = confidence;
			decision.from_cache = true;
			decision.success = true;
			decision.request_id = 0;  // Cache hit, no request ID
		}
	}
	
	freeReplyObject(reply);
	
	return decision;
}

/**
 * Submit request to ML service via PostgreSQL
 */
MobMLGateway::MLDecision MobMLGateway::request_ml_inference(const std::vector<float>& state, 
                                                            uint32 mob_id, const char* archetype) {
	MLDecision decision;
	
	if (!pg_conn_) {
		decision.success = false;
		decision.error_message = "PostgreSQL not connected";
		decision.action = MLAction::TRADITIONAL_AI;
		return decision;
	}
	
	auto request_start = std::chrono::high_resolution_clock::now();
	
	// Build state vector array string for PostgreSQL
	std::ostringstream state_array;
	state_array << "ARRAY[";
	for (size_t i = 0; i < state.size(); i++) {
		if (i > 0) state_array << ",";
		state_array << std::fixed << std::setprecision(6) << state[i];
	}
	state_array << "]";
	
	// Insert request into ai_requests table
	std::ostringstream insert_query;
	insert_query << "INSERT INTO ai_requests (monster_id, mob_id, archetype, state_vector, "
	             << "map_id, position_x, position_y, hp_ratio, sp_ratio, status, priority) "
	             << "VALUES (" << mob_id << ", " << mob_id << ", '" << archetype << "', "
	             << state_array.str() << "::REAL[], 0, 0, 0, " 
	             << state[0] << ", " << state[1] << ", 'pending', 5) "
	             << "RETURNING request_id";
	
	PGresult* res = PQexec(PGCONN(), insert_query.str().c_str());
	
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ShowWarning("[ML-GATEWAY] Failed to insert request: %s\n", PQerrorMessage(PGCONN()));
		PQclear(res);
		decision.success = false;
		decision.error_message = "Failed to insert request";
		decision.action = MLAction::TRADITIONAL_AI;
		return decision;
	}
	
	// Get request_id
	uint32 request_id = 0;
	if (PQntuples(res) > 0) {
		request_id = (uint32)std::stoul(PQgetvalue(res, 0, 0));
		decision.request_id = request_id;
	}
	PQclear(res);
	
	if (request_id == 0) {
		ShowWarning("[ML-GATEWAY] Failed to get request_id\n");
		decision.success = false;
		decision.error_message = "Invalid request_id";
		decision.action = MLAction::TRADITIONAL_AI;
		return decision;
	}
	
	// Poll for response with timeout
	int32 max_latency_ms = battle_config.ml_max_latency_ms;
	int32 poll_interval_ms = 2;  // Poll every 2ms
	int32 polls = max_latency_ms / poll_interval_ms;
	
	std::ostringstream poll_query;
	poll_query << "SELECT action_id, confidence FROM ai_responses "
	           << "WHERE request_id = " << request_id 
	           << " LIMIT 1";
	
	for (int32 i = 0; i < polls; i++) {
		// Execute query
		PGresult* poll_res = PQexec(PGCONN(), poll_query.str().c_str());
		
		if (PQresultStatus(poll_res) == PGRES_TUPLES_OK && PQntuples(poll_res) > 0) {
			// Response found
			int action_id = std::stoi(PQgetvalue(poll_res, 0, 0));
			float confidence = std::stof(PQgetvalue(poll_res, 0, 1));
			
			decision.action = static_cast<MLAction>(action_id);
			decision.confidence = confidence;
			decision.success = true;
			decision.from_cache = false;
			
			PQclear(poll_res);
			
			// Calculate inference latency
			auto end_time = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
				end_time - request_start);
			
			if (battle_config.ml_debug_logging) {
				ShowInfo("[ML-GATEWAY] ML response received in %lldms: action=%d, confidence=%.2f\n",
				         (long long)duration.count(), action_id, confidence);
			}
			
			return decision;
		}
		
		PQclear(poll_res);
		
		// Sleep before next poll (use usleep for microsecond precision)
		usleep(poll_interval_ms * 1000);
	}
	
	// Timeout
	if (battle_config.ml_debug_logging) {
		ShowWarning("[ML-GATEWAY] ML request timeout after %dms (request_id=%u)\n",
		            max_latency_ms, request_id);
	}
	
	decision.success = false;
	decision.error_message = "Timeout waiting for ML response";
	decision.action = MLAction::TRADITIONAL_AI;
	
	// Mark request as timeout in database (background cleanup)
	std::ostringstream timeout_update;
	timeout_update << "UPDATE ai_requests SET status = 'timeout' "
	               << "WHERE request_id = " << request_id;
	PGresult* update_res = PQexec(PGCONN(), timeout_update.str().c_str());
	PQclear(update_res);
	
	return decision;
}

/**
 * Update Redis cache with result
 */
void MobMLGateway::update_cache(const std::vector<float>& state, MLAction action, 
                                float confidence, uint32 mob_id) {
	if (!redis_ctx_) return;
	
	// Generate cache key
	std::string cache_key = generate_cache_key(state, mob_id);
	
	// Format value: "action_id:confidence"
	std::ostringstream value;
	value << (int)action << ":" << std::fixed << std::setprecision(3) << confidence;
	
	// Set in Redis with TTL (configurable, default 5-10 seconds)
	int ttl_seconds = 10;  // 10 second cache
	
	redisReply* reply = (redisReply*)redisCommand(redis_ctx_, 
	                                               "SETEX ml:action:%s %d %s",
	                                               cache_key.c_str(),
	                                               ttl_seconds,
	                                               value.str().c_str());
	
	if (reply) {
		freeReplyObject(reply);
	}
}

/**
 * Generate cache key from state vector
 * Uses MD5 hash of quantized state vector
 */
std::string MobMLGateway::generate_cache_key(const std::vector<float>& state, uint32 mob_id) {
	// Quantize state to 2 decimal places for better cache hit rate
	std::ostringstream quantized;
	for (size_t i = 0; i < state.size(); i++) {
		if (i > 0) quantized << ",";
		quantized << std::fixed << std::setprecision(2) << state[i];
	}
	
	// Add mob_id to key to separate different monster types
	quantized << ":" << mob_id;
	
	// Generate MD5 hash
	std::string quantized_str = quantized.str();
	char md5_out[33] = {0};
	MD5_String(quantized_str.c_str(), md5_out);
	
	return std::string(md5_out);
}

/**
 * Determine archetype from mob_data
 * Maps monster characteristics to one of 6 archetypes
 */
const char* MobMLGateway::determine_archetype(const struct mob_data* md) {
	// Same logic as in encoder, but return string
	if (status_has_mode(&md->status, MD_STATUSIMMUNE) || md->status.max_hp > 100000) {
		return "tank";
	} else if (md->status.rhw.range > 3) {
		return "ranged";
	} else if (md->status.int_ > md->status.str && md->status.rhw.atk < 500) {
		return "mage";
	} else if (md->status.def > 50 || md->status.mdef > 30) {
		return "defensive";
	} else if (!md->db->skill.empty()) {
		// Check for support skills
		for (const auto& skill : md->db->skill) {
			if (skill->skill_id == AL_HEAL || skill->skill_id == AL_BLESSING) {
				return "support";
			}
		}
	}
	
	return "aggressive";  // Default
}

/**
 * Connect to Redis
 */
bool MobMLGateway::connect_redis() {
	// Connect to localhost:6379 (default Redis/DragonFly port)
	struct timeval timeout = {1, 500000};  // 1.5 seconds
	redis_ctx_ = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
	
	if (!redis_ctx_ || redis_ctx_->err) {
		if (redis_ctx_) {
			ShowError("[ML-GATEWAY] Redis connection error: %s\n", redis_ctx_->errstr);
			redisFree(redis_ctx_);
			redis_ctx_ = nullptr;
		}
		return false;
	}
	
	// Test connection with PING
	redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "PING");
	if (!reply || reply->type != REDIS_REPLY_STATUS || 
	    strcmp(reply->str, "PONG") != 0) {
		ShowError("[ML-GATEWAY] Redis PING failed\n");
		if (reply) freeReplyObject(reply);
		redisFree(redis_ctx_);
		redis_ctx_ = nullptr;
		return false;
	}
	freeReplyObject(reply);
	
	return true;
}

/**
 * Connect to PostgreSQL
 */
bool MobMLGateway::connect_postgresql() {
	// Connection string (from environment or config)
	const char* conninfo = "host=localhost port=5432 dbname=ragnarok_ml "
	                       "user=ml_user connect_timeout=10";
	
	pg_conn_ = static_cast<void*>(PQconnectdb(conninfo));
	
	if (PQstatus(PGCONN()) != CONNECTION_OK) {
		ShowError("[ML-GATEWAY] PostgreSQL connection error: %s\n", 
		          PQerrorMessage(PGCONN()));
		PQfinish(PGCONN());
		pg_conn_ = nullptr;
		return false;
	}
	
	// Test connection
	PGresult* res = PQexec(PGCONN(), "SELECT 1");
	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		ShowError("[ML-GATEWAY] PostgreSQL test query failed: %s\n", 
		          PQerrorMessage(PGCONN()));
		PQclear(res);
		PQfinish(PGCONN());
		pg_conn_ = nullptr;
		return false;
	}
	PQclear(res);
	
	// Set client encoding
	PQexec(PGCONN(), "SET client_encoding = 'UTF8'");
	
	return true;
}

/**
 * Perform health check
 */
void MobMLGateway::health_check() {
	bool prev_healthy = healthy_;
	healthy_ = true;  // Assume healthy until proven otherwise
	
	// Check Redis (optional)
	if (redis_ctx_) {
		redisReply* reply = (redisReply*)redisCommand(redis_ctx_, "PING");
		if (!reply || reply->type != REDIS_REPLY_STATUS) {
			ShowWarning("[ML-GATEWAY] Redis health check failed, reconnecting...\n");
			if (reply) freeReplyObject(reply);
			
			// Try reconnect
			redisFree(redis_ctx_);
			redis_ctx_ = nullptr;
			connect_redis();
		} else {
			freeReplyObject(reply);
		}
	}
	
	// Check PostgreSQL (required)
	if (pg_conn_) {
		PGresult* res = PQexec(PGCONN(), "SELECT 1");
		if (PQresultStatus(res) != PGRES_TUPLES_OK) {
			ShowWarning("[ML-GATEWAY] PostgreSQL health check failed: %s\n", 
			            PQerrorMessage(PGCONN()));
			PQclear(res);
			healthy_ = false;
			
			// Try reconnect
			PQfinish(PGCONN());
			pg_conn_ = nullptr;
			
			if (connect_postgresql()) {
				ShowStatus("[ML-GATEWAY] PostgreSQL reconnected successfully\n");
				healthy_ = true;
			} else {
				ShowError("[ML-GATEWAY] PostgreSQL reconnection failed\n");
			}
		} else {
			PQclear(res);
		}
	} else {
		healthy_ = false;
	}
	
	// Log health status change
	if (prev_healthy && !healthy_) {
		ShowError("[ML-GATEWAY] System became UNHEALTHY\n");
	} else if (!prev_healthy && healthy_) {
		ShowStatus("[ML-GATEWAY] System became HEALTHY\n");
	}
}

/**
 * Reconnect if connection lost
 */
bool MobMLGateway::reconnect_if_needed() {
	if (!pg_conn_ || PQstatus(PGCONN()) != CONNECTION_OK) {
		ShowWarning("[ML-GATEWAY] PostgreSQL connection lost, reconnecting...\n");
		
		if (pg_conn_) {
			PQfinish(PGCONN());
			pg_conn_ = nullptr;
		}
		
		return connect_postgresql();
	}
	
	return true;
}
