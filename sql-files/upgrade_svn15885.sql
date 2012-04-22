ALTER TABLE `char` ADD COLUMN `elemental_id` int(11) unsigned NOT NULL default '0';

--
-- Table structure for table `elemental`
--

CREATE TABLE IF NOT EXISTS `elemental` (
  `ele_id` int(11) unsigned NOT NULL auto_increment,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL default '0',
  `mode` int(11) unsigned NOT NULL default '1',
  `hp` int(12) NOT NULL default '1',
  `sp` int(12) NOT NULL default '1',
  `max_hp` mediumint(8) unsigned NOT NULL default '0',
  `max_sp` mediumint(6) unsigned NOT NULL default '0',
  `str` smallint(4) unsigned NOT NULL default '0',
  `agi` smallint(4) unsigned NOT NULL default '0',
  `vit` smallint(4) unsigned NOT NULL default '0',
  `int` smallint(4) unsigned NOT NULL default '0',
  `dex` smallint(4) unsigned NOT NULL default '0',
  `luk` smallint(4) unsigned NOT NULL default '0',
  `life_time` int(11) NOT NULL default '0',
  PRIMARY KEY  (`ele_id`)
) ENGINE=MyISAM;
