/**
 * @file ai_beggar_agent.hpp
 * @brief AI Beggar agent for managing the Nameless Beggar character
 * @author rAthena Development Team
 * @date 2025-02-27
 *
 * This file contains the AI Beggar agent, which manages the Nameless Beggar character
 * that roams cities and interacts with players.
 */

#ifndef AI_BEGGAR_AGENT_HPP
#define AI_BEGGAR_AGENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "../common/ai_agent_base.hpp"
#include "../memory/langchain_memory.hpp"
#include "../config/dynamic_config.hpp"

/**
 * @brief AI Beggar agent
 * 
 * This class manages the Nameless Beggar character that roams cities and interacts with players.
 */
class AIBeggarAgent : public AIAgentBase {
public:
    /**
     * @brief Constructor
     */
    AIBeggarAgent();

    /**
     * @brief Destructor
     */
    virtual ~AIBeggarAgent();

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
     * @brief Handle player trade request
     * @param sd Player data
     * @return True if trade request was accepted, false otherwise
     */
    bool handlePlayerTradeRequest(struct map_session_data* sd);

    /**
     * @brief Handle player trade
     * @param sd Player data
     * @param items Items being traded
     * @param item_count Number of items being traded
     * @return True if trade was successful, false otherwise
     */
    bool handlePlayerTrade(struct map_session_data* sd, struct item* items, int item_count);

    /**
     * @brief Check if item is food
     * @param item_id Item ID
     * @return True if item is food, false otherwise
     */
    bool isFood(int item_id);

    /**
     * @brief Give reward to player
     * @param sd Player data
     * @return True if reward was given, false otherwise
     */
    bool giveReward(struct map_session_data* sd);

    /**
     * @brief Update player feeding streak
     * @param sd Player data
     * @param food_count Number of food items given
     * @return True if streak was updated, false otherwise
     */
    bool updateFeedingStreak(struct map_session_data* sd, int food_count);

    /**
     * @brief Check if player has completed feeding streak
     * @param sd Player data
     * @return True if streak is complete, false otherwise
     */
    bool hasCompletedFeedingStreak(struct map_session_data* sd);

    /**
     * @brief Give streak reward to player
     * @param sd Player data
     * @return True if reward was given, false otherwise
     */
    bool giveStreakReward(struct map_session_data* sd);

    /**
     * @brief Update player fragment collection
     * @param sd Player data
     * @param fragment_count Number of fragments given
     * @return True if collection was updated, false otherwise
     */
    bool updateFragmentCollection(struct map_session_data* sd, int fragment_count);

    /**
     * @brief Check if player has enough fragments for special event
     * @param sd Player data
     * @return True if player has enough fragments, false otherwise
     */
    bool hasEnoughFragmentsForEvent(struct map_session_data* sd);

    /**
     * @brief Trigger special event for player
     * @param sd Player data
     * @return True if event was triggered, false otherwise
     */
    bool triggerSpecialEvent(struct map_session_data* sd);

    /**
     * @brief Complete special event for player
     * @param sd Player data
     * @param result Event result (WIN, LOSE, DRAW)
     * @return True if event was completed, false otherwise
     */
    bool completeSpecialEvent(struct map_session_data* sd, const std::string& result);

    /**
     * @brief Give event reward to player
     * @param sd Player data
     * @return True if reward was given, false otherwise
     */
    bool giveEventReward(struct map_session_data* sd);

    /**
     * @brief Get current city
     * @return Current city
     */
    std::string getCurrentCity();

    /**
     * @brief Set current city
     * @param city City name
     */
    void setCurrentCity(const std::string& city);

    /**
     * @brief Get next city
     * @return Next city
     */
    std::string getNextCity();

    /**
     * @brief Move to next city
     * @return True if move was successful, false otherwise
     */
    bool moveToNextCity();

    /**
     * @brief Spawn in city
     * @param city City name
     * @return True if spawn was successful, false otherwise
     */
    bool spawnInCity(const std::string& city);

    /**
     * @brief Despawn from current city
     * @return True if despawn was successful, false otherwise
     */
    bool despawnFromCurrentCity();

    /**
     * @brief Get players in current city
     * @return Players in current city
     */
    std::vector<struct map_session_data*> getPlayersInCurrentCity();

    /**
     * @brief Get players near beggar
     * @param range Range in cells
     * @return Players near beggar
     */
    std::vector<struct map_session_data*> getPlayersNearBeggar(int range);

