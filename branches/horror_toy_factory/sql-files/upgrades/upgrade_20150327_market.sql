--
-- Table `market` for market shop persistency
--

CREATE TABLE IF NOT EXISTS `market` (
  `name` varchar(32) NOT NULL DEFAULT '',
  `nameid` SMALLINT(5) UNSIGNED NOT NULL,
  `price` INT(11) UNSIGNED NOT NULL,
  `amount` SMALLINT(5) UNSIGNED NOT NULL,
  `flag` TINYINT(2) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY  (`name`,`nameid`)
) ENGINE = MyISAM;
