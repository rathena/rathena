#include "AIServiceClient.h"
#include "Logger.h"
#include <stdexcept>
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

AIServiceClient::AIServiceClient() {
    endpoint_ = "http://localhost:8000/api/npc";
}

std::optional<std::string> AIServiceClient::getNpcState(const std::string& npcId) {
    try {
        httplib::Client cli("localhost", 8000);
        cli.set_connection_timeout(5);
        
        auto res = cli.Get(("/api/npc/" + npcId).c_str());
        if (res && res->status == 200) {
            Logger::info("Queried AI service for NPC state: " + npcId);
            return res->body;
        }
        Logger::warn("AI service returned status " + std::to_string(res ? res->status : 0) + " for NPC: " + npcId);
        return std::nullopt;
    } catch (const std::exception& ex) {
        Logger::error("Failed to query AI service for NPC " + npcId + ": " + ex.what());
        return std::nullopt;
    }
}

bool AIServiceClient::updateNpcState(const std::string& npcId, const std::string& state) {
    try {
        httplib::Client cli("localhost", 8000);
        cli.set_connection_timeout(5);
        
        auto res = cli.Post(("/api/npc/" + npcId + "/state").c_str(), state, "application/json");
        if (res && res->status == 200) {
            Logger::info("Updated AI service NPC state: " + npcId);
            return true;
        }
        Logger::warn("Failed to update NPC state via AI service: " + npcId);
        return false;
    } catch (const std::exception& ex) {
        Logger::error("Failed to update NPC state for " + npcId + ": " + ex.what());
        return false;
    }
}

void AIServiceClient::connect() {
    try {
        httplib::Client cli("localhost", 8000);
        cli.set_connection_timeout(5);
        auto res = cli.Get("/health");
        if (res && res->status == 200) {
            Logger::info("Connected to AI service successfully");
        } else {
            Logger::warn("AI service health check failed");
        }
    } catch (const std::exception& ex) {
        Logger::warn("AI service not available: " + std::string(ex.what()));
    }
}
