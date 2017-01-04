DROP TABLE IF EXISTS `cashlog`;

CREATE TABLE `cashlog` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `time` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `char_id` int(11) NOT NULL DEFAULT '0',
  `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','$') NOT NULL DEFAULT 'S',
  `cash_type` enum('O','K','C') NOT NULL DEFAULT 'O',
  `amount` int(11) NOT NULL DEFAULT '0',
  `map` varchar(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX `type` (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

DROP TABLE IF EXISTS `item_cash_db`;
CREATE TABLE `item_cash_db` (
  `tab` smallint(6) NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `price` mediumint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`tab`,`item_id`)
) ENGINE=MyISAM;

DROP TABLE IF EXISTS `item_cash_db2`;
CREATE TABLE `item_cash_db2` (
  `tab` smallint(6) NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `price` mediumint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`tab`,`item_id`)
) ENGINE=MyISAM;