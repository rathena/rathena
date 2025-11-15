#pragma once
// aiworld_types.hpp
// Common type definitions for rAthena <-> AIWorld ZeroMQ IPC

#include <cstdint>

namespace aiworld {

// IPC message types (strongly typed enum class)
enum class IPCMessageType : int32_t {
    ENTITY_STATE_SYNC = 1,
    MISSION_ASSIGNMENT = 2,
    EVENT_NOTIFICATION = 3,
    AI_ACTION_REQUEST = 4,
    AI_ACTION_RESPONSE = 5,
    HEARTBEAT = 6,
    ERROR = 99
};

} // namespace aiworld