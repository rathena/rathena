
--
-- Table structure for table `guild_emblems`
--

CREATE TABLE IF NOT EXISTS `guild_emblems` (
  `guild_id` int(11) unsigned NOT NULL,
  `world_name` varchar(32) NOT NULL,
  `file_type` varchar(255) NOT NULL,
  `file_data` blob,
  `version` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`guild_id`, `world_name`)
) ENGINE=MyISAM;

--
-- Table structure for table `user_configs`
--

CREATE TABLE IF NOT EXISTS `user_configs` (
  `account_id` int(11) unsigned NOT NULL,
  `world_name` varchar(32) NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`account_id`, `world_name`)
) ENGINE=MyISAM;


--
-- Table structure for table `char_configs`
--

CREATE TABLE IF NOT EXISTS `char_configs` (
  `account_id` int(11) unsigned NOT NULL,
  `char_id` int(11) unsigned NOT NULL,
  `world_name` varchar(32) NOT NULL,
  `data` longtext NOT NULL,
  PRIMARY KEY (`account_id`, `char_id`, `world_name`)
) ENGINE=MyISAM;

--
-- Table structure for table `merchant_configs`
--

CREATE TABLE IF NOT EXISTS `merchant_configs` (
  `account_id` int(11) unsigned NOT NULL,
  `char_id` INT(11) UNSIGNED NOT NULL,
  `world_name` varchar(32) NOT NULL,
  `store_type` tinyint(3) UNSIGNED NOT NULL DEFAULT 0,
  `data` longtext NOT NULL,
  PRIMARY KEY (`account_id`, `char_id`, `world_name`)
) ENGINE=MyISAM;