    /**
     * @brief Get random player in current city
     * @return Random player in current city
     */
    struct map_session_data* getRandomPlayerInCurrentCity();

    /**
     * @brief Get random player near beggar
     * @param range Range in cells
     * @return Random player near beggar
     */
    struct map_session_data* getRandomPlayerNearBeggar(int range);

    /**
     * @brief Approach player
     * @param sd Player data
     * @return True if approach was successful, false otherwise
     */
    bool approachPlayer(struct map_session_data* sd);

    /**
     * @brief Initiate trade with player
     * @param sd Player data
     * @return True if trade initiation was successful, false otherwise
     */
    bool initiateTradeWithPlayer(struct map_session_data* sd);

    /**
     * @brief Chat with player
     * @param sd Player data
     * @param message Message to send
     * @return True if chat was successful, false otherwise
     */
    bool chatWithPlayer(struct map_session_data* sd, const std::string& message);

    /**
     * @brief Generate chat message for player
     * @param sd Player data
     * @param context Context for message generation
     * @return Generated message
     */
    std::string generateChatMessage(struct map_session_data* sd, const std::string& context);

    /**
     * @brief Log beggar activity
     * @param char_id Character ID (0 for none)
     * @param log_type Log type
     * @param message Log message
     * @return True if log was successful, false otherwise
     */
    bool logBeggarActivity(int char_id, const std::string& log_type, const std::string& message);

    /**
     * @brief Update beggar statistics
     * @param stat_type Statistic type
     * @param value Value to add
     * @return True if update was successful, false otherwise
     */
    bool updateBeggarStatistics(const std::string& stat_type, int value);

private:
    /**
     * @brief Load beggar configuration
     * @return True if load was successful, false otherwise
     */
    bool loadBeggarConfiguration();

    /**
     * @brief Initialize beggar memory
     * @return True if initialization was successful, false otherwise
     */
    bool initializeBeggarMemory();

    /**
     * @brief Schedule beggar appearances
     * @return True if scheduling was successful, false otherwise
     */
    bool scheduleBeggarAppearances();

    /**
     * @brief Check for scheduled appearances
     * @param tick Current tick
     */
    void checkScheduledAppearances(uint64 tick);

    /**
     * @brief Check for scheduled disappearances
     * @param tick Current tick
     */
    void checkScheduledDisappearances(uint64 tick);

    /**
     * @brief Check for player interactions
     * @param tick Current tick
     */
    void checkPlayerInteractions(uint64 tick);

    /**
     * @brief Reset daily statistics
     * @return True if reset was successful, false otherwise
     */
    bool resetDailyStatistics();

    std::string m_currentCity;                           ///< Current city
    std::vector<std::string> m_cities;                   ///< Cities to visit
    int m_currentCityIndex;                              ///< Current city index
    std::string m_visitOrder;                            ///< Visit order (sequential, random)
    time_t m_lastAppearanceTime;                         ///< Last appearance time
    time_t m_nextAppearanceTime;                         ///< Next appearance time
    time_t m_nextDisappearanceTime;                      ///< Next disappearance time
    time_t m_nextCityMoveTime;                           ///< Next city move time
    int m_currentAppearanceId;                           ///< Current appearance ID
    std::unordered_map<int, time_t> m_playerInteractionCooldowns; ///< Player interaction cooldowns
    std::unordered_map<int, int> m_playerInteractionsToday; ///< Player interactions today
    std::vector<int> m_foodItemIds;                      ///< Food item IDs
    int m_rewardItemId;                                  ///< Reward item ID
    int m_rewardAmount;                                  ///< Reward amount
    int m_fragmentThreshold;                             ///< Fragment threshold for special event
    int m_requiredFeedingDays;                           ///< Required feeding days for streak
    int m_requiredFeedingCount;                          ///< Required feeding count per day
    bool m_streakResetOnMiss;                            ///< Whether to reset streak on miss
    int m_streakRewardSkillId;                           ///< Streak reward skill ID
    std::string m_streakRewardSkillName;                 ///< Streak reward skill name
    std::shared_ptr<LangChainMemory> m_memory;           ///< Beggar memory
    std::shared_ptr<DynamicConfig> m_config;             ///< Beggar configuration
    bool m_initialized;                                  ///< Initialization flag
    uint64 m_lastUpdateTick;                             ///< Last update tick
    uint64 m_lastDailyResetTick;                         ///< Last daily reset tick
    std::mutex m_mutex;                                  ///< Mutex for thread safety
};

#endif // AI_BEGGAR_AGENT_HPP