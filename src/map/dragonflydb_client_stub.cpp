// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// Production implementation of DragonflyDBClient for distributed state management

#include "dragonflydb_client.hpp"
#include <hiredis/hiredis.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <mutex>
#include <sstream>

class DragonflyDBClientImpl : public DragonflyDBClient {
public:
    DragonflyDBClientImpl() : redis_context_(nullptr) {
        connect();
    }

    ~DragonflyDBClientImpl() {
        disconnect();
    }

    void update_entity_assignment(uint64_t entity_id, int worker_id) override {
        std::lock_guard<std::mutex> lock(redis_mutex_);
        
        if (!ensure_connection()) {
            std::cerr << "[DragonflyDB] Failed to connect for entity assignment" << std::endl;
            return;
        }

        // HSET worker:{worker_id}:entities {entity_id} {entity_type}
        std::string hset_cmd = "HSET worker:" + std::to_string(worker_id) + ":entities " +
                               std::to_string(entity_id) + " npc";
        
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(redis_context_, hset_cmd.c_str())
        );
        
        if (!reply) {
            std::cerr << "[DragonflyDB] HSET command failed: " << redis_context_->errstr << std::endl;
            reconnect();
            return;
        }
        
        freeReplyObject(reply);
        
        // SET entity:{entity_id}:worker {worker_id}
        std::string set_cmd = "SET entity:" + std::to_string(entity_id) + ":worker " +
                              std::to_string(worker_id);
        
        reply = static_cast<redisReply*>(
            redisCommand(redis_context_, set_cmd.c_str())
        );
        
        if (!reply) {
            std::cerr << "[DragonflyDB] SET command failed: " << redis_context_->errstr << std::endl;
            reconnect();
            return;
        }
        
        freeReplyObject(reply);
        
        std::cout << "[DragonflyDB] ✓ Entity " << entity_id << " assigned to worker "
                  << worker_id << std::endl;
    }

    void update_entity_migration(uint64_t entity_id, int from_worker, int to_worker) override {
        std::lock_guard<std::mutex> lock(redis_mutex_);
        
        if (!ensure_connection()) {
            std::cerr << "[DragonflyDB] Failed to connect for entity migration" << std::endl;
            return;
        }

        // Remove from old worker's entity set
        std::string hdel_cmd = "HDEL worker:" + std::to_string(from_worker) + ":entities " +
                               std::to_string(entity_id);
        
        redisReply* reply = static_cast<redisReply*>(
            redisCommand(redis_context_, hdel_cmd.c_str())
        );
        
        if (reply) {
            freeReplyObject(reply);
        }
        
        // Add to new worker's entity set
        std::string hset_cmd = "HSET worker:" + std::to_string(to_worker) + ":entities " +
                               std::to_string(entity_id) + " npc";
        
        reply = static_cast<redisReply*>(
            redisCommand(redis_context_, hset_cmd.c_str())
        );
        
        if (reply) {
            freeReplyObject(reply);
        }
        
        // Update entity worker mapping
        std::string set_cmd = "SET entity:" + std::to_string(entity_id) + ":worker " +
                              std::to_string(to_worker);
        
        reply = static_cast<redisReply*>(
            redisCommand(redis_context_, set_cmd.c_str())
        );
        
        if (reply) {
            freeReplyObject(reply);
        }
        
        // Publish migration event for subscribers
        std::ostringstream event_json;
        event_json << "{\"entity\":\"" << entity_id << "\","
                   << "\"from\":\"" << from_worker << "\","
                   << "\"to\":\"" << to_worker << "\"}";
        
        std::string publish_cmd = "PUBLISH entity:migrations \"" + event_json.str() + "\"";
        
        reply = static_cast<redisReply*>(
            redisCommand(redis_context_, publish_cmd.c_str())
        );
        
        if (reply) {
            freeReplyObject(reply);
        }
        
        std::cout << "[DragonflyDB] ✓ Entity " << entity_id << " migrated from worker "
                  << from_worker << " to " << to_worker << std::endl;
    }

private:
    redisContext* redis_context_;
    std::mutex redis_mutex_;

    void connect() {
        // Read config from environment variables
        const char* host_env = std::getenv("DRAGONFLY_HOST");
        const char* port_env = std::getenv("DRAGONFLY_PORT");
        const char* password_env = std::getenv("DRAGONFLY_PASSWORD");
        
        std::string host = host_env ? host_env : "127.0.0.1";
        int port = port_env ? std::atoi(port_env) : 6379;
        std::string password = password_env ? password_env : "";
        
        std::cout << "[DragonflyDB] Connecting to " << host << ":" << port << std::endl;
        
        redis_context_ = redisConnect(host.c_str(), port);
        
        if (redis_context_ == nullptr || redis_context_->err) {
            if (redis_context_) {
                std::cerr << "[DragonflyDB] Connection error: " << redis_context_->errstr << std::endl;
                redisFree(redis_context_);
                redis_context_ = nullptr;
            } else {
                std::cerr << "[DragonflyDB] Connection error: can't allocate redis context" << std::endl;
            }
            return;
        }
        
        // Authenticate if password provided
        if (!password.empty()) {
            redisReply* reply = static_cast<redisReply*>(
                redisCommand(redis_context_, "AUTH %s", password.c_str())
            );
            
            if (!reply || reply->type == REDIS_REPLY_ERROR) {
                std::cerr << "[DragonflyDB] Authentication failed" << std::endl;
                if (reply) freeReplyObject(reply);
                disconnect();
                return;
            }
            
            freeReplyObject(reply);
        }
        
        std::cout << "[DragonflyDB] ✓ Connected successfully" << std::endl;
    }

    void disconnect() {
        if (redis_context_) {
            redisFree(redis_context_);
            redis_context_ = nullptr;
        }
    }

    void reconnect() {
        std::cerr << "[DragonflyDB] Reconnecting..." << std::endl;
        disconnect();
        connect();
    }

    bool ensure_connection() {
        if (!redis_context_) {
            connect();
        }
        
        if (redis_context_ && redis_context_->err) {
            reconnect();
        }
        
        return redis_context_ != nullptr && !redis_context_->err;
    }
};

// Factory for use in map server initialization
std::shared_ptr<DragonflyDBClient> create_dragonflydb_client_stub() {
    return std::make_shared<DragonflyDBClientImpl>();
}