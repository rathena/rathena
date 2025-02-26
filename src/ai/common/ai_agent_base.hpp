/**
 * @file ai_agent_base.hpp
 * @brief Base class for AI agents with LangChain memory and dynamic configuration
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the base class for AI agents with LangChain memory and dynamic configuration.
 */

#ifndef AI_AGENT_BASE_HPP
#define AI_AGENT_BASE_HPP

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "ai_agent.hpp"
#include "ai_request.hpp"
#include "ai_response.hpp"
#include "../memory/langchain_memory.hpp"
#include "../config/dynamic_config.hpp"

/**
 * @brief Base class for AI agents with LangChain memory and dynamic configuration
 * 
 * This class provides a base implementation for AI agents with LangChain memory and dynamic configuration.
 */
class AIAgentBase : public AIAgent {
public:
    /**
     * @brief Constructor
     * @param name Agent name
     * @param description Agent description
     */
    AIAgentBase(const std::string& name, const std::string& description);

    /**
     * @brief Destructor
     */
    virtual ~AIAgentBase();

    /**
     * @brief Initialize the agent
     * @param config Configuration parameters
     * @return True if initialization was successful, false otherwise
     */
    virtual bool initialize(const std::unordered_map<std::string, std::string>& config) override;

    /**
     * @brief Finalize the agent
     */
    virtual void finalize() override;

    /**
     * @brief Process a request
     * @param request Request to process
     * @param provider Provider to use (optional)
     * @return Response
     */
    virtual AIResponse processRequest(const AIRequest& request, AIProvider* provider = nullptr) override;

    /**
     * @brief Get agent name
     * @return Agent name
     */
    virtual std::string getName() const override;

    /**
     * @brief Get agent description
     * @return Agent description
     */
    virtual std::string getDescription() const override;

    /**
     * @brief Get agent status
     * @return Agent status
     */
    virtual std::unordered_map<std::string, std::string> getStatus() const override;

    /**
     * @brief Get agent statistics
     * @return Agent statistics
     */
    virtual std::unordered_map<std::string, std::string> getStatistics() const override;

    /**
     * @brief Get agent configuration
     * @return Agent configuration
     */
    virtual std::unordered_map<std::string, std::string> getConfiguration() const override;

    /**
     * @brief Set agent configuration
     * @param config Configuration parameters
     * @return True if configuration was set successfully, false otherwise
     */
    virtual bool setConfiguration(const std::unordered_map<std::string, std::string>& config) override;

    /**
     * @brief Get agent memory
     * @return Agent memory
     */
    virtual std::shared_ptr<LangChainMemory> getMemory() const;

    /**
     * @brief Get agent dynamic configuration
     * @return Agent dynamic configuration
     */
    virtual std::shared_ptr<DynamicConfig> getDynamicConfig() const;

protected:
    /**
     * @brief Process a request with memory and dynamic configuration
     * @param request Request to process
     * @param provider Provider to use
     * @return Response
     */
    virtual AIResponse processRequestWithMemory(const AIRequest& request, AIProvider* provider);

    /**
     * @brief Build prompt for the request
     * @param request Request to build prompt for
     * @return Prompt
     */
    virtual std::string buildPrompt(const AIRequest& request);

    /**
     * @brief Parse response from the provider
     * @param response Response from the provider
     * @param request Original request
     * @return Parsed response
     */
    virtual AIResponse parseResponse(const std::string& response, const AIRequest& request);

    /**
     * @brief Add request to memory
     * @param request Request to add
     */
    virtual void addRequestToMemory(const AIRequest& request);

    /**
     * @brief Add response to memory
     * @param response Response to add
     * @param request Original request
     */
    virtual void addResponseToMemory(const AIResponse& response, const AIRequest& request);

    /**
     * @brief Retrieve relevant memories for the request
     * @param request Request to retrieve memories for
     * @return Relevant memories
     */
    virtual std::vector<MemoryEntry> retrieveRelevantMemories(const AIRequest& request);

    /**
     * @brief Format memories for prompt
     * @param memories Memories to format
     * @return Formatted memories
     */
    virtual std::string formatMemoriesForPrompt(const std::vector<MemoryEntry>& memories);

    /**
     * @brief Initialize dynamic configuration
     * @return True if initialization was successful, false otherwise
     */
    virtual bool initializeDynamicConfig();

    /**
     * @brief Initialize memory
     * @return True if initialization was successful, false otherwise
     */
    virtual bool initializeMemory();

    /**
     * @brief Register configuration change callbacks
     */
    virtual void registerConfigChangeCallbacks();

    /**
     * @brief Handle configuration change
     * @param key Configuration key
     * @param value New value
     */
    virtual void handleConfigChange(const std::string& key, const ConfigValue& value);

    std::string m_name;                                  ///< Agent name
    std::string m_description;                           ///< Agent description
    std::shared_ptr<LangChainMemory> m_memory;           ///< Agent memory
    std::shared_ptr<DynamicConfig> m_dynamicConfig;      ///< Agent dynamic configuration
    std::unordered_map<std::string, int> m_configCallbacks; ///< Configuration change callbacks
    bool m_initialized;                                  ///< Initialization flag
    std::unordered_map<std::string, std::string> m_status; ///< Agent status
    std::unordered_map<std::string, std::string> m_statistics; ///< Agent statistics
};

#endif // AI_AGENT_BASE_HPP