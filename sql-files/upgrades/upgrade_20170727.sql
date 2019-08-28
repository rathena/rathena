ALTER TABLE `char`
	ADD COLUMN `title_id` int(11) unsigned NOT NULL default '0' AFTER `clan_id`;

--
-- Table structure for table `achievement`
--

CREATE TABLE IF NOT EXISTS `achievement` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `id` bigint(11) unsigned NOT NULL,
  `count1` mediumint(8) unsigned NOT NULL default '0',
  `count2` mediumint(8) unsigned NOT NULL default '0',
  `count3` mediumint(8) unsigned NOT NULL default '0',
  `count4` mediumint(8) unsigned NOT NULL default '0',
  `count5` mediumint(8) unsigned NOT NULL default '0',
  `count6` mediumint(8) unsigned NOT NULL default '0',
  `count7` mediumint(8) unsigned NOT NULL default '0',
  `count8` mediumint(8) unsigned NOT NULL default '0',
  `count9` mediumint(8) unsigned NOT NULL default '0',
  `count10` mediumint(8) unsigned NOT NULL default '0',
  `completed` datetime,
  `rewarded` datetime,
  PRIMARY KEY (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;
