/**
 * @file ai_legends_agent.hpp
 * @brief AI Legends agent for managing AI-controlled legend characters
 * @author rAthena Development Team
 * @date 2025-02-27
 *
 * This file contains the AI Legends agent, which manages AI-controlled legend characters
 * that roam the world and interact with players.
 */

#ifndef AI_LEGENDS_AGENT_HPP
#define AI_LEGENDS_AGENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "../common/ai_agent_base.hpp"
#include "../memory/langchain_memory.hpp"
#include "../config/dynamic_config.hpp"
#include "../../map/pc.hpp"
#include "../../map/mob.hpp"
#include "../../map/party.hpp"

/**
 * @brief AI Legend character structure
 * 
 * This structure represents an AI-controlled legend character.
 */
struct AILegendCharacter {
    int legend_id;                                  ///< Legend ID
    std::string name;                               ///< Character name
    int class_id;                                   ///< Class ID
    int base_level;                                 ///< Base level
    int job_level;                                  ///< Job level
    std::string mbti;                               ///< MBTI personality type
    bool enabled;                                   ///< Whether the legend is enabled
    std::string last_map;                           ///< Last map the legend was on
    int last_x;                                     ///< Last X coordinate
    int last_y;                                     ///< Last Y coordinate
    int current_party_id;                           ///< Current party ID
    int current_mvp_id;                             ///< Current MVP ID
    int pvp_challenges_today;                       ///< Number of PvP challenges today
    std::string last_pvp_reset;                     ///< Last PvP reset date
    std::vector<std::string> appearance_maps;       ///< Maps where the legend can appear
    std::vector<std::string> preferred_mvps;        ///< Preferred MVP monsters
    int backstory_pieces;                           ///< Number of backstory pieces
    int secret_skill_id;                            ///< ID of the secret skill
    std::string secret_skill_name;                  ///< Name of the secret skill
    std::string secret_skill_condition;             ///< Condition to learn the skill
    std::string custom_script;                      ///< Custom script for the legend
    std::shared_ptr<LangChainMemory> memory;        ///< Memory for the legend
    std::shared_ptr<DynamicConfig> config;          ///< Configuration for the legend
    time_t last_appearance;                         ///< Last appearance time
    time_t last_movement;                           ///< Last movement time
    time_t last_interaction;                        ///< Last interaction time
    time_t next_scheduled_appearance;               ///< Next scheduled appearance time
    time_t next_scheduled_movement;                 ///< Next scheduled movement time
    std::unordered_map<int, time_t> interaction_cooldowns; ///< Interaction cooldowns for players
    std::unordered_map<int, int> player_relationships; ///< Relationships with players
};

/**
 * @brief AI Legends agent
 * 
 * This class manages AI-controlled legend characters that roam the world and interact with players.
 */
class AILegendsAgent : public AIAgentBase {
public:
    /**
     * @brief Constructor
     */
    AILegendsAgent();

    /**
     * @brief Destructor
     */
    virtual ~AILegendsAgent();

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
     * @param provider Provider to use
     * @return Response
     */
    virtual AIResponse processRequest(const AIRequest& request, AIProvider* provider = nullptr) override;

    /**
     * @brief Update the agent
     * @param tick Current tick
     */
    void update(uint64 tick);

    /**
     * @brief Handle player login
     * @param sd Player data
     */
    void handlePlayerLogin(struct map_session_data* sd);

    /**
     * @brief Handle player logout
     * @param sd Player data
     */
    void handlePlayerLogout(struct map_session_data* sd);

    /**
     * @brief Handle player movement
     * @param sd Player data
     * @param x X coordinate
     * @param y Y coordinate
     */
    void handlePlayerMovement(struct map_session_data* sd, int x, int y);

    /**
     * @brief Handle player chat
     * @param sd Player data
     * @param message Chat message
     */
    void handlePlayerChat(struct map_session_data* sd, const std::string& message);

    /**
     * @brief Handle player death
     * @param sd Player data
     * @param src Source of death
     */
    void handlePlayerDeath(struct map_session_data* sd, struct block_list* src);

    /**
     * @brief Handle player kill
     * @param sd Player data
     * @param target Target killed
     */
    void handlePlayerKill(struct map_session_data* sd, struct block_list* target);

    /**
     * @brief Handle MVP spawn
     * @param md MVP monster data
     */
    void handleMVPSpawn(struct mob_data* md);

    /**
     * @brief Handle MVP death
     * @param md MVP monster data
     * @param mvp_sd MVP player data
     */
    void handleMVPDeath(struct mob_data* md, struct map_session_data* mvp_sd);

