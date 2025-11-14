#pragma once
// aiworld_messages.hpp
// Message format definitions for rAthena <-> AIWorld ZeroMQ IPC

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace aiworld {

// Message base struct
struct AIWorldMessage {
    int32_t message_type; // See IPCMessageType
    std::string correlation_id; // For request/response matching
    nlohmann::json payload;     // JSON-encoded message data

    AIWorldMessage() : message_type(0) {}
    AIWorldMessage(int32_t type, const std::string& corr, const nlohmann::json& data)
        : message_type(type), correlation_id(corr), payload(data) {}
};

// Entity state sync message
struct EntityStateSync {
    std::string entity_id;
    std::string entity_type; // "player", "npc"
    nlohmann::json state;    // Arbitrary state data
};

// Mission assignment message
struct MissionAssignment {
    std::string mission_id;
    std::string assignee_id;
    nlohmann::json mission_data;
};

// Event notification message
struct EventNotification {
    std::string event_id;
    std::string event_type;
    nlohmann::json event_data;
};

// AI action request/response message
struct AIAction {
    std::string entity_id;
    std::string action_type;
    nlohmann::json action_params;
    nlohmann::json result; // For responses
};

// Serialization helpers
inline nlohmann::json to_json(const EntityStateSync& msg) {
    return nlohmann::json{
        {"entity_id", msg.entity_id},
        {"entity_type", msg.entity_type},
        {"state", msg.state}
    };
}
inline nlohmann::json to_json(const MissionAssignment& msg) {
    return nlohmann::json{
        {"mission_id", msg.mission_id},
        {"assignee_id", msg.assignee_id},
        {"mission_data", msg.mission_data}
    };
}
inline nlohmann::json to_json(const EventNotification& msg) {
    return nlohmann::json{
        {"event_id", msg.event_id},
        {"event_type", msg.event_type},
        {"event_data", msg.event_data}
    };
}
inline nlohmann::json to_json(const AIAction& msg) {
    return nlohmann::json{
        {"entity_id", msg.entity_id},
        {"action_type", msg.action_type},
        {"action_params", msg.action_params},
        {"result", msg.result}
    };
}

} // namespace aiworld