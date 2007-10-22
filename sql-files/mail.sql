DROP TABLE IF EXISTS `mail`;
CREATE TABLE `mail` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `send_name` tinytext NOT NULL DEFAULT '',
  `send_id` int(11) unsigned NOT NULL default '0',
  `dest_name` tinytext NOT NULL DEFAULT '',
  `dest_id` int(11) unsigned NOT NULL default '0',
  `title` tinytext NOT NULL DEFAULT '',
  `message` text NOT NULL DEFAULT '',
  `time` int(11) unsigned NOT NULL default '0',
  `read_flag` tinyint(1) unsigned NOT NULL default '0',
  `zeny` int(11) unsigned NOT NULL default '0',
  `nameid` int(11) unsigned NOT NULL default '0',
  `amount` int(11) unsigned NOT NULL default '0',
  `refine` tinyint(3) unsigned NOT NULL default '0',
  `attribute` tinyint(4) unsigned NOT NULL default '0',
  `identify` smallint(6) NOT NULL default '0',
  `card0` smallint(11) NOT NULL default '0',
  `card1` smallint(11) NOT NULL default '0',
  `card2` smallint(11) NOT NULL default '0',
  `card3` smallint(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM;
