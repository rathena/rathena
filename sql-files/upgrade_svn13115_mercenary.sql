--
-- Table structure for table `mercenary`
--

CREATE TABLE IF NOT EXISTS `mercenary` (
  `mer_id` int(11) unsigned NOT NULL auto_increment,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL default '0',
  `hp` int(12) NOT NULL default '1',
  `sp` int(12) NOT NULL default '1',
  `kill_counter` int(11) NOT NULL,
  `life_time` int(11) NOT NULL default '0',
  PRIMARY KEY  (`mer_id`)
) ENGINE=MyISAM;
