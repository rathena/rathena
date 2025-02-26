/**
 * @file mayor_agent.hpp
 * @brief Mayor AI agent for analyzing stats and creating events
 * @author rAthena Development Team
 * @date 2025-02-27
 *
 * This file contains the Mayor AI agent, which analyzes server statistics
 * and creates events to retain current players and attract new ones.
 */

#ifndef MAYOR_AGENT_HPP
#define MAYOR_AGENT_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

#include "../common/ai_agent_base.hpp"
#include "../memory/langchain_memory.hpp"
#include "../config/dynamic_config.hpp"

/**
 * @brief Event type enumeration
 */
enum class EventType {
    COMBAT,             ///< Combat-focused event
    COLLECTION,         ///< Collection-focused event
    EXPLORATION,        ///< Exploration-focused event
    SOCIAL,             ///< Social-focused event
    CRAFTING,           ///< Crafting-focused event
    TRADING,            ///< Trading-focused event
    SPECIAL,            ///< Special event
    SEASONAL            ///< Seasonal event
};

/**
 * @brief Event difficulty enumeration
 */
enum class EventDifficulty {
    BEGINNER,           ///< Beginner difficulty
    INTERMEDIATE,       ///< Intermediate difficulty
    ADVANCED,           ///< Advanced difficulty
    EXPERT,             ///< Expert difficulty
    MIXED               ///< Mixed difficulty
};

/**
 * @brief Event target enumeration
 */
enum class EventTarget {
    NEW_PLAYERS,        ///< New players
    CASUAL_PLAYERS,     ///< Casual players
    HARDCORE_PLAYERS,   ///< Hardcore players
    RETURNING_PLAYERS,  ///< Returning players
    ALL_PLAYERS         ///< All players
};

/**
 * @brief Event structure
 */
struct Event {
    std::string name;                   ///< Event name
    std::string description;            ///< Event description
    EventType type;                     ///< Event type
    EventDifficulty difficulty;         ///< Event difficulty
    EventTarget target;                 ///< Event target
    std::string start_date;             ///< Event start date
    std::string end_date;               ///< Event end date
    std::vector<std::string> rewards;   ///< Event rewards
    std::string script_path;            ///< Path to event script
    bool is_active;                     ///< Whether the event is active
    bool is_recurring;                  ///< Whether the event is recurring
    std::string recurrence_pattern;     ///< Recurrence pattern (e.g., "weekly", "monthly")
    std::unordered_map<std::string, std::string> custom_parameters; ///< Custom parameters
};

/**
 * @brief Server statistics structure
 */
struct ServerStatistics {
    int total_players;                  ///< Total number of players
    int new_players;                    ///< Number of new players
    int active_players;                 ///< Number of active players
    int returning_players;              ///< Number of returning players
    int inactive_players;               ///< Number of inactive players
    float retention_rate;               ///< Player retention rate
    float churn_rate;                   ///< Player churn rate
    std::unordered_map<std::string, int> player_distribution_by_level; ///< Player distribution by level
    std::unordered_map<std::string, int> player_distribution_by_class; ///< Player distribution by class
    std::unordered_map<std::string, int> player_distribution_by_map;   ///< Player distribution by map
    std::unordered_map<std::string, int> player_activity_by_hour;      ///< Player activity by hour
    std::unordered_map<std::string, int> player_activity_by_day;       ///< Player activity by day
    std::unordered_map<std::string, int> popular_activities;           ///< Popular activities
    std::unordered_map<std::string, int> item_transactions;            ///< Item transactions
    std::unordered_map<std::string, int> zeny_flow;                    ///< Zeny flow
    std::unordered_map<std::string, int> mvp_kills;                    ///< MVP kills
    std::unordered_map<std::string, int> quest_completions;            ///< Quest completions
    std::unordered_map<std::string, int> event_participation;          ///< Event participation
    std::unordered_map<std::string, float> event_satisfaction;         ///< Event satisfaction
    std::string start_date;                                            ///< Statistics start date
    std::string end_date;                                              ///< Statistics end date
};

