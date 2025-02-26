--
-- Table structure for Mayor Agent system
--

--
-- Table structure for server statistics
--

CREATE TABLE IF NOT EXISTS `mayor_server_statistics` (
  `stat_id` int(11) NOT NULL AUTO_INCREMENT,
  `start_date` date NOT NULL,
  `end_date` date NOT NULL,
  `total_players` int(11) NOT NULL DEFAULT '0',
  `new_players` int(11) NOT NULL DEFAULT '0',
  `active_players` int(11) NOT NULL DEFAULT '0',
  `returning_players` int(11) NOT NULL DEFAULT '0',
  `inactive_players` int(11) NOT NULL DEFAULT '0',
  `retention_rate` float NOT NULL DEFAULT '0',
  `churn_rate` float NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`stat_id`),
  UNIQUE KEY `date_range` (`start_date`, `end_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player distribution by level
--

CREATE TABLE IF NOT EXISTS `mayor_player_distribution_level` (
  `distribution_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `level_range` varchar(32) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`distribution_id`),
  UNIQUE KEY `stat_level` (`stat_id`, `level_range`),
  CONSTRAINT `mayor_player_distribution_level_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player distribution by class
--

CREATE TABLE IF NOT EXISTS `mayor_player_distribution_class` (
  `distribution_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `class_name` varchar(32) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`distribution_id`),
  UNIQUE KEY `stat_class` (`stat_id`, `class_name`),
  CONSTRAINT `mayor_player_distribution_class_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player distribution by map
--

CREATE TABLE IF NOT EXISTS `mayor_player_distribution_map` (
  `distribution_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `map_name` varchar(32) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`distribution_id`),
  UNIQUE KEY `stat_map` (`stat_id`, `map_name`),
  CONSTRAINT `mayor_player_distribution_map_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player activity by hour
--

CREATE TABLE IF NOT EXISTS `mayor_player_activity_hour` (
  `activity_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `hour` int(11) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`activity_id`),
  UNIQUE KEY `stat_hour` (`stat_id`, `hour`),
  CONSTRAINT `mayor_player_activity_hour_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player activity by day
--

CREATE TABLE IF NOT EXISTS `mayor_player_activity_day` (
  `activity_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `day` varchar(10) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`activity_id`),
  UNIQUE KEY `stat_day` (`stat_id`, `day`),
  CONSTRAINT `mayor_player_activity_day_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for popular activities
--

CREATE TABLE IF NOT EXISTS `mayor_popular_activities` (
  `activity_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `activity_name` varchar(64) NOT NULL,
  `activity_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`activity_id`),
  UNIQUE KEY `stat_activity` (`stat_id`, `activity_name`),
  CONSTRAINT `mayor_popular_activities_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for item transactions
--

CREATE TABLE IF NOT EXISTS `mayor_item_transactions` (
  `transaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `item_id` int(11) NOT NULL,
  `transaction_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`transaction_id`),
  UNIQUE KEY `stat_item` (`stat_id`, `item_id`),
  CONSTRAINT `mayor_item_transactions_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for zeny flow
--

CREATE TABLE IF NOT EXISTS `mayor_zeny_flow` (
  `flow_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `source` varchar(64) NOT NULL,
  `amount` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`flow_id`),
  UNIQUE KEY `stat_source` (`stat_id`, `source`),
  CONSTRAINT `mayor_zeny_flow_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for MVP kills
--

CREATE TABLE IF NOT EXISTS `mayor_mvp_kills` (
  `kill_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `mvp_id` int(11) NOT NULL,
  `kill_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`kill_id`),
  UNIQUE KEY `stat_mvp` (`stat_id`, `mvp_id`),
  CONSTRAINT `mayor_mvp_kills_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for quest completions
--

CREATE TABLE IF NOT EXISTS `mayor_quest_completions` (
  `completion_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `quest_id` int(11) NOT NULL,
  `completion_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`completion_id`),
  UNIQUE KEY `stat_quest` (`stat_id`, `quest_id`),
  CONSTRAINT `mayor_quest_completions_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for event participation
--

CREATE TABLE IF NOT EXISTS `mayor_event_participation` (
  `participation_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `event_id` int(11) NOT NULL,
  `participation_count` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`participation_id`),
  UNIQUE KEY `stat_event` (`stat_id`, `event_id`),
  CONSTRAINT `mayor_event_participation_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for event satisfaction
--

CREATE TABLE IF NOT EXISTS `mayor_event_satisfaction` (
  `satisfaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `event_id` int(11) NOT NULL,
  `satisfaction_rating` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`satisfaction_id`),
  UNIQUE KEY `stat_event` (`stat_id`, `event_id`),
  CONSTRAINT `mayor_event_satisfaction_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for events
--

CREATE TABLE IF NOT EXISTS `mayor_events` (
  `event_id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `description` text NOT NULL,
  `type` enum('COMBAT','COLLECTION','EXPLORATION','SOCIAL','CRAFTING','TRADING','SPECIAL','SEASONAL') NOT NULL,
  `difficulty` enum('BEGINNER','INTERMEDIATE','ADVANCED','EXPERT','MIXED') NOT NULL,
  `target` enum('NEW_PLAYERS','CASUAL_PLAYERS','HARDCORE_PLAYERS','RETURNING_PLAYERS','ALL_PLAYERS') NOT NULL,
  `start_date` date NOT NULL,
  `end_date` date NOT NULL,
  `script_path` varchar(255) NOT NULL,
  `is_active` tinyint(1) NOT NULL DEFAULT '0',
  `is_recurring` tinyint(1) NOT NULL DEFAULT '0',
  `recurrence_pattern` varchar(32) DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`event_id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for event rewards
--

CREATE TABLE IF NOT EXISTS `mayor_event_rewards` (
  `reward_id` int(11) NOT NULL AUTO_INCREMENT,
  `event_id` int(11) NOT NULL,
  `reward_description` varchar(255) NOT NULL,
  PRIMARY KEY (`reward_id`),
  KEY `event_id` (`event_id`),
  CONSTRAINT `mayor_event_rewards_ibfk_1` FOREIGN KEY (`event_id`) REFERENCES `mayor_events` (`event_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for event custom parameters
