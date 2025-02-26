/**
 * @file langchain_memory.hpp
 * @brief LangChain memory integration for AI agents
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the LangChain memory integration for AI agents.
 */

#ifndef LANGCHAIN_MEMORY_HPP
#define LANGCHAIN_MEMORY_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "../../common/cbasetypes.hpp"

/**
 * @brief Memory entry structure
 * 
 * This structure represents a memory entry in the LangChain memory system.
 */
struct MemoryEntry {
    std::string id;                  ///< Memory ID
    std::string content;             ///< Memory content
    std::string type;                ///< Memory type
    float relevance;                 ///< Relevance score
    uint64 timestamp;                ///< Creation timestamp
    uint64 last_accessed;            ///< Last accessed timestamp
    int importance;                  ///< Importance score
    bool is_long_term;               ///< Whether the memory is long-term
};

/**
 * @brief Memory retrieval result
 * 
 * This structure represents the result of a memory retrieval operation.
 */
struct MemoryRetrievalResult {
    std::vector<MemoryEntry> entries;    ///< Retrieved memory entries
    float total_relevance;               ///< Total relevance score
    int count;                           ///< Number of entries
};

/**
 * @brief LangChain memory interface
 * 
 * This class provides an interface for LangChain memory integration.
 */
class LangChainMemory {
public:
    /**
     * @brief Constructor
     */
    LangChainMemory();

    /**
     * @brief Destructor
     */
    virtual ~LangChainMemory();

    /**
     * @brief Initialize the memory system
     * @param config Configuration parameters
     * @return True if initialization was successful, false otherwise
     */
    bool initialize(const std::unordered_map<std::string, std::string>& config);

    /**
     * @brief Finalize the memory system
     */
    void finalize();

    /**
     * @brief Add a memory entry
     * @param content Memory content
     * @param type Memory type
     * @param importance Importance score
     * @param is_long_term Whether the memory is long-term
     * @return Memory ID
     */
    std::string addMemory(const std::string& content, const std::string& type, int importance = 1, bool is_long_term = false);

    /**
     * @brief Retrieve memory entries
     * @param query Query string
     * @param type Memory type (empty for all types)
     * @param limit Maximum number of entries to retrieve
     * @param min_relevance Minimum relevance score
     * @return Memory retrieval result
     */
    MemoryRetrievalResult retrieveMemory(const std::string& query, const std::string& type = "", int limit = 5, float min_relevance = 0.5);

    /**
     * @brief Update a memory entry
     * @param id Memory ID
     * @param content New memory content
     * @param importance New importance score
     * @param is_long_term New long-term status
     * @return True if update was successful, false otherwise
     */
    bool updateMemory(const std::string& id, const std::string& content, int importance = -1, bool is_long_term = false);

    /**
     * @brief Delete a memory entry
     * @param id Memory ID
     * @return True if deletion was successful, false otherwise
     */
    bool deleteMemory(const std::string& id);

    /**
     * @brief Clear all memory entries
     * @param type Memory type (empty for all types)
     * @return Number of entries cleared
     */
    int clearMemory(const std::string& type = "");

    /**
     * @brief Get memory statistics
     * @return Statistics as JSON string
     */
    std::string getStatistics() const;

    /**
     * @brief Set vector store URL
     * @param url Vector store URL
     */
    void setVectorStoreUrl(const std::string& url);

    /**
     * @brief Set embedding model
     * @param model Embedding model name
     */
    void setEmbeddingModel(const std::string& model);

    /**
     * @brief Set memory decay rate
     * @param rate Memory decay rate
     */
    void setMemoryDecayRate(float rate);

    /**
     * @brief Set long-term memory threshold
     * @param threshold Long-term memory threshold
     */
    void setLongTermMemoryThreshold(int threshold);

private:
    /**
     * @brief Generate embeddings for a text
     * @param text Text to generate embeddings for
     * @return Embeddings as a vector of floats
     */
    std::vector<float> generateEmbeddings(const std::string& text);

    /**
     * @brief Calculate relevance score
     * @param query_embedding Query embedding
     * @param memory_embedding Memory embedding
     * @return Relevance score
     */
    float calculateRelevance(const std::vector<float>& query_embedding, const std::vector<float>& memory_embedding);

    /**
     * @brief Apply memory decay
     * @param entry Memory entry
     * @param current_time Current time
     * @return Updated memory entry
     */
    MemoryEntry applyMemoryDecay(const MemoryEntry& entry, uint64 current_time);

    /**
     * @brief Convert memory entry to JSON
     * @param entry Memory entry
     * @return JSON string
     */
    std::string memoryEntryToJson(const MemoryEntry& entry) const;

    /**
     * @brief Parse memory entry from JSON
     * @param json JSON string
     * @return Memory entry
     */
    MemoryEntry memoryEntryFromJson(const std::string& json) const;

    std::string m_vectorStoreUrl;                ///< Vector store URL
    std::string m_embeddingModel;                ///< Embedding model name
    float m_memoryDecayRate;                     ///< Memory decay rate
    int m_longTermMemoryThreshold;               ///< Long-term memory threshold
    std::unordered_map<std::string, std::vector<float>> m_embeddings;  ///< Cached embeddings
    std::unordered_map<std::string, MemoryEntry> m_memories;           ///< Memory entries
    mutable std::mutex m_mutex;                  ///< Mutex for thread safety
    bool m_initialized;                          ///< Initialization flag
};

#endif // LANGCHAIN_MEMORY_HPP