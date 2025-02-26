--
-- Table structure for AI system configuration
--

CREATE TABLE IF NOT EXISTS `ai_system_config` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `config_key` varchar(64) NOT NULL,
  `config_value` text NOT NULL,
  `last_updated` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `config_key` (`config_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI providers
--

CREATE TABLE IF NOT EXISTS `ai_providers` (
  `provider_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '1',
  `is_primary` tinyint(1) NOT NULL DEFAULT '0',
  `is_fallback` tinyint(1) NOT NULL DEFAULT '0',
  `model` varchar(64) NOT NULL,
  `api_key` varchar(255) NOT NULL,
  `endpoint` varchar(255) DEFAULT NULL,
  `config` text NOT NULL,
  `last_used` datetime DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`provider_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI agents
--

CREATE TABLE IF NOT EXISTS `ai_agents` (
  `agent_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '1',
  `description` text NOT NULL,
  `config` text NOT NULL,
  `last_update` datetime DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`agent_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI requests
--

CREATE TABLE IF NOT EXISTS `ai_requests` (
  `request_id` int(11) NOT NULL AUTO_INCREMENT,
  `agent_id` int(11) NOT NULL,
  `provider_id` int(11) NOT NULL,
  `request_type` varchar(64) NOT NULL,
  `request_data` text NOT NULL,
  `response_data` text NOT NULL,
  `tokens` int(11) NOT NULL DEFAULT '0',
  `response_time` int(11) NOT NULL DEFAULT '0',
  `cost` decimal(10,6) NOT NULL DEFAULT '0.000000',
  `success` tinyint(1) NOT NULL DEFAULT '1',
  `error_message` text,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`request_id`),
  KEY `agent_id` (`agent_id`),
  KEY `provider_id` (`provider_id`),
  CONSTRAINT `ai_requests_ibfk_1` FOREIGN KEY (`agent_id`) REFERENCES `ai_agents` (`agent_id`) ON DELETE CASCADE,
  CONSTRAINT `ai_requests_ibfk_2` FOREIGN KEY (`provider_id`) REFERENCES `ai_providers` (`provider_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI request cache
--

CREATE TABLE IF NOT EXISTS `ai_request_cache` (
  `cache_id` int(11) NOT NULL AUTO_INCREMENT,
  `request_hash` varchar(64) NOT NULL,
  `response_data` text NOT NULL,
  `tokens` int(11) NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `expires_at` datetime NOT NULL,
  PRIMARY KEY (`cache_id`),
  UNIQUE KEY `request_hash` (`request_hash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for AI system logs
--

CREATE TABLE IF NOT EXISTS `ai_system_logs` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `level` enum('ERROR','WARNING','INFO','DEBUG','TRACE') NOT NULL,
  `component` varchar(64) NOT NULL,
  `message` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `level` (`level`),
  KEY `component` (`component`),
  KEY `created_at` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for world state
--

CREATE TABLE IF NOT EXISTS `ai_world_state` (
  `state_id` int(11) NOT NULL AUTO_INCREMENT,
  `map_id` int(11) NOT NULL,
  `weather` varchar(32) NOT NULL DEFAULT 'clear',
  `season` varchar(32) NOT NULL DEFAULT 'spring',
  `territory_control` int(11) DEFAULT NULL,
  `resource_state` text,
  `event_state` text,
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`state_id`),
  UNIQUE KEY `map_id` (`map_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player bloodlines
--

CREATE TABLE IF NOT EXISTS `ai_player_bloodlines` (
  `bloodline_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `parent_id` int(11) DEFAULT NULL,
  `generation` int(11) NOT NULL DEFAULT '1',
  `legendary_status` tinyint(1) NOT NULL DEFAULT '0',
  `power_level` int(11) NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`bloodline_id`),
  UNIQUE KEY `char_id` (`char_id`),
  KEY `parent_id` (`parent_id`),
  CONSTRAINT `ai_player_bloodlines_ibfk_1` FOREIGN KEY (`parent_id`) REFERENCES `ai_player_bloodlines` (`bloodline_id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for bloodline traits
--

CREATE TABLE IF NOT EXISTS `ai_bloodline_traits` (
  `trait_id` int(11) NOT NULL AUTO_INCREMENT,
  `bloodline_id` int(11) NOT NULL,
  `trait_name` varchar(64) NOT NULL,
  `trait_description` text NOT NULL,
  `trait_effect` text NOT NULL,
  `trait_power` int(11) NOT NULL DEFAULT '1',
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`trait_id`),
  KEY `bloodline_id` (`bloodline_id`),
  CONSTRAINT `ai_bloodline_traits_ibfk_1` FOREIGN KEY (`bloodline_id`) REFERENCES `ai_player_bloodlines` (`bloodline_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for skill fusions
--

CREATE TABLE IF NOT EXISTS `ai_skill_fusions` (
  `fusion_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `base_skill_id` int(11) NOT NULL,
  `component_skill_ids` varchar(255) NOT NULL,
  `fusion_name` varchar(64) NOT NULL,
  `fusion_description` text NOT NULL,
  `fusion_effect` text NOT NULL,
  `fusion_formula` text NOT NULL,
  `stability_factor` float NOT NULL DEFAULT '1.0',
  `is_experimental` tinyint(1) NOT NULL DEFAULT '0',
  `cooldown_end` datetime DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`fusion_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for dynamic quests
--

CREATE TABLE IF NOT EXISTS `ai_dynamic_quests` (
  `quest_id` int(11) NOT NULL AUTO_INCREMENT,
  `title` varchar(64) NOT NULL,
  `description` text NOT NULL,
  `objectives` text NOT NULL,
  `rewards` text NOT NULL,
  `difficulty` int(11) NOT NULL DEFAULT '1',
  `min_level` int(11) NOT NULL DEFAULT '1',
  `max_level` int(11) NOT NULL DEFAULT '999',
  `is_group_quest` tinyint(1) NOT NULL DEFAULT '0',
  `min_group_size` int(11) NOT NULL DEFAULT '1',
  `max_group_size` int(11) NOT NULL DEFAULT '12',
  `story_arc_id` int(11) DEFAULT NULL,
  `expiry_date` datetime DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`quest_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player quest progress
--

CREATE TABLE IF NOT EXISTS `ai_player_quests` (
  `progress_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `quest_id` int(11) NOT NULL,
  `status` enum('AVAILABLE','ACCEPTED','IN_PROGRESS','COMPLETED','FAILED') NOT NULL DEFAULT 'AVAILABLE',
  `progress_data` text,
  `start_time` datetime DEFAULT NULL,
  `completion_time` datetime DEFAULT NULL,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`progress_id`),
  UNIQUE KEY `char_quest` (`char_id`,`quest_id`),
  KEY `quest_id` (`quest_id`),
  CONSTRAINT `ai_player_quests_ibfk_1` FOREIGN KEY (`quest_id`) REFERENCES `ai_dynamic_quests` (`quest_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for economic data
--

CREATE TABLE IF NOT EXISTS `ai_economic_data` (
  `economy_id` int(11) NOT NULL AUTO_INCREMENT,
  `item_id` int(11) NOT NULL,
  `region_id` int(11) NOT NULL DEFAULT '0',
  `base_price` int(11) NOT NULL,
  `current_price` int(11) NOT NULL,
  `supply` int(11) NOT NULL DEFAULT '100',
  `demand` int(11) NOT NULL DEFAULT '100',
  `price_history` text,
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`economy_id`),
  UNIQUE KEY `item_region` (`item_id`,`region_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for trade routes
--

CREATE TABLE IF NOT EXISTS `ai_trade_routes` (
  `route_id` int(11) NOT NULL AUTO_INCREMENT,
  `start_region` int(11) NOT NULL,
  `end_region` int(11) NOT NULL,
  `item_id` int(11) NOT NULL,
  `profit_margin` float NOT NULL DEFAULT '0.0',
  `risk_level` int(11) NOT NULL DEFAULT '1',
  `travel_time` int(11) NOT NULL DEFAULT '3600',
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`route_id`),
  UNIQUE KEY `route_item` (`start_region`,`end_region`,`item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for factions
--

CREATE TABLE IF NOT EXISTS `ai_factions` (
  `faction_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `description` text NOT NULL,
  `leader_name` varchar(64) DEFAULT NULL,
  `power_level` int(11) NOT NULL DEFAULT '100',
  `territory` text,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`faction_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for faction relations
--

CREATE TABLE IF NOT EXISTS `ai_faction_relations` (
  `relation_id` int(11) NOT NULL AUTO_INCREMENT,
  `faction_id1` int(11) NOT NULL,
  `faction_id2` int(11) NOT NULL,
  `relation_type` enum('ALLIED','FRIENDLY','NEUTRAL','UNFRIENDLY','HOSTILE','WAR') NOT NULL DEFAULT 'NEUTRAL',
  `relation_value` int(11) NOT NULL DEFAULT '0',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`relation_id`),
  UNIQUE KEY `faction_pair` (`faction_id1`,`faction_id2`),
  KEY `faction_id2` (`faction_id2`),
  CONSTRAINT `ai_faction_relations_ibfk_1` FOREIGN KEY (`faction_id1`) REFERENCES `ai_factions` (`faction_id`) ON DELETE CASCADE,
  CONSTRAINT `ai_faction_relations_ibfk_2` FOREIGN KEY (`faction_id2`) REFERENCES `ai_factions` (`faction_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player faction reputation
--

CREATE TABLE IF NOT EXISTS `ai_player_reputation` (
  `reputation_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `faction_id` int(11) NOT NULL,
  `reputation_value` int(11) NOT NULL DEFAULT '0',
  `reputation_level` varchar(32) NOT NULL DEFAULT 'Neutral',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`reputation_id`),
  UNIQUE KEY `char_faction` (`char_id`,`faction_id`),
  KEY `faction_id` (`faction_id`),
  CONSTRAINT `ai_player_reputation_ibfk_1` FOREIGN KEY (`faction_id`) REFERENCES `ai_factions` (`faction_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for combat environment effects
--

CREATE TABLE IF NOT EXISTS `ai_combat_effects` (
  `effect_id` int(11) NOT NULL AUTO_INCREMENT,
  `map_id` int(11) NOT NULL,
  `terrain_type` varchar(32) NOT NULL,
  `weather_type` varchar(32) NOT NULL,
  `time_of_day` varchar(32) NOT NULL,
  `effect_type` varchar(32) NOT NULL,
  `effect_value` float NOT NULL DEFAULT '0.0',
  `effect_description` text NOT NULL,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`effect_id`),
  UNIQUE KEY `map_terrain_weather_time` (`map_id`,`terrain_type`,`weather_type`,`time_of_day`,`effect_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player housing
--

CREATE TABLE IF NOT EXISTS `ai_player_housing` (
  `house_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `house_name` varchar(64) NOT NULL,
  `house_type` varchar(32) NOT NULL DEFAULT 'small',
  `location` varchar(64) NOT NULL,
  `size` int(11) NOT NULL DEFAULT '1',
  `upgrade_level` int(11) NOT NULL DEFAULT '1',
  `customization_data` text,
  `furniture_data` text,
  `storage_capacity` int(11) NOT NULL DEFAULT '100',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`house_id`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for time events
--

CREATE TABLE IF NOT EXISTS `ai_time_events` (
  `event_id` int(11) NOT NULL AUTO_INCREMENT,
  `event_name` varchar(64) NOT NULL,
  `event_description` text NOT NULL,
  `event_type` varchar(32) NOT NULL,
  `start_time` datetime NOT NULL,
  `end_time` datetime NOT NULL,
  `event_data` text,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`event_id`),
  KEY `start_time` (`start_time`),
  KEY `end_time` (`end_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for NPC personalities
--

CREATE TABLE IF NOT EXISTS `ai_npc_personality` (
  `personality_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) NOT NULL,
  `mbti_type` varchar(4) NOT NULL,
  `extroversion` int(11) NOT NULL DEFAULT '50',
  `sensing` int(11) NOT NULL DEFAULT '50',
  `thinking` int(11) NOT NULL DEFAULT '50',
  `judging` int(11) NOT NULL DEFAULT '50',
  `mood_baseline` int(11) NOT NULL DEFAULT '0',
  `mood_volatility` int(11) NOT NULL DEFAULT '50',
  `interests` text,
  `dislikes` text,
  `conversation_style` text,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`personality_id`),
  UNIQUE KEY `npc_id` (`npc_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for NPC memories
--

CREATE TABLE IF NOT EXISTS `ai_npc_memory` (
  `memory_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `memory_type` varchar(32) NOT NULL,
  `content` text NOT NULL,
  `emotional_impact` int(11) NOT NULL DEFAULT '0',
  `importance` int(11) NOT NULL DEFAULT '1',
  `is_long_term` tinyint(1) NOT NULL DEFAULT '0',
  `decay_rate` float NOT NULL DEFAULT '0.1',
  `created_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_recalled_time` datetime DEFAULT NULL,
  `expiry_time` datetime DEFAULT NULL,
  PRIMARY KEY (`memory_id`),
  KEY `npc_id` (`npc_id`),
  KEY `char_id` (`char_id`),
  KEY `memory_type` (`memory_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for NPC relationships
--

CREATE TABLE IF NOT EXISTS `ai_npc_relationships` (
  `relationship_id` int(11) NOT NULL AUTO_INCREMENT,
  `npc_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `relationship_value` int(11) NOT NULL DEFAULT '0',
  `relationship_type` varchar(32) NOT NULL DEFAULT 'Neutral',
  `last_interaction` datetime DEFAULT NULL,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`relationship_id`),
  UNIQUE KEY `npc_char` (`npc_id`,`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for guild evolution
--

CREATE TABLE IF NOT EXISTS `ai_guild_evolution` (
  `evolution_id` int(11) NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) NOT NULL,
  `specialization_path` varchar(32) DEFAULT NULL,
  `technology_level` int(11) NOT NULL DEFAULT '1',
  `research_points` int(11) NOT NULL DEFAULT '0',
  `territory_data` text,
  `last_update` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`evolution_id`),
  UNIQUE KEY `guild_id` (`guild_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for guild technologies
--

CREATE TABLE IF NOT EXISTS `ai_guild_technologies` (
  `technology_id` int(11) NOT NULL AUTO_INCREMENT,
  `guild_id` int(11) NOT NULL,
  `tech_name` varchar(64) NOT NULL,
  `tech_description` text NOT NULL,
  `tech_level` int(11) NOT NULL DEFAULT '1',
  `tech_effect` text NOT NULL,
  `research_cost` int(11) NOT NULL DEFAULT '100',
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `unlock_time` datetime DEFAULT NULL,
  PRIMARY KEY (`technology_id`),
  UNIQUE KEY `guild_tech` (`guild_id`,`tech_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for dimensional data
--

CREATE TABLE IF NOT EXISTS `ai_dimensional_data` (
  `dimension_id` int(11) NOT NULL AUTO_INCREMENT,
  `dimension_name` varchar(64) NOT NULL,
  `dimension_description` text NOT NULL,
  `stability` float NOT NULL DEFAULT '1.0',
  `access_level` int(11) NOT NULL DEFAULT '1',
  `entry_points` text,
  `special_effects` text,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`dimension_id`),
  UNIQUE KEY `dimension_name` (`dimension_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player dimensional access
--

CREATE TABLE IF NOT EXISTS `ai_player_dimensions` (
  `access_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `dimension_id` int(11) NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '1',
  `last_visit` datetime DEFAULT NULL,
  `visit_count` int(11) NOT NULL DEFAULT '0',
  `cooldown_end` datetime DEFAULT NULL,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`access_id`),
  UNIQUE KEY `char_dimension` (`char_id`,`dimension_id`),
  KEY `dimension_id` (`dimension_id`),
  CONSTRAINT `ai_player_dimensions_ibfk_1` FOREIGN KEY (`dimension_id`) REFERENCES `ai_dimensional_data` (`dimension_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Initial data for AI agents
--

INSERT IGNORE INTO `ai_agents` (`name`, `enabled`, `description`, `config`) VALUES
('world_evolution', 1, 'Dynamic world evolution with weather, seasons, and environments that adapt to player actions', '{}'),
('legend_bloodlines', 1, 'Player legacy system with inherited traits and abilities', '{}'),
('cross_class_synthesis', 1, 'Skill fusion and hybrid abilities from different classes', '{}'),
('quest_generation', 1, 'AI-driven quests and storylines tailored to player progress', '{}'),
('economic_ecosystem', 1, 'Dynamic economy with supply and demand mechanics', '{}'),
('social_dynamics', 1, 'Dynamic faction relationships and reputation systems', '{}'),
('combat_mechanics', 1, 'Environmental combat factors and terrain effects', '{}'),
('housing_system', 1, 'Customizable player housing system', '{}'),
('time_manipulation', 1, 'Time-based mechanics and historical events', '{}'),
('npc_intelligence', 1, 'Enhanced NPCs with MBTI personalities and memory', '{}'),
('guild_evolution', 1, 'Guild specialization and technology systems', '{}'),
('dimensional_warfare', 1, 'Multi-dimensional battle systems', '{}');

--
-- Initial data for AI providers
--

INSERT IGNORE INTO `ai_providers` (`name`, `enabled`, `is_primary`, `is_fallback`, `model`, `api_key`, `endpoint`, `config`) VALUES
('azure_openai', 1, 1, 0, 'gpt-4o', 'your_api_key_here', 'https://your-resource.openai.azure.com/', '{}'),
('openai', 1, 0, 0, 'gpt-4o', 'your_api_key_here', NULL, '{}'),
('deepseek', 1, 0, 0, 'deepseek-v3', 'your_api_key_here', NULL, '{}'),
('local', 1, 0, 1, 'llama3', '', NULL, '{}');

--
-- Initial data for AI system configuration
--

INSERT IGNORE INTO `ai_system_config` (`config_key`, `config_value`) VALUES
('ai_enabled', 'true'),
('ai_log_level', '3'),
('ai_cache_enabled', 'true'),
('ai_cache_ttl', '86400'),
('ai_primary_provider', 'azure_openai'),
('ai_fallback_provider', 'local');