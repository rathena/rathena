#pragma once
#include <string>
#include <vector>
#include <optional>
#include <mutex>

class RedisClient {
public:
    RedisClient();
    bool connect(const std::string& host, int port);
    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    std::vector<std::string> keys(const std::string& pattern);
    bool hset(const std::string& hash, const std::string& field, const std::string& value);
    std::optional<std::string> hget(const std::string& hash, const std::string& field);
    bool hdel(const std::string& hash, const std::string& field);
    std::vector<std::string> hkeys(const std::string& hash);

private:
    std::mutex mutex_;
    // Internal connection handle, e.g., hiredis context
    void* context_;
    bool connected_;
    void reconnect();
};