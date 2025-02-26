/**
 * @file ai_request.hpp
 * @brief AI request class for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the AI request class that represents a request to an AI provider.
 */

#ifndef AI_REQUEST_HPP
#define AI_REQUEST_HPP

#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief AI request class
 * 
 * This class represents a request to an AI provider.
 */
class AIRequest {
public:
    /**
     * @brief Constructor
     * @param type Request type
     * @param prompt Request prompt
     */
    AIRequest(const std::string& type, const std::string& prompt);

    /**
     * @brief Get the request type
     * @return Request type
     */
    const std::string& getType() const;

    /**
     * @brief Get the request prompt
     * @return Request prompt
     */
    const std::string& getPrompt() const;

    /**
     * @brief Set a parameter
     * @param key Parameter key
     * @param value Parameter value
     */
    void setParameter(const std::string& key, const std::string& value);

    /**
     * @brief Get a parameter
     * @param key Parameter key
     * @return Parameter value, or empty string if not found
     */
    std::string getParameter(const std::string& key) const;

    /**
     * @brief Check if a parameter exists
     * @param key Parameter key
     * @return True if the parameter exists, false otherwise
     */
    bool hasParameter(const std::string& key) const;

    /**
     * @brief Get all parameters
     * @return Map of all parameters
     */
    const std::unordered_map<std::string, std::string>& getParameters() const;

    /**
     * @brief Add a message to the conversation history
     * @param role Message role (system, user, assistant)
     * @param content Message content
     */
    void addMessage(const std::string& role, const std::string& content);

    /**
     * @brief Get the conversation history
     * @return Vector of message pairs (role, content)
     */
    const std::vector<std::pair<std::string, std::string>>& getMessages() const;

    /**
     * @brief Create a hash of the request for caching
     * @return Hash string
     */
    std::string createHash() const;

private:
    std::string m_type;                                      ///< Request type
    std::string m_prompt;                                    ///< Request prompt
    std::unordered_map<std::string, std::string> m_parameters; ///< Request parameters
    std::vector<std::pair<std::string, std::string>> m_messages; ///< Conversation history
};

#endif // AI_REQUEST_HPP