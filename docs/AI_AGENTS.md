# AI Agents for rAthena

This document provides information about the AI agents implemented in rAthena, including how to configure and use them.

## Table of Contents

1. [Introduction](#introduction)
2. [Configuration](#configuration)
3. [AI Providers](#ai-providers)
4. [AI Agents](#ai-agents)
5. [Script Commands](#script-commands)
6. [Events](#events)
7. [Database Schema](#database-schema)
8. [Technical Requirements](#technical-requirements)

## Introduction

The AI agents system enhances rAthena with advanced artificial intelligence capabilities, creating a more dynamic and immersive gameplay experience. The system uses modern AI technologies like Azure OpenAI (GPT-4o), OpenAI (GPT-4o), or DeepSeek V3 to power various game features.

Key features include:
- Dynamic world evolution with weather changes and environmental adaptation
- Player bloodlines and inheritance systems
- Cross-class skill synthesis
- AI-driven quest generation
- Dynamic economic systems
- Advanced NPC intelligence with MBTI personalities
- And much more!

## Configuration

AI agents are configured in the `conf/ai_agents.conf` file. This file contains settings for:

- Global AI settings (enabled/disabled, logging, caching)
- AI provider configuration (Azure OpenAI, OpenAI, DeepSeek, Local)
- Individual agent settings

Example configuration:

```
// Global AI Settings
ai_enabled: true                     // Enable or disable AI functionality globally
ai_log_level: 3                      // Log level (0=none, 1=error, 2=warning, 3=info, 4=debug, 5=trace)
ai_cache_enabled: true               // Enable caching of AI responses to reduce API calls

// Provider Selection
ai_primary_provider: "azure_openai"  // Primary AI provider to use
ai_fallback_provider: "local"        // Fallback AI provider if primary fails

// Azure OpenAI Configuration
azure_openai: {
    enabled: true                    // Enable Azure OpenAI integration
    api_key: "your_api_key_here"     // Your Azure OpenAI API key
    endpoint: "your_endpoint_here"   // Azure OpenAI endpoint URL
    // ... other settings
}
```

## AI Providers

The system supports multiple AI providers:

### Azure OpenAI

Microsoft's Azure OpenAI service, which provides access to models like GPT-4o.

Configuration:
```
azure_openai: {
    enabled: true
    api_key: "your_api_key_here"
    endpoint: "https://your-resource.openai.azure.com/"
    api_version: "2023-05-15"
    deployment_id: "your_deployment_id"
    model: "gpt-4o"
    temperature: 0.7
    max_tokens: 1000
}
```

### OpenAI

Direct integration with OpenAI's API.

Configuration:
```
openai: {
    enabled: true
    api_key: "your_api_key_here"
    organization_id: "your_org_id" // Optional
    model: "gpt-4o"
    temperature: 0.7
    max_tokens: 1000
}
```

### DeepSeek V3

Integration with DeepSeek's V3 models.

Configuration:
```
deepseek: {
    enabled: true
    api_key: "your_api_key_here"
    model: "deepseek-v3"
    temperature: 0.7
    max_tokens: 1000
}
```

### Local (Fallback)

A local AI model for fallback when cloud services are unavailable.

Configuration:
```
local: {
    enabled: true
    model_path: "./models/llama3"
    model_type: "llama"
    context_size: 4096
    temperature: 0.7
    threads: 4
}
```

## AI Agents

The system includes the following AI agents:

### 1. World Evolution Agent

Creates a dynamic world with changing weather, seasons, and environments that adapt to player actions.

Features:
- Dynamic weather system affecting skills and movement
- Seasonal changes with unique effects
- Territory control affecting monster behavior
- Resource availability based on player actions

Configuration:
```
world_evolution: {
    enabled: true
    update_interval: 3600            // Update interval in seconds
    weather_change_probability: 0.3  // Probability of weather changing per update
    seasonal_enabled: true           // Enable seasonal changes
    territory_control_enabled: true  // Enable territory control features
}
```

### 2. Legend Bloodlines Agent

Implements a player legacy system with inherited traits and abilities.

Features:
- Bloodline tracking and inheritance
- Unique traits based on ancestry
- Family trees and generational challenges

Configuration:
```
legend_bloodlines: {
    enabled: true
    inheritance_probability: 0.7     // Probability of trait inheritance
    mutation_probability: 0.1        // Probability of trait mutation
    max_bloodline_depth: 5           // Maximum generations to track
}
```

### 3. Cross-Class Synthesis Agent

Enables players to combine skills from different classes to create unique abilities.

Features:
- Skill fusion system
- Hybrid ability creation
- Experimental skill development

Configuration:
```
cross_class_synthesis: {
    enabled: true
    max_fusion_skills: 5             // Maximum number of fusion skills per character
    skill_compatibility_threshold: 0.6 // Threshold for skill compatibility
    experimental_skills_enabled: true // Enable experimental skill combinations
}
```

### 4. Quest Generation Agent

Creates AI-driven quests and storylines tailored to player progress and actions.

Features:
- Personalized quest generation
- Interconnected storylines
- Adaptive difficulty

Configuration:
```
quest_generation: {
    enabled: true
    daily_quest_limit: 5             // Maximum daily quests per player
    quest_complexity: 3              // Quest complexity level (1-5)
    story_continuity_weight: 0.8     // Weight for maintaining story continuity
}
```

### 5. Economic Ecosystem Agent

Implements a dynamic economy with supply and demand mechanics.

Features:
- Dynamic pricing based on supply/demand
- Regional economic systems
- Trade routes and caravans

Configuration:
```
economic_ecosystem: {
    enabled: true
    price_volatility: 0.2            // Market price volatility (0.0-1.0)
    supply_demand_sensitivity: 0.5   // Sensitivity to supply/demand changes
    regional_variance: true          // Enable regional price differences
}
```

### 6. Social Dynamics Agent

Creates dynamic faction relationships and reputation systems.

Features:
- Dynamic alliance formation
- Reputation-based interactions
- Inter-guild politics

Configuration:
```
social_dynamics: {
    enabled: true
    faction_count: 10                // Number of dynamic factions
    reputation_memory: 30            // Days to remember reputation changes
    alliance_volatility: 0.3         // Alliance stability factor (0.0-1.0)
}
```

### 7. Combat Mechanics Agent

Enhances combat with environmental factors and terrain effects.

Features:
- Terrain-based advantages
- Weather effects on combat
- Dynamic cover system

Configuration:
```
combat_mechanics: {
    enabled: true
    terrain_effect_multiplier: 0.2   // Multiplier for terrain effects on combat
    weather_effect_multiplier: 0.15  // Multiplier for weather effects on combat
    cover_system_enabled: true       // Enable dynamic cover system
}
```

### 8. Housing System Agent

Implements a customizable player housing system.

Features:
- Player-owned territories
- Customizable architecture
- Functional workshops

Configuration:
```
housing_system: {
    enabled: true
    max_houses_per_player: 3         // Maximum houses per player
    upgrade_levels: 5                // Number of upgrade levels available
    customization_options: 100       // Number of customization options
}
```

### 9. Time Manipulation Agent

Enables time-based mechanics and historical events.

Features:
- Time-based skills
- Historical event participation
- Time-locked content

Configuration:
```
time_manipulation: {
    enabled: true
    time_skill_cooldown: 3600        // Cooldown for time-based skills in seconds
    historical_events_enabled: true  // Enable historical event participation
    time_locked_content_rotation: 7  // Days between time-locked content rotation
}
```

### 10. NPC Intelligence Agent

Creates more intelligent NPCs with memory and personality.

Features:
- MBTI personality types for NPCs
- Memory of player interactions
- Adaptive conversation systems
- Emotional relationship development

Configuration:
```
npc_intelligence: {
    enabled: true
    memory_retention: 30             // Days to retain player interaction memory
    conversation_depth: 3            // Depth of conversation trees
    personality_types: 16            // Number of MBTI personality types to use
    relationship_development_rate: 0.1 // Rate of relationship development per interaction
}
```

### 11. Guild Evolution Agent

Implements guild specialization and technology systems.

Features:
- Guild specialization paths
- Inter-guild alliances
- Guild technology trees

Configuration:
```
guild_evolution: {
    enabled: true
    specialization_paths: 5          // Number of guild specialization paths
    technology_tree_depth: 4         // Depth of guild technology trees
    alliance_limit: 3                // Maximum number of guild alliances
}
```

### 12. Dimensional Warfare Agent

Creates multi-dimensional battle systems.

Features:
- Cross-dimensional battles
- Reality-warping effects
- Dimensional territory control

Configuration:
```
dimensional_warfare: {
    enabled: true
    dimension_count: 5               // Number of accessible dimensions
    reality_warp_cooldown: 86400     // Cooldown for reality warping in seconds
    plane_shift_cost: 1000           // MP cost for plane shifting
}
```

## Script Commands

The AI system adds the following script commands:

### AI_Request

Process an AI request from a script.

```
AI_Request("<agent_name>", "<request_type>", "<request_data>");
```

Example:
```
// Generate NPC dialogue
set .response$, AI_Request("npc_intelligence", "generate_dialogue", "{\"npc_id\":123,\"char_id\":456,\"context\":\"greeting\"}");
mes .response$;
```

### AI_PlayerEvent

Trigger a player event for AI agents to process.

```
AI_PlayerEvent("<event_name>", <char_id>, "<params>");
```

Example:
```
// Notify AI about player level up
AI_PlayerEvent("level_up", getcharid(0), "{\"old_level\":10,\"new_level\":11}");
```

### AI_WorldEvent

Trigger a world event for AI agents to process.

```
AI_WorldEvent("<event_name>", "<params>");
```

Example:
```
// Start a server event
AI_WorldEvent("server_event_start", "{\"event_name\":\"Christmas\",\"duration\":86400}");
```

## Events

The AI system responds to various events:

### Player Events

- `login` - Player logs in
- `logout` - Player logs out
- `level_up` - Player levels up
- `job_change` - Player changes job
- `skill_learn` - Player learns a skill
- `item_obtain` - Player obtains an item
- `item_use` - Player uses an item
- `kill_monster` - Player kills a monster
- `death` - Player dies
- `quest_accept` - Player accepts a quest
- `quest_complete` - Player completes a quest
- `guild_join` - Player joins a guild
- `marriage` - Player gets married
- `divorce` - Player gets divorced
- `chat` - Player chats

### World Events

- `server_start` - Server starts
- `server_shutdown` - Server shuts down
- `server_event_start` - Server event starts
- `server_event_end` - Server event ends
- `woe_start` - WoE starts
- `woe_end` - WoE ends
- `guild_castle_capture` - Guild captures a castle
- `mvp_spawn` - MVP monster spawns
- `mvp_kill` - MVP monster is killed
- `weather_change` - Weather changes
- `season_change` - Season changes
- `economy_update` - Economy updates
- `faction_war` - Faction war occurs

## Database Schema

The AI system uses the following database tables:

- `ai_world_state` - Tracks global events and their impact
- `ai_player_bloodlines` - Stores player bloodline information
- `ai_player_traits` - Stores player traits
- `ai_skill_fusions` - Stores skill fusion data
- `ai_dynamic_quests` - Stores AI-generated quests
- `ai_economic_data` - Stores economic data
- `ai_factions` - Stores faction information
- `ai_faction_relations` - Stores relationships between factions
- `ai_environmental_conditions` - Stores environmental conditions
- `ai_player_housing` - Stores player housing data
- `ai_time_events` - Stores time-related events
- `ai_npc_memory` - Stores NPC memories
- `ai_npc_personality` - Stores NPC personality data
- `ai_guild_evolution` - Stores guild evolution data
- `ai_dimensional_data` - Stores dimensional warfare data
- `ai_request_cache` - Caches AI responses
- `ai_system_logs` - Logs AI system activity

See `sql-files/ai_agents.sql` for the complete schema.

## Technical Requirements

To use the AI agents system, you need:

- rAthena server with C++17 support
- MySQL/MariaDB database
- libcurl for API requests
- nlohmann/json for JSON parsing
- One of the following AI providers:
  - Azure OpenAI API key
  - OpenAI API key
  - DeepSeek API key
  - Local model files (for fallback)

For optimal performance, we recommend:
- At least 4GB RAM
- Modern CPU with 4+ cores
- Fast internet connection for API calls
- SSD storage for database

## Getting Started

1. Import the database schema:
   ```
   mysql -u username -p ragnarok < sql-files/ai_agents.sql
   ```

2. Configure your AI providers in `conf/ai_agents.conf`

3. Enable the AI agents you want to use

4. Start your server and enjoy the enhanced gameplay!

## Troubleshooting

If you encounter issues:

1. Check the server logs for AI-related errors
2. Verify your API keys and endpoints
3. Ensure your database schema is up to date
4. Check that the AI agents are properly enabled in the configuration
5. Verify network connectivity for API calls

For more help, refer to the rAthena forums or Discord server.