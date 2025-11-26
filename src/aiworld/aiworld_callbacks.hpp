#pragma once
/**
 * aiworld_callbacks.hpp
 * Callback Registry for IPC Event Dispatching
 * 
 * Provides thread-safe callback registration and dispatching for async IPC messages
 */

#include <functional>
#include <unordered_map>
#include <mutex>
#include <string>
#include <nlohmann/json.hpp>
#include "aiworld_types.hpp"

namespace aiworld {

// Callback function type: receives message type and payload JSON
using IPCCallback = std::function<void(IPCMessageType, const nlohmann::json&)>;

/**
 * Callback Registry (Singleton)
 * Thread-safe storage and invocation of IPC message handlers
 */
class CallbackRegistry {
public:
    static CallbackRegistry& getInstance();
    
    // Register callback for specific message type
    void registerCallback(IPCMessageType type, IPCCallback callback);
    
    // Unregister callback for message type
    void unregisterCallback(IPCMessageType type);
    
    // Dispatch message to registered callback
    bool dispatch(IPCMessageType type, const nlohmann::json& payload);
    
    // Check if handler exists for message type
    bool hasCallback(IPCMessageType type) const;
    
private:
    CallbackRegistry() = default;
    std::unordered_map<int32_t, IPCCallback> callbacks_;
    mutable std::mutex mutex_;
};

} // namespace aiworld