    /**
     * @brief Handle party creation
     * @param party_id Party ID
     * @param sd Party leader data
     */
    void handlePartyCreation(int party_id, struct map_session_data* sd);

    /**
     * @brief Handle party join
     * @param party_id Party ID
     * @param sd Player data
     */
    void handlePartyJoin(int party_id, struct map_session_data* sd);

    /**
     * @brief Handle party leave
     * @param party_id Party ID
     * @param sd Player data
     */
    void handlePartyLeave(int party_id, struct map_session_data* sd);

    /**
     * @brief Handle party disband
     * @param party_id Party ID
     */
    void handlePartyDisband(int party_id);

    /**
     * @brief Handle PvP challenge
     * @param sd_challenger Challenger player data
     * @param sd_target Target player data
     */
    void handlePvPChallenge(struct map_session_data* sd_challenger, struct map_session_data* sd_target);

    /**
     * @brief Handle PvP result
     * @param sd_winner Winner player data
     * @param sd_loser Loser player data
     */
    void handlePvPResult(struct map_session_data* sd_winner, struct map_session_data* sd_loser);

    /**
     * @brief Get legend by ID
     * @param legend_id Legend ID
     * @return Legend character
     */
    AILegendCharacter* getLegendById(int legend_id);

    /**
     * @brief Get legend by name
     * @param name Legend name
     * @return Legend character
     */
    AILegendCharacter* getLegendByName(const std::string& name);

    /**
     * @brief Get legend by class ID
     * @param class_id Class ID
     * @return Legend character
     */
    AILegendCharacter* getLegendByClassId(int class_id);

    /**
     * @brief Get all legends
     * @return All legend characters
     */
    std::vector<AILegendCharacter*> getAllLegends();

    /**
     * @brief Get enabled legends
     * @return Enabled legend characters
     */
    std::vector<AILegendCharacter*> getEnabledLegends();

    /**
     * @brief Get legends by map
     * @param map_name Map name
     * @return Legend characters on the map
     */
    std::vector<AILegendCharacter*> getLegendsByMap(const std::string& map_name);

    /**
     * @brief Get legends by preferred MVP
     * @param mvp_id MVP ID
     * @return Legend characters that prefer the MVP
     */
    std::vector<AILegendCharacter*> getLegendsByPreferredMVP(int mvp_id);

    /**
     * @brief Get legends by MBTI
     * @param mbti MBTI personality type
     * @return Legend characters with the MBTI type
     */
    std::vector<AILegendCharacter*> getLegendsByMBTI(const std::string& mbti);

    /**
     * @brief Spawn legend
     * @param legend_id Legend ID
     * @param map_name Map name
     * @param x X coordinate
     * @param y Y coordinate
     * @return True if spawn was successful, false otherwise
     */
    bool spawnLegend(int legend_id, const std::string& map_name, int x, int y);

    /**
     * @brief Despawn legend
     * @param legend_id Legend ID
     * @return True if despawn was successful, false otherwise
     */
    bool despawnLegend(int legend_id);

    /**
     * @brief Move legend
     * @param legend_id Legend ID
     * @param map_name Map name
     * @param x X coordinate
     * @param y Y coordinate
     * @return True if movement was successful, false otherwise
     */
    bool moveLegend(int legend_id, const std::string& map_name, int x, int y);

    /**
     * @brief Make legend join party
     * @param legend_id Legend ID
     * @param party_id Party ID
     * @return True if join was successful, false otherwise
     */
    bool legendJoinParty(int legend_id, int party_id);

    /**
     * @brief Make legend leave party
     * @param legend_id Legend ID
     * @return True if leave was successful, false otherwise
     */
    bool legendLeaveParty(int legend_id);

    /**
     * @brief Make legend challenge player to PvP
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if challenge was successful, false otherwise
     */
    bool legendChallengePvP(int legend_id, int char_id);

    /**
     * @brief Make legend give gift to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param item_id Item ID
     * @param amount Amount
     * @param refine Refine level
     * @param attribute Attribute
     * @param card0 Card 0
     * @param card1 Card 1
     * @param card2 Card 2
     * @param card3 Card 3
     * @return True if gift was successful, false otherwise
     */
    bool legendGiveGift(int legend_id, int char_id, int item_id, int amount = 1, int refine = 0, int attribute = 0, int card0 = 0, int card1 = 0, int card2 = 0, int card3 = 0);

