/**
 * @file ai_response.hpp
 * @brief AI response class for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the AI response class that represents a response from an AI provider.
 */

#ifndef AI_RESPONSE_HPP
#define AI_RESPONSE_HPP

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief AI response class
 * 
 * This class represents a response from an AI provider.
 */
class AIResponse {
public:
    /**
     * @brief Constructor
     * @param success Success flag
     * @param content Response content
     */
    AIResponse(bool success, const std::string& content);

    /**
     * @brief Check if the response was successful
     * @return True if successful, false otherwise
     */
    bool isSuccess() const;

    /**
     * @brief Get the response content
     * @return Response content
     */
    const std::string& getContent() const;

    /**
     * @brief Set a metadata value
     * @param key Metadata key
     * @param value Metadata value
     */
    void setMetadata(const std::string& key, const std::string& value);

    /**
     * @brief Get a metadata value
     * @param key Metadata key
     * @return Metadata value, or empty string if not found
     */
    std::string getMetadata(const std::string& key) const;

    /**
     * @brief Check if a metadata key exists
     * @param key Metadata key
     * @return True if the metadata key exists, false otherwise
     */
    bool hasMetadata(const std::string& key) const;

    /**
     * @brief Get all metadata
     * @return Map of all metadata
     */
    const std::unordered_map<std::string, std::string>& getMetadata() const;

    /**
     * @brief Set the error message
     * @param errorMessage Error message
     */
    void setErrorMessage(const std::string& errorMessage);

    /**
     * @brief Get the error message
     * @return Error message
     */
    const std::string& getErrorMessage() const;

    /**
     * @brief Create a successful response
     * @param content Response content
     * @return Successful response
     */
    static AIResponse createSuccess(const std::string& content);

    /**
     * @brief Create an error response
     * @param errorMessage Error message
     * @return Error response
     */
    static AIResponse createError(const std::string& errorMessage);

private:
    bool m_success;                                          ///< Success flag
    std::string m_content;                                   ///< Response content
    std::string m_errorMessage;                              ///< Error message
    std::unordered_map<std::string, std::string> m_metadata; ///< Response metadata
};

#endif // AI_RESPONSE_HPP