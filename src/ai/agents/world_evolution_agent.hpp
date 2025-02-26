/**
 * @file world_evolution_agent.hpp
 * @brief World Evolution agent for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the World Evolution agent implementation that handles
 * dynamic world state, weather changes, and environmental adaptation.
 */

#ifndef WORLD_EVOLUTION_AGENT_HPP
#define WORLD_EVOLUTION_AGENT_HPP

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
 * @brief World Evolution agent
 * 
 * This class implements the AI agent interface for World Evolution.
 * It handles dynamic world state, weather changes, and environmental adaptation.
 */
class WorldEvolutionAgent : public AIAgent {
public:
    /**
     * @brief Constructor
     */
    WorldEvolutionAgent();

    /**
     * @brief Destructor
     */
    virtual ~WorldEvolutionAgent();

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
     * @brief Record a global event
     * @param eventType Event type
     * @param eventName Event name
     * @param description Event description
     * @param impactLevel Impact level (1-10)
     * @param affectedMaps Affected maps
     * @param affectedNpcs Affected NPCs
     * @param affectedMobs Affected mobs
     * @param createdBy Creator (player or system)
     * @return Event ID
     */
    int recordGlobalEvent(const std::string& eventType, const std::string& eventName, const std::string& description,
                          int impactLevel, const std::string& affectedMaps, const std::string& affectedNpcs,
                          const std::string& affectedMobs, const std::string& createdBy);

    /**
     * @brief Update weather for a map
     * @param mapId Map ID
     * @param weatherType Weather type
     * @param intensity Weather intensity (0-10)
     * @param duration Duration in seconds (0 for indefinite)
     * @return True if successful, false otherwise
     */
    bool updateWeather(int mapId, const std::string& weatherType, int intensity, int duration);

    /**
     * @brief Get current weather for a map
     * @param mapId Map ID
     * @return Weather information as JSON string
     */
    std::string getCurrentWeather(int mapId) const;

    /**
     * @brief Update season for the world
     * @param season Season name
     * @return True if successful, false otherwise
     */
    bool updateSeason(const std::string& season);

    /**
     * @brief Get current season
     * @return Current season
     */
    std::string getCurrentSeason() const;

    /**
     * @brief Update territory control
     * @param mapId Map ID
     * @param factionId Faction ID
     * @param controlLevel Control level (0-100)
     * @return True if successful, false otherwise
     */
    bool updateTerritoryControl(int mapId, int factionId, int controlLevel);

    /**
     * @brief Get territory control information
     * @param mapId Map ID
     * @return Territory control information as JSON string
     */
    std::string getTerritoryControl(int mapId) const;

    /**
     * @brief Update resource availability
     * @param resourceType Resource type
     * @param mapId Map ID
     * @param availabilityLevel Availability level (0-100)
     * @param regenerationRate Regeneration rate (0-100)
     * @return True if successful, false otherwise
     */
    bool updateResourceAvailability(const std::string& resourceType, int mapId, int availabilityLevel, int regenerationRate);

    /**
     * @brief Get resource availability information
     * @param resourceType Resource type
     * @param mapId Map ID
     * @return Resource availability information as JSON string
     */
    std::string getResourceAvailability(const std::string& resourceType, int mapId) const;

private:
    /**
     * @brief Load configuration
     * @return True if loading was successful, false otherwise
     */
    bool loadConfig();

    /**
     * @brief Update world state
     */
    void updateWorldState();

    /**
     * @brief Update weather
     */
    void updateWeatherSystem();

    /**
     * @brief Update seasons
     */
    void updateSeasons();

    /**
     * @brief Update territory control
     */
    void updateTerritories();

    /**
     * @brief Update resources
     */
    void updateResources();

    /**
     * @brief Generate a weather event
     * @param mapId Map ID
     * @return True if a weather event was generated, false otherwise
     */
    bool generateWeatherEvent(int mapId);

    /**
     * @brief Apply weather effects
     * @param mapId Map ID
     * @param weatherType Weather type
     * @param intensity Weather intensity
     */
    void applyWeatherEffects(int mapId, const std::string& weatherType, int intensity);

    /**
     * @brief Apply seasonal effects
     * @param season Season name
     */
    void applySeasonalEffects(const std::string& season);

    /**
     * @brief Apply territory control effects
     * @param mapId Map ID
     * @param factionId Faction ID
     * @param controlLevel Control level
     */
    void applyTerritoryControlEffects(int mapId, int factionId, int controlLevel);

    /**
     * @brief Apply resource availability effects
     * @param resourceType Resource type
     * @param mapId Map ID
     * @param availabilityLevel Availability level
     */
    void applyResourceAvailabilityEffects(const std::string& resourceType, int mapId, int availabilityLevel);

    bool m_initialized;                      ///< Initialization flag
    bool m_enabled;                          ///< Enabled flag
    int m_updateInterval;                    ///< Update interval in seconds
    int m_memoryRetention;                   ///< Memory retention in days
    float m_weatherChangeProbability;        ///< Weather change probability
    bool m_seasonalEnabled;                  ///< Seasonal changes enabled flag
    bool m_territoryControlEnabled;          ///< Territory control enabled flag
    bool m_resourceDynamicsEnabled;          ///< Resource dynamics enabled flag
    uint64 m_lastUpdateTime;                 ///< Last update time
    std::string m_currentSeason;             ///< Current season
    std::unordered_map<int, std::string> m_mapWeather;  ///< Map weather (map_id -> weather_type)
    std::unordered_map<int, int> m_mapWeatherIntensity; ///< Map weather intensity (map_id -> intensity)
    std::unordered_map<int, uint64> m_mapWeatherExpiry; ///< Map weather expiry time (map_id -> expiry_time)
    std::unordered_map<int, std::pair<int, int>> m_territoryControl; ///< Territory control (map_id -> (faction_id, control_level))
    std::unordered_map<std::string, std::unordered_map<int, int>> m_resourceAvailability; ///< Resource availability (resource_type -> (map_id -> availability_level))
    mutable std::mutex m_mutex;              ///< Mutex for thread safety
};

#endif // WORLD_EVOLUTION_AGENT_HPP