--

CREATE TABLE IF NOT EXISTS `mayor_event_parameters` (
  `parameter_id` int(11) NOT NULL AUTO_INCREMENT,
  `event_id` int(11) NOT NULL,
  `parameter_name` varchar(64) NOT NULL,
  `parameter_value` varchar(255) NOT NULL,
  PRIMARY KEY (`parameter_id`),
  UNIQUE KEY `event_parameter` (`event_id`, `parameter_name`),
  CONSTRAINT `mayor_event_parameters_ibfk_1` FOREIGN KEY (`event_id`) REFERENCES `mayor_events` (`event_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for mayor analysis
--

CREATE TABLE IF NOT EXISTS `mayor_analysis` (
  `analysis_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `analysis_type` varchar(32) NOT NULL,
  `analysis_result` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`analysis_id`),
  KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_analysis_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player segments
--

CREATE TABLE IF NOT EXISTS `mayor_player_segments` (
  `segment_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `segment_name` varchar(64) NOT NULL,
  `player_count` int(11) NOT NULL DEFAULT '0',
  `segment_description` text,
  PRIMARY KEY (`segment_id`),
  UNIQUE KEY `stat_segment` (`stat_id`, `segment_name`),
  CONSTRAINT `mayor_player_segments_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for trends
--

CREATE TABLE IF NOT EXISTS `mayor_trends` (
  `trend_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `trend_name` varchar(64) NOT NULL,
  `trend_value` float NOT NULL DEFAULT '0',
  `trend_description` text,
  PRIMARY KEY (`trend_id`),
  UNIQUE KEY `stat_trend` (`stat_id`, `trend_name`),
  CONSTRAINT `mayor_trends_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for issues
--

CREATE TABLE IF NOT EXISTS `mayor_issues` (
  `issue_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `issue_description` text NOT NULL,
  `severity` enum('LOW','MEDIUM','HIGH','CRITICAL') NOT NULL,
  `status` enum('IDENTIFIED','IN_PROGRESS','RESOLVED','IGNORED') NOT NULL DEFAULT 'IDENTIFIED',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`issue_id`),
  KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_issues_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for opportunities
--

CREATE TABLE IF NOT EXISTS `mayor_opportunities` (
  `opportunity_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `opportunity_description` text NOT NULL,
  `potential` enum('LOW','MEDIUM','HIGH','VERY_HIGH') NOT NULL,
  `status` enum('IDENTIFIED','IN_PROGRESS','IMPLEMENTED','IGNORED') NOT NULL DEFAULT 'IDENTIFIED',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`opportunity_id`),
  KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_opportunities_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player retention strategies
--

CREATE TABLE IF NOT EXISTS `mayor_retention_strategies` (
  `strategy_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `strategy_description` text NOT NULL,
  `target_segment` varchar(64) DEFAULT NULL,
  `expected_impact` enum('LOW','MEDIUM','HIGH','VERY_HIGH') NOT NULL,
  `status` enum('PROPOSED','APPROVED','IMPLEMENTED','REJECTED') NOT NULL DEFAULT 'PROPOSED',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`strategy_id`),
  KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_retention_strategies_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for player acquisition strategies
--

CREATE TABLE IF NOT EXISTS `mayor_acquisition_strategies` (
  `strategy_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `strategy_description` text NOT NULL,
  `target_audience` varchar(64) DEFAULT NULL,
  `expected_impact` enum('LOW','MEDIUM','HIGH','VERY_HIGH') NOT NULL,
  `status` enum('PROPOSED','APPROVED','IMPLEMENTED','REJECTED') NOT NULL DEFAULT 'PROPOSED',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`strategy_id`),
  KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_acquisition_strategies_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for weekly reports
--

CREATE TABLE IF NOT EXISTS `mayor_weekly_reports` (
  `report_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_id` int(11) NOT NULL,
  `report_content` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`report_id`),
  UNIQUE KEY `stat_id` (`stat_id`),
  CONSTRAINT `mayor_weekly_reports_ibfk_1` FOREIGN KEY (`stat_id`) REFERENCES `mayor_server_statistics` (`stat_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for mayor logs
--

CREATE TABLE IF NOT EXISTS `mayor_logs` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `activity_type` varchar(32) NOT NULL,
  `message` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `activity_type` (`activity_type`),
  KEY `created_at` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for mayor memory
--

CREATE TABLE IF NOT EXISTS `mayor_memory` (
  `memory_id` int(11) NOT NULL AUTO_INCREMENT,
  `memory_type` varchar(32) NOT NULL,
  `content` text NOT NULL,
  `embedding` text,
  `importance` int(11) NOT NULL DEFAULT '1',
  `is_long_term` tinyint(1) NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_accessed` datetime DEFAULT NULL,
  `expiry_time` datetime DEFAULT NULL,
  PRIMARY KEY (`memory_id`),
  KEY `memory_type` (`memory_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Initial data for server statistics
--

INSERT IGNORE INTO `mayor_server_statistics` (`start_date`, `end_date`, `total_players`, `new_players`, `active_players`, `returning_players`, `inactive_players`, `retention_rate`, `churn_rate`)
VALUES (DATE_SUB(CURDATE(), INTERVAL 7 DAY), CURDATE(), 0, 0, 0, 0, 0, 0, 0);