    /**
     * @brief Make legend reveal backstory piece to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param piece_number Piece number
     * @return True if reveal was successful, false otherwise
     */
    bool legendRevealBackstoryPiece(int legend_id, int char_id, int piece_number);

    /**
     * @brief Make legend teach secret skill to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if teaching was successful, false otherwise
     */
    bool legendTeachSecretSkill(int legend_id, int char_id);

    /**
     * @brief Make legend start secret skill quest for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if quest start was successful, false otherwise
     */
    bool legendStartSecretSkillQuest(int legend_id, int char_id);

    /**
     * @brief Make legend complete secret skill quest for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if quest completion was successful, false otherwise
     */
    bool legendCompleteSecretSkillQuest(int legend_id, int char_id);

    /**
     * @brief Make legend interact with player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param interaction_type Interaction type
     * @param interaction_data Interaction data
     * @return True if interaction was successful, false otherwise
     */
    bool legendInteractWithPlayer(int legend_id, int char_id, const std::string& interaction_type, const std::string& interaction_data);

    /**
     * @brief Make legend chat with player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param message Chat message
     * @return Response message
     */
    std::string legendChatWithPlayer(int legend_id, int char_id, const std::string& message);

    /**
     * @brief Check if legend can appear at MVP
     * @param legend_id Legend ID
     * @param mvp_id MVP ID
     * @return True if legend can appear, false otherwise
     */
    bool canLegendAppearAtMVP(int legend_id, int mvp_id);

    /**
     * @brief Check if legend can challenge player to PvP
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if legend can challenge, false otherwise
     */
    bool canLegendChallengePvP(int legend_id, int char_id);

    /**
     * @brief Check if legend can join party
     * @param legend_id Legend ID
     * @param party_id Party ID
     * @return True if legend can join, false otherwise
     */
    bool canLegendJoinParty(int legend_id, int party_id);

    /**
     * @brief Check if legend can give gift to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if legend can give gift, false otherwise
     */
    bool canLegendGiveGift(int legend_id, int char_id);

    /**
     * @brief Check if legend can reveal backstory piece to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param piece_number Piece number
     * @return True if legend can reveal backstory piece, false otherwise
     */
    bool canLegendRevealBackstoryPiece(int legend_id, int char_id, int piece_number);

    /**
     * @brief Check if legend can teach secret skill to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return True if legend can teach secret skill, false otherwise
     */
    bool canLegendTeachSecretSkill(int legend_id, int char_id);

    /**
     * @brief Get relationship between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Relationship value
     */
    int getLegendPlayerRelationship(int legend_id, int char_id);

    /**
     * @brief Set relationship between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param value Relationship value
     * @return True if set was successful, false otherwise
     */
    bool setLegendPlayerRelationship(int legend_id, int char_id, int value);

    /**
     * @brief Change relationship between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param delta Change in relationship value
     * @return True if change was successful, false otherwise
     */
    bool changeLegendPlayerRelationship(int legend_id, int char_id, int delta);

    /**
     * @brief Get backstory pieces revealed to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Backstory pieces revealed
     */
    std::vector<int> getBackstoryPiecesRevealed(int legend_id, int char_id);

    /**
     * @brief Get secret skill quest status for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Quest status (0 = not started, 1 = in progress, 2 = completed)
     */
    int getSecretSkillQuestStatus(int legend_id, int char_id);

    /**
     * @brief Get secret skill quest progress for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Quest progress
     */
    int getSecretSkillQuestProgress(int legend_id, int char_id);

    /**
     * @brief Set secret skill quest progress for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param progress Quest progress
     * @return True if set was successful, false otherwise
     */
    bool setSecretSkillQuestProgress(int legend_id, int char_id, int progress);

    /**
     * @brief Change secret skill quest progress for player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param delta Change in quest progress
     * @return True if change was successful, false otherwise
     */
    bool changeSecretSkillQuestProgress(int legend_id, int char_id, int delta);

    /**
     * @brief Get conversation history between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param limit Maximum number of messages to retrieve
     * @return Conversation history
     */
    std::vector<std::pair<bool, std::string>> getConversationHistory(int legend_id, int char_id, int limit = 10);

    /**
     * @brief Add conversation message between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param is_player Whether the message is from the player
     * @param message Message content
     * @return True if add was successful, false otherwise
     */
    bool addConversationMessage(int legend_id, int char_id, bool is_player, const std::string& message);

    /**
     * @brief Get PvP history between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return PvP history
     */
    std::vector<std::pair<std::string, std::string>> getPvPHistory(int legend_id, int char_id);

