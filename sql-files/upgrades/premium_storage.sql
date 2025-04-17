--
-- Table structure for table `vip_storage`
--

CREATE TABLE IF NOT EXISTS `vip_storage` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT '0',
  `nameid` int unsigned NOT NULL DEFAULT '0',
  `amount` smallint unsigned NOT NULL DEFAULT '0',
  `equip` int unsigned NOT NULL DEFAULT '0',
  `identify` smallint unsigned NOT NULL DEFAULT '0',
  `refine` tinyint unsigned NOT NULL DEFAULT '0',
  `attribute` tinyint unsigned NOT NULL DEFAULT '0',
  `card0` int unsigned NOT NULL DEFAULT '0',
  `card1` int unsigned NOT NULL DEFAULT '0',
  `card2` int unsigned NOT NULL DEFAULT '0',
  `card3` int unsigned NOT NULL DEFAULT '0',
  `option_id0` smallint unsigned NOT NULL DEFAULT '0',
  `option_val0` smallint unsigned NOT NULL DEFAULT '0',
  `option_parm0` tinyint unsigned NOT NULL DEFAULT '0',
  `option_id1` smallint unsigned NOT NULL DEFAULT '0',
  `option_val1` smallint unsigned NOT NULL DEFAULT '0',
  `option_parm1` tinyint unsigned NOT NULL DEFAULT '0',
  `option_id2` smallint unsigned NOT NULL DEFAULT '0',
  `option_val2` smallint unsigned NOT NULL DEFAULT '0',
  `option_parm2` tinyint unsigned NOT NULL DEFAULT '0',
  `option_id3` smallint unsigned NOT NULL DEFAULT '0',
  `option_val3` smallint unsigned NOT NULL DEFAULT '0',
  `option_parm3` tinyint unsigned NOT NULL DEFAULT '0',
  `option_id4` smallint unsigned NOT NULL DEFAULT '0',
  `option_val4` smallint unsigned NOT NULL DEFAULT '0',
  `option_parm4` tinyint unsigned NOT NULL DEFAULT '0',
  `expire_time` int unsigned NOT NULL DEFAULT '0',
  `bound` tinyint unsigned NOT NULL DEFAULT '0',
  `unique_id` bigint unsigned NOT NULL DEFAULT '0',
  `enchantgrade` tinyint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;
