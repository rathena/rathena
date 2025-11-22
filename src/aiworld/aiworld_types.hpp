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
    AIACTION = 5,  // Alias for AI_ACTION_RESPONSE (used by native commands)
    HEARTBEAT = 6,
    DIALOGUE = 7,  // AI dialogue interactions
    DECISION = 8,  // AI decision-making
    ERROR = 99
};

} // namespace aiworld