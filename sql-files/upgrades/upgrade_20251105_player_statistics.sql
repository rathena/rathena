-- Player Statistics System
-- Upgrade Date: 2025-11-05
-- Description: Add player statistics tracking tables for monitoring player activities

-- ============================================================================
-- Table: player_statistics
-- Description: Main table for tracking cumulative player statistics
-- ============================================================================
CREATE TABLE IF NOT EXISTS `player_statistics` (
  `char_id` int(11) unsigned NOT NULL,
  `account_id` int(11) unsigned NOT NULL,

  -- Online Time Tracking
  `online_time` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total online time in seconds',
  `session_start` datetime DEFAULT NULL COMMENT 'Current session start time',
  `last_login` datetime DEFAULT NULL COMMENT 'Last login timestamp',
  `login_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Total number of logins',

  -- Item Usage Statistics
  `item_used_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total items used/consumed',
  `item_used_healing` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Healing items used',
  `item_used_buff` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Buff items used',
  `item_used_other` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Other items used',

  -- Movement & Teleport Statistics
  `teleport_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Teleport skill uses',
  `warp_portal_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Warp portal uses',
  `butterfly_wing_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Butterfly Wing uses',
  `movement_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total cell movements',
  `movement_distance` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total distance in cells',

  -- Healing & SP Recovery Statistics
  `heal_skill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Heal skill uses',
  `heal_amount_total` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total HP healed',
  `sp_recovery_skill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'SP recovery skill uses',
  `sp_recovery_amount` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total SP recovered',

  -- Item Pickup Statistics
  `item_picked_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total items picked up',
  `item_picked_from_mob` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items from monsters',
  `item_picked_from_ground` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items from ground',
  `zeny_picked` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total zeny picked',

  -- Monster Kill Statistics
  `mob_kill_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total monsters killed',
  `mvp_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'MVP monsters killed',
  `boss_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Boss monsters killed',
  `mini_boss_kill_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Mini-boss monsters killed',

  -- Skill Usage Statistics
  `skill_used_count` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total skills used',
  `skill_offensive_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Offensive skills',
  `skill_support_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Support skills',
  `skill_passive_triggered` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Passive skill triggers',

  -- Additional Statistics
  `death_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Total deaths',
  `damage_dealt` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total damage dealt',
  `damage_received` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Total damage received',
  `chat_message_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Chat messages sent',

  -- Timestamps
  `created_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (`char_id`),
  KEY `account_id` (`account_id`),
  KEY `online_time` (`online_time`),
  KEY `mob_kill_count` (`mob_kill_count`),
  KEY `updated_at` (`updated_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
-- Table: player_statistics_daily
-- Description: Daily aggregated statistics for trend analysis and rankings
-- ============================================================================
CREATE TABLE IF NOT EXISTS `player_statistics_daily` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `char_id` int(11) unsigned NOT NULL,
  `stat_date` date NOT NULL COMMENT 'Statistics date',

  -- Daily Statistics
  `online_time` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Daily online time in seconds',
  `item_used` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items used today',
  `teleport_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Teleports today',
  `movement_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Movements today',
  `heal_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Heals today',
  `item_picked` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Items picked today',
  `mob_kill` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Monsters killed today',
  `skill_used` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Skills used today',
  `exp_gained` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Experience gained today',
  `zeny_gained` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'Zeny gained today',
  `death_count` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'Deaths today',

  PRIMARY KEY (`id`),
  UNIQUE KEY `char_date` (`char_id`, `stat_date`),
  KEY `stat_date` (`stat_date`),
  KEY `char_id` (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
-- Table: player_statistics_detail
-- Description: Detailed statistics stored as JSON for advanced analytics
-- ============================================================================
CREATE TABLE IF NOT EXISTS `player_statistics_detail` (
  `char_id` int(11) unsigned NOT NULL,

  -- Top Items/Skills/Mobs (JSON format)
  `top_skills_used` TEXT DEFAULT NULL COMMENT 'JSON: {skill_id: count} - Top skills by usage',
  `top_items_used` TEXT DEFAULT NULL COMMENT 'JSON: {item_id: count} - Top items by usage',
  `top_mobs_killed` TEXT DEFAULT NULL COMMENT 'JSON: {mob_id: count} - Top monsters by kills',

  -- Map Statistics (JSON format)
  `map_visit_count` TEXT DEFAULT NULL COMMENT 'JSON: {map_name: count} - Map visit counts',
  `map_time_spent` TEXT DEFAULT NULL COMMENT 'JSON: {map_name: seconds} - Time spent per map',

  -- Session History (JSON array)
  `session_history` TEXT DEFAULT NULL COMMENT 'JSON: [{start, end, duration}] - Last 30 sessions',

  -- Hourly Activity Pattern (JSON array)
  `hourly_activity` TEXT DEFAULT NULL COMMENT 'JSON: [0-23] - Activity count by hour',

  -- Last Update
  `updated_at` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (`char_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
-- Insert default records for existing characters
-- ============================================================================
INSERT INTO `player_statistics` (`char_id`, `account_id`)
SELECT `char_id`, `account_id` FROM `char`
WHERE NOT EXISTS (
  SELECT 1 FROM `player_statistics` WHERE `player_statistics`.`char_id` = `char`.`char_id`
);

-- ============================================================================
-- End of upgrade script
-- ============================================================================
