/**
 * @file npc_intelligence_agent.hpp
 * @brief NPC Intelligence agent for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the NPC Intelligence agent implementation that handles
 * enhanced NPC behavior, memory of player interactions, and MBTI personality types.
 */

#ifndef NPC_INTELLIGENCE_AGENT_HPP
#define NPC_INTELLIGENCE_AGENT_HPP

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <deque>

#include "../../common/cbasetypes.hpp"
#include "../common/ai_agent.hpp"
#include "../common/ai_request.hpp"
#include "../common/ai_response.hpp"

/**
 * @brief MBTI personality type
 * 
 * This struct represents an MBTI personality type with its traits.
 */
struct MBTIPersonality {
    std::string type;           ///< MBTI type (e.g., "INTJ", "ESFP")
    int extroversion;           ///< Extroversion score (0-100)
    int sensing;                ///< Sensing score (0-100)
    int thinking;               ///< Thinking score (0-100)
    int judging;                ///< Judging score (0-100)
    int moodBaseline;           ///< Mood baseline (-50 to 50)
    int moodVolatility;         ///< Mood volatility (0-100)
    std::vector<std::string> interests;    ///< Interests
    std::vector<std::string> dislikes;     ///< Dislikes
    std::string conversationStyle;         ///< Conversation style description
};

/**
 * @brief NPC memory
 * 
 * This struct represents an NPC's memory of an interaction with a player.
 */
struct NPCMemory {
    int id;                     ///< Memory ID
    int npcId;                  ///< NPC ID
    int charId;                 ///< Character ID
    std::string memoryType;     ///< Memory type
    std::string content;        ///< Memory content
    int emotionalImpact;        ///< Emotional impact (-10 to 10)
    int importance;             ///< Importance (1-10)
    bool isLongTerm;            ///< Whether the memory is long-term
    float decayRate;            ///< Decay rate (0.0-1.0)
    uint64 createdTime;         ///< Creation time
    uint64 lastRecalledTime;    ///< Last recalled time
    uint64 expiryTime;          ///< Expiry time (0 for never)
};

/**
 * @brief NPC Intelligence agent
 * 
 * This class implements the AI agent interface for NPC Intelligence.
 * It handles enhanced NPC behavior, memory of player interactions, and MBTI personality types.
 */
class NPCIntelligenceAgent : public AIAgent {
public:
    /**
     * @brief Constructor
     */
    NPCIntelligenceAgent();

    /**
     * @brief Destructor
     */
    virtual ~NPCIntelligenceAgent();

    /**
     * @brief Initialize the agent
     * @return True if initialization was successful, false otherwise
     */
    bool initialize() override;

    /**
     * @brief Finalize the agent
     */
    void finalize() override;

    /**
     * @brief Check if the agent is enabled
     * @return True if enabled, false otherwise
     */
    bool isEnabled() const override;

    /**
     * @brief Get the agent name
     * @return Agent name
     */
    std::string getName() const override;

    /**
     * @brief Get the agent description
     * @return Agent description
     */
    std::string getDescription() const override;

    /**
     * @brief Update the agent
     * @param tick Current tick value
     */
    void update(uint64 tick) override;

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
     * @brief Get the agent's status
     * @return JSON string describing the agent's status
     */
    std::string getStatus() const override;

    /**
     * @brief Get the agent's statistics
     * @return JSON string containing the agent's statistics
     */
    std::string getStatistics() const override;

    /**
     * @brief Handle a script command
     * @param command Command name
     * @param params Command parameters
     * @return Command result
     */
    std::string handleScriptCommand(const std::string& command, const std::vector<std::string>& params) override;

    /**
     * @brief Handle a player event
     * @param eventName Event name
     * @param charId Character ID
     * @param params Event parameters
     */
    void handlePlayerEvent(const std::string& eventName, int charId, const std::unordered_map<std::string, std::string>& params) override;

    /**
     * @brief Handle a world event
     * @param eventName Event name
     * @param params Event parameters
     */
    void handleWorldEvent(const std::string& eventName, const std::unordered_map<std::string, std::string>& params) override;

    /**
     * @brief Set NPC personality
     * @param npcId NPC ID
     * @param mbtiType MBTI type (e.g., "INTJ", "ESFP")
     * @return True if successful, false otherwise
     */
    bool setNPCPersonality(int npcId, const std::string& mbtiType);

    /**
     * @brief Get NPC personality
     * @param npcId NPC ID
     * @return NPC personality as JSON string
     */
    std::string getNPCPersonality(int npcId) const;

    /**
     * @brief Create a memory
     * @param npcId NPC ID
     * @param charId Character ID
     * @param memoryType Memory type
     * @param content Memory content
     * @param emotionalImpact Emotional impact (-10 to 10)
     * @param importance Importance (1-10)
     * @param isLongTerm Whether the memory is long-term
     * @return Memory ID
     */
    int createMemory(int npcId, int charId, const std::string& memoryType, const std::string& content,
                     int emotionalImpact = 0, int importance = 1, bool isLongTerm = false);

