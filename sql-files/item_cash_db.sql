DROP TABLE IF EXISTS `item_cash_db`;
CREATE TABLE `item_cash_db` (
  `tab` smallint(6) NOT NULL,
  `item_id` int(10) unsigned NOT NULL,
  `price` mediumint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`tab`,`item_id`)
) ENGINE=MyISAM;
