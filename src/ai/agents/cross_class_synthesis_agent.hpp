/**
 * @file cross_class_synthesis_agent.hpp
 * @brief Cross-Class Synthesis agent for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the Cross-Class Synthesis agent implementation that handles
 * skill fusion, hybrid abilities, and class experimentation.
 */

#ifndef CROSS_CLASS_SYNTHESIS_AGENT_HPP
#define CROSS_CLASS_SYNTHESIS_AGENT_HPP

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "../../common/cbasetypes.hpp"
#include "../common/ai_agent.hpp"
#include "../common/ai_request.hpp"
#include "../common/ai_response.hpp"

/**
 * @brief Cross-Class Synthesis agent
 * 
 * This class implements the AI agent interface for Cross-Class Synthesis.
 * It handles skill fusion, hybrid abilities, and class experimentation.
 */
class CrossClassSynthesisAgent : public AIAgent {
public:
    /**
     * @brief Constructor
     */
    CrossClassSynthesisAgent();

    /**
     * @brief Destructor
     */
    virtual ~CrossClassSynthesisAgent();

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
     * @brief Create a skill fusion
     * @param charId Character ID
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @param fusionName Fusion name (optional, will be generated if empty)
     * @return Fusion ID
     */
    int createSkillFusion(int charId, int baseSkillId, const std::vector<int>& componentSkillIds, const std::string& fusionName = "");

    /**
     * @brief Get skill fusion information
     * @param fusionId Fusion ID
     * @return Fusion information as JSON string
     */
    std::string getSkillFusionInfo(int fusionId) const;

    /**
     * @brief Get character's skill fusions
     * @param charId Character ID
     * @return Character's skill fusions as JSON string
     */
    std::string getCharacterSkillFusions(int charId) const;

    /**
     * @brief Check skill compatibility
     * @param skillId1 First skill ID
     * @param skillId2 Second skill ID
     * @return Compatibility score (0.0-1.0)
     */
    float checkSkillCompatibility(int skillId1, int skillId2) const;

    /**
     * @brief Get compatible skills
     * @param skillId Skill ID
     * @param threshold Compatibility threshold (0.0-1.0)
     * @return Compatible skills as JSON string
     */
    std::string getCompatibleSkills(int skillId, float threshold = 0.6) const;

    /**
     * @brief Use skill fusion
     * @param charId Character ID
     * @param fusionId Fusion ID
     * @param targetId Target ID (0 for self)
     * @param x X coordinate (optional)
     * @param y Y coordinate (optional)
     * @return Result of skill fusion use as JSON string
     */
    std::string useSkillFusion(int charId, int fusionId, int targetId = 0, int x = 0, int y = 0);

    /**
     * @brief Generate experimental skill
     * @param charId Character ID
     * @param jobId Job ID
     * @param skillLevel Skill level (1-10)
     * @return Experimental skill information as JSON string
     */
    std::string generateExperimentalSkill(int charId, int jobId, int skillLevel = 1);

    /**
     * @brief Stabilize experimental skill
     * @param charId Character ID
     * @param fusionId Fusion ID
     * @return Stabilized skill information as JSON string
     */
    std::string stabilizeExperimentalSkill(int charId, int fusionId);

    /**
     * @brief Get skill fusion cooldown
     * @param charId Character ID
     * @param fusionId Fusion ID
     * @return Cooldown time in seconds
     */
    int getSkillFusionCooldown(int charId, int fusionId) const;

    /**
     * @brief Reset skill fusion cooldown
     * @param charId Character ID
     * @param fusionId Fusion ID
     * @return True if reset was successful, false otherwise
     */
    bool resetSkillFusionCooldown(int charId, int fusionId);

private:
    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Update skill fusions
     */
    void updateSkillFusions();

    /**
     * @brief Generate fusion name
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @return Generated fusion name
     */
    std::string generateFusionName(int baseSkillId, const std::vector<int>& componentSkillIds) const;

    /**
     * @brief Calculate fusion stats
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @param isExperimental Whether the fusion is experimental
     * @return Fusion stats as a map
     */
    std::unordered_map<std::string, int> calculateFusionStats(int baseSkillId, const std::vector<int>& componentSkillIds, bool isExperimental = false) const;

    /**
     * @brief Generate damage formula
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @param isExperimental Whether the fusion is experimental
     * @return Generated damage formula
     */
    std::string generateDamageFormula(int baseSkillId, const std::vector<int>& componentSkillIds, bool isExperimental = false) const;

    /**
     * @brief Generate effect description
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @param isExperimental Whether the fusion is experimental
     * @return Generated effect description
     */
    std::string generateEffectDescription(int baseSkillId, const std::vector<int>& componentSkillIds, bool isExperimental = false) const;

    /**
     * @brief Apply fusion effects
     * @param charId Character ID
     * @param fusionId Fusion ID
     * @param targetId Target ID
     * @param x X coordinate
     * @param y Y coordinate
     * @return Effect result
     */
    std::string applyFusionEffects(int charId, int fusionId, int targetId, int x, int y);

    /**
     * @brief Calculate stability factor
     * @param baseSkillId Base skill ID
     * @param componentSkillIds Component skill IDs
     * @param charLevel Character level
     * @return Stability factor (0.0-1.0)
     */
    float calculateStabilityFactor(int baseSkillId, const std::vector<int>& componentSkillIds, int charLevel) const;

    bool m_initialized;                      ///< Initialization flag
    bool m_enabled;                          ///< Enabled flag
    int m_maxFusionSkills;                   ///< Maximum fusion skills per character
    float m_skillCompatibilityThreshold;     ///< Skill compatibility threshold
    int m_fusionCooldown;                    ///< Fusion cooldown in seconds
    bool m_experimentalSkillsEnabled;        ///< Experimental skills enabled flag
    uint64 m_lastUpdateTime;                 ///< Last update time
    std::unordered_map<int, std::vector<int>> m_characterFusions; ///< Character fusions (char_id -> [fusion_id])
    std::unordered_map<int, uint64> m_fusionCooldowns; ///< Fusion cooldowns (fusion_id -> expiry_time)
    std::unordered_map<int, float> m_fusionStability; ///< Fusion stability (fusion_id -> stability_factor)
    std::unordered_set<int> m_experimentalFusions; ///< Experimental fusions
    mutable std::mutex m_mutex;              ///< Mutex for thread safety
};

#endif // CROSS_CLASS_SYNTHESIS_AGENT_HPP