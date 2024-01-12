CREATE TABLE IF NOT EXISTS `warp_unlocker_locations` (
  `char_id` int(11) unsigned NOT NULL,
  `mapname` varchar(24) NOT NULL,
  PRIMARY KEY (`char_id`,`mapname`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;
