#include "aiworld_messages.hpp"
#include <nlohmann/json.hpp>

namespace aiworld {

// --- NPCRegistration ---
nlohmann::json NPCRegistration::to_json() const {
    nlohmann::json j;
    j["npc_id"] = npc_id;
    j["entity_type"] = entity_type;
    j["personality"] = personality;
    j["background_story"] = background_story;
    j["skills"] = skills;
    j["physical_characteristics"] = physical_characteristics;
    j["initial_state"] = initial_state;
    return j;
}

NPCRegistration NPCRegistration::from_json(const nlohmann::json& j) {
    NPCRegistration reg;
    reg.npc_id = j.value("npc_id", "");
    reg.entity_type = j.value("entity_type", "npc");
    reg.personality = j.value("personality", nlohmann::json::object());
    reg.background_story = j.value("background_story", nlohmann::json::object());
    reg.skills = j.value("skills", nlohmann::json::object());
    reg.physical_characteristics = j.value("physical_characteristics", nlohmann::json::object());
    reg.initial_state = j.value("initial_state", nlohmann::json::object());
    return reg;
}

// --- EventNotification ---
nlohmann::json EventNotification::to_json() const {
    nlohmann::json j;
    j["event_id"] = event_id;
    j["event_type"] = event_type;
    j["source_id"] = source_id;
    j["target_id"] = target_id;
    j["event_data"] = event_data;
    j["timestamp"] = timestamp;
    return j;
}

EventNotification EventNotification::from_json(const nlohmann::json& j) {
    EventNotification ev;
    ev.event_id = j.value("event_id", "");
    ev.event_type = j.value("event_type", "");
    ev.source_id = j.value("source_id", "");
    ev.target_id = j.value("target_id", "");
    ev.event_data = j.value("event_data", nlohmann::json::object());
    ev.timestamp = j.value("timestamp", 0);
    return ev;
}

// --- InteractionRequest ---
nlohmann::json InteractionRequest::to_json() const {
    nlohmann::json j;
    j["npc_id"] = npc_id;
    j["interaction_type"] = interaction_type;
    j["initiator_id"] = initiator_id;
    j["interaction_data"] = interaction_data;
    j["timestamp"] = timestamp;
    return j;
}

InteractionRequest InteractionRequest::from_json(const nlohmann::json& j) {
    InteractionRequest req;
    req.npc_id = j.value("npc_id", "");
    req.interaction_type = j.value("interaction_type", "");
    req.initiator_id = j.value("initiator_id", "");
    req.interaction_data = j.value("interaction_data", nlohmann::json::object());
    req.timestamp = j.value("timestamp", 0);
    return req;
}

// --- NPCState ---
nlohmann::json NPCState::to_json() const {
    nlohmann::json j;
    j["npc_id"] = this->npc_id;
    j["personality"] = this->personality;
    j["background_story"] = this->background_story;
    j["skills"] = this->skills;
    j["physical"] = this->physical;
    j["moral_alignment"] = this->moral_alignment;
    j["episodic_memory"] = this->episodic_memory;
    j["semantic_memory"] = this->semantic_memory;
    j["procedural_memory"] = this->procedural_memory;
    j["goals"] = this->goals;
    j["emotional_state"] = this->emotional_state;
    j["relationships"] = this->relationships;
    j["environment_state"] = this->environment_state;
    j["extra"] = this->extra;
    j["state"] = this->state;
    return j;
}

NPCState NPCState::from_json(const nlohmann::json& j) {
    NPCState st;
    st.npc_id = j.value("npc_id", "");
    st.personality = j.value("personality", nlohmann::json::object());
    st.background_story = j.value("background_story", nlohmann::json::object());
    st.skills = j.value("skills", nlohmann::json::object());
    st.physical = j.value("physical", nlohmann::json::object());
    st.moral_alignment = j.value("moral_alignment", nlohmann::json::object());
    st.episodic_memory = j.value("episodic_memory", nlohmann::json::object());
    st.semantic_memory = j.value("semantic_memory", nlohmann::json::object());
    st.procedural_memory = j.value("procedural_memory", nlohmann::json::object());
    st.goals = j.value("goals", nlohmann::json::object());
    st.emotional_state = j.value("emotional_state", nlohmann::json::object());
    st.relationships = j.value("relationships", nlohmann::json::object());
    st.environment_state = j.value("environment_state", nlohmann::json::object());
    st.extra = j.value("extra", nlohmann::json::object());
    st.state = j.value("state", nlohmann::json::object());
    return st;
}

} // namespace aiworld