/**
 * @brief Mayor AI agent
 * 
 * This class analyzes server statistics and creates events to retain current players and attract new ones.
 */
class MayorAgent : public AIAgentBase {
public:
    /**
     * @brief Constructor
     */
    MayorAgent();

    /**
     * @brief Destructor
     */
    virtual ~MayorAgent();

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
     * @brief Analyze server statistics
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzeStatistics(const ServerStatistics& stats);

    /**
     * @brief Generate event ideas
     * @param stats Server statistics
     * @param count Number of event ideas to generate
     * @return Event ideas
     */
    std::vector<Event> generateEventIdeas(const ServerStatistics& stats, int count = 3);

    /**
     * @brief Create event
     * @param event Event to create
     * @return True if event was created successfully, false otherwise
     */
    bool createEvent(const Event& event);

    /**
     * @brief Update event
     * @param event_name Event name
     * @param event Updated event
     * @return True if event was updated successfully, false otherwise
     */
    bool updateEvent(const std::string& event_name, const Event& event);

    /**
     * @brief Delete event
     * @param event_name Event name
     * @return True if event was deleted successfully, false otherwise
     */
    bool deleteEvent(const std::string& event_name);

    /**
     * @brief Get event
     * @param event_name Event name
     * @return Event
     */
    Event getEvent(const std::string& event_name);

    /**
     * @brief Get all events
     * @return All events
     */
    std::vector<Event> getAllEvents();

    /**
     * @brief Get active events
     * @return Active events
     */
    std::vector<Event> getActiveEvents();

    /**
     * @brief Get upcoming events
     * @return Upcoming events
     */
    std::vector<Event> getUpcomingEvents();

    /**
     * @brief Get past events
     * @return Past events
     */
    std::vector<Event> getPastEvents();

    /**
     * @brief Get events by type
     * @param type Event type
     * @return Events of the specified type
     */
    std::vector<Event> getEventsByType(EventType type);

    /**
     * @brief Get events by difficulty
     * @param difficulty Event difficulty
     * @return Events of the specified difficulty
     */
    std::vector<Event> getEventsByDifficulty(EventDifficulty difficulty);

    /**
     * @brief Get events by target
     * @param target Event target
     * @return Events of the specified target
     */
    std::vector<Event> getEventsByTarget(EventTarget target);

    /**
     * @brief Get server statistics
     * @param start_date Start date
     * @param end_date End date
     * @return Server statistics
     */
    ServerStatistics getServerStatistics(const std::string& start_date, const std::string& end_date);

    /**
     * @brief Get weekly server statistics
     * @return Weekly server statistics
     */
    ServerStatistics getWeeklyServerStatistics();

    /**
     * @brief Get monthly server statistics
     * @return Monthly server statistics
     */
    ServerStatistics getMonthlyServerStatistics();

    /**
     * @brief Get yearly server statistics
     * @return Yearly server statistics
     */
    ServerStatistics getYearlyServerStatistics();

    /**
     * @brief Generate event script
     * @param event Event to generate script for
     * @return Path to generated script
     */
    std::string generateEventScript(const Event& event);

    /**
     * @brief Generate event announcement
     * @param event Event to generate announcement for
     * @return Announcement message
     */
    std::string generateEventAnnouncement(const Event& event);

    /**
     * @brief Send event announcement
     * @param event Event to announce
     * @return True if announcement was sent successfully, false otherwise
     */
    bool sendEventAnnouncement(const Event& event);

    /**
     * @brief Generate player retention strategies
     * @param stats Server statistics
     * @return Player retention strategies
     */
    std::vector<std::string> generatePlayerRetentionStrategies(const ServerStatistics& stats);

    /**
     * @brief Generate player acquisition strategies
     * @param stats Server statistics
     * @return Player acquisition strategies
     */
    std::vector<std::string> generatePlayerAcquisitionStrategies(const ServerStatistics& stats);

