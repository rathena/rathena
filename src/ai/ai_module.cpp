/**
 * @file ai_module.cpp
 * @brief Main AI module for rAthena AI World
 * @author rAthena Development Team
 * @date 2025-02-26
 *
 * This file contains the implementation of the main AI module that manages
 * AI providers and agents for the rAthena AI World project.
 */

#include "ai_module.hpp"

#include <fstream>
#include <iostream>
#include <mutex>

#include "../common/cbasetypes.hpp"
#include "../common/conf.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/nullpo.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"

#include "common/ai_request.hpp"
#include "common/ai_response.hpp"
#include "common/ai_provider.hpp"
#include "common/ai_agent.hpp"

// Provider implementations
#include "providers/azure_openai_provider.hpp"
#include "providers/openai_provider.hpp"
#include "providers/deepseek_provider.hpp"
#include "providers/local_provider.hpp"

// Agent implementations
#include "agents/world_evolution_agent.hpp"
#include "agents/legend_bloodlines_agent.hpp"
#include "agents/cross_class_synthesis_agent.hpp"
#include "agents/quest_generation_agent.hpp"
#include "agents/economic_ecosystem_agent.hpp"
#include "agents/social_dynamics_agent.hpp"
#include "agents/combat_mechanics_agent.hpp"
#include "agents/housing_system_agent.hpp"
#include "agents/time_manipulation_agent.hpp"
#include "agents/npc_intelligence_agent.hpp"
#include "agents/guild_evolution_agent.hpp"
#include "agents/dimensional_warfare_agent.hpp"

// Update interval for agents (in milliseconds)
#define AI_UPDATE_INTERVAL 1000

// Singleton instance
static std::unique_ptr<AIModule> s_instance = nullptr;
static std::mutex s_instanceMutex;

AIModule& AIModule::getInstance() {
    std::lock_guard<std::mutex> lock(s_instanceMutex);
    if (!s_instance) {
        s_instance = std::unique_ptr<AIModule>(new AIModule());
    }
    return *s_instance;
}

AIModule::AIModule()
    : m_initialized(false)
    , m_enabled(false)
    , m_logLevel(3)
    , m_cacheEnabled(true)
    , m_cacheExpiry(86400)
    , m_requestTimeout(10000)
    , m_fallbackEnabled(true)
    , m_primaryProviderName("azure_openai")
    , m_fallbackProviderName("local")
    , m_updateTimerId(-1)
{
}

AIModule::~AIModule() {
    finalize();
}

bool AIModule::initialize() {
    if (m_initialized) {
        ShowWarning("AI Module: Already initialized\n");
        return true;
    }

    ShowStatus("AI Module: Initializing...\n");

    // Load configuration
    if (!loadConfig()) {
        ShowError("AI Module: Failed to load configuration\n");
        return false;
    }

    // Check if AI is enabled
    if (!m_enabled) {
        ShowInfo("AI Module: Disabled in configuration\n");
        return true;
    }

    // Initialize providers
    if (!initializeProviders()) {
        ShowError("AI Module: Failed to initialize providers\n");
        return false;
    }

    // Initialize agents
    if (!initializeAgents()) {
        ShowError("AI Module: Failed to initialize agents\n");
        return false;
    }

    // Start update timer
    m_updateTimerId = add_timer_interval(gettick(), updateAgentsTimer, 0, 0, AI_UPDATE_INTERVAL);

    m_initialized = true;
    ShowStatus("AI Module: Initialized successfully\n");
    return true;
}

void AIModule::finalize() {
    if (!m_initialized) {
        return;
    }

    ShowStatus("AI Module: Finalizing...\n");

    // Stop update timer
    if (m_updateTimerId != -1) {
        delete_timer(m_updateTimerId, updateAgentsTimer);
        m_updateTimerId = -1;
    }

    // Clear agents
    m_agents.clear();

    // Clear providers
    m_providers.clear();

    m_initialized = false;
    ShowStatus("AI Module: Finalized\n");
}

