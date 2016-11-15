--
-- Table structure for table `storage_1`
--

CREATE TABLE IF NOT EXISTS `storage_1` (
  `id` int(11) unsigned NOT NULL auto_increment,
  `account_id` int(11) unsigned NOT NULL default '0',
  `nameid` smallint(5) unsigned NOT NULL default '0',
  `amount` smallint(11) unsigned NOT NULL default '0',
  `equip` int(11) unsigned NOT NULL default '0',
  `identify` smallint(6) unsigned NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `card0` smallint(5) unsigned NOT NULL default '0',
  `card1` smallint(5) unsigned NOT NULL default '0',
  `card2` smallint(5) unsigned NOT NULL default '0',
  `card3` smallint(5) unsigned NOT NULL default '0',
  `option_id0` smallint(5) unsigned NOT NULL default '0',
  `option_val0` smallint(5) unsigned NOT NULL default '0',
  `option_parm0` tinyint(3) unsigned NOT NULL default '0',
  `option_id1` smallint(5) unsigned NOT NULL default '0',
  `option_val1` smallint(5) unsigned NOT NULL default '0',
  `option_parm1` tinyint(3) unsigned NOT NULL default '0',
  `option_id2` smallint(5) unsigned NOT NULL default '0',
  `option_val2` smallint(5) unsigned NOT NULL default '0',
  `option_parm2` tinyint(3) unsigned NOT NULL default '0',
  `option_id3` smallint(5) unsigned NOT NULL default '0',
  `option_val3` smallint(5) unsigned NOT NULL default '0',
  `option_parm3` tinyint(3) unsigned NOT NULL default '0',
  `option_id4` smallint(5) unsigned NOT NULL default '0',
  `option_val4` smallint(5) unsigned NOT NULL default '0',
  `option_parm4` tinyint(3) unsigned NOT NULL default '0',
  `expire_time` int(11) unsigned NOT NULL default '0',
  `bound` tinyint(3) unsigned NOT NULL default '0',
  `unique_id` bigint(20) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;
