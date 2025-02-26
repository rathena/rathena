/**
 * @file ai_provider.hpp
 * @brief AI provider interface for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the AI provider interface that defines the contract
 * for AI providers (Azure OpenAI, OpenAI, DeepSeek, etc.).
 */

#ifndef AI_PROVIDER_HPP
#define AI_PROVIDER_HPP

#include <string>
#include <memory>

#include "ai_request.hpp"
#include "ai_response.hpp"

/**
 * @brief AI provider interface
 * 
 * This class defines the interface for AI providers.
 */
class AIProvider {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~AIProvider() = default;

    /**
     * @brief Initialize the provider
     * @return True if initialization was successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * @brief Finalize the provider
     */
    virtual void finalize() = 0;

    /**
     * @brief Check if the provider is enabled
     * @return True if enabled, false otherwise
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Get the provider name
     * @return Provider name
     */
    virtual std::string getName() const = 0;

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
     * @brief Get the provider's capabilities
     * @return JSON string describing the provider's capabilities
     */
    virtual std::string getCapabilities() const = 0;

    /**
     * @brief Get the provider's status
     * @return JSON string describing the provider's status
     */
    virtual std::string getStatus() const = 0;
};

#endif // AI_PROVIDER_HPP