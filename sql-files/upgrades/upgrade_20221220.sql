--
-- Table structure for table `party_bookings`
--

CREATE TABLE IF NOT EXISTS `party_bookings` (
  `world_name` varchar(32) NOT NULL,
  `account_id` int unsigned NOT NULL,
  `char_id` int unsigned NOT NULL,
  `char_name` varchar(23) NOT NULL,
  `purpose` smallint unsigned NOT NULL DEFAULT '0',
  `assist` tinyint unsigned NOT NULL DEFAULT '0',
  `damagedealer` tinyint unsigned NOT NULL DEFAULT '0',
  `healer` tinyint unsigned NOT NULL DEFAULT '0',
  `tanker` tinyint unsigned NOT NULL DEFAULT '0',
  `minimum_level` smallint unsigned NOT NULL,
  `maximum_level` smallint unsigned NOT NULL,
  `comment` varchar(255) NOT NULL DEFAULT '',
  `created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`world_name`, `account_id`, `char_id`)
) ENGINE=MyISAM;