    /**
     * @brief Add PvP history between legend and player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param winner Winner (LEGEND, PLAYER, DRAW)
     * @param map_name Map name
     * @param x X coordinate
     * @param y Y coordinate
     * @return True if add was successful, false otherwise
     */
    bool addPvPHistory(int legend_id, int char_id, const std::string& winner, const std::string& map_name, int x, int y);

    /**
     * @brief Get gifts given by legend to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Gifts given
     */
    std::vector<std::tuple<int, int, std::string>> getGiftsGiven(int legend_id, int char_id);

    /**
     * @brief Add gift given by legend to player
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param item_id Item ID
     * @param amount Amount
     * @param reason Reason for gift
     * @return True if add was successful, false otherwise
     */
    bool addGiftGiven(int legend_id, int char_id, int item_id, int amount, const std::string& reason);

    /**
     * @brief Get strongest player on server
     * @return Strongest player data
     */
    struct map_session_data* getStrongestPlayer();

    /**
     * @brief Get weakest party at MVP
     * @param mvp_id MVP ID
     * @return Weakest party ID
     */
    int getWeakestPartyAtMVP(int mvp_id);

    /**
     * @brief Get players of same class as legend
     * @param legend_id Legend ID
     * @return Players of same class
     */
    std::vector<struct map_session_data*> getPlayersOfSameClass(int legend_id);

    /**
     * @brief Get players on map
     * @param map_name Map name
     * @return Players on map
     */
    std::vector<struct map_session_data*> getPlayersOnMap(const std::string& map_name);

    /**
     * @brief Get players in party
     * @param party_id Party ID
     * @return Players in party
     */
    std::vector<struct map_session_data*> getPlayersInParty(int party_id);

    /**
     * @brief Get players near coordinates
     * @param map_name Map name
     * @param x X coordinate
     * @param y Y coordinate
     * @param range Range
     * @return Players near coordinates
     */
    std::vector<struct map_session_data*> getPlayersNearCoordinates(const std::string& map_name, int x, int y, int range);

    /**
     * @brief Get random map for legend
     * @param legend_id Legend ID
     * @return Random map name
     */
    std::string getRandomMapForLegend(int legend_id);

    /**
     * @brief Get random coordinates on map
     * @param map_name Map name
     * @return Random coordinates (x, y)
     */
    std::pair<int, int> getRandomCoordinatesOnMap(const std::string& map_name);

    /**
     * @brief Get random item for gift
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @return Random item ID
     */
    int getRandomItemForGift(int legend_id, int char_id);

    /**
     * @brief Generate prompt for legend
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param context Context for prompt
     * @return Generated prompt
     */
    std::string generatePromptForLegend(int legend_id, int char_id, const std::string& context);

    /**
     * @brief Parse response from legend
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param response Response from legend
     * @param context Context for response
     * @return Parsed response
     */
    std::string parseResponseFromLegend(int legend_id, int char_id, const std::string& response, const std::string& context);

    /**
     * @brief Log legend activity
     * @param legend_id Legend ID
     * @param char_id Character ID
     * @param log_type Log type
     * @param message Log message
     * @return True if log was successful, false otherwise
     */
    bool logLegendActivity(int legend_id, int char_id, const std::string& log_type, const std::string& message);

    /**
     * @brief Update legend statistics
     * @param legend_id Legend ID
     * @param stat_type Statistic type
     * @param value Value to add
     * @return True if update was successful, false otherwise
     */
    bool updateLegendStatistics(int legend_id, const std::string& stat_type, int value);

private:
    /**
     * @brief Load legends from database
     * @return True if load was successful, false otherwise
     */
    bool loadLegendsFromDatabase();

    /**
     * @brief Save legends to database
     * @return True if save was successful, false otherwise
     */
    bool saveLegendsToDatabase();

    /**
     * @brief Load legend equipment from database
     * @param legend_id Legend ID
     * @return True if load was successful, false otherwise
     */
    bool loadLegendEquipmentFromDatabase(int legend_id);

    /**
     * @brief Load legend skills from database
     * @param legend_id Legend ID
     * @return True if load was successful, false otherwise
     */
    bool loadLegendSkillsFromDatabase(int legend_id);

    /**
     * @brief Load legend backstory from database
     * @param legend_id Legend ID
     * @return True if load was successful, false otherwise
     */
    bool loadLegendBackstoryFromDatabase(int legend_id);

    /**
     * @brief Load legend secret skills from database
     * @param legend_id Legend ID
     * @return True if load was successful, false otherwise
     */
    bool loadLegendSecretSkillsFromDatabase(int legend_id);

