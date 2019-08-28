DROP TABLE IF EXISTS `item_cash_db2`;
CREATE TABLE `item_cash_db2` (
  `tab` smallint(6) NOT NULL,
  `item_id` smallint(5) unsigned NOT NULL,
  `price` mediumint(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`tab`,`item_id`)
) ENGINE=MyISAM;