bool AIModule::isEnabled() const {
    return m_enabled;
}

std::shared_ptr<AIProvider> AIModule::getPrimaryProvider() {
    return getProvider(m_primaryProviderName);
}

std::shared_ptr<AIProvider> AIModule::getFallbackProvider() {
    if (!m_fallbackEnabled) {
        return nullptr;
    }
    return getProvider(m_fallbackProviderName);
}

std::shared_ptr<AIProvider> AIModule::getProvider(const std::string& providerName) {
    auto it = m_providers.find(providerName);
    if (it != m_providers.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AIAgent> AIModule::getAgent(const std::string& agentName) {
    auto it = m_agents.find(agentName);
    if (it != m_agents.end()) {
        return it->second;
    }
    return nullptr;
}

AIResponse AIModule::processRequest(const AIRequest& request) {
    // Get the primary provider
    auto provider = getPrimaryProvider();
    if (!provider) {
        ShowError("AI Module: Primary provider not available\n");
        return AIResponse::createError("Primary provider not available");
    }

    // Try to process the request with the primary provider
    try {
        AIResponse response = provider->processRequest(request);
        if (response.isSuccess()) {
            return response;
        }

        // If primary provider failed and fallback is enabled, try the fallback provider
        if (m_fallbackEnabled) {
            auto fallbackProvider = getFallbackProvider();
            if (fallbackProvider) {
                ShowWarning("AI Module: Primary provider failed, trying fallback provider\n");
                return fallbackProvider->processRequest(request);
            }
        }

        return response;
    } catch (const std::exception& e) {
        ShowError("AI Module: Exception while processing request: %s\n", e.what());
        return AIResponse::createError(e.what());
    }
}

void AIModule::update(uint64 tick) {
    if (!m_initialized || !m_enabled) {
        return;
    }

    // Update all agents
    for (auto& pair : m_agents) {
        auto& agent = pair.second;
        if (agent && agent->isEnabled()) {
            agent->update(tick);
        }
    }
}

bool AIModule::reloadConfig() {
    ShowStatus("AI Module: Reloading configuration...\n");

    // Save old state
    bool wasEnabled = m_enabled;
    std::string oldPrimaryProvider = m_primaryProviderName;
    std::string oldFallbackProvider = m_fallbackProviderName;

    // Load new configuration
    if (!loadConfig()) {
        ShowError("AI Module: Failed to reload configuration\n");
        return false;
    }

    // Check if major settings changed
    bool needRestart = (wasEnabled != m_enabled) ||
                      (oldPrimaryProvider != m_primaryProviderName) ||
                      (oldFallbackProvider != m_fallbackProviderName);

    if (needRestart) {
        ShowInfo("AI Module: Major settings changed, reinitializing...\n");
        finalize();
        return initialize();
    }

    // Update provider configurations
    for (auto& pair : m_providers) {
        pair.second->reloadConfig();
    }

    // Update agent configurations
    for (auto& pair : m_agents) {
        pair.second->reloadConfig();
    }

    ShowStatus("AI Module: Configuration reloaded successfully\n");
    return true;
}

void AIModule::log(int level, const std::string& component, const std::string& message, const std::string& details) {
    if (level > m_logLevel) {
        return;
    }

    // Log to console
    switch (level) {
        case 1: // Error
            ShowError("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
        case 2: // Warning
            ShowWarning("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
        case 3: // Info
            ShowInfo("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
        case 4: // Debug
            ShowDebug("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
        case 5: // Trace
            ShowTrace("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
        default:
            ShowInfo("AI [%s]: %s\n", component.c_str(), message.c_str());
            break;
    }

    // TODO: Log to database
}

bool AIModule::loadConfig() {
    // Load AI configuration file
    std::string configFile = "conf/ai_agents.conf";
    ShowInfo("AI Module: Loading configuration from %s\n", configFile.c_str());

    struct s_aiconfig_type {
        struct {
            int enabled;
            int log_level;
            int cache_enabled;
            int cache_expiry;
            int request_timeout;
            int fallback_enabled;
            char primary_provider[32];
            char fallback_provider[32];
        } global;

        struct {
            int enabled;
            char api_key[128];
            char endpoint[256];
            char api_version[32];
            char deployment_id[64];
            char model[32];
            float temperature;
            int max_tokens;
            float top_p;
            float frequency_penalty;
            float presence_penalty;
            int request_timeout;
        } azure_openai;

        struct {
            int enabled;
            char api_key[128];
            char organization_id[64];
            char model[32];
            float temperature;
            int max_tokens;
            float top_p;
            float frequency_penalty;
            float presence_penalty;
            int request_timeout;
        } openai;

        struct {
            int enabled;
            char api_key[128];
            char model[32];
            float temperature;
            int max_tokens;
            float top_p;
            float frequency_penalty;
            float presence_penalty;
            int request_timeout;
        } deepseek;

        struct {
            int enabled;
            char model_path[256];
            char model_type[32];
            int context_size;
            float temperature;
            int max_tokens;
            int threads;
        } local;

        struct {
            int enabled;
            int update_interval;
            int memory_retention;
            float weather_change_probability;
            int seasonal_enabled;
            int territory_control_enabled;
            int resource_dynamics_enabled;
        } world_evolution;

        struct {
            int enabled;
            float inheritance_probability;
            float mutation_probability;
            int max_bloodline_depth;
            float trait_decay_rate;
        } legend_bloodlines;

        struct {
            int enabled;
            int max_fusion_skills;
            float skill_compatibility_threshold;
            int fusion_cooldown;
            int experimental_skills_enabled;
        } cross_class_synthesis;

        struct {
            int enabled;
            int daily_quest_limit;
            int quest_complexity;
            float story_continuity_weight;
            float personalization_level;
            int quest_generation_interval;
        } quest_generation;

        struct {
            int enabled;
            float price_volatility;
            float supply_demand_sensitivity;
            int regional_variance;
            int trade_route_count;
            int economic_update_interval;
        } economic_ecosystem;

        struct {
            int enabled;
            int faction_count;
            int reputation_memory;
            float alliance_volatility;
            int territory_influence_radius;
            int social_update_interval;
        } social_dynamics;

        struct {
            int enabled;
            float terrain_effect_multiplier;
            float weather_effect_multiplier;
            int cover_system_enabled;
            int environmental_damage_enabled;
        } combat_mechanics;

        struct {
            int enabled;
            int max_houses_per_player;
            int upgrade_levels;
            int customization_options;
            int functionality_options;
        } housing_system;

        struct {
            int enabled;
            int time_skill_cooldown;
            int historical_events_enabled;
            int time_locked_content_rotation;
            int paradox_quest_difficulty;
        } time_manipulation;

        struct {
            int enabled;
            int memory_retention;
            int conversation_depth;
            int personality_types;
            float relationship_development_rate;
            float npc_learning_rate;
        } npc_intelligence;

        struct {
            int enabled;
            int specialization_paths;
            int technology_tree_depth;
            int alliance_limit;
            int evolution_points_per_day;
            int organizational_challenge_interval;
        } guild_evolution;

        struct {
            int enabled;
            int dimension_count;
            int reality_warp_cooldown;
            int plane_shift_cost;
            float dimensional_stability;
        } dimensional_warfare;
    };

    struct s_aiconfig_type config;
    memset(&config, 0, sizeof(config));

    // Set default values
    config.global.enabled = 1;
    config.global.log_level = 3;
    config.global.cache_enabled = 1;
    config.global.cache_expiry = 86400;
    config.global.request_timeout = 10000;
    config.global.fallback_enabled = 1;
    safestrncpy(config.global.primary_provider, "azure_openai", sizeof(config.global.primary_provider));
    safestrncpy(config.global.fallback_provider, "local", sizeof(config.global.fallback_provider));

    // Load configuration
    if (!libconfig->read_file(&config, sizeof(config), configFile.c_str())) {
        ShowError("AI Module: Failed to read configuration file %s\n", configFile.c_str());
        return false;
    }

    // Apply global configuration
    m_enabled = (config.global.enabled != 0);
    m_logLevel = config.global.log_level;
    m_cacheEnabled = (config.global.cache_enabled != 0);
    m_cacheExpiry = config.global.cache_expiry;
    m_requestTimeout = config.global.request_timeout;
    m_fallbackEnabled = (config.global.fallback_enabled != 0);
    m_primaryProviderName = config.global.primary_provider;
    m_fallbackProviderName = config.global.fallback_provider;

    ShowInfo("AI Module: Configuration loaded successfully\n");
    ShowInfo("AI Module: Enabled: %s\n", m_enabled ? "Yes" : "No");
    ShowInfo("AI Module: Primary Provider: %s\n", m_primaryProviderName.c_str());
    ShowInfo("AI Module: Fallback Provider: %s\n", m_fallbackEnabled ? m_fallbackProviderName.c_str() : "Disabled");

    return true;
}

bool AIModule::initializeProviders() {
    ShowInfo("AI Module: Initializing providers...\n");

    // Clear existing providers
    m_providers.clear();

    // Create and initialize providers
    try {
        // Azure OpenAI provider
        auto azureOpenAI = std::make_shared<AzureOpenAIProvider>();
        if (azureOpenAI->initialize()) {
            m_providers["azure_openai"] = azureOpenAI;
            ShowInfo("AI Module: Azure OpenAI provider initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Azure OpenAI provider\n");
        }

        // OpenAI provider
        auto openAI = std::make_shared<OpenAIProvider>();
        if (openAI->initialize()) {
            m_providers["openai"] = openAI;
            ShowInfo("AI Module: OpenAI provider initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize OpenAI provider\n");
        }

        // DeepSeek provider
        auto deepSeek = std::make_shared<DeepSeekProvider>();
        if (deepSeek->initialize()) {
            m_providers["deepseek"] = deepSeek;
            ShowInfo("AI Module: DeepSeek provider initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize DeepSeek provider\n");
        }

        // Local provider
        auto local = std::make_shared<LocalProvider>();
        if (local->initialize()) {
            m_providers["local"] = local;
            ShowInfo("AI Module: Local provider initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Local provider\n");
        }
    } catch (const std::exception& e) {
        ShowError("AI Module: Exception while initializing providers: %s\n", e.what());
        return false;
    }

    // Check if primary provider is available
    if (m_providers.find(m_primaryProviderName) == m_providers.end()) {
        ShowError("AI Module: Primary provider '%s' not available\n", m_primaryProviderName.c_str());
        return false;
    }

    // Check if fallback provider is available (if enabled)
    if (m_fallbackEnabled && m_providers.find(m_fallbackProviderName) == m_providers.end()) {
        ShowWarning("AI Module: Fallback provider '%s' not available, disabling fallback\n", m_fallbackProviderName.c_str());
        m_fallbackEnabled = false;
    }

    ShowInfo("AI Module: Providers initialized successfully\n");
    return true;
}

bool AIModule::initializeAgents() {
    ShowInfo("AI Module: Initializing agents...\n");

    // Clear existing agents
    m_agents.clear();

    // Create and initialize agents
    try {
        // World Evolution agent
        auto worldEvolution = std::make_shared<WorldEvolutionAgent>();
        if (worldEvolution->initialize()) {
            m_agents["world_evolution"] = worldEvolution;
            ShowInfo("AI Module: World Evolution agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize World Evolution agent\n");
        }

        // Legend Bloodlines agent
        auto legendBloodlines = std::make_shared<LegendBloodlinesAgent>();
        if (legendBloodlines->initialize()) {
            m_agents["legend_bloodlines"] = legendBloodlines;
            ShowInfo("AI Module: Legend Bloodlines agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Legend Bloodlines agent\n");
        }

        // Cross-Class Synthesis agent
        auto crossClassSynthesis = std::make_shared<CrossClassSynthesisAgent>();
        if (crossClassSynthesis->initialize()) {
            m_agents["cross_class_synthesis"] = crossClassSynthesis;
            ShowInfo("AI Module: Cross-Class Synthesis agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Cross-Class Synthesis agent\n");
        }

        // Quest Generation agent
        auto questGeneration = std::make_shared<QuestGenerationAgent>();
        if (questGeneration->initialize()) {
            m_agents["quest_generation"] = questGeneration;
            ShowInfo("AI Module: Quest Generation agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Quest Generation agent\n");
        }

        // Economic Ecosystem agent
        auto economicEcosystem = std::make_shared<EconomicEcosystemAgent>();
        if (economicEcosystem->initialize()) {
            m_agents["economic_ecosystem"] = economicEcosystem;
            ShowInfo("AI Module: Economic Ecosystem agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Economic Ecosystem agent\n");
        }

        // Social Dynamics agent
        auto socialDynamics = std::make_shared<SocialDynamicsAgent>();
        if (socialDynamics->initialize()) {
            m_agents["social_dynamics"] = socialDynamics;
            ShowInfo("AI Module: Social Dynamics agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Social Dynamics agent\n");
        }

        // Combat Mechanics agent
        auto combatMechanics = std::make_shared<CombatMechanicsAgent>();
        if (combatMechanics->initialize()) {
            m_agents["combat_mechanics"] = combatMechanics;
            ShowInfo("AI Module: Combat Mechanics agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Combat Mechanics agent\n");
        }

        // Housing System agent
        auto housingSystem = std::make_shared<HousingSystemAgent>();
        if (housingSystem->initialize()) {
            m_agents["housing_system"] = housingSystem;
            ShowInfo("AI Module: Housing System agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Housing System agent\n");
        }

        // Time Manipulation agent
        auto timeManipulation = std::make_shared<TimeManipulationAgent>();
        if (timeManipulation->initialize()) {
            m_agents["time_manipulation"] = timeManipulation;
            ShowInfo("AI Module: Time Manipulation agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Time Manipulation agent\n");
        }

        // NPC Intelligence agent
        auto npcIntelligence = std::make_shared<NPCIntelligenceAgent>();
        if (npcIntelligence->initialize()) {
            m_agents["npc_intelligence"] = npcIntelligence;
            ShowInfo("AI Module: NPC Intelligence agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize NPC Intelligence agent\n");
        }

        // Guild Evolution agent
        auto guildEvolution = std::make_shared<GuildEvolutionAgent>();
        if (guildEvolution->initialize()) {
            m_agents["guild_evolution"] = guildEvolution;
            ShowInfo("AI Module: Guild Evolution agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Guild Evolution agent\n");
        }

        // Dimensional Warfare agent
        auto dimensionalWarfare = std::make_shared<DimensionalWarfareAgent>();
        if (dimensionalWarfare->initialize()) {
            m_agents["dimensional_warfare"] = dimensionalWarfare;
            ShowInfo("AI Module: Dimensional Warfare agent initialized\n");
        } else {
            ShowWarning("AI Module: Failed to initialize Dimensional Warfare agent\n");
        }
    } catch (const std::exception& e) {
        ShowError("AI Module: Exception while initializing agents: %s\n", e.what());
        return false;
    }

    ShowInfo("AI Module: Agents initialized successfully\n");
    return true;
}

TIMER_FUNC(AIModule::updateAgentsTimer) {
    AIModule::getInstance().update(tick);
    return AI_UPDATE_INTERVAL;
}