    /**
     * @brief Load legend statistics from database
     * @param legend_id Legend ID
     * @return True if load was successful, false otherwise
     */
    bool loadLegendStatisticsFromDatabase(int legend_id);

    /**
     * @brief Initialize legend memory
     * @param legend_id Legend ID
     * @return True if initialization was successful, false otherwise
     */
    bool initializeLegendMemory(int legend_id);

    /**
     * @brief Initialize legend configuration
     * @param legend_id Legend ID
     * @return True if initialization was successful, false otherwise
     */
    bool initializeLegendConfiguration(int legend_id);

    /**
     * @brief Schedule legend appearances
     * @return True if scheduling was successful, false otherwise
     */
    bool scheduleLegendAppearances();

    /**
     * @brief Schedule legend movements
     * @return True if scheduling was successful, false otherwise
     */
    bool scheduleLegendMovements();

    /**
     * @brief Reset daily PvP challenges
     * @return True if reset was successful, false otherwise
     */
    bool resetDailyPvPChallenges();

    /**
     * @brief Check for scheduled appearances
     * @param tick Current tick
     */
    void checkScheduledAppearances(uint64 tick);

    /**
     * @brief Check for scheduled movements
     * @param tick Current tick
     */
    void checkScheduledMovements(uint64 tick);

    /**
     * @brief Check for MVP assistance
     * @param tick Current tick
     */
    void checkMVPAssistance(uint64 tick);

    /**
     * @brief Check for PvP challenges
     * @param tick Current tick
     */
    void checkPvPChallenges(uint64 tick);

    /**
     * @brief Check for party assistance
     * @param tick Current tick
     */
    void checkPartyAssistance(uint64 tick);

    /**
     * @brief Check for player interactions
     * @param tick Current tick
     */
    void checkPlayerInteractions(uint64 tick);

    /**
     * @brief Check for gift giving
     * @param tick Current tick
     */
    void checkGiftGiving(uint64 tick);

    /**
     * @brief Check for backstory revelation
     * @param tick Current tick
     */
    void checkBackstoryRevelation(uint64 tick);

    /**
     * @brief Check for secret skill teaching
     * @param tick Current tick
     */
    void checkSecretSkillTeaching(uint64 tick);

    /**
     * @brief Check for legend disappearances
     * @param tick Current tick
     */
    void checkLegendDisappearances(uint64 tick);

    std::unordered_map<int, AILegendCharacter> m_legends; ///< Legend characters by ID
    std::unordered_map<std::string, int> m_legendsByName; ///< Legend IDs by name
    std::unordered_map<int, int> m_legendsByClassId;      ///< Legend IDs by class ID
    std::unordered_map<std::string, std::vector<int>> m_legendsByMap; ///< Legend IDs by map
    std::unordered_map<int, std::vector<int>> m_legendsByPreferredMVP; ///< Legend IDs by preferred MVP
    std::unordered_map<std::string, std::vector<int>> m_legendsByMBTI; ///< Legend IDs by MBTI
    std::unordered_map<int, std::vector<int>> m_activeLegendsByMap; ///< Active legend IDs by map
    std::unordered_map<int, int> m_legendsByParty;       ///< Legend IDs by party
    std::unordered_map<int, int> m_legendsByMVP;         ///< Legend IDs by MVP
    std::unordered_map<int, std::vector<int>> m_playerInteractions; ///< Player interactions by legend ID
    std::unordered_map<int, std::unordered_map<int, int>> m_playerRelationships; ///< Player relationships by legend ID and char ID
    std::unordered_map<int, std::unordered_map<int, std::vector<int>>> m_backstoryPiecesRevealed; ///< Backstory pieces revealed by legend ID and char ID
    std::unordered_map<int, std::unordered_map<int, int>> m_secretSkillQuestStatus; ///< Secret skill quest status by legend ID and char ID
    std::unordered_map<int, std::unordered_map<int, int>> m_secretSkillQuestProgress; ///< Secret skill quest progress by legend ID and char ID
    std::unordered_map<int, std::vector<std::pair<int, std::string>>> m_legendLogs; ///< Legend logs by legend ID
    std::unordered_map<int, std::unordered_map<std::string, int>> m_legendStatistics; ///< Legend statistics by legend ID and stat type
    bool m_initialized;                                  ///< Initialization flag
    uint64 m_lastUpdateTick;                            ///< Last update tick
    uint64 m_lastDailyResetTick;                        ///< Last daily reset tick
    std::mutex m_mutex;                                 ///< Mutex for thread safety
};

#endif // AI_LEGENDS_AGENT_HPP