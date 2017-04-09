ALTER TABLE `char`
	ADD COLUMN `title_id` int(11) unsigned NOT NULL default '0' AFTER `clan_id`;

--
-- Table structure for table `achievement`
--
-- `char_id`,`id`,`score`,`complete`,`count1`,`count2`,`count3`,`count4`,`count5`,`count6`,`count7`,`count8`,`count9`,`count10`,`completeDate`,`rewardDate`

CREATE TABLE IF NOT EXISTS `achievement` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `id` bigint(11) unsigned NOT NULL,
  `complete` enum('0','1') NOT NULL default '0',
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
  `completeDate` int(11) unsigned NOT NULL default '0',
  `gotReward` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY (`char_id`,`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;
