/**
 * @file legend_bloodlines_agent.hpp
 * @brief Legend Bloodlines agent for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the Legend Bloodlines agent implementation that handles
 * player legacy, bloodline abilities, and inheritance.
 */

#ifndef LEGEND_BLOODLINES_AGENT_HPP
#define LEGEND_BLOODLINES_AGENT_HPP

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "../../common/cbasetypes.hpp"
#include "../common/ai_agent.hpp"
#include "../common/ai_request.hpp"
#include "../common/ai_response.hpp"

/**
 * @brief Legend Bloodlines agent
 * 
 * This class implements the AI agent interface for Legend Bloodlines.
 * It handles player legacy, bloodline abilities, and inheritance.
 */
class LegendBloodlinesAgent : public AIAgent {
public:
    /**
     * @brief Constructor
     */
    LegendBloodlinesAgent();

    /**
     * @brief Destructor
     */
    virtual ~LegendBloodlinesAgent();

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
     * @brief Create a new bloodline
     * @param charId Character ID
     * @param legendName Legend name (optional)
     * @param ancestorId Ancestor character ID (optional)
     * @return Bloodline ID
     */
    int createBloodline(int charId, const std::string& legendName = "", int ancestorId = 0);

    /**
     * @brief Get bloodline information
     * @param charId Character ID
     * @return Bloodline information as JSON string
     */
    std::string getBloodlineInfo(int charId) const;

    /**
     * @brief Add a trait to a character
     * @param charId Character ID
     * @param traitName Trait name
     * @param traitLevel Trait level
     * @param traitType Trait type
     * @param description Trait description
     * @param isInherited Whether the trait is inherited
     * @param inheritanceStrength Inheritance strength (0.0-1.0)
     * @return Trait ID
     */
    int addTrait(int charId, const std::string& traitName, int traitLevel, const std::string& traitType,
                 const std::string& description, bool isInherited = false, float inheritanceStrength = 1.0);

    /**
     * @brief Get character traits
     * @param charId Character ID
     * @return Character traits as JSON string
     */
    std::string getCharacterTraits(int charId) const;

    /**
     * @brief Inherit traits from ancestor
     * @param charId Character ID
     * @param ancestorId Ancestor character ID
     * @return Number of traits inherited
     */
    int inheritTraits(int charId, int ancestorId);

    /**
     * @brief Generate a random trait mutation
     * @param charId Character ID
     * @param traitId Trait ID to mutate (0 for random)
     * @return Mutated trait ID
     */
    int generateTraitMutation(int charId, int traitId = 0);

    /**
     * @brief Apply bloodline effects
     * @param charId Character ID
     * @return True if effects were applied, false otherwise
     */
    bool applyBloodlineEffects(int charId);

    /**
     * @brief Get family tree
     * @param charId Character ID
     * @param depth Maximum depth (generations) to retrieve
     * @return Family tree as JSON string
     */
    std::string getFamilyTree(int charId, int depth = 5) const;

    /**
     * @brief Generate a bloodline challenge
     * @param charId Character ID
     * @param difficulty Challenge difficulty (1-5)
     * @return Challenge information as JSON string
     */
    std::string generateBloodlineChallenge(int charId, int difficulty = 3);

    /**
     * @brief Complete a bloodline challenge
     * @param charId Character ID
     * @param challengeId Challenge ID
     * @param success Whether the challenge was successful
     * @return Reward information as JSON string
     */
    std::string completeBloodlineChallenge(int charId, int challengeId, bool success);

private:
    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Update bloodlines
     */
    void updateBloodlines();

    /**
     * @brief Update traits
     */
    void updateTraits();

    /**
     * @brief Generate a trait
     * @param traitType Trait type
     * @param baseLevel Base level
     * @param isInherited Whether the trait is inherited
     * @return Generated trait as a pair (name, description)
     */
    std::pair<std::string, std::string> generateTrait(const std::string& traitType, int baseLevel, bool isInherited);

    /**
     * @brief Calculate trait inheritance
     * @param parentTraitLevel Parent trait level
     * @param inheritanceStrength Inheritance strength (0.0-1.0)
     * @param mutationProbability Mutation probability (0.0-1.0)
     * @return Inherited trait level
     */
    int calculateTraitInheritance(int parentTraitLevel, float inheritanceStrength, float mutationProbability);

    /**
     * @brief Apply trait effects
     * @param charId Character ID
     * @param traitId Trait ID
     * @param traitLevel Trait level
     * @param traitType Trait type
     */
    void applyTraitEffects(int charId, int traitId, int traitLevel, const std::string& traitType);

    bool m_initialized;                      ///< Initialization flag
    bool m_enabled;                          ///< Enabled flag
    float m_inheritanceProbability;          ///< Inheritance probability
    float m_mutationProbability;             ///< Mutation probability
    int m_maxBloodlineDepth;                 ///< Maximum bloodline depth
    float m_traitDecayRate;                  ///< Trait decay rate
    uint64 m_lastUpdateTime;                 ///< Last update time
    std::unordered_map<int, int> m_characterBloodlines; ///< Character bloodlines (char_id -> bloodline_id)
    std::unordered_map<int, std::vector<int>> m_bloodlineTraits; ///< Bloodline traits (bloodline_id -> [trait_id])
    std::unordered_map<int, std::vector<int>> m_activeChallenges; ///< Active challenges (char_id -> [challenge_id])
    mutable std::mutex m_mutex;              ///< Mutex for thread safety
};

#endif // LEGEND_BLOODLINES_AGENT_HPP