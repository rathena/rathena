--
-- Analytics Platform Database Schema
--

-- Player behavior tracking (extensive)
CREATE TABLE IF NOT EXISTS `analytics_player_behavior` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `account_id` int(11) UNSIGNED NOT NULL,
    `char_id` int(11) UNSIGNED NOT NULL,
    `action_type` varchar(32) NOT NULL,
    `map_name` varchar(32) NOT NULL,
    `x` smallint(5) UNSIGNED NOT NULL,
    `y` smallint(5) UNSIGNED NOT NULL,
    `level` smallint(5) UNSIGNED NOT NULL,
    `zeny` int(11) UNSIGNED NOT NULL,
    `party_id` int(11) UNSIGNED DEFAULT NULL,
    `guild_id` int(11) UNSIGNED DEFAULT NULL,
    `target_type` varchar(32) DEFAULT NULL,
    `target_id` int(11) UNSIGNED DEFAULT NULL,
    `skill_id` smallint(5) UNSIGNED DEFAULT NULL,
    `item_id` int(11) UNSIGNED DEFAULT NULL,
    `amount` int(11) UNSIGNED DEFAULT NULL,
    `duration` int(11) UNSIGNED DEFAULT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `additional_data` json DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `account_id` (`account_id`),
    KEY `char_id` (`char_id`),
    KEY `action_type` (`action_type`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Economy monitoring (extensive)
CREATE TABLE IF NOT EXISTS `analytics_economy` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `transaction_type` varchar(32) NOT NULL,
    `item_id` int(11) UNSIGNED NOT NULL,
    `amount` int(11) UNSIGNED NOT NULL,
    `price` bigint(20) UNSIGNED NOT NULL,
    `seller_account_id` int(11) UNSIGNED DEFAULT NULL,
    `seller_char_id` int(11) UNSIGNED DEFAULT NULL,
    `buyer_account_id` int(11) UNSIGNED DEFAULT NULL,
    `buyer_char_id` int(11) UNSIGNED DEFAULT NULL,
    `map_name` varchar(32) NOT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `market_tax` int(11) UNSIGNED DEFAULT NULL,
    `currency_type` varchar(32) NOT NULL DEFAULT 'zeny',
    `additional_data` json DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `item_id` (`item_id`),
    KEY `transaction_type` (`transaction_type`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Resource utilization (minimal)
CREATE TABLE IF NOT EXISTS `analytics_resource` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `resource_type` varchar(32) NOT NULL,
    `usage_value` float NOT NULL,
    `threshold` float NOT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `resource_type` (`resource_type`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Performance metrics (minimal)
CREATE TABLE IF NOT EXISTS `analytics_performance` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `metric_type` varchar(32) NOT NULL,
    `value` float NOT NULL,
    `map_name` varchar(32) DEFAULT NULL,
    `instance_id` int(11) UNSIGNED DEFAULT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `metric_type` (`metric_type`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Security incidents (minimal)
CREATE TABLE IF NOT EXISTS `analytics_security` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `incident_type` varchar(32) NOT NULL,
    `severity` tinyint(3) UNSIGNED NOT NULL,
    `account_id` int(11) UNSIGNED DEFAULT NULL,
    `char_id` int(11) UNSIGNED DEFAULT NULL,
    `ip_address` varchar(45) DEFAULT NULL,
    `description` text NOT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `resolved` boolean NOT NULL DEFAULT FALSE,
    `resolution_time` datetime DEFAULT NULL,
    `additional_data` json DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `incident_type` (`incident_type`),
    KEY `severity` (`severity`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Network analysis (minimal)
CREATE TABLE IF NOT EXISTS `analytics_network` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `connection_type` varchar(32) NOT NULL,
    `latency` float NOT NULL,
    `packet_loss` float NOT NULL,
    `bandwidth` float NOT NULL,
    `ip_address` varchar(45) NOT NULL,
    `map_name` varchar(32) DEFAULT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`id`),
    KEY `connection_type` (`connection_type`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Alert management (critical only)
CREATE TABLE IF NOT EXISTS `analytics_alerts` (
    `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `alert_type` varchar(32) NOT NULL,
    `severity` tinyint(3) UNSIGNED NOT NULL,
    `message` text NOT NULL,
    `source` varchar(32) NOT NULL,
    `acknowledged` boolean NOT NULL DEFAULT FALSE,
    `acknowledged_by` varchar(32) DEFAULT NULL,
    `acknowledged_time` datetime DEFAULT NULL,
    `timestamp` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `additional_data` json DEFAULT NULL,
    PRIMARY KEY (`id`),
    KEY `alert_type` (`alert_type`),
    KEY `severity` (`severity`),
    KEY `timestamp` (`timestamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Views for analysis
CREATE OR REPLACE VIEW `vw_player_activity` AS
SELECT 
    DATE(timestamp) as date,
    action_type,
    COUNT(*) as action_count,
    COUNT(DISTINCT account_id) as unique_players
FROM analytics_player_behavior
GROUP BY DATE(timestamp), action_type;

CREATE OR REPLACE VIEW `vw_economy_summary` AS
SELECT 
    DATE(timestamp) as date,
    transaction_type,
    item_id,
    COUNT(*) as transaction_count,
    SUM(amount) as total_amount,
    AVG(price) as avg_price,
    MIN(price) as min_price,
    MAX(price) as max_price
FROM analytics_economy
GROUP BY DATE(timestamp), transaction_type, item_id;