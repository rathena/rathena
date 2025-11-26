/**
 * aiworld_callbacks.cpp
 * Callback Registry Implementation
 */

#include "aiworld_callbacks.hpp"
#include "aiworld_utils.hpp"

namespace aiworld {

CallbackRegistry& CallbackRegistry::getInstance() {
    static CallbackRegistry instance;
    return instance;
}

void CallbackRegistry::registerCallback(IPCMessageType type, IPCCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_[static_cast<int32_t>(type)] = callback;
    log_info("CallbackRegistry: Registered callback for message type " + 
             std::to_string(static_cast<int32_t>(type)));
}

void CallbackRegistry::unregisterCallback(IPCMessageType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(static_cast<int32_t>(type));
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
        log_info("CallbackRegistry: Unregistered callback for message type " + 
                 std::to_string(static_cast<int32_t>(type)));
    }
}

bool CallbackRegistry::dispatch(IPCMessageType type, const nlohmann::json& payload) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = callbacks_.find(static_cast<int32_t>(type));
    
    if (it == callbacks_.end()) {
        log_warn("CallbackRegistry: No callback registered for message type " +
                 std::to_string(static_cast<int32_t>(type)));
        return false;
    }
    
    try {
        // Invoke callback (unlock before calling to avoid deadlock)
        IPCCallback callback = it->second;
        mutex_.unlock();
        callback(type, payload);
        mutex_.lock();
        return true;
    } catch (const std::exception& e) {
        log_error("CallbackRegistry: Exception in callback dispatch: " + std::string(e.what()));
        return false;
    }
}

bool CallbackRegistry::hasCallback(IPCMessageType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return callbacks_.find(static_cast<int32_t>(type)) != callbacks_.end();
}

} // namespace aiworld