    /**
     * @brief Get NPC memories
     * @param npcId NPC ID
     * @param charId Character ID (0 for all characters)
     * @param limit Maximum number of memories to retrieve
     * @param onlyLongTerm Whether to retrieve only long-term memories
     * @return NPC memories as JSON string
     */
    std::string getNPCMemories(int npcId, int charId = 0, int limit = 10, bool onlyLongTerm = false) const;

    /**
     * @brief Recall memory
     * @param memoryId Memory ID
     * @return Memory content
     */
    std::string recallMemory(int memoryId);

    /**
     * @brief Forget memory
     * @param memoryId Memory ID
     * @return True if successful, false otherwise
     */
    bool forgetMemory(int memoryId);

    /**
     * @brief Convert short-term memory to long-term
     * @param memoryId Memory ID
     * @return True if successful, false otherwise
     */
    bool convertToLongTermMemory(int memoryId);

    /**
     * @brief Generate NPC dialogue
     * @param npcId NPC ID
     * @param charId Character ID
     * @param context Dialogue context
     * @param options Dialogue options
     * @return Generated dialogue as JSON string
     */
    std::string generateNPCDialogue(int npcId, int charId, const std::string& context, const std::vector<std::string>& options = {});

    /**
     * @brief Generate NPC reaction
     * @param npcId NPC ID
     * @param charId Character ID
     * @param action Player action
     * @param intensity Action intensity (1-10)
     * @return Generated reaction as JSON string
     */
    std::string generateNPCReaction(int npcId, int charId, const std::string& action, int intensity = 5);

    /**
     * @brief Update NPC relationship
     * @param npcId NPC ID
     * @param charId Character ID
     * @param delta Relationship delta (-10 to 10)
     * @return New relationship value
     */
    int updateNPCRelationship(int npcId, int charId, int delta);

    /**
     * @brief Get NPC relationship
     * @param npcId NPC ID
     * @param charId Character ID
     * @return Relationship value
     */
    int getNPCRelationship(int npcId, int charId) const;

    /**
     * @brief Generate random MBTI personality
     * @return Generated MBTI personality
     */
    MBTIPersonality generateRandomMBTIPersonality() const;

private:
    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Update memories
     */
    void updateMemories();

    /**
     * @brief Update relationships
     */
    void updateRelationships();

    /**
     * @brief Create MBTI personality
     * @param mbtiType MBTI type (e.g., "INTJ", "ESFP")
     * @return Created MBTI personality
     */
    MBTIPersonality createMBTIPersonality(const std::string& mbtiType) const;

    /**
     * @brief Get relevant memories
     * @param npcId NPC ID
     * @param charId Character ID
     * @param context Context for relevance
     * @param limit Maximum number of memories to retrieve
     * @return Relevant memories
     */
    std::vector<NPCMemory> getRelevantMemories(int npcId, int charId, const std::string& context, int limit = 5) const;

    /**
     * @brief Calculate memory decay
     * @param memory Memory to calculate decay for
     * @param currentTime Current time
     * @return Updated memory with decay applied
     */
    NPCMemory calculateMemoryDecay(const NPCMemory& memory, uint64 currentTime) const;

    /**
     * @brief Generate conversation prompt
     * @param npcId NPC ID
     * @param charId Character ID
     * @param context Dialogue context
     * @param memories Relevant memories
     * @return Generated prompt
     */
    std::string generateConversationPrompt(int npcId, int charId, const std::string& context, const std::vector<NPCMemory>& memories) const;

    bool m_initialized;                      ///< Initialization flag
    bool m_enabled;                          ///< Enabled flag
    int m_memoryRetention;                   ///< Memory retention in days
    int m_conversationDepth;                 ///< Conversation depth
    int m_personalityTypes;                  ///< Number of personality types
    float m_relationshipDevelopmentRate;     ///< Relationship development rate
    float m_npcLearningRate;                 ///< NPC learning rate
    uint64 m_lastUpdateTime;                 ///< Last update time
    std::unordered_map<int, MBTIPersonality> m_npcPersonalities; ///< NPC personalities (npc_id -> personality)
    std::unordered_map<int, std::deque<NPCMemory>> m_npcMemories; ///< NPC memories (npc_id -> memories)
    std::unordered_map<std::pair<int, int>, int, std::hash<std::pair<int, int>>> m_npcRelationships; ///< NPC relationships (npc_id, char_id) -> relationship_value
    int m_nextMemoryId;                      ///< Next memory ID
    mutable std::mutex m_mutex;              ///< Mutex for thread safety
};

#endif // NPC_INTELLIGENCE_AGENT_HPP