    /**
     * @brief Generate weekly report
     * @param stats Server statistics
     * @return Weekly report
     */
    std::string generateWeeklyReport(const ServerStatistics& stats);

    /**
     * @brief Log mayor activity
     * @param activity_type Activity type
     * @param message Activity message
     * @return True if log was successful, false otherwise
     */
    bool logMayorActivity(const std::string& activity_type, const std::string& message);

private:
    /**
     * @brief Load mayor configuration
     * @return True if load was successful, false otherwise
     */
    bool loadMayorConfiguration();

    /**
     * @brief Initialize mayor memory
     * @return True if initialization was successful, false otherwise
     */
    bool initializeMayorMemory();

    /**
     * @brief Schedule weekly analysis
     * @return True if scheduling was successful, false otherwise
     */
    bool scheduleWeeklyAnalysis();

    /**
     * @brief Check for scheduled analysis
     * @param tick Current tick
     */
    void checkScheduledAnalysis(uint64 tick);

    /**
     * @brief Check for event start/end
     * @param tick Current tick
     */
    void checkEventStartEnd(uint64 tick);

    /**
     * @brief Reset weekly statistics
     * @return True if reset was successful, false otherwise
     */
    bool resetWeeklyStatistics();

    /**
     * @brief Load events from database
     * @return True if load was successful, false otherwise
     */
    bool loadEventsFromDatabase();

    /**
     * @brief Save events to database
     * @return True if save was successful, false otherwise
     */
    bool saveEventsToDatabase();

    /**
     * @brief Generate event name
     * @param type Event type
     * @param target Event target
     * @return Generated event name
     */
    std::string generateEventName(EventType type, EventTarget target);

    /**
     * @brief Generate event description
     * @param event Event to generate description for
     * @return Generated event description
     */
    std::string generateEventDescription(const Event& event);

    /**
     * @brief Generate event rewards
     * @param event Event to generate rewards for
     * @return Generated event rewards
     */
    std::vector<std::string> generateEventRewards(const Event& event);

    /**
     * @brief Analyze player behavior
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzePlayerBehavior(const ServerStatistics& stats);

    /**
     * @brief Analyze player demographics
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzePlayerDemographics(const ServerStatistics& stats);

    /**
     * @brief Analyze player activity
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzePlayerActivity(const ServerStatistics& stats);

    /**
     * @brief Analyze economy
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzeEconomy(const ServerStatistics& stats);

    /**
     * @brief Analyze event performance
     * @param stats Server statistics
     * @return Analysis result
     */
    std::string analyzeEventPerformance(const ServerStatistics& stats);

    /**
     * @brief Identify player segments
     * @param stats Server statistics
     * @return Player segments
     */
    std::unordered_map<std::string, std::vector<int>> identifyPlayerSegments(const ServerStatistics& stats);

    /**
     * @brief Identify trends
     * @param stats Server statistics
     * @return Trends
     */
    std::unordered_map<std::string, float> identifyTrends(const ServerStatistics& stats);

    /**
     * @brief Identify issues
     * @param stats Server statistics
     * @return Issues
     */
    std::vector<std::string> identifyIssues(const ServerStatistics& stats);

    /**
     * @brief Identify opportunities
     * @param stats Server statistics
     * @return Opportunities
     */
    std::vector<std::string> identifyOpportunities(const ServerStatistics& stats);

    std::shared_ptr<LangChainMemory> m_memory;           ///< Mayor memory
    std::shared_ptr<DynamicConfig> m_config;             ///< Mayor configuration
    std::unordered_map<std::string, Event> m_events;     ///< Events
    time_t m_lastAnalysisTime;                           ///< Last analysis time
    time_t m_nextAnalysisTime;                           ///< Next analysis time
    bool m_initialized;                                  ///< Initialization flag
    uint64 m_lastUpdateTick;                             ///< Last update tick
    std::mutex m_mutex;                                  ///< Mutex for thread safety
};

#endif // MAYOR_AGENT_HPP