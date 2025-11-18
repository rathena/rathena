#pragma once
// aiworld_messages.hpp
// Message format definitions for rAthena <-> AIWorld ZeroMQ IPC

#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "aiworld_types.hpp"

namespace aiworld {

// Message base struct
struct AIWorldMessage {
    IPCMessageType message_type; // See IPCMessageType (enum class)
    std::string correlation_id; // For request/response matching
    nlohmann::json payload;     // JSON-encoded message data

    AIWorldMessage() : message_type(IPCMessageType::ERROR) {}
    AIWorldMessage(IPCMessageType type, const std::string& corr, const nlohmann::json& data)
        : message_type(type), correlation_id(corr), payload(data) {}
};

// Entity state sync message (expanded for world concept design)
struct EntityStateSync {
    std::string entity_id;
    std::string entity_type; // "player", "npc"

    // Core Identity
    nlohmann::json personality; // Big Five + custom traits
    std::string background_story;
    nlohmann::json skills; // combat, social, economic, creative
    nlohmann::json physical; // appearance, age, health

    // Moral Alignment
    nlohmann::json moral_alignment; // dynamic alignment spectrum

    // Memory System
    nlohmann::json episodic_memory;
    nlohmann::json semantic_memory;
    nlohmann::json procedural_memory;

    // Goal and Motivation
    nlohmann::json goals; // short-term, long-term, hierarchy

    // Emotional System
    nlohmann::json emotional_state; // current emotion, mood, triggers

    // Social/Relationship System
    nlohmann::json relationships; // type, strength, reputation

    // World/Environment context (optional)
    nlohmann::json environment_state;

    // For extensibility
    nlohmann::json extra;

    // For backward compatibility
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

/**
 * Serialization helpers
 * Expanded to include all new fields for world concept design.
 */
inline nlohmann::json to_json(const EntityStateSync& msg) {
    return nlohmann::json{
        {"entity_id", msg.entity_id},
        {"entity_type", msg.entity_type},
        {"personality", msg.personality},
        {"background_story", msg.background_story},
        {"skills", msg.skills},
        {"physical", msg.physical},
        {"moral_alignment", msg.moral_alignment},
        {"episodic_memory", msg.episodic_memory},
        {"semantic_memory", msg.semantic_memory},
        {"procedural_memory", msg.procedural_memory},
        {"goals", msg.goals},
        {"emotional_state", msg.emotional_state},
        {"relationships", msg.relationships},
        {"environment_state", msg.environment_state},
        {"extra", msg.extra},
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