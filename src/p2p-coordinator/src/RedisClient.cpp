#include "RedisClient.h"
#include "Logger.h"
#include <stdexcept>
#include <hiredis/hiredis.h>

RedisClient::RedisClient() : context_(nullptr), connected_(false) {}

RedisClient::~RedisClient() {
    disconnect();
}

bool RedisClient::connect(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    context_ = redisConnect(host.c_str(), port);
    if (context_ == nullptr || context_->err) {
        if (context_) {
            Logger::error("Redis connection error: " + std::string(context_->errstr));
            redisFree(context_);
            context_ = nullptr;
        } else {
            Logger::error("Redis connection error: can't allocate context");
        }
        connected_ = false;
        return false;
    }
    
    connected_ = true;
    Logger::info("Connected to Redis at " + host + ":" + std::to_string(port));
    return connected_;
}

void RedisClient::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
    connected_ = false;
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return false;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "SET %s %s", key.c_str(), value.c_str()));
    if (!reply) {
        Logger::error("Redis SET failed: " + std::string(context_->errstr));
        return false;
    }
    bool success = (reply->type != REDIS_REPLY_ERROR);
    freeReplyObject(reply);
    return success;
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return std::nullopt;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "GET %s", key.c_str()));
    if (!reply || reply->type == REDIS_REPLY_ERROR || reply->type == REDIS_REPLY_NIL) {
        if (reply) freeReplyObject(reply);
        return std::nullopt;
    }
    std::string result(reply->str, reply->len);
    freeReplyObject(reply);
    return result;
}

bool RedisClient::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return false;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "DEL %s", key.c_str()));
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

std::vector<std::string> RedisClient::keys(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    if (!context_) return result;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "KEYS %s", pattern.c_str()));
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        if (reply) freeReplyObject(reply);
        return result;
    }
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            result.push_back(std::string(reply->element[i]->str, reply->element[i]->len));
        }
    }
    freeReplyObject(reply);
    return result;
}

bool RedisClient::hset(const std::string& hash, const std::string& field, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return false;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "HSET %s %s %s", hash.c_str(), field.c_str(), value.c_str()));
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

std::optional<std::string> RedisClient::hget(const std::string& hash, const std::string& field) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return std::nullopt;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "HGET %s %s", hash.c_str(), field.c_str()));
    if (!reply || reply->type == REDIS_REPLY_ERROR || reply->type == REDIS_REPLY_NIL) {
        if (reply) freeReplyObject(reply);
        return std::nullopt;
    }
    std::string result(reply->str, reply->len);
    freeReplyObject(reply);
    return result;
}

bool RedisClient::hdel(const std::string& hash, const std::string& field) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!context_) return false;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "HDEL %s %s", hash.c_str(), field.c_str()));
    if (!reply) return false;
    freeReplyObject(reply);
    return true;
}

std::vector<std::string> RedisClient::hkeys(const std::string& hash) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> result;
    if (!context_) return result;
    
    redisReply* reply = static_cast<redisReply*>(redisCommand(context_, "HKEYS %s", hash.c_str()));
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        if (reply) freeReplyObject(reply);
        return result;
    }
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (size_t i = 0; i < reply->elements; i++) {
            result.push_back(std::string(reply->element[i]->str, reply->element[i]->len));
        }
    }
    freeReplyObject(reply);
    return result;
}

void RedisClient::reconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    disconnect();
    // Reconnection requires host/port to be stored - for now just mark disconnected
    connected_ = false;
    Logger::warn("Redis reconnection requested - connection parameters not stored for auto-reconnect");
}
