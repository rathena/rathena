/**
 * @file azure_openai_provider.hpp
 * @brief Azure OpenAI provider for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the Azure OpenAI provider implementation.
 */

#ifndef AZURE_OPENAI_PROVIDER_HPP
#define AZURE_OPENAI_PROVIDER_HPP

#include <string>
#include <memory>
#include <mutex>

#include "../common/ai_provider.hpp"
#include "../common/ai_request.hpp"
#include "../common/ai_response.hpp"

/**
 * @brief Azure OpenAI provider
 * 
 * This class implements the AI provider interface for Azure OpenAI.
 */
class AzureOpenAIProvider : public AIProvider {
public:
    /**
     * @brief Constructor
     */
    AzureOpenAIProvider();

    /**
     * @brief Destructor
     */
    virtual ~AzureOpenAIProvider();

    /**
     * @brief Initialize the provider
     * @return True if initialization was successful, false otherwise
     */
    bool initialize() override;

    /**
     * @brief Finalize the provider
     */
    void finalize() override;

    /**
     * @brief Check if the provider is enabled
     * @return True if enabled, false otherwise
     */
    bool isEnabled() const override;

    /**
     * @brief Get the provider name
     * @return Provider name
     */
    std::string getName() const override;

    /**
     * @brief Process an AI request
     * @param request The AI request to process
     * @return The AI response
     */
    AIResponse processRequest(const AIRequest& request) override;

    /**
     * @brief Reload configuration
     * @return True if reload was successful, false otherwise
     */
    bool reloadConfig() override;

    /**
     * @brief Get the provider's capabilities
     * @return JSON string describing the provider's capabilities
     */
    std::string getCapabilities() const override;

    /**
     * @brief Get the provider's status
     * @return JSON string describing the provider's status
     */
    std::string getStatus() const override;

private:
    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Make an API request to Azure OpenAI
     * @param endpoint API endpoint
     * @param requestBody Request body
     * @param apiKey API key
     * @return Response body
     */
    std::string makeApiRequest(const std::string& endpoint, const std::string& requestBody, const std::string& apiKey);

    /**
     * @brief Create a chat completion request
     * @param request AI request
     * @return Response from the API
     */
    AIResponse createChatCompletion(const AIRequest& request);

    /**
     * @brief Create an embeddings request
     * @param request AI request
     * @return Response from the API
     */
    AIResponse createEmbeddings(const AIRequest& request);

    bool m_initialized;                  ///< Initialization flag
    bool m_enabled;                      ///< Enabled flag
    std::string m_apiKey;                ///< API key
    std::string m_endpoint;              ///< API endpoint
    std::string m_apiVersion;            ///< API version
    std::string m_deploymentId;          ///< Deployment ID
    std::string m_model;                 ///< Model name
    float m_temperature;                 ///< Temperature
    int m_maxTokens;                     ///< Maximum tokens
    float m_topP;                        ///< Top-p sampling parameter
    float m_frequencyPenalty;            ///< Frequency penalty
    float m_presencePenalty;             ///< Presence penalty
    int m_requestTimeout;                ///< Request timeout in milliseconds
    mutable std::mutex m_mutex;          ///< Mutex for thread safety
};

#endif // AZURE_OPENAI_PROVIDER_HPP