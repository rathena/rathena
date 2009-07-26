--
-- Table structure for table `questlog`
--

CREATE TABLE IF NOT EXISTS `questlog` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `quest_id` int(10) unsigned NOT NULL,
  `state` enum('2','1','0') NOT NULL default '0',
  `time` int(11) unsigned NOT NULL default '0',
  `mob1` mediumint(9) unsigned NOT NULL default '0',
  `count1` mediumint(8) unsigned NOT NULL default '0',
  `mob2` mediumint(9) unsigned NOT NULL default '0',
  `count2` mediumint(8) unsigned NOT NULL default '0',
  `mob3` mediumint(9) unsigned NOT NULL default '0',
  `count3` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  USING BTREE (`char_id`,`quest_id`)
) ENGINE=MyISAM;

DROP TABLE `quest_objective`;
DROP TABLE `quest`;