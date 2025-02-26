/**
 * @file local_provider.hpp
 * @brief Local provider for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the Local provider implementation that uses
 * locally installed models for inference when cloud services are unavailable.
 */

#ifndef LOCAL_PROVIDER_HPP
#define LOCAL_PROVIDER_HPP

#include <string>
#include <memory>
#include <mutex>

#include "../common/ai_provider.hpp"
#include "../common/ai_request.hpp"
#include "../common/ai_response.hpp"

// Forward declaration for the model implementation
class LocalModel;

/**
 * @brief Local provider
 * 
 * This class implements the AI provider interface for local models.
 * It serves as a fallback when cloud providers are unavailable.
 */
class LocalProvider : public AIProvider {
public:
    /**
     * @brief Constructor
     */
    LocalProvider();

    /**
     * @brief Destructor
     */
    virtual ~LocalProvider();

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
     * @brief Load the model
     * @return True if loading was successful, false otherwise
     */
    bool loadModel();

    /**
     * @brief Unload the model
     */
    void unloadModel();

    /**
     * @brief Generate a response using the local model
     * @param request AI request
     * @return Generated response
     */
    AIResponse generateResponse(const AIRequest& request);

    bool m_initialized;                  ///< Initialization flag
    bool m_enabled;                      ///< Enabled flag
    std::string m_modelPath;             ///< Path to model files
    std::string m_modelType;             ///< Type of model (llama, mistral, etc.)
    int m_contextSize;                   ///< Context size in tokens
    float m_temperature;                 ///< Temperature
    int m_maxTokens;                     ///< Maximum tokens
    int m_threads;                       ///< Number of threads for inference
    std::unique_ptr<LocalModel> m_model; ///< Model implementation
    mutable std::mutex m_mutex;          ///< Mutex for thread safety
};

#endif // LOCAL_PROVIDER_HPP