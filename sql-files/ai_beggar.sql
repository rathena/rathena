--
-- Table structure for AI Beggar system
--

--
-- Table structure for Beggar appearance history
--

CREATE TABLE IF NOT EXISTS `ai_beggar_appearances` (
  `appearance_id` int(11) NOT NULL AUTO_INCREMENT,
  `city` varchar(32) NOT NULL,
  `appearance_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `disappearance_time` datetime DEFAULT NULL,
  `interactions` int(11) NOT NULL DEFAULT '0',
  `foods_received` int(11) NOT NULL DEFAULT '0',
  `rewards_given` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`appearance_id`),
  KEY `city` (`city`),
  KEY `appearance_time` (`appearance_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar player interactions
--

CREATE TABLE IF NOT EXISTS `ai_beggar_interactions` (
  `interaction_id` int(11) NOT NULL AUTO_INCREMENT,
  `appearance_id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `interaction_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `food_given` tinyint(1) NOT NULL DEFAULT '0',
  `food_item_id` int(11) DEFAULT NULL,
  `food_amount` int(11) DEFAULT NULL,
  `reward_given` tinyint(1) NOT NULL DEFAULT '0',
  `reward_item_id` int(11) DEFAULT NULL,
  `reward_amount` int(11) DEFAULT NULL,
  `conversation` text,
  PRIMARY KEY (`interaction_id`),
  KEY `appearance_id` (`appearance_id`),
  KEY `char_id` (`char_id`),
  KEY `interaction_time` (`interaction_time`),
  CONSTRAINT `ai_beggar_interactions_ibfk_1` FOREIGN KEY (`appearance_id`) REFERENCES `ai_beggar_appearances` (`appearance_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar player feeding streaks
--

CREATE TABLE IF NOT EXISTS `ai_beggar_feeding_streaks` (
  `streak_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `current_streak` int(11) NOT NULL DEFAULT '0',
  `max_streak` int(11) NOT NULL DEFAULT '0',
  `last_feeding_date` date DEFAULT NULL,
  `feedings_today` int(11) NOT NULL DEFAULT '0',
  `total_feedings` int(11) NOT NULL DEFAULT '0',
  `streak_completed` tinyint(1) NOT NULL DEFAULT '0',
  `streak_completion_date` date DEFAULT NULL,
  `reward_given` tinyint(1) NOT NULL DEFAULT '0',
  `reward_given_date` datetime DEFAULT NULL,
  PRIMARY KEY (`streak_id`),
  UNIQUE KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar player fragment collection
--

CREATE TABLE IF NOT EXISTS `ai_beggar_fragments` (
  `fragment_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `fragments_collected` int(11) NOT NULL DEFAULT '0',
  `last_fragment_time` datetime DEFAULT NULL,
  `event_triggered` tinyint(1) NOT NULL DEFAULT '0',
  `event_trigger_time` datetime DEFAULT NULL,
  `event_completed` tinyint(1) NOT NULL DEFAULT '0',
  `event_completion_time` datetime DEFAULT NULL,
  `skill_fragment_received` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`fragment_id`),
  UNIQUE KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar special events
--

CREATE TABLE IF NOT EXISTS `ai_beggar_events` (
  `event_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `event_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `event_map` varchar(32) NOT NULL,
  `event_x` int(11) NOT NULL,
  `event_y` int(11) NOT NULL,
  `event_duration` int(11) NOT NULL,
  `event_completed` tinyint(1) NOT NULL DEFAULT '0',
  `event_completion_time` datetime DEFAULT NULL,
  `event_result` enum('WIN','LOSE','DRAW','INCOMPLETE') NOT NULL DEFAULT 'INCOMPLETE',
  `skill_fragment_given` tinyint(1) NOT NULL DEFAULT '0',
  `participants` text,
  PRIMARY KEY (`event_id`),
  KEY `char_id` (`char_id`),
  KEY `event_time` (`event_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar memory
--

CREATE TABLE IF NOT EXISTS `ai_beggar_memory` (
  `memory_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `memory_type` varchar(32) NOT NULL,
  `content` text NOT NULL,
  `embedding` text,
  `importance` int(11) NOT NULL DEFAULT '1',
  `is_long_term` tinyint(1) NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_accessed` datetime DEFAULT NULL,
  `expiry_time` datetime DEFAULT NULL,
  PRIMARY KEY (`memory_id`),
  KEY `char_id` (`char_id`),
  KEY `memory_type` (`memory_type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar conversations
--

CREATE TABLE IF NOT EXISTS `ai_beggar_conversations` (
  `conversation_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `message` text NOT NULL,
  `is_player` tinyint(1) NOT NULL DEFAULT '1',
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`conversation_id`),
  KEY `char_id` (`char_id`),
  KEY `created_at` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar logs
--

CREATE TABLE IF NOT EXISTS `ai_beggar_logs` (
  `log_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) DEFAULT NULL,
  `log_type` varchar(32) NOT NULL,
  `message` text NOT NULL,
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`),
  KEY `char_id` (`char_id`),
  KEY `log_type` (`log_type`),
  KEY `created_at` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar statistics
--

CREATE TABLE IF NOT EXISTS `ai_beggar_statistics` (
  `stat_id` int(11) NOT NULL AUTO_INCREMENT,
  `stat_date` date NOT NULL,
  `appearances` int(11) NOT NULL DEFAULT '0',
  `cities_visited` int(11) NOT NULL DEFAULT '0',
  `interactions` int(11) NOT NULL DEFAULT '0',
  `foods_received` int(11) NOT NULL DEFAULT '0',
  `rewards_given` int(11) NOT NULL DEFAULT '0',
  `fragments_collected` int(11) NOT NULL DEFAULT '0',
  `events_triggered` int(11) NOT NULL DEFAULT '0',
  `events_completed` int(11) NOT NULL DEFAULT '0',
  `skill_fragments_given` int(11) NOT NULL DEFAULT '0',
  `streaks_completed` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`stat_id`),
  UNIQUE KEY `stat_date` (`stat_date`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Table structure for Beggar player skill fragments
--

CREATE TABLE IF NOT EXISTS `ai_beggar_skill_fragments` (
  `fragment_id` int(11) NOT NULL AUTO_INCREMENT,
  `char_id` int(11) NOT NULL,
  `skill_id` int(11) NOT NULL,
  `skill_name` varchar(64) NOT NULL,
  `acquisition_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `acquisition_method` enum('STREAK','EVENT') NOT NULL,
  `is_active` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`fragment_id`),
  UNIQUE KEY `char_skill` (`char_id`,`skill_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Initial data for Beggar statistics
--

INSERT IGNORE INTO `ai_beggar_statistics` (`stat_date`, `appearances`, `cities_visited`, `interactions`, `foods_received`, `rewards_given`, `fragments_collected`, `events_triggered`, `events_completed`, `skill_fragments_given`, `streaks_completed`)
VALUES (CURDATE(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);