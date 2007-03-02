
CREATE TABLE `chatlog` (
  `id` bigint(20) NOT NULL auto_increment,
  `time` datetime NOT NULL default '0000-00-00 00:00:00',
  `type` enum('W','P','G') NOT NULL default 'W',
  `type_id` int(11) NOT NULL default '0',
  `src_charid` int(11) NOT NULL default '0',
  `src_accountid` int(11) NOT NULL default '0',
  `src_map` varchar(17) NOT NULL default '',
  `src_map_x` tinyint(4) NOT NULL default '0',
  `src_map_y` tinyint(4) NOT NULL default '0',
  `dst_charname` varchar(25) NOT NULL default '',
  `message` varchar(150) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 ;



