/**
 * @file ai_agent.hpp
 * @brief AI agent interface for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the AI agent interface that defines the contract
 * for AI agents (World Evolution, Legend Bloodlines, etc.).
 */

#ifndef AI_AGENT_HPP
#define AI_AGENT_HPP

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "../../common/cbasetypes.hpp"
#include "ai_request.hpp"
#include "ai_response.hpp"

// Forward declaration
class AIModule;

/**
 * @brief AI agent interface
 * 
 * This class defines the interface for AI agents.
 */
class AIAgent {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~AIAgent() = default;

    /**
     * @brief Initialize the agent
     * @return True if initialization was successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Finalize the agent
     */
    virtual void finalize() = 0;

    /**
     * @brief Check if the agent is enabled
     * @return True if enabled, false otherwise
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Get the agent name
     * @return Agent name
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get the agent description
     * @return Agent description
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Update the agent
     * @param tick Current tick value
     */
    virtual void update(uint64 tick) = 0;

    /**
     * @brief Process an AI request
     * @param request The AI request to process
     * @return The AI response
     */
    virtual AIResponse processRequest(const AIRequest& request) = 0;

    /**
     * @brief Reload configuration
     * @return True if reload was successful, false otherwise
     */
    virtual bool reloadConfig() = 0;

    /**
     * @brief Get the agent's status
     * @return JSON string describing the agent's status
     */
    virtual std::string getStatus() const = 0;

    /**
     * @brief Get the agent's statistics
     * @return JSON string containing the agent's statistics
     */
    virtual std::string getStatistics() const = 0;

    /**
     * @brief Handle a script command
     * @param command Command name
     * @param params Command parameters
     * @return Command result
     */
    virtual std::string handleScriptCommand(const std::string& command, const std::vector<std::string>& params) = 0;

    /**
     * @brief Handle a player event
     * @param eventName Event name
     * @param charId Character ID
     * @param params Event parameters
     */
    virtual void handlePlayerEvent(const std::string& eventName, int charId, const std::unordered_map<std::string, std::string>& params) = 0;

    /**
     * @brief Handle a world event
     * @param eventName Event name
     * @param params Event parameters
     */
    virtual void handleWorldEvent(const std::string& eventName, const std::unordered_map<std::string, std::string>& params) = 0;
};

#endif // AI_AGENT_HPP