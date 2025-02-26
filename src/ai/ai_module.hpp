/**
 * @file ai_module.hpp
 * @brief Main AI module for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the main AI module that manages AI providers and agents
 * for the rAthena AI World project.
 */

#ifndef AI_MODULE_HPP
#define AI_MODULE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp"
#include "../common/timer.hpp"

// Forward declarations
class AIProvider;
class AIAgent;
class AIRequest;
class AIResponse;

/**
 * @brief Main AI module class
 * 
 * This class manages AI providers and agents, handles configuration loading,
 * and provides the main interface for AI functionality.
 */
class AIModule {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static AIModule& getInstance();

    /**
     * @brief Initialize the AI module
     * @return True if initialization was successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Finalize the AI module
     */
    void finalize();

    /**
     * @brief Check if the AI module is enabled
     * @return True if enabled, false otherwise
     */
    bool isEnabled() const;

    /**
     * @brief Get the primary AI provider
     * @return Pointer to the primary AI provider
     */
    std::shared_ptr<AIProvider> getPrimaryProvider();

    /**
     * @brief Get the fallback AI provider
     * @return Pointer to the fallback AI provider
     */
    std::shared_ptr<AIProvider> getFallbackProvider();

    /**
     * @brief Get an AI provider by name
     * @param providerName Name of the provider
     * @return Pointer to the requested provider, or nullptr if not found
     */
    std::shared_ptr<AIProvider> getProvider(const std::string& providerName);

    /**
     * @brief Get an AI agent by name
     * @param agentName Name of the agent
     * @return Pointer to the requested agent, or nullptr if not found
     */
    std::shared_ptr<AIAgent> getAgent(const std::string& agentName);

    /**
     * @brief Process an AI request
     * @param request The AI request to process
     * @return The AI response
     */
    AIResponse processRequest(const AIRequest& request);

    /**
     * @brief Update all AI agents
     * @param tick Current tick value
     */
    void update(uint64 tick);

    /**
     * @brief Reload configuration
     * @return True if reload was successful, false otherwise
     */
    bool reloadConfig();

    /**
     * @brief Log an AI-related message
     * @param level Log level
     * @param component Component name
     * @param message Log message
     * @param details Additional details (optional)
     */
    void log(int level, const std::string& component, const std::string& message, const std::string& details = "");

private:
    /**
     * @brief Constructor
     */
    AIModule();

    /**
     * @brief Destructor
     */
    ~AIModule();

    /**
     * @brief Copy constructor (deleted)
     */
    AIModule(const AIModule&) = delete;

    /**
     * @brief Assignment operator (deleted)
     */
    AIModule& operator=(const AIModule&) = delete;

    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Initialize providers
     * @return True if initialization was successful, false otherwise
     */
    bool initializeProviders();

    /**
     * @brief Initialize agents
     * @return True if initialization was successful, false otherwise
     */
    bool initializeAgents();

    /**
     * @brief Timer callback for updating agents
     * @param tid Timer ID
     * @param tick Current tick value
     * @param id User data ID
     * @param data User data pointer
     * @return Interval for the next timer call
     */
    static TIMER_FUNC(updateAgentsTimer);

    bool m_initialized;                                                  ///< Initialization flag
    bool m_enabled;                                                      ///< Enabled flag
    int m_logLevel;                                                      ///< Log level
    bool m_cacheEnabled;                                                 ///< Cache enabled flag
    int m_cacheExpiry;                                                   ///< Cache expiry time in seconds
    int m_requestTimeout;                                                ///< Request timeout in milliseconds
    bool m_fallbackEnabled;                                              ///< Fallback enabled flag
    std::string m_primaryProviderName;                                   ///< Primary provider name
    std::string m_fallbackProviderName;                                  ///< Fallback provider name
    std::unordered_map<std::string, std::shared_ptr<AIProvider>> m_providers; ///< AI providers
    std::unordered_map<std::string, std::shared_ptr<AIAgent>> m_agents;       ///< AI agents
    int m_updateTimerId;                                                 ///< Update timer ID
};

#endif // AI_MODULE_HPP